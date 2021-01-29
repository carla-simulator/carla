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

# @file    routes_Join.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    11.09.2009

"""
Joins routes belonging to two networks.
A map file with the following syntax is read first:
 <EDGE_ID>-><EDGE_ID>
It describes at which edges the networks overlapped and which edge
 was kept in the resulting one.
Vehicles from both given route files starting at the first of those
 edges will be at first not regarded, but their routes are kept in memory.
All other vehicles are kept, but if they pass one of the mapped edges, the
 route is continued by choosing a random route from the list of previously
 discarded ones which start at the corresponding edge.

Attention! The routes are not sorted in time!
"""
from __future__ import absolute_import
from __future__ import print_function
import sys
from xml.sax import make_parser, handler


class Route:

    def __init__(self, vPars, edges):
        self._vPars = vPars
        self._edges = edges

    def encodeVehicle(self):
        ret = "<vehicle"
        for a in self._vPars:
            ret = ret + " " + a + "=\"" + self._vPars[a] + "\""
        ret = ret + ">"
        return ret

    def firstEdge(self):
        return self._edges[0]


class RoutesReader(handler.ContentHandler):

    def __init__(self, prefix, replacements):
        self._prefix = prefix
        self._replacements = {}
        for r in replacements:
            rs = replacements[r].split(",")
            for r2 in rs:
                self._replacements[r2] = 1
        self._routes = []
        self._continuations = {}
        self._continuationsSum = {}

    def endDocument(self):
        self._continuationsBackup = {}
        self._continuationsSumBackup = {}
        for e in self._continuations:
            self._continuationsBackup[e] = []
            self._continuationsBackup[e].extend(self._continuations[e])
            self._continuationsSumBackup[e] = self._continuationsSum[e]

    def startElement(self, name, attrs):
        if name == "vehicle":
            self._vPars = {}
            for a in attrs.keys():
                self._vPars[a] = attrs[a]
        if name == "route":
            edges = attrs["edges"].split(" ")
            for i in range(0, len(edges)):
                edges[i] = self._prefix + edges[i]
            self._routes.append(Route(self._vPars, edges))
            for i in range(0, len(edges)):
                if edges[i] in self._replacements:
                    if edges[i] not in self._continuations:
                        self._continuations[edges[i]] = []
                        self._continuationsSum[edges[i]] = 0
                    self._continuations[edges[i]].append(edges[i:])
                    self._continuationsSum[
                        edges[i]] = self._continuationsSum[edges[i]] + 1

    def getContinuation(self, beginEdge, replacements):
        assert(beginEdge in replacements)
        rEdge = replacements[beginEdge]
        if rEdge.find(",") >= 0:
            rEdge = rEdge.split(",")[0]
        edges = self._continuations[rEdge][-1]
        self._continuations[rEdge].pop()
        self._continuationsSum[rEdge] = self._continuationsSum[rEdge] - 1
        if self._continuationsSum[rEdge] == 0:
            self._continuations[rEdge] = []
            self._continuations[rEdge].extend(self._continuationsBackup[rEdge])
            self._continuationsSum[rEdge] = self._continuationsSumBackup[rEdge]
        return edges


def writeRoute(fdo, edges, conts1, conts2, replacements):
    replaceFrom = None
    for e in edges:
        if e in replacements:
            if not replaceFrom:
                replaceFrom = e
            continue
        fdo.write(e + " ")
    if replaceFrom:
        if replaceFrom[0] == conts2._prefix:
            cont = conts1.getContinuation(replaceFrom, replacements)
            writeRoute(fdo, cont, conts1, conts2, replacements)
        else:
            cont = conts2.getContinuation(replaceFrom, replacements)
            writeRoute(fdo, cont, conts1, conts2, replacements)


def writeVehicle(fdo, route, conts1, conts2, replacements):
    fdo.write("    " + route.encodeVehicle() + "<route edges=\"")
    writeRoute(fdo, route._edges, conts1, conts2, replacements)
    fdo.write("\"></route></vehicle>\n")


if len(sys.argv) < 6:
    print("Usage: " +
          sys.argv[0] + " <prefix#1> <routes#1> <prefix#2> <routes#2> <mapfile>")
    sys.exit()
# read mapping
mmap = {}
fd = open(sys.argv[5])
for line in fd:
    if line.find("->") < 0:
        continue
    (orig, dest) = line.strip().split("->")
    mmap[orig] = dest
fd.close()
# read routes
parser = make_parser()
print("Reading routes#1")
routes1 = RoutesReader(sys.argv[1], mmap)
parser.setContentHandler(routes1)
parser.parse(sys.argv[2])
print("Reading routes#2")
routes2 = RoutesReader(sys.argv[3], mmap)
parser.setContentHandler(routes2)
parser.parse(sys.argv[4])

print("Processing routes#1")
fdo = open("joined.rou.xml", "w")
fdo.write("<routes>\n")
for r in routes1._routes:
    if r.firstEdge() in mmap:
        continue
        # skip, starts at a replaced edge
    writeVehicle(fdo, r, routes1, routes2, mmap)
print("Processing routes#2")
for r in routes2._routes:
    if r.firstEdge() in mmap:
        continue
        # skip, starts at a replaced edge
    writeVehicle(fdo, r, routes1, routes2, mmap)
fdo.write("</routes>\n")
fdo.close()
