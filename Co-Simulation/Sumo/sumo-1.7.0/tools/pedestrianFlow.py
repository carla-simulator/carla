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

# @file    pedestrianFlow.py
# @author  Jakob Erdmann
# @date    2014-01-16

from __future__ import absolute_import
import os
import sys
import random
from optparse import OptionParser

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
    import sumolib  # noqa
    from sumolib.miscutils import Colorgen  # noqa
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")


def get_options():
    optParser = OptionParser()
    optParser.add_option(
        "-w", "--width", type="float", default=0.7, help="pedestrian width, negative numbers denote the center " +
        "of a uniform distribution [x-0.2, x+0.2]")
    optParser.add_option(
        "-l", "--length", type="float", default=0.5, help="pedestrian length, negative numbers denote the center " +
        "of a uniform distribution [x-0.2, x+0.2]")
    optParser.add_option(
        "-m", "--minGap", type="float", default=0.2, help="pedestrian min gap, negative numbers denote the center " +
        "of a uniform distribution [x-0.2, x+0.2]")
    optParser.add_option(
        "-s", "--maxSpeed", type="float", default=1.2, help="pedestrian max speed, negative numbers denote the " +
        "center of a uniform distribution [x-0.4, x+0.4]")
    optParser.add_option(
        "-d", "--departPos", type="float", default=0, help="depart position")
    optParser.add_option(
        "-a", "--arrivalPos", type="float", default=-1, help="arrival position")
    optParser.add_option(
        "-p", "--prob", type="float", default=0.1, help="depart probability per second")
    optParser.add_option("-r", "--route", help="edge list")
    optParser.add_option("-t", "--trip", help="Define walking trip as FROM_EDGE,TO_EDGE")
    optParser.add_option("-c", "--color", help="the color to use or 'random'")
    optParser.add_option(
        "-b", "--begin", type="int", default=0, help="begin time")
    optParser.add_option(
        "-e", "--end", type="int", default=600, help="end time")
    optParser.add_option("-i", "--index", type="int",
                         default=0, help="starting index for naming pedestrians")
    optParser.add_option(
        "-n", "--name", default="p", help="base name for pedestrians")
    (options, args) = optParser.parse_args()

    if len(args) != 1:
        sys.exit("Output file argument missing")
    options.output = args[0]

    if ((options.route is None and options.trip is None) or
            (options.route is not None and options.trip is not None)):
        sys.exit("Either --route or --trip must be given")
    if options.route is not None:
        options.route = options.route.split(',')
    if options.trip is not None:
        options.trip = tuple(options.trip.split(','))
        if len(options.trip) != 2:
            sys.exit("trip must contain of exactly two edges")

    return options


def randomOrFixed(value, offset=0.2):
    if value >= 0:
        return value
    return random.uniform(-value - offset, -value + offset)


def write_ped(f, index, options, depart):
    if options.color is None:
        color = ''
    elif options.color == "random":
        color = ' color="%s"' % Colorgen(("random", 1, 1))()
    else:
        color = ' color="%s"' % options.color

    if options.trip is not None:
        edges = 'from="%s" to="%s"' % options.trip
    else:
        edges = 'edges="%s"' % ' '.join(options.route)

    f.write(('    <vType id="%s%s" vClass="pedestrian" width="%s" length="%s" minGap="%s" maxSpeed="%s" ' +
             'guiShape="pedestrian"%s/>\n') % (
        options.name, index,
        randomOrFixed(options.width),
        randomOrFixed(options.length),
        randomOrFixed(options.minGap),
        randomOrFixed(options.maxSpeed, 0.4), color))
    f.write('    <person id="%s%s" type="%s%s" depart="%s" departPos="%s">\n' %
            (options.name, index, options.name, index, depart, options.departPos))
    f.write('        <walk %s arrivalPos="%s"/>\n' %
            (edges, options.arrivalPos))
    f.write('    </person>\n')


def main():
    options = get_options()
    with open(options.output, 'w') as f:
        sumolib.writeXMLHeader(f, "$Id$", "routes")  # noqa
        index = options.index
        for depart in range(options.begin, options.end):
            if random.random() < options.prob:
                write_ped(
                    f, index, options, depart)
                index += 1
        f.write('</routes>')


if __name__ == "__main__":
    main()
