import math
import time
import collections
import os


from PIL import Image

from graph import Graph
from astar import Astar

compare = lambda x, y: collections.Counter(x) == collections.Counter(y)


def angle_between(v1, v2):
    return np.arccos(np.dot(v1, v2) / np.linalg.norm(v1) / np.linalg.norm(v2))

sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)


def color_to_angle(color):
    return ((float(color)/255.0)) * 2*math.pi


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



        #self.astar.
        track_source = self._city_track.project_node(source,source_ori)
        track_target = self._city_track.project_node(target,target_ori)



        # reach the goal

        if self._city_track.is_at_goal(track_source,track_target)
            return 0,0
        #if node_source == node_target:
        #    return 0, 0


        if self._city_track.is_at_new_node(track_source)  
           and self._city_track.is_away_from_intersection(track_source):



            # print route
            route= self._city_track.compute_route(track_source,source_ori,
                            track_target,target_ori) 

            #(node_source,source_ori,node_target,target_ori)


            # We recompute the distance based on route
            #self.distance = self.get_distance_closest_node_route(
            #    node_source, route)

            #self.complete_distance = self.get_full_distance_route(
            #    node_source, route)*50.0*16.42 # This is bad


            self._commands = self._route_to_commands(route)




            if self._city_track.is_far_away_from_route_intersection(track_source):
                return 2.0
            else:
                if self.commands:
                    return self.commands[0]
                else:
                    return 2.0
        else:

            if self._city_track.is_far_away_from_route_intersection(track_source):
                return 2.0

            # If there is computed commands
            if self.commands:
                return self.commands[0]
            else:
                return 2.0


    def get_shortest_path_distance(self, source, source_ori,  target, target_ori):
        import collections
        distance = 0
        # if self.graph.intersection_nodes() == set():
        current_pos = pos
        for node_iter in route:

            distance += sldist(node_iter, current_pos)
            current_pos = node_iter

        return distance# *50.0*16.42 , maybe this goes to another layer


    def is_there_posible_route(self,source,source_ori,target,target_ori):

        track_source = self._city_track.project_node(source,source_ori)
        track_target = self._city_track.project_node(target,target_ori)


        return len(self._city_track.compute_route(
                   node_source,source_ori,node_target,target_ori))>0



    # from the shortest path graph, transform it into a list of commands
    # @param the sub graph containing the shortest path
    # @param Orientation of the car
    # returns list of commands ( 3 is left, 4 is right, 5 is straight)


    def _route_to_commands(self, route):

        commands_list = []

        for i in range(0, len(route)):
            if route[i] not in self.astar.intersection_nodes():
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
                command = 4.0
            elif angle > 0.1:
                command = 3.0
            else:
                command = 5.0

            commands_list.append(command)

        return commands_list

"""
# print node_source
# print node_target
added_walls = self.set_grid_direction(
    node_source, source_ori, node_target)
# print added_walls
added_walls = added_walls.union(
    self.set_grid_direction_target(node_target, target_ori, node_source))
# print added_walls
self.previous_source = node_source

# print self.grid

self.a_star = AStar()
self.init(node_source, node_target)
route = self.solve()
# print route # JuSt a Corner Case
if route == None:
    for i in added_walls:
        self.walls.remove(i)

        self.grid[i[0], i[1]] = 0.0
    added_walls = self.set_grid_direction(
        node_source, source_ori, node_target)
    self.a_star = AStar()
    self.init(node_source, node_target)
    route = self.solve()


    def check_command_completed(self, commands, previous_commands):
        if compare(commands, previous_commands):
            return False, False
        elif (len(commands) + 1) < len(previous_commands):
            return True, False

        elif len(commands) < len(previous_commands):

            return True, compare(commands, previous_commands[1:])
        else:
            return True, False


"""