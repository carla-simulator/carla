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

# @file    network.py
# @author  Daniel Krajzewicz
# @date    2013-10-10

from __future__ import absolute_import
from __future__ import print_function
import sumolib
import os
import subprocess
import tempfile


class Node:

    def __init__(self, nid, x, y, nodeType):
        self.nid = nid
        self.x = x
        self.y = y
        self.nodeType = nodeType
        self.crossings = []

    def getNetworkCoordinates(self):
        t = self.nid.split("/")
        return [int(t[0]), int(t[1])]

    def addCrossing(self, fromEdge, toEdge):
        self.crossings.append([fromEdge, toEdge])


class Lane:

    def __init__(self, dirs=None, allowed=None, disallowed=None):
        self.dirs = dirs
        self.allowed = allowed
        self.disallowed = disallowed
        if self.dirs is None:
            self.dirs = []


class Split:

    def __init__(self, distance, lanes):
        self.distance = distance
        self.lanes = lanes


class Edge:

    def __init__(self, eid=None, fromNode=None, toNode=None, numLanes=None, maxSpeed=None, lanes=None, splits=None):
        self.eid = eid
        self.fromNode = fromNode
        self.toNode = toNode
        self.numLanes = numLanes
        self.maxSpeed = maxSpeed
        self.lanes = lanes
        if self.lanes is None:
            self.lanes = [Lane() for _ in range(numLanes)]
        self.splits = splits
        if self.splits is None:
            self.splits = []
        if numLanes is None:
            numLanes = len(self.lanes)

    def addSplit(self, distance, lanesToRight=None, lanesToLeft=None):
        if len(self.splits) == 0:
            if lanesToRight is None:
                lanesToRight = 0
            if lanesToLeft is None:
                lanesToLeft = 0
            lanes = range(lanesToRight, self.numLanes + lanesToRight)
            self.splits.append(Split(0, lanes))
            lanes = range(0, self.numLanes + lanesToRight + lanesToLeft)
            self.splits.append(Split(distance, lanes))
            self.lanes = [Lane() for _ in range(lanesToRight)] + self.lanes
            self.lanes += [Lane() for _ in range(lanesToLeft)]

    def getConnections(self, net):
        ret = []

        seen = {}
        for i, l in enumerate(self.lanes):
            for d in l.dirs:
                if d not in seen:
                    seen[d] = 0
                c = net.dir2connection(d, self, i, seen[d])
                if c is not None:
                    ret.append(c)
                seen[d] = seen[d] + 1
        return ret

    def getDirection(self):
        n1c = self.fromNode.getNetworkCoordinates()
        n2c = self.toNode.getNetworkCoordinates()
        return [n2c[0] - n1c[0], n2c[1] - n1c[1]]


class Connection:

    def __init__(self, fromEdge, fromLane, toEdge, toLane):
        self.fromEdge = fromEdge
        self.fromLane = fromLane
        self.toEdge = toEdge
        self.toLane = toLane


class E1:

    def __init__(self, id, laneID, pos, freq, outputFile):
        self.id = id
        self.laneID = laneID
        self.pos = pos
        self.freq = freq
        self.outputFile = outputFile


class Net:

    def __init__(self, defaultNode, defaultEdge):
        self._nodes = {}
        self._edges = {}
        self._defaultEdge = defaultEdge
        if self._defaultEdge is None:
            self._defaultEdge = Edge(None, None, None, 2, 13.89)
        self._defaultNode = defaultNode
        if self._defaultNode is None:
            self._defaultNode = Node(None, None, None, "traffic_light")
        self._e1 = {}
        self.netName = None

    def addNode(self, n):
        self._nodes[n.nid] = n

    def getNode(self, id):
        if id in self._nodes:
            return self._nodes[id]
        return None

    def addEdge(self, e):
        self._edges[e.eid] = e

    def addE1Detectors(self, id, laneID, pos, freq, outputFile):
        e1 = E1(id, laneID, pos, freq, outputFile)
        self._e1[e1.id] = e1
        return e1

    def getEdge(self, id):
        if id in self._edges:
            return self._edges[id]
        return None

    def getDefaultEdge(self, n1, n2):
        return self._defaultEdge

    def buildEdge(self, n1, n2):
        defEdge = self.getDefaultEdge(n1, n2)
        splits = []
        for s in defEdge.splits:
            splits.append(s)
        lanes = []
        for l in defEdge.lanes:
            lanes.append(l)
        numLanes = defEdge.numLanes
        maxSpeed = defEdge.maxSpeed
        e = Edge(n1.nid + "_to_" + n2.nid, n1, n2, numLanes=numLanes,
                 maxSpeed=maxSpeed, lanes=lanes, splits=splits)
        return e

    def connectNodes(self, node1, node2, bidi, centralReservation):
        n1 = self._nodes[node1]
        n2 = self._nodes[node2]
        self.addEdge(self.buildEdge(n1, n2))
        if bidi:
            self.addEdge(self.buildEdge(n2, n1))

    def getDirectionFromNode(self, n, dir):
        nc = n.getNetworkCoordinates()
        eid = n.nid + "_to_" + str(nc[0] + dir[0]) + "/" + str(nc[1] + dir[1])
        if eid in self._edges:
            return self._edges[eid]
        return None

    def getMatchingOutgoing(self, edge, direction):
        edir = edge.getDirection()
        if direction == "s":
            return self.getDirectionFromNode(edge.toNode, edir)
        elif direction == "t":
            return self.getDirectionFromNode(edge.toNode, [-1 * edir[0], -1 * edir[1]])
        elif direction == "r":
            # look, the following is because SUMO's coordinates don't match:
            #  the y-direction starts at the bottom, while x on right
            if edir[0] == 0:
                return self.getDirectionFromNode(edge.toNode, [1 * edir[1], 1 * edir[0]])
            else:
                return self.getDirectionFromNode(edge.toNode, [-1 * edir[1], -1 * edir[0]])
        elif direction == "l":
            # the same as above
            if edir[0] != 0:
                return self.getDirectionFromNode(edge.toNode, [1 * edir[1], 1 * edir[0]])
            else:
                return self.getDirectionFromNode(edge.toNode, [-1 * edir[1], -1 * edir[0]])
        else:
            raise RuntimeError("Unrecognized direction '%s'" % direction)

    def dir2connection(self, direction, edge, lane, seen):
        toEdge = self.getMatchingOutgoing(edge, direction)
        if toEdge is not None:
            if toEdge.lanes[seen].allowed != edge.lanes[lane].allowed:
                seen = seen + 1
            return Connection(edge, lane, toEdge, seen)
        return None

    def build(self, netName="net.net.xml"):
        connections = []
        nodesFile = tempfile.NamedTemporaryFile(mode="w", delete=False)
        print("<nodes>", file=nodesFile)
        for nid in self._nodes:
            n = self._nodes[nid]
            print('    <node id="%s" x="%s" y="%s" type="%s"/>' % (
                n.nid, n.x, n.y, n.nodeType), file=nodesFile)
        print("</nodes>", file=nodesFile)
        nodesFile.close()

        edgesFile = tempfile.NamedTemporaryFile(mode="w", delete=False)
        print("<edges>", file=edgesFile)
        for eid in self._edges:
            e = self._edges[eid]
            print('    <edge id="%s" from="%s" to="%s" numLanes="%s" speed="%s">' % (
                e.eid, e.fromNode.nid, e.toNode.nid, e.numLanes, e.maxSpeed), file=edgesFile)
            for s in e.splits:
                print('        <split pos="%s" lanes="%s"/>' %
                      (-s.distance, " ".join(map(str, s.lanes))), file=edgesFile)

            """
        for i,l in enumerate(e.lanes):
            if l.allowed==None and l.disallowed==None:
                continue
            ls =  '        <lane index="%s" ' % (i)
            if l.allowed!=None:
                ls = ls + 'allow="%s"' % l.allowed
            if l.disallowed!=None:
                ls = ls + 'disallow="%s"' % l.disallowed
            print >> edgesFile, ls+'/>'
        """

            connections.extend(e.getConnections(self))
            print('    </edge>', file=edgesFile)

            hadConstraints = False
            for i, l in enumerate(e.lanes):
                if l.allowed is None and l.disallowed is None:
                    continue
                hadConstraints = True
            if hadConstraints:
                for s in e.splits:
                    eid = e.eid
                    if s.distance != 0:
                        eid = eid + ".%s" % -s.distance
                    print('    <edge id="%s">' % (eid), file=edgesFile)
                    for i, l in enumerate(e.lanes):
                        # if i not in s.lanes:
                        #    continue
                        if l.allowed is None and l.disallowed is None:
                            continue
                        ls = '        <lane index="%s" ' % (i)
                        if l.allowed is not None:
                            ls = ls + 'allow="%s"' % l.allowed
                        if l.disallowed is not None:
                            ls = ls + 'disallow="%s"' % l.disallowed
                        print(ls + '/>', file=edgesFile)
                    print('    </edge>', file=edgesFile)

        print("</edges>", file=edgesFile)
        edgesFile.close()

        connectionsFile = tempfile.NamedTemporaryFile(mode="w", delete=False)
        print("<connections>", file=connectionsFile)
        for c in connections:
            eid = c.fromEdge.eid
            if len(c.fromEdge.splits) > 1:
                eid = eid + ".-" + str(c.fromEdge.splits[-1].distance)
            print('    <connection from="%s" to="%s" fromLane="%s" toLane="%s"/>' % (
                eid, c.toEdge.eid, c.fromLane, c.toLane), file=connectionsFile)
        for n in self._nodes:
            if len(self._nodes[n].crossings) == 0:
                continue
            for c in self._nodes[n].crossings:
                print('    <crossing node="%s" edges="%s"/>' % (
                    n, " ".join(c)), file=connectionsFile)
        print("</connections>", file=connectionsFile)
        connectionsFile.close()

        netconvert = sumolib.checkBinary("netconvert")

        subprocess.call([netconvert, "-v", "-n", nodesFile.name, "-e", edgesFile.name, "-x", connectionsFile.name,
                         "-o", netName])
        os.remove(nodesFile.name)
        os.remove(edgesFile.name)
        os.remove(connectionsFile.name)
        self.netName = netName
        return netName

    def buildDetectors(self, filename):
        e1File = filename
        fdo = open(e1File, "w")
        print("<additional>", file=fdo)
        for e1id in self._e1:
            e1 = self._e1[e1id]
            print('    <e1Detector id="%s" lane="%s" pos="%s" freq="%s" file="%s"/>' % (
                e1.id, e1.laneID, e1.pos, e1.freq, e1.outputFile), file=fdo)
        print("</additional>", file=fdo)
        fdo.close()
