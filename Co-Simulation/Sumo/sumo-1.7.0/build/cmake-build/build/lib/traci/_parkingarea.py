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

# @file    _parkingarea.py
# @author  Jakob Erdmann
# @date    2020-06-02

from __future__ import absolute_import
from . import constants as tc
from .domain import Domain


class ParkingAreaDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "parkingarea", tc.CMD_GET_PARKINGAREA_VARIABLE, tc.CMD_SET_PARKINGAREA_VARIABLE,
                        tc.CMD_SUBSCRIBE_PARKINGAREA_VARIABLE, tc.RESPONSE_SUBSCRIBE_PARKINGAREA_VARIABLE,
                        tc.CMD_SUBSCRIBE_PARKINGAREA_CONTEXT, tc.RESPONSE_SUBSCRIBE_PARKINGAREA_CONTEXT)

    def getLaneID(self, stopID):
        """getLaneID(string) -> string
        Returns the lane of this calibrator (if it applies to a single lane)
        """
        return self._getUniversal(tc.VAR_LANE_ID, stopID)

    def getStartPos(self, stopID):
        """getStartPos(string) -> double

        The starting position of the stop along the lane measured in m.
        """
        return self._getUniversal(tc.VAR_POSITION, stopID)

    def getEndPos(self, stopID):
        """getStartPos(string) -> double

        The end position of the stop along the lane measured in m.
        """
        return self._getUniversal(tc.VAR_LANEPOSITION, stopID)

    def getName(self, stopID):
        """getName(string) -> string

        Returns the name of this stop
        """
        return self._getUniversal(tc.VAR_NAME, stopID)

    def getVehicleCount(self, stopID):
        """getParkingAreaWaiting() -> integer
        Get the total number of vehicles stopped at the named parking area.
        """
        return self._getUniversal(tc.VAR_STOP_STARTING_VEHICLES_NUMBER, stopID)

    def getVehicleIDs(self, stopID):
        """getParkingAreaWaiting() -> list(string)
        Get the IDs of vehicles stopped at the named parking area.
        """
        return self._getUniversal(tc.VAR_STOP_STARTING_VEHICLES_IDS, stopID)
