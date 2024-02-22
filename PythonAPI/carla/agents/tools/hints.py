
import sys
if sys.version_info < (3, 6):
    from collections import namedtuple
    ObstacleDetectionResult = namedtuple('ObstacleDetectionResult', ['obstacle_was_found', 'obstacle', 'distance'])
    TrafficLightDetectionResult = namedtuple('TrafficLightDetectionResult', ['traffic_light_was_found', 'traffic_light'])
else:
    #from agents.tools.hints import ObstacleDetectionResult, TrafficLightDetectionResult
    from typing import NamedTuple, Union
    from carla import Actor, TrafficLight
    """
    class ObstacleDetectionResult(NamedTuple):
        obstacle_was_found : bool
        obstacle : Union[Actor, None]
        distance : float 
        # distance : Union[float, Literal[-1]] # Python 3.8+ only

    class TrafficLightDetectionResult(NamedTuple):
        traffic_light_was_found : bool
        traffic_light : Union[TrafficLight, None]
    """
    ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', float)])
    TrafficLightDetectionResult = NamedTuple('TrafficLightDetectionResult', [('traffic_light_was_found', bool), ('traffic_light', Union[TrafficLight, None])])
