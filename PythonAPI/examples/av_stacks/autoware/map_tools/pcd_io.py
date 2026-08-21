"""Minimal PCD point-cloud I/O and voxel downsampling with pure numpy.

No open3d / pcl dependency. Supports the subset of the PCD format that
Autoware's pointcloud_map_loader consumes: binary (plus ascii and
binary_compressed for the reader -- the prebuilt autoware-contents maps are
binary_compressed), float32 fields, HEIGHT 1 unorganized clouds.
"""

from __future__ import annotations

import numpy as np

PCD_FIELDS = ("x", "y", "z", "intensity")


def write_pcd(path, points):
    """Write an Nx4 (x, y, z, intensity) float32 array as a binary PCD file.

    Nx3 input is accepted; a zero intensity column is appended.
    """
    points = np.asarray(points, dtype=np.float32)
    if points.ndim != 2 or points.shape[1] not in (3, 4):
        raise ValueError(f"expected Nx3 or Nx4 array, got shape {points.shape}")
    if points.shape[1] == 3:
        points = np.hstack([points, np.zeros((points.shape[0], 1), dtype=np.float32)])
    n = points.shape[0]
    header = (
        "# .PCD v0.7 - Point Cloud Data file format\n"
        "VERSION 0.7\n"
        "FIELDS x y z intensity\n"
        "SIZE 4 4 4 4\n"
        "TYPE F F F F\n"
        "COUNT 1 1 1 1\n"
        f"WIDTH {n}\n"
        "HEIGHT 1\n"
        "VIEWPOINT 0 0 0 1 0 0 0\n"
        f"POINTS {n}\n"
        "DATA binary\n"
    )
    with open(path, "wb") as f:
        f.write(header.encode("ascii"))
        f.write(np.ascontiguousarray(points, dtype="<f4").tobytes())


def read_pcd(path):
    """Read a PCD file written by :func:`write_pcd` (or any float32-only PCD).

    Returns ``(header_dict, points)`` where ``points`` is an ``N x len(FIELDS)``
    float32 array. Supports ``DATA binary`` and ``DATA ascii``.
    """
    header = {}
    with open(path, "rb") as f:
        while True:
            line = f.readline()
            if not line:
                raise ValueError("unexpected EOF while reading PCD header")
            text = line.decode("ascii", errors="replace").strip()
            if not text or text.startswith("#"):
                continue
            key, _, value = text.partition(" ")
            header[key.upper()] = value
            if key.upper() == "DATA":
                break
        fields = header["FIELDS"].split()
        sizes = [int(s) for s in header["SIZE"].split()]
        types = header["TYPE"].split()
        counts = [int(c) for c in header.get("COUNT", " ".join(["1"] * len(fields))).split()]
        if any(t != "F" or s != 4 or c != 1 for t, s, c in zip(types, sizes, counts)):
            raise ValueError("read_pcd only supports scalar float32 fields")
        n = int(header["POINTS"])
        ncols = len(fields)
        mode = header["DATA"].lower()
        if mode == "binary":
            buf = f.read(n * ncols * 4)
            if len(buf) < n * ncols * 4:
                raise ValueError("PCD payload truncated")
            points = np.frombuffer(buf, dtype="<f4", count=n * ncols).reshape(n, ncols).copy()
        elif mode == "ascii":
            points = np.loadtxt(f, dtype=np.float32, max_rows=n)
            points = np.atleast_2d(points).reshape(n, ncols)
        elif mode == "binary_compressed":
            comp_size, uncomp_size = np.frombuffer(f.read(8), dtype="<u4")
            raw = _lzf_decompress(f.read(int(comp_size)), int(uncomp_size))
            if len(raw) != uncomp_size or uncomp_size < n * ncols * 4:
                raise ValueError("binary_compressed payload has unexpected size")
            # binary_compressed stores fields as a structure of arrays.
            soa = np.frombuffer(raw, dtype="<f4", count=n * ncols).reshape(ncols, n)
            points = np.ascontiguousarray(soa.T)
        else:
            raise ValueError(f"unsupported DATA mode: {mode}")
    return header, points


def _lzf_decompress(data, expected_size):
    """Pure-python liblzf decompression (as used by PCL's binary_compressed).

    Slow (python byte loop) but dependency-free; meant for validation and
    tooling, not hot paths.
    """
    out = bytearray(expected_size)
    i, o, n = 0, 0, len(data)
    while i < n:
        ctrl = data[i]
        i += 1
        if ctrl < 32:  # literal run of ctrl+1 bytes
            run = ctrl + 1
            out[o:o + run] = data[i:i + run]
            i += run
            o += run
        else:  # back reference
            length = ctrl >> 5
            if length == 7:
                length += data[i]
                i += 1
            length += 2
            ref = o - ((ctrl & 0x1F) << 8) - data[i] - 1
            i += 1
            if ref < 0:
                raise ValueError("corrupt LZF stream (negative back reference)")
            # Byte-by-byte: back references may overlap the output cursor.
            for _ in range(length):
                out[o] = out[ref]
                o += 1
                ref += 1
    return bytes(out[:o])


def voxel_downsample(points, resolution):
    """Voxel-grid downsample an Nx3/Nx4 array to one centroid per voxel.

    Pure numpy: quantize to an integer grid at `resolution`, average all
    points (and extra columns, e.g. intensity) falling into each voxel.
    """
    points = np.asarray(points, dtype=np.float64)
    if points.shape[0] == 0:
        return points.astype(np.float32)
    if resolution <= 0:
        raise ValueError("resolution must be > 0")
    coords = np.floor(points[:, :3] / resolution).astype(np.int64)
    mins = coords.min(axis=0)
    coords -= mins
    dims = coords.max(axis=0) + 1
    # Linear voxel index; int64 is ample for any drivable map extent at >= 1 cm.
    lin = (coords[:, 0] * dims[1] + coords[:, 1]) * dims[2] + coords[:, 2]
    _, inverse, counts = np.unique(lin, return_inverse=True, return_counts=True)
    out = np.zeros((counts.shape[0], points.shape[1]), dtype=np.float64)
    np.add.at(out, inverse, points)
    out /= counts[:, None]
    return out.astype(np.float32)
