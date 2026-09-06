# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Data-quality gates for collected runs.

A run is split into fixed-length segments; each segment passes or fails a set
of gates. The acceptance rate across segments feeds the "data-quality
acceptance rate" KPI, and the per-run flags feed "replay completeness".
"""

from __future__ import annotations

import math
from dataclasses import asdict, dataclass, field
from typing import Any, Dict, Iterable, List, Optional, Sequence

REQUIRED_FIELDS = ("t", "frame", "lat", "lon", "speed_mps", "heading_deg", "accel_x", "accel_y")


@dataclass
class Gate:
    id: str
    description: str
    passed: bool
    value: float
    threshold: float


@dataclass
class SegmentReport:
    index: int
    t_start: float
    t_end: float
    sample_count: int
    gates: List[Gate] = field(default_factory=list)

    @property
    def passed(self) -> bool:
        return all(g.passed for g in self.gates)

    def to_dict(self) -> Dict[str, Any]:
        data = asdict(self)
        data["passed"] = self.passed
        return data


@dataclass
class QualityReport:
    run_id: str
    sample_count: int
    duration_s: float
    distance_km: float
    expected_rate_hz: float
    segments: List[SegmentReport]
    streams: Dict[str, bool]
    privacy_status: str
    issues: List[str] = field(default_factory=list)

    @property
    def accepted_segments(self) -> int:
        return sum(1 for s in self.segments if s.passed)

    @property
    def acceptance_rate(self) -> float:
        return self.accepted_segments / len(self.segments) if self.segments else 0.0

    @property
    def replay_complete(self) -> bool:
        return all(self.streams.get(k, False) for k in ("gnss", "imu", "events")) and self.sample_count > 0

    @property
    def passed(self) -> bool:
        return self.acceptance_rate >= 0.9 and not self.issues

    def to_dict(self) -> Dict[str, Any]:
        return {
            "run_id": self.run_id,
            "sample_count": self.sample_count,
            "duration_s": round(self.duration_s, 3),
            "distance_km": round(self.distance_km, 4),
            "expected_rate_hz": self.expected_rate_hz,
            "segments": [s.to_dict() for s in self.segments],
            "segment_count": len(self.segments),
            "accepted_segments": self.accepted_segments,
            "acceptance_rate": round(self.acceptance_rate, 4),
            "streams": dict(self.streams),
            "replay_complete": self.replay_complete,
            "privacy_status": self.privacy_status,
            "issues": list(self.issues),
            "passed": self.passed,
        }


def haversine_km(lat1: float, lon1: float, lat2: float, lon2: float) -> float:
    r = 6371.0088
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dphi = p2 - p1
    dlmb = math.radians(lon2 - lon1)
    a = math.sin(dphi / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dlmb / 2) ** 2
    return 2 * r * math.asin(math.sqrt(a))


def route_distance_km(samples: Sequence[Dict[str, Any]]) -> float:
    total = 0.0
    for prev, cur in zip(samples, samples[1:]):
        try:
            total += haversine_km(float(prev["lat"]), float(prev["lon"]), float(cur["lat"]), float(cur["lon"]))
        except (KeyError, TypeError, ValueError):
            continue
    return total


def _segment_gates(samples: Sequence[Dict[str, Any]], expected_rate_hz: float, segment_len_s: float) -> List[Gate]:
    gates: List[Gate] = []
    n = len(samples)
    expected = expected_rate_hz * segment_len_s
    completeness = n / expected if expected > 0 else 0.0
    gates.append(Gate("Q1_completeness", "Sample count relative to expected rate", completeness >= 0.8, round(completeness, 3), 0.8))

    times = [float(s["t"]) for s in samples if "t" in s]
    max_gap = 0.0
    non_monotonic = 0
    for a, b in zip(times, times[1:]):
        gap = b - a
        if gap < 0:
            non_monotonic += 1
        max_gap = max(max_gap, gap)
    nominal_dt = 1.0 / expected_rate_hz if expected_rate_hz > 0 else 1.0
    gates.append(Gate("Q2_timestamp_drift", "Largest gap between consecutive samples versus nominal period", max_gap <= nominal_dt * 5, round(max_gap, 4), round(nominal_dt * 5, 4)))
    gates.append(Gate("Q3_monotonic", "Timestamps never decrease", non_monotonic == 0, non_monotonic, 0))

    missing = 0
    for s in samples:
        for key in REQUIRED_FIELDS:
            if s.get(key) is None:
                missing += 1
    missing_ratio = missing / (n * len(REQUIRED_FIELDS)) if n else 1.0
    gates.append(Gate("Q4_missing_fields", "Fraction of required fields that are missing", missing_ratio <= 0.02, round(missing_ratio, 4), 0.02))

    frozen = 0
    for a, b in zip(samples, samples[1:]):
        if a.get("lat") == b.get("lat") and a.get("lon") == b.get("lon") and float(b.get("speed_mps", 0.0)) > 1.0:
            frozen += 1
    frozen_ratio = frozen / (n - 1) if n > 1 else 0.0
    gates.append(Gate("Q5_gnss_frozen", "Position unchanged while moving (GNSS stall)", frozen_ratio <= 0.1, round(frozen_ratio, 4), 0.1))
    return gates


def assess_run(run_id: str, samples: Sequence[Dict[str, Any]], events: Sequence[Dict[str, Any]],
               streams: Optional[Dict[str, bool]] = None, expected_rate_hz: float = 20.0,
               segment_len_s: float = 30.0, privacy_status: str = "unprocessed") -> QualityReport:
    """Split a run into segments and evaluate every gate."""
    samples = sorted(samples, key=lambda s: float(s.get("t", 0.0)))
    streams = dict(streams or {})
    streams.setdefault("gnss", any(s.get("lat") is not None for s in samples))
    streams.setdefault("imu", any(s.get("accel_x") is not None for s in samples))
    streams.setdefault("events", len(events) > 0)
    streams.setdefault("video", False)
    streams.setdefault("can", any(s.get("throttle") is not None and s.get("source") == "vehicle" for s in samples))

    issues: List[str] = []
    if not samples:
        issues.append("no telemetry samples")
        return QualityReport(run_id, 0, 0.0, 0.0, expected_rate_hz, [], streams, privacy_status, issues)

    t0 = float(samples[0]["t"])
    t1 = float(samples[-1]["t"])
    duration = max(0.0, t1 - t0)
    segments: List[SegmentReport] = []
    count = max(1, int(math.ceil(duration / segment_len_s))) if duration > 0 else 1
    for index in range(count):
        start = t0 + index * segment_len_s
        end = start + segment_len_s
        chunk = [s for s in samples if start <= float(s["t"]) < end or (index == count - 1 and float(s["t"]) == end)]
        # The trailing segment may be short; scale expectation by its real length.
        seg_len = min(segment_len_s, max(1e-6, t1 - start)) if index == count - 1 else segment_len_s
        gates = _segment_gates(chunk, expected_rate_hz, seg_len) if chunk else [Gate("Q1_completeness", "Sample count relative to expected rate", False, 0.0, 0.8)]
        segments.append(SegmentReport(index, start, end, len(chunk), gates))

    if privacy_status not in ("unprocessed", "redacted", "not_required"):
        issues.append("unknown privacy status '%s'" % privacy_status)
    if streams.get("video") and privacy_status == "unprocessed":
        issues.append("video present but redaction not run")

    return QualityReport(
        run_id=run_id,
        sample_count=len(samples),
        duration_s=duration,
        distance_km=route_distance_km(samples),
        expected_rate_hz=expected_rate_hz,
        segments=segments,
        streams=streams,
        privacy_status=privacy_status,
        issues=issues,
    )


def events_per_100km(events: Iterable[Dict[str, Any]], distance_km: float) -> Dict[str, float]:
    """Edge-case discovery rate by event class."""
    counts: Dict[str, int] = {}
    for e in events:
        cls = e.get("event_class", "unknown")
        counts[cls] = counts.get(cls, 0) + 1
    if distance_km <= 0:
        return {k: 0.0 for k in counts}
    return {k: round(v * 100.0 / distance_km, 3) for k, v in counts.items()}
