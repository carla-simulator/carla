"""
Module to add high-level semantic return types for obstacle and traffic light detection results via named tuples.

The code is compatible with Python 2.7, <3.6 and >=3.6. The later uses the typed version of named tuples.
"""


import sys
if sys.version_info < (3, 6):
    from collections import namedtuple
    ObstacleDetectionResult = namedtuple('ObstacleDetectionResult', ['obstacle_was_found', 'obstacle', 'distance'])
    TrafficLightDetectionResult = namedtuple('TrafficLightDetectionResult', ['traffic_light_was_found', 'traffic_light'])
else:
    from typing import NamedTuple, Union, TYPE_CHECKING
    from carla import Actor, TrafficLight
    """
    # Python 3.6+, incompatible with Python 2.7 syntax
    class ObstacleDetectionResult(NamedTuple):
        obstacle_was_found : bool
        obstacle : Union[Actor, None]
        distance : float
        # distance : Union[float, Literal[-1]] # Python 3.8+ only

    class TrafficLightDetectionResult(NamedTuple):
        traffic_light_was_found : bool
        traffic_light : Union[TrafficLight, None]
    """
    if TYPE_CHECKING:
        from typing import Literal
        ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', Union[float, Literal[-1]])])
    else:
        ObstacleDetectionResult = NamedTuple('ObstacleDetectionResult', [('obstacle_was_found', bool), ('obstacle', Union[Actor, None]), ('distance', float)])

    TrafficLightDetectionResult = NamedTuple('TrafficLightDetectionResult', [('traffic_light_was_found', bool), ('traffic_light', Union[TrafficLight, None])])
