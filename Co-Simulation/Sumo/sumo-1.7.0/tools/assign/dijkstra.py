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

# @file    dijkstra.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2007-10-25

"""
This script is based on the script from David Eppstein, UC Irvine.
This script is to find the shortest path from the given origin 'start' to the other nodes in the investigated network.
The Dijkstra algorithm is used for searching the respective shortest paths.
the link information about the shortest paths and the corresponding travel times
will be stored in the lists P and D respectively.
"""

import os
import sys
from collections import defaultdict
from xml.sax import make_parser, handler
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from sumolib.net import readNet  # noqa


class priorityDictionary(dict):

    def __init__(self):
        '''Initialize priorityDictionary by creating binary heap
            of pairs (value,key).  Note that changing or removing a dict entry will
            not remove the old pair from the heap until it is found by smallest() or
            until the heap is rebuilt.'''
        self.__heap = []
        dict.__init__(self)

    def smallest(self):
        '''Find smallest item after removing deleted items from heap.'''
        if len(self) == 0:
            raise IndexError("smallest of empty priorityDictionary")
        heap = self.__heap
        while heap[0][1] not in self or self[heap[0][1]] != heap[0][0]:
            lastItem = heap.pop()
            insertionPoint = 0
            while 1:
                smallChild = 2 * insertionPoint + 1
                if smallChild + 1 < len(heap) and \
                        heap[smallChild][0] > heap[smallChild + 1][0]:
                    smallChild += 1
                if smallChild >= len(heap) or lastItem <= heap[smallChild]:
                    heap[insertionPoint] = lastItem
                    break
                heap[insertionPoint] = heap[smallChild]
                insertionPoint = smallChild
        return heap[0][1]

    def __iter__(self):
        '''Create destructive sorted iterator of priorityDictionary.'''
        def iterfn():
            while len(self) > 0:
                x = self.smallest()
                yield x
                del self[x]
        return iterfn()

    def __setitem__(self, key, val):
        '''Change value stored in dictionary and add corresponding
            pair to heap.  Rebuilds the heap if the number of deleted items grows
            too large, to avoid memory leakage.'''
        dict.__setitem__(self, key, val)
        heap = self.__heap
        if len(heap) > 2 * len(self):
            self.__heap = [(v, k) for k, v in self.iteritems()]
            self.__heap.sort()  # builtin sort likely faster than O(n) heapify
        else:
            newPair = (val, key)
            insertionPoint = len(heap)
            heap.append(None)
            while insertionPoint > 0 and val < heap[(insertionPoint - 1) // 2][0]:
                heap[insertionPoint] = heap[(insertionPoint - 1) // 2]
                insertionPoint = (insertionPoint - 1) // 2
            heap[insertionPoint] = newPair

    def setdefault(self, key, val):
        '''Reimplement setdefault to call our customized __setitem__.'''
        if key not in self:
            self[key] = val
        return self[key]

    def update(self, other):
        for key in other.keys():
            self[key] = other[key]


def dijkstra(start, targets):
    # dictionary of final distances
    D = {}
    # dictionary of predecessors
    P = {}
    # est.dist. of non-final vert.
    Q = priorityDictionary()
    Q[start] = 0
    for v in Q:
        D[v] = Q[v]
        if targets.discard(v):
            if len(targets) == 0:
                return (D, P)
        isConflictCandidate = (v != start) and (P[v].conflictlink is not None)
        for edge in v.outEdges:
            w = edge.target
            vwLength = D[v] + edge.helpacttime
            if isConflictCandidate:
                if (edge.kind == "junction" and iter(edge.target.outEdges).next() in P[v].leftlink) or\
                   (edge.kind != "junction" and edge in P[v].leftlink):
                    vwLength += P[v].penalty

            if w not in D and (w not in Q or vwLength < Q[w]):
                Q[w] = vwLength
                P[w] = edge
    return (D, P)


def dijkstraPlain(start, targets):
    # dictionary of final distances
    D = {}
    # dictionary of predecessors
    P = {}
    # est.dist. of non-final vert.
    Q = priorityDictionary()
    Q[start] = 0
    for v in Q:
        D[v] = Q[v]
        if targets.discard(v):
            if len(targets) == 0:
                return (D, P)
        for edge in v.getOutgoing():
            w = edge._to
            vwLength = D[v] + edge.helpacttime

            if w not in D and (w not in Q or vwLength < Q[w]):
                Q[w] = vwLength
                P[w] = edge
    return (D, P)


def dijkstraBoost(boostGraph, start):
    from boost.graph import dijkstra_shortest_paths
    dijkstra_shortest_paths(boostGraph, start,
                            distance_map=boostGraph.vertex_properties[
                                'distance'],
                            predecessor_map=boostGraph.vertex_properties[
                                'predecessor'],
                            weight_map=boostGraph.edge_properties['weight'])

    # dictionary of final distances
    D = {}
    # dictionary of predecessors
    P = {}
    for v in boostGraph.vertices:
        D[v.partner] = v.distance
        for edge in v.partner.inEdges:
            if edge.source == v.predecessor.partner:
                P[v.partner] = edge
                break
    return (D, P)


class DijkstraRouter(handler.ContentHandler):

    """standalone class for routing on a sumolib network.
    The edges from the network recieve new attribute 'cost' based on
    a loaded meanData output
    """

    def __init__(self, netfile, weightfile=None):
        self.net = readNet(netfile)
        self.cost_attribute = 'traveltime'
        self.weightfile = weightfile
        if self.weightfile is not None:
            self.load_weights(self.weightfile)

    def load_weights(self, weightfile):
        # reset weights before loading
        for e in self.net.getEdges():
            e.cost = e.getLength() / e.getSpeed()
        self.weightfile = weightfile
        self.intervals = 0
        parser = make_parser()
        parser.setContentHandler(self)
        parser.parse(weightfile)

    def startElement(self, name, attrs):
        if name == 'interval':
            self.intervals += 1
            if self.intervals > 1:
                print("ignoring weights from interval [%s,%s]" % (
                    attrs['begin'], attrs['end']))
        if name == 'edge':
            if self.intervals > 1:
                return
            if self.cost_attribute in attrs:  # may be missing for some
                self.net.getEdge(attrs['id']).cost = float(
                    attrs[self.cost_attribute])

    def _route(self, start, dest, costFactors):
        if self.weightfile is None:
            raise Exception("not weights loaded")
        # dictionary of final distances
        D = {}
        # dictionary of predecessors
        P = {}
        # est.dist. of non-final vert.
        Q = priorityDictionary()
        Q[start] = 0
        for edge in Q:
            D[edge] = Q[edge]
            # print("final const to %s: %s" % (edge.getID(), D[edge]))
            if edge == dest:
                # print [(e.getID(), c) for e,c in Q.items()]
                return (D, P)
            cost = Q[edge] + edge.cost * costFactors[edge.getID()]
            for succ in edge.getOutgoing():
                if succ not in Q or Q[succ] > cost:
                    # print("reaching %s in %s (from %s)" % (succ.getID(), cost, edge.getID()))
                    Q[succ] = cost
                    P[succ] = edge
        return (D, P)

    def _getEdge(self, *ids):
        """retrieves edge objects based on their ids"""
        result = []
        for id in ids:
            if type(id) == str:
                if self.net.hasEdge(id):
                    result.append(self.net.getEdge(id))
                else:
                    raise Exception("unkown edge '%s'" % id)
            else:
                result.append(id)
        return result

    def least_cost(self, start, dest, costFactors=defaultdict(lambda: 1.0)):
        """return the cost of the shortest path from start to destination edge"""
        start, dest = self._getEdge(start, dest)
        D, P = self._route(start, dest, costFactors)
        if dest in D:
            return D[dest]
        else:
            raise("No path between %s and %s found" %
                  (start.getID(), dest.getID()))
