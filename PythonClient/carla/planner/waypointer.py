import os
import random
import numpy as np
from numpy import linalg as LA

from carla.planner.converter import Converter
from carla.planner.city_track import CityTrack
from carla.planner import bezier, sldist, angle_between


class Waypointer(object):
    """
        Class used to make waypoints, local planning, given a route.
        TODO  0.10: remodel it, now it is very specific for the Town01 and Town02
        WARNING: there are specific situations were the waypointer does not work.

    """

    def __init__(self, city_name):
        """
            Init function
        Args:
            city_name: the name of the CARLA city used, "Town01" or "Town02"
        """
        # Open the necessary files
        dir_path = os.path.dirname(__file__)
        self.city_file = os.path.join(dir_path, city_name + '.txt')
        self.city_map_file = os.path.join(dir_path, city_name + '.png')
        self.city_name = city_name

        # Define the specif parameter for the waypointer. Where is the middle of the road,
        # how open are the curves being made, etc.
        extra_spacing = (random.randint(0, 4) - 2)
        # The amount of shifting from the center the car should go
        self.lane_shift_distance = 13 + extra_spacing
        self.extra_spacing_rights = -2 - extra_spacing
        self.extra_spacing_lefts = 10 + extra_spacing
        self.way_key_points_predicted = 7
        self.number_of_waypoints = 30

        self._converter = Converter(self.city_file, 0.1643, 50.0)
        self._city_track = CityTrack(self.city_name)
        self._map = self._city_track.get_map()

        # The internal state variable
        self.previous_map = [0, 0]
        self.last_trajectory = []
        self._route = []
        self.previous_map = [0, 0]
        self._previous_source = None
        self.last_map_points = None
        self.points = None

    def _search_around_square(self, map_point, map_central_2d):
        """
        Function to search the map point in the central line.
        Args:
            map_point: the used map point
            map_central_2d: the 2d map containing the central lines in red

        Returns:
            projected point in the central line

        """

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

        return np.array([x + closest_point[0], y + closest_point[1]])

    def _shift_points(self, distance_to_center, lane_points, inflection_position):
        """
        Function to take the route points in the middle of the road and shift then to the
        center of the lane
        Args:
            distance_to_center: The distance you want to shift
            lane_points: the lane points used
            inflection_position: A corner case, when there is a turn.

        Returns:

        """

        shifted_lane_vec = []

        for i in range(len(lane_points[:-1])):
            # if cross > 0:
            # right side
            lane_point = lane_points[i]
            unit_vec = self._get_unit(lane_points[i + 1], lane_points[i])

            shifted_lane = [lane_point[0] + unit_vec[0] * distance_to_center[i],
                            lane_point[1] + unit_vec[1] * distance_to_center[i]]
            # One interesting thing is to replicate the point where the turn happens
            if i == inflection_position:
                unit_vec = self._get_unit(lane_points[i], lane_points[i - 1])

                shifted_lane_vec.append([lane_point[0] + unit_vec[0] * distance_to_center[i],
                                         lane_point[1] + unit_vec[1] * distance_to_center[i]])

            shifted_lane_vec.append(shifted_lane)

        last_lane_point = lane_points[-1]
        shifted_lane = [last_lane_point[0] + unit_vec[0] * distance_to_center[-1],
                        last_lane_point[1] + unit_vec[1] * distance_to_center[-1]]

        shifted_lane_vec.append(shifted_lane)
        return shifted_lane_vec

    # Given a list, find the 3 curve points that this list correspond

    def _find_curve_points(self, points):
        """
        Function to find points when there is a curve.
        Args:
            points: the search space

        Returns:
            the points when there is a curve.

        """
        curve_points = None
        first_time = True
        prev_unit_vec = None
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
        """
        Get a unity vector from two 2D points, graph nodes.

        """

        vector_dir = ((last_pos - first_pos) / LA.norm(last_pos - first_pos))
        vector_s_dir = [0, 0]
        vector_s_dir[0] = -vector_dir[1]
        vector_s_dir[1] = vector_dir[0]

        return vector_s_dir

    def generate_final_trajectory(self, coarse_trajectory):
        """
        Smooth the waypoints trajectory using a bezier curve.
        Args:
            coarse_trajectory: a list with the trajectory of 2D points.

        Returns:
            world_points: A list of 3D points in the world after smoothing
            points_list: A list of 3D points in the map after smoothing
        """

        total_course_trajectory_distance = 0
        previous_point = coarse_trajectory[0]
        for i in range(1, len(coarse_trajectory)):
            total_course_trajectory_distance += sldist(coarse_trajectory[i], previous_point)

        points = bezier.bezier_curve(coarse_trajectory,
                                     max(1, int(total_course_trajectory_distance / 10.0)))
        world_points = []
        points = np.transpose(points)
        points_list = []
        for point in points:
            world_points.append(self._converter.convert_to_world(point))
            points_list.append(point.tolist())

        return world_points, points_list

    def get_free_node_direction_target(self, pos, pos_ori, source):
        """
        Get free positions to drive in the direction of the target point.
        """

        free_nodes = self._map.get_adjacent_free_nodes(pos)

        added_walls = set()
        heading_start = np.array([pos_ori[0], pos_ori[1]])

        for adj in free_nodes:

            start_to_goal = np.array([adj[0] - pos[0], adj[1] - pos[1]])
            angle = angle_between(heading_start, start_to_goal)

            if angle < 2 and adj != source:
                added_walls.add((adj[0], adj[1]))

        return added_walls

    def graph_to_waypoints(self, next_route):
        """
        Convert the graph to raw waypoints, with the same size as as the route.
        Basically just project the route to the map and shift to the center of the lane.
        Args:
            next_route: the graph points (nodes) that are going to be converted.

        Returns:
            the list of waypoints

        """

        # Take the map with the central lines
        lane_points = []
        for point in next_route:
            map_point = self._converter.convert_to_pixel([int(point[0]), int(point[1])])
            lane_points.append(self._search_around_square(map_point, self._map.map_image_center))

        _, points_indexes, curve_direction = self._find_curve_points(lane_points)
        # If it is a intersection we divide this in two parts

        lan_shift_distance_vec = [self.lane_shift_distance] * len(lane_points)

        if points_indexes is not None:
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

    def add_extra_points(self, route, node_source):
        """
        Hacky: Add extra points after the target. With this the route never
        finishes and waypoints can be always computed.
        """
        # print self.grid
        new_route = route

        while len(new_route) < 10:  # ADD EXTRA POINTS AFTER

            if len(route) > 1:
                direction_route = self._get_unit(np.array(route[-1]), np.array(route[-2]))
            else:
                direction_route = self._get_unit(np.array(route[-1], node_source))

            direction = (round(new_route[-1][0] + direction_route[1]),
                         round(new_route[-1][1] - direction_route[0]))

            new_route.append(direction)

        return new_route

    def get_next_waypoints(self, source, source_ori, target, target_ori):
        """
        Get the next waypoints, from a list of generated waypoints.
        Args:
            source: source position
            source_ori: source orientation
            target: the desired end position
            target_ori: the desired target orientation

        Returns:

        """
        track_source = self._city_track.project_node(source)
        track_target = self._city_track.project_node(target)

        # reach the goal
        if track_source == track_target:
            return self.last_trajectory, self.last_map_points

        # This is to avoid computing a new route when inside the route

        distance_node = self._city_track.closest_intersection_position(track_source)

        if distance_node > 2 and self._previous_source != track_source:

            # print node_source
            # print node_target
            route = self._city_track.compute_route(track_source, source_ori, track_target,
                                                   target_ori)

            # IF needed we add points after the objective, that is very hacky.
            route = self.add_extra_points(route, track_source)
            self.points = self.graph_to_waypoints(route[1:(1 + self.way_key_points_predicted)])

            self.last_trajectory, self.last_map_points = self.generate_final_trajectory(
                [np.array(self._converter.convert_to_pixel(source))] + self.points)

            return self.last_trajectory, self.last_map_points

        else:
            if sldist(self.previous_map, self._converter.convert_to_pixel(source)) > 3.0:

                # That is because no route was ever computed. This is a problem we should solve.

                route = self._city_track.compute_route(track_source, source_ori,
                                                       track_target, target_ori)
                # print self._route

                route = self.add_extra_points(route, track_source)

                self.points = self.graph_to_waypoints(route[1:(1 + self.way_key_points_predicted)])

                self.last_trajectory, self.last_map_points = self.generate_final_trajectory(
                    [np.array(self._converter.convert_to_pixel(source))] + self.points)

                # We have to find the current node position
                self.previous_map = self._converter.convert_to_pixel(source)
                # Make a source not replaced

                for point in self.last_map_points:
                    point_vec = self._get_unit(np.array(self._converter.convert_to_pixel(source)),
                                               point)
                    cross_product = np.cross(source_ori[0:2], point_vec)

                    if (cross_product > 0.0 and sldist(point, self._converter.convert_to_pixel(
                            source)) < 50) or sldist(
                        point,
                        self._converter.convert_to_pixel(
                            source)) < 15.0:
                        self.last_trajectory.remove(
                            self._converter.convert_to_world(
                                point))  # = [self.make_world_map(point)] + self.last_trajc
                        self.last_map_points.remove(point)

            return self.last_trajectory, self.last_map_points
