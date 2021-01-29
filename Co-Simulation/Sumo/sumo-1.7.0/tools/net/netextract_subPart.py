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

# @file    netextract_subPart.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-02-21

"""

This script reads in the network given as first parameter.
It extracts nodes given on the command line as well as the edges
 that participate in these nodes and the nodes these edges
 start/end at. Write the so obtained nodes and edges
 into "<prefix>_nodes.nod.xml" and "<prefix>_edges.edg.xml"
 for their reuse in NETCONVERT
"""
from __future__ import absolute_import
from __future__ import print_function


import os
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def writeNodes(net, file):
    fd = open(file, "w")
    fd.write("<nodes>\n")
    for node in net._nodes:
        fd.write("   <node id=\"" + node._id + "\" x=\"" +
                 str(node._coord[0]) + "\" y=\"" + str(node._coord[1]) + "\"/>\n")
    fd.write("</nodes>\n")


def writeEdges(net, file):
    fd = open(file, "w")
    fd.write("<edges>\n")
    for edge in net._edges:
        fd.write("   <edge id=\"" + edge._id + "\" fromNode=\"" +
                 edge._from._id + "\" toNode=\"" + edge._to._id)
        fd.write("\" speed=\"" + str(edge._speed))
        fd.write("\" priority=\"" + str(edge._priority))
        fd.write("\" spreadType=\"center")
        fd.write("\" numLanes=\"" + str(len(edge._lanes)) + "\"")
        shape = edge.getShape()
        fd.write(" shape=\"")
        for i, c in enumerate(shape):
            if i != 0:
                fd.write(" ")
            fd.write(str(c[0]) + "," + str(c[1]))
        fd.write("\"")
        fd.write("/>\n")
    fd.write("</edges>\n")


if len(sys.argv) < 4:
    print("Usage: " + sys.argv[0] + " <net> <prefix> <nodes>")
    sys.exit()
print("Reading net...")
net = sumolib.net.readNet(sys.argv[1])

edges = set()
nodes = set()
for n in sys.argv[3].split(","):
    n = net.getNode(n)
    nodes.add(n)
    for e in n._incoming:
        edges.add(e)
        nodes.add(e._from)
    for e in n._outgoing:
        edges.add(e)
        nodes.add(e._to)

net = sumolib.net.Net()
for e in edges:
    c = net.addEdge(e._id, e._from._id, e._to._id, e._priority, e._function)
    for l in e._lanes:
        lane = sumolib.net.Lane(c, l.getSpeed(), l.getLength())
        lane.setShape(l.getShape())
    c.rebuildShape()
for n in nodes:
    net.addNode(n._id, n._coord)

print("Writing nodes...")
writeNodes(net, sys.argv[2] + "_nodes.nod.xml")
print("Writing edges...")
writeEdges(net, sys.argv[2] + "_edges.edg.xml")
