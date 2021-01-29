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

# @file    createRoundaboutConnections.py
# @author  Laura Bieker
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-02-21

"""
This script reads in the network given as
 first parameter and generates additional connections for roundabouts.
"""
from __future__ import absolute_import
from __future__ import print_function


import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def writeConnections(net):
    fd = open("roundabout-connection.con.xml", "w")
    fd.write("<connections>\n")
    for ra in net.getRoundabouts():
        for node in ra.getNodes():
            edgesOut = net.getNode(node)._outgoing
            edgesIn = net.getNode(node)._incoming
            for edgeOut in edgesOut:
                outNumber = edgeOut.getLaneNumber()
                for edgeIn in edgesIn:
                    if edgeOut not in edgeIn._outgoing:
                        continue
                    inNumber = edgeIn.getLaneNumber()
                    for x in range(inNumber):
                        if x < inNumber and x < outNumber:
                            fd.write("   <connection from=\"" + str(edgeIn._id) + "\" to=\"" + str(
                                edgeOut._id) + "\" lane=\"" + str(x) + ":" + str(x) + "\" />\n")
    fd.write("</connections>\n")


if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <net>")
    sys.exit()
print("Reading net...")
net = sumolib.net.readNet(sys.argv[1])
print("Writing connections...")
writeConnections(net)
