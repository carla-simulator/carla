#!/usr/bin/env python

# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

client = carla.Client('localhost', 2000)
world = client.get_world()

location = carla.Location(200.0, 200.0, 200.0)
rotation = carla.Rotation(0.0, 0.0, 0.0)
transform = carla.Transform(location, rotation)

bp_library = world.get_blueprint_library()
bp_audi = bp_library.find('vehicle.audi.tt')
audi = world.spawn_actor(bp_audi, transform)

component_transform = audi.get_component_world_transform('front-blinker-r-1')
print(component_transform)

