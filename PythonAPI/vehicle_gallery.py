#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

try:
    sys.path.append(glob.glob('**/*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import math
import random
import time


def get_transform(vehicle_location, angle, d=6.4):
    a = math.radians(angle)
    location = carla.Location(d * math.cos(a), d * math.sin(a), 2.0) + vehicle_location
    return carla.Transform(location, carla.Rotation(yaw=180 + angle, pitch=-15))


def main():
    client = carla.Client('localhost', 2000)
    client.set_timeout(2.0)
    world = client.get_world()
    spectator = world.get_spectator()
    vehicle_blueprints = world.get_blueprint_library().filter('vehicle')

    location = random.choice(world.get_map().get_spawn_points()).location

    for blueprint in vehicle_blueprints:
        transform = carla.Transform(location, carla.Rotation(yaw=-45.0))
        vehicle = world.spawn_actor(blueprint, transform)

        try:

            print(vehicle.type_id)

            angle = 0
            while angle < 356:
                timestamp = world.wait_for_tick()
                angle += timestamp.delta_seconds * 60.0
                spectator.set_transform(get_transform(vehicle.get_location(), angle - 90))

        finally:

            vehicle.destroy()


if __name__ == '__main__':

    main()
