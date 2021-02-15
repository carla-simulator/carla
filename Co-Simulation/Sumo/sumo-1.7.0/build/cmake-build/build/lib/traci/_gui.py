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

# @file    _gui.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2011-03-09

from __future__ import absolute_import
from .domain import Domain
from . import constants as tc

_RETURN_VALUE_FUNC = {tc.VAR_HAS_VIEW: lambda result: bool(result.read("!i")[0])}


class GuiDomain(Domain):
    DEFAULT_VIEW = 'View #0'

    def __init__(self):
        Domain.__init__(self, "gui", tc.CMD_GET_GUI_VARIABLE, tc.CMD_SET_GUI_VARIABLE,
                        tc.CMD_SUBSCRIBE_GUI_VARIABLE, tc.RESPONSE_SUBSCRIBE_GUI_VARIABLE,
                        tc.CMD_SUBSCRIBE_GUI_CONTEXT, tc.RESPONSE_SUBSCRIBE_GUI_CONTEXT,
                        _RETURN_VALUE_FUNC)

    def getZoom(self, viewID=DEFAULT_VIEW):
        """getZoom(string): -> double

        Returns the current zoom factor.
        """
        return self._getUniversal(tc.VAR_VIEW_ZOOM, viewID)

    def getOffset(self, viewID=DEFAULT_VIEW):
        """getOffset(string): -> (double, double)

        Returns the x and y offset of the center of the current view.
        """
        return self._getUniversal(tc.VAR_VIEW_OFFSET, viewID)

    def getSchema(self, viewID=DEFAULT_VIEW):
        """getSchema(string): -> string

        Returns the name of the current coloring scheme.
        """
        return self._getUniversal(tc.VAR_VIEW_SCHEMA, viewID)

    def getBoundary(self, viewID=DEFAULT_VIEW):
        """getBoundary(string): -> ((double, double), (double, double))

        Returns the coordinates of the lower left and the upper right corner of the currently visible view.
        """
        return self._getUniversal(tc.VAR_VIEW_BOUNDARY, viewID)

    def setZoom(self, viewID, zoom):
        """setZoom(string, double) -> None

        Set the current zoom factor for the given view.
        """
        self._setCmd(tc.VAR_VIEW_ZOOM, viewID, "d", zoom)

    def setOffset(self, viewID, x, y):
        """setOffset(string, double, double) -> None

        Set the current offset for the given view.
        """
        self._setCmd(tc.VAR_VIEW_OFFSET, viewID, "o", [x, y])

    def setSchema(self, viewID, schemeName):
        """setSchema(string, string) -> None

        Set the current coloring scheme for the given view.
        """
        self._setCmd(tc.VAR_VIEW_SCHEMA, viewID, "s", schemeName)

    def setBoundary(self, viewID, xmin, ymin, xmax, ymax):
        """setBoundary(string, double, double, double, double) -> None
        Sets the boundary of the visible network. If the window has a different
        aspect ratio than the given boundary, the view is expanded along one
        axis to meet the window aspect ratio and contain the given boundary.
        """
        self._setCmd(tc.VAR_VIEW_BOUNDARY, viewID, "p", [[xmin, ymin], [xmax, ymax]])

    def screenshot(self, viewID, filename, width=-1, height=-1):
        """screenshot(string, string, int, int) -> None

        Save a screenshot for the given view to the given filename.
        The fileformat is guessed from the extension, the available
        formats differ from platform to platform but should at least
        include ps, svg and pdf, on linux probably gif, png and jpg as well.
        Width and height of the image can be given as optional parameters.
        """
        self._setCmd(tc.VAR_SCREENSHOT, viewID, "tsii", 3, filename, width, height)

    def trackVehicle(self, viewID, vehID):
        """trackVehicle(string, string) -> None

        Start visually tracking the given vehicle on the given view.
        """
        self._setCmd(tc.VAR_TRACK_VEHICLE, viewID, "s", vehID)

    def hasView(self, viewID=DEFAULT_VIEW):
        """hasView(string): -> bool

        Check whether the given view exists.
        """
        return self._getUniversal(tc.VAR_HAS_VIEW, viewID)

    def getTrackedVehicle(self, viewID=DEFAULT_VIEW):
        """getTrackedVehicle(string): -> string

        Returns the id of the currently tracked vehicle
        """
        return self._getUniversal(tc.VAR_TRACK_VEHICLE, viewID)

    def track(self, objID, viewID=DEFAULT_VIEW):
        """track(string, string) -> None
        Start visually tracking the given vehicle or person on the given view.
        """
        self._setCmd(tc.VAR_TRACK_VEHICLE, viewID, "s", objID)

    def isSelected(self, objID, objType="vehicle"):
        """isSelected(string, string) -> int
        Return 1 if the object of the given type and id is select, 0 otherwise
        """
        return self._getUniversal(tc.VAR_SELECT, objID, "s", objType)

    def toggleSelection(self, objID, objType="vehicle"):
        """toggleSelection(string, string) -> int
        Toggle selection status for the object of the given type and id
        """
        self._setCmd(tc.VAR_SELECT, objID, "s", objType)
