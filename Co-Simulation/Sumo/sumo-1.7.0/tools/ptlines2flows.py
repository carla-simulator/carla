#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    ptlines2flows.py
# @author  Gregor Laemmel
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2017-06-23

from __future__ import print_function
import os
import sys
import codecs
import subprocess
import collections
import random

from optparse import OptionParser

import sumolib
from sumolib.xml import quoteattr

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options(args=None):
    optParser = OptionParser()
    optParser.add_option("-n", "--net-file", dest="netfile", help="network file")
    optParser.add_option("-l", "--ptlines-file", dest="ptlines", help="public transit lines file")
    optParser.add_option("-s", "--ptstops-file", dest="ptstops", help="public transit stops file")
    optParser.add_option("-o", "--output-file", dest="outfile", default="flows.rou.xml", help="output flows file")
    optParser.add_option("-i", "--stopinfos-file", dest="stopinfos",
                         default="stopinfos.xml", help="file from '--stop-output'")
    optParser.add_option(
        "-r", "--routes-file", dest="routes", default="vehroutes.xml", help="file from '--vehroute-output'")
    optParser.add_option("-t", "--trips-file", dest="trips", default="trips.trips.xml", help="output trips file")
    optParser.add_option("-p", "--period", type=float, default=600,
                         help="the default service period (in seconds) to use if none is specified in the ptlines file")
    optParser.add_option("-b", "--begin", type=float, default=0, help="start time")
    optParser.add_option("-e", "--end", type=float, default=3600, help="end time")
    optParser.add_option("--min-stops", type=int, default=2,
                         help="only import lines with at least this number of stops")
    optParser.add_option("-f", "--flow-attributes", dest="flowattrs",
                         default="", help="additional flow attributes")
    optParser.add_option("--use-osm-routes", default=False, action="store_true",
                         dest='osmRoutes', help="use osm routes")
    optParser.add_option("--extend-to-fringe", default=False, action="store_true", dest='extendFringe',
                         help="let routes of incomplete lines start/end at the network border " +
                              "if the route edges are known")
    optParser.add_option("--random-begin", default=False, action="store_true",
                         dest='randomBegin', help="randomize begin times within period")
    optParser.add_option("--seed", type="int", help="random seed")
    optParser.add_option("--ignore-errors", default=False, action="store_true",
                         dest='ignoreErrors', help="ignore problems with the input data")
    optParser.add_option("--no-vtypes", default=False, action="store_true",
                         dest='novtypes', help="do not write vtypes for generated flows")
    optParser.add_option("--types", help="only export the given list of types (using OSM nomenclature)")
    optParser.add_option("--bus.parking", default=False, action="store_true",
                         dest='busparking', help="let busses clear the road while stopping")
    optParser.add_option("--vtype-prefix", default="", dest='vtypeprefix', help="prefix for vtype ids")
    optParser.add_option("-d", "--stop-duration", default=30, type="float", dest='stopduration',
                         help="Configure the minimum stopping duration")
    optParser.add_option("-H", "--human-readable-time", dest="hrtime", default=False,
                         action="store_true", help="write times as h:m:s")
    optParser.add_option("--night", action="store_true", default=False, help="Export night service lines")
    optParser.add_option("-v", "--verbose", action="store_true", default=False, help="tell me what you are doing")
    (options, args) = optParser.parse_args(args=args)

    if options.netfile is None or options.ptlines is None or options.ptstops is None:
        sys.stderr.write("Error: net-file, ptlines-file and ptstops-file must be set\n")
        optParser.print_help()
        sys.exit(1)

    if options.begin >= options.end:
        sys.stderr.write("Error: end time must be larger than begin time\n")
        optParser.print_help()
        sys.exit(1)

    if options.types is not None:
        options.types = options.types.split(',')

    return options


def writeTypes(fout, prefix):
    print("""    <vType id="%sbus" vClass="bus"/>
    <vType id="%stram" vClass="tram"/>
    <vType id="%strain" vClass="rail"/>
    <vType id="%ssubway" vClass="rail_urban"/>
    <vType id="%slight_rail" vClass="rail_urban"/>
    <vType id="%smonorail" vClass="rail"/>
    <vType id="%strolleybus" vClass="bus"/>
    <vType id="%saerialway" vClass="bus"/>
    <vType id="%sferry" vClass="ship"/>""" % tuple([prefix] * 9), file=fout)


def getStopEdge(stopsLanes, stop):
    return stopsLanes[stop].rsplit("_", 1)[0]


def createTrips(options):
    print("generating trips...")
    stopsLanes = {}
    stopNames = {}
    for stop in sumolib.output.parse(options.ptstops, 'busStop'):
        stopsLanes[stop.id] = stop.lane
        if stop.name:
            stopNames[stop.id] = stop.attr_name

    trpMap = {}
    with codecs.open(options.trips, 'w', encoding="UTF8") as fouttrips:
        sumolib.writeXMLHeader(
            fouttrips, "$Id: ptlines2flows.py v1_3_1+0313-ccb31df3eb jakob.erdmann@dlr.de 2019-09-02 13:26:32 +0200 $",
            "routes")
        writeTypes(fouttrips, options.vtypeprefix)

        departTimes = [options.begin for line in sumolib.output.parse_fast(options.ptlines, 'ptLine', ['id'])]
        if options.randomBegin:
            departTimes = sorted([options.begin
                                  + int(random.random() * options.period) for t in departTimes])

        lineCount = collections.defaultdict(int)
        typeCount = collections.defaultdict(int)
        numLines = 0
        numStops = 0
        numSkipped = 0
        for trp_nr, line in enumerate(sumolib.output.parse(options.ptlines, 'ptLine', heterogeneous=True)):
            stop_ids = []
            if not line.hasAttribute("period"):
                line.setAttribute("period", options.period)
            if line.busStop is not None:
                for stop in line.busStop:
                    if stop.id not in stopsLanes:
                        sys.stderr.write("Warning: skipping unknown stop '%s'\n" % stop.id)
                        continue
                    laneId = stopsLanes[stop.id]
                    try:
                        edge_id, lane_index = laneId.rsplit("_", 1)
                    except ValueError:
                        if options.ignoreErrors:
                            sys.stderr.write("Warning: ignoring stop '%s' on invalid lane '%s'\n" % (stop.id, laneId))
                            continue
                        else:
                            sys.exit("Invalid lane '%s' for stop '%s'" % (laneId, stop.id))
                    stop_ids.append(stop.id)

            if options.types is not None and line.type not in options.types:
                if options.verbose:
                    print("Skipping line '%s' because it has type '%s'" % (line.id, line.type))
                numSkipped += 1
                continue

            if line.hasAttribute("nightService"):
                if line.nightService == "only" and not options.night:
                    if options.verbose:
                        print("Skipping line '%s' because because it only drives at night" % (line.id))
                    numSkipped += 1
                    continue
                if line.nightService == "no" and options.night:
                    if options.verbose:
                        print("Skipping line '%s' because because it only drives during the day" % (line.id))
                    numSkipped += 1
                    continue

            lineRefOrig = line.line.replace(" ", "_")
            lineRefOrig = lineRefOrig.replace(";", "+")
            lineRefOrig = lineRefOrig.replace(">", "")

            if len(stop_ids) < options.min_stops:
                sys.stderr.write("Warning: skipping line '%s' (%s_%s) because it has too few stops\n" % (
                    line.id, line.type, lineRefOrig))
                numSkipped += 1
                continue

            lineRef = "%s:%s" % (lineRefOrig, lineCount[lineRefOrig])
            lineCount[lineRefOrig] += 1
            tripID = "%s_%s_%s" % (trp_nr, line.type, lineRef)

            begin = departTimes[trp_nr]
            edges = []
            if line.route is not None:
                edges = line.route[0].edges.split()
            if options.osmRoutes and len(edges) == 0 and options.verbose:
                print("Cannot use OSM route for line '%s' (no edges given)" % line.id)
            elif options.osmRoutes and len(edges) > 0:
                edges = line.route[0].edges.split()
                vias = ''
                if len(edges) > 2:
                    vias = ' via="%s"' % (' '.join(edges[1:-1]))
                fouttrips.write(
                    ('    <trip id="%s" type="%s%s" depart="%s" departLane="best" from="%s" ' +
                     'to="%s"%s>\n') % (
                        tripID, options.vtypeprefix, line.type, begin, edges[0], edges[-1], vias))
            else:
                if options.extendFringe and len(edges) > len(stop_ids):
                    fr = edges[0]
                    to = edges[-1]
                    # ensure that route actually covers the terminal stops
                    # (otherwise rail network may be invalid beyond stops)
                    if len(stop_ids) > 0:
                        firstStop = getStopEdge(stopsLanes, stop_ids[0])
                        lastStop = getStopEdge(stopsLanes, stop_ids[-1])
                        if firstStop not in edges:
                            fr = firstStop
                            if options.verbose:
                                print(("Cannot extend route before first stop for line '%s' " +
                                       "(stop edge %s not in route)") % (line.id, firstStop))
                        if lastStop not in edges:
                            to = lastStop
                            if options.verbose:
                                print(("Cannot extend route after last stop for line '%s' " +
                                       "(stop edge %s not in route)") % (line.id, lastStop))
                else:
                    if options.extendFringe and options.verbose:
                        print("Cannot extend route to fringe for line '%s' (not enough edges given)" % line.id)
                    if len(stop_ids) == 0:
                        sys.stderr.write("Warning: skipping line '%s' because it has no stops\n" % line.id)
                        numSkipped += 1
                        continue
                    fr = getStopEdge(stopsLanes, stop_ids[0])
                    to = getStopEdge(stopsLanes, stop_ids[-1])
                fouttrips.write(
                    ('    <trip id="%s" type="%s%s" depart="%s" departLane="best" from="%s" ' +
                     'to="%s">\n') % (
                        tripID, options.vtypeprefix, line.type, begin, fr, to))

            trpMap[tripID] = (lineRef, line.attr_name, line.completeness, line.period)
            for stop in stop_ids:
                fouttrips.write('        <stop busStop="%s" duration="%s"/>\n' % (stop, options.stopduration))
            fouttrips.write('    </trip>\n')
            typeCount[line.type] += 1
            numLines += 1
            numStops += len(stop_ids)
        fouttrips.write("</routes>\n")
    if options.verbose:
        print("Imported %s lines with %s stops and skipped %s lines" % (numLines, numStops, numSkipped))
        for lineType, count in typeCount.items():
            print("   %s: %s" % (lineType, count))
    print("done.")
    return trpMap, stopNames


def runSimulation(options):
    print("running SUMO to determine actual departure times...")
    subprocess.call([sumolib.checkBinary("sumo"),
                     "-n", options.netfile,
                     "-r", options.trips,
                     "--begin", str(options.begin),
                     "--no-step-log",
                     "--ignore-route-errors",
                     "--error-log", options.trips + ".errorlog",
                     "-a", options.ptstops,
                     "--device.rerouting.adaptation-interval", "0",  # ignore tls and traffic effects
                     "--vehroute-output", options.routes,
                     "--stop-output", options.stopinfos, ])
    print("done.")
    sys.stdout.flush()


def formatTime(seconds):
    return "%02i:%02i:%02i" % (seconds / 3600, (seconds % 3600) / 60, (seconds % 60))


def createRoutes(options, trpMap, stopNames):
    print("creating routes...")
    stopsUntil = collections.defaultdict(list)
    for stop in sumolib.output.parse_fast(options.stopinfos, 'stopinfo', ['id', 'ended', 'busStop']):
        stopsUntil[(stop.id, stop.busStop)].append(float(stop.ended))

    ft = formatTime if options.hrtime else lambda x: x

    with codecs.open(options.outfile, 'w', encoding="UTF8") as foutflows:
        flows = []
        actualDepart = {}  # departure may be delayed when the edge is not yet empty
        sumolib.writeXMLHeader(
            foutflows, "$Id: ptlines2flows.py v1_3_1+0313-ccb31df3eb jakob.erdmann@dlr.de 2019-09-02 13:26:32 +0200 $",
            "routes")
        if not options.novtypes:
            writeTypes(foutflows, options.vtypeprefix)
        collections.defaultdict(int)
        for vehicle in sumolib.output.parse(options.routes, 'vehicle'):
            id = vehicle.id
            lineRef, name, completeness, period = trpMap[id]
            flowID = "%s_%s" % (vehicle.type, lineRef)
            try:
                if vehicle.route is not None:
                    edges = vehicle.route[0].edges
                else:
                    edges = vehicle.routeDistribution[0].route[1].edges
            except BaseException:
                if options.ignoreErrors:
                    sys.stderr.write("Warning: Could not parse edges for vehicle '%s'\n" % id)
                    continue
                else:
                    sys.exit("Could not parse edges for vehicle '%s'\n" % id)
            flows.append((id, flowID, lineRef, vehicle.type, float(vehicle.depart)))
            actualDepart[id] = float(vehicle.depart)
            parking = ' parking="true"' if vehicle.type == "bus" and options.busparking else ''
            stops = vehicle.stop
            foutflows.write('    <route id="%s" edges="%s" >\n' % (flowID, edges))
            if vehicle.stop is not None:
                for stop in stops:
                    if (id, stop.busStop) in stopsUntil:
                        until = stopsUntil[(id, stop.busStop)]
                        stopname = ' <!-- %s -->' % stopNames[stop.busStop] if stop.busStop in stopNames else ''
                        untilZeroBased = until[0] - actualDepart[id]
                        if len(until) > 1:
                            stopsUntil[(id, stop.busStop)] = until[1:]
                        foutflows.write(
                            '        <stop busStop="%s" duration="%s" until="%s"%s/>%s\n' % (
                                stop.busStop, stop.duration, ft(untilZeroBased), parking, stopname))
                    else:
                        sys.stderr.write("Warning: Missing stop '%s' for flow '%s'\n" % (stop.busStop, id))
            else:
                sys.stderr.write("Warning: No stops for flow '%s'\n" % id)
            foutflows.write('    </route>\n')
        flow_duration = options.end - options.begin
        for vehID, flowID, lineRef, type, begin in flows:
            line, name, completeness, period = trpMap[vehID]
            foutflows.write('    <flow id="%s" type="%s" route="%s" begin="%s" end="%s" period="%s" line="%s" %s>\n' % (
                flowID, type, flowID, ft(begin), ft(begin + flow_duration),
                int(float(period)), lineRef, options.flowattrs))
            if name is not None:
                foutflows.write('        <param key="name" value=%s/>\n' % quoteattr(name))
            if completeness is not None:
                foutflows.write('        <param key="completeness" value=%s/>\n' % quoteattr(completeness))
            foutflows.write('    </flow>\n')
        foutflows.write('</routes>\n')

    print("done.")


def main(options):
    if options.seed:
        random.seed(options.seed)
    sys.stderr.flush()
    trpMap, stopNames = createTrips(options)
    sys.stderr.flush()
    runSimulation(options)
    createRoutes(options, trpMap, stopNames)


if __name__ == "__main__":
    main(get_options())
