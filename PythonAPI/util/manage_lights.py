#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

import argparse
import sys
import carla


LIGHT_GROUP = {
    'NONE': carla.LightGroup.NONE,
    'Vehicle':  carla.LightGroup.Vehicle,
    'Street': carla.LightGroup.Street,
    'Building':  carla.LightGroup.Building,
    'Other':  carla.LightGroup.Other,
}

def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument('--host', metavar='H', default='127.0.0.1', help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument('-p', '--port', metavar='P', default=2000, type=int, help='TCP port to listen to (default: 2000)')
    argparser.add_argument('-g', '--group', default='NONE', help='Light group')

    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    light_manager = world.get_lightmanager()

    try:
        group = LIGHT_GROUP[args.group]
    except KeyError:
        print(f"Light group '{args.group}' doesn't exist. The possible ones are:")
        print(list(LIGHT_GROUP.keys()))
        sys.exit()

    lights = light_manager.get_all_lights(group)

    # get_color(lights)
    # get_intensity(lights)
    # get_light_group(lights)
    # get_light_state(lights)
    # get_turned_off_lights(group)
    # get_turned_on_lights(group)

    # light_manager.set_active(lights, [True] * len(lights))
    light_manager.set_color(lights, carla.Color(255,0,0))
    # light_manager.set_intensity(lights, 1000)
    # light_manager.set_light_state(lights, )
    # light_manager.turn_on(lights)
    # light_manager.turn_off(lights)

if __name__ == '__main__':
    main()
