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

# @file    flowFromRoutes.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2007-06-28

from __future__ import absolute_import
from __future__ import print_function
import math
import sys
import os

from xml.sax import make_parser, handler
from optparse import OptionParser

SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))

from sumolib.miscutils import uMin, uMax  # noqa
import detector  # noqa
from detector import relError  # noqa


def make_geh(interval_minutes):
    def geh(m, c):
        # GEH expects hourly flow
        m = m * 60 / interval_minutes
        c = c * 60 / interval_minutes
        """Error function for hourly traffic flow measures after Geoffrey E. Havers"""
        if m + c == 0:
            return 0
        else:
            return math.sqrt(2 * (m - c) * (m - c) / float(m + c))
    return geh


SEP = ";"


def print_record(*args, **kwargs):
    comment = '#' + SEP if kwargs.get('comment', False) else ''
    print(comment + SEP.join([repr(a) if type(a) is float else str(a) for a in args]))


class LaneMap:

    def get(self, key, default):
        return key[0:-2]


class DetectorRouteEmitterReader(handler.ContentHandler):

    def __init__(self, detFile):
        self._routes = {}
        self._detReader = detector.DetectorReader(detFile, LaneMap())
        self._edgeFlow = {}
        self._parser = make_parser()
        self._parser.setContentHandler(self)
        self._begin = None
        self._end = None
        self.minTime = uMax
        self.maxTime = uMin

    def reset(self, start, end):
        self._routes = {}
        self._edgeFlow = {}
        self._begin = 60 * start
        self._end = 60 * end

    def addRouteFlow(self, route, flow):
        for edge in self._routes[route]:
            if edge not in self._edgeFlow:
                self._edgeFlow[edge] = 0
            self._edgeFlow[edge] += flow

    def startElement(self, name, attrs):
        if name == 'route':
            if 'id' in attrs:
                self._routes[attrs['id']] = attrs['edges'].split()
        if name == 'vehicle':
            if self._begin is None or float(attrs['depart']) >= self._begin:
                self.addRouteFlow(attrs['route'], 1)
                self.minTime = min(self.minTime, float(attrs['depart']))
                self.maxTime = max(self.maxTime, float(attrs['depart']))
        if name == 'flow':
            if 'route' in attrs:
                if self._begin is None or float(attrs['begin']) >= self._begin and float(attrs['end']) <= self._end:
                    self.addRouteFlow(attrs['route'], float(attrs['number']))
                    self.minTime = min(self.minTime, float(attrs['begin']))
                    self.maxTime = max(self.maxTime, float(attrs['end']))
        if name == 'routeDistribution':
            if 'routes' in attrs:
                routes = attrs['routes'].split()
                nums = attrs['probabilities'].split()
                for r, n in zip(routes, nums):
                    self.addRouteFlow(r, float(n))

    def readDetFlows(self, flowFile, flowCol):
        if self._begin is None:
            return self._detReader.readFlows(flowFile, flow=flowCol)
        else:
            return self._detReader.readFlows(flowFile, flow=options.flowcol, time="Time",
                                             timeVal=self._begin / 60, timeMax=self._end / 60)

    def getDataIntervalMinutes(self, fallback=60):
        if self.maxTime == uMin:
            return fallback
        interval = (self.maxTime - self.minTime) / 60
        if interval == 0:
            interval = fallback
        return interval

    def clearFlows(self):
        self._detReader.clearFlows()

    def calcStatistics(self, interval, geh_threshold):
        rSum = 0.
        dSum = 0.
        sumAbsDev = 0.
        sumSquaredDev = 0.
        sumSquaredPercent = 0.
        sumGEH = 0.
        nGEHthresh = 0.
        n = 0
        geh = make_geh(interval)
        for edge, detData in self._detReader._edge2DetData.items():
            rFlow = self._edgeFlow.get(edge, 0)
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
                        sumGEH += geh(rFlow, dFlow)
                        if geh(rFlow, dFlow) < geh_threshold:
                            nGEHthresh += 1
                        n += 1
        if self._begin is not None:
            print_record('interval', self._begin, comment=True)
        print_record('avgRouteFlow', 'avgDetFlow', 'avgDev', 'RMSE', 'RMSPE', 'GEH', 'GEH%', comment=True)
        if n == 0:
            # avoid division by zero
            n = -1
        print_record(
            rSum / n,
            dSum / n,
            sumAbsDev / n,
            math.sqrt(sumSquaredDev / n),
            math.sqrt(sumSquaredPercent / n),
            sumGEH / n,
            100. * nGEHthresh / n,
            comment=True)

    def printFlows(self, includeDets, useGEH, interval):
        edgeIDCol = ["edge"] if options.edgenames else []
        timeCol = ["Time"] if options.writetime else []
        measureCol = "ratio"
        measure = relError
        if useGEH:
            measureCol = "GEH"
            measure = make_geh(interval)

        if includeDets:
            cols = ['Detector'] + edgeIDCol + timeCol + ['RouteFlow', 'DetFlow', measureCol]
        else:
            cols = ['Detector'] + edgeIDCol + timeCol + ['qPkw']
        print_record(*cols)
        output = []
        time = self._begin if self._begin is not None else 0
        for edge, detData in self._detReader._edge2DetData.items():
            detString = []
            dFlow = []
            for group in detData:
                if group.isValid:
                    detString.append(group.getName(
                        options.longnames, options.firstname))
                    dFlow.append(group.totalFlow)
            rFlow = len(detString) * [self._edgeFlow.get(edge, 0)]
            edges = len(detString) * [edge]
            if includeDets:
                output.extend(zip(detString, edges, rFlow, dFlow))
            else:
                output.extend(zip(detString, edges, rFlow))
        if includeDets:
            for group, edge, rflow, dflow in sorted(output):
                if dflow > 0 or options.respectzero:
                    edgeCol = [edge] if options.edgenames else []
                    timeCol = [time] if options.writetime else []
                    cols = [group] + edgeCol + timeCol + [rflow, dflow, measure(rflow, dflow)]
                    print_record(*cols)
        else:
            for group, edge, flow in sorted(output):
                edgeCol = [edge] if options.edgenames else []
                timeCol = [time] if options.writetime else []
                cols = [group] + edgeCol + timeCol + [flow]
                print_record(*cols)


optParser = OptionParser()
optParser.add_option("-d", "--detector-file", dest="detfile",
                     help="read detectors from FILE (mandatory)", metavar="FILE")
optParser.add_option("-r", "--routes", dest="routefile",
                     help="read routes from FILE (mandatory)", metavar="FILE")
optParser.add_option("-e", "--emitters", dest="emitfile",
                     help="read emitters from FILE (mandatory)", metavar="FILE")
optParser.add_option("-f", "--detector-flow-file", dest="flowfile",
                     help="read detector flows to compare to from FILE", metavar="FILE")
optParser.add_option("-c", "--flow-column", dest="flowcol", default="qPKW",
                     help="which column contains flows", metavar="STRING")
optParser.add_option("-z", "--respect-zero", action="store_true", dest="respectzero",
                     default=False, help="respect detectors without data (or with permanent zero) with zero flow")
optParser.add_option("-D", "--dfrouter-style", action="store_true", dest="dfrstyle",
                     default=False, help="emitter files in dfrouter style (explicit routes)")
optParser.add_option("-i", "--interval", type="int", help="aggregation interval in minutes")
optParser.add_option("--long-names", action="store_true", dest="longnames",
                     default=False, help="do not use abbreviated names for detector groups")
optParser.add_option("--first-name", action="store_true", dest="firstname",
                     default=False, help="use first id in group as representative")
optParser.add_option("--edge-names", action="store_true", dest="edgenames",
                     default=False, help="include detector group edge name in output")
optParser.add_option("-b", "--begin", type="float", default=0, help="begin time in minutes")
optParser.add_option("--end", type="float", default=None, help="end time in minutes")
optParser.add_option("--geh", action="store_true", dest="geh",
                     default=False, help="compare flows using GEH measure")
optParser.add_option("--geh-treshold", type="float", default=5, dest="geh_threshold",
                     help="report percentage of detectors below threshold")
optParser.add_option("--write-time", action="store_true", dest="writetime",
                     default=False, help="write time in output")
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
(options, args) = optParser.parse_args()
if not options.detfile or not options.routefile or not options.emitfile:
    optParser.print_help()
    sys.exit()
parser = make_parser()
if options.verbose:
    print("Reading detectors")
reader = DetectorRouteEmitterReader(options.detfile)
parser.setContentHandler(reader)
if options.interval:
    haveFlows = True
    start = options.begin  # minutes
    while ((options.end is None and haveFlows) or
            (options.end is not None and start < options.end)):
        end = start + options.interval
        if options.end is not None:
            end = min(end, options.end)
        reader.reset(start, end)
        if options.verbose:
            print("Reading routes")
        parser.parse(options.routefile)
        if options.verbose:
            print("Reading emitters")
        parser.parse(options.emitfile)
        if options.flowfile:
            if options.verbose:
                print("Reading flows")
            haveFlows = reader.readDetFlows(options.flowfile, options.flowcol)
        if haveFlows:
            reader.printFlows(bool(options.flowfile), options.geh, options.interval)
            if options.flowfile:
                reader.calcStatistics(options.interval, options.geh_threshold)
            reader.clearFlows()
        start += options.interval
else:
    if options.verbose:
        print("Reading routes")
    parser.parse(options.routefile)
    if options.verbose:
        print("Reading emitters")
    parser.parse(options.emitfile)
    if options.flowfile:
        if options.verbose:
            print("Reading flows")
        reader.readDetFlows(options.flowfile, options.flowcol)
    fallbackInterval = 60
    if options.begin and options.end:
        fallbackInterval = options.end - options.begin
    dataInterval = reader.getDataIntervalMinutes(fallbackInterval)
    reader.printFlows(bool(options.flowfile), options.geh, dataInterval)
    if options.flowfile:
        reader.calcStatistics(dataInterval, options.geh_threshold)
