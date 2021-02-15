/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSLCM_SL2015.cpp
/// @author  Jakob Erdmann
/// @date    Tue, 06.10.2015
///
// A lane change model for heterogeneous traffic (based on sub-lanes)
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/RandHelper.h>
#include <utils/common/StringUtils.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSDriverState.h>
#include <microsim/MSGlobals.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/transportables/MSPModel.h>
#include "MSLCHelper.h"
#include "MSLCM_SL2015.h"

// ===========================================================================
// variable definitions
// ===========================================================================
#define MAGIC_OFFSET  1.
#define LOOK_FORWARD 10.

#define JAM_FACTOR 1.
//#define JAM_FACTOR 2. // VARIANT_8 (makes vehicles more focused but also more "selfish")

#define LCA_RIGHT_IMPATIENCE -1.
#define CUT_IN_LEFT_SPEED_THRESHOLD 27.
#define MAX_ONRAMP_LENGTH 200.

#define LOOK_AHEAD_MIN_SPEED 0.0
#define LOOK_AHEAD_SPEED_MEMORY 0.9

#define HELP_DECEL_FACTOR 1.0

#define HELP_OVERTAKE  (10.0 / 3.6)
#define MIN_FALLBEHIND  (7.0 / 3.6)

#define KEEP_RIGHT_HEADWAY 2.0

#define URGENCY 2.0

#define ROUNDABOUT_DIST_BONUS 100.0

#define KEEP_RIGHT_TIME 5.0 // the number of seconds after which a vehicle should move to the right lane
#define KEEP_RIGHT_ACCEPTANCE 7.0 // calibration factor for determining the desire to keep right

#define RELGAIN_NORMALIZATION_MIN_SPEED 10.0

#define TURN_LANE_DIST 200.0 // the distance at which a lane leading elsewhere is considered to be a turn-lane that must be avoided
#define GAIN_PERCEPTION_THRESHOLD 0.05 // the minimum relative speed gain which affects the behavior

#define SPEED_GAIN_MIN_SECONDS 20.0

#define ARRIVALPOS_LAT_THRESHOLD 100.0

// the speed at which the desired lateral gap grows now further
#define LATGAP_SPEED_THRESHOLD (50 / 3.6)
// the speed at which the desired lateral gap shrinks now further.
// @note: when setting LATGAP_SPEED_THRESHOLD = LATGAP_SPEED_THRESHOLD2, no speed-specif reduction of minGapLat is done
#define LATGAP_SPEED_THRESHOLD2 (50 / 3.6)

// intention to change decays over time
#define SPEEDGAIN_DECAY_FACTOR 0.5
// exponential averaging factor for expected sublane speeds
#define SPEEDGAIN_MEMORY_FACTOR 0.5



// ===========================================================================
// Debug flags
// ===========================================================================
//#define DEBUG_ACTIONSTEPS
//#define DEBUG_STATE
//#define DEBUG_SURROUNDING
//#define DEBUG_MANEUVER
//#define DEBUG_COMMITTED_SPEED
//#define DEBUG_PATCHSPEED
//#define DEBUG_INFORM
//#define DEBUG_ROUNDABOUTS
//#define DEBUG_WANTSCHANGE
//#define DEBUG_COOPERATE
//#define DEBUG_SLOWDOWN
//#define DEBUG_SAVE_BLOCKER_LENGTH
//#define DEBUG_BLOCKING
//#define DEBUG_TRACI
//#define DEBUG_STRATEGIC_CHANGE
//#define DEBUG_KEEP_LATGAP
//#define DEBUG_EXPECTED_SLSPEED
//#define DEBUG_COND (myVehicle.getID() == "moped.18" || myVehicle.getID() == "moped.16")
//#define DEBUG_COND (myVehicle.getID() == "Togliatti_71_0")
#define DEBUG_COND (myVehicle.isSelected())
//#define DEBUG_COND (myVehicle.getID() == "pkw150478" || myVehicle.getID() == "pkw150494" || myVehicle.getID() == "pkw150289")
//#define DEBUG_COND (myVehicle.getID() == "A" || myVehicle.getID() == "B") // fail change to left
//#define DEBUG_COND (myVehicle.getID() == "disabled") // test stops_overtaking
//#define DEBUG_COND true


// ===========================================================================
// member method definitions
// ===========================================================================
MSLCM_SL2015::MSLCM_SL2015(MSVehicle& v) :
    MSAbstractLaneChangeModel(v, LCM_SL2015),
    mySpeedGainProbabilityRight(0),
    mySpeedGainProbabilityLeft(0),
    myKeepRightProbability(0),
    myLeadingBlockerLength(0),
    myLeftSpace(0),
    myLookAheadSpeed(LOOK_AHEAD_MIN_SPEED),
    myLastEdge(nullptr),
    myCanChangeFully(true),
    mySafeLatDistRight(0),
    mySafeLatDistLeft(0),
    myStrategicParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_STRATEGIC_PARAM, 1)),
    myCooperativeParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_COOPERATIVE_PARAM, 1)),
    mySpeedGainParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SPEEDGAIN_PARAM, 1)),
    myKeepRightParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_KEEPRIGHT_PARAM, 1)),
    mySublaneParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SUBLANE_PARAM, 1)),
    // by default use SUMO_ATTR_LCA_PUSHY. If that is not set, try SUMO_ATTR_LCA_PUSHYGAP
    myPushy(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_PUSHY,
            1 - (v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_PUSHYGAP,
                    MAX2(NUMERICAL_EPS, v.getVehicleType().getMinGapLat())) /
                 MAX2(NUMERICAL_EPS, v.getVehicleType().getMinGapLat())))),
    myAssertive(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_ASSERTIVE, 1)),
    myImpatience(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_IMPATIENCE, 0)),
    myMinImpatience(myImpatience),
    myTimeToImpatience(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_TIME_TO_IMPATIENCE, std::numeric_limits<double>::max())),
    myAccelLat(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_ACCEL_LAT, 1.0)),
    myTurnAlignmentDist(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE, 0.0)),
    myLookaheadLeft(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_LOOKAHEADLEFT, 2.0)),
    mySpeedGainRight(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SPEEDGAINRIGHT, 0.1)),
    myLaneDiscipline(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_LANE_DISCIPLINE, 0.0)),
    mySpeedGainLookahead(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD, 5)),
    myRoundaboutBonus(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT, myCooperativeParam)),
    myCooperativeSpeed(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_COOPERATIVE_SPEED, myCooperativeParam)),
    mySigmaState(0) {
    initDerivedParameters();
}

MSLCM_SL2015::~MSLCM_SL2015() {
    changed();
}


void
MSLCM_SL2015::initDerivedParameters() {
    myChangeProbThresholdRight = ((0.2 / mySpeedGainRight) / MAX2(NUMERICAL_EPS, mySpeedGainParam));
    myChangeProbThresholdLeft = (0.2 / MAX2(NUMERICAL_EPS, mySpeedGainParam));
    mySpeedLossProbThreshold = (-0.1 + (1 - mySublaneParam));
}


bool
MSLCM_SL2015::debugVehicle() const {
    return DEBUG_COND;
}


int
MSLCM_SL2015::wantsChangeSublane(
    int laneOffset,
    LaneChangeAction alternatives,
    const MSLeaderDistanceInfo& leaders,
    const MSLeaderDistanceInfo& followers,
    const MSLeaderDistanceInfo& blockers,
    const MSLeaderDistanceInfo& neighLeaders,
    const MSLeaderDistanceInfo& neighFollowers,
    const MSLeaderDistanceInfo& neighBlockers,
    const MSLane& neighLane,
    const std::vector<MSVehicle::LaneQ>& preb,
    MSVehicle** lastBlocked,
    MSVehicle** firstBlocked,
    double& latDist, double& maneuverDist, int& blocked) {

    gDebugFlag2 = DEBUG_COND;
    const std::string changeType = laneOffset == -1 ? "right" : (laneOffset == 1 ? "left" : "current");

#ifdef DEBUG_MANEUVER
    if (gDebugFlag2) {
        std::cout << "\n" << SIMTIME
                  << std::setprecision(gPrecision)
                  << " veh=" << myVehicle.getID()
                  << " lane=" << myVehicle.getLane()->getID()
                  << " pos=" << myVehicle.getPositionOnLane()
                  << " posLat=" << myVehicle.getLateralPositionOnLane()
                  << " posLatError=" << mySigmaState
                  << " speed=" << myVehicle.getSpeed()
                  << " considerChangeTo=" << changeType
                  << "\n";
    }
#endif

    int result = _wantsChangeSublane(laneOffset,
                                     alternatives,
                                     leaders, followers, blockers,
                                     neighLeaders, neighFollowers, neighBlockers,
                                     neighLane, preb,
                                     lastBlocked, firstBlocked, latDist, maneuverDist, blocked);

    result = keepLatGap(result, leaders, followers, blockers,
                        neighLeaders, neighFollowers, neighBlockers,
                        neighLane, laneOffset, latDist, maneuverDist, blocked);

    result |= getLCA(result, latDist);
    // take into account lateral acceleration
#if defined(DEBUG_MANEUVER) || defined(DEBUG_STATE)
    double latDistTmp = latDist;
#endif
    latDist = SPEED2DIST(computeSpeedLat(latDist, maneuverDist));
#if defined(DEBUG_MANEUVER) || defined(DEBUG_STATE)
    if (gDebugFlag2 && latDist != latDistTmp) {
        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " maneuverDist=" << maneuverDist << " latDist=" << latDistTmp << " mySpeedPrev=" << mySpeedLat << " speedLat=" << DIST2SPEED(latDist) << " latDist2=" << latDist << "\n";
    }

    if (gDebugFlag2) {
        if (result & LCA_WANTS_LANECHANGE) {
            std::cout << SIMTIME
                      << " veh=" << myVehicle.getID()
                      << " wantsChangeTo=" << changeType
                      << " latDist=" << latDist
                      << " maneuverDist=" << maneuverDist
                      << " state=" << toString((LaneChangeAction)result)
                      << ((blocked & LCA_BLOCKED) ? " (blocked)" : "")
                      << ((blocked & LCA_OVERLAPPING) ? " (overlap)" : "")
                      << "\n\n";
        } else {
            std::cout << SIMTIME
                      << " veh=" << myVehicle.getID()
                      << " wantsNoChangeTo=" << changeType
                      << " state=" << toString((LaneChangeAction)result)
                      << "\n\n";
        }
    }
#endif
    gDebugFlag2 = false;
    return result;
}

void
MSLCM_SL2015::setOwnState(const int state) {
    MSAbstractLaneChangeModel::setOwnState(state);
    if (myVehicle.isActive()) {
        if ((state & (LCA_STRATEGIC | LCA_SPEEDGAIN)) != 0 && (state & LCA_BLOCKED) != 0) {
            myImpatience = MIN2(1.0, myImpatience + myVehicle.getActionStepLengthSecs() / myTimeToImpatience);
        } else {
            // impatience decays only to the driver-specific level
            myImpatience = MAX2(myMinImpatience, myImpatience - myVehicle.getActionStepLengthSecs() / myTimeToImpatience);
        }
#ifdef DEBUG_STATE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID()
                      << " setOwnState=" << toString((LaneChangeAction)state)
                      << " myMinImpatience=" << myMinImpatience
                      << " myImpatience=" << myImpatience
                      << "\n";
        }
#endif
        if ((state & LCA_STAY) != 0) {
            myCanChangeFully = true;
//            if (DEBUG_COND) {
//                std::cout << "    myCanChangeFully=true\n";
//            }
        }
    }
}


void
MSLCM_SL2015::updateSafeLatDist(const double travelledLatDist) {
    mySafeLatDistLeft -= travelledLatDist;
    mySafeLatDistRight += travelledLatDist;

    if (fabs(mySafeLatDistLeft) < NUMERICAL_EPS) {
        mySafeLatDistLeft = 0.;
    }
    if (fabs(mySafeLatDistRight) < NUMERICAL_EPS) {
        mySafeLatDistRight = 0.;
    }
}


double
MSLCM_SL2015::patchSpeed(const double min, const double wanted, const double max, const MSCFModel& cfModel) {
    gDebugFlag2 = DEBUG_COND;
    // negative min speed may be passed when using ballistic updated
    const double newSpeed = _patchSpeed(MAX2(min, 0.0), wanted, max, cfModel);
#ifdef DEBUG_PATCHSPEED
    if (gDebugFlag2) {
        const std::string patched = (wanted != newSpeed ? " patched=" + toString(newSpeed) : "");
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " lane=" << myVehicle.getLane()->getID()
                  << " pos=" << myVehicle.getPositionOnLane()
                  << " v=" << myVehicle.getSpeed()
                  << " min=" << min
                  << " wanted=" << wanted
                  << " max=" << max
                  << patched
                  << "\n\n";
    }
#endif
    gDebugFlag2 = false;
    return newSpeed;
}


double
MSLCM_SL2015::_patchSpeed(const double min, const double wanted, const double max, const MSCFModel& cfModel) {
    if (wanted <= 0) {
        return wanted;
    }

    int state = myOwnState;

    double nVSafe = wanted;
    bool gotOne = false;
    // letting vehicles merge in at the end of the lane in case of counter-lane change, step#2
    //   if we want to change and have a blocking leader and there is enough room for him in front of us
    if (myLeadingBlockerLength != 0) {
        double space = myLeftSpace - myLeadingBlockerLength - MAGIC_OFFSET - myVehicle.getVehicleType().getMinGap();
#ifdef DEBUG_PATCHSPEED
        if (gDebugFlag2) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " myLeadingBlockerLength=" << myLeadingBlockerLength << " space=" << space << "\n";
        }
#endif
        if (space >= 0) { // XXX space > -MAGIC_OFFSET
            // compute speed for decelerating towards a place which allows the blocking leader to merge in in front
            double safe = cfModel.stopSpeed(&myVehicle, myVehicle.getSpeed(), space);
            // if we are approaching this place
            if (safe < wanted) {
#ifdef DEBUG_PATCHSPEED
                if (gDebugFlag2) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " slowing down for leading blocker, safe=" << safe << (safe + NUMERICAL_EPS < min ? " (not enough)" : "") << "\n";
                }
#endif
                nVSafe = MAX2(min, safe);
                gotOne = true;
            }
        }
    }
    const double coopWeight = MAX2(0.0, MIN2(1.0, myCooperativeSpeed));
    for (std::vector<double>::const_iterator i = myLCAccelerationAdvices.begin(); i != myLCAccelerationAdvices.end(); ++i) {
        double v = myVehicle.getSpeed() + ACCEL2SPEED(*i);
        if (v >= min && v <= max) {
            nVSafe = MIN2(v * coopWeight + (1 - coopWeight) * wanted, nVSafe);
            gotOne = true;
#ifdef DEBUG_PATCHSPEED
            if (gDebugFlag2) {
                std::cout << SIMTIME << " veh=" << myVehicle.getID() << " got accel=" << (*i) << " nVSafe=" << nVSafe << "\n";
            }
#endif
        } else {
#ifdef DEBUG_PATCHSPEED
            if (v < min) {
                if (gDebugFlag2) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " ignoring low nVSafe=" << v << " min=" << min << "\n";
                }
            } else {
                if (gDebugFlag2) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " ignoring high nVSafe=" << v << " max=" << max << "\n";
                }
            }
#endif
        }
    }

    if (gotOne && !myDontBrake) {
#ifdef DEBUG_PATCHSPEED
        if (gDebugFlag2) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " got vSafe\n";
        }
#endif
        return nVSafe;
    }

    // check whether the vehicle is blocked
    if ((state & LCA_WANTS_LANECHANGE) != 0 && (state & LCA_BLOCKED) != 0) {
        if ((state & LCA_STRATEGIC) != 0) {
            // necessary decelerations are controlled via vSafe. If there are
            // none it means we should speed up
#if defined(DEBUG_PATCHSPEED) || defined(DEBUG_STATE)
            if (gDebugFlag2) {
                std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_WANTS_LANECHANGE (strat, no vSafe)\n";
            }
#endif
            return (max + wanted) / 2.0;
        } else if ((state & LCA_COOPERATIVE) != 0) {
            // only minor adjustments in speed should be done
            if ((state & LCA_BLOCKED_BY_LEADER) != 0) {
#if defined(DEBUG_PATCHSPEED) || defined(DEBUG_STATE)
                if (gDebugFlag2) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_BLOCKED_BY_LEADER (coop)\n";
                }
#endif
                return (min + wanted) / 2.0;
            }
            if ((state & LCA_BLOCKED_BY_FOLLOWER) != 0) {
#if defined(DEBUG_PATCHSPEED) || defined(DEBUG_STATE)
                if (gDebugFlag2) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_BLOCKED_BY_FOLLOWER (coop)\n";
                }
#endif
                return (max + wanted) / 2.0;
            }
            //} else { // VARIANT_16
            //    // only accelerations should be performed
            //    if ((state & LCA_BLOCKED_BY_FOLLOWER) != 0) {
            //        if (gDebugFlag2) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_BLOCKED_BY_FOLLOWER\n";
            //        return (max + wanted) / 2.0;
            //    }
        }
    }

    /*
    // decelerate if being a blocking follower
    //  (and does not have to change lanes)
    if ((state & LCA_AMBLOCKINGFOLLOWER) != 0) {
        if (fabs(max - myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle)) < 0.001 && min == 0) { // !!! was standing
            if (gDebugFlag2) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBLOCKINGFOLLOWER (standing)\n";
            return 0;
        }
        if (gDebugFlag2) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBLOCKINGFOLLOWER\n";

        //return min; // VARIANT_3 (brakeStrong)
        return (min + wanted) / 2.0;
    }
    if ((state & LCA_AMBACKBLOCKER) != 0) {
        if (max <= myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle) && min == 0) { // !!! was standing
            if (gDebugFlag2) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBACKBLOCKER (standing)\n";
            //return min; VARIANT_9 (backBlockVSafe)
            return nVSafe;
        }
    }
    if ((state & LCA_AMBACKBLOCKER_STANDING) != 0) {
        if (gDebugFlag2) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBACKBLOCKER_STANDING\n";
        //return min;
        return nVSafe;
    }
    */

    // accelerate if being a blocking leader or blocking follower not able to brake
    //  (and does not have to change lanes)
    if ((state & LCA_AMBLOCKINGLEADER) != 0) {
#if defined(DEBUG_PATCHSPEED) || defined(DEBUG_STATE)
        if (gDebugFlag2) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBLOCKINGLEADER\n";
        }
#endif
        return (max + wanted) / 2.0;
    }

    if ((state & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0) {
#if defined(DEBUG_PATCHSPEED) || defined(DEBUG_STATE)
        if (gDebugFlag2) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBLOCKINGFOLLOWER_DONTBRAKE\n";
        }
#endif
        /*
        // VARIANT_4 (dontbrake)
        if (max <= myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle) && min == 0) { // !!! was standing
            return wanted;
        }
        return (min + wanted) / 2.0;
        */
    }
    return wanted;
}


void*
MSLCM_SL2015::inform(void* info, MSVehicle* sender) {
    Info* pinfo = (Info*) info;
    if (pinfo->first >= 0) {
        addLCSpeedAdvice(pinfo->first);
    }
    //myOwnState &= 0xffffffff; // reset all bits of MyLCAEnum but only those
    myOwnState |= pinfo->second;
#ifdef DEBUG_INFORM
    if (gDebugFlag2 || DEBUG_COND || sender->getLaneChangeModel().debugVehicle()) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " informedBy=" << sender->getID()
                  << " info=" << pinfo->second
                  << " vSafe=" << pinfo->first
                  << "\n";
    }
#else
    UNUSED_PARAMETER(sender);
#endif
    delete pinfo;
    return (void*) true;
}


void
MSLCM_SL2015::msg(const CLeaderDist& cld, double speed, int state) {
    assert(cld.first != 0);
    ((MSVehicle*)cld.first)->getLaneChangeModel().inform(new Info(speed, state), &myVehicle);
}


double
MSLCM_SL2015::informLeader(int blocked,
                           int dir,
                           const CLeaderDist& neighLead,
                           double remainingSeconds) {
    double plannedSpeed = MIN2(myVehicle.getSpeed(),
                               myVehicle.getCarFollowModel().stopSpeed(&myVehicle, myVehicle.getSpeed(), myLeftSpace - myLeadingBlockerLength));
    for (std::vector<double>::const_iterator i = myLCAccelerationAdvices.begin(); i != myLCAccelerationAdvices.end(); ++i) {
        double v = myVehicle.getSpeed() + ACCEL2SPEED(*i);
        if (v >= myVehicle.getSpeed() - ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())) {
            plannedSpeed = MIN2(plannedSpeed, v);
        }
    }
#ifdef DEBUG_INFORM
    if (gDebugFlag2) {
        std::cout << " informLeader speed=" <<  myVehicle.getSpeed() << " planned=" << plannedSpeed << "\n";
    }
#endif

    if ((blocked & LCA_BLOCKED_BY_LEADER) != 0 && neighLead.first != 0) {
        const MSVehicle* nv = neighLead.first;
#ifdef DEBUG_INFORM
        if (gDebugFlag2) std::cout << " blocked by leader nv=" <<  nv->getID() << " nvSpeed=" << nv->getSpeed() << " needGap="
                                       << myVehicle.getCarFollowModel().getSecureGap(&myVehicle, nv, myVehicle.getSpeed(), nv->getSpeed(), nv->getCarFollowModel().getMaxDecel()) << "\n";
#endif
        // decide whether we want to overtake the leader or follow it
        const double dv = plannedSpeed - nv->getSpeed();
        const double overtakeDist = (neighLead.second // drive to back of follower
                                     + nv->getVehicleType().getLengthWithGap() // drive to front of follower
                                     + myVehicle.getVehicleType().getLength() // ego back reaches follower front
                                     + nv->getCarFollowModel().getSecureGap( // save gap to follower
                                         nv, &myVehicle, nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel()));

        if (dv < NUMERICAL_EPS
                // overtaking on the right on an uncongested highway is forbidden (noOvertakeLCLeft)
                || (dir == LCA_MLEFT && !myVehicle.congested() && !myAllowOvertakingRight)
                // not enough space to overtake? (we will start to brake when approaching a dead end)
                || myLeftSpace - myLeadingBlockerLength - myVehicle.getCarFollowModel().brakeGap(myVehicle.getSpeed()) < overtakeDist
                // not enough time to overtake?
                || dv * remainingSeconds < overtakeDist) {
            // cannot overtake
            msg(neighLead, -1, dir | LCA_AMBLOCKINGLEADER);
            // slow down smoothly to follow leader
            const double targetSpeed = myCarFollowModel.followSpeed(
                                           &myVehicle, myVehicle.getSpeed(), neighLead.second, nv->getSpeed(), nv->getCarFollowModel().getMaxDecel());
            if (targetSpeed < myVehicle.getSpeed()) {
                // slow down smoothly to follow leader
                const double decel = ACCEL2SPEED(MIN2(myVehicle.getCarFollowModel().getMaxDecel(),
                                                      MAX2(MIN_FALLBEHIND, (myVehicle.getSpeed() - targetSpeed) / remainingSeconds)));
                //const double nextSpeed = MAX2(0., MIN2(plannedSpeed, myVehicle.getSpeed() - decel));
                const double nextSpeed = MIN2(plannedSpeed, MAX2(0.0, myVehicle.getSpeed() - decel));
#ifdef DEBUG_INFORM
                if (gDebugFlag2) {
                    std::cout << SIMTIME
                              << " cannot overtake leader nv=" << nv->getID()
                              << " dv=" << dv
                              << " remainingSeconds=" << remainingSeconds
                              << " targetSpeed=" << targetSpeed
                              << " nextSpeed=" << nextSpeed
                              << "\n";
                }
#endif
                addLCSpeedAdvice(nextSpeed);
                return nextSpeed;
            } else {
                // leader is fast enough anyway
#ifdef DEBUG_INFORM
                if (gDebugFlag2) {
                    std::cout << SIMTIME
                              << " cannot overtake fast leader nv=" << nv->getID()
                              << " dv=" << dv
                              << " remainingSeconds=" << remainingSeconds
                              << " targetSpeed=" << targetSpeed
                              << "\n";
                }
#endif
                addLCSpeedAdvice(targetSpeed);
                return plannedSpeed;
            }
        } else {
#ifdef DEBUG_INFORM
            if (gDebugFlag2) {
                std::cout << SIMTIME
                          << " wants to overtake leader nv=" << nv->getID()
                          << " dv=" << dv
                          << " remainingSeconds=" << remainingSeconds
                          << " currentGap=" << neighLead.second
                          << " secureGap=" << nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel())
                          << " overtakeDist=" << overtakeDist
                          << " leftSpace=" << myLeftSpace
                          << " blockerLength=" << myLeadingBlockerLength
                          << "\n";
            }
#endif
            // overtaking, leader should not accelerate
            msg(neighLead, nv->getSpeed(), dir | LCA_AMBLOCKINGLEADER);
            return -1;
        }
    } else if (neighLead.first != 0) { // (remainUnblocked)
        // we are not blocked now. make sure we stay far enough from the leader
        const MSVehicle* nv = neighLead.first;
        double dv, nextNVSpeed;
        if (MSGlobals::gSemiImplicitEulerUpdate) {
            // XXX: the decrement (HELP_OVERTAKE) should be scaled with timestep length, I think.
            //      It seems to function as an estimate nv's speed in the next simstep!? (so HELP_OVERTAKE should be an acceleration value.)
            nextNVSpeed = nv->getSpeed() - HELP_OVERTAKE; // conservative
            dv = SPEED2DIST(myVehicle.getSpeed() - nextNVSpeed);
        } else {
            // Estimate neigh's speed after actionstep length
            // @note The possible breaking can be underestimated by the formula, so this is a potential
            //       source of collisions if actionsteplength>simsteplength.
            const double nvMaxDecel = HELP_OVERTAKE;
            nextNVSpeed = nv->getSpeed() - nvMaxDecel * myVehicle.getActionStepLengthSecs(); // conservative
            // Estimated gap reduction until next action step if own speed stays constant
            dv = SPEED2DIST(myVehicle.getSpeed() - nextNVSpeed);
        }
        const double targetSpeed = myCarFollowModel.followSpeed(
                                       &myVehicle, myVehicle.getSpeed(), neighLead.second - dv, nextNVSpeed, nv->getCarFollowModel().getMaxDecel());
        addLCSpeedAdvice(targetSpeed);
#ifdef DEBUG_INFORM
        if (gDebugFlag2) {
            std::cout << " not blocked by leader nv=" <<  nv->getID()
                      << " nvSpeed=" << nv->getSpeed()
                      << " gap=" << neighLead.second
                      << " nextGap=" << neighLead.second - dv
                      << " needGap=" << myVehicle.getCarFollowModel().getSecureGap(&myVehicle, nv, myVehicle.getSpeed(), nv->getSpeed(), nv->getCarFollowModel().getMaxDecel())
                      << " targetSpeed=" << targetSpeed
                      << "\n";
        }
#endif
        return MIN2(targetSpeed, plannedSpeed);
    } else {
        // not overtaking
        return plannedSpeed;
    }
}


void
MSLCM_SL2015::informFollower(int blocked,
                             int dir,
                             const CLeaderDist& neighFollow,
                             double remainingSeconds,
                             double plannedSpeed) {
    if ((blocked & LCA_BLOCKED_BY_FOLLOWER) != 0 && neighFollow.first != 0) {
        const MSVehicle* nv = neighFollow.first;
#ifdef DEBUG_INFORM
        if (gDebugFlag2) std::cout << " blocked by follower nv=" <<  nv->getID() << " nvSpeed=" << nv->getSpeed() << " needGap="
                                       << nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel()) << "\n";
#endif

        // are we fast enough to cut in without any help?
        if (plannedSpeed - nv->getSpeed() >= HELP_OVERTAKE) {
            const double neededGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
            if ((neededGap - neighFollow.second) / remainingSeconds < (plannedSpeed - nv->getSpeed())) {
#ifdef DEBUG_INFORM
                if (gDebugFlag2) {
                    std::cout << " wants to cut in before  nv=" << nv->getID() << " without any help neededGap=" << neededGap << "\n";
                }
#endif
                // follower might even accelerate but not to much
                msg(neighFollow, plannedSpeed - HELP_OVERTAKE, dir | LCA_AMBLOCKINGFOLLOWER);
                return;
            }
        }
        // decide whether we will request help to cut in before the follower or allow to be overtaken

        // PARAMETERS
        // assume other vehicle will assume the equivalent of 1 second of
        // maximum deceleration to help us (will probably be spread over
        // multiple seconds)
        // -----------
        const double helpDecel = nv->getCarFollowModel().getMaxDecel() * HELP_DECEL_FACTOR ;

        // change in the gap between ego and blocker over 1 second (not STEP!)
        const double neighNewSpeed = MAX2(0., nv->getSpeed() - ACCEL2SPEED(helpDecel));
        const double neighNewSpeed1s = MAX2(0., nv->getSpeed() - helpDecel);
        const double dv = plannedSpeed - neighNewSpeed1s;
        // new gap between follower and self in case the follower does brake for 1s
        const double decelGap = neighFollow.second + dv;
        const double secureGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, neighNewSpeed1s, plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
#ifdef DEBUG_INFORM
        if (gDebugFlag2) {
            std::cout << SIMTIME
                      << " egoV=" << myVehicle.getSpeed()
                      << " egoNV=" << plannedSpeed
                      << " nvNewSpeed=" << neighNewSpeed
                      << " nvNewSpeed1s=" << neighNewSpeed1s
                      << " deltaGap=" << dv
                      << " decelGap=" << decelGap
                      << " secGap=" << secureGap
                      << "\n";
        }
#endif
        if (decelGap > 0 && decelGap >= secureGap) {
            // if the blocking neighbor brakes it could actually help
            // how hard does it actually need to be?
            // to be safe in the next step the following equation has to hold:
            //   vsafe <= followSpeed(gap=currentGap - SPEED2DIST(vsafe), ...)
            // we compute an upper bound on vsafe by doing the computation twice
            const double vsafe1 = MAX2(neighNewSpeed, nv->getCarFollowModel().followSpeed(
                                           nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel()));
            const double vsafe = MAX2(neighNewSpeed, nv->getCarFollowModel().followSpeed(
                                          nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed - vsafe1), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel()));
            // the following assertion cannot be guaranteed because the CFModel handles small gaps differently, see MSCFModel::maximumSafeStopSpeed
            // assert(vsafe <= vsafe1);
            msg(neighFollow, vsafe, dir | LCA_AMBLOCKINGFOLLOWER);
#ifdef DEBUG_INFORM
            if (gDebugFlag2) {
                std::cout << " wants to cut in before nv=" << nv->getID()
                          << " vsafe1=" << vsafe1
                          << " vsafe=" << vsafe
                          << " newSecGap=" << nv->getCarFollowModel().getSecureGap(nv, &myVehicle, vsafe, plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel())
                          << "\n";
            }
#endif
        } else if (dv > 0 && dv * remainingSeconds > (secureGap - decelGap + POSITION_EPS)) {
            // decelerating once is sufficient to open up a large enough gap in time
            msg(neighFollow, neighNewSpeed, dir | LCA_AMBLOCKINGFOLLOWER);
#ifdef DEBUG_INFORM
            if (gDebugFlag2) {
                std::cout << " wants to cut in before nv=" << nv->getID() << " (eventually)\n";
            }
#endif
        } else if (dir == LCA_MRIGHT && !myAllowOvertakingRight && !nv->congested()) {
            const double vhelp = MAX2(neighNewSpeed, HELP_OVERTAKE);
            msg(neighFollow, vhelp, dir | LCA_AMBLOCKINGFOLLOWER);
#ifdef DEBUG_INFORM
            if (gDebugFlag2) {
                std::cout << " wants to cut in before nv=" << nv->getID() << " (nv cannot overtake right)\n";
            }
#endif
        } else {
            double vhelp = MAX2(nv->getSpeed(), myVehicle.getSpeed() + HELP_OVERTAKE);
            if (nv->getSpeed() > myVehicle.getSpeed() &&
                    ((dir == LCA_MRIGHT && myVehicle.getWaitingSeconds() > LCA_RIGHT_IMPATIENCE)
                     || (dir == LCA_MLEFT && plannedSpeed > CUT_IN_LEFT_SPEED_THRESHOLD) // VARIANT_22 (slowDownLeft)
                     // XXX this is a hack to determine whether the vehicles is on an on-ramp. This information should be retrieved from the network itself
                     || (dir == LCA_MLEFT && myLeftSpace > MAX_ONRAMP_LENGTH)
                    )) {
                // let the follower slow down to increase the likelyhood that later vehicles will be slow enough to help
                // follower should still be fast enough to open a gap
                vhelp = MAX2(neighNewSpeed, myVehicle.getSpeed() + HELP_OVERTAKE);
#ifdef DEBUG_INFORM
                if (gDebugFlag2) {
                    std::cout << " wants right follower to slow down a bit\n";
                }
#endif
                if ((nv->getSpeed() - myVehicle.getSpeed()) / helpDecel < remainingSeconds) {
#ifdef DEBUG_INFORM
                    if (gDebugFlag2) {
                        std::cout << " wants to cut in before right follower nv=" << nv->getID() << " (eventually)\n";
                    }
#endif
                    msg(neighFollow, vhelp, dir | LCA_AMBLOCKINGFOLLOWER);
                    return;
                }
            }
            msg(neighFollow, vhelp, dir | LCA_AMBLOCKINGFOLLOWER);
            // this follower is supposed to overtake us. slow down smoothly to allow this
            const double overtakeDist = (neighFollow.second // follower reaches ego back
                                         + myVehicle.getVehicleType().getLengthWithGap() // follower reaches ego front
                                         + nv->getVehicleType().getLength() // follower back at ego front
                                         + myVehicle.getCarFollowModel().getSecureGap( // follower has safe dist to ego
                                             &myVehicle, nv, plannedSpeed, vhelp, nv->getCarFollowModel().getMaxDecel()));
            // speed difference to create a sufficiently large gap
            const double needDV = overtakeDist / remainingSeconds;
            // make sure the deceleration is not to strong
            addLCSpeedAdvice(MAX2(vhelp - needDV, myVehicle.getSpeed() - ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())));

#ifdef DEBUG_INFORM
            if (gDebugFlag2) {
                std::cout << SIMTIME
                          << " veh=" << myVehicle.getID()
                          << " wants to be overtaken by=" << nv->getID()
                          << " overtakeDist=" << overtakeDist
                          << " vneigh=" << nv->getSpeed()
                          << " vhelp=" << vhelp
                          << " needDV=" << needDV
                          << " vsafe=" << myVehicle.getSpeed() + ACCEL2SPEED(myLCAccelerationAdvices.back())
                          << "\n";
            }
#endif
        }
    } else if (neighFollow.first != 0) {
        // we are not blocked no, make sure it remains that way
        const MSVehicle* nv = neighFollow.first;
        const double vsafe1 = nv->getCarFollowModel().followSpeed(
                                  nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
        const double vsafe = nv->getCarFollowModel().followSpeed(
                                 nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed - vsafe1), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
        msg(neighFollow, vsafe, dir | LCA_AMBLOCKINGFOLLOWER);
#ifdef DEBUG_INFORM
        if (gDebugFlag2) {
            std::cout << " wants to cut in before non-blocking follower nv=" << nv->getID() << "\n";
        }
#endif
    }
}

double
MSLCM_SL2015::informLeaders(int blocked, int dir,
                            const std::vector<CLeaderDist>& blockers,
                            double remainingSeconds) {
    double plannedSpeed = myVehicle.getSpeed();
    double space = myLeftSpace;
    if (myLeadingBlockerLength != 0) {
        // see patchSpeed @todo: refactor
        space -= myLeadingBlockerLength - MAGIC_OFFSET - myVehicle.getVehicleType().getMinGap();
        if (space <= 0) {
            // ignore leading blocker
            space = myLeftSpace;
        }
    }
    double safe = myVehicle.getCarFollowModel().stopSpeed(&myVehicle, myVehicle.getSpeed(), space);
    plannedSpeed = MIN2(plannedSpeed, safe);

    for (std::vector<CLeaderDist>::const_iterator it = blockers.begin(); it != blockers.end(); ++it) {
        plannedSpeed = MIN2(plannedSpeed, informLeader(blocked, dir, *it, remainingSeconds));
    }
    return plannedSpeed;
}


void
MSLCM_SL2015::informFollowers(int blocked, int dir,
                              const std::vector<CLeaderDist>& blockers,
                              double remainingSeconds,
                              double plannedSpeed) {
    // #3727
    for (std::vector<CLeaderDist>::const_iterator it = blockers.begin(); it != blockers.end(); ++it) {
        informFollower(blocked, dir, *it, remainingSeconds, plannedSpeed);
    }
}


void
MSLCM_SL2015::prepareStep() {
    MSAbstractLaneChangeModel::prepareStep();
    // keep information about strategic change direction
    myOwnState = (myOwnState & (LCA_STRATEGIC | LCA_COOPERATIVE)) ? (myOwnState & LCA_WANTS_LANECHANGE) : 0;
#ifdef DEBUG_INFORM
    if (debugVehicle()) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " prepareStep"
                  << " myCanChangeFully=" << myCanChangeFully
                  << "\n";
    }
#endif
    myLeadingBlockerLength = 0;
    myLeftSpace = 0;
    myLCAccelerationAdvices.clear();
    myDontBrake = false;
    myCFRelated.clear();
    myCFRelatedReady = false;
    const double halfWidth = getWidth() * 0.5;
    const double center = myVehicle.getCenterOnEdge();
    mySafeLatDistRight = center - halfWidth;
    mySafeLatDistLeft = myVehicle.getLane()->getEdge().getWidth() - center - halfWidth;
    // truncate to work around numerical instability between different builds
    mySpeedGainProbabilityRight = ceil(mySpeedGainProbabilityRight * 100000.0) * 0.00001;
    mySpeedGainProbabilityLeft = ceil(mySpeedGainProbabilityLeft * 100000.0) * 0.00001;
    myKeepRightProbability = ceil(myKeepRightProbability * 100000.0) * 0.00001;
    // updated myExpectedSublaneSpeeds
    // XXX only do this when (sub)lane changing is possible
    std::vector<double> newExpectedSpeeds;
#ifdef DEBUG_INFORM
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " myExpectedSublaneSpeeds=" << toString(myExpectedSublaneSpeeds) << "\n";
    }
#endif
    if (myExpectedSublaneSpeeds.size() != myVehicle.getLane()->getEdge().getSubLaneSides().size()) {
        // initialize
        const MSEdge* currEdge = &myVehicle.getLane()->getEdge();
        const std::vector<MSLane*>& lanes = currEdge->getLanes();
        for (std::vector<MSLane*>::const_iterator it_lane = lanes.begin(); it_lane != lanes.end(); ++it_lane) {
            const int subLanes = MAX2(1, int(ceil((*it_lane)->getWidth() / MSGlobals::gLateralResolution)));
            for (int i = 0; i < subLanes; ++i) {
                newExpectedSpeeds.push_back((*it_lane)->getVehicleMaxSpeed(&myVehicle));
            }
        }
        if (myExpectedSublaneSpeeds.size() > 0) {
            // copy old values
            assert(myLastEdge != 0);
            if (myLastEdge->getSubLaneSides().size() == myExpectedSublaneSpeeds.size()) {
                const int subLaneShift = computeSublaneShift(myLastEdge, currEdge);
                if (subLaneShift < std::numeric_limits<int>::max()) {
                    for (int i = 0; i < (int)myExpectedSublaneSpeeds.size(); ++i) {
                        const int newI = i + subLaneShift;
                        if (newI > 0 && newI < (int)newExpectedSpeeds.size()) {
                            newExpectedSpeeds[newI] = myExpectedSublaneSpeeds[i];
                        }
                    }
                }
            }
        }
        myExpectedSublaneSpeeds = newExpectedSpeeds;
        myLastEdge = currEdge;
    }
    assert(myExpectedSublaneSpeeds.size() == myVehicle.getLane()->getEdge().getSubLaneSides().size());
    if (mySigma > 0) {
        mySigmaState += getLateralDrift();
    }
}

double
MSLCM_SL2015::getLateralDrift() {
    //OUProcess::step(double state, double dt, double timeScale, double noiseIntensity)
    const double deltaState = OUProcess::step(mySigmaState,
                              myVehicle.getActionStepLengthSecs(),
                              MAX2(NUMERICAL_EPS, (1 - mySigma) * 100), mySigma) - mySigmaState;
    const double scaledDelta = deltaState * myVehicle.getSpeed() / myVehicle.getLane()->getSpeedLimit();
    return scaledDelta;
}

double
MSLCM_SL2015::getPosLat() {
    return myVehicle.getLateralPositionOnLane() + mySigmaState;
}

int
MSLCM_SL2015::computeSublaneShift(const MSEdge* prevEdge, const MSEdge* curEdge) {
    // find the first lane that targets the new edge
    int prevShift = 0;
    const std::vector<MSLane*>& lanes = prevEdge->getLanes();
    for (std::vector<MSLane*>::const_iterator it_lane = lanes.begin(); it_lane != lanes.end(); ++it_lane) {
        const MSLane* lane = *it_lane;
        for (MSLinkCont::const_iterator it_link = lane->getLinkCont().begin(); it_link != lane->getLinkCont().end(); ++it_link) {
            if (&((*it_link)->getLane()->getEdge()) == curEdge) {
                int curShift = 0;
                const MSLane* target = (*it_link)->getLane();
                const std::vector<MSLane*>& lanes2 = curEdge->getLanes();
                for (std::vector<MSLane*>::const_iterator it_lane2 = lanes2.begin(); it_lane2 != lanes2.end(); ++it_lane2) {
                    const MSLane* lane2 = *it_lane2;
                    if (lane2 == target) {
                        return prevShift + curShift;
                    }
                    MSLeaderInfo ahead(lane2);
                    curShift += ahead.numSublanes();
                }
                assert(false);
            }
        }
        MSLeaderInfo ahead(lane);
        prevShift -= ahead.numSublanes();
    }
    return std::numeric_limits<int>::max();
}


void
MSLCM_SL2015::changed() {
    if (!myCanChangeFully) {
        // do not reset state yet
#ifdef DEBUG_STATE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " state not reset\n";
        }
#endif
        return;
    }
    myOwnState = 0;
    // XX do not reset values for unfinished maneuvers
    mySpeedGainProbabilityRight = 0;
    mySpeedGainProbabilityLeft = 0;
    myKeepRightProbability = 0;

    if (myVehicle.getBestLaneOffset() == 0) {
        // if we are not yet on our best lane there might still be unseen blockers
        // (during patchSpeed)
        myLeadingBlockerLength = 0;
        myLeftSpace = 0;
    }
    myLookAheadSpeed = LOOK_AHEAD_MIN_SPEED;
    myLCAccelerationAdvices.clear();
    myDontBrake = false;
#if defined(DEBUG_MANEUVER) || defined(DEBUG_STATE)
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " changed()\n";
    }
#endif
}


int
MSLCM_SL2015::_wantsChangeSublane(
    int laneOffset,
    LaneChangeAction alternatives,
    const MSLeaderDistanceInfo& leaders,
    const MSLeaderDistanceInfo& followers,
    const MSLeaderDistanceInfo& blockers,
    const MSLeaderDistanceInfo& neighLeaders,
    const MSLeaderDistanceInfo& neighFollowers,
    const MSLeaderDistanceInfo& neighBlockers,
    const MSLane& neighLane,
    const std::vector<MSVehicle::LaneQ>& preb,
    MSVehicle** lastBlocked,
    MSVehicle** firstBlocked,
    double& latDist, double& maneuverDist, int& blocked) {

    const SUMOTime currentTime = MSNet::getInstance()->getCurrentTimeStep();
    // compute bestLaneOffset
    MSVehicle::LaneQ curr, neigh, best;
    int bestLaneOffset = 0;
    double currentDist = 0;
    double neighDist = 0;
    int currIdx = 0;
    MSLane* prebLane = myVehicle.getLane();
    if (prebLane->getEdge().isInternal()) {
        // internal edges are not kept inside the bestLanes structure
        prebLane = prebLane->getLinkCont()[0]->getLane();
    }
    // special case: vehicle considers changing to the opposite direction edge
    const bool checkOpposite = &neighLane.getEdge() != &myVehicle.getLane()->getEdge();
    const int prebOffset = (checkOpposite ? 0 : laneOffset);
    for (int p = 0; p < (int) preb.size(); ++p) {
        if (preb[p].lane == prebLane && p + laneOffset >= 0) {
            assert(p + prebOffset < (int)preb.size());
            curr = preb[p];
            neigh = preb[p + prebOffset];
            currentDist = curr.length;
            neighDist = neigh.length;
            bestLaneOffset = curr.bestLaneOffset;
            // VARIANT_13 (equalBest)
            if (bestLaneOffset == 0 && preb[p + laneOffset].bestLaneOffset == 0) {
#ifdef DEBUG_WANTSCHANGE
                if (gDebugFlag2) {
                    std::cout << STEPS2TIME(currentTime)
                              << " veh=" << myVehicle.getID()
                              << " bestLaneOffsetOld=" << bestLaneOffset
                              << " bestLaneOffsetNew=" << laneOffset
                              << "\n";
                }
#endif
                bestLaneOffset = laneOffset;
            }
            best = preb[p + bestLaneOffset];
            currIdx = p;
            break;
        }
    }
    double driveToNextStop = -std::numeric_limits<double>::max();
    UNUSED_PARAMETER(driveToNextStop); // XXX use when computing usableDist
    if (myVehicle.nextStopDist() < std::numeric_limits<double>::max()
            && &myVehicle.getNextStop().lane->getEdge() == &myVehicle.getLane()->getEdge()) {
        // vehicle can always drive up to stop distance
        // @note this information is dynamic and thus not available in updateBestLanes()
        // @note: nextStopDist was compute before the vehicle moved
        driveToNextStop = myVehicle.nextStopDist();
        const double stopPos = myVehicle.getPositionOnLane() + myVehicle.nextStopDist() - myVehicle.getLastStepDist();
#ifdef DEBUG_WANTS_CHANGE
        if (DEBUG_COND) {
            std::cout << SIMTIME << std::setprecision(gPrecision) << " veh=" << myVehicle.getID()
                      << " stopDist=" << myVehicle.nextStopDist()
                      << " lastDist=" << myVehicle.getLastStepDist()
                      << " stopPos=" << stopPos
                      << " currentDist=" << currentDist
                      << " neighDist=" << neighDist
                      << "\n";
        }
#endif
        currentDist = MAX2(currentDist, stopPos);
        neighDist = MAX2(neighDist, stopPos);
    }
    // direction specific constants
    const bool right = (laneOffset == -1);
    const bool left = (laneOffset == 1);
    const int myLca = (right ? LCA_MRIGHT : (left ? LCA_MLEFT : 0));
    const int lcaCounter = (right ? LCA_LEFT : (left ? LCA_RIGHT : LCA_NONE));
    const bool changeToBest = (right && bestLaneOffset < 0) || (left && bestLaneOffset > 0) || (laneOffset == 0 && bestLaneOffset == 0);
    // keep information about being a leader/follower but remove information
    // about previous lane change request or urgency
    int ret = (myOwnState & 0xffff0000);

    // compute the distance when changing to the neighboring lane
    // (ensure we do not lap into the line behind neighLane since there might be unseen blockers)
    // minimum distance to move the vehicle fully onto the new lane
    double latLaneDist = laneOffset == 0 ? 0. : myVehicle.lateralDistanceToLane(laneOffset);

    // VARIANT_5 (disableAMBACKBLOCKER1)
    /*
    if (leader.first != 0
            && (myOwnState & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0
            && (leader.first->getLaneChangeModel().getOwnState() & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0) {

        myOwnState &= (0xffffffff - LCA_AMBLOCKINGFOLLOWER_DONTBRAKE);
        if (myVehicle.getSpeed() > SUMO_const_haltingSpeed) {
            myOwnState |= LCA_AMBACKBLOCKER;
        } else {
            ret |= LCA_AMBACKBLOCKER;
            myDontBrake = true;
        }
    }
    */

#ifdef DEBUG_WANTSCHANGE
    if (gDebugFlag2) {
        std::cout << STEPS2TIME(currentTime)
                  << " veh=" << myVehicle.getID()
                  << " myState=" << toString((LaneChangeAction)myOwnState)
                  << " firstBlocked=" << Named::getIDSecure(*firstBlocked)
                  << " lastBlocked=" << Named::getIDSecure(*lastBlocked)
                  << "\n         leaders=" << leaders.toString()
                  << "\n       followers=" << followers.toString()
                  << "\n        blockers=" << blockers.toString()
                  << "\n    neighLeaders=" << neighLeaders.toString()
                  << "\n  neighFollowers=" << neighFollowers.toString()
                  << "\n   neighBlockers=" << neighBlockers.toString()
                  << "\n   changeToBest=" << changeToBest
                  << " latLaneDist=" << latLaneDist
                  << "\n   expectedSpeeds=" << toString(myExpectedSublaneSpeeds)
                  << std::endl;
    }
#endif

    ret = slowDownForBlocked(lastBlocked, ret);
    // VARIANT_14 (furtherBlock)
    if (lastBlocked != firstBlocked) {
        ret = slowDownForBlocked(firstBlocked, ret);
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
    // lookAheadDistance:
    // we do not want the lookahead distance to change all the time so we discrectize the speed a bit

    // VARIANT_18 (laHyst)
    if (myVehicle.getSpeed() > myLookAheadSpeed) {
        myLookAheadSpeed = myVehicle.getSpeed();
    } else {
        // FIXME: This strongly dependent on the value of TS, see LC2013 for the fix (l.1153, currently)
        myLookAheadSpeed = MAX2(LOOK_AHEAD_MIN_SPEED,
                                (LOOK_AHEAD_SPEED_MEMORY * myLookAheadSpeed + (1 - LOOK_AHEAD_SPEED_MEMORY) * myVehicle.getSpeed()));
    }
    //myLookAheadSpeed = myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle);

    //double laDist = laSpeed > LOOK_FORWARD_SPEED_DIVIDER
    //              ? laSpeed *  LOOK_FORWARD_FAR
    //              : laSpeed *  LOOK_FORWARD_NEAR;
    double laDist = myLookAheadSpeed * LOOK_FORWARD * myStrategicParam * (right ? 1 : myLookaheadLeft);
    laDist += myVehicle.getVehicleType().getLengthWithGap() * 2.;
    // aggressive drivers may elect to use reduced strategic lookahead to optimize speed
    /*
    if (mySpeedGainProbabilityRight > myChangeProbThresholdRight
            || mySpeedGainProbabilityLeft > myChangeProbThresholdLeft) {
        laDist *= MAX2(0.0, (1 - myPushy));
        laDist *= MAX2(0,0, (1 - myAssertive));
        laDist *= MAX2(0,0, (2 - mySpeedGainParam));
    }
    */

    // react to a stopped leader on the current lane
    if (bestLaneOffset == 0 && leaders.hasStoppedVehicle()) {
        // value is doubled for the check since we change back and forth
        // laDist = 0.5 * (myVehicle.getVehicleType().getLengthWithGap() + leader.first->getVehicleType().getLengthWithGap());
        // XXX determine length of longest stopped vehicle
        laDist = myVehicle.getVehicleType().getLengthWithGap();
    }
    if (myStrategicParam < 0) {
        laDist = -1e3; // never perform strategic change
    }

    // free space that is available for changing
    //const double neighSpeed = (neighLead.first != 0 ? neighLead.first->getSpeed() :
    //        neighFollow.first != 0 ? neighFollow.first->getSpeed() :
    //        best.lane->getSpeedLimit());
    // @note: while this lets vehicles change earlier into the correct direction
    // it also makes the vehicles more "selfish" and prevents changes which are necessary to help others

    const double roundaboutBonus = MSLCHelper::getRoundaboutDistBonus(myVehicle, myRoundaboutBonus, curr, neigh, best);
    currentDist += roundaboutBonus;
    neighDist += roundaboutBonus;

    if (laneOffset != 0) {
        ret = checkStrategicChange(ret,
                                   laneOffset,
                                   preb,
                                   leaders,
                                   neighLeaders,
                                   currIdx,
                                   bestLaneOffset,
                                   changeToBest,
                                   currentDist,
                                   neighDist,
                                   laDist,
                                   roundaboutBonus,
                                   latLaneDist,
                                   latDist);
    }

    if ((ret & LCA_STAY) != 0 && latDist == 0) {
        // ensure that mySafeLatDistLeft / mySafeLatDistRight are up to date for the
        // subsquent check with laneOffset = 0
        const double center = myVehicle.getCenterOnEdge();
        updateGaps(neighLeaders, neighLane.getRightSideOnEdge(), center, 1.0, mySafeLatDistRight, mySafeLatDistLeft);
        updateGaps(neighFollowers, neighLane.getRightSideOnEdge(), center, 1.0, mySafeLatDistRight, mySafeLatDistLeft);
        return ret;
    }
    if ((ret & LCA_URGENT) != 0) {
        // prepare urgent lane change maneuver
        if (changeToBest && abs(bestLaneOffset) > 1
                && curr.bestContinuations.back()->getLinkCont().size() != 0
           ) {
            // there might be a vehicle which needs to counter-lane-change one lane further and we cannot see it yet
            const double reserve = MIN2(myLeftSpace - MAGIC_OFFSET - myVehicle.getVehicleType().getMinGap(), right ? 20.0 : 40.0);
            myLeadingBlockerLength = MAX2(reserve, myLeadingBlockerLength);
#ifdef DEBUG_WANTSCHANGE
            if (gDebugFlag2) {
                std::cout << "  reserving space for unseen blockers myLeadingBlockerLength=" << myLeadingBlockerLength << "\n";
            }
#endif
        }

        // letting vehicles merge in at the end of the lane in case of counter-lane change, step#1
        //   if there is a leader and he wants to change to the opposite direction
        const MSVehicle* neighLeadLongest = getLongest(neighLeaders).first;
        saveBlockerLength(neighLeadLongest, lcaCounter);
        if (*firstBlocked != neighLeadLongest) {
            saveBlockerLength(*firstBlocked, lcaCounter);
        }
        std::vector<CLeaderDist> collectLeadBlockers;
        std::vector<CLeaderDist> collectFollowBlockers;
        int blockedFully = 0; // wether execution of the full maneuver is blocked
        maneuverDist = latDist;
        const double gapFactor = computeGapFactor(LCA_STRATEGIC);
        blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                leaders, followers, blockers,
                                neighLeaders, neighFollowers, neighBlockers, &collectLeadBlockers, &collectFollowBlockers,
                                false, gapFactor, &blockedFully);

        const double absLaneOffset = fabs(bestLaneOffset != 0 ? bestLaneOffset : latDist / SUMO_const_laneWidth);
        const double remainingSeconds = ((ret & LCA_TRACI) == 0 ?
                                         MAX2(STEPS2TIME(TS), myLeftSpace / MAX2(myLookAheadSpeed, NUMERICAL_EPS) / absLaneOffset / URGENCY) :
                                         myVehicle.getInfluencer().changeRequestRemainingSeconds(currentTime));
        const double plannedSpeed = informLeaders(blocked, myLca, collectLeadBlockers, remainingSeconds);
        // coordinate with direct obstructions
        if (plannedSpeed >= 0) {
            // maybe we need to deal with a blocking follower
            informFollowers(blocked, myLca, collectFollowBlockers, remainingSeconds, plannedSpeed);
        }
        if (plannedSpeed > 0) {
            commitManoeuvre(blocked, blockedFully, leaders, neighLeaders, neighLane, maneuverDist);
        }
#if defined(DEBUG_WANTSCHANGE) || defined(DEBUG_STATE)
        if (gDebugFlag2) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " myLeftSpace=" << myLeftSpace
                      << " changeFully=" << myCanChangeFully
                      << " blockedFully=" << toString((LaneChangeAction)blockedFully)
                      << " remainingSeconds=" << remainingSeconds
                      << " plannedSpeed=" << plannedSpeed
                      << " mySafeLatDistRight=" << mySafeLatDistRight
                      << " mySafeLatDistLeft=" << mySafeLatDistLeft
                      << "\n";
        }
#endif
        return ret;
    }
    // VARIANT_15
    if (roundaboutBonus > 0) {

#ifdef DEBUG_WANTS_CHANGE
        if (DEBUG_COND) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " roundaboutBonus=" << roundaboutBonus
                      << " myLeftSpace=" << myLeftSpace
                      << "\n";
        }
#endif
        // try to use the inner lanes of a roundabout to increase throughput
        // unless we are approaching the exit
        if (left) {
            ret |= LCA_COOPERATIVE;
            if (!cancelRequest(ret, laneOffset)) {
                if ((ret & LCA_STAY) == 0) {
                    latDist = latLaneDist;
                    maneuverDist = latLaneDist;
                    blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                            leaders, followers, blockers,
                                            neighLeaders, neighFollowers, neighBlockers);
                }
                return ret;
            }
        } else {
            myKeepRightProbability = 0;
        }
    }

    // --------

    // -------- make place on current lane if blocking follower
    //if (amBlockingFollowerPlusNB()) {
    //    std::cout << myVehicle.getID() << ", " << currentDistAllows(neighDist, bestLaneOffset, laDist)
    //        << " neighDist=" << neighDist
    //        << " currentDist=" << currentDist
    //        << "\n";
    //}
    const double inconvenience = (latLaneDist < 0
                                  ? -mySpeedGainProbabilityRight / myChangeProbThresholdRight
                                  : -mySpeedGainProbabilityLeft / myChangeProbThresholdLeft);
    if (laneOffset != 0
            && ((amBlockingFollowerPlusNB()
                 // VARIANT_6 : counterNoHelp
                 && ((myOwnState & myLca) != 0))
                ||
                // continue previous cooperative change
                ((myPreviousState & LCA_COOPERATIVE) != 0
                 && !myCanChangeFully
                 // change is in the right direction
                 && (laneOffset * getManeuverDist() > 0)))
            && (inconvenience < myCooperativeParam)
            && (changeToBest || currentDistAllows(neighDist, abs(bestLaneOffset) + 1, laDist))) {

        // VARIANT_2 (nbWhenChangingToHelp)
#ifdef DEBUG_COOPERATE
        if (gDebugFlag2) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " amBlocking=" << amBlockingFollowerPlusNB()
                      << " prevState=" << toString((LaneChangeAction)myPreviousState)
                      << " origLatDist=" << getManeuverDist()
                      << " wantsChangeToHelp=" << (right ? "right" : "left")
                      << " state=" << myOwnState
                      //<< (((myOwnState & myLca) == 0) ? " (counter)" : "")
                      << "\n";
        }
#endif

        ret |= LCA_COOPERATIVE | LCA_URGENT ;//| LCA_CHANGE_TO_HELP;
        if (!cancelRequest(ret, laneOffset)) {
            latDist = amBlockingFollowerPlusNB() ? latLaneDist : getManeuverDist();
            maneuverDist = latDist;
            blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                    leaders, followers, blockers,
                                    neighLeaders, neighFollowers, neighBlockers);
            return ret;
        }
    }

    // --------


    //// -------- security checks for krauss
    ////  (vsafe fails when gap<0)
    //if ((blocked & LCA_BLOCKED) != 0) {
    //    return ret;
    //}
    //// --------

    // -------- higher speed
    //if ((congested(neighLead.first) && neighLead.second < 20) || predInteraction(leader.first)) { //!!!
    //    return ret;
    //}

    // iterate over all possible combinations of sublanes this vehicle might cover and check the potential speed
    const MSEdge& edge = myVehicle.getLane()->getEdge();
    const std::vector<double>& sublaneSides = edge.getSubLaneSides();
    assert(sublaneSides.size() == myExpectedSublaneSpeeds.size());
    const double vehWidth = getWidth();
    const double rightVehSide = myVehicle.getCenterOnEdge() - 0.5 * vehWidth;
    const double leftVehSide = rightVehSide + vehWidth;
    // figure out next speed when staying where we are
    double defaultNextSpeed = std::numeric_limits<double>::max();
    /// determine the leftmost and rightmost sublanes currently occupied
    int leftmostOnEdge = (int)sublaneSides.size() - 1;
    while (leftmostOnEdge > 0 && sublaneSides[leftmostOnEdge] > leftVehSide) {
        leftmostOnEdge--;
    }
    int rightmostOnEdge = leftmostOnEdge;
    while (rightmostOnEdge > 0 && sublaneSides[rightmostOnEdge] > rightVehSide + NUMERICAL_EPS) {
        defaultNextSpeed = MIN2(defaultNextSpeed, myExpectedSublaneSpeeds[rightmostOnEdge]);
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << "   adapted to current sublane=" << rightmostOnEdge << " defaultNextSpeed=" << defaultNextSpeed << "\n";
            std::cout << "   sublaneSides[rightmostOnEdge]=" << sublaneSides[rightmostOnEdge] << " rightVehSide=" << rightVehSide << "\n";
        }
#endif
        rightmostOnEdge--;
    }
    defaultNextSpeed = MIN2(defaultNextSpeed, myExpectedSublaneSpeeds[rightmostOnEdge]);
#ifdef DEBUG_WANTSCHANGE
    if (gDebugFlag2) {
        std::cout << "   adapted to current sublane=" << rightmostOnEdge << " defaultNextSpeed=" << defaultNextSpeed << "\n";
        std::cout << "   sublaneSides[rightmostOnEdge]=" << sublaneSides[rightmostOnEdge] << " rightVehSide=" << rightVehSide << "\n";
    }
#endif
    double maxGain = -std::numeric_limits<double>::max();
    double maxGainRight = -std::numeric_limits<double>::max();
    double maxGainLeft = -std::numeric_limits<double>::max();
    double latDistNice = std::numeric_limits<double>::max();

    const int iMin = MIN2(myVehicle.getLane()->getRightmostSublane(), neighLane.getRightmostSublane());
    const double leftMax = MAX2(
                               myVehicle.getLane()->getRightSideOnEdge() + myVehicle.getLane()->getWidth(),
                               neighLane.getRightSideOnEdge() + neighLane.getWidth());
    assert(leftMax <= edge.getWidth());
    int sublaneCompact = MAX2(iMin, rightmostOnEdge - 1); // try to compactify to the right by default

#ifdef DEBUG_WANTSCHANGE
    if (gDebugFlag2) std::cout
                << "  checking sublanes rightmostOnEdge=" << rightmostOnEdge
                << " leftmostOnEdge=" << leftmostOnEdge
                << " iMin=" << iMin
                << " leftMax=" << leftMax
                << " sublaneCompact=" << sublaneCompact
                << "\n";
#endif
    const double laneBoundary = laneOffset < 0 ? myVehicle.getLane()->getRightSideOnEdge() : neighLane.getRightSideOnEdge();
    for (int i = iMin; i < (int)sublaneSides.size(); ++i) {
        if (sublaneSides[i] + vehWidth < leftMax) {
            // i is the rightmost sublane and the left side of vehicles still fits on the edge,
            // compute min speed of all sublanes covered by the vehicle in this case
            double vMin = myExpectedSublaneSpeeds[i];
            //std::cout << "   i=" << i << "\n";
            int j = i;
            while (vMin > 0 && j < (int)sublaneSides.size() && sublaneSides[j] < sublaneSides[i] + vehWidth) {
                vMin = MIN2(vMin, myExpectedSublaneSpeeds[j]);
                //std::cout << "     j=" << j << " vMin=" << vMin << " sublaneSides[j]=" << sublaneSides[j] << " leftVehSide=" << leftVehSide << " rightVehSide=" << rightVehSide << "\n";
                ++j;
            }
            // check whether the vehicle is between lanes
            if (laneOffset != 0 && overlap(sublaneSides[i], sublaneSides[i] + vehWidth, laneBoundary, laneBoundary)) {
                vMin *= (1 - myLaneDiscipline);
            }
            const double relativeGain = (vMin - defaultNextSpeed) / MAX2(vMin, RELGAIN_NORMALIZATION_MIN_SPEED);
            const double currentLatDist = sublaneSides[i] - rightVehSide;
            // @note this is biased for changing to the left since we compare the sublanes in ascending order
            if (relativeGain > maxGain) {
                maxGain = relativeGain;
                if (maxGain > GAIN_PERCEPTION_THRESHOLD) {
                    sublaneCompact = i;
                    latDist = currentLatDist;
#ifdef DEBUG_WANTSCHANGE
                    if (gDebugFlag2) {
                        std::cout << "      i=" << i << " newLatDist=" << latDist << " relGain=" << relativeGain << "\n";
                    }
#endif
                }
            } else {
                // if anticipated gains to the left are higher then to the right and current gains are equal, prefer left
                if (currentLatDist > 0
                        //&& latDist < 0 // #7184 compensates for #7185
                        && mySpeedGainProbabilityLeft > mySpeedGainProbabilityRight
                        && relativeGain > GAIN_PERCEPTION_THRESHOLD
                        && maxGain - relativeGain < NUMERICAL_EPS) {
                    latDist = currentLatDist;
                }
            }
#ifdef DEBUG_WANTSCHANGE
            if (gDebugFlag2) {
                std::cout << "    i=" << i << " rightmostOnEdge=" << rightmostOnEdge << " vMin=" << vMin << " relGain=" << relativeGain << " sublaneCompact=" << sublaneCompact << " curLatDist=" << currentLatDist << "\n";
            }
#endif
            if (currentLatDist < -NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) {
                maxGainRight = MAX2(maxGainRight, relativeGain);
            } else if (currentLatDist > NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) {
                maxGainLeft = MAX2(maxGainLeft, relativeGain);
            }
            const double subAlignDist = sublaneSides[i] - rightVehSide;
            if (fabs(subAlignDist) < fabs(latDistNice)) {
                latDistNice = subAlignDist;
#ifdef DEBUG_WANTSCHANGE
                if (gDebugFlag2) std::cout
                            << "    nicest sublane=" << i
                            << " side=" << sublaneSides[i]
                            << " rightSide=" << rightVehSide
                            << " latDistNice=" << latDistNice
                            << " maxGainR=" << maxGainRight
                            << " maxGainL=" << maxGainLeft
                            << "\n";
#endif
            }
        }
    }
    // updated change probabilities
    if (maxGainRight != -std::numeric_limits<double>::max()) {
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << "  speedGainR_old=" << mySpeedGainProbabilityRight;
        }
#endif
        mySpeedGainProbabilityRight += myVehicle.getActionStepLengthSecs() * maxGainRight;
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << "  speedGainR_new=" << mySpeedGainProbabilityRight << "\n";
        }
#endif
    }
    if (maxGainLeft != -std::numeric_limits<double>::max()) {
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << "  speedGainL_old=" << mySpeedGainProbabilityLeft;
        }
#endif
        mySpeedGainProbabilityLeft += myVehicle.getActionStepLengthSecs() * maxGainLeft;
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << "  speedGainL_new=" << mySpeedGainProbabilityLeft << "\n";
        }
#endif
    }
    // decay if there is no reason for or against changing (only if we have enough information)
    if ((fabs(maxGainRight) < NUMERICAL_EPS || maxGainRight == -std::numeric_limits<double>::max())
            && (right || (alternatives & LCA_RIGHT) == 0)) {
        mySpeedGainProbabilityRight *= pow(SPEEDGAIN_DECAY_FACTOR, myVehicle.getActionStepLengthSecs());
    }
    if ((fabs(maxGainLeft) < NUMERICAL_EPS || maxGainLeft == -std::numeric_limits<double>::max())
            && (left || (alternatives & LCA_LEFT) == 0)) {
        mySpeedGainProbabilityLeft *= pow(SPEEDGAIN_DECAY_FACTOR, myVehicle.getActionStepLengthSecs());
    }


#ifdef DEBUG_WANTSCHANGE
    if (gDebugFlag2) std::cout << SIMTIME
                                   << " veh=" << myVehicle.getID()
                                   << " defaultNextSpeed=" << defaultNextSpeed
                                   << " maxGain=" << maxGain
                                   << " maxGainRight=" << maxGainRight
                                   << " maxGainLeft=" << maxGainLeft
                                   << " latDist=" << latDist
                                   << " latDistNice=" << latDistNice
                                   << " sublaneCompact=" << sublaneCompact
                                   << "\n";
#endif

    if (!left) {
        // ONLY FOR CHANGING TO THE RIGHT
        // start keepRight maneuver when no speed loss is expected and continue
        // started maneuvers if the loss isn't too big
        if (right && myVehicle.getSpeed() > 0 && (maxGainRight >= 0
                || ((myPreviousState & LCA_KEEPRIGHT) != 0 && maxGainRight >= -myKeepRightParam))) {
            // honor the obligation to keep right (Rechtsfahrgebot)
            // XXX consider fast approaching followers on the current lane
            //const double vMax = myLookAheadSpeed;
            const double vMax = myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle);
            const double acceptanceTime = KEEP_RIGHT_ACCEPTANCE * vMax * MAX2(1., myVehicle.getSpeed()) / myVehicle.getLane()->getSpeedLimit();
            double fullSpeedGap = MAX2(0., neighDist - myVehicle.getCarFollowModel().brakeGap(vMax));
            double fullSpeedDrivingSeconds = MIN2(acceptanceTime, fullSpeedGap / vMax);
            CLeaderDist neighLead = getSlowest(neighLeaders);
            if (neighLead.first != 0 && neighLead.first->getSpeed() < vMax) {
                fullSpeedGap = MAX2(0., MIN2(fullSpeedGap,
                                             neighLead.second - myVehicle.getCarFollowModel().getSecureGap(&myVehicle, neighLead.first,
                                                     vMax, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel())));
                fullSpeedDrivingSeconds = MIN2(fullSpeedDrivingSeconds, fullSpeedGap / (vMax - neighLead.first->getSpeed()));
            }
            const double deltaProb = (myChangeProbThresholdRight * (fullSpeedDrivingSeconds / acceptanceTime) / KEEP_RIGHT_TIME);
            myKeepRightProbability += myVehicle.getActionStepLengthSecs() * deltaProb;

#ifdef DEBUG_WANTSCHANGE
            if (gDebugFlag2) {
                std::cout << STEPS2TIME(currentTime)
                          << " considering keepRight:"
                          << " vMax=" << vMax
                          << " neighDist=" << neighDist
                          << " brakeGap=" << myVehicle.getCarFollowModel().brakeGap(myVehicle.getSpeed())
                          << " leaderSpeed=" << (neighLead.first == 0 ? -1 : neighLead.first->getSpeed())
                          << " secGap=" << (neighLead.first == 0 ? -1 : myVehicle.getCarFollowModel().getSecureGap(&myVehicle, neighLead.first,
                                            myVehicle.getSpeed(), neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel()))
                          << " acceptanceTime=" << acceptanceTime
                          << " fullSpeedGap=" << fullSpeedGap
                          << " fullSpeedDrivingSeconds=" << fullSpeedDrivingSeconds
                          << " dProb=" << deltaProb
                          << " keepRight=" << myKeepRightProbability
                          << " speedGainL=" << mySpeedGainProbabilityLeft
                          << "\n";
            }
#endif
            if (myKeepRightProbability * myKeepRightParam > MAX2(myChangeProbThresholdRight, mySpeedGainProbabilityLeft)
                    /*&& latLaneDist <= -NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()*/) {
                ret |= LCA_KEEPRIGHT;
                assert(myVehicle.getLane()->getIndex() > neighLane.getIndex());
                if (!cancelRequest(ret, laneOffset)) {
                    latDist = latLaneDist;
                    maneuverDist = latLaneDist;
                    blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                            leaders, followers, blockers,
                                            neighLeaders, neighFollowers, neighBlockers);
                    return ret;
                }
            }
        }

#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << STEPS2TIME(currentTime)
                      << " speedGainR=" << mySpeedGainProbabilityRight
                      << " speedGainL=" << mySpeedGainProbabilityLeft
                      << " neighDist=" << neighDist
                      << " neighTime=" << neighDist / MAX2(.1, myVehicle.getSpeed())
                      << " rThresh=" << myChangeProbThresholdRight
                      << " latDist=" << latDist
                      << "\n";
        }
#endif

        if (latDist < 0 && mySpeedGainProbabilityRight >= MAX2(myChangeProbThresholdRight, mySpeedGainProbabilityLeft)
                && neighDist / MAX2(.1, myVehicle.getSpeed()) > 20.) {
            ret |= LCA_SPEEDGAIN;
            if (!cancelRequest(ret, laneOffset)) {
                int blockedFully = 0;
                maneuverDist = latDist;
                blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                        leaders, followers, blockers,
                                        neighLeaders, neighFollowers, neighBlockers,
                                        nullptr, nullptr, false, 0, &blockedFully);
                //commitManoeuvre(blocked, blockedFully, leaders, neighLeaders, neighLane);
                return ret;
            }
        }
    }
    if (!right) {

        const bool stayInLane = myVehicle.getLateralPositionOnLane() + latDist < 0.5 * myVehicle.getLane()->getWidth();
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) {
            std::cout << STEPS2TIME(currentTime)
                      << " speedGainL=" << mySpeedGainProbabilityLeft
                      << " speedGainR=" << mySpeedGainProbabilityRight
                      << " latDist=" << latDist
                      << " neighDist=" << neighDist
                      << " neighTime=" << neighDist / MAX2(.1, myVehicle.getSpeed())
                      << " lThresh=" << myChangeProbThresholdLeft
                      << " stayInLane=" << stayInLane
                      << "\n";
        }
#endif

        if (latDist > 0 && mySpeedGainProbabilityLeft > myChangeProbThresholdLeft &&
                // if we leave our lane, we should be able to stay in the new
                // lane for some time
                (stayInLane || neighDist / MAX2(.1, myVehicle.getSpeed()) > SPEED_GAIN_MIN_SECONDS)) {
            ret |= LCA_SPEEDGAIN;
            if (!cancelRequest(ret, laneOffset)) {
                int blockedFully = 0;
                maneuverDist = latDist;
                blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                        leaders, followers, blockers,
                                        neighLeaders, neighFollowers, neighBlockers,
                                        nullptr, nullptr, false, 0, &blockedFully);
                //commitManoeuvre(blocked, blockedFully, leaders, neighLeaders, neighLane);
                return ret;
            }
        }
    }

    double latDistSublane = 0.;
    const double halfLaneWidth = myVehicle.getLane()->getWidth() * 0.5;
    const double halfVehWidth = getWidth() * 0.5;
    if (myVehicle.getParameter().arrivalPosLatProcedure != ArrivalPosLatDefinition::DEFAULT
            && myVehicle.getRoute().getLastEdge() == &myVehicle.getLane()->getEdge()
            && bestLaneOffset == 0
            && (myVehicle.getArrivalPos() - myVehicle.getPositionOnLane()) < ARRIVALPOS_LAT_THRESHOLD) {
        // vehicle is on its final edge, on the correct lane and close to
        // its arrival position. Change to the desired lateral position
        switch (myVehicle.getParameter().arrivalPosLatProcedure) {
            case ArrivalPosLatDefinition::GIVEN:
                latDistSublane = myVehicle.getParameter().arrivalPosLat - myVehicle.getLateralPositionOnLane();
                break;
            case ArrivalPosLatDefinition::RIGHT:
                latDistSublane = -halfLaneWidth + halfVehWidth - myVehicle.getLateralPositionOnLane();
                break;
            case ArrivalPosLatDefinition::CENTER:
                latDistSublane = -myVehicle.getLateralPositionOnLane();
                break;
            case ArrivalPosLatDefinition::LEFT:
                latDistSublane = halfLaneWidth - halfVehWidth - myVehicle.getLateralPositionOnLane();
                break;
            default:
                assert(false);
        }
#ifdef DEBUG_WANTSCHANGE
        if (gDebugFlag2) std::cout << SIMTIME
                                       << " arrivalPosLatProcedure=" << (int)myVehicle.getParameter().arrivalPosLatProcedure
                                       << " arrivalPosLat=" << myVehicle.getParameter().arrivalPosLat << "\n";
#endif

    } else {

        LateralAlignment align = myVehicle.getVehicleType().getPreferredLateralAlignment();
        // Check whether the vehicle should adapt its alignment to an upcoming turn
        if (myTurnAlignmentDist > 0) {
            const std::pair<double, LinkDirection>& turnInfo = myVehicle.getNextTurn();
            if (turnInfo.first < myTurnAlignmentDist) {
                // Vehicle is close enough to the link to change its default alignment
                switch (turnInfo.second) {
                    case LinkDirection::TURN:
                    case LinkDirection::LEFT:
                    case LinkDirection::PARTLEFT:
                        align = MSGlobals::gLefthand ? LATALIGN_RIGHT : LATALIGN_LEFT;
                        break;
                    case LinkDirection::TURN_LEFTHAND:
                    case LinkDirection::RIGHT:
                    case LinkDirection::PARTRIGHT:
                        align = MSGlobals::gLefthand ? LATALIGN_LEFT : LATALIGN_RIGHT;
                        break;
                    case LinkDirection::STRAIGHT:
                    case LinkDirection::NODIR:
                    default:
                        break;
                }
            }
        }
        switch (align) {
            case LATALIGN_RIGHT:
                latDistSublane = -halfLaneWidth + halfVehWidth - getPosLat();
                break;
            case LATALIGN_LEFT:
                latDistSublane = halfLaneWidth - halfVehWidth - getPosLat();
                break;
            case LATALIGN_CENTER:
                latDistSublane = -getPosLat();
                break;
            case LATALIGN_NICE:
                latDistSublane = latDistNice;
                break;
            case LATALIGN_COMPACT:
                latDistSublane = sublaneSides[sublaneCompact] - rightVehSide;
                break;
            case LATALIGN_ARBITRARY:
                latDistSublane = getLateralDrift();
                break;
        }
    }
    // only factor in preferred lateral alignment if there is no speedGain motivation or it runs in the same direction
    if (fabs(latDist) <= NUMERICAL_EPS * myVehicle.getActionStepLengthSecs() ||
            latDistSublane * latDist > 0) {

#if defined(DEBUG_WANTSCHANGE) || defined(DEBUG_STATE) || defined(DEBUG_MANEUVER)
        if (gDebugFlag2) std::cout << SIMTIME
                                       << " alignment=" << toString(myVehicle.getVehicleType().getPreferredLateralAlignment())
                                       << " mySpeedGainR=" << mySpeedGainProbabilityRight
                                       << " mySpeedGainL=" << mySpeedGainProbabilityLeft
                                       << " latDist=" << latDist
                                       << " latDistSublane=" << latDistSublane
                                       << " relGainSublane=" << computeSpeedGain(latDistSublane, defaultNextSpeed)
                                       << " maneuverDist=" << maneuverDist
                                       << " myCanChangeFully=" << myCanChangeFully
                                       << " myTurnAlignmentDist=" << myTurnAlignmentDist
                                       << " nextTurn=" << myVehicle.getNextTurn().first << ":" << toString(myVehicle.getNextTurn().second)
                                       << " prevState=" << toString((LaneChangeAction)myPreviousState)
                                       << "\n";
#endif

        if ((latDistSublane < 0 && mySpeedGainProbabilityRight < mySpeedLossProbThreshold)
                || (latDistSublane > 0 && mySpeedGainProbabilityLeft < mySpeedLossProbThreshold)
                || computeSpeedGain(latDistSublane, defaultNextSpeed) < -mySublaneParam) {
            // do not risk losing speed
#if defined(DEBUG_WANTSCHANGE)
            if (gDebugFlag2) std::cout << "   aborting sublane change to avoid speed loss (mySpeedLossProbThreshold=" << mySpeedLossProbThreshold
                                           << " speedGain=" << computeSpeedGain(latDistSublane, defaultNextSpeed) << ")\n";
#endif
            latDistSublane = 0;
        }
        // Ignore preferred lateral alignment if we are in the middle of an unfinished non-alignment maneuver into the opposite direction
        if (!myCanChangeFully
                && (myPreviousState & (LCA_STRATEGIC | LCA_COOPERATIVE | LCA_KEEPRIGHT | LCA_SPEEDGAIN)) != 0
                && ((getManeuverDist() < 0 && latDistSublane > 0) || (getManeuverDist() > 0 && latDistSublane < 0))) {
#if defined(DEBUG_WANTSCHANGE)
            if (gDebugFlag2) {
                std::cout << "   aborting sublane change due to prior maneuver\n";
            }
#endif
            latDistSublane = 0;
        }
        latDist = latDistSublane;
        // XXX first compute preferred adaptation and then override with speed
        // (this way adaptation is still done if changing for speedgain is
        // blocked)
        if (fabs(latDist) >= NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) {
#ifdef DEBUG_WANTSCHANGE
            if (gDebugFlag2) std::cout << SIMTIME
                                           << " adapting to preferred alignment=" << toString(myVehicle.getVehicleType().getPreferredLateralAlignment())
                                           << " latDist=" << latDist
                                           << "\n";
#endif
            ret |= LCA_SUBLANE;
            // include prior motivation when sublane-change is part of finishing an ongoing maneuver in the same direction
            if (getPreviousManeuverDist() * latDist > 0) {
                int priorReason = (myPreviousState & LCA_CHANGE_REASONS & ~LCA_SUBLANE);
                ret |= priorReason;
#ifdef DEBUG_WANTSCHANGE
                if (gDebugFlag2 && priorReason != 0) std::cout << "   including prior reason " << toString((LaneChangeAction)priorReason)
                            << " prevManeuverDist=" << getPreviousManeuverDist() << "\n";
#endif
            }
            if (!cancelRequest(ret, laneOffset)) {
                maneuverDist = latDist;
                blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset,
                                        leaders, followers, blockers,
                                        neighLeaders, neighFollowers, neighBlockers);
                return ret;
            }
        } else {
            return ret | LCA_SUBLANE | LCA_STAY;
        }
    }
    latDist = 0;


    // --------
    /*
    if (changeToBest && bestLaneOffset == curr.bestLaneOffset && laneOffset != 0
            && (right
                ? mySpeedGainProbabilityRight > MAX2(0., mySpeedGainProbabilityLeft)
                : mySpeedGainProbabilityLeft  > MAX2(0., mySpeedGainProbabilityRight))) {
        // change towards the correct lane, speedwise it does not hurt
        ret |= LCA_STRATEGIC;
        if (!cancelRequest(ret, laneOffset)) {
            latDist = latLaneDist;
            blocked = checkBlocking(neighLane, latDist, laneOffset,
                    leaders, followers, blockers,
                    neighLeaders, neighFollowers, neighBlockers);
            return ret;
        }
    }
    */
#ifdef DEBUG_WANTSCHANGE
    if (gDebugFlag2) {
        std::cout << STEPS2TIME(currentTime)
                  << " veh=" << myVehicle.getID()
                  << " mySpeedGainR=" << mySpeedGainProbabilityRight
                  << " mySpeedGainL=" << mySpeedGainProbabilityLeft
                  << " myKeepRight=" << myKeepRightProbability
                  << "\n";
    }
#endif
    return ret;
}


int
MSLCM_SL2015::slowDownForBlocked(MSVehicle** blocked, int state) {
    //  if this vehicle is blocking someone in front, we maybe decelerate to let him in
    if ((*blocked) != nullptr) {
        double gap = (*blocked)->getPositionOnLane() - (*blocked)->getVehicleType().getLength() - myVehicle.getPositionOnLane() - myVehicle.getVehicleType().getMinGap();
#ifdef DEBUG_SLOWDOWN
        if (gDebugFlag2) {
            std::cout << SIMTIME
                      << " veh=" << myVehicle.getID()
                      << " blocked=" << Named::getIDSecure(*blocked)
                      << " gap=" << gap
                      << "\n";
        }
#endif
        if (gap > POSITION_EPS) {
            //const bool blockedWantsUrgentRight = (((*blocked)->getLaneChangeModel().getOwnState() & LCA_RIGHT != 0)
            //    && ((*blocked)->getLaneChangeModel().getOwnState() & LCA_URGENT != 0));

            if (myVehicle.getSpeed() < ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())
                    //|| blockedWantsUrgentRight  // VARIANT_10 (helpblockedRight)
               ) {
                if ((*blocked)->getSpeed() < SUMO_const_haltingSpeed) {
                    state |= LCA_AMBACKBLOCKER_STANDING;
                } else {
                    state |= LCA_AMBACKBLOCKER;
                }
                addLCSpeedAdvice(myCarFollowModel.followSpeed(
                                     &myVehicle, myVehicle.getSpeed(),
                                     (gap - POSITION_EPS), (*blocked)->getSpeed(),
                                     (*blocked)->getCarFollowModel().getMaxDecel()));
                //(*blocked) = 0; // VARIANT_14 (furtherBlock)
            }
        }
    }
    return state;
}


void
MSLCM_SL2015::saveBlockerLength(const MSVehicle* blocker, int lcaCounter) {
#ifdef DEBUG_SAVE_BLOCKER_LENGTH
    if (gDebugFlag2) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " saveBlockerLength blocker=" << Named::getIDSecure(blocker)
                  << " bState=" << (blocker == 0 ? "None" : toString((LaneChangeAction)blocker->getLaneChangeModel().getOwnState()))
                  << "\n";
    }
#endif
    if (blocker != nullptr && (blocker->getLaneChangeModel().getOwnState() & lcaCounter) != 0) {
        // is there enough space in front of us for the blocker?
        const double potential = myLeftSpace - myVehicle.getCarFollowModel().brakeGap(
                                     myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel(), 0);
        if (blocker->getVehicleType().getLengthWithGap() <= potential) {
            // save at least his length in myLeadingBlockerLength
            myLeadingBlockerLength = MAX2(blocker->getVehicleType().getLengthWithGap(), myLeadingBlockerLength);
#ifdef DEBUG_SAVE_BLOCKER_LENGTH
            if (gDebugFlag2) {
                std::cout << "    saving myLeadingBlockerLength=" << myLeadingBlockerLength << "\n";
            }
#endif
        } else {
            // we cannot save enough space for the blocker. It needs to save
            // space for ego instead
#ifdef DEBUG_SAVE_BLOCKER_LENGTH
            if (gDebugFlag2) {
                std::cout << "    cannot save space=" << blocker->getVehicleType().getLengthWithGap() << " potential=" << potential << " (blocker must save)\n";
            }
#endif
            ((MSVehicle*)blocker)->getLaneChangeModel().saveBlockerLength(myVehicle.getVehicleType().getLengthWithGap());
        }
    }
}


void MSLCM_SL2015::addLCSpeedAdvice(const double vSafe) {
    const double accel = SPEED2ACCEL(vSafe - myVehicle.getSpeed());
    myLCAccelerationAdvices.push_back(accel);
#ifdef DEBUG_INFORM
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " accepted LC speed advice "
                  << "vSafe=" << vSafe << " -> accel=" << accel <<  "\n";
    }
#endif
}


void
MSLCM_SL2015::updateExpectedSublaneSpeeds(const MSLeaderDistanceInfo& ahead, int sublaneOffset, int laneIndex) {
    const std::vector<MSLane*>& lanes = myVehicle.getLane()->getEdge().getLanes();
    const std::vector<MSVehicle::LaneQ>& preb = myVehicle.getBestLanes();
    const MSLane* lane = lanes[laneIndex];
    const double vMax = lane->getVehicleMaxSpeed(&myVehicle);
    assert(preb.size() == lanes.size());

    for (int sublane = 0; sublane < (int)ahead.numSublanes(); ++sublane) {
        const int edgeSublane = sublane + sublaneOffset;
        if (edgeSublane >= (int)myExpectedSublaneSpeeds.size()) {
            // this may happen if a sibling lane is wider than the changer lane
            continue;
        }
        if (lane->allowsVehicleClass(myVehicle.getVehicleType().getVehicleClass())) {
            // lane allowed, find potential leaders and compute safe speeds
            // XXX anticipate future braking if leader has a lower speed than myVehicle
            const MSVehicle* leader = ahead[sublane].first;
            const double gap = ahead[sublane].second;
            double vSafe;
            if (leader == nullptr) {
                const double dist = preb[laneIndex].length - myVehicle.getPositionOnLane();
                vSafe = myCarFollowModel.followSpeed(&myVehicle, vMax, dist, 0, 0);
            } else {
                if (leader->getAcceleration() > 0.5 * leader->getCarFollowModel().getMaxAccel()) {
                    // assume that the leader will continue accelerating to its maximum speed
                    vSafe = leader->getLane()->getVehicleMaxSpeed(leader);
                } else {
                    vSafe = myCarFollowModel.followSpeed(
                                &myVehicle, vMax, gap, leader->getSpeed(), leader->getCarFollowModel().getMaxDecel());
#ifdef DEBUG_EXPECTED_SLSPEED
                    if (DEBUG_COND) {
                        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " updateExpectedSublaneSpeeds edgeSublane=" << edgeSublane << " leader=" << leader->getID() << " gap=" << gap << " vSafe=" << vSafe << "\n";
                    }
#endif
                    const double deltaV = vMax - leader->getSpeed();
                    if (deltaV > 0 && gap / deltaV < mySpeedGainLookahead) {
                        // anticipate future braking by computing the average
                        // speed over the next few seconds
                        const double foreCastTime = mySpeedGainLookahead * 2;
                        const double gapClosingTime = gap / deltaV;
                        vSafe = (gapClosingTime * vSafe + (foreCastTime - gapClosingTime) * leader->getSpeed()) / foreCastTime;
                    }
                }
            }
            // take pedestrians into account
            if (lane->getEdge().getPersons().size() > 0 && lane->hasPedestrians()) {
                /// XXX this could be done faster by checking all sublanes at once (but would complicate the MSPModel API)
                double foeRight, foeLeft;
                ahead.getSublaneBorders(sublane, 0, foeRight, foeLeft);
                // get all leaders ahead or overlapping
                PersonDist leader = lane->nextBlocking(myVehicle.getPositionOnLane() - myVehicle.getVehicleType().getLength(), foeRight, foeLeft);
                if (leader.first != 0) {
                    const double gap = leader.second - myVehicle.getVehicleType().getMinGap() - myVehicle.getVehicleType().getLength();
                    const double vSafePed = myCarFollowModel.stopSpeed(&myVehicle, vMax, gap);
                    vSafe = MIN2(vSafe, vSafePed);
                }
            }
            vSafe = MIN2(vMax, vSafe);
            const double memoryFactor = pow(SPEEDGAIN_MEMORY_FACTOR, myVehicle.getActionStepLengthSecs());
            myExpectedSublaneSpeeds[edgeSublane] = memoryFactor * myExpectedSublaneSpeeds[edgeSublane] + (1 - memoryFactor) * vSafe;
        } else {
            // lane forbidden
            myExpectedSublaneSpeeds[edgeSublane] = -1;
        }
    }
    // XXX deal with leaders on subsequent lanes based on preb
}


double
MSLCM_SL2015::computeSpeedGain(double latDistSublane, double defaultNextSpeed) const {
    double result = std::numeric_limits<double>::max();
    const std::vector<double>& sublaneSides = myVehicle.getLane()->getEdge().getSubLaneSides();
    const double vehWidth = getWidth();
    const double rightVehSide = myVehicle.getCenterOnEdge() - vehWidth * 0.5 + latDistSublane;
    const double leftVehSide = rightVehSide + vehWidth;
    for (int i = 0; i < (int)sublaneSides.size(); ++i) {
        const double leftSide = i + 1 < (int)sublaneSides.size() ? sublaneSides[i + 1] : myVehicle.getLane()->getEdge().getWidth();
        if (overlap(rightVehSide, leftVehSide, sublaneSides[i], leftSide)) {
            result = MIN2(result, myExpectedSublaneSpeeds[i]);
        }
        //std::cout << "    i=" << i << " rightVehSide=" << rightVehSide << " leftVehSide=" << leftVehSide << " sublaneR=" << sublaneSides[i] << " sublaneL=" << leftSide << " overlap=" << overlap(rightVehSide, leftVehSide, sublaneSides[i], leftSide) << " speed=" << myExpectedSublaneSpeeds[i] << " result=" << result << "\n";
    }
    return result - defaultNextSpeed;
}


CLeaderDist
MSLCM_SL2015::getLongest(const MSLeaderDistanceInfo& ldi) {
    int iMax = 0;
    double maxLength = -1;
    for (int i = 0; i < ldi.numSublanes(); ++i) {
        if (ldi[i].first != 0) {
            const double length = ldi[i].first->getVehicleType().getLength();
            if (length > maxLength) {
                maxLength = length;
                iMax = i;
            }
        }
    }
    return ldi[iMax];
}


CLeaderDist
MSLCM_SL2015::getSlowest(const MSLeaderDistanceInfo& ldi) {
    int iMax = 0;
    double minSpeed = std::numeric_limits<double>::max();
    for (int i = 0; i < ldi.numSublanes(); ++i) {
        if (ldi[i].first != 0) {
            const double speed = ldi[i].first->getSpeed();
            if (speed < minSpeed) {
                minSpeed = speed;
                iMax = i;
            }
        }
    }
    return ldi[iMax];
}


int
MSLCM_SL2015::checkBlocking(const MSLane& neighLane, double& latDist, double maneuverDist, int laneOffset,
                            const MSLeaderDistanceInfo& leaders,
                            const MSLeaderDistanceInfo& followers,
                            const MSLeaderDistanceInfo& /*blockers */,
                            const MSLeaderDistanceInfo& neighLeaders,
                            const MSLeaderDistanceInfo& neighFollowers,
                            const MSLeaderDistanceInfo& /* neighBlockers */,
                            std::vector<CLeaderDist>* collectLeadBlockers,
                            std::vector<CLeaderDist>* collectFollowBlockers,
                            bool keepLatGapManeuver,
                            double gapFactor,
                            int* retBlockedFully) {
    // truncate latDist according to maxSpeedLat
    const double maxDist = SPEED2DIST(myVehicle.getVehicleType().getMaxSpeedLat());
    latDist = MAX2(MIN2(latDist, maxDist), -maxDist);
    if (myVehicle.hasInfluencer() && myVehicle.getInfluencer().getLatDist() != 0 && myVehicle.getInfluencer().ignoreOverlap()) {
        return 0;
    }

    if (!myCFRelatedReady) {
        updateCFRelated(leaders, myVehicle.getLane()->getRightSideOnEdge(), true);
        updateCFRelated(followers, myVehicle.getLane()->getRightSideOnEdge(), false);
        if (laneOffset != 0) {
            updateCFRelated(neighLeaders, neighLane.getRightSideOnEdge(), true);
            updateCFRelated(neighFollowers, neighLane.getRightSideOnEdge(), false);
        }
        myCFRelatedReady = true;
    }

    // reduce latDist to avoid blockage with overlapping vehicles (no minGapLat constraints)
    const double center = myVehicle.getCenterOnEdge();
    updateGaps(leaders, myVehicle.getLane()->getRightSideOnEdge(), center, gapFactor, mySafeLatDistRight, mySafeLatDistLeft, false, 0, latDist, collectLeadBlockers);
    updateGaps(followers, myVehicle.getLane()->getRightSideOnEdge(), center, gapFactor, mySafeLatDistRight, mySafeLatDistLeft, false, 0, latDist, collectFollowBlockers);
    if (laneOffset != 0) {
        updateGaps(neighLeaders, neighLane.getRightSideOnEdge(), center, gapFactor, mySafeLatDistRight, mySafeLatDistLeft, false, 0, latDist, collectLeadBlockers);
        updateGaps(neighFollowers, neighLane.getRightSideOnEdge(), center, gapFactor, mySafeLatDistRight, mySafeLatDistLeft, false, 0, latDist, collectFollowBlockers);
    }
#ifdef DEBUG_BLOCKING
    if (gDebugFlag2) {
        std::cout << "    checkBlocking latDist=" << latDist << " mySafeLatDistRight=" << mySafeLatDistRight << " mySafeLatDistLeft=" << mySafeLatDistLeft << "\n";
    }
#endif
    // if we can move at least a little bit in the desired direction, do so (rather than block)
    const bool forcedTraCIChange = (myVehicle.hasInfluencer()
                                    && myVehicle.getInfluencer().getLatDist() != 0
                                    && myVehicle.getInfluencer().ignoreOverlap());
    if (latDist < 0) {
        if (mySafeLatDistRight <= NUMERICAL_EPS) {
            return LCA_BLOCKED_RIGHT | LCA_OVERLAPPING;
        } else if (!forcedTraCIChange) {
            latDist = MAX2(latDist, -mySafeLatDistRight);
        }
    } else {
        if (mySafeLatDistLeft <= NUMERICAL_EPS) {
            return LCA_BLOCKED_LEFT | LCA_OVERLAPPING;
        } else if (!forcedTraCIChange) {
            latDist = MIN2(latDist, mySafeLatDistLeft);
        }
    }

    myCanChangeFully = (maneuverDist == 0 || latDist == maneuverDist);
#ifdef DEBUG_BLOCKING
    if (gDebugFlag2) {
        std::cout << "    checkBlocking fully=" << myCanChangeFully << " latDist=" << latDist << " maneuverDist=" << maneuverDist << "\n";
    }
#endif
    // destination sublanes must be safe
    // intermediate sublanes must not be blocked by overlapping vehicles

    // XXX avoid checking the same leader multiple times
    // XXX ensure that only changes within the same lane are undertaken if laneOffset = 0

    int blocked = 0;
    blocked |= checkBlockingVehicles(&myVehicle, leaders, latDist, myVehicle.getLane()->getRightSideOnEdge(), true, LCA_BLOCKED_BY_LEADER,
                                     mySafeLatDistRight, mySafeLatDistLeft, collectLeadBlockers);
    blocked |= checkBlockingVehicles(&myVehicle, followers, latDist, myVehicle.getLane()->getRightSideOnEdge(), false, LCA_BLOCKED_BY_FOLLOWER,
                                     mySafeLatDistRight, mySafeLatDistLeft, collectFollowBlockers);
    if (laneOffset != 0) {
        blocked |= checkBlockingVehicles(&myVehicle, neighLeaders, latDist, neighLane.getRightSideOnEdge(), true,
                                         (laneOffset == -1 ? LCA_BLOCKED_BY_RIGHT_LEADER : LCA_BLOCKED_BY_LEFT_LEADER),
                                         mySafeLatDistRight, mySafeLatDistLeft, collectLeadBlockers);
        blocked |= checkBlockingVehicles(&myVehicle, neighFollowers, latDist, neighLane.getRightSideOnEdge(), false,
                                         (laneOffset == -1 ? LCA_BLOCKED_BY_RIGHT_FOLLOWER : LCA_BLOCKED_BY_LEFT_FOLLOWER),
                                         mySafeLatDistRight, mySafeLatDistLeft, collectFollowBlockers);
    }

    int blockedFully = 0;
    blockedFully |= checkBlockingVehicles(&myVehicle, leaders, maneuverDist, myVehicle.getLane()->getRightSideOnEdge(), true, LCA_BLOCKED_BY_LEADER,
                                          mySafeLatDistRight, mySafeLatDistLeft, collectLeadBlockers);
    blockedFully |= checkBlockingVehicles(&myVehicle, followers, maneuverDist, myVehicle.getLane()->getRightSideOnEdge(), false, LCA_BLOCKED_BY_FOLLOWER,
                                          mySafeLatDistRight, mySafeLatDistLeft, collectFollowBlockers);
    if (laneOffset != 0) {
        blockedFully |= checkBlockingVehicles(&myVehicle, neighLeaders, maneuverDist, neighLane.getRightSideOnEdge(), true,
                                              (laneOffset == -1 ? LCA_BLOCKED_BY_RIGHT_LEADER : LCA_BLOCKED_BY_LEFT_LEADER),
                                              mySafeLatDistRight, mySafeLatDistLeft, collectLeadBlockers);
        blockedFully |= checkBlockingVehicles(&myVehicle, neighFollowers, maneuverDist, neighLane.getRightSideOnEdge(), false,
                                              (laneOffset == -1 ? LCA_BLOCKED_BY_RIGHT_FOLLOWER : LCA_BLOCKED_BY_LEFT_FOLLOWER),
                                              mySafeLatDistRight, mySafeLatDistLeft, collectFollowBlockers);
    }
    if (retBlockedFully != nullptr) {
        *retBlockedFully = blockedFully;
    }
    if (blocked == 0 && !myCanChangeFully && myPushy == 0 && !keepLatGapManeuver) {
        // aggressive drivers immediately start moving towards potential
        // blockers and only check that the start of their maneuver (latDist) is safe. In
        // contrast, cautious drivers need to check latDist and origLatDist to
        // ensure that the maneuver can be finished without encroaching on other vehicles.
        blocked |= blockedFully;
    } else {
        // XXX: in case of action step length > simulation step length, pushing may lead to collisions,
        //      because maneuver is continued until maneuverDist is reached (perhaps set maneuverDist=latDist)
    }
    if (collectFollowBlockers != nullptr && collectLeadBlockers != nullptr) {
        // prevent vehicles from being classified as leader and follower simultaneously
        for (std::vector<CLeaderDist>::const_iterator it2 = collectLeadBlockers->begin(); it2 != collectLeadBlockers->end(); ++it2) {
            for (std::vector<CLeaderDist>::iterator it = collectFollowBlockers->begin(); it != collectFollowBlockers->end();) {
                if ((*it2).first == (*it).first) {
#ifdef DEBUG_BLOCKING
                    if (gDebugFlag2) {
                        std::cout << "    removed follower " << (*it).first->getID() << " because it is already a leader\n";
                    }
#endif
                    it = collectFollowBlockers->erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    return blocked;
}


int
MSLCM_SL2015::checkBlockingVehicles(
    const MSVehicle* ego, const MSLeaderDistanceInfo& vehicles,
    double latDist, double foeOffset, bool leaders, LaneChangeAction blockType,
    double& safeLatGapRight, double& safeLatGapLeft,
    std::vector<CLeaderDist>* collectBlockers) const {
    // determine borders where safety/no-overlap conditions must hold
    const double vehWidth = getWidth();
    const double rightVehSide = ego->getRightSideOnEdge();
    const double leftVehSide = rightVehSide + vehWidth;
    const double rightVehSideDest = rightVehSide + latDist;
    const double leftVehSideDest = leftVehSide + latDist;
    const double rightNoOverlap = MIN2(rightVehSideDest, rightVehSide);
    const double leftNoOverlap = MAX2(leftVehSideDest, leftVehSide);
#ifdef DEBUG_BLOCKING
    if (gDebugFlag2) {
        std::cout << "  checkBlockingVehicles"
                  << " latDist=" << latDist
                  << " foeOffset=" << foeOffset
                  << " vehRight=" << rightVehSide
                  << " vehLeft=" << leftVehSide
                  << " rightNoOverlap=" << rightNoOverlap
                  << " leftNoOverlap=" << leftNoOverlap
                  << " destRight=" << rightVehSideDest
                  << " destLeft=" << leftVehSideDest
                  << " leaders=" << leaders
                  << " blockType=" << toString((LaneChangeAction) blockType)
                  << "\n";
    }
#endif
    int result = 0;
    for (int i = 0; i < vehicles.numSublanes(); ++i) {
        CLeaderDist vehDist = vehicles[i];
        if (vehDist.first != 0 && myCFRelated.count(vehDist.first) == 0) {
            const MSVehicle* leader = vehDist.first;
            const MSVehicle* follower = ego;
            if (!leaders) {
                std::swap(leader, follower);
            }
            // only check the current stripe occupied by foe (transform into edge-coordinates)
            double foeRight, foeLeft;
            vehicles.getSublaneBorders(i, foeOffset, foeRight, foeLeft);
            const bool overlapBefore = overlap(rightVehSide, leftVehSide, foeRight, foeLeft);
            const bool overlapDest = overlap(rightVehSideDest, leftVehSideDest, foeRight, foeLeft);
            const bool overlapAny = overlap(rightNoOverlap, leftNoOverlap, foeRight, foeLeft);
#ifdef DEBUG_BLOCKING
            if (gDebugFlag2) {
                std::cout << "   foe=" << vehDist.first->getID()
                          << " gap=" << vehDist.second
                          << " secGap=" << follower->getCarFollowModel().getSecureGap(follower, leader, follower->getSpeed(), leader->getSpeed(), leader->getCarFollowModel().getMaxDecel())
                          << " foeRight=" << foeRight
                          << " foeLeft=" << foeLeft
                          << " overlapBefore=" << overlapBefore
                          << " overlap=" << overlapAny
                          << " overlapDest=" << overlapDest
                          << "\n";
            }
#endif
            if (overlapAny) {
                if (vehDist.second < 0) {
                    if (overlapBefore && !overlapDest) {
#ifdef DEBUG_BLOCKING
                        if (gDebugFlag2) {
                            std::cout << "    ignoring current overlap to come clear\n";
                        }
#endif
                    } else {
#ifdef DEBUG_BLOCKING
                        if (gDebugFlag2) {
                            std::cout << "    overlap (" << toString((LaneChangeAction)blockType) << ")\n";
                        }
#endif
                        result |= (blockType | LCA_OVERLAPPING);
                        if (collectBlockers == nullptr) {
                            return result;
                        } else {
                            collectBlockers->push_back(vehDist);
                        }
                    }
                } else if (overlapDest || !myCanChangeFully) {
                    // Estimate state after actionstep (follower may be accelerating!)
                    // A comparison between secure gap depending on the expected speeds and the extrapolated gap
                    // determines whether the s is blocking the lane change.
                    // (Note that the longitudinal state update has already taken effect before LC dynamics (thus "-TS" below), would be affected by #3665)

                    // Use conservative estimate for time until next action step
                    // (XXX: how can the ego know the foe's action step length?)
                    const double timeTillAction = MAX2(follower->getActionStepLengthSecs(), leader->getActionStepLengthSecs()) - TS;
                    // Ignore decel for follower
                    const double followerAccel = MAX2(0., follower->getAcceleration());
                    const double leaderAccel = leader->getAcceleration();
                    // Expected gap after next actionsteps
                    const double expectedGap = MSCFModel::gapExtrapolation(timeTillAction, vehDist.second, leader->getSpeed(), follower->getSpeed(), leaderAccel, followerAccel, std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

                    // Determine expected speeds and corresponding secure gap at the extrapolated timepoint
                    const double followerExpectedSpeed = follower->getSpeed() + timeTillAction * followerAccel;
                    const double leaderExpectedSpeed = MAX2(0., leader->getSpeed() + timeTillAction * leaderAccel);
                    const double expectedSecureGap = follower->getCarFollowModel().getSecureGap(follower, leader, followerExpectedSpeed, leaderExpectedSpeed, leader->getCarFollowModel().getMaxDecel());

#if defined(DEBUG_ACTIONSTEPS) && defined(DEBUG_BLOCKING)
                    if (gDebugFlag2) {
                        std::cout << "    timeTillAction=" << timeTillAction
                                  << " followerAccel=" << followerAccel
                                  << " followerExpectedSpeed=" << followerExpectedSpeed
                                  << " leaderAccel=" << leaderAccel
                                  << " leaderExpectedSpeed=" << leaderExpectedSpeed
                                  << "\n    gap=" << vehDist.second
                                  << " gapChange=" << (expectedGap - vehDist.second)
                                  << " expectedGap=" << expectedGap
                                  << " expectedSecureGap=" << expectedSecureGap
                                  << " safeLatGapLeft=" << safeLatGapLeft
                                  << " safeLatGapRight=" << safeLatGapRight
                                  << std::endl;
                    }
#endif

                    // @note for euler-update, a different value for secureGap2 may be obtained when applying safetyFactor to followerDecel rather than secureGap
                    const double secureGap2 = expectedSecureGap * getSafetyFactor();
                    if (expectedGap < secureGap2) {
                        // Foe is a blocker. Update lateral safe gaps accordingly.
                        if (foeRight > leftVehSide) {
                            safeLatGapLeft = MIN2(safeLatGapLeft, foeRight - leftVehSide);
                        } else if (foeLeft < rightVehSide) {
                            safeLatGapRight = MIN2(safeLatGapRight, rightVehSide - foeLeft);
                        }

#ifdef DEBUG_BLOCKING
                        if (gDebugFlag2) {
                            std::cout << "    blocked by " << vehDist.first->getID() << " gap=" << vehDist.second << " expectedGap=" << expectedGap
                                      << " expectedSecureGap=" << expectedSecureGap << " secGap2=" << secureGap2 << " safetyFactor=" << getSafetyFactor()
                                      << " safeLatGapLeft=" << safeLatGapLeft << " safeLatGapRight=" << safeLatGapRight
                                      << "\n";
                        }
#endif
                        result |= blockType;
                        if (collectBlockers == nullptr) {
                            return result;
                        }
#ifdef DEBUG_BLOCKING
                    } else if (gDebugFlag2 && expectedGap < expectedSecureGap) {
                        std::cout << "    ignore blocker " << vehDist.first->getID() << " gap=" << vehDist.second << " expectedGap=" << expectedGap
                                  << " expectedSecureGap=" << expectedSecureGap << " secGap2=" << secureGap2 << " safetyFactor=" << getSafetyFactor() << "\n";
#endif
                    }
                    if (collectBlockers != nullptr) {
                        // collect non-blocking followers as well to make sure
                        // they remain non-blocking
                        collectBlockers->push_back(vehDist);
                    }
                }
            }
        }
    }
    return result;

}


void
MSLCM_SL2015::updateCFRelated(const MSLeaderDistanceInfo& vehicles, double foeOffset, bool leaders) {
    // to ensure that we do not ignore the wrong vehicles due to numerical
    // instability we slightly reduce the width
    const double vehWidth = myVehicle.getVehicleType().getWidth() - NUMERICAL_EPS;
    const double rightVehSide = myVehicle.getCenterOnEdge() - 0.5 * vehWidth;
    const double leftVehSide = rightVehSide + vehWidth;
#ifdef DEBUG_BLOCKING
    if (gDebugFlag2) {
        std::cout << " updateCFRelated foeOffset=" << foeOffset << " vehicles=" << vehicles.toString() << "\n";
    }
#endif
    for (int i = 0; i < vehicles.numSublanes(); ++i) {
        CLeaderDist vehDist = vehicles[i];
        if (vehDist.first != 0 && myCFRelated.count(vehDist.first) == 0) {
            double foeRight, foeLeft;
            vehicles.getSublaneBorders(i, foeOffset, foeRight, foeLeft);
            if (overlap(rightVehSide, leftVehSide, foeRight, foeLeft) && (vehDist.second >= 0
                    // avoid deadlock due to #3729
                    || (!leaders
                        && myVehicle.getPositionOnLane() >= myVehicle.getVehicleType().getLength()
                        && myVehicle.getSpeed() < SUMO_const_haltingSpeed
                        && vehDist.first->getSpeed() < SUMO_const_haltingSpeed
                        && -vehDist.second < vehDist.first->getVehicleType().getMinGap()
                        && &(myVehicle.getLane()->getEdge()) != &(vehDist.first->getLane()->getEdge()))
                                                                         )) {
#ifdef DEBUG_BLOCKING
                if (gDebugFlag2) {
                    std::cout << " ignoring cfrelated foe=" << vehDist.first->getID() << " gap=" << vehDist.second
                              << " sublane=" << i
                              << " foeOffset=" << foeOffset
                              << " egoR=" << rightVehSide << " egoL=" << leftVehSide
                              << " iR=" << foeRight << " iL=" << foeLeft
                              << " egoV=" << myVehicle.getSpeed() << " foeV=" << vehDist.first->getSpeed()
                              << " egoE=" << myVehicle.getLane()->getEdge().getID() << " egoE=" << vehDist.first->getLane()->getEdge().getID()
                              << "\n";
                }
#endif
                myCFRelated.insert(vehDist.first);
            }
        }
    }
}


bool
MSLCM_SL2015::overlap(double right, double left, double right2, double left2) {
    assert(right <= left);
    assert(right2 <= left2);
    return left2 >= right + NUMERICAL_EPS && left >= right2 + NUMERICAL_EPS;
}


int
MSLCM_SL2015::lowest_bit(int changeReason) {
    if ((changeReason & LCA_STRATEGIC) != 0) {
        return LCA_STRATEGIC;
    }
    if ((changeReason & LCA_COOPERATIVE) != 0) {
        return LCA_COOPERATIVE;
    }
    if ((changeReason & LCA_SPEEDGAIN) != 0) {
        return LCA_SPEEDGAIN;
    }
    if ((changeReason & LCA_KEEPRIGHT) != 0) {
        return LCA_KEEPRIGHT;
    }
    if ((changeReason & LCA_TRACI) != 0) {
        return LCA_TRACI;
    }
    return changeReason;
}


MSLCM_SL2015::StateAndDist
MSLCM_SL2015::decideDirection(StateAndDist sd1, StateAndDist sd2) const {
    // ignore dummy decisions (returned if mayChange() failes)
    if (sd1.state == 0) {
        return sd2;
    } else if (sd2.state == 0) {
        return sd1;
    }
    // LCA_SUBLANE is special because LCA_STAY|LCA_SUBLANE may override another LCA_SUBLANE command
    const bool want1 = ((sd1.state & LCA_WANTS_LANECHANGE) != 0) || ((sd1.state & LCA_SUBLANE) != 0 && (sd1.state & LCA_STAY) != 0);
    const bool want2 = ((sd2.state & LCA_WANTS_LANECHANGE) != 0) || ((sd2.state & LCA_SUBLANE) != 0 && (sd2.state & LCA_STAY) != 0);
    const bool can1 = ((sd1.state & LCA_BLOCKED) == 0);
    const bool can2 = ((sd2.state & LCA_BLOCKED) == 0);
    int reason1 = lowest_bit(sd1.state & LCA_CHANGE_REASONS);
    int reason2 = lowest_bit(sd2.state & LCA_CHANGE_REASONS);
#ifdef DEBUG_WANTSCHANGE
    if (DEBUG_COND) std::cout << SIMTIME
                                  << " veh=" << myVehicle.getID()
                                  << " state1=" << toString((LaneChangeAction)sd1.state)
                                  << " want1=" << (sd1.state & LCA_WANTS_LANECHANGE)
                                  << " dist1=" << sd1.latDist
                                  << " dir1=" << sd1.dir
                                  << " state2=" << toString((LaneChangeAction)sd2.state)
                                  << " want2=" << (sd2.state & LCA_WANTS_LANECHANGE)
                                  << " dist2=" << sd2.latDist
                                  << " dir2=" << sd2.dir
                                  << " reason1=" << toString((LaneChangeAction)reason1)
                                  << " reason2=" << toString((LaneChangeAction)reason2)
                                  << "\n";
#endif
    if (want1) {
        if (want2) {
            // decide whether right or left has higher priority (lower value in enum LaneChangeAction)
            if (reason1 < reason2) {
                //if (DEBUG_COND) std::cout << "   " << (sd1.state & LCA_CHANGE_REASONS) << " < " << (sd2.state & LCA_CHANGE_REASONS) << "\n";
                return (!can1 && can2 && sd1.sameDirection(sd2)) ? sd2 : sd1;
                //return sd1;
            } else if (reason1 > reason2) {
                //if (DEBUG_COND) std::cout << "   " << (sd1.state & LCA_CHANGE_REASONS) << " > " << (sd2.state & LCA_CHANGE_REASONS) << "\n";
                return (!can2 && can1 && sd1.sameDirection(sd2)) ? sd1 : sd2;
                //return sd2;
            } else {
                // same priority.
                if ((sd1.state & LCA_SUBLANE) != 0) {
                    // special treatment: prefer action with dir != 0
                    if (sd1.dir == 0) {
                        return sd2;
                    } else if (sd2.dir == 0) {
                        return sd1;
                    } else {
                        // prefer action that knows more about the desired direction
                        // @note when deciding between right and left, right is always given as sd1
                        assert(sd1.dir == -1);
                        assert(sd2.dir == 1);
                        if (sd1.latDist <= 0) {
                            return sd1;
                        } else if (sd2.latDist >= 0) {
                            return sd2;
                        }
                        // when in doubt, prefer moving to the right
                        return sd1.latDist <= sd2.latDist ? sd1 : sd2;
                    }
                } else {
                    // see which one is allowed
                    return can1 ? sd1 : sd2;
                }
            }
        } else {
            return sd1;
        }
    } else {
        return sd2;
    }

}


LaneChangeAction
MSLCM_SL2015::getLCA(int state, double latDist) {
    return ((latDist == 0 || (state & LCA_CHANGE_REASONS) == 0)
            ? LCA_NONE : (latDist < 0 ? LCA_RIGHT : LCA_LEFT));
}


int
MSLCM_SL2015::checkStrategicChange(int ret,
                                   int laneOffset,
                                   const std::vector<MSVehicle::LaneQ>& preb,
                                   const MSLeaderDistanceInfo& leaders,
                                   const MSLeaderDistanceInfo& neighLeaders,
                                   int currIdx,
                                   int bestLaneOffset,
                                   bool changeToBest,
                                   double currentDist,
                                   double neighDist,
                                   double laDist,
                                   double roundaboutBonus,
                                   double latLaneDist,
                                   double& latDist
                                  ) {
    const bool right = (laneOffset == -1);
    const bool left = (laneOffset == 1);
    const MSVehicle::LaneQ& curr = preb[currIdx];
    const MSVehicle::LaneQ& neigh = preb[currIdx + laneOffset];
    const MSVehicle::LaneQ& best = preb[currIdx + bestLaneOffset];

    myLeftSpace = currentDist - myVehicle.getPositionOnLane();
    const double usableDist = (currentDist - myVehicle.getPositionOnLane() - best.occupation *  JAM_FACTOR);
    //- (best.lane->getVehicleNumber() * neighSpeed)); // VARIANT 9 jfSpeed
    const double maxJam = MAX2(neigh.occupation, curr.occupation);
    const double neighLeftPlace = MAX2(0., neighDist - myVehicle.getPositionOnLane() - maxJam);
    // save the left space

#ifdef DEBUG_STRATEGIC_CHANGE
    if (gDebugFlag2) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " laSpeed=" << myLookAheadSpeed
                  << " laDist=" << laDist
                  << " currentDist=" << currentDist
                  << " usableDist=" << usableDist
                  << " bestLaneOffset=" << bestLaneOffset
                  << " best.length=" << best.length
                  << " maxJam=" << maxJam
                  << " neighLeftPlace=" << neighLeftPlace
                  << " myLeftSpace=" << myLeftSpace
                  << "\n";
    }
#endif

    if (laneOffset != 0 && changeToBest && bestLaneOffset == curr.bestLaneOffset
            && currentDistDisallows(usableDist, bestLaneOffset, laDist)) {
        /// @brief we urgently need to change lanes to follow our route
        latDist = latLaneDist;
        ret |= LCA_STRATEGIC | LCA_URGENT;
    } else {
        // VARIANT_20 (noOvertakeRight)
        if (!myAllowOvertakingRight && left && !myVehicle.congested() && neighLeaders.hasVehicles() && myVehicle.getVehicleType().getVehicleClass() != SVC_EMERGENCY) {
            // check for slower leader on the left. we should not overtake but
            // rather move left ourselves (unless congested)
            // XXX only adapt as much as possible to get a lateral gap
            CLeaderDist cld = getSlowest(neighLeaders);
            const MSVehicle* nv = cld.first;
            if (nv->getSpeed() < myVehicle.getSpeed()) {
                const double vSafe = myCarFollowModel.followSpeed(
                                         &myVehicle, myVehicle.getSpeed(), cld.second, nv->getSpeed(), nv->getCarFollowModel().getMaxDecel());
                addLCSpeedAdvice(vSafe);
                if (vSafe < myVehicle.getSpeed()) {
                    mySpeedGainProbabilityRight += myVehicle.getActionStepLengthSecs() * myChangeProbThresholdLeft / 3;
                }
#ifdef DEBUG_STRATEGIC_CHANGE
                if (gDebugFlag2) {
                    std::cout << SIMTIME
                              << " avoid overtaking on the right nv=" << nv->getID()
                              << " nvSpeed=" << nv->getSpeed()
                              << " mySpeedGainProbabilityR=" << mySpeedGainProbabilityRight
                              << " plannedSpeed=" << myVehicle.getSpeed() + ACCEL2SPEED(myLCAccelerationAdvices.back())
                              << "\n";
                }
#endif
            }
        }

        if (!changeToBest && (currentDistDisallows(neighLeftPlace, abs(bestLaneOffset) + 2, laDist))) {
            // the opposite lane-changing direction should be done than the one examined herein
            //  we'll check whether we assume we could change anyhow and get back in time...
            //
            // this rule prevents the vehicle from moving in opposite direction of the best lane
            //  unless the way till the end where the vehicle has to be on the best lane
            //  is long enough
#ifdef DEBUG_STRATEGIC_CHANGE
            if (gDebugFlag2) {
                std::cout << " veh=" << myVehicle.getID() << " could not change back and forth in time (1) neighLeftPlace=" << neighLeftPlace << "\n";
            }
#endif
            ret |= LCA_STAY | LCA_STRATEGIC;
        } else if (
            laneOffset != 0
            && bestLaneOffset == 0
            && !leaders.hasStoppedVehicle()
            && neigh.bestContinuations.back()->getLinkCont().size() != 0
            && roundaboutBonus == 0
            && neighDist < TURN_LANE_DIST
            && myStrategicParam >= 0) {
            // VARIANT_21 (stayOnBest)
            // we do not want to leave the best lane for a lane which leads elsewhere
            // unless our leader is stopped or we are approaching a roundabout
#ifdef DEBUG_STRATEGIC_CHANGE
            if (gDebugFlag2) {
                std::cout << " veh=" << myVehicle.getID() << " does not want to leave the bestLane (neighDist=" << neighDist << ")\n";
            }
#endif
            ret |= LCA_STAY | LCA_STRATEGIC;
        } else if (right
                   && bestLaneOffset == 0
                   && myVehicle.getLane()->getSpeedLimit() > 80. / 3.6
                   && myLookAheadSpeed > SUMO_const_haltingSpeed
                  ) {
            // let's also regard the case where the vehicle is driving on a highway...
            //  in this case, we do not want to get to the dead-end of an on-ramp
#ifdef DEBUG_STRATEGIC_CHANGE
            if (gDebugFlag2) {
                std::cout << " veh=" << myVehicle.getID() << " does not want to get stranded on the on-ramp of a highway\n";
            }
#endif
            ret |= LCA_STAY | LCA_STRATEGIC;
        }
    }
    if ((ret & LCA_URGENT) == 0 && getShadowLane() != nullptr &&
            // ignore overlap if it goes in the correct direction
            bestLaneOffset * myVehicle.getLateralPositionOnLane() <= 0) {
        // no decision or decision to stay
        // make sure to stay within lane bounds in case the shadow lane ends
        //const double requiredDist = MAX2(2 * myVehicle.getLateralOverlap(), getSublaneWidth()) / SUMO_const_laneWidth * laDist;
        const double requiredDist = 2 * myVehicle.getLateralOverlap() / SUMO_const_laneWidth * laDist;
        double currentShadowDist = -myVehicle.getPositionOnLane();
        MSLane* shadowPrev = nullptr;
        for (std::vector<MSLane*>::const_iterator it = curr.bestContinuations.begin(); it != curr.bestContinuations.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            MSLane* shadow = getShadowLane(*it);
            if (shadow == nullptr || currentShadowDist >= requiredDist) {
                break;
            }
            if (shadowPrev != nullptr) {
                currentShadowDist += shadowPrev->getEdge().getInternalFollowingLengthTo(&shadow->getEdge());
            }
            currentShadowDist += shadow->getLength();
            shadowPrev = shadow;
#ifdef DEBUG_STRATEGIC_CHANGE
            if (gDebugFlag2) {
                std::cout << "    shadow=" << shadow->getID() << " currentShadowDist=" << currentShadowDist << "\n";
            }
#endif
        }
#ifdef DEBUG_STRATEGIC_CHANGE
        if (gDebugFlag2) {
            std::cout << " veh=" << myVehicle.getID() << " currentShadowDist=" << currentShadowDist << " requiredDist=" << requiredDist << " overlap=" << myVehicle.getLateralOverlap() << "\n";
        }
#endif
        if (currentShadowDist < requiredDist && currentShadowDist < usableDist) {
            myLeftSpace = currentShadowDist;
            latDist = myVehicle.getLateralPositionOnLane() < 0 ? myVehicle.getLateralOverlap() : - myVehicle.getLateralOverlap();
#ifdef DEBUG_STRATEGIC_CHANGE
            if (gDebugFlag2) {
                std::cout << "    must change for shadowLane end latDist=" << latDist << " myLeftSpace=" << myLeftSpace << "\n";
            }
#endif
            ret |= LCA_STRATEGIC | LCA_URGENT | LCA_STAY ;
        }
    }

    // check for overriding TraCI requests
#if defined(DEBUG_STRATEGIC_CHANGE) || defined(DEBUG_TRACI)
    if (gDebugFlag2) {
        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " ret=" << ret;
    }
#endif
    // store state before canceling
    getCanceledState(laneOffset) |= ret;
    int retTraCI = myVehicle.influenceChangeDecision(ret);
    if ((retTraCI & LCA_TRACI) != 0) {
        if ((retTraCI & LCA_STAY) != 0) {
            ret = retTraCI;
            latDist = 0;
        } else if (((retTraCI & LCA_RIGHT) != 0 && laneOffset < 0)
                   || ((retTraCI & LCA_LEFT) != 0 && laneOffset > 0)) {
            ret = retTraCI;
            latDist = latLaneDist;
        }
    }
#if defined(DEBUG_STRATEGIC_CHANGE) || defined(DEBUG_TRACI)
    if (gDebugFlag2) {
        std::cout << " reqAfterInfluence=" << ret << " ret=" << ret << "\n";
    }
#endif
    return ret;
}


double
MSLCM_SL2015::computeGapFactor(int state) const {
    return (state & LCA_STRATEGIC) != 0 ? MAX2(0.0, (1.0 - myPushy * (1 + 0.5 * myImpatience))) : 1.0;
}


int
MSLCM_SL2015::keepLatGap(int state,
                         const MSLeaderDistanceInfo& leaders,
                         const MSLeaderDistanceInfo& followers,
                         const MSLeaderDistanceInfo& blockers,
                         const MSLeaderDistanceInfo& neighLeaders,
                         const MSLeaderDistanceInfo& neighFollowers,
                         const MSLeaderDistanceInfo& neighBlockers,
                         const MSLane& neighLane,
                         int laneOffset,
                         double& latDist,
                         double& maneuverDist,
                         int& blocked) {

    /* @notes
     * vehicles may need to compromise between fulfilling lane change objectives
     * (LCA_STRATEGIC, LCA_SPEED etc) and maintaining lateral gap. The minimum
     * acceptable lateral gap depends on
     * - the cultural context (China vs Europe)
     * - the driver agressiveness (willingness to encroach on other vehicles to force them to move laterally as well)
     *    - see @note in checkBlocking
     * - the vehicle type (car vs motorcycle)
     * - the current speed
     * - the speed difference
     * - the importance / urgency of the desired maneuver
     *
     * the object of this method is to evaluate the above circumstances and
     * either:
     * - allow the current maneuver (state, latDist)
     * - to override the current maneuver with a distance-keeping maneuver
     *
     *
     * laneChangeModel/driver parameters
     * - bool pushy (willingness to encroach)
     * - float minGap at 100km/h (to be interpolated for lower speeds (assume 0 at speed 0)
     * - gapFactors (a factor for each of the change reasons
     *
     * further assumptions
     * - the maximum of egoSpeed and deltaSpeed can be used when interpolating minGap
     * - distance keeping to the edges of the road can be ignored (for now)
     *
     * currentMinGap = minGap * min(1.0, max(v, abs(v - vOther)) / 100) * gapFactor[lc_reason]
     *
     * */

    /// XXX to be made configurable
    double gapFactor = computeGapFactor(state);
    const bool stayInLane = laneOffset == 0 || ((state & LCA_STRATEGIC) != 0 && (state & LCA_STAY) != 0);
    const double oldLatDist = latDist;
    const double oldManeuverDist = maneuverDist;

    // compute gaps after maneuver
    const double halfWidth = getWidth() * 0.5;
    // if the current maneuver is blocked we will stay where we are
    const double oldCenter = myVehicle.getCenterOnEdge();
    // surplus gaps. these are used to collect various constraints
    // if they do not permit the desired maneuvre, should override it to better maintain distance
    // stay within the current edge
    double surplusGapRight = oldCenter - halfWidth;
    double surplusGapLeft = myVehicle.getLane()->getEdge().getWidth() - oldCenter - halfWidth;
#ifdef DEBUG_KEEP_LATGAP
    if (gDebugFlag2) {
        std::cout << "\n  " << SIMTIME << " keepLatGap() laneOffset=" << laneOffset
                  << " latDist=" << latDist
                  << " maneuverDist=" << maneuverDist
                  << " state=" << toString((LaneChangeAction)state)
                  << " blocked=" << toString((LaneChangeAction)blocked)
                  << " gapFactor=" << gapFactor
                  << " stayInLane=" << stayInLane << "\n"
                  << "       stayInEdge: surplusGapRight=" << surplusGapRight << " surplusGapLeft=" << surplusGapLeft << "\n";
    }
#endif
    // staying within the edge overrides all minGap considerations
    if (surplusGapLeft < 0 || surplusGapRight < 0) {
        gapFactor = 0;
    }

    // maintain gaps to vehicles on the current lane
    // ignore vehicles that are too far behind
    const double netOverlap = -myVehicle.getVehicleType().getLength() * 0.5;
    updateGaps(leaders, myVehicle.getLane()->getRightSideOnEdge(), oldCenter, gapFactor, surplusGapRight, surplusGapLeft, true);
    updateGaps(followers, myVehicle.getLane()->getRightSideOnEdge(), oldCenter, gapFactor, surplusGapRight, surplusGapLeft, true, netOverlap);

    if (laneOffset != 0) {
        // maintain gaps to vehicles on the target lane
        updateGaps(neighLeaders, neighLane.getRightSideOnEdge(), oldCenter, gapFactor, surplusGapRight, surplusGapLeft, true);
        updateGaps(neighFollowers, neighLane.getRightSideOnEdge(), oldCenter, gapFactor, surplusGapRight, surplusGapLeft, true, netOverlap);
    }
#ifdef DEBUG_KEEP_LATGAP
    if (gDebugFlag2) {
        std::cout << "       minGapLat: surplusGapRight=" << surplusGapRight << " surplusGapLeft=" << surplusGapLeft << "\n"
                  << "       lastGaps: right=" << myLastLateralGapRight << " left=" << myLastLateralGapLeft << "\n";
    }
#endif
    // we also need to track the physical gap, in addition to the psychological gap
    double physicalGapLeft = myLastLateralGapLeft == NO_NEIGHBOR ? surplusGapLeft : myLastLateralGapLeft;
    double physicalGapRight = myLastLateralGapRight == NO_NEIGHBOR ? surplusGapRight : myLastLateralGapRight;

    const double halfLaneWidth = myVehicle.getLane()->getWidth() * 0.5;
    if (stayInLane || laneOffset == 1) {
        // do not move past the right boundary of the current lane (traffic wasn't checked there)
        // but assume it's ok to be where we are in case we are already beyond
        surplusGapRight  = MIN2(surplusGapRight,  MAX2(0.0, halfLaneWidth + myVehicle.getLateralPositionOnLane() - halfWidth));
        physicalGapRight = MIN2(physicalGapRight, MAX2(0.0, halfLaneWidth + myVehicle.getLateralPositionOnLane() - halfWidth));
    }
    if (stayInLane || laneOffset == -1) {
        // do not move past the left boundary of the current lane (traffic wasn't checked there)
        // but assume it's ok to be where we are in case we are already beyond
        surplusGapLeft  = MIN2(surplusGapLeft,  MAX2(0.0, halfLaneWidth - myVehicle.getLateralPositionOnLane() - halfWidth));
        physicalGapLeft = MIN2(physicalGapLeft, MAX2(0.0, halfLaneWidth - myVehicle.getLateralPositionOnLane() - halfWidth));
    }
#ifdef DEBUG_KEEP_LATGAP
    if (gDebugFlag2) {
        std::cout << "       stayInLane: surplusGapRight=" << surplusGapRight << " surplusGapLeft=" << surplusGapLeft << "\n";
    }
#endif

    if (surplusGapRight + surplusGapLeft < 0) {
        // insufficient lateral space to fulfill all requirements. apportion space proportionally
        if ((state & LCA_CHANGE_REASONS) == 0) {
            state |= LCA_SUBLANE;
        }
        const double equalDeficit = 0.5 * (surplusGapLeft + surplusGapRight);
        if (surplusGapRight < surplusGapLeft) {
            // shift further to the left but no further than there is physical space
            const double delta = MIN2(equalDeficit - surplusGapRight, physicalGapLeft);
            latDist = delta;
            maneuverDist = delta;
#ifdef DEBUG_KEEP_LATGAP
            if (gDebugFlag2) {
                std::cout << "    insufficient latSpace, move left: delta=" << delta << "\n";
            }
#endif
        } else {
            // shift further to the right but no further than there is physical space
            const double delta = MIN2(equalDeficit - surplusGapLeft, physicalGapRight);
            latDist = -delta;
            maneuverDist = -delta;
#ifdef DEBUG_KEEP_LATGAP
            if (gDebugFlag2) {
                std::cout << "    insufficient latSpace, move right: delta=" << delta << "\n";
            }
#endif
        }
    } else {
        // sufficient space. move as far as the gaps permit
        latDist = MAX2(MIN2(latDist, surplusGapLeft), -surplusGapRight);
        maneuverDist = MAX2(MIN2(maneuverDist, surplusGapLeft), -surplusGapRight);
        if ((state & LCA_KEEPRIGHT) != 0 && maneuverDist != oldManeuverDist) {
            // don't start keepRight unless it can be completed
            latDist = oldLatDist;
            maneuverDist = oldManeuverDist;
        }
#ifdef DEBUG_KEEP_LATGAP
        if (gDebugFlag2) {
            std::cout << "     adapted latDist=" << latDist << " maneuverDist=" << maneuverDist << " (old=" << oldLatDist << ")\n";
        }
#endif
    }
    // take into account overriding traci sublane-request
    if (myVehicle.hasInfluencer() && myVehicle.getInfluencer().getLatDist() != 0) {
        // @note: the influence is reset in MSAbstractLaneChangeModel::setOwnState at the end of the lane-changing code for this vehicle
        latDist = myVehicle.getInfluencer().getLatDist();
        maneuverDist = myVehicle.getInfluencer().getLatDist();
        state |= LCA_TRACI;
#ifdef DEBUG_KEEP_LATGAP
        if (gDebugFlag2) {
            std::cout << "     traci influenced latDist=" << latDist << "\n";
        }
#endif
    }
    // if we cannot move in the desired direction, consider the maneuver blocked anyway
    const bool nonSublaneChange = (state & (LCA_STRATEGIC | LCA_COOPERATIVE | LCA_SPEEDGAIN | LCA_KEEPRIGHT)) != 0;
    const bool traciChange = (state & LCA_TRACI) != 0;
    if (nonSublaneChange && !traciChange) {
        if ((latDist < NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) && (oldLatDist > 0)) {
#ifdef DEBUG_KEEP_LATGAP
            if (gDebugFlag2) {
                std::cout << "     wanted changeToLeft oldLatDist=" << oldLatDist << ", blocked latGap changeToRight\n";
            }
#endif
            latDist = oldLatDist; // restore old request for usage in decideDirection()
            blocked = LCA_OVERLAPPING | LCA_BLOCKED_LEFT;
        } else if ((latDist > -NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) && (oldLatDist < 0)) {
#ifdef DEBUG_KEEP_LATGAP
            if (gDebugFlag2) {
                std::cout << "     wanted changeToRight oldLatDist=" << oldLatDist << ", blocked latGap changeToLeft\n";
            }
#endif
            latDist = oldLatDist; // restore old request for usage in decideDirection()
            blocked = LCA_OVERLAPPING | LCA_BLOCKED_RIGHT;
        }
    }
    // if we move, even though we wish to stay, update the change reason (except for TraCI)
    if (fabs(latDist) > NUMERICAL_EPS * myVehicle.getActionStepLengthSecs() && oldLatDist == 0) {
        state &= (~(LCA_CHANGE_REASONS | LCA_STAY) | LCA_TRACI);
    }
    // update blocked status
    if (fabs(latDist - oldLatDist) > NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) {
#ifdef DEBUG_KEEP_LATGAP
        if (gDebugFlag2) {
            std::cout << "     latDistUpdated=" << latDist << " oldLatDist=" << oldLatDist << "\n";
        }
#endif
        blocked = checkBlocking(neighLane, latDist, maneuverDist, laneOffset, leaders, followers, blockers, neighLeaders, neighFollowers, neighBlockers, nullptr, nullptr, nonSublaneChange);
    }
    if (fabs(latDist) > NUMERICAL_EPS * myVehicle.getActionStepLengthSecs()) {
        state = (state & ~LCA_STAY);
        if ((state & LCA_CHANGE_REASONS) == 0) {
            state |= LCA_SUBLANE;
        }
    } else {
        if ((state & LCA_SUBLANE) != 0) {
            state |= LCA_STAY;
        }
        // avoid setting blinker due to numerical issues
        latDist = 0;
    }
#if defined(DEBUG_KEEP_LATGAP) || defined(DEBUG_STATE)
    if (gDebugFlag2) {
        std::cout << "       latDist2=" << latDist
                  << " state2=" << toString((LaneChangeAction)state)
                  << " lastGapLeft=" << myLastLateralGapLeft
                  << " lastGapRight=" << myLastLateralGapRight
                  << " blockedAfter=" << toString((LaneChangeAction)blocked)
                  << "\n";
    }
#endif
    return state;
}


void
MSLCM_SL2015::updateGaps(const MSLeaderDistanceInfo& others, double foeOffset, double oldCenter, double gapFactor,
                         double& surplusGapRight, double& surplusGapLeft,
                         bool saveMinGap, double netOverlap,
                         double latDist,
                         std::vector<CLeaderDist>* collectBlockers) {
    if (others.hasVehicles()) {
        const double halfWidth = getWidth() * 0.5 + NUMERICAL_EPS;
        const double baseMinGap = myVehicle.getVehicleType().getMinGapLat();
        for (int i = 0; i < others.numSublanes(); ++i) {
            if (others[i].first != 0 && others[i].second <= 0
                    && myCFRelated.count(others[i].first) == 0
                    && (netOverlap == 0 || others[i].second + others[i].first->getVehicleType().getMinGap() < netOverlap)) {
                /// foe vehicle occupies full sublanes
                const MSVehicle* foe = others[i].first;
                const double res = MSGlobals::gLateralResolution > 0 ? MSGlobals::gLateralResolution : others[i].first->getLane()->getWidth();
                double foeRight, foeLeft;
                others.getSublaneBorders(i, foeOffset, foeRight, foeLeft);
                const double foeCenter = foeRight + 0.5 * res;
                const double gap = MIN2(fabs(foeRight - oldCenter), fabs(foeLeft - oldCenter)) - halfWidth;
                const double deltaV = MIN2(LATGAP_SPEED_THRESHOLD, MAX3(LATGAP_SPEED_THRESHOLD2, myVehicle.getSpeed(), fabs(myVehicle.getSpeed() - foe->getSpeed())));
                const double desiredMinGap = baseMinGap * deltaV / LATGAP_SPEED_THRESHOLD;
                const double currentMinGap = desiredMinGap * gapFactor; // pushy vehicles may accept a lower lateral gap temporarily
                /*
                if (netOverlap != 0) {
                    // foe vehicle is follower with its front ahead of the ego midpoint
                    // scale gap requirements so it gets lower for foe which are further behind ego
                    //
                    // relOverlap approaches 0 as the foe gets closer to the midpoint and it equals 1 if the foe is driving head-to-head
                    const double relOverlap = 1 - (others[i].second + others[i].first->getVehicleType().getMinGap()) / netOverlap;
                    currentMinGap *= currOverlap * relOverlap;
                }
                */
#if defined(DEBUG_BLOCKING) || defined(DEBUG_KEEP_LATGAP)
                if (debugVehicle()) {
                    std::cout << "  updateGaps"
                              << " i=" << i
                              << " foe=" << foe->getID()
                              << " foeRight=" << foeRight
                              << " foeLeft=" << foeLeft
                              << " oldCenter=" << oldCenter
                              << " gap=" << others[i].second
                              << " latgap=" << gap
                              << " currentMinGap=" << currentMinGap
                              << " surplusGapRight=" << surplusGapRight
                              << " surplusGapLeft=" << surplusGapLeft
                              << "\n";
                }
#endif

                // If foe is maneuvering towards ego, reserve some additional distance.
                // But don't expect the foe to come closer than currentMinGap if it isn't already there.
                //   (XXX: How can the ego know the foe's maneuver dist?)
                if (foeCenter < oldCenter) { // && foe->getLaneChangeModel().getSpeedLat() > 0) {
                    const double foeManeuverDist = MAX2(0., foe->getLaneChangeModel().getManeuverDist());
                    surplusGapRight = MIN3(surplusGapRight, gap - currentMinGap, MAX2(currentMinGap, gap - foeManeuverDist));
                } else { //if (foeCenter > oldCenter && foe->getLaneChangeModel().getSpeedLat() < 0) {
                    const double foeManeuverDist = -MIN2(0., foe->getLaneChangeModel().getManeuverDist());
                    surplusGapLeft = MIN3(surplusGapLeft, gap - currentMinGap, MAX2(currentMinGap, gap - foeManeuverDist));
                }
                if (saveMinGap) {
                    if (foeCenter < oldCenter) {
#if defined(DEBUG_BLOCKING) || defined(DEBUG_KEEP_LATGAP)
                        if (gDebugFlag2 && gap < myLastLateralGapRight) {
                            std::cout << "    new minimum rightGap=" << gap << "\n";
                        }
#endif
                        myLastLateralGapRight = MIN2(myLastLateralGapRight, gap);
                    } else {
#if defined(DEBUG_BLOCKING) || defined(DEBUG_KEEP_LATGAP)
                        if (gDebugFlag2 && gap < myLastLateralGapLeft) {
                            std::cout << "    new minimum leftGap=" << gap << "\n";
                        }
#endif
                        myLastLateralGapLeft = MIN2(myLastLateralGapLeft, gap);
                    }
                }
                if (collectBlockers != nullptr) {
                    // check if the vehicle is blocking a desire lane change
                    if ((foeCenter < oldCenter && latDist < 0 && gap < (desiredMinGap - latDist))
                            || (foeCenter > oldCenter && latDist > 0 && gap < (desiredMinGap + latDist))) {
                        collectBlockers->push_back(others[i]);
                    }
                }
            }
        }
    }
}


double
MSLCM_SL2015::getWidth() const {
    return myVehicle.getVehicleType().getWidth() + NUMERICAL_EPS;
}


double
MSLCM_SL2015::computeSpeedLat(double latDist, double& maneuverDist) const {
    int currentDirection = mySpeedLat >= 0 ? 1 : -1;
    int directionWish = latDist >= 0 ? 1 : -1;
    double maxSpeedLat = myVehicle.getVehicleType().getMaxSpeedLat();
    if (myLeftSpace > POSITION_EPS) {
        double speedBound = myMaxSpeedLatStanding + myMaxSpeedLatFactor * myVehicle.getSpeed();
        maxSpeedLat = MIN2(maxSpeedLat, speedBound);
    }

#ifdef DEBUG_MANEUVER
    if (debugVehicle()) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " computeSpeedLat()"
                  << " currentDirection=" << currentDirection
                  << " directionWish=" << directionWish
                  << std::endl;
    }
#endif
    // reduced lateral speed (in the desired direction). Don't change direction against desired.
    double speedDecel;
    if (directionWish == 1) {
        speedDecel = MAX2(mySpeedLat - ACCEL2SPEED(myAccelLat), 0.);
    } else {
        speedDecel = MIN2(mySpeedLat + ACCEL2SPEED(myAccelLat), 0.);
    }
    // Eventually reduce lateral speed even more to ensure safety
    double speedDecelSafe = MAX2(MIN2(speedDecel, DIST2SPEED(mySafeLatDistLeft)), DIST2SPEED(-mySafeLatDistRight));

    // increased lateral speed (in the desired direction)
    double speedAccel = MAX2(MIN2(mySpeedLat + directionWish * ACCEL2SPEED(myAccelLat), maxSpeedLat), -maxSpeedLat);
    // increase lateral speed more strongly to ensure safety (when moving in the wrong direction)
    double speedAccelSafe = latDist * speedAccel >= 0 ? speedAccel : 0;

    // can we reach the target distance in a single step? (XXX: assumes "Euler" update)
    double speedBound = DIST2SPEED(latDist);
    // for lat-gap keeping maneuvres myOrigLatDist may be 0
    const double fullLatDist = latDist > 0 ? MIN2(mySafeLatDistLeft, MAX2(maneuverDist, latDist)) : MAX2(-mySafeLatDistRight, MIN2(maneuverDist, latDist));

    // update maneuverDist, if safety constraints apply in its direction
    if (maneuverDist * latDist > 0) {
        maneuverDist = fullLatDist;
    }

#ifdef DEBUG_MANEUVER
    if (debugVehicle()) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " speedLat=" << mySpeedLat
                  << " latDist=" << latDist
                  << " maneuverDist=" << maneuverDist
                  << " mySafeLatDistRight=" << mySafeLatDistRight
                  << " mySafeLatDistLeft=" << mySafeLatDistLeft
                  << " fullLatDist=" << fullLatDist
                  << " speedAccel=" << speedAccel
                  << " speedDecel=" << speedDecel
                  << " speedDecelSafe=" << speedDecelSafe
                  << " speedBound=" << speedBound
                  << std::endl;
    }
#endif
    if (speedDecel * speedAccel <= 0 && (
                // speedAccel and speedDecel bracket speed 0. This means we can end the maneuver
                (latDist >= 0 && speedAccel >= speedBound && speedBound >= speedDecel)
                || (latDist <= 0 && speedAccel <= speedBound && speedBound <= speedDecel))) {
        // we can reach the desired value in this step
#ifdef DEBUG_MANEUVER
        if (debugVehicle()) {
            std::cout << "   computeSpeedLat a)\n";
        }
#endif
        return speedBound;
    }
    // are we currently moving in the wrong direction?
    if (latDist * mySpeedLat < 0) {
#ifdef DEBUG_MANEUVER
        if (debugVehicle()) {
            std::cout << "   computeSpeedLat b)\n";
        }
#endif
        return speedAccelSafe;
    }
    // check if the remaining distance allows to accelerate laterally
    double minDistAccel = SPEED2DIST(speedAccel) + currentDirection * MSCFModel::brakeGapEuler(fabs(speedAccel), myAccelLat, 0); // most we can move in the target direction
    if ((fabs(minDistAccel) < fabs(fullLatDist)) || (fabs(minDistAccel - fullLatDist) < NUMERICAL_EPS)) {
#ifdef DEBUG_MANEUVER
        if (debugVehicle()) {
            std::cout << "   computeSpeedLat c)\n";
        }
#endif
        return speedAccel;
    } else {
#ifdef DEBUG_MANEUVER
        if (debugVehicle()) {
            std::cout << "      minDistAccel=" << minDistAccel << "\n";
        }
#endif
        // check if the remaining distance allows to maintain current lateral speed
        double minDistCurrent = SPEED2DIST(mySpeedLat) + currentDirection * MSCFModel::brakeGapEuler(fabs(mySpeedLat), myAccelLat, 0);
        if ((fabs(minDistCurrent) < fabs(fullLatDist)) || (fabs(minDistCurrent - fullLatDist) < NUMERICAL_EPS)) {
#ifdef DEBUG_MANEUVER
            if (debugVehicle()) {
                std::cout << "   computeSpeedLat d)\n";
            }
#endif
            return mySpeedLat;
        }
    }
    // reduce lateral speed
#ifdef DEBUG_MANEUVER
    if (debugVehicle()) {
        std::cout << "   computeSpeedLat e)\n";
    }
#endif
    return speedDecelSafe;
}


void
MSLCM_SL2015::commitManoeuvre(int blocked, int blockedFully,
                              const MSLeaderDistanceInfo& leaders,
                              const MSLeaderDistanceInfo& neighLeaders,
                              const MSLane& neighLane,
                              double maneuverDist) {
    if (!blocked && !blockedFully && !myCanChangeFully) {
        // round to full action steps
        double secondsToLeaveLane;
        if (MSGlobals::gSemiImplicitEulerUpdate) {
            secondsToLeaveLane = ceil(fabs(maneuverDist) / myVehicle.getVehicleType().getMaxSpeedLat() / myVehicle.getActionStepLengthSecs()) * myVehicle.getActionStepLengthSecs();
            // XXX myAccelLat must be taken into account (refs #3601, see ballistic case for solution)

            // XXX This also causes probs: if the difference between the current speed and the committed is higher than the maximal decel,
            //     the vehicle may pass myLeftSpace before completing the maneuver.
            myCommittedSpeed = MIN3(myLeftSpace / secondsToLeaveLane,
                                    myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle),
                                    myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle));
#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
            if (debugVehicle()) {
                std::cout << SIMTIME << " veh=" << myVehicle.getID() << " myCommittedSpeed=" << myCommittedSpeed << " leftSpace=" << myLeftSpace << " secondsToLeave=" << secondsToLeaveLane << "\n";
            }
#endif
        } else {

            // Calculate seconds needed for leaving lane assuming start from lateral speed zero, and lat.accel == -lat.decel
            secondsToLeaveLane = MSCFModel::estimateArrivalTime(fabs(maneuverDist), 0., 0., myVehicle.getVehicleType().getMaxSpeedLat(), myAccelLat, myAccelLat);
            // round to full action steps
            secondsToLeaveLane = ceil(secondsToLeaveLane / myVehicle.getActionStepLengthSecs()) * myVehicle.getActionStepLengthSecs();

            // committed speed will eventually be pushed into a drive item during the next planMove() step. This item
            // will not be read before the next action step at current time + actionStepLength-TS, so we need to schedule the corresponding speed.
            const double timeTillActionStep = myVehicle.getActionStepLengthSecs() - TS;
            const double nextActionStepSpeed = MAX2(0., myVehicle.getSpeed() + timeTillActionStep * myVehicle.getAcceleration());
            double nextLeftSpace;
            if (nextActionStepSpeed > 0.) {
                nextLeftSpace = myLeftSpace - timeTillActionStep * (myVehicle.getSpeed() + nextActionStepSpeed) * 0.5;
            } else if (myVehicle.getAcceleration() == 0) {
                nextLeftSpace = myLeftSpace;
            } else {
                assert(myVehicle.getAcceleration() < 0.);
                nextLeftSpace = myLeftSpace + (myVehicle.getSpeed() * myVehicle.getSpeed() / myVehicle.getAcceleration()) * 0.5;
            }
            const double avoidArrivalSpeed = nextActionStepSpeed + ACCEL2SPEED(MSCFModel::avoidArrivalAccel(
                                                 nextLeftSpace, secondsToLeaveLane - timeTillActionStep, nextActionStepSpeed, myVehicle.getCarFollowModel().getEmergencyDecel()));

            myCommittedSpeed = MIN3(avoidArrivalSpeed,
                                    myVehicle.getSpeed() + myVehicle.getCarFollowModel().getMaxAccel() * myVehicle.getActionStepLengthSecs(),
                                    myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle));

#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
            if (gDebugFlag2) {
                std::cout << SIMTIME
                          << " veh=" << myVehicle.getID()
                          << " avoidArrivalSpeed=" << avoidArrivalSpeed
                          << " currentSpeed=" << myVehicle.getSpeed()
                          << " myLeftSpace=" << myLeftSpace
                          << "\n             nextLeftSpace=" << nextLeftSpace
                          << " nextActionStepSpeed=" << nextActionStepSpeed
                          << " nextActionStepRemainingSeconds=" << secondsToLeaveLane - timeTillActionStep
                          << "\n";
            }
#endif
        }
        myCommittedSpeed = commitFollowSpeed(myCommittedSpeed, maneuverDist, secondsToLeaveLane, leaders, myVehicle.getLane()->getRightSideOnEdge());
        myCommittedSpeed = commitFollowSpeed(myCommittedSpeed, maneuverDist, secondsToLeaveLane, neighLeaders, neighLane.getRightSideOnEdge());
        if (myCommittedSpeed < myVehicle.getCarFollowModel().minNextSpeed(myVehicle.getSpeed(), &myVehicle)) {
            myCommittedSpeed = 0;
        }
#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
        if (gDebugFlag2) {
            std::cout << SIMTIME
                      << " veh=" << myVehicle.getID()
                      << " secondsToLeave=" << secondsToLeaveLane
                      << " maxNext=" << myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle)
                      << " committed=" << myCommittedSpeed
                      << "\n";
        }
#endif
    }
}

double
MSLCM_SL2015::commitFollowSpeed(double speed, double latDist, double secondsToLeaveLane, const MSLeaderDistanceInfo& leaders, double foeOffset) const {
    if (leaders.hasVehicles()) {
        // we distinguish 3 cases
        // - vehicles with lateral overlap at the end of the maneuver: try to follow safely
        // - vehicles with overlap at the start of the maneuver: avoid collision within secondsToLeaveLane
        // - vehicles without overlap: ignore

        const double maxDecel = myVehicle.getCarFollowModel().getMaxDecel();
        // temporarily use another decel value
        MSCFModel& cfmodel = const_cast<MSCFModel&>(myVehicle.getCarFollowModel());
        cfmodel.setMaxDecel(maxDecel / getSafetyFactor());

        const double vehWidth = getWidth();
        const double rightVehSide = myVehicle.getCenterOnEdge() - 0.5 * vehWidth;
        const double leftVehSide = rightVehSide + vehWidth;
        const double rightVehSideDest = rightVehSide + latDist;
        const double leftVehSideDest = leftVehSide + latDist;
#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
        if (gDebugFlag2) {
            std::cout << "  commitFollowSpeed"
                      << " latDist=" << latDist
                      << " foeOffset=" << foeOffset
                      << " vehRight=" << rightVehSide
                      << " vehLeft=" << leftVehSide
                      << " destRight=" << rightVehSideDest
                      << " destLeft=" << leftVehSideDest
                      << "\n";
        }
#endif
        for (int i = 0; i < leaders.numSublanes(); ++i) {
            CLeaderDist vehDist = leaders[i];
            if (vehDist.first != 0) {
                const MSVehicle* leader = vehDist.first;
                // only check the current stripe occuped by foe (transform into edge-coordinates)
                double foeRight, foeLeft;
                leaders.getSublaneBorders(i, foeOffset, foeRight, foeLeft);
#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
                if (gDebugFlag2) {
                    std::cout << "   foe=" << vehDist.first->getID()
                              << " gap=" << vehDist.second
                              << " secGap=" << myVehicle.getCarFollowModel().getSecureGap(&myVehicle, leader, myVehicle.getSpeed(), leader->getSpeed(), leader->getCarFollowModel().getMaxDecel())
                              << " foeRight=" << foeRight
                              << " foeLeft=" << foeLeft
                              << " overlapBefore=" << overlap(rightVehSide, leftVehSide, foeRight, foeLeft)
                              << " overlapDest=" << overlap(rightVehSideDest, leftVehSideDest, foeRight, foeLeft)
                              << "\n";
                }
#endif
                if (overlap(rightVehSideDest, leftVehSideDest, foeRight, foeLeft)) {
                    // case 1
                    const double vSafe = myVehicle.getCarFollowModel().followSpeed(
                                             &myVehicle, speed, vehDist.second, leader->getSpeed(), leader->getCarFollowModel().getMaxDecel());
                    speed = MIN2(speed, vSafe);
#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
                    if (gDebugFlag2) {
                        std::cout << "     case1 vsafe=" << vSafe << " speed=" << speed << "\n";
                    }
#endif
                } else if (overlap(rightVehSide, leftVehSide, foeRight, foeLeft)) {
                    // case 2
                    const double vSafe = myVehicle.getCarFollowModel().followSpeedTransient(
                                             secondsToLeaveLane,
                                             &myVehicle, speed, vehDist.second, leader->getSpeed(), leader->getCarFollowModel().getMaxDecel());
                    speed = MIN2(speed, vSafe);
#if defined(DEBUG_MANEUVER) || defined(DEBUG_COMMITTED_SPEED)
                    if (gDebugFlag2) {
                        std::cout << "     case2 vsafe=" << vSafe << " speed=" << speed << "\n";
                    }
#endif
                }
            }
        }
        // restore original deceleration
        cfmodel.setMaxDecel(maxDecel);

    }
    return speed;
}

double
MSLCM_SL2015::getSafetyFactor() const {
    return 1 / ((1 + 0.5 * myImpatience) * myAssertive);
}


std::string
MSLCM_SL2015::getParameter(const std::string& key) const {
    if (key == toString(SUMO_ATTR_LCA_STRATEGIC_PARAM)) {
        return toString(myStrategicParam);
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_PARAM)) {
        return toString(myCooperativeParam);
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAIN_PARAM)) {
        return toString(mySpeedGainParam);
    } else if (key == toString(SUMO_ATTR_LCA_KEEPRIGHT_PARAM)) {
        return toString(myKeepRightParam);
    } else if (key == toString(SUMO_ATTR_LCA_SUBLANE_PARAM)) {
        return toString(mySublaneParam);
    } else if (key == toString(SUMO_ATTR_LCA_PUSHY)) {
        return toString(myPushy);
    } else if (key == toString(SUMO_ATTR_LCA_ASSERTIVE)) {
        return toString(myAssertive);
    } else if (key == toString(SUMO_ATTR_LCA_IMPATIENCE)) {
        return toString(myImpatience);
    } else if (key == toString(SUMO_ATTR_LCA_TIME_TO_IMPATIENCE)) {
        return toString(myTimeToImpatience);
    } else if (key == toString(SUMO_ATTR_LCA_ACCEL_LAT)) {
        return toString(myAccelLat);
    } else if (key == toString(SUMO_ATTR_LCA_LOOKAHEADLEFT)) {
        return toString(myLookaheadLeft);
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAINRIGHT)) {
        return toString(mySpeedGainRight);
    } else if (key == toString(SUMO_ATTR_LCA_LANE_DISCIPLINE)) {
        return toString(myLaneDiscipline);
    } else if (key == toString(SUMO_ATTR_LCA_SIGMA)) {
        return toString(mySigma);
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD)) {
        return toString(mySpeedGainLookahead);
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT)) {
        return toString(myRoundaboutBonus);
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_SPEED)) {
        return toString(myCooperativeSpeed);
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for laneChangeModel of type '" + toString(myModel) + "'");
}

void
MSLCM_SL2015::setParameter(const std::string& key, const std::string& value) {
    double doubleValue;
    try {
        doubleValue = StringUtils::toDouble(value);
    } catch (NumberFormatException&) {
        throw InvalidArgument("Setting parameter '" + key + "' requires a number for laneChangeModel of type '" + toString(myModel) + "'");
    }
    if (key == toString(SUMO_ATTR_LCA_STRATEGIC_PARAM)) {
        myStrategicParam = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_PARAM)) {
        myCooperativeParam = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAIN_PARAM)) {
        mySpeedGainParam = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_KEEPRIGHT_PARAM)) {
        myKeepRightParam = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_SUBLANE_PARAM)) {
        mySublaneParam = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_PUSHY)) {
        myPushy = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_ASSERTIVE)) {
        myAssertive = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_IMPATIENCE)) {
        myImpatience = doubleValue;
        myMinImpatience = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_TIME_TO_IMPATIENCE)) {
        myTimeToImpatience = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_ACCEL_LAT)) {
        myAccelLat = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE)) {
        myTurnAlignmentDist = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_LOOKAHEADLEFT)) {
        myLookaheadLeft = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAINRIGHT)) {
        mySpeedGainRight = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_LANE_DISCIPLINE)) {
        myLaneDiscipline = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_SIGMA)) {
        mySigma = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD)) {
        mySpeedGainLookahead = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT)) {
        myRoundaboutBonus = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_SPEED)) {
        myCooperativeSpeed = doubleValue;
    } else {
        throw InvalidArgument("Setting parameter '" + key + "' is not supported for laneChangeModel of type '" + toString(myModel) + "'");
    }
    initDerivedParameters();
}


int
MSLCM_SL2015::wantsChange(
    int laneOffset,
    MSAbstractLaneChangeModel::MSLCMessager& /* msgPass */,
    int blocked,
    const std::pair<MSVehicle*, double>& leader,
    const std::pair<MSVehicle*, double>& neighLead,
    const std::pair<MSVehicle*, double>& neighFollow,
    const MSLane& neighLane,
    const std::vector<MSVehicle::LaneQ>& preb,
    MSVehicle** lastBlocked,
    MSVehicle** firstBlocked) {

    const LaneChangeAction alternatives = LCA_NONE; // @todo pas this data

#ifdef DEBUG_WANTSCHANGE
    if (DEBUG_COND) {
        std::cout << "\nWANTS_CHANGE\n" << SIMTIME
                  //<< std::setprecision(10)
                  << " veh=" << myVehicle.getID()
                  << " lane=" << myVehicle.getLane()->getID()
                  << " pos=" << myVehicle.getPositionOnLane()
                  << " posLat=" << myVehicle.getLateralPositionOnLane()
                  << " speed=" << myVehicle.getSpeed()
                  << " considerChangeTo=" << (laneOffset == -1  ? "right" : "left")
                  << "\n";
    }
#endif

    double latDist = 0;
    const MSLane* dummy = myVehicle.getLane();
    MSLeaderDistanceInfo leaders(leader, dummy);
    MSLeaderDistanceInfo followers(std::make_pair((MSVehicle*)nullptr, -1), dummy);
    MSLeaderDistanceInfo blockers(std::make_pair((MSVehicle*)nullptr, -1), dummy);
    MSLeaderDistanceInfo neighLeaders(neighLead, dummy);
    MSLeaderDistanceInfo neighFollowers(neighFollow, dummy);
    MSLeaderDistanceInfo neighBlockers(std::make_pair((MSVehicle*)nullptr, -1), dummy);

    double maneuverDist;
    int result = _wantsChangeSublane(laneOffset,
                                     alternatives,
                                     leaders, followers, blockers,
                                     neighLeaders, neighFollowers, neighBlockers,
                                     neighLane, preb,
                                     lastBlocked, firstBlocked, latDist, maneuverDist, blocked);

    myCanChangeFully = true;
    // ignore sublane motivation
    result &= ~LCA_SUBLANE;
    result |= getLCA(result, latDist);

#if defined(DEBUG_WANTSCHANGE) || defined(DEBUG_STATE)
    if (DEBUG_COND) {
        if (result & LCA_WANTS_LANECHANGE) {
            std::cout << SIMTIME
                      << " veh=" << myVehicle.getID()
                      << " wantsChangeTo=" << (laneOffset == -1  ? "right" : "left")
                      << ((result & LCA_URGENT) ? " (urgent)" : "")
                      << ((result & LCA_CHANGE_TO_HELP) ? " (toHelp)" : "")
                      << ((result & LCA_STRATEGIC) ? " (strat)" : "")
                      << ((result & LCA_COOPERATIVE) ? " (coop)" : "")
                      << ((result & LCA_SPEEDGAIN) ? " (speed)" : "")
                      << ((result & LCA_KEEPRIGHT) ? " (keepright)" : "")
                      << ((result & LCA_TRACI) ? " (traci)" : "")
                      << ((blocked & LCA_BLOCKED) ? " (blocked)" : "")
                      << ((blocked & LCA_OVERLAPPING) ? " (overlap)" : "")
                      << "\n\n\n";
        }
    }
#endif

    return result;
}


/****************************************************************************/
