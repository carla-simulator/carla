"""Frame-exact multi-camera capture from a live or replayed CARLA world.

The world runs in synchronous mode at ``fixed_delta_seconds = 1/fps`` (previous
settings are restored on exit).  Every sensor image is matched to the frame id
returned by ``world.tick()``; a missing or skipped frame raises
:class:`FrameDesyncError` — frames are never dropped silently.  Before frame 0
the sensors run for a settle tick plus ``warmup`` ticks whose images are
consumed and discarded frame by frame, so the renderer's temporal history
(TAA, Lumen) has converged when the first recorded frame is rendered.
"""

from __future__ import annotations

import logging
import queue
import shutil
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable

import numpy as np

import carla

from . import controls, scene
from .clip import SCENE_DIR, Clip, video_file_name
from .contracts import BackendContract, ClipManifest, RecorderInfo
from .rig import MountedCamera, Rig, rear_axle_local_ue

log = logging.getLogger(__name__)

AOV_BLUEPRINTS = {
    "rgb": "sensor.camera.rgb",
    "depth": "sensor.camera.depth",
    "semantic": "sensor.camera.semantic_segmentation",
    "instance": "sensor.camera.instance_segmentation",
}

_WEATHER_ATTRS = ("cloudiness", "precipitation", "precipitation_deposits", "wind_intensity",
                  "sun_azimuth_angle", "sun_altitude_angle", "fog_density", "fog_distance",
                  "fog_falloff", "wetness", "scattering_intensity", "mie_scattering_scale",
                  "rayleigh_scattering_scale", "dust_storm")


class FrameDesyncError(RuntimeError):
    """A sensor did not deliver the image for the ticked frame."""


# ----------------------------------------------------------------------------- tick sources

class LiveTicks:
    """Tick a live world (Traffic Manager or user-controlled actors)."""

    def __init__(self, world: carla.World) -> None:
        self.world = world

    def __enter__(self) -> "LiveTicks":
        return self

    def __exit__(self, *exc) -> None:
        return None

    def tick(self) -> int:
        """Advance one frame; returns the frame id."""
        return self.world.tick()


class ReplayTicks:
    """Drive the same tick loop from a recorder log (``client.replay_file``).

    Enter the context, then :meth:`find_ego` (replayed actors get new ids; the
    hero is located by ``role_name``), then hand the instance to
    :class:`Capture` as ``ticks``.
    """

    def __init__(self, client: carla.Client, log_file: str, start: float = 0.0,
                 duration: float = 0.0, ego_role: str = "hero") -> None:
        self.client = client
        self.world = client.get_world()
        self.log_file = log_file
        self.start = start
        self.duration = duration
        self.ego_role = ego_role

    def __enter__(self) -> "ReplayTicks":
        self.client.set_replayer_time_factor(1.0)
        info = self.client.replay_file(self.log_file, self.start, self.duration, 0, False, False)
        log.info("replay started: %s", info.strip().splitlines()[0] if info.strip() else self.log_file)
        return self

    def __exit__(self, *exc) -> None:
        self.client.stop_replayer(False)

    def tick(self) -> int:
        """Advance one frame of the replay; returns the frame id."""
        return self.world.tick()

    def find_ego(self, settle_ticks: int = 2) -> carla.Vehicle:
        """Locate the replayed hero vehicle by role name."""
        for _ in range(settle_ticks):
            self.world.tick()
        for actor in self.world.get_actors().filter("vehicle.*"):
            if actor.attributes.get("role_name") == self.ego_role:
                return actor
        raise RuntimeError(f"no vehicle with role_name='{self.ego_role}' in the replay")


TickSource = LiveTicks | ReplayTicks


# ----------------------------------------------------------------------------- capture

@dataclass
class _CameraStreams:
    """Per-camera sensors, queues and output writers."""

    mounted: MountedCamera
    sensors: dict[str, carla.Sensor] = field(default_factory=dict)
    queues: dict[str, "queue.Queue[carla.Image]"] = field(default_factory=dict)
    writers: dict[str, controls.VideoWriter] = field(default_factory=dict)
    depth_store: np.memmap | None = None


class Capture:
    """Capture a :class:`~carla_cosmos.clip.Clip` from a CARLA world.

    Parameters
    ----------
    world, ego, rig
        The world, the ego vehicle to mount ``rig`` on, and the rig.
    contract
        Optional :class:`BackendContract`; when given, ``frames``/``fps`` are
        validated against it up front (fail fast, before touching the world).
    frames, fps
        Clip length in frames and the fixed simulation rate.
    aovs
        Subset of ``{"rgb", "depth", "semantic", "instance"}``.  ``rgb`` writes
        the RGB video, ``depth`` the depth control, ``instance``/``semantic``
        the segmentation control (instance colouring preferred when both are
        captured).
    edge
        Also write a semantic-masked Canny edge control (needs ``rgb``).
    seg_mode
        ``"instance"`` (default) or ``"semantic"`` colouring for ``seg_*.mp4``.
    depth_mode
        ``"inverse"`` (Depth-Anything-like, default) or ``"linear"``.
    ticks
        Tick source; defaults to :class:`LiveTicks` on ``world``.
    warmup
        Ticks run with the sensors listening, and their images discarded,
        between the settle tick and frame 0 (default 5).  Freshly spawned
        cameras have no temporal history, so their first frames are smeared
        RGB (TAA/Lumen accumulation) and noisy depth/segmentation; the warm-up
        lets them converge.  The world (or replay) advances by these ticks
        before the clip starts.  ``0`` disables it.
    """

    def __init__(self, world: carla.World, ego: carla.Vehicle, rig: Rig,
                 contract: BackendContract | None = None, *, frames: int, fps: int = 30,
                 aovs: tuple[str, ...] = ("rgb", "depth", "semantic", "instance"),
                 edge: bool = False, seg_mode: str = "instance", depth_mode: str = "inverse",
                 ticks: TickSource | None = None, sensor_timeout: float = 30.0, warmup: int = 5) -> None:
        unknown = set(aovs) - set(AOV_BLUEPRINTS)
        if unknown:
            raise ValueError(f"unknown AOVs {sorted(unknown)}; supported: {sorted(AOV_BLUEPRINTS)}")
        if seg_mode not in ("instance", "semantic"):
            raise ValueError(f"seg_mode must be instance|semantic, not '{seg_mode}'")
        if seg_mode == "semantic" and "semantic" not in aovs and "instance" in aovs:
            raise ValueError("seg_mode='semantic' needs the 'semantic' AOV")
        if edge and "rgb" not in aovs:
            raise ValueError("edge control needs the 'rgb' AOV")
        if contract is not None:
            self._check_contract(contract, frames, fps)
        self.world = world
        self.ego = ego
        self.rig = rig
        self.frames = frames
        self.fps = fps
        self.aovs = tuple(aovs)
        self.edge = edge
        self.seg_mode = seg_mode
        self.depth_mode = depth_mode
        self.ticks: TickSource = ticks if ticks is not None else LiveTicks(world)
        self.sensor_timeout = sensor_timeout
        if warmup < 0:
            raise ValueError("warmup must be >= 0")
        self.warmup = warmup

    @staticmethod
    def _check_contract(contract: BackendContract, frames: int, fps: int) -> None:
        errors = []
        if fps not in contract.fps.source:
            errors.append(f"fps {fps} not accepted by '{contract.id}' (accepted: {contract.fps.source})")
        else:
            model_fps = contract.fps.model_fps(fps)
            rule = contract.frame_rule(set())
            n_model = frames // (fps // model_fps)
            if not rule.allows(n_model):
                near = [n * (fps // model_fps) for n in rule.nearest(n_model)]
                errors.append(f"{frames} frames invalid for '{contract.id}' ({rule.describe()} at "
                              f"{model_fps} fps); nearest valid: {near}")
        if errors:
            raise ValueError("; ".join(errors))

    # ------------------------------------------------------------------ public API
    def run(self, out_dir: str | Path, clip_id: str, *, seed: int | None = None,
            recorder: RecorderInfo | None = None, carla_version: str = "0.10.0",
            progress: Callable[[int, int], None] | None = None) -> Clip:
        """Capture ``self.frames`` frames and write the clip to ``out_dir/clip_id``."""
        clip_dir = Path(out_dir) / clip_id
        clip_dir.mkdir(parents=True, exist_ok=True)
        settings = self.world.get_settings()
        streams: list[_CameraStreams] = []
        tmp_dir = Path(tempfile.mkdtemp(prefix="carla_cosmos_depth_", dir=clip_dir))
        exporter: scene.SceneExporter | None = None
        try:
            self._enter_sync_mode(settings)
            # First tick: make the actor transform and bone transforms consistent
            # (freshly spawned actors report an identity transform until the first
            # snapshot that contains them).
            self.ticks.tick()
            axle = rear_axle_local_ue(self.ego)
            mounted = self.rig.mount_on(self.ego, axle)
            exporter = scene.SceneExporter(self.world, clip_id, self.ego, axle)
            streams = [self._open_streams(m, clip_dir, tmp_dir) for m in mounted]
            # Second settle tick: flush sensor spawns.
            settle_fid = self.ticks.tick()
            self._drain_stale(streams, settle_fid)
            # Warm-up: keep ticking with the sensors live and throw their frames away
            # until the temporal history has converged.  Every tick is drained
            # frame-exactly, so the queues hold nothing older than the last warm-up
            # tick when frame 0 is ticked.
            for _ in range(self.warmup):
                self._drain_stale(streams, self.ticks.tick())
            log.info("capturing %d frames at %d fps (%d cameras, aovs=%s) after %d warm-up tick(s)",
                     self.frames, self.fps, len(streams), ",".join(self.aovs), self.warmup)
            for i in range(self.frames):
                fid = self.ticks.tick()
                snap = self.world.get_snapshot()
                if snap.frame != fid:
                    raise FrameDesyncError(f"snapshot frame {snap.frame} != tick {fid}")
                if i == 0:
                    # anchor the FLU world frame at the first captured ego pose
                    exporter.begin()
                exporter.record_tick(snap)
                for s in streams:
                    self._process_frame(s, i, fid, first=(i == 0))
                if progress is not None:
                    progress(i + 1, self.frames)
            for s in streams:
                for w in s.writers.values():
                    w.close()
            self._encode_depth(streams, clip_dir)
            manifest = self._build_manifest(clip_id, mounted, seed, recorder, carla_version)
            exporter.write(clip_dir / SCENE_DIR, [m.manifest() for m in mounted])
            clip = Clip(path=clip_dir, manifest=manifest)
            clip.save_manifest()
            return clip
        finally:
            for s in streams:
                for sensor in s.sensors.values():
                    try:
                        sensor.stop()
                        sensor.destroy()
                    except RuntimeError as exc:  # server may already have dropped it
                        log.warning("sensor cleanup failed: %s", exc)
                for w in s.writers.values():
                    try:
                        w.close()
                    except RuntimeError:
                        pass
            shutil.rmtree(tmp_dir, ignore_errors=True)
            self.world.apply_settings(settings)

    # ------------------------------------------------------------------ internals
    def _enter_sync_mode(self, previous: carla.WorldSettings) -> None:
        s = self.world.get_settings()
        s.synchronous_mode = True
        s.fixed_delta_seconds = 1.0 / self.fps
        self.world.apply_settings(s)
        log.info("synchronous mode on (fixed_delta_seconds=%.5f); previous sync=%s",
                 1.0 / self.fps, previous.synchronous_mode)

    def _needed_aovs(self) -> list[str]:
        need = [a for a in ("rgb", "depth") if a in self.aovs]
        if "instance" in self.aovs and self.seg_mode == "instance":
            need.append("instance")
        if "semantic" in self.aovs and (self.seg_mode == "semantic" or self.edge
                                        or "instance" not in self.aovs):
            need.append("semantic")
        return need

    def _open_streams(self, m: MountedCamera, clip_dir: Path, tmp_dir: Path) -> _CameraStreams:
        s = _CameraStreams(mounted=m)
        bps = self.world.get_blueprint_library()
        cam = m.camera
        for aov in self._needed_aovs():
            bp = bps.find(AOV_BLUEPRINTS[aov])
            bp.set_attribute("image_size_x", str(cam.width))
            bp.set_attribute("image_size_y", str(cam.height))
            bp.set_attribute("fov", str(cam.hfov))
            sensor = self.world.spawn_actor(bp, m.attach, attach_to=self.ego)
            q: queue.Queue = queue.Queue()
            sensor.listen(q.put)
            s.sensors[aov] = sensor
            s.queues[aov] = q
        if "rgb" in self.aovs:
            s.writers["rgb"] = controls.VideoWriter(clip_dir / video_file_name("rgb", cam.name),
                                                    self.fps, cam.width, cam.height, "rgb")
        if self._seg_source() is not None:
            s.writers["seg"] = controls.VideoWriter(clip_dir / video_file_name("seg", cam.name),
                                                    self.fps, cam.width, cam.height, "control")
        if self.edge:
            s.writers["edge"] = controls.VideoWriter(clip_dir / video_file_name("edge", cam.name),
                                                     self.fps, cam.width, cam.height, "control")
        if "depth" in self.aovs:
            s.depth_store = np.memmap(tmp_dir / f"{cam.canonical}.f32", dtype=np.float32, mode="w+",
                                      shape=(self.frames, cam.height, cam.width))
        return s

    def _seg_source(self) -> str | None:
        if "instance" in self.aovs and self.seg_mode == "instance":
            return "instance"
        if "semantic" in self.aovs:
            return "semantic"
        return None

    def _drain_stale(self, streams: list[_CameraStreams], fid: int) -> None:
        """Consume and discard every sensor's images up to and including tick ``fid``.

        Used for the settle tick and each warm-up tick: blocks until the image
        for ``fid`` has arrived (older leftovers are dropped on the way), so the
        queues never accumulate warm-up frames and frame 0 sees only its own
        image (or, at worst, stale ones that :meth:`_get_image` drains as
        ``first``).  An image newer than ``fid`` is put back.
        """
        for s in streams:
            for aov, q in s.queues.items():
                while True:
                    try:
                        img = q.get(timeout=self.sensor_timeout)
                    except queue.Empty as exc:
                        raise FrameDesyncError(
                            f"{s.mounted.camera.name}/{aov}: no image within {self.sensor_timeout}s "
                            f"after tick {fid} (settle/warm-up)") from exc
                    if img.frame > fid:
                        q.put(img)  # produced early for the next tick; keep it
                        break
                    if img.frame == fid:
                        break

    def _get_image(self, s: _CameraStreams, aov: str, fid: int, first: bool) -> carla.Image:
        q = s.queues[aov]
        while True:
            try:
                img = q.get(timeout=self.sensor_timeout)
            except queue.Empty as exc:
                raise FrameDesyncError(
                    f"{s.mounted.camera.name}/{aov}: frame {fid} not delivered within "
                    f"{self.sensor_timeout}s") from exc
            if img.frame == fid:
                return img
            if img.frame < fid and first:
                log.debug("%s/%s: draining settle frame %d", s.mounted.camera.name, aov, img.frame)
                continue
            raise FrameDesyncError(
                f"{s.mounted.camera.name}/{aov}: expected frame {fid}, got {img.frame}")

    def _process_frame(self, s: _CameraStreams, index: int, fid: int, first: bool) -> None:
        cam = s.mounted.camera.name
        rgb = tags = None
        if "rgb" in s.sensors:
            rgb = controls.rgb_from_bgra(controls.bgra_view(self._get_image(s, "rgb", fid, first)))
        if "semantic" in s.sensors:
            tags = np.copy(controls.semantic_tags(controls.bgra_view(
                self._get_image(s, "semantic", fid, first))))
        if "rgb" in s.writers:
            assert rgb is not None
            s.writers["rgb"].write(rgb)
        if s.depth_store is not None:
            img = self._get_image(s, "depth", fid, first)
            s.depth_store[index] = controls.depth_to_metres(controls.bgra_view(img))
        if "seg" in s.writers:
            if self._seg_source() == "instance":
                ids = controls.instance_ids(controls.bgra_view(self._get_image(s, "instance", fid, first)))
                s.writers["seg"].write(controls.colourise_instances(ids))
            else:
                assert tags is not None, f"{cam}: semantic AOV required for seg"
                s.writers["seg"].write(controls.colourise_semantic(tags))
        if "edge" in s.writers:
            assert rgb is not None
            s.writers["edge"].write(controls.masked_canny(rgb, tags))

    def _encode_depth(self, streams: list[_CameraStreams], clip_dir: Path) -> None:
        """Second pass: per-camera per-clip normalisation, then encode."""
        for s in streams:
            if s.depth_store is None:
                continue
            cam = s.mounted.camera
            rng = controls.depth_clip_range(s.depth_store, mode=self.depth_mode)
            out = clip_dir / video_file_name("depth", cam.name)
            n = controls.encode_frames(
                out, (controls.normalise_depth(d, rng, mode=self.depth_mode) for d in s.depth_store),
                self.fps, "control")
            log.info("depth %s: range=%s frames=%d", cam.name, np.round(rng, 4).tolist(), n)

    def _build_manifest(self, clip_id: str, mounted: list[MountedCamera], seed: int | None,
                        recorder: RecorderInfo | None, carla_version: str) -> ClipManifest:
        weather = self.world.get_weather()
        videos: dict[str, str] = {}
        for m in mounted:
            if "rgb" in self.aovs:
                videos[f"rgb/{m.camera.canonical}"] = video_file_name("rgb", m.camera.name)
            if "depth" in self.aovs:
                videos[f"depth/{m.camera.canonical}"] = video_file_name("depth", m.camera.name)
            if self._seg_source() is not None:
                videos[f"seg/{m.camera.canonical}"] = video_file_name("seg", m.camera.name)
            if self.edge:
                videos[f"edge/{m.camera.canonical}"] = video_file_name("edge", m.camera.name)
        axle = rear_axle_local_ue(self.ego)
        return ClipManifest(
            clip_id=clip_id,
            carla_version=carla_version,
            map=self.world.get_map().name,
            weather={a: float(getattr(weather, a)) for a in _WEATHER_ATTRS if hasattr(weather, a)},
            rig=self.rig.manifest(mounted),
            fps=self.fps,
            frames=self.frames,
            ego_id=self.ego.id,
            ego_type_id=self.ego.type_id,
            rear_axle_offset_ue=[float(v) for v in axle],
            aovs=list(self.aovs) + (["edge"] if self.edge else []),
            videos=videos,
            scene_dir=SCENE_DIR,
            recorder=recorder,
            seed=seed,
        )
