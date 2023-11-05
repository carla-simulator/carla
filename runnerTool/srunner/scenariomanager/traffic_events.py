#!/usr/bin/env python

# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Collection of TrafficEvents
"""

from enum import Enum


class TrafficEventType(Enum):

    """
    This enum represents different traffic events that occur during driving.
    """

    NORMAL_DRIVING = 0
    COLLISION_STATIC = 1
    COLLISION_VEHICLE = 2
    COLLISION_PEDESTRIAN = 3
    ROUTE_DEVIATION = 4
    ROUTE_COMPLETION = 5
    ROUTE_COMPLETED = 6
    TRAFFIC_LIGHT_INFRACTION = 7
    WRONG_WAY_INFRACTION = 8
    ON_SIDEWALK_INFRACTION = 9
    STOP_INFRACTION = 10
    OUTSIDE_LANE_INFRACTION = 11
    OUTSIDE_ROUTE_LANES_INFRACTION = 12
    VEHICLE_BLOCKED = 13


class TrafficEvent(object):

    """
    TrafficEvent definition
    """

    def __init__(self, event_type, message=None, dictionary=None):
        """
        Initialize object

        :param event_type: TrafficEventType defining the type of traffic event
        :param message: optional message to inform users of the event
        :param dictionary: optional dictionary with arbitrary keys and values
        """
        self._type = event_type
        self._message = message
        self._dict = dictionary

    def get_type(self):
        """
        @return type
        """
        return self._type

    def get_message(self):
        """
        @return message
        """
        if self._message:
            return self._message

        return ""

    def set_message(self, message):
        """
        Set message
        """
        self._message = message

    def get_dict(self):
        """
        @return dictionary
        """
        return self._dict

    def set_dict(self, dictionary):
        """
        Set dictionary
        """
        self._dict = dictionary
