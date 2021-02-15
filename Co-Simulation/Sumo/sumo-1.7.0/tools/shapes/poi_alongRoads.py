#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

# @file    poi_alongRoads.py
# @author  Ronald Nippold
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2009-20-10

"""
Usage:   poi_alongRoads.py <NET> <EDGE_ID>[,<EDGE_ID>]* <DISTANCE>

Spatial distribute of POIs along given edges on a given network.
Edges are separated with comma and without spaces in between.
The distance between POIs may be any positive real number

So far POIs are situated in the middle on all edges without regard to the type of the edge (street, junction).
 Edges may be given in arbitrary order, connected edges are found automatically
 Therefore: crossing chains of connected edges are not allowed -> this needs two different runs of this script
 Output is written in file 'pois.add.xml'
"""
from __future__ import absolute_import
from __future__ import print_function

import os
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import sumolib.net  # noqa

if len(sys.argv) < 4:
    print("Usage: " +
          sys.argv[0] + " <NET> <EDGE_ID>[,<EDGE_ID>]* <DISTANCE>", file=sys.stderr)
    sys.exit()
edgeList = sys.argv[2].split(',')
POIdist = float(sys.argv[3])


def findPred(connEdgesTmp, edgeList, success=0):
    for edge in edgeList:
        # check first element of connEdgesTmp for predecessor
        if connEdgesTmp[0][1] == edge[2]:
            connEdgesTmp.insert(0, edge)
            edgeList.remove(edge)
            success = 1
            return success
    return success


def findSucc(connEdgesTmp, edgeList, success=0):
    for edge in edgeList:
        # check last element of connEdgesTmp for successor
        if connEdgesTmp[len(connEdgesTmp) - 1][2] == edge[1]:
            connEdgesTmp.append(edge)
            edgeList.remove(edge)
            success = 1
            return success
    return success


def poiAppend(poi_edge, poi_nr, poi_x, poi_y, poi_buf):
    poi_buf.append('    <poi id="%s-%s" type="default" color="1,0,0" layer="0" x="%.2f" y="%.2f"/>' %
                   (poi_edge, poi_nr, poi_x, poi_y))


print("Reading net ...")
net = sumolib.net.readNet(sys.argv[1])

# find all predecessor/successor nodes for the given edges
nodeList = []
for edge in net._edges:
    for elm in sys.argv[2].split(','):
        if str(edge._id) == elm:
            data = []
            data.append(edge._id)
            data.append(edge._from._id)
            data.append(edge._to._id)
            data.append(edge.getShape())
            nodeList.append(data)

# find connected edges
connEdges = []
connEdgesTmp = []
pred = 1
succ = 1
# real copy, not just reference because of loop in nodeList
tmpList = nodeList[:]
for elm in nodeList:
    if elm in tmpList:								# Has this edge already been processed?
        while pred == 1:							# search for predecessor
            if elm in tmpList:
                # don't look up the current edge in search list
                tmpList.remove(elm)
            if elm not in connEdgesTmp:
                # for the first element when searching for connected edges
                connEdgesTmp.append(elm)
            pred = findPred(connEdgesTmp, tmpList)
        while succ == 1:							# search for successor
            # all other things already done in predecessor
            succ = findSucc(connEdgesTmp, tmpList)
        connEdges.append(connEdgesTmp)
        connEdgesTmp = []							# clean up
        pred = 1									# reset
        succ = 1

# prepare output
POIbuf = []
POIbuf.append("<?xml version=\"1.0\"?>\n")
POIbuf.append("<!--\n\nPOIs for edges:\n\t" +
              str(sys.argv[2]) + "\n\non network:\n\t" + str(sys.argv[1]) + "\n-->\n")
POIbuf.append("<POIs>")
# space out POIs on the found (connected) edges
for elm in connEdges:
    # tmp array for storing all shape coordinate of connected edges
    shapes = []
    offset = 0										# Anything left from the edge before?
    for edge in elm:
        for p in edge[3]:
            #                shapes.append(p)
            # collect all shapes of connected edges
            shapes.append([p[0], p[1], edge[0]])
    for i in range(0, len(shapes) - 1):
        x1 = shapes[i][0]							# point at the beginning of the current edge
        y1 = shapes[i][1]
        x2 = shapes[i + 1][0]							# point at the end of the current edge
        y2 = shapes[i + 1][1]
        dX = x2 - x1
        dY = y2 - y1
        eucDist = pow(pow(dX, 2) + pow(dY, 2), 0.5)  # Euclidean distance
        # normalized Euclidean distance in x
        normAscX = 0 if dX == 0 else dX / eucDist
        # normalized Euclidean distance in y
        normAscY = 0 if dY == 0 else dY / eucDist
        xCur = x1									# the current POI position
        yCur = y1
        if i == 0:
            POIid = 1
            poiAppend(shapes[i][2], POIid, xCur, yCur, POIbuf)
            POIid = POIid + 1
            POIpos = POIdist
        else:
            if shapes[i][2] != shapes[i - 1][2]:		# begin of new edge
                POIid = 1
                # reset POI-position at the beginning of an edge
                POIpos = offset
        # check if first (new) point is on current edge, consider possible
        # offset from the edge before
        if eucDist + offset > POIdist:
            xCur = xCur + normAscX * (POIdist - offset)
            yCur = yCur + normAscY * (POIdist - offset)
            poiAppend(shapes[i][2], POIid, xCur, yCur, POIbuf)
            POIid = POIid + 1
            # always add POIdist, reset to current offset when changing the
            # edge
            POIpos = POIpos + POIdist
            offset = 0								# reset offset
            # other points - if possible - with normal spacing in between
            while pow(pow((x2 - xCur), 2) + pow((y2 - yCur), 2), 0.5) > POIdist:
                xCur = xCur + normAscX * POIdist
                yCur = yCur + normAscY * POIdist
                poiAppend(shapes[i][2], POIid, xCur, yCur, POIbuf)
                POIid = POIid + 1
                # always add POIdist, reset to current offset when changing the
                # edge
                POIpos = POIpos + POIdist
        # calculate offset (residual) for the next connected edge
        offset = offset + pow(pow((x2 - xCur), 2) + pow((y2 - yCur), 2), 0.5)
POIbuf.append("</POIs>")

print("writing output")
# finally write output in file
outfile = open('pois.add.xml', 'w')
for POIs in POIbuf:
    outfile.write(POIs + "\n")
outfile.close()
print("done")
