#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
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
    logging.debug("Spawning vehicle: {}".format(config.get("type")))

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
        logging.debug("Spawning sensor: {}".format(sensor))

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
    traffic_manager = None
    original_settings = None
    synchronous_master = False

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)

        world = client.get_world()

        original_settings = world.get_settings()
        settings = world.get_settings()

        traffic_manager = client.get_trafficmanager(args.tm_port)
        if not args.asynch:
            traffic_manager.set_synchronous_mode(True)

        # A synchronous world must have exactly one client ticking it. By
        # default we only take ownership of the clock (become the synchronous
        # master) if no other client already runs the world synchronously
        # (e.g. generate_traffic.py); otherwise we would double-step the
        # simulation and destabilise Traffic Manager control of the autopilot
        # vehicle. --force-sync overrides this, --asynch opts out entirely.
        if args.force_sync and settings.synchronous_mode:
            logging.warning(
                "--force-sync: the world is already in synchronous mode; "
                "ticking it from a second client may double-step the simulation.")

        if not args.asynch and (args.force_sync or not settings.synchronous_mode):
            synchronous_master = True
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = args.delta
            world.apply_settings(settings)

        with open(args.file) as f:
            config = json.load(f)

        vehicle = _setup_vehicle(world, config)
        sensors = _setup_sensors(world, vehicle, config.get("sensors", []))

        if synchronous_master:
            world.tick()
        else:
            world.wait_for_tick()

        vehicle.set_autopilot(True, args.tm_port)

        logging.info("Running...")

        while True:
            if synchronous_master:
                world.tick()
            else:
                world.wait_for_tick()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')

    finally:
        # Only the synchronous master that took ownership of the clock should
        # release it: drop Traffic Manager back to async and restore the world
        # settings. If another client (e.g. generate_traffic.py) owns the clock
        # we leave its synchronous mode untouched.
        if synchronous_master:
            if traffic_manager:
                traffic_manager.set_synchronous_mode(False)
            if original_settings:
                world.apply_settings(original_settings)

        for sensor in sensors:
            sensor.destroy()

        if vehicle:
            vehicle.destroy()


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='CARLA ROS2 native')
    argparser.add_argument('--host', metavar='H', default='localhost', help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port', metavar='P', default=2000, type=int, help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('-f', '--file', default='', required=True, help='File to be executed')
    argparser.add_argument('-v', '--verbose', action='store_true', dest='debug', help='print debug information')
    argparser.add_argument('--tm-port', metavar='P', default=8000, type=int, help='Port of the Traffic Manager to register the autopilot vehicle with (default: 8000). Must match the port used by generate_traffic.py.')
    argparser.add_argument('--delta', metavar='S', default=0.05, type=float, help='Fixed simulation time step in seconds, applied only when this client becomes the synchronous master (default: 0.05)')
    sync_group = argparser.add_mutually_exclusive_group()
    sync_group.add_argument('--force-sync', action='store_true', help='Always become the synchronous master and tick the world, even if another client already runs it synchronously (may double-step the simulation)')
    sync_group.add_argument('--asynch', action='store_true', help='Do not take control of the simulation clock: never enable synchronous mode and never tick, only wait for ticks from whoever owns the world')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('Listening to server %s:%s', args.host, args.port)

    main(args)
