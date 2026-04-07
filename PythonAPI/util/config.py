#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Configure and inspect an instance of CARLA Simulator.

For further details, visit
https://carla.readthedocs.io/en/latest/configuring_the_simulation/
"""

from __future__ import annotations

import argparse
import datetime
import os
import socket
import sys
import textwrap

import carla


def get_ip(host: str) -> str:
    """
    Get local IP address for a given host.

    Args:
        host: Hostname to check (e.g., 'localhost', '127.0.0.1')

    Returns:
        Local IP address if host is localhost, otherwise the original host.
    """
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


def list_options(client: carla.Client) -> None:
    """
    List all available maps in the CARLA server.

    Args:
        client: CARLA client instance.
    """
    maps = [m.replace('/Game/Carla/Maps/', '') for m in client.get_available_maps()]
    indent = 4 * ' '

    def wrap(text):
        return '\n'.join(textwrap.wrap(text, initial_indent=indent, subsequent_indent=indent))

    print('available maps:\n')
    print(wrap(', '.join(sorted(maps))) + '.\n')


def list_blueprints(world: carla.World, bp_filter: str) -> None:
    """
    List all available blueprints matching a filter.

    Args:
        world: CARLA world instance.
        bp_filter: Filter pattern for blueprints (e.g., 'vehicle.*').

    Raises:
        ValueError: If world is None.
    """
    if world is None:
        raise ValueError('world')
    blueprint_library = world.get_blueprint_library()
    blueprints = [bp.id for bp in blueprint_library.filter(bp_filter)]
    print(f'available blueprints (filter {bp_filter!r}):\n')
    for bp in sorted(blueprints):
        print('    ' + bp)
    print()


def inspect(args: argparse.Namespace, client: carla.Client) -> None:
    """
    Inspect and print details about a CARLA server instance.

    Args:
        args: Parsed command-line arguments.
        client: CARLA client instance.

    Raises:
        ValueError: If client is None.
    """
    if client is None:
        raise ValueError('client')
    address = f'{get_ip(args.host)}:{args.port}'

    world = client.get_world()
    elapsed_time = world.get_snapshot().timestamp.elapsed_seconds
    elapsed_time = datetime.timedelta(seconds=int(elapsed_time))

    actors = world.get_actors()
    s = world.get_settings()

    if s.fixed_delta_seconds is None:
        frame_rate = 'variable'
    else:
        frame_rate = f'{1000.0 * s.fixed_delta_seconds:.2f} ms ({int(1.0 / s.fixed_delta_seconds)} FPS)'

    print('---------------------------------')
    print(f'address:{address:>26}')
    print(f'version:{client.get_server_version():>26}\n')
    print(f'map:        {world.get_map().name:>22}')
    print(f'time:       {elapsed_time!s:>22}\n')
    print(f'frame rate: {frame_rate:>22}')
    print(f'rendering:  {"disabled" if s.no_rendering_mode else "enabled":>22}')
    print(f'sync mode:  {"disabled" if not s.synchronous_mode else "enabled":>22}\n')
    print(f'actors:     {len(actors):>22}')
    print(f'  * spectator:{len(actors.filter("spectator")):>20}')
    print(f'  * static:   {len(actors.filter("static.*")):>20}')
    print(f'  * traffic:  {len(actors.filter("traffic.*")):>20}')
    print(f'  * vehicles: {len(actors.filter("vehicle.*")):>20}')
    print(f'  * walkers:  {len(actors.filter("walker.*")):>20}')
    print('---------------------------------')


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '--host', metavar='H', default='localhost', help='IP of the host CARLA Simulator (default: localhost)'
    )
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int, help='TCP port of CARLA Simulator (default: 2000)'
    )
    argparser.add_argument('-l', '--list', action='store_true', help='list available options')
    argparser.add_argument(
        '-b',
        '--list-blueprints',
        metavar='FILTER',
        help="list available blueprints matching FILTER (use '*' to list them all)",
    )
    argparser.add_argument('-m', '--map', help='load a new map, use --list to see available maps')
    argparser.add_argument('-r', '--reload-map', action='store_true', help='reload current map')
    argparser.add_argument(
        '--delta-seconds', metavar='S', type=float, help='set fixed delta seconds, zero for variable frame rate'
    )
    argparser.add_argument(
        '--fps', metavar='N', type=float, help='set fixed FPS, zero for variable FPS (similar to --delta-seconds)'
    )
    argparser.add_argument('--rendering', action='store_true', help='enable rendering')
    argparser.add_argument('--no-rendering', action='store_true', help='disable rendering')
    argparser.add_argument('--no-sync', action='store_true', help='disable synchronous mode')
    argparser.add_argument('-i', '--inspect', action='store_true', help='inspect simulation')
    argparser.add_argument(
        '-x',
        '--xodr-path',
        metavar='XODR_FILE_PATH',
        help='load a new map with a minimum physical road representation of the provided OpenDRIVE',
    )
    argparser.add_argument(
        '--osm-path',
        metavar='OSM_FILE_PATH',
        help='load a new map with a minimum physical road representation of the provided OpenStreetMaps',
    )
    argparser.add_argument(
        '--tile-stream-distance', metavar='N', type=float, help='Set tile streaming distance (large maps only)'
    )
    argparser.add_argument(
        '--actor-active-distance', metavar='N', type=float, help='Set actor active distance (large maps only)'
    )

    args = argparser.parse_args()

    if len(sys.argv) < 2:
        argparser.print_help()
        return

    client = carla.Client(args.host, args.port, worker_threads=1)
    client.set_timeout(10.0)

    if args.map is not None:
        print(f'Load map {args.map!r}')
        world = client.load_world(args.map)

    elif args.reload_map:
        print('Reload map')
        world = client.reload_world()

    elif args.xodr_path is not None:
        if os.path.exists(args.xodr_path):
            with open(args.xodr_path, encoding='utf-8') as od_file:
                try:
                    data = od_file.read()
                except OSError:
                    print('File could not be readed')
                    sys.exit()
            print(f'Loading opendrive map {os.path.basename(args.xodr_path)!r}')
            world = client.generate_opendrive_world(
                data,
                carla.OpendriveGenerationParameters(
                    vertex_distance=2.0,
                    max_road_length=500.0,
                    wall_height=1.0,
                    additional_width=0.6,
                    smooth_junctions=True,
                    enable_mesh_visibility=True,
                ),
            )

        else:
            print('File not found')

    elif args.osm_path is not None:
        if os.path.exists(args.osm_path):
            with open(args.osm_path, encoding='utf-8') as od_file:
                try:
                    data = od_file.read()
                except OSError:
                    print('File could not be readed')
                    sys.exit()
            print('Converting OSM data to opendrive')
            xodr_data = carla.Osm2Odr.convert(data)
            print('Loading opendrive map')
            world = client.generate_opendrive_world(
                xodr_data,
                carla.OpendriveGenerationParameters(
                    vertex_distance=2.0,
                    max_road_length=500.0,
                    wall_height=1.0,
                    additional_width=0.6,
                    smooth_junctions=True,
                    enable_mesh_visibility=True,
                ),
            )

        else:
            print('File not found.')

    else:
        world = client.get_world()

    settings = world.get_settings()

    if args.no_rendering:
        print('Disabling rendering mode')
        settings.no_rendering_mode = True
    elif args.rendering:
        print('Enabling rendering mode')
        settings.no_rendering_mode = False

    if args.no_sync:
        print('Disabling synchronous mode')
        settings.synchronous_mode = False

    if args.delta_seconds is not None:
        settings.fixed_delta_seconds = args.delta_seconds
    elif args.fps is not None:
        settings.fixed_delta_seconds = (1.0 / args.fps) if args.fps > 0.0 else 0.0

    if args.delta_seconds is not None or args.fps is not None:
        if settings.fixed_delta_seconds > 0.0:
            print(
                f'Set fixed frame rate {1000.0 * settings.fixed_delta_seconds:.2f} milliseconds ({int(1.0 / settings.fixed_delta_seconds)} FPS)'
            )
        else:
            print('Set variable frame rate.')
            settings.fixed_delta_seconds = None

    if args.tile_stream_distance is not None:
        settings.tile_stream_distance = args.tile_stream_distance
    if args.actor_active_distance is not None:
        settings.actor_active_distance = args.actor_active_distance

    world.apply_settings(settings)

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
