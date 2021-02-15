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

# @file    lane.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Karol Stosiek
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2011-11-28


import sumolib.geomhelper
from functools import reduce

# taken from sumo/src/utils/common/SUMOVehicleClass.cpp
SUMO_VEHICLE_CLASSES = (
    "public_emergency",  # deprecated
    "public_authority",  # deprecated
    "public_army",       # deprecated
    "public_transport",  # deprecated
    "transport",         # deprecated
    "lightrail",         # deprecated
    "cityrail",          # deprecated
    "rail_slow",         # deprecated
    "rail_fast",         # deprecated

    "private",
    "emergency",
    "authority",
    "army",
    "vip",
    "passenger",
    "hov",
    "taxi",
    "bus",
    "coach",
    "delivery",
    "truck",
    "trailer",
    "tram",
    "rail_urban",
    "rail",
    "rail_electric",
    "motorcycle",
    "moped",
    "bicycle",
    "pedestrian",
    "evehicle",
    "ship",
    "custom1",
    "custom2")


def get_allowed(allow, disallow):
    """ Normalize the given string attributes as a list of all allowed vClasses."""
    if allow is None and disallow is None:
        return SUMO_VEHICLE_CLASSES
    elif disallow is None:
        return allow.split()
    elif disallow == "all":
        return ()
    else:
        disallow = disallow.split()
        return tuple([c for c in SUMO_VEHICLE_CLASSES if c not in disallow])


def addJunctionPos(shape, fromPos, toPos):
    """Extends shape with the given positions in case they differ from the
    existing endpoints. assumes that shape and positions have the same dimensionality"""
    result = list(shape)
    if fromPos != shape[0]:
        result = [fromPos] + result
    if toPos != shape[-1]:
        result.append(toPos)
    return result


class Lane:

    """ Lanes from a sumo network """

    def __init__(self, edge, speed, length, width, allow, disallow):
        self._edge = edge
        self._speed = speed
        self._length = length
        self._width = width
        self._shape = None
        self._shape3D = None
        self._shapeWithJunctions = None
        self._shapeWithJunctions3D = None
        self._outgoing = []
        self._params = {}
        self._allowed = get_allowed(allow, disallow)
        self._neigh = None
        edge.addLane(self)

    def getSpeed(self):
        return self._speed

    def getLength(self):
        return self._length

    def getWidth(self):
        return self._width

    def setShape(self, shape):
        """Set the shape of the lane

        shape must be a list containing x,y,z coords as numbers
        to represent the shape of the lane
        """
        for pp in shape:
            if len(pp) != 3:
                raise ValueError('shape point must consist of x,y,z')

        self._shape3D = shape
        self._shape = [(x, y) for x, y, z in shape]

    def getShape(self, includeJunctions=False):
        """Returns the shape of the lane in 2d.

        This function returns the shape of the lane, as defined in the net.xml
        file. The returned shape is a list containing numerical
        2-tuples representing the x,y coordinates of the shape points.

        For includeJunctions=True the returned list will contain
        additionally the coords (x,y) of the fromNode of the
        corresponding edge as first element and the coords (x,y)
        of the toNode as last element.

        For internal lanes, includeJunctions is ignored and the unaltered
        shape of the lane is returned.
        """

        if includeJunctions and not self._edge.isSpecial():
            if self._shapeWithJunctions is None:
                self._shapeWithJunctions = addJunctionPos(self._shape,
                                                          self._edge.getFromNode().getCoord(),
                                                          self._edge.getToNode().getCoord())
            return self._shapeWithJunctions
        return self._shape

    def getShape3D(self, includeJunctions=False):
        """Returns the shape of the lane in 3d.

        This function returns the shape of the lane, as defined in the net.xml
        file. The returned shape is a list containing numerical
        3-tuples representing the x,y,z coordinates of the shape points
        where z defaults to zero.

        For includeJunction=True the returned list will contain
        additionally the coords (x,y,z) of the fromNode of the
        corresponding edge as first element and the coords (x,y,z)
        of the toNode as last element.

        For internal lanes, includeJunctions is ignored and the unaltered
        shape of the lane is returned.
        """

        if includeJunctions and not self._edge.isSpecial():
            if self._shapeWithJunctions3D is None:
                self._shapeWithJunctions3D = addJunctionPos(self._shape3D,
                                                            self._edge.getFromNode(
                                                            ).getCoord3D(),
                                                            self._edge.getToNode().getCoord3D())
            return self._shapeWithJunctions3D
        return self._shape3D

    def getBoundingBox(self, includeJunctions=True):
        s = self.getShape(includeJunctions)
        xmin = s[0][0]
        xmax = s[0][0]
        ymin = s[0][1]
        ymax = s[0][1]
        for p in s[1:]:
            xmin = min(xmin, p[0])
            xmax = max(xmax, p[0])
            ymin = min(ymin, p[1])
            ymax = max(ymax, p[1])
        return (xmin, ymin, xmax, ymax)

    def getClosestLanePosAndDist(self, point, perpendicular=False):
        return sumolib.geomhelper.polygonOffsetAndDistanceToPoint(point, self.getShape(), perpendicular)

    def getIndex(self):
        return self._edge._lanes.index(self)

    def getID(self):
        return "%s_%s" % (self._edge._id, self.getIndex())

    def getEdge(self):
        return self._edge

    def addOutgoing(self, conn):
        self._outgoing.append(conn)

    def getOutgoing(self):
        return self._outgoing

    def getIncoming(self, onlyDirect=False):
        """
        Returns all incoming lanes for this lane, i.e. lanes, which have a connection to this lane.
        If onlyDirect is True, then only incoming internal lanes are returned for a normal lane if they exist
        """
        candidates = reduce(lambda x, y: x + y, [cons for e, cons in self._edge.getIncoming().items()], [])
        lanes = [c.getFromLane() for c in candidates if self == c.getToLane()]
        if onlyDirect:
            hasInternal = False
            for l in lanes:
                if l.getID()[0] == ":":
                    hasInternal = True
                    break
            if hasInternal:
                return [l for l in lanes if l.getID()[0] == ":" and
                        l.getOutgoing()[0].getViaLaneID() == ""]
        return lanes

    def getConnection(self, toLane):
        """Returns the connection to the given target lane or None"""
        for conn in self._outgoing:
            if conn.getToLane() == toLane or conn.getViaLaneID() == toLane.getID():
                return conn
        return None

    def allows(self, vClass):
        """true if this lane allows the given vehicle class"""
        return vClass in self._allowed

    def setNeigh(self, neigh):
        self._neigh = neigh

    def getNeigh(self):
        return self._neigh

    def setParam(self, key, value):
        self._params[key] = value

    def getParam(self, key, default=None):
        return self._params.get(key, default)

    def getParams(self):
        return self._params
