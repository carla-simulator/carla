#!/usr/bin/env python3

""" Generate the default Amitran OD-matrix from TAZ weights.

    Author: Lara CODECA

    This program and the accompanying materials are made available under the
    terms of the Eclipse Public License 2.0 which is available at
    http://www.eclipse.org/legal/epl-2.0.
"""

import argparse
import csv
import logging
import sys

def logs():
    """ Log init. """
    stdout_handler = logging.StreamHandler(sys.stdout)
    logging.basicConfig(handlers=[stdout_handler], level=logging.WARN,
                        format='[%(asctime)s] %(levelname)s: %(message)s',
                        datefmt='%m/%d/%Y %I:%M:%S %p')

def get_options(cmd_args=None):
    """ Argument Parser """
    parser = argparse.ArgumentParser(
        prog='generateAmitranFromTAZWeights.py', usage='%(prog)s [options]',
        description='Generate the default Amitran OD-matrix from TAZ weights.')
    parser.add_argument('--taz-weights', type=str, dest='taz_file', required=True,
                        help='Weighted TAZ file (CSV).')
    parser.add_argument('--out', type=str, dest='output', required=True,
                        help='OD matrix in Amitran format.')
    parser.add_argument('--density', type=float, dest='density', default=3000.0,
                        help='Average population density in square kilometers.')
    return parser.parse_args(cmd_args)

class AmitranFromTAZWeightsGenerator():
    """ Generate the default Amitran OD-matrix from TAZ weights. """

    def __init__(self, options):
        self._options = options
        self._taz_weights = dict()
        self._odpairs = list()
        self._load_weights_from_csv()
        self._generate_odpairs_from_taz()

    def _load_weights_from_csv(self):
        """ Load the TAZ weight from a CSV file. """
        with open(self._options.taz_file, 'r') as csvfile:
            weightreader = csv.reader(csvfile)
            header = None
            for row in weightreader:
                if not header:
                    header = row
                elif row: # ignoring empty lines
                    self._taz_weights[row[0]] = {
                        header[0]: row[0],
                        header[1]: row[1],
                        header[2]: int(row[2]),
                        header[3]: float(row[3]),
                    }

    def _generate_odpairs_from_taz(self):
        """ Generate all the possible OD pairs. """
        _single_taz = (len(self._taz_weights) == 1)
        for origin, taz_orig in self._taz_weights.items():
            for destination, _ in self._taz_weights.items():
                if origin == destination and not _single_taz:
                    continue
                amount = round(
                    self._options.density * taz_orig['Area'] / 1e6,     # from mq to square kmq
                    0)
                if amount <= 0:
                    continue
                self._odpairs.append({
                    'origin': origin,
                    'destination': destination,
                    'amount': amount,
                })

    AMITRAN_TPL = """<demand xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="https://sumo.dlr.de/xsd/amitran/od.xsd">
    <actorConfig id="0">
        <timeSlice duration="86400000" startTime="0">{odpair}
        </timeSlice>
    </actorConfig>
</demand>
"""

    ODPAIR_TPL = """
            <odPair amount="{amount}" destination="{dest}" origin="{orig}"/>"""

    def save_odmatrix_to_file(self, filename):
        """ Save the OD-matric in Amitran format. """
        logging.info("Creation of %s", filename)
        with open(filename, 'w') as outfile:
            list_of_odpairs = ''
            for pair in self._odpairs:
                list_of_odpairs += self.ODPAIR_TPL.format(amount=int(pair['amount']),
                                                          dest=pair['destination'],
                                                          orig=pair['origin'])
            outfile.write(self.AMITRAN_TPL.format(odpair=list_of_odpairs))
        logging.info("%s created.", filename)

def main(cmd_args):
    """ Generate the default Amitran OD-matrix from TAZ weights. """
    options = get_options(cmd_args)

    odmatrix = AmitranFromTAZWeightsGenerator(options)
    odmatrix.save_odmatrix_to_file(options.output)

    logging.info('Done.')

if __name__ == "__main__":
    logs()
    main(sys.argv[1:])
