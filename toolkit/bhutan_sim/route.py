# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Digital route reconstruction from real GNSS traces.

Turns a passive-capture GNSS log into route archetype statistics (curvature
and grade per segment) and recommends scenario families whose parameter
ranges cover the observed geometry. This closes the loop from real-world
routes to targeted synthetic generation.
"""

from __future__ import annotations

import csv
import json
import math
from dataclasses import asdict, dataclass
from typing import Any, Dict, Iterable, List, Sequence

from .quality import haversine_km
from .taxonomy import Taxonomy


@dataclass
class RouteSegment:
    index: int
    start_lat: float
    start_lon: float
    length_m: float
    curvature: float     # mean absolute curvature, 1/m
    grade_pct: float
    route_class: str

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


def _bearing(lat1: float, lon1: float, lat2: float, lon2: float) -> float:
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dl = math.radians(lon2 - lon1)
    x = math.sin(dl) * math.cos(p2)
    y = math.cos(p1) * math.sin(p2) - math.sin(p1) * math.cos(p2) * math.cos(dl)
    return math.degrees(math.atan2(x, y)) % 360.0


def classify(curvature: float, grade_pct: float) -> str:
    if curvature >= 0.08:
        geometry = "hairpin"
    elif curvature >= 0.03:
        geometry = "curve"
    else:
        geometry = "straight"
    if grade_pct <= -6:
        slope = "steep_descent"
    elif grade_pct >= 6:
        slope = "steep_climb"
    else:
        slope = "flat"
    return "%s_%s" % (geometry, slope)


def load_trace(path: str) -> List[Dict[str, float]]:
    """Load a GNSS trace from CSV (lat,lon[,alt][,t]) or JSONL."""
    rows: List[Dict[str, float]] = []
    if path.endswith(".jsonl"):
        with open(path, "r", encoding="utf-8") as handle:
            for line in handle:
                if line.strip():
                    rows.append(json.loads(line))
    else:
        with open(path, "r", encoding="utf-8", newline="") as handle:
            for row in csv.DictReader(handle):
                rows.append({k: float(v) for k, v in row.items() if v not in (None, "")})
    return [r for r in rows if "lat" in r and "lon" in r]


def segment_route(points: Sequence[Dict[str, float]], segment_len_m: float = 100.0, min_step_m: float = 3.0) -> List[RouteSegment]:
    """Split a trace into segments and measure curvature and grade for each."""
    # Thin the trace so bearing changes are measured over meaningful distances.
    thinned: List[Dict[str, float]] = []
    for p in points:
        if not thinned:
            thinned.append(p)
            continue
        q = thinned[-1]
        if haversine_km(q["lat"], q["lon"], p["lat"], p["lon"]) * 1000.0 >= min_step_m:
            thinned.append(p)
    segments: List[RouteSegment] = []
    if len(thinned) < 3:
        return segments
    seg_pts: List[Dict[str, float]] = [thinned[0]]
    seg_len = 0.0
    index = 0
    for prev, cur in zip(thinned, thinned[1:]):
        step = haversine_km(prev["lat"], prev["lon"], cur["lat"], cur["lon"]) * 1000.0
        seg_pts.append(cur)
        seg_len += step
        if seg_len >= segment_len_m:
            segments.append(_measure(index, seg_pts, seg_len))
            index += 1
            seg_pts = [cur]
            seg_len = 0.0
    if seg_len >= segment_len_m * 0.3 and len(seg_pts) >= 3:
        segments.append(_measure(index, seg_pts, seg_len))
    return segments


def _measure(index: int, pts: Sequence[Dict[str, float]], length_m: float) -> RouteSegment:
    curv_sum = 0.0
    n = 0
    for a, b, c in zip(pts, pts[1:], pts[2:]):
        b1 = _bearing(a["lat"], a["lon"], b["lat"], b["lon"])
        b2 = _bearing(b["lat"], b["lon"], c["lat"], c["lon"])
        dyaw = abs((b2 - b1 + 180.0) % 360.0 - 180.0)
        ds = haversine_km(b["lat"], b["lon"], c["lat"], c["lon"]) * 1000.0
        if ds > 0:
            curv_sum += math.radians(dyaw) / ds
            n += 1
    curvature = curv_sum / n if n else 0.0
    alt0, alt1 = pts[0].get("alt"), pts[-1].get("alt")
    grade = ((alt1 - alt0) / length_m * 100.0) if (alt0 is not None and alt1 is not None and length_m > 0) else 0.0
    return RouteSegment(index, pts[0]["lat"], pts[0]["lon"], round(length_m, 1), round(curvature, 4), round(grade, 2), classify(curvature, grade))


def recommend_families(segments: Iterable[RouteSegment], taxonomy: Taxonomy) -> List[Dict[str, Any]]:
    """Rank families by how many observed segments fall inside their ranges."""
    segments = list(segments)
    ranked = []
    for fam in taxonomy.families:
        c_lo, c_hi = fam.ranges.get("road_curvature", (0.0, 1.0))
        g_lo, g_hi = fam.ranges.get("grade_pct", (-100.0, 100.0))
        hits = sum(1 for s in segments if c_lo <= s.curvature <= c_hi and g_lo <= s.grade_pct <= g_hi)
        if hits:
            ranked.append({"family": fam.id, "group": fam.group, "matching_segments": hits,
                           "coverage": round(hits / len(segments), 3) if segments else 0.0})
    ranked.sort(key=lambda r: (-r["matching_segments"], r["family"]))
    return ranked


def profile(points: Sequence[Dict[str, float]], taxonomy: Taxonomy) -> Dict[str, Any]:
    segments = segment_route(points)
    classes: Dict[str, int] = {}
    for s in segments:
        classes[s.route_class] = classes.get(s.route_class, 0) + 1
    return {
        "points": len(points),
        "segments": [s.to_dict() for s in segments],
        "route_classes": classes,
        "total_km": round(sum(s.length_m for s in segments) / 1000.0, 3),
        "recommended_families": recommend_families(segments, taxonomy),
    }
