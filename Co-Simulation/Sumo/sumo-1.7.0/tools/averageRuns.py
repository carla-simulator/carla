#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    averageRuns.py
# @author  Jakob Erdmann
# @date    2018-08-24

"""
This runs the given configuration multiple times with different random seeds and
averages the trip statistics output
"""
from __future__ import absolute_import
from __future__ import print_function
import sys
import random

from subprocess import check_output
from optparse import OptionParser
import re

import sumolib
from sumolib.miscutils import Statistics


def getOptions(args=None):
    optParser = OptionParser()
    optParser.add_option("-n", "--num-runs", type="int", default=10, dest="numRuns", help="repeat simulation INT times")
    optParser.add_option("-s", "--seed", type="int", default=42, help="initial seed for generation random seeds")
    optParser.add_option("--sumo", help="override sumo binary")

    options, args = optParser.parse_args(args=args)
    if len(args) != 1:
        sys.exit("mandatory argument CFGFILE missing")
    options.config = args[0]
    random.seed(options.seed)
    return options


def build_retriever(key):
    reString = r" %s: ([\d.]*)" % key
    regex = re.compile(reString)
    # values should all be positive so -1 is a suitable flag
    invalidResult = -1

    def retriever(output):
        foundStats = False
        for line in output.split('\n'):
            if 'Vehicles:' in line:
                foundStats = True
            elif foundStats and key in line:
                reResult = regex.search(line)
                if reResult is None:
                    print("could not find key '%s' in output '%s'" % (key, line))
                    return invalidResult
                else:
                    return float(reResult.group(1))
        return invalidResult
    return retriever


def main(options):
    if options.sumo is None:
        SUMO = sumolib.checkBinary('sumo')
    else:
        SUMO = options.sumo

    statsRetrievers = [(Statistics("%11s" % key), build_retriever(key)) for key in [
        'Inserted',
        'Running',
        'RouteLength',
        'Duration',
        'WaitingTime',
        'TimeLoss',
        'DepartDelay',
    ]]

    for i in range(options.numRuns):
        sys.stdout.write('.')
        sys.stdout.flush()
        seed = str(random.randint(0, 2**31))
        output = check_output([SUMO, '-c', options.config,
                               '--duration-log.statistics',
                               '--no-duration-log', 'false',
                               '--seed', seed])
        for stats, retriever in statsRetrievers:
            stats.add(retriever(output), seed)
    print()
    for stats, retriever in statsRetrievers:
        print(stats)


if __name__ == "__main__":
    main(getOptions())
