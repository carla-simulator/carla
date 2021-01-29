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
/// @file    MEVehicle.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// A vehicle from the mesoscopic point of view
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <cassert>
#include <utils/common/StdDefs.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include <microsim/devices/MSDevice_Vehroutes.h>
#include <microsim/output/MSStopOut.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicleType.h>
#include <microsim/MSLink.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/devices/MSDevice.h>
#include "MELoop.h"
#include "MEVehicle.h"
#include "MESegment.h"


// ===========================================================================
// method definitions
// ===========================================================================
MEVehicle::MEVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
                     MSVehicleType* type, const double speedFactor) :
    MSBaseVehicle(pars, route, type, speedFactor),
    mySegment(nullptr),
    myQueIndex(0),
    myEventTime(SUMOTime_MIN),
    myLastEntryTime(SUMOTime_MIN),
    myBlockTime(SUMOTime_MAX),
    myInfluencer(nullptr) {
    if (!(*myCurrEdge)->isTazConnector()) {
        if ((*myCurrEdge)->allowedLanes(type->getVehicleClass()) == nullptr) {
            throw ProcessError("Vehicle '" + pars->id + "' is not allowed to depart on any lane of edge '" + (*myCurrEdge)->getID() + "'.");
        }
        if (pars->departSpeedProcedure == DepartSpeedDefinition::GIVEN && pars->departSpeed > type->getMaxSpeed()) {
            throw ProcessError("Departure speed for vehicle '" + pars->id +
                               "' is too high for the vehicle type '" + type->getID() + "'.");
        }
    }
}


double
MEVehicle::getBackPositionOnLane(const MSLane* /* lane */) const {
    return getPositionOnLane() - getVehicleType().getLength();
}


double
MEVehicle::getPositionOnLane() const {
// the following interpolation causes problems with arrivals and calibrators
//    const double fracOnSegment = MIN2(double(1), STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() - myLastEntryTime) / STEPS2TIME(myEventTime - myLastEntryTime));
    return mySegment == nullptr ? 0 : (double(mySegment->getIndex()) /* + fracOnSegment */) * mySegment->getLength();
}


double
MEVehicle::getAngle() const {
    const MSLane* const lane = getEdge()->getLanes()[0];
    return lane->getShape().rotationAtOffset(lane->interpolateLanePosToGeometryPos(getPositionOnLane()));
}


double
MEVehicle::getSlope() const {
    const MSLane* const lane = getEdge()->getLanes()[0];
    return lane->getShape().slopeDegreeAtOffset(lane->interpolateLanePosToGeometryPos(getPositionOnLane()));
}


Position
MEVehicle::getPosition(const double offset) const {
    const MSLane* const lane = getEdge()->getLanes()[0];
    return lane->geometryPositionAtOffset(getPositionOnLane() + offset);
}


double
MEVehicle::getSpeed() const {
    if (getWaitingTime() > 0) {
        return 0;
    } else {
        return getAverageSpeed();
    }
}


double
MEVehicle::getAverageSpeed() const {
    return mySegment->getLength() / STEPS2TIME(myEventTime - myLastEntryTime);
}


double
MEVehicle::estimateLeaveSpeed(const MSLink* link) const {
    /// @see MSVehicle.cpp::estimateLeaveSpeed
    const double v = getSpeed();
    return MIN2(link->getViaLaneOrLane()->getVehicleMaxSpeed(this),
                (double)sqrt(2 * link->getLength() * getVehicleType().getCarFollowModel().getMaxAccel() + v * v));
}


double
MEVehicle::getConservativeSpeed(SUMOTime& earliestArrival) const {
    earliestArrival = MAX2(myEventTime, earliestArrival - DELTA_T); // event times have subsecond resolution
    return mySegment->getLength() / STEPS2TIME(earliestArrival - myLastEntryTime);
}


bool
MEVehicle::moveRoutePointer() {
    // vehicle has just entered a new edge. Position is 0
    if (myCurrEdge == myRoute->end() - 1) { // may happen during teleport
        return true;
    }
    ++myCurrEdge;
    if ((*myCurrEdge)->isVaporizing()) {
        return true;
    }
    // update via
    if (myParameter->via.size() > 0 && (*myCurrEdge)->getID() == myParameter->via.front()) {
        myParameter->via.erase(myParameter->via.begin());
    }
    return hasArrived();
}



bool
MEVehicle::hasArrived() const {
    // mySegment may be 0 due to teleporting or arrival
    return myCurrEdge == myRoute->end() - 1 && (
               (mySegment == nullptr)
               || myEventTime == SUMOTime_MIN
               || getPositionOnLane() > myArrivalPos - POSITION_EPS);
}

bool
MEVehicle::isOnRoad() const {
    return getSegment() != nullptr;
}

bool
MEVehicle::isIdling() const {
    return false;
}

bool
MEVehicle::isParking() const {
    return false; // parking attribute of a stop is not yet evaluated /implemented
}


bool
MEVehicle::replaceRoute(const MSRoute* newRoute, const std::string& info,  bool onInit, int offset, bool addStops, bool removeStops) {
    UNUSED_PARAMETER(addStops); // @todo recheck!
    UNUSED_PARAMETER(removeStops); // @todo recheck!
    const ConstMSEdgeVector& edges = newRoute->getEdges();
    // assert the vehicle may continue (must not be "teleported" or whatever to another position)
    if (!onInit && !newRoute->contains(*myCurrEdge)) {
        return false;
    }
    MSLink* oldLink = nullptr;
    MSLink* newLink = nullptr;
    if (mySegment != nullptr) {
        oldLink = mySegment->getLink(this);
    }
    // rebuild in-vehicle route information
    if (onInit) {
        myCurrEdge = newRoute->begin();
    } else {
        myCurrEdge = std::find(edges.begin() + offset, edges.end(), *myCurrEdge);
    }
    // check whether the old route may be deleted (is not used by anyone else)
    newRoute->addReference();
    myRoute->release();
    // assign new route
    myRoute = newRoute;
    if (mySegment != nullptr) {
        newLink = mySegment->getLink(this);
    }
    // update approaching vehicle information
    if (oldLink != nullptr && oldLink != newLink) {
        oldLink->removeApproaching(this);
        MELoop::setApproaching(this, newLink);
    }
    // update arrival definition
    calculateArrivalParams();
    // save information that the vehicle was rerouted
    myNumberReroutes++;
    MSNet::getInstance()->informVehicleStateListener(this, MSNet::VEHICLE_STATE_NEWROUTE, info);
    calculateArrivalParams();
    return true;
}


bool
MEVehicle::addStop(const SUMOVehicleParameter::Stop& stopPar, std::string& /*errorMsg*/, SUMOTime untilOffset, bool /*collision*/,
                   MSRouteIterator* /* searchStart */) {
    const MSEdge* const edge = MSEdge::dictionary(stopPar.lane.substr(0, stopPar.lane.rfind('_')));
    assert(edge != 0);
    MESegment* stopSeg = MSGlobals::gMesoNet->getSegmentForEdge(*edge, stopPar.endPos);
    std::vector<SUMOVehicleParameter::Stop>& segmentStops = myStops[stopSeg];
    bool cyclicRoute = (myStopEdges.size() > 0 && myStopEdges.back() == edge
                        && segmentStops.size() > 0 && segmentStops.back().endPos > stopPar.endPos
                        && stopPar.index != STOP_INDEX_FIT);
    segmentStops.push_back(stopPar);
    if (segmentStops.back().until >= 0) {
        segmentStops.back().until += untilOffset;
    }
    if (segmentStops.back().arrival >= 0) {
        segmentStops.back().arrival += untilOffset;
    }
    if (myStopEdges.empty() || myStopEdges.back() != edge || cyclicRoute) {
        myStopEdges.push_back(edge);
    }
    return true;
}


bool
MEVehicle::isStopped() const {
    return myStops.find(mySegment) != myStops.end();
}


bool
MEVehicle::isStoppedTriggered() const {
    return false;
}


bool
MEVehicle::isStoppedInRange(const double /* pos */, const double /* tolerance */) const {
    return isStopped();
}


SUMOTime
MEVehicle::getStoptime(const MESegment* const seg, SUMOTime time) const {
    if (myStops.find(seg) != myStops.end()) {
        for (const SUMOVehicleParameter::Stop& stop : myStops.find(seg)->second) {
            time += stop.duration;
            if (stop.until > time) {
                // @note: this assumes the stop is reached at time. With the way this is called in MESegment (time == entryTime),
                // travel time is overestimated of the stop is not at the start of the segment
                time = stop.until;
            }
        }
    }
    return time;
}


double
MEVehicle::getCurrentStoppingTimeSeconds() const {
    return STEPS2TIME(getStoptime(mySegment, myLastEntryTime) - myLastEntryTime);
}


const ConstMSEdgeVector
MEVehicle::getStopEdges(double& firstPos, double& lastPos) const {
    if (myStopEdges.size() > 0) {
        // always try to skip
        firstPos = myStopEdges.front()->getLength();
        lastPos = 0;
    }
    return myStopEdges;
}


std::vector<std::pair<int, double> >
MEVehicle::getStopIndices() const {
    std::vector<std::pair<int, double> > result;
    auto it = myCurrEdge;
    for (const MSEdge* e : myStopEdges) {
        auto it2 = std::find(it, myRoute->end(), e);
        if (it2 != myRoute->end()) {
            result.push_back(std::make_pair((int)(it2 - myRoute->begin()), 0));
            it = it2;
        }
    }
    return result;
}


bool
MEVehicle::abortNextStop(int nextStopIndex) {
    if (!myStops.empty() && nextStopIndex < (int)myStops.size()) {
        if (nextStopIndex == 0 && isStopped()) {
            // resumeFromStopping();
        } else {
            auto stopIt = myStopEdges.begin();
            std::advance(stopIt, nextStopIndex);
            myStopEdges.erase(stopIt);
        }
        return true;
    } else {
        return false;
    }
}


void
MEVehicle::processStop() {
    assert(isStopped());
    MSEdge* edge = const_cast<MSEdge*>(getEdge());
    auto segStopsIt = myStops.find(mySegment);
    std::vector<SUMOVehicleParameter::Stop>& stops = segStopsIt->second;
    double lastPos = 0;
    for (auto it = stops.begin(); it != stops.end();) {
        SUMOVehicleParameter::Stop stop = *it;
        if (stop.endPos <= lastPos) {
            break;
        }
        lastPos = stop.endPos;
        if (MSStopOut::active()) {
            MSStopOut::getInstance()->stopStarted(this, getPersonNumber(), getContainerNumber(), myLastEntryTime);
        }
        MSNet* const net = MSNet::getInstance();
        SUMOTime dummy = -1; // boarding- and loading-time are not considered
        if (net->hasPersons()) {
            net->getPersonControl().boardAnyWaiting(edge, this, stop, dummy, dummy);
        }
        if (net->hasContainers()) {
            net->getContainerControl().loadAnyWaiting(edge, this, stop, dummy, dummy);
        }
        MSDevice_Vehroutes* vehroutes = static_cast<MSDevice_Vehroutes*>(getDevice(typeid(MSDevice_Vehroutes)));
        if (vehroutes != nullptr) {
            vehroutes->stopEnded(stop);
        }
        if (MSStopOut::active()) {
            MSStopOut::getInstance()->stopEnded(this, stop, mySegment->getEdge().getID());
        }
        it = stops.erase(it);
    }
    mySegment->getEdge().removeWaiting(this);
    // clean up stops
    if (stops.size() == 0) {
        myStops.erase(segStopsIt);
    }
    bool removeStopEdge = true;
    // remove the current stop edge if there are no stops on further segments of this edge
    for (MESegment* next = mySegment->getNextSegment(); next != nullptr; next = next->getNextSegment()) {
        if (myStops.count(next) != 0) {
            removeStopEdge = false;
            break;
        }
    }
    if (removeStopEdge) {
        if (myStopEdges.size() > 0) {
            myStopEdges.erase(myStopEdges.begin());
        } else {
            assert(false);
        }
    }
}


bool
MEVehicle::mayProceed() const {
    return mySegment == nullptr || mySegment->isOpen(this);
}


double
MEVehicle::getCurrentLinkPenaltySeconds() const {
    if (mySegment == nullptr) {
        return 0;
    } else {
        return STEPS2TIME(mySegment->getLinkPenalty(this));
    }
}


void
MEVehicle::updateDetectorForWriting(MSMoveReminder* rem, SUMOTime currentTime, SUMOTime exitTime) {
    for (MoveReminderCont::iterator i = myMoveReminders.begin(); i != myMoveReminders.end(); ++i) {
        if (i->first == rem) {
            rem->updateDetector(*this, mySegment->getIndex() * mySegment->getLength(),
                                (mySegment->getIndex() + 1) * mySegment->getLength(),
                                getLastEntryTime(), currentTime, exitTime, false);
#ifdef _DEBUG
            if (myTraceMoveReminders) {
                traceMoveReminder("notifyMove", i->first, i->second, true);
            }
#endif
            return;
        }
    }
}


void
MEVehicle::updateDetectors(SUMOTime currentTime, const bool isLeave, const MSMoveReminder::Notification reason) {
    // segments of the same edge have the same reminder so no cleaning up must take place
    if (reason == MSMoveReminder::NOTIFICATION_JUNCTION || reason == MSMoveReminder::NOTIFICATION_TELEPORT) {
        myOdometer += getEdge()->getLength();
    }
    const bool cleanUp = isLeave && (reason != MSMoveReminder::NOTIFICATION_SEGMENT);
    for (MoveReminderCont::iterator rem = myMoveReminders.begin(); rem != myMoveReminders.end();) {
        if (currentTime != getLastEntryTime()) {
            rem->first->updateDetector(*this, mySegment->getIndex() * mySegment->getLength(),
                                       (mySegment->getIndex() + 1) * mySegment->getLength(),
                                       getLastEntryTime(), currentTime, getEventTime(), cleanUp);
#ifdef _DEBUG
            if (myTraceMoveReminders) {
                traceMoveReminder("notifyMove", rem->first, rem->second, true);
            }
#endif
        }
        if (!isLeave || rem->first->notifyLeave(*this, mySegment->getLength(), reason)) {
#ifdef _DEBUG
            if (isLeave && myTraceMoveReminders) {
                traceMoveReminder("notifyLeave", rem->first, rem->second, true);
            }
#endif
            ++rem;
        } else {
#ifdef _DEBUG
            if (myTraceMoveReminders) {
                traceMoveReminder("remove", rem->first, rem->second, false);
            }
#endif
            rem = myMoveReminders.erase(rem);
        }
    }
}


MEVehicle::BaseInfluencer&
MEVehicle::getBaseInfluencer() {
    if (myInfluencer == nullptr) {
        myInfluencer = new BaseInfluencer();
    }
    return *myInfluencer;
}


const MEVehicle::BaseInfluencer*
MEVehicle::getBaseInfluencer() const {
    return myInfluencer;
}


void
MEVehicle::onRemovalFromNet(const MSMoveReminder::Notification reason) {
    MSGlobals::gMesoNet->removeLeaderCar(this);
    MSGlobals::gMesoNet->changeSegment(this, MSNet::getInstance()->getCurrentTimeStep(), nullptr, reason);
}

void
MEVehicle::saveState(OutputDevice& out) {
    if (mySegment != nullptr && MESegment::isInvalid(mySegment)) {
        // segment is vaporization target, do not write this vehicle
        return;
    }
    MSBaseVehicle::saveState(out);
    assert(mySegment == 0 || *myCurrEdge == &mySegment->getEdge());
    std::vector<SUMOTime> internals;
    internals.push_back(myDeparture);
    internals.push_back((SUMOTime)distance(myRoute->begin(), myCurrEdge));
    internals.push_back((SUMOTime)myDepartPos * 1000); // store as mm
    internals.push_back(mySegment == nullptr ? (SUMOTime) - 1 : (SUMOTime)mySegment->getIndex());
    internals.push_back((SUMOTime)getQueIndex());
    internals.push_back(myEventTime);
    internals.push_back(myLastEntryTime);
    internals.push_back(myBlockTime);
    out.writeAttr(SUMO_ATTR_STATE, toString(internals));
    // save stops and parameters
    for (const auto& it : myStops) {
        for (const SUMOVehicleParameter::Stop& stop : it.second) {
            stop.write(out);
        }
    }
    myParameter->writeParams(out);
    for (MSDevice* dev : myDevices) {
        dev->saveState(out);
    }
    out.closeTag();
}


void
MEVehicle::loadState(const SUMOSAXAttributes& attrs, const SUMOTime offset) {
    if (attrs.hasAttribute(SUMO_ATTR_POSITION)) {
        throw ProcessError("Error: Invalid vehicles in state (may be a micro state)!");
    }
    int routeOffset;
    int segIndex;
    int queIndex;
    std::istringstream bis(attrs.getString(SUMO_ATTR_STATE));
    bis >> myDeparture;
    bis >> routeOffset;
    bis >> myDepartPos;
    bis >> segIndex;
    bis >> queIndex;
    bis >> myEventTime;
    bis >> myLastEntryTime;
    bis >> myBlockTime;
    myDepartPos *= 1000; // was stored as mm
    if (hasDeparted()) {
        myDeparture -= offset;
        myEventTime -= offset;
        myLastEntryTime -= offset;
        myCurrEdge += routeOffset;
        if (segIndex >= 0) {
            MESegment* seg = MSGlobals::gMesoNet->getSegmentForEdge(**myCurrEdge);
            while (seg->getIndex() != (int)segIndex) {
                seg = seg->getNextSegment();
                assert(seg != 0);
            }
            setSegment(seg, queIndex);
        } else {
            // on teleport
            setSegment(nullptr, 0);
            assert(myEventTime != SUMOTime_MIN);
            MSGlobals::gMesoNet->addLeaderCar(this, nullptr);
        }
    }
    if (myBlockTime != SUMOTime_MAX) {
        myBlockTime -= offset;
    }
}


/****************************************************************************/
