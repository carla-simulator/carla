# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""GPX import and export (dashcams, phone loggers, GPS units).

Only the standard library is used so field laptops need nothing extra. Speed
and course are read from the common GPX extensions when present and derived
from consecutive fixes otherwise.
"""

from __future__ import annotations

import datetime as _dt
import xml.etree.ElementTree as ET
from typing import Any, Dict, Iterable, List

from .common import derive_motion

GPX_NS = "http://www.topografix.com/GPX/1/1"


def _local(tag: str) -> str:
    return tag.rsplit("}", 1)[-1]


def _parse_time(text: str) -> float:
    text = text.strip()
    if text.endswith("Z"):
        text = text[:-1] + "+00:00"
    stamp = _dt.datetime.fromisoformat(text)
    if stamp.tzinfo is None:
        stamp = stamp.replace(tzinfo=_dt.timezone.utc)
    return stamp.timestamp()


def read_gpx(path: str, source: str = "vehicle") -> List[Dict[str, Any]]:
    """Return unified-timeline samples for every track point in a GPX file."""
    tree = ET.parse(path)
    points: List[Dict[str, Any]] = []
    for element in tree.iter():
        if _local(element.tag) not in ("trkpt", "rtept"):
            continue
        point: Dict[str, Any] = {"lat": float(element.attrib["lat"]), "lon": float(element.attrib["lon"])}
        for child in element.iter():
            name = _local(child.tag)
            text = (child.text or "").strip()
            if not text:
                continue
            if name == "time":
                point["t"] = _parse_time(text)
            elif name == "ele":
                point["alt"] = float(text)
            elif name in ("speed", "gpxtpx:speed"):
                point["speed_mps"] = float(text)
            elif name in ("course", "bearing", "heading"):
                point["heading_deg"] = float(text)
        if "t" in point:
            points.append(point)
    return derive_motion(points, source=source)


def write_gpx(samples: Iterable[Dict[str, Any]], path: str, name: str = "Bhutan Atlas run") -> str:
    """Write samples as a GPX 1.1 track (for QGIS, GPS units and GPX-based tools)."""
    ET.register_namespace("", GPX_NS)
    root = ET.Element("{%s}gpx" % GPX_NS, {"version": "1.1", "creator": "bhutan_sim"})
    trk = ET.SubElement(root, "{%s}trk" % GPX_NS)
    ET.SubElement(trk, "{%s}name" % GPX_NS).text = name
    seg = ET.SubElement(trk, "{%s}trkseg" % GPX_NS)
    for s in samples:
        pt = ET.SubElement(seg, "{%s}trkpt" % GPX_NS, {"lat": "%.7f" % float(s["lat"]), "lon": "%.7f" % float(s["lon"])})
        if s.get("alt") is not None:
            ET.SubElement(pt, "{%s}ele" % GPX_NS).text = "%.1f" % float(s["alt"])
        ET.SubElement(pt, "{%s}time" % GPX_NS).text = _dt.datetime.fromtimestamp(float(s["t"]), _dt.timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3] + "Z"
    ET.ElementTree(root).write(path, encoding="utf-8", xml_declaration=True)
    return path
