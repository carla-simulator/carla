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

# @file    mpl_dump_onNet.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2007-10-25

"""

This script reads a network and a dump file and
 draws the network, coloring it by the values
 found within the dump-file.

matplotlib has to be installed for this purpose
"""
from __future__ import absolute_import
from __future__ import print_function

from matplotlib import rcParams
from pylab import arange, figure, plot, savefig, show, xlim, xticks, ylim, yticks
import math
from optparse import OptionParser
from xml.sax import make_parser, handler


def toHex(val):
    """Converts the given value (0-255) into its hexadecimal representation"""
    hex = "0123456789abcdef"
    return hex[int(val / 16)] + hex[int(val - int(val / 16) * 16)]


def toFloat(val):
    """Converts the given value (0-255) into its hexadecimal representation"""
    hex = "0123456789abcdef"
    return float(hex.find(val[0]) * 16 + hex.find(val[1]))


def toColor(val, colormap):
    """Converts the given value (0-1) into a color definition parseable by matplotlib"""
    for i in range(0, len(colormap) - 1):
        if colormap[i + 1][0] > val:
            scale = (val - colormap[i][0]) / \
                (colormap[i + 1][0] - colormap[i][0])
            r = colormap[i][1][0] + \
                (colormap[i + 1][1][0] - colormap[i][1][0]) * scale
            g = colormap[i][1][1] + \
                (colormap[i + 1][1][1] - colormap[i][1][1]) * scale
            b = colormap[i][1][2] + \
                (colormap[i + 1][1][2] - colormap[i][1][2]) * scale
            return "#" + toHex(r) + toHex(g) + toHex(b)
    return "#" + toHex(colormap[-1][1][0]) + toHex(colormap[-1][1][1]) + toHex(colormap[-1][1][2])


def parseColorMap(mapDef):
    ret = []
    defs = mapDef.split(",")
    for d in defs:
        (value, color) = d.split(":")
        r = color[1:3]
        g = color[3:5]
        b = color[5:7]
        ret.append((float(value), (toFloat(r), toFloat(g), toFloat(b))))
    return ret


class NetReader(handler.ContentHandler):

    """Reads a network, storing the edge geometries, lane numbers and max. speeds"""

    def __init__(self):
        self._id = ''
        self._edge2lanes = {}
        self._edge2speed = {}
        self._edge2shape = {}
        self._edge2from = {}
        self._edge2to = {}
        self._node2x = {}
        self._node2y = {}
        self._currentShapes = []
        self._parseLane = False

    def startElement(self, name, attrs):
        self._parseLane = False
        if name == 'edge':
            if 'function' not in attrs or attrs['function'] != 'internal':
                self._id = attrs['id']
                self._edge2from[attrs['id']] = attrs['from']
                self._edge2to[attrs['id']] = attrs['to']
                self._edge2lanes[attrs['id']] = 0
                self._currentShapes = []
            else:
                self._id = ""
        if name == 'lane' and self._id != "":
            self._edge2speed[self._id] = float(attrs['speed'])
            self._edge2lanes[self._id] = self._edge2lanes[self._id] + 1
            self._parseLane = True
            self._currentShapes.append(attrs["shape"])
        if name == 'junction':
            self._id = attrs['id']
            if self._id[0] != ':':
                self._node2x[attrs['id']] = attrs['x']
                self._node2y[attrs['id']] = attrs['y']
            else:
                self._id = ""

    def endElement(self, name):
        if self._parseLane:
            self._parseLane = False
        if name == 'edge' and self._id != "":
            noShapes = len(self._currentShapes)
            if noShapes % 2 == 1 and noShapes > 0:
                self._edge2shape[self._id] = self._currentShapes[
                    int(noShapes / 2)]
            elif noShapes % 2 == 0 and len(self._currentShapes[0]) != 2:
                cshapes = []
                minLen = -1
                for i in self._currentShapes:
                    cshape = []
                    es = i.split(" ")
                    for e in es:
                        p = e.split(",")
                        cshape.append((float(p[0]), float(p[1])))
                    cshapes.append(cshape)
                    if minLen == -1 or minLen > len(cshape):
                        minLen = len(cshape)
                self._edge2shape[self._id] = ""
                if minLen > 2:
                    for i in range(0, minLen):
                        x = 0.
                        y = 0.
                        for j in range(0, noShapes):
                            x = x + cshapes[j][i][0]
                            y = y + cshapes[j][i][1]
                        x = x / float(noShapes)
                        y = y / float(noShapes)
                        if self._edge2shape[self._id] != "":
                            self._edge2shape[self._id] = self._edge2shape[
                                self._id] + " "
                        self._edge2shape[self._id] = self._edge2shape[
                            self._id] + str(x) + "," + str(y)

    def plotData(self, weights, options, values1, values2, saveName, colorMap):
        edge2plotLines = {}
        edge2plotColors = {}
        edge2plotWidth = {}
        xmin = 10000000.
        xmax = -10000000.
        ymin = 10000000.
        ymax = -10000000.
        min_width = 0
        if options.min_width:
            min_width = options.min_width
        for edge in self._edge2from:
            # compute shape
            xs = []
            ys = []
            if edge not in self._edge2shape or self._edge2shape[edge] == "":
                xs.append(float(self._node2x[self._edge2from[edge]]))
                xs.append(float(self._node2x[self._edge2to[edge]]))
                ys.append(float(self._node2y[self._edge2from[edge]]))
                ys.append(float(self._node2y[self._edge2to[edge]]))
            else:
                shape = self._edge2shape[edge].split(" ")
                for s in shape:
                    p = s.split(",")
                    xs.append(float(p[0]))
                    ys.append(float(p[1]))
            for x in xs:
                if x < xmin:
                    xmin = x
                if x > xmax:
                    xmax = x
            for y in ys:
                if y < ymin:
                    ymin = y
                if y > ymax:
                    ymax = y
            # save shape
            edge2plotLines[edge] = (xs, ys)
            # compute color
            if edge in values2:
                c = values2[edge]
            else:
                c = 0
            edge2plotColors[edge] = toColor(c, colorMap)
            # compute width
            if edge in values1:
                w = values1[edge]
                if w > 0:
                    w = 10. * math.log(1 + values1[edge]) + min_width
                else:
                    w = min_width
                if options.max_width and w > options.max_width:
                    w = options.max_width
                if w < min_width:
                    w = min_width
                edge2plotWidth[edge] = w
            else:
                edge2plotWidth[edge] = min_width
        if options.verbose:
            print("x-limits: " + str(xmin) + " - " + str(xmax))
            print("y-limits: " + str(ymin) + " - " + str(ymax))
        if not options.show:
            rcParams['backend'] = 'Agg'
        # set figure size
        if options.size and not options.show:
            figure(figsize=(options.size.split(",")))
        else:
            figure()
        for edge in edge2plotLines:
            plot(edge2plotLines[edge][0], edge2plotLines[edge][
                 1], color=edge2plotColors[edge], linewidth=edge2plotWidth[edge])
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
            xlim(xmin, xmax)
        if options.ylim != "":
            (yb, ye) = options.ylim.split(",")
            ylim(int(yb), int(ye))
        else:
            ylim(ymin, ymax)

        if saveName:
            savefig(saveName)
        if options.show:
            show()

    def plot(self, weights, options, colorMap):
        self._minValue1 = weights._minValue1
        self._minValue2 = weights._minValue2
        self._maxValue1 = weights._maxValue1
        self._maxValue2 = weights._maxValue2

        if options.join:
            self.plotData(weights, options, weights._edge2value1,
                          weights._edge2value2, options.output, colorMap)
        else:
            for i in weights._intervalBegins:
                if options.verbose:
                    print(" Processing step %d..." % i)
                output = options.output
                if output:
                    output = output.replace("HERE", "%")
                    output = output % i
                self.plotData(weights, options, weights._unaggEdge2value1[
                              i], weights._unaggEdge2value2[i], output, colorMap)

    def knowsEdge(self, id):
        return id in self._edge2from


class WeightsReader(handler.ContentHandler):

    """Reads the dump file"""

    def __init__(self, net, value1, value2):
        self._id = ''
        self._edge2value2 = {}
        self._edge2value1 = {}
        self._edge2no1 = {}
        self._edge2no2 = {}
        self._net = net
        self._intervalBegins = []
        self._unaggEdge2value2 = {}
        self._unaggEdge2value1 = {}
        self._beginTime = -1
        self._value1 = value1
        self._value2 = value2

    def startElement(self, name, attrs):
        if name == 'interval':
            self._beginTime = float(attrs['begin'])
            self._intervalBegins.append(self._beginTime)
            self._unaggEdge2value2[self._beginTime] = {}
            self._unaggEdge2value1[self._beginTime] = {}
        if name == 'edge':
            if self._net.knowsEdge(attrs['id']):
                self._id = attrs['id']
                if self._id not in self._edge2value2:
                    self._edge2value2[self._id] = 0
                    self._edge2value1[self._id] = 0
                    self._edge2no1[self._id] = 0
                    self._edge2no2[self._id] = 0
                value1 = self._value1
                if value1 in attrs:
                    value1 = float(attrs[value1])
                    self._edge2no1[self._id] = self._edge2no1[self._id] + 1
                else:
                    value1 = 0
                self._edge2value1[self._id] = self._edge2value1[
                    self._id] + value1
                self._unaggEdge2value1[self._beginTime][self._id] = value1
                value2 = self._value2
                if value2 in attrs:
                    value2 = float(attrs[value2])
                    self._edge2no2[self._id] = self._edge2no2[self._id] + 1
                else:
                    value2 = 0
                self._edge2value2[self._id] = self._edge2value2[
                    self._id] + value2
                self._unaggEdge2value2[self._beginTime][self._id] = value2

    def updateExtrema(self, values1ByEdge, values2ByEdge):
        for edge in values1ByEdge:
            if self._minValue1 == -1 or self._minValue1 > values1ByEdge[edge]:
                self._minValue1 = values1ByEdge[edge]
            if self._maxValue1 == -1 or self._maxValue1 < values1ByEdge[edge]:
                self._maxValue1 = values1ByEdge[edge]
            if self._minValue2 == -1 or self._minValue2 > values2ByEdge[edge]:
                self._minValue2 = values2ByEdge[edge]
            if self._maxValue2 == -1 or self._maxValue2 < values2ByEdge[edge]:
                self._maxValue2 = values2ByEdge[edge]

    def valueDependantNorm(self, values, minV, maxV, tendency, percSpeed):
        if tendency:
            for edge in self._edge2value2:
                if values[edge] < 0:
                    values[edge] = 0
                else:
                    values[edge] = 1
        elif percSpeed:
            for edge in self._edge2value2:
                values[edge] = (values[edge] / self._net._edge2speed[edge])
        elif minV != maxV:
            for edge in self._edge2value2:
                values[edge] = (values[edge] - minV) / (maxV - minV)

    def norm(self, tendency, percSpeed):
        self._minValue1 = -1
        self._maxValue1 = -1
        self._minValue2 = -1
        self._maxValue2 = -1
        # compute mean value if join is set
        if options.join:
            for edge in self._edge2value2:
                if float(self._edge2no1[edge]) != 0:
                    self._edge2value1[edge] = float(
                        self._edge2value1[edge]) / float(self._edge2no1[edge])
                else:
                    self._edge2value1[edge] = float(self._edge2value1[edge])
                if float(self._edge2no2[edge]) != 0:
                    self._edge2value2[edge] = float(
                        self._edge2value2[edge]) / float(self._edge2no2[edge])
                else:
                    self._edge2value2[edge] = float(self._edge2value2[edge])
        # compute min/max
        if options.join:
            self.updateExtrema(self._edge2value1, self._edge2value2)
        else:
            for i in weights._intervalBegins:
                self.updateExtrema(
                    self._unaggEdge2value1[i], self._unaggEdge2value2[i])
        # norm
        if options.verbose:
            print("w range: " + str(self._minValue1) +
                  " - " + str(self._maxValue1))
            print("c range: " + str(self._minValue2) +
                  " - " + str(self._maxValue2))
        if options.join:
            self.valueDependantNorm(
                self._edge2value1, self._minValue1, self._maxValue1, False, percSpeed and self._value1 == "speed")
            self.valueDependantNorm(
                self._edge2value2, self._minValue2, self._maxValue2, tendency, percSpeed and self._value2 == "speed")
        else:
            for i in weights._intervalBegins:
                self.valueDependantNorm(self._unaggEdge2value1[
                                        i], self._minValue1, self._maxValue1, False, percSpeed and
                                        self._value1 == "speed")
                self.valueDependantNorm(self._unaggEdge2value2[
                                        i], self._minValue2, self._maxValue2, tendency, percSpeed and
                                        self._value2 == "speed")


# initialise
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
# i/o
optParser.add_option("-n", "--net-file", dest="net",
                     help="SUMO network to use (mandatory)", metavar="FILE")
optParser.add_option("-d", "--dump", dest="dump",
                     help="dump file to use", metavar="FILE")
optParser.add_option("-o", "--output", dest="output",
                     help="(base) name for the output", metavar="FILE")
# data handling
optParser.add_option("-j", "--join", action="store_true", dest="join",
                     default=False, help="sums up values from all read intervals")
optParser.add_option("-w", "--min-width", dest="min_width",
                     type="float", help="sets minimum line width")
optParser.add_option("-W", "--max-width", dest="max_width",
                     type="float", help="sets maximum line width")
optParser.add_option("-c", "--min-color", dest="min_color",
                     type="float", help="sets minimum color (between 0 and 1)")
optParser.add_option("-C", "--max-color", dest="max_color",
                     type="float", help="sets maximum color (between 0 and 1)")
optParser.add_option("--tendency-coloring", action="store_true", dest="tendency_coloring",
                     default=False, help="show only 0/1 color for egative/positive values")
optParser.add_option("--percentage-speed", action="store_true", dest="percentage_speed",
                     default=False, help="speed is normed to maximum allowed speed on an edge")
optParser.add_option("--values", dest="values",
                     type="string", default="entered,speed", help="which values shall be parsed")
optParser.add_option("--color-map", dest="colormap",
                     type="string", default="0:#ff0000,.5:#ffff00,1:#00ff00", help="Defines the color map")
# axes/legend
optParser.add_option("--xticks", dest="xticks", type="string", default="",
                     help="defines ticks on x-axis")
optParser.add_option("--yticks", dest="yticks", type="string", default="",
                     help="defines ticks on y-axis")
optParser.add_option("--xlim", dest="xlim", type="string", default="",
                     help="defines x-axis range")
optParser.add_option("--ylim", dest="ylim", type="string", default="",
                     help="defines y-axis range")
# output
optParser.add_option("--size", dest="size", type="string", default="",
                     help="defines the output size")
# processing
optParser.add_option("-s", "--show", action="store_true", dest="show",
                     default=False, help="shows each plot after generating it")
# parse options
(options, args) = optParser.parse_args()
# check set options
if not options.show and not options.output:
    print("Neither show (--show) not write (--output <FILE>)? Exiting...")
    exit()


# init color map
colorMap = parseColorMap(options.colormap)
# read network
if options.verbose:
    print("Reading net...")
parser = make_parser()
net = NetReader()
parser.setContentHandler(net)
parser.parse(options.net)
# read weights
if options.verbose:
    print("Reading weights...")
mValues = options.values.split(",")
weights = WeightsReader(net, mValues[0], mValues[1])
parser.setContentHandler(weights)
parser.parse(options.dump)
# process
if options.verbose:
    print("Norming weights...")
weights.norm(options.tendency_coloring, options.percentage_speed)
if options.verbose:
    print("Plotting...")
net.plot(weights, options, colorMap)
