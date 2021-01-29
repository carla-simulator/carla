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

# @file    visum_parseZaehlstelle.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2008-11-30

"""

This script reads "Zaehlstellen" from a given VISUM-network
 and projects them onto a given SUMO-network.
The parsed "Zaehlstellen" are written as POIs.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import os
from xml.sax import make_parser

sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), "../../lib"))
import sumonet  # noqa

if len(sys.argv) < 3:
    print("Usage: " + sys.argv[0] + " <SUMO-net> <VISUM-net> <output>")
    sys.exit()
print("Reading net...")
parser = make_parser()
net = sumonet.NetReader()
parser.setContentHandler(net)
parser.parse(sys.argv[1])

print("Reading VISUM...")
fd = open(sys.argv[2])
fdo = open(sys.argv[3], "w")
fdo.write("<pois>\n")
parsingCounts = False
lastKnown = ""
for line in fd:
    if parsingCounts:
        if line[0] == '*' or line[0] == '$' or line.find(";") < 0:
            parsingCounts = False
            continue

        print(line)
        vals = line.split(";")
        id = vals[0] + ";" + vals[1]
        fromNode = vals[3]
        toNode = vals[4]
        strID = vals[5]
        rest = ";".join(vals[lastKnown + 1:]).strip()
        fN = net.getNet()._id2node[fromNode]
        me = None
        for e in fN._outgoing:
            if e._id == strID or e._id == "-" + strID:
                me = e
        if me is None:
            print("Not found " + line)
        else:
            p = me._length * float(vals[6])
            fdo.write('    <poi id="%s" type="%s" lane="%s_0" pos="%s" color="0,1,0" values="%s" layer="1"/>\n' %
                      (id, vals[7], me._id, p, rest))

    if line.find("$ZAEHLSTELLE") == 0:
        parsingCounts = True
        lastKnown = line.split(";").index("ISTINAUSWAHL")
fdo.write("</pois>\n")
