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

# @file    netextract.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Michael Behrisch
# @date    2007-02-21

"""
This script reads in the network given as
 first parameter and extracts nodes and edges
 from it which are saved into "nodes.xml" and
 "edges.xml" for their reuse in NETCONVERT

todo:
- parse connections
- parse tls information
"""
from __future__ import absolute_import
from __future__ import print_function


import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def writeNodes(net):
    """ Writes nodes in a xml file """

    fd = open("nodes.xml", "w")
    fd.write("<nodes>\n")
    for node in net._nodes:
        fd.write("   <node id=\"" + node._id + "\" x=\"" +
                 str(node._coord[0]) + "\" y=\"" + str(node._coord[1]) + "\"/>\n")
    fd.write("</nodes>\n")


def writeEdges(net):
    """ Writes edges in a xml file """

    fd = open("edges.xml", "w")
    fd.write("<edges>\n")
    for edge in net._edges:
        fd.write("   <edge id=\"" + edge._id + "\" from=\"" +
                 edge._from._id + "\" to=\"" + edge._to._id)
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


def writeConnections(net):
    with open("connections.xml", "w") as fd:
        fd.write("<connections>\n")

        # take care to write output in stable order
        for edge in sorted(net._edges, key=lambda e: e.getID()):
            for toEdge in sorted(edge.getOutgoing().keys(), key=lambda e: e.getID()):
                for conn in edge.getOutgoing()[toEdge]:
                    fd.write('<connection from="%s" to="%s" fromLane="%s" toLane="%s"/>\n' %
                             (edge.getID(),
                              toEdge.getID(),
                              conn.getFromLane().getIndex(),
                              conn.getToLane().getIndex()))

        fd.write("</connections>\n")


if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <net>")
    sys.exit()
print("Reading net...")
net = sumolib.net.readNet(sys.argv[1])
print("Writing nodes...")
writeNodes(net)
print("Writing edges...")
writeEdges(net)
print("Writing connections...")
writeConnections(net)
