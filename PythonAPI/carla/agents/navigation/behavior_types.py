# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""This module contains the different parameters sets for each behavior."""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum, auto


class BehaviorKind(Enum):
    """Enumeration of available driving behavior types."""

    CAUTIOUS = auto()
    NORMAL = auto()
    AGGRESSIVE = auto()


@dataclass(frozen=True, slots=True)
class BehaviorProfile:
    """Configuration for a specific driving behavior.

    Attributes:
        max_speed: Maximum speed in km/h.
        speed_lim_dist: Distance below speed limit to maintain.
        speed_decrease: Speed reduction when approaching obstacles.
        safety_time: Safety time threshold in seconds.
        min_proximity_threshold: Minimum proximity threshold in meters.
        braking_distance: Braking distance in meters.
        tailgate_counter: Initial tailgate counter value.
    """

    max_speed: float
    speed_lim_dist: float
    speed_decrease: float
    safety_time: float
    min_proximity_threshold: float
    braking_distance: float
    tailgate_counter: int = field(default=0)


CAUTIOUS = BehaviorProfile(
    max_speed=40.0,
    speed_lim_dist=6.0,
    speed_decrease=12.0,
    safety_time=3.0,
    min_proximity_threshold=12.0,
    braking_distance=6.0,
)

NORMAL = BehaviorProfile(
    max_speed=50.0,
    speed_lim_dist=3.0,
    speed_decrease=10.0,
    safety_time=3.0,
    min_proximity_threshold=10.0,
    braking_distance=5.0,
)

AGGRESSIVE = BehaviorProfile(
    max_speed=70.0,
    speed_lim_dist=1.0,
    speed_decrease=8.0,
    safety_time=3.0,
    min_proximity_threshold=8.0,
    braking_distance=4.0,
    tailgate_counter=-1,
)

BEHAVIOR_MAP: dict[BehaviorKind, BehaviorProfile] = {
    BehaviorKind.CAUTIOUS: CAUTIOUS,
    BehaviorKind.NORMAL: NORMAL,
    BehaviorKind.AGGRESSIVE: AGGRESSIVE,
}
