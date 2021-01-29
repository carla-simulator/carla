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

# @file    _simulation.py
# @author  Daniel Krajzewicz
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2011-03-15

from __future__ import absolute_import
import warnings
from . import constants as tc
from .domain import Domain
from .exceptions import FatalTraCIError


class Stage(object):

    def __init__(self,
                 type=tc.INVALID_INT_VALUE,
                 vType="",
                 line="",
                 destStop="",
                 edges=[],
                 travelTime=tc.INVALID_DOUBLE_VALUE,
                 cost=tc.INVALID_DOUBLE_VALUE,
                 length=tc.INVALID_DOUBLE_VALUE,
                 intended="",
                 depart=tc.INVALID_DOUBLE_VALUE,
                 departPos=tc.INVALID_DOUBLE_VALUE,
                 arrivalPos=tc.INVALID_DOUBLE_VALUE,
                 description=""):
        self.type = type
        self.vType = vType
        self.line = line
        self.destStop = destStop
        self.edges = edges
        self.travelTime = travelTime
        self.cost = cost
        self.length = length
        self.intended = intended
        self.depart = depart
        self.departPos = departPos
        self.arrivalPos = arrivalPos
        self.description = description

    def __attr_repr__(self, attrname, default=""):
        if getattr(self, attrname) == default:
            return ""
        else:
            val = getattr(self, attrname)
            if val == tc.INVALID_DOUBLE_VALUE:
                val = "INVALID"
            return "%s=%s" % (attrname, val)

    def __repr__(self):
        return "Stage(%s)" % ', '.join([v for v in [
            self.__attr_repr__("type"),
            self.__attr_repr__("vType"),
            self.__attr_repr__("line"),
            self.__attr_repr__("destStop"),
            self.__attr_repr__("edges"),
            self.__attr_repr__("travelTime"),
            self.__attr_repr__("cost"),
            self.__attr_repr__("length"),
            self.__attr_repr__("intended"),
            self.__attr_repr__("depart"),
            self.__attr_repr__("departPos"),
            self.__attr_repr__("arrivalPos"),
            self.__attr_repr__("description"),
        ] if v != ""])


def _readStage(result):
    # compound size and type
    assert(result.read("!i")[0] == 13)
    stageType = result.readTypedInt()
    vType = result.readTypedString()
    line = result.readTypedString()
    destStop = result.readTypedString()
    edges = result.readTypedStringList()
    travelTime = result.readTypedDouble()
    cost = result.readTypedDouble()
    length = result.readTypedDouble()
    intended = result.readTypedString()
    depart = result.readTypedDouble()
    departPos = result.readTypedDouble()
    arrivalPos = result.readTypedDouble()
    description = result.readTypedString()
    return Stage(stageType, vType, line, destStop, edges, travelTime, cost,
                 length, intended, depart, departPos, arrivalPos, description)


def _writeStage(stage):
    format = "tisssldddsddds"
    values = [13, stage.type, stage.vType, stage.line, stage.destStop, stage.edges,
              stage.travelTime, stage.cost, stage.length, stage.intended,
              stage.depart, stage.departPos, stage.arrivalPos, stage.description]
    return format, values


_RETURN_VALUE_FUNC = {tc.FIND_ROUTE: _readStage}


class SimulationDomain(Domain):

    Stage = Stage

    def __init__(self):
        Domain.__init__(self, "simulation", tc.CMD_GET_SIM_VARIABLE, tc.CMD_SET_SIM_VARIABLE,
                        tc.CMD_SUBSCRIBE_SIM_VARIABLE, tc.RESPONSE_SUBSCRIBE_SIM_VARIABLE,
                        tc.CMD_SUBSCRIBE_SIM_CONTEXT, tc.RESPONSE_SUBSCRIBE_SIM_CONTEXT,
                        _RETURN_VALUE_FUNC)

    @staticmethod
    def walkingStage(edges, arrivalPos, destStop="", description=""):
        return Stage(2, "", "", destStop, edges, 0, 0, 0, "", 0, 0, arrivalPos, description)

    def getTime(self):
        """getTime() -> double

        Returns the current simulation time in s.
        """
        return self._getUniversal(tc.VAR_TIME)

    def step(self, time=0.):
        """step(double) -> None
        Make a simulation step and simulate up to the given sim time (in seconds).
        If the given value is 0 or absent, exactly one step is performed.
        Values smaller than or equal to the current sim time result in no action.
        """
        if self._connection is None:
            raise FatalTraCIError("Not connected.")
        return self._connection.simulationStep(time)

    def getCurrentTime(self):
        """getCurrentTime() -> integer

        Returns the current simulation time in ms.
        """
        warnings.warn("getCurrentTime is deprecated, please use getTime which returns floating point seconds",
                      stacklevel=2)
        return self._getUniversal(tc.VAR_TIME_STEP)

    def getLoadedNumber(self):
        """getLoadedNumber() -> integer

        Returns the number of vehicles which were loaded in this time step.
        """
        return self._getUniversal(tc.VAR_LOADED_VEHICLES_NUMBER)

    def getLoadedIDList(self):
        """getLoadedIDList() -> list(string)

        Returns a list of ids of vehicles which were loaded in this time step.
        """
        return self._getUniversal(tc.VAR_LOADED_VEHICLES_IDS)

    def getDepartedNumber(self):
        """getDepartedNumber() -> integer

        Returns the number of vehicles which departed (were inserted into the road network) in this time step.
        """
        return self._getUniversal(tc.VAR_DEPARTED_VEHICLES_NUMBER)

    def getDepartedIDList(self):
        """getDepartedIDList() -> list(string)

        Returns a list of ids of vehicles which departed (were inserted into the road network) in this time step.
        """
        return self._getUniversal(tc.VAR_DEPARTED_VEHICLES_IDS)

    def getArrivedNumber(self):
        """getArrivedNumber() -> integer

        Returns the number of vehicles which arrived (have reached their destination and are removed from the road
        network) in this time step.
        """
        return self._getUniversal(tc.VAR_ARRIVED_VEHICLES_NUMBER)

    def getArrivedIDList(self):
        """getArrivedIDList() -> list(string)

        Returns a list of ids of vehicles which arrived (have reached their destination and are removed from the road
        network) in this time step.
        """
        return self._getUniversal(tc.VAR_ARRIVED_VEHICLES_IDS)

    def getParkingStartingVehiclesNumber(self):
        """getParkingStartingVehiclesNumber() -> integer

        .
        """
        return self._getUniversal(tc.VAR_PARKING_STARTING_VEHICLES_NUMBER)

    def getParkingStartingVehiclesIDList(self):
        """getParkingStartingVehiclesIDList() -> list(string)

        .
        """
        return self._getUniversal(tc.VAR_PARKING_STARTING_VEHICLES_IDS)

    def getParkingEndingVehiclesNumber(self):
        """getParkingEndingVehiclesNumber() -> integer

        .
        """
        return self._getUniversal(tc.VAR_PARKING_ENDING_VEHICLES_NUMBER)

    def getParkingEndingVehiclesIDList(self):
        """getParkingEndingVehiclesIDList() -> list(string)

        .
        """
        return self._getUniversal(tc.VAR_PARKING_ENDING_VEHICLES_IDS)

    def getStopStartingVehiclesNumber(self):
        """getStopStartingVehiclesNumber() -> integer

        .
        """
        return self._getUniversal(tc.VAR_STOP_STARTING_VEHICLES_NUMBER)

    def getStopStartingVehiclesIDList(self):
        """getStopStartingVehiclesIDList() -> list(string)

        .
        """
        return self._getUniversal(tc.VAR_STOP_STARTING_VEHICLES_IDS)

    def getStopEndingVehiclesNumber(self):
        """getStopEndingVehiclesNumber() -> integer

        .
        """
        return self._getUniversal(tc.VAR_STOP_ENDING_VEHICLES_NUMBER)

    def getStopEndingVehiclesIDList(self):
        """getStopEndingVehiclesIDList() -> list(string)

        .
        """
        return self._getUniversal(tc.VAR_STOP_ENDING_VEHICLES_IDS)

    def getCollidingVehiclesNumber(self):
        """getCollidingVehiclesNumber() -> integer
        Return number of vehicles involved in a collision (typically 2 per
        collision).
        """
        return self._getUniversal(tc.VAR_COLLIDING_VEHICLES_NUMBER)

    def getCollidingVehiclesIDList(self):
        """getCollidingVehiclesIDList() -> list(string)
        Return Ids of vehicles involved in a collision (typically 2 per
        collision).
        """
        return self._getUniversal(tc.VAR_COLLIDING_VEHICLES_IDS)

    def getEmergencyStoppingVehiclesNumber(self):
        """getEmergencyStoppingVehiclesNumber() -> integer
        Return number of vehicles that performed an emergency stop in the last step
        """
        return self._getUniversal(tc.VAR_EMERGENCYSTOPPING_VEHICLES_NUMBER)

    def getEmergencyStoppingVehiclesIDList(self):
        """getEmergencyStoppingVehiclesIDList() -> list(string)
        Return Ids of vehicles that peformed an emergency stop in the last step
        """
        return self._getUniversal(tc.VAR_EMERGENCYSTOPPING_VEHICLES_IDS)

    def getMinExpectedNumber(self):
        """getMinExpectedNumber() -> integer

        Returns the number of vehicles which are in the net plus the
        ones still waiting to start. This number may be smaller than
        the actual number of vehicles still to come because of delayed
        route file parsing. If the number is 0 however, it is
        guaranteed that all route files have been parsed completely
        and all vehicles have left the network.
        """
        return self._getUniversal(tc.VAR_MIN_EXPECTED_VEHICLES)

    def getBusStopIDList(self):
        return self._getUniversal(tc.VAR_BUS_STOP_ID_LIST)

    def getBusStopWaiting(self, stopID):
        """getBusStopWaiting() -> integer
        Get the total number of waiting persons at the named bus stop.
        """
        return self._getUniversal(tc.VAR_BUS_STOP_WAITING, stopID)

    def getBusStopWaitingIDList(self, stopID):
        """getBusStopWaiting() -> list(string)
        Get the IDs of waiting persons at the named bus stop.
        """
        return self._getUniversal(tc.VAR_BUS_STOP_WAITING_IDS, stopID)

    def getStartingTeleportNumber(self):
        """getStartingTeleportNumber() -> integer

        Returns the number of vehicles which started to teleport in this time step.
        """
        return self._getUniversal(tc.VAR_TELEPORT_STARTING_VEHICLES_NUMBER)

    def getStartingTeleportIDList(self):
        """getStartingTeleportIDList() -> list(string)

        Returns a list of ids of vehicles which started to teleport in this time step.
        """
        return self._getUniversal(tc.VAR_TELEPORT_STARTING_VEHICLES_IDS)

    def getEndingTeleportNumber(self):
        """getEndingTeleportNumber() -> integer

        Returns the number of vehicles which ended to be teleported in this time step.
        """
        return self._getUniversal(tc.VAR_TELEPORT_ENDING_VEHICLES_NUMBER)

    def getEndingTeleportIDList(self):
        """getEndingTeleportIDList() -> list(string)

        Returns a list of ids of vehicles which ended to be teleported in this time step.
        """
        return self._getUniversal(tc.VAR_TELEPORT_ENDING_VEHICLES_IDS)

    def getDeltaT(self):
        """getDeltaT() -> double
        Returns the length of one simulation step in seconds
        """
        return self._getUniversal(tc.VAR_DELTA_T)

    def getNetBoundary(self):
        """getNetBoundary() -> ((double, double), (double, double))

        The boundary box of the simulation network.
        """
        return self._getUniversal(tc.VAR_NET_BOUNDING_BOX)

    def convert2D(self, edgeID, pos, laneIndex=0, toGeo=False):
        posType = tc.POSITION_2D
        if toGeo:
            posType = tc.POSITION_LON_LAT
        return self._getUniversal(tc.POSITION_CONVERSION, "", "trB", 2, (edgeID, pos, laneIndex), posType)

    def convert3D(self, edgeID, pos, laneIndex=0, toGeo=False):
        posType = tc.POSITION_3D
        if toGeo:
            posType = tc.POSITION_LON_LAT_ALT
        return self._getUniversal(tc.POSITION_CONVERSION, "", "trB", 2, (edgeID, pos, laneIndex), posType)

    def convertRoad(self, x, y, isGeo=False, vClass="ignoring"):
        format = "toBs"
        if isGeo:
            format = "tgBs"
        result = self._getCmd(tc.POSITION_CONVERSION, "", format, 3, (x, y), tc.POSITION_ROADMAP, vClass)
        result.read("!B")
        return result.readString(), result.readDouble(), result.read("!B")[0]

    def convertGeo(self, x, y, fromGeo=False):
        format = "toB"
        toType = tc.POSITION_LON_LAT
        if fromGeo:
            format = "tgB"
            toType = tc.POSITION_2D
        return self._getUniversal(tc.POSITION_CONVERSION, "", format, 2, (x, y), toType)

    def getDistance2D(self, x1, y1, x2, y2, isGeo=False, isDriving=False):
        """getDistance2D(double, double, double, double, boolean, boolean) -> double

        Returns the distance between the two coordinate pairs (x1,y1) and (x2,y2)

        If isGeo=True, coordinates are interpreted as longitude and latitude rather
        than cartesian coordinates in meters.

        If isDriving=True, the coordinates are mapped onto the road network and the
        length of the shortest route in the network is returned. Otherwise, the
        straight-line distance is returned.
        """
        format = "tggu" if isGeo else "toou"
        distType = tc.REQUEST_AIRDIST
        if isDriving:
            distType = tc.REQUEST_DRIVINGDIST
        return self._getUniversal(tc.DISTANCE_REQUEST, "", format, 3, (x1, y1), (x2, y2), distType)

    def getDistanceRoad(self, edgeID1, pos1, edgeID2, pos2, isDriving=False):
        """getDistanceRoad(string, double, string, double, boolean) -> double

        Reads two positions on the road network and an indicator whether the air or the driving distance shall be
        computed. Returns the according distance.
        """
        distType = tc.REQUEST_AIRDIST
        if isDriving:
            distType = tc.REQUEST_DRIVINGDIST
        return self._getUniversal(tc.DISTANCE_REQUEST, "", "trru", 3,
                                  (edgeID1, pos1, 0), (edgeID2, pos2, 0), distType)

    def findRoute(self, fromEdge, toEdge, vType="", depart=-1., routingMode=0):
        return self._getUniversal(tc.FIND_ROUTE, "", "tsssdi", 5, fromEdge, toEdge, vType, depart, routingMode)

    def findIntermodalRoute(self, fromEdge, toEdge, modes="", depart=-1., routingMode=0, speed=-1.,
                            walkFactor=-1., departPos=0., arrivalPos=tc.INVALID_DOUBLE_VALUE, departPosLat=0.,
                            pType="", vType="", destStop=""):
        answer = self._getCmd(tc.FIND_INTERMODAL_ROUTE, "", "tsssdidddddsss", 13,
                              fromEdge, toEdge, modes, depart, routingMode, speed, walkFactor,
                              departPos, arrivalPos, departPosLat, pType, vType, destStop)
        answer.read("!B")                   # Type
        result = []
        for _ in range(answer.readInt()):
            answer.read("!B")                   # Type
            result.append(_readStage(answer))
        return tuple(result)

    def clearPending(self, routeID=""):
        self._setCmd(tc.CMD_CLEAR_PENDING_VEHICLES, "", "s", routeID)

    def saveState(self, fileName):
        self._setCmd(tc.CMD_SAVE_SIMSTATE, "", "s", fileName)

    def loadState(self, fileName):
        self._setCmd(tc.CMD_LOAD_SIMSTATE, "", "s", fileName)

    def writeMessage(self, msg):
        self._setCmd(tc.CMD_MESSAGE, "", "s", msg)

    def subscribe(self, varIDs=(tc.VAR_DEPARTED_VEHICLES_IDS,), begin=0, end=2**31 - 1):
        """subscribe(list(integer), double, double) -> None

        Subscribe to one or more simulation values for the given interval.
        """
        Domain.subscribe(self, "", varIDs, begin, end)

    def getSubscriptionResults(self):
        """getSubscriptionResults() -> dict(integer: <value_type>)

        Returns the subscription results for the last time step.
        It is not possible to retrieve older subscription results than the ones
        from the last time step.
        """
        return Domain.getSubscriptionResults(self, "")
