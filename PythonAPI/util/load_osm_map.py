#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Load OpenStreetMap files to Carla simulator.
Uses SUMO's netconvert tool to transform OpenStreetMap format to OpenDRIVE.
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
from opendrive_fixer import fix_opendrive

def main():
    """ """
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '-i', '--input',
        metavar='I',
        default='',
        type=str,
        help='Path to the input file')
    argparser.add_argument(
        '--keep_opendrive',
        action='store_true',
        help='Whether the temporary OpenDRIVE file should be removed')
    argparser.add_argument(
        '-n', '--netconvert_path',
        metavar='N',
        default='netconvert',
        type=str,
        help='Path to netconvert')
    argparser.add_argument(
        '-a', '--netconvert_args',
        metavar='A',
        default='',
        type=str,
        help='netconvert arguments')
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
        '-c', '--center_map',
        action="store_true",
        help='Centers the map')
    args = argparser.parse_args()

    netconvert_path = args.netconvert_path
    netconvert_args = args.netconvert_args
    if netconvert_args == '':
        # default netconvert arguments
        netconvert_args = '--proj "+proj=merc +units=m" --geometry.remove --ramps.guess --edges.join --junctions.join --keep-edges.by-type highway.motorway,highway.motorway_link,highway.trunk,highway.trunk_link,highway.primary,highway.primary_link,highway.secondary,highway.secondary_link,highway.tertiary,highway.tertiary_link,highway.unclassified,highway.residential --tls.discard-loaded --tls.discard-simple --default.lanewidth 4.0 --osm.layer-elevation 4'

    input_file = args.input
    if input_file != '':
        netconvert_command_line = netconvert_path + ' --osm-files ' + input_file + ' --opendrive-output temp_opendrive.xodr ' + netconvert_args
        print(netconvert_command_line)
        os.system(netconvert_command_line)
    else:
        raise Exception('Error: Missing input file')

    print('OSM converted to OpenDRIVE format. Fixing broken OpenDRIVE...')
    temp_file = open('temp_opendrive.xodr', 'r')
    broken_xodr = temp_file.read()
    temp_file.close()

    fixed_xodr = fix_opendrive(broken_xodr, args.center_map)
    temp_file = open('temp_opendrive.xodr', 'w')
    temp_file.write(fixed_xodr)
    temp_file.close()
    print('OpenDRIVE file fixed, connecting to simulation...')

    client = carla.Client(args.host, args.port, worker_threads=1)
    client.set_timeout(600.0)

    vertex_distance = 2.0  # in meters
    max_road_length = 50.0 # in meters
    wall_height = 0.0      # in meters
    extra_width = 0.8      # in meters
    world = client.generate_opendrive_world(
                fixed_xodr, carla.OpendriveGenerationParameters(
                    vertex_distance=vertex_distance,
                    max_road_length=max_road_length,
                    wall_height=wall_height,
                    additional_width=extra_width,
                    smooth_junctions=True,
                    enable_mesh_visibility=True,
                    enable_pedestrian_navigation=False))

    # Clean up temporary file
    if not args.keep_opendrive:
        os.remove('temp_opendrive.xodr')
    print('\nDone.')

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nExit by user.')
