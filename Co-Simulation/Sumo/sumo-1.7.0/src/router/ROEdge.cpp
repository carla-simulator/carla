/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    ROEdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @author  Melanie Knocke
/// @author  Yun-Pang Floetteroed
/// @date    Sept 2002
///
// A basic edge for routing applications
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/emissions/HelpersHarmonoise.h>
#include "ROLane.h"
#include "RONet.h"
#include "ROVehicle.h"
#include "ROEdge.h"


// ===========================================================================
// static member definitions
// ===========================================================================
bool ROEdge::myInterpolate = false;
bool ROEdge::myHaveTTWarned = false;
bool ROEdge::myHaveEWarned = false;
ROEdgeVector ROEdge::myEdges;
double ROEdge::myPriorityFactor(0);
double ROEdge::myMinEdgePriority(std::numeric_limits<double>::max());
double ROEdge::myEdgePriorityRange(0);


// ===========================================================================
// method definitions
// ===========================================================================
ROEdge::ROEdge(const std::string& id, RONode* from, RONode* to, int index, const int priority) :
    Named(id),
    myFromJunction(from),
    myToJunction(to),
    myIndex(index),
    myPriority(priority),
    mySpeed(-1),
    myLength(0),
    myAmSink(false),
    myAmSource(false),
    myUsingTTTimeLine(false),
    myUsingETimeLine(false),
    myCombinedPermissions(0),
    myOtherTazConnector(nullptr),
    myTimePenalty(0) {
    while ((int)myEdges.size() <= index) {
        myEdges.push_back(0);
    }
    myEdges[index] = this;
    if (from == nullptr && to == nullptr) {
        // TAZ edge, no lanes
        myCombinedPermissions = SVCAll;
    } else {
        // TODO we should not calculate the boundary here, the position for the nodes is not valid yet
        myBoundary.add(from->getPosition());
        myBoundary.add(to->getPosition());
    }
}


ROEdge::~ROEdge() {
    for (std::vector<ROLane*>::iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        delete (*i);
    }
}


void
ROEdge::addLane(ROLane* lane) {
    assert(myLanes.empty() || lane->getLength() == myLength);
    myLength = lane->getLength();
    const double speed = lane->getSpeed();
    mySpeed = speed > mySpeed ? speed : mySpeed;
    myLanes.push_back(lane);

    // integrate new allowed classes
    myCombinedPermissions |= lane->getPermissions();
}


void
ROEdge::addSuccessor(ROEdge* s, ROEdge* via, std::string) {
    if (isInternal()) {
        // for internal edges after an internal junction,
        // this is called twice and only the second call counts
        myFollowingEdges.clear();
        myFollowingViaEdges.clear();
    }
    if (find(myFollowingEdges.begin(), myFollowingEdges.end(), s) == myFollowingEdges.end()) {
        myFollowingEdges.push_back(s);
        myFollowingViaEdges.push_back(std::make_pair(s, via));
        if (isTazConnector() && s->getFromJunction() != nullptr) {
            myBoundary.add(s->getFromJunction()->getPosition());
        }
        if (!isInternal()) {
            s->myApproachingEdges.push_back(this);
            if (s->isTazConnector() && getToJunction() != nullptr) {
                s->myBoundary.add(getToJunction()->getPosition());
            }
            if (via != nullptr) {
                if (via->myApproachingEdges.size() == 0) {
                    via->myApproachingEdges.push_back(this);
                }
            }
        }
    }
}


void
ROEdge::addEffort(double value, double timeBegin, double timeEnd) {
    myEfforts.add(timeBegin, timeEnd, value);
    myUsingETimeLine = true;
}


void
ROEdge::addTravelTime(double value, double timeBegin, double timeEnd) {
    myTravelTimes.add(timeBegin, timeEnd, value);
    myUsingTTTimeLine = true;
}


double
ROEdge::getEffort(const ROVehicle* const veh, double time) const {
    double ret = 0;
    if (!getStoredEffort(time, ret)) {
        return myLength / MIN2(veh->getType()->maxSpeed, mySpeed) + myTimePenalty;
    }
    return ret;
}


double
ROEdge::getDistanceTo(const ROEdge* other, const bool doBoundaryEstimate) const {
    assert(this != other);
    if (doBoundaryEstimate) {
        return myBoundary.distanceTo2D(other->myBoundary);
    }
    if (isTazConnector()) {
        if (other->isTazConnector()) {
            return myBoundary.distanceTo2D(other->myBoundary);
        }
        return myBoundary.distanceTo2D(other->getFromJunction()->getPosition());
    }
    if (other->isTazConnector()) {
        return other->myBoundary.distanceTo2D(getToJunction()->getPosition());
    }
    return getLanes()[0]->getShape()[-1].distanceTo2D(other->getLanes()[0]->getShape()[0]);
    //return getToJunction()->getPosition().distanceTo2D(other->getFromJunction()->getPosition());
}


bool
ROEdge::hasLoadedTravelTime(double time) const {
    return myUsingTTTimeLine && myTravelTimes.describesTime(time);
}


double
ROEdge::getTravelTime(const ROVehicle* const veh, double time) const {
    if (myUsingTTTimeLine) {
        if (myTravelTimes.describesTime(time)) {
            double lineTT = myTravelTimes.getValue(time);
            if (myInterpolate) {
                const double inTT = lineTT;
                const double split = (double)(myTravelTimes.getSplitTime(time, time + inTT) - time);
                if (split >= 0) {
                    lineTT = myTravelTimes.getValue(time + inTT) * ((double)1. - split / inTT) + split;
                }
            }
            return MAX2(getMinimumTravelTime(veh), lineTT);
        } else {
            if (!myHaveTTWarned) {
                WRITE_WARNING("No interval matches passed time " + toString(time)  + " in edge '" + myID + "'.\n Using edge's length / max speed.");
                myHaveTTWarned = true;
            }
        }
    }
    const double speed = veh != nullptr ? MIN2(veh->getType()->maxSpeed, veh->getType()->speedFactor.getParameter()[0] * mySpeed) : mySpeed;
    return myLength / speed + myTimePenalty;
}


double
ROEdge::getNoiseEffort(const ROEdge* const edge, const ROVehicle* const veh, double time) {
    double ret = 0;
    if (!edge->getStoredEffort(time, ret)) {
        const double v = MIN2(veh->getType()->maxSpeed, edge->mySpeed);
        ret = HelpersHarmonoise::computeNoise(veh->getType()->emissionClass, v, 0);
    }
    return ret;
}


bool
ROEdge::getStoredEffort(double time, double& ret) const {
    if (myUsingETimeLine) {
        if (!myEfforts.describesTime(time)) {
            if (!myHaveEWarned) {
                WRITE_WARNING("No interval matches passed time " + toString(time)  + " in edge '" + myID + "'.\n Using edge's length / edge's speed.");
                myHaveEWarned = true;
            }
            return false;
        }
        if (myInterpolate) {
            const double inTT = myTravelTimes.getValue(time);
            const double ratio = (myEfforts.getSplitTime(time, time + inTT) - time) / inTT;
            if (ratio >= 0.) {
                ret = ratio * myEfforts.getValue(time) + (1. - ratio) * myEfforts.getValue(time + inTT);
                return true;
            }
        }
        ret = myEfforts.getValue(time);
        return true;
    }
    return false;
}


int
ROEdge::getNumSuccessors() const {
    if (myAmSink) {
        return 0;
    }
    return (int) myFollowingEdges.size();
}


int
ROEdge::getNumPredecessors() const {
    if (myAmSource) {
        return 0;
    }
    return (int) myApproachingEdges.size();
}


const ROEdge*
ROEdge::getNormalBefore() const {
    const ROEdge* result = this;
    while (result->isInternal()) {
        assert(myApproachingEdges.size() == 1);
        result = myApproachingEdges.front();
    }
    return result;
}


const ROEdge*
ROEdge::getNormalAfter() const {
    const ROEdge* result = this;
    while (result->isInternal()) {
        assert(myFollowingEdges.size() == 1);
        result = myFollowingEdges.front();
    }
    return result;
}


void
ROEdge::buildTimeLines(const std::string& measure, const bool boundariesOverride) {
    if (myUsingETimeLine) {
        double value = myLength / mySpeed;
        const SUMOEmissionClass c = PollutantsInterface::getClassByName("unknown");
        if (measure == "CO") {
            value = PollutantsInterface::compute(c, PollutantsInterface::CO, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        if (measure == "CO2") {
            value = PollutantsInterface::compute(c, PollutantsInterface::CO2, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        if (measure == "HC") {
            value = PollutantsInterface::compute(c, PollutantsInterface::HC, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        if (measure == "PMx") {
            value = PollutantsInterface::compute(c, PollutantsInterface::PM_X, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        if (measure == "NOx") {
            value = PollutantsInterface::compute(c, PollutantsInterface::NO_X, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        if (measure == "fuel") {
            value = PollutantsInterface::compute(c, PollutantsInterface::FUEL, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        if (measure == "electricity") {
            value = PollutantsInterface::compute(c, PollutantsInterface::ELEC, mySpeed, 0, 0) * value; // @todo: give correct slope
        }
        myEfforts.fillGaps(value, boundariesOverride);
    }
    if (myUsingTTTimeLine) {
        myTravelTimes.fillGaps(myLength / mySpeed + myTimePenalty, boundariesOverride);
    }
}


void
ROEdge::cacheParamRestrictions(const std::vector<std::string>& restrictionKeys) {
    for (const std::string& key : restrictionKeys) {
        const std::string value = getParameter(key, "1e40");
        myParamRestrictions.push_back(StringUtils::toDouble(value));
    }
}


double
ROEdge::getLengthGeometryFactor() const {
    return myLanes.empty() ? 1. : myLanes[0]->getShape().length() / myLanes[0]->getLength();
}


bool
ROEdge::allFollowersProhibit(const ROVehicle* const vehicle) const {
    for (ROEdgeVector::const_iterator i = myFollowingEdges.begin(); i != myFollowingEdges.end(); ++i) {
        if (!(*i)->prohibits(vehicle)) {
            return false;
        }
    }
    return true;
}


const ROEdgeVector&
ROEdge::getAllEdges() {
    return myEdges;
}


const Position
ROEdge::getStopPosition(const SUMOVehicleParameter::Stop& stop) {
    const double middle = (stop.endPos + stop.startPos) / 2.;
    const ROEdge* const edge = RONet::getInstance()->getEdge(SUMOXMLDefinitions::getEdgeIDFromLane(stop.lane));
    return (edge->getFromJunction()->getPosition() + edge->getToJunction()->getPosition()) * (middle / edge->getLength());
}


const ROEdgeVector&
ROEdge::getSuccessors(SUMOVehicleClass vClass) const {
    if (vClass == SVC_IGNORING || !RONet::getInstance()->hasPermissions() || isTazConnector()) {
        return myFollowingEdges;
    }
#ifdef HAVE_FOX
    FXMutexLock locker(myLock);
#endif
    std::map<SUMOVehicleClass, ROEdgeVector>::const_iterator i = myClassesSuccessorMap.find(vClass);
    if (i != myClassesSuccessorMap.end()) {
        // can use cached value
        return i->second;
    }
    // this vClass is requested for the first time. rebuild all successors
    std::set<ROEdge*> followers;
    for (const ROLane* const lane : myLanes) {
        if ((lane->getPermissions() & vClass) != 0) {
            for (const auto& next : lane->getOutgoingViaLanes()) {
                if ((next.first->getPermissions() & vClass) != 0 && (next.second == nullptr || (next.second->getPermissions() & vClass) != 0)) {
                    followers.insert(&next.first->getEdge());
                }
            }
        }
    }
    // also add district edges (they are not connected at the lane level
    for (ROEdgeVector::const_iterator it = myFollowingEdges.begin(); it != myFollowingEdges.end(); ++it) {
        if ((*it)->isTazConnector()) {
            followers.insert(*it);
        }
    }
    myClassesSuccessorMap[vClass].insert(myClassesSuccessorMap[vClass].begin(),
                                         followers.begin(), followers.end());
    return myClassesSuccessorMap[vClass];
}


const ROConstEdgePairVector&
ROEdge::getViaSuccessors(SUMOVehicleClass vClass) const {
    if (vClass == SVC_IGNORING || !RONet::getInstance()->hasPermissions() || isTazConnector()) {
        return myFollowingViaEdges;
    }
#ifdef HAVE_FOX
    FXMutexLock locker(myLock);
#endif
    std::map<SUMOVehicleClass, ROConstEdgePairVector>::const_iterator i = myClassesViaSuccessorMap.find(vClass);
    if (i != myClassesViaSuccessorMap.end()) {
        // can use cached value
        return i->second;
    }
    // this vClass is requested for the first time. rebuild all successors
    std::set<std::pair<const ROEdge*, const ROEdge*> > followers;
    for (const ROLane* const lane : myLanes) {
        if ((lane->getPermissions() & vClass) != 0) {
            for (const auto& next : lane->getOutgoingViaLanes()) {
                if ((next.first->getPermissions() & vClass) != 0 && (next.second == nullptr || (next.second->getPermissions() & vClass) != 0)) {
                    followers.insert(std::make_pair(&next.first->getEdge(), next.second));
                }
            }
        }
    }
    // also add district edges (they are not connected at the lane level
    for (const ROEdge* e : myFollowingEdges) {
        if (e->isTazConnector()) {
            followers.insert(std::make_pair(e, e));
        }
    }
    myClassesViaSuccessorMap[vClass].insert(myClassesViaSuccessorMap[vClass].begin(),
                                            followers.begin(), followers.end());
    return myClassesViaSuccessorMap[vClass];
}


bool
ROEdge::isConnectedTo(const ROEdge& e, const SUMOVehicleClass vClass) const {
    const ROEdgeVector& followers = getSuccessors(vClass);
    return std::find(followers.begin(), followers.end(), &e) != followers.end();
}

bool
ROEdge::initPriorityFactor(double priorityFactor) {
    myPriorityFactor = priorityFactor;
    double maxEdgePriority = -std::numeric_limits<double>::max();
    for (ROEdge* edge : myEdges) {
        maxEdgePriority = MAX2(maxEdgePriority, (double)edge->getPriority());
        myMinEdgePriority = MIN2(myMinEdgePriority, (double)edge->getPriority());
    }
    myEdgePriorityRange = maxEdgePriority - myMinEdgePriority;
    if (myEdgePriorityRange == 0) {
        WRITE_WARNING("Option weights.priority-factor does not take effect because all edges have the same priority.");
        myPriorityFactor = 0;
        return false;
    }
    return true;
}


/****************************************************************************/
