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

# @file    _routeprobe.py
# @author  Michael Behrisch
# @author  Lena Kalleske
# @date    2008-10-09

from __future__ import absolute_import
from . import constants as tc
from .domain import Domain


class RouteProbeDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "routeprobe", tc.CMD_GET_ROUTEPROBE_VARIABLE, tc.CMD_SET_ROUTEPROBE_VARIABLE,
                        tc.CMD_SUBSCRIBE_ROUTEPROBE_VARIABLE, tc.RESPONSE_SUBSCRIBE_ROUTEPROBE_VARIABLE,
                        tc.CMD_SUBSCRIBE_ROUTEPROBE_CONTEXT, tc.RESPONSE_SUBSCRIBE_ROUTEPROBE_CONTEXT)

    def getEdgeID(self, probeID):
        """getEdgeID(string) -> string
        Returns the edge id of the given route probe
        """
        return self._getUniversal(tc.VAR_ROAD_ID, probeID)

    def sampleLastRouteID(self, probeID):
        """sampleLastRouteID(string) -> string
        Returns a random routeID from the distribution collected by this route
        proble in the previous collectin interval
        """
        return self._getUniversal(tc.VAR_SAMPLE_LAST, probeID)

    def sampleCurrentRouteID(self, probeID):
        """sampleCurrentRouteID(string) -> string
        Returns a random routeID from the distribution collected by this route
        proble in the current collectin interval
        """
        return self._getUniversal(tc.VAR_SAMPLE_CURRENT, probeID)
