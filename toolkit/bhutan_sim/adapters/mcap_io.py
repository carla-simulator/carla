# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""MCAP import and export (github.com/foxglove/mcap).

MCAP is the container used by rosbag2, Foxglove Studio and most modern
robotics loggers. This adapter needs the ``mcap`` package
(``pip install mcap``); the rest of ``bhutan_sim`` does not.

Export writes three JSON-encoded channels that Foxglove renders natively:

* ``/gnss/fix``        foxglove.LocationFix (map panel)
* ``/atlas/telemetry`` the full unified-timeline sample (plot panel)
* ``/atlas/events``    foxglove.Log (log panel)

Import reads JSON-encoded ``foxglove.LocationFix`` / ``sensor_msgs/NavSatFix``
style messages (``latitude``/``longitude``) plus optional speed and heading
fields, and Atlas telemetry samples written by this module or the dashboard.
"""

from __future__ import annotations

import json
from typing import Any, Dict, Iterable, List, Optional

from .common import derive_motion

try:  # pragma: no cover - import guard
    from mcap.reader import make_reader
    from mcap.writer import Writer
    HAVE_MCAP = True
except ImportError:  # pragma: no cover
    HAVE_MCAP = False

LOG_LEVEL = {"info": 2, "warning": 3, "critical": 4}

LOCATION_FIX_SCHEMA = {
    "title": "foxglove.LocationFix",
    "type": "object",
    "properties": {
        "timestamp": {"type": "object", "properties": {"sec": {"type": "integer"}, "nsec": {"type": "integer"}}},
        "frame_id": {"type": "string"},
        "latitude": {"type": "number"},
        "longitude": {"type": "number"},
        "altitude": {"type": "number"},
        "position_covariance": {"type": "array", "items": {"type": "number"}},
        "position_covariance_type": {"type": "integer"},
    },
}

LOG_SCHEMA = {
    "title": "foxglove.Log",
    "type": "object",
    "properties": {
        "timestamp": {"type": "object", "properties": {"sec": {"type": "integer"}, "nsec": {"type": "integer"}}},
        "level": {"type": "integer"},
        "message": {"type": "string"},
        "name": {"type": "string"},
        "file": {"type": "string"},
        "line": {"type": "integer"},
    },
}

SAMPLE_SCHEMA = {
    "title": "atlas.TelemetrySample",
    "type": "object",
    "properties": {k: {"type": "string" if k == "source" else "number"} for k in (
        "t", "frame", "lat", "lon", "alt", "speed_mps", "heading_deg", "accel_x", "accel_y", "accel_z", "yaw_rate",
        "throttle", "brake", "steer", "grade_pct", "lead_distance_m", "lead_rel_speed_mps", "source")},
}


def _require() -> None:
    if not HAVE_MCAP:
        raise RuntimeError("the mcap package is required: pip install mcap")


def _stamp(t: float) -> Dict[str, int]:
    sec = int(t)
    return {"sec": sec, "nsec": int(round((t - sec) * 1e9))}


def write_mcap(path: str, samples: Iterable[Dict[str, Any]], events: Iterable[Dict[str, Any]] = (), metadata: Optional[Dict[str, str]] = None) -> str:
    """Write samples and events to an MCAP file readable by Foxglove Studio."""
    _require()
    with open(path, "wb") as handle:
        writer = Writer(handle)
        writer.start(profile="", library="bhutan_sim")
        fix_schema = writer.register_schema("foxglove.LocationFix", "jsonschema", json.dumps(LOCATION_FIX_SCHEMA).encode("utf-8"))
        log_schema = writer.register_schema("foxglove.Log", "jsonschema", json.dumps(LOG_SCHEMA).encode("utf-8"))
        sample_schema = writer.register_schema("atlas.TelemetrySample", "jsonschema", json.dumps(SAMPLE_SCHEMA).encode("utf-8"))
        fix_channel = writer.register_channel("/gnss/fix", "json", fix_schema)
        sample_channel = writer.register_channel("/atlas/telemetry", "json", sample_schema)
        log_channel = writer.register_channel("/atlas/events", "json", log_schema)
        if metadata:
            writer.add_metadata("atlas.run", {k: str(v) for k, v in metadata.items()})
        for seq, s in enumerate(samples):
            ns = int(round(float(s["t"]) * 1e9))
            fix = {"timestamp": _stamp(float(s["t"])), "frame_id": "gnss", "latitude": s["lat"], "longitude": s["lon"], "altitude": s.get("alt", 0.0) or 0.0,
                   "position_covariance": [0.0] * 9, "position_covariance_type": 0}
            writer.add_message(fix_channel, ns, json.dumps(fix).encode("utf-8"), ns, sequence=seq)
            writer.add_message(sample_channel, ns, json.dumps(s).encode("utf-8"), ns, sequence=seq)
        for seq, e in enumerate(events):
            ns = int(round(float(e["t"]) * 1e9))
            rule = e.get("rule_id")
            log = {"timestamp": _stamp(float(e["t"])), "level": LOG_LEVEL.get(e.get("severity", "info"), 2),
                   "message": ("%s%s: %s" % (e.get("event_class"), " [%s]" % rule if rule else "", e.get("description") or "")).strip(),
                   "name": rule or e.get("event_class", ""), "file": str(metadata.get("run_id", "")) if metadata else "", "line": int(e.get("frame") or 0)}
            writer.add_message(log_channel, ns, json.dumps(log).encode("utf-8"), ns, sequence=seq)
        writer.finish()
    return path


def read_mcap(path: str, topics: Optional[List[str]] = None, source: str = "vehicle") -> List[Dict[str, Any]]:
    """Return unified-timeline samples from JSON-encoded position messages in an MCAP file.

    Atlas telemetry samples are returned as-is; LocationFix / NavSatFix style
    messages are converted with speed and heading derived when absent.
    """
    _require()
    atlas: List[Dict[str, Any]] = []
    points: List[Dict[str, Any]] = []
    with open(path, "rb") as handle:
        reader = make_reader(handle)
        for schema, channel, message in reader.iter_messages(topics=topics):
            if channel.message_encoding != "json":
                continue
            try:
                payload = json.loads(message.data.decode("utf-8"))
            except (UnicodeDecodeError, ValueError):
                continue
            if not isinstance(payload, dict):
                continue
            name = schema.name if schema else ""
            if name == "atlas.TelemetrySample" or channel.topic == "/atlas/telemetry":
                atlas.append(payload)
                continue
            lat = payload.get("latitude", payload.get("lat"))
            lon = payload.get("longitude", payload.get("lon"))
            if lat is None or lon is None:
                continue
            stamp = payload.get("timestamp") or payload.get("header", {}).get("stamp") or {}
            if isinstance(stamp, dict) and "sec" in stamp:
                t = float(stamp["sec"]) + float(stamp.get("nsec", stamp.get("nanosec", 0))) / 1e9
            else:
                t = message.log_time / 1e9
            points.append({"t": t, "lat": float(lat), "lon": float(lon), "alt": payload.get("altitude", payload.get("alt")),
                           "speed_mps": payload.get("speed_mps", payload.get("speed")), "heading_deg": payload.get("heading_deg", payload.get("heading", payload.get("course")))})
    if atlas:
        return sorted(atlas, key=lambda s: float(s["t"]))
    return derive_motion(points, source=source)
