#!/usr/bin/env python

"""
Script to control weather parameters in simulations
"""

import glob
import os
import sys
import argparse

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

SUN_PRESETS = {
    'day': (60.0, 0.0),
    'night': (-90.0, 0.0),
    'sunset': (0.5, 180.0)}

WEATHER_PRESETS = {
    'clear': [10.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0],
    'overcast': [80.0, 0.0, 0.0, 50.0, 2.0, 0.0, 10.0],
    'rain': [100.0, 80.0, 90.0, 100.0, 20.0, 0.0, 100.0]}


def apply_sun_presets(args, weather):
    """Uses sun presets to set the sun position"""
    if args.sun is not None:
        if args.sun in SUN_PRESETS:
            weather.sun_altitude_angle = SUN_PRESETS[args.sun][0]
            weather.sun_azimuth_angle = SUN_PRESETS[args.sun][1]
        else:
            print("[ERROR]: Command [--sun | -s] '" + args.sun + "' not known")
            sys.exit(1)


def apply_weather_presets(args, weather):
    """Uses weather presets to set the weather parameters"""
    if args.weather is not None:
        if args.weather in WEATHER_PRESETS:
            weather.cloudiness = WEATHER_PRESETS[args.weather][0]
            weather.precipitation = WEATHER_PRESETS[args.weather][1]
            weather.precipitation_deposits = WEATHER_PRESETS[args.weather][2]
            weather.wind_intensity = WEATHER_PRESETS[args.weather][3]
            weather.fog_density = WEATHER_PRESETS[args.weather][4]
            weather.fog_distance = WEATHER_PRESETS[args.weather][5]
            weather.wetness = WEATHER_PRESETS[args.weather][6]
        else:
            print("[ERROR]: Command [--weather | -w] '" + args.weather + "' not known")
            sys.exit(1)


def apply_weather_values(args, weather):
    """Set weather values individually"""
    if args.azimuth is not None:
        weather.sun_azimuth_angle = args.azimuth
    if args.altitude is not None:
        weather.sun_altitude_angle = args.altitude
    if args.clouds is not None:
        weather.cloudiness = args.clouds
    if args.rain is not None:
        weather.precipitation = args.rain
    if args.puddles is not None:
        weather.precipitation_deposits = args.puddles
    if args.wind is not None:
        weather.wind_intensity = args.wind
    if args.fog is not None:
        weather.fog_density = args.fog
    if args.fogdist is not None:
        weather.fog_distance = args.fogdist
    if args.wetness is not None:
        weather.wetness = args.wetness


def main():
    """Start function"""
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
        '--sun',
        default=None,
        type=str,
        help='Sun position presets [' + ' | '.join([i for i in SUN_PRESETS]) + ']')
    argparser.add_argument(
        '--weather',
        default=None,
        type=str,
        help='Weather condition presets [' + ' | '.join([i for i in WEATHER_PRESETS]) + ']')
    argparser.add_argument(
        '--altitude', '-alt',
        metavar='A',
        default=None,
        type=float,
        help='Sun altitude [-90.0, 90.0]')
    argparser.add_argument(
        '--azimuth', '-azm',
        metavar='A',
        default=None,
        type=float,
        help='Sun azimuth [0.0, 360.0]')
    argparser.add_argument(
        '--clouds', '-c',
        metavar='C',
        default=None,
        type=float,
        help='Clouds amount [0.0, 100.0]')
    argparser.add_argument(
        '--rain', '-r',
        metavar='R',
        default=None,
        type=float,
        help='Rain amount [0.0, 100.0]')
    argparser.add_argument(
        '--puddles', '-pd',
        metavar='Pd',
        default=None,
        type=float,
        help='Puddles amount [0.0, 100.0]')
    argparser.add_argument(
        '--wind', '-w',
        metavar='W',
        default=None,
        type=float,
        help='Wind intensity [0.0, 100.0]')
    argparser.add_argument(
        '--fog', '-f',
        metavar='F',
        default=None,
        type=float,
        help='Fog intensity [0.0, 100.0]')
    argparser.add_argument(
        '--fogdist', '-fd',
        metavar='Fd',
        default=None,
        type=float,
        help='Fog Distance [0.0, inf)')
    argparser.add_argument(
        '--wetness', '-wet',
        metavar='Wet',
        default=None,
        type=float,
        help='Wetness intensity [0.0, 100.0]')
    args = argparser.parse_args()

    # since all the arguments are None by default
    # (except for the first 2, host and port)
    # we can check if all the arguments have been provided
    arg_values = [v for _, v in args.__dict__.items()][2:]
    if all(i is (None and False) for i in arg_values):
        argparser.error('No arguments provided.')

    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)
    world = client.get_world()

    weather = world.get_weather()

    # apply presets
    apply_sun_presets(args, weather)
    apply_weather_presets(args, weather)

    # apply weather values individually
    apply_weather_values(args, weather)

    world.set_weather(weather)


if __name__ == '__main__':
    main()
