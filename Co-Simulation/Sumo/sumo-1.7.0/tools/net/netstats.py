#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    netstats.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2008-08-13

from __future__ import absolute_import
from __future__ import print_function
import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


def renderHTML(values):
    print("<html><body>")
    print("<h1>" + values["netname"] + "</h1></br>")
    # network
    print("<h2>Network</h2></br>")
    # edges
    print("<h2>Edges</h2></br>")
    print("Edge number: " + str(values["edgeNumber"]) + "</br>")
    print("Edgelength sum: " + str(values["edgeLengthSum"]) + "</br>")
    print("Lanelength sum: " + str(values["laneLengthSum"]) + "</br>")
    # nodes
    print("<h2>Nodes</h2></br>")
    print("Node number: " + str(values["nodeNumber"]) + "</br>")
    print("</body></html>")


def renderPNG(values):
    from pylab import bar, show
    bar([0], [values["edgeNumber"]], 1, color='r')
    show()


if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <net>")
    sys.exit()
print("Reading net...")
net = sumolib.net.readNet(sys.argv[1])

values = {}
values["netname"] = "hallo"
values["edgesPerLaneNumber"] = {}
values["edgeLengthSum"] = 0
values["laneLengthSum"] = 0
values["edgeNumber"] = len(net._edges)
values["nodeNumber"] = len(net._nodes)
for e in net._edges:
    values["edgeLengthSum"] = values["edgeLengthSum"] + e._length
    values["laneLengthSum"] = values["laneLengthSum"] + \
        (e._length * float(len(e._lanes)))
    if len(e._lanes) not in values["edgesPerLaneNumber"]:
        values["edgesPerLaneNumber"][len(e._lanes)] = 0
    values["edgesPerLaneNumber"][
        len(e._lanes)] = values["edgesPerLaneNumber"][len(e._lanes)] + 1

renderHTML(values)
renderPNG(values)
