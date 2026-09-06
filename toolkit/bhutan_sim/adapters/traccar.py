# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Traccar (github.com/traccar/traccar) position import.

Reads the JSON returned by ``GET /api/positions`` or ``GET /api/reports/route``
on a Traccar server, or a file of forwarded positions, and converts it to
unified-timeline samples. The dashboard accepts the same payloads live at
``POST /api/ingest/traccar``.
"""

from __future__ import annotations

import datetime as _dt
import json
from typing import Any, Dict, List, Optional

from .common import derive_motion

KNOTS_TO_MPS = 0.514444


def _time(value: Any) -> Optional[float]:
    if value is None:
        return None
    if isinstance(value, (int, float)):
        return value / 1000.0 if value > 1e12 else float(value)
    text = str(value).strip()
    if text.endswith("Z"):
        text = text[:-1] + "+00:00"
    try:
        stamp = _dt.datetime.fromisoformat(text)
    except ValueError:
        return None
    if stamp.tzinfo is None:
        stamp = stamp.replace(tzinfo=_dt.timezone.utc)
    return stamp.timestamp()


def position_to_point(position: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """One Traccar position (possibly wrapped in a forward envelope) to a point dict."""
    if "position" in position and isinstance(position["position"], dict):
        position = position["position"]
    t = _time(position.get("fixTime")) or _time(position.get("deviceTime")) or _time(position.get("serverTime"))
    if t is None or position.get("latitude") is None or position.get("longitude") is None:
        return None
    speed = position.get("speed")
    attrs = position.get("attributes") or {}
    return {
        "t": t,
        "lat": float(position["latitude"]),
        "lon": float(position["longitude"]),
        "alt": float(position["altitude"]) if position.get("altitude") is not None else None,
        "speed_mps": float(speed) * KNOTS_TO_MPS if speed is not None else None,
        "heading_deg": float(position["course"]) if position.get("course") is not None else None,
        "device_id": str(position.get("deviceId", "")),
        "throttle": attrs.get("throttle"),
        "brake": attrs.get("brake"),
    }


def read_traccar(path: str, device_id: Optional[str] = None, source: str = "vehicle") -> List[Dict[str, Any]]:
    """Return samples for one device from a Traccar JSON export (list of positions or forward envelopes)."""
    with open(path, "r", encoding="utf-8") as handle:
        data = json.load(handle)
    if isinstance(data, dict):
        data = data.get("positions") or data.get("data") or [data]
    points = [p for p in (position_to_point(item) for item in data) if p is not None]
    if device_id is not None:
        points = [p for p in points if p["device_id"] == str(device_id)]
    return derive_motion(points, source=source)
