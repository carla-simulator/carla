#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Module with auxiliary functions."""

from __future__ import annotations

import math
from typing import TYPE_CHECKING

import numpy as np

import carla

if TYPE_CHECKING:
    from collections.abc import Iterable

    from carla import Location, TrafficLight, Transform, Vehicle, Waypoint, World


def draw_waypoints(world: World, waypoints: Iterable[Waypoint], z: float = 0.5) -> None:
    """Draw a list of waypoints at a certain height given in z.

    Args:
        world: carla.World object.
        waypoints: Iterable container with the waypoints to draw.
        z: Height in meters.
    """
    for wpt in waypoints:
        wpt_t = wpt.transform
        begin = wpt_t.location + carla.Location(z=z)
        angle = math.radians(wpt_t.rotation.yaw)
        end = begin + carla.Location(x=math.cos(angle), y=math.sin(angle))
        world.debug.draw_arrow(begin, end, arrow_size=0.3, life_time=1.0)


def get_speed(vehicle: Vehicle) -> float:
    """Compute speed of a vehicle in Km/h.

    Args:
        vehicle: The vehicle for which speed is calculated.

    Returns:
        Speed as a float in Km/h.
    """
    vel = vehicle.get_velocity()
    return 3.6 * math.sqrt(vel.x**2 + vel.y**2 + vel.z**2)


def get_trafficlight_trigger_location(traffic_light: TrafficLight) -> Location:
    """Calculate the location of the trigger volume of the traffic light.

    Args:
        traffic_light: The traffic light to get trigger location for.

    Returns:
        Location of the trigger volume.
    """

    def rotate_point(point: carla.Vector3D, radians: float) -> carla.Vector3D:
        """Rotate a given point by a given angle."""
        rotated_x = math.cos(radians) * point.x - math.sin(radians) * point.y
        rotated_y = math.sin(radians) * point.x - math.cos(radians) * point.y
        return carla.Vector3D(rotated_x, rotated_y, point.z)

    base_transform = traffic_light.get_transform()
    base_rot = base_transform.rotation.yaw
    area_loc = base_transform.transform(traffic_light.trigger_volume.location)
    area_ext = traffic_light.trigger_volume.extent

    point = rotate_point(carla.Vector3D(0, 0, area_ext.z), math.radians(base_rot))
    point_location = area_loc + carla.Location(x=point.x, y=point.y)

    return carla.Location(point_location.x, point_location.y, point_location.z)


def is_within_distance(
    target_transform: Transform,
    reference_transform: Transform,
    max_distance: float,
    angle_interval: tuple[float, float] | None = None,
) -> bool:
    """Check if a location is within a certain distance from a reference object.

    By using 'angle_interval', the angle between the location and reference transform
    will also be taken into account, being 0 a location in front and 180, one behind.

    Args:
        target_transform: Transform of the target object.
        reference_transform: Transform of the reference object.
        max_distance: Maximum allowed distance.
        angle_interval: Only locations between [min, max] angles will be considered.

    Returns:
        True if within distance and angle constraints.
    """
    target_vector = np.array(
        [
            target_transform.location.x - reference_transform.location.x,
            target_transform.location.y - reference_transform.location.y,
        ]
    )
    norm_target = np.linalg.norm(target_vector)

    if norm_target < 0.001:
        return True

    if norm_target > max_distance:
        return False

    if not angle_interval:
        return True

    min_angle = angle_interval[0]
    max_angle = angle_interval[1]

    fwd = reference_transform.get_forward_vector()
    forward_vector = np.array([fwd.x, fwd.y])
    angle = math.degrees(math.acos(np.clip(np.dot(forward_vector, target_vector) / norm_target, -1.0, 1.0)))

    return min_angle < angle < max_angle


def compute_magnitude_angle(
    target_location: Location,
    current_location: Location,
    orientation: float,
) -> tuple[float, float]:
    """Compute relative angle and distance between a target_location and a current_location.

    Args:
        target_location: Location of the target object.
        current_location: Location of the reference object.
        orientation: Orientation of the reference object in degrees.

    Returns:
        Tuple of (distance, angle) between the objects.
    """
    target_vector = np.array([target_location.x - current_location.x, target_location.y - current_location.y])
    norm_target = np.linalg.norm(target_vector)

    forward_vector = np.array([math.cos(math.radians(orientation)), math.sin(math.radians(orientation))])
    d_angle = math.degrees(math.acos(np.clip(np.dot(forward_vector, target_vector) / norm_target, -1.0, 1.0)))

    return norm_target, d_angle


def distance_vehicle(waypoint: Waypoint, vehicle_transform: Transform) -> float:
    """Return the 2D distance from a waypoint to a vehicle.

    Args:
        waypoint: Actual waypoint.
        vehicle_transform: Transform of the target vehicle.

    Returns:
        2D distance in meters.
    """
    loc = vehicle_transform.location
    x = waypoint.transform.location.x - loc.x
    y = waypoint.transform.location.y - loc.y

    return math.sqrt(x * x + y * y)


def vector(location_1: Location, location_2: Location) -> list[float]:
    """Return the unit vector from location_1 to location_2.

    Args:
        location_1: Starting location.
        location_2: Ending location.

    Returns:
        Unit vector as [x, y, z].
    """
    x = location_2.x - location_1.x
    y = location_2.y - location_1.y
    z = location_2.z - location_1.z
    norm = np.linalg.norm([x, y, z]) + np.finfo(float).eps

    return [x / norm, y / norm, z / norm]


def compute_distance(location_1: Location, location_2: Location) -> float:
    """Euclidean distance between 3D points.

    Args:
        location_1: First location.
        location_2: Second location.

    Returns:
        Distance in meters.
    """
    x = location_2.x - location_1.x
    y = location_2.y - location_1.y
    z = location_2.z - location_1.z
    return np.linalg.norm([x, y, z]) + np.finfo(float).eps


def positive(num: float) -> float:
    """Return the given number if positive, else 0.

    Args:
        num: Value to check.

    Returns:
        num if positive, else 0.0.
    """
    return max(0.0, num)
