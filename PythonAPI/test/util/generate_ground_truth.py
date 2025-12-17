#!/usr/bin/env python

# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import numpy as np
from pathlib import Path

from agents.navigation.behavior_agent import BehaviorAgent  # pylint: disable=import-error
from agents.navigation.basic_agent import BasicAgent  # pylint: disable=import-error
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent  # pylint: disable=import-error

def main():
    from argparse import ArgumentParser
    print('Parsing command line')
    parser = ArgumentParser()
    parser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    parser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    parser.add_argument(
        '--res', metavar='WIDTHxHEIGHT', default='1280x720',
        help='window resolution (default: 1280x720)')
    parser.add_argument(
        '--filter', metavar='PATTERN', default='lincoln',
        help='actor filter (default: "lincoln")')
    parser.add_argument(
        '--sync', action='store_true',
        help='Activate synchronous mode execution')
    parser.add_argument(
        '--spawn-point', type=int, default=0,
        help='Vehicle spawn point index.')
    parser.add_argument(
        '--gamma', default=1.0, type=float,
        help='Gamma correction of the camera (default: 1.0)')
    parser.add_argument(
        '--sensor', type=str, default='sensor.camera.rgb',
        help='Sensor ID')
    parser.add_argument(
        '-a', '--agent', type=str, choices=['Behavior', 'Basic', 'Constant'], default='Behavior',
        help='select which agent to run')
    parser.add_argument(
        '-b', '--behavior', type=str, choices=["cautious", "normal", "aggressive"], default='normal',
        help='Choose one of the possible agent behaviors (default: normal)')
    parser.add_argument(
        '-s', '--seed', type=int, default=0xcdcdcdcd,
        help='select which agent to run')
    argv = parser.parse_args()
    try:
        print('Setting up client')
        underlying_prng = np.random.PCG64(argv.seed)
        prng = np.random.Generator(underlying_prng)
        client = carla.Client()
        client.set_timeout(10.0)
        world = client.get_world()
        traffic_manager = client.get_trafficmanager()
        map = world.get_map()
        settings = world.get_settings()
        blueprint_library = world.get_blueprint_library()
        sensor_bp = blueprint_library.find(argv.sensor)
        assert sensor_bp != None
        if argv.sync:
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)
            traffic_manager.set_synchronous_mode(True)
        vehicle_bp = blueprint_library.filter(argv.filter)
        assert len(vehicle_bp) != 0
        vehicle_bp = vehicle_bp[0]
        spawn_points = map.get_spawn_points()
        spawn_point_index = argv.spawn_point
        # Vehicle setup:
        vehicle = None
        while True:
            spawn_point = spawn_points[spawn_point_index]
            vehicle = world.try_spawn_actor(vehicle_bp, spawn_point)
            if vehicle != None:
                world.tick()
                break
            spawn_point_index += 1
            if spawn_point_index == 100:
                raise Exception('Failed to spawn vehicle after 100 retries.')
        if argv.sensor.startswith('sensor.camera'):
            width, height = (float(e) for e in argv.res.split('x'))
            print(f'Image resolution: {width} x {height}')
            sensor_bp.set_attribute('image_size_x', str(width))
            sensor_bp.set_attribute('image_size_y', str(height))
            if sensor_bp.has_attribute('gamma'):
                sensor_bp.set_attribute('gamma', str(argv.gamma))
            ppp = 'Default'
            if 'Town10HD_Opt' in map.name:
                ppp = 'Town10HD_Opt'
            sensor_bp.set_attribute('post_process_profile', ppp)
            print(f'Image gamma: {argv.gamma}')
        bound_x = 0.5 + vehicle.bounding_box.extent.x
        bound_y = 0.5 + vehicle.bounding_box.extent.y
        bound_z = 0.5 + vehicle.bounding_box.extent.z
        sensor = world.spawn_actor(
            sensor_bp,
            carla.Transform(carla.Location(x=+0.8*bound_x, y=+0.0*bound_y, z=1.3*bound_z)),
            attach_to=vehicle,
            attachment_type=carla.AttachmentType.Rigid)
        def SaveImage(image):
            image.convert(carla.ColorConverter.Raw)
            image.save_to_disk(f'_ground_truth/{image.frame}.png')
        sensor.listen(SaveImage)
        # Agent setup:
        agent = None
        if argv.agent == 'Basic':
            agent = BasicAgent(vehicle, 30)
            agent.follow_speed_limits(True)
        elif argv.agent == 'Constant':
            agent = ConstantVelocityAgent(vehicle, 30)
            ground_loc = world.world.ground_projection(vehicle.get_location(), 5)
            if ground_loc:
                world.player.set_location(ground_loc.location + carla.Location(z=0.01))
            agent.follow_speed_limits(True)
        elif argv.agent == 'Behavior':
            agent = BehaviorAgent(vehicle, behavior=argv.behavior)
        print(f'Vehicle {vehicle} spawned at {spawn_point}')

        def GetNextDestination():
            return prng.choice(spawn_points).location
        
        agent.set_destination(GetNextDestination())
        while True:
            if argv.sync:
                world.tick()
            else:
                world.wait_for_tick()
            if agent.done():
                agent.set_destination(GetNextDestination())
            control = agent.run_step()
            control.manual_gear_shift = False
            vehicle.apply_control(control)

    except Exception as e:
        print(f'Exception thrown: {e}')
    finally:
        if world != None:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)
            traffic_manager.set_synchronous_mode(True)

        def TryDestroy(target):
            if target == None:
                return
            target.destroy()
            
        TryDestroy(vehicle)
        TryDestroy(sensor)


if __name__ == '__main__':
    main()