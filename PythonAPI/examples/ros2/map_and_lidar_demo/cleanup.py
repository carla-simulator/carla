#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Destroys leftover demo actors: every vehicle with the given role_name and the
# sensors attached to it. Also restores asynchronous mode, which a killed stack
# can leave enabled with nobody ticking. Run by launcher.sh before
# spawning so that a previous demo stack that died without cleaning up (killed
# container, double Ctrl+C, docker stop grace timeout) does not leave a second
# vehicle publishing on the same topics.

import argparse

import carla


def main(args):
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()
    actors = world.get_actors()

    leftovers = [vehicle for vehicle in actors.filter('vehicle.*')
                 if vehicle.attributes.get('role_name') == args.role_name]
    for vehicle in leftovers:
        for sensor in actors.filter('sensor.*'):
            if sensor.parent is not None and sensor.parent.id == vehicle.id:
                sensor.destroy()
        vehicle.destroy()
        print("Destroyed leftover vehicle {} (role_name '{}')".format(vehicle.id, args.role_name))

    if not leftovers:
        print("No leftover vehicles with role_name '{}'".format(args.role_name))

    settings = world.get_settings()
    if settings.synchronous_mode:
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        world.apply_settings(settings)
        print('Restored asynchronous mode')


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='CARLA demo leftover actor cleanup')
    argparser.add_argument('--host', metavar='H', default='localhost', help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port', metavar='P', default=2000, type=int, help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('--role-name', metavar='NAME', default='hero', help='role_name of the vehicles to destroy (default: hero)')

    main(argparser.parse_args())
