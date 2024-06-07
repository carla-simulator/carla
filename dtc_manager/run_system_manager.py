#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

import os
import argparse
import yaml
import logging
import carla
import rclpy
import threading
from rclpy.node import Node
from std_msgs.msg import Bool

import time

from optparse import OptionParser
import inspect

# import unreal

python_file_path = os.path.realpath(__file__)
python_file_path = python_file_path.replace('run_system_manager.py', '')

class SimulationStatusNode(Node):

    def __init__(self):
        super().__init__("carla_simulation_status")
        self._status = False
        self.publisher = self.create_publisher(Bool, "/simulation_ready", 10)
        self._timer = self.create_timer(0.1, self.publish_status)

    def publish_status(self):
        msg = Bool()
        msg.data = self._status
        self.publisher.publish(msg)
    
    def set_status(self, status):
        self._status = status


def _setup_vehicle_actors(world):
    actors = []

    try:
        # vehicle settings, static for P1
        vehicle_type = "WaypointVehicle"
        vehicle_id   = "hero"

        logging.debug(" Spawning vehicle: %s", vehicle_type)

        bp_library = world.get_blueprint_library()
        # print(bp_library)
        bp = bp_library.filter(vehicle_type)[0]
        #bp.set_attribute("role_name", vehicle_id)
        #bp.set_attribute("ros_name",  vehicle_id) 


        transform = carla.Transform(carla.Location(x=0, y=0, z=0), carla.Rotation(yaw=180))
        vehicle = world.spawn_actor(bp,transform)
        # vehicle = world.spawn_actor(bp,
        #                             world.get_map().get_spawn_points()[0],
        #                             attach_to=None)
        
        # method_list = [func for func in dir(vehicle) if callable(getattr(vehicle, func))]
        # print(method_list)

        
        functor_start_simulation_bp = bp_library.filter("FunctorStartSimulation")[0]
        # functor_start_simulation = world.spawn_actor(functor_start_simulation_bp, transform)

        functor_sent_casualties_bp = bp_library.filter("FunctorSendCasualties")[0]
        # method_list = [func for func in dir(functor_sent_casualties_bp) if callable(getattr(functor_sent_casualties_bp, func))]
        # print(method_list)
        print(functor_sent_casualties_bp.get_attribute('2'))
        attr = functor_sent_casualties_bp.get_attribute('1')
        functor_sent_casualties_bp.set_attribute('1', "BP_Casualty_02|4")
        # functor_sent_casualties_bp.set_attribute('60', "BP_Casualty_03|5")

        # attr.is_modifiable = True
        print(attr.is_modifiable)
        # functor_sent_casualties_bp.set_attribute('1', "foo")

        # print(functor_sent_casualties_bp.attributes)

        # functor_sent_casualties = world.spawn_actor(functor_sent_casualties_bp, transform)
        # print(functor_sent_casualties.attributes)

        functor_sent_waypoints_bp = bp_library.filter("FunctorSendWaypoints")[0]
        functor_sent_waypoints_bp.set_attribute('1', "3")
        functor_sent_waypoints_bp.set_attribute('2', "4")
        functor_sent_waypoints = world.spawn_actor(functor_sent_waypoints_bp, transform)


        # method_list = [func for func in dir(dummy) if callable(getattr(dummy, func))]
        # print()
        # print()
        # method_list = [func for func in dir(dummy_bp) if callable(getattr(dummy_bp, func))]

        # print(dummy.get_attribute('should_send_waypoint')())
        # print(dummy.attributes)
        # dummy.Hello()

        # u_world = unreal.UnrealEditorSubsystem().get_editor_world()
        # bp_asset = unreal.EditorUtilityLibrary.get_selected_assets()[0]
        # dummy.call_method("HelloWorld")



        # actors.append(dummy)

        actors.append(vehicle)

        # Create sensors based on this input:
        # This is done manually to ensure that all vehicle and sensor behavior in the simulation is static across runs
        logging.debug(' Creating LiDAR Sensor')
        sensor = bp_library.filter('sensor.lidar.ray_cast')[0]
        sensor.set_attribute("role_name",          'front_lidar')
        sensor.set_attribute("ros_name",           'front_lidar')
        sensor.set_attribute("range",              '50')
        sensor.set_attribute("channels",           '64')
        sensor.set_attribute("points_per_second",  '2621440')
        sensor.set_attribute("rotation_frequency", '20')
        sensor.set_attribute("upper_fov",          '22.5')
        sensor.set_attribute("lower_fov",          '-22.5')
        sensor.set_attribute("sensor_tick",        '0.1')
        sensor_spawn = carla.Transform(location=carla.Location(x=0, y=0, z=1.2), rotation=carla.Rotation(roll=0, pitch=-6.305, yaw=0))
        sensor_actor = world.spawn_actor(sensor, sensor_spawn, attach_to=vehicle)
        sensor_actor.enable_for_ros()
        actors.append(sensor_actor)

        logging.debug(' Creating RGB Sensor')
        sensor = bp_library.filter('sensor.camera.rgb')[0]
        sensor.set_attribute("role_name",    'front_rgb')
        sensor.set_attribute("ros_name",     'front_rgb')
        sensor.set_attribute("image_size_x", '1920')
        sensor.set_attribute("image_size_y", '1200')
        sensor.set_attribute("fov",          '90.0')
        sensor.set_attribute("sensor_tick",  '0.1')
        sensor_spawn = carla.Transform(location=carla.Location(x=0, y=0, z=1.2), rotation=carla.Rotation(roll=0, pitch=-20, yaw=0))
        sensor_actor = world.spawn_actor(sensor, sensor_spawn, attach_to=vehicle)
        sensor_actor.enable_for_ros()
        actors.append(sensor_actor)

        logging.debug(' Creating IR Sensor')
        sensor = bp_library.filter('sensor.camera.ir')[0]
        sensor.set_attribute("role_name",    'front_ir')
        sensor.set_attribute("ros_name",     'front_ir')
        sensor.set_attribute("image_size_x", '1920')
        sensor.set_attribute("image_size_y", '1200')
        sensor.set_attribute("fov",          '90.0')
        sensor.set_attribute("sensor_tick",  '0.1')
        sensor_spawn = carla.Transform(location=carla.Location(x=0, y=0, z=1.2), rotation=carla.Rotation(roll=0, pitch=-20, yaw=0))
        sensor_actor = world.spawn_actor(sensor, sensor_spawn, attach_to=vehicle)
        sensor_actor.enable_for_ros()
        actors.append(sensor_actor)

        logging.debug(' Creating GPS Sensor')
        sensor = bp_library.filter('sensor.other.gnss')[0]
        sensor.set_attribute("role_name",   'gnss')
        sensor.set_attribute("ros_name",    'gnss')
        sensor.set_attribute("sensor_tick", '0.1')
        sensor_spawn = carla.Transform(location=carla.Location(x=0, y=0, z=0), rotation=carla.Rotation(roll=0, pitch=0, yaw=0))
        sensor_actor = world.spawn_actor(sensor, sensor_spawn, attach_to=vehicle)
        sensor_actor.enable_for_ros()
        actors.append(sensor_actor)

        logging.debug(' Creating IMU Sensor')
        sensor = bp_library.filter('sensor.other.imu')[0]
        sensor.set_attribute("role_name",   'imu')
        sensor.set_attribute("ros_name",    'imu')
        sensor.set_attribute("sensor_tick", '0.1')
        sensor_spawn = carla.Transform(location=carla.Location(x=0, y=0, z=0), rotation=carla.Rotation(roll=0, pitch=0, yaw=0))
        sensor_actor = world.spawn_actor(sensor, sensor_spawn, attach_to=vehicle)
        sensor_actor.enable_for_ros()
        actors.append(sensor_actor)

        functor_start_simulation = world.spawn_actor(functor_start_simulation_bp, transform)


    except Exception as error:
        logging.info('  Error: %s', type(error))
        logging.info('  Error: %s', error)
        raise Exception("Failed to Spawn Vehicle and Sensors")

    return actors

def main(args):
    rclpy.init()
    simulation_status_node = SimulationStatusNode()
    executor = rclpy.executors.MultiThreadedExecutor()
    executor.add_node(simulation_status_node)
    world = None
    old_world = None
    vehicle_actors = None
    original_settings = None

    # Spin in a separate thread
    executor_thread = threading.Thread(target=executor.spin, daemon=True)
    executor_thread.start()

    try:
        # Load the Scenario File
        scenario_path = python_file_path + 'scenarios/' + args.file + '.yaml'
        logging.debug(' Loading Scenario File: %s', scenario_path)
        scenario_file = yaml.safe_load(open(scenario_path, 'r'))
        logging.debug('  %s', scenario_file)

        # Setup CARLA World
        logging.debug(' Setting up Carla Client and Settings')
        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)
        old_world = client.get_world()
        original_settings = old_world.get_settings()
        settings = old_world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05

        # Change CARLA Map to desired map
        if 'map' in scenario_file:
            logging.debug(' Checking for Map: %s', scenario_file['map'])
            logging.debug(' Available Map: %s', client.get_available_maps())
            for map in client.get_available_maps():
                if scenario_file['map'] in map:
                    logging.debug(' Loading Map: %s', map)
                    # client.load_world(map)
                    world = client.get_world()
                    world.apply_settings(settings)
                    break
        else:
            logging.info("  No Map in Scenario File")
            raise Exception("No Map in Scenario File")

        # Setup MetaHumans
        if 'casualties' not in scenario_file:
            logging.info("  No Casualties defined in Scenario File")
            raise Exception("No Casualties defined in Scenario File")
        # Needs Code 
  
        # time.sleep(7)

        # Create Vehicle with sensors
        vehicle_actors = _setup_vehicle_actors(world)

        # Setup Vehicle Waypoints
        if 'waypoints' not in scenario_file:
            logging.info("  No Waypoints defined in Scenario File")
            raise Exception("No Waypoints defined in Scenario File")
        # Needs Code 

        logging.info("  Running Mission...")
        _ = world.tick()
        simulation_status_node.set_status(True)
        while True:
            _ = world.tick()

    except KeyboardInterrupt:
        logging.info('  System Shutdown Command, closing out System Manager')
    except Exception as error:
        logging.info('  Error: %s', error)
        logging.info('  System Error, Check log, likely CARLA is not connected. See if CARLA is running.')

    finally:
        try:
            logging.info('  Reseting Game to original state...')
            if original_settings:
                client.load_world('StartingWorld')
                client.get_world().apply_settings(original_settings)

            for actor in vehicle_actors:
                actor.destroy()

            _ = world.tick()
            logging.info('  Game finished resetting, exiting System Manager...')
        except Exception as error:
            logging.info('  Error: %s', error)
            logging.info('  Failed to reset game to original state...')
            pass

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='DTC System Manager')
    argparser.add_argument('--host',          default='localhost', dest='host',    type=str,  help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port',          default=2000,        dest='port',    type=int,  help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('-f', '--file',    default='example',   dest='file',    type=str,  help='Scenario File to run, Note, Always looks in `dtc_manager/scenarios` and does not include the .yaml (default: example)')
    argparser.add_argument('-v', '--verbose', default=False,       dest='verbose', type=bool, help='print debug information (default: False)')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(filename=python_file_path + 'dtc_manager.log', level=log_level)
    with open(python_file_path + 'dtc_manager.log', 'w'):
        pass
    logging.info('  Starting DTC System Manager')
    logging.debug(' Listening to server %s:%s', args.host, args.port)

    main(args)

    print(open(python_file_path + "dtc_manager.log", "r").read())
