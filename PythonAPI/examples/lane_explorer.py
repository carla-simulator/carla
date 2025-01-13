#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Sequentially visualize the opendrive points"""

import carla

import argparse
import random
import time

red = carla.Color(255, 0, 0)
green = carla.Color(0, 255, 0)
blue = carla.Color(47, 210, 231)
cyan = carla.Color(0, 255, 255)
yellow = carla.Color(255, 255, 0)
orange = carla.Color(255, 162, 0)
white = carla.Color(255, 255, 255)

trail_life_time = 10
waypoint_separation = 4


def draw_transform(debug, trans, col=carla.Color(255, 0, 0), lt=-1):
    debug.draw_arrow(
        trans.location, trans.location + trans.get_forward_vector(),
        thickness=0.05, arrow_size=0.1, color=col, life_time=lt)


def draw_waypoint_union(debug, w0, w1, color=carla.Color(255, 0, 0), lt=5):
    debug.draw_line(
        w0.transform.location + carla.Location(z=0.25),
        w1.transform.location + carla.Location(z=0.25),
        thickness=0.1, color=color, life_time=lt, persistent_lines=False)
    debug.draw_point(w1.transform.location + carla.Location(z=0.25), 0.1, color, lt, False)


def draw_waypoint_info(debug, w, lt=5):
    w_loc = w.transform.location
    debug.draw_string(w_loc + carla.Location(z=0.5), "lane: " + str(w.lane_id), False, yellow, lt)
    debug.draw_string(w_loc + carla.Location(z=1.0), "road: " + str(w.road_id), False, blue, lt)
    debug.draw_string(w_loc + carla.Location(z=-.5), str(w.lane_change), False, red, lt)

def draw_junction(debug, junction, l_time=10):
    """Draws a junction bounding box and the initial and final waypoint of every lane."""
    # draw bounding box
    box = junction.bounding_box
    point1 = box.location + carla.Location(x=box.extent.x, y=box.extent.y, z=2)
    point2 = box.location + carla.Location(x=-box.extent.x, y=box.extent.y, z=2)
    point3 = box.location + carla.Location(x=-box.extent.x, y=-box.extent.y, z=2)
    point4 = box.location + carla.Location(x=box.extent.x, y=-box.extent.y, z=2)
    debug.draw_line(
        point1, point2,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    debug.draw_line(
        point2, point3,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    debug.draw_line(
        point3, point4,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    debug.draw_line(
        point4, point1,
        thickness=0.1, color=orange, life_time=l_time, persistent_lines=False)
    # draw junction pairs (begin-end) of every lane
    junction_w = junction.get_waypoints(carla.LaneType.Any)
    for pair_w in junction_w:
        draw_transform(debug, pair_w[0].transform, orange, l_time)
        debug.draw_point(
            pair_w[0].transform.location + carla.Location(z=0.75), 0.1, orange, l_time, False)
        draw_transform(debug, pair_w[1].transform, orange, l_time)
        debug.draw_point(
            pair_w[1].transform.location + carla.Location(z=0.75), 0.1, orange, l_time, False)
        debug.draw_line(
            pair_w[0].transform.location + carla.Location(z=0.75),
            pair_w[1].transform.location + carla.Location(z=0.75), 0.1, white, l_time, False)

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-i', '--info', action='store_true',
        help='Show text information')
    argparser.add_argument(
        '-x', default=0.0, type=float,
        help='X start position (default: 0.0)')
    argparser.add_argument(
        '-y', default=0.0, type=float,
        help='Y start position (default: 0.0)')
    argparser.add_argument(
        '-z', default=0.0, type=float,
        help='Z start position (default: 0.0)')
    argparser.add_argument(
        '-s', '--seed', metavar='S', default=None, type=int,
        help='Seed for the random path (default: None)')
    argparser.add_argument(
        '-t', '--tick-time', metavar='T', default=0.2, type=float,
        help='Tick time between updates (forward velocity) (default: 0.2)')
    args = argparser.parse_args()

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        world = client.get_world()
        m = world.get_map()
        debug = world.debug

        if args.seed:
            random.seed(args.seed)

        loc = carla.Location(args.x, args.y, args.z)
        current_w = m.get_waypoint(loc)

        while True:
            # list of potential next waypoints
            potential_w = list(current_w.next(waypoint_separation))

            # check for available right driving lanes
            if current_w.lane_change & carla.LaneChange.Right:
                right_w = current_w.get_right_lane()
                if right_w and right_w.lane_type == carla.LaneType.Driving:
                    potential_w += list(right_w.next(waypoint_separation))

            # check for available left driving lanes
            if current_w.lane_change & carla.LaneChange.Left:
                left_w = current_w.get_left_lane()
                if left_w and left_w.lane_type == carla.LaneType.Driving:
                    potential_w += list(left_w.next(waypoint_separation))

            # choose a random waypoint to be the next
            next_w = random.choice(potential_w)
            potential_w.remove(next_w)

            # Render some nice information, notice that you can't see the strings if you are using an editor camera
            if args.info:
                draw_waypoint_info(debug, current_w, trail_life_time)
            draw_waypoint_union(debug, current_w, next_w, cyan if current_w.is_junction else green, trail_life_time)
            draw_transform(debug, current_w.transform, white, trail_life_time)

            # print the remaining waypoints
            for p in potential_w:
                draw_waypoint_union(debug, current_w, p, red, trail_life_time)
                draw_transform(debug, p.transform, white, trail_life_time)

            # draw all junction waypoints and bounding box
            if next_w.is_junction:
                junction = next_w.get_junction()
                draw_junction(debug, junction, trail_life_time)

            # update the current waypoint and sleep for some time
            current_w = next_w
            time.sleep(args.tick_time)

    finally:
        pass


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nExit by user.')
    finally:
        print('\nExit.')
