# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
# Provides map data for users.

from __future__ import annotations

import contextlib
import os
import sys
from pathlib import Path
from typing import Any, TypedDict

with contextlib.suppress(IndexError):
    platform = 'win-amd64' if os.name == 'nt' else 'linux-x86_64'
    egg_pattern = f'dist/carla-*{sys.version_info.major}.{sys.version_info.minor}-{platform}.egg'
    sys.path.append(next(Path('dist').glob(egg_pattern)).as_posix())

import random

import carla


class WaypointDict(TypedDict, total=False):
    """Type definition for waypoint dictionary."""

    road_id: int
    lane_id: int
    position: list[float]
    orientation: list[float]
    left_margin_position: list[float]
    right_margin_position: list[float]
    next_waypoints_ids: list[int]
    left_lane_waypoint_id: int
    right_lane_waypoint_id: int


def get_scene_layout(carla_map: carla.Map) -> dict[str, WaypointDict]:
    """
    Function to extract the full scene layout to be used as a full scene description to be
    given to the user
    :return: a dictionary describing the scene.
    """
    if carla_map is None:
        raise ValueError('carla_map is None')

    def _lateral_shift(transform: carla.Transform, shift: float) -> carla.Location:
        transform.rotation.yaw += 90
        return transform.location + shift * transform.get_forward_vector()

    topology = [x[0] for x in carla_map.get_topology()]
    topology = sorted(topology, key=lambda w: w.transform.location.z)

    # A road contains a list of lanes, a each lane contains a list of waypoints
    map_dict: dict[int, dict[int, Any]] = {}
    precision = 0.05
    for waypoint in topology:
        waypoints = [waypoint]
        nxt = waypoint.next(precision)
        if len(nxt) > 0:
            nxt = nxt[0]
            while nxt.road_id == waypoint.road_id:
                waypoints.append(nxt)
                nxt = nxt.next(precision)
                if len(nxt) > 0:
                    nxt = nxt[0]
                else:
                    break

        left_marking = [_lateral_shift(w.transform, -w.lane_width * 0.5) for w in waypoints]
        right_marking = [_lateral_shift(w.transform, w.lane_width * 0.5) for w in waypoints]

        lane = {'waypoints': waypoints, 'left_marking': left_marking, 'right_marking': right_marking}

        if map_dict.get(waypoint.road_id) is None:
            map_dict[waypoint.road_id] = {}
        map_dict[waypoint.road_id][waypoint.lane_id] = lane

    # Generate waypoints graph
    waypoints_graph = {}
    for road_key in map_dict:  # noqa: PLC0206
        for lane_key in map_dict[road_key]:
            # List of waypoints
            lane = map_dict[road_key][lane_key]

            for i in range(len(lane['waypoints'])):
                next_ids = [w.id for w in lane['waypoints'][i + 1 : len(lane['waypoints'])]]

                # Get left and right lane keys
                left_lane_key = lane_key - 1 if lane_key - 1 != 0 else lane_key - 2
                right_lane_key = lane_key + 1 if lane_key + 1 != 0 else lane_key + 2

                # Get left and right waypoint ids only if they are valid
                left_lane_waypoint_id = -1
                if left_lane_key in map_dict[road_key]:
                    left_lane_waypoints = map_dict[road_key][left_lane_key]['waypoints']
                    if i < len(left_lane_waypoints):
                        left_lane_waypoint_id = left_lane_waypoints[i].id

                right_lane_waypoint_id = -1
                if right_lane_key in map_dict[road_key]:
                    right_lane_waypoints = map_dict[road_key][right_lane_key]['waypoints']
                    if i < len(right_lane_waypoints):
                        right_lane_waypoint_id = right_lane_waypoints[i].id

                # Get left and right margins (aka markings)
                lm = carla_map.transform_to_geolocation(lane['left_marking'][i])
                rm = carla_map.transform_to_geolocation(lane['right_marking'][i])

                # Waypoint Position
                wl = carla_map.transform_to_geolocation(lane['waypoints'][i].transform.location)

                # Waypoint Orientation
                wo = lane['waypoints'][i].transform.rotation

                # Waypoint dict
                waypoint_dict = {
                    'road_id': road_key,
                    'lane_id': lane_key,
                    'position': [wl.latitude, wl.longitude, wl.altitude],
                    'orientation': [wo.roll, wo.pitch, wo.yaw],
                    'left_margin_position': [lm.latitude, lm.longitude, lm.altitude],
                    'right_margin_position': [rm.latitude, rm.longitude, rm.altitude],
                    'next_waypoints_ids': next_ids,
                    'left_lane_waypoint_id': left_lane_waypoint_id,
                    'right_lane_waypoint_id': right_lane_waypoint_id,
                }
                waypoints_graph[map_dict[road_key][lane_key]['waypoints'][i].id] = waypoint_dict

    return waypoints_graph


class DynamicObjectDict(TypedDict, total=False):
    """Type definition for dynamic object dictionary."""

    id: int
    position: list[float]
    orientation: list[float]
    bounding_box: list[list[float]]
    trigger_volume: list[list[float]]
    state: int
    speed: int
    road_id: int
    lane_id: int


def get_dynamic_objects(carla_world: carla.World, carla_map: carla.Map) -> dict[str, Any]:
    """
    Extract dynamic objects from the scene.

    Args:
        carla_world: CARLA world instance
        carla_map: CARLA map instance

    Returns:
        Dictionary containing dynamic objects

    Raises:
        ValueError: If carla_world or carla_map is None
    """
    if carla_world is None:
        raise ValueError('carla_world is None')
    if carla_map is None:
        raise ValueError('carla_map is None')

    # Private helper functions
    def _get_bounding_box(actor: carla.Actor) -> list[carla.Location]:
        bb = actor.bounding_box.extent
        corners = [
            carla.Location(x=-bb.x, y=-bb.y),
            carla.Location(x=bb.x, y=-bb.y),
            carla.Location(x=bb.x, y=bb.y),
            carla.Location(x=-bb.x, y=bb.y),
        ]
        t = actor.get_transform()
        t.transform(corners)
        return [carla_map.transform_to_geolocation(p) for p in corners]

    def _get_trigger_volume(actor: carla.Actor) -> list[carla.Location]:
        bb = actor.trigger_volume.extent
        corners = [
            carla.Location(x=-bb.x, y=-bb.y),
            carla.Location(x=bb.x, y=-bb.y),
            carla.Location(x=bb.x, y=bb.y),
            carla.Location(x=-bb.x, y=bb.y),
            carla.Location(x=-bb.x, y=-bb.y),
        ]
        corners = [x + actor.trigger_volume.location for x in corners]
        t = actor.get_transform()
        t.transform(corners)
        return [carla_map.transform_to_geolocation(p) for p in corners]

    def _split_actors(
        actors: list[carla.Actor],
    ) -> tuple[
        list[carla.Actor],
        list[carla.Actor],
        list[carla.Actor],
        list[carla.Actor],
        list[carla.Actor],
        list[carla.Actor],
    ]:
        vehicles: list[carla.Actor] = []
        traffic_lights: list[carla.Actor] = []
        speed_limits: list[carla.Actor] = []
        walkers: list[carla.Actor] = []
        stops: list[carla.Actor] = []
        static_obstacles: list[carla.Actor] = []
        for actor in actors:
            if 'vehicle' in actor.type_id:
                vehicles.append(actor)
            elif 'traffic_light' in actor.type_id:
                traffic_lights.append(actor)
            elif 'speed_limit' in actor.type_id:
                speed_limits.append(actor)
            elif 'walker' in actor.type_id:
                walkers.append(actor)
            elif 'stop' in actor.type_id:
                stops.append(actor)
            elif 'static.prop' in actor.type_id:
                static_obstacles.append(actor)

        return (vehicles, traffic_lights, speed_limits, walkers, stops, static_obstacles)

    # Public functions
    def get_stop_signals(
        stops: list[carla.Actor],
    ) -> dict[str, dict[str, Any]]:
        stop_signals_dict: dict[str, dict[str, Any]] = {}
        for stop in stops:
            st_transform = stop.get_transform()
            location_gnss = carla_map.transform_to_geolocation(st_transform.location)
            st_dict = {
                'id': stop.id,
                'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                'trigger_volume': [[v.longitude, v.latitude, v.altitude] for v in _get_trigger_volume(stop)],
            }
            stop_signals_dict[stop.id] = st_dict
        return stop_signals_dict

    def get_traffic_lights(
        traffic_lights: list[carla.Actor],
    ) -> dict[str, dict[str, Any]]:
        traffic_lights_dict = {}
        for traffic_light in traffic_lights:
            tl_transform = traffic_light.get_transform()
            location_gnss = carla_map.transform_to_geolocation(tl_transform.location)
            tl_dict = {
                'id': traffic_light.id,
                'state': int(traffic_light.state),
                'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                'trigger_volume': [[v.longitude, v.latitude, v.altitude] for v in _get_trigger_volume(traffic_light)],
            }
            traffic_lights_dict[traffic_light.id] = tl_dict
        return traffic_lights_dict

    def get_vehicles(
        vehicles: list[carla.Actor],
    ) -> dict[str, dict[str, Any]]:
        vehicles_dict: dict[str, dict[str, Any]] = {}
        for vehicle in vehicles:
            v_transform = vehicle.get_transform()
            location_gnss = carla_map.transform_to_geolocation(v_transform.location)
            v_dict = {
                'id': vehicle.id,
                'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                'orientation': [v_transform.rotation.roll, v_transform.rotation.pitch, v_transform.rotation.yaw],
                'bounding_box': [[v.longitude, v.latitude, v.altitude] for v in _get_bounding_box(vehicle)],
            }
            vehicles_dict[vehicle.id] = v_dict
        return vehicles_dict

    def get_hero_vehicle(
        hero_vehicle: carla.Actor | None,
    ) -> dict[str, Any] | None:
        if hero_vehicle is None:
            return hero_vehicle

        hero_waypoint = carla_map.get_waypoint(hero_vehicle.get_location())
        hero_transform = hero_vehicle.get_transform()
        location_gnss = carla_map.transform_to_geolocation(hero_transform.location)

        return {
            'id': hero_vehicle.id,
            'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            'road_id': hero_waypoint.road_id,
            'lane_id': hero_waypoint.lane_id,
        }

    def get_walkers(walkers: list[carla.Actor]) -> dict[str, dict[str, Any]]:
        walkers_dict: dict[str, dict[str, Any]] = {}
        for walker in walkers:
            w_transform = walker.get_transform()
            location_gnss = carla_map.transform_to_geolocation(w_transform.location)
            w_dict = {
                'id': walker.id,
                'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                'orientation': [w_transform.rotation.roll, w_transform.rotation.pitch, w_transform.rotation.yaw],
                'bounding_box': [[v.longitude, v.latitude, v.altitude] for v in _get_bounding_box(walker)],
            }
            walkers_dict[walker.id] = w_dict
        return walkers_dict

    def get_speed_limits(
        speed_limits: list[carla.Actor],
    ) -> dict[str, dict[str, Any]]:
        speed_limits_dict: dict[str, dict[str, Any]] = {}
        for speed_limit in speed_limits:
            sl_transform = speed_limit.get_transform()
            location_gnss = carla_map.transform_to_geolocation(sl_transform.location)
            sl_dict = {
                'id': speed_limit.id,
                'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
                'speed': int(speed_limit.type_id.split('.')[2]),
            }
            speed_limits_dict[speed_limit.id] = sl_dict
        return speed_limits_dict

    def get_static_obstacles(
        static_obstacles: list[carla.Actor],
    ) -> dict[str, dict[str, Any]]:
        static_obstacles_dict: dict[str, dict[str, Any]] = {}
        for static_prop in static_obstacles:
            sl_transform = static_prop.get_transform()
            location_gnss = carla_map.transform_to_geolocation(sl_transform.location)
            sl_dict = {
                'id': static_prop.id,
                'position': [location_gnss.latitude, location_gnss.longitude, location_gnss.altitude],
            }
            static_obstacles_dict[static_prop.id] = sl_dict
        return static_obstacles_dict

    actors = carla_world.get_actors()
    vehicles, traffic_lights, speed_limits, walkers, stops, static_obstacles = _split_actors(actors)

    hero_vehicles = [
        vehicle for vehicle in vehicles if 'vehicle' in vehicle.type_id and vehicle.attributes['role_name'] == 'hero'
    ]
    hero = None if len(hero_vehicles) == 0 else random.choice(hero_vehicles)

    return {
        'vehicles': get_vehicles(vehicles),
        'hero_vehicle': get_hero_vehicle(hero),
        'walkers': get_walkers(walkers),
        'traffic_lights': get_traffic_lights(traffic_lights),
        'stop_signs': get_stop_signals(stops),
        'speed_limits': get_speed_limits(speed_limits),
        'static_obstacles': get_static_obstacles(static_obstacles),
    }
