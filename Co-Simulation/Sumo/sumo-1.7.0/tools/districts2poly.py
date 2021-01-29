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

# @file    districts2poly.py
# @author  Jakob Erdmann
# @date    2015-07-31

"""
From a sumo network and a taz (district) file, this script colors each district
with a unique color (by creating a colored polygon for each edge in that
district)
These polygons can then be visualized in sumo-gui
"""
from __future__ import absolute_import
import sys
import os
import random
from optparse import OptionParser
sys.path.append(os.path.join(os.environ["SUMO_HOME"], 'tools'))
from sumolib.output import parse  # noqa
from sumolib.net import readNet  # noqa
from sumolib.miscutils import Colorgen  # noqa


def parse_args():
    USAGE = "Usage: " + sys.argv[0] + " <netfile> <routefile> [options]"
    optParser = OptionParser()
    optParser.add_option("-o", "--outfile", help="name of output file")
    optParser.add_option("-u", "--hue", default="random",
                         help="hue for polygons (float from [0,1] or 'random')")
    optParser.add_option("-s", "--saturation", default=1,
                         help="saturation for polygons (float from [0,1] or 'random')")
    optParser.add_option("-b", "--brightness", default=1,
                         help="brightness for polygons (float from [0,1] or 'random')")
    optParser.add_option(
        "-l", "--layer", default=100, help="layer for generated polygons")
    options, args = optParser.parse_args()
    try:
        options.net, options.routefile = args
        options.colorgen = Colorgen(
            (options.hue, options.saturation, options.brightness))
    except Exception:
        sys.exit(USAGE)
    if options.outfile is None:
        options.outfile = options.routefile + ".poly.xml"
    return options


def generate_poly(net, id, color, layer, edges, outf):
    for edge in edges:
        shape = net.getEdge(edge).getLane(0).getShape()
        shapeString = ' '.join('%s,%s' % (x, y) for x, y in shape)
        outf.write('<poly id="%s_%s" color="%s" layer="%s" type="taz_edge" shape="%s"/>\n' % (
            id, net.getEdge(edge).getID(), color, layer, shapeString))


def main():
    random.seed(42)
    options = parse_args()
    net = readNet(options.net)
    with open(options.outfile, 'w') as outf:
        outf.write('<polygons>\n')
        for taz in parse(options.routefile, 'taz'):
            generate_poly(net, taz.id, options.colorgen(),
                          options.layer, taz.edges.split(), outf)
        outf.write('</polygons>\n')


if __name__ == "__main__":
    main()
