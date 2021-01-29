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

# @file    generateBidiDistricts.py
# @author  Jakob Erdmann
# @date    2015-07-31

"""
Generate a taz (district) file which groups edges in opposite directions
belonging to the same road. For each edge, a taz is created which contains this edge
and its opposite.
This allows routing without the need for an
initial/final turn-around by replacing the attribute names 'from' and 'to' with
'fromTaz' and 'toTaz'
"""
from __future__ import absolute_import
import sys
from optparse import OptionParser

import sumolib  # noqa
from functools import reduce


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <netfile> [options]"
    optParser = OptionParser()
    optParser.add_option("-o", "--outfile", help="name of output file")
    optParser.add_option("-r", "--radius", type=float, default=10., help="maximum air distance around the edge")
    optParser.add_option("-t", "--travel-distance", type=float, help="maximum travel distance in the graph")
    optParser.add_option("--symmetrical", action="store_true",
                         default=False, help="extend the bidi-relationship to be symmetrical")
    options, args = optParser.parse_args()
    try:
        options.net, = args
    except Exception:
        sys.exit(USAGE)
    if options.outfile is None:
        options.outfile = options.net + ".taz.xml"
    return options


def getCandidates(edge, net, radius):
    candidates = []
    r = min(radius, sumolib.geomhelper.polyLength(edge.getShape()) / 2)
    for x, y in edge.getShape():
        nearby = set()
        for edge2, dist in net.getNeighboringEdges(x, y, r):
            nearby.add(edge2)
        candidates.append(nearby)
    return candidates


ASYM_BIDI_CACHE = {}  # edge : opposites


def computeBidiTazAsymByRadius(edge, net, radius):
    if edge not in ASYM_BIDI_CACHE:
        candidates = getCandidates(edge, net, radius)
        opposites = reduce(lambda a, b: a.intersection(b), candidates)
        opposites.update(set(edge.getToNode().getOutgoing()).intersection(
            set(edge.getFromNode().getIncoming())))
        ASYM_BIDI_CACHE[edge] = opposites
    return ASYM_BIDI_CACHE[edge]


def computeAllBidiTaz(net, radius, travelDist, symmetrical):
    for edge in net.getEdges():
        travelOpposites = set()
        if travelDist is not None:
            queue = [(edge, -1.)]
            while not len(queue) == 0:
                edge2, dist = queue.pop()
                if edge2 not in travelOpposites and dist < travelDist:
                    travelOpposites.add(edge2)
                    if dist == -1.:
                        dist = 0.
                    else:
                        dist += edge2.getLength()
                    toN = edge2.getToNode()
                    fromN = edge2.getFromNode()
                    for e in toN.getOutgoing() + toN.getIncoming() + fromN.getOutgoing() + fromN.getIncoming():
                        queue.append((e, dist))
        if radius is not None and radius > 0.:
            opposites = computeBidiTazAsymByRadius(edge, net, radius)
            if symmetrical:
                candidates = reduce(
                    lambda a, b: a.union(b), getCandidates(edge, net, radius))
                for cand in candidates:
                    if edge in computeBidiTazAsymByRadius(cand, net, radius):
                        opposites.add(cand)
            travelOpposites.update(opposites)

        yield edge, travelOpposites


def main(netFile, outFile, radius, travelDist, symmetrical):
    net = sumolib.net.readNet(netFile, withConnections=False, withFoes=False)
    with open(outFile, 'w') as outf:
        sumolib.writeXMLHeader(
            outf, "$Id$")  # noqa
        outf.write('<tazs>\n')
        for taz, edges in computeAllBidiTaz(net, radius, travelDist, symmetrical):
            outf.write('    <taz id="%s" edges="%s"/>\n' % (
                taz.getID(), ' '.join(sorted([e.getID() for e in edges]))))
        outf.write('</tazs>\n')
    return net


if __name__ == "__main__":
    options = parse_args()
    main(options.net, options.outfile, options.radius,
         options.travel_distance, options.symmetrical)
