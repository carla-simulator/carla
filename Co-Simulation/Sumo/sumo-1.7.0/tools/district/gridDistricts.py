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

# @file    gridDistricts.py
# @author  Jakob Erdmann
# @date    2019-01-02

"""
Generate a grid-based TAZ file
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
from optparse import OptionParser
SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa
from sumolib.miscutils import Colorgen  # noqa


class TAZ:
    def __init__(self, id, shape, color):
        self.id = id
        self.shape = shape
        self.color = color
        self.edges = []

    def write(self, outf):
        outf.write('    <taz id="%s" shape="%s" color="%s" edges="%s"/>\n' % (
            self.id, ' '.join(["%s,%s" % (x, y) for x, y in self.shape]),
            self.color, ' '.join(self.edges)))


def getOptions():
    optParser = OptionParser()
    optParser.add_option("-v", "--verbose", action="store_true", default=False,
                         help="tell me what you are doing")
    optParser.add_option("-n", "--net-file", dest="netfile", help="the network to read lane and edge permissions")
    optParser.add_option("-o", "--output", help="output taz file")
    optParser.add_option("-w", "--grid-width", dest="gridWidth", type="float", default=100.0,
                         help="width of gride cells in m")
    optParser.add_option("-u", "--hue", default="random",
                         help="hue for taz (float from [0,1] or 'random')")
    optParser.add_option("-s", "--saturation", default=1,
                         help="saturation for taz (float from [0,1] or 'random')")
    optParser.add_option("-b", "--brightness", default=1,
                         help="brightness for taz (float from [0,1] or 'random')")
    (options, args) = optParser.parse_args()
    if not options.netfile or not options.output:
        optParser.print_help()
        optParser.exit("Error! net-file and output file")
    options.colorgen = Colorgen((options.hue, options.saturation, options.brightness))
    return options


if __name__ == "__main__":
    options = getOptions()
    if options.verbose:
        print("Reading net")
    net = sumolib.net.readNet(options.netfile)
    xmin, ymin, xmax, ymax = net.getBoundary()
    odpairs = {}  # (x,y) -> TAZ
    centerCoords = {}  # edge -> center pos
    w = options.gridWidth
    w2 = w * 0.5 - 1
    for edge in net.getEdges():
        x, y = sumolib.geomhelper.positionAtShapeOffset(edge.getShape(True), edge.getLength() / 2)
        xIndex = int((x - xmin + w2) / w)
        yIndex = int((y - ymin + w2) / w)
        ii = (xIndex, yIndex)
        x2 = xIndex * w
        y2 = yIndex * w
        if ii not in odpairs:
            odpairs[ii] = TAZ("%s_%s" % (xIndex, yIndex),
                              [(x2 - w2, y2 - w2),
                               (x2 + w2, y2 - w2),
                               (x2 + w2, y2 + w2),
                               (x2 - w2, y2 + w2),
                               (x2 - w2, y2 - w2)],
                              options.colorgen())
        odpairs[ii].edges.append(edge.getID())

    with open(options.output, 'w') as outf:
        outf.write("<tazs>\n")
        for ii, taz in sorted(odpairs.items()):
            taz.write(outf)
        outf.write("</tazs>\n")
