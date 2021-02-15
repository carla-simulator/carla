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

# @file    node.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Karol Stosiek
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2011-11-28


class Node:

    """ Nodes from a sumo network """

    def __init__(self, id, type, coord, incLanes, intLanes=None):
        self._id = id
        self._type = type
        self._coord = coord
        self._incoming = []
        self._outgoing = []
        self._foes = {}
        self._prohibits = {}
        self._incLanes = incLanes
        self._intLanes = intLanes
        self._shape3D = None
        self._shape = None
        self._params = {}

    def getID(self):
        return self._id

    def setShape(self, shape):
        """Set the shape of the node.

        Shape must be a list containing x,y,z coords as numbers
        to represent the shape of the node.
        """
        for pp in shape:
            if len(pp) != 3:
                raise ValueError('shape point must consist of x,y,z')
        self._shape3D = shape
        self._shape = [(x, y) for x, y, z in shape]

    def getShape(self):
        """Returns the shape of the node in 2d.

        This function returns the shape of the node, as defined in the net.xml
        file. The returned shape is a list containing numerical
        2-tuples representing the x,y coordinates of the shape points.

        If no shape is defined in the xml, an empty list will be returned.
        """

        return self._shape

    def getShape3D(self):
        """Returns the shape of the node in 3d.

        This function returns the shape of the node, as defined in the net.xml
        file. The returned shape is a list containing numerical
        3-tuples representing the x,y,z coordinates of the shape points.

        If no shape is defined in the xml, an empty list will be returned.
        """

        return self._shape3D

    def addOutgoing(self, edge):
        self._outgoing.append(edge)

    def getOutgoing(self):
        return self._outgoing

    def addIncoming(self, edge):
        self._incoming.append(edge)

    def getIncoming(self):
        return self._incoming

    def getInternal(self):
        return self._intLanes

    def setFoes(self, index, foes, prohibits):
        self._foes[index] = foes
        self._prohibits[index] = prohibits

    def areFoes(self, link1, link2):
        return self._foes[link1][len(self._foes[link1]) - link2 - 1] == '1'

    def getLinkIndex(self, conn):
        ret = 0
        for lane_id in self._incLanes:
            lastUnderscore = lane_id.rfind("_")
            if lastUnderscore > 0:
                edge_id = lane_id[:lastUnderscore]
                index = lane_id[lastUnderscore+1:]
                edge = [e for e in self._incoming if e.getID() == edge_id][0]
                for candidate_conn in edge.getLane(int(index)).getOutgoing():
                    fromFunction = candidate_conn.getFromLane().getEdge().getFunction()
                    toFunction = candidate_conn.getToLane().getEdge().getFunction()
                    if toFunction == "walkingarea" or (fromFunction == "walkingarea" and not toFunction == "crossing"):
                        continue
                    if candidate_conn == conn:
                        return ret
                    ret += 1
        return -1

    def forbids(self, possProhibitor, possProhibited):
        possProhibitorIndex = self.getLinkIndex(possProhibitor)
        possProhibitedIndex = self.getLinkIndex(possProhibited)
        if possProhibitorIndex < 0 or possProhibitedIndex < 0:
            return False
        ps = self._prohibits[possProhibitedIndex]
        return ps[-(possProhibitorIndex - 1)] == '1'

    def getCoord(self):
        return tuple(self._coord[:2])

    def getCoord3D(self):
        return self._coord

    def getType(self):
        return self._type

    def getConnections(self, source=None, target=None):
        if source:
            incoming = [source]
        else:
            incoming = list(self._incoming)
        conns = []
        for e in incoming:
            if (hasattr(e, "getLanes")):
                lanes = e.getLanes()
            else:
                # assuming source is a lane
                lanes = [e]
            for l in lanes:
                all_outgoing = l.getOutgoing()
                outgoing = []
                if target:
                    if hasattr(target, "getLanes"):
                        for o in all_outgoing:
                            if o.getTo() == target:
                                outgoing.append(o)
                    else:
                        # assuming target is a lane
                        for o in all_outgoing:
                            if o.getToLane() == target:
                                outgoing.append(o)
                else:
                    outgoing = all_outgoing
                conns.extend(outgoing)
        return conns

    def setParam(self, key, value):
        self._params[key] = value

    def getParam(self, key, default=None):
        return self._params.get(key, default)

    def getParams(self):
        return self._params

    def getNeighboringNodes(self, outgoingNodes=True, incomingNodes=True):
        neighboring = []
        if incomingNodes:
            edges = self._incoming
            for e in edges:
                if not (e.getFromNode() in neighboring) and not(e.getFromNode().getID() == self.getID()):
                    neighboring.append(e.getFromNode())
        if outgoingNodes:
            edges = self._outgoing
            for e in edges:
                if not (e.getToNode() in neighboring)and not(e.getToNode().getID() == self.getID()):
                    neighboring.append(e.getToNode())
        return neighboring
