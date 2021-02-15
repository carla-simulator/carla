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

# @file    _inductionloop.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2011-03-16

from __future__ import absolute_import
from .domain import Domain
from . import constants as tc


def readVehicleData(result):
    result.readLength()
    nbData = result.readInt()
    data = []
    for _ in range(nbData):
        vehID = result.readTypedString()
        length = result.readTypedDouble()
        entryTime = result.readTypedDouble()
        leaveTime = result.readTypedDouble()
        typeID = result.readTypedString()
        data.append((vehID, length, entryTime, leaveTime, typeID))
    return data


_RETURN_VALUE_FUNC = {tc.LAST_STEP_VEHICLE_DATA: readVehicleData}


class InductionLoopDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "inductionloop", tc.CMD_GET_INDUCTIONLOOP_VARIABLE, None,
                        tc.CMD_SUBSCRIBE_INDUCTIONLOOP_VARIABLE, tc.RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE,
                        tc.CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT, tc.RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT,
                        _RETURN_VALUE_FUNC,
                        subscriptionDefault=(tc.LAST_STEP_VEHICLE_NUMBER,))

    def getPosition(self, loopID):
        """getPosition(string) -> double

        Returns the position measured from the beginning of the lane in meters.
        """
        return self._getUniversal(tc.VAR_POSITION, loopID)

    def getLaneID(self, loopID):
        """getLaneID(string) -> string

        Returns the id of the lane the loop is on.
        """
        return self._getUniversal(tc.VAR_LANE_ID, loopID)

    def getLastStepVehicleNumber(self, loopID):
        """getLastStepVehicleNumber(string) -> integer

        Returns the number of vehicles that were on the named induction loop within the last simulation step.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_NUMBER, loopID)

    def getLastStepMeanSpeed(self, loopID):
        """getLastStepMeanSpeed(string) -> double

        Returns the mean speed in m/s of vehicles that were on the named induction loop within the last simulation step.
        """
        return self._getUniversal(tc.LAST_STEP_MEAN_SPEED, loopID)

    def getLastStepVehicleIDs(self, loopID):
        """getLastStepVehicleIDs(string) -> list(string)

        Returns the list of ids of vehicles that were on the named induction loop in the last simulation step.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_ID_LIST, loopID)

    def getLastStepOccupancy(self, loopID):
        """getLastStepOccupancy(string) -> double

        Returns the percentage of time the detector was occupied by a vehicle.
        """
        return self._getUniversal(tc.LAST_STEP_OCCUPANCY, loopID)

    def getLastStepMeanLength(self, loopID):
        """getLastStepMeanLength(string) -> double

        Returns the mean length in m of vehicles which were on the detector in the last step.
        """
        return self._getUniversal(tc.LAST_STEP_LENGTH, loopID)

    def getTimeSinceDetection(self, loopID):
        """getTimeSinceDetection(string) -> double

        Returns the time in s since last detection.
        """
        return self._getUniversal(tc.LAST_STEP_TIME_SINCE_DETECTION, loopID)

    def getVehicleData(self, loopID):
        """getVehicleData(string) -> [(veh_id, veh_length, entry_time, exit_time, vType), ...]

        Returns a complex structure containing several information about vehicles which passed the detector.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_DATA, loopID)
