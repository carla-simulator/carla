#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Script to modify automatically vtypes to carla type ids in sumo route files.
"""

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import argparse
import fnmatch
import json
import logging
import lxml.etree as ET
import random

# ==============================================================================
# -- load vtypes ---------------------------------------------------------------
# ==============================================================================

with open('../data/vtypes.json') as f:    
    VTYPES = json.load(f)['carla_blueprints'].keys()

# ==============================================================================
# -- main ----------------------------------------------------------------------
# ==============================================================================

def main(route_files, vtypes, _random=False):
    for filename in route_files:
        tree = ET.parse(filename)
        root = tree.getroot()

        if not _random:
            index = 0

        counter = 0
        for vtype in root.iter('vehicle'):
            if _random:
                new_type = random.choice(vtypes)
            else:
                new_type = vtypes[index]
                index = (index + 1) if index < (len(vtypes) - 1) else 0 
            
            vtype.set('type', new_type)
            counter += 1

        tree = ET.ElementTree(root)
        tree.write(filename, pretty_print=True, encoding='UTF-8', xml_declaration=True)

        logging.info('modified {counter:} vtype(s) in {file:}'.format(
            counter=counter,
            file=filename
        ))

if __name__ == '__main__':
    # Define arguments that will be received and parsed.
    argparser = argparse.ArgumentParser(
        description=__doc__
    )
    argparser.add_argument(
        '--route-files', '-r',
        metavar='FILES',
        nargs='+',
        default=[],
        help='sumo route files'
    )
    argparser.add_argument(
        '--random',
        action='store_true',
        help='apply vtypes randomly or sequentially'
    )
    argparser.add_argument(
        '--filterv',
        metavar='PATTERN',
        default='vehicle.*',
        help='vehicles filter (default: "vehicle.*")'
    )
    argparser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='increase output verbosity'
    )
    args = argparser.parse_args()

    if args.verbose:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.WARNING)

    vtypes = [vtype for vtype in VTYPES if fnmatch.fnmatch(vtype, args.filterv)]
    main(args.route_files, vtypes, args.random)