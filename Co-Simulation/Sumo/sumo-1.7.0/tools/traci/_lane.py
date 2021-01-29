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

# @file    _lane.py
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @author  Laura Bieker
# @author  Jakob Erdmann
# @date    2011-03-17

from __future__ import absolute_import
from .domain import Domain
from . import constants as tc


def _readLinks(result):
    result.read("!Bi")  # Type Compound, Length
    nbLinks = result.readInt()
    links = []
    for _ in range(nbLinks):
        result.read("!B")                           # Type String
        approachedLane = result.readString()
        result.read("!B")                           # Type String
        approachedInternal = result.readString()
        result.read("!B")                           # Type Byte
        hasPrio = bool(result.read("!B")[0])
        result.read("!B")                           # Type Byte
        isOpen = bool(result.read("!B")[0])
        result.read("!B")                           # Type Byte
        hasFoe = bool(result.read("!B")[0])
        result.read("!B")                           # Type String
        state = result.readString()
        result.read("!B")                           # Type String
        direction = result.readString()
        result.read("!B")                           # Type Float
        length = result.readDouble()
        links.append((approachedLane, hasPrio, isOpen, hasFoe,
                      approachedInternal, state, direction, length))
    return links


_RETURN_VALUE_FUNC = {tc.LANE_LINKS: _readLinks}


class LaneDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "lane", tc.CMD_GET_LANE_VARIABLE, tc.CMD_SET_LANE_VARIABLE,
                        tc.CMD_SUBSCRIBE_LANE_VARIABLE, tc.RESPONSE_SUBSCRIBE_LANE_VARIABLE,
                        tc.CMD_SUBSCRIBE_LANE_CONTEXT, tc.RESPONSE_SUBSCRIBE_LANE_CONTEXT,
                        _RETURN_VALUE_FUNC, subscriptionDefault=(tc.LAST_STEP_VEHICLE_NUMBER,))

    def getLength(self, laneID):
        """getLength(string) -> double

        Returns the length in m.
        """
        return self._getUniversal(tc.VAR_LENGTH, laneID)

    def getMaxSpeed(self, laneID):
        """getMaxSpeed(string) -> double

        Returns the maximum allowed speed on the lane in m/s.
        """
        return self._getUniversal(tc.VAR_MAXSPEED, laneID)

    def getWidth(self, laneID):
        """getWidth(string) -> double

        Returns the width of the lane in m.
        """
        return self._getUniversal(tc.VAR_WIDTH, laneID)

    def getAllowed(self, laneID):
        """getAllowed(string) -> list(string)

        Returns a list of allowed vehicle classes. An empty list means all vehicles are allowed.
        """
        return self._getUniversal(tc.LANE_ALLOWED, laneID)

    def getDisallowed(self, laneID):
        """getDisallowed(string) -> list(string)

        Returns a list of disallowed vehicle classes.
        """
        return self._getUniversal(tc.LANE_DISALLOWED, laneID)

    def getLinkNumber(self, laneID):
        """getLinkNumber(string) -> integer

        Returns the number of connections to successive lanes.
        """
        return self._getUniversal(tc.LANE_LINK_NUMBER, laneID)

    def getLinks(self, laneID, extended=True):
        """getLinks(string) -> list((string, bool, bool, bool))
        A list containing id of successor lane together with priority, open and foe
        for each link.
        if extended=True, each result tuple contains
        (string approachedLane, bool hasPrio, bool isOpen, bool hasFoe,
        string approachedInternal, string state, string direction, float length)
        """
        complete_data = self._getUniversal(tc.LANE_LINKS, laneID)
        if extended:
            return complete_data
        else:
            # for downward compatibility
            return [tuple(d[:4]) for d in complete_data]

    def getShape(self, laneID):
        """getShape(string) -> list((double, double))

        List of 2D positions (cartesian) describing the geometry.
        """
        return self._getUniversal(tc.VAR_SHAPE, laneID)

    def getEdgeID(self, laneID):
        """getEdgeID(string) -> string

        Returns the id of the edge the lane belongs to.
        """
        return self._getUniversal(tc.LANE_EDGE_ID, laneID)

    def getCO2Emission(self, laneID):
        """getCO2Emission(string) -> double

        Returns the CO2 emission in mg for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_CO2EMISSION, laneID)

    def getCOEmission(self, laneID):
        """getCOEmission(string) -> double

        Returns the CO emission in mg for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_COEMISSION, laneID)

    def getHCEmission(self, laneID):
        """getHCEmission(string) -> double

        Returns the HC emission in mg for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_HCEMISSION, laneID)

    def getPMxEmission(self, laneID):
        """getPMxEmission(string) -> double

        Returns the particular matter emission in mg for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_PMXEMISSION, laneID)

    def getNOxEmission(self, laneID):
        """getNOxEmission(string) -> double

        Returns the NOx emission in mg for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_NOXEMISSION, laneID)

    def getFuelConsumption(self, laneID):
        """getFuelConsumption(string) -> double

        Returns the fuel consumption in ml for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_FUELCONSUMPTION, laneID)

    def getNoiseEmission(self, laneID):
        """getNoiseEmission(string) -> double

        Returns the noise emission in db for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_NOISEEMISSION, laneID)

    def getElectricityConsumption(self, laneID):
        """getElectricityConsumption(string) -> double

        Returns the electricity consumption in ml for the last time step.
        """
        return self._getUniversal(tc.VAR_ELECTRICITYCONSUMPTION, laneID)

    def getLastStepMeanSpeed(self, laneID):
        """getLastStepMeanSpeed(string) -> double

        Returns the average speed in m/s for the last time step on the given lane.
        """
        return self._getUniversal(tc.LAST_STEP_MEAN_SPEED, laneID)

    def getLastStepOccupancy(self, laneID):
        """getLastStepOccupancy(string) -> double

        Returns the occupancy in % for the last time step on the given lane.
        """
        return self._getUniversal(tc.LAST_STEP_OCCUPANCY, laneID)

    def getLastStepLength(self, laneID):
        """getLastStepLength(string) -> double

        Returns the mean vehicle length in m for the last time step on the given lane.
        """
        return self._getUniversal(tc.LAST_STEP_LENGTH, laneID)

    def getWaitingTime(self, laneID):
        """getWaitingTime() -> double

        .
        """
        return self._getUniversal(tc.VAR_WAITING_TIME, laneID)

    def getTraveltime(self, laneID):
        """getTraveltime(string) -> double

        Returns the estimated travel time in s for the last time step on the given lane.
        """
        return self._getUniversal(tc.VAR_CURRENT_TRAVELTIME, laneID)

    def getLastStepVehicleNumber(self, laneID):
        """getLastStepVehicleNumber(string) -> integer

        Returns the total number of vehicles for the last time step on the given lane.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_NUMBER, laneID)

    def getLastStepHaltingNumber(self, laneID):
        """getLastStepHaltingNumber(string) -> integer

        Returns the total number of halting vehicles for the last time step on the given lane.
        A speed of less than 0.1 m/s is considered a halt.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_HALTING_NUMBER, laneID)

    def getLastStepVehicleIDs(self, laneID):
        """getLastStepVehicleIDs(string) -> list(string)

        Returns the ids of the vehicles for the last time step on the given lane.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_ID_LIST, laneID)

    def getFoes(self, laneID, toLaneID):
        """getFoes(string, string) -> list(string)
        Returns the ids of incoming lanes that have right of way over the connection from laneID to toLaneID
        """
        return self._getUniversal(tc.VAR_FOES, laneID, "s", toLaneID)

    def getInternalFoes(self, laneID):
        """getFoes(string) -> list(string)
        Returns the ids of internal lanes that are in conflict with the given internal lane id
        """
        return self.getFoes(laneID, "")

    def setAllowed(self, laneID, allowedClasses):
        """setAllowed(string, list) -> None

        Sets a list of allowed vehicle classes. Setting an empty list means all vehicles are allowed.
        """
        if isinstance(allowedClasses, str):
            allowedClasses = [allowedClasses]
        self._setCmd(tc.LANE_ALLOWED, laneID, "l", allowedClasses)

    def setDisallowed(self, laneID, disallowedClasses):
        """setDisallowed(string, list) -> None

        Sets a list of disallowed vehicle classes.
        """
        if isinstance(disallowedClasses, str):
            disallowedClasses = [disallowedClasses]
        self._setCmd(tc.LANE_DISALLOWED, laneID, "l", disallowedClasses)

    def setMaxSpeed(self, laneID, speed):
        """setMaxSpeed(string, double) -> None

        Sets a new maximum allowed speed on the lane in m/s.
        """
        self._setCmd(tc.VAR_MAXSPEED, laneID, "d", speed)

    def setLength(self, laneID, length):
        """setLength(string, double) -> None

        Sets the length of the lane in m.
        """
        self._setCmd(tc.VAR_LENGTH, laneID, "d", length)
