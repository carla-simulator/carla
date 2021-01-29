#!/usr/bin/env python3

""" Generate the default values for the SUMOActivityGen.

    Author: Lara CODECA

    This program and the accompanying materials are made available under the
    terms of the Eclipse Public License 2.0 which is available at
    http://www.eclipse.org/legal/epl-2.0.
"""

import argparse
import logging
import json
import sys
import xml.etree.ElementTree

def logs():
    """ Log init. """
    stdout_handler = logging.StreamHandler(sys.stdout)
    logging.basicConfig(handlers=[stdout_handler], level=logging.INFO,
                        format='[%(asctime)s] %(levelname)s: %(message)s',
                        datefmt='%m/%d/%Y %I:%M:%S %p')

def get_options(cmd_args=None):
    """ Argument Parser """
    parser = argparse.ArgumentParser(
        prog='generateDefaultsActivityGen.py', usage='%(prog)s [options]',
        description='Generate the default values for the SUMOActivityGen.')
    parser.add_argument('--conf', type=str, dest='conf_file', required=True,
                        help='Default configuration file.')
    parser.add_argument('--od-amitran', type=str, dest='amitran_file', required=True,
                        help='OD matrix in Amitran format.')
    parser.add_argument('--out', type=str, dest='output', required=True,
                        help='Output file.')
    parser.add_argument('--population', type=int, dest='population', default=1000,
                        help='Population: number of entities to generate.')
    return parser.parse_args(cmd_args)

class ActivitygenDefaultGenerator():
    """ Generate the default values for SUMOActivityGen. """

    def __init__(self, options):

        self._options = options
        self._config_struct = None
        self._amitran_struct = None
        self._load_configurations()
        self._load_odmatrix()
        self._generate_taz()
        self._generate_slices()

    def _load_configurations(self):
        """ Load JSON configuration file in a dict. """
        self._config_struct = json.loads(open(self._options.conf_file).read())

    def _load_odmatrix(self):
        """ Load the Amitran XML configuration file."""
        self._amitran_struct = self._parse_xml_file(self._options.amitran_file)

    def _generate_slices(self):
        """ Generate population and slices from Amitran definition."""
        population = 0.0
        for odpair in self._amitran_struct:
            population += float(odpair['amount'])

        for odpair in self._amitran_struct:
            perc = round(float(odpair['amount']) / population, 4)
            if perc <= 0:
                continue
            slice_name = '{}_{}'.format(odpair['origin'], odpair['destination'])
            self._config_struct['slices'][slice_name] = {
                'perc': perc,
                'loc_origin': odpair['origin'],
                'loc_primary': odpair['destination'],
                'activityChains': self._config_struct['slices']['default']['activityChains'],
            }

        self._config_struct['slices'].pop('default', None)
        self._config_struct['population']['entities'] = self._options.population

    def _generate_taz(self):
        """ Generate TAZ from Amitran definition. """
        for odpair in self._amitran_struct:
            self._config_struct['taz'][odpair['origin']] = [odpair['origin']]
            self._config_struct['taz'][odpair['destination']] = [odpair['destination']]

    @staticmethod
    def _parse_xml_file(xml_file):
        """ Extract all odPair info from an Amitran XML file. """
        xml_tree = xml.etree.ElementTree.parse(xml_file).getroot()
        list_xml = list()
        for child in xml_tree.iter('odPair'):
            parsed = {}
            for key, value in child.attrib.items():
                parsed[key] = value
            list_xml.append(parsed)
        return list_xml

    def save_configuration_file(self, filename):
        """ Save the configuration file. """
        logging.info("Creation of %s", filename)
        with open(filename, 'w') as outfile:
            outfile.write(json.dumps(self._config_struct, indent=4))
        logging.info("%s created.", filename)

def main(cmd_args):
    """ Generate the default values for SUMOActivityGen. """
    options = get_options(cmd_args)

    defaults = ActivitygenDefaultGenerator(options)
    defaults.save_configuration_file(options.output)

    logging.info('Done.')

if __name__ == "__main__":
    logs()
    main(sys.argv[1:])
