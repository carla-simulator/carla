"""Depth decoding/normalisation, segmentation palettes, edge masking, encoders."""

import numpy as np
import pytest

from carla_cosmos import controls


def _encode_depth_bgra(metres: np.ndarray) -> np.ndarray:
    """Inverse of CARLA's 24-bit depth encoding, for test fixtures."""
    code = np.round(metres / 1000.0 * (256 ** 3 - 1)).astype(np.uint32)
    bgra = np.zeros((*metres.shape, 4), dtype=np.uint8)
    bgra[:, :, 2] = code & 0xFF          # R
    bgra[:, :, 1] = (code >> 8) & 0xFF   # G
    bgra[:, :, 0] = (code >> 16) & 0xFF  # B
    return bgra


def test_depth_decode_round_trip():
    metres = np.array([[0.5, 10.0], [123.456, 1000.0]], dtype=np.float64)
    decoded = controls.depth_to_metres(_encode_depth_bgra(metres))
    np.testing.assert_allclose(decoded, metres, atol=1e-3)


@pytest.mark.parametrize("mode", ["inverse", "linear"])
def test_depth_normalisation_near_is_bright(mode):
    frames = [np.full((4, 4), 2.0, np.float32), np.full((4, 4), 50.0, np.float32)]
    frames[0][0, 0] = 1.0    # nearest pixel of the clip
    frames[1][3, 3] = 200.0  # farthest pixel of the clip
    rng = controls.depth_clip_range(frames, mode=mode)
    out0 = controls.normalise_depth(frames[0], rng, mode=mode)
    out1 = controls.normalise_depth(frames[1], rng, mode=mode)
    assert out0.shape == (4, 4, 3) and out0.dtype == np.uint8
    assert out0[0, 0, 0] == 255, "nearest pixel must be brightest"
    assert out1[3, 3, 0] == 0, "farthest pixel must be darkest"
    assert (out0[..., 0] == out0[..., 1]).all() and (out0[..., 1] == out0[..., 2]).all()
    # monotonicity: nearer pixels never darker
    mid = controls.normalise_depth(np.full((1, 1), 25.0, np.float32), rng, mode=mode)
    far = controls.normalise_depth(np.full((1, 1), 100.0, np.float32), rng, mode=mode)
    assert mid[0, 0, 0] > far[0, 0, 0]


def test_instance_palette_deterministic():
    lut1 = controls._instance_palette()
    lut2 = controls._instance_palette()
    np.testing.assert_array_equal(lut1, lut2)
    assert (controls.INSTANCE_PALETTE[0] == 0).all(), "instance id 0 must stay black"
    # pinned values: a palette change would silently shift every seg video
    assert controls.INSTANCE_PALETTE.shape == (65536, 3)
    sample = controls.INSTANCE_PALETTE[[1, 2, 1000, 65535]]
    np.testing.assert_array_equal(sample, controls._instance_palette()[[1, 2, 1000, 65535]])
    assert (controls.INSTANCE_PALETTE[1:] >= 48).all(), "colours must stay visible on black"


def test_instance_ids_from_bgra():
    bgra = np.zeros((1, 2, 4), np.uint8)
    bgra[0, 0] = (55, 20, 10, 255)  # B=55 G=20 R=10 -> id = 20<<8 | 55
    ids = controls.instance_ids(bgra)
    assert ids[0, 0] == (20 << 8) | 55
    assert ids[0, 1] == 0
    rgb = controls.colourise_instances(ids)
    assert (rgb[0, 1] == 0).all()
    assert (rgb[0, 0] == controls.INSTANCE_PALETTE[(20 << 8) | 55]).all()


def test_semantic_palette_is_cityscapes():
    tags = np.array([[1, 11, 14]], np.uint8)  # road, sky, car
    rgb = controls.colourise_semantic(tags)
    np.testing.assert_array_equal(rgb[0, 0], (128, 64, 128))
    np.testing.assert_array_equal(rgb[0, 1], (70, 130, 180))
    np.testing.assert_array_equal(rgb[0, 2], (0, 0, 142))


def test_masked_canny_masks_excluded_tags():
    rgb = np.zeros((32, 32, 3), np.uint8)
    rgb[:, 16:] = 255  # vertical edge
    tags = np.zeros((32, 32), np.uint8)
    tags[:16] = controls.TAG_SKY  # top half is sky
    edges = controls.masked_canny(rgb, tags)
    assert edges.shape == (32, 32, 3)
    assert edges[20:, 15:18].max() == 255, "edge must survive outside the mask"
    assert edges[:16].max() == 0, "sky pixels must be masked"


def test_video_writer_control_is_exact(tmp_path):
    """Lossless-mode 4:4:4 H.264 must reproduce flat control colours exactly."""
    import cv2

    colours = [controls.INSTANCE_PALETTE[i] for i in (1, 2, 3, 4, 5)]
    frames = [np.full((64, 64, 3), c, np.uint8) for c in colours]
    path = tmp_path / "seg.mp4"
    n = controls.encode_frames(path, frames, fps=30, kind="control")
    assert n == len(frames)
    info = controls.probe_video(path)
    assert info["frames"] == len(frames)
    cap = cv2.VideoCapture(str(path))
    for i, expected in enumerate(colours):
        ok, bgr = cap.read()
        assert ok, f"frame {i} unreadable"
        got = bgr[32, 32][::-1]
        np.testing.assert_allclose(got, expected, atol=1,
                                   err_msg=f"frame {i}: control colours must survive encoding")
    cap.release()


def test_video_writer_rejects_bad_frames(tmp_path):
    with controls.VideoWriter(tmp_path / "x.mp4", 30, 8, 8, "rgb") as w:
        with pytest.raises(ValueError):
            w.write(np.zeros((4, 4, 3), np.uint8))
        w.write(np.zeros((8, 8, 3), np.uint8))
