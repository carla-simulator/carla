#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    makeRoutes.py
# @author  Daniel Krajzewicz
# @date    2014-10-06


from __future__ import print_function

flows = [
    ["nm", [
        ["ms", 160, 8],
        ["me", 60, 10],
        ["mw", 67, 11]
    ]],

    ["wm", [
        ["me", 662, 11],
        ["mn", 64, 9],
        ["ms", 193, 5]
    ]],

    ["em", [
        ["mw", 617, 11],
        ["mn", 55, 6],
        ["ms", 50, 4]
    ]],

    ["sm", [
        ["mn", 156, 9],
        ["me", 59, 3],
        ["mw", 191, 5]
    ]]

]


fd = open("input_flows.flows.xml", "w")
print("<routes>", file=fd)


for s in flows:
    for d in s[1]:
        id = s[0] + '2' + d[0]
        noLKW = int(float(d[1]) * float(d[2]) / 100.)  # hmph, so korrekt?
        noPKW = d[1] - noLKW
        print(('     <flow id="%sPKW" from="%s" to="%s" number="%s" type="PKW" begin="0" end="3600" ' +
               'departPos="base" arrivalPos="-1" departSpeed="max" departLane="best"/>') % (
            id, s[0], d[0], noPKW), file=fd)
        print(('     <flow id="%sLKW" from="%s" to="%s" number="%s" type="LKW" begin="0" end="3600" ' +
               'departPos="base" arrivalPos="-1" departSpeed="max" departLane="best"/>') % (
            id, s[0], d[0], noLKW), file=fd)
    print("", file=fd)

print("</routes>\n", file=fd)
fd.close()
