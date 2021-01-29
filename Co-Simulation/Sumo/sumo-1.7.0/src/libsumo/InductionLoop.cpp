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
/// @file    InductionLoop.cpp
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
#include <microsim/output/MSInductLoop.h>
#include <microsim/MSNet.h>
#include <libsumo/TraCIDefs.h>
#include <libsumo/TraCIConstants.h>
#include "InductionLoop.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults InductionLoop::mySubscriptionResults;
ContextSubscriptionResults InductionLoop::myContextSubscriptionResults;
NamedRTree* InductionLoop::myTree(nullptr);


// ===========================================================================
// member definitions
// ===========================================================================
std::vector<std::string>
InductionLoop::getIDList() {
    std::vector<std::string> ids;
    MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).insertIDs(ids);
    return ids;
}


int
InductionLoop::getIDCount() {
    std::vector<std::string> ids;
    return (int)MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).size();
}


double
InductionLoop::getPosition(const std::string& detID) {
    return getDetector(detID)->getPosition();
}


std::string
InductionLoop::getLaneID(const std::string& detID) {
    return getDetector(detID)->getLane()->getID();
}


int
InductionLoop::getLastStepVehicleNumber(const std::string& detID) {
    return (int)getDetector(detID)->getEnteredNumber((int)DELTA_T);
}


double
InductionLoop::getLastStepMeanSpeed(const std::string& detID) {
    return getDetector(detID)->getSpeed((int)DELTA_T);
}


std::vector<std::string>
InductionLoop::getLastStepVehicleIDs(const std::string& detID) {
    return getDetector(detID)->getVehicleIDs((int)DELTA_T);
}


double
InductionLoop::getLastStepOccupancy(const std::string& detID) {
    return getDetector(detID)->getOccupancy();
}


double
InductionLoop::getLastStepMeanLength(const std::string& detID) {
    return getDetector(detID)->getVehicleLength((int)DELTA_T);
}


double
InductionLoop::getTimeSinceDetection(const std::string& detID) {
    return getDetector(detID)->getTimeSinceLastDetection();
}


std::vector<libsumo::TraCIVehicleData>
InductionLoop::getVehicleData(const std::string& detID) {
    const std::vector<MSInductLoop::VehicleData> vd = getDetector(detID)->collectVehiclesOnDet(SIMSTEP - DELTA_T, true, true);
    std::vector<libsumo::TraCIVehicleData> tvd;
    for (const MSInductLoop::VehicleData& vdi : vd) {
        tvd.push_back(libsumo::TraCIVehicleData());
        tvd.back().id = vdi.idM;
        tvd.back().length = vdi.lengthM;
        tvd.back().entryTime = vdi.entryTimeM;
        tvd.back().leaveTime = vdi.leaveTimeM;
        tvd.back().typeID = vdi.typeIDM;
    }
    return tvd;
}


MSInductLoop*
InductionLoop::getDetector(const std::string& id) {
    MSInductLoop* il = dynamic_cast<MSInductLoop*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).get(id));
    if (il == nullptr) {
        throw TraCIException("Induction loop '" + id + "' is not known");
    }
    return il;
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(InductionLoop, INDUCTIONLOOP)


NamedRTree*
InductionLoop::getTree() {
    if (myTree == nullptr) {
        myTree = new NamedRTree();
        for (const auto& i : MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP)) {
            MSInductLoop* il = static_cast<MSInductLoop*>(i.second);
            Position p = il->getLane()->getShape().positionAtOffset(il->getPosition());
            const float cmin[2] = {(float) p.x(), (float) p.y()};
            const float cmax[2] = {(float) p.x(), (float) p.y()};
            myTree->Insert(cmin, cmax, il);
        }
    }
    return myTree;
}

void
InductionLoop::cleanup() {
    delete myTree;
    myTree = nullptr;
}

void
InductionLoop::storeShape(const std::string& id, PositionVector& shape) {
    MSInductLoop* const il = getDetector(id);
    shape.push_back(il->getLane()->getShape().positionAtOffset(il->getPosition()));
}


std::shared_ptr<VariableWrapper>
InductionLoop::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
InductionLoop::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_POSITION:
            return wrapper->wrapDouble(objID, variable, getPosition(objID));
        case VAR_LANE_ID:
            return wrapper->wrapString(objID, variable, getLaneID(objID));
        case LAST_STEP_VEHICLE_NUMBER:
            return wrapper->wrapInt(objID, variable, getLastStepVehicleNumber(objID));
        case LAST_STEP_MEAN_SPEED:
            return wrapper->wrapDouble(objID, variable, getLastStepMeanSpeed(objID));
        case LAST_STEP_VEHICLE_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getLastStepVehicleIDs(objID));
        case LAST_STEP_OCCUPANCY:
            return wrapper->wrapDouble(objID, variable, getLastStepOccupancy(objID));
        case LAST_STEP_LENGTH:
            return wrapper->wrapDouble(objID, variable, getLastStepMeanLength(objID));
        case LAST_STEP_TIME_SINCE_DETECTION:
            return wrapper->wrapDouble(objID, variable, getTimeSinceDetection(objID));
        default:
            return false;
    }
}


}


/****************************************************************************/
