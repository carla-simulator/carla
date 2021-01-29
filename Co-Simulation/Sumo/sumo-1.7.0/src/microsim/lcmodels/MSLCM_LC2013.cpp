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
/// @file    MSLCM_LC2013.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Friedemann Wesner
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Leonhard Luecken
/// @date    Fri, 08.10.2013
///
// A lane change model developed by J. Erdmann
// based on the model of D. Krajzewicz developed between 2004 and 2011 (MSLCM_DK2004)
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/RandHelper.h>
#include <utils/common/StringUtils.h>
#include <microsim/transportables/MSPModel.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSDriverState.h>
#include <microsim/MSNet.h>
#include "MSLCHelper.h"
#include "MSLCM_LC2013.h"


// ===========================================================================
// variable definitions
// ===========================================================================
#define LOOK_FORWARD 10.

#define JAM_FACTOR 1.

#define LCA_RIGHT_IMPATIENCE -1.
#define CUT_IN_LEFT_SPEED_THRESHOLD 27.

#define LOOK_AHEAD_MIN_SPEED 0.0
#define LOOK_AHEAD_SPEED_MEMORY 0.9

#define HELP_DECEL_FACTOR 1.0

#define HELP_OVERTAKE  (10.0 / 3.6)
#define MIN_FALLBEHIND  (7.0 / 3.6)

// allow overtaking to the right below this speed difference
#define OVERTAKE_RIGHT_THRESHOLD (5/3.6)

#define RELGAIN_NORMALIZATION_MIN_SPEED 10.0
#define URGENCY 2.0
#define OPPOSITE_URGENCY 5.0

#define KEEP_RIGHT_TIME 5.0 // the number of seconds after which a vehicle should move to the right lane
#define KEEP_RIGHT_ACCEPTANCE 7.0 // calibration factor for determining the desire to keep right

#define ROUNDABOUT_DIST_FACTOR 10.0 // Must be >=1.0, serves an alternative way of decreasing sense lc-urgency by multiplying the distance along the next roundabout

#define KEEP_RIGHT_HEADWAY 2.0
#define MAX_ONRAMP_LENGTH 200.
#define TURN_LANE_DIST 200.0 // the distance at which a lane leading elsewhere is considered to be a turn-lane that must be avoided

#define LC_RESOLUTION_SPEED_LAT 0.5 // the lateral speed (in m/s) for a standing vehicle which was unable to finish a continuous LC in time (in case mySpeedLatStanding==0), see #3771
#define LC_ASSUMED_DECEL 1.0 // the minimal constant deceleration assumed to estimate the duration of a continuous lane-change at its initiation.

#define REACT_TO_STOPPED_DISTANCE 100

// ===========================================================================
// debug defines
// ===========================================================================
//#define DEBUG_PATCH_SPEED
//#define DEBUG_INFORMED
//#define DEBUG_INFORMER
//#define DEBUG_CONSTRUCTOR
//#define DEBUG_WANTS_CHANGE
//#define DEBUG_SLOW_DOWN
//#define DEBUG_COOPERATE
//#define DEBUG_SAVE_BLOCKER_LENGTH

//#define DEBUG_COND (myVehicle.getID() == "disabled")
#define DEBUG_COND (myVehicle.isSelected())
//#define DEBUG_COND (false)

// ===========================================================================
// member method definitions
// ===========================================================================
MSLCM_LC2013::MSLCM_LC2013(MSVehicle& v) :
    MSAbstractLaneChangeModel(v, LCM_LC2013),
    mySpeedGainProbability(0),
    myKeepRightProbability(0),
    myLeadingBlockerLength(0),
    myLeftSpace(0),
    myLookAheadSpeed(LOOK_AHEAD_MIN_SPEED),
    myStrategicParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_STRATEGIC_PARAM, 1)),
    myCooperativeParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_COOPERATIVE_PARAM, 1)),
    mySpeedGainParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SPEEDGAIN_PARAM, 1)),
    myKeepRightParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_KEEPRIGHT_PARAM, 1)),
    myOppositeParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_OPPOSITE_PARAM, 1)),
    myLookaheadLeft(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_LOOKAHEADLEFT, 2.0)),
    mySpeedGainRight(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SPEEDGAINRIGHT, 0.1)),
    myAssertive(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_ASSERTIVE, 1)),
    mySpeedGainLookahead(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD, 0)),
    myRoundaboutBonus(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT, myCooperativeParam)),
    myCooperativeSpeed(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_COOPERATIVE_SPEED, myCooperativeParam)),
    myOvertakeRightParam(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_OVERTAKE_RIGHT, 0)),
    myExperimentalParam1(v.getVehicleType().getParameter().getLCParam(SUMO_ATTR_LCA_EXPERIMENTAL1, 0)) {
    initDerivedParameters();
#ifdef DEBUG_CONSTRUCTOR
    if (DEBUG_COND) {
        std::cout << SIMTIME
                  << " create lcModel veh=" << myVehicle.getID()
                  << " lcStrategic=" << myStrategicParam
                  << " lcCooperative=" << myCooperativeParam
                  << " lcSpeedGain=" << mySpeedGainParam
                  << " lcKeepRight=" << myKeepRightParam
                  << "\n";
    }
#endif
}

MSLCM_LC2013::~MSLCM_LC2013() {
    changed();
}


void
MSLCM_LC2013::initDerivedParameters() {
    myChangeProbThresholdRight = (0.2 / mySpeedGainRight) / MAX2(NUMERICAL_EPS, mySpeedGainParam);
    myChangeProbThresholdLeft = 0.2 / MAX2(NUMERICAL_EPS, mySpeedGainParam);
}


bool
MSLCM_LC2013::debugVehicle() const {
    return DEBUG_COND;
}


int
MSLCM_LC2013::wantsChange(
    int laneOffset,
    MSAbstractLaneChangeModel::MSLCMessager& msgPass,
    int blocked,
    const std::pair<MSVehicle*, double>& leader,
    const std::pair<MSVehicle*, double>& neighLead,
    const std::pair<MSVehicle*, double>& neighFollow,
    const MSLane& neighLane,
    const std::vector<MSVehicle::LaneQ>& preb,
    MSVehicle** lastBlocked,
    MSVehicle** firstBlocked) {

#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << "\nWANTS_CHANGE\n" << SIMTIME
                  << std::setprecision(gPrecision)
                  << " veh=" << myVehicle.getID()
                  << " lane=" << myVehicle.getLane()->getID()
                  << " pos=" << myVehicle.getPositionOnLane()
                  << " posLat=" << myVehicle.getLateralPositionOnLane()
                  << " speed=" << myVehicle.getSpeed()
                  << " considerChangeTo=" << (laneOffset == -1  ? "right" : "left")
                  << "\n";
    }
#endif

    const int result = _wantsChange(laneOffset, msgPass, blocked, leader, neighLead, neighFollow, neighLane, preb, lastBlocked, firstBlocked);

#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << myVehicle.getID() << " result=" << toString((LaneChangeAction)result) << " blocked=" << toString((LaneChangeAction)blocked) << "\n\n\n";
    }
#endif

    return result;
}


double
MSLCM_LC2013::patchSpeed(const double min, const double wanted, const double max, const MSCFModel& cfModel) {

#ifdef DEBUG_PATCH_SPEED
    if (DEBUG_COND) {
        std::cout << "\nPATCH_SPEED\n"
                  << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " lane=" << myVehicle.getLane()->getID()
                  << " pos=" << myVehicle.getPositionOnLane()
                  << " v=" << myVehicle.getSpeed()
                  << " min=" << min
                  << " wanted=" << wanted
                  << " max=" << max
                  << "\n";
    }
#endif

    // negative min speed may be passed when using ballistic updated
    const double newSpeed = _patchSpeed(MAX2(min, 0.0), wanted, max, cfModel);

#ifdef DEBUG_PATCH_SPEED
    if (DEBUG_COND) {
        const std::string patched = (wanted != newSpeed ? " patched=" + toString(newSpeed) : "");
        std::cout << patched
                  << "\n";
    }
#endif

    return newSpeed;
}


double
MSLCM_LC2013::_patchSpeed(const double min, const double wanted, const double max, const MSCFModel& cfModel) {
    int state = myOwnState;
#ifdef DEBUG_PATCH_SPEED
    if (DEBUG_COND) {
        std::cout
                << "\n" << SIMTIME << std::setprecision(gPrecision)
                << " patchSpeed state=" << toString((LaneChangeAction)state) << " myLCAccelerationAdvices=" << toString(myLCAccelerationAdvices)
                << " \nspeed=" << myVehicle.getSpeed()
                << " min=" << min
                << " wanted=" << wanted << std::endl;
    }
#endif

    // letting vehicles merge in at the end of the lane in case of counter-lane change, step#2
    double MAGIC_offset = 1.;
    double nVSafe = wanted;
    bool gotOne = false;
    //   if we want to change and have a blocking leader and there is enough room for him in front of us
    if (myLeadingBlockerLength != 0) {
        double space = myLeftSpace - myLeadingBlockerLength - MAGIC_offset - myVehicle.getVehicleType().getMinGap();
#ifdef DEBUG_PATCH_SPEED
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " myLeadingBlockerLength=" << myLeadingBlockerLength << " space=" << space << "\n";
        }
#endif
        if (space > 0) { // XXX space > -MAGIC_offset
            // compute speed for decelerating towards a place which allows the blocking leader to merge in in front
            double safe = cfModel.stopSpeed(&myVehicle, myVehicle.getSpeed(), space);
            // if we are approaching this place
            if (safe < wanted) {
                // return this speed as the speed to use
#ifdef DEBUG_PATCH_SPEED
                if (DEBUG_COND) {
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
        double a = (*i);
        double v = myVehicle.getSpeed() + ACCEL2SPEED(a);

        if (v >= min && v <= max && (MSGlobals::gSemiImplicitEulerUpdate
                                     // ballistic update: (negative speeds may appear, e.g. min<0, v<0), BUT:
                                     // XXX: LaneChanging returns -1 to indicate no restrictions, which leads to probs here (Leo), refs. #2577
                                     //      As a quick fix, we just dismiss cases where v=-1
                                     //      VERY rarely (whenever a requested help-acceleration is really indicated by v=-1)
                                     //      this can lead to failing lane-change attempts, though)
                                     || v != -1)) {
            nVSafe = MIN2(v * coopWeight + (1 - coopWeight) * wanted, nVSafe);
            gotOne = true;
#ifdef DEBUG_PATCH_SPEED
            if (DEBUG_COND) {
                std::cout << SIMTIME << " veh=" << myVehicle.getID() << " got nVSafe=" << nVSafe << "\n";
            }
#endif
        } else {
            if (v < min) {
#ifdef DEBUG_PATCH_SPEED
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " ignoring low nVSafe=" << v << " min=" << min << "\n";
                }
#endif
            } else {
#ifdef DEBUG_PATCH_SPEED
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " ignoring high nVSafe=" << v << " max=" << max << "\n";
                }
#endif
            }
        }
    }

    if (gotOne && !myDontBrake) { // XXX: myDontBrake is initialized as false and seems not to be changed anywhere... What's its purpose???
#ifdef DEBUG_PATCH_SPEED
        if (DEBUG_COND) {
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
#ifdef DEBUG_PATCH_SPEED
            if (DEBUG_COND) {
                std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_WANTS_LANECHANGE (strat, no vSafe)\n";
            }
#endif
            return (max + wanted) /  2.0;
        } else if ((state & LCA_COOPERATIVE) != 0) {
            // only minor adjustments in speed should be done
            if ((state & LCA_BLOCKED_BY_LEADER) != 0) {
#ifdef DEBUG_PATCH_SPEED
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_BLOCKED_BY_LEADER (coop)\n";
                }
#endif
                if (wanted >= 0.) {
                    return (MAX2(0., min) + wanted) /  2.0;
                } else {
                    return wanted;
                }
            }
            if ((state & LCA_BLOCKED_BY_FOLLOWER) != 0) {
#ifdef DEBUG_PATCH_SPEED
                if (DEBUG_COND) {
                    std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_BLOCKED_BY_FOLLOWER (coop)\n";
                }
#endif
                return (max + wanted) /  2.0;
            }
            //} else { // VARIANT_16
            //    // only accelerations should be performed
            //    if ((state & LCA_BLOCKED_BY_FOLLOWER) != 0) {
            //        if (gDebugFlag2) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_BLOCKED_BY_FOLLOWER\n";
            //        return (max + wanted) /  2.0;
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
        return (min + wanted) /  2.0;
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
#ifdef DEBUG_PATCH_SPEED
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBLOCKINGLEADER\n";
        }
#endif
        return (max + wanted) /  2.0;
    }

    if ((state & LCA_AMBLOCKINGFOLLOWER_DONTBRAKE) != 0) {
#ifdef DEBUG_PATCH_SPEED
        if (DEBUG_COND) {
            std::cout << SIMTIME << " veh=" << myVehicle.getID() << " LCA_AMBLOCKINGFOLLOWER_DONTBRAKE\n";
        }
#endif
        /*
        // VARIANT_4 (dontbrake)
        if (max <= myVehicle.getCarFollowModel().maxNextSpeed(myVehicle.getSpeed(), &myVehicle) && min == 0) { // !!! was standing
            return wanted;
        }
        return (min + wanted) /  2.0;
        */
    }
    if (!myVehicle.getLane()->getEdge().hasLaneChanger()) {
        // remove chaning information if on a road with a single lane
        changed();
    }
    return wanted;
}


void*
MSLCM_LC2013::inform(void* info, MSVehicle* sender) {
    UNUSED_PARAMETER(sender);
    Info* pinfo = (Info*)info;
    assert(pinfo->first >= 0 || !MSGlobals::gSemiImplicitEulerUpdate);
    addLCSpeedAdvice(pinfo->first);
    myOwnState |= pinfo->second;
#ifdef DEBUG_INFORMED
    if (DEBUG_COND) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " informedBy=" << sender->getID()
                  << " info=" << pinfo->second
                  << " vSafe=" << pinfo->first
                  << "\n";
    }
#endif
    delete pinfo;
    return (void*) true;
}

double
MSLCM_LC2013::overtakeDistance(const MSVehicle* follower, const MSVehicle* leader, const double gap, double followerSpeed, double leaderSpeed) {
    followerSpeed = followerSpeed == INVALID_SPEED ? follower->getSpeed() : followerSpeed;
    leaderSpeed = leaderSpeed == INVALID_SPEED ? leader->getSpeed() : leaderSpeed;
    double overtakeDist = (gap // drive to back of leader
                           + leader->getVehicleType().getLengthWithGap() // drive to front of leader
                           + follower->getVehicleType().getLength() // follower back reaches leader front
                           + leader->getCarFollowModel().getSecureGap( // save gap to leader
                               leader, follower, leaderSpeed, followerSpeed, follower->getCarFollowModel().getMaxDecel()));
    return MAX2(overtakeDist, 0.);
}


double
MSLCM_LC2013::informLeader(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                           int blocked,
                           int dir,
                           const std::pair<MSVehicle*, double>& neighLead,
                           double remainingSeconds) {
    double plannedSpeed = MIN2(myVehicle.getSpeed(),
                               myVehicle.getCarFollowModel().stopSpeed(&myVehicle, myVehicle.getSpeed(), myLeftSpace - myLeadingBlockerLength));
    for (std::vector<double>::const_iterator i = myLCAccelerationAdvices.begin(); i != myLCAccelerationAdvices.end(); ++i) {
        const double a = *i;
        if (a >= -myVehicle.getCarFollowModel().getMaxDecel()) {
            plannedSpeed = MIN2(plannedSpeed, myVehicle.getSpeed() + ACCEL2SPEED(a));
        }
    }
#ifdef DEBUG_INFORMER
    if (DEBUG_COND) {
        std::cout << "\nINFORM_LEADER"
                  << "\nspeed=" <<  myVehicle.getSpeed() << " planned=" << plannedSpeed << "\n";
    }
#endif

    const MSVehicle* const nv = neighLead.first;
    if (nv == nullptr) {
        // not overtaking
        return plannedSpeed;
    }
    const double neighNextSpeed = nv->getSpeed() - ACCEL2SPEED(MAX2(1.0, -nv->getAcceleration()));
    double neighNextGap;
    if (MSGlobals::gSemiImplicitEulerUpdate) {
        neighNextGap = neighLead.second + SPEED2DIST(neighNextSpeed - plannedSpeed);
    } else {
        neighNextGap = neighLead.second + SPEED2DIST((nv->getSpeed() + neighNextSpeed) / 2) - SPEED2DIST((myVehicle.getSpeed() + plannedSpeed) / 2);
    }
    if ((blocked & LCA_BLOCKED_BY_LEADER) != 0) {
#ifdef DEBUG_INFORMER
        if (DEBUG_COND) {
            std::cout << " blocked by leader nv=" <<  nv->getID() << " nvSpeed=" << nv->getSpeed() << " needGap="
                      << myVehicle.getCarFollowModel().getSecureGap(&myVehicle, nv, myVehicle.getSpeed(), nv->getSpeed(), nv->getCarFollowModel().getMaxDecel()) << "\n";
        }
#endif
        // decide whether we want to overtake the leader or follow it
        double overtakeTime;
        const double overtakeDist = overtakeDistance(&myVehicle, nv, neighLead.second);
        const double dv = plannedSpeed - nv->getSpeed();

        if (dv > 0) {
            overtakeTime = overtakeDist / dv;
        } else {
            // -> set overtakeTime to something indicating impossibility of overtaking
            overtakeTime = remainingSeconds + 1;
        }

#ifdef DEBUG_INFORMER
        if (DEBUG_COND) {
            std::cout << SIMTIME << " informLeader() of " << myVehicle.getID()
                      << "\nnv = " << nv->getID()
                      << "\nplannedSpeed = " << plannedSpeed
                      << "\nleaderSpeed = " << nv->getSpeed()
                      << "\nmyLeftSpace = " << myLeftSpace
                      << "\nremainingSeconds = " << remainingSeconds
                      << "\novertakeDist = " << overtakeDist
                      << "\novertakeTime = " << overtakeTime
                      << std::endl;
        }
#endif

        if ((dv < 0
                // overtaking on the right on an uncongested highway is forbidden (noOvertakeLCLeft)
                || (dir == LCA_MLEFT && !myVehicle.congested() && !myAllowOvertakingRight)
                // not enough space to overtake?
                || (MSGlobals::gSemiImplicitEulerUpdate && myLeftSpace - myLeadingBlockerLength - myVehicle.getCarFollowModel().brakeGap(myVehicle.getSpeed()) < overtakeDist)
                // using brakeGap() without headway seems adequate in a situation where the obstacle (the lane end) is not moving [XXX implemented in branch ticket860, can be used in general if desired, refs. #2575] (Leo).
                || (!MSGlobals::gSemiImplicitEulerUpdate && myLeftSpace - myLeadingBlockerLength - myVehicle.getCarFollowModel().brakeGap(myVehicle.getSpeed(), myCarFollowModel.getMaxDecel(), 0.) < overtakeDist)
                // not enough time to overtake?        (skipped for a stopped leader [currently only for ballistic update XXX: check if appropriate for euler, too, refs. #2575] to ensure that it can be overtaken if only enough space is exists) (Leo)
                || (remainingSeconds < overtakeTime && (MSGlobals::gSemiImplicitEulerUpdate || !nv->isStopped())))
                // opposite driving and must overtake
                && !(isOpposite() && neighLead.second < 0 && neighLead.first->isStopped())) {
            // cannot overtake
            msgPass.informNeighLeader(new Info(std::numeric_limits<double>::max(), dir | LCA_AMBLOCKINGLEADER), &myVehicle);
            // slow down smoothly to follow leader
            // account for minor decelerations by the leader (dawdling)
            const double targetSpeed = MAX2(
                                           myVehicle.getCarFollowModel().minNextSpeed(myVehicle.getSpeed(), &myVehicle),
                                           myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighNextGap, neighNextSpeed, nv->getCarFollowModel().getMaxDecel()));
            if (targetSpeed < myVehicle.getSpeed()) {
                // slow down smoothly to follow leader
                const double decel = remainingSeconds == 0. ? myVehicle.getCarFollowModel().getMaxDecel() :
                                     MIN2(myVehicle.getCarFollowModel().getMaxDecel(),
                                          MAX2(MIN_FALLBEHIND, (myVehicle.getSpeed() - targetSpeed) / remainingSeconds));
                const double nextSpeed = MIN2(plannedSpeed, MAX2(0.0, myVehicle.getSpeed() - ACCEL2SPEED(decel)));
#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    std::cout << SIMTIME
                              << " cannot overtake leader nv=" << nv->getID()
                              << " dv=" << dv
                              << " myLookAheadSpeed=" << myLookAheadSpeed
                              << " myLeftSpace=" << myLeftSpace
                              << " overtakeDist=" << overtakeDist
                              << " overtakeTime=" << overtakeTime
                              << " remainingSeconds=" << remainingSeconds
                              << " currentGap=" << neighLead.second
                              << " brakeGap=" << myVehicle.getCarFollowModel().brakeGap(myVehicle.getSpeed(), myCarFollowModel.getMaxDecel(), 0.)
                              << " neighNextSpeed=" << neighNextSpeed
                              << " neighNextGap=" << neighNextGap
                              << " targetSpeed=" << targetSpeed
                              << " nextSpeed=" << nextSpeed
                              << "\n";
                }
#endif
                addLCSpeedAdvice(nextSpeed);
                return nextSpeed;
            } else {
                // leader is fast enough anyway
#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    std::cout << SIMTIME
                              << " cannot overtake fast leader nv=" << nv->getID()
                              << " dv=" << dv
                              << " myLookAheadSpeed=" << myLookAheadSpeed
                              << " myLeftSpace=" << myLeftSpace
                              << " overtakeDist=" << overtakeDist
                              << " myLeadingBlockerLength=" << myLeadingBlockerLength
                              << " overtakeTime=" << overtakeTime
                              << " remainingSeconds=" << remainingSeconds
                              << " currentGap=" << neighLead.second
                              << " targetSpeed=" << targetSpeed
                              << "\n";
                }
#endif
                addLCSpeedAdvice(targetSpeed);
                return plannedSpeed;
            }
        } else {
            // overtaking, leader should not accelerate
#ifdef DEBUG_INFORMER
            if (DEBUG_COND) {
                std::cout << SIMTIME
                          << " wants to overtake leader nv=" << nv->getID()
                          << " dv=" << dv
                          << " overtakeDist=" << overtakeDist
                          << " remainingSeconds=" << remainingSeconds
                          << " overtakeTime=" << overtakeTime
                          << " currentGap=" << neighLead.second
                          << " secureGap=" << nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel())
                          << "\n";
            }
#endif
            msgPass.informNeighLeader(new Info(nv->getSpeed(), dir | LCA_AMBLOCKINGLEADER), &myVehicle);
            return -1;  // XXX: using -1 is ambiguous for the ballistic update! Currently this is being catched in patchSpeed() (Leo), consider returning INVALID_SPEED, refs. #2577
        }
    } else { // (remainUnblocked)
        // we are not blocked now. make sure we stay far enough from the leader
        const double targetSpeed = MAX2(
                                       myVehicle.getCarFollowModel().minNextSpeed(myVehicle.getSpeed(), &myVehicle),
                                       myCarFollowModel.followSpeed(&myVehicle, myVehicle.getSpeed(), neighNextGap, neighNextSpeed, nv->getCarFollowModel().getMaxDecel()));
        addLCSpeedAdvice(targetSpeed);
#ifdef DEBUG_INFORMER
        if (DEBUG_COND) {
            std::cout << " not blocked by leader nv=" <<  nv->getID()
                      << " nvSpeed=" << nv->getSpeed()
                      << " gap=" << neighLead.second
                      << " neighNextSpeed=" << neighNextSpeed
                      << " neighNextGap=" << neighNextGap
                      << " needGap=" << myVehicle.getCarFollowModel().getSecureGap(&myVehicle, nv, myVehicle.getSpeed(), nv->getSpeed(), nv->getCarFollowModel().getMaxDecel())
                      << " targetSpeed=" << targetSpeed
                      << "\n";
        }
#endif
        return MIN2(targetSpeed, plannedSpeed);
    }
}

void
MSLCM_LC2013::informFollower(MSAbstractLaneChangeModel::MSLCMessager& msgPass,
                             int blocked,
                             int dir,
                             const std::pair<MSVehicle*, double>& neighFollow,
                             double remainingSeconds,
                             double plannedSpeed) {

    MSVehicle* nv = neighFollow.first;
    const double plannedAccel = SPEED2ACCEL(MAX2(MIN2(myCarFollowModel.getMaxAccel(), plannedSpeed - myVehicle.getSpeed()), -myCarFollowModel.getMaxDecel()));

#ifdef DEBUG_INFORMER
    if (DEBUG_COND) {
        std::cout << "\nINFORM_FOLLOWER"
                  << "\nspeed=" <<  myVehicle.getSpeed() << " planned=" << plannedSpeed << "\n";
    }

#endif
    if ((blocked & LCA_BLOCKED_BY_FOLLOWER) != 0 && nv != nullptr) {
#ifdef DEBUG_INFORMER
        if (DEBUG_COND) {
            std::cout << " blocked by follower nv=" <<  nv->getID() << " nvSpeed=" << nv->getSpeed() << " needGap="
                      << nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel()) << " planned=" << plannedSpeed <<  "\n";
        }
#endif

        // are we fast enough to cut in without any help?
        if (MAX2(plannedSpeed, 0.) - nv->getSpeed() >= HELP_OVERTAKE) {
            const double neededGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
            if ((neededGap - neighFollow.second) / remainingSeconds < (MAX2(plannedSpeed, 0.) - nv->getSpeed())) {
#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    std::cout << " wants to cut in before  nv=" << nv->getID() << " without any help." << "\nneededGap = " << neededGap << "\n";
                }
#endif
                // follower might even accelerate but not to much
                // XXX: I don't understand this. The needed gap was determined for nv->getSpeed(), not for (plannedSpeed - HELP_OVERTAKE)?! (Leo), refs. #2578
                msgPass.informNeighFollower(new Info(MAX2(plannedSpeed, 0.) - HELP_OVERTAKE, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
                return;
            }
        }

        // decide whether we will request help to cut in before the follower or allow to be overtaken

        // PARAMETERS
        // assume other vehicle will assume the equivalent of 1 second of
        // maximum deceleration to help us (will probably be spread over
        // multiple seconds)
        // -----------
        const double helpDecel = nv->getCarFollowModel().getMaxDecel() * HELP_DECEL_FACTOR;

        // follower's new speed in next step
        double neighNewSpeed;
        // follower's new speed after 1s.
        double neighNewSpeed1s;
        // velocity difference, gap after follower-deceleration
        double dv, decelGap;

        if (MSGlobals::gSemiImplicitEulerUpdate) {
            // euler
            neighNewSpeed = MAX2(0., nv->getSpeed() - ACCEL2SPEED(helpDecel));
            neighNewSpeed1s = MAX2(0., nv->getSpeed() - helpDecel); // TODO: consider introduction of a configurable anticipationTime here (see far below in the !blocked part). Refs. #2578
            // change in the gap between ego and blocker over 1 second (not STEP!)
            // XXX: though here it is calculated as if it were one step!? (Leo) Refs. #2578
            dv = plannedSpeed - neighNewSpeed1s; // XXX: what is this quantity (if TS!=1)?
            // new gap between follower and self in case the follower does brake for 1s
            // XXX: if the step-length is not 1s., this is not the gap after 1s. deceleration!
            //      And this formula overestimates the real gap. Isn't that problematic? (Leo)
            //      Below, it seems that decelGap > secureGap is taken to indicate the possibility
            //      to cut in within the next time-step. However, this is not the case, if TS<1s.,
            //      since decelGap is (not exactly, though!) the gap after 1s. Refs. #2578
            decelGap = neighFollow.second + dv;
        } else {
            // ballistic
            // negative newSpeed-extrapolation possible, if stop lies within the next time-step
            // XXX: this code should work for the euler case as well, since gapExtrapolation() takes
            //      care of this, but for TS!=1 we will have different behavior (see previous remark) Refs. #2578
            neighNewSpeed = nv->getSpeed() - ACCEL2SPEED(helpDecel);
            neighNewSpeed1s = nv->getSpeed() - helpDecel;

            dv = myVehicle.getSpeed() - nv->getSpeed(); // current velocity difference
            decelGap = myCarFollowModel.gapExtrapolation(1., neighFollow.second, myVehicle.getSpeed(),
                       nv->getSpeed(), plannedAccel, -helpDecel, myVehicle.getMaxSpeedOnLane(), nv->getMaxSpeedOnLane());
        }

        const double secureGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, MAX2(neighNewSpeed1s, 0.),
                                 MAX2(plannedSpeed, 0.), myVehicle.getCarFollowModel().getMaxDecel());

        const double onRampThreshold = myVehicle.getLane()->getSpeedLimit() * 0.8 * myExperimentalParam1 * (1 - myVehicle.getImpatience());

#ifdef DEBUG_INFORMER
        if (DEBUG_COND) {
            std::cout << SIMTIME
                      << " speed=" << myVehicle.getSpeed()
                      << " plannedSpeed=" << plannedSpeed
                      << " threshold=" << onRampThreshold
                      << " neighNewSpeed=" << neighNewSpeed
                      << " neighNewSpeed1s=" << neighNewSpeed1s
                      << " dv=" << dv
                      << " gap=" << neighFollow.second
                      << " decelGap=" << decelGap
                      << " secureGap=" << secureGap
                      << "\n";
        }
#endif
        // prevent vehicles on an on ramp stopping the main flow
        if (dir == LCA_MLEFT
                && myVehicle.getLane()->isAccelLane()
                && neighNewSpeed1s < onRampThreshold) {
            return;
        }

        if (decelGap > 0 && decelGap >= secureGap) {
            // XXX: This does not assure that the leader can cut in in the next step if TS < 1 (see above)
            //      this seems to be supposed in the following (euler code)...?! (Leo) Refs. #2578

            // if the blocking follower brakes it could help
            // how hard does it actually need to be?
            // to be safe in the next step the following equation has to hold for the follower's vsafe:
            //   vsafe <= followSpeed(gap=currentGap - SPEED2DIST(vsafe), ...)
            double vsafe, vsafe1;

            if (MSGlobals::gSemiImplicitEulerUpdate) {
                // euler
                // we compute an upper bound on vsafe by doing the computation twice
                vsafe1 = MAX2(neighNewSpeed, nv->getCarFollowModel().followSpeed(
                                  nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed), plannedSpeed, myCarFollowModel.getMaxDecel()));
                vsafe = MAX2(neighNewSpeed, nv->getCarFollowModel().followSpeed(
                                 nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed - vsafe1), plannedSpeed, myCarFollowModel.getMaxDecel()));
                assert(vsafe <= vsafe1);
            } else {
                // ballistic

                // XXX: This block should actually do as well for euler update (TODO: test!), refs #2575
                // we compute an upper bound on vsafe
                // next step's gap without help deceleration (nv's speed assumed constant)
                double nextGap = myCarFollowModel.gapExtrapolation(TS,
                                 neighFollow.second, myVehicle.getSpeed(),
                                 nv->getSpeed(), plannedAccel, 0,
                                 myVehicle.getMaxSpeedOnLane(), nv->getMaxSpeedOnLane());
#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    std::cout << "nextGap=" << nextGap << " (without help decel) \n";
                }
#endif

                // NOTE: the second argument of MIN2() can get larger than nv->getSpeed()
                vsafe1 = MIN2(nv->getSpeed(), MAX2(neighNewSpeed,
                                                   nv->getCarFollowModel().followSpeed(nv,
                                                           nv->getSpeed(), nextGap,
                                                           MAX2(0., plannedSpeed),
                                                           myCarFollowModel.getMaxDecel())));


                // next step's gap with possibly less than maximal help deceleration (in case vsafe1 > neighNewSpeed)
                double decel2 = SPEED2ACCEL(nv->getSpeed() - vsafe1);
                nextGap = myCarFollowModel.gapExtrapolation(TS,
                          neighFollow.second, myVehicle.getSpeed(),
                          nv->getSpeed(), plannedAccel, -decel2,
                          myVehicle.getMaxSpeedOnLane(), nv->getMaxSpeedOnLane());

                // vsafe = MAX(neighNewSpeed, safe speed assuming next_gap)
                // Thus, the gap resulting from vsafe is larger or equal to next_gap
                // in contrast to the euler case, where nv's follow speed doesn't depend on the actual speed,
                // we need to assure, that nv doesn't accelerate
                vsafe = MIN2(nv->getSpeed(), MAX2(neighNewSpeed,
                                                  nv->getCarFollowModel().followSpeed(nv,
                                                          nv->getSpeed(), nextGap,
                                                          MAX2(0., plannedSpeed),
                                                          myCarFollowModel.getMaxDecel())));

                assert(vsafe >= vsafe1 - NUMERICAL_EPS);

#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    std::cout << "nextGap=" << nextGap
                              << " (with vsafe1 and help decel) \nvsafe1=" << vsafe1
                              << " vsafe=" << vsafe
                              << "\n";
                }
#endif

                // For subsecond simulation, this might not lead to secure gaps for a long time,
                // we seek to establish a secure gap as soon as possible
                double nextSecureGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, vsafe, plannedSpeed, myCarFollowModel.getMaxDecel());

                if (nextGap < nextSecureGap) {
                    // establish a secureGap as soon as possible
                    vsafe = neighNewSpeed;
                }

#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    std::cout << "nextGap=" << nextGap
                              << " minNextSecureGap=" << nextSecureGap
                              << " vsafe=" << vsafe << "\n";
                }
#endif

            }
            msgPass.informNeighFollower(
                new Info(vsafe, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);

#ifdef DEBUG_INFORMER
            if (DEBUG_COND) {
                std::cout << " wants to cut in before nv=" << nv->getID()
                          << " vsafe1=" << vsafe1 << " vsafe=" << vsafe
                          << " newSecGap="
                          << nv->getCarFollowModel().getSecureGap(nv, &myVehicle, vsafe,
                                  plannedSpeed,
                                  myVehicle.getCarFollowModel().getMaxDecel())
                          << "\n";
            }
#endif
        } else if ((MSGlobals::gSemiImplicitEulerUpdate && dv > 0 && dv * remainingSeconds > (secureGap - decelGap + POSITION_EPS))
                   || (!MSGlobals::gSemiImplicitEulerUpdate && dv > 0 && dv * (remainingSeconds - 1) > secureGap - decelGap + POSITION_EPS)
                  ) {

            // XXX: Alternative formulation (encapsulating differences of euler and ballistic) TODO: test, refs. #2575
            // double eventualGap = myCarFollowModel.gapExtrapolation(remainingSeconds - 1., decelGap, plannedSpeed, neighNewSpeed1s);
            // } else if (eventualGap > secureGap + POSITION_EPS) {


            // NOTE: This case corresponds to the situation, where some time is left to perform the lc
            // For the ballistic case this is interpreted as follows:
            // If the follower breaks with helpDecel for one second, this vehicle maintains the plannedSpeed,
            // and both continue with their speeds for remainingSeconds seconds the gap will suffice for a laneChange
            // For the euler case we had the following comment:
            // 'decelerating once is sufficient to open up a large enough gap in time', but:
            // XXX: 1) Decelerating *once* does not necessarily lead to the gap decelGap! (if TS<1s.) (Leo)
            //      2) Probably, the if() for euler should test for dv * (remainingSeconds-1) > ..., too ?!, refs. #2578
            msgPass.informNeighFollower(new Info(neighNewSpeed, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
#ifdef DEBUG_INFORMER
            if (DEBUG_COND) {
                std::cout << " wants to cut in before nv=" << nv->getID() << " (eventually)\n";
            }
#endif
        } else if (dir == LCA_MRIGHT && !myAllowOvertakingRight && !nv->congested()) {
            // XXX: check if this requires a special treatment for the ballistic update, refs. #2575
            const double vhelp = MAX2(neighNewSpeed, HELP_OVERTAKE);
            msgPass.informNeighFollower(new Info(vhelp, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
#ifdef DEBUG_INFORMER
            if (DEBUG_COND) {
                std::cout << " wants to cut in before nv=" << nv->getID() << " (nv cannot overtake right)\n";
            }
#endif
        } else {
            double vhelp = MAX2(nv->getSpeed(), myVehicle.getSpeed() + HELP_OVERTAKE);
            //if (dir == LCA_MRIGHT && myVehicle.getWaitingSeconds() > LCA_RIGHT_IMPATIENCE &&
            //        nv->getSpeed() > myVehicle.getSpeed()) {
            if (nv->getSpeed() > myVehicle.getSpeed() &&
                    ((dir == LCA_MRIGHT && myVehicle.getWaitingSeconds() > LCA_RIGHT_IMPATIENCE) // NOTE: it might be considered to use myVehicle.getAccumulatedWaitingSeconds() > LCA_RIGHT_IMPATIENCE instead (Leo). Refs. #2578
                     || (dir == LCA_MLEFT && plannedSpeed > CUT_IN_LEFT_SPEED_THRESHOLD) // VARIANT_22 (slowDownLeft)
                     // XXX this is a hack to determine whether the vehicles is on an on-ramp. This information should be retrieved from the network itself
                     || (dir == LCA_MLEFT && myVehicle.getLane()->getLength() > MAX_ONRAMP_LENGTH)
                    )) {
                // let the follower slow down to increase the likelihood that later vehicles will be slow enough to help
                // follower should still be fast enough to open a gap
                // XXX: The probability for that success would be larger if the slow down of the appropriate following vehicle
                //      would take place without the immediate follower slowing down. We might consider to model reactions of
                //      vehicles that are not immediate followers. (Leo) -> see ticket #2532
                vhelp = MAX2(neighNewSpeed, myVehicle.getSpeed() + HELP_OVERTAKE);
#ifdef DEBUG_INFORMER
                if (DEBUG_COND) {
                    // NOTE: the condition labeled "VARIANT_22" seems to imply that this could as well concern the *left* follower?! (Leo)
                    //       Further, vhelp might be larger than nv->getSpeed(), so the request issued below is not to slow down!? (see below) Refs. #2578
                    std::cout << " wants right follower to slow down a bit\n";
                }
#endif
                if (MSGlobals::gSemiImplicitEulerUpdate) {
                    // euler
                    if ((nv->getSpeed() - myVehicle.getSpeed()) / helpDecel < remainingSeconds) {

#ifdef DEBUG_INFORMER
                        if (DEBUG_COND) {
                            // NOTE: the condition labeled "VARIANT_22" seems to imply that this could as well concern the *left* follower?! Refs. #2578
                            std::cout << " wants to cut in before right follower nv=" << nv->getID() << " (eventually)\n";
                        }
#endif
                        // XXX: I don't understand. This vhelp might be larger than nv->getSpeed() but the above condition seems to rely
                        //      on the reasoning that if nv breaks with helpDecel for remaining Seconds, nv will be so slow, that this
                        //      vehicle will be able to cut in. But nv might have overtaken this vehicle already (or am I missing sth?). (Leo)
                        //      Ad: To my impression, the intention behind allowing larger speeds for the blocking follower is to prevent a
                        //      situation, where an overlapping follower keeps blocking the ego vehicle. Refs. #2578
                        msgPass.informNeighFollower(new Info(vhelp, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
                        return;
                    }
                } else {

                    // ballistic (this block is a bit different to the logic in the euler part, but in general suited to work on euler as well.. must be tested <- TODO, refs. #2575)
                    // estimate gap after remainingSeconds.
                    // Assumptions:
                    // (A1) leader continues with currentSpeed. (XXX: That might be wrong: Think of accelerating on an on-ramp or of a congested region ahead!)
                    // (A2) follower breaks with helpDecel.
                    const double gapAfterRemainingSecs = myCarFollowModel.gapExtrapolation(
                            remainingSeconds, neighFollow.second, myVehicle.getSpeed(), nv->getSpeed(), 0, -helpDecel, myVehicle.getMaxSpeedOnLane(), nv->getMaxSpeedOnLane());
                    const double secureGapAfterRemainingSecs = nv->getCarFollowModel().getSecureGap(nv, &myVehicle,
                            MAX2(nv->getSpeed() - remainingSeconds * helpDecel, 0.), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel());
                    if (gapAfterRemainingSecs >= secureGapAfterRemainingSecs) { // XXX: here it would be wise to check whether there is enough space for eventual braking if the maneuver doesn't succeed
#ifdef DEBUG_INFORMER
                        if (DEBUG_COND) {
                            std::cout << " wants to cut in before follower nv=" << nv->getID() << " (eventually)\n";
                        }
#endif
                        // NOTE: ballistic uses neighNewSpeed instead of vhelp, see my note above. (Leo)
                        // TODO: recheck if this might cause suboptimal behaviour in some LC-situations. Refs. #2578
                        msgPass.informNeighFollower(new Info(neighNewSpeed, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
                        return;
                    }
                }


            }

#ifdef DEBUG_INFORMER
            if (DEBUG_COND) {
                std::cout << SIMTIME
                          << " veh=" << myVehicle.getID()
                          << " informs follower " << nv->getID()
                          << " vhelp=" << vhelp
                          << "\n";
            }
#endif

            msgPass.informNeighFollower(new Info(vhelp, dir | LCA_AMBLOCKINGFOLLOWER), &myVehicle);
            // This follower is supposed to overtake us. Slow down smoothly to allow this.
            const double overtakeDist = overtakeDistance(nv, &myVehicle, neighFollow.second, vhelp, plannedSpeed);
            // speed difference to create a sufficiently large gap
            const double needDV = overtakeDist / remainingSeconds;
            // make sure the deceleration is not to strong (XXX: should be assured in finalizeSpeed -> TODO: remove the MAX2 if agreed) -> prob with possibly non-existing maximal deceleration for som CF Models(?) Refs. #2578
            addLCSpeedAdvice(MAX2(vhelp - needDV, myVehicle.getSpeed() - ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxDecel())));

#ifdef DEBUG_INFORMER
            if (DEBUG_COND) {
                std::cout << SIMTIME
                          << " veh=" << myVehicle.getID()
                          << " wants to be overtaken by=" << nv->getID()
                          << " overtakeDist=" << overtakeDist
                          << " vneigh=" << nv->getSpeed()
                          << " vhelp=" << vhelp
                          << " needDV=" << needDV
                          << " vsafe=" << myLCAccelerationAdvices.back()
                          << "\n";
            }
#endif
        }
    } else if (neighFollow.first != 0 && (blocked & LCA_BLOCKED_BY_LEADER)) {
        // we are not blocked by the follower now, make sure it remains that way
        // XXX: Does the below code for the euler case really assure that? Refs. #2578
        double vsafe, vsafe1;
        if (MSGlobals::gSemiImplicitEulerUpdate) {
            // euler
            MSVehicle* nv = neighFollow.first;
            vsafe1 = nv->getCarFollowModel().followSpeed(
                         nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
            vsafe = nv->getCarFollowModel().followSpeed(
                        nv, nv->getSpeed(), neighFollow.second + SPEED2DIST(plannedSpeed - vsafe1), plannedSpeed, myVehicle.getCarFollowModel().getMaxDecel());
            // NOTE: since vsafe1 > nv->getSpeed() is possible, we don't have vsafe1 < vsafe < nv->getSpeed here (similar pattern above works differently)

        } else {
            // ballistic
            // XXX This should actually do for euler and ballistic cases (TODO: test!) Refs. #2575

            double anticipationTime = 1.;
            double anticipatedSpeed =  MIN2(myVehicle.getSpeed() + plannedAccel * anticipationTime, myVehicle.getMaxSpeedOnLane());
            double anticipatedGap = myCarFollowModel.gapExtrapolation(anticipationTime, neighFollow.second, myVehicle.getSpeed(), nv->getSpeed(),
                                    plannedAccel, 0, myVehicle.getMaxSpeedOnLane(), nv->getMaxSpeedOnLane());
            double secureGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), anticipatedSpeed, myCarFollowModel.getMaxDecel());

            // propose follower speed corresponding to first estimation of gap
            vsafe = nv->getCarFollowModel().followSpeed(
                        nv, nv->getSpeed(), anticipatedGap, plannedSpeed, myCarFollowModel.getMaxDecel());
            double helpAccel = SPEED2ACCEL(vsafe - nv->getSpeed()) / anticipationTime;

            if (anticipatedGap > secureGap) {
                // follower may accelerate, implying vhelp >= vsafe >= nv->getSpeed()
                // calculate gap for the assumed acceleration
                anticipatedGap = myCarFollowModel.gapExtrapolation(anticipationTime, neighFollow.second, myVehicle.getSpeed(), nv->getSpeed(),
                                 plannedAccel, helpAccel, myVehicle.getMaxSpeedOnLane(), nv->getMaxSpeedOnLane());
                double anticipatedHelpSpeed = MIN2(nv->getSpeed() + anticipationTime * helpAccel, nv->getMaxSpeedOnLane());
                secureGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, anticipatedHelpSpeed, anticipatedSpeed, myCarFollowModel.getMaxDecel());
                if (anticipatedGap < secureGap) {
                    // don't accelerate
                    vsafe = nv->getSpeed();
                }
            } else {
                // follower is too fast, implying that vhelp <= vsafe <= nv->getSpeed()
                // we use the above vhelp
            }
        }
        msgPass.informNeighFollower(new Info(vsafe, dir), &myVehicle);

#ifdef DEBUG_INFORMER
        if (DEBUG_COND) {
            std::cout << " wants to cut in before non-blocking follower nv=" << nv->getID() << "\n";
        }
#endif
    }
}


void
MSLCM_LC2013::prepareStep() {
    MSAbstractLaneChangeModel::prepareStep();
    // keep information about strategic change direction
    if (!isChangingLanes()) {
        myOwnState = (myOwnState & LCA_STRATEGIC) ? (myOwnState & LCA_WANTS_LANECHANGE) : 0;
    }
    myLeadingBlockerLength = 0;
    myLeftSpace = 0;
    myLCAccelerationAdvices.clear();
    myDontBrake = false;
    // truncate to work around numerical instability between different builds
    mySpeedGainProbability = ceil(mySpeedGainProbability * 100000.0) * 0.00001;
    myKeepRightProbability = ceil(myKeepRightProbability * 100000.0) * 0.00001;
    if (mySigma > 0 && !isChangingLanes()) {
        // disturb lateral position directly
        const double oldPosLat = myVehicle.getLateralPositionOnLane();
        const double deltaPosLat = OUProcess::step(oldPosLat,
                                   myVehicle.getActionStepLengthSecs(),
                                   MAX2(NUMERICAL_EPS, (1 - mySigma) * 100), mySigma) - oldPosLat;
        const double scaledDelta = deltaPosLat * myVehicle.getSpeed() / myVehicle.getLane()->getSpeedLimit();
        myVehicle.setLateralPositionOnLane(oldPosLat + scaledDelta);
    }
}


void
MSLCM_LC2013::changed() {
    myOwnState = 0;
    mySpeedGainProbability = 0;
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
}


int
MSLCM_LC2013::_wantsChange(
    int laneOffset,
    MSAbstractLaneChangeModel::MSLCMessager& msgPass,
    int blocked,
    const std::pair<MSVehicle*, double>& leader,
    const std::pair<MSVehicle*, double>& neighLead,
    const std::pair<MSVehicle*, double>& neighFollow,
    const MSLane& neighLane,
    const std::vector<MSVehicle::LaneQ>& preb,
    MSVehicle** lastBlocked,
    MSVehicle** firstBlocked) {
    assert(laneOffset == 1 || laneOffset == -1);
    const SUMOTime currentTime = MSNet::getInstance()->getCurrentTimeStep();
    // compute bestLaneOffset
    MSVehicle::LaneQ curr, neigh, best;
    int bestLaneOffset = 0;
    // What do these "dists" mean? Please comment. (Leo) Ad: I now think the following:
    // currentDist is the distance that the vehicle can go on its route without having to
    // change lanes from the current lane. neighDist as currentDist for the considered target lane (i.e., neigh)
    // If this is true I suggest to put this into the docu of wantsChange()
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
            if (bestLaneOffset == 0 && preb[p + prebOffset].bestLaneOffset == 0) {
#ifdef DEBUG_WANTS_CHANGE
                if (DEBUG_COND) {
                    std::cout << STEPS2TIME(currentTime)
                              << " veh=" << myVehicle.getID()
                              << " bestLaneOffsetOld=" << bestLaneOffset
                              << " bestLaneOffsetNew=" << laneOffset
                              << "\n";
                }
#endif
                bestLaneOffset = prebOffset;
            }
            best = preb[p + bestLaneOffset];
            currIdx = p;
            break;
        }
    }
    // direction specific constants
    const bool right = (laneOffset == -1);
    if (isOpposite() && right) {
        neigh = preb[preb.size() - 1];
        curr = neigh;
        best = neigh;
        bestLaneOffset = -1;
        curr.bestLaneOffset = -1;
        neighDist = neigh.length;
        currentDist = curr.length;
    }
    double driveToNextStop = -std::numeric_limits<double>::max();
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
    const double posOnLane = isOpposite() ? myVehicle.getLane()->getOppositePos(myVehicle.getPositionOnLane()) : myVehicle.getPositionOnLane();
    const int lca = (right ? LCA_RIGHT : LCA_LEFT);
    const int myLca = (right ? LCA_MRIGHT : LCA_MLEFT);
    const int lcaCounter = (right ? LCA_LEFT : LCA_RIGHT);
    const bool changeToBest = (right && bestLaneOffset < 0) || (!right && bestLaneOffset > 0);
    // keep information about being a leader/follower
    int ret = (myOwnState & 0xffff0000);
    int req = 0; // the request to change or stay

    ret = slowDownForBlocked(lastBlocked, ret);
    if (lastBlocked != firstBlocked) {
        ret = slowDownForBlocked(firstBlocked, ret);
    }

#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " _wantsChange state=" << myOwnState
                  << " myLCAccelerationAdvices=" << toString(myLCAccelerationAdvices)
                  << " firstBlocked=" << Named::getIDSecure(*firstBlocked)
                  << " lastBlocked=" << Named::getIDSecure(*lastBlocked)
                  << " leader=" << Named::getIDSecure(leader.first)
                  << " leaderGap=" << leader.second
                  << " neighLead=" << Named::getIDSecure(neighLead.first)
                  << " neighLeadGap=" << neighLead.second
                  << " neighFollow=" << Named::getIDSecure(neighFollow.first)
                  << " neighFollowGap=" << neighFollow.second
                  << "\n";
    }
#endif

    // we try to estimate the distance which is necessary to get on a lane
    //  we have to get on in order to keep our route
    // we assume we need something that depends on our velocity
    // and compare this with the free space on our wished lane
    //
    // if the free space is somehow(<-?) less than the space we need, we should
    //  definitely try to get to the desired lane
    //
    // this rule forces our vehicle to change the lane if a lane changing is necessary soon


    // we do not want the lookahead distance to change all the time so we let it decay slowly
    // (in contrast, growth is applied instantaneously)
    if (myVehicle.getSpeed() > myLookAheadSpeed) {
        myLookAheadSpeed = myVehicle.getSpeed();
    } else {
        // memory decay factor for this action step
        const double memoryFactor = 1. - (1. - LOOK_AHEAD_SPEED_MEMORY) * myVehicle.getActionStepLengthSecs();
        assert(memoryFactor > 0.);
        myLookAheadSpeed = MAX2(LOOK_AHEAD_MIN_SPEED,
                                (memoryFactor * myLookAheadSpeed + (1 - memoryFactor) * myVehicle.getSpeed()));
    }
    double laDist = myLookAheadSpeed * LOOK_FORWARD * myStrategicParam * (right ? 1 : myLookaheadLeft);
    laDist += myVehicle.getVehicleType().getLengthWithGap() *  2.;


    if (bestLaneOffset == 0 && leader.first != 0 && leader.first->isStopped() && leader.second < (currentDist - posOnLane)) {
        // react to a stopped leader on the current lane
        // The value of laDist is doubled below for the check whether the lc-maneuver can be taken out
        // on the remaining distance (because the vehicle has to change back and forth). Therefore multiply with 0.5.
        laDist = 0.5 * (myVehicle.getVehicleType().getLengthWithGap()
                        + leader.first->getVehicleType().getLengthWithGap()
                        + leader.second);
    } else if (bestLaneOffset == laneOffset && neighLead.first != 0 && neighLead.first->isStopped() && neighLead.second < (currentDist - posOnLane)) {
        // react to a stopped leader on the target lane (if it is the bestLane)
        laDist = (myVehicle.getVehicleType().getLengthWithGap()
                  + neighLead.first->getVehicleType().getLengthWithGap()
                  + neighLead.second);
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



    // Next we assign to roundabout edges a larger distance than to normal edges
    // in order to decrease sense of lc urgency and induce higher usage of inner roundabout lanes.
    const double roundaboutBonus = MSLCHelper::getRoundaboutDistBonus(myVehicle, myRoundaboutBonus, curr, neigh, best);
    currentDist += roundaboutBonus;
    neighDist += roundaboutBonus;

    const double usableDist = MAX2(currentDist - posOnLane - best.occupation * JAM_FACTOR, driveToNextStop);
    //- (best.lane->getVehicleNumber() * neighSpeed)); // VARIANT 9 jfSpeed
    const double maxJam = MAX2(preb[currIdx + prebOffset].occupation, preb[currIdx].occupation);
    const double neighLeftPlace = MAX2(0.0, neighDist - posOnLane - maxJam);
    const double vMax = myVehicle.getLane()->getVehicleMaxSpeed(&myVehicle);
    // upper bound which will be restricted successively
    double thisLaneVSafe = vMax;
    const bool checkOverTakeRight = (!myAllowOvertakingRight
                                     && !myVehicle.congested()
                                     && myVehicle.getVehicleType().getVehicleClass() != SVC_EMERGENCY
                                     && (myOvertakeRightParam == 0 || myOvertakeRightParam < RandHelper::rand(myVehicle.getRNG())));

#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << STEPS2TIME(currentTime)
                  << " veh=" << myVehicle.getID()
                  << " laSpeed=" << myLookAheadSpeed
                  << " laDist=" << laDist
                  << " currentDist=" << currentDist
                  << " usableDist=" << usableDist
                  << " bestLaneOffset=" << bestLaneOffset
                  << " best.occupation=" << best.occupation
                  << " best.length=" << best.length
                  << "\n roundaboutBonus=" << roundaboutBonus
                  << " maxJam=" << maxJam
                  << " neighDist=" << neighDist
                  << " neighLeftPlace=" << neighLeftPlace
                  << "\n";
    }
#endif

    bool changeLeftToAvoidOvertakeRight = false;
    if (changeToBest && bestLaneOffset == curr.bestLaneOffset
            && currentDistDisallows(usableDist, bestLaneOffset, laDist)) {
        /// @brief we urgently need to change lanes to follow our route
        ret = ret | lca | LCA_STRATEGIC | LCA_URGENT;
    } else {
        // VARIANT_20 (noOvertakeRight)
        if (neighLead.first != 0 && checkOverTakeRight && !right) {
            // check for slower leader on the left. we should not overtake but
            // rather move left ourselves (unless congested)
            MSVehicle* nv = neighLead.first;
            const double deltaV = MAX2(vMax - neighLane.getVehicleMaxSpeed(nv),
                                       myVehicle.getSpeed() - nv->getSpeed());
            if (deltaV > 0) {
                const double vMaxDecel = myCarFollowModel.getSpeedAfterMaxDecel(myVehicle.getSpeed());
                const double vSafeFollow = myCarFollowModel.followSpeed(
                                               &myVehicle, myVehicle.getSpeed(), neighLead.second, nv->getSpeed(), nv->getCarFollowModel().getMaxDecel());
                const double vStayBehind = nv->getSpeed() - HELP_OVERTAKE;
                double vSafe;
                if (vSafeFollow >= vMaxDecel) {
                    vSafe = vSafeFollow;
                } else {
                    vSafe = MAX2(vMaxDecel, vStayBehind);
                }
                if (mySpeedGainProbability < myChangeProbThresholdLeft) {
                    vSafe = MAX2(vSafe, nv->getSpeed());
                }
                thisLaneVSafe = MIN2(thisLaneVSafe, vSafe);
                addLCSpeedAdvice(vSafe);
                // only generate impulse for overtaking left shortly before braking would be necessary
                const double deltaGapFuture = deltaV * 8;
                const double vSafeFuture = myCarFollowModel.followSpeed(
                                               &myVehicle, myVehicle.getSpeed(), neighLead.second - deltaGapFuture, nv->getSpeed(), nv->getCarFollowModel().getMaxDecel());
                if (vSafeFuture < vSafe) {
                    const double relativeGain = deltaV / MAX2(vMax,
                                                RELGAIN_NORMALIZATION_MIN_SPEED);
                    mySpeedGainProbability += myVehicle.getActionStepLengthSecs() * relativeGain;
                    changeLeftToAvoidOvertakeRight = true;
                }
#ifdef DEBUG_WANTS_CHANGE
                if (DEBUG_COND) {
                    std::cout << STEPS2TIME(currentTime)
                              << " avoid overtaking on the right nv=" << nv->getID()
                              << " deltaV=" << deltaV
                              << " nvSpeed=" << nv->getSpeed()
                              << " mySpeedGainProbability=" << mySpeedGainProbability
                              << " planned acceleration =" << myLCAccelerationAdvices.back()
                              << "\n";
                }
#endif
            }
        }
        const double overtakeDist = (leader.first == 0 ? -1 :
                                     leader.second + myVehicle.getVehicleType().getLength() + leader.first->getVehicleType().getLengthWithGap());
        if (leader.first != 0 && leader.first->isStopped() && leader.second < REACT_TO_STOPPED_DISTANCE
                // current destination leaves enough space to overtake the leader
                && MIN2(neighDist, currentDist) - posOnLane > overtakeDist
                // maybe do not overtake on the right at high speed
                && (!checkOverTakeRight || !right)
                && (neighLead.first == 0 || !neighLead.first->isStopped()
                    // neighboring stopped vehicle leaves enough space to overtake leader
                    || neighLead.second > overtakeDist)) {
            // avoid becoming stuck behind a stopped leader
            currentDist = myVehicle.getPositionOnLane() + leader.second;
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << " veh=" << myVehicle.getID() << " overtake stopped leader=" << leader.first->getID()
                          << " overtakeDist=" << overtakeDist
                          << " remaining=" << MIN2(neighDist, currentDist) - posOnLane
                          << "\n";
            }
#endif
            ret = ret | lca | LCA_STRATEGIC | LCA_URGENT;
        } else if (!changeToBest && (currentDistDisallows(neighLeftPlace, abs(bestLaneOffset) + 2, laDist))) {
            // the opposite lane-changing direction should be done than the one examined herein
            //  we'll check whether we assume we could change anyhow and get back in time...
            //
            // this rule prevents the vehicle from moving in opposite direction of the best lane
            //  unless the way till the end where the vehicle has to be on the best lane
            //  is long enough
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << " veh=" << myVehicle.getID() << " could not change back and forth in time (1) neighLeftPlace=" << neighLeftPlace << "\n";
            }
#endif
            ret = ret | LCA_STAY | LCA_STRATEGIC;
        } else if (bestLaneOffset == 0 && (neighLeftPlace * 2. < laDist)) {
            // the current lane is the best and a lane-changing would cause a situation
            //  of which we assume we will not be able to return to the lane we have to be on.
            // this rule prevents the vehicle from leaving the current, best lane when it is
            //  close to this lane's end
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << " veh=" << myVehicle.getID() << " could not change back and forth in time (2) neighLeftPlace=" << neighLeftPlace << "\n";
            }
#endif
            ret = ret | LCA_STAY | LCA_STRATEGIC;
        } else if (bestLaneOffset == 0
                   && (leader.first == 0 || !leader.first->isStopped())
                   && neigh.bestContinuations.back()->getLinkCont().size() != 0
                   && roundaboutBonus == 0
                   && !checkOpposite
                   && neighDist < TURN_LANE_DIST) {
            // VARIANT_21 (stayOnBest)
            // we do not want to leave the best lane for a lane which leads elsewhere
            // unless our leader is stopped or we are approaching a roundabout
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << " veh=" << myVehicle.getID() << " does not want to leave the bestLane (neighDist=" << neighDist << ")\n";
            }
#endif
            ret = ret | LCA_STAY | LCA_STRATEGIC;
        }
    }
    // check for overriding TraCI requests
#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << STEPS2TIME(currentTime) << " veh=" << myVehicle.getID() << " ret=" << ret;
    }
#endif
    // store state before canceling
    getCanceledState(laneOffset) |= ret;
    ret = myVehicle.influenceChangeDecision(ret);
    if ((ret & lcaCounter) != 0) {
        // we are not interested in traci requests for the opposite direction here
        ret &= ~(LCA_TRACI | lcaCounter | LCA_URGENT);
    }
#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << " retAfterInfluence=" << ret << "\n";
    }
#endif

    if ((ret & LCA_STAY) != 0) {
        return ret;
    }
    if ((ret & LCA_URGENT) != 0) {
        // prepare urgent lane change maneuver
        // save the left space
        myLeftSpace = currentDist - posOnLane;
        if (changeToBest && abs(bestLaneOffset) > 1) {
            // there might be a vehicle which needs to counter-lane-change one lane further and we cannot see it yet
            myLeadingBlockerLength = MAX2((right ? 20.0 : 40.0), myLeadingBlockerLength);
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << "  reserving space for unseen blockers myLeadingBlockerLength=" << myLeadingBlockerLength << "\n";
            }
#endif
        }

        // letting vehicles merge in at the end of the lane in case of counter-lane change, step#1
        //   if there is a leader and he wants to change to the opposite direction
        saveBlockerLength(neighLead.first, lcaCounter);
        if (*firstBlocked != neighLead.first) {
            saveBlockerLength(*firstBlocked, lcaCounter);
        }

        const int remainingLanes = MAX2(1, abs(bestLaneOffset));
        const double urgency = isOpposite() ? OPPOSITE_URGENCY : URGENCY;
        const double remainingSeconds = ((ret & LCA_TRACI) == 0 ?
                                         //MAX2(STEPS2TIME(TS), (myLeftSpace-myLeadingBlockerLength) / MAX2(myLookAheadSpeed, NUMERICAL_EPS) / remainingLanes / urgency) :
                                         MAX2(STEPS2TIME(TS), myLeftSpace / MAX2(myLookAheadSpeed, NUMERICAL_EPS) / remainingLanes / urgency) :
                                         myVehicle.getInfluencer().changeRequestRemainingSeconds(currentTime));
        const double plannedSpeed = informLeader(msgPass, blocked, myLca, neighLead, remainingSeconds);
        // NOTE: for the  ballistic update case negative speeds may indicate a stop request,
        //       while informLeader returns -1 in that case. Refs. #2577
        if (plannedSpeed >= 0 || (!MSGlobals::gSemiImplicitEulerUpdate && plannedSpeed != -1)) {
            // maybe we need to deal with a blocking follower
            informFollower(msgPass, blocked, myLca, neighFollow, remainingSeconds, plannedSpeed);
        }

#ifdef DEBUG_WANTS_CHANGE
        if (DEBUG_COND) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " myLeftSpace=" << myLeftSpace
                      << " remainingSeconds=" << remainingSeconds
                      << " plannedSpeed=" << plannedSpeed
                      << "\n";
        }
#endif

        return ret;
    }
    // a high inconvenience prevents cooperative changes.
    const double inconvenience = MIN2(1.0, (laneOffset < 0
                                            ? mySpeedGainProbability / myChangeProbThresholdRight
                                            : -mySpeedGainProbability / myChangeProbThresholdLeft));
    const bool speedGainInconvenient = inconvenience > myCooperativeParam;
    const bool neighOccupancyInconvenient = neigh.lane->getBruttoOccupancy() > curr.lane->getBruttoOccupancy();

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
        if (lca == LCA_LEFT) {
            // if inconvenience is not too high, request collaborative change (currently only for ballistic update)
            // TODO: test this for euler update! Refs. #2575
            if (MSGlobals::gSemiImplicitEulerUpdate || !neighOccupancyInconvenient) {
//                if(MSGlobals::gSemiImplicitEulerUpdate || !speedGainInconvenient){
                req = ret | lca | LCA_COOPERATIVE;
            }
        } else {
            // if inconvenience is not too high, request collaborative change (currently only for ballistic update)
            if (MSGlobals::gSemiImplicitEulerUpdate || neighOccupancyInconvenient) {
//            if(MSGlobals::gSemiImplicitEulerUpdate || speedGainInconvenient){
                req = ret | LCA_STAY | LCA_COOPERATIVE;
            }
        }
        if (!cancelRequest(req, laneOffset)) {
            return ret | req;
        }
    }

    // let's also regard the case where the vehicle is driving on a highway...
    //  in this case, we do not want to get to the dead-end of an on-ramp
    if (right) {
        if (bestLaneOffset == 0 && myVehicle.getLane()->getSpeedLimit() > 80. / 3.6 && myLookAheadSpeed > SUMO_const_haltingSpeed) {
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << " veh=" << myVehicle.getID() << " does not want to get stranded on the on-ramp of a highway\n";
            }
#endif
            req = ret | LCA_STAY | LCA_STRATEGIC;
            if (!cancelRequest(req, laneOffset)) {
                return ret | req;
            }
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

    if (amBlockingFollowerPlusNB()
            && (!speedGainInconvenient)
            && ((myOwnState & myLca) != 0) // VARIANT_6 : counterNoHelp
            && (changeToBest || currentDistAllows(neighDist, abs(bestLaneOffset) + 1, laDist))) {

        // VARIANT_2 (nbWhenChangingToHelp)
#ifdef DEBUG_COOPERATE
        if (DEBUG_COND) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " wantsChangeToHelp=" << (right ? "right" : "left")
                      << " state=" << myOwnState
                      << (((myOwnState & myLca) == 0) ? " (counter)" : "")
                      << "\n";
        }
#endif
        req = ret | lca | LCA_COOPERATIVE | LCA_URGENT ;//| LCA_CHANGE_TO_HELP;
        if (!cancelRequest(req, laneOffset)) {
            return ret | req;
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

    // we wish to anticipate future speeds. This is difficult when the leading
    // vehicles are still accelerating so we resort to comparing speeds for the near future (1s) in this case
    const bool acceleratingLeader = (neighLead.first != 0 && neighLead.first->getAcceleration() > 0)
                                    || (leader.first != 0 && leader.first->getAcceleration() > 0);
    double neighLaneVSafe = anticipateFollowSpeed(neighLead, neighDist, vMax, acceleratingLeader);
    thisLaneVSafe = MIN2(thisLaneVSafe, anticipateFollowSpeed(leader, currentDist, vMax, acceleratingLeader));

    //std::cout << SIMTIME << " veh=" << myVehicle.getID() << " thisLaneVSafe=" << thisLaneVSafe << " neighLaneVSafe=" << neighLaneVSafe << "\n";

    if (neighLane.getEdge().getPersons().size() > 0) {
        // react to pedestrians
        adaptSpeedToPedestrians(myVehicle.getLane(), thisLaneVSafe);
        adaptSpeedToPedestrians(&neighLane, neighLaneVSafe);
    }

    const double relativeGain = (neighLaneVSafe - thisLaneVSafe) / MAX2(neighLaneVSafe,
                                RELGAIN_NORMALIZATION_MIN_SPEED);

#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << STEPS2TIME(currentTime)
                  << " veh=" << myVehicle.getID()
                  << " currentDist=" << currentDist
                  << " neighDist=" << neighDist
                  << " thisVSafe=" << thisLaneVSafe
                  << " neighVSafe=" << neighLaneVSafe
                  << " relGain=" << toString(relativeGain, 8)
                  << "\n";
    }
#endif

    if (right) {
        // ONLY FOR CHANGING TO THE RIGHT
        if (thisLaneVSafe - 5 / 3.6 > neighLaneVSafe) {
            // ok, the current lane is faster than the right one...
            if (mySpeedGainProbability < 0) {
                mySpeedGainProbability *= pow(0.5, myVehicle.getActionStepLengthSecs());
                //myKeepRightProbability /= 2.0;
            }
        } else {
            // ok, the current lane is not (much) faster than the right one
            // @todo recheck the 5 km/h discount on thisLaneVSafe, refs. #2068

            // do not promote changing to the left just because changing to the right is bad
            // XXX: The following code may promote it, though!? (recheck!)
            //      (Think of small negative mySpeedGainProbability and larger negative relativeGain)
            //      One might think of replacing '||' by '&&' to exclude that possibility...
            //      Still, for negative relativeGain, we might want to decrease the inclination for
            //      changing to the left. Another solution could be the seperation of mySpeedGainProbability into
            //      two variables (one for left and one for right). Refs #2578
            if (mySpeedGainProbability < 0 || relativeGain > 0) {
                mySpeedGainProbability -= myVehicle.getActionStepLengthSecs() * relativeGain;
            }

            // honor the obligation to keep right (Rechtsfahrgebot)
            // XXX consider fast approaching followers on the current lane. Refs #2578
            //const double vMax = myLookAheadSpeed;
            const double acceptanceTime = KEEP_RIGHT_ACCEPTANCE * vMax * MAX2(1.0, myVehicle.getSpeed()) / myVehicle.getLane()->getSpeedLimit();
            double fullSpeedGap = MAX2(0., neighDist - myVehicle.getCarFollowModel().brakeGap(vMax));
            double fullSpeedDrivingSeconds = MIN2(acceptanceTime, fullSpeedGap / vMax);
            if (neighLead.first != 0 && neighLead.first->getSpeed() < vMax) {
                fullSpeedGap = MAX2(0., MIN2(fullSpeedGap,
                                             neighLead.second - myVehicle.getCarFollowModel().getSecureGap(&myVehicle, neighLead.first,
                                                     vMax, neighLead.first->getSpeed(), neighLead.first->getCarFollowModel().getMaxDecel())));
                fullSpeedDrivingSeconds = MIN2(fullSpeedDrivingSeconds, fullSpeedGap / (vMax - neighLead.first->getSpeed()));
            }
            // stay on the current lane if we cannot overtake a slow leader on the right
            if (checkOverTakeRight && leader.first != 0
                    && leader.first->getLane()->getVehicleMaxSpeed(leader.first) < vMax) {
                fullSpeedGap = MIN2(fullSpeedGap, leader.second);
                fullSpeedDrivingSeconds = MIN2(fullSpeedDrivingSeconds, fullSpeedGap / (vMax - leader.first->getSpeed()));
                const double relativeGain = (vMax - leader.first->getLane()->getVehicleMaxSpeed(leader.first)) / MAX2(vMax,
                                            RELGAIN_NORMALIZATION_MIN_SPEED);
                // tiebraker to avoid buridans paradox see #1312
                mySpeedGainProbability += myVehicle.getActionStepLengthSecs() * relativeGain;
            }

            const double deltaProb = (myChangeProbThresholdRight * (fullSpeedDrivingSeconds / acceptanceTime) / KEEP_RIGHT_TIME);
            myKeepRightProbability -= myVehicle.getActionStepLengthSecs() * deltaProb;

#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << STEPS2TIME(currentTime)
                          << " veh=" << myVehicle.getID()
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
                          << " myKeepRightProbability=" << myKeepRightProbability
                          << "\n";
            }
#endif
            if (myKeepRightProbability * myKeepRightParam < -myChangeProbThresholdRight) {
                req = ret | lca | LCA_KEEPRIGHT;
                if (!cancelRequest(req, laneOffset)) {
                    return ret | req;
                }
            }
        }

#ifdef DEBUG_WANTS_CHANGE
        if (DEBUG_COND) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " speed=" << myVehicle.getSpeed()
                      << " mySpeedGainProbability=" << mySpeedGainProbability
                      << " thisLaneVSafe=" << thisLaneVSafe
                      << " neighLaneVSafe=" << neighLaneVSafe
                      << " relativeGain=" << relativeGain
                      << " blocked=" << blocked
                      << "\n";
        }
#endif

        if (mySpeedGainProbability < -myChangeProbThresholdRight
                && neighDist / MAX2(.1, myVehicle.getSpeed()) > 20.) { //./MAX2( .1, myVehicle.getSpeed())) { // -.1
            req = ret | lca | LCA_SPEEDGAIN;
            if (!cancelRequest(req, laneOffset)) {
                return ret | req;
            }
        }
    } else {
        // ONLY FOR CHANGING TO THE LEFT
        if (thisLaneVSafe > neighLaneVSafe) {
            // this lane is better
            if (mySpeedGainProbability > 0) {
                mySpeedGainProbability *= pow(0.5, myVehicle.getActionStepLengthSecs());
            }
        } else if (thisLaneVSafe == neighLaneVSafe) {
            if (mySpeedGainProbability > 0) {
                mySpeedGainProbability *= pow(0.8, myVehicle.getActionStepLengthSecs());
            }
        } else {
            // left lane is better
            mySpeedGainProbability += myVehicle.getActionStepLengthSecs() * relativeGain;
        }
        // VARIANT_19 (stayRight)
        //if (neighFollow.first != 0) {
        //    MSVehicle* nv = neighFollow.first;
        //    const double secGap = nv->getCarFollowModel().getSecureGap(nv, &myVehicle, nv->getSpeed(), myVehicle.getSpeed(), myVehicle.getCarFollowModel().getMaxDecel());
        //    if (neighFollow.second < secGap * KEEP_RIGHT_HEADWAY) {
        //        // do not change left if it would inconvenience faster followers
        //        return ret | LCA_STAY | LCA_SPEEDGAIN;
        //    }
        //}

#ifdef DEBUG_WANTS_CHANGE
        if (DEBUG_COND) {
            std::cout << STEPS2TIME(currentTime)
                      << " veh=" << myVehicle.getID()
                      << " speed=" << myVehicle.getSpeed()
                      << " mySpeedGainProbability=" << mySpeedGainProbability
                      << " thisLaneVSafe=" << thisLaneVSafe
                      << " neighLaneVSafe=" << neighLaneVSafe
                      << " relativeGain=" << relativeGain
                      << " blocked=" << blocked
                      << "\n";
        }
#endif

        if (mySpeedGainProbability > myChangeProbThresholdLeft
                && (relativeGain > NUMERICAL_EPS || changeLeftToAvoidOvertakeRight)
                && neighDist / MAX2(.1, myVehicle.getSpeed()) > 20.) { // .1
            req = ret | lca | LCA_SPEEDGAIN;
            if (!cancelRequest(req, laneOffset)) {
                return ret | req;
            }
        }
    }
    // --------
    if (changeToBest && bestLaneOffset == curr.bestLaneOffset
            && relativeGain >= 0
            && (right ? mySpeedGainProbability < 0 : mySpeedGainProbability > 0)) {
        // change towards the correct lane, speedwise it does not hurt
        req = ret | lca | LCA_STRATEGIC;
        if (!cancelRequest(req, laneOffset)) {
            return ret | req;
        }
    }
#ifdef DEBUG_WANTS_CHANGE
    if (DEBUG_COND) {
        std::cout << STEPS2TIME(currentTime)
                  << " veh=" << myVehicle.getID()
                  << " mySpeedGainProbability=" << mySpeedGainProbability
                  << " myKeepRightProbability=" << myKeepRightProbability
                  << " thisLaneVSafe=" << thisLaneVSafe
                  << " neighLaneVSafe=" << neighLaneVSafe
                  << "\n";
    }
#endif

    return ret;
}


double
MSLCM_LC2013::anticipateFollowSpeed(const std::pair<MSVehicle*, double>& leaderDist, double dist, double vMax, bool acceleratingLeader) {
    const MSVehicle* leader = leaderDist.first;
    const double gap = leaderDist.second;
    double futureSpeed;
    if (acceleratingLeader) {
        // XXX see #6562
        const double maxSpeed1s = (myVehicle.getSpeed() + myVehicle.getCarFollowModel().getMaxAccel()
                                   - ACCEL2SPEED(myVehicle.getCarFollowModel().getMaxAccel()));
        if (leader == nullptr) {
            futureSpeed = myCarFollowModel.followSpeed(&myVehicle, maxSpeed1s, dist, 0, 0);
        } else {
            futureSpeed = myCarFollowModel.followSpeed(&myVehicle, maxSpeed1s, gap, leader->getSpeed(), leader->getCarFollowModel().getMaxDecel());
        }
    } else {
        // onInsertion = true because the vehicle has already moved
        if (leader == nullptr) {
            futureSpeed = myCarFollowModel.maximumSafeStopSpeed(dist, myVehicle.getSpeed(), true);
        } else {
            futureSpeed = myCarFollowModel.maximumSafeFollowSpeed(gap, myVehicle.getSpeed(), leader->getSpeed(), leader->getCarFollowModel().getMaxDecel(), true);
        }
    }
    futureSpeed = MIN2(vMax, futureSpeed);
    if (leader != nullptr && gap > 0 && mySpeedGainLookahead > 0) {
        const double futureLeaderSpeed = acceleratingLeader ? leader->getLane()->getVehicleMaxSpeed(leader) : leader->getSpeed();
        const double deltaV = vMax - futureLeaderSpeed;
        if (deltaV > 0 && gap > 0) {
            const double secGap = myCarFollowModel.getSecureGap(&myVehicle, leader, futureSpeed, leader->getSpeed(), myCarFollowModel.getMaxDecel());
            const double fullSpeedGap = gap - secGap;
            if (fullSpeedGap / deltaV < mySpeedGainLookahead) {
                // anticipate future braking by computing the average
                // speed over the next few seconds
                const double gapClosingTime = fullSpeedGap / deltaV;
                const double foreCastTime = mySpeedGainLookahead * 2;
                //if (DEBUG_COND) std::cout << SIMTIME << " veh=" << myVehicle.getID() << " leader=" << leader->getID() << " gap=" << gap << " deltaV=" << deltaV << " futureSpeed=" << futureSpeed << " futureLeaderSpeed=" << futureLeaderSpeed;
                futureSpeed = MIN2(futureSpeed, (gapClosingTime * futureSpeed + (foreCastTime - gapClosingTime) * futureLeaderSpeed) / foreCastTime);
                //if (DEBUG_COND) std::cout << " newFutureSpeed=" << futureSpeed << "\n";
            }
        }
    }
    return futureSpeed;
}


int
MSLCM_LC2013::slowDownForBlocked(MSVehicle** blocked, int state) {
    //  if this vehicle is blocking someone in front, we maybe decelerate to let him in
    if ((*blocked) != nullptr) {
        double gap = (*blocked)->getPositionOnLane() - (*blocked)->getVehicleType().getLength() - myVehicle.getPositionOnLane() - myVehicle.getVehicleType().getMinGap();
#ifdef DEBUG_SLOW_DOWN
        if (DEBUG_COND) {
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
                                     gap - POSITION_EPS, (*blocked)->getSpeed(),
                                     (*blocked)->getCarFollowModel().getMaxDecel()));

                //(*blocked) = 0; // VARIANT_14 (furtherBlock)
#ifdef DEBUG_SLOW_DOWN
                if (DEBUG_COND) {
                    std::cout << SIMTIME
                              << " veh=" << myVehicle.getID()
                              << " slowing down for"
                              << " blocked=" << Named::getIDSecure(*blocked)
                              << " helpSpeed=" << myLCAccelerationAdvices.back()
                              << "\n";
                }
#endif
            } /* else {
            	// experimental else-branch...
                state |= LCA_AMBACKBLOCKER;
                myVSafes.push_back(myCarFollowModel.followSpeed(
                                       &myVehicle, myVehicle.getSpeed(),
                                       (gap - POSITION_EPS), (*blocked)->getSpeed(),
                                       (*blocked)->getCarFollowModel().getMaxDecel()));
            }*/
        }
    }
    return state;
}


void
MSLCM_LC2013::saveBlockerLength(MSVehicle* blocker, int lcaCounter) {
#ifdef DEBUG_SAVE_BLOCKER_LENGTH
    if (DEBUG_COND) {
        std::cout << SIMTIME
                  << " veh=" << myVehicle.getID()
                  << " saveBlockerLength blocker=" << Named::getIDSecure(blocker)
                  << " bState=" << (blocker == 0 ? "None" : toString(blocker->getLaneChangeModel().getOwnState()))
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
            if (DEBUG_COND) {
                std::cout << SIMTIME
                          << " veh=" << myVehicle.getID()
                          << " blocker=" << Named::getIDSecure(blocker)
                          << " saving myLeadingBlockerLength=" << myLeadingBlockerLength
                          << "\n";
            }
#endif
        } else {
            // we cannot save enough space for the blocker. It needs to save
            // space for ego instead
#ifdef DEBUG_SAVE_BLOCKER_LENGTH
            if (DEBUG_COND) {
                std::cout << SIMTIME
                          << " veh=" << myVehicle.getID()
                          << " blocker=" << Named::getIDSecure(blocker)
                          << " cannot save space=" << blocker->getVehicleType().getLengthWithGap()
                          << " potential=" << potential
                          << "\n";
            }
#endif
            blocker->getLaneChangeModel().saveBlockerLength(myVehicle.getVehicleType().getLengthWithGap());
        }
    }
}


void
MSLCM_LC2013::adaptSpeedToPedestrians(const MSLane* lane, double& v) {
    if (lane->hasPedestrians()) {
#ifdef DEBUG_WANTS_CHANGE
        if (DEBUG_COND) {
            std::cout << SIMTIME << " adapt to pedestrians on lane=" << lane->getID() << "\n";
        }
#endif
        PersonDist leader = lane->nextBlocking(myVehicle.getPositionOnLane(),
                                               myVehicle.getRightSideOnLane(), myVehicle.getRightSideOnLane() + myVehicle.getVehicleType().getWidth(),
                                               ceil(myVehicle.getSpeed() / myVehicle.getCarFollowModel().getMaxDecel()));
        if (leader.first != 0) {
            const double stopSpeed = myVehicle.getCarFollowModel().stopSpeed(&myVehicle, myVehicle.getSpeed(), leader.second - myVehicle.getVehicleType().getMinGap());
            v = MIN2(v, stopSpeed);
#ifdef DEBUG_WANTS_CHANGE
            if (DEBUG_COND) {
                std::cout << SIMTIME << "    pedLeader=" << leader.first->getID() << " dist=" << leader.second << " v=" << v << "\n";
            }
#endif
        }
    }
}


void MSLCM_LC2013::addLCSpeedAdvice(const double vSafe) {
    const double accel = SPEED2ACCEL(vSafe - myVehicle.getSpeed());
    myLCAccelerationAdvices.push_back(accel);
}


double
MSLCM_LC2013::computeSpeedLat(double latDist, double& maneuverDist) const {
    double result = MSAbstractLaneChangeModel::computeSpeedLat(latDist, maneuverDist);
    if (DEBUG_COND) {
        std::cout << " myLeftSpace=" << myLeftSpace << " latDist=" << latDist << " maneuverDist=" << maneuverDist << "result1=" << result << "\n";
    }
    if (myLeftSpace > POSITION_EPS) {
        double speedBound = myMaxSpeedLatStanding + myMaxSpeedLatFactor * myVehicle.getSpeed();
        if (isChangingLanes()) {
            speedBound = MAX2(LC_RESOLUTION_SPEED_LAT, speedBound);
        }
        result = MAX2(-speedBound, MIN2(speedBound, result));
    }
    return result;
}

double
MSLCM_LC2013::getAssumedDecelForLaneChangeDuration() const {
    return MAX2(LC_ASSUMED_DECEL, -myVehicle.getAcceleration());
}

double
MSLCM_LC2013::getSafetyFactor() const {
    return 1 / myAssertive;
}

double
MSLCM_LC2013::getOppositeSafetyFactor() const {
    return myOppositeParam <= 0 ? std::numeric_limits<double>::max() : 1 / myOppositeParam;
}

std::string
MSLCM_LC2013::getParameter(const std::string& key) const {
    if (key == toString(SUMO_ATTR_LCA_STRATEGIC_PARAM)) {
        return toString(myStrategicParam);
    } else if (key == toString(SUMO_ATTR_LCA_COOPERATIVE_PARAM)) {
        return toString(myCooperativeParam);
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAIN_PARAM)) {
        return toString(mySpeedGainParam);
    } else if (key == toString(SUMO_ATTR_LCA_KEEPRIGHT_PARAM)) {
        return toString(myKeepRightParam);
    } else if (key == toString(SUMO_ATTR_LCA_OPPOSITE_PARAM)) {
        return toString(myOppositeParam);
    } else if (key == toString(SUMO_ATTR_LCA_LOOKAHEADLEFT)) {
        return toString(myLookaheadLeft);
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAINRIGHT)) {
        return toString(mySpeedGainRight);
    } else if (key == toString(SUMO_ATTR_LCA_ASSERTIVE)) {
        return toString(myAssertive);
    } else if (key == toString(SUMO_ATTR_LCA_OVERTAKE_RIGHT)) {
        return toString(myOvertakeRightParam);
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
MSLCM_LC2013::setParameter(const std::string& key, const std::string& value) {
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
    } else if (key == toString(SUMO_ATTR_LCA_OPPOSITE_PARAM)) {
        myOppositeParam = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_LOOKAHEADLEFT)) {
        myLookaheadLeft = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_SPEEDGAINRIGHT)) {
        mySpeedGainRight = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_ASSERTIVE)) {
        myAssertive = doubleValue;
    } else if (key == toString(SUMO_ATTR_LCA_OVERTAKE_RIGHT)) {
        myOvertakeRightParam = doubleValue;
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


/****************************************************************************/
