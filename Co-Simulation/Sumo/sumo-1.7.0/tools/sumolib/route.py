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

# @file    route.py
# @author  Michael Behrisch
# @date    2013-10-23

from __future__ import print_function
import os
import sys
SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
from sumolib.miscutils import euclidean  # noqa
from sumolib.geomhelper import polygonOffsetWithMinimumDistanceToPoint  # noqa

try:
    basestring
    # Allows isinstance(foo, basestring) to work in Python 3
except NameError:
    basestring = str


def getLength(net, edges):
    """
    Calculates the length of a route including internal edges.
    The input network has to contain internal edges (withInternal needs to be set when parsing).
    The list of edges can either contain edge objects or edge ids as strings.
    If there is no connection between two consecutive edges, length 0 is assumed (no error is thrown).
    If there are multiple connections of different length, the shortest is used.
    """
    if len(edges) == 0:
        return 0
    if isinstance(edges[0], basestring):
        edges = [net.getEdge(e) for e in edges]
    last = edges[0]
    length = last.getLength()
    for e in edges[1:]:
        if net.hasInternal:
            minInternalCost = 1e400
            for c in last.getConnections(e):
                if c.getViaLaneID() != "":
                    minInternalCost = min(minInternalCost, net.getLane(c.getViaLaneID()).getLength())
            if minInternalCost < 1e400:
                length += minInternalCost
        length += e.getLength()
        last = e
    return length


def _getMinPath(paths):
    minDist = 1e400
    minPath = None
    for path, (dist, _) in paths.items():
        if dist < minDist:
            minPath = path
            minDist = dist
    return minPath


def mapTrace(trace, net, delta, verbose=False, airDistFactor=2, fillGaps=0, gapPenalty=-1, debug=False):
    """
    matching a list of 2D positions to consecutive edges in a network.
    The positions are assumed to be dense (i.e. covering each edge of the route) and in the correct order.
    """
    result = ()
    paths = {}
    lastPos = None
    if verbose:
        print("mapping trace with %s points" % len(trace))
    for pos in trace:
        newPaths = {}
        candidates = net.getNeighboringEdges(pos[0], pos[1], delta, not net.hasInternal)
        if debug:
            print("\n\npos:%s, %s" % (pos[0], pos[1]))
            print("candidates:%s\n" % candidates)
        if len(candidates) == 0 and verbose:
            print("Found no candidate edges for %s,%s" % pos)

        for edge, d in candidates:
            base = polygonOffsetWithMinimumDistanceToPoint(pos, edge.getShape())
            if paths:
                advance = euclidean(lastPos, pos)  # should become a vector
                minDist = 1e400
                minPath = None
                for path, (dist, lastBase) in paths.items():
                    if debug:
                        print("*** extending path %s by edge '%s'" % ([e.getID() for e in path], edge.getID()))
                        print("              lastBase: %s, base: %s, advance: %s, old dist: %s, minDist: %s" %
                              (lastBase, base, advance, dist, minDist))
                    if dist < minDist:
                        if edge == path[-1]:
                            baseDiff = lastBase + advance - base
                            extension = ()
                            if debug:
                                print("---------- same edge")
                        else:
                            maxGap = min(airDistFactor * advance + edge.getLength() + path[-1].getLength(), fillGaps)
                            extension, cost = net.getShortestPath(path[-1], edge, maxGap, includeFromToCost=False)
                            if extension is None:
                                airLineDist = euclidean(
                                    path[-1].getToNode().getCoord(),
                                    edge.getFromNode().getCoord())
                                if gapPenalty < 0:
                                    gapPenalty = airDistFactor * advance
                                baseDiff = abs(lastBase + advance -
                                               path[-1].getLength() - base - airLineDist) + gapPenalty
                                extension = (edge,)
                            else:
                                baseDiff = lastBase + advance - (cost - edge.getLength()) - base
                                extension = extension[1:]
                            if debug:
                                print("---------- extension path: %s, cost: %s, baseDiff: %s" %
                                      (extension, cost, baseDiff))
                        dist += baseDiff * baseDiff
                        if dist < minDist:
                            minDist = dist
                            minPath = path + extension
                        if debug:
                            print("*** new dist: %s baseDiff: %s minDist: %s" % (dist, baseDiff, minDist))
                if minPath:
                    newPaths[minPath] = (minDist, base)
            else:
                newPaths[(edge,)] = (d * d, base)
        if not newPaths:
            if paths:
                result += _getMinPath(paths)
        paths = newPaths
        lastPos = pos
    if paths:
        if debug:
            print("**************** result:")
            for i in result + _getMinPath(paths):
                print("path:%s" % i.getID())
        return result + _getMinPath(paths)
    return result
