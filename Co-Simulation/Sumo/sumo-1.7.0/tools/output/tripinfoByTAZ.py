#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

# @file    tripinfoByTAZ.py
# @author  Jakob Erdmann
# @date    2019-01-03

"""
Aggregate tripinfo statistics for TAZ-pairs. The relationship between tripinfos
and TAZ can be obtained from route files with 'fromTaz','toTaz' attributes or
from a TAZ-file.
"""

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import argparse
from collections import defaultdict
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
from sumolib.output import parse  # noqa
from sumolib.miscutils import Statistics, parseTime  # noqa


def get_options(args=None):
    argParser = argparse.ArgumentParser()
    argParser.add_argument("-t", "--tripinfo-file", dest="tripinfoFile",
                           help="tripinfo file written by the simulation")
    argParser.add_argument("-r", "--route-files", dest="routeFiles",
                           help="demand input (trips, flows, vehicles) for reading fromTaz and toTaz info")
    argParser.add_argument("-z", "--taz-files", dest="tazFiles",
                           help="taz definitions to assign trips to taz based on departure and arrival edge")
    argParser.add_argument("-a", "--attribute", default="duration",
                           help="attribute to use for statistics")
    argParser.add_argument("-o", "--output", help="the output file")
    options = argParser.parse_args(args=args)
    options.routeFiles = options.routeFiles.split(',') if options.routeFiles else []
    options.tazFiles = options.tazFiles.split(',') if options.tazFiles else []
    if not options.tripinfoFile:
        sys.exit("Required argument --tripinfo-file is missing")
    if not (options.routeFiles or options.tazFiles):
        sys.exit("At least one  --route-files or --taz-files must be defined")
    return options


def writeTraveltimeMatrix(options):
    id2TAZ = {}  # vehicleID : (fromTaz, toTaz)
    flowIds2TAZ = {}  # flowID : (fromTaz, toTaz)
    flowIDs = set()
    sinkEdge2TAZ = {}  # edgeID : TAZ
    sourceEdge2TAZ = {}  # edgeID : TAZ
    attrs = defaultdict(lambda: ['id', 'fromTaz', 'toTaz'])
    for routeFile in options.routeFiles:
        for veh in parse(routeFile, ['trip', 'vehicle'], attrs):
            if veh.fromTaz and veh.toTaz:
                id2TAZ[veh.id] = (veh.fromTaz, veh.toTaz)
        for flow in parse(routeFile, 'flow', attrs):
            flowIDs.add(flow.id)
            if flow.fromTaz and flow.toTaz:
                flowIds2TAZ[flow.id] = (flow.fromTaz, flow.toTaz)

    for tazFile in options.tazFiles:
        for taz in parse(tazFile, 'taz'):
            sourceEdges = []
            sinkEdges = []
            if taz.edges:
                sourceEdges = taz.edges.split()
                sinkEdges = sourceEdges
            if taz.tazSource:
                for ts in taz.tazSource:
                    sourceEdges.append(ts.id)
            if taz.tazSink:
                for ts in taz.tazSink:
                    sinkEdges.append(ts.id)
            for e in sourceEdges:
                if e in sourceEdge2TAZ:
                    print("edge %s s already assigned as source for taz %s.  Reasignemnt to taz %s is not supported" % (
                        e, sourceEdge2TAZ[e], taz.id))
                else:
                    sourceEdge2TAZ[e] = taz.id
            for e in sinkEdges:
                if e in sinkEdge2TAZ:
                    print("edge %s s already assigned as sink for taz %s.  Reasignemnt to taz %s is not supported" % (
                        e, sinkEdge2TAZ[e], taz.id))
                else:
                    sinkEdge2TAZ[e] = taz.id

    odpairs = {}
    for trip in parse(options.tripinfoFile, 'tripinfo'):
        odpair = id2TAZ.get(trip.id)
        tripID = trip.id
        if odpair is None and '.' in trip.id:
            flowID = trip.id[:trip.id.rfind('.')]
            if flowID in flowIDs:
                tripID = flowID
                odpair = flowIds2TAZ.get(tripID)
        if odpair is None:
            fromEdge = trip.departLane[:trip.departLane.rfind('_')]
            toEdge = trip.arrivalLane[:trip.arrivalLane.rfind('_')]
            odpair = (sourceEdge2TAZ.get(fromEdge, '?'), sinkEdge2TAZ.get(toEdge, '?'))
        if odpair not in odpairs:
            odpairs[odpair] = Statistics(' '.join(odpair))

        odpairs[odpair].add(parseTime(getattr(trip, options.attribute)), tripID)

    if options.output:
        with open(options.output, 'w') as outf:
            outf.write('<tripinfosByTAZ attribute="%s">\n' % options.attribute)
            for (fromTaz, toTaz), stats in sorted(odpairs.items()):
                q1, median, q3 = stats.quartiles()
                outf.write('    <odInfo fromTaz="%s" toTaz="%s" count="%s" min="%s" minVeh="%s"' %
                           (fromTaz, toTaz, stats.count(), stats.min, stats.min_label))
                outf.write(' max="%s" maxVeh="%s" mean="%s" Q1="%s" median="%s" Q3="%s"/>\n' %
                           (stats.max, stats.max_label, stats.avg(), q1, median, q3))
            outf.write('</tripinfosByTAZ>\n')
    else:
        for (fromTaz, toTaz), stats in sorted(odpairs.items()):
            print(stats)


if __name__ == "__main__":
    options = get_options()
    writeTraveltimeMatrix(options)
