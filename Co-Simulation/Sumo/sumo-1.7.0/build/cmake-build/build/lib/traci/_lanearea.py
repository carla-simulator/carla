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

# @file    _lanearea.py
# @author  Mario Krumnow
# @author  Laura Bieker
# @date    2011-03-16

from __future__ import absolute_import
from . import constants as tc
from .domain import Domain


class LaneAreaDomain(Domain):

    def __init__(self, name="lanearea", deprecatedFor=None):
        Domain.__init__(self, name, tc.CMD_GET_LANEAREA_VARIABLE, None,
                        tc.CMD_SUBSCRIBE_LANEAREA_VARIABLE, tc.RESPONSE_SUBSCRIBE_LANEAREA_VARIABLE,
                        tc.CMD_SUBSCRIBE_LANEAREA_CONTEXT, tc.RESPONSE_SUBSCRIBE_LANEAREA_CONTEXT,
                        {}, deprecatedFor,
                        subscriptionDefault=(tc.LAST_STEP_VEHICLE_NUMBER,))

    def getJamLengthVehicle(self, detID):
        """getJamLengthVehicle(string) -> integer

        Returns the jam length in vehicles within the last simulation step.
        """
        return self._getUniversal(tc.JAM_LENGTH_VEHICLE, detID)

    def getJamLengthMeters(self, detID):
        """getJamLengthMeters(string) -> double

        Returns the jam length in meters within the last simulation step.
        """
        return self._getUniversal(tc.JAM_LENGTH_METERS, detID)

    def getLastStepMeanSpeed(self, detID):
        """getLastStepMeanSpeed(string) -> double

        Returns the current mean speed in m/s of vehicles that were on the named e2.
        """
        return self._getUniversal(tc.LAST_STEP_MEAN_SPEED, detID)

    def getLastStepVehicleIDs(self, detID):
        """getLastStepVehicleIDs(string) -> list(string)

        Returns the list of ids of vehicles that were on the named detector in the last simulation step.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_ID_LIST, detID)

    def getLastStepOccupancy(self, detID):
        """getLastStepMeanSpeed(string) -> double

        Returns the percentage of space the detector was occupied by a vehicle [%]
        """
        return self._getUniversal(tc.LAST_STEP_OCCUPANCY, detID)

    def getPosition(self, detID):
        """getPosition(string) -> double

        Returns the starting position of the detector measured from the beginning of the lane in meters.
        """
        return self._getUniversal(tc.VAR_POSITION, detID)

    def getLaneID(self, detID):
        """getLaneID(string) -> string

        Returns the id of the lane the detector is on.
        """
        return self._getUniversal(tc.VAR_LANE_ID, detID)

    def getLength(self, detID):
        """getLength(string) -> double

        Returns the length of the detector
        """
        return self._getUniversal(tc.VAR_LENGTH, detID)

    def getLastStepVehicleNumber(self, detID):
        """getLastStepVehicleNumber(string) -> integer

        Returns the number of vehicles that were on the named detector within the last simulation step.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_NUMBER, detID)

    def getLastStepHaltingNumber(self, detID):
        """getLastStepHaltingNumber(string) -> integer

        Returns the number of vehicles which were halting during the last time step.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_HALTING_NUMBER, detID)
