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

# @file    flowFromEdgeData.py
# @author  Jakob Erdmann
# @date    2017-11-27

from __future__ import absolute_import
from __future__ import print_function
import math
import sys
import os

from optparse import OptionParser
from collections import defaultdict

import detector
from detector import relError

SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa
from sumolib.xml import parse  # noqa
from sumolib.miscutils import parseTime  # noqa
DEBUG = False


def get_options(args=None):
    optParser = OptionParser()
    optParser.add_option("-d", "--detector-file", dest="detfile",
                         help="read detectors from FILE (mandatory)", metavar="FILE")
    optParser.add_option("-e", "--edgedata-file", dest="edgeDataFile",
                         help="read edgeData from FILE (mandatory)", metavar="FILE")
    optParser.add_option("-f", "--detector-flow-file", dest="flowfile",
                         help="read detector flows to compare to from FILE (mandatory)", metavar="FILE")
    optParser.add_option("-c", "--flow-column", dest="flowcol", default="qPKW",
                         help="which column contains flows", metavar="STRING")
    optParser.add_option("-z", "--respect-zero", action="store_true", dest="respectzero",
                         default=False, help="respect detectors without data (or with permanent zero) with zero flow")
    optParser.add_option("-i", "--interval", type="int", default="1440", help="aggregation interval in minutes")
    optParser.add_option("--long-names", action="store_true", dest="longnames",
                         default=False, help="do not use abbreviated names for detector groups")
    optParser.add_option("--edge-names", action="store_true", dest="edgenames",
                         default=False, help="include detector group edge name in output")
    optParser.add_option("-b", "--begin", type="int", default=0, help="begin time in minutes")
    optParser.add_option("--end", type="int", default=None, help="end time in minutes")
    optParser.add_option("-o", "--output-file", dest="output",
                         help="write output to file instead of printing it to console", metavar="FILE")
    optParser.add_option("--flow-output", dest="flowout",
                         help="write output in flowfile format to FILE", metavar="FILE")
    optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                         default=False, help="tell me what you are doing")
    (options, args) = optParser.parse_args(args=args)
    if not options.detfile or not options.edgeDataFile or (not options.flowfile and
                                                           not options.flowout):
        optParser.print_help()
        sys.exit()

    return options


def readEdgeData(edgeDataFile, begin, end, detReader, flowout):
    edgeFlow = defaultdict(lambda: 0)
    for interval in parse(edgeDataFile, "interval", attr_conversions={"begin": parseTime, "end": parseTime}):
        if DEBUG:
            print("reading intervals for begin=%s end=%s (current interval begin=%s end=%s)" %
                  (begin, end, interval.begin, interval.end))
        if interval.begin < end and interval.end > begin:
            # if read interval is partly outside comparison interval we must scale demand
            validInterval = interval.end - interval.begin
            if interval.begin < begin:
                validInterval -= begin - interval.begin
            if interval.end > end:
                validInterval -= interval.end - end
            scale = validInterval / (interval.end - interval.begin)
            # store data
            if flowout:
                f = open(flowout, 'a')
            if interval.edge is None:
                continue
            for edge in interval.edge:
                flow = (int(edge.departed) + int(edge.entered)) * scale
                edgeFlow[edge.id] += flow
                # print(interval.begin, interval.end, edge.id, edge.departed, edge.entered, scale, edgeFlow[edge.id])
                if flowout:
                    for group in detReader.getEdgeDetGroups(edge.id):
                        f.write(";".join(map(str, [group.ids[0], interval.begin / 60,
                                                   flow, edge.speed])) + "\n")
            if flowout:
                f.close()
            if DEBUG:
                print("    validInterval=%s scale=%s" % (validInterval, scale))
    return edgeFlow


def printFlows(options, edgeFlow, detReader):
    edgeIDCol = "edge " if options.edgenames else ""
    print('# detNames %sRouteFlow DetFlow ratio' % edgeIDCol, file=options.outfile)
    output = []
    for edge, detData in detReader._edge2DetData.items():
        detString = []
        dFlow = []
        for group in detData:
            if group.isValid:
                groupName = os.path.commonprefix(group.ids)
                if groupName == "" or options.longnames:
                    groupName = ';'.join(sorted(group.ids))
                detString.append(groupName)
                dFlow.append(group.totalFlow)
        rFlow = len(detString) * [edgeFlow.get(edge, 0)]
        edges = len(detString) * [edge]
        output.extend(zip(detString, edges, rFlow, dFlow))
    for group, edge, rflow, dflow in sorted(output):
        if dflow > 0 or options.respectzero:
            if options.edgenames:
                print(group, edge, repr(rflow), repr(dflow), repr(relError(rflow, dflow)), file=options.outfile)
            else:
                print(group, repr(rflow), repr(dflow), repr(relError(rflow, dflow)), file=options.outfile)


def calcStatistics(options, begin, edgeFlow, detReader):
    rSum = 0.
    dSum = 0.
    sumAbsDev = 0.
    sumSquaredDev = 0.
    sumSquaredPercent = 0.
    n = 0
    for edge, detData in detReader._edge2DetData.items():
        rFlow = edgeFlow.get(edge, 0)
        for group in detData:
            if group.isValid:
                dFlow = group.totalFlow
                if dFlow > 0 or options.respectzero:
                    rSum += rFlow
                    dSum += dFlow
                    dev = float(abs(rFlow - dFlow))
                    sumAbsDev += dev
                    sumSquaredDev += dev * dev
                    if dFlow > 0:
                        sumSquaredPercent += dev * dev / dFlow / dFlow
                    n += 1
    print('# interval', begin, file=options.outfile)
    print('# avgRouteFlow avgDetFlow avgDev RMSE RMSPE', file=options.outfile)
    if n == 0:
        # avoid division by zero
        n = -1
    print('#', " ".join(map(repr, [rSum / n, dSum / n, sumAbsDev / n,
                                   math.sqrt(sumSquaredDev / n), math.sqrt(sumSquaredPercent / n)])),
          file=options.outfile)


class LaneMap:
    def get(self, key, default):
        return key[0:-2]


def main(options):
    if options.output is None:
        options.outfile = sys.stdout
    else:
        options.outfile = open(options.output, 'w')
    if options.flowout:
        with open(options.flowout, 'w') as f:
            f.write("Detector;Time;qPKW;vPKW\n")
        options.begin = None
        for interval in parse(options.edgeDataFile, "interval", attr_conversions={"begin": float, "end": float}):
            if options.begin is None:
                options.begin = interval.begin / 60
            options.end = interval.end / 60

    detReader = detector.DetectorReader(options.detfile, LaneMap())
    intervalBeginM = options.begin
    haveDetFlows = True
    while ((options.end is None and haveDetFlows) or
            (options.end is not None and intervalBeginM < options.end)):
        intervalEndM = intervalBeginM + options.interval
        if options.end is not None:
            intervalEndM = min(intervalEndM, options.end)
        if options.flowfile:
            if options.verbose:
                print("Reading flows")
            haveDetFlows = detReader.readFlows(options.flowfile, flow=options.flowcol,
                                               time="Time", timeVal=intervalBeginM, timeMax=intervalEndM)
        if options.verbose:
            print("Reading edgeData")
        edgeFlow = readEdgeData(
            options.edgeDataFile, intervalBeginM * 60, intervalEndM * 60,
            detReader, options.flowout)
        if haveDetFlows and options.flowfile:
            printFlows(options, edgeFlow, detReader)
            calcStatistics(options, intervalBeginM, edgeFlow, detReader)
            detReader.clearFlows()
        intervalBeginM += options.interval
    options.outfile.close()


if __name__ == "__main__":
    main(get_options())
