from typing import NamedTuple, Union

from carla import Actor, TrafficLight

class ObstacleDetectionResult(NamedTuple):
    obstacle_was_found : bool
    obstacle : Union[Actor, None]
    distance : float 
    # distance : Union[float, Literal[-1]] # Python 3.8+ only

class TrafficLightDetectionResult(NamedTuple):
    traffic_light_was_found : bool
    traffic_light : Union[TrafficLight, None]