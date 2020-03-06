#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Script to create sumo vtypes based on carla blueprints.
"""

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import argparse
import glob
import datetime
import json
import logging
import os
import sys

import lxml.etree as ET  # pylint: disable=import-error

# ==================================================================================================
# -- find carla module -----------------------------------------------------------------------------
# ==================================================================================================

try:
    sys.path.append(
        glob.glob('../../../PythonAPI/carla/dist/carla-*%d.%d-%s.egg' %
                  (sys.version_info.major, sys.version_info.minor,
                   'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla  # pylint: disable=import-error, wrong-import-position


# ==================================================================================================
# -- load specs definition -------------------------------------------------------------------------
# ==================================================================================================

with open('../data/vtypes.json') as f:
    SPECS = json.load(f)

    DEFAULT_2_WHEELED_VEHICLE = SPECS['DEFAULT_2_WHEELED_VEHICLE']
    DEFAULT_WHEELED_VEHICLE = SPECS['DEFAULT_WHEELED_VEHICLE']
    CARLA_BLUEPRINTS_SPECS = SPECS['carla_blueprints']

# ==================================================================================================
# -- main ------------------------------------------------------------------------------------------
# ==================================================================================================


def write_vtype_xml(filename, vtypes):
    """
    Write route xml file.
    """
    root = ET.Element('routes')

    root.addprevious(
        ET.Comment('generated on {date:%Y-%m-%d %H:%M:%S} by {script:}'.format(
            date=datetime.datetime.now(), script=os.path.basename(__file__))))

    for vtype in vtypes:
        ET.SubElement(root, 'vType', vtype)

    tree = ET.ElementTree(root)
    tree.write(filename, pretty_print=True, encoding='UTF-8', xml_declaration=True)


def generate_vtype(vehicle):
    """Generates sumo vtype specification for a given carla vehicle.

        :param vehicle: carla actor (carla.Actor)
        :return: sumo vtype specifications
    """
    type_id = vehicle.type_id

    if type_id not in CARLA_BLUEPRINTS_SPECS:
        number_of_wheels = int(vehicle.attributes['number_of_wheels'])
        if number_of_wheels == 2:
            logging.warning(
                '''type id %s not mapped to any sumo vtype.
                \tUsing default specification for two-wheeled vehicles: %s''', type_id,
                DEFAULT_2_WHEELED_VEHICLE)
            user_specs = DEFAULT_2_WHEELED_VEHICLE
        else:
            logging.warning(
                '''type id %s not mapped to any sumo vtype.
                \tUsing default specification for wheeled vehicles: %s''', type_id,
                DEFAULT_WHEELED_VEHICLE)
            user_specs = DEFAULT_WHEELED_VEHICLE

    else:
        logging.info('type id %s mapped to the following specifications: %s', type_id,
                     CARLA_BLUEPRINTS_SPECS[type_id])
        user_specs = CARLA_BLUEPRINTS_SPECS[type_id]

    specs = {
        'id': vehicle.type_id,
        'length': str(2.0 * vehicle.bounding_box.extent.x),
        'width': str(2.0 * vehicle.bounding_box.extent.y),
        'height': str(2.0 * vehicle.bounding_box.extent.z)
    }

    specs.update(user_specs)
    return specs


def main(args):
    """
    Main method.
    """
    client = carla.Client(args.carla_host, args.carla_port)
    client.set_timeout(2.0)

    try:
        world = client.get_world()
        vehicle_blueprints = world.get_blueprint_library().filter('vehicle.*')

        transform = world.get_map().get_spawn_points()[0]

        vtypes = []
        for blueprint in vehicle_blueprints:
            logging.info('processing vtype for %s', blueprint.id)
            vehicle = world.spawn_actor(blueprint, transform)

            vtype = generate_vtype(vehicle)
            if vtype:
                vtypes.append(vtype)
            else:
                logging.error(
                    'type id %s could no be mapped to any vtype', vehicle.type_id)

            vehicle.destroy()

        write_vtype_xml(args.output_file, vtypes)

    finally:
        logging.info('done')


if __name__ == '__main__':
    # Define arguments that will be received and parsed.
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('--carla-host',
                           metavar='H',
                           default='127.0.0.1',
                           help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument('--carla-port',
                           metavar='P',
                           default=2000,
                           type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--output-file',
        '-o',
        metavar='FILE',
        default='carlavtypes.rou.xml',
        type=str,
        help='the generated vtypes will be written to FILE (default: carlavtypes.rou.xml)')
    argparser.add_argument('--verbose', '-v', action='store_true', help='increase output verbosity')
    arguments = argparser.parse_args()

    if arguments.verbose:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.WARNING)

    main(arguments)
