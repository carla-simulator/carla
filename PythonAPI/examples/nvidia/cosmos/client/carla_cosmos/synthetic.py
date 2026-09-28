"""Synthetic clips (ffmpeg test pattern) for smoke tests and CI — no CARLA needed."""

from __future__ import annotations

import secrets
import subprocess
from pathlib import Path

import numpy as np

from .clip import SCENE_DIR, Clip, video_file_name
from .contracts import AV_CAMERAS, CameraManifest, ClipManifest, RigManifest, canonical_camera_name

SCENE_SPEED_MS = 10.0
"""Speed of the synthetic ego along +x (world FLU), in m/s."""

SCENE_T0 = 1_000_000_000
"""First timestamp of a synthetic clip, in microseconds."""


def make_clip(out_dir: str | Path, frames: int = 93, fps: int = 16, cameras: list[str] | None = None,
              width: int = 320, height: int = 176, kinds: tuple[str, ...] = ("rgb", "depth", "seg", "edge"),
              scene: bool = False, clip_id: str | None = None, ffmpeg: str = "ffmpeg") -> Clip:
    """Write a clip whose videos are ffmpeg ``testsrc`` patterns with the exact frame count."""
    cameras = cameras or ["camera:front:wide:120fov"]
    clip_id = clip_id or f"synthetic_{secrets.token_hex(3)}"
    path = Path(out_dir) / clip_id
    path.mkdir(parents=True, exist_ok=True)
    videos: dict[str, str] = {}
    for i, cam in enumerate(cameras):
        for kind in kinds:
            name = video_file_name(kind, cam)
            src = "testsrc" if kind == "rgb" else ("smptebars" if kind == "seg" else "testsrc2")
            cmd = [ffmpeg, "-y", "-loglevel", "error", "-f", "lavfi", "-i",
                   f"{src}=size={width}x{height}:rate={fps}", "-frames:v", str(frames),
                   "-vf", f"hue=h={i * 40}", "-c:v", "libx264", "-preset", "ultrafast",
                   "-pix_fmt", "yuv420p" if kind == "rgb" else "yuv444p", str(path / name)]
            subprocess.run(cmd, check=True)
            videos[f"{kind}/{canonical_camera_name(cam)}"] = name
    scene_dir = None
    manifest = ClipManifest(
        clip_id=clip_id, carla_version="synthetic", map="none", weather={},
        rig=RigManifest(name="synthetic", mount="exact", cameras=[
            CameraManifest(name=c, hfov=120.0 if "120" in c else 70.0 if "70" in c else 30.0, width=width,
                           height=height, t_flu=[1.7, 0.0, 1.5], rpy_flu=[0.0, 0.0, 0.0],
                           attach_ue={"x": 1.7, "y": 0.0, "z": 1.5, "pitch": 0.0, "yaw": 0.0, "roll": 0.0})
            for c in cameras]),
        fps=fps, frames=frames, ego_id=0, ego_type_id="synthetic", rear_axle_offset_ue=[-1.4, 0.0, 0.0],
        aovs=list(kinds), videos=videos, scene_dir=scene_dir, seed=0,
    )
    clip = Clip(path=path, manifest=manifest)
    if scene:
        write_scene(path / SCENE_DIR, manifest)
        manifest.scene_dir = SCENE_DIR
    clip.save_manifest()
    return clip


def write_scene(out_dir: str | Path, manifest: ClipManifest) -> Path:
    """A **valid** minimal ClipGT scene package for ``manifest``: the ego drives straight
    at :data:`SCENE_SPEED_MS` down a two-lane road with one car ahead and one parked.

    Stub files are not enough: NVIDIA's world-scenario renderer reads these tables with
    pyarrow, so an 8-byte placeholder fails the job with ``ArrowInvalid`` instead of
    testing the AV path.  The tables here are the real schemas
    (:mod:`carla_cosmos.clipgt`) with synthetic geometry, so a smoke test against a real
    node exercises rendering, not the loader's error handling.
    """
    from . import clipgt  # pyarrow, i.e. the 'capture' extra: only needed for a scene package

    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    clip_id = manifest.clip_id
    step = round(1_000_000 / manifest.fps)
    ts = [SCENE_T0 + i * step for i in range(manifest.frames)]
    ident = {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}

    ego = [{"key": {"clip_id": clip_id, "timestamp_micros": t},
            "egomotion_estimate": {"name": "rig",
                                   "location": clipgt.xyz(SCENE_SPEED_MS * (t - SCENE_T0) / 1e6, 0.0, 0.0),
                                   "orientation": ident},
            "version": clipgt.VERSION} for t in ts]

    def obstacle(track: str, at: list[int], centre) -> list[dict]:
        return [{"key": {"clip_id": clip_id, "timestamp_micros": t, "label_class_id": "obstacle"},
                 "obstacle": {"trackline_id": track, "center": centre(t), "size": clipgt.xyz(4.5, 2.0, 1.6),
                              "orientation": ident, "category": "automobile"},
                 "version": clipgt.VERSION} for t in at]

    obstacles = obstacle("synthetic:automobile:0", ts,
                         lambda t: clipgt.xyz(SCENE_SPEED_MS * (t - SCENE_T0) / 1e6 + 20.0, 0.0, 0.8))
    obstacles += obstacle("static:automobile:0", [ts[0], ts[-1]], lambda t: clipgt.xyz(35.0, -5.5, 0.8))

    span = SCENE_SPEED_MS * (ts[-1] - ts[0]) / 1e6
    xs = np.arange(-20.0, span + 60.0, 2.0)

    def polyline(y: float, style: str, colour: str) -> dict:
        rail = [clipgt.xyz(x, y, 0.0) for x in xs]
        return {"key": clipgt.mapkey(clip_id, "lanelines"),
                "lane_line": {"line_rail": rail, "styles": [style] * len(rail), "colors": [colour] * len(rail),
                              "left_driving_direction": [], "right_driving_direction": []},
                "version": clipgt.VERSION}

    lanes = [polyline(1.75, "SOLID_SINGLE", "YELLOW"), polyline(-1.75, "DASHED_SINGLE", "WHITE")]

    def boundary(y: float) -> dict:
        return {"key": clipgt.mapkey(clip_id, "road_boundaries"),
                "road_boundary": {"category": "curb", "location": [clipgt.xyz(x, y, 0.0) for x in xs]},
                "version": clipgt.VERSION}

    tables: dict[str, list[dict]] = {name: [] for name in clipgt.TABLES}
    tables["egomotion_estimate"] = ego
    tables["obstacle"] = obstacles
    tables["lane_line"] = lanes
    tables["road_boundary"] = [boundary(6.0), boundary(-6.0)]
    tables["calibration_estimate"] = [clipgt.calibration_row(clip_id, ts[0], manifest.rig.cameras)]
    for name, rows in tables.items():
        clipgt.write_table(out, clip_id, name, rows)
    clipgt.write_timestamps(out, clip_id, ts)
    return out


def av7_clip(out_dir: str | Path, seconds: int = 3, **kw) -> Clip:
    """A 7-camera clip valid for ``transfer2.5-av`` (30 fps, 29+28k frames at 10 fps -> 87 at 30)."""
    frames = kw.pop("frames", 3 * (29 + 28 * (seconds - 1)))
    return make_clip(out_dir, frames=frames, fps=30, cameras=list(AV_CAMERAS), scene=True, **kw)
