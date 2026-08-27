"""Mask-out classes: remove chosen CARLA semantic classes from a clip's model inputs.

The user names semantic classes (``vehicle``, ``pedestrian``, ``vegetation``...);
every input that is *derived from the captured pixels* — depth, seg, edge,
vis/blur and the RGB video when it is uploaded — is blanked inside those
regions, so the generator has no evidence there and is free to hallucinate.

What "removed" means
--------------------
One value for every modality: **black (0, 0, 0)**.  It is not an arbitrary
choice, it is the value each Cosmos control branch already reads as *nothing
here* in a CARLA clip:

* ``depth`` is relative **inverse** depth with near = bright (``controls.py``),
  so 0 is "infinitely far" — exactly what the sky decodes to;
* ``seg`` id 0 is CityScapes *unlabeled*, whose palette colour is black, and the
  instance colouring gives instance 0 black as well;
* ``edge`` 0 is "no edge" — what :func:`controls.masked_canny` already writes
  over the sky;
* ``vis``/``blur`` and ``rgb`` 0 is black.

Using a single value keeps the branches consistent with each other: a masked
region is the same "no information" region in all four, instead of one branch
saying *far away* and another *foreground object*.

What it does **not** touch
--------------------------
The world-scenario inputs — ``hdmap_bbox`` (Transfer 2.5 AV) and ``wsm``
(Cosmos 3) — are rendered server-side from the ClipGT scene package.  They are
geometric ground truth (lanes, boxes, poses), not captured pixels, and the AV
model treats them as the scenario definition.  Blanking pixels there would
either be meaningless (the renderer draws from Parquet tables, not from our
video) or would silently change the scenario.  To drop an object from the world
model, drop it from the scene package instead.

Recovering the class ids
------------------------
The mask is built from a **CityScapes-palette** video: ``semantic_<camera>.mp4``
when the clip has one, otherwise ``seg_<camera>.mp4`` when the clip was captured
with ``seg_mode="semantic"`` (auto-detected, see :func:`semantic_video`).

That video is H.264 4:4:4 ``-qp 0``: the *coding* is lossless and there is no
chroma subsampling, but the RGB -> YUV444 -> RGB round trip still rounds, so the
palette colours do not come back bit-exact (measured on a real Town10 clip:
44 % of pixels exact, worst case off by L1 = 2, e.g. road ``(128, 64, 128)`` ->
``(129, 64, 129)``).  Class ids are therefore recovered by **nearest palette
colour**, which is exact here with a wide margin: the closest two palette
entries are L1 = 20 apart (bus vs train), ten times the observed codec error,
and a decoded frame contains exactly one colour per class present (21 distinct
colours for 21 classes) — 4:4:4 does not blend across class boundaries.
:data:`MAX_CODEC_L1` guards the margin and a frame that exceeds it is rejected
rather than silently mis-labelled.

Dilation
--------
The mask is dilated by :data:`DEFAULT_DILATE` pixels (elliptical kernel) before
it is applied, so the one- or two-pixel rim of an object — anti-aliased in RGB,
half-covered in the semantic buffer, smeared by the RGB encoder — falls inside
the hole instead of leaving a visible outline for the model to trace.
"""

from __future__ import annotations

import logging
from pathlib import Path
from typing import Iterable, Iterator, Sequence

import numpy as np

from .clip import Clip, video_file_name
from .contracts import canonical_camera_name
from .controls import CITYSCAPES_PALETTE, VideoReader, VideoWriter

log = logging.getLogger(__name__)

N_TAGS = 30
"""Number of CARLA ``CityObjectLabel`` values (``LibCarla/source/carla/rpc/ObjectLabel.h``)."""

TAG_NAMES: tuple[str, ...] = (
    "unlabeled", "road", "sidewalk", "building", "wall", "fence", "pole", "traffic_light",
    "traffic_sign", "vegetation", "terrain", "sky", "pedestrian", "rider", "car", "truck",
    "bus", "train", "motorcycle", "bicycle", "static", "dynamic", "other", "water",
    "road_line", "ground", "bridge", "rail_track", "guard_rail", "rock",
)
"""Canonical name of every tag, indexed by id (matches ``CityScapesPalette.h``)."""

ALIASES: dict[str, int] = {
    "none": 0, "unlabelled": 0, "roads": 1, "sidewalks": 2, "buildings": 3, "walls": 4,
    "fences": 5, "poles": 6, "trafficlight": 7, "traffic_lights": 7, "trafficsign": 8,
    "traffic_signs": 8, "pedestrians": 12, "person": 12, "walker": 12, "car": 14, "cars": 14,
    "trucks": 15, "buses": 16, "motorcycles": 18, "motorbike": 18, "bike": 19, "bicycles": 19,
    "roadline": 24, "road_lines": 24, "railtrack": 27, "guardrail": 28,
}
"""Extra spellings accepted for a single tag."""

GROUPS: dict[str, tuple[int, ...]] = {
    "vehicle": (14, 15, 16, 17, 18, 19),
    "vehicles": (14, 15, 16, 17, 18, 19),
    "vru": (12, 13, 18, 19),
    "movable": (12, 13, 14, 15, 16, 17, 18, 19, 21),
}
"""Names that expand to several tags (``vru`` = vulnerable road users)."""

MAX_CODEC_L1 = 8
"""Largest L1 RGB distance from a decoded pixel to its palette colour we accept.

Measured worst case is 2; the closest two palette entries are 20 apart, so
anything under 10 still decides the class unambiguously.  Above this the video
is not a CityScapes-palette video (or was re-encoded lossily) and is rejected.
"""

DEFAULT_DILATE = 3
"""Pixels the mask grows by, so object rims do not leak."""

MASK_KEEP, MASK_DROP = 255, 0
"""Mask-video values: white = control applies here, black = ignore (Cosmos ``mask_path``)."""

FILL = 0
"""Value written into a masked region — black, "no information", for every modality."""

MASKABLE_KINDS: tuple[str, ...] = ("rgb", "depth", "seg", "edge")
"""Clip video kinds the mask is applied to.  ``wsm``/``hdmap_bbox`` are not clip videos."""


class MaskError(ValueError):
    """A class name is unknown, or the clip carries no semantic source."""


# ----------------------------------------------------------------------------- class names

def class_table() -> list[tuple[str, int, list[str]]]:
    """``(name, id, aliases)`` for every tag, then the group names (id ``-1``)."""
    rows: list[tuple[str, int, list[str]]] = []
    for tag, name in enumerate(TAG_NAMES):
        rows.append((name, tag, sorted(a for a, t in ALIASES.items() if t == tag and a != name)))
    for name, tags in sorted(GROUPS.items()):
        rows.append((name, -1, [TAG_NAMES[t] for t in tags]))
    return rows


def resolve_classes(specs: Sequence[str | int] | None) -> tuple[int, ...]:
    """Names, ids or groups -> sorted unique tag ids.

    Accepts ``"vehicle"``, ``"Vehicle"``, ``"vehicle,pedestrian"``, ``14``,
    ``"14"``.  Raises :class:`MaskError` naming the valid classes otherwise.
    """
    if not specs:
        return ()
    out: set[int] = set()
    for spec in specs:
        for token in (str(spec).split(",") if isinstance(spec, str) else [str(spec)]):
            name = token.strip().lower().replace(" ", "_").replace("-", "_")
            if not name:
                continue
            out.update(_resolve_one(name))
    return tuple(sorted(out))


def _resolve_one(name: str) -> tuple[int, ...]:
    if name in GROUPS:
        return GROUPS[name]
    if name in TAG_NAMES:
        return (TAG_NAMES.index(name),)
    if name in ALIASES:
        return (ALIASES[name],)
    if name.isdigit():
        tag = int(name)
        if 0 <= tag < N_TAGS:
            return (tag,)
        raise MaskError(f"class id {tag} is out of range (0..{N_TAGS - 1}); "
                        f"see 'carla-cosmos classes'")
    raise MaskError(f"unknown semantic class '{name}'; valid names: "
                    f"{', '.join(sorted(set(TAG_NAMES) | set(GROUPS)))} (or a tag id 0..{N_TAGS - 1}); "
                    f"see 'carla-cosmos classes'")


def class_names(tags: Iterable[int]) -> list[str]:
    """Canonical names of tag ids (for manifests and log lines)."""
    return [TAG_NAMES[t] if 0 <= t < N_TAGS else str(t) for t in tags]


# ----------------------------------------------------------------------------- semantic source

def semantic_video(clip: Clip, camera: str) -> Path:
    """Path of the CityScapes-palette video the mask is built from.

    ``semantic_<camera>.mp4`` when the clip has one, else ``seg_<camera>.mp4``
    when it is palette-coloured (``Capture(seg_mode="semantic")``).  Raises
    :class:`MaskError` with what to do otherwise — an instance-coloured ``seg``
    video carries instance ids only, not classes, so no mask can be derived.
    """
    name = clip.manifest.video("semantic", camera)
    if name is not None:
        return clip.path / name
    direct = clip.path / video_file_name("semantic", camera)
    if direct.is_file():
        return direct
    seg = clip.manifest.video("seg", camera)
    if seg is not None and is_palette_video(clip.path / seg):
        return clip.path / seg
    raise MaskError(
        f"clip {clip.manifest.clip_id} has no semantic class information for camera '{camera}': "
        f"expected {video_file_name('semantic', camera)}, or a CityScapes-coloured "
        f"{video_file_name('seg', camera)}. Its seg video is instance-coloured (random palette over "
        f"instance ids), which carries no class. Recapture with Capture(seg_mode='semantic') "
        f"(or a 'semantic' AOV video) to use --mask-classes.")


def is_palette_video(path: str | Path, sample_frames: int = 1, tolerance: float = 0.999) -> bool:
    """Whether a video's first frame(s) are CityScapes-palette colours."""
    try:
        with VideoReader(path) as reader:
            for i, frame in enumerate(reader):
                if i >= sample_frames:
                    break
                d = _palette_distance(frame)
                if float((d <= MAX_CODEC_L1).mean()) < tolerance:
                    return False
    except (OSError, RuntimeError, ValueError):
        return False
    return True


def _palette_distance(frame: np.ndarray) -> np.ndarray:
    """L1 distance of every pixel to its nearest palette entry (per-pixel min)."""
    return _nearest(frame)[1]


def _nearest(frame: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    """``(tag, distance)`` per pixel, over the ``N_TAGS`` used palette entries.

    Works on the frame's distinct colours (a 4:4:4 palette frame has a few
    dozen), not on every pixel, so a 720p frame costs a lookup, not a 2.7 M x 30
    distance matrix.
    """
    flat = frame.reshape(-1, 3)
    colours, inverse = np.unique(flat, axis=0, return_inverse=True)
    d = np.abs(colours[:, None, :].astype(np.int16)
               - CITYSCAPES_PALETTE[None, :N_TAGS, :].astype(np.int16)).sum(axis=2)
    tag = d.argmin(axis=1).astype(np.uint8)
    dist = d.min(axis=1).astype(np.int16)
    shape = frame.shape[:2]
    return tag[inverse].reshape(shape), dist[inverse].reshape(shape)


def tags_from_frame(frame: np.ndarray) -> np.ndarray:
    """Decoded palette frame -> ``(H, W)`` uint8 CARLA tag ids (nearest palette colour).

    Raises :class:`MaskError` if a pixel is further from every palette entry than
    :data:`MAX_CODEC_L1` — that video is not a semantic AOV.
    """
    tag, dist = _nearest(frame)
    worst = int(dist.max())
    if worst > MAX_CODEC_L1:
        raise MaskError(f"frame is not a CityScapes-palette semantic video: a pixel is L1 {worst} "
                        f"from the nearest class colour (limit {MAX_CODEC_L1})")
    return tag


# ----------------------------------------------------------------------------- masks

def dilate_mask(mask: np.ndarray, pixels: int) -> np.ndarray:
    """Grow a boolean mask by ``pixels`` (elliptical kernel); ``0`` returns it unchanged."""
    if pixels <= 0:
        return mask
    import cv2

    k = 2 * int(pixels) + 1
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (k, k))
    return cv2.dilate(mask.astype(np.uint8), kernel).astype(bool)


def frame_mask(tags: np.ndarray, classes: Sequence[int], dilate: int = DEFAULT_DILATE) -> np.ndarray:
    """Boolean "remove me" mask for ``classes`` in a tag image, dilated."""
    if not classes:
        return np.zeros(tags.shape, dtype=bool)
    return dilate_mask(np.isin(tags, np.asarray(classes, dtype=np.uint8)), dilate)


def iter_frame_masks(semantic: str | Path, classes: Sequence[int],
                     dilate: int = DEFAULT_DILATE) -> Iterator[np.ndarray]:
    """Per-frame boolean masks read from a semantic video."""
    with VideoReader(semantic) as reader:
        for frame in reader:
            yield frame_mask(tags_from_frame(frame), classes, dilate)


def apply_mask(frame: np.ndarray, mask: np.ndarray, fill: int = FILL) -> np.ndarray:
    """Blank ``frame`` where ``mask`` is set (returns a new array)."""
    out = frame.copy()
    out[mask] = fill
    return out


# ----------------------------------------------------------------------------- video products

def write_masked_video(src: str | Path, dest: str | Path, masks: Iterable[np.ndarray], fps: int,
                       kind: str = "control", fill: int = FILL) -> int:
    """Re-encode ``src`` with every mask frame blanked; returns the frame count.

    ``masks`` is consumed in step with the source frames; a shorter mask stream
    leaves the remaining frames untouched (and is logged), a longer one is cut.
    """
    it = iter(masks)
    written = 0
    with VideoReader(src) as reader, VideoWriter(dest, fps, reader.width, reader.height, kind) as writer:
        for frame in reader:
            mask = next(it, None)
            if mask is None:
                log.warning("%s: mask stream ended after %d frames, rest is unmasked", Path(src).name, written)
                writer.write(frame)
            else:
                if mask.shape != frame.shape[:2]:
                    raise MaskError(f"mask {mask.shape} does not match {Path(src).name} {frame.shape[:2]}")
                writer.write(apply_mask(frame, mask, fill))
            written += 1
    return written


def write_mask_video(dest: str | Path, masks: Iterable[np.ndarray], fps: int) -> int:
    """Write the binary mask itself (white = keep the control, black = ignore).

    This is the form Cosmos Transfer 2.5 takes as a per-control ``mask_path``:
    the control's weight is zeroed inside the black region instead of the
    control merely carrying blank pixels there.
    """
    writer: VideoWriter | None = None
    written = 0
    try:
        for mask in masks:
            frame = np.where(mask[:, :, None], np.uint8(MASK_DROP), np.uint8(MASK_KEEP))
            frame = np.repeat(frame, 3, axis=2) if frame.shape[2] == 1 else frame
            if writer is None:
                writer = VideoWriter(dest, fps, frame.shape[1], frame.shape[0], "control")
            writer.write(np.ascontiguousarray(frame))
            written += 1
    finally:
        if writer is not None:
            writer.close()
    if written == 0:
        raise MaskError("no mask frames to write")
    return written


def masked_clip_videos(clip: Clip, camera: str, classes: Sequence[int], kinds: Sequence[str],
                       out_dir: str | Path, dilate: int = DEFAULT_DILATE,
                       mask_video: bool = False) -> dict[str, Path]:
    """Write masked copies of a camera's clip videos (and optionally the mask itself).

    Returns ``{kind: path}`` for every requested kind the clip actually has, plus
    ``{"mask": path}`` when ``mask_video`` is set.  The semantic video is decoded
    once and the masks are cached, so ``n`` kinds cost one semantic pass.
    """
    unknown = [k for k in kinds if k not in MASKABLE_KINDS]
    if unknown:
        raise MaskError(f"cannot mask video kind(s) {unknown}; maskable: {list(MASKABLE_KINDS)}")
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    fps = clip.manifest.fps
    masks = [m for m in iter_frame_masks(semantic_video(clip, camera), classes, dilate)]
    if not masks:
        raise MaskError(f"semantic video for '{camera}' decoded to 0 frames")
    covered = float(np.mean([m.mean() for m in masks]))
    log.info("mask %s: %s -> %.2f%% of pixels over %d frames (dilate %d px)", camera,
             ",".join(class_names(classes)), 100.0 * covered, len(masks), dilate)
    written: dict[str, Path] = {}
    stem = canonical_camera_name(camera)
    for kind in kinds:
        if clip.manifest.video(kind, camera) is None:
            continue
        dest = out / f"{kind}_{stem}.mp4"
        write_masked_video(clip.video(kind, camera), dest, masks, fps,
                           kind="rgb" if kind == "rgb" else "control")
        written[kind] = dest
    if mask_video:
        dest = out / f"mask_{stem}.mp4"
        write_mask_video(dest, masks, fps)
        written["mask"] = dest
    return written
