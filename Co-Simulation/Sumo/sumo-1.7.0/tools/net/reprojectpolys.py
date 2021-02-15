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

# @file    reprojectpolys.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2008-03-27

"""
This script computes projection needed to make net2 (-2) fit
 to net1 (-1). The read polygons (-p) are then reprojected
 using this projection.

This means, the resulting polygons, written to the output
 specified by --output, are net2's polygons as they would
 be on net1.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
from optparse import OptionParser
from xml.sax import make_parser, handler
sys.path.append("../lib")
import rmsd  # noqa


class NetReader(handler.ContentHandler):

    """Reads a network, storing nodes and their positions"""

    def __init__(self):
        self._id = ''
        self._node2x = {}
        self._node2y = {}
        self._nodes = []

    def startElement(self, name, attrs):
        if name == 'junction':
            self._id = attrs['id']
            if self._id[0] != ':':
                self._node2x[attrs['id']] = float(attrs['x'])
                self._node2y[attrs['id']] = float(attrs['y'])
                self._nodes.append(attrs['id'])
            else:
                self._id = ""

    def getNodeIndex(self, name):
        for i, n in enumerate(self._nodes):
            if n == name:
                return i
        return -1

    def getNodePositionList(self):
        ret = []
        for n in self._nodes:
            ret.append((self._node2x[n], self._node2y[n]))
        return ret


class PolyReader(handler.ContentHandler):

    """Reads a list of polygons, stores them"""

    def __init__(self):
        self._polys = []
        self._parsingPoly = False
        self._shapes = []

    def startElement(self, name, attrs):
        self._parsingPoly = False
        self._chars = ""
        if name == 'poly':
            self._parsingPoly = True
            self._polys.append({})
            for attr in attrs.keys():
                self._polys[-1][attr] = attrs[attr]

    def characters(self, chars):
        if self._parsingPoly:
            self._chars = self._chars + chars

    def endElement(self, name):
        if self._parsingPoly:
            poses = self._chars.split(' ')
            shape = []
            for pos in poses:
                coord = pos.split(',')
                shape.append([float(coord[0]), float(coord[1])])
                self._shapes.append([float(coord[0]), float(coord[1])])
            self._polys[-1]['shape'] = shape

    def getPositionList(self):
        return self._shapes

    def write(self, into):
        for poly in self._polys:
            into.write("    <poly id=\"" + poly['id'])
            for attr in poly:
                if attr != "id" and attr != "shape":
                    into.write("\" " + attr + "=\"" + poly[attr])
            into.write("\">")
            shape = poly["shape"]
            for i, c in enumerate(shape):
                if i != 0:
                    into.write(" ")
                into.write(str(c[0]) + "," + str(c[1]))
            into.write("</poly>\n")


class PolyReprojector:

    def __init__(self, net1, net2):
        self._net1 = net1
        self._net2 = net2

    def match(self, nodes1, nodes2, polys, verbose):
        nodes1 = nodes1.split(',')
        nodes2 = nodes2.split(',')
        # build match matrix for nodes
        #  and lists of matching indices
        rmsdSelection1 = []
        rmsdSelection2 = []
        if verbose:
            print(" Setting initial nodes...")
        for i in range(0, len(nodes1)):
            index1 = self._net1.getNodeIndex(nodes1[i])
            index2 = self._net2.getNodeIndex(nodes2[i])
            rmsdSelection1.append(index1)
            rmsdSelection2.append(index2)
            if verbose:
                print(str(index1) + " " + str(index2))
        # build rmsd matrices
        if verbose:
            print(" Computing projection...")
        rmsdNodePositions1 = self._net1.getNodePositionList()
        rmsdNodePositions2 = self._net2.getNodePositionList()
        rmsdNodePositions2.extend(polys.getPositionList())
        projection = rmsd.superpose(
            rmsdNodePositions1, rmsdNodePositions2, rmsdSelection1, rmsdSelection2)
        # we now have new coordinates for the second node set in projection
        #  transfer to net
        if verbose:
            print(" Applying projection...")
        index = 0
        for i, n in enumerate(self._net2._nodes):
            self._net2._node2x[n] = projection[i][0]
            self._net2._node2y[n] = projection[i][1]
            index = index + 1
        for poly in polys._polys:
            for i in range(0, len(poly["shape"])):
                poly["shape"][i][0] = projection[i][0]
                poly["shape"][i][1] = projection[i][1]
                index = index + 1


# initialise
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
# i/o
optParser.add_option("-1", "--net1", dest="net1",
                     help="The network to project at", metavar="FILE")
optParser.add_option("-2", "--net2", dest="net2",
                     help="The network to reproject", metavar="FILE")
optParser.add_option("-p", "--polys", dest="polys",
                     help="The polygons to reproject", metavar="FILE")
optParser.add_option("-a", "--nodes1", dest="nodes1",
                     help="The first matching nodes", metavar="FILE")
optParser.add_option("-b", "--nodes2", dest="nodes2",
                     help="The second matching nodes", metavar="FILE")
optParser.add_option("-o", "--output", dest="output",
                     help="File to save reprojected polys into", metavar="FILE")
# parse options
(options, args) = optParser.parse_args()


# read network#1
if options.verbose:
    print("Reading net#1...")
parser = make_parser()
net1 = NetReader()
parser.setContentHandler(net1)
parser.parse(options.net1)

# read network#2
if options.verbose:
    print("Reading net#2...")
parser = make_parser()
net2 = NetReader()
parser.setContentHandler(net2)
parser.parse(options.net2)

# read polygons
if options.verbose:
    print("Reading polygons...")
parser = make_parser()
polys = PolyReader()
parser.setContentHandler(polys)
parser.parse(options.polys)

# match
if options.verbose:
    print("(Re)Projecting...")
matcher = PolyReprojector(net1, net2)
matcher.match(options.nodes1, options.nodes2, polys, options.verbose)

# write
if options.verbose:
    print("Writing...")
fd = open(options.output, "w")
fd.write("<polygons>\n\n")
polys.write(fd)
fd.write("</polygons>\n")
fd.close()
