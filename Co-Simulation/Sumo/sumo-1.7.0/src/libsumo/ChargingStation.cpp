/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    ChargingStation.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSStoppingPlace.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "ChargingStation.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults ChargingStation::mySubscriptionResults;
ContextSubscriptionResults ChargingStation::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
ChargingStation::getIDList() {
    std::vector<std::string> ids;
    for (auto& item : MSNet::getInstance()->getStoppingPlaces(SUMO_TAG_CHARGING_STATION)) {
        ids.push_back(item.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int
ChargingStation::getIDCount() {
    return (int)getIDList().size();
}


std::string
ChargingStation::getLaneID(const std::string& stopID) {
    return getChargingStation(stopID)->getLane().getID();
}

double
ChargingStation::getStartPos(const std::string& stopID) {
    return getChargingStation(stopID)->getBeginLanePosition();
}

double
ChargingStation::getEndPos(const std::string& stopID) {
    return getChargingStation(stopID)->getEndLanePosition();
}

std::string
ChargingStation::getName(const std::string& stopID) {
    return getChargingStation(stopID)->getMyName();
}

int
ChargingStation::getVehicleCount(const std::string& stopID) {
    return (int)getChargingStation(stopID)->getStoppedVehicles().size();
}

std::vector<std::string>
ChargingStation::getVehicleIDs(const std::string& stopID) {
    std::vector<std::string> result;
    for (const SUMOVehicle* veh : getChargingStation(stopID)->getStoppedVehicles()) {
        result.push_back(veh->getID());
    }
    return result;
}



std::string
ChargingStation::getParameter(const std::string& stopID, const std::string& param) {
    const MSStoppingPlace* s = getChargingStation(stopID);
    return s->getParameter(param, "");
}

LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(ChargingStation)

void
ChargingStation::setParameter(const std::string& stopID, const std::string& key, const std::string& value) {
    MSStoppingPlace* s = getChargingStation(stopID);
    s->setParameter(key, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(ChargingStation, CHARGINGSTATION)


MSStoppingPlace*
ChargingStation::getChargingStation(const std::string& id) {
    MSStoppingPlace* s = MSNet::getInstance()->getStoppingPlace(id, SUMO_TAG_CHARGING_STATION);
    if (s == nullptr) {
        throw TraCIException("ChargingStation '" + id + "' is not known");
    }
    return s;
}


std::shared_ptr<VariableWrapper>
ChargingStation::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
ChargingStation::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_LANE_ID:
            return wrapper->wrapString(objID, variable, getLaneID(objID));
        case VAR_POSITION:
            return wrapper->wrapDouble(objID, variable, getStartPos(objID));
        case VAR_LANEPOSITION:
            return wrapper->wrapDouble(objID, variable, getEndPos(objID));
        case VAR_NAME:
            return wrapper->wrapString(objID, variable, getName(objID));
        case VAR_STOP_STARTING_VEHICLES_NUMBER:
            return wrapper->wrapInt(objID, variable, getVehicleCount(objID));
        case VAR_STOP_STARTING_VEHICLES_IDS:
            return wrapper->wrapStringList(objID, variable, getVehicleIDs(objID));
        default:
            return false;
    }
}

}


/****************************************************************************/
