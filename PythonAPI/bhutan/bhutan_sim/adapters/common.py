# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Helpers shared by the ingestion adapters."""

from __future__ import annotations

import math
from typing import Any, Dict, Iterable, List, Optional, Sequence

from ..quality import haversine_km
from ..telemetry import Event, RunManifest, Sample, TelemetryLogger


def derive_motion(points: Sequence[Dict[str, Any]], source: str = "vehicle") -> List[Dict[str, Any]]:
    """Turn sparse position fixes into full samples.

    Speed, heading, longitudinal acceleration and grade are derived from
    consecutive fixes whenever the logger did not report them. Points must
    have ``t``, ``lat`` and ``lon``; ``alt``, ``speed_mps`` and ``heading_deg``
    are optional.
    """
    ordered = sorted((p for p in points if p.get("t") is not None), key=lambda p: float(p["t"]))
    samples: List[Dict[str, Any]] = []
    prev: Optional[Dict[str, Any]] = None
    prev_speed = 0.0
    for index, p in enumerate(ordered):
        t = float(p["t"])
        lat, lon = float(p["lat"]), float(p["lon"])
        alt = float(p["alt"]) if p.get("alt") is not None else 0.0
        speed = float(p["speed_mps"]) if p.get("speed_mps") is not None else None
        heading = float(p["heading_deg"]) if p.get("heading_deg") is not None else None
        accel = 0.0
        grade = 0.0
        if prev is not None:
            dt = t - prev["t"]
            km = haversine_km(prev["lat"], prev["lon"], lat, lon)
            if speed is None:
                speed = (km * 1000.0) / dt if dt > 0 else prev_speed
            if heading is None:
                heading = (math.degrees(math.atan2(lon - prev["lon"], lat - prev["lat"])) + 360.0) % 360.0
            accel = (speed - prev_speed) / dt if dt > 0 else 0.0
            if km > 0 and p.get("alt") is not None and prev.get("alt_raw") is not None:
                grade = (alt - prev["alt_raw"]) / (km * 1000.0) * 100.0
        sample = Sample(t=t, frame=index, lat=lat, lon=lon, alt=alt, speed_mps=speed or 0.0, heading_deg=heading or 0.0,
                        accel_x=accel, accel_y=0.0, accel_z=0.0, yaw_rate=0.0, grade_pct=grade, source=source)
        d = sample.to_dict()
        for key in ("accel_y", "accel_z", "yaw_rate", "throttle", "brake", "steer"):
            if p.get(key) is not None:
                d[key] = float(p[key])
        samples.append(d)
        prev = {"t": t, "lat": lat, "lon": lon, "alt_raw": p.get("alt")}
        prev_speed = speed or 0.0
    return samples


def write_run_dir(run_dir: str, manifest: RunManifest, samples: Iterable[Dict[str, Any]], events: Iterable[Dict[str, Any]] = ()) -> Dict[str, Any]:
    """Write samples and events with the standard :class:`TelemetryLogger` layout."""
    logger = TelemetryLogger(run_dir, manifest)
    first = last = None
    for s in samples:
        logger.sample(Sample(**{k: v for k, v in s.items() if k in Sample.__dataclass_fields__}))
        first = s["t"] if first is None else first
        last = s["t"]
    for e in events:
        logger.event(Event(**{k: v for k, v in e.items() if k in Event.__dataclass_fields__}))
    if first is not None:
        manifest.started_at = manifest.started_at or float(first)
        manifest.ended_at = manifest.ended_at or float(last)
    manifest.streams["gnss"] = logger.sample_count > 0
    logger.close()
    return {"run_dir": run_dir, "samples": logger.sample_count, "events": logger.event_count}
