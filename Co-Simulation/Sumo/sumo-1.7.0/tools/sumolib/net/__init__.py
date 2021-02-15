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

# @file    __init__.py
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Karol Stosiek
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @author  Robert Hilbrich
# @date    2008-03-27

"""
This file contains a content handler for parsing sumo network xml files.
It uses other classes from this module to represent the road network.
"""

from __future__ import print_function
from __future__ import absolute_import
import sys
import math
import heapq
import gzip
from xml.sax import handler, parse
from copy import copy
from collections import defaultdict

import sumolib
from . import lane, edge, node, connection, roundabout
from .connection import Connection


class TLS:

    """Traffic Light Signal for a sumo network"""

    def __init__(self, id):
        self._id = id
        self._connections = []
        self._maxConnectionNo = -1
        self._programs = {}

    def addConnection(self, inLane, outLane, linkNo):
        self._connections.append([inLane, outLane, linkNo])
        if linkNo > self._maxConnectionNo:
            self._maxConnectionNo = linkNo

    def getConnections(self):
        return self._connections

    def getID(self):
        return self._id

    def getLinks(self):
        links = {}
        for the_connection in self._connections:
            if the_connection[2] not in links:
                links[the_connection[2]] = []
            links[the_connection[2]].append(the_connection)
        return links

    def getEdges(self):
        edges = set()
        for c in self._connections:
            edges.add(c[0].getEdge())
        return edges

    def addProgram(self, program):
        self._programs[program._id] = program

    def removePrograms(self):
        self._programs.clear()

    def toXML(self):
        ret = ""
        for p in self._programs:
            ret = ret + self._programs[p].toXML(self._id)
        return ret

    def getPrograms(self):
        return self._programs


class Phase:

    def __init__(self, duration, state, minDur=-1, maxDur=-1, next=None, name=""):
        self.duration = duration
        self.state = state
        self.minDur = minDur  # minimum duration (only for actuated tls)
        self.maxDur = maxDur  # maximum duration (only for actuated tls)
        self.next = [] if next is None else next
        self.name = name

    def __repr__(self):
        name = "" if self.name == "" else ", name='%s'" % self.name
        next = "" if len(self.next) == 0 else ", next='%s'" % self.next
        return ("Phase(duration=%s, state='%s', minDur=%s, maxDur=%s%s%s" %
                (self.duration, self.state, self.minDur, self.maxDur, name, next))


class TLSProgram:

    def __init__(self, id, offset, type):
        self._id = id
        self._type = type
        self._offset = offset
        self._phases = []
        self._params = {}

    def addPhase(self, state, duration, minDur=-1, maxDur=-1, next=None, name=""):
        self._phases.append(Phase(duration, state, minDur, maxDur, next, name))

    def toXML(self, tlsID):
        ret = '  <tlLogic id="%s" type="%s" programID="%s" offset="%s">\n' % (
            tlsID, self._type, self._id, self._offset)
        for p in self._phases:
            minDur = '' if p.minDur < 0 else ' minDur="%s"' % p.minDur
            maxDur = '' if p.maxDur < 0 else ' maxDur="%s"' % p.maxDur
            name = '' if p.name == '' else ' name="%s"' % p.name
            next = '' if len(p.next) == 0 else ' next="%s"' % ' '.join(map(str, p.next))
            ret += '    <phase duration="%s" state="%s"%s%s%s%s/>\n' % (
                p.duration, p.state, minDur, maxDur, name, next)
        for k, v in self._params.items():
            ret += '    <param key="%s" value="%s"/>\n' % (k, v)
        ret += '  </tlLogic>\n'
        return ret

    def getPhases(self):
        return self._phases

    def getType(self):
        return self._type

    def setParam(self, key, value):
        self._params[key] = value

    def getParam(self, key, default=None):
        return self._params.get(key, default)

    def getParams(self):
        return self._params


class Net:

    """The whole sumo network."""

    def __init__(self):
        self._location = {}
        self._id2node = {}
        self._id2edge = {}
        self._crossings_and_walkingAreas = set()
        self._macroConnectors = set()
        self._id2tls = {}
        self._nodes = []
        self._edges = []
        self._tlss = []
        self._ranges = [[10000, -10000], [10000, -10000]]
        self._roundabouts = []
        self._rtreeEdges = None
        self._rtreeLanes = None
        self._allLanes = []
        self._origIdx = None
        self._proj = None
        self.hasWarnedAboutMissingRTree = False
        self.hasInternal = False

    def setLocation(self, netOffset, convBoundary, origBoundary, projParameter):
        self._location["netOffset"] = netOffset
        self._location["convBoundary"] = convBoundary
        self._location["origBoundary"] = origBoundary
        self._location["projParameter"] = projParameter

    def addNode(self, id, type=None, coord=None, incLanes=None, intLanes=None):
        if id is None:
            return None
        if id not in self._id2node:
            n = node.Node(id, type, coord, incLanes, intLanes)
            self._nodes.append(n)
            self._id2node[id] = n
        self.setAdditionalNodeInfo(
            self._id2node[id], type, coord, incLanes, intLanes)
        return self._id2node[id]

    def setAdditionalNodeInfo(self, node, type, coord, incLanes, intLanes=None):
        if coord is not None and node._coord is None:
            node._coord = coord
            self._ranges[0][0] = min(self._ranges[0][0], coord[0])
            self._ranges[0][1] = max(self._ranges[0][1], coord[0])
            self._ranges[1][0] = min(self._ranges[1][0], coord[1])
            self._ranges[1][1] = max(self._ranges[1][1], coord[1])
        if incLanes is not None and node._incLanes is None:
            node._incLanes = incLanes
        if intLanes is not None and node._intLanes is None:
            node._intLanes = intLanes
        if type is not None and node._type is None:
            node._type = type

    def addEdge(self, id, fromID, toID, prio, function, name, edgeType=''):
        if id not in self._id2edge:
            fromN = self.addNode(fromID)
            toN = self.addNode(toID)
            e = edge.Edge(id, fromN, toN, prio, function, name, edgeType)
            self._edges.append(e)
            self._id2edge[id] = e
            if function:
                self.hasInternal = True
        return self._id2edge[id]

    def addLane(self, edge, speed, length, width, allow=None, disallow=None):
        return lane.Lane(edge, speed, length, width, allow, disallow)

    def addRoundabout(self, nodes, edges=None):
        r = roundabout.Roundabout(nodes, edges)
        self._roundabouts.append(r)
        return r

    def addConnection(self, fromEdge, toEdge, fromlane, tolane, direction, tls, tllink, state, viaLaneID=None):
        conn = connection.Connection(
            fromEdge, toEdge, fromlane, tolane, direction, tls, tllink, state, viaLaneID)
        fromEdge.addOutgoing(conn)
        fromlane.addOutgoing(conn)
        toEdge._addIncoming(conn)
        if viaLaneID:
            try:
                # internal lanes are only available when building with option withInternal=True
                viaLane = self.getLane(viaLaneID)
                viaEdge = viaLane.getEdge()
                viaEdge._addIncoming(connection.Connection(
                    fromEdge, viaEdge, fromlane, viaLane, direction, tls,
                    tllink, state, ''))
            except Exception:
                pass
        return conn

    def getEdges(self, withInternal=True):
        if not withInternal:
            return [e for e in self._edges if e.getFunction() == '']
        else:
            return self._edges

    def getRoundabouts(self):
        return self._roundabouts

    def hasEdge(self, id):
        return id in self._id2edge

    def getEdge(self, id):
        return self._id2edge[id]

    def getLane(self, laneID):
        edge_id, lane_index = laneID.rsplit("_", 1)
        return self.getEdge(edge_id).getLane(int(lane_index))

    def _initRTree(self, shapeList, includeJunctions=True):
        import rtree  # noqa
        result = rtree.index.Index()
        result.interleaved = True
        for ri, shape in enumerate(shapeList):
            result.add(ri, shape.getBoundingBox(includeJunctions))
        return result

    # Please be aware that the resulting list of edges is NOT sorted
    def getNeighboringEdges(self, x, y, r=0.1, includeJunctions=True,
                            allowFallback=True):
        edges = []
        try:
            if self._rtreeEdges is None:
                self._rtreeEdges = self._initRTree(self._edges, includeJunctions)
            for i in self._rtreeEdges.intersection((x - r, y - r, x + r, y + r)):
                e = self._edges[i]
                d = sumolib.geomhelper.distancePointToPolygon(
                    (x, y), e.getShape(includeJunctions))
                if d < r:
                    edges.append((e, d))
        except ImportError:
            if allowFallback:
                if not self.hasWarnedAboutMissingRTree:
                    sys.stderr.write("Warning: Module 'rtree' not available. Using brute-force fallback\n")
                    self.hasWarnedAboutMissingRTree = True
            else:
                sys.stderr.write("Error: Module 'rtree' not available.\n")
                sys.exit(1)

            for the_edge in self._edges:
                d = sumolib.geomhelper.distancePointToPolygon(
                    (x, y), the_edge.getShape(includeJunctions))
                if d < r:
                    edges.append((the_edge, d))
        return edges

    def getNeighboringLanes(self, x, y, r=0.1, includeJunctions=True,
                            allowFallback=True):
        lanes = []
        try:
            if self._rtreeLanes is None:
                for the_edge in self._edges:
                    self._allLanes += the_edge.getLanes()
                self._rtreeLanes = self._initRTree(self._allLanes, includeJunctions)
            for i in self._rtreeLanes.intersection((x - r, y - r, x + r, y + r)):
                lane = self._allLanes[i]
                d = sumolib.geomhelper.distancePointToPolygon(
                    (x, y), lane.getShape(includeJunctions))
                if d < r:
                    lanes.append((lane, d))
        except ImportError:
            if allowFallback:
                if not self.hasWarnedAboutMissingRTree:
                    sys.stderr.write("Warning: Module 'rtree' not available. Using brute-force fallback\n")
                    self.hasWarnedAboutMissingRTree = True
            else:
                sys.stderr.write("Error: Module 'rtree' not available.\n")
                sys.exit(1)

            for the_edge in self._edges:
                for lane in the_edge.getLanes():
                    d = sumolib.geomhelper.distancePointToPolygon(
                        (x, y), lane.getShape(includeJunctions))
                    if d < r:
                        lanes.append((lane, d))
        return lanes

    def hasNode(self, id):
        return id in self._id2node

    def getNode(self, id):
        return self._id2node[id]

    def getNodes(self):
        return self._nodes

    def getTLS(self, tlid):
        return self._id2tls[tlid]

    def getTLSSecure(self, tlid):
        if tlid in self._id2tls:
            tls = self._id2tls[tlid]
        else:
            tls = TLS(tlid)
            self._id2tls[tlid] = tls
            self._tlss.append(tls)
        return tls

    def getTrafficLights(self):
        return self._tlss

    def addTLS(self, tlid, inLane, outLane, linkNo):
        tls = self.getTLSSecure(tlid)
        tls.addConnection(inLane, outLane, linkNo)
        return tls

    def addTLSProgram(self, tlid, programID, offset, type, removeOthers):
        tls = self.getTLSSecure(tlid)
        program = TLSProgram(programID, offset, type)
        if removeOthers:
            tls.removePrograms()
        tls.addProgram(program)
        return program

    def setFoes(self, junctionID, index, foes, prohibits):
        self._id2node[junctionID].setFoes(index, foes, prohibits)

    def forbids(self, possProhibitor, possProhibited):
        return possProhibitor.getFrom().getToNode().forbids(possProhibitor, possProhibited)

    def getDownstreamEdges(self, edge, distance, stopOnTLS, stopOnTurnaround):
        """return a list of lists of the form
           [[firstEdge, pos, [edge_0, edge_1, ..., edge_k], aborted], ...]
           where
             firstEdge: is the downstream edge furthest away from the intersection,
             [edge_0, ..., edge_k]: is the list of edges from the intersection downstream to firstEdge
             pos: is the position on firstEdge with distance to the end of the input edge
             aborted: a flag indicating whether the downstream
                 search stopped at a TLS or a node without incoming edges before reaching the distance threshold
        """
        ret = []
        seen = set()
        toProc = []
        toProc.append([edge, 0, []])
        while not len(toProc) == 0:
            ie = toProc.pop()
            if ie[0] in seen:
                continue
            seen.add(ie[0])
            if ie[1] + ie[0].getLength() >= distance:
                ret.append(
                    [ie[0], ie[0].getLength() + ie[1] - distance, ie[2], False])
                continue
            if len(ie[0]._incoming) == 0:
                ret.append([ie[0], ie[0].getLength() + ie[1], ie[2], True])
                continue
            mn = []
            stop = False
            for ci in ie[0]._incoming:
                if ci not in seen:
                    prev = copy(ie[2])
                    if stopOnTLS and ci._tls and ci != edge and not stop:
                        ret.append([ie[0], ie[1], prev, True])
                        stop = True
                    elif (stopOnTurnaround and ie[0]._incoming[ci][0].getDirection() == Connection.LINKDIR_TURN and
                          not stop):
                        ret.append([ie[0], ie[1], prev, True])
                        stop = True
                    else:
                        prev.append(ie[0])
                        mn.append([ci, ie[0].getLength() + ie[1], prev])
            if not stop:
                toProc.extend(mn)
        return ret

    def getEdgesByOrigID(self, origID):
        if self._origIdx is None:
            self._origIdx = defaultdict(set)
            for the_edge in self._edges:
                for the_lane in the_edge.getLanes():
                    for oID in the_lane.getParam("origId", "").split():
                        self._origIdx[oID].add(the_edge)
        return self._origIdx[origID]

    def getBBoxXY(self):
        """
        Get the bounding box (bottom left and top right coordinates) for a net;
        Coordinates are in X and Y (not Lat and Lon)

        :return [(bottom_left_X, bottom_left_Y), (top_right_X, top_right_Y)]
        """
        return [(self._ranges[0][0], self._ranges[1][0]),
                (self._ranges[0][1], self._ranges[1][1])]

    # the diagonal of the bounding box of all nodes
    def getBBoxDiameter(self):
        return math.sqrt(
            (self._ranges[0][0] - self._ranges[0][1]) ** 2 +
            (self._ranges[1][0] - self._ranges[1][1]) ** 2)

    def getGeoProj(self):
        if self._proj is None:
            import pyproj
            try:
                self._proj = pyproj.Proj(projparams=self._location["projParameter"])
            except RuntimeError:
                if hasattr(pyproj.datadir, 'set_data_dir'):
                    pyproj.datadir.set_data_dir('/usr/share/proj')
                    self._proj = pyproj.Proj(projparams=self._location["projParameter"])
                raise
        return self._proj

    def getLocationOffset(self):
        """ offset to be added after converting from geo-coordinates to UTM"""
        return list(map(float, self._location["netOffset"].split(",")))

    def getBoundary(self):
        """ return xmin,ymin,xmax,ymax network coordinates"""
        return list(map(float, self._location["convBoundary"].split(",")))

    def convertLonLat2XY(self, lon, lat, rawUTM=False):
        x, y = self.getGeoProj()(lon, lat)
        if rawUTM:
            return x, y
        else:
            x_off, y_off = self.getLocationOffset()
            return x + x_off, y + y_off

    def convertXY2LonLat(self, x, y, rawUTM=False):
        if not rawUTM:
            x_off, y_off = self.getLocationOffset()
            x -= x_off
            y -= y_off
        return self.getGeoProj()(x, y, inverse=True)

    def move(self, dx, dy, dz=0):
        for n in self._nodes:
            n._coord = (n._coord[0] + dx, n._coord[1] + dy, n._coord[2] + dz)
        for e in self._edges:
            for l in e._lanes:
                l._shape = [(p[0] + dx, p[1] + dy, p[2] + dz)
                            for p in l.getShape3D()]
            e.rebuildShape()

    def getShortestPath(self, fromEdge, toEdge, maxCost=1e400, vClass=None, reversalPenalty=0, includeFromToCost=True):
        """
        Finds the shortest path from fromEdge to toEdge respecting vClass, using Dijkstra's algorithm.
        It returns a pair of a tuple of edges and the cost. If no path is found the first element is None.
        The cost for the returned path is equal to the sum of all edge lengths in the path,
        including the internal connectors, if they are present in the network.
        The path itself does not include internal edges except for the case
        when the start or end edge are internal edges.
        The search may be limited using the given threshold.
        """
        if self.hasInternal:
            appendix = ()
            appendixCost = 0.
            while toEdge.getFunction() == "internal":
                appendix = (toEdge,) + appendix
                appendixCost += toEdge.getLength()
                toEdge = list(toEdge.getIncoming().keys())[0]
        q = [(fromEdge.getLength() if includeFromToCost else 0, fromEdge.getID(), fromEdge, ())]
        seen = set()
        dist = {fromEdge: fromEdge.getLength()}
        while q:
            cost, _, e1, path = heapq.heappop(q)
            if e1 in seen:
                continue
            seen.add(e1)
            path += (e1,)
            if e1 == toEdge:
                if self.hasInternal:
                    return path + appendix, cost + appendixCost
                if includeFromToCost:
                    return path, cost
                return path, cost - toEdge.getLength()
            if cost > maxCost:
                return None, cost

            for e2, conn in e1.getAllowedOutgoing(vClass).items():
                # print(cost, e1.getID(), e2.getID(), e2 in seen)
                if e2 not in seen:
                    newCost = cost + e2.getLength()
                    if e2 == e1.getBidi():
                        newCost += reversalPenalty
                    if self.hasInternal:
                        minInternalCost = 1e400
                        for c in conn:
                            if c.getViaLaneID() != "":
                                minInternalCost = min(minInternalCost, self.getLane(c.getViaLaneID()).getLength())
                        if minInternalCost < 1e400:
                            newCost += minInternalCost
                    if e2 not in dist or newCost < dist[e2]:
                        dist[e2] = newCost
                        heapq.heappush(q, (newCost, e2.getID(), e2, path))
        return None, 1e400


class NetReader(handler.ContentHandler):

    """Reads a network, storing the edge geometries, lane numbers and max. speeds"""

    def __init__(self, **others):
        self._net = others.get('net', Net())
        self._currentEdge = None
        self._currentNode = None
        self._currentConnection = None
        self._currentLane = None
        self._crossingID2edgeIDs = {}
        self._withPhases = others.get('withPrograms', False)
        self._latestProgram = others.get('withLatestPrograms', False)
        if self._latestProgram:
            self._withPhases = True
        self._withConnections = others.get('withConnections', True)
        self._withFoes = others.get('withFoes', True)
        self._withPedestrianConnections = others.get('withPedestrianConnections', False)
        self._withMacroConnectors = others.get('withMacroConnectors', False)
        self._withInternal = others.get('withInternal', self._withPedestrianConnections)
        if self._withPedestrianConnections and not self._withInternal:
            sys.stderr.write("Warning: Option withPedestrianConnections requires withInternal\n")
            self._withInternal = True
        self._bidiEdgeIDs = {}

    def startElement(self, name, attrs):
        if name == 'location':
            self._net.setLocation(attrs["netOffset"], attrs["convBoundary"], attrs[
                                  "origBoundary"], attrs["projParameter"])
        if name == 'edge':
            function = attrs.get('function', '')
            if (function == ''
                    or (self._withInternal and function in ['internal', 'crossing', 'walkingarea'])
                    or (self._withMacroConnectors and function == 'connector')):
                prio = -1
                if 'priority' in attrs:
                    prio = int(attrs['priority'])

                # get the  ids
                edgeID = attrs['id']
                fromNodeID = attrs.get('from', None)
                toNodeID = attrs.get('to', None)

                # for internal junctions use the junction's id for from and to node
                if function == 'internal':
                    fromNodeID = toNodeID = edgeID[1:edgeID.rfind('_')]

                # remember edges crossed by pedestrians to link them later to the crossing objects
                if function == 'crossing':
                    self._crossingID2edgeIDs[edgeID] = attrs.get('crossingEdges').split(' ')

                self._currentEdge = self._net.addEdge(edgeID, fromNodeID, toNodeID, prio, function,
                                                      attrs.get('name', ''), attrs.get('type', ''))

                self._currentEdge.setRawShape(convertShape(attrs.get('shape', '')))

                bidi = attrs.get('bidi', '')
                if bidi:
                    self._bidiEdgeIDs[edgeID] = bidi
            else:
                if function in ['crossing', 'walkingarea']:
                    self._net._crossings_and_walkingAreas.add(attrs['id'])
                elif function == 'connector':
                    self._net._macroConnectors.add(attrs['id'])
                self._currentEdge = None
        if name == 'lane' and self._currentEdge is not None:
            self._currentLane = self._net.addLane(
                self._currentEdge,
                float(attrs['speed']),
                float(attrs['length']),
                float(attrs.get('width', 3.2)),
                attrs.get('allow'),
                attrs.get('disallow'))
            self._currentLane.setShape(convertShape(attrs.get('shape', '')))
        if name == 'neigh' and self._currentLane is not None:
            self._currentLane.setNeigh(attrs['lane'])
        if name == 'junction':
            if attrs['id'][0] != ':':
                intLanes = None
                if self._withInternal:
                    intLanes = attrs["intLanes"].split(" ")
                self._currentNode = self._net.addNode(attrs['id'], attrs['type'],
                                                      tuple(
                                                          map(float, [attrs['x'], attrs['y'],
                                                                      attrs['z'] if 'z' in attrs else '0'])),
                                                      attrs['incLanes'].split(" "), intLanes)
                self._currentNode.setShape(
                    convertShape(attrs.get('shape', '')))
        if name == 'succ' and self._withConnections:  # deprecated
            if attrs['edge'][0] != ':':
                self._currentEdge = self._net.getEdge(attrs['edge'])
                self._currentLane = attrs['lane']
                self._currentLane = int(
                    self._currentLane[self._currentLane.rfind('_') + 1:])
            else:
                self._currentEdge = None
        if name == 'succlane' and self._withConnections:  # deprecated
            lid = attrs['lane']
            if lid[0] != ':' and lid != "SUMO_NO_DESTINATION" and self._currentEdge:
                connected = self._net.getEdge(lid[:lid.rfind('_')])
                tolane = int(lid[lid.rfind('_') + 1:])
                if 'tl' in attrs and attrs['tl'] != "":
                    tl = attrs['tl']
                    tllink = int(attrs['linkIdx'])
                    tlid = attrs['tl']
                    toEdge = self._net.getEdge(lid[:lid.rfind('_')])
                    tolane2 = toEdge._lanes[tolane]
                    tls = self._net.addTLS(
                        tlid, self._currentEdge._lanes[self._currentLane], tolane2, tllink)
                    self._currentEdge.setTLS(tls)
                else:
                    tl = ""
                    tllink = -1
                toEdge = self._net.getEdge(lid[:lid.rfind('_')])
                tolane = toEdge._lanes[tolane]
                viaLaneID = attrs['via']
                self._net.addConnection(self._currentEdge, connected, self._currentEdge._lanes[
                                        self._currentLane], tolane,
                                        attrs['dir'], tl, tllink, attrs['state'], viaLaneID)
        if name == 'connection' and self._withConnections and (attrs['from'][0] != ":" or self._withInternal):
            fromEdgeID = attrs['from']
            toEdgeID = attrs['to']
            if ((self._withPedestrianConnections or not (fromEdgeID in self._net._crossings_and_walkingAreas or
                                                         toEdgeID in self._net._crossings_and_walkingAreas))
                and (self._withMacroConnectors or not (fromEdgeID in self._net._macroConnectors or toEdgeID in
                                                       self._net._macroConnectors))):
                fromEdge = self._net.getEdge(fromEdgeID)
                toEdge = self._net.getEdge(toEdgeID)
                fromLane = fromEdge.getLane(int(attrs['fromLane']))
                toLane = toEdge.getLane(int(attrs['toLane']))
                if 'tl' in attrs and attrs['tl'] != "":
                    tl = attrs['tl']
                    tllink = int(attrs['linkIndex'])
                    tls = self._net.addTLS(tl, fromLane, toLane, tllink)
                    fromEdge.setTLS(tls)
                else:
                    tl = ""
                    tllink = -1
                try:
                    viaLaneID = attrs['via']
                except KeyError:
                    viaLaneID = ''

                self._currentConnection = self._net.addConnection(
                    fromEdge, toEdge, fromLane, toLane, attrs['dir'], tl,
                    tllink, attrs['state'], viaLaneID)

        # 'row-logic' is deprecated!!!
        if self._withFoes and name == 'ROWLogic':
            self._currentNode = attrs['id']
        if name == 'logicitem' and self._withFoes:  # deprecated
            self._net.setFoes(
                self._currentNode, int(attrs['request']), attrs["foes"], attrs["response"])
        if name == 'request' and self._withFoes:
            self._currentNode.setFoes(
                int(attrs['index']), attrs["foes"], attrs["response"])
        # tl-logic is deprecated!!! NOTE: nevertheless, this is still used by
        # netconvert... (Leo)
        if self._withPhases and name == 'tlLogic':
            self._currentProgram = self._net.addTLSProgram(
                attrs['id'], attrs['programID'], float(attrs['offset']), attrs['type'], self._latestProgram)
        if self._withPhases and name == 'phase':
            self._currentProgram.addPhase(
                attrs['state'], int(attrs['duration']),
                int(attrs['minDur']) if 'minDur' in attrs else -1,
                int(attrs['maxDur']) if 'maxDur' in attrs else -1,
                list(map(int, attrs['next'].split())) if 'next' in attrs else [],
                attrs['name'] if 'name' in attrs else ""
            )
        if name == 'roundabout':
            self._net.addRoundabout(
                attrs['nodes'].split(), attrs['edges'].split())
        if name == 'param':
            if self._currentLane is not None:
                self._currentLane.setParam(attrs['key'], attrs['value'])
            elif self._currentEdge is not None:
                self._currentEdge.setParam(attrs['key'], attrs['value'])
            elif self._currentNode is not None:
                self._currentNode.setParam(attrs['key'], attrs['value'])
            elif self._currentConnection is not None:
                self._currentConnection.setParam(attrs['key'], attrs['value'])
            elif self._withPhases and self._currentProgram is not None:
                self._currentProgram.setParam(attrs['key'], attrs['value'])

    def endElement(self, name):
        if name == 'lane':
            self._currentLane = None
        if name == 'edge':
            self._currentEdge = None
        if name == 'junction':
            self._currentNode = None
        if name == 'connection':
            self._currentConnection = None
        # 'row-logic' is deprecated!!!
        if name == 'ROWLogic' or name == 'row-logic':
            self._haveROWLogic = False
        # tl-logic is deprecated!!!
        if self._withPhases and (name == 'tlLogic' or name == 'tl-logic'):
            self._currentProgram = None
        if name == 'net':
            for edgeID, bidiID in self._bidiEdgeIDs.items():
                self._net.getEdge(edgeID)._bidi = self._net.getEdge(bidiID)

    def endDocument(self):
        # set crossed edges of pedestrian crossings
        for crossingID, crossedEdgeIDs in self._crossingID2edgeIDs.items():
            pedCrossing = self._net.getEdge(crossingID)
            for crossedEdgeID in crossedEdgeIDs:
                pedCrossing._addCrossingEdge(self._net.getEdge(crossedEdgeID))

    def getNet(self):
        return self._net


def convertShape(shapeString):
    """ Convert xml shape string into float tuples.

    This method converts the 2d or 3d shape string from SUMO's xml file
    into a list containing 3d float-tuples. Non existant z coordinates default
    to zero. If shapeString is empty, an empty list will be returned.
    """

    cshape = []
    for pointString in shapeString.split():
        p = [float(e) for e in pointString.split(",")]
        if len(p) == 2:
            cshape.append((p[0], p[1], 0.))
        elif len(p) == 3:
            cshape.append(tuple(p))
        else:
            raise ValueError(
                'Invalid shape point "%s", should be either 2d or 3d' % pointString)
    return cshape


def readNet(filename, **others):
    """ load a .net.xml file
    The following named options are supported:

        'net' : initialize data structurs with an existing net object (default Net())
        'withPrograms' : import all traffic light programs (default False)
        'withLatestPrograms' : import only the last program for each traffic light.
                               This is the program that would be active in sumo by default.
                               (default False)
        'withConnections' : import all connections (default True)
        'withFoes' : import right-of-way information (default True)
        'withInternal' : import internal edges and lanes (default False)
        'withPedestrianConnections' : import connections between sidewalks, crossings (default False)
    """
    netreader = NetReader(**others)
    try:
        parse(gzip.open(filename), netreader)
    except IOError:
        parse(filename, netreader)
    return netreader.getNet()
