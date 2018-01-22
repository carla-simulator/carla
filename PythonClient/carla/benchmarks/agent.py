#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# @author: german,felipecode


from __future__ import print_function
import abc

from carla.planner.planner import Planner


class Agent(object):
    def __init__(self, city_name):
        self.__metaclass__ = abc.ABCMeta
        self._planner = Planner(city_name)

    def get_distance(self, start_point, end_point):
        path_distance = self._planner.get_shortest_path_distance(
            [start_point.location.x, start_point.location.y, 22]
            , [start_point.orientation.x, start_point.orientation.y, 22]
            , [end_point.location.x, end_point.location.y, 22]
            , [end_point.orientation.x, end_point.orientation.y, 22])
        # We calculate the timout based on the distance

        return path_distance

    @abc.abstractmethod
    def run_step(self, measurements, sensor_data, target):
        """
        Function to be redefined by an agent.
        :param The measurements like speed, the image data and a target
        :returns A carla Control object, with the steering/gas/brake for the agent
        """
