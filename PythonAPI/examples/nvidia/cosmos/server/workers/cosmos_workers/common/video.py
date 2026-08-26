"""ffmpeg helpers shared by workers (stdlib + the ffmpeg binary)."""

from __future__ import annotations

import json
import subprocess
from pathlib import Path


def probe(path: Path) -> dict:
    """``{"frames", "fps", "width", "height"}`` via ffprobe."""
    out = subprocess.run(["ffprobe", "-v", "error", "-select_streams", "v:0", "-count_frames",
                          "-show_entries", "stream=nb_read_frames,r_frame_rate,width,height", "-of", "json", str(path)],
                         capture_output=True, text=True, check=True).stdout
    s = json.loads(out)["streams"][0]
    num, den = s["r_frame_rate"].split("/")
    return {"frames": int(s["nb_read_frames"]), "fps": float(num) / float(den), "width": s["width"],
            "height": s["height"]}


def resample(src: Path, dest: Path, fps: int, frames: int, src_fps: float | None = None) -> None:
    """Re-time ``src`` to ``fps`` (frame decimation, no interpolation) and cut to ``frames`` frames, lossless 4:4:4."""
    src_fps = src_fps or probe(src)["fps"]
    vf = []
    if abs(src_fps - fps) > 1e-3:
        if src_fps < fps or (src_fps / fps) % 1 > 1e-6:
            raise ValueError(f"cannot decimate {src_fps:.3f} fps to {fps} fps (needs an integer factor)")
        step = int(round(src_fps / fps))
        vf.append(f"select='not(mod(n\\,{step}))',setpts=N/({fps}*TB)")
    cmd = ["ffmpeg", "-y", "-loglevel", "error", "-i", str(src)]
    if vf:
        cmd += ["-vf", ",".join(vf)]
    cmd += ["-r", str(fps), "-frames:v", str(frames), "-c:v", "libx264", "-preset", "fast", "-qp", "0",
            "-pix_fmt", "yuv444p", str(dest)]
    subprocess.run(cmd, check=True)


def test_pattern(dest: Path, frames: int, fps: int, size: str = "1280x720", hue: int = 0) -> None:
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-f", "lavfi", "-i", f"testsrc2=size={size}:rate={fps}",
                    "-vf", f"hue=h={hue}", "-frames:v", str(frames), "-c:v", "libx264", "-preset", "ultrafast",
                    "-pix_fmt", "yuv420p", str(dest)], check=True)
