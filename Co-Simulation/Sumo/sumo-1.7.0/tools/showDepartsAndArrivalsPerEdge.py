#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    showDepartsAndArrivalsPerEdge.py
# @author  Jakob Erdmann
# @date    2015-08-05

from __future__ import print_function
import sys
from optparse import OptionParser
from collections import defaultdict
from sumolib.output import parse_fast
from sumolib.miscutils import Statistics


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <routefile> [options]"
    optParser = OptionParser()
    optParser.add_option("-o", "--output-file", dest="outfile",
                         help="name of output file")
    optParser.add_option("--subpart",
                         help="Restrict counts to routes that contain the given consecutive edge sequence")
    optParser.add_option("--subpart-file", dest="subpart_file",
                         help="Restrict counts to routes that contain one of the consecutive edge sequences " +
                              "in the given input file (one sequence per line)")
    optParser.add_option("-i", "--intermediate", action="store_true", default=False,
                         help="count all edges of a route")
    optParser.add_option("--taz", action="store_true", default=False,
                         help="use fromTaz and toTaz instead of from and to")
    options, args = optParser.parse_args()
    try:
        options.routefile, = args
    except Exception:
        sys.exit(USAGE)
    if options.outfile is None:
        options.outfile = options.routefile + ".departsAndArrivals.xml"

    options.subparts = []
    if options.subpart is not None:
        options.subparts.append(options.subpart.split(','))
    if options.subpart_file is not None:
        for line in open(options.subpart_file):
            options.subparts.append(line.strip().split(','))

    return options


def hasSubpart(edges, subparts):
    if not subparts:
        return True
    for subpart in subparts:
        for i in range(len(edges)):
            if edges[i:i + len(subpart)] == subpart:
                return True
    return False


def main():
    options = parse_args()
    departCounts = defaultdict(lambda: 0)
    arrivalCounts = defaultdict(lambda: 0)
    intermediateCounts = defaultdict(lambda: 0)

    for route in parse_fast(options.routefile, 'route', ['edges']):
        edges = route.edges.split()
        if not hasSubpart(edges, options.subparts):
            continue
        departCounts[edges[0]] += 1
        arrivalCounts[edges[-1]] += 1
        for e in edges:
            intermediateCounts[e] += 1

    for walk in parse_fast(options.routefile, 'walk', ['edges']):
        edges = walk.edges.split()
        if not hasSubpart(edges, options.subparts):
            continue
        departCounts[edges[0]] += 1
        arrivalCounts[edges[-1]] += 1
        for e in edges:
            intermediateCounts[e] += 1

    # warn about potentially missing edges
    fromAttr, toAttr = ('fromTaz', 'toTaz') if options.taz else ('from', 'to')
    for trip in parse_fast(options.routefile, 'trip', ['id', fromAttr, toAttr]):
        if options.subparts:
            sys.stderr.write("Warning: Ignoring trips when using --subpart\n")
            break
        departCounts[trip[1]] += 1
        arrivalCounts[trip[2]] += 1
    for walk in parse_fast(options.routefile, 'walk', ['from', 'to']):
        if options.subparts:
            sys.stderr.write("Warning: Ignoring trips when using --subpart\n")
            break
        departCounts[walk.attr_from] += 1
        arrivalCounts[walk.to] += 1

    departStats = Statistics("departEdges")
    arrivalStats = Statistics("arrivalEdges")
    intermediateStats = Statistics("intermediateEdges")
    for e in sorted(departCounts.keys()):
        departStats.add(departCounts[e], e)
    for e in sorted(arrivalCounts.keys()):
        arrivalStats.add(arrivalCounts[e], e)
    print(departStats)
    print(arrivalStats)
    if options.intermediate:
        for e in sorted(intermediateCounts.keys()):
            intermediateStats.add(intermediateCounts[e], e)
        print(intermediateStats)

    with open(options.outfile, 'w') as outf:
        outf.write("<edgedata>\n")
        outf.write('   <interval begin="0" end="10000" id="routeStats">\n')
        allEdges = set(departCounts.keys())
        allEdges.update(arrivalCounts.keys())
        if options.intermediate:
            allEdges.update(intermediateCounts.keys())
        for e in sorted(allEdges):
            intermediate = ' intermediate="%s"' % intermediateCounts[e] if options.intermediate else ''
            outf.write('      <edge id="%s" departed="%s" arrived="%s" delta="%s"%s/>\n' %
                       (e, departCounts[e], arrivalCounts[e], arrivalCounts[e] - departCounts[e], intermediate))
        outf.write("   </interval>\n")
        outf.write("</edgedata>\n")


if __name__ == "__main__":
    main()
