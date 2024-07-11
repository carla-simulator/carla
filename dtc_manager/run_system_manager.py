#!/usr/bin/env python

import os
import argparse
import yaml
import logging
import carla
import rclpy
import threading
from rclpy.node import Node
from std_msgs.msg import Bool
from std_msgs.msg import Empty
from sensor_msgs.msg import Imu
import time

python_file_path = os.path.realpath(__file__)
python_file_path = python_file_path.replace('run_system_manager.py', '')
global_transform = carla.Transform(carla.Location(x=0, y=0, z=0), carla.Rotation(yaw=180))

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

class SimulationStartNode(Node):

    def __init__(self):
        super().__init__("carla_simulation_start")
        self._start = False
        self.subscriber = self.create_subscription(Empty, "/simulation_start", self.set_start, 10)
    
    def get_start(self):
        return self._start

    def set_start(self, msg):
        self._start = True

class SimulationIMUNode(Node):

    def __init__(self):
        super().__init__("carla_simulation_imu_tracker")
        self._current_message = None
        self._last_message = None
        self._has_moved = False
        self._eps = 0.0001
        self.subscriber = self.create_subscription(Imu, "/carla/dtc_vehicle/imu", self.set_imu, 10)
    
    def is_stationary(self):
        # Check if the previous and current IMU messages are identical.
        # When Linear Accel and Angular Vel differentials are both 0,
        # The vehicle is assumed to be stationary
        if self._current_message is None or self._last_message is None:
            return False

        # Assume stationary and set to False if movement detected in any direction
        # this is done on a per axis basis but could be made better is linear math used
        is_stationary = True

        if abs(self._last_message.angular_velocity.x - self._current_message.angular_velocity.x) > self._eps:
            is_stationary = False
        if abs(self._last_message.angular_velocity.y - self._current_message.angular_velocity.y) > self._eps:
            is_stationary = False
        if abs(self._last_message.angular_velocity.z - self._current_message.angular_velocity.z) > self._eps:
            is_stationary = False
        if abs(self._last_message.linear_acceleration.x - self._current_message.linear_acceleration.x) > self._eps:
            is_stationary = False
        if abs(self._last_message.linear_acceleration.y - self._current_message.linear_acceleration.y) > self._eps:
            is_stationary = False
        if abs(self._last_message.linear_acceleration.z - self._current_message.linear_acceleration.z) > self._eps:
            is_stationary = False

        # Check if the vehicle has moved at all. We want this to only trigger at the end, the sim starts with the vehicle
        # stationary. So return False until the vehicle has moved.
        if not self._has_moved:
            if not is_stationary:
                self._has_moved = True
            return False

        return is_stationary

    def set_imu(self, msg):
        if self._current_message is not None:
            self._last_message = self._current_message
        self._current_message = msg

def _setup_waypoint_actors(world, scenario_file, bp_library):
    if 'waypoints' not in scenario_file:
        logging.info("  No Waypoints defined in Scenario File")
        raise Exception("No Waypoints defined in Scenario File")
    logging.info("  Setting Waypoints...")
    functor_sent_waypoints_bp = bp_library.filter("functorsendwaypoints")[0]
    iteration = 1
    for waypoint_name in scenario_file['waypoints']:
        waypoint = scenario_file['waypoints'][waypoint_name]
        if 'zone' not in waypoint:
            logging.info("  No Zone defined in Waypoint Scenario Loadout")
            raise Exception("No Zone defined in Waypoint Scenario Loadout")
        # Set Casualty in Attribute
        functor_sent_waypoints_bp.set_attribute(str(iteration), str(waypoint['zone']))
        iteration+=1
    return world.spawn_actor(functor_sent_waypoints_bp, global_transform)

def _setup_casualty_actors(world, scenario_file, bp_library):
    if 'casualties' not in scenario_file:
        logging.info("  No Casualties defined in Scenario File")
        raise Exception("No Casualties defined in Scenario File")
    logging.info("  Setting Casualties...")
    functor_sent_casualties_bp = bp_library.filter("functorsendcasualties")[0]
    iteration = 1
    logging.info("  Begin iterating through Casualties...")
    for casualty_name in scenario_file['casualties']:
        casualty = scenario_file['casualties'][casualty_name]
        if 'zone' not in casualty:
            logging.info("  No Zone defined in Casualty Scenario Loadout")
            raise Exception("No Zone defined in Casualty Scenario Loadout")
        if 'casualty_type' not in casualty:
            logging.info("  No Type defined in Casualty Scenario Loadout")
            raise Exception("No Type defined in Casualty Scenario Loadout")
        casualty_string = casualty['casualty_type'] + "|" + str(casualty['zone'])
        functor_sent_casualties_bp.set_attribute(str(iteration), casualty_string)
        iteration+=1
    return world.spawn_actor(functor_sent_casualties_bp, global_transform)

def _setup_vehicle_actors(world, scenario_file, bp_library):
    actors = []
    try:
        # vehicle settings, static for P1
        vehicle_type = "waypointvehicle"
        logging.debug(" Spawning vehicle: %s", vehicle_type)
        bp = bp_library.filter(vehicle_type)[0]
        logging.debug(" Setting attributes for: %s", vehicle_type)
        bp.set_attribute("role_name", 'dtc_vehicle')
        bp.set_attribute("ros_name",  'dtc_vehicle')
        logging.debug(" Spawning vehicle in world: %s", vehicle_type)
        vehicle = world.spawn_actor(bp, world.get_map().get_spawn_points()[0], attach_to=None)
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

        logging.debug(' Creating RADAR Sensor')
        sensor = bp_library.filter('sensor.other.radar')[0]
        sensor.set_attribute("role_name",          'front_radar')
        sensor.set_attribute("ros_name",           'front_radar')
        sensor.set_attribute("horizontal_fov",     '30')
        sensor.set_attribute("vertical_fov",       '30')
        sensor.set_attribute("range",              '50')
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
        sensor.set_attribute("sensor_tick",  '0.05')
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
        sensor.set_attribute("sensor_tick",  '0.05')
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

    except Exception as error:
        logging.info('  Error: %s', type(error))
        logging.info('  Error: %s', error)
        raise Exception("Failed to Spawn Vehicle and Sensors")

    return actors

def main(args):
    rclpy.init()
    simulation_status_node = SimulationStatusNode()
    simulation_start_node = SimulationStartNode()
    simulation_imu_node = SimulationIMUNode()
    executor = rclpy.executors.MultiThreadedExecutor()
    executor.add_node(simulation_status_node)
    executor.add_node(simulation_start_node)
    executor.add_node(simulation_imu_node)
    world = None
    old_world = None
    original_settings = None
    tracked_actors = []
    mission_started = False

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

        # Get and modify the weather
        weather = old_world.get_weather()            
        weather.cloudiness=10.000000
        weather.precipitation=0.00000
        weather.precipitation_deposits=0.00000
        weather.wind_intensity=5.000000
        weather.sun_azimuth_angle=-1.000000
        weather.sun_altitude_angle=45.000000
        weather.fog_density=0.000000
        weather.fog_distance=0.750000
        weather.fog_falloff=0.250000
        weather.wetness=0.000000
        weather.scattering_intensity=1.000000
        weather.mie_scattering_scale=0.003996
        weather.rayleigh_scattering_scale=0.033100
        weather.dust_storm=0.000000

        # Change CARLA Map to desired map
        if 'map' in scenario_file:
            logging.debug(' Checking for Map: %s', scenario_file['map'])
            logging.debug(' Available Map: %s', client.get_available_maps())
            for map in client.get_available_maps():
                if scenario_file['map'] in map:
                    logging.debug(' Loading Map: %s', map)
                    client.load_world(map)
                    world = client.get_world()
                    world.apply_settings(settings)
                    world.set_weather(weather)
                    break
        else:
            logging.info("  No Map in Scenario File")
            raise Exception("No Map in Scenario File")

        # Create the simulation starter
        bp_library = world.get_blueprint_library()
        functor_start_simulation_bp = bp_library.filter("functorstartsimulation")[0]

        # Setup MetaHumans
        tracked_actors.append(_setup_casualty_actors(world, scenario_file, bp_library))

        # Setup Vehicle Waypoints
        tracked_actors.append(_setup_waypoint_actors(world, scenario_file, bp_library))

        # Create Vehicle with sensors
        vehicle_actors = _setup_vehicle_actors(world, scenario_file, bp_library)
        for actor in vehicle_actors:
           tracked_actors.append(actor)

        # Start Simulation, need to process a few frames to fully load things
        logging.info("  Starting Simulation...")
        for step in range(5):
            _ = world.tick()
        simulation_status_node.set_status(True)
        stationary_count = 0
        while True:
            try:
                # Check if the simulation has not been started but should, if so, trigger the start command in Unreal
                if not mission_started and simulation_start_node.get_start():
                    logging.info("  Running Mission...")
                    tracked_actors.append(world.spawn_actor(functor_start_simulation_bp, global_transform))
                    mission_started = True

                # Tick the simulation if the mission has been started, otherwise, wait for the start command
                if mission_started:
                    _ = world.tick()
                else:
                    time.sleep(settings.fixed_delta_seconds)
                    # Ensures that rogue messages on startup don't contribute to stationary counts
                    stationary_count=0

                # Check if the vehicle is stationary, if not, reset the stationary count
                if simulation_imu_node.is_stationary():
                    stationary_count+=1
                else:
                    stationary_count=0
                
                # Check if the vehicle has been stationary long enough to close the simulation (10 seconds)
                if stationary_count >= 4 / settings.fixed_delta_seconds:
                    logging.info("  Mission Completed...")
                    break
            except:
                logging.info('  CARLA Client no longer connected, likely a system crash or the mission completed')
                raise Exception("CARLA Client no longer connected")
    except KeyboardInterrupt:
        logging.info('  System Shutdown Command, closing out System Manager')
    except Exception as error:
        logging.info('  Error: %s', error)
        logging.info('  System Error, Check log, likely CARLA is not connected. See if CARLA is running.')

    finally:
        try:
            if args.cleankill:
                logging.info('  Clean Kill enabled, End World...')
                if original_settings:
                    client.get_world().apply_settings(original_settings)
                    _ = world.tick()
                    client.load_world('EndingWorld')
                logging.info('  Game Instance Closed, exiting System Manager...')
            else:
                logging.info('  Reseting Game to original state...')
                for actor in tracked_actors:
                    actor.destroy()

                if original_settings:
                    client.load_world('StartingWorld')
                    client.get_world().apply_settings(original_settings)

                _ = world.tick()
                logging.info('  Game finished resetting, exiting System Manager...')
        except Exception as error:
            logging.info('  Error: %s', error)
            logging.info('  Failed to reset game to original state...')
            pass

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='DTC System Manager')
    argparser.add_argument('--host',          default='localhost', dest='host',      type=str,  help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port',          default=2000,        dest='port',      type=int,  help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('-f', '--file',    default='example',   dest='file',      type=str,  help='Scenario File to run, Note, Always looks in `dtc_manager/scenarios` and does not include the .yaml (default: example)')
    argparser.add_argument('-v', '--verbose', default=False,       dest='verbose',   type=bool, help='print debug information (default: False)')
    argparser.add_argument('--clean-kill',    default=False,       dest='cleankill', type=bool, help='When true, cleanly kills the simulation when the python script is exited')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(filename=python_file_path + 'dtc_manager.log', level=log_level)
    with open(python_file_path + 'dtc_manager.log', 'w'):
        pass
    logging.info('  Starting DTC System Manager')
    logging.debug(' Listening to server %s:%s', args.host, args.port)

    main(args)

    print(open(python_file_path + "dtc_manager.log", "r").read())
