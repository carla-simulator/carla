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

# @file    generateParkingLots.py
# @author  Yun-Pang Floetteroed
# @date    2017-10-13

"""
generate parking lots
"""

from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import optparse
import math

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
    import sumolib  # noqa
    from sumolib.geomhelper import isWithin
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options(args=None):
    optParser = optparse.OptionParser()
    optParser.add_option("-i", "--parking-id", dest="parkId",
                         default="parkingLot", help="define the name/id of the parking lot")
    optParser.add_option("-x", "--x-axis", dest="xaxis", type="int",
                         help="define x-position of the parking lot")
    optParser.add_option("-y", "--y-axis", dest="yaxis", type="int",
                         help="define y-position of the parking lot")
    optParser.add_option("-b", "--bounding-box", dest="bbox",
                         help="define the xmin, ymin, xmax, ymax of the parking lot")
    optParser.add_option("--shape", dest="shape",
                         help="define the boundary shape of the parking lot")
    optParser.add_option("-n", "--parking-spaces", dest="spaces", type="int",
                         default=5, help="define the number of the parking spaces")
    optParser.add_option("-c", "--connecting-edge", dest="connEdge",
                         help="define the connecting edge of the parking lot")
    optParser.add_option("-s", "--start-position", dest="start", type="int",
                         default=0, help="define the begin position of the enterance/exit of the parking lot")
    optParser.add_option("-e", "--end-position", dest="end", type="int",
                         default=2, help="define the end position of the enterance/exit of the parking lot")
    optParser.add_option("-l", "--space-length", dest="length", type="int",
                         default=5, help="define the length of each parking space")
    optParser.add_option("-a", "--space-angle", dest="angle", type="int",
                         default=315, help="define the angle of each parking space")
    optParser.add_option("--x-space-distance", dest="xdist", type="float",
                         default=5, help="define the lateral distance (x-direction) between the locations of two " +
                         "parking spaces")
    optParser.add_option("--y-space-distance", dest="ydist", type="float",
                         default=5, help="define the longitudinal (y-direction) distance between the locations of " +
                         "two parking spaces")
    optParser.add_option("-r", "--rotation-degree", dest="rotation", type="int",
                         default=0, help="define the rotation degree of the parking lot")
    optParser.add_option("--adjustrate-x", dest="factorX", type="float",
                         default=0.28, help="define the modification rate of x-axis if the rotation exists")
    optParser.add_option("--adjustrate-y", dest="factorY", type="float",
                         default=0.7, help="define the modification rate of y-axis if the rotation exists")
    optParser.add_option("--output-suffix", dest="suffix", help="output suffix", default="")
    optParser.add_option("--fullname", dest="fullname", help="full name of parking area", default=None)
    optParser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                         default=False, help="tell me what you are doing")
    (options, args) = optParser.parse_args(args=args)

    if not options.bbox and not options.shape and (not options.xaxis or not options.yaxis) or not options.connEdge:
        optParser.print_help()
        sys.exit()

    return options


def main(options):
    movingX = options.xdist
    movingY = options.ydist
    factorX = 0.
    factorY = 1.
    row = 0
    xmax = None
    ymax = None

    if options.shape:
        if ' ' in options.shape:
            options.shape = [tuple(map(float, xy.split(','))) for xy in options.shape.split(' ')]
        else:
            numbers = list(map(float, options.shape.split(',')))
            options.shape = [(numbers[i], numbers[i+1]) for i in range(0, len(numbers), 2)]
        xmin = min([x for x, y in options.shape])
        xmax = max([x for x, y in options.shape])
        ymin = min([y for x, y in options.shape])
        ymax = max([y for x, y in options.shape])
        if options.shape[0] != options.shape[-1]:
            # close shape
            options.shape.append(options.shape[0])
        x = xmin
        y = ymin
    elif options.bbox:
        xmin, ymin, xmax, ymax = map(float, options.bbox.split(','))
        x = xmin
        y = ymin
    else:
        x = options.xaxis
        y = options.yaxis

    if options.rotation != 0:
        movingX = options.xdist * (math.cos(options.rotation * math.pi / 180.))
        movingY = options.ydist * (math.sin(options.rotation * math.pi / 180.))
        factorX = options.factorX
        factorY = options.factorY

    if options.suffix:
        outfile = 'parking_%s.add.xml' % options.suffix
    else:
        outfile = 'parking_%s.add.xml' % options.parkId

    with open(outfile, 'w') as outf:
        outf.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        outf.write('<additional>\n')
        name = ' name="%s"' % options.fullname if options.fullname is not None else ""
        outf.write('    <parkingArea id="%s" lane="%s" startPos="%s" endPos="%s"%s>\n' %
                   (options.parkId, options.connEdge, options.start, options.end, name))
        lots = 0
        i = 0
        while lots < options.spaces:
            if not options.shape or isWithin((x, y), options.shape):
                outf.write(
                    '        <space x="%.2f" y="%.2f" length="%s" angle="%s"/>\n' %
                    (x, y, options.length, options.angle))
                lots += 1
            if xmax is not None:
                if x > xmax:
                    row += 1
                    if y < ymax:
                        y = ymin + row * options.ydist * factorY
                    else:
                        print("Warning: The area has been filed and only %s of %s lots were written" % (
                            lots, options.spaces), file=sys.stderr)
                        break
                    x = xmin + row * movingX * factorX
                else:
                    x += movingX
                    if options.rotation != 0:
                        y += movingY
            else:
                # No rotation degree is considered.
                x += options.xdist
                y += options.ydist
            i += 1
        outf.write('    </parkingArea>\n')
        outf.write('</additional>\n')


if __name__ == "__main__":
    options = get_options(sys.argv)
    main(options)
