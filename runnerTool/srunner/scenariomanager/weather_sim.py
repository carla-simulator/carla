#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides a weather class and py_trees behavior
to simulate weather in CARLA according to the astronomic
behavior of the sun.
"""

import datetime
import math
import operator

import ephem
import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.timer import GameTime


class Weather(object):

    """
    Class to simulate weather in CARLA according to the astronomic behavior of the sun

    The sun position (azimuth and altitude angles) is obtained by calculating its
    astronomic position for the CARLA reference position (x=0, y=0, z=0) using the ephem
    library.

    Args:
        carla_weather (carla.WeatherParameters): Initial weather settings.
        dtime (datetime): Initial date and time in UTC (required for animation only).
            Defaults to None.
        animation (bool): Flag to allow animating the sun position over time.
            Defaults to False.

    Attributes:
        carla_weather (carla.WeatherParameters): Weather parameters for CARLA.
        animation (bool): Flag to allow animating the sun position over time.
        _sun (ephem.Sun): The sun as astronomic entity.
        _observer_location (ephem.Observer): Holds the geographical position (lat/lon/altitude)
            for which the sun position is obtained.
        datetime (datetime): Date and time in UTC (required for animation only).
    """

    def __init__(self, carla_weather, dtime=None, animation=False):
        """
        Class constructor
        """
        self.carla_weather = carla_weather
        self.animation = animation

        self._sun = ephem.Sun()  # pylint: disable=no-member
        self._observer_location = ephem.Observer()
        geo_location = CarlaDataProvider.get_map().transform_to_geolocation(carla.Location(0, 0, 0))
        self._observer_location.lon = str(geo_location.longitude)
        self._observer_location.lat = str(geo_location.latitude)

        # @TODO This requires the time to be in UTC to be accurate
        self.datetime = dtime
        if self.datetime:
            self._observer_location.date = self.datetime

        self.update()

    def update(self, delta_time=0):
        """
        If the weather animation is true, the new sun position is calculated w.r.t delta_time

        Nothing happens if animation or datetime are None.

        Args:
            delta_time (float): Time passed since self.datetime [seconds].
        """
        if not self.animation or not self.datetime:
            return

        self.datetime = self.datetime + datetime.timedelta(seconds=delta_time)
        self._observer_location.date = self.datetime

        self._sun.compute(self._observer_location)
        self.carla_weather.sun_altitude_angle = math.degrees(self._sun.alt)
        self.carla_weather.sun_azimuth_angle = math.degrees(self._sun.az)


class WeatherBehavior(py_trees.behaviour.Behaviour):

    """
    Atomic to read weather settings from the blackboard and apply these in CARLA.
    Used in combination with UpdateWeather() to have a continuous weather simulation.

    This behavior is always in a running state and must never terminate.
    The user must not add this behavior. It is automatically added by the ScenarioManager.

    This atomic also sets the datetime to blackboard variable, used by TimeOfDayComparison atomic

    Args:
        name (string): Name of the behavior.
            Defaults to 'WeatherBehavior'.

    Attributes:
        _weather (srunner.scenariomanager.weather_sim.Weather): Weather settings.
        _current_time (float): Current CARLA time [seconds].
    """

    def __init__(self, name="WeatherBehavior"):
        """
        Setup parameters
        """
        super(WeatherBehavior, self).__init__(name)
        self._weather = None
        self._current_time = None

    def initialise(self):
        """
        Set current time to current CARLA time
        """
        self._current_time = GameTime.get_time()

    def update(self):
        """
        Check if new weather settings are available on the blackboard, and if yes fetch these
        into the _weather attribute.

        Apply the weather settings from _weather to CARLA.

        Note:
            To minimize CARLA server interactions, the weather is only updated, when the blackboard
            is updated, or if the weather animation flag is true. In the latter case, the update
            frequency is 1 Hz.

        returns:
            py_trees.common.Status.RUNNING
        """

        weather = None

        try:
            check_weather = operator.attrgetter("CarlaWeather")
            weather = check_weather(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if weather:
            self._weather = weather
            delattr(py_trees.blackboard.Blackboard(), "CarlaWeather")
            CarlaDataProvider.get_world().set_weather(self._weather.carla_weather)
            py_trees.blackboard.Blackboard().set("Datetime", self._weather.datetime, overwrite=True)

        if self._weather and self._weather.animation:
            new_time = GameTime.get_time()
            delta_time = new_time - self._current_time

            if delta_time > 1:
                self._weather.update(delta_time)
                self._current_time = new_time
                CarlaDataProvider.get_world().set_weather(self._weather.carla_weather)

                py_trees.blackboard.Blackboard().set("Datetime", self._weather.datetime, overwrite=True)

        return py_trees.common.Status.RUNNING
