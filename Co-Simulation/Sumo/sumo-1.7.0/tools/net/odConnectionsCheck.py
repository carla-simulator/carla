#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    odConnectionsCheck.py
# @author  Daniel Krajzewicz
# @author  Yun-Pang Floetteroed
# @author  Michael Behrisch
# @date    2007-03-20

from __future__ import absolute_import
from __future__ import print_function

import os
import sys

from xml.sax import make_parser, handler
from optparse import OptionParser

pyPath = os.path.dirname(sys.argv[0])
sys.path.append(
    os.path.join("..", "..", "..", "..", "extern", "sumo", "tools", "assign"))
from dijkstra import dijkstraPlain  # noqa

# This class is used to build the nodes in the investigated network and
# includes the update-function for searching the k shortest paths.


class Vertex:

    """
    This class is to store node attributes and the respective incoming/outgoing links.
    """

    def __init__(self, num):
        self.inEdges = []
        self.outEdges = []
        self.label = "%s" % num
        self.sourceEdges = []
        self.sinkEdges = []

    def __repr__(self):
        return self.label

# This class is uesed to store link information and estimate
# as well as flow and capacity for the flow computation and some parameters
# read from the net.


class Edge:

    """
    This class is to record link attributes
    """

    def __init__(self, label, source, target, kind="junction"):
        self.label = label
        self.source = source
        self.target = target
        self.capacity = sys.maxsize
        self.kind = kind
        self.maxspeed = 1.0
        self.length = 0.0
        self.freeflowtime = 0.0
        self.numberlane = 0
        self.helpacttime = self.freeflowtime

    def init(self, speed, length, laneNumber):
        self.maxspeed = float(speed)
        self.length = float(length)
        self.numberlane = float(laneNumber)
        if self.source.label == self.target.label:
            self.freeflowtime = 0.0
        else:
            self.freeflowtime = self.length / self.maxspeed
            self.helpacttime = self.freeflowtime

    def __repr__(self):
        str(self.capacity)
        if self.capacity == sys.maxsize or self.connection != 0:
            pass
        return "%s_<%s|%s|%s>" % (self.label, self.kind, self.source, self.target)


class Net:

    def __init__(self):
        self._vertices = []
        self._edges = {}
        self._endVertices = []
        self._startVertices = []
        self.separateZones = []
        self.sinkEdges = []
        self.sourceEdges = []

    def newVertex(self):
        v = Vertex(len(self._vertices))
        self._vertices.append(v)
        return v

    def getEdge(self, edgeLabel):
        return self._edges[edgeLabel]

    def addEdge(self, edgeObj):
        edgeObj.source.outEdges.append(edgeObj)
        edgeObj.target.inEdges.append(edgeObj)
        if edgeObj.kind == "real":
            self._edges[edgeObj.label] = edgeObj

    def addIsolatedRealEdge(self, edgeLabel):
        self.addEdge(Edge(edgeLabel, self.newVertex(), self.newVertex(),
                          "real"))

    def getTargets(self, separateZones):
        target = set()
        for end in self._endVertices:
            if end not in separateZones:
                target.add(end)
        return target

    def checkRoute(self, startVertex, endVertex, totalCounts, subCounts, P, odPairSet, matrixEntry, skipList):
        totalCounts += 1
        vertex = endVertex
        if startVertex.label not in skipList and endVertex.label not in skipList:
            if vertex not in P:
                subCounts += 1
                odPairSet.append(
                    (startVertex.label, endVertex.label, matrixEntry))
                if options.verbose:
                    print("no connection to", endVertex.label)

        return totalCounts, subCounts, odPairSet

# The class is for parsing the XML input file (network file). The data
# parsed is written into the net.


class NetworkReader(handler.ContentHandler):

    def __init__(self, net):
        self._net = net
        self._edge = ''
        self._maxSpeed = 0
        self._laneNumber = 0
        self._length = 0
        self._edgeObj = None
        self._chars = ''
        self._counter = 0
        self._turnlink = None

    def startElement(self, name, attrs):
        self._chars = ''
        if name == 'edge' and ('function' not in attrs or attrs['function'] != 'internal'):
            self._edge = attrs['id']
            self._net.addIsolatedRealEdge(self._edge)
            self._edgeObj = self._net.getEdge(self._edge)
            self._edgeObj.source.label = attrs['from']
            self._edgeObj.target.label = attrs['to']
            self._maxSpeed = 0
            self._laneNumber = 0
            self._length = 0
        elif name == 'succ':
            self._edge = attrs['edge']
            if self._edge[0] != ':':
                self._edgeObj = self._net.getEdge(self._edge)
        elif name == 'succlane' and self._edge != "":
            lane = attrs['lane']
            if lane != "SUMO_NO_DESTINATION":
                toEdge = self._net.getEdge(lane[:lane.rfind('_')])
                newEdge = Edge(
                    self._edge + "_" + lane[:lane.rfind('_')], self._edgeObj.target, toEdge.source)
                self._net.addEdge(newEdge)
                self._edgeObj.finalizer = lane[:lane.rfind('_')]
        elif name == 'lane' and self._edge != '':
            self._maxSpeed = max(self._maxSpeed, float(attrs['speed']))
            self._laneNumber = self._laneNumber + 1
            self._length = float(attrs['length'])

    def characters(self, content):
        self._chars += content

    def endElement(self, name):
        if name == 'edge' and self._edge != '':
            self._edgeObj.init(self._maxSpeed, self._length, self._laneNumber)
            self._edge = ''

# The class is for parsing the XML input file (districts). The data parsed
# is written into the net.


class DistrictsReader(handler.ContentHandler):

    def __init__(self, net):
        self._net = net
        self._StartDTIn = None
        self._StartDTOut = None
        self.index = 100

    def startElement(self, name, attrs):
        if name == 'taz':
            self._StartDTIn = self._net.newVertex()
            self._StartDTIn.label = attrs['id']
            self._StartDTOut = self._net.newVertex()
            self._StartDTOut.label = self._StartDTIn.label
            self._net._startVertices.append(self._StartDTIn)
            self._net._endVertices.append(self._StartDTOut)
        elif name == 'tazSink':
            sinklink = self._net.getEdge(attrs['id'])
            self.index += 1
            conlink = self._StartDTOut.label + str(self.index)
            newEdge = Edge(conlink, sinklink.target, self._StartDTOut, "real")
            self._net.addEdge(newEdge)
            newEdge.weight = attrs['weight']
            newEdge.connection = 1
        elif name == 'tazSource':
            sourcelink = self._net.getEdge(attrs['id'])
            self.index += 1
            conlink = self._StartDTIn.label + str(self.index)
            newEdge = Edge(conlink, self._StartDTIn, sourcelink.source, "real")
            self._net.addEdge(newEdge)
            newEdge.weight = attrs['weight']
            newEdge.connection = 2


def getMatrix(net, verbose, matrix, MatrixSum):  # , mtxplfile, mtxtfile):
    """
    This method is to read matrix from the given file.
    """
    matrixPshort = []
    startVertices = []
    endVertices = []
    Pshort_EffCells = 0
    periodList = []

    ODpairs = 0
    origins = 0
    CurrentMatrixSum = 0.0
    skipCount = 0
    zones = 0
    smallDemandNum = 0
    checkCounts = 0
    for line in open(matrix):
        checkCounts += 1
        if line[0] == '$':
            visumCode = line[1:3]
            if visumCode != 'VM':
                skipCount += 1
        elif line[0] != '*' and line[0] != '$':
            skipCount += 1
            if skipCount == 2:
                for elem in line.split():
                    periodList.append(float(elem))
            elif skipCount > 3:
                if zones == 0:
                    for elem in line.split():
                        zones = int(elem)
                elif len(startVertices) < zones:
                    for elem in line.split():
                        if len(elem) > 0:
                            for startVertex in net._startVertices:
                                if startVertex.label == elem:
                                    startVertices.append(startVertex)
                            for endVertex in net._endVertices:
                                if endVertex.label == elem:
                                    endVertices.append(endVertex)
                    origins = len(startVertices)
                    len(endVertices)
                elif len(startVertices) == zones:
                    if ODpairs % origins == 0:
                        matrixPshort.append([])
                    for item in line.split():
                        matrixPshort[-1].append(float(item))
                        ODpairs += 1
                        MatrixSum += float(item)
                        CurrentMatrixSum += float(item)
                        if float(item) > 0.:
                            Pshort_EffCells += 1
                        if float(item) < 1. and float(item) > 0.:
                            smallDemandNum += 1
    begintime = int(periodList[0])
    int(periodList[1]) - begintime
    float(smallDemandNum) / float(Pshort_EffCells)

    return matrixPshort, startVertices, endVertices


def main():
    parser = make_parser()
    os.getcwd()
    dataDir = options.datadir
    districts = os.path.join(dataDir, options.districtfile)
    netfile = os.path.join(dataDir, options.netfile)
    sumDemand = 0.
    MatrixSum = 0.
    totalCounts = 0
    subCounts = 0
    separateZones = []
    odPairSet = []
    skipList = []
    net = Net()
    if options.spearatezones:
        for item in options.spearatezones.split(','):
            for district in net._startVertices:
                if district.label == item:
                    separateZones.append(district)

    parser.setContentHandler(NetworkReader(net))
    parser.parse(netfile)

    parser.setContentHandler(DistrictsReader(net))
    parser.parse(districts)
    if options.skipList:
        for elem in options.skipList.split(','):
            skipList.append(elem)
    if options.mtxfile:
        matrixPshort, startVertices, endVertices = getMatrix(
            net, options.verbose, os.path.join(dataDir, options.mtxfile), MatrixSum)
    else:
        matrixPshort = None
        startVertices = net._startVertices
        endVertices = net._endVertices

    if options.verbose:
        print(len(net._edges), "edges read")
        print(len(startVertices), "start vertices read")
        print(len(endVertices), "target vertices read")

    for start, startVertex in enumerate(startVertices):
        if startVertex not in separateZones:
            targets = net.getTargets(separateZones)
            if options.verbose:
                print("checking start vertex", startVertex.label)
            D, P = dijkstraPlain(startVertex, targets)

            for end, endVertex in enumerate(endVertices):
                if startVertex.label != endVertex.label and endVertex not in separateZones:
                    if matrixPshort:
                        entry = matrixPshort[start][end]
                    else:
                        entry = 1
                    if entry > 0.:
                        totalCounts, subCounts, odPairSet = net.checkRoute(
                            startVertex, endVertex, totalCounts, subCounts, P, odPairSet, entry, skipList)
        else:
            for endVertex in separateZones:
                if startVertex.label != endVertex.label:
                    totalCounts, subCounts, odPairSet = net.checkRoute(
                        startVertex, endVertex, totalCounts, subCounts, P, odPairSet, matrixPshort[start][end],
                        skipList)

    print('total OD connections:', totalCounts)
    if len(odPairSet) > 0:
        foutzones = open('absentConnections.txt', 'w')
        for pair in odPairSet:
            sumDemand += float(pair[2])
            foutzones.write('from: %s   to: %s; demand:%s\n' %
                            (pair[0], pair[1], pair[2]))
        foutzones.close()
        print(subCounts, 'connections are absent!')
        print(sumDemand, 'vehicles are absent.')
    else:
        print('all connections exist! ')


optParser = OptionParser()
optParser.add_option("-r", "--data-dir", dest="datadir",
                     default=os.getcwd(), help="give the data directory path")
optParser.add_option("-n", "--net-file", dest="netfile",
                     help="define the net file")
optParser.add_option("-m", "--matrix-file", dest="mtxfile",
                     help="define the matrix file")
optParser.add_option("-d", "--districts-file", dest="districtfile",
                     help="define the district file")
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("-i", "--separate-zones", dest="spearatezones", type='string',
                     help="define the zones which should be separated")  # e.g. dist_00101,dist_00102
optParser.add_option("-s", "--skip-list", dest="skipList", type='string',
                     help="define the zones which will not be compared with each other")  # e.g. dist_00101,dist_00102
optParser.add_option("-b", "--debug", action="store_true", dest="debug",
                     default=False, help="debug the program")

(options, args) = optParser.parse_args()

if not options.netfile or not options.districtfile:
    optParser.print_help()
    sys.exit()

main()
