# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module contains the different parameters sets for each behavior. """

from dataclasses import dataclass
from agents.conf.agent_settings_backend import SimpleBehaviorAgentSettings

@dataclass
class Cautious(SimpleBehaviorAgentSettings):
    """Class for Cautious agent."""
    max_speed = 40
    speed_lim_dist = 6
    speed_decrease = 12
    safety_time = 3
    min_proximity_threshold = 12
    braking_distance = 6
    avoid_tailgators = True

@dataclass
class Normal(SimpleBehaviorAgentSettings):
    """Class for Normal agent."""
    max_speed = 50
    speed_lim_dist = 3
    speed_decrease = 10
    safety_time = 3
    min_proximity_threshold = 10
    braking_distance = 5
    avoid_tailgators = True

@dataclass
class Aggressive(SimpleBehaviorAgentSettings):
    """Class for Aggressive agent."""
    max_speed = 70
    speed_lim_dist = 1
    speed_decrease = 8
    safety_time = 3
    min_proximity_threshold = 8
    braking_distance = 4
    avoid_tailgators = False
