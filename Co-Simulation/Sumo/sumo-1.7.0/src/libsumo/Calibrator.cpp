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
/// @file    Calibrator.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/output/MSRouteProbe.h>
#include <microsim/trigger/MSCalibrator.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "Calibrator.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults Calibrator::mySubscriptionResults;
ContextSubscriptionResults Calibrator::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
Calibrator::getIDList() {
    std::vector<std::string> ids;
    for (auto& item : MSCalibrator::getInstances()) {
        ids.push_back(item.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int
Calibrator::getIDCount() {
    return (int)getIDList().size();
}

std::string
Calibrator::getEdgeID(const std::string& calibratorID) {
    return getCalibrator(calibratorID)->getEdge()->getID();
}

std::string
Calibrator::getLaneID(const std::string& calibratorID) {
    const MSLane* lane = getCalibrator(calibratorID)->getLane();
    if (lane == nullptr) {
        return "";
    } else {
        return lane->getID();
    }
}

double
Calibrator::getVehsPerHour(const std::string& calibratorID) {
    return getCalibratorState(getCalibrator(calibratorID)).q;
}

double
Calibrator::getSpeed(const std::string& calibratorID) {
    return getCalibratorState(getCalibrator(calibratorID)).v;
}

std::string
Calibrator::getTypeID(const std::string& calibratorID) {
    return getCalibratorState(getCalibrator(calibratorID)).vehicleParameter->vtypeid;
}

double
Calibrator::getBegin(const std::string& calibratorID) {
    return STEPS2TIME(getCalibratorState(getCalibrator(calibratorID)).begin);
}

double
Calibrator::getEnd(const std::string& calibratorID) {
    return STEPS2TIME(getCalibratorState(getCalibrator(calibratorID)).end);
}

std::string
Calibrator::getRouteID(const std::string& calibratorID) {
    return getCalibratorState(getCalibrator(calibratorID)).vehicleParameter->routeid;
}

std::string
Calibrator::getRouteProbeID(const std::string& calibratorID) {
    const MSRouteProbe* rp = getCalibrator(calibratorID)->getRouteProbe();
    if (rp == nullptr) {
        return "";
    } else {
        return rp->getID();
    }
}

std::vector<std::string>
Calibrator::getVTypes(const std::string& calibratorID) {
    std::vector<std::string> result;
    const std::set<std::string>& vTypes = getCalibrator(calibratorID)->getVehicleTypes();
    result.insert(result.begin(), vTypes.begin(), vTypes.end());
    std::sort(result.begin(), result.end());
    return result;
}


int
Calibrator::getPassed(const std::string& calibratorID) {
    return getCalibrator(calibratorID)->passed();
}

int
Calibrator::getInserted(const std::string& calibratorID) {
    return getCalibrator(calibratorID)->getInserted();
}

int
Calibrator::getRemoved(const std::string& calibratorID) {
    return getCalibrator(calibratorID)->getRemoved();
}

std::string
Calibrator::getParameter(const std::string& calibratorID, const std::string& param) {
    const MSCalibrator* c = getCalibrator(calibratorID);
    return c->getParameter(param, "");
}

LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(Calibrator)

void
Calibrator::setParameter(const std::string& calibratorID, const std::string& key, const std::string& value) {
    MSCalibrator* c = getCalibrator(calibratorID);
    c->setParameter(key, value);
}

void
Calibrator::setFlow(const std::string& calibratorID, double begin, double end, double vehsPerHour, double speed, const std::string& typeID,
                    const std::string& routeID,
                    const std::string& departLane,
                    const std::string& departSpeed) {
    std::string error;
    SUMOVehicleParameter vehicleParams;
    vehicleParams.vtypeid = typeID;
    vehicleParams.routeid = routeID;
    MSVehicleType* t = MSNet::getInstance()->getVehicleControl().getVType(typeID);
    if (t == nullptr) {
        throw TraCIException("Vehicle type '" + typeID + "' is not known");
    }
    if (!SUMOVehicleParameter::parseDepartLane(departLane, "calibrator", calibratorID, vehicleParams.departLane, vehicleParams.departLaneProcedure, error)) {
        throw TraCIException(error);
    }
    if (!SUMOVehicleParameter::parseDepartSpeed(departSpeed, "calibrator", calibratorID, vehicleParams.departSpeed, vehicleParams.departSpeedProcedure, error)) {
        throw TraCIException(error);
    }
    getCalibrator(calibratorID)->setFlow(TIME2STEPS(begin), TIME2STEPS(end), vehsPerHour, speed, vehicleParams);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(Calibrator, CALIBRATOR)


MSCalibrator*
Calibrator::getCalibrator(const std::string& id) {
    const auto& dict = MSCalibrator::getInstances();
    auto it = dict.find(id);
    if (it == dict.end()) {
        throw TraCIException("Calibrator '" + id + "' is not known");
    }
    return it->second;
}

MSCalibrator::AspiredState
Calibrator::getCalibratorState(const MSCalibrator* c) {
    try {
        return c->getCurrentStateInterval();
    } catch (ProcessError& e) {
        throw TraCIException(e.what());
    }
}

std::shared_ptr<VariableWrapper>
Calibrator::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
Calibrator::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_ROAD_ID:
            return wrapper->wrapString(objID, variable, getEdgeID(objID));
        case VAR_LANE_ID:
            return wrapper->wrapString(objID, variable, getLaneID(objID));
        case VAR_VEHSPERHOUR:
            return wrapper->wrapDouble(objID, variable, getVehsPerHour(objID));
        case VAR_SPEED:
            return wrapper->wrapDouble(objID, variable, getSpeed(objID));
        case VAR_TYPE:
            return wrapper->wrapString(objID, variable, getTypeID(objID));
        case VAR_BEGIN:
            return wrapper->wrapDouble(objID, variable, getBegin(objID));
        case VAR_END:
            return wrapper->wrapDouble(objID, variable, getEnd(objID));
        case VAR_ROUTE_ID:
            return wrapper->wrapString(objID, variable, getRouteID(objID));
        case VAR_ROUTE_PROBE:
            return wrapper->wrapString(objID, variable, getRouteProbeID(objID));
        case VAR_VTYPES:
            return wrapper->wrapStringList(objID, variable, getVTypes(objID));
        case VAR_PASSED:
            return wrapper->wrapInt(objID, variable, getPassed(objID));
        case VAR_INSERTED:
            return wrapper->wrapInt(objID, variable, getInserted(objID));
        case VAR_REMOVED:
            return wrapper->wrapInt(objID, variable, getRemoved(objID));
        default:
            return false;
    }
}

}


/****************************************************************************/
