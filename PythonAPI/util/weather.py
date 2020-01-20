#!/usr/bin/env python

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse


def main():
    argparser = argparse.ArgumentParser(
        description="CARLA weather example")
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
        help='Sun position presets [day | night | sunset]')
    argparser.add_argument(
        '--weather',
        default=None,
        type=str,
        help='Weather condition presets [clear | overcast | rain]')
    argparser.add_argument(
        '--altitude', '-alt',
        metavar='A',
        default=None,
        type=float,
        help='Sun altitude [-90 to 90]')
    argparser.add_argument(
        '--azimuth', '-azm',
        metavar='A',
        default=None,
        type=float,
        help='Sun azimuth [0 to 360]')
    argparser.add_argument(
        '--clouds', '-c',
        metavar='C',
        default=None,
        type=float,
        help='Clouds amount [0.0 to 100.0]')
    argparser.add_argument(
        '--rain', '-r',
        metavar='R',
        default=None,
        type=float,
        help='Rain amount [0.0 to 100.0]')
    argparser.add_argument(
        '--puddles', '-pd',
        metavar='Pd',
        default=None,
        type=float,
        help='Puddles amount [0.0 to 100.0]')
    argparser.add_argument(
        '--wind', '-w',
        metavar='W',
        default=None,
        type=float,
        help='Wind intensity [0.0 to 100.0]')
    argparser.add_argument(
        '--fog', '-f',
        metavar='F',
        default=None,
        type=float,
        help='Fog intensity [0.0 to 100.0]')
    argparser.add_argument(
        '--fogdist', '-fd',
        metavar='Fd',
        default=None,
        type=float,
        help='Fog Distance')
    argparser.add_argument(
        '--wetness', '-wet',
        metavar='Wet',
        default=None,
        type=float,
        help='Wetness')
    args = argparser.parse_args()

    if not (args.sun or args.weather or args.altitude or args.azimuth or args.clouds or args.rain or args.puddles or args.wind or args.fog or args.fogdist or args.wetness):
        argparser.error('No arguments provided.')

    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)
    world = client.get_world()

    weather = world.get_weather()

    ####   PRESETS   ###
    if args.sun is not None:
        ##  Sun Position  ##
        if args.sun == 'day':
            weather.sun_altitude_angle = 90
            weather.sun_azimuth_angle = 220.0

        elif args.sun == 'night':
            weather.sun_altitude_angle = -90
            weather.sun_azimuth_angle = 220.0

        elif args.sun == 'sunset':
            weather.sun_altitude_angle = 181
            weather.sun_azimuth_angle = 100

        else:
            print("[ERROR]: Command [--sun | -s] '" + args.sun + "' not known")
            sys.exit(1)

    ##  Weather  ##
    if args.weather is not None:
        if args.weather == 'clear':
            weather.cloudiness = 10.0
            weather.precipitation = 0.0
            weather.precipitation_deposits = 0.0
            weather.wind_intensity = 5.0

        elif args.weather == 'overcast':
            weather.cloudiness = 80.0
            weather.precipitation = 0.0
            weather.precipitation_deposits = 0.0
            weather.wind_intensity = 50.0

        elif args.weather == 'rain':
            weather.cloudiness = 100.0
            weather.precipitation = 80.0
            weather.precipitation_deposits = 90.0
            weather.wind_intensity = 100.0
            weather.fog_density = 20.0
            weather.fog_distance = 50.0
            weather.wetness = 100.0

        else:
            print("[ERROR]: Command [--weather | -w] '" + args.weather + "' not known")
            sys.exit(1)

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


if __name__ == '__main__':
    main()
