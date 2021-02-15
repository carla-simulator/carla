/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPModel_NonInteracting.cpp
/// @author  Jakob Erdmann
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model (prototype)
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <algorithm>
#include <utils/common/RandHelper.h>
#include <utils/geom/GeomHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/router/IntermodalNetwork.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSJunction.h>
#include <microsim/MSEventControl.h>
#include "MSPModel_NonInteracting.h"


// ===========================================================================
// DEBUGGING HELPERS
// ===========================================================================
#define DEBUG1 "disabled"
#define DEBUG2 "disabled"
#define DEBUGCOND(PEDID) (PEDID == DEBUG1 || PEDID == DEBUG2)


// ===========================================================================
// static members
// ===========================================================================
const double MSPModel_NonInteracting::CState::LATERAL_OFFSET(0);

// ===========================================================================
// MSPModel_NonInteracting method definitions
// ===========================================================================
MSPModel_NonInteracting::MSPModel_NonInteracting(const OptionsCont& oc, MSNet* net) :
    myNet(net),
    myNumActivePedestrians(0) {
    assert(myNet != 0);
    UNUSED_PARAMETER(oc);
}


MSPModel_NonInteracting::~MSPModel_NonInteracting() {
}


MSTransportableStateAdapter*
MSPModel_NonInteracting::add(MSTransportable* transportable, MSStageMoving* stage, SUMOTime now) {
    myNumActivePedestrians++;
    MoveToNextEdge* cmd = new MoveToNextEdge(transportable, *stage, this);
    if (transportable->isPerson()) {
        PState* state = new PState(cmd);
        const SUMOTime firstEdgeDuration = state->computeWalkingTime(nullptr, *stage, now);
        myNet->getBeginOfTimestepEvents()->addEvent(cmd, now + firstEdgeDuration);

        //if DEBUGCOND(person->getID()) std::cout << SIMTIME << " " << person->getID() << " inserted on " << stage->getEdge()->getID() << "\n";
        return state;
    }
    CState* state = new CState(cmd);
    const SUMOTime firstEdgeDuration = state->computeTranshipTime(nullptr, *stage, now);
    myNet->getBeginOfTimestepEvents()->addEvent(cmd, now + firstEdgeDuration);
    return state;
}


void
MSPModel_NonInteracting::remove(MSTransportableStateAdapter* state) {
    myNumActivePedestrians--;
    dynamic_cast<PState*>(state)->getCommand()->abortWalk();
}

MSPModel_NonInteracting::MoveToNextEdge::~MoveToNextEdge() {
    myModel->registerArrived();
}

SUMOTime
MSPModel_NonInteracting::MoveToNextEdge::execute(SUMOTime currentTime) {
    if (myTransportable == nullptr) {
        return 0; // descheduled
    }
    const MSEdge* old = myParent.getEdge();
    const bool arrived = myParent.moveToNextEdge(myTransportable, currentTime);
    if (arrived) {
        return 0;
    }
    if (myTransportable->isPerson()) {
        PState* state = dynamic_cast<PState*>(myParent.getState());
        return state->computeWalkingTime(old, myParent, currentTime);
    } else {
        CState* state = dynamic_cast<CState*>(myParent.getState());
        return state->computeTranshipTime(old, myParent, currentTime);
    }
}


SUMOTime
MSPModel_NonInteracting::PState::computeWalkingTime(const MSEdge* prev, const MSStageMoving& stage, SUMOTime currentTime) {
    myLastEntryTime = currentTime;
    const MSEdge* edge = stage.getEdge();
    const MSEdge* next = stage.getNextRouteEdge();
    int dir = UNDEFINED_DIRECTION;
    if (prev == nullptr) {
        myCurrentBeginPos = stage.getDepartPos();
    } else {
        // default to FORWARD if not connected
        dir = (edge->getToJunction() == prev->getToJunction() || edge->getToJunction() == prev->getFromJunction()) ? BACKWARD : FORWARD;
        myCurrentBeginPos = dir == FORWARD ? 0 : edge->getLength();
    }
    if (next == nullptr) {
        myCurrentEndPos = stage.getArrivalPos();
    } else {
        if (dir == UNDEFINED_DIRECTION) {
            // default to FORWARD if not connected
            dir = (edge->getFromJunction() == next->getFromJunction() || edge->getFromJunction() == next->getToJunction()) ? BACKWARD : FORWARD;
        }
        myCurrentEndPos = dir == FORWARD ? edge->getLength() : 0;
    }
    // ensure that a result > 0 is returned even if the walk ends immediately
    // adding 0.5ms is done to ensure proper rounding
    myCurrentDuration = MAX2((SUMOTime)1, TIME2STEPS(fabs(myCurrentEndPos - myCurrentBeginPos) / stage.getMaxSpeed(myCommand->getTransportable())));
    //std::cout << std::setprecision(8) << SIMTIME << " curBeg=" << myCurrentBeginPos << " curEnd=" << myCurrentEndPos << " speed=" << stage.getMaxSpeed(myCommand->getTransportable()) << " dur=" << myCurrentDuration << "\n";
    // round to the next timestep to avoid systematic higher walking speed
    if ((myCurrentDuration % DELTA_T) > 0) {
        myCurrentDuration += DELTA_T;
    }
    return myCurrentDuration;
}


double
MSPModel_NonInteracting::PState::getEdgePos(const MSStageMoving&, SUMOTime now) const {
    //std::cout << SIMTIME << " lastEntryTime=" << myLastEntryTime << " pos=" << (myCurrentBeginPos + (myCurrentEndPos - myCurrentBeginPos) / myCurrentDuration * (now - myLastEntryTime)) << "\n";
    return myCurrentBeginPos + (myCurrentEndPos - myCurrentBeginPos) / myCurrentDuration * (now - myLastEntryTime);
}


Position
MSPModel_NonInteracting::PState::getPosition(const MSStageMoving& stage, SUMOTime now) const {
    const MSLane* lane = getSidewalk<MSEdge, MSLane>(stage.getEdge());
    if (lane == nullptr) {
        //std::string error = "Pedestrian '" + myCommand->myPerson->getID() + "' could not find sidewalk on edge '" + state.getEdge()->getID() + "', time="
        //    + time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".";
        //if (!OptionsCont::getOptions().getBool("ignore-route-errors")) {
        //    throw ProcessError(error);
        //}
        lane = stage.getEdge()->getLanes().front();
    }
    const double lateral_offset = (lane->allowsVehicleClass(SVC_PEDESTRIAN) ? 0 : SIDEWALK_OFFSET
                                   * (MSGlobals::gLefthand ? -1 : 1));
    return stage.getLanePosition(lane, getEdgePos(stage, now), lateral_offset);
}


double
MSPModel_NonInteracting::PState::getAngle(const MSStageMoving& stage, SUMOTime now) const {
    //std::cout << SIMTIME << " rawAngle=" << stage.getEdgeAngle(stage.getEdge(), getEdgePos(stage, now)) << " angle=" << stage.getEdgeAngle(stage.getEdge(), getEdgePos(stage, now)) + (myCurrentEndPos < myCurrentBeginPos ? 180 : 0) << "\n";
    double angle = stage.getEdgeAngle(stage.getEdge(), getEdgePos(stage, now)) + (myCurrentEndPos < myCurrentBeginPos ? M_PI : 0);
    if (angle > M_PI) {
        angle -= 2 * M_PI;
    }
    return angle;
}


SUMOTime
MSPModel_NonInteracting::PState::getWaitingTime(const MSStageMoving&, SUMOTime) const {
    return 0;
}


double
MSPModel_NonInteracting::PState::getSpeed(const MSStageMoving& stage) const {
    return stage.getMaxSpeed(myCommand->getTransportable());
}


const MSEdge*
MSPModel_NonInteracting::PState::getNextEdge(const MSStageMoving& stage) const {
    return stage.getNextRouteEdge();
}



Position
MSPModel_NonInteracting::CState::getPosition(const MSStageMoving& stage, SUMOTime now) const {
    const double dist = myCurrentBeginPosition.distanceTo2D(myCurrentEndPosition);    //distance between begin and end position of this tranship stage
    double pos = MIN2(STEPS2TIME(now - myLastEntryTime) * stage.getMaxSpeed(), dist);    //the containerd shall not go beyond its end position
    return PositionVector::positionAtOffset2D(myCurrentBeginPosition, myCurrentEndPosition, pos, 0);
}


double
MSPModel_NonInteracting::CState::getAngle(const MSStageMoving& stage, SUMOTime now) const {
    double angle = stage.getEdgeAngle(stage.getEdge(), getEdgePos(stage, now)) + (myCurrentEndPos < myCurrentBeginPos ? 1.5 * M_PI : 0.5 * M_PI);
    if (angle > M_PI) {
        angle -= 2 * M_PI;
    }
    return angle;
}


SUMOTime
MSPModel_NonInteracting::CState::computeTranshipTime(const MSEdge* /* prev */, const MSStageMoving& stage, SUMOTime currentTime) {
    myLastEntryTime = currentTime;

    myCurrentBeginPos = stage.getDepartPos();
    myCurrentEndPos = stage.getArrivalPos();

    const MSLane* fromLane = stage.getFromEdge()->getLanes().front(); //the lane the container starts from during its tranship stage
    myCurrentBeginPosition = stage.getLanePosition(fromLane, myCurrentBeginPos, LATERAL_OFFSET);
    const MSLane* toLane = stage.getEdges().back()->getLanes().front(); //the lane the container ends during its tranship stage
    myCurrentEndPosition = stage.getLanePosition(toLane, myCurrentEndPos, LATERAL_OFFSET);

    myCurrentDuration = MAX2((SUMOTime)1, TIME2STEPS(fabs(myCurrentEndPosition.distanceTo(myCurrentBeginPosition)) / stage.getMaxSpeed()));
    return myCurrentDuration;
}


/****************************************************************************/
