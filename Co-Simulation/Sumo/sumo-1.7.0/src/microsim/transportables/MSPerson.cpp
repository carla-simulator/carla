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
/// @file    MSPerson.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Mon, 9 Jul 2001
///
// The class for modelling person-movements
/****************************************************************************/
#include <config.h>

#include <string>
#include <vector>
#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/GeomHelper.h>
#include <utils/router/IntermodalNetwork.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include "MSPerson.h"
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/devices/MSDevice_Tripinfo.h>
#include <microsim/devices/MSDevice_Taxi.h>
#include "MSPModel.h"

// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * MSPerson::MSPersonStage_Walking - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Walking::MSPersonStage_Walking(const std::string& personID,
        const ConstMSEdgeVector& route,
        MSStoppingPlace* toStop,
        SUMOTime walkingTime, double speed,
        double departPos, double arrivalPos, double departPosLat) :
    MSStageMoving(route, toStop, speed, departPos, arrivalPos, departPosLat, MSStageType::WALKING),
    myWalkingTime(walkingTime) {
    myDepartPos = SUMOVehicleParameter::interpretEdgePos(departPos, route.front()->getLength(), SUMO_ATTR_DEPARTPOS,
                  "person '" + personID + "' walking from " + route.front()->getID());
    myArrivalPos = SUMOVehicleParameter::interpretEdgePos(arrivalPos, route.back()->getLength(), SUMO_ATTR_ARRIVALPOS,
                   "person '" + personID + "' walking to " + route.back()->getID());
    if (walkingTime > 0) {
        mySpeed = computeAverageSpeed();
    }
}


MSPerson::MSPersonStage_Walking::~MSPersonStage_Walking() {
}


MSStage*
MSPerson::MSPersonStage_Walking::clone() const {
    return new MSPersonStage_Walking("dummyID", myRoute, myDestinationStop, myWalkingTime, mySpeed, myDepartPos, myArrivalPos, myDepartPosLat);
}


void
MSPerson::MSPersonStage_Walking::proceed(MSNet* net, MSTransportable* person, SUMOTime now, MSStage* previous) {
    myDeparted = now;
    myRouteStep = myRoute.begin();
    myLastEdgeEntryTime = now;
    if (myWalkingTime == 0) {
        if (!person->proceed(net, now)) {
            MSNet::getInstance()->getPersonControl().erase(person);
        }
        return;
    }
    if (previous->getEdgePos(now) >= 0 && previous->getEdge() == *myRouteStep) {
        myDepartPos = previous->getEdgePos(now);
        if (myWalkingTime > 0) {
            mySpeed = computeAverageSpeed();
        }
    }
    MSTransportableControl& pControl = net->getPersonControl();
    myState = pControl.getMovementModel()->add(dynamic_cast<MSPerson*>(person), this, now);
    if (myState == nullptr) {
        pControl.erase(person);
        return;
    }
    const MSEdge* edge = *myRouteStep;
    const MSLane* lane = getSidewalk<MSEdge, MSLane>(getEdge());
    if (lane != nullptr) {
        for (MSMoveReminder* rem : lane->getMoveReminders()) {
            rem->notifyEnter(*person, MSMoveReminder::NOTIFICATION_DEPARTED, lane);
        }
    }
    edge->addPerson(person);
}


void
MSPerson::MSPersonStage_Walking::abort(MSTransportable*) {
    MSNet::getInstance()->getPersonControl().getMovementModel()->remove(myState);
}


void
MSPerson::MSPersonStage_Walking::setSpeed(double speed) {
    mySpeed = speed;
}


double
MSPerson::MSPersonStage_Walking::computeAverageSpeed() const {
    return walkDistance() / STEPS2TIME(myWalkingTime + 1); // avoid systematic rounding errors
}


double
MSPerson::MSPersonStage_Walking::walkDistance() const {
    double length = 0;
    for (const MSEdge* edge : myRoute) {
        length += edge->getLength();
    }
    if (myRoute.size() > 1 && MSNet::getInstance()->getPersonControl().getMovementModel()->usingInternalLanes()) {
        // use lower bound for distance to pass the intersection
        for (ConstMSEdgeVector::const_iterator i = myRoute.begin(); i != myRoute.end() - 1; ++i) {
            const MSEdge* fromEdge = *i;
            const MSEdge* toEdge = *(i + 1);
            const MSLane* from = getSidewalk<MSEdge, MSLane>(fromEdge);
            const MSLane* to = getSidewalk<MSEdge, MSLane>(toEdge);
            Position fromPos;
            Position toPos;
            if (from != nullptr && to != nullptr) {
                if (fromEdge->getToJunction() == toEdge->getFromJunction()) {
                    fromPos = from->getShape().back();
                    toPos = to->getShape().front();
                } else if (fromEdge->getToJunction() == toEdge->getToJunction()) {
                    fromPos = from->getShape().back();
                    toPos = to->getShape().back();
                } else if (fromEdge->getFromJunction() == toEdge->getFromJunction()) {
                    fromPos = from->getShape().front();
                    toPos = to->getShape().front();
                } else if (fromEdge->getFromJunction() == toEdge->getToJunction()) {
                    fromPos = from->getShape().front();
                    toPos = to->getShape().back();
                }
                length += fromPos.distanceTo2D(toPos);
            }
        }
    }
    // determine walking direction for depart and arrival
    const int departFwdArrivalDir = MSPModel::canTraverse(MSPModel::FORWARD, myRoute);
    const int departBwdArrivalDir = MSPModel::canTraverse(MSPModel::BACKWARD, myRoute);
    const bool mayStartForward = departFwdArrivalDir != MSPModel::UNDEFINED_DIRECTION;
    const bool mayStartBackward = departBwdArrivalDir != MSPModel::UNDEFINED_DIRECTION;
    const double lengthFwd = (length - myDepartPos - (
                                  departFwdArrivalDir == MSPModel::BACKWARD
                                  ? myArrivalPos
                                  : myRoute.back()->getLength() - myArrivalPos));
    const double lengthBwd = (length - (myRoute.front()->getLength() - myDepartPos) - (
                                  departBwdArrivalDir == MSPModel::BACKWARD
                                  ? myArrivalPos
                                  : myRoute.back()->getLength() - myArrivalPos));

    if (myRoute.size() == 1) {
        if (myDepartPos > myArrivalPos) {
            length = lengthBwd;
        } else {
            length = lengthFwd;
        }
    } else {
        if (mayStartForward && mayStartBackward) {
            length = lengthFwd < lengthBwd ? lengthFwd : lengthBwd;
        } else if (mayStartForward) {
            length = lengthFwd;
        } else if (mayStartBackward) {
            length = lengthBwd;
        } else {
            length = lengthFwd;
        }
    }
    //std::cout << SIMTIME << " route=" << toString(myRoute)
    //    << " depPos=" << myDepartPos << " arPos=" << myArrivalPos
    //    << " dFwdADir=" << departFwdArrivalDir
    //    << " dBwdADir=" << departBwdArrivalDir
    //    << " lengthFwd=" << lengthFwd
    //    << " lengthBwd=" << lengthBwd
    //    << "\n";

    return MAX2(POSITION_EPS, length);
}


void
MSPerson::MSPersonStage_Walking::tripInfoOutput(OutputDevice& os, const MSTransportable* const person) const {
    const double distance = walkDistance();
    const double maxSpeed = getMaxSpeed(person);
    const SUMOTime duration = myArrived - myDeparted;
    SUMOTime timeLoss = myArrived == -1 ? 0 : duration - TIME2STEPS(distance / maxSpeed);
    if (timeLoss < 0 && timeLoss > TIME2STEPS(-0.1)) {
        // avoid negative timeLoss due to rounding errors
        timeLoss = 0;
    }
    MSDevice_Tripinfo::addPedestrianData(distance, duration, timeLoss);
    os.openTag("walk");
    os.writeAttr("depart", time2string(myDeparted));
    os.writeAttr("departPos", myDepartPos);
    os.writeAttr("arrival", myArrived >= 0 ? time2string(myArrived) : "-1");
    os.writeAttr("arrivalPos", myArrivalPos);
    os.writeAttr("duration", myDeparted < 0 ? "-1" :
                 time2string(myArrived >= 0 ? duration : MSNet::getInstance()->getCurrentTimeStep() - myDeparted));
    os.writeAttr("routeLength", distance);
    os.writeAttr("timeLoss", time2string(timeLoss));
    os.writeAttr("maxSpeed", maxSpeed);
    os.closeTag();
}


void
MSPerson::MSPersonStage_Walking::routeOutput(const bool /* isPerson */, OutputDevice& os, const bool withRouteLength, const MSStage* const /* previous */) const {
    os.openTag("walk").writeAttr(SUMO_ATTR_EDGES, myRoute);
    std::string comment = "";
    if (myDestinationStop != nullptr) {
        os.writeAttr(SUMO_ATTR_BUS_STOP, myDestinationStop->getID());
        if (myDestinationStop->getMyName() != "") {
            comment =  " <!-- " + StringUtils::escapeXML(myDestinationStop->getMyName(), true) + " -->";
        }
    }
    if (myWalkingTime > 0) {
        os.writeAttr(SUMO_ATTR_DURATION, time2string(myWalkingTime));
    } else if (mySpeed > 0) {
        os.writeAttr(SUMO_ATTR_SPEED, mySpeed);
    }
    if (withRouteLength) {
        os.writeAttr("routeLength", walkDistance());
    }
    os.closeTag(comment);
}


bool
MSPerson::MSPersonStage_Walking::moveToNextEdge(MSTransportable* person, SUMOTime currentTime, MSEdge* nextInternal) {
    ((MSEdge*)getEdge())->removePerson(person);
    const MSLane* lane = getSidewalk<MSEdge, MSLane>(getEdge());
    const bool arrived = myRouteStep == myRoute.end() - 1;
    if (lane != nullptr) {
        for (MSMoveReminder* rem : lane->getMoveReminders()) {
            rem->updateDetector(*person, 0.0, lane->getLength(), myLastEdgeEntryTime, currentTime, currentTime, true);
            rem->notifyLeave(*person,
                             arrived ? getArrivalPos() : lane->getLength(),
                             arrived ? MSMoveReminder::NOTIFICATION_ARRIVED : MSMoveReminder::NOTIFICATION_JUNCTION);
        }
    }
    myLastEdgeEntryTime = currentTime;
    //std::cout << SIMTIME << " moveToNextEdge person=" << person->getID() << "\n";
    if (arrived) {
        MSPerson* p = dynamic_cast<MSPerson*>(person);
        if (p->hasInfluencer() && p->getInfluencer().isRemoteControlled()) {
            myCurrentInternalEdge = nextInternal;
            ((MSEdge*) getEdge())->addPerson(person);
            return false;
        }
        if (myDestinationStop != nullptr) {
            myDestinationStop->addTransportable(person);
        }
        if (!person->proceed(MSNet::getInstance(), currentTime)) {
            MSNet::getInstance()->getPersonControl().erase(person);
        }
        //std::cout << " end walk. myRouteStep=" << (*myRouteStep)->getID() << "\n";
        return true;
    } else {
        if (nextInternal == nullptr) {
            ++myRouteStep;
        }
        myCurrentInternalEdge = nextInternal;
        const MSLane* nextLane = getSidewalk<MSEdge, MSLane>(getEdge());
        if (nextLane != nullptr) {
            for (MSMoveReminder* rem : nextLane->getMoveReminders()) {
                rem->notifyEnter(*person, MSMoveReminder::NOTIFICATION_JUNCTION, nextLane);
            }
        }
        ((MSEdge*) getEdge())->addPerson(person);
        return false;
    }
}

double
MSPerson::MSPersonStage_Walking::getMaxSpeed(const MSTransportable* const person) const {
    return mySpeed >= 0 ? mySpeed : person->getVehicleType().getMaxSpeed() * person->getSpeedFactor();
}

std::string
MSPerson::MSPersonStage_Walking::getStageSummary(const bool /* isPerson */) const {
    const std::string dest = (getDestinationStop() == nullptr ?
                              " edge '" + getDestination()->getID() + "'" :
                              " stop '" + getDestinationStop()->getID() + "'" + (
                                  getDestinationStop()->getMyName() != "" ? " (" + getDestinationStop()->getMyName() + ")" : ""));
    return "walking to " + dest;
}


/* -------------------------------------------------------------------------
* MSPerson::MSPersonStage_Access - methods
* ----------------------------------------------------------------------- */
MSPerson::MSPersonStage_Access::MSPersonStage_Access(const MSEdge* destination, MSStoppingPlace* toStop,
        const double arrivalPos, const double dist, const bool isExit) :
    MSStage(destination, toStop, arrivalPos, MSStageType::ACCESS),
    myDist(dist), myAmExit(isExit) {
    myPath.push_back(destination->getLanes()[0]->geometryPositionAtOffset(myDestinationStop->getAccessPos(destination)));
    myPath.push_back(toStop->getLane().geometryPositionAtOffset((toStop->getEndLanePosition() + toStop->getBeginLanePosition()) / 2));
    if (isExit) {
        myPath = myPath.reverse();
    }
}


MSPerson::MSPersonStage_Access::~MSPersonStage_Access() {}

MSStage*
MSPerson::MSPersonStage_Access::clone() const {
    return new MSPersonStage_Access(myDestination, myDestinationStop, myArrivalPos, myDist, myAmExit);
}

void
MSPerson::MSPersonStage_Access::proceed(MSNet* net, MSTransportable* person, SUMOTime now, MSStage* /* previous */) {
    myDeparted = now;
    myEstimatedArrival = now + TIME2STEPS(myDist / person->getVehicleType().getMaxSpeed());
    net->getBeginOfTimestepEvents()->addEvent(new ProceedCmd(person, &myDestinationStop->getLane().getEdge()), myEstimatedArrival);
    myDestinationStop->getLane().getEdge().addPerson(person);
}


std::string
MSPerson::MSPersonStage_Access::getStageDescription(const bool /* isPerson */) const {
    return "access";
}


std::string
MSPerson::MSPersonStage_Access::getStageSummary(const bool /* isPerson */) const {
    return (myAmExit ? "access from stop '" : "access to stop '") + getDestinationStop()->getID() + "'";
}


Position
MSPerson::MSPersonStage_Access::getPosition(SUMOTime now) const {
    return myPath.positionAtOffset(myPath.length() * (now - myDeparted) / (myEstimatedArrival - myDeparted));
}


double
MSPerson::MSPersonStage_Access::getAngle(SUMOTime /* now */) const {
    return myPath.angleAt2D(0);
}


void
MSPerson::MSPersonStage_Access::tripInfoOutput(OutputDevice& os, const MSTransportable* const) const {
    os.openTag("access");
    os.writeAttr("stop", getDestinationStop()->getID());
    os.writeAttr("duration", myArrived > 0 ? time2string(myArrived - myDeparted) : "-1");
    os.writeAttr("routeLength", myDist);
    os.closeTag();
}


SUMOTime
MSPerson::MSPersonStage_Access::ProceedCmd::execute(SUMOTime currentTime) {
    myStopEdge->removePerson(myPerson);
    if (!myPerson->proceed(MSNet::getInstance(), currentTime)) {
        MSNet::getInstance()->getPersonControl().erase(myPerson);
    }
    return 0;
}


/* -------------------------------------------------------------------------
 * MSPerson - methods
 * ----------------------------------------------------------------------- */
MSPerson::MSPerson(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan, const double speedFactor) :
    MSTransportable(pars, vtype, plan, true),
    myInfluencer(nullptr), myChosenSpeedFactor(speedFactor) {
}


MSPerson::~MSPerson() {
}


void
MSPerson::checkAccess(const MSStage* const prior, const bool isDisembark) {
    MSStoppingPlace* const prevStop = prior->getDestinationStop();
    if (prevStop != nullptr) {
        if (isDisembark) {
            const double accessDist = prevStop->getAccessDistance(prior->getDestination());
            if (accessDist > 0.) {
                const double arrivalAtBs = (prevStop->getBeginLanePosition() + prevStop->getEndLanePosition()) / 2;
                myStep = myPlan->insert(myStep, new MSPersonStage_Access(prior->getDestination(), prevStop, arrivalAtBs, accessDist, false));
            }
        } else {
            if (prior->getStageType() != MSStageType::TRIP) {
                const double accessDist = prevStop->getAccessDistance((*myStep)->getFromEdge());
                if (accessDist > 0.) {
                    myStep = myPlan->insert(myStep, new MSPersonStage_Access((*myStep)->getFromEdge(), prevStop, prevStop->getAccessPos((*myStep)->getFromEdge()), accessDist, true));
                }
            }
        }
    }
}


const std::string&
MSPerson::getNextEdge() const {
//    if (getCurrentStageType() == WALKING) {
//        MSPersonStage_Walking* walkingStage =  dynamic_cast<MSPersonStage_Walking*>(*myStep);
//        assert(walkingStage != 0);
//        const MSEdge* nextEdge = walkingStage->getPedestrianState()->getNextEdge(*walkingStage);
//        if (nextEdge != 0) {
//            return nextEdge->getID();
//        }
//    }
//    return StringUtils::emptyString;
    const MSEdge* nextEdge = getNextEdgePtr();
    if (nextEdge != nullptr) {
        return nextEdge->getID();
    }
    return StringUtils::emptyString;
}


const MSEdge*
MSPerson::getNextEdgePtr() const {
    if (getCurrentStageType() == MSStageType::WALKING) {
        MSPersonStage_Walking* walkingStage =  dynamic_cast<MSPersonStage_Walking*>(*myStep);
        assert(walkingStage != nullptr);
        return walkingStage->getState()->getNextEdge(*walkingStage);
    }
    return nullptr;
}



void
MSPerson::reroute(ConstMSEdgeVector& newEdges, double departPos, int firstIndex, int nextIndex) {
    assert(nextIndex > firstIndex);
    //std::cout << SIMTIME << " reroute person " << getID()
    //    << "  newEdges=" << toString(newEdges)
    //    << " firstIndex=" << firstIndex
    //    << " nextIndex=" << nextIndex
    //    << " departPos=" << getEdgePos()
    //    << " arrivalPos=" <<  getNextStage(nextIndex - 1)->getArrivalPos()
    //    << "\n";
    MSPerson::MSPersonStage_Walking* newStage = new MSPerson::MSPersonStage_Walking(getID(), newEdges,
            getNextStage(nextIndex - 1)->getDestinationStop(), -1,
            -1,
            departPos,
            getNextStage(nextIndex - 1)->getArrivalPos(),
            0);
    appendStage(newStage, nextIndex);
    // remove stages in reverse order so that proceed will only be called at the last removal
    for (int i = nextIndex - 1; i >= firstIndex; i--) {
        //std::cout << " removeStage=" << i << "\n";
        removeStage(i);
    }
}


MSPerson::Influencer&
MSPerson::getInfluencer() {
    if (myInfluencer == nullptr) {
        myInfluencer = new Influencer();
    }
    return *myInfluencer;
}


const MSPerson::Influencer*
MSPerson::getInfluencer() const {
    return myInfluencer;
}



/* -------------------------------------------------------------------------
 * methods of MSPerson::Influencer
 * ----------------------------------------------------------------------- */
MSPerson::Influencer::Influencer() {}


MSPerson::Influencer::~Influencer() {}


void
MSPerson::Influencer::setRemoteControlled(Position xyPos, MSLane* l, double pos, double posLat, double angle, int edgeOffset, const ConstMSEdgeVector& route, SUMOTime t) {
    myRemoteXYPos = xyPos;
    myRemoteLane = l;
    myRemotePos = pos;
    myRemotePosLat = posLat;
    myRemoteAngle = angle;
    myRemoteEdgeOffset = edgeOffset;
    myRemoteRoute = route;
    myLastRemoteAccess = t;
}


bool
MSPerson::Influencer::isRemoteControlled() const {
    return myLastRemoteAccess == MSNet::getInstance()->getCurrentTimeStep();
}


bool
MSPerson::Influencer::isRemoteAffected(SUMOTime t) const {
    return myLastRemoteAccess >= t - TIME2STEPS(10);
}


void
MSPerson::Influencer::postProcessRemoteControl(MSPerson* p) {
    /*
    std::cout << SIMTIME << " moveToXY person=" << p->getID()
        << " xyPos=" << myRemoteXYPos
        << " lane=" << Named::getIDSecure(myRemoteLane)
        << " pos=" << myRemotePos
        << " posLat=" << myRemotePosLat
        << " angle=" << myRemoteAngle
        << " eOf=" << myRemoteEdgeOffset
        << " route=" << toString(myRemoteRoute)
        << " aTime=" << time2string(myLastRemoteAccess)
        << "\n";
        */
    switch (p->getStageType(0)) {
        case MSStageType::WALKING: {
            MSPersonStage_Walking* s = dynamic_cast<MSPerson::MSPersonStage_Walking*>(p->getCurrentStage());
            assert(s != nullptr);
            s->getState()->moveToXY(p, myRemoteXYPos, myRemoteLane, myRemotePos, myRemotePosLat, myRemoteAngle, myRemoteEdgeOffset, myRemoteRoute,
                                    MSNet::getInstance()->getCurrentTimeStep());
        }
        break;
        default:
            break;
    }
}


/****************************************************************************/
