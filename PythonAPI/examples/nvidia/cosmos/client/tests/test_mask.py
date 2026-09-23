"""Mask-out classes: name resolution, class recovery through the codec, dilation,
masked control videos and what the submit path does with them.

The fixtures are synthetic clips: a two-class semantic frame (road + car) encoded
with the real control encoder, so the tests exercise the actual palette recovery
rather than a mocked one.
"""

import json

import numpy as np
import pytest

from carla_cosmos import controls, mask
from carla_cosmos.clip import Clip
from carla_cosmos.contracts import (
    BUILTIN_CONTRACTS,
    CameraManifest,
    ClipManifest,
    ControlInput,
    JobRequest,
    RigManifest,
    validate_request,
)

CAMERA = "camera:front:wide:120fov"
W, H, FRAMES, FPS = 96, 64, 6, 16
CAR, ROAD, BUILDING = 14, 1, 3
BOX = (slice(20, 44), slice(30, 66))  # the "car" rectangle in every frame


# ----------------------------------------------------------------------------- fixtures

def _tag_frame() -> np.ndarray:
    tags = np.full((H, W), ROAD, np.uint8)
    tags[:16] = BUILDING
    tags[BOX] = CAR
    return tags


def _write_clip(tmp_path, kinds=("rgb", "depth", "seg", "edge"), semantic="seg"):
    """A clip whose ``semantic`` video is CityScapes-coloured; other videos are flat greys."""
    tags = _tag_frame()
    d = tmp_path / "clip"
    d.mkdir(parents=True)
    videos = {}
    for kind in kinds:
        name = f"{kind}_camera_front_wide_120fov.mp4"
        if kind == semantic:
            frames = [controls.colourise_semantic(tags)] * FRAMES
        else:
            frames = [np.full((H, W, 3), 200, np.uint8)] * FRAMES
        controls.encode_frames(d / name, frames, FPS, "rgb" if kind == "rgb" else "control")
        videos[f"{kind}/camera_front_wide_120fov"] = name
    manifest = ClipManifest(
        clip_id="t", carla_version="0.10.0", map="Town10", weather={},
        rig=RigManifest(name="single", mount="roofline", cameras=[CameraManifest(
            name=CAMERA, hfov=90.0, width=W, height=H, t_flu=[0, 0, 1], rpy_flu=[0, 0, 0],
            attach_ue={"x": 0.0, "y": 0.0, "z": 1.0, "pitch": 0.0, "yaw": 0.0, "roll": 0.0})]),
        fps=FPS, frames=FRAMES, ego_id=1, ego_type_id="vehicle.lincoln.mkz",
        rear_axle_offset_ue=[0, 0, 0], aovs=list(kinds), videos=videos)
    clip = Clip(path=d, manifest=manifest)
    clip.save_manifest()
    return clip


def _first_frame(path):
    with controls.VideoReader(path) as r:
        return next(iter(r)).copy()


# ----------------------------------------------------------------------------- class names

def test_resolve_names_ids_and_groups():
    assert mask.resolve_classes(["car"]) == (14,)
    assert mask.resolve_classes(["vehicle"]) == (14, 15, 16, 17, 18, 19)
    assert mask.resolve_classes(["vehicle,pedestrian"]) == (12, 14, 15, 16, 17, 18, 19)
    assert mask.resolve_classes([14, "14", "Car", "cars"]) == (14,)
    assert mask.resolve_classes(["Traffic-Light"]) == (7,)
    assert mask.resolve_classes(None) == () and mask.resolve_classes([]) == ()


def test_resolve_rejects_unknown_name_and_out_of_range_id():
    with pytest.raises(mask.MaskError) as exc:
        mask.resolve_classes(["vehicel"])
    assert "unknown semantic class 'vehicel'" in str(exc.value)
    assert "vehicle" in str(exc.value), "the error must list the valid names"
    with pytest.raises(mask.MaskError, match="out of range"):
        mask.resolve_classes([99])


def test_class_table_covers_every_tag_and_group():
    rows = mask.class_table()
    assert [r[0] for r in rows[:mask.N_TAGS]] == list(mask.TAG_NAMES)
    assert {r[0] for r in rows[mask.N_TAGS:]} == set(mask.GROUPS)
    assert mask.class_names([1, 14]) == ["road", "car"]


# ----------------------------------------------------------------------------- class recovery

def test_palette_recovery_survives_the_control_encoder(tmp_path):
    """The 4:4:4 encoder is not bit-exact in RGB; nearest palette colour still recovers every id."""
    tags = _tag_frame()
    path = tmp_path / "sem.mp4"
    controls.encode_frames(path, [controls.colourise_semantic(tags)] * 2, FPS, "control")
    decoded = _first_frame(path)
    assert not (decoded == controls.colourise_semantic(tags)).all(), \
        "fixture would be vacuous if the codec were bit-exact"
    np.testing.assert_array_equal(mask.tags_from_frame(decoded), tags)
    assert mask.is_palette_video(path)


def test_non_palette_video_is_rejected(tmp_path):
    """An instance-coloured seg video carries no class information and must not be guessed at."""
    rng = np.random.default_rng(0)
    frame = rng.integers(48, 256, size=(H, W, 3), dtype=np.uint8)
    path = tmp_path / "inst.mp4"
    controls.encode_frames(path, [frame] * 2, FPS, "control")
    assert not mask.is_palette_video(path)
    with pytest.raises(mask.MaskError, match="not a CityScapes-palette"):
        mask.tags_from_frame(_first_frame(path))


def test_semantic_source_prefers_semantic_video_and_explains_when_absent(tmp_path):
    clip = _write_clip(tmp_path, kinds=("rgb", "seg"), semantic="seg")
    assert mask.semantic_video(clip, CAMERA).name == "seg_camera_front_wide_120fov.mp4"

    other = _write_clip(tmp_path / "b", kinds=("rgb", "seg"), semantic=None)
    with pytest.raises(mask.MaskError) as exc:
        mask.semantic_video(other, CAMERA)
    assert "instance-coloured" in str(exc.value) and "'semantic' AOV" in str(exc.value)


# ----------------------------------------------------------------------------- mask geometry

def test_frame_mask_selects_only_the_named_classes():
    m = mask.frame_mask(_tag_frame(), (CAR,), dilate=0)
    assert m[BOX].all()
    assert not m[50:, :10].any(), "road must be untouched"
    assert not m[:10].any(), "buildings must be untouched"
    assert mask.frame_mask(_tag_frame(), (), dilate=0).sum() == 0


@pytest.mark.parametrize("px", [0, 1, 3])
def test_dilation_grows_the_mask_by_the_requested_pixels(px):
    m = mask.frame_mask(_tag_frame(), (CAR,), dilate=px)
    rows = np.where(m.any(axis=1))[0]
    cols = np.where(m.any(axis=0))[0]
    assert rows.min() == BOX[0].start - px and rows.max() == BOX[0].stop - 1 + px
    assert cols.min() == BOX[1].start - px and cols.max() == BOX[1].stop - 1 + px


def test_dilation_is_monotone():
    sizes = [mask.frame_mask(_tag_frame(), (CAR,), dilate=p).sum() for p in (0, 2, 5)]
    assert sizes[0] < sizes[1] < sizes[2]


# ----------------------------------------------------------------------------- masked videos

def test_masked_clip_videos_blank_every_kind_and_keep_the_rest(tmp_path):
    clip = _write_clip(tmp_path)
    out = mask.masked_clip_videos(clip, CAMERA, (CAR,), ("rgb", "depth", "seg", "edge"),
                                  tmp_path / "out", dilate=0, mask_video=True)
    assert set(out) == {"rgb", "depth", "seg", "edge", "mask"}
    for kind in ("depth", "edge"):
        frame = _first_frame(out[kind])
        assert frame[BOX].max() <= 2, f"{kind} must be black inside the mask"
        assert frame[55, 5].min() > 150, f"{kind} must be untouched outside the mask"
    seg = _first_frame(out["seg"])
    assert seg[BOX].max() <= 2
    np.testing.assert_array_equal(mask.tags_from_frame(seg)[BOX], 0)      # -> unlabeled
    np.testing.assert_array_equal(mask.tags_from_frame(seg)[55:, :10], ROAD)


def test_mask_video_is_white_outside_and_black_inside(tmp_path):
    clip = _write_clip(tmp_path, kinds=("rgb", "seg"))
    out = mask.masked_clip_videos(clip, CAMERA, (CAR,), ("seg",), tmp_path / "out",
                                  dilate=2, mask_video=True)
    frame = _first_frame(out["mask"])
    assert frame[32, 48].max() <= 2, "inside the car: black = ignore the control"
    assert frame[55, 5].min() >= 250, "outside: white = the control applies"
    assert controls.probe_video(out["mask"])["frames"] == FRAMES


def test_masked_video_keeps_the_frame_count_and_size(tmp_path):
    clip = _write_clip(tmp_path, kinds=("rgb", "seg", "depth"))
    out = mask.masked_clip_videos(clip, CAMERA, (CAR,), ("depth",), tmp_path / "out")
    info = controls.probe_video(out["depth"])
    assert (info["frames"], info["width"], info["height"]) == (FRAMES, W, H)


def test_masking_a_kind_the_clip_lacks_is_skipped_not_invented(tmp_path):
    clip = _write_clip(tmp_path, kinds=("rgb", "seg"))
    out = mask.masked_clip_videos(clip, CAMERA, (CAR,), ("rgb", "seg", "depth"), tmp_path / "out")
    assert set(out) == {"rgb", "seg"}


def test_unmaskable_kind_is_rejected(tmp_path):
    clip = _write_clip(tmp_path, kinds=("rgb", "seg"))
    with pytest.raises(mask.MaskError, match="cannot mask video kind"):
        mask.masked_clip_videos(clip, CAMERA, (CAR,), ("wsm",), tmp_path / "out")


# ----------------------------------------------------------------------------- contract surface

def test_only_transfer25_accepts_a_mask_video():
    assert BUILTIN_CONTRACTS["transfer2.5"].maskable_controls == ["edge", "vis", "depth", "seg"]
    assert BUILTIN_CONTRACTS["cosmos3-nano"].maskable_controls == []
    assert BUILTIN_CONTRACTS["transfer2.5-av"].maskable_controls == [], \
        "hdmap_bbox is geometric GT rendered from the scene package, never masked"


def _request(backend, **kw):
    return JobRequest(backend=backend, prompt="p", **kw)


def test_mask_blobs_validate_against_the_contract(tmp_path):
    clip = _write_clip(tmp_path)
    m = clip.manifest.model_copy(update={"fps": 16, "frames": 93})
    req = _request("transfer2.5", controls={"depth": ControlInput(blob="a")}, rgb={CAMERA: "b"},
                   masks={CAMERA: "c"}, mask_classes=["car"], mask_dilate=3)
    assert validate_request(BUILTIN_CONTRACTS["transfer2.5"], req, m) == []

    req.masks = {"camera:rear:tele:30fov": "c"}
    errors = validate_request(BUILTIN_CONTRACTS["transfer2.5"], req, m)
    assert any("mask video missing for view" in e for e in errors)


def test_mask_video_rejected_where_no_control_takes_one(tmp_path):
    clip = _write_clip(tmp_path)
    m = clip.manifest.model_copy(update={"fps": 16, "frames": 96})
    req = _request("cosmos3-nano", controls={"depth": ControlInput(blob="a")}, masks={CAMERA: "c"})
    errors = validate_request(BUILTIN_CONTRACTS["cosmos3-nano"], req, m)
    assert any("takes no mask video" in e for e in errors)


def test_mask_classes_are_recorded_in_the_request_json(tmp_path):
    req = _request("transfer2.5", controls={"depth": ControlInput(blob="a")},
                   mask_classes=["car", "truck"], mask_dilate=3)
    body = json.loads(req.model_dump_json())
    assert body["mask_classes"] == ["car", "truck"] and body["mask_dilate"] == 3
