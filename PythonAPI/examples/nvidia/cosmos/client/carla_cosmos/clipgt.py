"""The ClipGT scene-package format: Parquet schemas, the rig calibration row, the writer.

This is the *format*, with no CARLA in it — :mod:`carla_cosmos.scene` fills the
tables from a live world, :mod:`carla_cosmos.synthetic` fills them with a
straight-line drive for tests, and both write them through :func:`write_table`.
The schemas were dumped from NVIDIA's ``multiview_example1`` and are what
``cosmos_transfer2/_src/imaginaire/auxiliary/world_scenario/dataloaders/clipgt_loader.py``
reads back.

It needs ``pyarrow`` (the ``capture`` extra); importing the module without it
fails with a message naming the extra, so the core client stays dependency-free.
"""

from __future__ import annotations

import json
import math
from pathlib import Path
from typing import Iterable

import numpy as np

try:
    import pyarrow as pa
    import pyarrow.parquet as pq
except ModuleNotFoundError as exc:  # pragma: no cover - depends on the install
    raise ModuleNotFoundError("carla_cosmos.clipgt needs pyarrow: pip install 'carla-cosmos[capture]'") from exc

from .contracts import CameraManifest

VERSION = 1725328440
"""Constant ``version`` column value (same convention as NVIDIA's example)."""

_XYZ = pa.struct([("x", pa.float32()), ("y", pa.float32()), ("z", pa.float32())])
_QUAT = pa.struct([("x", pa.float32()), ("y", pa.float32()), ("z", pa.float32()), ("w", pa.float32())])
_MAPKEY = pa.struct([("clip_id", pa.string()), ("label_class_id", pa.string()),
                     ("map_id", pa.string()), ("map_id_version", pa.string())])
_TSKEY = pa.struct([("clip_id", pa.string()), ("timestamp_micros", pa.int64())])


def _poly_schema(name: str) -> pa.Schema:
    return pa.schema([("key", _MAPKEY),
                      (name, pa.struct([("category", pa.string()), ("location", pa.list_(_XYZ))])),
                      ("version", pa.uint64())])


SCHEMAS: dict[str, pa.Schema] = {
    "obstacle": pa.schema([
        ("key", pa.struct([("clip_id", pa.string()), ("timestamp_micros", pa.int64()),
                           ("label_class_id", pa.string())])),
        ("obstacle", pa.struct([("trackline_id", pa.string()), ("center", _XYZ), ("size", _XYZ),
                                ("orientation", _QUAT), ("category", pa.string())])),
        ("version", pa.uint64())]),
    "egomotion_estimate": pa.schema([
        ("key", _TSKEY),
        ("egomotion_estimate", pa.struct([("name", pa.string()), ("location", _XYZ), ("orientation", _QUAT)])),
        ("version", pa.uint64())]),
    "calibration_estimate": pa.schema([
        ("key", _TSKEY),
        ("calibration_estimate", pa.struct([("name", pa.string()), ("rig_json", pa.string())])),
        ("version", pa.uint64())]),
    "lane_line": pa.schema([
        ("key", _MAPKEY),
        ("lane_line", pa.struct([("line_rail", pa.list_(_XYZ)), ("styles", pa.list_(pa.string())),
                                 ("colors", pa.list_(pa.string())),
                                 ("left_driving_direction", pa.list_(pa.string())),
                                 ("right_driving_direction", pa.list_(pa.string()))])),
        ("version", pa.uint64())]),
    "road_boundary": _poly_schema("road_boundary"),
    "crosswalk": _poly_schema("crosswalk"),
    "pole": _poly_schema("pole"),
    "road_marking": _poly_schema("road_marking"),
    "wait_line": pa.schema([
        ("key", _MAPKEY),
        ("wait_line", pa.struct([("category", pa.string()), ("location", pa.list_(_XYZ)),
                                 ("is_implicit", pa.bool_()), ("intersection_subtype", pa.string())])),
        ("version", pa.uint64())]),
    "traffic_light": pa.schema([
        ("key", _MAPKEY),
        ("traffic_light", pa.struct([("center", _XYZ), ("dimensions", _XYZ), ("orientation", _QUAT),
                                     ("category", pa.string())])),
        ("version", pa.uint64())]),
    "traffic_sign": pa.schema([
        ("key", _MAPKEY),
        ("traffic_sign", pa.struct([("center", _XYZ), ("dimensions", _XYZ), ("orientation", _QUAT),
                                    ("category", pa.string())])),
        ("version", pa.uint64())]),
}

TABLES: tuple[str, ...] = tuple(SCHEMAS)
"""Every table of a scene package, in no particular order."""

TIMESTAMP_CAMERA = "camera_front_wide_120fov"
"""The camera whose ``<clip_id>.<camera>.json`` carries the clip's timestamps."""


def mapkey(clip_id: str, label: str) -> dict:
    """``key`` of a static map layer row."""
    return {"clip_id": clip_id, "label_class_id": f"minimap:{label}:carla:v0",
            "map_id": "carla", "map_id_version": "1"}


def xyz(x: float, y: float, z: float) -> dict:
    return {"x": float(x), "y": float(y), "z": float(z)}


def write_table(out: Path, clip_id: str, name: str, rows: list[dict]) -> Path:
    """Write one table (possibly empty) as ``<out>/<clip_id>.<name>.parquet``."""
    path = Path(out) / f"{clip_id}.{name}.parquet"
    pq.write_table(pa.Table.from_pylist(rows, schema=SCHEMAS[name]), path)
    return path


def write_timestamps(out: Path, clip_id: str, timestamps: Iterable[int]) -> Path:
    """The camera-timestamps sidecar the ClipGT loader reads the clip's frame times from."""
    path = Path(out) / f"{clip_id}.{TIMESTAMP_CAMERA}.json"
    path.write_text(json.dumps([{"timestamp": int(ts)} for ts in timestamps]))
    return path


# ----------------------------------------------------------------------------- camera model

def pinhole_focal_px(width: int, hfov_deg: float) -> float:
    """Pinhole focal length in pixels for a horizontal FOV."""
    return (width / 2.0) / math.tan(math.radians(hfov_deg) / 2.0)


def pinhole_ftheta_poly(width: int, height: int, hfov_deg: float, order: int = 5,
                        samples: int = 400) -> tuple[list[float], float]:
    """Fit NVIDIA's ``pixeldistance-to-angle`` polynomial to a pinhole camera.

    theta(r) = sum_{i=1..order} k_i r^i (k_0 = 0) is least-squares fitted to
    ``atan(r / f)`` on ``r in [0, r_corner]``.  Returns ``(poly[6], max_resid_rad)``.
    A pinhole is not a polynomial, so the residual grows with FOV; for the 120
    degree views it stays below 1e-3 rad at 1280x720 (checked in tests).
    """
    f = pinhole_focal_px(width, hfov_deg)
    r_max = math.hypot(width / 2.0, height / 2.0)
    r = np.linspace(0.0, r_max, samples)
    theta = np.arctan(r / f)
    design = np.stack([r ** i for i in range(1, order + 1)], axis=1)
    k, *_ = np.linalg.lstsq(design, theta, rcond=None)
    poly = [0.0] + [float(v) for v in k]
    poly += [0.0] * (6 - len(poly))
    resid = float(np.abs(design @ k - theta).max())
    return poly, resid


def calibration_row(clip_id: str, timestamp_micros: int, cameras: Iterable[CameraManifest]) -> dict:
    """The single ``calibration_estimate`` row: the rig, as NVIDIA's loader wants it."""
    sensors = []
    for cam in cameras:
        measured = getattr(cam, "ftheta", None)
        if measured is None:
            # No measured lens: fit the polynomial that reproduces this camera's pinhole FOV.
            poly, _ = pinhole_ftheta_poly(cam.width, cam.height, cam.hfov)
            cx, cy = cam.width / 2.0, cam.height / 2.0
            linear_c, linear_d, linear_e = 1.0, 0.0, 0.0
        else:
            # A real lens (a NuRec clip's own calibration): write it verbatim, never re-fit.
            poly = list(measured.poly)
            cx, cy = float(measured.cx), float(measured.cy)
            linear_c, linear_d, linear_e = (float(v) for v in measured.linear_cde)
        sensors.append({
            "name": cam.name,
            "properties": {"Model": "ftheta", "cx": cx, "cy": cy,
                           "width": cam.width, "height": cam.height,
                           "polynomial": " ".join(f"{k:.10g}" for k in poly),
                           "polynomial-type": "pixeldistance-to-angle",
                           "linear-c": linear_c, "linear-d": linear_d, "linear-e": linear_e},
            "nominalSensor2Rig_FLU": {"t": list(cam.t_flu), "roll-pitch-yaw": list(cam.rpy_flu)},
        })
    return {"key": {"clip_id": clip_id, "timestamp_micros": int(timestamp_micros)},
            "calibration_estimate": {"name": "carla_rig", "rig_json": json.dumps({"rig": {"sensors": sensors}})},
            "version": VERSION}
