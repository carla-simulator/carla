#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""A client for benchmarking the CARLA server."""

import argparse
import logging
import os
import random
import sys
import time

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

import carla

from carla import sensor
from carla.client import make_carla_client
from carla.sensor import Camera
from carla.settings import CarlaSettings
from carla.util import StopWatch

TEXT = \
"""===========================
Annotated {count:d} frames.
---------------------------
average = {avg:.2f} FPS
maximum = {max:.2f} FPS
minimum = {min:.2f} FPS
===========================
"""

def make_base_settings():
    return CarlaSettings(
        WeatherId=1,
        SendNonPlayerAgentsInfo=False,
        SynchronousMode=False,
        NumberOfVehicles=20,
        NumberOfPedestrians=30,
        SeedVehicles=123456789,
        SeedPedestrians=123456789,
        QualityLevel='Epic')


def generate_settings_scenario_001():
    logging.info('Scenario 001: no sensors')
    return make_base_settings()


def generate_settings_scenario_002():
    logging.info('Scenario 002: no sensors, no agents at all')
    settings = make_base_settings()
    settings.set(NumberOfVehicles=0, NumberOfPedestrians=0)
    return settings


def generate_settings_scenario_003():
    logging.info('Scenario 003: no sensors, no pedestrians')
    settings = make_base_settings()
    settings.set(NumberOfPedestrians=0)
    return settings


def generate_settings_scenario_004():
    logging.info('Scenario 004: no sensors, no vehicles')
    settings = make_base_settings()
    settings.set(NumberOfVehicles=0)
    return settings


def generate_settings_scenario_005():
    logging.info('Scenario 005: no sensors, hard rain')
    settings = make_base_settings()
    settings.set(WeatherId=13)
    return settings


def generate_settings_scenario_006():
    logging.info('Scenario 006: no sensors, sending agents info')
    settings = make_base_settings()
    settings.set(SendNonPlayerAgentsInfo=True)
    return settings


def generate_settings_scenario_007():
    logging.info('Scenario 007: single camera RGB')
    settings = make_base_settings()
    settings.add_sensor(Camera('DefaultRGBCamera'))
    return settings


def generate_settings_scenario_008():
    logging.info('Scenario 008: single camera Depth')
    settings = make_base_settings()
    settings.add_sensor(Camera('DefaultDepthCamera', PostProcessing='Depth'))
    return settings


def generate_settings_scenario_009():
    logging.info('Scenario 009: single camera SemanticSegmentation')
    settings = make_base_settings()
    settings.add_sensor(Camera('DefaultSemSegCamera', PostProcessing='SemanticSegmentation'))
    return settings


def generate_settings_scenario_010():
    logging.info('Scenario 010: 3 cameras')
    settings = make_base_settings()
    settings.add_sensor(Camera('DefaultRGBCamera'))
    settings.add_sensor(Camera('DefaultDepthCamera', PostProcessing='Depth'))
    settings.add_sensor(Camera('DefaultSemSegCamera', PostProcessing='SemanticSegmentation'))
    return settings


class FPSWatch(object):
    def __init__(self):
        self.stop_watch = StopWatch()
        self.sum = 0.0
        self.count = 0
        self.max = 0.0
        self.min = float("inf")

    def annotate(self):
        self.stop_watch.stop()
        fps = 1.0 / self.stop_watch.seconds()
        self.sum += fps
        self.count += 1
        self.max = max(self.max, fps)
        self.min = min(self.min, fps)
        self.stop_watch.restart()

    def __str__(self):
        return TEXT.format(
            count=self.count,
            avg=self.sum/self.count,
            max=self.max,
            min=self.min)


def run_carla_client(args, settings_generators):
    with make_carla_client(args.host, args.port, timeout=25) as client:
        for settings_generator in settings_generators:
            scene = client.load_settings(settings_generator())
            client.start_episode(0)
            watch = FPSWatch()
            for frame in range(0, 3000):
                measurements, sensor_data = client.read_data()
                client.send_control(measurements.player_measurements.autopilot_control)
                watch.annotate()
            print(str(watch))
        print('done.')


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--log',
        metavar='LOG_FILE',
        default=None,
        help='print output to file')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host server (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-s', '--scenario',
        metavar='N',
        default=-1,
        type=int,
        help='benchmark scenario to use')
    argparser.add_argument(
        '-l', '--list',
        action='store_true',
        help='list available benchmark scenarios')

    args = argparser.parse_args()

    logging_config = {
        'format': '%(levelname)s: %(message)s',
        'level': logging.DEBUG if args.debug else logging.INFO
    }
    if args.log:
        logging_config['filename'] = args.log
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    self_module = sys.modules[__name__]
    generators = sorted(x for x in dir(self_module) if x.startswith('generate_settings_scenario_'))

    if args.list:
        for generator_name in generators:
            getattr(self_module, generator_name)()
        return

    if args.scenario == -1:
        generators_to_use = generators
    elif args.scenario < 1 or args.scenario > len(generators):
        logging.error('invalid scenario %d', args.scenario)
        sys.exit(1)
    else:
        generators_to_use = [generators[args.scenario - 1]]

    settings_generators = [getattr(self_module, x) for x in generators_to_use]

    logging.info('listening to server %s:%s', args.host, args.port)

    while True:
        try:

            run_carla_client(args, settings_generators)
            return

        except AssertionError as assertion:
            raise assertion
        except Exception as exception:
            logging.error('exception: %s', exception)
            time.sleep(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
