#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
Created on Sat Jun 10 19:12:37 2017

@author: german,felipecode
"""
from __future__ import print_function
import time
import math
from carla import sensor
from carla.client import make_carla_client
from carla.sensor import Camera
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line
from carla.planner.planner  import Planner

from carla.carla_server_pb2 import Control



class Agent(object, ):
    def __init__(self,city_name, **kwargs):
        import os
        dir_path = os.path.dirname(__file__)

        self._planner = Planner(dir_path+'/../carla/planner/' + city_name + '.txt',\
          dir_path+'/../carla/planner/' + city_name + '.png')



    def get_distance(self,start_point,end_point):

        _,path_distance=self._planner.get_next_command([start_point.location.x\
        ,start_point.location.y,22],[start_point.orientation.x\
        ,start_point.orientation.y,22],[end_point.location.x\
        ,end_point.location.y,22],(1,0,0))
        # We calculate the timout based on the distance

        return path_distance

    
    # Function to be redefined by the AI.
    def run_step(self, data):
        pass



"""
    def compute_distance(self, curr, prev, target):
        # no history info
        if(prev[0] == -1 and prev[1] == -1):
            distance = math.sqrt((curr[0] - target[0])*(curr[0] - target[0]) + (curr[1] - target[1])*(curr[1] - target[1]))
        else:
            # distance point to segment
            v1 = [target[0]-curr[0], target[1]-curr[1]]
            v2 = [prev[0]-curr[0], prev[1]-curr[1]]
            
            w1 = v1[0]*v2[0] + v1[1]*v2[1]
            w2 = v2[0]*v2[0] + v2[1]*v2[1]
            t_hat = w1 / (w2 + 1e-4)
            t_start = min(max(t_hat, 0.0), 1.0)
            
            s = [0, 0]
            s[0] = curr[0] + t_start * (prev[0] - curr[0])
            s[1] = curr[1] + t_start * (prev[1] - curr[1])
            distance = math.sqrt((s[0] - target[0])*(s[0] - target[0]) + (s[1] - target[1])*(s[1] - target[1]))

        
        return distance
"""
