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

# @file    route2poly.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2012-11-15

"""
From a sumo network and a route file, this script generates a polygon (polyline) for every route
which can be loaded with sumo-gui for visualization
"""
from __future__ import absolute_import
import sys
import os
import itertools
import random
from collections import defaultdict
from optparse import OptionParser
sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
from sumolib.output import parse  # noqa
from sumolib.net import readNet  # noqa
from sumolib.miscutils import Colorgen  # noqa
from sumolib import geomhelper  # noqa


def parse_args(args):
    USAGE = "Usage: " + sys.argv[0] + " <netfile> <routefile> [options]"
    optParser = OptionParser()
    optParser.add_option("-o", "--outfile", help="name of output file")
    optParser.add_option("-u", "--hue", default="random",
                         help="hue for polygons (float from [0,1] or 'random')")
    optParser.add_option("-s", "--saturation", default=1,
                         help="saturation for polygons (float from [0,1] or 'random')")
    optParser.add_option("-b", "--brightness", default=1,
                         help="brightness for polygons (float from [0,1] or 'random')")
    optParser.add_option("-l", "--layer", default=100, help="layer for generated polygons")
    optParser.add_option("--geo", action="store_true",
                         default=False, help="write polgyons with geo-coordinates")
    optParser.add_option("--internal", action="store_true",
                         default=False, help="include internal edges in generated shapes")
    optParser.add_option("--spread", type="float", help="spread polygons laterally to avoid overlap")
    optParser.add_option("--blur", type="float",
                         default=0, help="maximum random disturbance to route geometry")
    optParser.add_option("--scale-width", type="float", dest="scaleWidth",
                         help="group similar routes and scale width by " +
                              "group size multiplied with the given factor (in m)")
    optParser.add_option("--standalone", action="store_true", default=False,
                         help="Parse stand-alone routes that are not define as child-element of a vehicle")
    optParser.add_option("--filter-output.file", dest="filterOutputFile",
                         help="only write output for edges in the given selection file")
    optParser.add_option("--seed", type="int", help="random seed")
    options, args = optParser.parse_args(args=args)
    if options.seed:
        random.seed(options.seed)
    if len(args) < 2:
        sys.exit(USAGE)
    try:
        options.net = args[0]
        options.routefiles = args[1:]
        options.colorgen = Colorgen(
            (options.hue, options.saturation, options.brightness))
    except Exception:
        sys.exit(USAGE)
    if options.outfile is None:
        options.outfile = options.routefiles[0] + ".poly.xml"

    return options


def randomize_pos(pos, blur):
    return tuple([val + random.uniform(-blur, blur) for val in pos])


MISSING_EDGES = set()
SPREAD = defaultdict(set)
SPREAD_MAX = [0]


def getSpread(lanes):
    """find the smalles spread value that is available for all lanes"""
    cands = [0]
    for i in range(1, SPREAD_MAX[0] + 2):
        cands += [i, -i]
    for i in cands:
        if all([i not in SPREAD[l] for l in lanes]):
            SPREAD_MAX[0] = max(SPREAD_MAX[0], i)
            [SPREAD[l].add(i) for l in lanes]
            return i
        else:
            pass
            # print(i, [l.getID() for l in lanes])
    assert(False)


def generate_poly(options, net, id, color, edges, outf, type="route", lineWidth=None, params={}):
    lanes = []
    spread = 0
    for e in edges:
        if net.hasEdge(e):
            lanes.append(net.getEdge(e).getLane(0))
        else:
            if e not in MISSING_EDGES:
                sys.stderr.write("Warning: unknown edge '%s'\n" % e)
                MISSING_EDGES.add(e)
    if not lanes:
        return
    if options.internal and len(lanes) > 1:
        lanes2 = []
        preferedCon = -1
        for i, lane in enumerate(lanes):
            edge = lane.getEdge()
            if i == 0:
                cons = edge.getConnections(lanes[i + 1].getEdge())
                if cons:
                    lanes2.append(cons[preferedCon].getFromLane())
                else:
                    lanes2.append(lane)
            else:
                cons = lanes[i - 1].getEdge().getConnections(edge)
                if cons:
                    viaID = cons[preferedCon].getViaLaneID()
                    if viaID:
                        via = net.getLane(viaID)
                        lanes2.append(via)
                        cons2 = via.getEdge().getConnections(edge)
                        if cons2:
                            viaID2 = cons2[preferedCon].getViaLaneID()
                            if viaID2:
                                via2 = net.getLane(viaID2)
                                lanes2.append(via2)
                        lanes2.append(cons[preferedCon].getToLane())
                else:
                    lanes2.append(lane)
        lanes = lanes2

    shape = list(itertools.chain(*list(l.getShape() for l in lanes)))
    if options.spread:
        spread = getSpread(lanes)
        if spread:
            shape = geomhelper.move2side(shape, options.spread * spread)
            params["spread"] = str(spread)
    if options.blur > 0:
        shape = [randomize_pos(pos, options.blur) for pos in shape]

    geoFlag = ""
    lineWidth = '' if lineWidth is None else ' lineWidth="%s"' % lineWidth
    if options.geo:
        shape = [net.convertXY2LonLat(*pos) for pos in shape]
        geoFlag = ' geo="true"'
    shapeString = ' '.join('%s,%s' % (x, y) for x, y in shape)
    close = '/'
    if params:
        close = ''
    outf.write('<poly id="%s" color="%s" layer="%s" type="%s" shape="%s"%s%s%s>\n' % (
        id, color, options.layer, type, shapeString, geoFlag, lineWidth, close))
    if params:
        for key, value in params.items():
            outf.write('    <param key="%s" value="%s"/>\n' % (key, value))
        outf.write('</poly>\n')


def filterEdges(edges, keep):
    if keep is None:
        return edges
    else:
        return [e for e in edges if e in keep]


def parseRoutes(options):
    known_ids = set()

    def unique_id(cand, index=0):
        cand2 = cand
        if index > 0:
            cand2 = "%s#%s" % (cand, index)
        if cand2 in known_ids:
            return unique_id(cand, index + 1)
        else:
            known_ids.add(cand2)
            return cand2

    keep = None
    if options.filterOutputFile is not None:
        keep = set()
        for line in open(options.filterOutputFile):
            if line.startswith('edge:'):
                keep.add(line.replace('edge:', '').strip())

    for routefile in options.routefiles:
        print("parsing %s" % routefile)
        if options.standalone:
            for route in parse(routefile, 'route'):
                # print("found veh", vehicle.id)
                yield unique_id(route.id), filterEdges(route.edges.split(), keep)
        else:
            for vehicle in parse(routefile, 'vehicle'):
                # print("found veh", vehicle.id)
                yield unique_id(vehicle.id), filterEdges(vehicle.route[0].edges.split(), keep)


def main(args):
    options = parse_args(args)
    net = readNet(options.net, withInternal=options.internal)

    with open(options.outfile, 'w') as outf:
        outf.write('<polygons>\n')
        if options.scaleWidth is None:
            for route_id, edges in parseRoutes(options):
                generate_poly(options, net, route_id, options.colorgen(), edges, outf)
        else:
            count = {}
            for route_id, edges in parseRoutes(options):
                edges = tuple(edges)
                if edges in count:
                    count[edges][0] += 1
                else:
                    count[edges] = [1, route_id]
            for edges, (n, route_id) in count.items():
                width = options.scaleWidth * n
                params = {'count': str(n)}
                generate_poly(options, net, route_id, options.colorgen(), edges, outf, lineWidth=width, params=params)

        outf.write('</polygons>\n')


if __name__ == "__main__":
    main(sys.argv[1:])
