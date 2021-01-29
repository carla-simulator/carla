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
/// @file    MSStage.cpp
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @author  Michael Behrisch
/// @date    Thu, 12 Jun 2014
///
// The common superclass for modelling transportable objects like persons and containers
/****************************************************************************/
#include <config.h>

#include <utils/common/StringTokenizer.h>
#include <utils/geom/GeomHelper.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/router/PedestrianRouter.h>
#include <utils/router/IntermodalRouter.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/devices/MSTransportableDevice.h>
#include <microsim/devices/MSDevice_Taxi.h>
#include <microsim/devices/MSDevice_Tripinfo.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/transportables/MSStageDriving.h>
#include <microsim/transportables/MSPModel.h>


/* -------------------------------------------------------------------------
* static member definitions
* ----------------------------------------------------------------------- */
const double MSStage::ROADSIDE_OFFSET(3);


// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * MSStage - methods
 * ----------------------------------------------------------------------- */
MSStage::MSStage(const MSEdge* destination, MSStoppingPlace* toStop, const double arrivalPos, MSStageType type, const std::string& group) :
    myDestination(destination),
    myDestinationStop(toStop),
    myArrivalPos(arrivalPos),
    myDeparted(-1),
    myArrived(-1),
    myType(type),
    myGroup(group)
{}

MSStage::~MSStage() {}

const MSEdge*
MSStage::getDestination() const {
    return myDestination;
}


const MSEdge*
MSStage::getEdge() const {
    return myDestination;
}


const MSEdge*
MSStage::getFromEdge() const {
    return myDestination;
}


double
MSStage::getEdgePos(SUMOTime /* now */) const {
    return myArrivalPos;
}


SUMOTime
MSStage::getWaitingTime(SUMOTime /* now */) const {
    return 0;
}


double
MSStage::getSpeed() const {
    return 0.;
}


ConstMSEdgeVector
MSStage::getEdges() const {
    ConstMSEdgeVector result;
    result.push_back(getDestination());
    return result;
}


void
MSStage::setDeparted(SUMOTime now) {
    if (myDeparted < 0) {
        myDeparted = now;
    }
}

SUMOTime
MSStage::getDeparted() const {
    return myDeparted;
}

SUMOTime
MSStage::getArrived() const {
    return myArrived;
}

const std::string
MSStage::setArrived(MSNet* /* net */, MSTransportable* /* transportable */, SUMOTime now, const bool /* vehicleArrived */) {
    myArrived = now;
    return "";
}

bool
MSStage::isWaitingFor(const SUMOVehicle* /*vehicle*/) const {
    return false;
}

Position
MSStage::getEdgePosition(const MSEdge* e, double at, double offset) const {
    return getLanePosition(e->getLanes()[0], at, offset);
}

Position
MSStage::getLanePosition(const MSLane* lane, double at, double offset) const {
    return lane->getShape().positionAtOffset(lane->interpolateLanePosToGeometryPos(at), offset);
}


double
MSStage::getEdgeAngle(const MSEdge* e, double at) const {
    return e->getLanes()[0]->getShape().rotationAtOffset(at);
}


void
MSStage::setDestination(const MSEdge* newDestination, MSStoppingPlace* newDestStop) {
    myDestination = newDestination;
    myDestinationStop = newDestStop;
    if (newDestStop != nullptr) {
        myArrivalPos = (newDestStop->getBeginLanePosition() + newDestStop->getEndLanePosition()) / 2;
    }
}



/* -------------------------------------------------------------------------
* MSStageTrip - methods
* ----------------------------------------------------------------------- */
MSStageTrip::MSStageTrip(const MSEdge* origin, MSStoppingPlace* fromStop,
                         const MSEdge* destination, MSStoppingPlace* toStop,
                         const SUMOTime duration, const SVCPermissions modeSet,
                         const std::string& vTypes, const double speed, const double walkFactor,
                         const double departPosLat, const bool hasArrivalPos, const double arrivalPos):
    MSStage(destination, toStop, arrivalPos, MSStageType::TRIP),
    myOrigin(origin),
    myOriginStop(fromStop),
    myDuration(duration),
    myModeSet(modeSet),
    myVTypes(vTypes),
    mySpeed(speed),
    myWalkFactor(walkFactor),
    myDepartPosLat(departPosLat),
    myHaveArrivalPos(hasArrivalPos) {
}


MSStageTrip::~MSStageTrip() {}

MSStage*
MSStageTrip::clone() const {
    return new MSStageTrip(myOrigin, const_cast<MSStoppingPlace*>(myOriginStop),
                           myDestination, myDestinationStop, myDuration,
                           myModeSet, myVTypes, mySpeed, myWalkFactor, myDepartPosLat, myHaveArrivalPos, myArrivalPos);
}


Position
MSStageTrip::getPosition(SUMOTime /* now */) const {
    // may be called concurrently while the trip is still being routed
    return getEdgePosition(myOrigin, myDepartPos, ROADSIDE_OFFSET * (MSGlobals::gLefthand ? -1 : 1));
}


double
MSStageTrip::getAngle(SUMOTime /* now */) const {
    // may be called concurrently while the trip is still being routed
    return getEdgeAngle(myOrigin, myDepartPos) + M_PI / 2 * (MSGlobals::gLefthand ? -1 : 1);
}


const MSEdge*
MSStageTrip::getEdge() const {
    return myOrigin;
}


double
MSStageTrip::getEdgePos(SUMOTime /* now */) const {
    return myDepartPos;
}


const std::string
MSStageTrip::setArrived(MSNet* net, MSTransportable* transportable, SUMOTime now, const bool vehicleArrived) {
    MSStage::setArrived(net, transportable, now, vehicleArrived);
    MSVehicleControl& vehControl = net->getVehicleControl();
    std::vector<SUMOVehicleParameter*> pars;
    for (StringTokenizer st(myVTypes); st.hasNext();) {
        pars.push_back(new SUMOVehicleParameter());
        pars.back()->vtypeid = st.next();
        pars.back()->parametersSet |= VEHPARS_VTYPE_SET;
        pars.back()->departProcedure = DEPART_TRIGGERED;
        pars.back()->id = transportable->getID() + "_" + toString(pars.size() - 1);
    }
    if (pars.empty()) {
        if ((myModeSet & SVC_PASSENGER) != 0) {
            pars.push_back(new SUMOVehicleParameter());
            pars.back()->id = transportable->getID() + "_0";
            pars.back()->departProcedure = DEPART_TRIGGERED;
        } else if ((myModeSet & SVC_TAXI) != 0) {
            pars.push_back(new SUMOVehicleParameter());
            pars.back()->vtypeid = DEFAULT_TAXITYPE_ID;
            pars.back()->id = transportable->getID() + "_taxi";
            pars.back()->line = "taxi";
        } else if ((myModeSet & SVC_BICYCLE) != 0) {
            pars.push_back(new SUMOVehicleParameter());
            pars.back()->vtypeid = DEFAULT_BIKETYPE_ID;
            pars.back()->id = transportable->getID() + "_b0";
            pars.back()->departProcedure = DEPART_TRIGGERED;
        } else {
            // allow shortcut via busStop even when not intending to ride
            pars.push_back(nullptr);
        }
    }
    MSStage* previous;
    SUMOTime time = MSNet::getInstance()->getCurrentTimeStep();
    if (transportable->getNumStages() == transportable->getNumRemainingStages()) { // this is a difficult way to check that we are the first stage
        myDepartPos = transportable->getParameter().departPos;
        if (transportable->getParameter().departPosProcedure == DepartPosDefinition::RANDOM) {
            myDepartPos = RandHelper::rand(myOrigin->getLength());
        }
        previous = new MSStageWaiting(myOrigin, nullptr, -1, transportable->getParameter().depart, myDepartPos, "start", true);
        time = transportable->getParameter().depart;
    } else {
        previous = transportable->getNextStage(-1);
        myDepartPos = previous->getArrivalPos();
    }
    // TODO This works currently only for a single vehicle type
    const int oldNumStages = transportable->getNumStages();
    for (SUMOVehicleParameter* vehPar : pars) {
        SUMOVehicle* vehicle = nullptr;
        bool isTaxi = false;
        if (vehPar != nullptr) {
            isTaxi = vehPar->vtypeid == DEFAULT_TAXITYPE_ID && vehPar->line == "taxi";
            if (myDepartPos != 0) {
                vehPar->departPosProcedure = DepartPosDefinition::GIVEN;
                vehPar->departPos = myDepartPos;
                vehPar->parametersSet |= VEHPARS_DEPARTPOS_SET;
            }
            MSVehicleType* type = vehControl.getVType(vehPar->vtypeid);
            if (type->getVehicleClass() != SVC_IGNORING && (myOrigin->getPermissions() & type->getVehicleClass()) == 0 && !isTaxi) {
                WRITE_WARNING("Ignoring vehicle type '" + type->getID() + "' when routing person '" + transportable->getID() + "' because it is not allowed on the start edge.");
            } else {
                const MSRoute* const routeDummy = new MSRoute(vehPar->id, ConstMSEdgeVector({ myOrigin }), false, nullptr, std::vector<SUMOVehicleParameter::Stop>());
                vehicle = vehControl.buildVehicle(vehPar, routeDummy, type, !MSGlobals::gCheckRoutes);
            }
        }
        bool carUsed = false;
        std::vector<MSNet::MSIntermodalRouter::TripItem> result;
        int stageIndex = 1;
        if (net->getIntermodalRouter(0).compute(myOrigin, myDestination, previous->getArrivalPos(), myArrivalPos, myDestinationStop == nullptr ? "" : myDestinationStop->getID(),
                                                transportable->getMaxSpeed() * myWalkFactor, vehicle, myModeSet, time, result)) {
            for (std::vector<MSNet::MSIntermodalRouter::TripItem>::iterator it = result.begin(); it != result.end(); ++it) {
                if (!it->edges.empty()) {
                    MSStoppingPlace* bs = MSNet::getInstance()->getStoppingPlace(it->destStop, SUMO_TAG_BUS_STOP);
                    double localArrivalPos = bs != nullptr ? bs->getAccessPos(it->edges.back()) : it->edges.back()->getLength() / 2.;
                    if (it + 1 == result.end() && myHaveArrivalPos) {
                        localArrivalPos = myArrivalPos;
                    }
                    if (it->line == "") {
                        // determine walk departPos
                        double depPos = previous->getArrivalPos();
                        if (previous->getDestinationStop() != nullptr) {
                            depPos = previous->getDestinationStop()->getAccessPos(it->edges.front());
                        } else if (myOrigin->isTazConnector()) {
                            // walk the whole length of the first edge
                            const MSEdge* first = it->edges.front();
                            if (std::find(first->getPredecessors().begin(), first->getPredecessors().end(), myOrigin) != first->getPredecessors().end()) {
                                depPos = 0;
                            } else {
                                depPos = first->getLength();
                            }
                        } else if (previous->getEdge() != it->edges.front()) {
//                            if (previous->getEdge()->getToJunction() == it->edges.front()->getToJunction()) {
//                                depPos = it->edges.front()->getLength();
//                            } else {
                            depPos = 0.;
//                            }
                        }
                        if (myDestination->isTazConnector()) {
                            // walk the whole length of the last edge
                            const MSEdge* last = it->edges.back();
                            if (std::find(last->getSuccessors().begin(), last->getSuccessors().end(), myDestination) != last->getSuccessors().end()) {
                                localArrivalPos = last->getLength();
                            } else {
                                localArrivalPos = 0;
                            }
                        }
                        previous = new MSPerson::MSPersonStage_Walking(transportable->getID(), it->edges, bs, myDuration, mySpeed, depPos, localArrivalPos, myDepartPosLat);
                        transportable->appendStage(previous, stageIndex++);
                    } else if (isTaxi) {
                        const ConstMSEdgeVector& prevEdges = previous->getEdges();
                        if (prevEdges.size() >= 2) {
                            // determine walking direction and let the previous
                            // stage end after entering its final edge
                            const MSEdge* last = prevEdges.back();
                            const MSEdge* prev = prevEdges[prevEdges.size() - 2];
                            if (last->getFromJunction() == prev->getToJunction() || prev->getFromJunction() == last->getFromJunction()) {
                                previous->setArrivalPos(MIN2(last->getLength(), 10.0));
                            } else {
                                previous->setArrivalPos(MAX2(0.0, last->getLength() - 10));
                            }
                        }
                        previous = new MSStageDriving(it->edges.back(), bs, localArrivalPos, std::vector<std::string>({ "taxi" }));
                        transportable->appendStage(previous, stageIndex++);
                    } else if (vehicle != nullptr && it->line == vehicle->getID()) {
                        if (bs == nullptr && it + 1 != result.end()) {
                            // we have no defined endpoint and are in the middle of the trip, drive as far as possible
                            localArrivalPos = it->edges.back()->getLength();
                        }
                        previous = new MSStageDriving(it->edges.back(), bs, localArrivalPos, std::vector<std::string>({ it->line }));
                        transportable->appendStage(previous, stageIndex++);
                        vehicle->replaceRouteEdges(it->edges, -1, 0, "person:" + transportable->getID(), true);
                        vehicle->setArrivalPos(localArrivalPos);
                        vehControl.addVehicle(vehPar->id, vehicle);
                        carUsed = true;
                    } else {
                        previous = new MSStageDriving(it->edges.back(), bs, localArrivalPos, std::vector<std::string>({ it->line }), myGroup, it->intended, TIME2STEPS(it->depart));
                        transportable->appendStage(previous, stageIndex++);
                    }
                }
            }
        } else {
            // append stage so the GUI won't crash due to inconsistent state
            transportable->appendStage(new MSPerson::MSPersonStage_Walking(transportable->getID(), ConstMSEdgeVector({ myOrigin, myDestination }), myDestinationStop, myDuration, mySpeed, previous->getArrivalPos(), myArrivalPos, myDepartPosLat), stageIndex++);
            if (MSGlobals::gCheckRoutes) {  // if not pedestrians will teleport
                return "No connection found between edge '" + myOrigin->getID() + "' and edge '" + (myDestinationStop != nullptr ? myDestinationStop->getID() : myDestination->getID()) + "' for person '" + transportable->getID() + "'.";
            }
        }
        if (vehicle != nullptr && (isTaxi || !carUsed)) {
            vehControl.deleteVehicle(vehicle, true);
        }
    }
    if (transportable->getNumStages() == oldNumStages) {
        // append stage so the GUI won't crash due to inconsistent state
        transportable->appendStage(new MSPerson::MSPersonStage_Walking(transportable->getID(), ConstMSEdgeVector({ myOrigin, myDestination }), myDestinationStop, myDuration, mySpeed, previous->getArrivalPos(), myArrivalPos, myDepartPosLat), -1);
        if (MSGlobals::gCheckRoutes) {  // if not pedestrians will teleport
            return "Empty route between edge '" + myOrigin->getID() + "' and edge '" + (myDestinationStop != nullptr ? myDestinationStop->getID() : myDestination->getID()) + "' for person '" + transportable->getID() + "'.";
        }
    }
    return "";
}


void
MSStageTrip::proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* /* previous */) {
    // just skip the stage, every interesting happens in setArrived
    transportable->proceed(net, now);
}


std::string
MSStageTrip::getStageSummary(const bool) const {
    return "trip from '" + myOrigin->getID() + "' to '" + getDestination()->getID() + "'";
}


/* -------------------------------------------------------------------------
* MSStageWaiting - methods
* ----------------------------------------------------------------------- */
MSStageWaiting::MSStageWaiting(const MSEdge* destination, MSStoppingPlace* toStop,
                               SUMOTime duration, SUMOTime until, double pos, const std::string& actType,
                               const bool initial) :
    MSStage(destination, toStop, SUMOVehicleParameter::interpretEdgePos(
                pos, destination->getLength(), SUMO_ATTR_DEPARTPOS, "stopping at " + destination->getID()),
            initial ? MSStageType::WAITING_FOR_DEPART : MSStageType::WAITING),
    myWaitingDuration(duration),
    myWaitingUntil(until),
    myActType(actType) {
}


MSStageWaiting::~MSStageWaiting() {}

MSStage*
MSStageWaiting::clone() const {
    return new MSStageWaiting(myDestination, myDestinationStop, myWaitingDuration, myWaitingUntil, myArrivalPos, myActType, myType == MSStageType::WAITING_FOR_DEPART);
}

SUMOTime
MSStageWaiting::getUntil() const {
    return myWaitingUntil;
}


Position
MSStageWaiting::getPosition(SUMOTime /* now */) const {
    return getEdgePosition(myDestination, myArrivalPos,
                           ROADSIDE_OFFSET * (MSGlobals::gLefthand ? -1 : 1));
}


double
MSStageWaiting::getAngle(SUMOTime /* now */) const {
    return getEdgeAngle(myDestination, myArrivalPos) + M_PI / 2 * (MSGlobals::gLefthand ? -1 : 1);
}


void
MSStageWaiting::proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous) {
    myDeparted = now;
    const SUMOTime until = MAX3(now, now + myWaitingDuration, myWaitingUntil);
    if (myDestinationStop != nullptr) {
        myDestinationStop->addTransportable(transportable);
    }
    if (dynamic_cast<MSPerson*>(transportable) != nullptr) {
        previous->getEdge()->addPerson(transportable);
        net->getPersonControl().setWaitEnd(until, transportable);
    } else {
        previous->getEdge()->addContainer(transportable);
        net->getContainerControl().setWaitEnd(until, transportable);
    }
}


void
MSStageWaiting::tripInfoOutput(OutputDevice& os, const MSTransportable* const) const {
    if (myType != MSStageType::WAITING_FOR_DEPART) {
        os.openTag("stop");
        os.writeAttr("duration", time2string(myArrived - myDeparted));
        os.writeAttr("arrival", time2string(myArrived));
        os.writeAttr("arrivalPos", toString(myArrivalPos));
        os.writeAttr("actType", toString(myActType));
        os.closeTag();
    }
}


void
MSStageWaiting::routeOutput(const bool /* isPerson */, OutputDevice& os, const bool, const MSStage* const /* previous */) const {
    if (myType != MSStageType::WAITING_FOR_DEPART) {
        os.openTag("stop");
        std::string comment = "";
        if (myDestinationStop != nullptr) {
            os.writeAttr(SUMO_ATTR_BUS_STOP, myDestinationStop->getID());
            if (myDestinationStop->getMyName() != "") {
                comment =  " <!-- " + StringUtils::escapeXML(myDestinationStop->getMyName(), true) + " -->";
            }
        } else {
            // lane index is arbitrary
            os.writeAttr(SUMO_ATTR_LANE, getDestination()->getID() + "_0");
            os.writeAttr(SUMO_ATTR_ENDPOS, getArrivalPos());
        }
        if (myWaitingDuration >= 0) {
            os.writeAttr(SUMO_ATTR_DURATION, time2string(myWaitingDuration));
        }
        if (myWaitingUntil >= 0) {
            os.writeAttr(SUMO_ATTR_UNTIL, time2string(myWaitingUntil));
        }
        os.closeTag(comment);
    }
}


SUMOTime
MSStageWaiting::getWaitingTime(SUMOTime now) const {
    return now - myDeparted;
}


void
MSStageWaiting::abort(MSTransportable* t) {
    MSTransportableControl& tc = (t->isPerson() ?
                                  MSNet::getInstance()->getPersonControl() :
                                  MSNet::getInstance()->getContainerControl());
    tc.abortWaiting(t);
}


std::string
MSStageWaiting::getStageSummary(const bool /* isPerson */) const {
    std::string timeInfo;
    if (myWaitingUntil >= 0) {
        timeInfo += " until " + time2string(myWaitingUntil);
    }
    if (myWaitingDuration >= 0) {
        timeInfo += " duration " + time2string(myWaitingDuration);
    }
    return "stopping at edge '" + getDestination()->getID() + "' " + timeInfo + " (" + myActType + ")";
}


/* -------------------------------------------------------------------------
* MSStageMoving - methods
* ----------------------------------------------------------------------- */
MSStageMoving::~MSStageMoving() {
    delete myState;
}

const MSEdge*
MSStageMoving::getEdge() const {
    if (myCurrentInternalEdge != nullptr) {
        return myCurrentInternalEdge;
    } else {
        return *myRouteStep;
    }
}

const MSEdge*
MSStageMoving::getFromEdge() const {
    return myRoute.front();
}

ConstMSEdgeVector
MSStageMoving::getEdges() const {
    return myRoute;
}


double
MSStageMoving::getEdgePos(SUMOTime now) const {
    return myState == nullptr ? 0. : myState->getEdgePos(*this, now);
}

Position
MSStageMoving::getPosition(SUMOTime now) const {
    return myState == nullptr ? Position::INVALID : myState->getPosition(*this, now);
}

double
MSStageMoving::getAngle(SUMOTime now) const {
    return myState == nullptr ? 0. : myState->getAngle(*this, now);
}

SUMOTime
MSStageMoving::getWaitingTime(SUMOTime now) const {
    return myState == nullptr ? 0 : myState->getWaitingTime(*this, now);
}

double
MSStageMoving::getSpeed() const {
    return myState == nullptr ? 0. : myState->getSpeed(*this);
}

const MSLane*
MSStageMoving::getLane() const {
    return myState == nullptr ? nullptr : myState->getLane();
}

void
MSStageMoving::setRouteIndex(MSTransportable* const transportable, int routeOffset) {
    assert(routeOffset >= 0);
    assert(routeOffset < (int)myRoute.size());
    getEdge()->removePerson(transportable);
    myRouteStep = myRoute.begin() + routeOffset;
    getEdge()->addPerson(transportable);
}

void
MSStageMoving::replaceRoute(MSTransportable* const transportable, const ConstMSEdgeVector& edges, int routeOffset) {
    assert(routeOffset >= 0);
    assert(routeOffset < (int)edges.size());
    getEdge()->removePerson(transportable);
    myRoute = edges;
    myRouteStep = myRoute.begin() + routeOffset;
    getEdge()->addPerson(transportable);
}

/****************************************************************************/
