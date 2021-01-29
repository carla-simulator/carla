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
/// @file    Person.cpp
/// @author  Leonhard Luecken
/// @date    15.09.2017
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSNet.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/transportables/MSStageDriving.h>
#include <microsim/devices/MSDevice_Taxi.h>
#include <microsim/devices/MSDispatch_TraCI.h>
#include <libsumo/TraCIConstants.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/SUMOTime.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/router/PedestrianRouter.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include "VehicleType.h"
#include "Person.h"

#define FAR_AWAY 1000.0

//#define DEBUG_MOVEXY
//#define DEBUG_MOVEXY_ANGLE

namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults Person::mySubscriptionResults;
ContextSubscriptionResults Person::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
Person::getIDList() {
    MSTransportableControl& c = MSNet::getInstance()->getPersonControl();
    std::vector<std::string> ids;
    for (MSTransportableControl::constVehIt i = c.loadedBegin(); i != c.loadedEnd(); ++i) {
        if (i->second->getCurrentStageType() != MSStageType::WAITING_FOR_DEPART) {
            ids.push_back(i->first);
        }
    }
    return ids;
}


int
Person::getIDCount() {
    return MSNet::getInstance()->getPersonControl().size();
}


TraCIPosition
Person::getPosition(const std::string& personID, const bool includeZ) {
    return Helper::makeTraCIPosition(getPerson(personID)->getPosition(), includeZ);
}


TraCIPosition
Person::getPosition3D(const std::string& personID) {
    return Helper::makeTraCIPosition(getPerson(personID)->getPosition(), true);
}


double
Person::getAngle(const std::string& personID) {
    return GeomHelper::naviDegree(getPerson(personID)->getAngle());
}


double
Person::getSlope(const std::string& personID) {
    MSPerson* person = getPerson(personID);
    const double ep = person->getEdgePos();
    const MSLane* lane = getSidewalk<MSEdge, MSLane>(person->getEdge());
    if (lane == nullptr) {
        lane = person->getEdge()->getLanes()[0];
    }
    const double gp = lane->interpolateLanePosToGeometryPos(ep);
    return lane->getShape().slopeDegreeAtOffset(gp);
}


double
Person::getSpeed(const std::string& personID) {
    return getPerson(personID)->getSpeed();
}


std::string
Person::getRoadID(const std::string& personID) {
    return getPerson(personID)->getEdge()->getID();
}


std::string
Person::getLaneID(const std::string& personID) {
    return Named::getIDSecure(getPerson(personID)->getLane(), "");
}


double
Person::getLanePosition(const std::string& personID) {
    return getPerson(personID)->getEdgePos();
}

std::vector<TraCIReservation>
Person::getTaxiReservations(int onlyNew) {
    std::vector<TraCIReservation> result;
    MSDispatch* dispatcher = MSDevice_Taxi::getDispatchAlgorithm();
    if (dispatcher != nullptr) {
        MSDispatch_TraCI* traciDispatcher = dynamic_cast<MSDispatch_TraCI*>(dispatcher);
        if (traciDispatcher == nullptr) {
            throw TraCIException("device.taxi.dispatch-algorithm 'traci' has not been loaded");
        }
        for (Reservation* res : dispatcher->getReservations()) {
            if (onlyNew != 0) {
                if (res->recheck != SUMOTime_MAX) {
                    continue;
                }
                // reservations become the responsibility of the traci client
                res->recheck = SUMOTime_MAX;
            }
            std::vector<std::string> personIDs;
            for (MSTransportable* p : res->persons) {
                personIDs.push_back(p->getID());
            }
            result.push_back(TraCIReservation(traciDispatcher->getReservationID(res),
                                              personIDs,
                                              res->group,
                                              res->from->getID(),
                                              res->to->getID(),
                                              res->fromPos,
                                              res->toPos,
                                              STEPS2TIME(res->pickupTime),
                                              STEPS2TIME(res->reservationTime)
                                             ));
        }
    }
    return result;
}


TraCIColor
Person::getColor(const std::string& personID) {
    const RGBColor& col = getPerson(personID)->getParameter().color;
    TraCIColor tcol;
    tcol.r = col.red();
    tcol.g = col.green();
    tcol.b = col.blue();
    tcol.a = col.alpha();
    return tcol;
}


std::string
Person::getTypeID(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getID();
}


double
Person::getWaitingTime(const std::string& personID) {
    return getPerson(personID)->getWaitingSeconds();
}


std::string
Person::getNextEdge(const std::string& personID) {
    return getPerson(personID)->getNextEdge();
}


std::vector<std::string>
Person::getEdges(const std::string& personID, int nextStageIndex) {
    MSTransportable* p = getPerson(personID);
    if (nextStageIndex >= p->getNumRemainingStages()) {
        throw TraCIException("The stage index must be lower than the number of remaining stages.");
    }
    if (nextStageIndex < (p->getNumRemainingStages() - p->getNumStages())) {
        throw TraCIException("The negative stage index must refer to a valid previous stage.");
    }
    std::vector<std::string> edgeIDs;
    for (auto& e : p->getEdges(nextStageIndex)) {
        if (e != nullptr) {
            edgeIDs.push_back(e->getID());
        }
    }
    return edgeIDs;
}


TraCIStage
Person::getStage(const std::string& personID, int nextStageIndex) {
    MSTransportable* p = getPerson(personID);
    TraCIStage result;
    if (nextStageIndex >= p->getNumRemainingStages()) {
        throw TraCIException("The stage index must be lower than the number of remaining stages.");
    }
    if (nextStageIndex < (p->getNumRemainingStages() - p->getNumStages())) {
        throw TraCIException("The negative stage index " + toString(nextStageIndex) + " must refer to a valid previous stage.");
    }
    //stageType, arrivalPos, edges, destStop, vType, and description can be retrieved directly from the base Stage class.
    MSStage* stage = p->getNextStage(nextStageIndex);
    result.type = (int)stage->getStageType();
    result.arrivalPos = stage->getArrivalPos();
    for (auto e : stage->getEdges()) {
        if (e != nullptr) {
            result.edges.push_back(e->getID());
        }
    }
    MSStoppingPlace* destinationStop = stage->getDestinationStop();
    if (destinationStop != nullptr) {
        result.destStop = destinationStop->getID();
    }
    result.description = stage->getStageDescription(p->isPerson());
    result.length = stage->getDistance();
    if (result.length == -1.) {
        result.length = INVALID_DOUBLE_VALUE;
    }
    result.departPos = INVALID_DOUBLE_VALUE;
    result.cost = INVALID_DOUBLE_VALUE;
    result.travelTime = INVALID_DOUBLE_VALUE;
    result.depart = stage->getDeparted() >= 0 ? STEPS2TIME(stage->getDeparted()) : INVALID_DOUBLE_VALUE;
    result.travelTime = stage->getArrived() >= 0 ? STEPS2TIME(stage->getArrived() - stage->getDeparted()) : INVALID_DOUBLE_VALUE;
    // Some stage type dependant attributes
    switch (stage->getStageType()) {
        case MSStageType::DRIVING: {
            MSStageDriving* const drivingStage = static_cast<MSStageDriving*>(stage);
            result.vType = drivingStage->getVehicleType();
            result.intended = drivingStage->getIntendedVehicleID();
            if (result.depart < 0 && drivingStage->getIntendedDepart() >= 0) {
                result.depart = STEPS2TIME(drivingStage->getIntendedDepart());
            }
            const std::set<std::string> lines = drivingStage->getLines();
            for (auto line = lines.begin(); line != lines.end(); line++) {
                if (line != lines.begin()) {
                    result.line += " ";
                }
                result.line += *line;
            }
            break;
        }
        case MSStageType::WALKING: {
            auto* walkingStage = (MSPerson::MSPersonStage_Walking*) stage;
            result.departPos = walkingStage->getDepartPos();
            break;
        }
        default:
            break;
    }
    return result;
}


int
Person::getRemainingStages(const std::string& personID) {
    return getPerson(personID)->getNumRemainingStages();
}


std::string
Person::getVehicle(const std::string& personID) {
    const SUMOVehicle* veh = getPerson(personID)->getVehicle();
    if (veh == nullptr) {
        return "";
    } else {
        return veh->getID();
    }
}


std::string
Person::getParameter(const std::string& personID, const std::string& param) {
    return getPerson(personID)->getParameter().getParameter(param, "");
}


LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(Person)


std::string
Person::getEmissionClass(const std::string& personID) {
    return PollutantsInterface::getName(getPerson(personID)->getVehicleType().getEmissionClass());
}


std::string
Person::getShapeClass(const std::string& personID) {
    return getVehicleShapeName(getPerson(personID)->getVehicleType().getGuiShape());
}


double
Person::getLength(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getLength();
}


double
Person::getSpeedFactor(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getSpeedFactor().getParameter()[0];
}


double
Person::getAccel(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getCarFollowModel().getMaxAccel();
}


double
Person::getDecel(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getCarFollowModel().getMaxDecel();
}


double Person::getEmergencyDecel(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getCarFollowModel().getEmergencyDecel();
}


double Person::getApparentDecel(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getCarFollowModel().getApparentDecel();
}


double Person::getActionStepLength(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getActionStepLengthSecs();
}


double
Person::getTau(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getCarFollowModel().getHeadwayTime();
}


double
Person::getImperfection(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getCarFollowModel().getImperfection();
}


double
Person::getSpeedDeviation(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getSpeedFactor().getParameter()[1];
}


std::string
Person::getVehicleClass(const std::string& personID) {
    return toString(getPerson(personID)->getVehicleType().getVehicleClass());
}


double
Person::getMinGap(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getMinGap();
}


double
Person::getMinGapLat(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getMinGapLat();
}


double
Person::getMaxSpeed(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getMaxSpeed();
}


double
Person::getMaxSpeedLat(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getMaxSpeedLat();
}


std::string
Person::getLateralAlignment(const std::string& personID) {
    return toString(getPerson(personID)->getVehicleType().getPreferredLateralAlignment());
}


double
Person::getWidth(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getWidth();
}


double
Person::getHeight(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getHeight();
}


int
Person::getPersonCapacity(const std::string& personID) {
    return getPerson(personID)->getVehicleType().getPersonCapacity();
}


void
Person::setSpeed(const std::string& personID, double speed) {
    getPerson(personID)->setSpeed(speed);
}


void
Person::setType(const std::string& personID, const std::string& typeID) {
    MSVehicleType* vehicleType = MSNet::getInstance()->getVehicleControl().getVType(typeID);
    if (vehicleType == nullptr) {
        throw TraCIException("The vehicle type '" + typeID + "' is not known.");
    }
    getPerson(personID)->replaceVehicleType(vehicleType);
}


void
Person::add(const std::string& personID, const std::string& edgeID, double pos, double departInSecs, const std::string typeID) {
    MSTransportable* p;
    try {
        p = getPerson(personID);
    } catch (TraCIException&) {
        p = nullptr;
    }

    if (p != nullptr) {
        throw TraCIException("The person " + personID + " to add already exists.");
    }

    SUMOTime depart = TIME2STEPS(departInSecs);
    SUMOVehicleParameter vehicleParams;
    vehicleParams.id = personID;

    MSVehicleType* vehicleType = MSNet::getInstance()->getVehicleControl().getVType(typeID);
    if (!vehicleType) {
        throw TraCIException("Invalid type '" + typeID + "' for person '" + personID + "'");
    }

    const MSEdge* edge = MSEdge::dictionary(edgeID);
    if (!edge) {
        throw TraCIException("Invalid edge '" + edgeID + "' for person: '" + personID + "'");
    }

    if (departInSecs < 0.) {
        const int proc = (int) - departInSecs;
        if (proc >= static_cast<int>(DEPART_DEF_MAX)) {
            throw TraCIException("Invalid departure time." + toString(depart) + " " + toString(proc));
        }
        vehicleParams.departProcedure = (DepartDefinition)proc;
        vehicleParams.depart = MSNet::getInstance()->getCurrentTimeStep();
    } else if (depart < MSNet::getInstance()->getCurrentTimeStep()) {
        vehicleParams.depart = MSNet::getInstance()->getCurrentTimeStep();
        WRITE_WARNING("Departure time " + toString(departInSecs) + " for person '" + personID
                      + "' is in the past; using current time " + time2string(vehicleParams.depart) + " instead.");
    } else {
        vehicleParams.depart = depart;
    }

    vehicleParams.departPosProcedure = DepartPosDefinition::GIVEN;
    if (fabs(pos) > edge->getLength()) {
        throw TraCIException("Invalid departure position.");
    }
    if (pos < 0) {
        pos += edge->getLength();
    }
    vehicleParams.departPos = pos;

    SUMOVehicleParameter* params = new SUMOVehicleParameter(vehicleParams);
    MSTransportable::MSTransportablePlan* plan = new MSTransportable::MSTransportablePlan();
    plan->push_back(new MSStageWaiting(edge, nullptr, 0, depart, pos, "awaiting departure", true));

    try {
        MSTransportable* person = MSNet::getInstance()->getPersonControl().buildPerson(params, vehicleType, plan, nullptr);
        MSNet::getInstance()->getPersonControl().add(person);
    } catch (ProcessError& e) {
        delete params;
        delete plan;
        throw TraCIException(e.what());
    }
}

MSStage*
Person::convertTraCIStage(const TraCIStage& stage, const std::string personID) {
    MSStoppingPlace* bs = nullptr;
    if (!stage.destStop.empty()) {
        bs = MSNet::getInstance()->getStoppingPlace(stage.destStop, SUMO_TAG_BUS_STOP);
        if (bs == nullptr) {
            bs = MSNet::getInstance()->getStoppingPlace(stage.destStop, SUMO_TAG_PARKING_AREA);
            if (bs == nullptr) {
                throw TraCIException("Invalid stopping place id '" + stage.destStop + "' for person: '" + personID + "'");
            } else {
                // parkingArea is not a proper arrival place
                bs = nullptr;
            }
        }
    }
    switch (stage.type) {
        case STAGE_DRIVING: {
            if (stage.edges.empty()) {
                throw TraCIException("The stage should have at least one edge");
            }
            std::string edgeId = stage.edges.back();
            MSEdge* edge = MSEdge::dictionary(edgeId);
            if (!edge) {
                throw TraCIException("Invalid edge '" + edgeId + "' for person: '" + personID + "'");
            }
            if (stage.line.empty()) {
                throw TraCIException("Empty lines parameter for person: '" + personID + "'");
            }
            return new MSStageDriving(edge, bs, edge->getLength() - NUMERICAL_EPS, StringTokenizer(stage.line).getVector());
        }

        case STAGE_WALKING: {
            MSTransportable* p = getPerson(personID);
            ConstMSEdgeVector edges;
            try {
                MSEdge::parseEdgesList(stage.edges, edges, "<unknown>");
            } catch (ProcessError& e) {
                throw TraCIException(e.what());
            }
            if (edges.empty()) {
                throw TraCIException("Empty edge list for walking stage of person '" + personID + "'.");
            }
            double arrivalPos = stage.arrivalPos;
            if (fabs(arrivalPos) > edges.back()->getLength()) {
                throw TraCIException("Invalid arrivalPos for walking stage of person '" + personID + "'.");
            }
            if (arrivalPos < 0) {
                arrivalPos += edges.back()->getLength();
            }
            double speed = p->getVehicleType().getMaxSpeed();
            return new MSPerson::MSPersonStage_Walking(p->getID(), edges, bs, -1, speed, p->getArrivalPos(), arrivalPos, 0);
        }

        case STAGE_WAITING: {
            MSTransportable* p = getPerson(personID);
            if (stage.travelTime < 0) {
                throw TraCIException("Duration for person: '" + personID + "' must not be negative");
            }
            return new MSStageWaiting(p->getArrivalEdge(), nullptr, TIME2STEPS(stage.travelTime), 0, p->getArrivalPos(), stage.description, false);
        }
        default:
            return nullptr;
    }
}


void
Person::appendStage(const std::string& personID, const TraCIStage& stage) {
    MSTransportable* p = getPerson(personID);
    MSStage* personStage = convertTraCIStage(stage, personID);
    p->appendStage(personStage);
}


void
Person::replaceStage(const std::string& personID, const int stageIndex, const TraCIStage& stage) {
    MSTransportable* p = getPerson(personID);
    if (stageIndex >= p->getNumRemainingStages()) {
        throw TraCIException("Specified stage index:  is not valid for person " + personID);
    }
    MSStage* personStage = convertTraCIStage(stage, personID);
    // removing the current stage triggers abort+proceed so the replacement
    // stage must be ready beforehand
    p->appendStage(personStage, stageIndex + 1);
    p->removeStage(stageIndex);
}


void
Person::appendDrivingStage(const std::string& personID, const std::string& toEdge, const std::string& lines, const std::string& stopID) {
    MSTransportable* p = getPerson(personID);
    const MSEdge* edge = MSEdge::dictionary(toEdge);
    if (!edge) {
        throw TraCIException("Invalid edge '" + toEdge + "' for person: '" + personID + "'");
    }
    if (lines.size() == 0) {
        return throw TraCIException("Empty lines parameter for person: '" + personID + "'");
    }
    MSStoppingPlace* bs = nullptr;
    if (stopID != "") {
        bs = MSNet::getInstance()->getStoppingPlace(stopID, SUMO_TAG_BUS_STOP);
        if (bs == nullptr) {
            throw TraCIException("Invalid stopping place id '" + stopID + "' for person: '" + personID + "'");
        }
    }
    p->appendStage(new MSStageDriving(edge, bs, edge->getLength() - NUMERICAL_EPS, StringTokenizer(lines).getVector()));
}


void
Person::appendWaitingStage(const std::string& personID, double duration, const std::string& description, const std::string& stopID) {
    MSTransportable* p = getPerson(personID);
    if (duration < 0) {
        throw TraCIException("Duration for person: '" + personID + "' must not be negative");
    }
    MSStoppingPlace* bs = nullptr;
    if (stopID != "") {
        bs = MSNet::getInstance()->getStoppingPlace(stopID, SUMO_TAG_BUS_STOP);
        if (bs == nullptr) {
            throw TraCIException("Invalid stopping place id '" + stopID + "' for person: '" + personID + "'");
        }
    }
    p->appendStage(new MSStageWaiting(p->getArrivalEdge(), nullptr, TIME2STEPS(duration), 0, p->getArrivalPos(), description, false));
}


void
Person::appendWalkingStage(const std::string& personID, const std::vector<std::string>& edgeIDs, double arrivalPos, double duration, double speed, const std::string& stopID) {
    MSTransportable* p = getPerson(personID);
    ConstMSEdgeVector edges;
    try {
        MSEdge::parseEdgesList(edgeIDs, edges, "<unknown>");
    } catch (ProcessError& e) {
        throw TraCIException(e.what());
    }
    if (edges.empty()) {
        throw TraCIException("Empty edge list for walking stage of person '" + personID + "'.");
    }
    if (fabs(arrivalPos) > edges.back()->getLength()) {
        throw TraCIException("Invalid arrivalPos for walking stage of person '" + personID + "'.");
    }
    if (arrivalPos < 0) {
        arrivalPos += edges.back()->getLength();
    }
    if (speed < 0) {
        speed = p->getVehicleType().getMaxSpeed();
    }
    MSStoppingPlace* bs = nullptr;
    if (stopID != "") {
        bs = MSNet::getInstance()->getStoppingPlace(stopID, SUMO_TAG_BUS_STOP);
        if (bs == nullptr) {
            throw TraCIException("Invalid stopping place id '" + stopID + "' for person: '" + personID + "'");
        }
    }
    p->appendStage(new MSPerson::MSPersonStage_Walking(p->getID(), edges, bs, TIME2STEPS(duration), speed, p->getArrivalPos(), arrivalPos, 0));
}


void
Person::removeStage(const std::string& personID, int nextStageIndex) {
    MSTransportable* p = getPerson(personID);
    if (nextStageIndex >= p->getNumRemainingStages()) {
        throw TraCIException("The stage index must be lower than the number of remaining stages.");
    }
    if (nextStageIndex < 0) {
        throw TraCIException("The stage index may not be negative.");
    }
    p->removeStage(nextStageIndex);
}


void
Person::rerouteTraveltime(const std::string& personID) {
    MSPerson* p = getPerson(personID);
    if (p->getNumRemainingStages() == 0) {
        throw TraCIException("Person '" + personID + "' has no remaining stages.");
    }
    const MSEdge* from = p->getEdge();
    double  departPos = p->getEdgePos();
    // reroute to the start of the next-non-walking stage
    int firstIndex;
    if (p->getCurrentStageType() == MSStageType::WALKING) {
        firstIndex = 0;
    } else if (p->getCurrentStageType() == MSStageType::WAITING) {
        if (p->getNumRemainingStages() < 2 || p->getStageType(1) != MSStageType::WALKING) {
            throw TraCIException("Person '" + personID + "' cannot reroute after the current stop.");
        }
        firstIndex = 1;
    } else {
        throw TraCIException("Person '" + personID + "' cannot reroute in stage type '" + toString((int)p->getCurrentStageType()) + "'.");
    }
    int nextIndex = firstIndex + 1;
    for (; nextIndex < p->getNumRemainingStages(); nextIndex++) {
        if (p->getStageType(nextIndex) != MSStageType::WALKING) {
            break;
        }
    }
    MSStage* destStage = p->getNextStage(nextIndex - 1);
    const MSEdge* to = destStage->getEdges().back();
    double arrivalPos = destStage->getArrivalPos();
    double speed = p->getVehicleType().getMaxSpeed();
    ConstMSEdgeVector newEdges;
    MSNet::getInstance()->getPedestrianRouter(0).compute(from, to, departPos, arrivalPos, speed, 0, nullptr, newEdges);
    if (newEdges.empty()) {
        throw TraCIException("Could not find new route for person '" + personID + "'.");
    }
    ConstMSEdgeVector oldEdges = p->getEdges(firstIndex);
    assert(!oldEdges.empty());
    if (oldEdges.front()->getFunction() != SumoXMLEdgeFunc::NORMAL) {
        oldEdges.erase(oldEdges.begin());
    }
    //std::cout << " remainingStages=" << p->getNumRemainingStages() << " oldEdges=" << toString(oldEdges) << " newEdges=" << toString(newEdges) << " firstIndex=" << firstIndex << " nextIndex=" << nextIndex << "\n";
    if (newEdges == oldEdges && (firstIndex + 1 == nextIndex)) {
        return;
    }
    if (newEdges.front() != from) {
        // @note: maybe this should be done automatically by the router
        newEdges.insert(newEdges.begin(), from);
    }
    p->reroute(newEdges, departPos, firstIndex, nextIndex);
}


void
Person::moveTo(const std::string& personID, const std::string& edgeID, double /* position */) {
    MSPerson* p = getPerson(personID);
    MSEdge* e = MSEdge::dictionary(edgeID);
    if (e == nullptr) {
        throw TraCIException("Unknown edge '" + edgeID + "'.");
    }
    /*
    switch (p->getStageType(0)) {
       case MSTransportable::MOVING_WITHOUT_VEHICLE: {
           MSPerson::MSPersonStage_Walking* s = dynamic_cast<MSPerson::MSPersonStage_Walking*>(p->getCurrentStage());
            assert(s != 0);
            const std::string error = s->moveTo(p, Simulation::getCurrentTime());
            if (error != "") {
                throw TraCIException("Command moveTo failed for person '" + personID + "' (" + error + ").");
            }
            break;
        }
        default:
        */
    throw TraCIException("Command moveTo is not supported for person '" + personID + "' while " + p->getCurrentStageDescription() + ".");
    //}
}


void
Person::moveToXY(const std::string& personID, const std::string& edgeID, const double x, const double y, double angle, const int keepRoute) {
    MSPerson* p = getPerson(personID);
    const bool doKeepRoute = (keepRoute & 1) != 0;
    const bool mayLeaveNetwork = (keepRoute & 2) != 0;
    const bool ignorePermissions = (keepRoute & 4) != 0;
    SUMOVehicleClass vClass = ignorePermissions ? SVC_IGNORING : p->getVClass();
    Position pos(x, y);
#ifdef DEBUG_MOVEXY
    const double origAngle = angle;
#endif
    // angle must be in [0,360] because it will be compared against those returned by naviDegree()
    // angle set to INVALID_DOUBLE_VALUE is ignored in the evaluated and later set to the angle of the matched lane
    if (angle != INVALID_DOUBLE_VALUE) {
        while (angle >= 360.) {
            angle -= 360.;
        }
        while (angle < 0.) {
            angle += 360.;
        }
    }
    Position currentPos = p->getPosition();
#ifdef DEBUG_MOVEXY
    std::cout << std::endl << "begin person " << p->getID() << " lanePos:" << p->getEdgePos() << " edge:" << Named::getIDSecure(p->getEdge()) << "\n";
    std::cout << " want pos:" << pos << " edgeID:" << edgeID <<  " origAngle:" << origAngle << " angle:" << angle << " keepRoute:" << keepRoute << std::endl;
#endif

    ConstMSEdgeVector edges;
    MSLane* lane = nullptr;
    double lanePos;
    double lanePosLat = 0;
    double bestDistance = std::numeric_limits<double>::max();
    int routeOffset = 0;
    bool found = false;
    double maxRouteDistance = 100;

    ConstMSEdgeVector ev;
    ev.push_back(p->getEdge());
    int routeIndex = 0;
    MSLane* currentLane = const_cast<MSLane*>(getSidewalk<MSEdge, MSLane>(p->getEdge()));
    switch (p->getStageType(0)) {
        case MSStageType::WALKING: {
            MSPerson::MSPersonStage_Walking* s = dynamic_cast<MSPerson::MSPersonStage_Walking*>(p->getCurrentStage());
            assert(s != 0);
            ev = s->getEdges();
            routeIndex = (int)(s->getRouteStep() - s->getRoute().begin());
        }
        break;
        default:
            break;
    }
    if (doKeepRoute) {
        // case a): vehicle is on its earlier route
        //  we additionally assume it is moving forward (SUMO-limit);
        //  note that the route ("edges") is not changed in this case
        found = Helper::moveToXYMap_matchingRoutePosition(pos, edgeID,
                ev, routeIndex, vClass, true,
                bestDistance, &lane, lanePos, routeOffset);
    } else {
        double speed = pos.distanceTo2D(p->getPosition()); // !!!veh->getSpeed();
        found = Helper::moveToXYMap(pos, maxRouteDistance, mayLeaveNetwork, edgeID, angle,
                                    speed, ev, routeIndex, currentLane, p->getEdgePos(), currentLane != nullptr,
                                    vClass, true,
                                    bestDistance, &lane, lanePos, routeOffset, edges);
        if (edges.size() != 0 && ev.size() > 1) {
            // try to rebuild the route
            const MSEdge* origEdge = p->getEdge();
            assert(lane != nullptr);
            const MSJunction* originalTarget = nullptr;
            if (origEdge->isNormal()) {
                if (routeIndex == 0) {
                    if (origEdge->getToJunction() == ev[1]->getToJunction() || origEdge->getToJunction() == ev[1]->getFromJunction()) {
                        originalTarget = origEdge->getToJunction();
                    } else {
                        originalTarget = origEdge->getFromJunction();
                    }
                } else {
                    if (origEdge->getToJunction() == ev[routeIndex - 1]->getToJunction() || origEdge->getToJunction() == ev[routeIndex - 1]->getFromJunction()) {
                        originalTarget = origEdge->getFromJunction();
                    } else {
                        originalTarget = origEdge->getToJunction();
                    }
                }
            } else {
                originalTarget = origEdge->getToJunction();
                assert(originalTarget == origEdge->getFromJunction());
            }
            const MSEdge* newEdge = edges[0];
            if (edges[0]->getFromJunction() == originalTarget || edges[0]->getToJunction() == originalTarget) {
                edges = ev;
                edges[routeIndex] = newEdge;
            }
        }
    }
    if ((found && bestDistance <= maxRouteDistance) || mayLeaveNetwork) {
        // compute lateral offset
        if (found) {
            const double perpDist = lane->getShape().distance2D(pos, false);
            if (perpDist != GeomHelper::INVALID_OFFSET) {
                lanePosLat = perpDist;
                if (!mayLeaveNetwork) {
                    lanePosLat = MIN2(lanePosLat, 0.5 * (lane->getWidth() + p->getVehicleType().getWidth()));
                }
                // figure out whether the offset is to the left or to the right
                PositionVector tmp = lane->getShape();
                try {
                    tmp.move2side(-lanePosLat); // moved to left
                } catch (ProcessError&) {
                    WRITE_WARNING("Could not determine position on lane '" + lane->getID() + " at lateral position " + toString(-lanePosLat) + ".");
                }
                //std::cout << " lane=" << lane->getID() << " posLat=" << lanePosLat << " shape=" << lane->getShape() << " tmp=" << tmp << " tmpDist=" << tmp.distance2D(pos) << "\n";
                if (tmp.distance2D(pos) > perpDist) {
                    lanePosLat = -lanePosLat;
                }
            }
        }
        if (found && !mayLeaveNetwork && MSGlobals::gLateralResolution < 0) {
            // mapped position may differ from pos
            pos = lane->geometryPositionAtOffset(lanePos, -lanePosLat);
        }
        assert((found && lane != 0) || (!found && lane == 0));
        switch (p->getStageType(0)) {
            case MSStageType::WALKING: {
                if (angle == INVALID_DOUBLE_VALUE) {
                    // walking angle cannot be deduced from road angle so we always use the last pos
                    angle = GeomHelper::naviDegree(p->getPosition().angleTo2D(pos));
                }
                break;
            }
            case MSStageType::WAITING_FOR_DEPART:
                MSNet::getInstance()->getPersonControl().forceDeparture();
                FALLTHROUGH;
            case MSStageType::WAITING: {
                if (p->getNumRemainingStages() <= 1 || p->getStageType(1) != MSStageType::WALKING) {
                    // insert walking stage after the current stage
                    ConstMSEdgeVector route({p->getEdge()});
                    const double departPos = p->getCurrentStage()->getArrivalPos();
                    p->appendStage(new MSPerson::MSPersonStage_Walking(p->getID(), route, nullptr, -1, -1, departPos, departPos, 0), 1);
                }
                // abort waiting stage and proceed to walking stage
                p->removeStage(0);
                assert(p->getStageType(0) == MSStageType::WALKING);
                if (angle == INVALID_DOUBLE_VALUE) {
                    if (lane != nullptr && !lane->getEdge().isWalkingArea()) {
                        angle = GeomHelper::naviDegree(lane->getShape().rotationAtOffset(lanePos));
                    } else {
                        // compute angle outside road network or on walkingarea from old and new position
                        angle = GeomHelper::naviDegree(p->getPosition().angleTo2D(pos));
                    }
                }
                break;
            }
            default:
                throw TraCIException("Command moveToXY is not supported for person '" + personID + "' while " + p->getCurrentStageDescription() + ".");
        }
        Helper::setRemoteControlled(p, pos, lane, lanePos, lanePosLat, angle, routeOffset, edges, MSNet::getInstance()->getCurrentTimeStep());
    } else {
        if (lane == nullptr) {
            throw TraCIException("Could not map person '" + personID + "' no road found within " + toString(maxRouteDistance) + "m.");
        } else {
            throw TraCIException("Could not map person '" + personID + "' distance to road is " + toString(bestDistance) + ".");
        }
    }
}


/** untested setter functions which alter the person's vtype ***/

void
Person::setParameter(const std::string& personID, const std::string& key, const std::string& value) {
    MSTransportable* p = getPerson(personID);
    ((SUMOVehicleParameter&)p->getParameter()).setParameter(key, value);
}

void
Person::setLength(const std::string& personID, double length) {
    getPerson(personID)->getSingularType().setLength(length);
}


void
Person::setMaxSpeed(const std::string& personID, double speed) {
    getPerson(personID)->getSingularType().setMaxSpeed(speed);
}


void
Person::setVehicleClass(const std::string& personID, const std::string& clazz) {
    getPerson(personID)->getSingularType().setVClass(getVehicleClassID(clazz));
}


void
Person::setShapeClass(const std::string& personID, const std::string& clazz) {
    getPerson(personID)->getSingularType().setShape(getVehicleShapeID(clazz));
}


void
Person::setEmissionClass(const std::string& personID, const std::string& clazz) {
    getPerson(personID)->getSingularType().setEmissionClass(PollutantsInterface::getClassByName(clazz));
}


void
Person::setWidth(const std::string& personID, double width) {
    getPerson(personID)->getSingularType().setWidth(width);
}


void
Person::setHeight(const std::string& personID, double height) {
    getPerson(personID)->getSingularType().setHeight(height);
}


void
Person::setMinGap(const std::string& personID, double minGap) {
    getPerson(personID)->getSingularType().setMinGap(minGap);
}


void
Person::setAccel(const std::string& personID, double accel) {
    getPerson(personID)->getSingularType().setAccel(accel);
}


void
Person::setDecel(const std::string& personID, double decel) {
    getPerson(personID)->getSingularType().setDecel(decel);
}


void
Person::setEmergencyDecel(const std::string& personID, double decel) {
    getPerson(personID)->getSingularType().setEmergencyDecel(decel);
}


void
Person::setApparentDecel(const std::string& personID, double decel) {
    getPerson(personID)->getSingularType().setApparentDecel(decel);
}


void
Person::setImperfection(const std::string& personID, double imperfection) {
    getPerson(personID)->getSingularType().setImperfection(imperfection);
}


void
Person::setTau(const std::string& personID, double tau) {
    getPerson(personID)->getSingularType().setTau(tau);
}


void
Person::setMinGapLat(const std::string& personID, double minGapLat) {
    getPerson(personID)->getSingularType().setMinGapLat(minGapLat);
}


void
Person::setMaxSpeedLat(const std::string& personID, double speed) {
    getPerson(personID)->getSingularType().setMaxSpeedLat(speed);
}


void
Person::setLateralAlignment(const std::string& personID, const std::string& latAlignment) {
    getPerson(personID)->getSingularType().setPreferredLateralAlignment(SUMOXMLDefinitions::LateralAlignments.get(latAlignment));
}


void
Person::setSpeedFactor(const std::string& personID, double factor) {
    getPerson(personID)->getSingularType().setSpeedFactor(factor);
}


void
Person::setActionStepLength(const std::string& personID, double actionStepLength, bool resetActionOffset) {
    getPerson(personID)->getSingularType().setActionStepLength(SUMOVehicleParserHelper::processActionStepLength(actionStepLength), resetActionOffset);
}


void
Person::setColor(const std::string& personID, const TraCIColor& c) {
    const SUMOVehicleParameter& p = getPerson(personID)->getParameter();
    p.color.set((unsigned char)c.r, (unsigned char)c.g, (unsigned char)c.b, (unsigned char)c.a);
    p.parametersSet |= VEHPARS_COLOR_SET;
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(Person, PERSON)


MSPerson*
Person::getPerson(const std::string& personID) {
    return Helper::getPerson(personID);
}


void
Person::storeShape(const std::string& id, PositionVector& shape) {
    shape.push_back(getPerson(id)->getPosition());
}


std::shared_ptr<VariableWrapper>
Person::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
Person::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_POSITION:
            return wrapper->wrapPosition(objID, variable, getPosition(objID));
        case VAR_POSITION3D:
            return wrapper->wrapPosition(objID, variable, getPosition(objID, true));
        case VAR_ANGLE:
            return wrapper->wrapDouble(objID, variable, getAngle(objID));
        case VAR_SLOPE:
            return wrapper->wrapDouble(objID, variable, getSlope(objID));
        case VAR_SPEED:
            return wrapper->wrapDouble(objID, variable, getSpeed(objID));
        case VAR_ROAD_ID:
            return wrapper->wrapString(objID, variable, getRoadID(objID));
        case VAR_LANE_ID:
            return wrapper->wrapString(objID, variable, getLaneID(objID));
        case VAR_LANEPOSITION:
            return wrapper->wrapDouble(objID, variable, getLanePosition(objID));
        case VAR_COLOR:
            return wrapper->wrapColor(objID, variable, getColor(objID));
        case VAR_WAITING_TIME:
            return wrapper->wrapDouble(objID, variable, getWaitingTime(objID));
        case VAR_TYPE:
            return wrapper->wrapString(objID, variable, getTypeID(objID));
        case VAR_NEXT_EDGE:
            return wrapper->wrapString(objID, variable, getNextEdge(objID));
        case VAR_STAGES_REMAINING:
            return wrapper->wrapInt(objID, variable, getRemainingStages(objID));
        case VAR_VEHICLE:
            return wrapper->wrapString(objID, variable, getVehicle(objID));
        case VAR_TAXI_RESERVATIONS:
            return false;
        default:
            return libsumo::VehicleType::handleVariable(getTypeID(objID), variable, wrapper);
    }
}


}


/****************************************************************************/
