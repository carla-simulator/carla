#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import sys

import carla


TRAFFIC_LIGHT_STATES = {
    'Green': carla.TrafficLightState.Green,
    'Yellow':  carla.TrafficLightState.Yellow,
    'Red':  carla.TrafficLightState.Red,
    'Off':  carla.TrafficLightState.Off,
}

def get_traffic_light(world, tl_id):
    tmap = world.get_map()
    topology = tmap.get_topology()
    junction_ids = []
    junctions = []
    for (entry_wp, _) in topology:
        if entry_wp.is_junction and entry_wp.junction_id not in junction_ids:
            junctions.append(entry_wp.get_junction())
            junction_ids.append(entry_wp.junction_id)

    tls = []
    for junction in junctions:
        tls.extend(world.get_traffic_lights_in_junction(junction.id))

    for tl in tls:
        if tl.get_opendrive_id() == tl_id:
            return tl


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--id', required=True,
        help='OpenDrive ID of the traffic light to control')
    argparser.add_argument(
        '-gt', '--green-time', default=-1, type=int,
        help='Sets the green time of the traffic light')
    argparser.add_argument(
        '-yt', '--yellow-time', default=-1, type=int,
        help='Sets the yellow time of show the traffic light')
    argparser.add_argument(
        '-rt', '--red-time', default=-1, type=int,
        help='Sets the red time of the traffic light')
    argparser.add_argument(
        '-st', '--state', default="NONE",
        help='Sets the state time of the traffic light')
    argparser.add_argument(
        '-f', '--freeze', action="store_true",
        help='Freezes the traffic light')
    argparser.add_argument(
        '-rg', '--reset-group', action="store_true",
        help="Resets the traffic light's group")
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()

    traffic_light = get_traffic_light(world, args.id)
    if traffic_light is None:
        print(f"Couldn't find a traffic light with OpenDrive ID {args.id}")
        sys.exit()

    if args.green_time >= 0:
        print(f"Changing the green time to {args.green_time}")
        traffic_light.set_green_time(args.green_time)
    if args.yellow_time >= 0:
        print(f"Changing the yellow time to {args.yellow_time}")
        traffic_light.set_yellow_time(args.yellow_time)
    if args.red_time >= 0:
        print(f"Changing the red time to {args.red_time}")
        traffic_light.set_red_time(args.red_time)

    if args.state != 'NONE':
        try:
            state = TRAFFIC_LIGHT_STATES[args.state]
            print(f"Changing the state to {state}")
            traffic_light.set_state(state)
        except KeyError:
            print(f"Light State '{args.state}' doesn't exist. The possible ones are:")
            print(list(TRAFFIC_LIGHT_STATES.keys()))
            sys.exit()

    if args.reset_group:
        print("Resetting the traffic light group")
        traffic_light.reset_group()

    if args.freeze:
        print("Freezing all traffic lights")
        traffic_light.freeze(False)

if __name__ == '__main__':
    main()
