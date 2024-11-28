#!/usr/bin/env python

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Example script to generate traffic in the simulation"""

import glob
import os
import sys
import time
import math

import carla

from carla import VehicleLightState as vls

import argparse
import logging
from numpy import random

def get_actor_blueprints(world, filter, generation):
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed
    # and therefore, we ignore the generation
    if len(bps) == 1:
        return bps

    try:
        int_generation = int(generation)
        # Check if generation is in available generations
        if int_generation in [1, 2, 3, 4]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []

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
        default=30,
        type=int,
        help='Number of vehicles (default: 30)')
    argparser.add_argument(
        '-w', '--number-of-walkers',
        metavar='W',
        default=10,
        type=int,
        help='Number of walkers (default: 10)')
    argparser.add_argument(
        '--safe',
        action='store_true',
        help='Avoid spawning vehicles prone to accidents')
    argparser.add_argument(
        '--filterv',
        metavar='PATTERN',
        default='vehicle.*',
        help='Filter vehicle model (default: "vehicle.*")')
    argparser.add_argument(
        '--generationv',
        metavar='G',
        default='3',
        help='restrict to certain vehicle generation (values: "1","2","All" - default: "All")')
    argparser.add_argument(
        '--filterw',
        metavar='PATTERN',
        default='walker.pedestrian.*',
        help='Filter pedestrian type (default: "walker.pedestrian.*")')
    argparser.add_argument(
        '--generationw',
        metavar='G',
        default='2',
        help='restrict to certain pedestrian generation (values: "1","2","All" - default: "2")')
    argparser.add_argument(
        '--tm-port',
        metavar='P',
        default=8000,
        type=int,
        help='Port to communicate with TM (default: 8000)')
    argparser.add_argument(
        '--asynch',
        action='store_true',
        help='Activate asynchronous mode execution')
    argparser.add_argument(
        '--hybrid',
        action='store_true',
        help='Activate hybrid mode for Traffic Manager')
    argparser.add_argument(
        '-s', '--seed',
        metavar='S',
        type=int,
        help='Set random device seed and deterministic mode for Traffic Manager')
    argparser.add_argument(
        '--seedw',
        metavar='S',
        default=0,
        type=int,
        help='Set the seed for pedestrians module')
    argparser.add_argument(
        '--car-lights-on',
        action='store_true',
        default=False,
        help='Enable automatic car light management')
    argparser.add_argument(
        '--hero',
        action='store_true',
        default=False,
        help='Set one of the vehicles as hero')
    argparser.add_argument(
        '--respawn',
        action='store_true',
        default=False,
        help='Automatically respawn dormant vehicles (only in large maps)')
    argparser.add_argument(
        '--no-rendering',
        action='store_true',
        default=False,
        help='Activate no rendering mode')

    args = argparser.parse_args()

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    vehicles_list = []
    walkers_list = []
    all_id = []
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    synchronous_master = False
    random.seed(args.seed if args.seed is not None else int(time.time()))

    try:
        world = client.get_world()

        traffic_manager = client.get_trafficmanager(args.tm_port)
        traffic_manager.set_global_distance_to_leading_vehicle(2.5)
        traffic_manager.global_percentage_speed_difference(40.0)
        if args.respawn:
            traffic_manager.set_respawn_dormant_vehicles(True)
        if args.hybrid:
            traffic_manager.set_hybrid_physics_mode(True)
            traffic_manager.set_hybrid_physics_radius(70.0)
        if args.seed is not None:
            traffic_manager.set_random_device_seed(args.seed)

        settings = world.get_settings()

        FPS = 30
        traffic_manager.set_synchronous_mode(True)
        if not settings.synchronous_mode:
            synchronous_master = True
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 1 / FPS
        else:
            synchronous_master = False

        if args.no_rendering:
            settings.no_rendering_mode = True
        world.apply_settings(settings)

        blueprints = get_actor_blueprints(world, args.filterv, args.generationv)
        if not blueprints:
            raise ValueError("Couldn't find any vehicles with the specified filters")
        blueprintsWalkers = get_actor_blueprints(world, args.filterw, args.generationw)
        if not blueprintsWalkers:
            raise ValueError("Couldn't find any walkers with the specified filters")

        if args.safe:
            blueprints = [x for x in blueprints if x.get_attribute('base_type') == 'car']

        blueprints = sorted(blueprints, key=lambda bp: bp.id)

        tmap = world.get_map()

        ############### CHANGES HERE ###############
        duration = 150       # s
        pedestrian_amount = 20
        #ego_location = carla.Location( 74, -67, 0)
        #ego_location = carla.Location(-48, 85, 0)
        ego_location = carla.Location( -103, -5, 0)
        #ego_location = carla.Location( -52, -42, 0)
        #ego_location = carla.Location( -88, 28, 0)
        #ego_location = carla.Location( -60, 140, 0)
        #ego_location = carla.Location( -41.5, 180, 0)
        #ego_location = carla.Location( -45, 103, 0)

        spectator_offset_x = 1.5
        spectator_offset_z = 1.3
        spectator_offset_pitch = -3

        pedestrian_dist = 50
        pedestrian_point = 100

        side_dist = 20
        opposite_dist = 60
        opposite_interval = 20
        opposite_amount = 8
        same_dist = 40
        same_interval = 20
        #############################################

        spawn_transforms = []
        ego_wp = tmap.get_waypoint(ego_location)
        ego_lane_wps = [ego_wp]
        opposite_lane_wps = []
        spawn_transforms.append(ego_wp.transform)

        # Vehicle to the side
        right_wp = ego_wp.get_right_lane()
        if right_wp is not None and right_wp.lane_type == carla.LaneType.Driving:
            print("Found a right lane, spawnign a vehicle")
            dist = 0
            while dist == 0:
                dist = random.randint(-side_dist, side_dist)
            right_wp = right_wp.next(dist)[0] if dist > 0 else right_wp.previous(-dist)[0]
            spawn_transforms.append(right_wp.transform)
            ego_lane_wps.append(right_wp)

        left_wp = ego_wp.get_left_lane()
        if left_wp is not None and left_wp.lane_id * ego_wp.lane_id > 0:  # Same direction
            print("Found a left lane, spawning a vehicle")
            dist = 0
            while dist == 0:
                dist = random.randint(-side_dist, side_dist)
            left_wp = left_wp.next(dist)[0] if dist > 0 else left_wp.previous(-dist)[0]
            spawn_transforms.append(left_wp.transform)
            ego_lane_wps.append(left_wp)

            opposite_lane_wp = left_wp.get_left_lane()

        elif left_wp is not None:
            opposite_lane_wp = left_wp

        if opposite_lane_wp is not None:
            opposite_lane_wps.append(opposite_lane_wp)
            opposite_lane_wp_2 = opposite_lane_wp.get_right_lane()
            if opposite_lane_wp_2 is not None and right_wp.lane_type == carla.LaneType.Driving:
                opposite_lane_wps.append(opposite_lane_wp_2)

        for spawn_wp in opposite_lane_wps:
            print("Found an opposite lane, spawning a vehicle")

            for _ in range(opposite_amount):
                dist = 0
                while dist == 0:
                    dist = opposite_dist + random.randint(-opposite_interval, opposite_interval)
                spawn_wp = spawn_wp.previous(dist)[0]
                spawn_transforms.append(spawn_wp.transform)

        for wp in ego_lane_wps:
            print("Found an ego lane, spawning a vehicle")
            dist = 0
            while dist == 0:
                dist = same_dist + random.randint(-same_interval, same_interval)
            spawn_wp = wp.previous(dist)[0]
            spawn_transforms.append(spawn_wp.transform)

            dist = 0
            while dist == 0:
                dist = same_dist + random.randint(-same_interval, same_interval)
            spawn_wp = wp.next(dist)[0]
            spawn_transforms.append(spawn_wp.transform)
            dist = 0
            while dist == 0:
                dist = same_dist + random.randint(-same_interval, same_interval)
            spawn_wp = wp.next(dist)[0]
            spawn_transforms.append(spawn_wp.transform)

        batch = []
        hero = True
        for spawn_transform in spawn_transforms:

            spawn_transform.location.z += 0.5
            blueprint = random.choice(blueprints)
            if hero:
                blueprint = world.get_blueprint_library().filter('*mkz*')[0]
                blueprint.set_attribute('role_name', 'hero')
                hero = False
            else:
                blueprint.set_attribute('role_name', 'autopilot')

            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)

            batch.append(carla.command.SpawnActor(blueprint, spawn_transform)
                .then(carla.command.SetAutopilot(carla.command.FutureActor, True, traffic_manager.get_port())))

        for response in client.apply_batch_sync(batch, synchronous_master):
            if response.error:
                logging.error(response.error)
            else:
                vehicles_list.append(response.actor_id)

        if args.car_lights_on:
            all_vehicle_actors = world.get_actors(vehicles_list)
            for actor in all_vehicle_actors:
                traffic_manager.update_vehicle_lights(actor, True)

        # Get hero vehicle
        hero_v = None
        possible_vehicles = world.get_actors().filter('vehicle.*')
        for vehicle in possible_vehicles:
            if vehicle.attributes['role_name'] == 'hero':
                hero_v = vehicle
                break

        spawn_points = []
        hero_start_point = ego_wp.transform.location
        hero_end_point = ego_wp.next(pedestrian_point)[0].transform.location
        for i in range(pedestrian_amount):
            spawn_point = carla.Transform()
            loc = None
            while loc is None:
                loc = world.get_random_location_from_navigation()
                if loc.distance(hero_start_point) < pedestrian_dist or loc.distance(hero_end_point) < pedestrian_dist:
                    spawn_point.location = loc
                    spawn_point.location.z += 2
                    spawn_points.append(spawn_point)
                else:
                    loc = None

        batch = []
        walker_speed = []
        for spawn_point in spawn_points:
            walker_bp = random.choice(blueprintsWalkers)
            if walker_bp.has_attribute('is_invincible'):
                walker_bp.set_attribute('is_invincible', 'false')
            if walker_bp.has_attribute('speed'):
                walker_speed.append(walker_bp.get_attribute('speed').recommended_values[1])
            else:
                print("Walker has no speed")
                walker_speed.append(0.0)
            batch.append(carla.command.SpawnActor(walker_bp, spawn_point))
        results = client.apply_batch_sync(batch, True)
        walker_speed2 = []
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list.append({"id": results[i].actor_id})
                walker_speed2.append(walker_speed[i])
        walker_speed = walker_speed2

        batch = []
        walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
        for i in range(len(walkers_list)):
            batch.append(carla.command.SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
        results = client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list[i]["con"] = results[i].actor_id

        for i in range(len(walkers_list)):
            all_id.append(walkers_list[i]["con"])
            all_id.append(walkers_list[i]["id"])
        all_actors = world.get_actors(all_id)

        if args.asynch or not synchronous_master:
            world.wait_for_tick()
        else:
            world.tick()

        world.set_pedestrians_cross_factor(0.0)
        for i in range(0, len(all_id), 2):
            all_actors[i].start()
            all_actors[i].go_to_location(world.get_random_location_from_navigation())
            all_actors[i].set_max_speed(float(walker_speed[int(i/2)]))

        print('spawned %d vehicles and %d walkers, press Ctrl+C to exit.' % (len(vehicles_list), len(walkers_list)))

        for _ in range(duration * FPS):
            if not args.asynch and synchronous_master:
                world.tick()
            else:
                world.wait_for_tick()

            hero_t = hero_v.get_transform()
            hero_w = hero_t.get_forward_vector()

            yaw = hero_t.rotation.yaw
            spectator_l = hero_t.location + carla.Location(
                spectator_offset_x * math.cos(math.radians(yaw)),
                spectator_offset_x * math.sin(math.radians(yaw)),
                spectator_offset_z,
            )
            spectator_t = carla.Transform(spectator_l, hero_t.rotation)
            spectator_t.rotation.pitch -= spectator_offset_pitch
            world.get_spectator().set_transform(spectator_t)

    finally:

        if not args.asynch and synchronous_master:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.no_rendering_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)

        print('\ndestroying %d vehicles' % len(vehicles_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

        # stop walker controllers (list is [controller, actor, controller, actor ...])
        for i in range(0, len(all_id), 2):
            all_actors[i].stop()

        print('\ndestroying %d walkers' % len(walkers_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in all_id])

        time.sleep(0.5)

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
