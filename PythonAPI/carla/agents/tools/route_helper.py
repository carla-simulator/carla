# Copyright (c) # Copyright (c) 2018-2021 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This file has several useful functions related to the AD Map library
"""

from __future__ import print_function

from math import floor
import os
import numpy as np
import xml.etree.ElementTree as ET

import carla
import ad_map_access as ad

def initialize_map(wmap):
    """Initialize the AD map library and, creates the file needed to do so."""
    lat_ref = 0.0
    lon_ref = 0.0

    opendrive_contents = wmap.to_opendrive()
    xodr_name = 'RoutePlannerMap.xodr'
    txt_name = 'RoutePlannerMap.txt'

    # Save the opendrive data into a file
    with open(xodr_name, 'w') as f:
        f.write(opendrive_contents)

    # Get geo reference
    xml_tree = ET.parse(xodr_name)
    for geo_elem in xml_tree.find('header').find('geoReference').text.split(' '):
        if geo_elem.startswith('+lat_0'):
            lat_ref = float(geo_elem.split('=')[-1])
        elif geo_elem.startswith('+lon_0'):
            lon_ref = float(geo_elem.split('=')[-1])

    # Save the previous info
    with open(txt_name, 'w') as f:
        txt_content = "[ADMap]\n" \
                        "map=" + xodr_name + "\n" \
                        "[ENUReference]\n" \
                        "default=" + str(lat_ref) + " " + str(lon_ref) + " 0.0"
        f.write(txt_content)

    # Intialize the map and remove created files
    initialized = ad.map.access.init(txt_name)
    if not initialized:
        raise ValueError("Couldn't initialize the map")

    for fname in [txt_name, xodr_name]:
        if os.path.exists(fname):
            os.remove(fname)

def carla_loc_to_enu(carla_location):
    """Transform a CARLA location into an ENU point"""
    return ad.map.point.createENUPoint(carla_location.x, -carla_location.y, carla_location.z)

def carla_loc_to_ecef(carla_location):
    """Transform a CARLA location into an ENU point"""
    return ad.map.point.toECEF(carla_loc_to_enu(carla_location))

def enu_to_carla_loc(enu_point):
    """Transform an ENU point into a CARLA location"""
    return carla.Location(float(enu_point.x), float(-enu_point.y), float(enu_point.z))

def para_point_to_carla_waypoint_2(para_point, town_map, lane_type=carla.LaneType.Driving):
    """Transform a para point into a CARLA waypoint"""
    ad_lane_id = para_point.laneId
    num_lane_id = float(str(para_point.laneId))

    road_id = floor(num_lane_id / 10000)
    remnant = num_lane_id - road_id * 10000
    lane_segment_id = floor(remnant / 100)
    remnant = remnant - lane_segment_id * 100
    lane_id = floor(remnant - 50)

    length = float(ad.map.lane.calcLength(ad_lane_id))
    is_positive_lane = ad.map.lane.isLaneDirectionPositive(ad_lane_id)
    if is_positive_lane and not lane_id < 0 or not is_positive_lane and lane_id < 0:
        s = length * (1 - float(para_point.parametricOffset))
    else:
        s = length * float(para_point.parametricOffset)

    return  town_map.get_waypoint_xodr(road_id, lane_id, s)

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

def trace_route(start_waypoint, end_waypoint, town_map, sample_resolution=1, match_dist=1, max_match_dist=10):
    """
    Gets the shortest route between a starting and end waypoint. This transforms the given location
    to AD map paraPoints, and iterates through all permutations to return the shortest route to ensure
    that the correct route is chosen when starting / ending at intersections.
    Then, the route is transformed back to a list of [carla.Waypoint, RoadOption]

    Due to some bugs at the altitude parsing, the matching distance isn't just one value,
    but keeps increasing up to a max number, in order to increasing the probabilities of finding a match.

    :param start_waypoint (carla.Waypoint): Starting waypoint of the route
    :param end_waypoint (carla.Waypoint): Ending waypoint of the route
    :param town_map (carla.Map): CARLA map instance where the route will be computed
    :param sample_resolution (float): Distance between the waypoints that form the route
    :param match_dist (float): Max distance between the given location and the matched AD map para points.
        If this value is too large, the matching might result in waypoints on different lanes.
    :param max_match_dist (float): In case of failed match, the previous input keeps increasing up to this number.
    """
    wp_route = []
    start_location = start_waypoint.transform.location
    end_location = end_waypoint.transform.location

    # Get starting point matches. Due to errors in altitude parsing, iterate increasing the matching distance
    added_dist = 0
    start_matches = None
    while not start_matches and added_dist < max_match_dist:
        start_matches = _waypoint_matches(start_waypoint, town_map, match_dist + added_dist)
        added_dist += 0.2

    if not start_matches:
        print("WARNING: Couldn't find a paraPoint for location '{}'.".format(start_location))
        return wp_route

    # Get ending point matches. Due to errors in altitude parsing, iterate increasing the matching distance
    added_dist = 0
    end_matches = None
    while not end_matches and added_dist < max_match_dist:
        end_matches = _waypoint_matches(end_waypoint, town_map, match_dist + added_dist)
        added_dist += 0.2

    if not end_matches:
        print("WARNING: Couldn't find a paraPoint for location '{}'.".format(end_location))
        return wp_route

    # Get the shortest route
    route_segment = _filter_shortest_route(start_matches, end_matches)
    if not route_segment:
        print("WARNING: Couldn't find a viable route between locations "
              "'{}' and '{}'.".format(start_location, end_location))
        return wp_route

    # Change the route to waypoints
    wp_route = _get_route_waypoints(route_segment, sample_resolution, town_map)
    return wp_route

def _waypoint_matches(waypoint, town_map, max_distance, probability=0):
    """
    Given a waypoint, maps its transform to the AD map, returning a list of possible matches.
    All matches are filtered to make sure they represent driving lanes.
    """
    # ECEF location of the waypoint
    location = waypoint.transform.location
    ecef_location= carla_loc_to_ecef(location)

    # ECEF location of a point in front of the waypoint
    f_vec = waypoint.transform.get_right_vector()
    front_location = location + carla.Location(x=f_vec.x, y=f_vec.y)
    ecef_front_location= carla_loc_to_ecef(front_location)

    # Get the map matching and the heading hint
    ecef_heading = ad.map.point.createECEFHeading(ecef_location, ecef_front_location)
    ad_map_matching = ad.map.match.AdMapMatching()
    ad_map_matching.addHeadingHint(ecef_heading)

    # Get the matches and filter the none driving lane ones
    matches = ad_map_matching.getMapMatchedPositions(
        carla_loc_to_enu(location),
        ad.physics.Distance(max_distance),
        ad.physics.Probability(probability)
    )
    matches = [m for m in matches if is_point_at_driving_lane(m.lanePoint.paraPoint, town_map)]

    return matches

def _filter_shortest_route(start_matches, end_matches):
    """Given a set of starting and ending matches, computes all possible routes and selects the shortest one"""
    route_segment = None
    # Get the shortest route
    min_length = float('inf')
    for start_match in start_matches:
        start_point = start_match.lanePoint.paraPoint
        for end_match in end_matches:
            # Get the route
            new_route_segment = ad.map.route.planRoute(
                start_point, end_match.lanePoint.paraPoint, ad.map.route.RouteCreationMode.Undefined)
            if len(new_route_segment.roadSegments) == 0:
                continue  # The route doesn't exist, ignore it

            # Save the shortest route
            length = _get_route_length(new_route_segment)
            if length < min_length:
                min_length = length
                route_segment = new_route_segment

    return route_segment

def _get_route_length(route):
    """
    Gets the length of the route, being the sum of the road segment lengths.
    Each road segment length is the mean of the length of its lane segments.
    """
    route_length = 0
    for road_segment in route.roadSegments:
        road_length = 0
        for lane_segment in road_segment.drivableLaneSegments:
            lane_start = float(lane_segment.laneInterval.start)
            lane_end = float(lane_segment.laneInterval.end)
            lane_length = float(ad.map.lane.calcLength(lane_segment.laneInterval.laneId))

            road_length += lane_length * abs(lane_end - lane_start)

        num_lanes = len(road_segment.drivableLaneSegments)
        if num_lanes != 0:
            route_length += road_length / num_lanes

    return route_length

def _get_route_waypoints(route, resolution, town_map):
    """
    Given a route, transforms it into a list of [carla.Waypoint, RoadOption].
    Take into account that at locations where multiple lanes overlap,
    while the waypoints will be correctly placed, they might be part of a different lane.

    :param route (ad.map.route.FullRoute): AD map route instance created with RouteCreationMode Undefined.
        Other creation modes return mode than one lane, which would need a prefiltering.
    :param resolution (float): Distance between the waypoints that form the route.
    :param town_map (carla.Map): CARLA map instance where the route will be computed
    """
    # TODO: use para_point_to_carla_waypoint_2, which directly transforms from paraPoint to waypoint
    # to ensure all waypoints correspond to the correct lane

    wp_route = []
    for road_segment in route.roadSegments:
        for lane_segment in road_segment.drivableLaneSegments:
            # print(" ----------------- ")
            # border = ad.map.route.getENUBorder(lane_segment.laneInterval)
            # for point in border.left:
            #     world.debug.draw_point(enu_to_carla_loc(point), life_time=100, size=0.2)
            # for point in border.right:
            #     world.debug.draw_point(enu_to_carla_loc(point), life_time=100, size=0.2)
            # border_ = ad.map.lane.getLateralAlignmentEdge(border, ad.physics.ParametricValue(0.5))
            # length = 0
            # for i in range(0, len(border_)-1):
            #     p1 = border_[i]
            #     p2 = border_[i+1]
            #     dist = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2) + pow(p2.z - p1.z, 2))
            #     print("Adding: {}".format(dist))
            #     length += dist
            #     world.debug.draw_point(enu_to_carla_loc(p1), life_time=100, size=0.2, color=carla.Color(0,0,0))
            #     world.debug.draw_arrow(enu_to_carla_loc(p1), enu_to_carla_loc(p2), life_time=100, color=carla.Color(0,0,0))
            # print("Summ Length: {}".format(length))
            # print("Calc Length: {}".format(ad.map.lane.calcLength(lane_segment.laneInterval.laneId)))

            lane_id = lane_segment.laneInterval.laneId
            param_list = _get_lane_interval_list(lane_segment.laneInterval, resolution)
            for param in param_list:
                para_point = ad.map.point.createParaPoint(lane_id, ad.physics.ParametricValue(param))
                carla_waypoint = para_point_to_carla_waypoint(para_point, town_map)
                wp_route.append(carla_waypoint)

    return wp_route

def _get_lane_interval_list(lane_interval, distance=1):
    """
    Separates a given lane interval into smaller intervals of length equal to 'distance'
    """
    start = float(lane_interval.start)
    end = float(lane_interval.end)
    length = float(ad.map.lane.calcLength(lane_interval.laneId))
    if start == end:
        return []
    return np.arange(start, end, np.sign(end - start) * distance / length)
