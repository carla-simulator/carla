#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Spawn NPCs into the simulation using the Traffic Manager interface"""

import time
import random
import glob
import argparse
import logging
import sys
import os
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
import carla

def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-n', '--number-of-vehicles',
        metavar='N',
        default=50,
        type=int,
        help='number of vehicles (default: 10)')
    argparser.add_argument(
        '--safe',
        action='store_true',
        help='avoid spawning vehicles prone to accidents')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='vehicles filter (default: "vehicle.*")')
    args = argparser.parse_args()

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    vehicle_list = []
    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)

    try:
        traffic_manager = None
        world = client.get_world()
        blueprints = world.get_blueprint_library().filter(args.filter)
        debug = world.debug
        traffic_lights = world.get_actors().filter('*traffic_light*')
        list_of_all_groups = []
        all_groups_without_first_elem = []
        list_of_ids = []

        for tl in traffic_lights:
            if tl.id not in list_of_ids:
                tl_group = tl.get_group_traffic_lights()
                for tl_g in tl_group:
                    list_of_ids.append(tl_g.id)
                if tl_group not in list_of_all_groups:
                    list_of_all_groups.append(tl_group)
                    all_groups_without_first_elem.extend(tl_group[1:len(tl_group)])

        def show_tl_count_down(duration=30, indefinite=False):
            end_time = time.time() + duration
            color_dict = {
                'Red':carla.Color(255, 0, 0),
                'Yellow':carla.Color(255, 255, 0),
                'Green':carla.Color(0, 255, 0)
            }
            while time.time() < end_time or indefinite:
                for tl_group in list_of_all_groups:
                    for tl in tl_group:
                        debug.draw_string(tl.get_location() + carla.Location(0, 0, 5),
                                        str(tl.get_elapsed_time())[:4], False,
                                        color_dict[tl.get_state().name], 0.01)
                    time.sleep(0.002)

        if args.safe:
            blueprints = [x for x in blueprints if int(x.get_attribute('number_of_wheels')) == 4]
            blueprints = [x for x in blueprints if not x.id.endswith('isetta')]
            blueprints = [x for x in blueprints if not x.id.endswith('carlacola')]

        spawn_points = world.get_map().get_spawn_points()
        number_of_spawn_points = len(spawn_points)

        if args.number_of_vehicles < number_of_spawn_points:
            random.shuffle(spawn_points)
        elif args.number_of_vehicles > number_of_spawn_points:
            msg = 'Requested %d vehicles, but could only find %d spawn points'
            logging.warning(msg, args.number_of_vehicles, number_of_spawn_points)
            args.number_of_vehicles = number_of_spawn_points

        # --------------
        # Spawn vehicles
        # --------------

        for n, transform in enumerate(spawn_points):
            if n >= args.number_of_vehicles:
                break
            blueprint = random.choice(blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            blueprint.set_attribute('role_name', 'autopilot')
            vehicle = world.try_spawn_actor(blueprint, transform)
            vehicle_list.append(vehicle)

        print('Spawned %d vehicles, press Ctrl+C to exit.\n' % (len(vehicle_list)))

        traffic_manager = carla.GetTrafficManager(client)
        time.sleep(1)

        vehicle_vec = carla.TM_ActorList()
        vehicle_vec.extend(vehicle_list)

        traffic_manager.register_vehicles(vehicle_vec)

        # Arbitrarily setting some vehicle velocities to 15kmph
        time.sleep(1)
        for v in vehicle_list:
            if (v.id % 2 == 0):
                traffic_manager.set_vehicle_target_velocity(v, 15.0/3.6)
                traffic_manager.set_distance_to_leading_vehicle(v, 3)
            else:
                traffic_manager.set_distance_to_leading_vehicle(v, 6)
                pass

        time.sleep(10)
        print("Time to lane change!")

        for v in vehicle_list:
            traffic_manager.force_lane_change(v, True)

        show_tl_count_down(10)
        print("Time to reset the lights!")
        start = time.time()
        traffic_manager.reset_traffic_lights()
        print("Total time needed: " + str(time.time() - start))
        show_tl_count_down(15)

        while True:
            time.sleep(1)

    except Exception as e:

        print(e)
        print("Stopping TrafficManager!")

    finally:
        if traffic_manager:
            del traffic_manager

        print('Destroying %d vehicles.\n' % len(vehicle_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in vehicle_list])


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('Done.\n')
