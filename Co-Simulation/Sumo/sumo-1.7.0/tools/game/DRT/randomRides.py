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

# @file    randomRides.py
# @author  Jakob Erdmann
# @date    2019-02-24

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import random
import optparse

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
import sumolib  # noqa


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-n", "--net-file", dest="netfile",
                         help="define the net file")
    optParser.add_option("-a", "--additional-files", dest="additional",
                         help="define additional files for loading busStops (mandatory)")
    optParser.add_option("-o", "--output-file", dest="outfile",
                         help="define the output trip filename")
    optParser.add_option("--poi-output", dest="poiout",
                         help="define the output file for busStop pois")
    optParser.add_option("--prefix", dest="tripprefix",
                         default="", help="prefix for the trip ids")
    optParser.add_option("-t", "--trip-attributes", dest="tripattrs",
                         default="", help="additional trip attributes. When generating pedestrians, attributes for " +
                         "<person> and <walk> are supported.")
    optParser.add_option("-b", "--begin", type="float", default=0, help="begin time")
    optParser.add_option("-e", "--end", type="float", default=3600, help="end time (default 3600)")
    optParser.add_option("--poi-offset", dest="poiOffset", type="float",
                         default=12, help="offset of stop-poi from the lane in m")
    optParser.add_option("--initial-duration", dest="duration", type="int", default=5, help="inital stop duration in s")
    optParser.add_option("-p", "--period", type="float", default=1,
                         help="Generate vehicles with equidistant departure times and period=FLOAT (default 1.0).")
    optParser.add_option("-s", "--seed", type="int", help="random seed")
    optParser.add_option("--min-distance", type="float", dest="min_distance",
                         default=0.0, help="require start and end edges for each trip to be at least <FLOAT> m apart")
    optParser.add_option("--max-distance", type="float", dest="max_distance",
                         default=None, help="require start and end edges for each trip to be at most <FLOAT> m " +
                         "apart (default 0 which disables any checks)")
    optParser.add_option("-v", "--verbose", action="store_true",
                         default=False, help="tell me what you are doing")
    (options, args) = optParser.parse_args(args=args)

    if not options.additional or not options.outfile:
        optParser.print_help()
        sys.exit(1)

    if options.period <= 0:
        print("Error: Period must be positive", file=sys.stderr)
        sys.exit(1)

    if options.poiout is not None and options.netfile is None:
        print("Error: poi-output requires a net-file", file=sys.stderr)
        sys.exit(1)

    return options


def main(options):
    if options.seed:
        random.seed(options.seed)
    busStops = [bs.id for bs in sumolib.xml.parse_fast(options.additional, 'busStop', ['id'])]
    stopColors = {}
    if options.poiout:
        colorgen = sumolib.miscutils.Colorgen(('distinct', 'distinct', 'distinct'))
        net = sumolib.net.readNet(options.netfile)
        with open(options.poiout, 'w') as outf:
            outf.write('<additional>\n')
            for bs in sumolib.xml.parse(options.additional, 'busStop'):
                laneShape = net.getLane(bs.lane).getShape()
                sideShape = sumolib.geomhelper.move2side(laneShape, options.poiOffset)
                offset = (float(bs.startPos) + float(bs.endPos)) / 2
                x, y = sumolib.geomhelper.positionAtShapeOffset(sideShape, offset)
                stopColors[bs.id] = colorgen()
                outf.write('    <poi id="%s" x="%s" y="%s" color="%s" type="%s"/>\n' % (
                    bs.id, x, y, stopColors[bs.id], bs.attr_name))
            outf.write('</additional>\n')

    if len(busStops) < 2:
        print("Error: At least two busStops are required", file=sys.stderr)
        sys.exit(1)

    depart = options.begin
    idx = 0
    with open(options.outfile, 'w') as outf:
        outf.write('<routes>\n')
        while depart < options.end:
            bsFrom = random.choice(busStops)
            bsTo = random.choice(busStops)
            while bsTo == bsFrom:
                bsTo = random.choice(busStops)
            color = ""
            if options.poiout:
                color = ' color="%s"' % stopColors[bsTo]
            outf.write('    <person id="%s%s" depart="%s"%s>\n' % (
                options.tripprefix, idx, depart, color))
            outf.write('        <stop busStop="%s" duration="%s"/>\n' % (bsFrom, options.duration))
            outf.write('        <ride busStop="%s" lines="ANY"/>\n' % (bsTo))
            outf.write('    </person>\n')
            depart += options.period
            idx += 1
        outf.write('</routes>\n')


if __name__ == "__main__":
    if not main(get_options()):
        sys.exit(1)
