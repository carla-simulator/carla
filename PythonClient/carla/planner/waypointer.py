import random
import numpy as np
from numpy import linalg as LA


from carla.planner.converter import Converter
from carla.planner.city_track import CityTrack
from carla.planner.graph import *
from carla.planner import bezier
from PIL import Image
import math
import os
import time

""" This class that specialize the planner to produce waypoints



"""


def angle_between(v1, v2):
    return np.arccos(np.dot(v1, v2) / np.linalg.norm(v1) / np.linalg.norm(v2))


def distance(t1, t2):
    return math.sqrt((t1[0] - t2[0]) * (t1[0] - t2[0]) + (t1[1] - t2[1]) * (t1[1] - t2[1]))


class Waypointer(object):

    def __init__(self, city_name, debug=False):


        dir_path = os.path.dirname(__file__)
        self.city_file = os.path.join(dir_path , city_name + '.txt')
        self.city_map_file = os.path.join(dir_path, city_name + '.png')
        self.city_name = city_name


        extra_spacing = (random.randint(0, 4) - 2)
        self.lane_shift_distance = 13 + extra_spacing  # The amount of shifting from the center the car should go
        self.extra_spacing_rights = -2 - extra_spacing
        self.extra_spacing_lefts = 10 + extra_spacing
        self.way_key_points_predicted = 7
        self.number_of_waypoints = 30


        self.debug = debug

        self._converter = Converter(self.city_file, 0.1643, 50.0)
        self._city_track = CityTrack(self.city_name)
        self._map = self._city_track._map

        # dir_path = os.path.dirname(__file__)

        # city_map_file = os.path.join(dir_path, '../planner/' + config.city_name + '.png')
        # city_map_file_lanes = os.path.join(dir_path,'../planner/' + config.city_name + 'Lanes.png')
        # city_map_file_center = os.path.join(dir_path, '../planner/' + config.city_name + 'Central.png')

        # TODO: This map images are probably not needed, since they are inside the map function.
        """
        self.map_image_lanes = Image.open(city_map_file_lanes)
        self.map_image_lanes.load()
        self.map_image_lanes = np.asarray(self.map_image_lanes, dtype="int32")
        # Load the image
        self.map_image = Image.open(city_map_file)
        self.map_image.load()
        self.map_image = np.asarray(self.map_image, dtype="int32")

        # Load the lanes image
        self.map_image_center = Image.open(city_map_file_center)
        self.map_image_center.load()
        self.map_image_center = np.asarray(self.map_image_center, dtype="int32")
        """

        # self.city_file = config.city_file
        # self.map_file = config.city_map_file
        # Define here some specific configuration to produce waypoints
        self.last_trajectory = []
        self.lane_shift_distance = self.lane_shift_distance  # The amount of shifting from the center the car should go
        self.extra_spacing_rights = self.extra_spacing_rights
        self.extra_spacing_lefts = self.extra_spacing_lefts
        self.way_key_points_predicted = self.way_key_points_predicted
        self.number_of_waypoints = self.number_of_waypoints
        self.previous_map = [0, 0]

        if debug:
            self.search_image = self._map.map_image.astype(np.uint8)

        self.last_trajectory = []
        self._route = []
        self.previous_map = [0, 0]
        self._previous_source = None
        # self.grid = self.make_grid()
        # self.walls = self.make_walls()

    def get_debug_image(self, height=None):
        if height is not None:
            img = Image.fromarray(self.search_image.astype(np.uint8))

            aspect_ratio = height / float(self.search_image.shape[0])

            img = img.resize((int(aspect_ratio * self.search_image.shape[1]), height),
                             Image.ANTIALIAS)
            img.load()
            return np.asarray(img, dtype="int32")
        return np.fliplr(self.search_image)

    def reset(self):
        self.last_trajectory = []
        self._route = []
        self.previous_map = [0, 0]
        self._previous_source = None
        # self.grid = self.make_grid()
        # self.walls = self.make_walls()

    def _write_point_on_map(self, position, color, size=4):

        for i in range(0, size):
            self.search_image[int(position[1]), int(position[0])] = color
            self.search_image[int(position[1]) + i, int(position[0])] = color
            self.search_image[int(position[1]), int(position[0]) + i] = color
            self.search_image[int(position[1]) - i, int(position[0])] = color
            self.search_image[int(position[1]), int(position[0]) - i] = color
            self.search_image[int(position[1]) + i, int(position[0]) + i] = color
            self.search_image[int(position[1]) - i, int(position[0]) - i] = color
            self.search_image[int(position[1]) + i, int(position[0]) - i] = color
            self.search_image[int(position[1]) - i, int(position[0]) + i] = color

    def _print_trajectory(self, points, color, size):

        # points = np.transpose(points)
        for point in points:
            self._write_point_on_map(point, color, size)

    def _search_around_square(self, map_point, map_central_2d):

        x = int(map_point[0])
        y = int(map_point[1])

        square_crop = map_central_2d[(y - 30):(y + 30), (x - 30):(x + 30)]
        small_distance = 10000
        closest_point = [15 - square_crop.shape[1] / 2, 15 - square_crop.shape[0] / 2]

        # print square_crop
        for t in np.transpose(np.nonzero(square_crop)):

            distance = sldist(t, [square_crop.shape[1] / 2, square_crop.shape[0] / 2])

            if distance < small_distance:
                small_distance = distance

                closest_point = [t[0] - square_crop.shape[1] / 2, t[1] - square_crop.shape[0] / 2]

        # self.search_image[x,y ] = 128
        # search_image[x+closest_point[0],y +closest_point[1]] = 128

        return np.array([x + closest_point[0], y + closest_point[1]])

    def _shift_points(self, distance_to_center, lane_points, inflection_position):

        shifted_lane_vec = []

        for i in range(len(lane_points[:-1])):
            # if cross > 0:
            # right side
            lane_point = lane_points[i]
            unit_vec = self._get_unit(lane_points[i + 1], lane_points[i])

            shifted_lane = [lane_point[0] + unit_vec[0] * distance_to_center[i],
                            lane_point[1] + unit_vec[1] * distance_to_center[i]]
            # search_image[int(shifted_lane[0]),int(shifted_lane[1]) ] = [128,128,0,255]
            if i == inflection_position:  # One interesting thing is to replicate the point whre the turn happens

                # prev_unit_vec =unit_vec
                unit_vec = self._get_unit(lane_points[i], lane_points[i - 1])
                # print ' UNIT INFLECTION ',unit_vec
                # direction_curve =np.cross(prev_unit_vec,unit_vec)
                shifted_lane_vec.append([lane_point[0] + unit_vec[0] * distance_to_center[i],
                                         lane_point[1] + unit_vec[1] * distance_to_center[i]])
            # print 'ALL DISTANCES'
            # print distance_to_center
            # print 'VECTOR POINTING TO INFLECTION ', unit_vec
            # print 'VECTOR INFLECTION POINTS TO ', self._get_unit(lane_points[i+1],lane_points[i])

            # else:
            # left side
            #	shifted_lane = [lane_point[0]-vector_s_dir[0]*distance_to_center,lane_point[1]-vector_s_dir[1]*distance_to_center]
            # search_image[int(shifted_lane[0]),int(shifted_lane[1]) ] = [128,128,0,255]
            shifted_lane_vec.append(shifted_lane)

        last_lane_point = lane_points[-1]
        shifted_lane = [last_lane_point[0] + unit_vec[0] * distance_to_center[i],
                        last_lane_point[1] + unit_vec[1] * distance_to_center[i]]

        shifted_lane_vec.append(shifted_lane)
        return shifted_lane_vec

    # Given a list, find the 3 curve points that this list correspond

    def _find_curve_points(self, points):

        curve_points = None
        first_time = True
        inflection_point = None
        for i in range(len(points) - 1):

            unit_vec = self._get_unit(points[i + 1], points[i])
            unit_vec = [round(unit_vec[0]), round(unit_vec[1])]

            if not first_time:

                if unit_vec != prev_unit_vec:
                    curve_points = [points[i + 1], points[i], points[i - 1]]
                    return curve_points, [i + 1, i, i - 1], np.cross(unit_vec, prev_unit_vec)

            first_time = False
            prev_unit_vec = unit_vec

        return curve_points, None, None  # ,inflection_point

    def _get_unit(self, last_pos, first_pos):

        vector_dir = ((last_pos - first_pos) / LA.norm(last_pos - first_pos))
        vector_s_dir = [0, 0]
        vector_s_dir[0] = -vector_dir[1]
        vector_s_dir[1] = vector_dir[0]

        return vector_s_dir

    def generate_final_trajectory(self, coarse_trajectory):

        total_course_trajectory_distance = 0
        previous_point = coarse_trajectory[0]
        for i in range(1, len(coarse_trajectory)):
            total_course_trajectory_distance += sldist(coarse_trajectory[i], previous_point)

        # print ' COARSE TRAYEJECTORY ', coarse_trajectory
        # print ' DISTANCE  ',total_course_trajectory_distance

        points = bezier.bezier_curve(coarse_trajectory,
                                     max(1, int(total_course_trajectory_distance / 10.0)))
        world_points = []
        points = np.transpose(points)
        points_list = []
        for point in points:
            # print point
            # print self._converter.convert_to_pixel(point)
            world_points.append(self._converter.convert_to_world(point))
            points_list.append(point.tolist())

        return world_points, points_list

    def get_free_node_direction_target(self, pos, pos_ori, source):

        free_nodes = self._city_track._map._grid._get_adjacent_free_nodes(pos)

        added_walls = set()
        heading_start = np.array([pos_ori[0], pos_ori[1]])

        for adj in free_nodes:

            start_to_goal = np.array([adj[0] - pos[0], adj[1] - pos[1]])
            angle = angle_between(heading_start, start_to_goal)
            # print ' Angle between ',angle
            if (angle < 2 and adj != source):
                added_walls.add((adj[0], adj[1]))

        return added_walls

    def graph_to_waypoints(self, next_route):

        # Function to get the unit vector and the cross product.

        # Take the map with the central lines

        # print ' ROute to be transformed ',next_route
        lane_points = []
        for point in next_route:
            map_point = self._converter.convert_to_pixel([int(point[0]), int(point[1])])
            lane_points.append(self._search_around_square(map_point, self._map.map_image_center))

        if self.debug:
            self._print_trajectory(lane_points, [0, 0, 0, 255], 7)

        # print ' THE CURVE POINTS '
        _, points_indexes, curve_direction = self._find_curve_points(lane_points)
        # print ' Found CURVE on ',points_indexes
        # print 'curve direction ',curve_direction
        # If it is a intersection we divide this in two parts

        # if (1-vector_s_dir[0]) <0.1 or (1-vector_s_dir[1]) <0.1:
        # it is not a intersection
        lan_shift_distance_vec = [self.lane_shift_distance] * len(lane_points)

        if points_indexes != None:
            for i in points_indexes:
                if curve_direction > 0:
                    lan_shift_distance_vec[i] += (self.extra_spacing_lefts * 1)
                else:
                    lan_shift_distance_vec[i] += (self.extra_spacing_rights * -1)

            shifted_lane_vec = self._shift_points(lan_shift_distance_vec, lane_points,
                                                  points_indexes[1])
        else:
            shifted_lane_vec = self._shift_points(lan_shift_distance_vec, lane_points, None)

        return shifted_lane_vec

    def add_extra_points(self, node_target, target_ori, node_source):

        direction = node_target
        direction_ori = target_ori
        # print self.grid

        while len(self._route) < 10:  # aDD EXTRA POINTS AFTER

            try:
                free_nodes = list(
                    self.get_free_node_direction_target(direction, direction_ori, node_source))
                # print 'free -> ', free_nodes
                # print "Normal"
                direction_ori = self._get_unit(np.array(direction), np.array(free_nodes[0]))
                aux = -direction_ori[1]
                direction_ori[1] = direction_ori[0]
                direction_ori[0] = aux
                # print 'orit ',direction_ori
                direction = free_nodes[0]
            except:

                #    # Repeate some route point, there is no problem.
                direction = [round(self._route[-1][0] + direction_ori[0]),
                             round(self._route[-1][1] + direction_ori[1])]
                # direction = [round(self._route[-1][0]), round(self._route[-1][1])]

            # print 'ADDED ',direction
            self._route.append(direction)

    def get_next_waypoints(self, source, source_ori, target, target_ori):

        track_source = self._city_track.project_node(source)
        track_target = self._city_track.project_node(target)

        if math.fabs(target_ori[0]) > math.fabs(target_ori[1]):
            target_ori = (target_ori[0], 0.0, 0.0)
        else:
            target_ori = (0.0, target_ori[1], 0.0)

        if math.fabs(source_ori[0]) > math.fabs(source_ori[1]):
            source_ori = (source_ori[0], 0.0, 0.0)
        else:
            source_ori = (0.0, source_ori[1], 0.0)

        # print ''

        # print self.grid

        # reach the goal
        if track_source == track_target:
            return self.last_trajectory, self.last_map_points

        # This is to avoid computing a new route when inside the route

        distance_node = self._city_track._closest_intersection_position(track_source)

        if self.debug:
            self._write_point_on_map(self._converter.convert_to_pixel(source), [255, 0, 255, 255],
                                     size=7)

            self._write_point_on_map(self._converter.convert_to_pixel(target), [255, 255, 255, 255],
                                     size=6)

        if distance_node > 2 and self._previous_source != track_source:

            # print node_source
            # print node_target
            self._route = self._city_track.compute_route(track_source, source_ori, track_target,
                                                         target_ori)

            # print self._route

            # IF needed we add points after the objective, that is very hacky.
            self.add_extra_points(track_target, target_ori, track_source)

            # print added_walls
            if self.debug:
                self.search_image = self._map.map_image.astype(np.uint8)

            self.points = self.graph_to_waypoints(
                self._route[1:(1 + self.way_key_points_predicted)])
            self.last_trajectory, self.last_map_points = self.generate_final_trajectory(
                [np.array(self._converter.convert_to_pixel(source))] + self.points)

            return self.last_trajectory, self.last_map_points  # self.generate_final_trajectory([np.array(self.make_map_world(source))] +self.points)


        else:
            if distance(self.previous_map, self._converter.convert_to_pixel(source)) > 3.0:

                # That is because no route was ever computed. This is a problem we should solve.
                if not self._route:
                    self._route = self._city_track.compute_route(track_source, source_ori,
                                                                 track_target, target_ori)
                    # print self._route

                    self.add_extra_points(track_target, target_ori, track_source)
                    # print added_walls
                    if self.debug:
                        self.search_image = self._map.map_image.astype(np.uint8)
                    self.points = self.graph_to_waypoints(
                        self._route[1:(1 + self.way_key_points_predicted)])

                    self.last_trajectory, self.last_map_points = self.generate_final_trajectory(
                        [np.array(self._converter.convert_to_pixel(source))] + self.points)

                # We have to find the current node position
                self.previous_map = self._converter.convert_to_pixel(source)
                # Make a source not replaced
                # self.last_trajc,self.last_map_points = self.generate_final_trajectory([np.array(self.make_map_world(source))] +self.points)

                # self.last_trajc = self.generate_final_trajectory([np.array(self.make_map_world(source))] +self.points[(index_source):])
                # self.test = self.generate_final_trajectory([np.array(self.make_map_world(source))] +self.points[(index_source):])

                for point in self.last_map_points:
                    point_vec = self._get_unit(np.array(self._converter.convert_to_pixel(source)),
                                               point)
                    cross_product = np.cross(source_ori[0:2], point_vec)

                    # aux = point_vec[1]
                    # point_vec[1]= point_vec[0]
                    # point_vec[0]= aux

                    # print point_vec,source_ori[0:2]
                    # print cross_product,sldist(point,self.make_map_world(source))

                    if (cross_product > 0.0 and sldist(point, self._converter.convert_to_pixel(
                            source)) < 50) or sldist(
                            point,
                            self._converter.convert_to_pixel(
                                source)) < 15.0:
                        # print 'removed ',self.last_map_points.index(point),self.last_trajc.index(self.make_world_map(point))

                        self.last_trajectory.remove(
                            self._converter.convert_to_world(
                                point))  # = [self.make_world_map(point)] + self.last_trajc
                        self.last_map_points.remove(point)
                # print self.last_map_points
                # print point
                # print 'INDED', np.argwhere(self.last_map_points==point)
                # self.last_map_points.remove(point)
                if self.debug:
                    self._print_trajectory(self.last_map_points, [255, 0, 0, 255], 4)

                    self._print_trajectory(self.points, [255, 128, 0, 255], 7)

            return self.last_trajectory, self.last_map_points

        # This function uses the map to test if some specific position is too close to intersections

    """
        def _project_source(self, source, source_ori):
            node_source = self.make_node(source)

            source_ori = np.array([source_ori[0], source_ori[1], source_ori[2]])
            source_ori = source_ori.dot(self.worldrotation)

            # Trunkate !
            node_source = tuple([int(x) for x in node_source])

            # Set to zero if it is less than zero.

            node_source = (max(0, node_source[0]), max(0, node_source[1]))
            node_source = (min(self.resolution[0] - 1, node_source[0]), min(self.resolution[1] - 1, node_source[1]))
            # is it x or y ? Check to avoid  special corner cases

            if math.fabs(source_ori[0]) > math.fabs(source_ori[1]):
                source_ori = (source_ori[0], 0.0, 0.0)
            else:
                source_ori = (0.0, source_ori[1], 0.0)

            node_source = self.search(node_source[0], node_source[1])

            return node_source

        def _project_target(self, target):

            node_target = self.make_node(target)

            # Trunkate !
            node_target = tuple([int(x) for x in node_target])
            target_ori = self.get_target_ori(target)
            # Set to zero if it is less than zero.

            target_ori = np.array([target_ori[0], target_ori[1], 0])
            target_ori = target_ori.dot(self.worldrotation)

            node_target = self.search(node_target[0], node_target[1])
            return node_target, target_ori
    """
    """

        def _route_compute(self, node_source, source_ori, node_target, target_ori):
            added_walls = self.set_grid_direction(node_source, source_ori, node_target)
            # print added_walls
            added_walls = added_walls.union(self.set_grid_direction_target(node_target, target_ori, node_source))
            # print added_walls

            self._previous_source = node_source

            # print self.grid

            self.a_star = AStar()
            self.init(node_source, node_target)
            self._route = self.solve()
            # print route # JuSt a Corner Case
            if self._route == None:
                for i in added_walls:
                    self.walls.remove(i)

                    self.grid[i[0], i[1]] = 0.0
                added_walls = self.set_grid_direction(node_source, source_ori, node_target)
                self.a_star = AStar()
                self.init(node_source, node_target)
                self._route = self.solve()

            for i in added_walls:
                self.walls.remove(i)

                self.grid[i[0], i[1]] = 0.0

            return self._route

        def get_distance_closest_node_turn(self, pos):
            import collections
            distance = []
            for node_iter in self.graph.turn_nodes():
                distance.append(sldist(node_iter, pos))

            return sorted(distance)[0]

    """



    def route_test(self, node_source, source_ori, node_target, target_ori):
        route = self._city_track.compute_route(node_source, source_ori, node_target, target_ori)
        return not route == None

    def test_position(self, source):
        node_source = self._city_track.project_node(source)

        distance_node = self._city_track.get_distance_closest_node_turn(node_source)
        if distance_node > 2:
            return True
        else:
            return False

    def test_pair(self, source, source_ori, target, target_ori):

        node_source = self._city_track.project_node(source)

        node_target = self._city_track.project_node(target)

        return self.route_test(node_source, source_ori, node_target, target_ori)

