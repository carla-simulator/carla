/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    MSStopOut.cpp
/// @author  Jakob Erdmann
/// @date    Wed, 21.12.2016
///
// Ouput information about planned vehicle stop
/****************************************************************************/
#include <config.h>

#include <utils/vehicle/SUMOVehicle.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSParkingArea.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/trigger/MSChargingStation.h>
#include <microsim/trigger/MSOverheadWire.h>
#include "MSStopOut.h"


// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
MSStopOut* MSStopOut::myInstance = nullptr;

void
MSStopOut::init() {
    if (OptionsCont::getOptions().isSet("stop-output")) {
        myInstance = new MSStopOut(OutputDevice::getDeviceByOption("stop-output"));
    }
}

void
MSStopOut::cleanup() {
    delete myInstance;
    myInstance = nullptr;
}

// ===========================================================================
// method definitions
// ===========================================================================
MSStopOut::MSStopOut(OutputDevice& dev) :
    myDevice(dev) {
}

MSStopOut::~MSStopOut() {}


void
MSStopOut::stopStarted(const SUMOVehicle* veh, int numPersons, int numContainers, SUMOTime time) {
    assert(veh != 0);
    if (myStopped.count(veh) != 0) {
        WRITE_WARNING("Vehicle '" + veh->getID() + "' stops on edge '" + veh->getEdge()->getID()
                      + "', time " + time2string(time)
                      + " without ending the previous stop entered at time " + time2string(myStopped[veh].started));
    }
    StopInfo stopInfo(MSNet::getInstance()->getCurrentTimeStep(), numPersons, numContainers);
    myStopped[veh] = stopInfo;
}

void
MSStopOut::loadedPersons(const SUMOVehicle* veh, int n) {
    // ignore triggered vehicles
    if (veh->hasDeparted()) {
        myStopped[veh].loadedPersons += n;
    }
}

void
MSStopOut::unloadedPersons(const SUMOVehicle* veh, int n) {
    myStopped[veh].unloadedPersons += n;
}

void
MSStopOut::loadedContainers(const SUMOVehicle* veh, int n) {
    myStopped[veh].loadedContainers += n;
}

void
MSStopOut::unloadedContainers(const SUMOVehicle* veh, int n) {
    myStopped[veh].unloadedContainers += n;
}

void
MSStopOut::stopEnded(const SUMOVehicle* veh, const SUMOVehicleParameter::Stop& stop, const std::string& laneOrEdgeID) {
    assert(veh != 0);
    if (myStopped.count(veh) == 0) {
        WRITE_WARNING("Vehicle '" + veh->getID() + "' ends stop on edge '" + veh->getEdge()->getID()
                      + "', time " + time2string(MSNet::getInstance()->getCurrentTimeStep()) + " without entering the stop");
        return;
    }
    StopInfo& si = myStopped[veh];
    double delay = -1;
    double arrivalDelay = -1;
    if (stop.until >= 0) {
        delay = STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() - stop.until);
    }
    if (stop.arrival >= 0) {
        arrivalDelay = STEPS2TIME(si.started - stop.arrival);
    }
    myDevice.openTag("stopinfo");
    myDevice.writeAttr(SUMO_ATTR_ID, veh->getID());
    myDevice.writeAttr(SUMO_ATTR_TYPE, veh->getVehicleType().getID());
    if (MSGlobals::gUseMesoSim) {
        myDevice.writeAttr(SUMO_ATTR_EDGE, laneOrEdgeID);
    } else {
        myDevice.writeAttr(SUMO_ATTR_LANE, laneOrEdgeID);
    }
    myDevice.writeAttr(SUMO_ATTR_POSITION, veh->getPositionOnLane());
    myDevice.writeAttr(SUMO_ATTR_PARKING, stop.parking);
    myDevice.writeAttr("started", time2string(si.started));
    myDevice.writeAttr("ended", time2string(MSNet::getInstance()->getCurrentTimeStep()));
    myDevice.writeAttr("delay", delay);
    if (stop.arrival >= 0) {
        myDevice.writeAttr("arrivalDelay", arrivalDelay);
    }
    myDevice.writeAttr("initialPersons", si.initialNumPersons);
    myDevice.writeAttr("loadedPersons", si.loadedPersons);
    myDevice.writeAttr("unloadedPersons", si.unloadedPersons);
    myDevice.writeAttr("initialContainers", si.initialNumContainers);
    myDevice.writeAttr("loadedContainers", si.loadedContainers);
    myDevice.writeAttr("unloadedContainers", si.unloadedContainers);
    if (stop.busstop != "") {
        myDevice.writeAttr(SUMO_ATTR_BUS_STOP, stop.busstop);
    }
    if (stop.containerstop != "") {
        myDevice.writeAttr(SUMO_ATTR_CONTAINER_STOP, stop.containerstop);
    }
    if (stop.parkingarea != "") {
        myDevice.writeAttr(SUMO_ATTR_PARKING_AREA, stop.parkingarea);
    }
    if (stop.chargingStation != "") {
        myDevice.writeAttr(SUMO_ATTR_CHARGING_STATION, stop.chargingStation);
    }
    if (stop.overheadWireSegment != "") {
        myDevice.writeAttr(SUMO_ATTR_OVERHEAD_WIRE_SEGMENT, stop.overheadWireSegment);
    }
    if (stop.tripId != "") {
        myDevice.writeAttr(SUMO_ATTR_TRIP_ID, stop.tripId);
    }
    if (stop.line != "") {
        myDevice.writeAttr(SUMO_ATTR_LINE, stop.line);
    }
    if (stop.split != "") {
        myDevice.writeAttr(SUMO_ATTR_SPLIT, stop.split);
    }
    myDevice.closeTag();
    myStopped.erase(veh);
}


/****************************************************************************/
