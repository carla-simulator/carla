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

# @file    computeCoordination.py
# @author  Daniel Wesemeyer
# @author  Jakob Erdmann
# @date    2018-08-18

"""
This script analyses fcd output to compute the coordination factor for a given corridor
The coordination factor is defined as the fraction of vehicles that passed the
corridor without stopping to the total number of vehicles on the corridor
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from collections import defaultdict
import math  # noqa

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa
from sumolib.xml import parse_fast_nested  # noqa
from sumolib.miscutils import parseTime  # noqa


def getOptions(args=None):
    argParser = sumolib.options.ArgumentParser()
    argParser.add_argument("-f", "--fcd-file", dest="fcdfile",
                           help="Input fcd file (mandatory)")
    argParser.add_argument("-m", "--min-speed", dest="minspeed", type=float, default=5,
                           help="Minimum speed to consider vehicle undelayed")
    argParser.add_argument("--filter-route", dest="filterRoute",
                           help="only consider vehicles that passed the given list of edges in order " +
                                "(regardless of gaps)")
    argParser.add_argument("--entry",
                           help="detect delay after vehicles have passend one of the entry edges " +
                                "(defaults to the first edge of the route)")
    argParser.add_argument("--exit",
                           help="detect delay until vehicles have passend one of the exit edges " +
                                "(defaults to the last edge of the route)")
    argParser.add_argument("--full-output", dest="fullOutput",
                           help="For each vehicle that applies, write the time when it entered the corridor " +
                                "and the time when it was first delayed (-1 for undelayed)")
    options = argParser.parse_args()

    if options.fcdfile is None:
        sys.exit("mandatory argument FCD_FILE missing")

    if options.filterRoute is not None:
        options.filterRoute = options.filterRoute.split(',')
    else:
        options.filterRoute = []

    if options.entry is not None:
        options.entry = options.entry.split(',')
    elif options.filterRoute:
        options.entry = [options.filterRoute[0]]

    if options.exit is not None:
        options.exit = options.exit.split(',')
    elif options.filterRoute:
        options.exit = [options.filterRoute[-1]]

    return options


def main(options):

    routes = defaultdict(list)  # vehID -> recorded edges
    minSpeed = defaultdict(lambda: 1000)
    active = set()  # vehicles that have passed the first filterRoute edge
    entryTime = {}  # vehID -> time when entering corridor
    delayTime = {}  # vehID -> time when vehicle speed first dropped below threshold
    for timestep, vehicle in parse_fast_nested(options.fcdfile, 'timestep', ['time'],
                                               'vehicle', ['id', 'speed', 'lane']):
        time = parseTime(timestep.time)
        vehID = vehicle.id
        edge = vehicle.lane[0:vehicle.lane.rfind('_')]
        prevEdge = None if len(routes[vehID]) == 0 else routes[vehID][-1]
        if prevEdge != edge:
            if options.exit and prevEdge in options.exit:
                # vehicle has left the filtered corridor
                continue
            routes[vehID].append(edge)
        if vehID not in active:
            if not options.entry or edge in options.entry:
                # vehicle has entered the filtered corridor
                active.add(vehID)
                entryTime[vehID] = time
            else:
                continue
        speed = float(vehicle.speed)
        if speed < minSpeed[vehID]:
            minSpeed[vehID] = speed
            if speed < options.minspeed:
                delayTime[vehID] = time

    vehs = []
    numDelayed = 0

    for vehID, route in routes.items():
        skip = False
        for required in options.filterRoute:
            if required not in route:
                skip = True
                break
        if not skip:
            if minSpeed[vehID] < options.minspeed:
                numDelayed += 1
                vehs.append((entryTime[vehID], delayTime[vehID], vehID))
            else:
                vehs.append((entryTime[vehID], -1, vehID))

    vehs.sort()
    n = len(vehs)
    print("n=%s d=%s coordinationFactor=%.2f" % (n, numDelayed, (n - numDelayed) / float(n)))

    if options.fullOutput:
        with open(options.fullOutput, 'w') as outf:
            outf.write("# entryTime delayTime vehID\n")
            for record in vehs:
                outf.write(" ".join(map(str, record)) + "\n")


if __name__ == "__main__":
    main(getOptions())
