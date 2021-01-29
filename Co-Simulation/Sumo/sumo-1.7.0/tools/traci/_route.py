# -*- coding: utf-8 -*-
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

# @file    _route.py
# @author  Michael Behrisch
# @author  Lena Kalleske
# @date    2008-10-09

from __future__ import absolute_import
from . import constants as tc
from .domain import Domain


class RouteDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "route", tc.CMD_GET_ROUTE_VARIABLE, tc.CMD_SET_ROUTE_VARIABLE,
                        tc.CMD_SUBSCRIBE_ROUTE_VARIABLE, tc.RESPONSE_SUBSCRIBE_ROUTE_VARIABLE,
                        tc.CMD_SUBSCRIBE_ROUTE_CONTEXT, tc.RESPONSE_SUBSCRIBE_ROUTE_CONTEXT)

    def getEdges(self, routeID):
        """getEdges(string) -> list(string)

        Returns a list of all edges in the route.
        """
        return self._getUniversal(tc.VAR_EDGES, routeID)

    def add(self, routeID, edges):
        """add(string, list(string)) -> None

        Adds a new route with the given id consisting of the given list of edge IDs.
        """
        self._setCmd(tc.ADD, routeID, "l", edges)
