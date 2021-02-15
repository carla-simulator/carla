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

# @file    tls_csv2SUMO.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2009-08-01

"""
Converts a csv-tls-description into one SUMO can read as additional file.
Format of the csv-file:

key;105
subkey;utopia
offset;0
(link;<LINK_NUMBER>;<FROMEDGE>|<FROMLANE>;[<TOEDGE>|<TOLANE>];0)+
(param;<NAME>;<VALUE>)+
(<LINK_NUMBER>;<PHASE_LIGHT>[;<PHASE_LIGHT>]+)+
(min;<PHASE_LENGTH>[;<PHASE_LENGTH>]+)*
time;<PHASE_LENGTH>[;<PHASE_LENGTH>]+
(max;<PHASE_LENGTH>[;<PHASE_LENGTH>]+)*


"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import os
import argparse

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa


class Logic:
    def __init__(self, key):
        self.minTimes = []
        self.maxTimes = []
        self.normTimes = []
        self.defs = []
        self.links = []
        self.params = []
        self.key = key
        self.subkey = ""
        self.offset = 0
        self.links2index = {}
        self.links2sigGrpPhase = {}


def computeLinkPhasesAndTimes(logic):
    phases = {}
    # compute all boundaries between phases
    for phaseDef in logic.links2sigGrpPhase.values():
        for i in range(0, len(phaseDef), 2):
            phases[int(phaseDef[i])] = True
    lastTime = 0
    newTimes = []
    sortPhases = []

    # sort the boundaries and compute the normTimes (here newTimes)) list
    for time in sorted(phases):
        if int(time) != 0:
            sortPhases.append(time)
        if int(time) > lastTime:
            newTimes.append(str(int(time) - lastTime))
        lastTime = int(time)
    logic.normTimes = newTimes
    if args.debug:
        print('normTimes', newTimes, file=sys.stderr)

    # create the phase string
    for myKey, phaseDef in logic.links2sigGrpPhase.items():
        newPhases = []
        sigGrpPhaseIndex = 1
        if args.debug:
            print('phaseDef', phaseDef, 'len', len(phaseDef), file=sys.stderr)
        for currentPhase in sortPhases:
            if args.debug:
                print('SGPIndex', sigGrpPhaseIndex, 'startCurrPhase', currentPhase,
                      'endCurrPhase', phaseDef[sigGrpPhaseIndex + 1], file=sys.stderr)
            newPhases.append(phaseDef[sigGrpPhaseIndex])
            if int(phaseDef[sigGrpPhaseIndex + 1]) == currentPhase and sigGrpPhaseIndex + 2 < len(phaseDef):
                sigGrpPhaseIndex += 2
        if args.debug:
            print(myKey, newPhases, file=sys.stderr)
        logic.links2index[myKey] = len(logic.defs)
        logic.defs.append(newPhases)


parser = argparse.ArgumentParser(description='Create tls xml def from csv.')
parser.add_argument('TLS_CSV', help='tls definition')
parser.add_argument('NET', help='sumo net file')
parser.add_argument('-d', '--debug', action='store_true', help='print additional debug info')
args = parser.parse_args()

allLogics = []

for tlsFile in args.TLS_CSV.split(","):
    fd = open(tlsFile)
    logic = Logic(None)
    hasSigGrpPhase = False
    for line in fd:
        line = line.strip()
        if len(line) == 0 or line[0] == '#':
            continue
        v = line.split(";")
        if v[0] == "key":
            if logic.key is not None:
                allLogics.append(logic)
            logic = Logic(v[1])
        elif v[0] == "subkey":
            logic.subkey = v[1]
        elif v[0] == "offset":
            logic.offset = v[1]
        elif v[0] == "link":
            logic.links.append([v[2], v[3], v[1], v[4] == "1"])
        elif v[0] == "param":
            logic.params.append([v[1], v[2]])
        elif v[0] == "min":
            logic.minTimes = v[1:]
        elif v[0] == "max":
            logic.maxTimes = v[1:]
        elif v[0] == "time":
            logic.normTimes = v[1:]
        elif v[0] == "siggrpphase":
            logic.links2sigGrpPhase[v[1]] = v[2:]
            hasSigGrpPhase = True
        else:
            if len(v) > 1:
                logic.links2index[v[0]] = len(logic.defs)
                logic.defs.append(v[1:])

    if logic.key is None:
        print("Warning: No logic definition in", tlsFile, file=sys.stderr)
    else:
        if hasSigGrpPhase:
            computeLinkPhasesAndTimes(logic)
        if len(logic.defs) > 0:
            logic.links2index[-1] = len(logic.defs)
            logic.defs.append(['g'] * len(logic.defs[0]))
        allLogics.append(logic)
    fd.close()

net1 = sumolib.net.readNet(args.NET)

print('<?xml version="1.0" encoding="UTF-8"?>\n<add>')
for logic in allLogics:
    tls = net1._id2tls[logic.key]
    noConnections = tls._maxConnectionNo + 1
    linkMap = [0] * noConnections
    laneMap = [None] * noConnections
    for tl_c in tls._connections:
        li = tl_c[0]  # incoming lane in our net
        lo = tl_c[1]  # outgoing lane in our net
        for link in logic.links:
            valid = True
            if link[0].find('_') < 0:
                # edge only given
                if link[0] != li.getEdge().getID():
                    valid = False
            else:
                # lane given
                if link[0] != li.getID():
                    valid = False
            if link[1] != "":
                if link[1].find('_') < 0:
                    # edge only given
                    if link[1] != lo.getEdge().getID():
                        valid = False
                else:
                    # lane given
                    if link[1] != lo.getID():
                        valid = False
            if valid:
                linkMap[tl_c[2]] = link[2]
                laneMap[tl_c[2]] = (li, lo)
        if laneMap[tl_c[2]] is None:
            print("Warning: No link definition for connection (%s, %s)!. Using 'g' by default" % (
                li.getID(), lo.getID()), file=sys.stderr)
            linkMap[tl_c[2]] = -1
            laneMap[tl_c[2]] = (li, lo)

    nodes = set()
    for lane in laneMap:
        if lane:
            nodes.add(lane[0].getEdge()._to)

    indices = {}
    for n in nodes:
        indices[n] = {}
        index = 0
        for i in n._incLanes:
            e = '_'.join(i.split("_")[:-1])
            laneIndex = i.split("_")[-1]
            if e in net1._crossings_and_walkingAreas:
                continue
            e = net1._id2edge[e]
            li = e._lanes[int(laneIndex)]
            for c in li._outgoing:
                indices[n][(li, c._toLane)] = index
                index = index + 1

    for l in range(0, len(linkMap)):
        if linkMap[l] not in logic.links2index:
            print("Error: Link %s is not described (%s)!" % (
                l, linkMap[l]), file=sys.stderr)
            sys.exit()

    print('    <tlLogic id="' + logic.key + '" type="static" programID="' +
          logic.subkey + '" offset="' + logic.offset + '">')
    for p in logic.params:
        print('        <param key="' + p[0] + '" value="' + p[1] + '"/>')

    for i in range(0, len(logic.normTimes)):
        state = ""
        for l in range(0, len(linkMap)):
            index = logic.links2index[linkMap[l]]
            d = logic.defs[index]
            if d[i] == 'r':
                state = state + "r"
            elif d[i] == 'y' or d[i] == 'a':
                state = state + "y"
            elif d[i] == 'g':
                state = state + "g"
            elif d[i] == 'o' or d[i] == 'x':
                state = state + "o"
            else:
                sys.stderr.write(
                    "missing value %s at %s (%s); setting to g\n" % (d[i], index, linkMap[l]))
                state = state + "g"
        for l1 in range(0, len(state)):
            if state[l1] == 'g':
                wait = False
                for l2 in range(0, len(state)):
                    if l1 == l2:
                        continue
                    if state[l2] != 'g' and state[l2] != 'G' and state[l2] != 'y' and state[l2] != 'Y':
                        continue
                    link1 = laneMap[l1]
                    link2 = laneMap[l2]
                    node1 = link1[0].getEdge()._to
                    node2 = link2[0].getEdge()._to
                    mustWait = node1 == node2
                    if mustWait:
                        mindices = indices[node1]
                        mprohs = node1._prohibits
                        mustWait = mprohs[mindices[link1]][
                            len(mprohs) - mindices[link2] - 1] == '1'
                    wait = wait or mustWait
                if not wait:
                    state = state[:l1] + 'G' + state[l1 + 1:]

        pd = '        <phase duration="' + logic.normTimes[i] + '" '
        pd = pd + 'state="' + state + '"'
        if len(logic.minTimes) == len(logic.normTimes):
            pd = pd + ' minDur="' + logic.minTimes[i] + '"'
        if len(logic.maxTimes) == len(logic.normTimes):
            pd = pd + ' maxDur="' + logic.maxTimes[i] + '"'
        pd = pd + '/>'
        print(pd)
    print("    </tlLogic>")
print("</add>\n")
