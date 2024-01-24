#!/usr/bin/env python

# Copyright (c) 2019 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Summary of useful helper functions for scenarios
"""

import math
import shapely.geometry
import shapely.affinity

import numpy as np

import carla
from agents.tools.misc import vector
from agents.navigation.local_planner import RoadOption

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider


def get_distance_along_route(route, target_location):
    """
    Calculate the distance of the given location along the route

    Note: If the location is not along the route, the route length will be returned
    """

    wmap = CarlaDataProvider.get_map()
    covered_distance = 0
    prev_position = None
    found = False

    # Don't use the input location, use the corresponding wp as location
    target_location_from_wp = wmap.get_waypoint(target_location).transform.location

    for position, _ in route:

        location = target_location_from_wp

        # Don't perform any calculations for the first route point
        if not prev_position:
            prev_position = position
            continue

        # Calculate distance between previous and current route point
        interval_length_squared = ((prev_position.x - position.x) ** 2) + ((prev_position.y - position.y) ** 2)
        distance_squared = ((location.x - prev_position.x) ** 2) + ((location.y - prev_position.y) ** 2)

        # Close to the current position? Stop calculation
        if distance_squared < 0.01:
            break

        if distance_squared < 400 and not distance_squared < interval_length_squared:
            # Check if a neighbor lane is closer to the route
            # Do this only in a close distance to correct route interval, otherwise the computation load is too high
            starting_wp = wmap.get_waypoint(location)
            wp = starting_wp.get_left_lane()
            while wp is not None:
                new_location = wp.transform.location
                new_distance_squared = ((new_location.x - prev_position.x) ** 2) + (
                    (new_location.y - prev_position.y) ** 2)

                if np.sign(starting_wp.lane_id) != np.sign(wp.lane_id):
                    break

                if new_distance_squared < distance_squared:
                    distance_squared = new_distance_squared
                    location = new_location
                else:
                    break

                wp = wp.get_left_lane()

            wp = starting_wp.get_right_lane()
            while wp is not None:
                new_location = wp.transform.location
                new_distance_squared = ((new_location.x - prev_position.x) ** 2) + (
                    (new_location.y - prev_position.y) ** 2)

                if np.sign(starting_wp.lane_id) != np.sign(wp.lane_id):
                    break

                if new_distance_squared < distance_squared:
                    distance_squared = new_distance_squared
                    location = new_location
                else:
                    break

                wp = wp.get_right_lane()

        if distance_squared < interval_length_squared:
            # The location could be inside the current route interval, if route/lane ids match
            # Note: This assumes a sufficiently small route interval
            # An alternative is to compare orientations, however, this also does not work for
            # long route intervals

            curr_wp = wmap.get_waypoint(position)
            prev_wp = wmap.get_waypoint(prev_position)
            wp = wmap.get_waypoint(location)

            if prev_wp and curr_wp and wp:
                if wp.road_id in (prev_wp.road_id, curr_wp.road_id):
                    # Roads match, now compare the sign of the lane ids
                    if (np.sign(wp.lane_id) == np.sign(prev_wp.lane_id) or
                            np.sign(wp.lane_id) == np.sign(curr_wp.lane_id)):
                        # The location is within the current route interval
                        covered_distance += math.sqrt(distance_squared)
                        found = True
                        break

        covered_distance += math.sqrt(interval_length_squared)
        prev_position = position

    return covered_distance, found


def get_crossing_point(actor):
    """
    Get the next crossing point location in front of the ego vehicle

    @return point of crossing
    """
    wp_cross = CarlaDataProvider.get_map().get_waypoint(actor.get_location())

    while not wp_cross.is_intersection:
        wp_cross = wp_cross.next(2)[0]

    crossing = carla.Location(x=wp_cross.transform.location.x,
                              y=wp_cross.transform.location.y, z=wp_cross.transform.location.z)

    return crossing


def get_geometric_linear_intersection(ego_location, other_location):
    """
    Obtain a intersection point between two actor's location by using their waypoints (wp)

    @return point of intersection of the two vehicles
    """

    wp_ego_1 = CarlaDataProvider.get_map().get_waypoint(ego_location)
    wp_ego_2 = wp_ego_1.next(1)[0]
    ego_1_loc = wp_ego_1.transform.location
    ego_2_loc = wp_ego_2.transform.location

    wp_other_1 = CarlaDataProvider.get_world().get_map().get_waypoint(other_location)
    wp_other_2 = wp_other_1.next(1)[0]
    other_1_loc = wp_other_1.transform.location
    other_2_loc = wp_other_2.transform.location

    s = np.vstack([
        (ego_1_loc.x, ego_1_loc.y),
        (ego_2_loc.x, ego_2_loc.y),
        (other_1_loc.x, other_1_loc.y),
        (other_2_loc.x, other_2_loc.y)
    ])
    h = np.hstack((s, np.ones((4, 1))))
    line1 = np.cross(h[0], h[1])
    line2 = np.cross(h[2], h[3])
    x, y, z = np.cross(line1, line2)
    if z == 0:
        return None

    return carla.Location(x=x/z, y=y/z, z=0)


def get_location_in_distance(actor, distance):
    """
    Obtain a location in a given distance from the current actor's location.
    Note: Search is stopped on first intersection.

    @return obtained location and the traveled distance
    """
    waypoint = CarlaDataProvider.get_map().get_waypoint(actor.get_location())
    traveled_distance = 0
    while not waypoint.is_intersection and traveled_distance < distance:
        waypoint_new = waypoint.next(1.0)[-1]
        traveled_distance += waypoint_new.transform.location.distance(waypoint.transform.location)
        waypoint = waypoint_new

    return waypoint.transform.location, traveled_distance


def get_location_in_distance_from_wp(waypoint, distance, stop_at_junction=True):
    """
    Obtain a location in a given distance from the current actor's location.
    Note: Search is stopped on first intersection.

    @return obtained location and the traveled distance
    """
    traveled_distance = 0
    while not (waypoint.is_intersection and stop_at_junction) and traveled_distance < distance:
        wp_next = waypoint.next(1.0)
        if wp_next:
            waypoint_new = wp_next[-1]
            traveled_distance += waypoint_new.transform.location.distance(waypoint.transform.location)
            waypoint = waypoint_new
        else:
            break

    return waypoint.transform.location, traveled_distance


def get_waypoint_in_distance(waypoint, distance, stop_at_junction=True):
    """
    Obtain a waypoint in a given distance from the current actor's location.
    Note: Search is stopped on first intersection.
    @return obtained waypoint and the traveled distance
    """
    traveled_distance = 0
    while not (waypoint.is_intersection and stop_at_junction) and traveled_distance < distance:
        wp_next = waypoint.next(1.0)
        if wp_next:
            waypoint_new = wp_next[-1]
            traveled_distance += waypoint_new.transform.location.distance(waypoint.transform.location)
            waypoint = waypoint_new
        else:
            break

    return waypoint, traveled_distance


def generate_target_waypoint_list(waypoint, turn=0):
    """
    This method follow waypoints to a junction and choose path based on turn input.
    Turn input: LEFT -> -1, RIGHT -> 1, STRAIGHT -> 0
    @returns a waypoint list from the starting point to the end point according to turn input
    """
    reached_junction = False
    threshold = math.radians(0.1)
    plan = []
    while True:
        wp_choice = waypoint.next(2)
        if len(wp_choice) > 1:
            reached_junction = True
            waypoint = choose_at_junction(waypoint, wp_choice, turn)
        else:
            waypoint = wp_choice[0]
        plan.append((waypoint, RoadOption.LANEFOLLOW))
        #   End condition for the behavior
        if turn != 0 and reached_junction and len(plan) >= 3:
            v_1 = vector(
                plan[-2][0].transform.location,
                plan[-1][0].transform.location)
            v_2 = vector(
                plan[-3][0].transform.location,
                plan[-2][0].transform.location)
            angle_wp = math.acos(
                np.dot(v_1, v_2) / abs((np.linalg.norm(v_1) * np.linalg.norm(v_2))))
            if angle_wp < threshold:
                break
        elif reached_junction and not plan[-1][0].is_intersection:
            break

    return plan, plan[-1][0]


def generate_target_waypoint_list_multilane(waypoint, change='left',  # pylint: disable=too-many-return-statements
                                            distance_same_lane=10, distance_other_lane=25,
                                            total_lane_change_distance=25, check=True,
                                            lane_changes=1, step_distance=2):
    """
    This methods generates a waypoint list which leads the vehicle to a parallel lane.
    The change input must be 'left' or 'right', depending on which lane you want to change.

    The default step distance between waypoints on the same lane is 2m.
    The default step distance between the lane change is set to 25m.

    @returns a waypoint list from the starting point to the end point on a right or left parallel lane.
    The function might break before reaching the end point, if the asked behavior is impossible.
    """

    plan = []
    plan.append((waypoint, RoadOption.LANEFOLLOW))  # start position

    option = RoadOption.LANEFOLLOW

    # Same lane
    distance = 0
    while distance < distance_same_lane:
        next_wps = plan[-1][0].next(step_distance)
        if not next_wps:
            return None, None
        next_wp = next_wps[0]
        distance += next_wp.transform.location.distance(plan[-1][0].transform.location)
        plan.append((next_wp, RoadOption.LANEFOLLOW))

    if change == 'left':
        option = RoadOption.CHANGELANELEFT
    elif change == 'right':
        option = RoadOption.CHANGELANERIGHT
    else:
        # ERROR, input value for change must be 'left' or 'right'
        return None, None

    lane_changes_done = 0
    lane_change_distance = total_lane_change_distance / lane_changes

    # Lane change
    while lane_changes_done < lane_changes:

        # Move forward
        next_wps = plan[-1][0].next(lane_change_distance)
        if not next_wps:
            return None, None
        next_wp = next_wps[0]

        # Get the side lane
        if change == 'left':
            if check and str(next_wp.lane_change) not in ['Left', 'Both']:
                return None, None
            side_wp = next_wp.get_left_lane()
        else:
            if check and str(next_wp.lane_change) not in ['Right', 'Both']:
                return None, None
            side_wp = next_wp.get_right_lane()

        if not side_wp or side_wp.lane_type != carla.LaneType.Driving:
            return None, None

        # Update the plan
        plan.append((side_wp, option))
        lane_changes_done += 1

    # Other lane
    distance = 0
    while distance < distance_other_lane:
        next_wps = plan[-1][0].next(step_distance)
        if not next_wps:
            return None, None
        next_wp = next_wps[0]
        distance += next_wp.transform.location.distance(plan[-1][0].transform.location)
        plan.append((next_wp, RoadOption.LANEFOLLOW))

    target_lane_id = plan[-1][0].lane_id

    return plan, target_lane_id


def generate_target_waypoint(waypoint, turn=0):
    """
    This method follow waypoints to a junction and choose path based on turn input.
    Turn input: LEFT -> -1, RIGHT -> 1, STRAIGHT -> 0
    @returns a waypoint list according to turn input
    """
    sampling_radius = 1
    reached_junction = False
    wp_list = []
    while True:

        wp_choice = waypoint.next(sampling_radius)
        #   Choose path at intersection
        if not reached_junction and (len(wp_choice) > 1 or wp_choice[0].is_junction):
            reached_junction = True
            waypoint = choose_at_junction(waypoint, wp_choice, turn)
        else:
            waypoint = wp_choice[0]
        wp_list.append(waypoint)
        #   End condition for the behavior
        if reached_junction and not wp_list[-1].is_junction:
            break
    return wp_list[-1]


def generate_target_waypoint_in_route(waypoint, route):
    """
    This method follow waypoints to a junction
    @returns a waypoint list according to turn input
    """
    wmap = CarlaDataProvider.get_map()
    reached_junction = False

    # Get the route location
    shortest_distance = float('inf')
    for index, route_pos in enumerate(route):
        wp = route_pos[0]
        trigger_location = waypoint.transform.location

        dist_to_route = trigger_location.distance(wp)
        if dist_to_route <= shortest_distance:
            closest_index = index
            shortest_distance = dist_to_route

    route_location = route[closest_index][0]
    index = closest_index

    while True:
        # Get the next route location
        index = min(index + 1, len(route))
        route_location = route[index][0]
        road_option = route[index][1]

        # Enter the junction
        if not reached_junction and (road_option in (RoadOption.LEFT, RoadOption.RIGHT, RoadOption.STRAIGHT)):
            reached_junction = True

        # End condition for the behavior, at the end of the junction
        if reached_junction and (road_option not in (RoadOption.LEFT, RoadOption.RIGHT, RoadOption.STRAIGHT)):
            break

    return wmap.get_waypoint(route_location)


def choose_at_junction(current_waypoint, next_choices, direction=0):
    """
    This function chooses the appropriate waypoint from next_choices based on direction
    """
    current_transform = current_waypoint.transform
    current_location = current_transform.location
    projected_location = current_location + \
        carla.Location(
            x=math.cos(math.radians(current_transform.rotation.yaw)),
            y=math.sin(math.radians(current_transform.rotation.yaw)))
    current_vector = vector(current_location, projected_location)
    cross_list = []
    cross_to_waypoint = {}
    for waypoint in next_choices:
        waypoint = waypoint.next(10)[0]
        select_vector = vector(current_location, waypoint.transform.location)
        cross = np.cross(current_vector, select_vector)[2]
        cross_list.append(cross)
        cross_to_waypoint[cross] = waypoint
    select_cross = None
    if direction > 0:
        select_cross = max(cross_list)
    elif direction < 0:
        select_cross = min(cross_list)
    else:
        select_cross = min(cross_list, key=abs)

    return cross_to_waypoint[select_cross]


def get_intersection(ego_actor, other_actor):
    """
    Obtain a intersection point between two actor's location
    @return the intersection location
    """
    waypoint = CarlaDataProvider.get_map().get_waypoint(ego_actor.get_location())
    waypoint_other = CarlaDataProvider.get_map().get_waypoint(other_actor.get_location())
    max_dist = float("inf")
    distance = float("inf")
    while distance <= max_dist:
        max_dist = distance
        current_location = waypoint.transform.location
        waypoint_choice = waypoint.next(1)
        #   Select the straighter path at intersection
        if len(waypoint_choice) > 1:
            max_dot = -1 * float('inf')
            loc_projection = current_location + carla.Location(
                x=math.cos(math.radians(waypoint.transform.rotation.yaw)),
                y=math.sin(math.radians(waypoint.transform.rotation.yaw)))
            v_current = vector(current_location, loc_projection)
            for wp_select in waypoint_choice:
                v_select = vector(current_location, wp_select.transform.location)
                dot_select = np.dot(v_current, v_select)
                if dot_select > max_dot:
                    max_dot = dot_select
                    waypoint = wp_select
        else:
            waypoint = waypoint_choice[0]
        distance = current_location.distance(waypoint_other.transform.location)

    return current_location


def detect_lane_obstacle(actor, extension_factor=3, margin=1.02):
    """
    This function identifies if an obstacle is present in front of the reference actor
    """
    world = CarlaDataProvider.get_world()
    world_actors = world.get_actors().filter('vehicle.*')
    actor_bbox = actor.bounding_box
    actor_transform = actor.get_transform()
    actor_location = actor_transform.location
    actor_vector = actor_transform.rotation.get_forward_vector()
    actor_vector = np.array([actor_vector.x, actor_vector.y])
    actor_vector = actor_vector / np.linalg.norm(actor_vector)
    actor_vector = actor_vector * (extension_factor - 1) * actor_bbox.extent.x
    actor_location = actor_location + carla.Location(actor_vector[0], actor_vector[1])
    actor_yaw = actor_transform.rotation.yaw

    is_hazard = False
    for adversary in world_actors:
        if adversary.id != actor.id and \
                actor_transform.location.distance(adversary.get_location()) < 50:
            adversary_bbox = adversary.bounding_box
            adversary_transform = adversary.get_transform()
            adversary_loc = adversary_transform.location
            adversary_yaw = adversary_transform.rotation.yaw
            overlap_adversary = RotatedRectangle(
                adversary_loc.x, adversary_loc.y,
                2 * margin * adversary_bbox.extent.x, 2 * margin * adversary_bbox.extent.y, adversary_yaw)
            overlap_actor = RotatedRectangle(
                actor_location.x, actor_location.y,
                2 * margin * actor_bbox.extent.x * extension_factor, 2 * margin * actor_bbox.extent.y, actor_yaw)
            overlap_area = overlap_adversary.intersection(overlap_actor).area
            if overlap_area > 0:
                is_hazard = True
                break

    return is_hazard


def get_junction_topology(junction):
    """
    Given a junction, returns a two list of waypoints corresponding to the entry
    and exit lanes of the junction
    """
    def get_lane_key(waypoint):
        return str(waypoint.road_id) + '*' + str(waypoint.lane_id)

    def get_junction_entry_wp(entry_wp):
        while entry_wp.is_junction:
            entry_wps = entry_wp.previous(0.2)
            if len(entry_wps) == 0:
                return None
            entry_wp = entry_wps[0]
        return entry_wp

    def get_junction_exit_wp(exit_wp):
        while exit_wp.is_junction:
            exit_wps = exit_wp.next(0.2)
            if len(exit_wps) == 0:
                return None
            exit_wp = exit_wps[0]
        return exit_wp

    used_entry_lanes = []
    used_exit_lanes = []
    entry_wps = []
    exit_wps = []
    for entry_wp, exit_wp in junction.get_waypoints(carla.LaneType.Driving):
        entry_wp = get_junction_entry_wp(entry_wp)
        if not entry_wp:
            continue
        if get_lane_key(entry_wp) not in used_entry_lanes:
            used_entry_lanes.append(get_lane_key(entry_wp))
            entry_wps.append(entry_wp)

        exit_wp = get_junction_exit_wp(exit_wp)
        if not exit_wp:
            continue
        if get_lane_key(exit_wp) not in used_exit_lanes:
            used_exit_lanes.append(get_lane_key(exit_wp))
            exit_wps.append(exit_wp)

    return entry_wps, exit_wps


def filter_junction_wp_direction(reference_wp, wp_list, direction='opposite'):
    """
    Given a list of entry / exit wps of a junction, filters them according to a specific direction,
    returning all waypoint part of lanes that are at 'direction' with respect to the reference.
    This might fail for complex junctions, as only the wp yaws is checked, not their relative positions
    """

    filtered_wps = []
    reference_yaw = reference_wp.transform.rotation.yaw
    for wp in wp_list:
        diff = (wp.transform.rotation.yaw - reference_yaw) % 360
        if diff > 330.0:
            wp_direction = 'ref'
        elif diff > 225.0:
            wp_direction = 'right'
        elif diff > 135.0:
            wp_direction = 'opposite'
        elif diff > 30.0:
            wp_direction = 'left'
        else:
            wp_direction = 'ref'

        if wp_direction == direction:
            filtered_wps.append(wp)

    return filtered_wps


def get_closest_traffic_light(waypoint, traffic_lights=None):
    """
    Returns the traffic light closest to the waypoint. The distance is computed between the
    waypoint and the traffic light's bounding box.
    Checks all traffic lights part of 'traffic_lights', or all the town ones, if None are passed.
    """
    if not traffic_lights:
        traffic_lights = CarlaDataProvider.get_world().get_actors().filter('*traffic_light*')

    closest_dist = float('inf')
    closest_tl = None

    wp_location = waypoint.transform.location
    for tl in traffic_lights:
        tl_waypoints = tl.get_stop_waypoints()
        for tl_waypoint in tl_waypoints:
            distance = wp_location.distance(tl_waypoint.transform.location)
            if distance < closest_dist:
                closest_dist = distance
                closest_tl = tl

    return closest_tl


def get_offset_transform(transform, offset):
    """
    This function adjusts the give transform by offset and returns the new transform.
    """
    if offset != 0:
        forward_vector = transform.rotation.get_forward_vector()
        orthogonal_vector = carla.Vector3D(x=-forward_vector.y, y=forward_vector.x, z=forward_vector.z)
        transform.location.x = transform.location.x + offset * orthogonal_vector.x
        transform.location.y = transform.location.y + offset * orthogonal_vector.y
    return transform


def get_troad_from_transform(actor_transform):
    """
    This function finds the lateral road position (t) from actor_transform
    """
    actor_loc = actor_transform.location
    c_wp = CarlaDataProvider.get_map().get_waypoint(actor_loc)
    left_lanes, right_lanes = [], []
    # opendrive standard: (left ==> +ve lane_id) and (right ==> -ve lane_id)
    ref_lane = CarlaDataProvider.get_map().get_waypoint_xodr(c_wp.road_id, 0, c_wp.s)
    for i in range(-50, 50):
        _wp = CarlaDataProvider.get_map().get_waypoint_xodr(c_wp.road_id, i, c_wp.s)
        if _wp:
            if i < 0:
                left_lanes.append(_wp)
            elif i > 0:
                right_lanes.append(_wp)

    if left_lanes:
        left_lane_ids = [ln.lane_id for ln in left_lanes]
        lm_id = min(left_lane_ids)
        lm_lane = left_lanes[left_lane_ids.index(lm_id)]
        lm_lane_offset = lm_lane.lane_width / 2
    else:
        lm_lane, lm_lane_offset = ref_lane, 0
    lm_tr = get_offset_transform(carla.Transform(lm_lane.transform.location, lm_lane.transform.rotation),
                                 lm_lane_offset)
    distance_from_lm_lane_edge = lm_tr.location.distance(actor_loc)
    distance_from_lm_lane_ref_lane = lm_tr.location.distance(ref_lane.transform.location)
    if right_lanes:
        right_lane_ids = [ln.lane_id for ln in right_lanes]
        rm_id = max(right_lane_ids)
        rm_lane = right_lanes[right_lane_ids.index(rm_id)]
        rm_lane_offset = -rm_lane.lane_width / 2
    else:
        rm_lane, rm_lane_offset = ref_lane, -distance_from_lm_lane_ref_lane
    distance_from_rm_lane_edge = get_offset_transform(carla.Transform(rm_lane.transform.location,
                                                                      rm_lane.transform.rotation),
                                                      rm_lane_offset).location.distance(actor_loc)
    t_road = ref_lane.transform.location.distance(actor_loc)
    if not right_lanes or not left_lanes:
        closest_road_edge = min(distance_from_lm_lane_edge, distance_from_rm_lane_edge)
        if closest_road_edge == distance_from_lm_lane_edge:
            t_road = -1*t_road
    else:
        if c_wp.lane_id < 0:
            t_road = -1*t_road

    return t_road


def get_distance_between_actors(current, target, distance_type="euclidianDistance", freespace=False,
                                global_planner=None):
    """
    This function finds the distance between actors for different use cases described by distance_type and freespace
    attributes
    """

    target_transform = CarlaDataProvider.get_transform(target)
    current_transform = CarlaDataProvider.get_transform(current)
    target_wp = CarlaDataProvider.get_map().get_waypoint(target_transform.location)
    current_wp = CarlaDataProvider.get_map().get_waypoint(current_transform.location)

    extent_sum_x, extent_sum_y = 0, 0
    if freespace:
        if isinstance(target, (carla.Vehicle, carla.Walker)):
            extent_sum_x = target.bounding_box.extent.x + current.bounding_box.extent.x
            extent_sum_y = target.bounding_box.extent.y + current.bounding_box.extent.y
    if distance_type == "longitudinal":
        if not current_wp.road_id == target_wp.road_id:
            distance = 0
            # Get the route
            route = global_planner.trace_route(current_transform.location, target_transform.location)
            # Get the distance of the route
            for i in range(1, len(route)):
                curr_loc = route[i][0].transform.location
                prev_loc = route[i - 1][0].transform.location
                distance += curr_loc.distance(prev_loc)
        else:
            distance = abs(current_wp.s - target_wp.s)
        if freespace:
            distance = distance - extent_sum_x
    elif distance_type == "lateral":
        target_t = get_troad_from_transform(target_transform)
        current_t = get_troad_from_transform(current_transform)
        distance = abs(target_t - current_t)
        if freespace:
            distance = distance - extent_sum_y

    elif distance_type in ["cartesianDistance", "euclidianDistance"]:
        distance = target_transform.location.distance(current_transform.location)
        if freespace:
            distance = distance - extent_sum_x
    else:
        raise TypeError("unknown distance_type: {}".format(distance_type))

    # distance will be negative for feeespace when there is overlap condition
    # truncate to 0.0 when this happens
    distance = 0.0 if distance < 0.0 else distance

    return distance


class RotatedRectangle(object):

    """
    This class contains method to draw rectangle and find intersection point.
    """

    def __init__(self, c_x, c_y, width, height, angle):
        self.c_x = c_x
        self.c_y = c_y
        self.w = width      # pylint: disable=invalid-name
        self.h = height     # pylint: disable=invalid-name
        self.angle = angle

    def get_contour(self):
        """
        create contour
        """
        w = self.w
        h = self.h
        c = shapely.geometry.box(-w / 2.0, -h / 2.0, w / 2.0, h / 2.0)
        rc = shapely.affinity.rotate(c, self.angle)
        return shapely.affinity.translate(rc, self.c_x, self.c_y)

    def intersection(self, other):
        """
        Obtain a intersection point between two contour.
        """
        return self.get_contour().intersection(other.get_contour())
