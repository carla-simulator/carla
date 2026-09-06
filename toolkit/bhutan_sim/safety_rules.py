# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Safety-rule checks over a telemetry timeline.

Rules are deliberately simple and explainable: each violation names the rule,
the measured value and the threshold so it can be reviewed by a safety
manager. Thresholds are per vehicle class because a laden truck has different
limits from a shuttle or a car.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict, Iterable, List, Optional

from .telemetry import Event

KPH = 3.6


@dataclass(frozen=True)
class Thresholds:
    hard_brake_mps2: float
    lateral_mps2: float
    ttc_s: float
    downhill_grade_pct: float
    downhill_speed_kph: float
    speeding_margin: float          # fraction over the limit
    following_headway_s: float


THRESHOLDS: Dict[str, Thresholds] = {
    "truck": Thresholds(hard_brake_mps2=3.0, lateral_mps2=2.5, ttc_s=2.0, downhill_grade_pct=-6.0,
                        downhill_speed_kph=40.0, speeding_margin=0.10, following_headway_s=3.0),
    "shuttle": Thresholds(hard_brake_mps2=3.5, lateral_mps2=3.0, ttc_s=1.8, downhill_grade_pct=-8.0,
                          downhill_speed_kph=45.0, speeding_margin=0.10, following_headway_s=2.5),
    "car": Thresholds(hard_brake_mps2=4.5, lateral_mps2=4.0, ttc_s=1.5, downhill_grade_pct=-10.0,
                      downhill_speed_kph=60.0, speeding_margin=0.15, following_headway_s=2.0),
}

RULES = {
    "SR-01": ("hard_brake", "warning", "Longitudinal deceleration above the hard-braking threshold"),
    "SR-02": ("speeding", "warning", "Speed above the posted limit plus margin"),
    "SR-03": ("ttc_low", "critical", "Time-to-collision with the lead object below threshold"),
    "SR-04": ("lateral_accel_high", "warning", "Lateral acceleration near the load-shift limit"),
    "SR-05": ("collision", "critical", "Collision registered by the collision sensor"),
    "SR-06": ("downhill_overspeed", "critical", "Speed above the descent limit on a steep grade"),
    "SR-07": ("lead_vehicle_close", "warning", "Following headway below threshold"),
    "SR-08": ("lane_departure", "warning", "Lane departure without an obstacle avoidance context"),
}

CRITICAL_RULES = tuple(rule for rule, (_, severity, _) in RULES.items() if severity == "critical")


def _rule_event(rule_id: str, sample: Dict[str, Any], value: float, threshold: float, extra: Optional[Dict[str, Any]] = None) -> Event:
    event_class, severity, description = RULES[rule_id]
    data = {"value": round(value, 3), "threshold": threshold}
    if extra:
        data.update(extra)
    return Event(
        t=float(sample["t"]),
        frame=int(sample.get("frame", 0)),
        event_class=event_class,
        severity=severity,
        lat=sample.get("lat"),
        lon=sample.get("lon"),
        description=description,
        rule_id=rule_id,
        data=data,
        review_status="classified",
    )


def check_samples(samples: Iterable[Dict[str, Any]], vehicle_class: str = "truck",
                  speed_limit_kph: Optional[float] = None, debounce_s: float = 1.0) -> List[Event]:
    """Run the sample-level rules and return violation events.

    Consecutive violations of the same rule within ``debounce_s`` are merged
    so a two-second hard brake counts once rather than forty times.
    """
    th = THRESHOLDS.get(vehicle_class, THRESHOLDS["car"])
    violations: List[Event] = []
    last_fired: Dict[str, float] = {}

    def fire(rule_id: str, sample: Dict[str, Any], value: float, threshold: float, extra: Optional[Dict[str, Any]] = None) -> None:
        t = float(sample["t"])
        if rule_id in last_fired and t - last_fired[rule_id] < debounce_s:
            last_fired[rule_id] = t
            return
        last_fired[rule_id] = t
        violations.append(_rule_event(rule_id, sample, value, threshold, extra))

    for sample in samples:
        speed = float(sample.get("speed_mps", 0.0))
        accel_x = float(sample.get("accel_x", 0.0))
        accel_y = float(sample.get("accel_y", 0.0))
        grade = float(sample.get("grade_pct", 0.0))
        lead = sample.get("lead_distance_m")
        rel = sample.get("lead_rel_speed_mps")

        if -accel_x >= th.hard_brake_mps2:
            fire("SR-01", sample, -accel_x, th.hard_brake_mps2)
        if speed_limit_kph and speed * KPH > speed_limit_kph * (1.0 + th.speeding_margin):
            fire("SR-02", sample, speed * KPH, speed_limit_kph * (1.0 + th.speeding_margin))
        if lead is not None and rel is not None and rel < -0.1:
            ttc = float(lead) / (-float(rel))
            if ttc < th.ttc_s:
                fire("SR-03", sample, ttc, th.ttc_s, {"lead_distance_m": lead})
        if abs(accel_y) >= th.lateral_mps2:
            fire("SR-04", sample, abs(accel_y), th.lateral_mps2)
        if grade <= th.downhill_grade_pct and speed * KPH > th.downhill_speed_kph:
            fire("SR-06", sample, speed * KPH, th.downhill_speed_kph, {"grade_pct": grade})
        if lead is not None and speed > 1.0:
            headway = float(lead) / speed
            if headway < th.following_headway_s:
                fire("SR-07", sample, headway, th.following_headway_s, {"lead_distance_m": lead})
    return violations


def classify_events(events: Iterable[Dict[str, Any]]) -> List[Event]:
    """Promote sensor events (collision, lane invasion) into rule violations."""
    out: List[Event] = []
    for raw in events:
        if raw.get("rule_id"):
            continue
        if raw.get("event_class") == "collision":
            out.append(_rule_event("SR-05", raw, 1.0, 0.0, {"source_event": raw.get("description", "")}))
        elif raw.get("event_class") == "lane_departure":
            out.append(_rule_event("SR-08", raw, 1.0, 0.0, {"source_event": raw.get("description", "")}))
    return out


def summarize(violations: Iterable[Event]) -> Dict[str, Any]:
    counts: Dict[str, int] = {}
    critical = 0
    for v in violations:
        counts[v.rule_id or v.event_class] = counts.get(v.rule_id or v.event_class, 0) + 1
        if v.severity == "critical":
            critical += 1
    return {"by_rule": counts, "critical": critical, "total": sum(counts.values())}
