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

# @file    mpl_dump_twoAgainst.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

"""

This script reads two dump files and plots one of the values
 stored therein as an x-/y- plot.

matplotlib has to be installed for this purpose
"""
from __future__ import absolute_import
from __future__ import print_function

from matplotlib import rcParams
from pylab import arange, figure, iterable, plot, savefig, show, xlim, xticks, ylim, yticks
from optparse import OptionParser
from xml.sax import make_parser, handler


def toHex(val):
    """Converts the given value (0-255) into its hexadecimal representation"""
    hex = "0123456789abcdef"
    return hex[int(val / 16)] + hex[int(val - int(val / 16) * 16)]


def toColor(val):
    """Converts the given value (0-1) into a color definition as parseable by matplotlib"""
    g = 255. * val
    return "#" + toHex(g) + toHex(g) + toHex(g)


def updateMinMax(min, max, value):
    if min is None or min > value:
        min = value
    if max is None or max < value:
        max = value
    return (min, max)


class WeightsReader(handler.ContentHandler):

    """Reads the dump file"""

    def __init__(self, value):
        self._id = ''
        self._edge2value = {}
        self._edge2no = {}
        self._value = value

    def startElement(self, name, attrs):
        if name == 'interval':
            self._time = int(attrs['begin'])
            self._edge2value[self._time] = {}
        if name == 'edge':
            self._id = attrs['id']
            if self._id not in self._edge2value[self._time]:
                self._edge2value[self._time][self._id] = 0.
            self._edge2value[self._time][self._id] = self._edge2value[
                self._time][self._id] + float(attrs[self._value])


# initialise
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
# i/o
optParser.add_option("-1", "--dump1", dest="dump1",
                     help="First dump (mandatory)", metavar="FILE")
optParser.add_option("-2", "--dump2", dest="dump2",
                     help="Second dump (mandatory)", metavar="FILE")
optParser.add_option("-o", "--output", dest="output",
                     help="Name of the image to generate", metavar="FILE")
optParser.add_option("--size", dest="size", type="string", default="",
                     help="defines the output size")
# processing
optParser.add_option("--value", dest="value",
                     type="string", default="speed", help="which value shall be used")
optParser.add_option("-s", "--show", action="store_true", dest="show",
                     default=False, help="shows plot after generating it")
optParser.add_option("-j", "--join", action="store_true", dest="join",
                     default=False, help="aggregates each edge's values")
optParser.add_option("-C", "--time-coloring", action="store_true", dest="time_coloring",
                     default=False, help="colors the points by the time")
# axes/legend
optParser.add_option("--xticks", dest="xticks", type="string", default="",
                     help="defines ticks on x-axis")
optParser.add_option("--yticks", dest="yticks", type="string", default="",
                     help="defines ticks on y-axis")
optParser.add_option("--xlim", dest="xlim", type="string", default="",
                     help="defines x-axis range")
optParser.add_option("--ylim", dest="ylim", type="string", default="",
                     help="defines y-axis range")
# parse options
(options, args) = optParser.parse_args()
# check set options
if not options.show and not options.output:
    print("Neither show (--show) not write (--output <FILE>)? Exiting...")
    exit()


parser = make_parser()
# read dump1
if options.verbose:
    print("Reading dump1...")
weights1 = WeightsReader(options.value)
parser.setContentHandler(weights1)
parser.parse(options.dump1)
# read dump2
if options.verbose:
    print("Reading dump2...")
weights2 = WeightsReader(options.value)
parser.setContentHandler(weights2)
parser.parse(options.dump2)
# plot
if options.verbose:
    print("Processing data...")
# set figure size
if not options.show:
    rcParams['backend'] = 'Agg'
if options.size:
    f = figure(figsize=(options.size.split(",")))
else:
    f = figure()
xs = []
ys = []
# compute values and color(s)
c = 'k'
min = None
max = None
if options.join:
    values1 = {}
    values2 = {}
    nos1 = {}
    nos2 = {}
    for t in weights1._edge2value:
        for edge in weights1._edge2value[t]:
            if edge not in values1:
                nos1[edge] = 0
                values1[edge] = 0
            nos1[edge] = nos1[edge] + 1
            values1[edge] = values1[edge] + weights1._edge2value[t][edge]
        if t in weights2._edge2value:
            for edge in weights2._edge2value[t]:
                if edge not in values2:
                    nos2[edge] = 0
                    values2[edge] = 0
                nos2[edge] = nos2[edge] + 1
                values2[edge] = values2[edge] + weights2._edge2value[t][edge]
    for edge in values1:
        if edge in values2:
            xs.append(values1[edge] / nos1[edge])
            ys.append(values2[edge] / nos2[edge])
            (min, max) = updateMinMax(min, max, values1[edge] / nos1[edge])
            (min, max) = updateMinMax(min, max, values2[edge] / nos2[edge])
else:
    if options.time_coloring:
        c = []
    for t in weights1._edge2value:
        if options.time_coloring:
            xs.append([])
            ys.append([])
            cc = 1. - ((float(t) / 86400.) * .8 + .2)
            c.append(toColor(cc))
            for edge in weights1._edge2value[t]:
                if t in weights2._edge2value and edge in weights2._edge2value[t]:
                    xs[-1].append(weights1._edge2value[t][edge])
                    ys[-1].append(weights2._edge2value[t][edge])
                    (min, max) = updateMinMax(
                        min, max, weights1._edge2value[t][edge])
                    (min, max) = updateMinMax(
                        min, max, weights2._edge2value[t][edge])
        else:
            for edge in weights1._edge2value[t]:
                if t in weights2._edge2value and edge in weights2._edge2value[t]:
                    xs.append(weights1._edge2value[t][edge])
                    ys.append(weights2._edge2value[t][edge])
                    (min, max) = updateMinMax(
                        min, max, weights1._edge2value[t][edge])
                    (min, max) = updateMinMax(
                        min, max, weights2._edge2value[t][edge])
    # plot
print("data range: " + str(min) + " - " + str(max))
if options.verbose:
    print("Plotting...")
if options.time_coloring and iterable(c):
    for i in range(0, len(c)):
        plot(xs[i], ys[i], '.', color=c[i], mfc=c[i])
else:
    plot(xs, ys, ',', color=c)
# set axes
if options.xticks != "":
    (xb, xe, xd, xs) = options.xticks.split(",")
    xticks(arange(xb, xe, xd), size=xs)
if options.yticks != "":
    (yb, ye, yd, ys) = options.yticks.split(",")
    yticks(arange(yb, ye, yd), size=ys)
if options.xlim != "":
    (xb, xe) = options.xlim.split(",")
    xlim(int(xb), int(xe))
else:
    xlim(min, max)
if options.ylim != "":
    (yb, ye) = options.ylim.split(",")
    ylim(int(yb), int(ye))
else:
    ylim(min, max)
# show/save
if options.show:
    show()
if options.output:
    savefig(options.output)
