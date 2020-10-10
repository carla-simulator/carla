#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Configure and inspect an instance of CARLA Simulator.

For further details, visit
https://carla.readthedocs.io/en/latest/configuring_the_simulation/
"""

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
import datetime
import re
import socket
import textwrap


def get_ip(host):
    if host in ['localhost', '127.0.0.1']:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            sock.connect(('10.255.255.255', 1))
            host = sock.getsockname()[0]
        except RuntimeError:
            pass
        finally:
            sock.close()
    return host


def find_weather_presets():
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    return [(getattr(carla.WeatherParameters, x), x) for x in presets]


def list_options(client):
    maps = [m.replace('/Game/Carla/Maps/', '') for m in client.get_available_maps()]
    indent = 4 * ' '
    def wrap(text):
        return '\n'.join(textwrap.wrap(text, initial_indent=indent, subsequent_indent=indent))
    print('weather presets:\n')
    print(wrap(', '.join(x for _, x in find_weather_presets())) + '.\n')
    print('available maps:\n')
    print(wrap(', '.join(sorted(maps))) + '.\n')


def list_blueprints(world, bp_filter):
    blueprint_library = world.get_blueprint_library()
    blueprints = [bp.id for bp in blueprint_library.filter(bp_filter)]
    print('available blueprints (filter %r):\n' % bp_filter)
    for bp in sorted(blueprints):
        print('    ' + bp)
    print('')


def inspect(args, client):
    address = '%s:%d' % (get_ip(args.host), args.port)

    world = client.get_world()
    elapsed_time = world.get_snapshot().timestamp.elapsed_seconds
    elapsed_time = datetime.timedelta(seconds=int(elapsed_time))

    actors = world.get_actors()
    s = world.get_settings()

    weather = 'Custom'
    current_weather = world.get_weather()
    for preset, name in find_weather_presets():
        if current_weather == preset:
            weather = name

    if s.fixed_delta_seconds is None:
        frame_rate = 'variable'
    else:
        frame_rate = '%.2f ms (%d FPS)' % (
            1000.0 * s.fixed_delta_seconds,
            1.0 / s.fixed_delta_seconds)

    print('-' * 34)
    print('address:% 26s' % address)
    print('version:% 26s\n' % client.get_server_version())
    print('map:        % 22s' % world.get_map().name)
    print('weather:    % 22s\n' % weather)
    print('time:       % 22s\n' % elapsed_time)
    print('frame rate: % 22s' % frame_rate)
    print('rendering:  % 22s' % ('disabled' if s.no_rendering_mode else 'enabled'))
    print('sync mode:  % 22s\n' % ('disabled' if not s.synchronous_mode else 'enabled'))
    print('actors:     % 22d' % len(actors))
    print('  * spectator:% 20d' % len(actors.filter('spectator')))
    print('  * static:   % 20d' % len(actors.filter('static.*')))
    print('  * traffic:  % 20d' % len(actors.filter('traffic.*')))
    print('  * vehicles: % 20d' % len(actors.filter('vehicle.*')))
    print('  * walkers:  % 20d' % len(actors.filter('walker.*')))
    print('-' * 34)


def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '-d', '--default',
        action='store_true',
        help='set default settings')
    argparser.add_argument(
        '-m', '--map',
        help='load a new map, use --list to see available maps')
    argparser.add_argument(
        '-r', '--reload-map',
        action='store_true',
        help='reload current map')
    argparser.add_argument(
        '--delta-seconds',
        metavar='S',
        type=float,
        help='set fixed delta seconds, zero for variable frame rate')
    argparser.add_argument(
        '--fps',
        metavar='N',
        type=float,
        help='set fixed FPS, zero for variable FPS (similar to --delta-seconds)')
    argparser.add_argument(
        '--rendering',
        action='store_true',
        help='enable rendering')
    argparser.add_argument(
        '--no-rendering',
        action='store_true',
        help='disable rendering')
    argparser.add_argument(
        '--no-sync',
        action='store_true',
        help='disable synchronous mode')
    argparser.add_argument(
        '--weather',
        help='set weather preset, use --list to see available presets')
    argparser.add_argument(
        '-i', '--inspect',
        action='store_true',
        help='inspect simulation')
    argparser.add_argument(
        '-l', '--list',
        action='store_true',
        help='list available options')
    argparser.add_argument(
        '-b', '--list-blueprints',
        metavar='FILTER',
        help='list available blueprints matching FILTER (use \'*\' to list them all)')
    argparser.add_argument(
        '-x', '--xodr-path',
        metavar='XODR_FILE_PATH',
        help='load a new map with a minimum physical road representation of the provided OpenDRIVE')
    argparser.add_argument(
        '--osm-path',
        metavar='OSM_FILE_PATH',
        help='load a new map with a minimum physical road representation of the provided OpenStreetMaps')
    if len(sys.argv) < 2:
        argparser.print_help()
        return

    args = argparser.parse_args()

    client = carla.Client(args.host, args.port, worker_threads=1)
    client.set_timeout(10.0)

    if args.default:
        args.rendering = True
        args.delta_seconds = 0.0
        args.weather = 'Default'
        args.no_sync = True

    if args.map is not None:
        print('load map %r.' % args.map)
        world = client.load_world(args.map)
    elif args.reload_map:
        print('reload map.')
        world = client.reload_world()
    elif args.xodr_path is not None:
        if os.path.exists(args.xodr_path):
            with open(args.xodr_path, encoding='utf-8') as od_file:
                try:
                    data = od_file.read()
                except OSError:
                    print('file could not be readed.')
                    sys.exit()
            print('load opendrive map %r.' % os.path.basename(args.xodr_path))
            vertex_distance = 2.0  # in meters
            max_road_length = 500.0 # in meters
            wall_height = 1.0      # in meters
            extra_width = 0.6      # in meters
            world = client.generate_opendrive_world(
                data, carla.OpendriveGenerationParameters(
                    vertex_distance=vertex_distance,
                    max_road_length=max_road_length,
                    wall_height=wall_height,
                    additional_width=extra_width,
                    smooth_junctions=True,
                    enable_mesh_visibility=True))
        else:
            print('file not found.')
    elif args.osm_path is not None:
        if os.path.exists(args.osm_path):
            with open(args.osm_path) as od_file:
                try:
                    data = od_file.read()
                except OSError:
                    print('file could not be readed.')
                    sys.exit()
            print('Converting OSM data to opendrive')
            xodr_data = carla.Osm2Odr.convert(data)
            print('load opendrive map.')
            vertex_distance = 2.0  # in meters
            max_road_length = 500.0 # in meters
            wall_height = 0.0      # in meters
            extra_width = 0.6      # in meters
            world = client.generate_opendrive_world(
                xodr_data, carla.OpendriveGenerationParameters(
                    vertex_distance=vertex_distance,
                    max_road_length=max_road_length,
                    wall_height=wall_height,
                    additional_width=extra_width,
                    smooth_junctions=True,
                    enable_mesh_visibility=True))
        else:
            print('file not found.')

    else:
        world = client.get_world()

    settings = world.get_settings()

    if args.no_rendering:
        print('disable rendering.')
        settings.no_rendering_mode = True
    elif args.rendering:
        print('enable rendering.')
        settings.no_rendering_mode = False

    if args.no_sync:
        print('disable synchronous mode.')
        settings.synchronous_mode = False

    if args.delta_seconds is not None:
        settings.fixed_delta_seconds = args.delta_seconds
    elif args.fps is not None:
        settings.fixed_delta_seconds = (1.0 / args.fps) if args.fps > 0.0 else 0.0

    if args.delta_seconds is not None or args.fps is not None:
        if settings.fixed_delta_seconds > 0.0:
            print('set fixed frame rate %.2f milliseconds (%d FPS)' % (
                1000.0 * settings.fixed_delta_seconds,
                1.0 / settings.fixed_delta_seconds))
        else:
            print('set variable frame rate.')
            settings.fixed_delta_seconds = None

    world.apply_settings(settings)

    if args.weather is not None:
        if not hasattr(carla.WeatherParameters, args.weather):
            print('ERROR: weather preset %r not found.' % args.weather)
        else:
            print('set weather preset %r.' % args.weather)
            world.set_weather(getattr(carla.WeatherParameters, args.weather))

    if args.inspect:
        inspect(args, client)
    if args.list:
        list_options(client)
    if args.list_blueprints:
        list_blueprints(world, args.list_blueprints)


if __name__ == '__main__':

    try:

        main()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    except RuntimeError as e:
        print(e)
