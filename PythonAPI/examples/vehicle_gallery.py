#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Visualize the catalog of vehicles present in CARLA"""

import carla

import math


def get_transform(vehicle, angle, d=2.5):
    vehicle_location = vehicle.get_location()
    radius = vehicle.bounding_box.extent.x * d
    height = vehicle.bounding_box.extent.x * 0.8
    a = math.radians(angle)
    location = carla.Location(radius * math.cos(a), radius * math.sin(a), height) + vehicle_location
    return carla.Transform(location, carla.Rotation(yaw=180 + angle, pitch=-15))


def main():
    client = carla.Client()
    client.set_timeout(10.0)
    world = client.get_world()
    spectator = world.get_spectator()
    vehicle_blueprints = world.get_blueprint_library().filter('vehicle')

    location = carla.Location(-47, 20, 0.3)

    for blueprint in vehicle_blueprints:
        transform = carla.Transform(location, carla.Rotation(yaw=-45.0))
        vehicle = world.spawn_actor(blueprint, transform)

        try:

            print(vehicle.type_id)

            angle = 0
            while angle < 356:
                timestamp = world.wait_for_tick().timestamp
                angle += timestamp.delta_seconds * 60.0
                spectator.set_transform(get_transform(vehicle, angle - 90))

        finally:

            vehicle.destroy()


if __name__ == '__main__':

    main()
