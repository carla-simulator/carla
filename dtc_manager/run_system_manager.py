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


python_file_path = os.path.realpath(__file__)
python_file_path = python_file_path.replace('run_system_manager.py', '')

def _setup_vehicle(world, config):
    logging.debug(" Spawning vehicle: {}".format(config.get("type")))

    bp_library = world.get_blueprint_library()
    map_ = world.get_map()

    bp = bp_library.filter(config.get("type"))[0]
    bp.set_attribute("role_name", config.get("id"))
    bp.set_attribute("ros_name", config.get("id")) 

    return  world.spawn_actor(
        bp,
        map_.get_spawn_points()[0],
        attach_to=None)


def _setup_sensors(world, vehicle, sensors_config):
    logging.debug(' Creating Sensor')
    bp_library = world.get_blueprint_library()

    sensors = []
    for sensor in sensors_config:
        logging.debug(" Spawning sensor: {}".format(sensor))

        bp = bp_library.filter(sensor.get("type"))[0]
        bp.set_attribute("ros_name", sensor.get("id")) 
        bp.set_attribute("role_name", sensor.get("id")) 
        for key, value in sensor.get("attributes", {}).items():
            bp.set_attribute(str(key), str(value))

        wp = carla.Transform(
            location=carla.Location(x=sensor["spawn_point"]["x"], y=-sensor["spawn_point"]["y"], z=sensor["spawn_point"]["z"]),
            rotation=carla.Rotation(roll=sensor["spawn_point"]["roll"], pitch=-sensor["spawn_point"]["pitch"], yaw=-sensor["spawn_point"]["yaw"])
        )

        sensors.append(
            world.spawn_actor(
                bp,
                wp,
                attach_to=vehicle
            )
        )

        sensors[-1].enable_for_ros()

    return sensors


def main(args):

    world = None
    old_world = None
    vehicle = None
    sensors = []
    original_settings = None

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
                if scenario_file['map'] in map and 'Carla' in map:
                    logging.info('  Loading Map: %s', map)
                    client.load_world(map)
                    world = client.get_world()
                    world.apply_settings(settings)
                    break

            
        

        #with open(args.file) as f:
        #    config = json.load(f)

        #vehicle = _setup_vehicle(world, config)
        #sensors = _setup_sensors(world, vehicle, config.get("sensors", []))

        #_ = world.tick()

        #vehicle.set_autopilot(True)

        #logging.info("  Running...")

        while True:
            _ = world.tick()

    except KeyboardInterrupt:
        logging.debug(' System Shutdown Command, closing out System Manager')
    except Exception as error:
        logging.info('  Error: %s', error)
        logging.debug(' System Error, Check log, likely CARLA is not connected. See if CARLA is running.')

    finally:
        try:
            logging.info('  Reseting Game to original state...')
            if original_settings:
                client.load_world('NewWorld')
                client.get_world().apply_settings(original_settings)

            # for sensor in sensors:
            #     sensor.destroy()

            # if vehicle:
            #     vehicle.destroy()

            _ = world.tick()
            logging.info('  Game finished resetting, exiting System Manager...')
        except Exception as error:
            logging.debug(' Error: %s', error)
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
