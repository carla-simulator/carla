#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

import argparse
import json
import logging
import carla


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
    vehicle = None
    sensors = []
    original_settings = None

    try:
        # Load the Scenario File
        logging.debug(' Loading Scenario File: %s', args.file)


        # Setup CARLA World
        logging.debug(' Setting up Carla Client and Settings')
        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)
        world = client.get_world()
        original_settings = world.get_settings()
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)


        # Change CARLA Map to desired map




        #with open(args.file) as f:
        #    config = json.load(f)

        #vehicle = _setup_vehicle(world, config)
        #sensors = _setup_sensors(world, vehicle, config.get("sensors", []))

        #_ = world.tick()

        #vehicle.set_autopilot(True)

        #logging.info("  Running...")

        #while True:
        #    _ = world.tick()

    except KeyboardInterrupt:
        logging.debug(' System Shutdown Command, closing out System Manager')
    except:
        logging.debug(' System Error, Check log, likely CARLA is not connected. See if CARLA is running.')

    finally:
        try:
            logging.info('  Reseting Game to original state...')
            if original_settings:
                world.apply_settings(original_settings)

            for sensor in sensors:
                sensor.destroy()

            if vehicle:
                vehicle.destroy()
            logging.info('  Game finished resetting, exiting System Manager...')
        except:
            logging.info('  Failed to reset game to original state...')
            pass


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='DTC System Manager')
    argparser.add_argument('--host',          default='localhost', dest='host',    type=str,  help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port',          default=2000,        dest='port',    type=int,  help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('-f', '--file',    default='example',   dest='file',    type=str,  help='Scenario File to run, Note, Always looks in `dtc_manager/scenarios` and does not include the .yaml (default: example)')
    argparser.add_argument('-v', '--verbose', default=True,        dest='verbose', type=bool, help='print debug information (default: True)')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(filename='dtc_manager.log', level=log_level)
    with open('dtc_manager.log', 'w'):
        pass
    logging.debug(' Starting DTC System Manager')
    logging.info('  Listening to server %s:%s', args.host, args.port)

    main(args)

    print(open("dtc_manager.log", "r").read())
