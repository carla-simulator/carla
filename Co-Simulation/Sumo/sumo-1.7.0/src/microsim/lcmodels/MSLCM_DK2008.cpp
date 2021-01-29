/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSLCM_DK2008.cpp
/// @author  Daniel Krajzewicz
/// @author  Friedemann Wesner
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Tue, 29.05.2005
///
// A lane change model developed by D. Krajzewicz between 2004 and 2010
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/RandHelper.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include "MSLCM_DK2008.h"

// ===========================================================================
// variable definitions
// ===========================================================================
// 80km/h will be the threshold for dividing between long/short foresight
#define LOOK_FORWARD_SPEED_DIVIDER 14.

#define LOOK_FORWARD_FAR  15.
#define LOOK_FORWARD_NEAR 5.



#define JAM_FACTOR 2.
#define JAM_FACTOR2 1.


// ===========================================================================
// member method definitions
// ===========================================================================
MSLCM_DK2008::MSLCM_DK2008(MSVehicle& v)
    : MSAbstractLaneChangeModel(v, LCM_DK2008),
      myChangeProbability(0),
      myLeadingBlockerLength(0), myLeftSpace(0) {}

MSLCM_DK2008::~MSLCM_DK2008() {
    changed();
}

int
MSLCM_DK2008::wantsChange(
    int laneOffset,
    MSAbstractLaneChangeModel::MSLCMessager& msgPass, int blocked,
    const std::pair<MSVehicle*, double>& leader,
    const std::pair<MSVehicle*, double>& neighLead,
    const std::pair<MSVehicle*, double>& neighFollow,
    const MSLane& neighLane,
    const std::vector<MSVehicle::LaneQ>& preb,
    MSVehicle** lastBlocked,
    MSVehicle** firstBlocked) {
    UNUSED_PARAMETER(firstBlocked);
    return (laneOffset == -1 ?
            wantsChangeToRight(msgPass, blocked, leader, neighLead, neighFollow, neighLane, preb, lastBlocked, firstBlocked)
            : wantsChangeToLeft(msgPass, blocked, leader, neighLead, neighFollow, neighLane, preb, lastBlocked, firstBlocked));

}


int
MSLCM_DK2008::wantsChangeToRight(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                                 int blocked,
                                 const std::pair<MSVehicle*, double>& leader,
                                 const std::pair<MSVehicle*, double>& neighLead,
                                 const std::pair<MSVehicle*, double>& neighFollow,
                                 const MSLane& neighLane,
                                 const std::vector<MSVehicle::LaneQ>& preb,
                                 MSVehicle** lastBlocked,
                                 MSVehicle** firstBlocked) {
    UNUSED_PARAMETER(firstBlocked);
    MSVehicle::LaneQ curr, best;
    int bestLaneOffset = 0;
    double currentDist = 0;
    double neighDist = 0;
    double neighExtDist = 0;
    double currExtDist = 0;
    int currIdx = 0;
    MSLane* prebLane = myVehicle.getLane();
    if (prebLane->getEdge().isInternal()) {
        // internal edges are not kept inside the bestLanes structure
        prebLane = prebLane->getLinkCont()[0]->getLane();
    }
    for (int p = 0; p < (int) preb.size(); ++p) {
        if (preb[p].lane == prebLane && p > 0) {
            curr = preb[p];
            bestLaneOffset = curr.bestLaneOffset;
            currentDist = curr.length;
            currExtDist = curr.lane->getLength();
            neighDist = preb[p - 1].length;
            neighExtDist = preb[p - 1].lane->getLength();
            best = preb[p + bestLaneOffset];
            currIdx = p;
            break;
        }
    }

    // keep information about being a leader/follower
    int ret = (myOwnState & 0xffff0000);

    if (leader.first != 0
            &&
            (myOwnState & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0
            &&
            (leader.first->getLaneChangeModel().getOwnState()&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0) {

        myOwnState &= (0xffffffff - LCA_AMBLOCKINGFOLLOWER_DONTBRAKE);
        if (myVehicle.getSpeed() > SUMO_const_haltingSpeed) {
            myOwnState |= LCA_AMBACKBLOCKER;
        } else {
            ret |= LCA_AMBACKBLOCKER;
            myDontBrake = true;
        }
    }

    // process information about the last blocked vehicle
    //  if this vehicle is blocking someone in front, we maybe decelerate to let him in
    if ((*lastBlocked) != nullptr) {
        double gap = (*lastBlocked)->getPositionOnLane() - (*lastBlocked)->getVehicleType().getLength() - myVehicle.getPositionOnLane() - myVehicle.getVehicleType().getMinGap();
        if (gap > 0.1) {
            if (myVehicle.getSpeed() < ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())) {
                if ((*lastBlocked)->getSpeed() < SUMO_const_haltingSpeed) {
                    ret |= LCA_AMBACKBLOCKER_STANDING;
                } else {
                    ret |= LCA_AMBACKBLOCKER;
                }
                myVSafes.push_back(myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), (double)(gap - 0.1), (*lastBlocked)->getSpeed(), (*lastBlocked)->getCarFollowModel().getMaxDecel()));
                (*lastBlocked) = nullptr;
            }
            return ret;
        }
    }

    // we try to estimate the distance which is necessary to get on a lane
    //  we have to get on in order to keep our route
    // we assume we need something that depends on our velocity
    // and compare this with the free space on our wished lane
    //
    // if the free space is somehow less than the space we need, we should
    //  definitely try to get to the desired lane
    //
    // this rule forces our vehicle to change the lane if a lane changing is necessary soon
    double rv = myVehicle.getSpeed() > LOOK_FORWARD_SPEED_DIVIDER
                ? myVehicle.getSpeed() * (double) LOOK_FORWARD_FAR
                : myVehicle.getSpeed() * (double) LOOK_FORWARD_NEAR;
    rv += myVehicle.getVehicleType().getLengthWithGap() * (double) 2.;

    double tdist = currentDist - myVehicle.getPositionOnLane() - best.occupation * (double) JAM_FACTOR2;

    // assert(best.length > curr.length);
    // XXX if (curr.length != best.length) && ...
    if (fabs(best.length - curr.length) > MIN2((double) .1, best.lane->getLength()) && bestLaneOffset < 0 && currentDistDisallows(tdist/*currentDist*/, bestLaneOffset, rv)) {
        informBlocker(msgPass, blocked, LCA_MRIGHT, neighLead, neighFollow);
        if (neighLead.second > 0 && neighLead.second > leader.second) {
            myVSafes.push_back(myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighLead.second, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()) - (double) 0.5);
        }

        // letting vehicles merge in at the end of the lane in case of counter-lane change, step#1, right
        //   if there is a leader and he wants to change to left (we want to change to right)
        if (neighLead.first != 0 && (neighLead.first->getLaneChangeModel().getOwnState()&LCA_LEFT) != 0) {
            // save at least his length in myLeadingBlockerLength
            myLeadingBlockerLength = MAX2(neighLead.first->getVehicleType().getLengthWithGap(), myLeadingBlockerLength);
            // save the left space
            myLeftSpace = currentDist - myVehicle.getPositionOnLane();
        }
        //

        return ret | LCA_RIGHT | LCA_STRATEGIC | LCA_URGENT;
    }


    // the opposite lane-changing direction should be done than the one examined herein
    //  we'll check whether we assume we could change anyhow and get back in time...
    //
    // this rule prevents the vehicle from moving in opposite direction of the best lane
    //  unless the way till the end where the vehicle has to be on the best lane
    //  is long enough
    double maxJam = MAX2(preb[currIdx - 1].occupation, preb[currIdx].occupation);
    double neighLeftPlace = MAX2((double) 0, neighDist - myVehicle.getPositionOnLane() - maxJam);
    if (bestLaneOffset >= 0 && (currentDistDisallows(neighLeftPlace, bestLaneOffset + 2, rv))) {
        // ...we will not change the lane if not
        return ret | LCA_STAY | LCA_STRATEGIC;
    }


    // if the current lane is the best and a lane-changing would cause a situation
    //  of which we assume we will not be able to return to the lane we have to be on...
    //
    // this rule prevents the vehicle from leaving the current, best lane when it is
    //  close to this lane's end
    if (currExtDist > neighExtDist && (neighLeftPlace * 2. < rv/*||currE[currIdx+1].length<currentDist*/)) {
        return ret | LCA_STAY | LCA_STRATEGIC;
    }

    // let's also regard the case where the vehicle is driving on a highway...
    //  in this case, we do not want to get to the dead-end of an on-ramp
    //
    // THIS RULE APPLIES ONLY TO CHANGING TO THE RIGHT LANE
    if (bestLaneOffset == 0 && preb[currIdx - 1].bestLaneOffset != 0 && myVehicle.getLane()->getSpeedLimit() > 80. / 3.6) {
        return ret | LCA_STAY | LCA_STRATEGIC;
    }
    // --------

    // -------- make place on current lane if blocking follower
    if (amBlockingFollowerPlusNB()
            &&
            (currentDistAllows(neighDist, bestLaneOffset, rv) || neighDist >= currentDist)) {

        return ret | LCA_RIGHT | LCA_COOPERATIVE | LCA_URGENT;
    }
    // --------


    // -------- security checks for krauss
    //  (vsafe fails when gap<0)
    if ((blocked & LCA_BLOCKED) != 0) {
        return ret;
    }
    // --------

    // -------- higher speed
    if ((congested(neighLead.first) && neighLead.second < 20) || predInteraction(leader)) { //!!!
        return ret;
    }
    double thisLaneVSafe = myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle);
    double neighLaneVSafe = neighLane.getVehicleMaxSpeed(&myVehicle);
    if (neighLead.first == 0) {
        neighLaneVSafe = MIN2(neighLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighDist, 0, 0));
    } else {
        // @todo: what if leader is below safe gap?!!!
        neighLaneVSafe = MIN2(neighLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighLead.second, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()));
    }
    if (leader.first == 0) {
        thisLaneVSafe = MIN2(thisLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), currentDist, 0, 0));
    } else {
        // @todo: what if leader is below safe gap?!!!
        thisLaneVSafe = MIN2(thisLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), leader.second, leader.first->getSpeed(), leader.first->getCarFollowModel().getMaxDecel()));
    }

    thisLaneVSafe = MIN2(thisLaneVSafe, myVehicle.getVehicleType().getMaxSpeed());
    neighLaneVSafe = MIN2(neighLaneVSafe, myVehicle.getVehicleType().getMaxSpeed());
    if (thisLaneVSafe - neighLaneVSafe > 5. / 3.6) {
        // ok, the current lane is faster than the right one...
        if (myChangeProbability < 0) {
            myChangeProbability *= pow(0.5, TS);
        }
    } else {
        // ok, the right lane is faster than the current
        myChangeProbability -= TS * ((neighLaneVSafe - thisLaneVSafe) / (myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle)));
    }

    // let's recheck the "Rechtsfahrgebot"
    double vmax = MIN2(myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle), myVehicle.getVehicleType().getMaxSpeed());
    vmax -= (double)(5. / 2.6);
    if (neighLaneVSafe >= vmax) {
        myChangeProbability -= TS * ((neighLaneVSafe - vmax) / (vmax));
    }

    if (myChangeProbability < -2 && neighDist / MAX2((double) .1, myVehicle.getSpeed()) > 20.) { //./MAX2((double) .1, myVehicle.getSpeed())) { // -.1
        return ret | LCA_RIGHT | LCA_SPEEDGAIN;
    }
    // --------

    return ret;
}


int
MSLCM_DK2008::wantsChangeToLeft(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                                int blocked,
                                const std::pair<MSVehicle*, double>& leader,
                                const std::pair<MSVehicle*, double>& neighLead,
                                const std::pair<MSVehicle*, double>& neighFollow,
                                const MSLane& neighLane,
                                const std::vector<MSVehicle::LaneQ>& preb,
                                MSVehicle** lastBlocked,
                                MSVehicle** firstBlocked) {
    UNUSED_PARAMETER(firstBlocked);
    MSVehicle::LaneQ curr, best;
    int bestLaneOffset = 0;
    double currentDist = 0;
    double neighDist = 0;
    double neighExtDist = 0;
    double currExtDist = 0;
    int currIdx = 0;
    MSLane* prebLane = myVehicle.getLane();
    if (prebLane->getEdge().isInternal()) {
        // internal edges are not kept inside the bestLanes structure
        prebLane = prebLane->getLinkCont()[0]->getLane();
    }
    for (int p = 0; p < (int) preb.size(); ++p) {
        if (preb[p].lane == prebLane) {
            curr = preb[p];
            bestLaneOffset = curr.bestLaneOffset;
            currentDist = curr.length;
            currExtDist = curr.lane->getLength();
            neighDist = preb[p + 1].length;
            neighExtDist = preb[p + 1].lane->getLength();
            best = preb[p + bestLaneOffset];
            currIdx = p;
            break;
        }
    }
    // keep information about being a leader/follower
    int ret = (myOwnState & 0xffff0000);

    // ?!!!
    if (leader.first != 0
            &&
            (myOwnState & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0
            &&
            (leader.first->getLaneChangeModel().getOwnState()&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0) {

        myOwnState &= (0xffffffff - LCA_AMBLOCKINGFOLLOWER_DONTBRAKE);
        if (myVehicle.getSpeed() > SUMO_const_haltingSpeed) {
            myOwnState |= LCA_AMBACKBLOCKER;
        } else {
            ret |= LCA_AMBACKBLOCKER;
            myDontBrake = true;
        }
    }

    // process information about the last blocked vehicle
    //  if this vehicle is blocking someone in front, we maybe decelerate to let him in
    if ((*lastBlocked) != nullptr) {
        double gap = (*lastBlocked)->getPositionOnLane() - (*lastBlocked)->getVehicleType().getLength() - myVehicle.getPositionOnLane() - myVehicle.getVehicleType().getMinGap();
        if (gap > 0.1) {
            if (myVehicle.getSpeed() < ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())) {
                if ((*lastBlocked)->getSpeed() < SUMO_const_haltingSpeed) {
                    ret |= LCA_AMBACKBLOCKER_STANDING;
                } else {
                    ret |= LCA_AMBACKBLOCKER;
                }
                myVSafes.push_back(myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), (double)(gap - 0.1), (*lastBlocked)->getSpeed(), (*lastBlocked)->getCarFollowModel().getMaxDecel()));
                (*lastBlocked) = nullptr;
            }
            return ret;
        }
    }

    // we try to estimate the distance which is necessary to get on a lane
    //  we have to get on in order to keep our route
    // we assume we need something that depends on our velocity
    // and compare this with the free space on our wished lane
    //
    // if the free space is somehow less than the space we need, we should
    //  definitely try to get to the desired lane
    //
    // this rule forces our vehicle to change the lane if a lane changing is necessary soon
    double lv = myVehicle.getSpeed() > LOOK_FORWARD_SPEED_DIVIDER
                ? myVehicle.getSpeed() * (double) LOOK_FORWARD_FAR
                : myVehicle.getSpeed() * (double) LOOK_FORWARD_NEAR;
    lv += myVehicle.getVehicleType().getLengthWithGap() * (double) 2.;


    double tdist = currentDist - myVehicle.getPositionOnLane() - best.occupation * (double) JAM_FACTOR2;
    if (fabs(best.length - curr.length) > MIN2((double) .1, best.lane->getLength()) && bestLaneOffset > 0
            &&
            currentDistDisallows(tdist/*currentDist*/, bestLaneOffset, lv)) {
        informBlocker(msgPass, blocked, LCA_MLEFT, neighLead, neighFollow);
        if (neighLead.second > 0 && neighLead.second > leader.second) {
            myVSafes.push_back(myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighLead.second, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()) - (double) 0.5);
        }

        // letting vehicles merge in at the end of the lane in case of counter-lane change, step#1, left
        //   if there is a leader and he wants to change to right (we want to change to left)
        if (neighLead.first != 0 && (neighLead.first->getLaneChangeModel().getOwnState()&LCA_RIGHT) != 0) {
            // save at least his length in myLeadingBlockerLength
            myLeadingBlockerLength = MAX2(neighLead.first->getVehicleType().getLengthWithGap(), myLeadingBlockerLength);
            // save the left space
            myLeftSpace = currentDist - myVehicle.getPositionOnLane();
        }
        //

        return ret | LCA_LEFT | LCA_STRATEGIC | LCA_URGENT;
    }

    // the opposite lane-changing direction should be rather done, not
    //  the one examined herein
    //  we'll check whether we assume we could change anyhow and get back in time...
    //
    // this rule prevents the vehicle from moving in opposite direction of the best lane
    //  unless the way till the end where the vehicle has to be on the best lane
    //  is long enough
    double maxJam = MAX2(preb[currIdx + 1].occupation, preb[currIdx].occupation);
    double neighLeftPlace = MAX2((double) 0, neighDist - myVehicle.getPositionOnLane() - maxJam);
    if (bestLaneOffset <= 0 && (currentDistDisallows(neighLeftPlace, bestLaneOffset - 2, lv))) {
        // ...we will not change the lane if not
        return ret | LCA_STAY | LCA_STRATEGIC;
    }


    // if the current lane is the best and a lane-changing would cause a situation
    //  of which we assume we will not be able to return to the lane we have to be on...
    //
    // this rule prevents the vehicle from leaving the current, best lane when it is
    //  close to this lane's end
    if (currExtDist > neighExtDist && (neighLeftPlace * 2. < lv/*||currE[currIdx+1].length<currentDist*/)) {
        // ... let's not change the lane
        return ret | LCA_STAY | LCA_STRATEGIC;
    }

    /*
    // let's also regard the case where the vehicle is driving on a highway...
    //  in this case, we do not want to get to the dead-end of an on-ramp
    if(bestLaneOffset==0&&myVehicle.getLane().getMaxSpeed()>80./3.6) {
        return ret;
    }
    */


    /*
    // if the current lane is the
    if(bestLaneOffset==0&&(neighDist==0||curr.seenVehicles2*JAM_FACTOR>=neighExtDist-curr.length)) {
        return ret;
    }
    */
    // --------

    // -------- make place on current lane if blocking follower
    if (amBlockingFollowerPlusNB()
            &&
            (currentDistAllows(neighDist, bestLaneOffset, lv) || neighDist >= currentDist)) {

        return ret | LCA_LEFT | LCA_COOPERATIVE | LCA_URGENT;
    }
    // --------

    // -------- security checks for krauss
    //  (vsafe fails when gap<0)
    if ((blocked & LCA_BLOCKED) != 0) {
        return ret;
    }

    // -------- higher speed
    if ((congested(neighLead.first) && neighLead.second < 20) || predInteraction(leader)) { //!!!
        return ret;
    }
    double neighLaneVSafe = neighLane.getVehicleMaxSpeed(&myVehicle);
    double thisLaneVSafe = myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle);
    if (neighLead.first == 0) {
        neighLaneVSafe = MIN2(neighLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighDist, 0, 0)); // !!! warum nicht die Folgesgeschw.?
    } else {
        // @todo: what if leader is below safe gap?!!!
        neighLaneVSafe = MIN2(neighLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighLead.second, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()));
    }
    if (leader.first == 0) {
        thisLaneVSafe = MIN2(thisLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), currentDist, 0, 0));
    } else {
        // @todo: what if leader is below safe gap?!!!
        thisLaneVSafe = MIN2(thisLaneVSafe, myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), leader.second, leader.first->getSpeed(), leader.first->getCarFollowModel().getMaxDecel()));
    }
    thisLaneVSafe = MIN2(thisLaneVSafe, myVehicle.getVehicleType().getMaxSpeed());
    neighLaneVSafe = MIN2(neighLaneVSafe, myVehicle.getVehicleType().getMaxSpeed());
    if (thisLaneVSafe > neighLaneVSafe) {
        // this lane is better
        if (myChangeProbability > 0) {
            myChangeProbability *= pow(0.5, TS);
        }
    } else {
        // right lane is better
        myChangeProbability += TS * ((neighLaneVSafe - thisLaneVSafe) / (myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle))); // !!! Fahrzeuggeschw.!
    }
    if (myChangeProbability > .2 && neighDist / MAX2((double) .1, myVehicle.getSpeed()) > 20.) { // .1
        return ret | LCA_LEFT | LCA_SPEEDGAIN | LCA_URGENT;
    }
    // --------

    return ret;
}


double
MSLCM_DK2008::patchSpeed(const double min, const double wanted, const double max, const MSCFModel& cfModel) {
    int state = myOwnState;

    // letting vehicles merge in at the end of the lane in case of counter-lane change, step#2
    double MAGIC_offset = 1.;
    //   if we want to change and have a blocking leader and there is enough room for him in front of us
    if (myLeadingBlockerLength != 0) {
        double space = myLeftSpace - myLeadingBlockerLength - MAGIC_offset - myVehicle.getVehicleType().getMinGap();
        if (space > 0) {
            // compute speed for decelerating towards a place which allows the blocking leader to merge in in front
            double safe = cfModel.stopSpeed(&myVehicle, myVehicle.getSpeed(), space);
            // if we are approaching this place
            if (safe < wanted) {
                // return this speed as the speed to use
                return MAX2(min, safe);
            }
        }
    }

    // just to make sure to be notified about lane chaning end
    if (myVehicle.getLane()->getEdge().getLanes().size() == 1 || myVehicle.getLane()->getEdge().isInternal()) {
        // remove chaning information if on a road with a single lane
        changed();
        return wanted;
    }

    double nVSafe = wanted;
    bool gotOne = false;
    for (std::vector<double>::const_iterator i = myVSafes.begin(); i != myVSafes.end(); ++i) {
        double v = (*i);
        if (v >= min && v <= max) {
            nVSafe = MIN2(v, nVSafe);
            gotOne = true;
        }
    }

    // check whether the vehicle is blocked
    if ((state & LCA_WANTS_LANECHANGE) != 0) {
        if (gotOne && !myDontBrake) {
            return nVSafe;
        }
        // check whether the vehicle maybe has to be swapped with one of
        //  the blocking vehicles
        if ((state & LCA_BLOCKED) != 0) {
            if ((state & LCA_BLOCKED_BY_LEADER) != 0) {
                // if interacting with leader and not too slow
                return (min + wanted) / (double) 2.0;
            }
            if ((state & LCA_BLOCKED_BY_FOLLOWER) != 0) {
                return (max + wanted) / (double) 2.0;
            }
            return (min + wanted) / (double) 2.0;
        }
    }


    // decelerate if being a blocking follower
    //  (and does not have to change lanes)
    if ((state & LCA_AMBLOCKINGFOLLOWER) != 0) {
        if (fabs(max - myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle)) < 0.001 && min == 0) { // !!! was standing
            return 0;
        }
        return (min + wanted) / (double) 2.0;
    }
    if ((state & LCA_AMBACKBLOCKER) != 0) {
        if (max <= myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle) && min == 0) { // !!! was standing
            return min;
        }
    }
    if ((state & LCA_AMBACKBLOCKER_STANDING) != 0) {
        return min;
    }
    // accelerate if being a blocking leader or blocking follower not able to brake
    //  (and does not have to change lanes)
    if ((state & LCA_AMBLOCKINGLEADER) != 0) {
        return (max + wanted) / (double) 2.0;
    }
    if ((state & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0) {
        if (max <= myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle) && min == 0) { // !!! was standing
            return wanted;
        }
        return (min + wanted) / (double) 2.0;
    }
    return wanted;
}


void*
MSLCM_DK2008::inform(void* info, MSVehicle* /*sender*/) {
    Info* pinfo = (Info*) info;
    //myOwnState &= 0xffffffff; // reset all bits of MyLCAEnum but only those
    myOwnState |= pinfo->second;
    delete pinfo;
    return (void*) true;
}


void
MSLCM_DK2008::changed() {
    myOwnState = 0;
    myChangeProbability = 0;
    myLeadingBlockerLength = 0;
    myLeftSpace = 0;
    myVSafes.clear();
    myDontBrake = false;
}


void
MSLCM_DK2008::informBlocker(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                            int& blocked,
                            int dir,
                            const std::pair<MSVehicle*, double>& neighLead,
                            const std::pair<MSVehicle*, double>& neighFollow) {
    if ((blocked & LCA_BLOCKED_BY_FOLLOWER) != 0) {
        assert(neighFollow.first != 0);
        MSVehicle* nv = neighFollow.first;
        double decelGap =
            neighFollow.second
            + SPEED2DIST(myVehicle.getSpeed()) * (double) 2.0
            - MAX2(nv->getSpeed() - (double) ACCEL2DIST(nv->getCarFollowModel().getMaxDecel()) * (double) 2.0, (double) 0);
        if (neighFollow.second > 0 && decelGap > 0 && decelGap >= nv->getCarFollowModel().getSecureGap(nv, &myVehicle,
                nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel())) {
            double vsafe = myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighFollow.second, neighFollow.first->getSpeed(), neighFollow.first->getCarFollowModel().getMaxDecel());
            msgPass.informNeighFollower(new Info(vsafe, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
        } else {
            double vsafe = neighFollow.second <= 0 ? 0 : myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighFollow.second, neighFollow.first->getSpeed(), neighFollow.first->getCarFollowModel().getMaxDecel());
            msgPass.informNeighFollower(new Info(vsafe, dir | LCA_AMBLOCKINGFOLLOWER_DONTBRAKE), &myVehicle);
        }
    }
    if ((blocked & LCA_BLOCKED_BY_LEADER) != 0) {
        if (neighLead.first != 0 && neighLead.second > 0) {
            msgPass.informNeighLeader(new Info(0, dir | LCA_AMBLOCKINGLEADER), &myVehicle);
        }
    }
}


void
MSLCM_DK2008::prepareStep() {
    MSAbstractLaneChangeModel::prepareStep();
    myOwnState = 0;
    myLeadingBlockerLength = 0;
    myLeftSpace = 0;
    myVSafes.clear();
    myDontBrake = false;
    // truncate myChangeProbability to work around numerical instability between different builds
    myChangeProbability = ceil(myChangeProbability * 100000.0) * 0.00001;
}


/****************************************************************************/
