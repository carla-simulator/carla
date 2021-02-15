# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    ns2.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2013-01-15

"""
This module includes functions for converting SUMO's fcd-output into
data files read by ns2.
"""
from __future__ import print_function
from __future__ import absolute_import
import sumolib.output
import sumolib.net


def fcd2ns2mobility(inpFCD, outSTRM, further):
    vIDm = sumolib._Running(further["orig-ids"], True)
    checkGaps = not further["ignore-gaps"]
    begin = -1
    end = None
    area = [None, None, None, None]
    vehInfo = {}
    removed = set()
    ignoring = set()
    for timestep in inpFCD:
        if begin < 0:
            begin = timestep.time
        end = timestep.time
        seen = set()
        if not timestep.vehicle and checkGaps:
            _writeMissing(timestep.time, vIDm, seen, vehInfo, removed)
            continue
        for v in timestep.vehicle:
            if v.id in ignoring:
                continue
            if v.id in removed:
                print(
                    "Warning: vehicle %s reappeared after being gone and will be ignored" % v.id)
                ignoring.add(v.id)
                continue

            seen.add(v.id)
            if not vIDm.k(v.id):
                nid = vIDm.g(v.id)
                if outSTRM:
                    print("$node_(%s) set X_ %s" % (nid, v.x), file=outSTRM)
                    print("$node_(%s) set Y_ %s" % (nid, v.y), file=outSTRM)
                    print("$node_(%s) set Z_ %s" % (nid, 0), file=outSTRM)
                vehInfo[v.id] = [nid, timestep.time, 0]
            nid = vIDm.g(v.id)
            if outSTRM:
                print('$ns_ at %s "$node_(%s) setdest %s %s %s"' %
                      (timestep.time, nid, v.x, v.y, v.speed), file=outSTRM)
            if not area[0]:
                area[0] = v.x
                area[1] = v.y
                area[2] = v.x
                area[3] = v.y
            area[0] = min(area[0], v.x)
            area[1] = min(area[1], v.y)
            area[2] = max(area[2], v.x)
            area[3] = max(area[3], v.y)
        if checkGaps:
            _writeMissing(timestep.time, vIDm, seen, vehInfo, removed)
    _writeMissing(timestep.time, vIDm, seen, vehInfo, removed)
    return vIDm, vehInfo, begin, end, area


def writeNS2activity(outSTRM, vehInfo):
    for v in sorted(vehInfo):
        i = vehInfo[v]
        print('$ns_ at %s "$g(%s) start"; # SUMO-ID: %s' %
              (i[1], i[0], v), file=outSTRM)
        print('$ns_ at %s "$g(%s) stop"; # SUMO-ID: %s' %
              (i[2], i[0], v), file=outSTRM)


def writeNS2config(outSTRM, vehInfo, ns2activityfile, ns2mobilityfile, begin, end, area):
    print("# set number of nodes\nset opt(nn) %s\n" %
          len(vehInfo), file=outSTRM)
    if ns2activityfile:
        print("# set activity file\nset opt(af) $opt(config-path)\nappend opt(af) /%s\n" %
              ns2activityfile, file=outSTRM)
    if ns2mobilityfile:
        print("# set mobility file\nset opt(mf) $opt(config-path)\nappend opt(mf) /%s\n" %
              ns2mobilityfile, file=outSTRM)
    xmin = area[0]
    ymin = area[1]
    xmax = area[2]
    ymax = area[3]
    print("# set start/stop time\nset opt(start) %s\nset opt(stop) %s\n" %
          (begin, end), file=outSTRM)
    print("# set floor size\nset opt(x) %s\nset opt(y) %s\nset opt(min-x) %s\nset opt(min-y) %s\n" %
          (xmax, ymax, xmin, ymin), file=outSTRM)


def _writeMissing(t, vIDm, seen, vehInfo, removed):
    toDel = []
    for v in vIDm._m:
        if v in seen:
            continue
        vIDm.g(v)
        vehInfo[v][2] = t
        toDel.append(v)
        removed.add(v)
    for v in toDel:
        vIDm.d(v)
