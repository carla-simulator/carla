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

# @file    _edge.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2011-03-17

from __future__ import absolute_import
from . import constants as tc
from .domain import Domain
from .exceptions import TraCIException


class EdgeDomain(Domain):

    def __init__(self):
        Domain.__init__(self, "edge", tc.CMD_GET_EDGE_VARIABLE, tc.CMD_SET_EDGE_VARIABLE,
                        tc.CMD_SUBSCRIBE_EDGE_VARIABLE, tc.RESPONSE_SUBSCRIBE_EDGE_VARIABLE,
                        tc.CMD_SUBSCRIBE_EDGE_CONTEXT, tc.RESPONSE_SUBSCRIBE_EDGE_CONTEXT,
                        subscriptionDefault=(tc.LAST_STEP_VEHICLE_NUMBER,))

    def getAdaptedTraveltime(self, edgeID, time):
        """getAdaptedTraveltime(string, double) -> double

        Returns the travel time value (in s) used for (re-)routing
        which is valid on the edge at the given time.
        """
        return self._getUniversal(tc.VAR_EDGE_TRAVELTIME, edgeID, "d", time)

    def getWaitingTime(self, edgeID):
        """getWaitingTime(string) -> double
        Returns the sum of the waiting time of all vehicles currently on
        that edge (see traci.vehicle.getWaitingTime).
        """
        return self._getUniversal(tc.VAR_WAITING_TIME, edgeID)

    def getEffort(self, edgeID, time):
        """getEffort(string, double) -> double

        Returns the effort value used for (re-)routing
        which is valid on the edge at the given time.
        """
        return self._getUniversal(tc.VAR_EDGE_EFFORT, edgeID, "d", time)

    def getCO2Emission(self, edgeID):
        """getCO2Emission(string) -> double

        Returns the CO2 emission in mg for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_CO2EMISSION, edgeID)

    def getCOEmission(self, edgeID):
        """getCOEmission(string) -> double

        Returns the CO emission in mg for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_COEMISSION, edgeID)

    def getHCEmission(self, edgeID):
        """getHCEmission(string) -> double

        Returns the HC emission in mg for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_HCEMISSION, edgeID)

    def getPMxEmission(self, edgeID):
        """getPMxEmission(string) -> double

        Returns the particular matter emission in mg for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_PMXEMISSION, edgeID)

    def getNOxEmission(self, edgeID):
        """getNOxEmission(string) -> double

        Returns the NOx emission in mg for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_NOXEMISSION, edgeID)

    def getFuelConsumption(self, edgeID):
        """getFuelConsumption(string) -> double

        Returns the fuel consumption in ml for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_FUELCONSUMPTION, edgeID)

    def getNoiseEmission(self, edgeID):
        """getNoiseEmission(string) -> double

        Returns the noise emission in db for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_NOISEEMISSION, edgeID)

    def getElectricityConsumption(self, edgeID):
        """getElectricityConsumption(string) -> double

        Returns the electricity consumption in ml for the last time step.
        """
        return self._getUniversal(tc.VAR_ELECTRICITYCONSUMPTION, edgeID)

    def getLastStepMeanSpeed(self, edgeID):
        """getLastStepMeanSpeed(string) -> double

        Returns the average speed in m/s for the last time step on the given edge.
        """
        return self._getUniversal(tc.LAST_STEP_MEAN_SPEED, edgeID)

    def getLastStepOccupancy(self, edgeID):
        """getLastStepOccupancy(string) -> double

        Returns the net occupancy (excluding inter-vehicle gaps) in % for the last time step on the given edge.
        """
        return self._getUniversal(tc.LAST_STEP_OCCUPANCY, edgeID)

    def getLastStepLength(self, edgeID):
        """getLastStepLength(string) -> double

        Returns the mean vehicle length in m for the last time step on the given edge.
        """
        return self._getUniversal(tc.LAST_STEP_LENGTH, edgeID)

    def getLaneNumber(self, edgeID):
        """getLaneNumber(string) -> int

        Returns the number of lanes of this edge
        """
        return self._getUniversal(tc.VAR_LANE_INDEX, edgeID)

    def getStreetName(self, edgeID):
        """getStreetName(string) -> string

        Returns the street name of this edge
        """
        return self._getUniversal(tc.VAR_NAME, edgeID)

    def getTraveltime(self, edgeID):
        """getTraveltime(string) -> double

        Returns the estimated travel time in s for the last time step on the given edge.
        """
        return self._getUniversal(tc.VAR_CURRENT_TRAVELTIME, edgeID)

    def getLastStepVehicleNumber(self, edgeID):
        """getLastStepVehicleNumber(string) -> integer

        Returns the total number of vehicles for the last time step on the given edge.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_NUMBER, edgeID)

    def getLastStepHaltingNumber(self, edgeID):
        """getLastStepHaltingNumber(string) -> integer

        Returns the total number of halting vehicles for the last time step on the given edge.
        A speed of less than 0.1 m/s is considered a halt.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_HALTING_NUMBER, edgeID)

    def getLastStepVehicleIDs(self, edgeID):
        """getLastStepVehicleIDs(string) -> list(string)

        Returns the ids of the vehicles for the last time step on the given edge.
        """
        return self._getUniversal(tc.LAST_STEP_VEHICLE_ID_LIST, edgeID)

    def getLastStepPersonIDs(self, edgeID):
        """getLastStepPersonIDs(string) -> list(string)

        Returns the ids of the persons on the given edge during the last time step.
        """
        return self._getUniversal(tc.LAST_STEP_PERSON_ID_LIST, edgeID)

    def adaptTraveltime(self, edgeID, time, begin=None, end=None):
        """adaptTraveltime(string, double, double, double) -> None

        Adapt the travel time value (in s) used for (re-)routing for the given edge.

        When setting begin time and end time (in seconds), the changes only
        apply to that time range. Otherwise they apply all the time
        """
        if begin is None and end is None:
            self._setCmd(tc.VAR_EDGE_TRAVELTIME, edgeID, "td", 1, time)
        elif begin is not None and end is not None:
            self._setCmd(tc.VAR_EDGE_TRAVELTIME, edgeID, "tddd", 3, begin, end, time)
        else:
            raise TraCIException("Both, begin time and end time must be specified")

    def setEffort(self, edgeID, effort, begin=None, end=None):
        """setEffort(string, double, double, double) -> None

        Adapt the effort value used for (re-)routing for the given edge.

        When setting begin time and end time (in seconds), the changes only
        apply to that time range. Otherwise they apply all the time.
        """
        if begin is None and end is None:
            self._setCmd(tc.VAR_EDGE_EFFORT, edgeID, "td", 1, effort)
        elif begin is not None and end is not None:
            self._setCmd(tc.VAR_EDGE_EFFORT, edgeID, "tddd", 3, begin, end, effort)
        else:
            raise TraCIException("Both, begin time and end time must be specified")

    def setMaxSpeed(self, edgeID, speed):
        """setMaxSpeed(string, double) -> None

        Set a new maximum speed (in m/s) for all lanes of the edge.
        """
        self._setCmd(tc.VAR_MAXSPEED, edgeID, "d", speed)
