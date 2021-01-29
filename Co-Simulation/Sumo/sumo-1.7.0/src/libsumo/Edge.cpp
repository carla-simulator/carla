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
/// @file    Edge.cpp
/// @author  Gregor Laemmel
/// @date    15.09.2017
///
// C++ TraCI client API implementation
/****************************************************************************/

#include <iterator>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdgeWeightsStorage.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/MSVehicle.h>
#include <libsumo/TraCIDefs.h>
#include <libsumo/TraCIConstants.h>
#include <utils/emissions/HelpersHarmonoise.h>
#include "Edge.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults Edge::mySubscriptionResults;
ContextSubscriptionResults Edge::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
Edge::getIDList() {
    std::vector<std::string> ids;
    MSEdge::insertIDs(ids);
    return ids;
}


int
Edge::getIDCount() {
    return (int)getIDList().size();
}


double
Edge::getAdaptedTraveltime(const std::string& id, double time) {
    const MSEdge* e = getEdge(id);
    double value;
    if (!MSNet::getInstance()->getWeightsStorage().retrieveExistingTravelTime(e, time, value)) {
        return -1.;
    }
    return value;
}


double
Edge::getEffort(const std::string& id, double time) {
    const MSEdge* e = getEdge(id);
    double value;
    if (!MSNet::getInstance()->getWeightsStorage().retrieveExistingEffort(e, time, value)) {
        return -1.;
    }
    return value;
}


double
Edge::getTraveltime(const std::string& id) {
    return getEdge(id)->getCurrentTravelTime();
}


MSEdge*
Edge::getEdge(const std::string& id) {
    MSEdge* e = MSEdge::dictionary(id);
    if (e == nullptr) {
        throw TraCIException("Edge '" + id + "' is not known");
    }
    return e;
}


double
Edge::getWaitingTime(const std::string& id) {
    return getEdge(id)->getWaitingSeconds();
}


const std::vector<std::string>
Edge::getLastStepPersonIDs(const std::string& id) {
    std::vector<std::string> personIDs;
    std::vector<MSTransportable*> persons = getEdge(id)->getSortedPersons(MSNet::getInstance()->getCurrentTimeStep(), true);
    personIDs.reserve(persons.size());
    for (MSTransportable* p : persons) {
        personIDs.push_back(p->getID());
    }
    return personIDs;
}


const std::vector<std::string>
Edge::getLastStepVehicleIDs(const std::string& id) {
    std::vector<std::string> vehIDs;
    for (const SUMOVehicle* veh : getEdge(id)->getVehicles()) {
        vehIDs.push_back(veh->getID());
    }
    return vehIDs;
}


double
Edge::getCO2Emission(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getCO2Emissions();
    }
    return sum;
}


double
Edge::getCOEmission(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getCOEmissions();
    }
    return sum;
}


double
Edge::getHCEmission(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getHCEmissions();
    }
    return sum;
}


double
Edge::getPMxEmission(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getPMxEmissions();
    }
    return sum;
}


double
Edge::getNOxEmission(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getNOxEmissions();
    }
    return sum;
}


double
Edge::getFuelConsumption(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getFuelConsumption();
    }
    return sum;
}


double
Edge::getNoiseEmission(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += pow(10., (lane->getHarmonoise_NoiseEmissions() / 10.));
    }
    if (sum != 0) {
        return HelpersHarmonoise::sum(sum);
    }
    return sum;
}


double
Edge::getElectricityConsumption(const std::string& id) {
    double sum = 0;
    for (MSLane* lane : getEdge(id)->getLanes()) {
        sum += lane->getElectricityConsumption();
    }
    return sum;
}


int
Edge::getLastStepVehicleNumber(const std::string& id) {
    return getEdge(id)->getVehicleNumber();
}


double
Edge::getLastStepMeanSpeed(const std::string& id) {
    return getEdge(id)->getMeanSpeed();
}


double
Edge::getLastStepOccupancy(const std::string& id) {
    return getEdge(id)->getOccupancy();
}


int
Edge::getLastStepHaltingNumber(const std::string& id) {
    int result = 0;
    for (const SUMOVehicle* veh : getEdge(id)->getVehicles()) {
        if (veh->getSpeed() < SUMO_const_haltingSpeed) {
            result++;
        }
    }
    return result;
}


double
Edge::getLastStepLength(const std::string& id) {
    double lengthSum = 0;
    int numVehicles = 0;
    for (const SUMOVehicle* veh : getEdge(id)->getVehicles()) {
        numVehicles++;
        lengthSum += dynamic_cast<const MSBaseVehicle*>(veh)->getVehicleType().getLength();
    }
    if (numVehicles == 0) {
        return 0;
    }
    return lengthSum / numVehicles;
}


int
Edge::getLaneNumber(const std::string& id) {
    return (int)getEdge(id)->getLanes().size();
}


std::string
Edge::getStreetName(const std::string& id) {
    return getEdge(id)->getStreetName();
}


std::string
Edge::getParameter(const std::string& id, const std::string& paramName) {
    return getEdge(id)->getParameter(paramName, "");
}


LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(Edge)


void
Edge::setAllowedVehicleClasses(const std::string& id, std::vector<std::string> classes) {
    SVCPermissions permissions = parseVehicleClasses(classes);
    setAllowedSVCPermissions(id, permissions);
}


void
Edge::setDisallowedVehicleClasses(const std::string& id, std::vector<std::string> classes) {
    SVCPermissions permissions = invertPermissions(parseVehicleClasses(classes));
    setAllowedSVCPermissions(id, permissions);
}


void
Edge::setAllowedSVCPermissions(const std::string& id, int permissions) {
    MSEdge* e = getEdge(id);
    for (MSLane* lane : e->getLanes()) {
        lane->setPermissions(permissions, MSLane::CHANGE_PERMISSIONS_PERMANENT);
    }
    e->rebuildAllowedLanes();
    for (MSEdge* const pred : e->getPredecessors()) {
        pred->rebuildAllowedTargets();
    }
}


void
Edge::adaptTraveltime(const std::string& id, double value, double begTime, double endTime) {
    MSNet::getInstance()->getWeightsStorage().addTravelTime(getEdge(id), begTime, endTime, value);
}


void
Edge::setEffort(const std::string& id, double value, double begTime, double endTime) {
    MSNet::getInstance()->getWeightsStorage().addEffort(getEdge(id), begTime, endTime, value);
}


void
Edge::setMaxSpeed(const std::string& id, double value) {
    for (MSLane* lane : getEdge(id)->getLanes()) {
        lane->setMaxSpeed(value);
    }
}


void
Edge::setParameter(const std::string& id, const std::string& name, const std::string& value) {
    getEdge(id)->setParameter(name, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(Edge, EDGE)


void
Edge::storeShape(const std::string& id, PositionVector& shape) {
    const MSEdge* const e = getEdge(id);
    const std::vector<MSLane*>& lanes = e->getLanes();
    shape = lanes.front()->getShape();
    if (lanes.size() > 1) {
        copy(lanes.back()->getShape().begin(), lanes.back()->getShape().end(), back_inserter(shape));
    }
}


std::shared_ptr<VariableWrapper>
Edge::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
Edge::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_CURRENT_TRAVELTIME:
            return wrapper->wrapDouble(objID, variable, getTraveltime(objID));
        case VAR_WAITING_TIME:
            return wrapper->wrapDouble(objID, variable, getWaitingTime(objID));
        case LAST_STEP_PERSON_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getLastStepPersonIDs(objID));
        case LAST_STEP_VEHICLE_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getLastStepVehicleIDs(objID));
        case VAR_CO2EMISSION:
            return wrapper->wrapDouble(objID, variable, getCO2Emission(objID));
        case VAR_COEMISSION:
            return wrapper->wrapDouble(objID, variable, getCOEmission(objID));
        case VAR_HCEMISSION:
            return wrapper->wrapDouble(objID, variable, getHCEmission(objID));
        case VAR_PMXEMISSION:
            return wrapper->wrapDouble(objID, variable, getPMxEmission(objID));
        case VAR_NOXEMISSION:
            return wrapper->wrapDouble(objID, variable, getNOxEmission(objID));
        case VAR_FUELCONSUMPTION:
            return wrapper->wrapDouble(objID, variable, getFuelConsumption(objID));
        case VAR_NOISEEMISSION:
            return wrapper->wrapDouble(objID, variable, getNoiseEmission(objID));
        case VAR_ELECTRICITYCONSUMPTION:
            return wrapper->wrapDouble(objID, variable, getElectricityConsumption(objID));
        case LAST_STEP_VEHICLE_NUMBER:
            return wrapper->wrapInt(objID, variable, getLastStepVehicleNumber(objID));
        case LAST_STEP_MEAN_SPEED:
            return wrapper->wrapDouble(objID, variable, getLastStepMeanSpeed(objID));
        case LAST_STEP_OCCUPANCY:
            return wrapper->wrapDouble(objID, variable, getLastStepOccupancy(objID));
        case LAST_STEP_VEHICLE_HALTING_NUMBER:
            return wrapper->wrapInt(objID, variable, getLastStepHaltingNumber(objID));
        case LAST_STEP_LENGTH:
            return wrapper->wrapDouble(objID, variable, getLastStepLength(objID));
        case VAR_LANE_INDEX:
            return wrapper->wrapInt(objID, variable, getLaneNumber(objID));
        case VAR_NAME:
            return wrapper->wrapString(objID, variable, getStreetName(objID));
        default:
            return false;
    }
}

}


/****************************************************************************/
