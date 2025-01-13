#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Visualize the radar data"""

import argparse
import math
from numpy import random
import pygame

import carla


def radar_callback(radar_data, world):
    current_rot = radar_data.transform.rotation
    for detect in radar_data:
        azi = math.degrees(detect.azimuth)
        alt = math.degrees(detect.altitude)
        fw_vec = carla.Vector3D(x=detect.depth - 0.25)
        carla.Transform(
            carla.Location(),
            carla.Rotation(
                pitch=current_rot.pitch + alt,
                yaw=current_rot.yaw + azi,
                roll=current_rot.roll)
            ).transform(fw_vec)

        norm_velocity = detect.velocity / 7.5 # range [-1, 1]
        r = int(max(0.0, min(1.0, 1.0 - norm_velocity)) * 255.0)
        g = int(max(0.0, min(1.0, 1.0 - abs(norm_velocity))) * 255.0)
        b = int(abs(max(- 1.0, min(0.0, - 1.0 - norm_velocity))) * 255.0)
        world.debug.draw_point(radar_data.transform.location + fw_vec, size=0.1, life_time=0.06, color=carla.Color(r, g, b))


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
        '-hfov', '--horizontal-fov', default='35',
        help="RADAR's horizontal fov")
    argparser.add_argument(
        '-vfov', '--vertical-fov', default='20',
        help="RADAR's vertical fov")
    argparser.add_argument(
        '-pps', '--points', default='1500',
        help="RADAR's points per second")
    argparser.add_argument(
        '-r', '--range', default='100',
        help="RADAR's range")
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    tmap = world.get_map()

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    blueprint_library = world.get_blueprint_library()
    spawn_bp = blueprint_library.filter('lincoln')[0]

    spawn_points = tmap.get_spawn_points()
    spawn_transform = random.choice(spawn_points)

    vehicle = world.spawn_actor(spawn_bp, spawn_transform)

    world.tick()
    vehicle.set_autopilot(True)
    tm = client.get_trafficmanager()
    tm.ignore_lights_percentage(vehicle, 100)
    tm.ignore_signs_percentage(vehicle, 100)

    radar_bp = blueprint_library.find('sensor.other.radar')
    radar_bp.set_attribute('horizontal_fov', args.horizontal_fov)
    radar_bp.set_attribute('vertical_fov',  args.vertical_fov)
    radar_bp.set_attribute('points_per_second',  args.points)
    radar_bp.set_attribute('range',  args.range)

    radar = world.spawn_actor(
        radar_bp, carla.Transform(carla.Location(x=3, z=2), carla.Rotation(pitch=5)), attach_to=vehicle)
    radar.listen(lambda radar_data: radar_callback(radar_data, world))

    world.tick()
    clock = pygame.time.Clock()

    spectator = world.get_spectator()
    spec_offset_x = -5
    spec_offset_z = 3
    spec_offset_pitch = -15

    try:
        while True:
            clock.tick_busy_loop(20)
            world.tick()

            vehicle_tran = vehicle.get_transform()
            yaw = vehicle_tran.rotation.yaw
            spectator_l = vehicle_tran.location + carla.Location(
                spec_offset_x * math.cos(math.radians(yaw)),
                spec_offset_x * math.sin(math.radians(yaw)),
                spec_offset_z,
            )
            spectator_t = carla.Transform(spectator_l, carla.Rotation(pitch=spec_offset_pitch, yaw=yaw))
            spectator.set_transform(spectator_t)


    except KeyboardInterrupt:
        pass

    finally:
        radar.stop()
        radar.destroy()
        vehicle.destroy()

        settings = world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        world.apply_settings(settings)

if __name__ == '__main__':
    main()
