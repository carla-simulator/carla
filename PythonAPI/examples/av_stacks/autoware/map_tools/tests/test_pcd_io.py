#!/usr/bin/env python3
"""Unit tests for pcd_io: write a binary PCD, read it back with the tiny
parser, and check voxel downsampling. Runs with plain python (numpy only):

    python3 PythonAPI/examples/av_stacks/autoware/map_tools/tests/test_pcd_io.py
"""

import os
import sys
import tempfile

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from pcd_io import read_pcd, voxel_downsample, write_pcd  # noqa: E402


def test_write_read_roundtrip():
    pts = np.array([
        [0.0, 0.0, 0.0, 0.5],
        [1.25, -2.5, 3.75, 0.0],
        [-100.5, 200.25, -0.125, 1.0],
    ], dtype=np.float32)
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "t.pcd")
        write_pcd(path, pts)
        header, back = read_pcd(path)
    assert header["FIELDS"] == "x y z intensity", header
    assert header["DATA"] == "binary", header
    assert int(header["POINTS"]) == 3
    assert int(header["WIDTH"]) == 3 and int(header["HEIGHT"]) == 1
    assert back.shape == (3, 4)
    assert np.array_equal(back, pts), (back, pts)
    print("test_write_read_roundtrip OK")


def test_write_nx3_pads_intensity():
    pts = np.array([[1.0, 2.0, 3.0]], dtype=np.float32)
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "t3.pcd")
        write_pcd(path, pts)
        _, back = read_pcd(path)
    assert back.shape == (1, 4)
    assert back[0, 3] == 0.0
    print("test_write_nx3_pads_intensity OK")


def test_voxel_downsample_merges_and_averages():
    # Two points in the same 0.2 m voxel, one far away.
    pts = np.array([
        [0.01, 0.01, 0.01, 0.0],
        [0.03, 0.03, 0.03, 1.0],
        [10.0, 10.0, 10.0, 0.5],
    ], dtype=np.float32)
    out = voxel_downsample(pts, 0.2)
    assert out.shape == (2, 4), out
    merged = out[np.argmin(np.abs(out[:, 0]))]
    assert np.allclose(merged, [0.02, 0.02, 0.02, 0.5], atol=1e-6), merged
    print("test_voxel_downsample_merges_and_averages OK")


def test_voxel_downsample_negative_coords_and_empty():
    pts = np.array([[-0.35, -0.35, -0.35, 0.0], [-0.25, -0.25, -0.25, 0.0]], dtype=np.float32)
    out = voxel_downsample(pts, 0.2)
    assert out.shape[0] == 1, out  # both fall in voxel [-2,-2,-2]
    empty = voxel_downsample(np.empty((0, 4), dtype=np.float32), 0.2)
    assert empty.shape[0] == 0
    print("test_voxel_downsample_negative_coords_and_empty OK")


def test_large_roundtrip_through_downsample():
    rng = np.random.default_rng(7)
    pts = rng.uniform(-50, 50, size=(20000, 3)).astype(np.float32)
    down = voxel_downsample(pts, 0.5)
    assert 0 < down.shape[0] <= pts.shape[0]
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "big.pcd")
        write_pcd(path, down)
        header, back = read_pcd(path)
    assert int(header["POINTS"]) == down.shape[0]
    assert np.allclose(back[:, :3], down[:, :3])
    print("test_large_roundtrip_through_downsample OK")


if __name__ == "__main__":
    test_write_read_roundtrip()
    test_write_nx3_pads_intensity()
    test_voxel_downsample_merges_and_averages()
    test_voxel_downsample_negative_coords_and_empty()
    test_large_roundtrip_through_downsample()
    print("All pcd_io tests passed.")
