# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    edge.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Karol Stosiek
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2011-11-28

import sumolib.geomhelper
from .connection import Connection
from .lane import addJunctionPos


class Edge:

    """ Edges from a sumo network """

    def __init__(self, id, fromN, toN, prio, function, name, edgeType=''):
        self._id = id
        self._from = fromN
        self._to = toN
        self._priority = prio
        if fromN:
            fromN.addOutgoing(self)
        if toN:
            toN.addIncoming(self)
        self._lanes = []
        self._speed = None
        self._length = None
        self._incoming = {}
        self._outgoing = {}
        self._crossingEdges = []
        self._shape = None
        self._shapeWithJunctions = None
        self._shape3D = None
        self._shapeWithJunctions3D = None
        self._rawShape = None
        self._rawShape3D = None
        self._function = function
        self._tls = None
        self._name = name
        self._type = edgeType
        self._params = {}
        self._bidi = None

    def getName(self):
        return self._name

    def isSpecial(self):
        """ Check if the edge has a special function.

        Returns False if edge's function is 'normal', else False, e.g. for
        internal edges or connector edges """

        return self._function != ""

    def getFunction(self):
        return self._function

    def getPriority(self):
        return self._priority

    def getType(self):
        return self._type

    def getTLS(self):
        return self._tls

    def getCrossingEdges(self):
        return self._crossingEdges

    def addLane(self, lane):
        self._lanes.append(lane)
        self._speed = lane.getSpeed()
        self._length = lane.getLength()

    def addOutgoing(self, conn):
        if conn._to not in self._outgoing:
            self._outgoing[conn._to] = []
        self._outgoing[conn._to].append(conn)

    def _addIncoming(self, conn):
        if conn._from not in self._incoming:
            self._incoming[conn._from] = []
        self._incoming[conn._from].append(conn)

    def _addCrossingEdge(self, edge):
        if edge not in self._crossingEdges:
            self._crossingEdges.append(edge)

    def setRawShape(self, shape):
        self._rawShape3D = shape

    def getID(self):
        return self._id

    def getIncoming(self):
        return self._incoming

    def getOutgoing(self):
        return self._outgoing

    def getAllowedOutgoing(self, vClass):
        if vClass is None or vClass == "ignoring":
            return self._outgoing
        else:
            result = {}
            for e, conns in self._outgoing.items():
                allowedConns = [c for c in conns if
                                c.getFromLane().allows(vClass) and
                                c.getToLane().allows(vClass)]
                if allowedConns:
                    result[e] = allowedConns
            return result

    def getConnections(self, toEdge):
        """Returns all connections to the given target edge"""
        return self._outgoing.get(toEdge, [])

    def getRawShape(self):
        """Return the shape that was used in netconvert for building this edge (2D)."""
        if self._shape is None:
            self.rebuildShape()
        return self._rawShape

    def getRawShape3D(self):
        """Return the shape that was used in netconvert for building this edge (3D)."""
        if self._shape is None:
            self.rebuildShape()
        return self._rawShape3D

    def getShape(self, includeJunctions=False):
        """Return the 2D shape that is the average of all lane shapes (segment-wise)"""
        if self._shape is None:
            self.rebuildShape()
        if includeJunctions:
            return self._shapeWithJunctions
        return self._shape

    def getShape3D(self, includeJunctions=False):
        if self._shape is None:
            self.rebuildShape()
        if includeJunctions:
            return self._shapeWithJunctions3D
        return self._shape3D

    def getBoundingBox(self, includeJunctions=True):
        xmin, ymin, xmax, ymax = sumolib.geomhelper.addToBoundingBox(self.getShape(includeJunctions))
        assert(xmin != xmax or ymin != ymax or self._function == "internal")
        return (xmin, ymin, xmax, ymax)

    def getClosestLanePosDist(self, point, perpendicular=False):
        minDist = 1e400
        minIdx = None
        minPos = None
        for i, l in enumerate(self._lanes):
            pos, dist = l.getClosestLanePosAndDist(point, perpendicular)
            if dist < minDist:
                minDist = dist
                minIdx = i
                minPos = pos
        return minIdx, minPos, minDist

    def getSpeed(self):
        return self._speed

    def getLaneNumber(self):
        return len(self._lanes)

    def getLane(self, idx):
        return self._lanes[idx]

    def getLanes(self):
        return self._lanes

    def rebuildShape(self):
        numLanes = len(self._lanes)
        if numLanes % 2 == 1:
            self._shape3D = self._lanes[int(numLanes / 2)].getShape3D()
        else:
            self._shape3D = []
            minLen = -1
            for l in self._lanes:
                if minLen == -1 or minLen > len(l.getShape()):
                    minLen = len(l._shape)
            for i in range(minLen):
                x = 0.
                y = 0.
                z = 0.
                for l in self._lanes:
                    x += l.getShape3D()[i][0]
                    y += l.getShape3D()[i][1]
                    z += l.getShape3D()[i][2]
                self._shape3D.append(
                    (x / float(numLanes), y / float(numLanes), z / float(numLanes)))

        if self._function in ["crossing", "walkingarea"]:
            self._shapeWithJunctions3D = self._shape3D
            self._rawShape3D = self._shape3D
        else:
            self._shapeWithJunctions3D = addJunctionPos(self._shape3D,
                                                        self._from.getCoord3D(), self._to.getCoord3D())
            if self._rawShape3D == []:
                self._rawShape3D = [self._from.getCoord3D(), self._to.getCoord3D()]

        # 2d - versions
        self._shape = [(x, y) for x, y, z in self._shape3D]
        self._shapeWithJunctions = [(x, y)
                                    for x, y, z in self._shapeWithJunctions3D]
        self._rawShape = [(x, y) for x, y, z in self._rawShape3D]

    def getLength(self):
        return self._lanes[0].getLength()

    def setTLS(self, tls):
        self._tls = tls

    def getFromNode(self):
        return self._from

    def getToNode(self):
        return self._to

    def getBidi(self):
        return self._bidi

    def is_fringe(self, connections=None):
        """true if this edge has no incoming or no outgoing connections (except turnarounds)
           If connections is given, only those connections are considered"""
        if connections is None:
            return self.is_fringe(self._incoming) or self.is_fringe(self._outgoing)
        else:
            cons = sum([c for c in connections.values()], [])
            return len([c for c in cons if c._direction != Connection.LINKDIR_TURN]) == 0

    def allows(self, vClass):
        """true if this edge has a lane which allows the given vehicle class"""
        for lane in self._lanes:
            if lane.allows(vClass):
                return True
        return False

    def setParam(self, key, value):
        self._params[key] = value

    def getParam(self, key, default=None):
        return self._params.get(key, default)

    def getParams(self):
        return self._params

    def __repr__(self):
        if self.getFunction() == '':
            return '<edge id="%s" from="%s" to="%s"/>' % (self._id, self._from.getID(), self._to.getID())
        else:
            return '<edge id="%s" function="%s"/>' % (self._id, self.getFunction())
