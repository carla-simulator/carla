#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

import argparse
import sys

import carla

def get_traffic_light_bbs(traffic_light):
    base_transform = traffic_light.get_transform()

    def to_world(local_box):
        world_box = carla.BoundingBox(
            base_transform.transform(local_box.location), local_box.extent)
        world_box.rotation = (carla.Quaternion(base_transform.rotation) *
                              carla.Quaternion(local_box.rotation)).rotator()
        return world_box

    return to_world(traffic_light.bounding_box), to_world(traffic_light.trigger_volume)


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

    tls = []
    for junction in junctions:
        tls.extend(world.get_traffic_lights_in_junction(junction.id))

    for tl in tls:
        location = tl.get_location()
        bounding_box, trigger_volume = get_traffic_light_bbs(tl)

        bounding_box.location.z += offset
        trigger_volume.location.z += offset
        world.debug.draw_box(bounding_box, bounding_box.rotation, life_time=args.show, thickness=0.1, color=carla.Color(0, 0, 200))
        world.debug.draw_box(trigger_volume, trigger_volume.rotation, life_time=args.show, thickness=0.1, color=carla.Color(0, 200, 0))

        text = f"[{tl.get_pole_index()}] - [{tl.get_opendrive_id()}]"
        world.debug.draw_string(location, text, life_time=args.show, color=color)

        light_boxes = tl.get_light_boxes()
        for lb in light_boxes:
            lb.location.z += offset
            world.debug.draw_box(lb, lb.rotation, life_time=args.show, thickness=0.1, color=carla.Color(200, 0, 0))

        for wp in tl.get_affected_lane_waypoints():
            world.debug.draw_point(wp.transform.location, life_time=args.show, size=0.3, color=carla.Color(200, 0, 200))
            world.debug.draw_arrow(
                location + carla.Location(z=offset), wp.transform.location + carla.Location(z=offset),
                life_time=args.show, arrow_size=0.5, thickness=0.1, color=carla.Color(200, 0, 200)
            )

        for wp in tl.get_stop_waypoints():
            world.debug.draw_point(wp.transform.location, life_time=args.show, size=0.3, color=carla.Color(0, 200, 200))
            world.debug.draw_arrow(
                location + carla.Location(z=offset), wp.transform.location + carla.Location(z=offset),
                life_time=args.show, arrow_size=0.5, thickness=0.1, color=carla.Color(0, 200, 200)
            )

    print("Showing all the static information available for the traffic light. This includes:")
    print("- Bounding box (blue box)")
    print("- Trigger volume (green box)")
    print("- Light boxes (Red boxes)")
    print("- Affected waypoints (Pink arrows and points)")
    print("- Stop waypoints (Teal arrows and points)")
    print("- Pole index and OpenDrive ID (Dark blue text)")


if __name__ == '__main__':
    main()
