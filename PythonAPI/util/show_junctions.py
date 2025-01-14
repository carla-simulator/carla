#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import carla

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
        '--show', default='100', type=float,
        help='Duration of the visualization of the junctions')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    tmap = world.get_map()

    topology = tmap.get_topology()
    junction_ids = []
    junctions = []
    for (entry_wp, _) in topology:
        if entry_wp.is_junction and entry_wp.junction_id not in junction_ids:
            junctions.append(entry_wp.get_junction())
            junction_ids.append(entry_wp.junction_id)

    color = carla.Color(0, 0, 128)
    offset = 0.5
    for junction in junctions:
        junction_bb = junction.bounding_box
        junction_bb.location.z += offset
        world.debug.draw_box(junction_bb, junction_bb.rotation, life_time=args.show, thickness=0.2, color=color)

        text = f"[{junction.id}]"
        text_location = carla.Location(x=junction_bb.location.x, y=junction_bb.location.y, z=junction_bb.location.z)
        world.debug.draw_string(text_location, text, life_time=args.show, color=color)

        wps = junction.get_waypoints(carla.LaneType.Driving)
        for (entry_wp, exit_wp) in wps:
            world.debug.draw_arrow(
                entry_wp.transform.location + carla.Location(z=offset), exit_wp.transform.location + carla.Location(z=offset),
                life_time=args.show, arrow_size=0.5, thickness=0.1, color=carla.Color(0, 128, 0)
            )

        wps = junction.get_waypoints(carla.LaneType.Sidewalk)
        for (entry_wp, exit_wp) in wps:
            world.debug.draw_arrow(
                entry_wp.transform.location + carla.Location(z=offset), exit_wp.transform.location + carla.Location(z=offset),
                life_time=args.show, arrow_size=0.5, thickness=0.1, color=carla.Color(128, 0, 0)
            )

        tls = world.get_traffic_lights_in_junction(junction.id)
        for tl in tls:
            tl_location = tl.get_location()
            world.debug.draw_arrow(
                text_location + carla.Location(z=offset), tl_location + carla.Location(z=offset),
                life_time=args.show, arrow_size=0.5, thickness=0.1, color=carla.Color(128, 0, 128)
            )

if __name__ == '__main__':
    main()
