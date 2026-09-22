#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Destroys explicitly identified demo actors and their attached sensors. It is
# intentionally opt-in: a role_name such as "hero" is shared by common CARLA
# workflows, so treating it as ownership could destroy another client's actor.

import argparse

import carla


def main(args):
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()
    actors = world.get_actors()

    leftovers = []
    for actor_id in args.actor_id:
        actor = world.get_actor(actor_id)
        if actor is None:
            print("Actor {} no longer exists".format(actor_id))
        elif actor.type_id.startswith('vehicle.'):
            leftovers.append(actor)
        else:
            print("Actor {} is not a vehicle; skipping".format(actor_id))
    if args.role_name:
        for actor in actors.filter('vehicle.*'):
            if actor.attributes.get('role_name') == args.role_name:
                leftovers.append(actor)

    for vehicle in leftovers:
        for sensor in actors.filter('sensor.*'):
            if sensor.parent is not None and sensor.parent.id == vehicle.id:
                sensor.destroy()
        vehicle.destroy()
        print("Destroyed demo vehicle {}".format(vehicle.id))

    if not leftovers:
        print("No explicitly identified demo vehicles to destroy")


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='CARLA demo leftover actor cleanup')
    argparser.add_argument('--host', metavar='H', default='localhost', help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port', metavar='P', default=2000, type=int, help='TCP port of CARLA Simulator (default: 2000)')
    targets = argparser.add_mutually_exclusive_group()
    targets.add_argument('--actor-id', metavar='ID', type=int, action='append', default=[],
                           help='Vehicle actor id created by this demo (repeatable). No actors are destroyed unless supplied.')
    targets.add_argument('--role-name', metavar='NAME',
                           help='Private role_name owned by this demo. Matching vehicles and their attached sensors are destroyed.')

    main(argparser.parse_args())
