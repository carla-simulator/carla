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
/// @file    MSLaneChanger.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Friedemann Wesner
/// @author  Jakob Erdmann
/// @date    Fri, 01 Feb 2002
///
// Performs lane changing of vehicles
/****************************************************************************/
#include <config.h>

#include "MSLaneChanger.h"
#include "MSNet.h"
#include "MSVehicle.h"
#include "MSVehicleType.h"
#include "MSVehicleTransfer.h"
#include "MSGlobals.h"
#include <cassert>
#include <iterator>
#include <cstdlib>
#include <cmath>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/transportables/MSPModel.h>
#include <utils/common/MsgHandler.h>

#define OPPOSITE_OVERTAKING_SAFE_TIMEGAP 0.0
#define OPPOSITE_OVERTAKING_SAFETYGAP_HEADWAY_FACTOR 0.0
#define OPPOSITE_OVERTAKING_SAFETY_FACTOR 1.2
// XXX maxLookAhead should be higher if all leaders are stopped and lower when they are jammed/queued
#define OPPOSITE_OVERTAKING_MAX_LOOKAHEAD 150.0 // just a guess
#define OPPOSITE_OVERTAKING_MAX_LOOKAHEAD_EMERGENCY 1000.0 // just a guess
// this is used for finding oncoming vehicles while driving in the opposite direction
#define OPPOSITE_OVERTAKING_ONCOMING_LOOKAHEAD 200.0 // just a guess

// ===========================================================================
// debug defines
// ===========================================================================

//#define DEBUG_CONTINUE_CHANGE
//#define DEBUG_CHECK_CHANGE
//#define DEBUG_SURROUNDING_VEHICLES // debug getRealFollower() and getRealLeader()
//#define DEBUG_CHANGE_OPPOSITE
//#define DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
//#define DEBUG_ACTIONSTEPS
//#define DEBUG_STATE
//#define DEBUG_CANDIDATE
//#define DEBUG_COND (vehicle->getLaneChangeModel().debugVehicle())
#define DEBUG_COND (vehicle->isSelected())



// ===========================================================================
// ChangeElem member method definitions
// ===========================================================================
MSLaneChanger::ChangeElem::ChangeElem(MSLane* _lane) :
    lead(nullptr),
    lane(_lane),
    hoppedVeh(nullptr),
    lastBlocked(nullptr),
    firstBlocked(nullptr),
    ahead(lane),
    aheadNext(lane, nullptr, 0) {
}

void
MSLaneChanger::ChangeElem::registerHop(MSVehicle* vehicle) {
    //std::cout << SIMTIME << " registerHop lane=" << lane->getID() << " veh=" << vehicle->getID() << "\n";
    lane->myTmpVehicles.insert(lane->myTmpVehicles.begin(), vehicle);
    dens += vehicle->getVehicleType().getLengthWithGap();
    hoppedVeh = vehicle;
}


// ===========================================================================
// member method definitions
// ===========================================================================
MSLaneChanger::MSLaneChanger(const std::vector<MSLane*>* lanes, bool allowChanging) :
    myAllowsChanging(allowChanging),
    myChangeToOpposite(lanes->front()->getEdge().canChangeToOpposite()) {

    // Fill the changer with the lane-data.
    myChanger.reserve(lanes->size());
    for (std::vector<MSLane*>::const_iterator lane = lanes->begin(); lane != lanes->end(); ++lane) {
        myChanger.push_back(ChangeElem(*lane));
        myChanger.back().mayChangeRight = lane != lanes->begin();
        myChanger.back().mayChangeLeft = (lane + 1) != lanes->end();
        // avoid changing on internal sibling lane
        if ((*lane)->isInternal()) {
            if (myChanger.back().mayChangeRight && (*lane)->getLogicalPredecessorLane() == (*(lane - 1))->getLogicalPredecessorLane()) {
                myChanger.back().mayChangeRight = false;
            }
            if (myChanger.back().mayChangeLeft && (*lane)->getLogicalPredecessorLane() == (*(lane + 1))->getLogicalPredecessorLane()) {
                myChanger.back().mayChangeLeft = false;
            }
        }
    }
}


MSLaneChanger::~MSLaneChanger() {
}


void
MSLaneChanger::laneChange(SUMOTime t) {
    // This is what happens in one timestep. After initialization of the
    // changer, each vehicle will try to change. After that the changer
    // needs an update to prevent multiple changes of one vehicle.
    // Finally, the change-result has to be given back to the lanes.
    initChanger();
    try {
        while (vehInChanger()) {
            const bool haveChanged = change();
            updateChanger(haveChanged);
        }
        updateLanes(t);
    } catch (const ProcessError&) {
        // clean up locks or the gui may hang
        for (ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce) {
            ce->lane->releaseVehicles();
        }
        throw;
    }
}


void
MSLaneChanger::initChanger() {
    // Prepare myChanger with a safe state.
    for (ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce) {
        ce->lead = nullptr;
        ce->hoppedVeh = nullptr;
        ce->lastBlocked = nullptr;
        ce->firstBlocked = nullptr;
        ce->dens = 0;
        ce->lane->getVehiclesSecure();

        //std::cout << SIMTIME << " initChanger lane=" << ce->lane->getID() << " vehicles=" << toString(ce->lane->myVehicles) << "\n";
    }
}


void
MSLaneChanger::updateChanger(bool vehHasChanged) {
    assert(veh(myCandi) != 0);

    // "Push" the vehicles to the back, i.e. follower becomes vehicle,
    // vehicle becomes leader, and leader becomes predecessor of vehicle,
    // if it exists.
    if (!vehHasChanged) {
        //std::cout << SIMTIME << " updateChanger: lane=" << myCandi->lane->getID() << " has new lead=" << veh(myCandi)->getID() << "\n";
        myCandi->lead = veh(myCandi);
    }

    MSLane::VehCont& vehicles = myCandi->lane->myVehicles;
    vehicles.pop_back();
    //std::cout << SIMTIME << " updateChanger lane=" << myCandi->lane->getID() << " vehicles=" << toString(myCandi->lane->myVehicles) << "\n";
}


void
MSLaneChanger::updateLanes(SUMOTime t) {

    // Update the lane's vehicle-container.
    // First: it is bad style to change other classes members, but for
    // this release, other attempts were too time-consuming. In a next
    // release we will change from this lane-centered design to a vehicle-
    // centered. This will solve many problems.
    // Second: this swap would be faster if vehicle-containers would have
    // been pointers, but then I had to change too much of the MSLane code.
    for (ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce) {
        //std::cout << SIMTIME << " updateLanes lane=" << ce->lane->getID() << " myVehicles=" << toString(ce->lane->myVehicles) << " myTmpVehicles=" << toString(ce->lane->myTmpVehicles) << "\n";
        ce->lane->swapAfterLaneChange(t);
        ce->lane->releaseVehicles();
    }
}


MSLaneChanger::ChangerIt
MSLaneChanger::findCandidate() {
    // Find the vehicle in myChanger with the largest position. If there
    // is no vehicle in myChanger (shouldn't happen) , return myChanger.end().
    ChangerIt max = myChanger.end();
#ifdef DEBUG_CANDIDATE
    std::cout << SIMTIME << " findCandidate() on edge " << myChanger.begin()->lane->getEdge().getID() << std::endl;
#endif

    for (ChangerIt ce = myChanger.begin(); ce != myChanger.end(); ++ce) {
        if (veh(ce) == nullptr) {
            continue;
        }
#ifdef DEBUG_CANDIDATE
        std::cout << "     lane = " << ce->lane->getID() << "\n";
        std::cout << "     check vehicle=" << veh(ce)->getID() << " pos=" << veh(ce)->getPositionOnLane() << " lane=" << ce->lane->getID() << " isFrontOnLane=" << veh(ce)->isFrontOnLane(ce->lane)  << "\n";
#endif
        if (max == myChanger.end()) {
#ifdef DEBUG_CANDIDATE
            std::cout << "     new max vehicle=" << veh(ce)->getID() << " pos=" << veh(ce)->getPositionOnLane() << " lane=" << ce->lane->getID() << " isFrontOnLane=" << veh(ce)->isFrontOnLane(ce->lane)  << "\n";
#endif
            max = ce;
            continue;
        }
        assert(veh(ce)  != 0);
        assert(veh(max) != 0);
        if (veh(max)->getPositionOnLane() < veh(ce)->getPositionOnLane()) {
#ifdef DEBUG_CANDIDATE
            std::cout << "     new max vehicle=" << veh(ce)->getID() << " pos=" << veh(ce)->getPositionOnLane() << " lane=" << ce->lane->getID() << " isFrontOnLane=" << veh(ce)->isFrontOnLane(ce->lane)  << " oldMaxPos=" << veh(max)->getPositionOnLane() << "\n";
#endif
            max = ce;
        }
    }
    assert(max != myChanger.end());
    assert(veh(max) != 0);
    return max;
}


bool
MSLaneChanger::mayChange(int direction) const {
    if (direction == 0) {
        return true;
    }
    if (!myAllowsChanging) {
        return false;
    }
    if (direction == -1) {
        return myCandi->mayChangeRight && (myCandi - 1)->lane->allowsVehicleClass(veh(myCandi)->getVehicleType().getVehicleClass());
    } else if (direction == 1) {
        return myCandi->mayChangeLeft && (myCandi + 1)->lane->allowsVehicleClass(veh(myCandi)->getVehicleType().getVehicleClass());
    } else {
        return false;
    }
}


bool
MSLaneChanger::change() {
    // Find change-candidate. If it is on an allowed lane, try to change
    // to the right (there is a rule in Germany that you have to change
    // to the right, unless you are overtaking). If change to the right
    // isn't possible, check if there is a possibility to overtake (on the
    // left.
    // If candidate isn't on an allowed lane, changing to an allowed has
    // priority.

#ifdef DEBUG_ACTIONSTEPS
//        std::cout<< "\nCHANGE" << std::endl;
#endif


    myCandi = findCandidate();
    MSVehicle* vehicle = veh(myCandi);
    vehicle->getLaneChangeModel().clearNeighbors();

    if (vehicle->getLaneChangeModel().isChangingLanes() && !vehicle->getLaneChangeModel().alreadyChanged()) {
        return continueChange(vehicle, myCandi);
    }
    vehicle->getLaneChangeModel().setSpeedLat(0);
    if (!myAllowsChanging || vehicle->getLaneChangeModel().alreadyChanged() || vehicle->isStoppedOnLane()) {
        registerUnchanged(vehicle);
        return false;
    }

    if (!vehicle->isActive()) {
#ifdef DEBUG_ACTIONSTEPS
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh '" << vehicle->getID() << "' skips regular change checks." << std::endl;
        }
#endif
        bool changed = false;
        const int oldstate = vehicle->getLaneChangeModel().getOwnState();
        // let TraCI influence the wish to change lanes during non-actionsteps
        checkTraCICommands(vehicle);
        if (oldstate != vehicle->getLaneChangeModel().getOwnState()) {
            changed = applyTraCICommands(vehicle);
        }
        if (!changed) {
            registerUnchanged(vehicle);
        }
        return changed;
    }

    // Check for changes to the opposite lane if vehicle is active
    std::pair<MSVehicle* const, double> leader = getRealLeader(myCandi);
    if (myChanger.size() == 1 || vehicle->getLaneChangeModel().isOpposite() || (!mayChange(-1) && !mayChange(1))) {
        if (changeOpposite(leader)) {
            return true;
        }
        registerUnchanged(vehicle);
        return false;
    }

    vehicle->updateBestLanes(); // needed?
    for (int i = 0; i < (int) myChanger.size(); ++i) {
        vehicle->adaptBestLanesOccupation(i, myChanger[i].dens);
    }

    const std::vector<MSVehicle::LaneQ>& preb = vehicle->getBestLanes();
    // check whether the vehicle wants and is able to change to right lane
    int stateRight = 0;
    if (mayChange(-1)) {
        stateRight = checkChangeWithinEdge(-1, leader, preb);
        // change if the vehicle wants to and is allowed to change
        if ((stateRight & LCA_RIGHT) != 0 && (stateRight & LCA_BLOCKED) == 0) {
            vehicle->getLaneChangeModel().setOwnState(stateRight);
            return startChange(vehicle, myCandi, -1);
        }
        if ((stateRight & LCA_RIGHT) != 0 && (stateRight & LCA_URGENT) != 0) {
            (myCandi - 1)->lastBlocked = vehicle;
            if ((myCandi - 1)->firstBlocked == nullptr) {
                (myCandi - 1)->firstBlocked = vehicle;
            }
        }
    }

    // check whether the vehicle wants and is able to change to left lane
    int stateLeft = 0;
    if (mayChange(1)) {
        stateLeft = checkChangeWithinEdge(1, leader, preb);
        // change if the vehicle wants to and is allowed to change
        if ((stateLeft & LCA_LEFT) != 0 && (stateLeft & LCA_BLOCKED) == 0) {
            vehicle->getLaneChangeModel().setOwnState(stateLeft);
            return startChange(vehicle, myCandi, 1);
        }
        if ((stateLeft & LCA_LEFT) != 0 && (stateLeft & LCA_URGENT) != 0) {
            (myCandi + 1)->lastBlocked = vehicle;
            if ((myCandi + 1)->firstBlocked == nullptr) {
                (myCandi + 1)->firstBlocked = vehicle;
            }
        }
    }

    if ((stateRight & LCA_URGENT) != 0 && (stateLeft & LCA_URGENT) != 0) {
        // ... wants to go to the left AND to the right
        // just let them go to the right lane...
        stateLeft = 0;
    }
    vehicle->getLaneChangeModel().setOwnState(stateRight | stateLeft);

    // only emergency vehicles should change to the opposite side on a
    // multi-lane road
    if (vehicle->getVehicleType().getVehicleClass() == SVC_EMERGENCY
            && changeOpposite(leader)) {
        return true;
    }

    registerUnchanged(vehicle);
    return false;
}


void
MSLaneChanger::registerUnchanged(MSVehicle* vehicle) {
    //std::cout << SIMTIME << " registerUnchanged lane=" << myCandi->lane->getID() << " veh=" << vehicle->getID() << "\n";
    myCandi->lane->myTmpVehicles.insert(myCandi->lane->myTmpVehicles.begin(), veh(myCandi));
    myCandi->dens += vehicle->getVehicleType().getLengthWithGap();
    vehicle->getLaneChangeModel().unchanged();
}



void
MSLaneChanger::checkTraCICommands(MSVehicle* vehicle) {
#ifdef DEBUG_STATE
    const int oldstate = vehicle->getLaneChangeModel().getOwnState();
#endif
    vehicle->getLaneChangeModel().checkTraCICommands();
#ifdef DEBUG_STATE
    if (DEBUG_COND) {
        const int newstate = vehicle->getLaneChangeModel().getOwnState();
        std::cout << SIMTIME
                  << " veh=" << vehicle->getID()
                  << " oldState=" << toString((LaneChangeAction) oldstate)
                  << " newState=" << toString((LaneChangeAction) newstate)
                  << ((newstate & LCA_BLOCKED) != 0 ? " (blocked)" : "")
                  << ((newstate & LCA_OVERLAPPING) != 0 ? " (overlap)" : "")
                  << "\n";
    }
#endif
}


bool
MSLaneChanger::applyTraCICommands(MSVehicle* vehicle) {
    // Execute request if not blocked
    bool changed = false;
    const int state = vehicle->getLaneChangeModel().getOwnState();
    const int dir = (state & LCA_RIGHT) != 0 ? -1 : ((state & LCA_LEFT) != 0 ? 1 : 0);
    const bool execute = dir != 0 && ((state & LCA_BLOCKED) == 0);
    if (execute) {
        ChangerIt to = myCandi + dir;
        bool continuous = vehicle->getLaneChangeModel().startLaneChangeManeuver(myCandi->lane, to->lane, dir);
        if (continuous) {
            changed = continueChange(vehicle, myCandi);
        } else {
            // insert vehicle into target lane
            to->registerHop(vehicle);
            changed = true;
        }
    }
    return changed;
}


bool
MSLaneChanger::startChange(MSVehicle* vehicle, ChangerIt& from, int direction) {
    if (vehicle->isRemoteControlled()) {
        registerUnchanged(vehicle);
        return false;
    }
    ChangerIt to = from + direction;
    // @todo delay entering the target lane until the vehicle intersects it
    //       physically (considering lane width and vehicle width)
    //if (to->lane->getID() == "beg_1") std::cout << SIMTIME << " startChange to lane=" << to->lane->getID() << " myTmpVehiclesBefore=" << toString(to->lane->myTmpVehicles) << "\n";
    const bool continuous = vehicle->getLaneChangeModel().startLaneChangeManeuver(from->lane, to->lane, direction);
    if (continuous) {
        return continueChange(vehicle, myCandi);
    } else {
        to->registerHop(vehicle);
        to->lane->requireCollisionCheck();
        return true;
    }
}

bool
MSLaneChanger::continueChange(MSVehicle* vehicle, ChangerIt& from) {
    MSAbstractLaneChangeModel& lcm = vehicle->getLaneChangeModel();
    const int direction = lcm.isOpposite() ? 1 : lcm.getLaneChangeDirection();
    const bool pastMidpoint = lcm.updateCompletion(); // computes lcm.mySpeedLat as a side effect
    const double speedLat = lcm.isOpposite() ? -lcm.getSpeedLat() : lcm.getSpeedLat();
    vehicle->myState.myPosLat += SPEED2DIST(speedLat);
    vehicle->myCachedPosition = Position::INVALID;
    //std::cout << SIMTIME << " veh=" << vehicle->getID() << " dir=" << direction << " pm=" << pastMidpoint << " speedLat=" << speedLat << " posLat=" << vehicle->myState.myPosLat << "\n";
    if (pastMidpoint) {
        MSLane* source = myCandi->lane;
        MSLane* target = source->getParallelLane(direction);
        vehicle->myState.myPosLat -= direction * 0.5 * (source->getWidth() + target->getWidth());
        lcm.primaryLaneChanged(source, target, direction);
        if (&source->getEdge() == &target->getEdge()) {
            ChangerIt to = from + direction;
            to->registerHop(vehicle);
        }
        target->requireCollisionCheck();
    } else {
        from->registerHop(vehicle);
        from->lane->requireCollisionCheck();
    }
    if (!lcm.isChangingLanes()) {
        vehicle->myState.myPosLat = 0;
        lcm.endLaneChangeManeuver();
    }
    lcm.updateShadowLane();
    if (lcm.getShadowLane() != nullptr && &lcm.getShadowLane()->getEdge() == &vehicle->getLane()->getEdge()) {
        // set as hoppedVeh on the shadow lane so it is found as leader on both lanes
        ChangerIt shadow = pastMidpoint ? from : from + lcm.getShadowDirection();
        shadow->hoppedVeh = vehicle;
        lcm.getShadowLane()->requireCollisionCheck();
    }
    vehicle->myAngle = vehicle->computeAngle();
    if (lcm.isOpposite()) {
        vehicle->myAngle += M_PI;
    }

#ifdef DEBUG_CONTINUE_CHANGE
    if (DEBUG_COND) {
        std::cout << SIMTIME
                  << " continueChange veh=" << vehicle->getID()
                  << " from=" << Named::getIDSecure(from->lane)
                  << " dir=" << direction
                  << " speedLat=" << speedLat
                  << " pastMidpoint=" << pastMidpoint
                  << " posLat=" << vehicle->getLateralPositionOnLane()
                  //<< " completion=" << lcm.getLaneChangeCompletion()
                  << " shadowLane=" << Named::getIDSecure(lcm.getShadowLane())
                  //<< " shadowHopped=" << Named::getIDSecure(shadow->lane)
                  << "\n";
    }
#endif
    return pastMidpoint && lcm.getShadowLane() == nullptr;
}


std::pair<MSVehicle* const, double>
MSLaneChanger::getRealLeader(const ChangerIt& target) const {
    assert(veh(myCandi) != 0);
    MSVehicle* vehicle = veh(myCandi);
#ifdef DEBUG_SURROUNDING_VEHICLES
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh '" << vehicle->getID() << "' looks for leader on lc-target lane '" << target->lane->getID() << "'." << std::endl;
    }
#endif
    // get the leading vehicle on the lane to change to
    MSVehicle* neighLead = target->lead;

#ifdef DEBUG_SURROUNDING_VEHICLES
    if (DEBUG_COND) {
        if (neighLead != 0) {
            std::cout << "Considering '" << neighLead->getID() << "' at position " << neighLead->getPositionOnLane() << std::endl;
        }
    }
#endif

    // check whether the hopped vehicle became the leader
    if (target->hoppedVeh != nullptr) {
        double hoppedPos = target->hoppedVeh->getPositionOnLane();
#ifdef DEBUG_SURROUNDING_VEHICLES
        if (DEBUG_COND) {
            std::cout << "Considering hopped vehicle '" << target->hoppedVeh->getID() << "' at position " << hoppedPos << std::endl;
        }
#endif
        if (hoppedPos > vehicle->getPositionOnLane() && (neighLead == nullptr || neighLead->getPositionOnLane() > hoppedPos)) {
            neighLead = target->hoppedVeh;
            //if (vehicle->getID() == "flow.21") std::cout << SIMTIME << " neighLead=" << Named::getIDSecure(neighLead) << " (422)\n";
        }
    }
    if (neighLead == nullptr) {
#ifdef DEBUG_SURROUNDING_VEHICLES
        if (DEBUG_COND) {
            std::cout << "Looking for leader on consecutive lanes." << std::endl;
        }
#endif
        // There's no leader on the target lane. Look for leaders on consecutive lanes.
        // (there might also be partial leaders due to continuous lane changing)
        MSLane* targetLane = target->lane;
        const double egoBack = vehicle->getBackPositionOnLane();
        double leaderBack = targetLane->getLength();
        for (MSVehicle* pl : targetLane->myPartialVehicles) {
            double plBack = pl->getBackPositionOnLane(targetLane);
            if (plBack < leaderBack &&
                    pl->getPositionOnLane(targetLane) + pl->getVehicleType().getMinGap() >= egoBack) {
                neighLead = pl;
                leaderBack = plBack;
            }
        }
        if (neighLead != nullptr) {
#ifdef DEBUG_SURROUNDING_VEHICLES
            if (DEBUG_COND) {
                std::cout << "  found leader=" << neighLead->getID() << " (partial)\n";
            }
#endif
            return std::pair<MSVehicle*, double>(neighLead, leaderBack - vehicle->getPositionOnLane() - vehicle->getVehicleType().getMinGap());
        }
        double seen = myCandi->lane->getLength() - vehicle->getPositionOnLane();
        double speed = vehicle->getSpeed();
        double dist = vehicle->getCarFollowModel().brakeGap(speed) + vehicle->getVehicleType().getMinGap();
        // always check for link leaders while on an internal lane
        if (seen > dist && !myCandi->lane->isInternal()) {
#ifdef DEBUG_SURROUNDING_VEHICLES
            if (DEBUG_COND) {
                std::cout << "  found no leader within dist=" << dist << "\n";
            }
#endif
            return std::pair<MSVehicle* const, double>(static_cast<MSVehicle*>(nullptr), -1);
        }
        const std::vector<MSLane*>& bestLaneConts = vehicle->getBestLanesContinuation(targetLane);

        std::pair<MSVehicle* const, double> result = target->lane->getLeaderOnConsecutive(dist, seen, speed, *vehicle, bestLaneConts);
#ifdef DEBUG_SURROUNDING_VEHICLES
        if (DEBUG_COND) {
            std::cout << "  found consecutiveLeader=" << Named::getIDSecure(result.first) << "\n";
        }
#endif
        return result;
    } else {
#ifdef DEBUG_SURROUNDING_VEHICLES
        if (DEBUG_COND) {
            std::cout << "  found leader=" << neighLead->getID() << "\n";
        }
#endif
        return std::pair<MSVehicle* const, double>(neighLead, neighLead->getBackPositionOnLane(target->lane) - vehicle->getPositionOnLane() - vehicle->getVehicleType().getMinGap());
    }
}


std::pair<MSVehicle* const, double>
MSLaneChanger::getRealFollower(const ChangerIt& target) const {
    assert(veh(myCandi) != 0);
    MSVehicle* vehicle = veh(myCandi);
#ifdef DEBUG_SURROUNDING_VEHICLES
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh '" << vehicle->getID() << "' looks for follower on lc-target lane '" << target->lane->getID() << "'." << std::endl;
    }
#endif
    const double candiPos = vehicle->getPositionOnLane();
    MSVehicle* neighFollow = veh(target);

#ifdef DEBUG_SURROUNDING_VEHICLES
    if (DEBUG_COND) {
        if (neighFollow != 0) {
            std::cout << "veh(target) returns '" << neighFollow->getID() << "' at position " << neighFollow->getPositionOnLane() << std::endl;
        } else {
            std::cout << "veh(target) returns none." << std::endl;
        }
    }
#endif


#ifdef DEBUG_SURROUNDING_VEHICLES
    if (DEBUG_COND) {
        if (getCloserFollower(candiPos, neighFollow, target->hoppedVeh) != neighFollow) {
            std::cout << "Hopped vehicle '" << target->hoppedVeh->getID() << "' at position " << target->hoppedVeh->getPositionOnLane() << " is closer." <<  std::endl;
        }
    }
#endif

    // check whether the hopped vehicle became the follower
    neighFollow = getCloserFollower(candiPos, neighFollow, target->hoppedVeh);


#ifdef DEBUG_SURROUNDING_VEHICLES
    if (DEBUG_COND) {
        MSVehicle* partialBehind = getCloserFollower(candiPos, neighFollow, target->lane->getPartialBehind(vehicle));
        if (partialBehind != 0 && partialBehind != neighFollow) {
            std::cout << "'Partial behind'-vehicle '" << target->lane->getPartialBehind(vehicle)->getID() << "' at position " << partialBehind->getPositionOnLane() << " is closer." <<  std::endl;
        }
    }
#endif
    // or a follower which is partially lapping into the target lane
    neighFollow = getCloserFollower(candiPos, neighFollow, target->lane->getPartialBehind(vehicle));

    if (neighFollow == nullptr) {
        CLeaderDist consecutiveFollower = target->lane->getFollowersOnConsecutive(vehicle, vehicle->getBackPositionOnLane(), true)[0];
#ifdef DEBUG_SURROUNDING_VEHICLES
        if (DEBUG_COND) {
            if (consecutiveFollower.first == 0) {
                std::cout << "no follower found." <<  std::endl;
            } else {
                std::cout << "found follower '" << consecutiveFollower.first->getID() << "' on consecutive lanes." <<  std::endl;
            }
        }
#endif
        return std::make_pair(const_cast<MSVehicle*>(consecutiveFollower.first), consecutiveFollower.second);
    } else {
#ifdef DEBUG_SURROUNDING_VEHICLES
        if (DEBUG_COND) {
            std::cout << "found follower '" << neighFollow->getID() << "'." <<  std::endl;
        }
#endif
        return std::pair<MSVehicle* const, double>(neighFollow,
                vehicle->getPositionOnLane() - vehicle->getVehicleType().getLength() - neighFollow->getPositionOnLane() - neighFollow->getVehicleType().getMinGap());
    }
}


MSVehicle*
MSLaneChanger::getCloserFollower(const double maxPos, MSVehicle* follow1, MSVehicle* follow2) {
    if (follow1 == nullptr || follow1->getPositionOnLane() > maxPos) {
        return follow2;
    } else if (follow2 == nullptr || follow2->getPositionOnLane() > maxPos) {
        return follow1;
    } else {
        if (follow1->getPositionOnLane() > follow2->getPositionOnLane()) {
            return follow1;
        } else {
            return follow2;
        }
    }
}

int
MSLaneChanger::checkChangeWithinEdge(
    int laneOffset,
    const std::pair<MSVehicle* const, double>& leader,
    const std::vector<MSVehicle::LaneQ>& preb) const {

    std::pair<MSVehicle* const, double> neighLead = getRealLeader(myCandi + laneOffset);
    std::pair<MSVehicle*, double> neighFollow = getRealFollower(myCandi + laneOffset);
    if (neighLead.first != nullptr && neighLead.first == neighFollow.first) {
        // vehicles should not be leader and follower at the same time to avoid
        // contradictory behavior
        neighFollow.first = 0;
    }
    ChangerIt target = myCandi + laneOffset;
    return checkChange(laneOffset, target->lane, leader, neighLead, neighFollow, preb);
}

int
MSLaneChanger::checkChange(
    int laneOffset,
    const MSLane* targetLane,
    const std::pair<MSVehicle* const, double>& leader,
    const std::pair<MSVehicle* const, double>& neighLead,
    const std::pair<MSVehicle* const, double>& neighFollow,
    const std::vector<MSVehicle::LaneQ>& preb) const {

    MSVehicle* vehicle = veh(myCandi);

#ifdef DEBUG_CHECK_CHANGE
    if (DEBUG_COND) {
        std::cout
                << "\n" << SIMTIME << " checkChange() for vehicle '" << vehicle->getID() << "'"
                << std::endl;
    }
#endif

    int blocked = 0;
    int blockedByLeader = (laneOffset == -1 ? LCA_BLOCKED_BY_RIGHT_LEADER : LCA_BLOCKED_BY_LEFT_LEADER);
    int blockedByFollower = (laneOffset == -1 ? LCA_BLOCKED_BY_RIGHT_FOLLOWER : LCA_BLOCKED_BY_LEFT_FOLLOWER);
    // overlap
    if (neighFollow.first != nullptr && neighFollow.second < 0) {
        blocked |= (blockedByFollower | LCA_OVERLAPPING);

        // Debug (Leo)
#ifdef DEBUG_CHECK_CHANGE
        if (DEBUG_COND) {
            std::cout << SIMTIME
                      << " overlapping with follower..."
                      << std::endl;
        }
#endif

    }
    if (neighLead.first != nullptr && neighLead.second < 0) {
        blocked |= (blockedByLeader | LCA_OVERLAPPING);

#ifdef DEBUG_CHECK_CHANGE
        if (DEBUG_COND) {
            std::cout << SIMTIME
                      <<  " overlapping with leader..."
                      << std::endl;
        }
#endif

    }
    double secureFrontGap = MSAbstractLaneChangeModel::NO_NEIGHBOR;
    double secureBackGap = MSAbstractLaneChangeModel::NO_NEIGHBOR;
    double secureOrigFrontGap = MSAbstractLaneChangeModel::NO_NEIGHBOR;

    const double tauRemainder = vehicle->getActionStepLength() == DELTA_T ? 0 : MAX2(vehicle->getCarFollowModel().getHeadwayTime() - TS, 0.);
    // safe back gap
    if ((blocked & blockedByFollower) == 0 && neighFollow.first != nullptr) {
        // Calculate secure gap conservatively with vNextFollower / vNextLeader as
        // extrapolated speeds after the driver's expected reaction time (tau).
        // NOTE: there exists a possible source for collisions if the follower and the leader
        //       have desynchronized action steps as the extrapolated speeds can be exceeded in this case

        // Expected reaction time (tau) for the follower-vehicle.
        // (substracted TS since at this point the vehicles' states are already updated)
        const double vNextFollower = neighFollow.first->getSpeed() + MAX2(0., tauRemainder * neighFollow.first->getAcceleration());
        const double vNextLeader = vehicle->getSpeed() + MIN2(0., tauRemainder * vehicle->getAcceleration());
        // !!! eigentlich: vsafe braucht die Max. Geschwindigkeit beider Spuren
        secureBackGap = neighFollow.first->getCarFollowModel().getSecureGap(neighFollow.first, vehicle, vNextFollower,
                        vNextLeader, vehicle->getCarFollowModel().getMaxDecel());
#ifdef DEBUG_CHECK_CHANGE
        if (DEBUG_COND) {
            std::cout << SIMTIME
                      << " backGap=" << neighFollow.second
                      << " vNextFollower=" << vNextFollower
                      << " vNextEgo=" << vNextLeader
                      << " secureGap=" << secureBackGap
                      << " safetyFactor=" << vehicle->getLaneChangeModel().getSafetyFactor()
                      << " blocked=" << (neighFollow.second < secureBackGap * vehicle->getLaneChangeModel().getSafetyFactor())
                      << "\n";
        }
#endif
        if (neighFollow.second < secureBackGap * vehicle->getLaneChangeModel().getSafetyFactor()) {
            blocked |= blockedByFollower;
        }
    }

    // safe front gap
    if ((blocked & blockedByLeader) == 0 && neighLead.first != nullptr) {
        // Calculate secure gap conservatively with vNextFollower / vNextLeader as
        // extrapolated speeds after the driver's expected reaction time (tau).
        // NOTE: there exists a possible source for collisions if the follower and the leader
        //       have desynchronized action steps as the extrapolated speeds can be exceeded in this case

        // Expected reaction time (tau) for the follower-vehicle.
        // (substracted TS since at this point the vehicles' states are already updated)
        const double vNextFollower = vehicle->getSpeed() + MAX2(0., tauRemainder * vehicle->getAcceleration());
        const double vNextLeader = neighLead.first->getSpeed() + MIN2(0., tauRemainder * neighLead.first->getAcceleration());
        // !!! eigentlich: vsafe braucht die Max. Geschwindigkeit beider Spuren
        secureFrontGap = vehicle->getCarFollowModel().getSecureGap(vehicle, neighLead.first, vNextFollower,
                         vNextLeader, neighLead.first->getCarFollowModel().getMaxDecel());
#ifdef DEBUG_CHECK_CHANGE
        if (DEBUG_COND) {
            std::cout << SIMTIME
                      << " frontGap=" << neighFollow.second
                      << " vNextEgo=" << vNextFollower
                      << " vNextLeader=" << vNextLeader
                      << " secureGap=" << secureFrontGap
                      << " safetyFactor=" << vehicle->getLaneChangeModel().getSafetyFactor()
                      << " blocked=" << (neighLead.second < secureFrontGap * vehicle->getLaneChangeModel().getSafetyFactor())
                      << "\n";
        }
#endif
        if (neighLead.second < secureFrontGap * vehicle->getLaneChangeModel().getSafetyFactor()) {
            blocked |= blockedByLeader;
        }
    }
    if (blocked == 0 && targetLane->hasPedestrians()) {
        PersonDist leader = targetLane->nextBlocking(vehicle->getBackPositionOnLane(),
                            vehicle->getRightSideOnLane(), vehicle->getRightSideOnLane() + vehicle->getVehicleType().getWidth(),
                            ceil(vehicle->getSpeed() / vehicle->getCarFollowModel().getMaxDecel()));
        if (leader.first != 0) {
            const double brakeGap = vehicle->getCarFollowModel().brakeGap(vehicle->getSpeed());
            // returned gap value is relative to backPosition
            const double gap = leader.second - vehicle->getVehicleType().getLengthWithGap();
#ifdef DEBUG_CHECK_CHANGE
            if (DEBUG_COND) {
                std::cout << SIMTIME << "  pedestrian on road " + leader.first->getID() << " gap=" << gap << " brakeGap=" << brakeGap << "\n";
            }
#endif
            if (brakeGap > gap) {
                blocked |= blockedByLeader;
#ifdef DEBUG_CHECK_CHANGE
                if (DEBUG_COND) {
                    std::cout << SIMTIME << "  blocked by pedestrian " + leader.first->getID() << "\n";
                }
#endif
            }
        }
    }

    if (leader.first != nullptr) {
        secureOrigFrontGap = vehicle->getCarFollowModel().getSecureGap(vehicle, leader.first, vehicle->getSpeed(), leader.first->getSpeed(), leader.first->getCarFollowModel().getMaxDecel());
    }

    MSAbstractLaneChangeModel::MSLCMessager msg(leader.first, neighLead.first, neighFollow.first);
    int state = blocked | vehicle->getLaneChangeModel().wantsChange(
                    laneOffset, msg, blocked, leader, neighLead, neighFollow, *targetLane, preb, &(myCandi->lastBlocked), &(myCandi->firstBlocked));

    if (blocked == 0 && (state & LCA_WANTS_LANECHANGE) != 0 && neighLead.first != nullptr) {
        // do a more careful (but expensive) check to ensure that a
        // safety-critical leader is not being overlooked
        // while changing on an intersection, it is not sufficient to abort the
        // search with a leader on the current lane because all linkLeaders must
        // be considered as well
        const double seen = myCandi->lane->getLength() - vehicle->getPositionOnLane();
        const double speed = vehicle->getSpeed();
        const double dist = vehicle->getCarFollowModel().brakeGap(speed) + vehicle->getVehicleType().getMinGap();
        if (seen < dist || myCandi->lane->isInternal()) {
            std::pair<MSVehicle* const, double> neighLead2 = targetLane->getCriticalLeader(dist, seen, speed, *vehicle);
            if (neighLead2.first != nullptr && neighLead2.first != neighLead.first) {
                const double secureGap = vehicle->getCarFollowModel().getSecureGap(vehicle, neighLead2.first, vehicle->getSpeed(),
                                         neighLead2.first->getSpeed(), neighLead2.first->getCarFollowModel().getMaxDecel());
                const double secureGap2 = secureGap * vehicle->getLaneChangeModel().getSafetyFactor();
#ifdef DEBUG_SURROUNDING_VEHICLES
                if (DEBUG_COND) {
                    std::cout << SIMTIME << "   found critical leader=" << neighLead2.first->getID()
                              << " gap=" << neighLead2.second << " secGap=" << secureGap << " secGap2=" << secureGap2 << "\n";
                }
#endif
                if (neighLead2.second < secureGap2) {
                    state |= blockedByLeader;
                }
            }
        }
    }
    if (blocked == 0 && (state & LCA_WANTS_LANECHANGE)) {
        // ensure that merging is safe for any upcoming zipper links after changing
        if (vehicle->unsafeLinkAhead(targetLane)) {
            state |= blockedByLeader;
        }
    }

    if ((state & LCA_BLOCKED) == 0 && (state & LCA_WANTS_LANECHANGE) != 0 && MSGlobals::gLaneChangeDuration > DELTA_T) {
        // Ensure that a continuous lane change manoeuvre can be completed before the next turning movement.
        // Assume lateral position == 0. (If this should change in the future add + laneOffset*vehicle->getLateralPositionOnLane() to distToNeighLane)
        const double distToNeighLane = 0.5 * (vehicle->getLane()->getWidth() + targetLane->getWidth());
        // Extrapolate the LC duration if operating with speed dependent lateral speed.
        const MSAbstractLaneChangeModel& lcm = vehicle->getLaneChangeModel();
        const double assumedDecel = lcm.getAssumedDecelForLaneChangeDuration();
        const double estimatedLCDuration = lcm.estimateLCDuration(vehicle->getSpeed(), distToNeighLane, assumedDecel);
        if (estimatedLCDuration == -1) {
            // Can't guarantee that LC will succeed if vehicle is braking -> assert(lcm.myMaxSpeedLatStanding==0)
#ifdef DEBUG_CHECK_CHANGE
            if (DEBUG_COND) {
                std::cout << SIMTIME << " checkChange() too slow to guarantee completion of continuous lane change."
                          << "\nestimatedLCDuration=" << estimatedLCDuration
                          << "\ndistToNeighLane=" << distToNeighLane
                          << std::endl;
            }
#endif
            state |= LCA_INSUFFICIENT_SPEED;
        } else {
            // Compute covered distance, when braking for the whole lc duration
            const double decel = vehicle->getCarFollowModel().getMaxDecel() * estimatedLCDuration;
            const double avgSpeed = 0.5 * (
                                        MAX2(0., vehicle->getSpeed() - ACCEL2SPEED(vehicle->getCarFollowModel().getMaxDecel())) +
                                        MAX2(0., vehicle->getSpeed() - decel));
            // Distance required for lane change.
            const double space2change = avgSpeed * estimatedLCDuration;
            // Available distance for LC maneuver (distance till next turn)
            double seen = myCandi->lane->getLength() - vehicle->getPositionOnLane();
#ifdef DEBUG_CHECK_CHANGE
            if (DEBUG_COND) {
                std::cout << SIMTIME << " checkChange() checking continuous lane change..."
                          << "\ndistToNeighLane=" << distToNeighLane
                          << " estimatedLCDuration=" << estimatedLCDuration
                          << " space2change=" << space2change
                          << " avgSpeed=" << avgSpeed
                          << std::endl;
            }
#endif

            // for finding turns it doesn't matter whether we look along the current lane or the target lane
            const std::vector<MSLane*>& bestLaneConts = vehicle->getBestLanesContinuation();
            int view = 1;
            MSLane* nextLane = vehicle->getLane();
            MSLinkCont::const_iterator link = MSLane::succLinkSec(*vehicle, view, *nextLane, bestLaneConts);
            while (!nextLane->isLinkEnd(link) && seen <= space2change) {
                if ((*link)->getDirection() == LinkDirection::LEFT || (*link)->getDirection() == LinkDirection::RIGHT
                        // the lanes after an internal junction are on different
                        // edges and do not allow lane-changing
                        || (nextLane->getEdge().isInternal() && (*link)->getViaLaneOrLane()->getEdge().isInternal())
                   ) {
                    state |= LCA_INSUFFICIENT_SPACE;
                    break;
                }
                if ((*link)->getViaLane() == nullptr) {
                    view++;
                }
                nextLane = (*link)->getViaLaneOrLane();
                seen += nextLane->getLength();
                // get the next link used
                link = MSLane::succLinkSec(*vehicle, view, *nextLane, bestLaneConts);
            }
#ifdef DEBUG_CHECK_CHANGE
            if (DEBUG_COND) {
                std::cout << " available distance=" << seen << std::endl;
            }
#endif
            if (nextLane->isLinkEnd(link) && seen < space2change) {
#ifdef DEBUG_CHECK_CHANGE
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " checkChange insufficientSpace: seen=" << seen << " space2change=" << space2change << "\n";
                }
#endif
                state |= LCA_INSUFFICIENT_SPACE;
            }

            if ((state & LCA_BLOCKED) == 0) {
                // check for dangerous leaders in case the target lane changes laterally between
                // now and the lane-changing midpoint
                const double speed = vehicle->getSpeed();
                seen = myCandi->lane->getLength() - vehicle->getPositionOnLane();
                nextLane = vehicle->getLane();
                view = 1;
                const double dist = vehicle->getCarFollowModel().brakeGap(speed) + vehicle->getVehicleType().getMinGap();
                MSLinkCont::const_iterator link = MSLane::succLinkSec(*vehicle, view, *nextLane, bestLaneConts);
                while (!nextLane->isLinkEnd(link) && seen <= space2change && seen <= dist) {
                    nextLane = (*link)->getViaLaneOrLane();
                    MSLane* targetLane = nextLane->getParallelLane(laneOffset);
                    if (targetLane == nullptr) {
                        state |= LCA_INSUFFICIENT_SPACE;
                        break;
                    } else {
                        std::pair<MSVehicle* const, double> neighLead2 = targetLane->getLeader(vehicle, -seen, std::vector<MSLane*>());
                        if (neighLead2.first != nullptr && neighLead2.first != neighLead.first
                                && (neighLead2.second < vehicle->getCarFollowModel().getSecureGap(vehicle, neighLead2.first,
                                        vehicle->getSpeed(), neighLead2.first->getSpeed(), neighLead2.first->getCarFollowModel().getMaxDecel()))) {
                            state |= blockedByLeader;
                            break;
                        }
                    }
                    if ((*link)->getViaLane() == nullptr) {
                        view++;
                    }
                    seen += nextLane->getLength();
                    // get the next link used
                    link = MSLane::succLinkSec(*vehicle, view, *nextLane, bestLaneConts);
                }
            }
        }
    }
    const int oldstate = state;
    // let TraCI influence the wish to change lanes and the security to take
    state = vehicle->influenceChangeDecision(state);
#ifdef DEBUG_CHECK_CHANGE
    if (DEBUG_COND) {
        std::cout << SIMTIME
                  << " veh=" << vehicle->getID()
                  << " oldState=" << toString((LaneChangeAction)oldstate)
                  << " newState=" << toString((LaneChangeAction)state)
                  << ((blocked & LCA_BLOCKED) ? " (blocked)" : "")
                  << ((blocked & LCA_OVERLAPPING) ? " (overlap)" : "")
                  << "\n";
    }
#endif
    vehicle->getLaneChangeModel().saveLCState(laneOffset, oldstate, state);
    if (blocked == 0 && (state & LCA_WANTS_LANECHANGE)) {
        // this lane change will be executed, save gaps
        vehicle->getLaneChangeModel().setFollowerGaps(neighFollow, secureBackGap);
        vehicle->getLaneChangeModel().setLeaderGaps(neighLead, secureFrontGap);
        vehicle->getLaneChangeModel().setOrigLeaderGaps(leader, secureOrigFrontGap);
    }
    if (laneOffset != 0) {
        vehicle->getLaneChangeModel().saveNeighbors(laneOffset, neighFollow, neighLead);
    }
    return state;
}


bool
MSLaneChanger::changeOpposite(std::pair<MSVehicle*, double> leader) {
    if (!myChangeToOpposite) {
        return false;
    }
    myCandi = findCandidate();
    MSVehicle* vehicle = veh(myCandi);
    MSLane* source = vehicle->getLane();
    if (vehicle->getLaneChangeModel().getModelID() == LCM_SL2015) {
        // we have warned before but people may still try
        return false;
    }
    if (vehicle->isStopped()) {
        // stopped vehicles obviously should not change lanes. Usually this is
        // prevent by appropriate bestLane distances
        return false;
    }
    const bool isOpposite = vehicle->getLaneChangeModel().isOpposite();
    int ret = 0;
    ret = vehicle->influenceChangeDecision(ret);
    bool oppositeChangeByTraci = false;
    // Check whether a lane change to the opposite direction was requested via TraCI
    if ((ret & (LCA_TRACI)) != 0) {
        if (isOpposite && (ret & LCA_LEFT) != 0) {
            // stay on the opposite side
            return false;
        }
        oppositeChangeByTraci = true;
    }
    if (!isOpposite && leader.first == 0 && !oppositeChangeByTraci) {
        // no reason to change unless there is a leader
        // or we are changing back to the propper direction
        // XXX also check whether the leader is so far away as to be irrelevant
        return false;
    }
    if (!isOpposite && !oppositeChangeByTraci
            && vehicle->getVClass() != SVC_EMERGENCY
            && leader.first != 0) {
        if (leader.first->signalSet(MSGlobals::gLefthand
                                    ? MSVehicle::VEH_SIGNAL_BLINKER_RIGHT : MSVehicle::VEH_SIGNAL_BLINKER_LEFT)) {
            // do not try to overtake a vehicle that is about to turn left or wants
            // to change left itself
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << "   not overtaking leader " << leader.first->getID() << " that has blinker set\n";
            }
#endif
            return false;
        } else if (leader.second < 0) {
            // leaders is either a junction leader (that cannot be overtaken) or something else is wrong
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << "   not overtaking leader " << leader.first->getID() << " with gap " << leader.second << "\n";
            }
#endif
            return false;
        }
    }


    MSLane* opposite = source->getOpposite();
    //There is no lane for opposite driving
    if (opposite == nullptr || !opposite->allowsVehicleClass(vehicle->getVClass())) {
        return false;
    }
    // changing into the opposite direction is always to the left (XXX except for left-hand networkds)
    int direction = isOpposite ? -1 : 1;
    std::pair<MSVehicle*, double> neighLead((MSVehicle*)nullptr, -1);

    // preliminary sanity checks for overtaking space
    double timeToOvertake;
    double spaceToOvertake;

    // we need to find two vehicles:
    // 1) the leader that shall be overtaken (not necessarily the current leader but one of its leaders that has enough space in front)
    // 2) the oncoming vehicle (we need to look past vehicles that are currentlyovertaking through the opposite direction themselves)
    //
    // if the vehicle is driving normally, then the search for 1) starts on the current lane and 2) on the opposite lane
    // if the vehicle is driving on the opposite side then 1) is found on the neighboring lane and 2) on the current lane

    std::pair<MSVehicle*, double> overtaken;

    if (!isOpposite && !oppositeChangeByTraci) {
        overtaken = getColumnleader(vehicle, leader);
        if (overtaken.first == 0) {
            return false;
        }
#ifdef DEBUG_CHANGE_OPPOSITE
        if (DEBUG_COND) {
            std::cout << "   compute time/space to overtake for columnLeader=" << overtaken.first->getID() << " egoGap=" << overtaken.second << "\n";
        }
#endif
        computeOvertakingTime(vehicle, overtaken.first, overtaken.second, timeToOvertake, spaceToOvertake);
        // check for upcoming stops
        if (vehicle->nextStopDist() < spaceToOvertake) {
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << "   cannot changeOpposite due to upcoming stop (dist=" << vehicle->nextStopDist() << " spaceToOvertake=" << spaceToOvertake << ")\n";
            }
#endif
            return false;
        }
        neighLead = opposite->getOppositeLeader(vehicle, timeToOvertake * opposite->getSpeedLimit() * 2 + spaceToOvertake, true);

#ifdef DEBUG_CHANGE_OPPOSITE
        if (DEBUG_COND) {
            std::cout << SIMTIME
                      << " veh=" << vehicle->getID()
                      << " changeOpposite opposite=" << opposite->getID()
                      << " lead=" << Named::getIDSecure(leader.first)
                      << " timeToOvertake=" << timeToOvertake
                      << " spaceToOvertake=" << spaceToOvertake
                      << "\n";
        }
#endif
        // check for dangerous oncoming leader
        if (neighLead.first != 0) {
            const MSVehicle* oncoming = neighLead.first;
            // conservative: assume that the oncoming vehicle accelerates to its maximum speed
            const double oncomingSpeed = oncoming->isStopped() ? 0 : oncoming->getLane()->getVehicleMaxSpeed(oncoming);
            const double safetyGap = ((oncomingSpeed + vehicle->getLane()->getVehicleMaxSpeed(vehicle))
                                      * vehicle->getCarFollowModel().getHeadwayTime()
                                      * OPPOSITE_OVERTAKING_SAFETYGAP_HEADWAY_FACTOR);
            const double surplusGap = neighLead.second - spaceToOvertake - timeToOvertake * oncomingSpeed - safetyGap;
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << SIMTIME
                          << " oncoming=" << oncoming->getID()
                          << " oncomingGap=" << neighLead.second
                          << " leaderGap=" << leader.second
                          << " safetyGap=" << safetyGap
                          << " surplusGap=" << surplusGap
                          << "\n";
            }
#endif
            if (surplusGap < 0) {

#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    std::cout << "   cannot changeOpposite due to dangerous oncoming (surplusGap=" << surplusGap << ")\n";
                }
#endif

#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    if (oncoming->getLaneChangeModel().isOpposite()) {
                        std::cout << SIMTIME << " ego=" << vehicle->getID() << " does not changeOpposite due to dangerous oncoming " << oncoming->getID() << "  (but the leader is also opposite)\n";
                    }
                }
#endif
                return false;
            }
        }

    } else if (!oppositeChangeByTraci) {
        timeToOvertake = -1;
        // look forward as far as possible
        spaceToOvertake = std::numeric_limits<double>::max();
        double dist = OPPOSITE_OVERTAKING_ONCOMING_LOOKAHEAD;
        leader = source->getOppositeLeader(vehicle, dist, true);
        double gap = leader.second;
        while (leader.first != nullptr && leader.first->getLaneChangeModel().isOpposite() && dist > 0) {
            // look beyond leaders that are also driving in the opposite direction until finding an oncoming leader or exhausting the look-ahead distance
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << SIMTIME << " ego=" << vehicle->getID() << " opposite leader=" << leader.first->getID() << " gap=" << gap << " is driving against the flow\n";
            }
#endif
            const double gapToLeaderFront = leader.second + leader.first->getVehicleType().getLengthWithGap();
            if (gapToLeaderFront < 0) {
                return false;
            }
            dist -= gapToLeaderFront;
            leader = source->getOppositeLeader(leader.first, dist, true);
            if (leader.first != 0) {
                gap += gapToLeaderFront;
            }
        }
        leader.second = gap;
        // -1 will use getMaximumBrakeDist() as look-ahead distance
        neighLead = opposite->getOppositeLeader(vehicle, -1, false);
    } else {
        timeToOvertake = STEPS2TIME(vehicle->getInfluencer().getLaneTimeLineDuration());//todo discuss concept
        spaceToOvertake =  timeToOvertake * vehicle->getLane()->getVehicleMaxSpeed(vehicle);
    }
    // compute remaining space on the opposite side
    // 1. the part that remains on the current lane
    double usableDist = isOpposite ? vehicle->getPositionOnLane() : source->getLength() - vehicle->getPositionOnLane();

    if (usableDist < spaceToOvertake) {
        // look forward along the next lanes
        const std::vector<MSLane*>& bestLaneConts = vehicle->getBestLanesContinuation();
        assert(bestLaneConts.size() >= 1);
        std::vector<MSLane*>::const_iterator it = bestLaneConts.begin() + 1;
        while (usableDist < spaceToOvertake && it != bestLaneConts.end()) {
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << "      usableDist=" << usableDist << " opposite=" << Named::getIDSecure((*it)->getOpposite()) << "\n";
            }
#endif
            if ((*it)->getOpposite() == nullptr || !(*it)->getOpposite()->allowsVehicleClass(vehicle->getVClass())) {
                // opposite lane ends
                break;
            }
            // do not overtake past a minor link or turn
            if (*(it - 1) != nullptr) {
                MSLink* link = MSLinkContHelper::getConnectingLink(**(it - 1), **it);
                if (link == nullptr || link->getState() == LINKSTATE_ZIPPER
                        || (link->getDirection() != LinkDirection::STRAIGHT && vehicle->getVehicleType().getVehicleClass() != SVC_EMERGENCY)
                        || (!link->havePriority()
                            // consider traci-influence
                            && (!vehicle->hasInfluencer() || vehicle->getInfluencer().getRespectJunctionPriority())
                            // consider junction model parameters
                            && ((!link->haveRed() && !link->haveYellow()) || !vehicle->ignoreRed(link, true)))) {
#ifdef DEBUG_CHANGE_OPPOSITE
                    if (DEBUG_COND) {
                        std::cout << "   stop lookahead at link=" << (link == 0 ? "NULL" : link->getViaLaneOrLane()->getID()) << " state=" << (link == 0 ? "?" : toString(link->getState())) << " ignoreRed=" << vehicle->ignoreRed(link, true) << "\n";
                    }
#endif
                    break;
                }
            }
            usableDist += (*it)->getLength();
            ++it;
        }
    }
    if (!isOpposite && usableDist < spaceToOvertake) {
#ifdef DEBUG_CHANGE_OPPOSITE
        if (DEBUG_COND) {
            std::cout << "   cannot changeOpposite due to insufficient space (seen=" << usableDist << " spaceToOvertake=" << spaceToOvertake << ")\n";
        }
#endif
        return false;
    }
#ifdef DEBUG_CHANGE_OPPOSITE
    if (DEBUG_COND) {
        std::cout << "   usableDist=" << usableDist << " spaceToOvertake=" << spaceToOvertake << " timeToOvertake=" << timeToOvertake << "\n";
    }
#endif
    // compute wish to change
    // Does "preb" mean "previousBestLanes" ??? If so *rename*
    std::vector<MSVehicle::LaneQ> preb = vehicle->getBestLanes();
    if (isOpposite) {
        // compute the remaining distance that can be drive on the opposite side
        // this value will put into LaneQ.length of the leftmost lane
        // @note: length counts from the start of the current lane
        // @note: see MSLCM_LC2013::_wantsChange @1092 (isOpposite()
        MSVehicle::LaneQ& laneQ = preb[preb.size() - 1];
        // position on the target lane
        const double forwardPos = source->getOppositePos(vehicle->getPositionOnLane());

        // consider usableDist (due to minor links or end of opposite lanes)
        laneQ.length = MIN2(laneQ.length, usableDist + forwardPos);
        // consider upcoming stops
        laneQ.length = MIN2(laneQ.length, vehicle->nextStopDist() + forwardPos);
        // consider oncoming leaders
        if (leader.first != 0) {
            if (!leader.first->getLaneChangeModel().isOpposite()) {
                MSVehicle* oncoming = leader.first;
                const double oncomingSpeed = oncoming->getAcceleration() > 0 ? oncoming->getLane()->getVehicleMaxSpeed(oncoming) : oncoming->getSpeed();
                const double safetyGap = ((oncomingSpeed + vehicle->getLane()->getVehicleMaxSpeed(vehicle))
                                          * vehicle->getCarFollowModel().getHeadwayTime()
                                          * OPPOSITE_OVERTAKING_SAFETYGAP_HEADWAY_FACTOR);
                laneQ.length = MIN2(laneQ.length, leader.second / 2 + forwardPos - safetyGap);
#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " found oncoming leader=" << oncoming->getID() << " gap=" << leader.second << "\n";
                }
#endif
            } else {
#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " opposite leader=" << leader.first->getID() << " gap=" << leader.second << " is driving against the flow\n";
                }
#endif
            }
            if (neighLead.first != 0) {
                overtaken = getColumnleader(vehicle, neighLead);
                if (overtaken.first == 0) {
#ifdef DEBUG_CHANGE_OPPOSITE
                    if (DEBUG_COND) {
                        std::cout << SIMTIME << " ego=" << vehicle->getID() << " did not find columnleader to overtake\n";
                    }
#endif
                } else {
                    const double remainingDist = laneQ.length - forwardPos;
                    computeOvertakingTime(vehicle, overtaken.first, overtaken.second, timeToOvertake, spaceToOvertake);
#ifdef DEBUG_CHANGE_OPPOSITE
                    if (DEBUG_COND) {
                        std::cout << SIMTIME << " ego=" << vehicle->getID() << " is overtaking " << overtaken.first->getID()
                                  << " remainingDist=" << remainingDist <<  " spaceToOvertake=" << spaceToOvertake << " timeToOvertake=" << timeToOvertake << "\n";
                    }
#endif
                    if (remainingDist > spaceToOvertake) {
                        // exaggerate remaining dist so that the vehicle continues
                        // overtaking (otherwise the lane change model might abort prematurely)
                        laneQ.length += 1000;
                    }
                }
            }
            leader.first = 0; // ignore leader after this
        }
#ifdef DEBUG_CHANGE_OPPOSITE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh=" << vehicle->getID() << " remaining dist=" << laneQ.length - forwardPos << " forwardPos=" << forwardPos << " laneQ.length=" << laneQ.length << "\n";
        }
#endif
    }
    std::pair<MSVehicle* const, double> neighFollow = opposite->getOppositeFollower(vehicle);
    int state = checkChange(direction, opposite, leader, neighLead, neighFollow, preb);
    vehicle->getLaneChangeModel().setOwnState(state);

    bool changingAllowed = (state & LCA_BLOCKED) == 0;
    // change if the vehicle wants to and is allowed to change
    if ((state & LCA_WANTS_LANECHANGE) != 0 && changingAllowed
            // do not change to the opposite direction for cooperative reasons
            && (isOpposite || (state & LCA_COOPERATIVE) == 0)) {
        const bool continuous = vehicle->getLaneChangeModel().startLaneChangeManeuver(source, opposite, direction);
#ifdef DEBUG_CHANGE_OPPOSITE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " changing to opposite veh=" << vehicle->getID() << " dir=" << direction << " opposite=" << Named::getIDSecure(opposite) << " state=" << state << "\n";
        }
#endif
        if (continuous) {
            continueChange(vehicle, myCandi);
        }
        return true;
    }
#ifdef DEBUG_CHANGE_OPPOSITE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " not changing to opposite veh=" << vehicle->getID() << " dir=" << direction
                  << " opposite=" << Named::getIDSecure(opposite) << " state=" << toString((LaneChangeAction)state) << "\n";
    }
#endif
    return false;
}


void
MSLaneChanger::computeOvertakingTime(const MSVehicle* vehicle, const MSVehicle* leader, double gap, double& timeToOvertake, double& spaceToOvertake) {
    // Assumptions:
    // - leader maintains the current speed
    // - vehicle merges with maxSpeed ahead of leader
    // XXX affected by ticket #860 (the formula is invalid for the current position update rule)

    // first compute these values for the case where vehicle is accelerating
    // without upper bound on speed
    const double vMax = vehicle->getLane()->getVehicleMaxSpeed(vehicle);
    const double v = vehicle->getSpeed();
    const double u = leader->getAcceleration() > 0 ? leader->getLane()->getVehicleMaxSpeed(leader) : leader->getSpeed();
    const double a = vehicle->getCarFollowModel().getMaxAccel();
    const double d = vehicle->getCarFollowModel().getMaxDecel();
    const double g = MAX2(0.0, (
                              // drive up to the rear of leader
                              gap + vehicle->getVehicleType().getMinGap()
                              // drive head-to-head with the leader
                              + leader->getVehicleType().getLengthWithGap()
                              // drive past the leader
                              + vehicle->getVehicleType().getLength()
                              // allow for safe gap between leader and vehicle
                              + leader->getCarFollowModel().getSecureGap(leader, vehicle, u, vMax, d)));
    const double sign = -1; // XXX recheck
    // v*t + t*t*a*0.5 = g + u*t
    // solve t
    // t = ((u - v - (((((2.0*(u - v))**2.0) + (8.0*a*g))**(1.0/2.0))*sign/2.0))/a)
    double t = (u - v - sqrt(4 * (u - v) * (u - v) + 8 * a * g) * sign * 0.5) / a;
#ifdef DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
    if (DEBUG_COND) {
        std::cout << " computeOvertakingTime v=" << v << " vMax=" << vMax << " u=" << u << " a=" << a << " d=" << d << " gap=" << gap << " g=" << g << " t=" << t
                  << " distEgo=" << v* t + t* t* a * 0.5 << " distLead=" << g + u* t
                  << "\n";
    }
#endif
    assert(t >= 0);

    // allow for a safety time gap
    t += OPPOSITE_OVERTAKING_SAFE_TIMEGAP;
    // round to multiples of step length (TS)
    t = ceil(t / TS) * TS;

    /// XXX ignore speed limit when overtaking through the opposite lane?
    const double timeToMaxSpeed = (vMax - v) / a;

#ifdef DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
    if (DEBUG_COND) {
        std::cout << "   t=" << t << "  tvMax=" << timeToMaxSpeed << "\n";
    }
#endif
    if (t <= timeToMaxSpeed) {
        timeToOvertake = t;
        spaceToOvertake = v * t + t * t * a * 0.5;
#ifdef DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
        if (DEBUG_COND) {
            std::cout << "    sto=" << spaceToOvertake << "\n";
        }
#endif
    } else {
        // space until max speed is reached
        const double s = v * timeToMaxSpeed + timeToMaxSpeed * timeToMaxSpeed * a * 0.5;
        const double m = timeToMaxSpeed;
        // s + (t-m) * vMax = g + u*t
        // solve t
        t = (g - s + m * vMax) / (vMax - u);
        if (t < 0) {
            // cannot overtake in time
#ifdef DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
            if (DEBUG_COND) {
                std::cout << "     t2=" << t << "\n";
            }
#endif
            timeToOvertake = std::numeric_limits<double>::max();
            spaceToOvertake = std::numeric_limits<double>::max();
        } else {
            // allow for a safety time gap
            t += OPPOSITE_OVERTAKING_SAFE_TIMEGAP;
            // round to multiples of step length (TS)
            t = ceil(t / TS) * TS;

            timeToOvertake = t;
            spaceToOvertake = s + (t - m) * vMax;
#ifdef DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
            if (DEBUG_COND) {
                std::cout << "     t2=" << t << " s=" << s << " sto=" << spaceToOvertake << " m=" << m << "\n";
            }
#endif
        }
    }
    const double safetyFactor = OPPOSITE_OVERTAKING_SAFETY_FACTOR * vehicle->getLaneChangeModel().getOppositeSafetyFactor();
    timeToOvertake *= safetyFactor;
    spaceToOvertake *= safetyFactor;
#ifdef DEBUG_CHANGE_OPPOSITE_OVERTAKINGTIME
    if (DEBUG_COND) {
        if (safetyFactor != 1) {
            std::cout << "    applying safetyFactor=" << safetyFactor
                      << " tto=" << timeToOvertake << " sto=" << spaceToOvertake << "\n";
        }
    }
#endif

}



std::pair<MSVehicle*, double>
MSLaneChanger::getColumnleader(MSVehicle* vehicle, std::pair<MSVehicle*, double> leader, double maxLookAhead) {
    assert(leader.first != 0);
    MSLane* source = vehicle->getLane();
    // find a leader vehicle with sufficient space ahead for merging back
    const double overtakingSpeed = source->getVehicleMaxSpeed(vehicle); // just a guess
    const double mergeBrakeGap = vehicle->getCarFollowModel().brakeGap(overtakingSpeed);
    std::pair<MSVehicle*, double> columnLeader = leader;
    double egoGap = leader.second;
    bool foundSpaceAhead = false;
    double seen = leader.second + leader.first->getVehicleType().getLengthWithGap();
    std::vector<MSLane*> conts = vehicle->getBestLanesContinuation();
    if (maxLookAhead == std::numeric_limits<double>::max()) {
        maxLookAhead = (vehicle->getVehicleType().getVehicleClass() == SVC_EMERGENCY
                        ? OPPOSITE_OVERTAKING_MAX_LOOKAHEAD_EMERGENCY
                        : OPPOSITE_OVERTAKING_MAX_LOOKAHEAD);
    }
#ifdef DEBUG_CHANGE_OPPOSITE
    if (DEBUG_COND) {
        std::cout << " getColumnleader vehicle=" << vehicle->getID() << " leader=" << leader.first->getID() << " gap=" << leader.second << " maxLookAhead=" << maxLookAhead << "\n";
    }
#endif
    const double safetyFactor = OPPOSITE_OVERTAKING_SAFETY_FACTOR * vehicle->getLaneChangeModel().getOppositeSafetyFactor();
    while (!foundSpaceAhead) {
        const double requiredSpaceAfterLeader = (columnLeader.first->getCarFollowModel().getSecureGap(
                columnLeader.first, vehicle,
                columnLeader.first->getSpeed(), overtakingSpeed, vehicle->getCarFollowModel().getMaxDecel())
                                                + columnLeader.first->getVehicleType().getMinGap()
                                                + vehicle->getVehicleType().getLengthWithGap());


        // all leader vehicles on the current laneChanger edge are already moved into MSLane::myTmpVehicles
        const bool checkTmpVehicles = (&columnLeader.first->getLane()->getEdge() == &source->getEdge());
        std::pair<MSVehicle* const, double> leadLead = columnLeader.first->getLane()->getLeader(
                    columnLeader.first, columnLeader.first->getPositionOnLane(), conts, requiredSpaceAfterLeader + mergeBrakeGap,
                    checkTmpVehicles);

#ifdef DEBUG_CHANGE_OPPOSITE
        if (DEBUG_COND) {
            std::cout << "   leadLead=" << Named::getIDSecure(leadLead.first) << " gap=" << leadLead.second << "\n";
        }
#endif
        if (leadLead.first == nullptr) {
            double availableSpace = columnLeader.first->getLane()->getLength() - columnLeader.first->getPositionOnLane();
            const double requiredSpace = safetyFactor * (requiredSpaceAfterLeader
                                         + vehicle->getCarFollowModel().brakeGap(overtakingSpeed));
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << "   no direct leader found after columnLeader " << columnLeader.first->getID()
                          << " availableSpace=" << availableSpace
                          << " req1=" << requiredSpaceAfterLeader
                          << " req2=" << requiredSpace / safetyFactor
                          << " req3=" << requiredSpace
                          << "\n";
            }
#endif
            if (availableSpace > requiredSpace) {
                foundSpaceAhead = true;
            } else {
                // maybe the columnleader is stopped before a junction or takes a different turn.
                // try to find another columnleader on successive lanes
                MSLane* next = getLaneAfter(columnLeader.first->getLane(), conts);
#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    std::cout << "   look for another leader on lane " << Named::getIDSecure(next) << "\n";
                }
#endif
                while (next != nullptr && seen < maxLookAhead) {
                    seen += next->getLength();
                    MSVehicle* cand = next->getLastAnyVehicle();
                    if (cand == nullptr) {
                        availableSpace += next->getLength();
                        if (availableSpace > requiredSpace) {
                            foundSpaceAhead = true;
                            break;
                        }
                    } else {
                        availableSpace += cand->getBackPositionOnLane();
                        if (availableSpace > requiredSpace) {
                            foundSpaceAhead = true;
                            break;
                        } else {
                            return getColumnleader(vehicle, std::make_pair(cand, availableSpace + cand->getPositionOnLane()), maxLookAhead - seen);
                        }
                    }
                }
                if (!foundSpaceAhead) {
                    return std::make_pair(nullptr, -1);
                }
            }
        } else {
            const double requiredSpace = safetyFactor * (requiredSpaceAfterLeader
                                         + vehicle->getCarFollowModel().getSecureGap(vehicle, leadLead.first,
                                                 overtakingSpeed, leadLead.first->getSpeed(), leadLead.first->getCarFollowModel().getMaxDecel()));
#ifdef DEBUG_CHANGE_OPPOSITE
            if (DEBUG_COND) {
                std::cout << "   leader's leader " << leadLead.first->getID() << " space=" << leadLead.second
                          << " req1=" << requiredSpaceAfterLeader
                          << " req2=" << requiredSpace / safetyFactor
                          << " req3=" << requiredSpace
                          << "\n";
            }
#endif
            if (leadLead.second > requiredSpace) {
                foundSpaceAhead = true;
            } else {
#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    std::cout << "   not enough space after columnLeader=" << columnLeader.first->getID() << " required=" << requiredSpace << "\n";
                }
#endif
                seen += MAX2(0., leadLead.second) + leadLead.first->getVehicleType().getLengthWithGap();
                if (seen > maxLookAhead) {
#ifdef DEBUG_CHANGE_OPPOSITE
                    if (DEBUG_COND) {
                        std::cout << "   cannot changeOpposite due to insufficient free space after columnLeader (seen=" << seen << " columnLeader=" << columnLeader.first->getID() << ")\n";
                    }
#endif
                    return std::make_pair(nullptr, -1);
                }
                // see if merging after leadLead is possible
                egoGap += columnLeader.first->getVehicleType().getLengthWithGap() + leadLead.second;
                columnLeader = leadLead;
#ifdef DEBUG_CHANGE_OPPOSITE
                if (DEBUG_COND) {
                    std::cout << "   new columnLeader=" << columnLeader.first->getID() << "\n";
                }
#endif
            }
        }
    }
    columnLeader.second = egoGap;
    return columnLeader;
}


MSLane*
MSLaneChanger::getLaneAfter(MSLane* lane, const std::vector<MSLane*>& conts) {
    for (auto it = conts.begin(); it != conts.end(); ++it) {
        if (*it == lane) {
            if (it + 1 != conts.end()) {
                return *(it + 1);
            } else {
                return nullptr;
            }
        }
    }
    return nullptr;
}


/****************************************************************************/
