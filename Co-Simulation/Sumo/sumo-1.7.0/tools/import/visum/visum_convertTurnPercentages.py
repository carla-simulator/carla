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

# @file    visum_convertTurnPercentages.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2009-06-12

"""
Converts VISUM turning percentages into
 turning percentages JTRROUTER can read.
"""
from __future__ import absolute_import
from __future__ import print_function


import os
import sys
from xml.sax import make_parser

sys.path.append(
    os.path.join(os.path.abspath(os.path.dirname(sys.argv[0])), "../../lib"))
import sumonet  # noqa


if len(sys.argv) < 4:
    print("Usage: " + sys.argv[0] + " <SUMO_NET> <VISUM_TURNINGS> <OUTPUT>")
    sys.exit()
print("Reading net...")
parser = make_parser()
net = sumonet.NetReader()
parser.setContentHandler(net)
parser.parse(sys.argv[1])
net = net.getNet()

# initialise edges and nodes map
emap = {}
nmap = {}
for e in net._edges:
    emap[e._id] = {}
    if e._from._id not in nmap:
        nmap[e._from._id] = {}
    for o in e._outgoing:
        emap[e._id][o._id] = 0
        if o._to._id not in nmap[e._from._id]:
            nmap[e._from._id][o._to._id] = (e._id, o._id)


# fill with read values
print("Reading turning percentages...")
parse = False
found = 0
foundN = 0
missing = 0
missingN = 0
attributes = []
fd = open(sys.argv[2])
for line in fd:
    if line.find("$") == 0 or line.find("*") == 0 or line.find(";") < 0:
        parse = False

    if parse:
        values = line.strip().split(";")
        amap = {}
        for i in range(0, len(attributes)):
            amap[attributes[i]] = values[i]
        fromnode = amap["fromnodeno"]
        tonode = amap["tonodeno"]
        vianode = amap["vianodeno"]
        number = float(amap["volvehprt(ah)"])
        if fromnode not in nmap:
            if number != 0:
                print("Missing from-node '" + fromnode + "'; skipping")
            missing = missing + 1
            missingN = missingN + number
            continue
        if tonode not in nmap[fromnode]:
            if number != 0:
                print("No connection between from-node '" + fromnode +
                      "' and to-node '" + tonode + "'; skipping")
            missing = missing + 1
            missingN = missingN + number
            continue
        (fromedge, toedge) = nmap[fromnode][tonode]
        emap[fromedge][toedge] = number
        found = found + 1
        foundN = foundN + number

    if line.find("$TURN:") == 0:
        attributes = line[6:].strip().lower().split(";")
        parse = True
fd.close()
print(" " + str(found) + " connections found (" + str(foundN) + " vehs)")
print(" " + str(missing) + " connections missing (" + str(missingN) + " vehs)")


# write as read by jtrrouter
print("Writing jtrrouter turning percentages...")
fd = open(sys.argv[3], "w")
fd.write('<turns>\n')
fd.write('    <interval begin="0" end="86400">\n')
for i in emap:
    fd.write('        <fromEdge id="' + i + '">\n')
    sum = 0
    for o in emap[i]:
        sum = sum + emap[i][o]
    for o in emap[i]:
        if sum != 0:
            no = emap[i][o] / sum
        else:
            no = 0
        fd.write('            <toEdge id="' + o +
                 '" probability="' + str(no) + '"/>\n')
    fd.write('        </fromEdge>\n')
fd.write('    </interval>\n')
fd.write('</turns>\n')
