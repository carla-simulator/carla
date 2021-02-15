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

# @file    _junction.py
# @author  Michael Behrisch
# @author  Mario Krumnow
# @author  Jakob Erdmann
# @date    2011-03-17

from __future__ import absolute_import
from .domain import Domain
from . import constants as tc


class JunctionDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "junction", tc.CMD_GET_JUNCTION_VARIABLE, tc.CMD_SET_JUNCTION_VARIABLE,
                        tc.CMD_SUBSCRIBE_JUNCTION_VARIABLE, tc.RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE,
                        tc.CMD_SUBSCRIBE_JUNCTION_CONTEXT, tc.RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT)

    def getPosition(self, junctionID):
        """getPosition(string) -> (double, double)

        Returns the coordinates of the center of the junction.
        """
        return self._getUniversal(tc.VAR_POSITION, junctionID)

    def getShape(self, junctionID):
        """getShape(string) -> list((double, double))

        List of 2D positions (cartesian) describing the geometry.
        """
        return self._getUniversal(tc.VAR_SHAPE, junctionID)
