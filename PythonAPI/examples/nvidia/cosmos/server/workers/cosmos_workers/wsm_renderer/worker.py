"""World-scenario renderer worker: ClipGT Parquet scene package -> per-camera control videos.

Wraps NVIDIA's ``scripts/generate_control_videos.py`` pipeline from
cosmos-transfer2.5 (``load_scene`` -> ``convert_scene_data_for_rendering`` ->
``render_multi_camera_tiled``, moderngl/EGL) in-process, in the ``transfer25``
venv.  The renderer always produces 1280x720 @ 30 fps (``SETTINGS``); the
worker then decimates/trims to the clip's fps and frame count so the result
matches the RGB/other controls of the job frame for frame.

Used by the scheduler for ``hdmap_bbox`` (Transfer 2.5 AV) and ``wsm``
(Cosmos 3) controls supplied as a scene package; it is not a generation
backend (it serves the pseudo-backend id ``wsm-renderer``).

Render job::

    {"job_id": "...", "scene_dir": "/state/jobs/<id>/inputs/scene_hdmap_bbox",
     "cameras": ["camera:front:wide:120fov", ...], "fps": 30, "frames": 87,
     "out_dir": "/state/jobs/<id>/inputs/rendered_hdmap_bbox"}

Returns one ``<camera_canonical>.mp4`` per camera (``kind: control``).
"""

from __future__ import annotations

import argparse
import logging
import os
import re
import shutil
import tempfile
import time
from pathlib import Path
from typing import Any

from ..common import video
from ..common.base import RunContext, RunResult, Worker

log = logging.getLogger("cosmos_worker.wsm_renderer")

RENDER_FPS = 30
RENDER_HW = (720, 1280)
KNOWN_CAMERAS = ("camera:front:wide:120fov", "camera:cross:left:120fov", "camera:cross:right:120fov",
                 "camera:rear:left:70fov", "camera:rear:right:70fov", "camera:rear:tele:30fov",
                 "camera:front:tele:30fov")


def _canonical(name: str) -> str:
    return name.replace(":", "_")


class Renderer:
    info: dict[str, Any] = {}

    def render(self, scene_dir: Path, cameras: list[str], out_dir: Path) -> dict[str, Path]:
        """Render all ``cameras`` at 30 fps; return ``camera -> mp4``."""
        raise NotImplementedError


class NvidiaRenderer(Renderer):
    def __init__(self) -> None:
        from cosmos_transfer2._src.imaginaire.auxiliary.world_scenario.data_loaders import load_scene
        from cosmos_transfer2._src.imaginaire.auxiliary.world_scenario.rendering.config import SETTINGS
        from cosmos_transfer2._src.imaginaire.auxiliary.world_scenario.scripts.local import (
            convert_scene_data_for_rendering,
            render_multi_camera_tiled,
        )

        self._load_scene = load_scene
        self._convert = convert_scene_data_for_rendering
        self._render = render_multi_camera_tiled
        self._settings = SETTINGS
        self.info = {"renderer": "cosmos-transfer2.5 world_scenario", "pose_fps": SETTINGS["INPUT_POSE_FPS"],
                     "resolution_hw": list(SETTINGS["RESIZE_RESOLUTION"])}

    def check(self) -> None:
        """Create and drop a standalone EGL context (fails fast on nodes without EGL)."""
        import moderngl

        ctx = moderngl.create_standalone_context(backend="egl")
        ctx.release()

    def render(self, scene_dir: Path, cameras: list[str], out_dir: Path) -> dict[str, Path]:
        # NVIDIA's clipgt loader keys camera models by the underscore canonical name
        # (clipgt_loader._load_camera_calibrations); the rig speaks colon form -> KeyError otherwise
        canon = [_canonical(c) for c in cameras]
        scene = self._load_scene(scene_dir, camera_names=canon, max_frames=-1,
                                 input_pose_fps=self._settings["INPUT_POSE_FPS"],
                                 resize_resolution_hw=self._settings["RESIZE_RESOLUTION"], clip_id=None)
        models, poses = self._convert(scene, canon, self._settings["RESIZE_RESOLUTION"])
        self._render(models, poses, scene, canon, str(out_dir), scene.scene_id, max_frames=-1, chunk_output=False,
                     overlay_camera=False, alpha=0.5, clipgt_path=scene_dir, use_persistent_vbos=True,
                     multi_sample=4, simplified_output=True)
        return _collect(out_dir, cameras)


class FakeRenderer(Renderer):
    """Writes test-pattern videos named like the real renderer (tests, CPU)."""

    def __init__(self, delay: float = 0.2) -> None:
        self.delay = delay
        self.info = {"renderer": "fake", "pose_fps": RENDER_FPS, "resolution_hw": list(RENDER_HW)}

    def render(self, scene_dir: Path, cameras: list[str], out_dir: Path) -> dict[str, Path]:
        scene_id = _scene_id(scene_dir)
        n = _egomotion_rows(scene_dir) or 360  # 12 s of poses at 30 fps
        time.sleep(self.delay)
        for i, cam in enumerate(cameras):
            video.test_pattern(out_dir / f"{scene_id}.{_canonical(cam)}.mp4", frames=n, fps=RENDER_FPS,
                               size="320x176", hue=i * 40)
        return _collect(out_dir, cameras)


def _collect(out_dir: Path, cameras: list[str]) -> dict[str, Path]:
    found: dict[str, Path] = {}
    for cam in cameras:
        c = _canonical(cam)
        hits = sorted(out_dir.rglob(f"*.{c}.mp4")) or sorted(out_dir.rglob(f"*{c}*.mp4"))
        if not hits:
            raise RuntimeError(f"renderer produced no video for {cam} in {out_dir} (have: {sorted(p.name for p in out_dir.rglob('*.mp4'))})")
        found[cam] = hits[0]
    return found


def _scene_id(scene_dir: Path) -> str:
    for p in scene_dir.glob("*.egomotion_estimate.parquet"):
        return p.name.split(".")[0]
    return scene_dir.name


def _egomotion_rows(scene_dir: Path) -> int | None:
    """Row count of the egomotion table when pyarrow is available (fake renderer sizing)."""
    try:
        import pyarrow.parquet as pq

        for p in scene_dir.glob("*.egomotion_estimate.parquet"):
            return pq.ParquetFile(p).metadata.num_rows
    except Exception:  # noqa: BLE001 - optional dependency / placeholder files
        return None
    return None


class WsmRendererWorker(Worker):
    name = "wsm-renderer"
    backends = ("wsm-renderer",)

    def __init__(self, args: argparse.Namespace) -> None:
        super().__init__(args)
        self.renderer: Renderer | None = None
        self.scratch = Path(args.scratch_dir or tempfile.mkdtemp(prefix="wsm-render-"))

    def load(self) -> None:
        self.scratch.mkdir(parents=True, exist_ok=True)
        if self.args.engine == "fake":
            self.renderer = FakeRenderer(self.args.fake_delay)
        else:
            os.environ.setdefault("PYOPENGL_PLATFORM", "egl")
            self.renderer = NvidiaRenderer()
        log.info("renderer ready: %s", self.renderer.info)

    def smoke(self) -> None:
        if isinstance(self.renderer, NvidiaRenderer):
            self.renderer.check()
        if self.args.smoke_scene:
            with tempfile.TemporaryDirectory(dir=self.scratch) as tmp:
                self.renderer.render(Path(self.args.smoke_scene), [KNOWN_CAMERAS[0]], Path(tmp))

    def run(self, job: dict[str, Any], ctx: RunContext) -> RunResult:
        assert self.renderer is not None
        scene_dir = Path(job["scene_dir"])
        cameras = list(job.get("cameras") or [KNOWN_CAMERAS[0]])
        unknown = [c for c in cameras if c not in KNOWN_CAMERAS]
        if unknown:
            raise ValueError(f"cameras {unknown} are not renderer slots; use one of {KNOWN_CAMERAS}")
        fps, frames = int(job["fps"]), int(job["frames"])
        out_dir = Path(job["out_dir"])
        out_dir.mkdir(parents=True, exist_ok=True)
        ctx.check_cancelled()
        ctx.progress(0.05, f"rendering {len(cameras)} camera(s) from {scene_dir.name}")
        t0 = time.monotonic()
        with tempfile.TemporaryDirectory(dir=self.scratch, prefix=f"{job['job_id']}-") as tmp:
            rendered = self.renderer.render(scene_dir, cameras, Path(tmp))
            t_render = time.monotonic() - t0
            ctx.progress(0.8, f"rendered in {t_render:.0f}s; re-timing to {fps} fps / {frames} frames")
            files = []
            for cam, src in rendered.items():
                ctx.check_cancelled()
                dest = out_dir / f"{_canonical(cam)}.mp4"
                info = video.probe(src)
                if abs(info["fps"] - fps) < 1e-3 and info["frames"] == frames:
                    shutil.copyfile(src, dest)
                elif info["frames"] == frames:
                    # NVIDIA's renderer emits one frame per camera timestamp of the scene package (the clip's
                    # own frames) but labels the file RENDER_FPS: keep every frame, rewrite the rate
                    video.resample(src, dest, fps=fps, frames=frames, src_fps=info["fps"], relabel=True)
                else:
                    if info["fps"] % fps:
                        raise ValueError(f"rendered {cam} is {info['fps']:.0f} fps with {info['frames']} frames; "
                                         f"clip fps {fps} must divide it (or match its frame count {frames})")
                    if info["frames"] * fps / info["fps"] + 1e-6 < frames:
                        raise RuntimeError(f"rendered {cam} has {info['frames']} frames at {info['fps']:.0f} fps "
                                           f"(= {int(info['frames'] * fps / info['fps'])} at {fps} fps) but the clip "
                                           f"needs {frames}; the scene's egomotion table is too short")
                    video.resample(src, dest, fps=fps, frames=frames, src_fps=info["fps"])
                files.append({"name": dest.name, "view": cam, "kind": "control"})
        ctx.progress(1.0, "done")
        return RunResult(files=files, manifest={**self.renderer.info, "render_s": t_render, "fps": fps,
                                                "frames": frames, "cameras": cameras})


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--engine", choices=["real", "fake"], default=os.environ.get("WSM_RENDERER_ENGINE", "real"))
    p.add_argument("--fake-delay", type=float, default=0.2)
    p.add_argument("--scratch-dir", default=None)
    p.add_argument("--smoke-scene", default=os.environ.get("WSM_SMOKE_SCENE"),
                   help="scene package rendered (1 camera) as the smoke sample")
    return p
