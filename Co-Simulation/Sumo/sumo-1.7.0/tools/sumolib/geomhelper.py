# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    geomhelper.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2013-02-25

from __future__ import absolute_import
import math

INVALID_DISTANCE = -1

# back-ported from python 3 for backward compatibility
# https://www.python.org/dev/peps/pep-0485/#proposed-implementation


def isclose(a, b, rel_tol=1e-09, abs_tol=0.0):
    return abs(a-b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)


def distance(p1, p2):
    dx = p1[0] - p2[0]
    dy = p1[1] - p2[1]
    return math.sqrt(dx * dx + dy * dy)


def polyLength(polygon):
    return sum([distance(a, b) for a, b in zip(polygon[:-1], polygon[1:])])


def addToBoundingBox(coordList, bbox=None):
    if bbox is None:
        minX = 1e400
        minY = 1e400
        maxX = -1e400
        maxY = -1e400
    else:
        minX, minY, maxX, maxY = bbox
    for x, y in coordList:
        minX = min(x, minX)
        minY = min(y, minY)
        maxX = max(x, maxX)
        maxY = max(y, maxY)
    return minX, minY, maxX, maxY


def lineOffsetWithMinimumDistanceToPoint(point, line_start, line_end, perpendicular=False):
    """Return the offset from line (line_start, line_end) where the distance to
    point is minimal"""
    p = point
    p1 = line_start
    p2 = line_end
    d = distance(p1, p2)
    u = ((p[0] - p1[0]) * (p2[0] - p1[0])) + ((p[1] - p1[1]) * (p2[1] - p1[1]))
    if d == 0. or u < 0. or u > d * d:
        if perpendicular:
            return INVALID_DISTANCE
        if u < 0.:
            return 0.
        return d
    return u / d


def polygonOffsetAndDistanceToPoint(point, polygon, perpendicular=False):
    """Return the offset and the distance from the polygon start where the distance to the point is minimal"""
    p = point
    s = polygon
    seen = 0
    minDist = 1e400
    minOffset = INVALID_DISTANCE
    for i in range(len(s) - 1):
        pos = lineOffsetWithMinimumDistanceToPoint(
            p, s[i], s[i + 1], perpendicular)
        dist = minDist if pos == INVALID_DISTANCE else distance(
            p, positionAtOffset(s[i], s[i + 1], pos))
        if dist < minDist:
            minDist = dist
            minOffset = pos + seen
        if perpendicular and i != 0 and pos == INVALID_DISTANCE:
            # even if perpendicular is set we still need to check the distance
            # to the inner points
            cornerDist = distance(p, s[i])
            if cornerDist < minDist:
                pos1 = lineOffsetWithMinimumDistanceToPoint(
                    p, s[i - 1], s[i], False)
                pos2 = lineOffsetWithMinimumDistanceToPoint(
                    p, s[i], s[i + 1], False)
                if pos1 == distance(s[i - 1], s[i]) and pos2 == 0.:
                    minOffset = seen
                    minDist = cornerDist
        seen += distance(s[i], s[i + 1])
    return minOffset, minDist


def polygonOffsetWithMinimumDistanceToPoint(point, polygon, perpendicular=False):
    """Return the offset from the polygon start where the distance to the point is minimal"""
    return polygonOffsetAndDistanceToPoint(point, polygon, perpendicular)[0]


def distancePointToLine(point, line_start, line_end, perpendicular=False):
    """Return the minimum distance between point and the line (line_start, line_end)"""
    p1 = line_start
    p2 = line_end
    offset = lineOffsetWithMinimumDistanceToPoint(
        point, line_start, line_end, perpendicular)
    if offset == INVALID_DISTANCE:
        return INVALID_DISTANCE
    if offset == 0:
        return distance(point, p1)
    u = offset / distance(line_start, line_end)
    intersection = (p1[0] + u * (p2[0] - p1[0]), p1[1] + u * (p2[1] - p1[1]))
    return distance(point, intersection)


def distancePointToPolygon(point, polygon, perpendicular=False):
    """Return the minimum distance between point and polygon"""
    p = point
    s = polygon
    minDist = None
    for i in range(0, len(s) - 1):
        dist = distancePointToLine(p, s[i], s[i + 1], perpendicular)
        if dist == INVALID_DISTANCE and perpendicular and i != 0:
            # distance to inner corner
            dist = distance(point, s[i])
        if dist != INVALID_DISTANCE:
            if minDist is None or dist < minDist:
                minDist = dist
    if minDist is not None:
        return minDist
    else:
        return INVALID_DISTANCE


def positionAtOffset(p1, p2, offset):
    if isclose(offset, 0.):  # for pathological cases with dist == 0 and offset == 0
        return p1

    dist = distance(p1, p2)

    if isclose(dist, offset):
        return p2

    if offset > dist:
        return None

    return (p1[0] + (p2[0] - p1[0]) * (offset / dist), p1[1] + (p2[1] - p1[1]) * (offset / dist))


def positionAtShapeOffset(shape, offset):
    seenLength = 0
    curr = shape[0]
    for next in shape[1:]:
        nextLength = distance(curr, next)
        if seenLength + nextLength > offset:
            return positionAtOffset(curr, next, offset - seenLength)
        seenLength += nextLength
        curr = next
    return shape[-1]


def angle2D(p1, p2):
    theta1 = math.atan2(p1[1], p1[0])
    theta2 = math.atan2(p2[1], p2[0])
    dtheta = theta2 - theta1
    while dtheta > math.pi:
        dtheta -= 2.0 * math.pi
    while dtheta < -math.pi:
        dtheta += 2.0 * math.pi
    return dtheta


def naviDegree(rad):
    return normalizeAngle(math.degrees(math.pi / 2. - rad), 0, 360, 360)


def fromNaviDegree(degrees):
    return math.pi / 2. - math.radians(degrees)


def normalizeAngle(a, lower, upper, circle):
    while a < lower:
        a = a + circle
    while a > upper:
        a = a - circle
    return a


def minAngleDegreeDiff(d1, d2):
    return min(normalizeAngle(d1 - d2, 0, 360, 360),
               normalizeAngle(d2 - d1, 0, 360, 360))


def isWithin(pos, shape):
    angle = 0.
    for i in range(0, len(shape) - 1):
        p1 = ((shape[i][0] - pos[0]), (shape[i][1] - pos[1]))
        p2 = ((shape[i + 1][0] - pos[0]), (shape[i + 1][1] - pos[1]))
        angle = angle + angle2D(p1, p2)
    i = len(shape) - 1
    p1 = ((shape[i][0] - pos[0]), (shape[i][1] - pos[1]))
    p2 = ((shape[0][0] - pos[0]), (shape[0][1] - pos[1]))
    angle = angle + angle2D(p1, p2)
    return math.fabs(angle) >= math.pi


def sideOffset(fromPos, toPos, amount):
    scale = amount / distance(fromPos, toPos)
    return (scale * (fromPos[1] - toPos[1]),
            scale * (toPos[0] - fromPos[0]))


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1])


def add(a, b):
    return (a[0] + b[0], a[1] + b[1])


def mul(a, x):
    return (a[0] * x, a[1] * x)


def dotProduct(a, b):
    return a[0] * b[0] + a[1] * b[1]


def orthoIntersection(a, b):
    c = add(a, b)
    quot = dotProduct(c, a)
    if quot != 0:
        return mul(mul(c, dotProduct(a, a)), 1 / quot)
    else:
        return None


def length(a):
    return math.sqrt(dotProduct(a, a))


def norm(a):
    return mul(a, 1 / length(a))


def narrow(fromPos, pos, toPos, amount):
    """detect narrow turns which cannot be shifted regularly"""
    a = sub(toPos, pos)
    b = sub(pos, fromPos)
    c = add(a, b)
    dPac = dotProduct(a, c)
    if dPac == 0:
        return True
    x = dotProduct(a, a) * length(c) / dPac
    return x < amount


def move2side(shape, amount):
    shape = [s for i, s in enumerate(shape) if i == 0 or shape[i-1] != s]
    if len(shape) < 2:
        return shape
    if polyLength(shape) == 0:
        return shape
    result = []
    for i, pos in enumerate(shape):
        if i == 0:
            fromPos = pos
            toPos = shape[i + 1]
            if fromPos != toPos:
                result.append(sub(fromPos, sideOffset(fromPos, toPos, amount)))
        elif i == len(shape) - 1:
            fromPos = shape[i - 1]
            toPos = pos
            if fromPos != toPos:
                result.append(sub(toPos, sideOffset(fromPos, toPos, amount)))
        else:
            fromPos = shape[i - 1]
            toPos = shape[i + 1]
            # check for narrow turns
            if narrow(fromPos, pos, toPos, amount):
                # print("narrow at i=%s pos=%s" % (i, pos))
                pass
            else:
                a = sideOffset(fromPos, pos, -amount)
                b = sideOffset(pos, toPos, -amount)
                c = orthoIntersection(a, b)
                if orthoIntersection is not None:
                    pos2 = add(pos, c)
                else:
                    extend = norm(sub(pos, fromPos))
                    pos2 = add(pos, mul(extend, amount))
                result.append(pos2)
    # print("move2side", amount)
    # print(shape)
    # print(result)
    # print()
    return result
