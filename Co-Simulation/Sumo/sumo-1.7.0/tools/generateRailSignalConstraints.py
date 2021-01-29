#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    generateRailSignalConstraints.py
# @author  Jakob Erdmann
# @date    2020-08-31

"""
Generate railSignalConstrains definitions that enforce a loaded rail schedule

The schedule can either be based on 'arrival' or 'until' time of stops.
Vehicles that pass the same signal will be sorted based on their departure from
previous stops or their arrival on a subsequent stop.
"""

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import subprocess
from collections import defaultdict

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa
from sumolib.miscutils import parseTime  # noqa

DUAROUTER = sumolib.checkBinary('duarouter')


def get_options(args=None):
    parser = sumolib.options.ArgumentParser(description="Sample routes to match counts")
    parser.add_argument("-n", "--net-file", dest="netFile",
                        help="Input network file")
    parser.add_argument("-a", "--additional-file", dest="addFile",
                        help="Input additional file (busStops)")
    parser.add_argument("-t", "--trip-file", dest="tripFile",
                        help="Input trip file (will be processed into a route file)")
    parser.add_argument("-r", "--route-file", dest="routeFile",
                        help="Input route file (must contain routed vehicles rather than trips)")
    parser.add_argument("-o", "--output-file", dest="out", default="constraints.add.xml",
                        help="Output additional file")
    parser.add_argument("--arrivals", action="store_true", default=False,
                        help="Use stop arrival time instead of 'until' time for sorting")

    options = parser.parse_args(args=args)
    if (options.routeFile is None and options.tripFile is None) or options.netFile is None:
        parser.print_help()
        sys.exit()

    if options.routeFile is None:
        options.routeFile = options.tripFile + ".rou.xml"
        args = [DUAROUTER, '-n', options.netFile,
                '-r', options.tripFile,
                '-a', options.addFile,
                '-o', options.routeFile,
                '--ignore-errors', '--no-step-log',
                ]
        print("calling", " ".join(args))
        sys.stdout.flush()
        subprocess.call(args)
        sys.stdout.flush()

    return options


def findMergingSwitches(options, net):
    # find switches where routes merge and thus conflicts must be solved
    predEdges = defaultdict(set)
    numRoutes = 0
    for route in sumolib.xml.parse_fast(options.routeFile, 'route', ['edges']):
        numRoutes += 1
        edges = route.edges.split()
        for i, edge in enumerate(edges):
            if i > 0:
                pred = edges[i - 1]
                if net.getEdge(edge).getBidi() != net.getEdge(pred):
                    predEdges[edge].add(pred)

    mergeSwitches = set()
    for edge, preds in predEdges.items():
        if len(preds) > 1:
            print(edge, preds)
            mergeSwitches.add(edge)

    print("read %s routes across %s edges with %s merging switches" % (
        numRoutes, len(predEdges), len(mergeSwitches)))


def main(options):
    net = sumolib.net.readNet(options.netFile)
    findMergingSwitches(options, net)


if __name__ == "__main__":
    main(get_options())
