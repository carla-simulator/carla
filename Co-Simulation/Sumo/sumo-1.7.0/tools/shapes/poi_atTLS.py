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

# @file    poi_atTLS.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2010-02-20

"""
Generates a PoI-file containing a PoI for each tls controlled intersection
 from the given net.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <NET>", file=sys.stderr)
    sys.exit()

print("Reading net...")
net1 = sumolib.net.readNet(sys.argv[1], withPrograms=True)


print("Writing output...")
fdo = open('pois.add.xml', 'w')
print('<?xml version="1.0"?>', file=fdo)
print('<!-- poi_atTLS %s -->' % sys.argv, file=fdo)
print('<additional>', file=fdo)
for tlsID in net1._id2tls:
    tls = net1._id2tls[tlsID]
    nodes = set()
    for c in tls._connections:
        iLane = c[0]
        iEdge = iLane.getEdge()
        nodes.add(iEdge._to)
    if len(sys.argv) > 2 and sys.argv[2] != "nojoin":
        c = [0, 0]
        for n in nodes:
            c[0] += n._coord[0]
            c[1] += n._coord[1]
        if len(nodes) > 1:
            c[0] = c[0] / float(len(nodes))
            c[1] = c[1] / float(len(nodes))
        print('    <poi id="%s" type="default" color="1,0,0" layer="0" x="%s" y="%s"/>' % (
            tlsID, c[0], c[1]), file=fdo)
    else:
        for n in nodes:
            print('    <poi id="%s_at_%s" type="default" color="1,0,0" layer="0" x="%s" y="%s"/>' % (
                tlsID, n._id, n._coord[0], n._coord[1]), file=fdo)

print('</additional>', file=fdo)
fdo.close()
