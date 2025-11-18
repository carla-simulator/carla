#!/usr/bin/env python

# Copyright (c) 2024 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Example script for collecting driving data using BehaviorAgent with Bench2Drive format.

This script demonstrates:
- Spawning a vehicle with BehaviorAgent
- Attaching sensors for 360° coverage (RGB, depth, semantic, instance, LiDAR)
- Collecting data in Bench2Drive format
- Driving with different behavior profiles (cautious, normal, aggressive)
"""

import argparse
import random
import time
import sys
import os

# Add PythonAPI to path
try:
    sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
except IndexError:
    pass

import carla

from agents.navigation.behavior_agent import BehaviorAgent
from agents.tools.sensor_manager import SensorManager
from agents.tools.data_collector import DataCollector


def main():
    """Main function for data collection."""
    argparser = argparse.ArgumentParser(description='Behavior Agent Data Collection')
    argparser.add_argument('--host', default='127.0.0.1', help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument('-p', '--port', default=2000, type=int, help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--tm-port', default=8000, type=int, help='Traffic Manager port (default: 8000)')
    argparser.add_argument('--sync', action='store_true', help='Synchronous mode execution')
    argparser.add_argument('--behavior', default='normal', choices=['cautious', 'normal', 'aggressive'],
                           help='Driving behavior (default: normal)')
    argparser.add_argument('--town', default='Town01', help='Town/map to load (default: Town01)')
    argparser.add_argument('--weather', default=0, type=int, help='Weather preset ID (default: 0)')
    argparser.add_argument('--route-id', default=1, type=int, help='Route ID for naming (default: 1)')
    argparser.add_argument('--output-dir', default='./b2d_dataset', help='Output directory (default: ./b2d_dataset)')
    argparser.add_argument('--max-frames', default=1000, type=int, help='Maximum frames to collect (default: 1000)')
    argparser.add_argument('--num-vehicles', default=30, type=int, help='Number of vehicles to spawn (default: 30)')
    argparser.add_argument('--num-pedestrians', default=10, type=int, help='Number of pedestrians to spawn (default: 10)')
    args = argparser.parse_args()

    actor_list = []
    sensor_manager = None
    data_collector = None

    try:
        # Connect to CARLA
        client = carla.Client(args.host, args.port)
        client.set_timeout(10.0)

        # Load the desired map
        world = client.load_world(args.town)
        print(f"Loaded map: {args.town}")

        # Set up traffic manager
        traffic_manager = client.get_trafficmanager(args.tm_port)
        traffic_manager.set_global_distance_to_leading_vehicle(2.5)
        traffic_manager.set_hybrid_physics_mode(True)
        traffic_manager.set_hybrid_physics_radius(70.0)

        # Set synchronous mode
        if args.sync:
            original_settings = world.get_settings()
            settings = world.get_settings()
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)
            traffic_manager.set_synchronous_mode(True)

        # Set weather
        weather_presets = [
            carla.WeatherParameters.ClearNoon,
            carla.WeatherParameters.CloudyNoon,
            carla.WeatherParameters.WetNoon,
            carla.WeatherParameters.WetCloudyNoon,
            carla.WeatherParameters.MidRainyNoon,
            carla.WeatherParameters.HardRainNoon,
            carla.WeatherParameters.SoftRainNoon,
            carla.WeatherParameters.ClearSunset,
            carla.WeatherParameters.CloudySunset,
            carla.WeatherParameters.WetSunset,
            carla.WeatherParameters.WetCloudySunset,
            carla.WeatherParameters.MidRainSunset,
            carla.WeatherParameters.HardRainSunset,
            carla.WeatherParameters.SoftRainSunset,
        ]
        if args.weather < len(weather_presets):
            world.set_weather(weather_presets[args.weather])
            print(f"Set weather preset: {args.weather}")
        else:
            print(f"Warning: Weather preset {args.weather} not available, using default")

        # Get spawn points
        spawn_points = world.get_map().get_spawn_points()
        if not spawn_points:
            print("Error: No spawn points available in this map")
            return

        # Spawn ego vehicle
        blueprint_library = world.get_blueprint_library()
        ego_bp = blueprint_library.filter('vehicle.tesla.model3')[0]
        ego_bp.set_attribute('role_name', 'hero')

        spawn_point = random.choice(spawn_points)
        ego_vehicle = world.spawn_actor(ego_bp, spawn_point)
        actor_list.append(ego_vehicle)
        print(f"Spawned ego vehicle at {spawn_point.location}")

        # Wait a tick to ensure vehicle is properly spawned
        if args.sync:
            world.tick()
        else:
            world.wait_for_tick()

        # Create BehaviorAgent
        agent = BehaviorAgent(ego_vehicle, behavior=args.behavior)
        print(f"Created BehaviorAgent with '{args.behavior}' behavior")

        # Set destination (random spawn point far from start)
        destination = random.choice(spawn_points).location
        agent.set_destination(destination)
        print(f"Set destination to {destination}")

        # Create sensor manager
        print("Creating sensor manager with Bench2Drive configuration...")
        sensor_manager = SensorManager(ego_vehicle, config_preset='bench2drive')
        print(f"Spawned {len(sensor_manager.get_sensor_list())} sensors")

        # Wait for sensors to initialize
        print("Waiting for sensor data...")
        sensor_manager.wait_for_data(timeout=2.0)

        # Create data collector
        print("Creating data collector...")
        data_collector = DataCollector(
            sensor_manager=sensor_manager,
            vehicle=ego_vehicle,
            root_dir=args.output_dir,
            scenario_type='GeneralDriving',
            town=args.town,
            route_id=args.route_id,
            weather_id=args.weather,
            format='bench2drive'
        )

        # Spawn other vehicles for traffic
        print(f"Spawning {args.num_vehicles} traffic vehicles...")
        vehicles_list = spawn_surrounding_vehicles(client, world, blueprint_library, spawn_points,
                                                   args.num_vehicles, traffic_manager)
        actor_list.extend(vehicles_list)
        print(f"Spawned {len(vehicles_list)} traffic vehicles")

        # Start data collection
        data_collector.start_recording()
        print("Started data collection")
        print(f"Output directory: {data_collector.get_scenario_dir()}")
        print("-" * 60)

        # Main loop
        frame_count = 0
        start_time = time.time()

        while frame_count < args.max_frames:
            if args.sync:
                world.tick()

            # Run agent step
            if agent.done():
                print("Agent reached destination, setting new destination...")
                destination = random.choice(spawn_points).location
                agent.set_destination(destination)

            control = agent.run_step()
            ego_vehicle.apply_control(control)

            # Record frame
            data_collector.record_frame(control=control, waypoint_info=None)

            frame_count += 1

            # Print progress
            if frame_count % 100 == 0:
                elapsed = time.time() - start_time
                fps = frame_count / elapsed
                print(f"Frame {frame_count}/{args.max_frames} | FPS: {fps:.2f} | "
                      f"Speed: {ego_vehicle.get_velocity().length():.2f} m/s")

        # Stop recording
        data_collector.stop_recording()
        elapsed = time.time() - start_time
        print("-" * 60)
        print(f"Data collection completed!")
        print(f"Total frames: {frame_count}")
        print(f"Total time: {elapsed:.2f}s")
        print(f"Average FPS: {frame_count/elapsed:.2f}")
        print(f"Dataset saved to: {data_collector.get_scenario_dir()}")

    except KeyboardInterrupt:
        print("\nCancelled by user")

    finally:
        # Clean up
        print("\nCleaning up...")

        if data_collector and data_collector.is_recording():
            data_collector.stop_recording()

        if sensor_manager:
            sensor_manager.destroy()

        if args.sync:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)

        client.apply_batch([carla.command.DestroyActor(x) for x in actor_list])
        print("Done!")


def spawn_surrounding_vehicles(client, world, blueprint_library, spawn_points, num_vehicles, traffic_manager):
    """Spawn surrounding vehicles for traffic."""
    vehicles_list = []
    blueprints = blueprint_library.filter('vehicle.*')
    blueprints = [x for x in blueprints if int(x.get_attribute('number_of_wheels')) == 4]

    # Shuffle spawn points
    random.shuffle(spawn_points)

    batch = []
    for i, spawn_point in enumerate(spawn_points[:num_vehicles]):
        blueprint = random.choice(blueprints)
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)
        if blueprint.has_attribute('driver_id'):
            driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
            blueprint.set_attribute('driver_id', driver_id)

        blueprint.set_attribute('role_name', 'autopilot')
        batch.append(carla.command.SpawnActor(blueprint, spawn_point))

    # Apply batch spawn
    for response in client.apply_batch_sync(batch, True):
        if response.error:
            continue
        else:
            vehicles_list.append(response.actor_id)

    # Set vehicles to autopilot
    all_actors = world.get_actors(vehicles_list)
    for actor in all_actors:
        actor.set_autopilot(True, traffic_manager.get_port())

    return vehicles_list


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
