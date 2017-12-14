import math
import time
import collections
import os
import numpy as np


from PIL import Image

from city_track import CityTrack

compare = lambda x, y: collections.Counter(x) == collections.Counter(y)

""" 
Constants Used for the high level commands
"""

REACH_GOAL = 0.0
GO_STRAIGHT = 5.0
TURN_RIGHT = 4.0
TURN_LEFT = 3.0
LANE_FOLLOW =2.0


# Auxiliary algebra function
def angle_between(v1, v2):
    return np.arccos(np.dot(v1, v2) / np.linalg.norm(v1) / np.linalg.norm(v2))

sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)

def signal(v1, v2):
    return np.cross(v1, v2) / np.linalg.norm(v1) / np.linalg.norm(v2)


class Planner(object):


    def __init__(self, city_name):


        self._city_track = CityTrack(city_name) 

        #self.previous_source = (0, 0)
        ##self.distance = 0
        #self.complete_distance = 0

        # print self.map_image
        self._commands = []

        #self.route = []

        # The number of game units per pixel
        #self.pixel_density = 16.43
        # A pixel positions with respect to graph node position is:  Pixel =
        # Node*50 +2
        #self.node_density = 50.0
        # This function converts the 2d map into a 3D one in a vector.









    def get_next_command(self, source, source_ori,  target, target_ori):



        # Take the world position and project it on the road.
        # The road is represented in a grid

        track_source = self._city_track.project_node(source,source_ori)
        track_target = self._city_track.project_node(target,target_ori)



        # reach the goal

        if self._city_track.is_at_goal(track_source,track_target):
            return REACH_GOAL


        if (self._city_track.is_at_new_node(track_source)
           and self._city_track.is_away_from_intersection(track_source)):



            route = self._city_track.compute_route(track_source,source_ori,
                            track_target,target_ori) 
            if route == None:
                raise RuntimeError('Impossible to find route')


            self._commands = self._route_to_commands(route)


            if self._city_track.is_far_away_from_route_intersection(track_source):
                return LANE_FOLLOW
            else:
                if self._commands:
                    return self._commands[0]
                else:
                    return LANE_FOLLOW
        else:

            if self._city_track.is_far_away_from_route_intersection(track_source):
                return LANE_FOLLOW

            # If there is computed commands
            if self._commands:
                return self._commands[0]
            else:
                return LANE_FOLLOW


    def get_shortest_path_distance(self, source, source_ori,  target, target_ori):

        distance = 0
        track_source = self._city_track.project_node(source,source_ori)
        track_target = self._city_track.project_node(target,target_ori)

        current_pos = track_source

        route = self._city_track.compute_route(track_source,source_ori,
                            track_target,target_ori)
        # No Route, distance is zero
        if route == None:
            return 0.0

        for node_iter in route:

            distance += sldist(node_iter, current_pos)
            current_pos = node_iter

        # We multiply by these values to convert distance to world coordinates
        return distance  *50.0*16.42 # , maybe this goes to another layer


    def is_there_posible_route(self,source,source_ori,target,target_ori):

        track_source = self._city_track.project_node(source,source_ori)
        track_target = self._city_track.project_node(target,target_ori)


        return  not self._city_track.compute_route(
                   node_source,source_ori,node_target,target_ori)  ==  None



    # from the shortest path graph, transform it into a list of commands
    # @param the sub graph containing the shortest path
    # @param Orientation of the car
    # returns list of commands ( 3 is left, 4 is right, 5 is straight)


    def _route_to_commands(self, route):

        commands_list = []

        for i in range(0, len(route)):
            if route[i] not in self._city_track._map._graph.intersection_nodes():
                continue

            current = route[i]
            past = route[i-1]
            future = route[i+1]

            past_to_current = np.array(
                [current[0]-past[0], current[1]-past[1]])
            current_to_future = np.array(
                [future[0]-current[0], future[1]-current[1]])
            angle = signal(current_to_future, past_to_current)

            command = 0.0
            if angle < -0.1:
                command = TURN_RIGHT
            elif angle > 0.1:
                command = TURN_LEFT
            else:
                command = GO_STRAIGHT

            commands_list.append(command)

        return commands_list

