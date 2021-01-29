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

# @file    flowrouter.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2007-06-28

"""
This script does flow routing similar to the dfrouter.
It has three mandatory parameters, the SUMO net (.net.xml), a file
specifying detectors and one for the flows. It may detect the type
of the detectors (source, sink, inbetween) itself or read it from
the detectors file.
"""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import os
import random
import sys
import heapq
from xml.sax import make_parser, handler
from optparse import OptionParser
from collections import defaultdict
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib  # noqa
from sumolib.net.lane import get_allowed  # noqa
import detector  # noqa

# Vertex class which stores incoming and outgoing edges as well as
# auxiliary data for the flow computation. The members are accessed
# directly.

DEBUG = False
PULL_FLOW = True


class Vertex:

    def __init__(self):
        self.inEdges = []
        self.outEdges = []
        self.reset()

    def reset(self):
        self.inPathEdge = None
        self.flowDelta = sys.maxsize
        self.gain = 0
        self.value = 0
        if self.outEdges:
            for e in self.outEdges:
                self.value += (e.flow / e.numLanes if e.flow > 0 else -e.numLanes)
            self.value /= len(self.outEdges)

    def update(self, edge, flow, isForward):
        self.inPathEdge = edge
        self.flowDelta = flow
        if isForward:
            numSatEdges = edge.source.gain // edge.source.flowDelta
            self.gain = numSatEdges * flow
            if edge.startCapacity < sys.maxsize:
                self.gain += flow
        else:
            numSatEdges = edge.target.gain // edge.target.flowDelta
            self.gain = numSatEdges * flow
            if edge.startCapacity < sys.maxsize:
                self.gain -= flow

    def __repr__(self):
        return "<%s,%s,%s>" % (self.inPathEdge, self.flowDelta, self.gain)

    def __lt__(self, other):
        return self.value < other.value


# Edge class which stores start and end vertex, type amd label of the edge
# as well as flow and capacity for the flow computation and some parameters
# read from the net. The members are accessed directly.
class Edge:

    def __init__(self, label, source, target, kind, linkDir=None):
        self.label = label
        self.source = source
        self.target = target
        self.kind = kind
        self.maxSpeed = 0.0
        self.linkDir = linkDir
        self.length = 0.0
        self.numLanes = 0
        self.isOnSourcePath = False
        self.isOnSinkPath = False
        self.detGroup = []
        self.reset()

    def reset(self):
        self.capacity = sys.maxsize
        self.startCapacity = sys.maxsize
        self.flow = 0
        self.routes = []
        self.newRoutes = []

    def getDetFlow(self):
        result = 0
        for group in self.detGroup:
            if int(group.totalFlow) > result:
                result = int(group.totalFlow)
        return result

    def __repr__(self):
        cap = str(self.capacity)
        if self.capacity == sys.maxsize:
            cap = "inf"
        return self.kind + "_" + self.label + "<" + str(self.flow) + "|" + cap + ">"

    def __lt__(self, other):
        return self.label < other.label


# Route class storing the list of edges and the frequency of a route.
class Route:

    def __init__(self, freq, edges):
        self.frequency = freq
        self.edges = edges
        self.newFrequency = None

    def __repr__(self):
        result = str(self.frequency) + " * ["
        lastLabel = ""
        for edge in self.edges:
            if edge.kind == "real":
                if lastLabel:
                    result += lastLabel + ", "
                lastLabel = edge.label
        return result + lastLabel + "]"

    def __lt__(self, other):
        return self.edges < other.edges


# Net class which stores the network (vertex and edge collection) and the
# routes. All the algorithmic stuff and the output generation are also
# inside this class. The members are either "private" or have get/add/remove
# methods.
class Net:

    def __init__(self):
        self._vertices = []
        self._edges = {}
        self._internalEdges = []
        self._possibleSources = set()
        self._possibleSinks = set()
        self._source = self.newVertex()
        self._sink = self.newVertex()
        self._edgeRestriction = {}
        self._routeRestriction = {}
        if options.restrictionfile:
            for f in options.restrictionfile.split(","):
                for line in open(f):
                    ls = line.split()
                    if len(ls) == 2:
                        self._edgeRestriction[ls[1]] = int(ls[0])
                    else:
                        self._routeRestriction[tuple(ls[1:])] = int(ls[0])
            if options.verbose:
                print("Loaded %s edge restrictions and %s route restrictions" %
                      (len(self._edgeRestriction), len(self._routeRestriction)))

    def newVertex(self):
        v = Vertex()
        self._vertices.append(v)
        return v

    def getEdge(self, edgeLabel):
        if edgeLabel in self._edges:
            return self._edges[edgeLabel]
        else:
            raise RuntimeError("edge '%s' not found" % edgeLabel)

    def addEdge(self, edgeObj):
        edgeObj.source.outEdges.append(edgeObj)
        edgeObj.target.inEdges.append(edgeObj)
        if edgeObj.kind == "real":
            self._edges[edgeObj.label] = edgeObj
        else:
            assert edgeObj.target == self._sink or len(edgeObj.target.outEdges) == 1
            if len(edgeObj.target.outEdges) == 1:
                edgeObj.numLanes = edgeObj.target.outEdges[0].numLanes
            else:
                edgeObj.numLanes = 1
            self._internalEdges.append(edgeObj)

    def removeEdge(self, edgeObj):
        edgeObj.source.outEdges.remove(edgeObj)
        edgeObj.target.inEdges.remove(edgeObj)
        if edgeObj.kind == "real":
            del self._edges[edgeObj.label]
            checkEdges = set(edgeObj.source.inEdges).union(edgeObj.target.outEdges)
            for edge in checkEdges:
                if edge.kind != "real":
                    self.removeEdge(edge)

    def addIsolatedRealEdge(self, edgeLabel):
        self.addEdge(Edge(edgeLabel, self.newVertex(), self.newVertex(),
                          "real"))

    def addSourceEdge(self, edgeObj):
        newEdge = Edge("s_" + edgeObj.label, self._source, edgeObj.source,
                       "source")
        self.addEdge(newEdge)

    def addSinkEdge(self, edgeObj):
        newEdge = Edge("t_" + edgeObj.label, edgeObj.target, self._sink,
                       "sink")
        self.addEdge(newEdge)

    def trimNet(self):
        if options.minspeed > 0.0:
            if options.verbose:
                print("Removing edges with maxspeed < %s," %
                      options.minspeed, end=' ')
            # The code in the following loop assumes there are still all
            # auxiliary junction edges present.
            for edgeObj in self._edges.values():
                if edgeObj.maxSpeed < options.minspeed:
                    if len(edgeObj.detGroup) == 0 or not options.keepdet:
                        for auxpred in edgeObj.source.inEdges:
                            for realpred in auxpred.source.inEdges:
                                if realpred.maxSpeed >= options.minspeed:
                                    self._possibleSinks.add(realpred)
                        for auxsucc in edgeObj.target.outEdges:
                            for realsucc in auxsucc.target.outEdges:
                                if realsucc.maxSpeed >= options.minspeed:
                                    self._possibleSources.add(realsucc)
                        self.removeEdge(edgeObj)
            if options.verbose:
                print(len(self._edges), "left")
            if options.trimfile:
                trimOut = open(options.trimfile, 'w')
                for edge in self._edges.values():
                    print("edge:" + edge.label, file=trimOut)
                trimOut.close()

    def detectSourceSink(self, sources, sinks):
        self.trimNet()
        for id in sorted(sources):
            self.addSourceEdge(self.getEdge(id))
        for id in sorted(sinks):
            self.addSinkEdge(self.getEdge(id))
        foundSources = []
        foundSinks = []
        for edgeObj in sorted(self._edges.values()):
            if len(sources) == 0 and (len(edgeObj.source.inEdges) == 0 or edgeObj in self._possibleSources):
                if edgeObj.numLanes > 0:
                    self.addSourceEdge(edgeObj)
                    foundSources.append(edgeObj.label)
            if len(sinks) == 0 and (len(edgeObj.target.outEdges) == 0 or edgeObj in self._possibleSinks):
                if edgeObj.numLanes > 0:
                    if edgeObj.label in foundSources:
                        print("Warning! Edge '%s' is simultaneously source and sink." % edgeObj.label)
                        self.removeEdge(edgeObj)
                        foundSources.remove(edgeObj.label)
                        continue
                    self.addSinkEdge(edgeObj)
                    foundSinks.append(edgeObj.label)
        if options.verbose:
            print(("Loaded %s sources and %s sinks from detector file. Added %s sources and %s sinks " +
                   "from the network") % (
                len(sources), len(sinks), len(foundSources), len(foundSinks)))
        if options.source_sink_output:
            with open(options.source_sink_output, 'w') as outf:
                outf.write('<detectors>\n')
                freq = options.interval * 60 if options.interval else 24 * 3600
                for source in foundSources:
                    outf.write(('    <e1Detector id="%s_0" lane="%s_0" pos="1" type="source" friendlyPos="true" ' +
                                'file="NUL" freq="%s"/>\n') %
                               (source, source, freq))
                for sink in foundSinks:
                    outf.write(('    <e1Detector id="%s_0" lane="%s_0" pos="-1" type="sink" friendlyPos="true" ' +
                                'file="NUL" freq="%s"/>\n') %
                               (sink, sink, freq))
                outf.write('</detectors>\n')

        if len(self._sink.inEdges) == 0:
            print("Error! No sinks found.")
            return False
        if len(self._source.outEdges) == 0:
            print("Error! No sources found.")
            return False
        return True

    def initNet(self):
        for edge in self._internalEdges:
            edge.reset()
            flowRestriction = sys.maxsize
            if options.maxturnflow and edge.linkDir == 't':
                flowRestriction = int(options.maxturnflow * options.interval / 60)
            if edge.label in self._edgeRestriction:
                flowRestriction = int(self._edgeRestriction[edge.label] * options.interval / 60)
            edge.capacity = min(edge.capacity, flowRestriction)
        for edge in self._edges.values():
            edge.reset()
            if len(edge.detGroup) > 0:
                edge.capacity = 0
                for group in edge.detGroup:
                    if int(group.totalFlow) > edge.capacity:
                        edge.capacity = int(group.totalFlow)
            if not options.respectzero and edge.capacity == 0:
                edge.capacity = sys.maxsize
            edge.startCapacity = edge.capacity
            flowRestriction = sys.maxsize if edge.numLanes > 0 else 0
            if options.maxflow:
                flowRestriction = int(options.maxflow * edge.numLanes * options.interval / 60)
            if options.maxturnflow and edge.linkDir == 't':
                flowRestriction = int(options.maxturnflow * options.interval / 60)
            if edge.label in self._edgeRestriction:
                flowRestriction = int(self._edgeRestriction[edge.label] * options.interval / 60)
            edge.capacity = min(edge.capacity, flowRestriction)
        # collect limited source edges
        for s in self._source.outEdges:
            queue = [s]
            s.isOnSourcePath = True
            while queue:
                edgeObj = queue.pop(0)
                if edgeObj.startCapacity == sys.maxsize:
                    if len(edgeObj.target.inEdges) > 1:
                        s.isOnSourcePath = False
                        break
                    else:
                        queue += edgeObj.target.outEdges
        # collect limited sink edges
        for s in self._sink.inEdges:
            queue = [s]
            s.isOnSinkPath = True
            while queue:
                edgeObj = queue.pop(0)
                if edgeObj.startCapacity == sys.maxsize:
                    if len(edgeObj.source.outEdges) > 1:
                        s.isOnSinkPath = False
                        break
                    else:
                        queue += edgeObj.source.inEdges

        if options.verbose:
            unlimitedSource = 0
            for edgeObj in self._source.outEdges:
                for src in edgeObj.target.outEdges:
                    if src.capacity == sys.maxsize:
                        unlimitedSource += 1
            unlimitedSink = 0
            for edgeObj in self._sink.inEdges:
                for sink in edgeObj.source.inEdges:
                    if sink.capacity == sys.maxsize:
                        unlimitedSink += 1
            print(len(self._source.outEdges), "sources,", end=' ')
            print(unlimitedSource, "unlimited")
            print(len(self._sink.inEdges), "sinks,",
                  unlimitedSink, "unlimited")

    def splitRoutes(self, stubs, currEdge, upstreamBackEdges, newRoutes, alteredRoutes):
        newStubs = []
        backSet = set(upstreamBackEdges)
        while len(stubs) > 0:
            routeStub = stubs.pop()
            if len(routeStub.edges) > 0 and currEdge == routeStub.edges[0]:
                routeStub.edges.pop(0)
                newStubs.append(routeStub)
            else:
                if DEBUG:
                    print("      trying to split", routeStub)
                assert(len(currEdge.routes) > 0)
                for route in currEdge.routes + currEdge.newRoutes:
                    if route.newFrequency == 0:
                        continue
                    edgePos = route.edges.index(currEdge)
                    backPath = False
                    hadForward = False
                    for edge in route.edges[edgePos + 1:]:
                        if edge in backSet:
                            if hadForward:
                                if DEBUG:
                                    print("      skipping", route, "because", edge, "is in", backSet)
                                backPath = True
                                break
                        else:
                            hadForward = True
                    if backPath:
                        continue
                    routeFreq = route.frequency if route.newFrequency is None else route.newFrequency
                    newRoute = Route(min(routeStub.frequency, routeFreq),
                                     route.edges[:edgePos] + routeStub.edges)
                    newRoutes.append(newRoute)
                    for edge in newRoute.edges:
                        edge.newRoutes.append(route)
                    newStubs.append(Route(newRoute.frequency,
                                          route.edges[edgePos + 1:]))
                    if route.newFrequency is None:
                        route.newFrequency = route.frequency
                    route.newFrequency -= newRoute.frequency
                    alteredRoutes.append(route)
                    routeStub.frequency -= newRoute.frequency
                    if routeStub.frequency == 0:
                        break
                if routeStub.frequency > 0:
                    if DEBUG:
                        print("      Could not split", routeStub)
                    return False
        stubs.extend(newStubs)
        return True

    def updateFlow(self, startVertex, endVertex):
        assert endVertex.flowDelta < sys.maxsize
        if options.limit and endVertex.flowDelta > options.limit:
            endVertex.flowDelta = options.limit
        stubs = [Route(endVertex.flowDelta, [])]
        if DEBUG:
            print("  updateFlow start=%s end=%s flowDelta=%s" % (startVertex,
                                                                 endVertex, endVertex.flowDelta))
        upstreamBackEdges = list(self.getBackEdges(startVertex, endVertex))
        newRoutes = []
        alteredRoutes = []
        flowDeltas = []
        cycleStartStep = (startVertex == endVertex)
        currVertex = endVertex
        while currVertex != startVertex or cycleStartStep:
            cycleStartStep = False
            currEdge = currVertex.inPathEdge
            if currEdge.target == currVertex:
                if DEBUG:  # and not currEdge.kind == 'junction':
                    print("    incFlow edge=%s delta=%s" % (currEdge, endVertex.flowDelta))
                flowDeltas.append((currEdge, endVertex.flowDelta))
                currVertex = currEdge.source
                for routeStub in stubs:
                    routeStub.edges.insert(0, currEdge)
            else:
                if DEBUG:
                    print("    decFlow edge=%s delta=%s" % (currEdge, endVertex.flowDelta))
                flowDeltas.append((currEdge, -endVertex.flowDelta))
                currVertex = currEdge.target
                upstreamBackEdges.pop(0)
                if not self.splitRoutes(stubs, currEdge, upstreamBackEdges, newRoutes, alteredRoutes):
                    # resetting to previous state
                    for route in alteredRoutes:
                        route.newFrequency = None
                    for route in newRoutes:
                        for edge in route.edges:
                            del edge.newRoutes[:]
                    if DEBUG:
                        self.testFlowInvariants()
                    return False
        if DEBUG:
            self.testFlowInvariants()
        # up to here no modification of existing edges or routes in case splitting fails
        for edge, delta in flowDeltas:
            edge.flow += delta
        for route in alteredRoutes:
            if route.newFrequency is not None:  # otherwise it has been handled before
                if route.newFrequency == 0:
                    for edge in route.edges:
                        edge.routes.remove(route)
                else:
                    route.frequency = route.newFrequency
                route.newFrequency = None
        for route in stubs + newRoutes:
            for edge in route.edges:
                edge.routes.append(route)
                del edge.newRoutes[:]
        if DEBUG:
            self.testFlowInvariants()
        return True

    def endsRestrictedRoute(self, edge):
        if not self._routeRestriction:
            return False
        currVertex = edge.source
        routeEdgeObj = [edge]
        route = []
        count = currVertex.flowDelta
        if options.limit and count > options.limit:
            count = options.limit
        while currVertex != self._source:
            edge = currVertex.inPathEdge
            if edge.target == currVertex:
                if edge.kind == "real":
                    route.insert(0, edge.label)
                routeEdgeObj.insert(0, edge)
                currVertex = edge.source
            else:
                return False
        for r in edge.routes:
            if r.edges == routeEdgeObj:
                count += r.frequency
        if DEBUG:
            print("    checking limit for route %s count %s" % (route, count))
        return count > self._routeRestriction.get(tuple(route),
                                                  # check origin-destination restriction
                                                  self._routeRestriction.get((route[0], route[-1]), count))

    def getBackEdges(self, pathStart, currVertex):
        cycleStartStep = (pathStart == currVertex)
        while currVertex != pathStart or cycleStartStep:
            cycleStartStep = False
            edge = currVertex.inPathEdge
            if edge.source == currVertex:
                yield edge
                currVertex = edge.target
            else:
                currVertex = edge.source

    def findPath(self, startVertex, pathStart, limitedSource=True, limitedSink=True, allowBackward=True):
        queue = [startVertex]
        if DEBUG:
            print("  findPath start=%s pathStart=%s limits(%s, %s)" %
                  (startVertex, pathStart, limitedSource, limitedSink))
        while len(queue) > 0:
            currVertex = heapq.heappop(queue)
            if currVertex == self._sink or (currVertex == self._source and currVertex.inPathEdge):
                if self.updateFlow(pathStart, currVertex):
                    return True
                continue
            for edge in currVertex.outEdges:
                if limitedSource and currVertex == self._source and not edge.isOnSourcePath:
                    continue
                if limitedSink and edge.target == self._sink and not edge.isOnSinkPath:
                    continue
                if edge.target == self._sink and self.endsRestrictedRoute(edge):
                    continue
                if not edge.target.inPathEdge and edge.flow < edge.capacity:
                    if edge.target != self._sink or currVertex.gain > 0:
                        heapq.heappush(queue, edge.target)
                        edge.target.update(edge, min(currVertex.flowDelta,
                                                     edge.capacity - edge.flow),
                                           True)
            if allowBackward:
                for edge in currVertex.inEdges:
                    if not edge.source.inPathEdge and edge.flow > 0:
                        if edge.source != self._source or currVertex.gain > 0:
                            heapq.heappush(queue, edge.source)
                            edge.source.update(edge, min(currVertex.flowDelta,
                                                         edge.flow), False)
        return False

    def savePulledPath(self, startVertex, unsatEdge, pred):
        numSatEdges = 1
        currVertex = startVertex
        while currVertex != unsatEdge.source:
            currEdge = pred[currVertex]
            if currEdge.target == currVertex:
                currEdge.source.inPathEdge = currEdge
                currVertex = currEdge.source
                if currEdge.capacity < sys.maxsize:
                    numSatEdges -= 1
            else:
                currEdge.target.inPathEdge = currEdge
                currVertex = currEdge.target
                if currEdge.capacity < sys.maxsize:
                    numSatEdges += 1
        startVertex.inPathEdge = None
        unsatEdge.target.flowDelta = startVertex.flowDelta
        unsatEdge.target.gain = startVertex.flowDelta * numSatEdges

    def pullFlow(self, unsatEdge, limitSource, limitSink, allowBackward):
        if DEBUG:
            print("Trying to increase flow on", unsatEdge)
        for vertex in self._vertices:
            vertex.reset()
        pred = {unsatEdge.target: unsatEdge, unsatEdge.source: unsatEdge}
        unsatEdge.target.inPathEdge = unsatEdge
        unsatEdge.source.flowDelta = unsatEdge.capacity - unsatEdge.flow
        queue = [unsatEdge.source]
        while len(queue) > 0:
            currVertex = queue.pop(0)
            if ((currVertex == self._source and (not limitSource or pred[currVertex].isOnSourcePath)) or
                    currVertex == self._sink):
                self.savePulledPath(currVertex, unsatEdge, pred)
                return self.findPath(unsatEdge.target, currVertex, limitSource, limitSink, allowBackward)
            for edge in currVertex.inEdges:
                if edge.source not in pred and edge.flow < edge.capacity:
                    queue.append(edge.source)
                    pred[edge.source] = edge
                    edge.source.flowDelta = min(
                        currVertex.flowDelta, edge.capacity - edge.flow)
            if allowBackward:
                # inverse find path semantics
                for edge in currVertex.outEdges:
                    if edge.target not in pred and edge.flow > 0:
                        queue.append(edge.target)
                        pred[edge.target] = edge
                        edge.target.flowDelta = min(
                            currVertex.flowDelta, edge.flow)
        return False

    def testFlowInvariants(self):
        # the following code only tests assertions
        for vertex in self._vertices:
            flowSum = 0
            for preEdge in vertex.inEdges:
                flowSum += preEdge.flow
            for succEdge in vertex.outEdges:
                flowSum -= succEdge.flow
                totalEdgeFlow = 0
                for route in succEdge.routes:
                    assert route.frequency > 0
                    totalEdgeFlow += route.frequency
                if DEBUG and totalEdgeFlow != succEdge.flow:
                    print("total edge flow failed", totalEdgeFlow, succEdge)
                    for r in succEdge.routes:
                        print(r)
                assert totalEdgeFlow == succEdge.flow
            assert vertex == self._source or vertex == self._sink or flowSum == 0

    def calcRoutes(self, allowBackward=True):
        for limitSource, limitSink in ((True, True), (True, False), (False, True), (False, False)):
            pathFound = True
            while pathFound:
                for vertex in self._vertices:
                    vertex.reset()
                pathFound = self.findPath(self._source, self._source, limitSource, limitSink, allowBackward)
                if not pathFound and PULL_FLOW and not limitSource and not limitSink:
                    for i, edge in enumerate(sorted(self._edges.values())):
                        if DEBUG and options.verbose and i > 0 and i % 100 == 0:
                            print("pullFlow %.2f%%" % (100 * i / len(self._edges)))
                        if edge.startCapacity < sys.maxsize:
                            while (edge.flow < edge.capacity and
                                   self.pullFlow(edge, limitSource, limitSink, allowBackward)):
                                pathFound = True
                    if DEBUG:
                        totalDetFlow = 0
                        for edge in self._edges.values():
                            if edge.startCapacity < sys.maxsize:
                                totalDetFlow += edge.flow
                        print("detFlow", totalDetFlow)
        if DEBUG:
            self.testFlowInvariants()
        self.consolidateRoutes()
        self.testFlowInvariants()

    def consolidateRoutes(self):
        for edge in self._source.outEdges:
            routeByEdges = {}
            for route in edge.routes:
                key = tuple([e.label for e in route.edges if e.kind == "real"])
                if key in routeByEdges:
                    routeByEdges[key].frequency += route.frequency
                    for e in route.edges[1:]:
                        e.routes.remove(route)
                elif route.frequency > 0:
                    routeByEdges[key] = route
            edge.routes = sorted(routeByEdges.values())

    def applyRouteRestrictions(self):
        removed = False
        deleteRoute = []
        for edge in self._source.outEdges:
            for route in edge.routes:
                key = tuple([e.label for e in route.edges if e.kind == "real"])
                restriction = self._routeRestriction.get(key,
                                                         self._routeRestriction.get((key[0], key[-1]), sys.maxsize))
                surplus = route.frequency - restriction
                if surplus > 0:
                    if DEBUG:
                        print("route '%s' surplus=%s" % (" ".join(key), surplus))
                    removed = True
                    for e in route.edges:
                        e.flow -= surplus
                    for e in route.edges[1:]:
                        if restriction == 0:
                            e.routes.remove(route)
                    if restriction == 0:
                        deleteRoute.append(route)
                    else:
                        route.frequency = restriction
            if deleteRoute:
                edge.routes = [r for r in edge.routes if r not in deleteRoute]
        if DEBUG:
            self.testFlowInvariants()
        return removed

    def writeRoutes(self, routeOut, suffix=""):
        totalFlow = 0
        for edge in self._source.outEdges:
            totalFlow += edge.flow
            targetCount = defaultdict(lambda: 0)
            for route in edge.routes:
                if routeOut is None:
                    print(route)
                    continue
                firstReal = ''
                lastReal = None
                routeString = ''
                for redge in route.edges:
                    if redge.kind == "real":
                        if options.lanebased:
                            routeString += redge.label[:redge.label.rfind("_")] + " "
                        else:
                            routeString += redge.label + " "
                        if firstReal == '':
                            firstReal = redge.label
                        lastReal = redge
                assert firstReal != '' and lastReal is not None
                index = "" if targetCount[lastReal] == 0 else ".%s" % targetCount[lastReal]
                targetCount[lastReal] += 1
                route.routeID = "%s_%s%s%s" % (firstReal, lastReal.label, index, suffix)
                print('    <route id="%s" edges="%s"/>' % (
                    route.routeID, routeString.strip()), file=routeOut)
        if routeOut is None:
            print("total flow:", totalFlow)

    def writeEmitters(self, emitOut, begin=0, end=3600, suffix=""):
        if not emitOut:
            return
        totalFlow = 0
        numSources = 0
        unusedSources = []
        for srcEdge in self._source.outEdges:
            if len(srcEdge.routes) == 0:
                unusedSources.append(srcEdge.target.outEdges[0].label)
                continue
            assert len(srcEdge.target.outEdges) == 1
            totalFlow += srcEdge.flow
            numSources += 1
            edge = srcEdge.target.outEdges[0]
            if options.random:
                ids = " ".join(r.routeID for r in srcEdge.routes)
                probs = " ".join([str(route.frequency)
                                  for route in srcEdge.routes])
                print('    <flow id="%s%s" %s number="%s" begin="%s" end="%s">' %
                      (edge.label, suffix, options.params, int(srcEdge.flow), begin, end), file=emitOut)
                print('        <routeDistribution routes="%s" probabilities="%s"/>' % (ids, probs), file=emitOut)
                print('    </flow>', file=emitOut)
            else:
                for route in srcEdge.routes:
                    via = ""
                    if options.viadetectors:
                        realEdges = [e for e in route.edges if e.kind == "real"]
                        # exclude detectors on 'from' and 'to' edge
                        detEdges = [e.label for e in realEdges[1:-1] if e.getDetFlow() > 0]
                        # avoid duplicate via-edges
                        viaEdges = []
                        for e in detEdges:
                            if not viaEdges or viaEdges[-1] != e:
                                viaEdges.append(e)
                        if viaEdges:
                            via = ' via="%s"' % " ".join(viaEdges)
                    print('    <flow id="%s" %s route="%s" number="%s" begin="%s" end="%s"%s/>' %
                          (route.routeID, options.params, route.routeID,
                           int(route.frequency), begin, end, via), file=emitOut)

        if options.verbose:
            print("Writing %s vehicles from %s sources between time %s and %s (minutes)" % (
                totalFlow, numSources, int(begin / 60), int(end / 60)))
            if len(unusedSources) > 0:
                print("  unused sources:", " ".join(unusedSources))

        for s in self._sink.inEdges:
            queue = [s]
            s.isOnSinkPath = True
            while queue:
                queue.pop(0)

    def writeFlowPOIs(self, poiOut, suffix=""):
        if not poiOut:
            return
        for edge in self._edges.values():
            color = "0,0,1"
            for src in edge.source.inEdges:
                if src.source == self._source:
                    color = "0,1,0"
                    break
            for sink in edge.target.outEdges:
                if sink.target == self._sink:
                    color = "1," + color[2] + ",0"
                    break
            if edge.flow == edge.startCapacity:
                color = ".5,.5,.5"
            cap = ":c" + str(edge.startCapacity)
            if edge.startCapacity == sys.maxsize:
                cap = ""
            if edge.isOnSourcePath:
                cap += "-source"
            if edge.isOnSinkPath:
                cap += "-sink"
            lane = edge.label if options.lanebased else edge.label + "_0"
            print('    <poi id="%s_f%s%s%s" color="%s" lane="%s" pos="%s"/>' % (
                edge.label, edge.flow, cap, suffix, color, lane, random.random() * edge.length), file=poiOut)


# The class for parsing the XML and CSV input files. The data parsed is
# written into the net. All members are "private".
class NetDetectorFlowReader(handler.ContentHandler):

    def __init__(self, net):
        self._net = net
        self._edge = ''
        self._lane2edge = {}
        self._detReader = None

    def startElement(self, name, attrs):
        if name == 'edge' and ('function' not in attrs or attrs['function'] != 'internal'):
            self._edge = attrs['id']
            if not options.lanebased:
                self._net.addIsolatedRealEdge(attrs['id'])
        elif name == 'connection':
            fromEdgeID = attrs['from']
            if fromEdgeID[0] != ":":
                toEdgeID = attrs['to']
                if options.lanebased:
                    fromEdgeID += "_" + attrs["fromLane"]
                    toEdgeID += "_" + attrs["toLane"]
                v = self._net.getEdge(fromEdgeID).target
                eID = fromEdgeID + "->" + toEdgeID
                for e in v.outEdges:
                    if e.label == eID:
                        return
                newEdge = Edge(eID, v, self._net.getEdge(toEdgeID).source, "junction", attrs['dir'])
                self._net.addEdge(newEdge)
        elif name == 'lane' and self._edge != '':
            if options.lanebased:
                self._net.addIsolatedRealEdge(attrs['id'])
                self._edge = attrs['id']
            self._lane2edge[attrs['id']] = self._edge
            edgeObj = self._net.getEdge(self._edge)
            edgeObj.maxSpeed = max(edgeObj.maxSpeed, float(attrs['speed']))
            edgeObj.length = float(attrs['length'])
            if (options.vclass is None or
                    options.vclass in get_allowed(attrs.get('allow'), attrs.get('disallow'))):
                edgeObj.numLanes += 1

    def endElement(self, name):
        if name == 'edge':
            self._edge = ''

    def readDetectors(self, detFile):
        self._detReader = detector.DetectorReader(detFile, self._lane2edge)
        for edge, detGroups in self._detReader._edge2DetData.items():
            for group in detGroups:
                if group.isValid:
                    self._net.getEdge(edge).detGroup.append(group)
        sources = set()
        sinks = set()
        for det in sumolib.xml.parse(detFile, ["detectorDefinition", "e1Detector"]):
            if hasattr(det, "type"):
                if det.type == "source":
                    if options.lanebased:
                        sources.add(det.lane)
                    else:
                        sources.add(det.lane[:det.lane.rfind("_")])
                if det.type == "sink":
                    if options.lanebased:
                        sinks.add(det.lane)
                    else:
                        sinks.add(det.lane[:det.lane.rfind("_")])
        return sources, sinks

    def readFlows(self, flowFile, t=None, tMax=None):
        if t is None:
            return self._detReader.readFlows(flowFile, flow=options.flowcol)
        else:
            return self._detReader.readFlows(flowFile, flow=options.flowcol, time="Time", timeVal=t, timeMax=tMax)

    def clearFlows(self):
        self._detReader.clearFlows()

    def readSyntheticFlows(self, start=None, end=None):
        if options.syntheticflowfile is None:
            return
        if start is not None:
            times = [float(t) / 100. for t in options.timeline.split(",")]
            factor = sum([times[t] for t in range(start // 60, end // 60)])
        else:
            factor = 1.
        for f in options.syntheticflowfile.split(","):
            for line in open(f):
                flow, edge = line.split()
                edgeObj = self._net.getEdge(edge)
                groups = self._detReader.getEdgeDetGroups(edge)
                if len(groups) == 0:
                    self._detReader.addDetector(edge, edgeObj.length / 2, edge)
                    self._net.getEdge(edge).detGroup.append(self._detReader.getEdgeDetGroups(edge)[0])
                self._detReader.addFlow(edge, int(float(flow) * factor))


def addFlowFile(option, opt_str, value, parser):
    if not getattr(parser.values, option.dest, None):
        setattr(parser.values, option.dest, [])
    fileList = getattr(parser.values, option.dest)
    fileList.append(value)
    index = 0
    while index < len(parser.rargs) and not parser.rargs[index].startswith("-"):
        index += 1
    fileList.extend(parser.rargs[0:index])
    parser.rargs = parser.rargs[index:]


optParser = OptionParser()
optParser.add_option("-n", "--net-file", dest="netfile",
                     help="read SUMO network from FILE (mandatory)", metavar="FILE")
optParser.add_option("-d", "--detector-file", dest="detfile",
                     help="read detectors from FILE (mandatory)", metavar="FILE")
optParser.add_option("--revalidate-detectors", action="store_true", dest="revalidate",
                     default=False, help="ignore source and sink information in detector file")
optParser.add_option("-f", "--detector-flow-files", dest="flowfiles",
                     action="callback", callback=addFlowFile, type="string",
                     help="read detector flows from FILE(s) (mandatory)", metavar="FILE")
optParser.add_option("-c", "--flow-column", dest="flowcol", default="qPKW",
                     help="which column contains flows", metavar="STRING")
optParser.add_option("-o", "--routes-output", dest="routefile",
                     help="write routes to FILE", metavar="FILE")
optParser.add_option("-e", "--emitters-output", dest="emitfile",
                     help="write emitters to FILE and create files per emitter (needs -o)", metavar="FILE")
optParser.add_option("-y", "--params", help="vehicle / flow params to use (vType, departPos etc.)",
                     default='departSpeed="max" departPos="last" departLane="best"', metavar="STRING")
optParser.add_option("-t", "--trimmed-output", dest="trimfile",
                     help="write edges of trimmed network to FILE", metavar="FILE")
optParser.add_option("-p", "--flow-poi-output", dest="flowpoifile",
                     help="write resulting flows as SUMO POIs to FILE", metavar="FILE")
optParser.add_option("--source-sink-output", dest="source_sink_output",
                     help="write sources and sinks in detector format to FILE", metavar="FILE")
optParser.add_option("-m", "--min-speed", type="float", dest="minspeed",
                     default=0.0, help="only consider edges where the fastest lane allows at least this " +
                                       "maxspeed (m/s)")
optParser.add_option("-M", "--max-flow", type="int", dest="maxflow",
                     help="limit the number of vehicles per lane and hour to this value")
optParser.add_option("--max-turn-flow", type="int", dest="maxturnflow",
                     help="limit the number of vehicles per turn-around connection and hour to this value")
optParser.add_option("-r", "--flow-restrictions", dest="restrictionfile",
                     help="read edge and route restrictions from FILEs (each line starts with '<maxHourlyFlow> ' " +
                           "followed by <edgeID> or '<originEdgeID> <destEdgeID>' or '<e1> <e2> ... <en>')",
                           metavar="FILE+")
optParser.add_option("-s", "--synthetic-flows", dest="syntheticflowfile",
                     help="read artificial detector values from FILE (lines of the form '<dailyFlow> <edgeID>')",
                     metavar="FILE")
optParser.add_option("--timeline", default=("0.9,0.5,0.2,0.2,0.5,1.3,7.0,9.3,6.7,4.2,4.0,3.8," +
                                            "4.1,4.6,5.0,6.7,9.6,9.2,7.1,4.8,3.5,2.7,2.2,1.9"),
                     help="use time line for artificial detector values")
optParser.add_option("-D", "--keep-det", action="store_true", dest="keepdet",
                     default=False, help='keep edges with detectors when deleting "slow" edges')
optParser.add_option("-z", "--respect-zero", action="store_true", dest="respectzero",
                     default=False, help="respect detectors without data (or with permanent zero) with zero flow")
optParser.add_option("-l", "--lane-based", action="store_true", dest="lanebased",
                     default=False, help="do not aggregate detector data and connections to edges")
optParser.add_option("-i", "--interval", type="int", help="aggregation interval in minutes")
optParser.add_option("-b", "--begin", type="int", help="begin time in minutes")
optParser.add_option("--limit", type="int", help="limit the amount of flow assigned in a single step")
optParser.add_option("--vclass", help="only consider lanes that allow the given vehicle class")
optParser.add_option("-q", "--quiet", action="store_true", dest="quiet",
                     default=False, help="suppress warnings")
optParser.add_option("--random", action="store_true", dest="random",
                     default=False, help="write route distributions instead of separate flows")
optParser.add_option("--via-detectors", action="store_true", dest="viadetectors",
                     default=False, help="set used detectors as via-edges for generated flows")
optParser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                     default=False, help="tell me what you are doing")
optParser.add_option("--debug", action="store_true", default=False, help="tell me what you are doing in high detail")
(options, args) = optParser.parse_args()
if not options.netfile or not options.detfile or not options.flowfiles:
    optParser.print_help()
    sys.exit()
if options.emitfile and not options.routefile:
    optParser.print_help()
    sys.exit()
if (options.restrictionfile is not None or options.maxflow is not None) and options.interval is None:
    print("Restrictions need interval length")
    optParser.print_help()
    sys.exit()

DEBUG = options.debug
parser = make_parser()
if options.verbose:
    print("Reading net")
net = Net()
reader = NetDetectorFlowReader(net)
parser.setContentHandler(reader)
parser.parse(options.netfile)
if options.verbose:
    print(len(net._edges), "edges read")
    print("Reading detectors")
sources, sinks = reader.readDetectors(options.detfile)
if options.revalidate:
    sources = sinks = []
if net.detectSourceSink(sources, sinks):
    routeOut = None
    if options.routefile:
        routeOut = open(options.routefile, 'w')
        print("<routes>", file=routeOut)
    emitOut = None
    if options.emitfile:
        emitOut = open(options.emitfile, 'w')
        print("<additional>", file=emitOut)
    poiOut = None
    if options.flowpoifile:
        poiOut = open(options.flowpoifile, 'w')
        print("<pois>", file=poiOut)
    if options.interval:
        tMin = None
        tMax = None
        for flow in options.flowfiles:
            tMin, tMax = reader._detReader.findTimes(flow, tMin, tMax)
        if tMin is None:
            print("No flows in '%s'" % flow)
            sys.exit(1)
        if options.begin is not None and options.begin > tMin:
            tMin = options.begin
        if options.verbose:
            print("Reading flows between %s and %s" % (tMin, tMax))
        start = int(tMin - (tMin % options.interval))
        while start <= tMax:
            suffix = ".%s.%s" % (options.flowcol, start)
            for flow in options.flowfiles:
                haveFlows = reader.readFlows(
                    flow, start, start + options.interval)
            if haveFlows:
                reader.readSyntheticFlows(start, start + options.interval)
                if options.verbose:
                    print("Calculating routes")
                net.initNet()
                net.calcRoutes()
                # run again (in forward only mode) if restricted routes were removed
                if net.applyRouteRestrictions():
                    net.calcRoutes(False)
                net.writeRoutes(routeOut, suffix)
                net.writeEmitters(
                    emitOut, 60 * start, 60 * (start + options.interval), suffix)
                net.writeFlowPOIs(poiOut, suffix)
            else:
                if options.verbose:
                    print("No flows found")
            reader.clearFlows()
            start += options.interval
    else:
        if options.verbose:
            print("Reading flows")
        for flow in options.flowfiles:
            reader.readFlows(flow)
        reader.readSyntheticFlows()
        if options.verbose:
            print("Calculating routes")
        net.initNet()
        net.calcRoutes()
        # run again (in forward only mode) if restricted routes were removed
        if net.applyRouteRestrictions():
            net.calcRoutes(False)
        net.writeRoutes(routeOut, "." + options.flowcol)
        net.writeEmitters(emitOut, suffix=options.flowcol)
        net.writeFlowPOIs(poiOut)
    if routeOut:
        print("</routes>", file=routeOut)
        routeOut.close()
    if emitOut:
        print("</additional>", file=emitOut)
        emitOut.close()
    if poiOut:
        print("</pois>", file=poiOut)
        poiOut.close()
