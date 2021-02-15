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

# @file    elements.py
# @author  Yun-Pang Floetteroed
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @date    2007-10-25

"""
This script is to define the classes and functions for
- reading network geometric,
- calculating link characteristics, such as capacity, travel time and link cost function,
- recording vehicular and path information, and
- conducting statistic tests.
"""
from __future__ import absolute_import
from __future__ import print_function

import sys
import math
from tables import crCurveTable, laneTypeTable
import sumolib

# This class is used for finding the k shortest paths.


class Predecessor:

    def __init__(self, edge, pred, distance):
        self.edge = edge
        self.pred = pred
        self.distance = distance

# This class is used to build the nodes in the investigated network and
# includes the update-function for searching the k shortest paths.


class Vertex(sumolib.net.node.Node):

    """
    This class is to store node attributes and the respective incoming/outgoing links.
    """

    def __init__(self, id, type=None, coord=None, incLanes=None):
        sumolib.net.node.Node.__init__(self, id, type, coord, incLanes)
        self.preds = []
        self.wasUpdated = False

    def __repr__(self):
        return self._id

    def _addNewPredecessor(self, edge, updatePred, newPreds):
        for pred in newPreds:
            if pred.pred == updatePred:
                return
        pred = updatePred
        while pred.edge is not None:
            if pred.edge == edge:
                return
            pred = pred.pred
        newPreds.append(Predecessor(edge, updatePred,
                                    updatePred.distance + edge.actualtime))

    def update(self, KPaths, edge):
        updatePreds = edge._from.preds
        if len(self.preds) == KPaths\
           and updatePreds[0].distance + edge.actualtime >= self.preds[KPaths - 1].distance:
            return False
        newPreds = []
        updateIndex = 0
        predIndex = 0
        while len(newPreds) < KPaths\
            and (updateIndex < len(updatePreds) or
                 predIndex < len(self.preds)):
            if predIndex == len(self.preds):
                self._addNewPredecessor(
                    edge, updatePreds[updateIndex], newPreds)
                updateIndex += 1
            elif updateIndex == len(updatePreds):
                newPreds.append(self.preds[predIndex])
                predIndex += 1
            elif updatePreds[updateIndex].distance + edge.actualtime < self.preds[predIndex].distance:
                self._addNewPredecessor(
                    edge, updatePreds[updateIndex], newPreds)
                updateIndex += 1
            else:
                newPreds.append(self.preds[predIndex])
                predIndex += 1
        if predIndex == len(newPreds):
            return False
        self.preds = newPreds
        returnVal = not self.wasUpdated
        self.wasUpdated = True
        return returnVal

# This class is used to store link information and estimate
# as well as flow and capacity for the flow computation and some parameters
# read from the net.


class Edge(sumolib.net.edge.Edge):

    """
    This class is to record link attributes
    """

    def __init__(self, label, source, target, prio, function, name):
        sumolib.net.edge.Edge.__init__(
            self, label, source, target, prio, function, name)
        self.capacity = sys.maxsize
        # parameter for estimating capacities according to signal timing plans
        self.junction = None
        self.junctiontype = None
        self.rightturn = None
        self.straight = None
        self.leftturn = None
        self.uturn = None
        self.leftlink = []
        self.straightlink = []
        self.rightlink = []
        self.conflictlink = {}
#        self.againstlinkexist = None
        self.flow = 0.0
        self.helpflow = 0.0
        self.freeflowtime = 0.0
        self.queuetime = 0.0
        self.estcapacity = 0.0
        self.CRcurve = None
        self.actualtime = 0.0
        self.ratio = 0.0
        self.connection = 0
        self.edgetype = None
        # parameter in the Lohse traffic assignment
        self.helpacttime = 0.
        # parameter in the Lohse traffic assignment
        self.fTT = 0.
        # parameter in the Lohse traffic assignment
        self.TT = 0.
        # parameter in the Lohse traffic assignment
        self.delta = 0.
        # parameter in the Lohse traffic assignment
        self.helpacttimeEx = 0.
        # parameter in the matrix estimation
        self.detected = False
        self.detectorNum = 0.
        self.detecteddata = {}
        self.detectedlanes = 0.
        self.penalty = 0.
        self.capLeft = 0.
        self.capRight = 0.
        self.capThrough = 0.

    def addLane(self, lane):
        sumolib.net.edge.Edge.addLane(self, lane)
        if self._from._id == self._to._id:
            self.freeflowtime = 0.0
        else:
            self.freeflowtime = self._length / self._speed
            self.actualtime = self.freeflowtime
            self.helpacttime = self.freeflowtime

    def __repr__(self):
        cap = str(self.capacity)
        if self.capacity == sys.maxsize or self.connection != 0:
            cap = "inf"
        return "%s_%s_%s_%s<%s|%s|%s|%s|%s|%s|%s|%s|%s>" % (self._function, self._id, self._from, self._to,
                                                            self.junctiontype, self._speed,
                                                            self.flow, self._length, self._lanes,
                                                            self.CRcurve, self.estcapacity, cap, self.ratio)

    def getConflictLink(self):
        """
        method to get the conflict links for each link, when the respective left-turn behavior exists.
        """
        if self._function == 'real' and len(self.leftlink) > 0:
            for leftEdge in self.leftlink:
                affectedTurning = None
                for edge in leftEdge.source.inEdges:
                    if edge.source == self.target:
                        affectedTurning = edge
                        affectedTurning.freeflowtime = 6.
                        affectedTurning.actualtime = affectedTurning.freeflowtime
                        affectedTurning.helpacttime = affectedTurning.freeflowtime

                for edge in leftEdge.source.inEdges:
                    for upstreamlink in edge.source.inEdges:
                        if leftEdge in upstreamlink.rightlink and len(upstreamlink.straightlink) > 0:
                            if upstreamlink not in self.conflictlink:
                                self.conflictlink[upstreamlink] = []
                            self.conflictlink[upstreamlink].append(
                                affectedTurning)

    def getFreeFlowTravelTime(self):
        return self.freeflowtime

    def addDetectedData(self, detecteddataObj):
        self.detecteddata[detecteddataObj.label] = detecteddataObj

    def getCapacity(self):
        """
        method to read the link capacity and the cr-curve type from the table.py
        the applied CR-curve database is retrived from VISUM-Validate-network und VISUM-Koeln-network
        """
        typeList = laneTypeTable[min(len(self._lanes), 4)]
        for laneType in typeList:
            if laneType[0] >= self._speed:
                break

        self.estcapacity = len(self._lanes) * laneType[1]
        self.CRcurve = laneType[2]

    def getAdjustedCapacity(self, net):
        """
        method to adjust the link capacity based on the given signal timing plans
        """
        straightGreen = 0.
        rightGreen = 0.
        leftGreen = 0.
        greentime = 0.
        straightSymbol = -1
        rightSymbol = -1
        leftSymbol = -1
        cyclelength = 0.
        count = 0
        if self.junctiontype == 'signalized':
            junction = net._junctions[self.junction]
            if self.rightturn is not None and self.rightturn != 'O' and self.rightturn != 'o':
                rightSymbol = int(self.rightturn)
            if self.leftturn is not None and self.leftturn != 'O' and self.leftturn != 'o':
                leftSymbol = int(self.leftturn)
            if self.straight is not None and self.straight != 'O' and self.straight != 'o':
                straightSymbol = int(self.straight)
            for phase in junction.phases[:]:
                count += 1
                cyclelength += phase.duration
                if straightSymbol != -1 and phase.green[straightSymbol] == "1":
                    straightGreen += phase.duration
                if rightSymbol != -1 and phase.green[rightSymbol] == "1":
                    rightGreen += phase.duration
                if leftSymbol != -1 and phase.green[leftSymbol] == "1":
                    leftGreen += phase.duration
            if self.straight is not None:
                self.estcapacity = (
                    straightGreen * (3600. / cyclelength)) / 1.5 * len(self._lanes)
            else:
                greentime = max(rightGreen, leftGreen)
                self.estcapacity = (
                    greentime * (3600. / cyclelength)) / 1.5 * len(self._lanes)

    def getActualTravelTime(self, options, lohse):
        """
        method to calculate/update link travel time
        """
        foutcheck = open('queue_info.txt', 'a')

        if self.CRcurve in crCurveTable:
            curve = crCurveTable[self.CRcurve]
            if self.flow == 0.0 or self.connection > 0:
                self.actualtime = self.freeflowtime
            elif self.estcapacity != 0. and self.connection == 0:
                self.actualtime = self.freeflowtime * \
                    (1 + (curve[0] * (self.flow /
                                      (self.estcapacity * curve[2]))**curve[1]))

            if ((self.flow > self.estcapacity or self.flow == self.estcapacity) and
                    self.flow > 0. and self.connection == 0):
                self.queuetime = self.queuetime + options.lamda * \
                    (self.actualtime - self.freeflowtime * (1 + curve[0]))
                if self.queuetime < 1.:
                    self.queuetime = 0.
                else:
                    foutcheck.write(
                        'edge.label= %s: queuing time= %s.\n' % (self._id, self.queuetime))
                    foutcheck.write('travel time at capacity: %s; actual travel time: %s.\n' % (
                        self.freeflowtime * (1 + curve[0]), self.actualtime))
            else:
                self.queuetime = 0.

            self.actualtime += self.queuetime

            if lohse:
                self.getLohseParUpdate(options)
            else:
                self.helpacttime = self.actualtime

            self.penalty = 0.
            if len(self.conflictlink) > 0:
                for edge in self.conflictlink:
                    conflictEdge = edge
                flowCapRatio = conflictEdge.flow / conflictEdge.estcapacity

                weightFactor = 1.0
                if self.numberlane == 2.:
                    weightFactor = 0.85
                elif self.numberlane == 3.:
                    weightFactor = 0.75
                elif self.numberlane > 3.:
                    weightFactor = 0.6
                if options.dijkstra != 'extend':
                    for edge in self.conflictlink:
                        penalty = 0.
                        if edge.estcapacity > 0. and edge.flow / edge.estcapacity > 0.12:
                            penalty = weightFactor * \
                                (math.exp(self.flow / self.estcapacity) - 1. +
                                 math.exp(edge.flow / edge.estcapacity) - 1.)
                            for affectedTurning in self.conflictlink[edge]:
                                affectedTurning.actualtime = self.actualtime * \
                                    penalty
                                if lohse:
                                    affectedTurning.helpacttime = self.helpacttime * \
                                        penalty
                                else:
                                    affectedTurning.helpacttime = affectedTurning.actualtime
                else:
                    for edge in self.conflictlink:
                        if edge.estcapacity > 0. and edge.flow / edge.estcapacity >= flowCapRatio:
                            conflictEdge = edge
                            flowCapRatio = edge.flow / edge.estcapacity

                    if conflictEdge.estcapacity > 0. and conflictEdge.flow / conflictEdge.estcapacity > 0.12:
                        self.penalty = weightFactor * \
                            (math.exp(self.flow / self.estcapacity) - 1. +
                             math.exp(conflictEdge.flow / conflictEdge.estcapacity) - 1.)
                    if lohse:
                        self.penalty *= self.helpacttime
                    else:
                        self.penalty *= self.actualtime
        foutcheck.close()

    def cleanFlow(self):
        """ method to reset link flows """
        self.flow = 0.
        self.helpflow = 0.

    def getLohseParUpdate(self, options):
        """
        method to update the parameter used in the Lohse-assignment (learning method - Lernverfahren)
        """
        if self.helpacttime > 0.:
            self.TT = abs(self.actualtime - self.helpacttime) / \
                self.helpacttime
            self.fTT = options.v1 / \
                (1 + math.exp(options.v2 - options.v3 * self.TT))
            self.delta = options.under + \
                (options.upper - options.under) / ((1 + self.TT)**self.fTT)
            self.helpacttimeEx = self.helpacttime
            self.helpacttime = self.helpacttime + self.delta * \
                (self.actualtime - self.helpacttime)

    def stopCheck(self, options):
        """
        method to check if the convergence reaches in the Lohse-assignment
        """
        stop = False
        criteria = 0.
        criteria = options.cvg1 * \
            self.helpacttimeEx**(options.cvg2 / options.cvg3)

        if abs(self.actualtime - self.helpacttimeEx) <= criteria:
            stop = True
        return stop


class Vehicle:

    """
    This class is to store vehicle information, such as departure time, route and travel time.
    """

    def __init__(self, label):
        self.label = label
        self.method = None
        self.depart = 0.
        self.arrival = 0.
        self.speed = 0.
        self.route = []
        self.traveltime = 0.
        self.travellength = 0.
        self.departdelay = 0.
        self.waittime = 0.
        self.rank = 0.

    def __repr__(self):
        return "%s_%s_%s_%s_%s_%s<%s>" % (self.label, self.depart, self.arrival, self.speed,
                                          self.traveltime, self.travellength, self.route)


pathNum = 0


class Path:

    """
    This class is to store path information which is mainly for the C-logit and the Lohse models.
    """

    def __init__(self, source, target, edges):
        self.source = source
        self.target = target
        global pathNum
        self.label = "%s" % pathNum
        pathNum += 1
        self.edges = edges
        self.length = 0.0
        self.freepathtime = 0.0
        self.actpathtime = 0.0
        self.pathflow = 0.0
        self.helpflow = 0.0
        self.choiceprob = 0.0
        self.sumOverlap = 0.0
        self.utility = 0.0
        # parameter used in the Lohse traffic assignment
        self.usedcounts = 1
        # parameter used in the Lohse traffic assignment
        self.pathhelpacttime = 0.
        # record if this path is the currrent shortest one.
        self.currentshortest = True

    def __repr__(self):
        return "%s_%s_%s<%s|%s|%s|%s>" % (self.label, self.source, self.target, self.freepathtime,
                                          self.pathflow, self.actpathtime, self.edges)

    def getPathLength(self):
        for edge in self.edges:
            self.length += edge._length

    def updateSumOverlap(self, newpath, gamma):
        overlapLength = 0.
        for edge in self.edges:
            if edge in newpath.edges:
                overlapLength += edge._length
        overlapLength = overlapLength / 1000.
        lengthOne = self.length / 1000.
        lengthTwo = newpath.length / 1000.
        self.sumOverlap += math.pow(overlapLength /
                                    (math.pow(lengthOne, 0.5) * math.pow(lengthTwo, 0.5)), gamma)

    def getPathTimeUpdate(self):
        """
        used to update the path travel time in the c-logit and the Lohse traffic assignments
        """
        self.actpathtime = 0.
        self.pathhelpacttime = 0.
        for edge in self.edges:
            self.actpathtime += edge.actualtime
            self.pathhelpacttime += edge.helpacttime

        self.pathhelpacttime = self.pathhelpacttime / 3600.
        self.actpathtime = self.actpathtime / 3600.


class TLJunction:

    def __init__(self):
        self.label = None
        self.phaseNum = 0
        self.phases = []

    def __repr__(self):
        return "%s_%s<%s>" % (self.label, self.phaseNum, self.phases)


class Signalphase:

    def __init__(self, duration, state=None, phase=None, brake=None, yellow=None):
        self.label = None
        self.state = state
        self.duration = duration
        self.green = ''
        self.brake = ''
        self.yellow = ''

        if phase and brake and yellow:
            self.green = phase[::-1]
            self.brake = brake[::-1]
            self.yellow = yellow[::-1]
        elif self.state:
            for elem in self.state:
                if elem == 'G':
                    self.green += '1'
                    self.brake += '0'
                    self.yellow += '0'
                elif elem == 'y':
                    self.green += '0'
                    self.brake += '0'
                    self.yellow += '1'
                elif elem == 'r':
                    self.green += '0'
                    self.brake += '1'
                    self.yellow += '0'
        else:
            print('no timing plans exist!')

    def __repr__(self):
        return "%s_%s<%s|%s|%s>" % (self.label, self.duration, self.green, self.brake, self.yellow)
