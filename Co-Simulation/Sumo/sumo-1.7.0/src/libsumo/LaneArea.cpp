/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    LaneArea.cpp
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    30.05.2012
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/output/MSDetectorControl.h>
#include <microsim/output/MSE2Collector.h>
#include <microsim/MSNet.h>
#include <libsumo/TraCIConstants.h>
#include "LaneArea.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults LaneArea::mySubscriptionResults;
ContextSubscriptionResults LaneArea::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
LaneArea::getIDList() {
    std::vector<std::string> ids;
    MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_LANE_AREA_DETECTOR).insertIDs(ids);
    return ids;
}


int
LaneArea::getIDCount() {
    std::vector<std::string> ids;
    return (int)MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_LANE_AREA_DETECTOR).size();
}


int
LaneArea::getJamLengthVehicle(const std::string& detID) {
    return getDetector(detID)->getCurrentJamLengthInVehicles();
}


double
LaneArea::getJamLengthMeters(const std::string& detID) {
    return getDetector(detID)->getCurrentJamLengthInMeters();
}


double
LaneArea::getLastStepMeanSpeed(const std::string& detID) {
    return getDetector(detID)->getCurrentMeanSpeed();
}


std::vector<std::string>
LaneArea::getLastStepVehicleIDs(const std::string& detID) {
    return getDetector(detID)->getCurrentVehicleIDs();
}


double
LaneArea::getLastStepOccupancy(const std::string& detID) {
    return getDetector(detID)->getCurrentOccupancy();
}


double
LaneArea::getPosition(const std::string& detID) {
    return getDetector(detID)->getStartPos();
}


std::string
LaneArea::getLaneID(const std::string& detID) {
    return getDetector(detID)->getLane()->getID();
}


double
LaneArea::getLength(const std::string& detID) {
    const MSE2Collector* const e2 = getDetector(detID);
    return e2->getLength();
}


int
LaneArea::getLastStepVehicleNumber(const std::string& detID) {
    return getDetector(detID)->getCurrentVehicleNumber();
}


int
LaneArea::getLastStepHaltingNumber(const std::string& detID) {
    return getDetector(detID)->getCurrentHaltingNumber();
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(LaneArea, LANEAREA)


MSE2Collector*
LaneArea::getDetector(const std::string& id) {
    MSE2Collector* e2 = dynamic_cast<MSE2Collector*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_LANE_AREA_DETECTOR).get(id));
    if (e2 == nullptr) {
        throw TraCIException("Lane area detector '" + id + "' is not known");
    }
    return e2;
}


std::shared_ptr<VariableWrapper>
LaneArea::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
LaneArea::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case LAST_STEP_VEHICLE_NUMBER:
            return wrapper->wrapInt(objID, variable, getLastStepVehicleNumber(objID));
        case LAST_STEP_MEAN_SPEED:
            return wrapper->wrapDouble(objID, variable, getLastStepMeanSpeed(objID));
        case LAST_STEP_VEHICLE_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getLastStepVehicleIDs(objID));
        case LAST_STEP_VEHICLE_HALTING_NUMBER:
            return wrapper->wrapInt(objID, variable, getLastStepHaltingNumber(objID));
        case JAM_LENGTH_VEHICLE:
            return wrapper->wrapInt(objID, variable, getJamLengthVehicle(objID));
        case JAM_LENGTH_METERS:
            return wrapper->wrapDouble(objID, variable, getJamLengthMeters(objID));
        case LAST_STEP_OCCUPANCY:
            return wrapper->wrapDouble(objID, variable, getLastStepOccupancy(objID));
        case VAR_POSITION:
            return wrapper->wrapDouble(objID, variable, getPosition(objID));
        case VAR_LANE_ID:
            return wrapper->wrapString(objID, variable, getLaneID(objID));
        case VAR_LENGTH:
            return wrapper->wrapDouble(objID, variable, getLength(objID));
        default:
            return false;
    }
}


}


/****************************************************************************/
