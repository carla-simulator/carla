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
/// @file    OverheadWire.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/trigger/MSOverheadWire.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "OverheadWire.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults OverheadWire::mySubscriptionResults;
ContextSubscriptionResults OverheadWire::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
OverheadWire::getIDList() {
    std::vector<std::string> ids;
    for (auto& item : MSNet::getInstance()->getStoppingPlaces(SUMO_TAG_OVERHEAD_WIRE_SEGMENT)) {
        ids.push_back(item.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int
OverheadWire::getIDCount() {
    return (int)getIDList().size();
}


std::string
OverheadWire::getLaneID(const std::string& stopID) {
    return getOverheadWire(stopID)->getLane().getID();
}


double
OverheadWire::getStartPos(const std::string& stopID) {
    return getOverheadWire(stopID)->getBeginLanePosition();
}

double
OverheadWire::getEndPos(const std::string& stopID) {
    return getOverheadWire(stopID)->getEndLanePosition();
}

std::string
OverheadWire::getName(const std::string& stopID) {
    return getOverheadWire(stopID)->getMyName();
}

int
OverheadWire::getVehicleCount(const std::string& stopID) {
    MSOverheadWire* wire = dynamic_cast<MSOverheadWire*>(getOverheadWire(stopID));
    return (int)wire->getChargingVehicles().size();
}

std::vector<std::string>
OverheadWire::getVehicleIDs(const std::string& stopID) {
    MSOverheadWire* wire = dynamic_cast<MSOverheadWire*>(getOverheadWire(stopID));
    std::vector<std::string> result;
    for (const SUMOVehicle* veh : wire->getChargingVehicles()) {
        result.push_back(veh->getID());
    }
    return result;
}



std::string
OverheadWire::getParameter(const std::string& stopID, const std::string& param) {
    const MSStoppingPlace* s = getOverheadWire(stopID);
    return s->getParameter(param, "");
}

LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(OverheadWire)

void
OverheadWire::setParameter(const std::string& stopID, const std::string& key, const std::string& value) {
    MSStoppingPlace* s = getOverheadWire(stopID);
    s->setParameter(key, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(OverheadWire, OVERHEADWIRE)


MSStoppingPlace*
OverheadWire::getOverheadWire(const std::string& id) {
    MSStoppingPlace* s = MSNet::getInstance()->getStoppingPlace(id, SUMO_TAG_OVERHEAD_WIRE_SEGMENT);
    if (s == nullptr) {
        throw TraCIException("OverheadWire '" + id + "' is not known");
    }
    return s;
}


std::shared_ptr<VariableWrapper>
OverheadWire::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
OverheadWire::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
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
