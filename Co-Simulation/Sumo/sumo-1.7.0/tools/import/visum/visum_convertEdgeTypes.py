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

# @file    visum_convertEdgeTypes.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2009-05-27

"""

This script converts edge type definitions (STRECKENTYP) into their
 SUMO-representation.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys

if len(sys.argv) < 3:
    print("Usage: " + sys.argv[0] + " <VISUM-NET> <OUTPUT>")
    sys.exit()
print("Reading VISUM...")
fd = open(sys.argv[1])
fdo = open(sys.argv[2], "w")
fdo.write("<types>\n")
parsingTypes = False
attributes = []
for line in fd:
    if parsingTypes:
        if line[0] == '*' or line[0] == '$' or line.find(";") < 0:
            parsingTypes = False
            continue

        values = line.strip().split(";")
        map = {}
        for i in range(0, len(attributes)):
            map[attributes[i]] = values[i]
        fdo.write('   <type id="' + map["nr"])
        fdo.write('" priority="' + str(100 - int(map["rang"])))
        fdo.write('" numLanes="' + map["anzfahrstreifen"])
        fdo.write('" speed="' + str(float(map["v0iv"]) / 3.6))
        fdo.write('"/>\n')

    if line.find("$STRECKENTYP") == 0:
        parsingTypes = True
        attributes = line[len("$STRECKENTYP:"):].lower().split(";")
fdo.write("</types>\n")
