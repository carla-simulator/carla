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

# @file    routecompare.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2008-03-25

"""
This script compares two route sets by calculating
a similarity for any two routes based on the number of common edges
and determining a maximum weighted matching between the route sets.
It needs at least two parameters, which are the route sets to compare.
Optionally a district file may be given, then only routes with
the same origin and destination district are matched.
"""
from __future__ import absolute_import
from __future__ import print_function
import sys
import optparse
import array
from xml.sax import make_parser, handler

SCALE = 10000
INFINITY = 2**30


class RouteReader(handler.ContentHandler):

    def __init__(self, routeMap, edges):
        self._routes = routeMap
        self._edges = edges
        self._vID = ''
        self._routeID = ''
        self._routeString = ''

    def startElement(self, name, attrs):
        if name == 'vehicle':
            self._vID = attrs['id']
        elif name == 'route':
            if 'id' in attrs:
                self._routeID = attrs['id']
            else:
                self._routeID = self._vID
                self._vID = ''
            self._routeString = ''
            if 'edges' in attrs:
                self._routeString = attrs['edges']

    def endElement(self, name):
        if name == 'route':
            route = array.array('L')
            for edge in self._routeString.split():
                if edge not in self._edges:
                    self._edges[edge] = len(self._edges)
                route.append(self._edges[edge])
            self._routes[self._routeID] = route

    def characters(self, content):
        if self._routeID != '':
            self._routeString += content


class DistrictReader(handler.ContentHandler):

    def __init__(self, sourceEdges, sinkEdges, edges):
        self._sources = sourceEdges
        self._sinks = sinkEdges
        self._edges = edges
        self._districtID = ''

    def startElement(self, name, attrs):
        if name == 'taz':
            self._districtID = attrs['id']
        elif name == 'tazSource':
            if attrs['id'] in self._edges:
                self._sources[self._edges[attrs['id']]] = self._districtID
            else:
                if options.verbose:
                    print("Warning! No routes touching source edge %s of %s." % (
                        attrs['id'], self._districtID))
        elif name == 'tazSink':
            if attrs['id'] in self._edges:
                self._sinks[self._edges[attrs['id']]] = self._districtID
            else:
                if options.verbose:
                    print("Warning! No routes touching sink edge %s of %s." %
                          (attrs['id'], self._districtID))


def compare(first, second):
    commonEdges = 0
    for edge in first:
        if edge in second:
            commonEdges += SCALE
    return commonEdges // max(len(first), len(second))


def matching(routeIDs1, routeIDs2, similarityMatrix, match):
    matchVal = 0
    for id1 in routeIDs1:
        maxMatch = -1
        matchId = ""
        for id2 in routeIDs2:
            if id2 not in match and similarityMatrix[id1][id2] > maxMatch:
                maxMatch = similarityMatrix[id1][id2]
                matchId = id2
        if matchId:
            match[matchId] = id1
            matchVal += maxMatch
    return matchVal


def identityCount(routeIDs1, routeIDs2, similarityMatrix):
    matched = set()
    for id1 in routeIDs1:
        for id2 in routeIDs2:
            if id2 not in matched and similarityMatrix[id1][id2] == SCALE:
                matched.add(id2)
                break
    return len(matched)


class Node:

    def __init__(self, routeID, weight):
        self.routeID = routeID
        self.weight = weight
        self.eps = INFINITY
        self.level = INFINITY
        self.match = None


def augmentSimultan(vTerm):
    dead = set()
    for v in vTerm:
        path = [v]
        id = 0
        while True:
            while len(path[id].pre) > 0 and path[id].pre[0] in dead:
                path[id].pre.pop(0)
            if len(path[id].pre) == 0:
                if id == 0:
                    break
                dead.add(path[id - 1])
                id -= 2
            else:
                if id == len(path) - 1:
                    path.append(None)
                path[id + 1] = path[id].pre.pop(0)
                dead.add(path[id + 1])
                id += 1
                if path[id].level == 0:
                    for j in range(0, id + 1, 2):
                        path[j].match = path[j + 1]
                        path[j + 1].match = path[j]
                    break
                else:
                    if id == len(path) - 1:
                        path.append(None)
                    path[id + 1] = path[id].match
                    id += 1


def hungarianDAG(U, V, similarityMatrix):
    while True:
        S = set()
        T = set()
        Q = []
        vTerm = set()
        for u in U:
            u.level = INFINITY
            u.eps = INFINITY
            if not u.match:
                S.add(u)
                u.level = 0
                Q.append(u)
        for v in V:
            v.level = INFINITY
            v.eps = INFINITY
        while len(Q) > 0:
            s = Q.pop(0)
            for t in V:
                if s.weight + t.weight == similarityMatrix[s.routeID][t.routeID]:
                    if t.level > s.level:
                        if t.level == INFINITY:
                            T.add(t)
                            t.level = s.level + 1
                            t.pre = [s]
                            if not t.match:
                                vTerm.add(t)
                            else:
                                S.add(t.match)
                                t.match.level = t.level + 1
                                Q.append(t.match)
                        else:
                            t.pre.append(s)
                else:
                    t.eps = min(
                        t.eps, s.weight + t.weight - similarityMatrix[s.routeID][t.routeID])
        if len(vTerm) > 0:
            break
        epsilon = INFINITY
        for t in V:
            if t.eps < epsilon:
                epsilon = t.eps
        if epsilon == INFINITY:
            break
        for x in S:
            x.weight -= epsilon
        for x in T:
            x.weight += epsilon
    return vTerm


def maxMatching(routeIDs1, routeIDs2, similarityMatrix, match):
    maxSimilarity = 0
    for id1 in routeIDs1:
        for value in similarityMatrix[id1].values():
            if value > maxSimilarity:
                maxSimilarity = value
    U = []
    for id1 in routeIDs1:
        U.append(Node(id1, maxSimilarity))
    V = []
    for id2 in routeIDs2:
        V.append(Node(id2, 0))
    while True:
        vTerm = hungarianDAG(U, V, similarityMatrix)
        if len(vTerm) == 0:
            break
        augmentSimultan(vTerm)
    matchVal = 0
    for v in V:
        if v.match:
            match[v.routeID] = v.match.routeID
            matchVal += similarityMatrix[v.match.routeID][v.routeID]
    return matchVal


optParser = optparse.OptionParser(
    usage="usage: %prog [options] <routes1> <routes2>")
optParser.add_option("-d", "--districts-file", dest="districts",
                     default="", help="read districts from FILE", metavar="FILE")
optParser.add_option("-s", "--simple-match", action="store_true", dest="simple",
                     default=False, help="use simple matching algorithm")
optParser.add_option("-p", "--print-matching", action="store_true", dest="printmatch",
                     default=False, help="print the resulting matching")
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="print more info")
(options, args) = optParser.parse_args()


if len(args) < 2:
    optParser.print_help()
    sys.exit()
edges = {}
routes1 = {}
routes2 = {}
parser = make_parser()
if options.verbose:
    print("Reading first routes file %s" % args[0])
parser.setContentHandler(RouteReader(routes1, edges))
parser.parse(args[0])
if options.verbose:
    print("Reading second routes file %s" % args[1])
parser.setContentHandler(RouteReader(routes2, edges))
parser.parse(args[1])

routeMatrix1 = {}
routeMatrix2 = {}
if options.districts:
    sources = {}
    sinks = {}
    if options.verbose:
        print("Reading districts %s" % options.districts)
    parser.setContentHandler(DistrictReader(sources, sinks, edges))
    parser.parse(options.districts)
    for routes, routeMatrix in [(routes1, routeMatrix1), (routes2, routeMatrix2)]:
        for routeID in sorted(routes):
            route = routes[routeID]
            source = sources[route[0]]
            sink = sinks[route[-1]]
            if source not in routeMatrix:
                routeMatrix[source] = {}
            if sink not in routeMatrix[source]:
                routeMatrix[source][sink] = []
            routeMatrix[source][sink].append(routeID)
else:
    for routes, routeMatrix in [(routes1, routeMatrix1), (routes2, routeMatrix2)]:
        routeMatrix["dummySource"] = {}
        routeMatrix["dummySource"]["dummySink"] = list(sorted(routes.keys()))

match = {}
totalMatch = 0
totalIdentical = 0
for source in sorted(routeMatrix1):
    if source not in routeMatrix2:
        if options.verbose:
            print(
                "Warning! No routes starting at %s in second route set" % source)
        continue
    for sink in sorted(routeMatrix1[source]):
        routeIDs1 = routeMatrix1[source][sink]
        if sink not in routeMatrix2[source]:
            if options.verbose:
                print("Warning! No routes starting at %s and ending at %s in second route set" % (
                    source, sink))
            continue
        routeIDs2 = routeMatrix2[source][sink]
        if options.verbose and len(routeIDs1) != len(routeIDs2):
            print("Warning! Different route set sizes for start '%s' and end '%s'." % (
                source, sink))
        similarityMatrix = {}
        for idx, id1 in enumerate(routeIDs1):
            for oldID in routeIDs1[:idx]:
                if routes1[oldID] == routes1[id1]:
                    similarityMatrix[id1] = similarityMatrix[oldID]
                    break
            if id1 not in similarityMatrix:
                similarityMatrix[id1] = {}
                for id2 in routeIDs2:
                    similarityMatrix[id1][id2] = compare(
                        routes1[id1], routes2[id2])
        if options.simple:
            matchVal = matching(routeIDs1, routeIDs2, similarityMatrix, match)
        else:
            matchVal = maxMatching(
                routeIDs1, routeIDs2, similarityMatrix, match)
        totalMatch += matchVal
        identityVal = identityCount(routeIDs1, routeIDs2, similarityMatrix)
        totalIdentical += identityVal
        if options.verbose:
            print(source, sink, float(matchVal) / len(routeIDs1) / SCALE,
                  float(identityVal) / len(routeIDs1))
if options.printmatch:
    for r2, r1 in sorted(match.items()):
        print(r1, r2)
print(float(totalMatch) / len(routes1) / SCALE,
      float(totalIdentical) / len(routes1))
