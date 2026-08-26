"""Control-video conversions (pure functions) and ffmpeg encoders.

Depth
    CARLA encodes depth in 24 bits over BGRA: ``d_m = 1000 * (R + 256 G + 65536 B) / (2^24 - 1)``.
    Cosmos depth branches were trained on Depth-Anything output, i.e. *relative
    inverse depth* normalised per clip to [0, 255] with **near = bright**.  The
    default ``inverse`` mode reproduces that (``1/d`` min-max normalised over the
    whole clip); ``linear`` gives ``1 - (d - dmin) / (dmax - dmin)`` for
    comparison.  Output is 8-bit grey replicated to 3 channels.

Segmentation
    Instance colouring by default: every instance id gets a colour from a fixed
    seeded palette (deterministic across runs and machines); id 0 stays black.
    Semantic option uses CARLA's CityScapes palette.  Both operate on the raw
    BGRA buffers (tag in the R channel = byte 2; instance id in G, B).

Encoding
    RGB: ``libx264 -crf 14 yuv420p``.  Controls: ``libx264 -qp 0 -pix_fmt yuv444p``.
    Lossless-mode H.264 4:4:4 keeps flat segmentation colours and depth ramps
    intact at edges (no chroma subsampling bleed) while remaining decodable by
    every reader the backends use (imageio-ffmpeg, PyAV, decord).  FFV1 would be
    strictly lossless but is not reliably readable from ``.mp4`` by those
    decoders, so it was not chosen.
"""

from __future__ import annotations

import json
import logging
import subprocess
from pathlib import Path
from typing import Iterable

import cv2
import numpy as np

log = logging.getLogger(__name__)

DEPTH_MAX_M = 1000.0
_DEPTH_SCALE = DEPTH_MAX_M / (256.0 ** 3 - 1.0)

# CARLA CityObjectLabel -> CityScapes RGB (LibCarla/source/carla/image/CityScapesPalette.h).
CITYSCAPES_PALETTE = np.zeros((256, 3), dtype=np.uint8)
CITYSCAPES_PALETTE[:30] = [
    (0, 0, 0), (128, 64, 128), (244, 35, 232), (70, 70, 70), (102, 102, 156), (190, 153, 153),
    (153, 153, 153), (250, 170, 30), (220, 220, 0), (107, 142, 35), (152, 251, 152), (70, 130, 180),
    (220, 20, 60), (255, 0, 0), (0, 0, 142), (0, 0, 70), (0, 60, 100), (0, 80, 100), (0, 0, 230),
    (119, 11, 32), (110, 190, 160), (170, 120, 50), (55, 90, 80), (45, 60, 150), (157, 234, 50),
    (81, 0, 81), (150, 100, 100), (230, 150, 140), (180, 165, 180), (180, 130, 70),
]

TAG_SKY = 11


def _instance_palette(seed: int = 20260826) -> np.ndarray:
    rng = np.random.default_rng(seed)
    lut = rng.integers(48, 256, size=(65536, 3), dtype=np.int64).astype(np.uint8)
    lut[0] = 0
    return lut


INSTANCE_PALETTE = _instance_palette()
"""Deterministic 16-bit instance id -> RGB lookup (id 0 = black)."""


# ----------------------------------------------------------------------------- raw buffers

def bgra_view(image) -> np.ndarray:
    """``(H, W, 4)`` uint8 view of a ``carla.Image`` raw buffer (BGRA)."""
    return np.frombuffer(image.raw_data, dtype=np.uint8).reshape(image.height, image.width, 4)


def rgb_from_bgra(bgra: np.ndarray) -> np.ndarray:
    """BGRA -> contiguous RGB."""
    return np.ascontiguousarray(bgra[:, :, 2::-1])


def depth_to_metres(bgra: np.ndarray) -> np.ndarray:
    """Decode CARLA's 24-bit depth (BGRA buffer) to float32 metres."""
    b = bgra[:, :, 0].astype(np.float64)
    g = bgra[:, :, 1].astype(np.float64)
    r = bgra[:, :, 2].astype(np.float64)
    return ((r + 256.0 * g + 65536.0 * b) * _DEPTH_SCALE).astype(np.float32)


def semantic_tags(bgra: np.ndarray) -> np.ndarray:
    """Semantic tag per pixel (R channel)."""
    return bgra[:, :, 2]


def instance_ids(bgra: np.ndarray) -> np.ndarray:
    """Instance id per pixel (``G << 8 | B``), uint16."""
    return (bgra[:, :, 1].astype(np.uint16) << 8) | bgra[:, :, 0].astype(np.uint16)


# ----------------------------------------------------------------------------- depth

def depth_clip_range(depths: Iterable[np.ndarray], mode: str = "inverse",
                     min_depth_m: float = 0.1) -> tuple[float, float]:
    """Per-clip (lo, hi) of the quantity that gets normalised (inverse depth or depth)."""
    lo, hi = np.inf, -np.inf
    for d in depths:
        q = _depth_quantity(d, mode, min_depth_m)
        lo = min(lo, float(q.min()))
        hi = max(hi, float(q.max()))
    if not np.isfinite(lo) or not np.isfinite(hi):
        raise ValueError("no depth frames")
    return lo, hi


def normalise_depth(depth_m: np.ndarray, clip_range: tuple[float, float], mode: str = "inverse",
                    min_depth_m: float = 0.1) -> np.ndarray:
    """Depth (metres) -> 8-bit 3-channel control frame, near = bright."""
    lo, hi = clip_range
    q = _depth_quantity(depth_m, mode, min_depth_m)
    span = hi - lo if hi > lo else 1.0
    u = (q - lo) / span
    if mode == "linear":
        u = 1.0 - u
    grey = np.clip(np.round(u * 255.0), 0, 255).astype(np.uint8)
    return np.repeat(grey[:, :, None], 3, axis=2)


def _depth_quantity(depth_m: np.ndarray, mode: str, min_depth_m: float) -> np.ndarray:
    if mode == "inverse":
        return 1.0 / np.maximum(depth_m, min_depth_m)
    if mode == "linear":
        return depth_m.astype(np.float64)
    raise ValueError(f"unknown depth mode '{mode}' (inverse|linear)")


# ----------------------------------------------------------------------------- segmentation

def colourise_instances(ids: np.ndarray, palette: np.ndarray = INSTANCE_PALETTE) -> np.ndarray:
    """Instance ids -> flat RGB colours."""
    return palette[ids]


def colourise_semantic(tags: np.ndarray, palette: np.ndarray = CITYSCAPES_PALETTE) -> np.ndarray:
    """Semantic tags -> CityScapes RGB."""
    return palette[tags]


# ----------------------------------------------------------------------------- edge

def masked_canny(rgb: np.ndarray, tags: np.ndarray | None = None, exclude_tags: Iterable[int] = (TAG_SKY,),
                 low: int = 100, high: int = 200) -> np.ndarray:
    """Canny edges (Transfer 2.5 ``medium`` preset), zeroed where the semantic tag is excluded."""
    grey = cv2.cvtColor(rgb, cv2.COLOR_RGB2GRAY)
    edges = cv2.Canny(grey, low, high)
    if tags is not None:
        mask = np.isin(tags, list(exclude_tags))
        edges[mask] = 0
    return np.repeat(edges[:, :, None], 3, axis=2)


# ----------------------------------------------------------------------------- ffmpeg

ENCODER_ARGS: dict[str, list[str]] = {
    "rgb": ["-c:v", "libx264", "-preset", "medium", "-crf", "14", "-pix_fmt", "yuv420p"],
    "control": ["-c:v", "libx264", "-preset", "veryfast", "-qp", "0", "-pix_fmt", "yuv444p"],
}


class VideoWriter:
    """Stream RGB uint8 frames into an ffmpeg H.264 encoder.

    ``kind`` selects the encoder profile (``rgb`` or ``control``).  Use as a
    context manager; ``close`` waits for ffmpeg and raises on a non-zero exit.
    """

    def __init__(self, path: str | Path, fps: int, width: int, height: int, kind: str = "rgb",
                 ffmpeg: str = "ffmpeg") -> None:
        if kind not in ENCODER_ARGS:
            raise ValueError(f"unknown encoder kind '{kind}'")
        self.path = Path(path)
        self.width, self.height = width, height
        self.frames = 0
        cmd = [ffmpeg, "-hide_banner", "-loglevel", "error", "-y",
               "-f", "rawvideo", "-pix_fmt", "rgb24", "-s", f"{width}x{height}", "-r", str(fps),
               "-i", "pipe:0", *ENCODER_ARGS[kind], "-movflags", "+faststart", str(self.path)]
        self._proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stderr=subprocess.PIPE)

    def write(self, frame: np.ndarray) -> None:
        """Append one ``(H, W, 3)`` uint8 RGB frame."""
        if frame.shape != (self.height, self.width, 3) or frame.dtype != np.uint8:
            raise ValueError(f"frame must be ({self.height}, {self.width}, 3) uint8, got {frame.shape} {frame.dtype}")
        assert self._proc.stdin is not None
        self._proc.stdin.write(np.ascontiguousarray(frame).tobytes())
        self.frames += 1

    def close(self) -> None:
        """Finish the stream; raises ``RuntimeError`` if ffmpeg failed."""
        if self._proc.stdin is None or self._proc.stdin.closed:
            return
        _, err = self._proc.communicate()  # flushes and closes stdin -> EOF for ffmpeg
        if self._proc.returncode != 0:
            raise RuntimeError(f"ffmpeg failed for {self.path}: {err.decode(errors='replace')}")

    def __enter__(self) -> "VideoWriter":
        return self

    def __exit__(self, *exc) -> None:
        self.close()


def encode_frames(path: str | Path, frames: Iterable[np.ndarray], fps: int, kind: str) -> int:
    """Encode an iterable of RGB frames; returns the number of frames written."""
    it = iter(frames)
    first = next(it)
    with VideoWriter(path, fps, first.shape[1], first.shape[0], kind) as w:
        w.write(first)
        for f in it:
            w.write(f)
        return w.frames


def probe_video(path: str | Path, ffprobe: str = "ffprobe") -> dict:
    """``{"frames", "width", "height", "fps"}`` of a video via ffprobe (counts frames)."""
    cmd = [ffprobe, "-v", "error", "-select_streams", "v:0", "-count_frames",
           "-show_entries", "stream=width,height,r_frame_rate,nb_read_frames", "-of", "json", str(path)]
    out = subprocess.run(cmd, capture_output=True, text=True, check=True).stdout
    s = json.loads(out)["streams"][0]
    num, den = s["r_frame_rate"].split("/")
    return {"frames": int(s["nb_read_frames"]), "width": int(s["width"]), "height": int(s["height"]),
            "fps": float(num) / float(den)}
