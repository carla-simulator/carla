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

# @file    inputs.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

"""
This script is to retrieve the assignment parameters, the OD districts and the matrix from the input files.
Moreover, the link travel time for district connectors will be estimated.
"""
from __future__ import print_function

# read the analyzed matrix


def getMatrix(net, verbose, matrix, matrixSum, demandscale=None):
    matrixPshort = []
    startVertices = []
    endVertices = []
    Pshort_EffCells = 0
    periodList = []
    if verbose:
        print('matrix:', str(matrix))
    odpairs = 0
    origins = 0
    dest = 0
    currentMatrixSum = 0.0
    skipCount = 0
    zones = 0
    smallDemandNum = 0
    for line in open(matrix):
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
                            for startVertex in net.getstartVertices():
                                if startVertex._id == elem:
                                    startVertices.append(startVertex)
                            for endVertex in net.getendVertices():
                                if endVertex._id == elem:
                                    endVertices.append(endVertex)
                    origins = len(startVertices)
                    dest = len(endVertices)
                elif len(startVertices) == zones:
                    if odpairs % origins == 0:
                        matrixPshort.append([])
                    for item in line.split():
                        elem = float(item)
                        if demandscale is not None and demandscale != 1.:
                            elem *= demandscale
                        matrixPshort[-1].append(elem)
                        odpairs += 1
                        matrixSum += elem
                        currentMatrixSum += elem
                        if elem > 0.:
                            Pshort_EffCells += 1
                        if elem < 1. and elem > 0.:
                            smallDemandNum += 1
    begintime = int(periodList[0])
    assignPeriod = int(periodList[1]) - begintime
    smallDemandRatio = float(smallDemandNum) / float(Pshort_EffCells)

    if verbose:
        print('Number of zones:', zones)
        print('Number of origins:', origins)
        print('Number of destinations:', dest)
        print('begintime:', begintime)
        print('currentMatrixSum:', currentMatrixSum)
        print('Effective O-D Cells:', Pshort_EffCells)
        print('number of start Vertices:', net.getstartCounts())
        print('number of end Vertices):', net.getendCounts())
        print('smallDemandRatio):', smallDemandRatio)

    return matrixPshort, startVertices, endVertices, currentMatrixSum, begintime, assignPeriod, \
        Pshort_EffCells, matrixSum, smallDemandRatio

# estimate the travel times on the district connectors
# assumption: all vehilces can reach the access links within 20 min (1200
# sec) from the respective traffic zone


def getConnectionTravelTime(startVertices, endVertices):
    for vertex in startVertices:
        weightList = []
        for edge in vertex.getOutgoing():
            weightList.append(float(edge.ratio))
        if len(weightList) > 0:
            minWeight = min(weightList)

        for edge in vertex.getOutgoing():
            edge.freeflowtime = (float(edge.ratio) / minWeight) * 1200.
            edge.actualtime = edge.freeflowtime

    for vertex in endVertices:
        weightList = []
        for edge in vertex.getIncoming():
            weightList.append(float(edge.ratio))
        if len(weightList) > 0:
            minWeight = min(weightList)

        for edge in vertex.getIncoming():
            edge.freeflowtime = (float(edge.ratio) / minWeight) * 1200.
            edge.actualtime = edge.freeflowtime
