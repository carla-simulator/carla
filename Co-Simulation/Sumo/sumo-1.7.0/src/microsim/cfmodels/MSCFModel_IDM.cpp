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
/// @file    MSCFModel_IDM.cpp
/// @author  Tobias Mayer
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Thu, 03 Sep 2009
///
// The Intelligent Driver Model (IDM) car-following model
/****************************************************************************/
#include <config.h>

#include "MSCFModel_IDM.h"
#include <microsim/MSVehicle.h>

//#define DEBUG_V

// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_IDM::MSCFModel_IDM(const MSVehicleType* vtype, bool idmm) :
    MSCFModel(vtype),
    myIDMM(idmm),
    myDelta(idmm ? 4.0 : vtype->getParameter().getCFParam(SUMO_ATTR_CF_IDM_DELTA, 4.)),
    myAdaptationFactor(idmm ? vtype->getParameter().getCFParam(SUMO_ATTR_CF_IDMM_ADAPT_FACTOR, 1.8) : 1.0),
    myAdaptationTime(idmm ? vtype->getParameter().getCFParam(SUMO_ATTR_CF_IDMM_ADAPT_TIME, 600.0) : 0.0),
    myIterations(MAX2(1, int(TS / vtype->getParameter().getCFParam(SUMO_ATTR_CF_IDM_STEPPING, .25) + .5))),
    myTwoSqrtAccelDecel(double(2 * sqrt(myAccel * myDecel))) {
    // IDM does not drive very precise and may violate minGap on occasion
    myCollisionMinGapFactor = vtype->getParameter().getCFParam(SUMO_ATTR_COLLISION_MINGAP_FACTOR, 0.5);
}

MSCFModel_IDM::~MSCFModel_IDM() {}


double
MSCFModel_IDM::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    const double vNext = MSCFModel::finalizeSpeed(veh, vPos);
    if (myAdaptationFactor != 1.) {
        VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
        vars->levelOfService += (vNext / veh->getLane()->getVehicleMaxSpeed(veh) - vars->levelOfService) / myAdaptationTime * TS;
    }
    return vNext;
}


double
MSCFModel_IDM::freeSpeed(const MSVehicle* const veh, double speed, double seen, double maxSpeed, const bool /*onInsertion*/) const {
    if (maxSpeed < 0.) {
        // can occur for ballistic update (in context of driving at red light)
        return maxSpeed;
    }
    const double secGap = getSecureGap(veh, nullptr, maxSpeed, 0, myDecel);
    double vSafe;
    if (speed <= maxSpeed) {
        // accelerate
        vSafe = _v(veh, 1e6, speed, maxSpeed, veh->getLane()->getVehicleMaxSpeed(veh), false);
    } else {
        // decelerate
        // @note relax gap to avoid emergency braking
        // @note since the transition point does not move we set the leader speed to 0
        vSafe = _v(veh, MAX2(seen, secGap), speed, 0, veh->getLane()->getVehicleMaxSpeed(veh), false);
    }
    if (seen < secGap) {
        // avoid overshoot when close to change in speed limit
        vSafe = MIN2(vSafe, maxSpeed);
    }
    //std::cout << SIMTIME << " speed=" << speed << " maxSpeed=" << maxSpeed << " seen=" << seen << " secGap=" << secGap << " vSafe=" << vSafe << "\n";
    return vSafe;
}


double
MSCFModel_IDM::followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred) const {
    applyHeadwayAndSpeedDifferencePerceptionErrors(veh, speed, gap2pred, predSpeed, predMaxDecel, pred);
#ifdef DEBUG_V
    gDebugFlag1 = veh->isSelected();
#endif
    return _v(veh, gap2pred, speed, predSpeed, veh->getLane()->getVehicleMaxSpeed(veh));
}


double
MSCFModel_IDM::insertionFollowSpeed(const MSVehicle* const v, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const /*pred*/) const {
    // see definition of s in _v()
    double s = MAX2(0., speed * myHeadwayTime + speed * (speed - predSpeed) / myTwoSqrtAccelDecel);
    if (gap2pred >= s) {
        // followSpeed always stays below speed because s*s / (gap2pred * gap2pred) > 0. This would prevent insertion with maximum speed at all distances
        return speed;
    } else {
        return followSpeed(v, speed, gap2pred, predSpeed, predMaxDecel);
    }
}


double
MSCFModel_IDM::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    applyHeadwayPerceptionError(veh, speed, gap);
    if (gap < 0.01) {
        return 0;
    }
    double result = _v(veh, gap, speed, 0, veh->getLane()->getVehicleMaxSpeed(veh));
    if (gap > 0 && speed < NUMERICAL_EPS && result < NUMERICAL_EPS) {
        // ensure that stops can be reached:
        //std::cout << " switching to krauss: " << veh->getID() << " gap=" << gap << " speed=" << speed << " res1=" << result << " res2=" << maximumSafeStopSpeed(gap, speed, false, veh->getActionStepLengthSecs())<< "\n";
        result = maximumSafeStopSpeed(gap, speed, false, veh->getActionStepLengthSecs());
    }
    //if (result * TS > gap) {
    //    std::cout << "Maximum stop speed exceeded for gap=" << gap << " result=" << result << " veh=" << veh->getID() << " speed=" << speed << " t=" << SIMTIME << "\n";
    //}
    return result;
}


/// @todo update interactionGap logic to IDM
double
MSCFModel_IDM::interactionGap(const MSVehicle* const veh, double vL) const {
    // Resolve the IDM equation to gap. Assume predecessor has
    // speed != 0 and that vsafe will be the current speed plus acceleration,
    // i.e that with this gap there will be no interaction.
    const double acc = myAccel * (1. - pow(veh->getSpeed() / veh->getLane()->getVehicleMaxSpeed(veh), myDelta));
    const double vNext = veh->getSpeed() + acc;
    const double gap = (vNext - vL) * (veh->getSpeed() + vL) / (2 * myDecel) + vL;

    // Don't allow timeHeadWay < deltaT situations.
    return MAX2(gap, SPEED2DIST(vNext));
}

double
MSCFModel_IDM::getSecureGap(const MSVehicle* const /*veh*/, const MSVehicle* const /*pred*/, const double speed, const double leaderSpeed, const double /*leaderMaxDecel*/) const {
    const double delta_v = speed - leaderSpeed;
    return MAX2(0.0, speed * myHeadwayTime + speed * delta_v / myTwoSqrtAccelDecel);
}


double
MSCFModel_IDM::_v(const MSVehicle* const veh, const double gap2pred, const double egoSpeed,
                  const double predSpeed, const double desSpeed, const bool respectMinGap) const {
// this is more or less based on http://www.vwi.tu-dresden.de/~treiber/MicroApplet/IDM.html
// and http://arxiv.org/abs/cond-mat/0304337
// we assume however constant speed for the leader
    double headwayTime = myHeadwayTime;
    if (myAdaptationFactor != 1.) {
        const VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
        headwayTime *= myAdaptationFactor + vars->levelOfService * (1. - myAdaptationFactor);
    }
    double newSpeed = egoSpeed;
    double gap = gap2pred;
    if (respectMinGap) {
        // gap2pred comes with minGap already subtracted so we need to add it here again
        gap += myType->getMinGap();
    }
    for (int i = 0; i < myIterations; i++) {
        const double delta_v = newSpeed - predSpeed;
        double s = MAX2(0., newSpeed * headwayTime + newSpeed * delta_v / myTwoSqrtAccelDecel);
        if (respectMinGap) {
            s += myType->getMinGap();
        }
        gap = MAX2(NUMERICAL_EPS, gap); // avoid singularity
        const double acc = myAccel * (1. - pow(newSpeed / desSpeed, myDelta) - (s * s) / (gap * gap));
#ifdef DEBUG_V
        if (gDebugFlag1) {
            std::cout << " gap=" << gap << " t=" << myHeadwayTime << " t2=" << headwayTime << " s=" << s << " pow=" << pow(newSpeed / desSpeed, myDelta) << " gapDecel=" << (s * s) / (gap * gap) << " a=" << acc;
        }
#endif
        newSpeed = MAX2(0.0, newSpeed + ACCEL2SPEED(acc) / myIterations);
#ifdef DEBUG_V
        if (gDebugFlag1) {
            std::cout << " v2=" << newSpeed << "\n";
        }
#endif
        //TODO use more realistic position update which takes accelerated motion into account
        gap -= MAX2(0., SPEED2DIST(newSpeed - predSpeed) / myIterations);
    }
    return MAX2(0., newSpeed);
}


MSCFModel*
MSCFModel_IDM::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_IDM(vtype, myIDMM);
}
