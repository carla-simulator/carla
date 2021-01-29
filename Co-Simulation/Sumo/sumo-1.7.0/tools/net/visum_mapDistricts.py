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

# @file    visum_mapDistricts.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

"""

This script reads a network and a dump file and
 draws the network, coloring it by the values
 found within the dump-file.
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
import math
from optparse import OptionParser

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa
import netshiftadaptor  # noqa


def computeDistance(n1, n2):
    xd = n1._coord[0] - n2._coord[0]
    yd = n1._coord[1] - n2._coord[1]
    return math.sqrt(xd * xd + yd * yd)


def relAngle(angle1, angle2):
    angle2 -= angle1
    if angle2 > 180:
        angle2 = (360. - angle2) * -1.
    while angle2 < -180:
        angle2 = 360 + angle2
    return angle2


# initialise
optParser = OptionParser()
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
# i/o
optParser.add_option("-1", "--net1", dest="net1",
                     help="SUMO network to use (mandatory)", metavar="FILE")
optParser.add_option("-2", "--net2", dest="net2",
                     help="SUMO network to use (mandatory)", metavar="FILE")
optParser.add_option("-a", "--nodes1", dest="nodes1",
                     help="The first matching nodes", metavar="NODELIST")
optParser.add_option("-b", "--nodes2", dest="nodes2",
                     help="The second matching nodes", metavar="NODELIST")
# parse options
(options, args) = optParser.parse_args()


# read networks
if options.verbose:
    print("Reading net#1...")
net1 = sumolib.net.readNet(options.net1)

if options.verbose:
    print("Reading net#2...")
net2 = sumolib.net.readNet(options.net2)

# reproject the visum net onto the navteq net
adaptor = netshiftadaptor.NetShiftAdaptor(
    net1, net2, options.nodes1.split(","), options.nodes2.split(","))
adaptor.reproject(options.verbose)

# build a speed-up grid
xmin = 100000
xmax = -100000
ymin = 100000
ymax = -100000
for n in net1._nodes:
    xmin = min(xmin, n._coord[0])
    xmax = max(xmax, n._coord[0])
    ymin = min(ymin, n._coord[1])
    ymax = max(ymax, n._coord[1])
for n in net2._nodes:
    xmin = min(xmin, n._coord[0])
    xmax = max(xmax, n._coord[0])
    ymin = min(ymin, n._coord[1])
    ymax = max(ymax, n._coord[1])
xmin = xmin - .1
xmax = xmax + .1
ymin = ymin - .1
ymax = ymax + .1


CELLSIZE = 100
arr1 = []
arr2 = []
for y in range(0, CELLSIZE):
    arr1.append([])
    arr2.append([])
    for x in range(0, CELLSIZE):
        arr1[-1].append([])
        arr2[-1].append([])

cw = (xmax - xmin) / float(CELLSIZE)
ch = (ymax - ymin) / float(CELLSIZE)
for n in net2._nodes:
    cx = (n._coord[0] - xmin) / cw
    cy = (n._coord[1] - ymin) / ch
    arr1[int(cy)][int(cx)].append(n)
for n in net1._nodes:
    cx = (n._coord[0] - xmin) / cw
    cy = (n._coord[1] - ymin) / ch
    arr2[int(cy)][int(cx)].append(n)


# map
nmap1to2 = {}
nmap2to1 = {}
nodes1 = net2._nodes
nodes2 = net1._nodes
highwayNodes2 = set()
highwaySinks2 = set()
highwaySources2 = set()
urbanNodes2 = set()
for n2 in nodes2:
    noIncoming = 0
    noOutgoing = 0
    for e in n2._outgoing:
        if e.getSpeed() > 80. / 3.6 and e.getSpeed() < 99:
            highwayNodes2.add(n2)
        if e.getSpeed() < 99:
            noOutgoing = noOutgoing + 1
    for e in n2._incoming:
        if e.getSpeed() > 80. / 3.6 and e.getSpeed() < 99:
            highwayNodes2.add(n2)
        if e.getSpeed() < 99:
            noIncoming = noIncoming + 1
    if n2 in highwayNodes2:
        if noOutgoing == 0:
            highwaySinks2.add(n2)
        if noIncoming == 0:
            highwaySources2.add(n2)
    else:
        urbanNodes2.add(n2)
print("Found " + str(len(highwaySinks2)) + " highway sinks in net2")
cont = ""
for n in highwaySinks2:
    cont = cont + n._id + ", "
print(cont)
cont = ""
print("Found " + str(len(highwaySources2)) + " highway sources in net2")
for n in highwaySources2:
    cont = cont + n._id + ", "
print(cont)


fdd = open("dconns.con.xml", "w")
fdd.write("<connections>\n")
highwaySinks1 = set()
highwaySources1 = set()
origDistrictNodes = {}
nnn = {}
for n1 in nodes1:
    if n1._id.find('-', 1) < 0:
        continue
#   if n1._id.find("38208387")<0:
#       continue
    un1 = None
    for e in n1._outgoing:
        un1 = e._to
    for e in n1._incoming:
        un1 = e._from
    d = n1._id[:n1._id.find('-', 1)]
    if d[0] == '-':
        d = d[1:]
    if d not in origDistrictNodes:
        origDistrictNodes[d] = []
    if options.verbose:
        print("District: " + d)
    isHighwayNode = False
    isHighwaySink = False
    isHighwaySource = False
    noIncoming = 0
    noOutgoing = 0
    noInConns = 0
    noOutConns = 0
    for e in un1._outgoing:
        if e.getSpeed() > 80. / 3.6 and e.getSpeed() < 99:
            isHighwayNode = True
        if e.getSpeed() < 99:
            noOutgoing = noOutgoing + 1
        if e.getSpeed() > 99:
            noOutConns = noOutConns + 1
    for e in un1._incoming:
        if e.getSpeed() > 80. / 3.6 and e.getSpeed() < 99:
            isHighwayNode = True
        if e.getSpeed() < 99:
            noIncoming = noIncoming + 1
        if e.getSpeed() > 99:
            noInConns = noInConns + 1
    if options.verbose:
        print("Check", un1._id, noOutgoing, noIncoming)
    if isHighwayNode:
        if noOutgoing == 0:
            highwaySinks1.add(n1)
            isHighwaySink = True
        if noIncoming == 0:
            highwaySources1.add(n1)
            isHighwaySource = True
        # the next is a hack for bad visum-networks
        if noIncoming == 1 and noOutgoing == 1 and noInConns == 1 and noOutConns == 1:
            highwaySinks1.add(n1)
            isHighwaySink = True
            highwaySources1.add(n1)
            isHighwaySource = True

    best = None
    bestDist = -1
    check = urbanNodes2
    if n1 in highwaySinks1:
        check = highwaySinks2
    elif n1 in highwaySources1:
        check = highwaySources2
    elif isHighwayNode:
        check = highwayNodes2
    for n2 in check:
        dist = computeDistance(un1, n2)
        if bestDist == -1 or bestDist > dist:
            best = n2
            bestDist = dist
    if best:
        nnn[best] = n1
        if d not in nmap1to2:
            nmap1to2[d] = []
        if best not in nmap1to2[d]:
            nmap1to2[d].append(best)
        if best not in nmap2to1:
            nmap2to1[best] = []
        if n1 not in nmap2to1[best]:
            nmap2to1[best].append(n1)
        if options.verbose:
            print("a: " + d + "<->" + best._id)
        if best not in origDistrictNodes[d]:
            origDistrictNodes[d].append(best)

    preBest = best
    best = None
    bestDist = -1
    check = []
    if n1 in highwaySinks1 or preBest in highwaySinks2:
        check = highwaySources2
    elif n1 in highwaySources1 or preBest in highwaySources2:
        check = highwaySinks2
    elif isHighwayNode:
        check = highwayNodes2
    for n2 in check:
        dist = computeDistance(un1, n2)
        if (bestDist == -1 or bestDist > dist) and n2 != preBest:
            best = n2
            bestDist = dist
    if best:
        nnn[best] = n1
        if d not in nmap1to2:
            nmap1to2[d] = []
        if best not in nmap1to2[d]:
            nmap1to2[d].append(best)
        if best not in nmap2to1:
            nmap2to1[best] = []
        if n1 not in nmap2to1[best]:
            nmap2to1[best].append(n1)
        print("b: " + d + "<->" + best._id)
        if best not in origDistrictNodes[d]:
            origDistrictNodes[d].append(best)


if options.verbose:
    print("Found " + str(len(highwaySinks1)) + " highway sinks in net1")
    for n in highwaySinks1:
        print(n._id)
    print("Found " + str(len(highwaySources1)) + " highway sources in net1")
    for n in highwaySources1:
        print(n._id)


connectedNodesConnections = {}
for d in nmap1to2:
    for n2 in nmap1to2[d]:
        if n2 in connectedNodesConnections:
            continue
        n1i = net1.addNode("i" + n2._id, nnn[n2]._coord)
        n1o = net1.addNode("o" + n2._id, nnn[n2]._coord)
        haveIncoming = False
        incomingLaneNo = 0
        for e in n2._incoming:
            if e._id[0] != "i" and e._id[0] != "o":
                haveIncoming = True
                incomingLaneNo = incomingLaneNo + e.getLaneNumber()
        haveOutgoing = False
        outgoingLaneNo = 0
        for e in n2._outgoing:
            if e._id[0] != "i" and e._id[0] != "o":
                haveOutgoing = True
                outgoingLaneNo = outgoingLaneNo + e.getLaneNumber()
        if haveIncoming:
            e1 = net1.addEdge("o" + n2._id, n2._id, n1o._id, -2)
            if haveOutgoing:
                net1.addLane(e1, 20, 100.)
            else:
                for i in range(0, incomingLaneNo):
                    net1.addLane(e1, 20, 100.)
                    if len(n2._incoming) == 1:
                        fdd.write('    <connection from="' + n2._incoming[
                                  0]._id + '" to="' + e1._id + '" lane="' + str(i) + ':' + str(i) + '"/>\n')
        if haveOutgoing:
            if options.verbose:
                print("has outgoing")
            e2 = net1.addEdge("i" + n2._id, n1i._id, n2._id, -2)
            if haveIncoming:
                net1.addLane(e2, 20, 100.)
            else:
                for i in range(0, outgoingLaneNo):
                    net1.addLane(e2, 20, 100.)
                    if len(n2._outgoing) == 1:
                        fdd.write('    <connection from="' + e2._id + '" to="' +
                                  n2._outgoing[0]._id + '" lane="' + str(i) + ':' + str(i) + '"/>\n')
        connectedNodesConnections[n2] = [n1i, n1o]


newDistricts = {}
districtSources = {}
districtSinks = {}
mappedDistrictNodes = {}
connNodes = {}
dRemap = {}
for d in nmap1to2:
    newDistricts[d] = []
    if len(nmap1to2[d]) == 1:
        n = nmap1to2[d][0]
        if n in dRemap:
            districtSources[d] = districtSources[dRemap[n]]
            districtSinks[d] = districtSinks[dRemap[n]]
            newDistricts[d] = []
            newDistricts[d].append(n._id)
            continue
        else:
            dRemap[n] = d
        [ni, no] = connectedNodesConnections[n]
        if len(ni._outgoing) > 0:
            districtSources[d] = ni._outgoing[0]._id
        if len(no._incoming) > 0:
            districtSinks[d] = no._incoming[0]._id
        fdd.write('    <connection from="' + no._incoming[0]._id + '"/>\n')
    else:
        incomingLaneNoG = 0
        outgoingLaneNoG = 0
        for n in nmap1to2[d]:
            for e in n._incoming:
                if e._id[0] != "i" and e._id[0] != "o":
                    incomingLaneNoG = incomingLaneNoG + e.getLaneNumber()
            for e in n._outgoing:
                if e._id[0] != "i" and e._id[0] != "o":
                    outgoingLaneNoG = outgoingLaneNoG + e.getLaneNumber()
        p1 = [0, 0]
        p11 = [0, 0]
        p12 = [0, 0]
        p2 = [0, 0]
        for n in nmap1to2[d]:
            p1[0] = p1[0] + n._coord[0]
            p1[1] = p1[1] + n._coord[1]
            p2[0] = p2[0] + nnn[n]._coord[0]
            p2[1] = p2[1] + nnn[n]._coord[1]
        p2[0] = (p1[0] + p2[0]) / float(len(origDistrictNodes[d]) * 2)
        p2[1] = (p1[1] + p2[1]) / float(len(origDistrictNodes[d]) * 2)
        dn2i = net1.addNode("cci" + d, p2)
        dn2o = net1.addNode("cci" + d, p2)
        p11[0] = p1[0] / float(len(origDistrictNodes[d]))
        p11[1] = p1[1] / float(len(origDistrictNodes[d]))
        dn1o = net1.addNode("co" + d, p11)
        e1 = net1.addEdge("co" + d, dn1o._id, dn2o._id, -2)
        for i in range(0, incomingLaneNoG):
            net1.addLane(e1, 22, 100.)
        districtSinks[d] = e1._id
        p12[0] = p1[0] / float(len(origDistrictNodes[d]))
        p12[1] = p1[1] / float(len(origDistrictNodes[d]))
        dn1i = net1.addNode("ci" + d, p12)
        e2 = net1.addEdge("ci" + d, dn2i._id, dn1i._id, -2)
        for i in range(0, outgoingLaneNoG):
            net1.addLane(e2, 21, 100.)
        districtSources[d] = e2._id
        runningOutLaneNumber = 0
        runningInLaneNumber = 0
        for n2 in nmap1to2[d]:
            [ni, no] = connectedNodesConnections[n2]
            print("In: " + ni._id + " " + str(len(ni._incoming)) +
                  " " + str(len(ni._outgoing)))
            print("Out: " + no._id + " " + str(len(no._incoming)) +
                  " " + str(len(no._outgoing)))
            if len(no._incoming) > 0:
                incomingLaneNo = 0
                for e in n2._incoming:
                    if e._id[0] != "i" and e._id[0] != "o":
                        incomingLaneNo = incomingLaneNo + e.getLaneNumber()
                e1 = net1.addEdge("o" + d + "#" + n2._id, no._id, dn1o._id, -2)
                for i in range(0, incomingLaneNo):
                    net1.addLane(e1, 19, 100.)
                    fdd.write('    <connection from="' + "o" + d + "#" + n2._id + '" to="' + dn1o._outgoing[
                              0]._id + '" lane="' + str(i) + ':' + str(runningOutLaneNumber) + '"/>\n')
                    runningOutLaneNumber = runningOutLaneNumber + 1
                fdd.write(
                    '    <connection from="' + dn1o._outgoing[0]._id + '"/>\n')
                if incomingLaneNo == 0:
                    net1.addLane(e1, 19, 100.)
                    runningOutLaneNumber = runningOutLaneNumber + 1
            if len(ni._outgoing) > 0:
                outgoingLaneNo = 0
                for e in n2._outgoing:
                    if e._id[0] != "i" and e._id[0] != "o":
                        outgoingLaneNo = outgoingLaneNo + e.getLaneNumber()
                e2 = net1.addEdge("i" + d + "#" + n2._id, dn1i._id, ni._id, -2)
                for i in range(0, outgoingLaneNo):
                    net1.addLane(e2, 18, 100.)
                    fdd.write('    <connection from="' + dn1i._incoming[
                              0]._id + '" to="' + "i" + d + "#" + n2._id + '" lane="' + str(runningInLaneNumber) +
                              ':' + str(i) + '"/>\n')
                    runningInLaneNumber = runningInLaneNumber + 1
                if outgoingLaneNo == 0:
                    net1.addLane(e2, 18, 100.)
                    runningInLaneNumber = runningInLaneNumber + 1

fd = open("districts.xml", "w")
fd.write("<tazs>\n")
for d in newDistricts:
    fd.write('    <taz id="' + d + '">\n')
    if d in districtSources:
        fd.write(
            '        <tazSource id="' + districtSources[d] + '" weight="1"/>\n')
    if d in districtSinks:
        fd.write(
            '        <tazSink id="' + districtSinks[d] + '" weight="1"/>\n')
    fd.write('    </taz>\n')
fd.write("</tazs>\n")
fd.close()


def writeNode(fd, node):
    fd.write("   <node id=\"" + node._id + "\" x=\"" +
             str(node._coord[0]) + "\" y=\"" + str(node._coord[1]) + "\"/>\n")


def writeEdge(fd, edge, withGeom=True):
    fd.write("   <edge id=\"" + edge._id + "\" fromNode=\"" +
             edge._from._id + "\" toNode=\"" + edge._to._id)
    fd.write("\" speed=\"" + str(edge._speed))
    fd.write("\" priority=\"" + str(edge._priority))
    if withGeom:
        fd.write("\" spreadType=\"center")
    fd.write("\" numLanes=\"" + str(len(edge._lanes)) + "\"")
    shape = edge.getShape()
    if withGeom:
        fd.write(" shape=\"")
        for i, c in enumerate(shape):
            if i != 0:
                fd.write(" ")
            fd.write(str(c[0]) + "," + str(c[1]))
        fd.write("\"")
    fd.write("/>\n")


def writeNodes(net):
    fd = open("nodes.xml", "w")
    fd.write("<nodes>\n")
    for node in net._nodes:
        writeNode(fd, node)
    fd.write("</nodes>\n")
    fd.close()


def writeEdges(net):
    fd = open("edges.xml", "w")
    fd.write("<edges>\n")
    for edge in net._edges:
        if edge._id.find("#") > 0 or edge._id.find("c") >= 0 or edge._id.find("i") >= 0:
            writeEdge(fd, edge, False)
        else:
            writeEdge(fd, edge)
    fd.write("</edges>\n")
    fd.close()


fdd.write("</connections>\n")
writeNodes(net1)
writeEdges(net1)
