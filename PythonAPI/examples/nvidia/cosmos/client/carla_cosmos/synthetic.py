"""Synthetic clips (ffmpeg test pattern) for smoke tests and CI — no CARLA needed."""

from __future__ import annotations

import secrets
import subprocess
from pathlib import Path

from .clip import SCENE_DIR, Clip, video_file_name
from .contracts import AV_CAMERAS, CameraManifest, ClipManifest, RigManifest, canonical_camera_name

SCENE_TABLES = ("egomotion_estimate", "obstacle", "calibration_estimate", "lane_line", "road_boundary",
                "crosswalk", "pole", "traffic_light", "traffic_sign", "wait_line", "road_marking")


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
    if scene:
        sd = path / SCENE_DIR
        sd.mkdir(exist_ok=True)
        for table in SCENE_TABLES:
            (sd / f"{clip_id}.{table}.parquet").write_bytes(b"PAR1PAR1")  # placeholder, not a real table
        for cam in cameras:
            (sd / f"{clip_id}.{canonical_camera_name(cam)}.json").write_text("{}")
        scene_dir = SCENE_DIR
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
    clip.save_manifest()
    return clip


def av7_clip(out_dir: str | Path, seconds: int = 3, **kw) -> Clip:
    """A 7-camera clip valid for ``transfer2.5-av`` (30 fps, 29+28k frames at 10 fps -> 87 at 30)."""
    frames = kw.pop("frames", 3 * (29 + 28 * (seconds - 1)))
    return make_clip(out_dir, frames=frames, fps=30, cameras=list(AV_CAMERAS), scene=True, **kw)
