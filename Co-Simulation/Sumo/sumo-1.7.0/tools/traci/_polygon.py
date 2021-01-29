# -*- coding: utf-8 -*-
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

# @file    _polygon.py
# @author  Michael Behrisch
# @date    2011-03-16

from __future__ import absolute_import
from .domain import Domain
from . import constants as tc

_RETURN_VALUE_FUNC = {tc.VAR_FILL: lambda result: bool(result.read("!i")[0])}


class PolygonDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "polygon", tc.CMD_GET_POLYGON_VARIABLE, tc.CMD_SET_POLYGON_VARIABLE,
                        tc.CMD_SUBSCRIBE_POLYGON_VARIABLE, tc.RESPONSE_SUBSCRIBE_POLYGON_VARIABLE,
                        tc.CMD_SUBSCRIBE_POLYGON_CONTEXT, tc.RESPONSE_SUBSCRIBE_POLYGON_CONTEXT,
                        _RETURN_VALUE_FUNC)

    def getType(self, polygonID):
        """getType(string) -> string

        Returns the (abstract) type of the polygon.
        """
        return self._getUniversal(tc.VAR_TYPE, polygonID)

    def getShape(self, polygonID):
        """getShape(string) -> list((double, double))

        Returns the shape (list of 2D-positions) of this polygon.
        """
        return self._getUniversal(tc.VAR_SHAPE, polygonID)

    def getColor(self, polygonID):
        """getColor(string) -> (integer, integer, integer, integer)

        Returns the rgba color of this polygon.
        """
        return self._getUniversal(tc.VAR_COLOR, polygonID)

    def getFilled(self, polygonID):
        """getFilled(string) -> bool
        Returns whether the polygon is filled
        """
        return self._getUniversal(tc.VAR_FILL, polygonID)

    def getLineWidth(self, polygonID):
        """getLineWidth(string) -> double
        Returns drawing width of unfilled polygon
        """
        return self._getUniversal(tc.VAR_WIDTH, polygonID)

    def setType(self, polygonID, polygonType):
        """setType(string, string) -> None

        Sets the (abstract) type of the polygon.
        """
        self._setCmd(tc.VAR_TYPE, polygonID, "s", polygonType)

    def setShape(self, polygonID, shape):
        """setShape(string, list((double, double))) -> None

        Sets the shape (list of 2D-positions) of this polygon.
        """
        self._setCmd(tc.VAR_SHAPE, polygonID, "p", shape)

    def setColor(self, polygonID, color):
        """setColor(string, (integer, integer, integer, integer)) -> None

        Sets the rgba color of this polygon, i.e. (255,0,0) for the color red.
        The fourth component (alpha) is optional.
        """
        self._setCmd(tc.VAR_COLOR, polygonID, "c", color)

    def setFilled(self, polygonID, filled):
        """setFilled(string, bool) -> None
        Sets the filled status of the polygon
        """
        self._setCmd(tc.VAR_FILL, polygonID, "i", filled)

    def setLineWidth(self, polygonID, lineWidth):
        """setFilled(string, double) -> None
        Sets the line width for drawing unfilled polygon
        """
        self._setCmd(tc.VAR_WIDTH, polygonID, "d", lineWidth)

    def add(self, polygonID, shape, color, fill=False, polygonType="", layer=0, lineWidth=1):
        """add(string,  list((double, double)), (integer, integer, integer, integer), bool, string, integer, double) -> None
        Adds a new polygon.
        """
        self._setCmd(tc.ADD, polygonID, "tscBipd", 6, polygonType, color, fill, layer, shape, lineWidth)

    def addDynamics(self, polygonID, trackedObjectID="", timeSpan=(), alphaSpan=(), looped=False, rotate=True):
        """ addDynamics(string, string, list(float), list(float), bool) -> void
            polygonID - ID of the polygon, upon which the specified dynamics shall act
            trackedObjectID - ID of a SUMO traffic object, which shall be tracked by the polygon
            timeSpan - list of time points for timing the animation keyframes (must start with element zero)
                       If it has length zero, no animation is taken into account.
            alphaSpan - list of alpha values to be attained at keyframes intermediate values are
                        obtained by linear interpolation. Must have length equal to timeSpan, or zero
                        if no alpha animation is desired.
            looped - Whether the animation should restart when the last keyframe is reached. In that case
                     the animation jumps to the first keyframe as soon as the last is reached.
                     If looped==false, the controlled polygon is removed as soon as the timeSpan elapses.
            rotate - Whether, the polygon should be rotated with the tracked object (only applies when such is given)
                     The center of rotation is the object's position.
        """
        self._setCmd(tc.VAR_ADD_DYNAMICS, polygonID, "tsffBB", 5, trackedObjectID, timeSpan, alphaSpan, looped, rotate)

    def remove(self, polygonID, layer=0):
        """remove(string, integer) -> None
        Removes a polygon with the given ID
        """
        self._setCmd(tc.REMOVE, polygonID, "i", layer)
