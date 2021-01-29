#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    implausibleRoutes.py
# @author  Jakob Erdmann
# @date    2017-03-28

"""
Find routes that are implausible due to:
 - being longer than the shortest path between the first and last edge
 - being longer than the air-distance between the first and the last edge

The script computes an implausibility-score from configurable factors and
reports all routes above the specified threshold.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
from optparse import OptionParser
import subprocess

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import sumolib  # noqa
    from sumolib.xml import parse, parse_fast_nested  # noqa
    from sumolib.net import readNet  # noqa
    from sumolib.miscutils import Statistics, euclidean, Colorgen  # noqa
    from route2poly import generate_poly  # noqa
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options():
    USAGE = "Usage %prog [options] <net.xml> <rou.xml>"
    optParser = OptionParser(usage=USAGE)
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="Give more output")
    optParser.add_option("--threshold", type="float", default=2.5,
                         help="Routes with an implausibility-score above treshold are reported")
    optParser.add_option("--airdist-ratio-factor", type="float", default=1, dest="airdist_ratio_factor",
                         help="Implausibility factor for the ratio of routeDist/airDist ")
    optParser.add_option("--detour-ratio-factor", type="float", default=1, dest="detour_ratio_factor",
                         help="Implausibility factor for the ratio of routeDuration/shortestDuration ")
    optParser.add_option("--detour-factor", type="float", default=0.01, dest="detour_factor",
                         help="Implausibility factor for the absolute detour time in (routeDuration-shortestDuration)" +
                              " in seconds")
    optParser.add_option("--min-dist", type="float", default=0, dest="min_dist",
                         help="Minimum shortest-path distance below which routes are implausible")
    optParser.add_option("--min-air-dist", type="float", default=0, dest="min_air_dist",
                         help="Minimum air distance below which routes are implausible")
    optParser.add_option("--standalone", action="store_true",
                         default=False, help="Parse stand-alone routes that are not define as child-element of " +
                                             "a vehicle")
    optParser.add_option("--blur", type="float", default=0,
                         help="maximum random disturbance to output polygon geometry")
    optParser.add_option("--ignore-routes", dest="ignore_routes",
                         help="List of route IDs (one per line) that are filtered when generating polygons and " +
                              "command line output (they will still be added to restrictions-output)")
    optParser.add_option("--restriction-output", dest="restrictions_output",
                         help="Write flow-restriction output suitable for passing to flowrouter.py to FILE")
    optParser.add_option("--od-restrictions", action="store_true", dest="odrestrictions",
                         default=False, help="Write restrictions for origin-destination relations rather than " +
                                             "whole routes")
    optParser.add_option("--edge-loops", action="store_true",
                         default=False, help="report routes which use edges twice")
    optParser.add_option("--node-loops", action="store_true",
                         default=False, help="report routes which use junctions twice")
    optParser.add_option("--threads", default=1, type=int,
                         help="number of threads to use for duarouter")
    optParser.add_option("--min-edges", default=2, type=int,
                         help="number of edges a route needs to have to be analyzed")
    optParser.add_option("--heterogeneous", action="store_true",
                         default=False, help="Use slow parsing for route files with different formats in one file")
    optParser.add_option("--reuse-routing", action="store_true",
                         default=False, help="do not run duarouter again if output file exists")
    options, args = optParser.parse_args()

    if len(args) < 2:
        sys.exit(USAGE)
    options.network = args[0]
    options.routeFiles = args[1:]
    # options for generate_poly
    options.layer = 100
    options.geo = False
    options.internal = False
    options.spread = None

    return options


class RouteInfo:
    def __init__(self, route):
        self.edges = route.edges.split()


def calcDistAndLoops(rInfo, net, options):
    if net.hasInternal:
        rInfo.airDist = euclidean(
            net.getEdge(rInfo.edges[0]).getShape()[0],
            net.getEdge(rInfo.edges[-1]).getShape()[-1])
    else:
        rInfo.airDist = euclidean(
            net.getEdge(rInfo.edges[0]).getFromNode().getCoord(),
            net.getEdge(rInfo.edges[-1]).getToNode().getCoord())
    rInfo.length = sumolib.route.getLength(net, rInfo.edges)
    rInfo.airDistRatio = rInfo.length / rInfo.airDist
    rInfo.edgeLoop = False
    rInfo.nodeLoop = False
    if options.edge_loops:
        seen = set()
        for e in rInfo.edges:
            if e in seen:
                rInfo.edgeLoop = True
                rInfo.nodeLoop = True
                break
            seen.add(e)
    if options.node_loops and not rInfo.nodeLoop:
        seen = set()
        for e in rInfo.edges:
            t = net.getEdge(e).getToNode()
            if t in seen:
                rInfo.nodeLoop = True
                break
            seen.add(t)


def addOrSkip(routeInfos, skipped, rid, route, min_edges):
    ri = RouteInfo(route)
    if len(ri.edges) >= min_edges:
        routeInfos[rid] = ri
    else:
        skipped.add(rid)


def main():
    options = get_options()
    if options.verbose:
        print("parsing network from", options.network)
    net = readNet(options.network, withInternal=True)
    read = 0
    routeInfos = {}  # id-> RouteInfo
    skipped = set()
    for routeFile in options.routeFiles:
        if options.verbose:
            print("parsing routes from", routeFile)
        idx = 0
        if options.standalone:
            for idx, route in enumerate(parse(routeFile, 'route')):
                if options.verbose and idx > 0 and idx % 100000 == 0:
                    print(idx, "routes read")
                addOrSkip(routeInfos, skipped, route.id, route, options.min_edges)
        else:
            if options.heterogeneous:
                for idx, vehicle in enumerate(parse(routeFile, 'vehicle')):
                    if options.verbose and idx > 0 and idx % 100000 == 0:
                        print(idx, "vehicles read")
                    addOrSkip(routeInfos, skipped, vehicle.id, vehicle.route[0], options.min_edges)
            else:
                prev = (None, None)
                for vehicle, route in parse_fast_nested(routeFile, 'vehicle', 'id', 'route', 'edges'):
                    if prev[0] != vehicle.id:
                        if options.verbose and idx > 0 and idx % 500000 == 0:
                            print(idx, "vehicles read")
                        if prev[0] is not None:
                            addOrSkip(routeInfos, skipped, prev[0], prev[1], options.min_edges)
                        prev = (vehicle.id, route)
                        idx += 1
                if prev[0] is not None:
                    addOrSkip(routeInfos, skipped, prev[0], prev[1], options.min_edges)
        read += idx
    if options.verbose:
        print(read, "routes read", len(skipped), "short routes skipped")

    if options.verbose:
        print("calculating air distance and checking loops")
    for idx, ri in enumerate(routeInfos.values()):
        if options.verbose and idx > 0 and idx % 100000 == 0:
            print(idx, "routes checked")
        calcDistAndLoops(ri, net, options)

    prefix = os.path.commonprefix(options.routeFiles)
    duarouterOutput = prefix + '.rerouted.rou.xml'
    duarouterAltOutput = prefix + '.rerouted.rou.alt.xml'
    if os.path.exists(duarouterAltOutput) and options.reuse_routing:
        if options.verbose:
            print("reusing old duarouter file", duarouterAltOutput)
    else:
        if options.standalone:
            duarouterInput = prefix
            # generate suitable input file for duarouter
            duarouterInput += ".vehRoutes.xml"
            with open(duarouterInput, 'w') as outf:
                outf.write('<routes>\n')
                for rID, rInfo in routeInfos.items():
                    outf.write('    <vehicle id="%s" depart="0">\n' % rID)
                    outf.write('        <route edges="%s"/>\n' % ' '.join(rInfo.edges))
                    outf.write('    </vehicle>\n')
                outf.write('</routes>\n')
        else:
            duarouterInput = ",".join(options.routeFiles)

        command = [sumolib.checkBinary('duarouter'), '-n', options.network,
                   '-r', duarouterInput, '-o', duarouterOutput,
                   '--no-step-log', '--routing-threads', str(options.threads),
                   '--routing-algorithm', 'astar', '--aggregate-warnings',  '1']
        if options.verbose:
            command += ["-v"]
        if options.verbose:
            print("calling duarouter:", " ".join(command))
        subprocess.call(command)

    for vehicle in parse(duarouterAltOutput, 'vehicle'):
        if vehicle.id in skipped:
            continue
        routeAlts = vehicle.routeDistribution[0].route
        if len(routeAlts) == 1:
            routeInfos[vehicle.id].detour = 0
            routeInfos[vehicle.id].detourRatio = 1
            routeInfos[vehicle.id].shortest_path_distance = routeInfos[vehicle.id].length
        else:
            oldCosts = float(routeAlts[0].cost)
            newCosts = float(routeAlts[1].cost)
            assert(routeAlts[0].edges.split() == routeInfos[vehicle.id].edges)
            routeInfos[vehicle.id].shortest_path_distance = sumolib.route.getLength(net, routeAlts[1].edges.split())
            if oldCosts <= newCosts:
                routeInfos[vehicle.id].detour = 0
                routeInfos[vehicle.id].detourRatio = 1
                if oldCosts < newCosts:
                    sys.stderr.write(("Warning: fastest route for '%s' is slower than original route " +
                                      "(old=%s, new=%s). Check vehicle types\n") % (
                        vehicle.id, oldCosts, newCosts))
            else:
                routeInfos[vehicle.id].detour = oldCosts - newCosts
                routeInfos[vehicle.id].detourRatio = oldCosts / newCosts

    implausible = []
    allRoutesStats = Statistics("overall implausiblity")
    implausibleRoutesStats = Statistics("implausiblity above threshold")
    for rID in sorted(routeInfos.keys()):
        ri = routeInfos[rID]
        ri.implausibility = (options.airdist_ratio_factor * ri.airDistRatio +
                             options.detour_factor * ri.detour +
                             options.detour_ratio_factor * ri.detourRatio +
                             max(0, options.min_dist / ri.shortest_path_distance - 1) +
                             max(0, options.min_air_dist / ri.airDist - 1))
        allRoutesStats.add(ri.implausibility, rID)
        if ri.implausibility > options.threshold or ri.edgeLoop or ri.nodeLoop:
            implausible.append((ri.implausibility, rID, ri))
            implausibleRoutesStats.add(ri.implausibility, rID)

    # generate restrictions
    if options.restrictions_output is not None:
        with open(options.restrictions_output, 'w') as outf:
            for score, rID, ri in sorted(implausible):
                edges = ri.edges
                if options.odrestrictions and len(edges) > 2:
                    edges = [edges[0], edges[-1]]
                outf.write("0 %s\n" % " ".join(edges))

    if options.ignore_routes is not None:
        numImplausible = len(implausible)
        ignored = set([r.strip() for r in open(options.ignore_routes)])
        implausible = [r for r in implausible if r not in ignored]
        print("Loaded %s routes to ignore. Reducing implausible from %s to %s" % (
            len(ignored), numImplausible, len(implausible)))

    # generate polygons
    polyOutput = prefix + '.implausible.add.xml'
    colorgen = Colorgen(("random", 1, 1))
    with open(polyOutput, 'w') as outf:
        outf.write('<additional>\n')
        for score, rID, ri in sorted(implausible):
            generate_poly(options, net, rID, colorgen(), ri.edges, outf, score)
        outf.write('</additional>\n')

    sys.stdout.write('score\troute\t(airDistRatio, detourRatio, detour, shortestDist, airDist, edgeLoop, nodeLoop)\n')
    for score, rID, ri in sorted(implausible):
        # , ' '.join(ri.edges)))
        sys.stdout.write('%.7f\t%s\t%s\n' % (score, rID, (ri.airDistRatio, ri.detourRatio,
                                                          ri.detour, ri.shortest_path_distance,
                                                          ri.airDist, ri.edgeLoop, ri.nodeLoop)))

    print(allRoutesStats)
    print(implausibleRoutesStats)


if __name__ == "__main__":
    main()
