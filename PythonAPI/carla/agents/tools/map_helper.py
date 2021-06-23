# Copyright (c) # Copyright (c) 2018-2021 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This file has several useful functions related to the AD Map library
"""

from __future__ import print_function

import carla
import numpy as np
import ad_map_access as ad

def carla_loc_to_enu(carla_location):
    """Transform a CARLA location into an ENU point"""
    return ad.map.point.createENUPoint(carla_location.x, -carla_location.y, carla_location.z)

def enu_to_carla_loc(enu_point):
    """Transform an ENU point into a CARLA location"""
    return carla.Location(float(enu_point.x), float(-enu_point.y), float(enu_point.z))

def para_point_to_carla_waypoint(para_point, town_map, lane_type=carla.LaneType.Driving):
    """Transform a para point into a CARLA waypoint"""
    enu_point = ad.map.lane.getENULanePoint(para_point)
    carla_point = enu_to_carla_loc(enu_point)
    carla_waypoint = town_map.get_waypoint(carla_point, lane_type=lane_type)
    return carla_waypoint

def is_point_at_driving_lane(para_point, town_map):
    """Checks if a parapoint is part of a CARLA driving lane"""
    carla_waypoint = para_point_to_carla_waypoint(para_point, town_map, carla.LaneType.Any)
    return carla_waypoint.lane_type == carla.LaneType.Driving

def get_shortest_route(start_location, end_location, town_map, sample_resolution=1, distance=1, probability=0, world=None):
    """
    Gets the shortest route between a starting and end location.
    """
    route = []

    ad_distance = ad.physics.Distance(distance)
    ad_probability = ad.physics.Probability(probability)
    ad_map_matching = ad.map.match.AdMapMatching()
    route_segment = None

    start_matches = ad_map_matching.getMapMatchedPositions(
        carla_loc_to_enu(start_location), ad_distance, ad_probability)
    end_matches = ad_map_matching.getMapMatchedPositions(
        carla_loc_to_enu(end_location), ad_distance, ad_probability)

    if not start_matches:
        print("WARNING: Couldn't find a paraPoint for location '{}'.".format(start_location))
        return route
    if not end_matches:
        print("WARNING: Couldn't find a paraPoint for location '{}'.".format(end_location))
        return route

    # Filter the paraPoints that aren't part of driving lanes
    start_matches = [match for match in start_matches if is_point_at_driving_lane(match.lanePoint.paraPoint, town_map)]
    end_matches = [match for match in end_matches if is_point_at_driving_lane(match.lanePoint.paraPoint, town_map)]

    # Get the shortest route
    min_length = float('inf')
    for start_match in start_matches:
        start_point = start_match.lanePoint.paraPoint
        for end_match in end_matches:
            # Get the route
            new_route_segment = ad.map.route.planRoute(start_point, end_match.lanePoint.paraPoint)
            if len(new_route_segment.roadSegments) == 0:
                continue  # The route doesn't exist, ignore it

            # Calculate route length, as a sum of the mean of the road's lanes length
            length = 0
            for road_segment in new_route_segment.roadSegments:
                road_length = 0
                number_lanes = 0
                for lane_segment in road_segment.drivableLaneSegments:
                    seg_start = float(lane_segment.laneInterval.start)
                    seg_end = float(lane_segment.laneInterval.end)
                    seg_length = float(ad.map.lane.calcLength(lane_segment.laneInterval.laneId))

                    road_length += seg_length * abs(seg_end - seg_start)
                    number_lanes += 1

                if number_lanes != 0:
                    length += road_length / number_lanes

            # Save the shortest route
            if length < min_length:
                min_length = length
                route_segment = new_route_segment
                start_lane_id = start_point.laneId

    if not route_segment:
        print("WARNING: Couldn't find a viable route between locations "
              "'{}' and '{}'.".format(start_location, end_location))
        return route

    # print(route_segment)

    # Transform the AD map route representation into waypoints (If not needed to parse the altitude)
    for segment in get_route_lane_list(route_segment, start_lane_id):
        lane_id = segment.laneInterval.laneId
        param_list = get_lane_interval_list(segment.laneInterval, sample_resolution)
        for i in range(len(param_list)):
            para_point = ad.map.point.createParaPoint(lane_id, ad.physics.ParametricValue(param_list[i]))
            carla_waypoint = para_point_to_carla_waypoint(para_point, town_map)
            route.append(carla_waypoint)
            if i == 0:
                world.debug.draw_point(carla_waypoint.transform.location, size=0.2, life_time=10000, color=carla.Color(255,0,0))
            world.debug.draw_point(carla_waypoint.transform.location, size=0.1, life_time=10000, color=carla.Color(255,255,0))

    return route

def get_lane_altitude_list(start_z, end_z, length):
    """
    Gets the z values of a lane. This is a simple linear interpolation
    and it won't be necessary whenever the AD map parses the altitude
    """
    if length == 0:
        return []
    if start_z == end_z:
        return start_z*np.ones(length)
    return np.arange(start_z, end_z, (end_z - start_z) / length)

def get_route_lane_list(route, start_lane_id, prev_lane_id=None, prev_successors=[]):
    """
    Given a route returns the lane segments corresponding to the route.
    'start_lane_id' is the lane id of the first point in the route.
    In case this function is called more than once, use 'prev_lane_id'
    and 'prev_successors' to keep computing the lanes without any information loss.
    """
    segments = []

    for road_segment in route.roadSegments:
        current_segment = None
        for lane_segment in road_segment.drivableLaneSegments:
            successors = lane_segment.successors
            predecessors = lane_segment.predecessors
            lane_id = lane_segment.laneInterval.laneId

            if not prev_lane_id:
                # First lane, match the lane with the starting lane id
                if lane_segment.laneInterval.laneId != start_lane_id:
                    continue  # match the lane to the starting point

            if prev_lane_id and prev_lane_id not in predecessors:
                continue

            if prev_successors and lane_id not in prev_successors:
                continue

            current_segment = lane_segment
            break

        # Basically, the next lane won't be chosen if the current one has no successors, as it means
        # that the next lane diverges from the route. Watch out with false empty successors though

        if not current_segment:
            # print(len(prev_successors))
            # print("The previous lane diverged from the route, lane changing")
            # TODO: This shouldn't really be random
            current_segment = np.random.choice(road_segment.drivableLaneSegments)
        # else:
        #     print(len(prev_successors))
        #     print("Found a connected route lane")

        segments.append(current_segment)
        prev_lane_id = current_segment.laneInterval.laneId
        prev_successors = current_segment.successors

    return segments

def to_ad_paraPoint(location, distance=1, probability=0):
    """
    Transforms a carla.Location into an ad.map.point.ParaPoint()
    """
    ad_distance = ad.physics.Distance(distance)
    ad_probability = ad.physics.Probability(probability)
    ad_map_matching = ad.map.match.AdMapMatching()
    ad_location = carla_loc_to_enu(location)

    match_results = ad_map_matching.getMapMatchedPositions(ad_location, ad_distance, ad_probability)

    if not match_results:
        print("WARNING: Couldn't find a para point for CARLA location {}".format(location))
        return None

    # Filter the closest one to the given location
    distance = [float(mmap.matchedPointDistance) for mmap in match_results]
    return match_results[distance.index(min(distance))].lanePoint.paraPoint

def get_lane_interval_list(lane_interval, distance=1):
    """
    Separates a given lane interval into smaller intervals of length equal to 'distance'
    """
    start = float(lane_interval.start)
    end = float(lane_interval.end)
    length = float(ad.map.lane.calcLength(lane_interval.laneId))
    if start == end:
        return []
    return np.arange(start, end, np.sign(end - start) * distance / length)