# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA Leaderboard-style driving score.

The CARLA Leaderboard (github.com/carla-simulator/leaderboard) scores a route as
``driving_score = route_completion * infraction_penalty`` where the penalty is
the product of one coefficient per infraction. The table below uses the
Leaderboard 2.0 coefficients where an Atlas event class maps onto a Leaderboard
infraction and documents Atlas-specific coefficients for the safety rules that
have no Leaderboard equivalent.

The dashboard computes the same numbers in ``Dashboard/src/driving_score.ts``
when a run is finished; keep the two tables identical.
"""

from __future__ import annotations

import re
from dataclasses import asdict, dataclass, field
from typing import Any, Dict, Iterable, List, Optional

PENALTIES: Dict[str, float] = {
    "collision_pedestrian": 0.50,   # Leaderboard 2.0
    "collision_vehicle": 0.60,      # Leaderboard 2.0
    "collision_static": 0.65,       # Leaderboard 2.0
    "driver_intervention": 0.60,    # Atlas: a disengagement is treated like an agent-blocked event
    "ttc_low": 0.80,                # Atlas: near miss (SR-03)
    "downhill_overspeed": 0.80,     # Atlas: SR-06
    "speeding": 0.90,               # Atlas: SR-02
    "lane_departure": 0.95,         # Atlas: SR-08
    "hard_brake": 0.97,             # Atlas: SR-01
    "lateral_accel_high": 0.97,     # Atlas: SR-04
    "lead_vehicle_close": 0.97,     # Atlas: SR-07
}

_PEDESTRIAN = re.compile(r"walker|pedestrian|person|human|cyclist|bicycle|animal|livestock|cattle|dog", re.I)
_VEHICLE = re.compile(r"vehicle|car|truck|bus|motor|bike|van|shuttle", re.I)


@dataclass
class Infraction:
    event_class: str
    kind: str
    count: int
    penalty: float


@dataclass
class DrivingScore:
    driving_score: float
    route_completion: float
    infraction_penalty: float
    route_completion_basis: str
    infractions: List[Infraction] = field(default_factory=list)

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


def _collision_kind(event: Dict[str, Any]) -> str:
    data = event.get("data") or {}
    if not isinstance(data, dict):
        data = {}
    text = " ".join(str(v) for v in (event.get("description"), data.get("source_event"), data.get("other_actor"), data.get("other"), data.get("with")) if isinstance(v, str))
    if _PEDESTRIAN.search(text):
        return "collision_pedestrian"
    if _VEHICLE.search(text):
        return "collision_vehicle"
    return "collision_static"


def infraction_kind(event: Dict[str, Any]) -> Optional[str]:
    cls = event.get("event_class")
    if cls == "collision":
        return _collision_kind(event)
    return cls if cls in PENALTIES else None


def compute_driving_score(events: Iterable[Dict[str, Any]], actual_duration_s: Optional[float],
                          planned_duration_s: Optional[float]) -> DrivingScore:
    """Score a run from its events.

    ``planned_duration_s`` is the scenario template's duration for simulated
    runs; pass ``None`` for real-world runs, which are assumed complete.
    """
    counts: Dict[str, Infraction] = {}
    penalty = 1.0
    for event in events:
        kind = infraction_kind(event)
        if kind is None:
            continue
        coefficient = PENALTIES[kind]
        penalty *= coefficient
        key = "%s|%s" % (event.get("event_class"), kind)
        entry = counts.get(key)
        if entry is None:
            entry = Infraction(str(event.get("event_class")), kind, 0, coefficient)
            counts[key] = entry
        entry.count += 1
    completion = 1.0
    basis = "assumed_complete"
    if planned_duration_s and planned_duration_s > 0 and actual_duration_s is not None and actual_duration_s >= 0:
        completion = min(1.0, actual_duration_s / planned_duration_s)
        basis = "duration_vs_planned"
    infractions = sorted(counts.values(), key=lambda i: (i.penalty, -i.count))
    return DrivingScore(
        driving_score=round(100.0 * completion * penalty, 4),
        route_completion=round(100.0 * completion, 4),
        infraction_penalty=round(penalty, 4),
        route_completion_basis=basis,
        infractions=infractions,
    )
