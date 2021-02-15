#!/usr/bin/env python3

""" From the Tripinfo file, generate the activities report.

    Author: Lara CODECA

    This program and the accompanying materials are made available under the
    terms of the Eclipse Public License 2.0 which is available at
    http://www.eclipse.org/legal/epl-2.0.
"""

import argparse
import collections
import json
import logging
import os
from pprint import pformat
import sys

from lxml import etree
import numpy as np

def logs():
    """ Log init. """
    stdout_handler = logging.StreamHandler(sys.stdout)
    logging.basicConfig(handlers=[stdout_handler], level=logging.INFO,
                        format='[%(asctime)s] %(levelname)s: %(message)s',
                        datefmt='%m/%d/%Y %I:%M:%S %p')

def get_options(cmd_args=None):
    """ Argument Parser. """
    parser = argparse.ArgumentParser(
        prog='{}'.format(sys.argv[0]), usage='%(prog)s [options]',
        description='SAGA Live Monitoring')
    parser.add_argument(
        '--tripinfo', type=str, required=True, 
        help='SUMO TripInfo file (XML).')
    parser.add_argument(
        '--out', type=str, required=True, 
        help='Output file (CSV).')
    return parser.parse_args(cmd_args)

class SAGAReport(object):
    """ SAGA Activities Report """

    TRIPINFO_SCHEMA = os.path.join(os.environ['SUMO_HOME'], 'data/xsd/tripinfo_file.xsd')

    def __init__(self, cfg):
        self.tripinfo_file = cfg.tripinfo
        self.output_file = cfg.out

        self.tripinfo = collections.defaultdict(dict)
        self.personinfo = collections.defaultdict(dict)

        self.activity_stats = collections.defaultdict(list)

    def loadTripinfo(self):
        # just in case..
        self.tripinfo = collections.defaultdict(dict)
        self.personinfo = collections.defaultdict(dict)

        tree = None
        try:
            # Faster, but it may fail.
            schema = etree.XMLSchema(file=self.TRIPINFO_SCHEMA)
            parser = etree.XMLParser(schema=schema)
            tree = etree.parse(self.tripinfo_file, parser)
        except etree.XMLSyntaxError as excp:
            logging.warning('Unable to use %s schema due to exception %s.', 
                            self.TRIPINFO_SCHEMA, pformat(excp))
            tree = etree.parse(self.tripinfo_file)

        logging.info('Loading %s tripinfo file.', self.tripinfo_file)
        for element in tree.getroot():
            if element.tag == 'tripinfo':
                self.tripinfo[element.attrib['id']] = dict(element.attrib)
            elif element.tag == 'personinfo':
                self.personinfo[element.attrib['id']] = dict(element.attrib)
                stages = []
                for stage in element:
                    stages.append([stage.tag, dict(stage.attrib)])
                self.personinfo[element.attrib['id']]['stages'] = stages
            else:
                raise Exception('Unrecognized element in the tripinfo file.')
        logging.debug('TRIPINFO: \n%s', pformat(self.tripinfo))
        logging.debug('PERSONINFO: \n%s', pformat(self.personinfo))

    def processTripinfo(self):
        logging.info('Processing %s tripinfo file.', self.tripinfo_file)
        for person, data in self.personinfo.items():
            for tag, stage in data['stages']:
                logging.debug('[%s] %s \n%s', person, tag, pformat(stage))
                if tag == 'stop':
                    self.activity_stats[stage['actType']].append({
                        'arrival': stage['arrival'],
                        'duration': stage['duration'],
                    })

    def computeStats(self):
        logging.info('Computing statistics..')
        stats = dict()
        for activity, data in self.activity_stats.items():
            duration = list()
            start = list()
            for value in data:
                start.append(float(value['arrival']) - float(value['duration']))
                duration.append(float(value['duration']))
            stats[activity] = {
                'duration': {
                    'min': min(duration),
                    'max': max(duration),
                    'mean': np.mean(duration),
                    'median': np.median(duration),
                    'std': np.std(duration),
                },
                'start': {
                    'min': min(start),
                    'max': max(start),
                    'mean': np.mean(start),
                    'median': np.median(start),
                    'std': np.std(start),
                }
            }
            logging.info('[%s] \n%s', activity, pformat(stats[activity]))

        logging.info('Saving statistics to %s file.', self.output_file)
        with open(self.output_file, 'w') as output:
            json.dump(stats, output)

def main(cmd_args):
    """ SAGA Activities Report """

    args = get_options(cmd_args)
    logging.debug('%s', args)

    report = SAGAReport(args)
    report.loadTripinfo()
    report.processTripinfo()
    report.computeStats()
    logging.info('Done.')

if __name__ == '__main__':
    logs()
    main(sys.argv[1:])
