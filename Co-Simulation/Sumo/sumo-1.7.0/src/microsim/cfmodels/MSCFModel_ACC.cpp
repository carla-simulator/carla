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
/// @file    MSCFModel_ACC.cpp
/// @author  Kallirroi Porfyri
/// @date    Feb 2018
///
// ACC car-following model based on [1], [2].
// [1] Milanes, V., and S. E. Shladover. Handling Cut-In Vehicles in Strings
//    of Cooperative Adaptive Cruise Control Vehicles. Journal of Intelligent
//     Transportation Systems, Vol. 20, No. 2, 2015, pp. 178-191.
// [2] Xiao, L., M. Wang and B. van Arem. Realistic Car-Following Models for
//    Microscopic Simulation of Adaptive and Cooperative Adaptive Cruise
//     Control Vehicles. Transportation Research Record: Journal of the
//     Transportation Research Board, No. 2623, 2017. (DOI: 10.3141/2623-01).
/****************************************************************************/
#include <config.h>

#include <stdio.h>
#include <iostream>

#include "MSCFModel_ACC.h"
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOTime.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <math.h>
#include <microsim/MSNet.h>

// ===========================================================================
// debug flags
// ===========================================================================
//#define DEBUG_ACC
//#define DEBUG_COND (true)
//#define DEBUG_COND (veh->isSelected())


// ===========================================================================
// defaults
// ===========================================================================
#define DEFAULT_SC_GAIN -0.4
#define DEFAULT_GCC_GAIN_SPEED 0.8
#define DEFAULT_GCC_GAIN_SPACE 0.04
#define DEFAULT_GC_GAIN_SPEED 0.07
#define DEFAULT_GC_GAIN_SPACE 0.23
#define DEFAULT_CA_GAIN_SPACE 0.8
#define DEFAULT_CA_GAIN_SPEED 0.23

// ===========================================================================
// thresholds
// ===========================================================================
#define GAP_THRESHOLD_SPEEDCTRL 120
#define GAP_THRESHOLD_GAPCTRL 100




// override followSpeed when deemed unsafe by the given margin (the value was selected to reduce the number of necessary interventions)
#define DEFAULT_EMERGENCY_OVERRIDE_THRESHOLD 2.0

/// @todo: add attributes for myCollisionAvoidanceGainSpeed and myCollisionAvoidanceGainSpace

// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_ACC::MSCFModel_ACC(const MSVehicleType* vtype) :
    MSCFModel(vtype),
    mySpeedControlGain(vtype->getParameter().getCFParam(SUMO_ATTR_SC_GAIN, DEFAULT_SC_GAIN)),
    myGapClosingControlGainSpeed(vtype->getParameter().getCFParam(SUMO_ATTR_GCC_GAIN_SPEED, DEFAULT_GCC_GAIN_SPEED)),
    myGapClosingControlGainSpace(vtype->getParameter().getCFParam(SUMO_ATTR_GCC_GAIN_SPACE, DEFAULT_GCC_GAIN_SPACE)),
    myGapControlGainSpeed(vtype->getParameter().getCFParam(SUMO_ATTR_GC_GAIN_SPEED, DEFAULT_GC_GAIN_SPEED)),
    myGapControlGainSpace(vtype->getParameter().getCFParam(SUMO_ATTR_GC_GAIN_SPACE, DEFAULT_GC_GAIN_SPACE)),
    myCollisionAvoidanceGainSpeed(vtype->getParameter().getCFParam(SUMO_ATTR_CA_GAIN_SPEED, DEFAULT_CA_GAIN_SPEED)),
    myCollisionAvoidanceGainSpace(vtype->getParameter().getCFParam(SUMO_ATTR_CA_GAIN_SPACE, DEFAULT_CA_GAIN_SPACE)) {
    // ACC does not drive very precise and often violates minGap
    myCollisionMinGapFactor = vtype->getParameter().getCFParam(SUMO_ATTR_COLLISION_MINGAP_FACTOR, 0.1);
}

MSCFModel_ACC::~MSCFModel_ACC() {}


double
MSCFModel_ACC::followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const /*pred*/) const {
    const double desSpeed = MIN2(veh->getLane()->getSpeedLimit(), veh->getMaxSpeed());
    const double vACC = _v(veh, gap2pred, speed, predSpeed, desSpeed, true);
    const double vSafe = maximumSafeFollowSpeed(gap2pred, speed, predSpeed, predMaxDecel);
    if (vSafe + DEFAULT_EMERGENCY_OVERRIDE_THRESHOLD < vACC) {
        //ACCVehicleVariables* vars = (ACCVehicleVariables*)veh->getCarFollowVariables();
        //std::cout << SIMTIME << " veh=" << veh->getID() << " v=" << speed << " vL=" << predSpeed << " gap=" << gap2pred << " vACC=" << vACC << " vSafe=" << vSafe << " cm=" << vars->ACC_ControlMode << "\n";
        return vSafe + DEFAULT_EMERGENCY_OVERRIDE_THRESHOLD;
    }
    return vACC;
}


double
MSCFModel_ACC::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    // NOTE: This allows return of smaller values than minNextSpeed().
    // Only relevant for the ballistic update: We give the argument headway=TS, to assure that
    // the stopping position is approached with a uniform deceleration also for tau!=TS.
    return MIN2(maximumSafeStopSpeed(gap, speed, false, veh->getActionStepLengthSecs()), maxNextSpeed(speed, veh));
}


double
MSCFModel_ACC::getSecureGap(const MSVehicle* const /*veh*/, const MSVehicle* const /*pred*/, const double speed, const double leaderSpeed, const double /* leaderMaxDecel */) const {
    // Accel in gap mode should vanish:
    //      0 = myGapControlGainSpeed * (leaderSpeed - speed) + myGapControlGainSpace * (g - myHeadwayTime * speed);
    // <=>  myGapControlGainSpace * g = - myGapControlGainSpeed * (leaderSpeed - speed) + myGapControlGainSpace * myHeadwayTime * speed;
    // <=>  g = - myGapControlGainSpeed * (leaderSpeed - speed) / myGapControlGainSpace + myHeadwayTime * speed;
    return myGapControlGainSpeed * (speed - leaderSpeed) / myGapControlGainSpace + myHeadwayTime * speed;
}


double
MSCFModel_ACC::insertionFollowSpeed(const MSVehicle* const v, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const /*pred*/) const {
//#ifdef DEBUG_ACC
//        std::cout << "MSCFModel_ACC::insertionFollowSpeed(), speed="<<speed<< std::endl;
//#endif
    // iterate to find a stationary value for
    //    speed = followSpeed(v, speed, gap2pred, predSpeed, predMaxDecel, nullptr)
    const int max_iter = 50;
    int n_iter = 0;
    const double tol = 0.1;
    const double damping = 0.1;

    double res = speed;
    while (n_iter < max_iter) {
        // proposed acceleration
        const double a = SPEED2ACCEL(followSpeed(v, res, gap2pred, predSpeed, predMaxDecel, nullptr) - res);
        res = res + damping * a;
//#ifdef DEBUG_ACC
//        std::cout << "   n_iter=" << n_iter << ", a=" << a << ", res=" << res << std::endl;
//#endif
        if (fabs(a) < tol) {
            break;
        } else {
            n_iter++;
        }
    }
    return res;
}


/// @todo update interactionGap logic
double
MSCFModel_ACC::interactionGap(const MSVehicle* const /* veh */, double /* vL */) const {
    /*maximum radar range is ACC is enabled*/
    return 250;
}

double MSCFModel_ACC::accelSpeedControl(double vErr) const {
    // Speed control law
    return mySpeedControlGain * vErr;
}

double MSCFModel_ACC::accelGapControl(const MSVehicle* const /* veh */, const double gap2pred, const double speed, const double predSpeed, double vErr) const {

#ifdef DEBUG_ACC
    if (DEBUG_COND) {
        std::cout << "        applying gapControl" << std::endl;
    }
#endif

// Gap control law
    double gclAccel = 0.0;
    double desSpacing = myHeadwayTime * speed;
    double spacingErr = gap2pred - desSpacing;
    double deltaVel = predSpeed - speed;


    if (fabs(spacingErr) < 0.2 && fabs(vErr) < 0.1) {
        // gap mode
        gclAccel = myGapControlGainSpeed * deltaVel + myGapControlGainSpace * spacingErr;
    } else if (spacingErr < 0)  {
        // collision avoidance mode
        gclAccel = myCollisionAvoidanceGainSpeed * deltaVel + myCollisionAvoidanceGainSpace * spacingErr;
    } else {
        // gap closing mode
        gclAccel = myGapClosingControlGainSpeed * deltaVel + myGapClosingControlGainSpace * spacingErr;
    }

    return gclAccel;
}


double
MSCFModel_ACC::_v(const MSVehicle* const veh, const double gap2pred, const double speed,
                  const double predSpeed, const double desSpeed, const bool /* respectMinGap */) const {

    double accelACC = 0;
    double gapLimit_SC = GAP_THRESHOLD_SPEEDCTRL; // lower gap limit in meters to enable speed control law
    double gapLimit_GC = GAP_THRESHOLD_GAPCTRL; // upper gap limit in meters to enable gap control law

#ifdef DEBUG_ACC
    if (DEBUG_COND) {
        std::cout << SIMTIME << " MSCFModel_ACC::_v() for veh '" << veh->getID() << "'\n"
                  << "        gap=" << gap2pred << " speed="  << speed << " predSpeed=" << predSpeed
                  << " desSpeed=" << desSpeed << std::endl;
    }
#endif


    /* Velocity error */
    double vErr = speed - desSpeed;
    int setControlMode = 0;
    ACCVehicleVariables* vars = (ACCVehicleVariables*) veh->getCarFollowVariables();
    if (vars->lastUpdateTime != MSNet::getInstance()->getCurrentTimeStep()) {
        vars->lastUpdateTime = MSNet::getInstance()->getCurrentTimeStep();
        setControlMode = 1;
    }
    if (gap2pred > gapLimit_SC) {

#ifdef DEBUG_ACC
        if (DEBUG_COND) {
            std::cout << "        applying speedControl" << std::endl;
        }
#endif
        // Find acceleration - Speed control law
        accelACC = accelSpeedControl(vErr);
        // Set cl to vehicle parameters
        if (setControlMode) {
            vars->ACC_ControlMode = 0;
        }
    } else if (gap2pred < gapLimit_GC) {
        // Find acceleration - Gap control law
        accelACC = accelGapControl(veh, gap2pred, speed, predSpeed, vErr);
        // Set cl to vehicle parameters
        if (setControlMode) {
            vars->ACC_ControlMode = 1;
        }
    } else {
        // Follow previous applied law
        int cm = vars->ACC_ControlMode;
        if (!cm) {

#ifdef DEBUG_ACC
            if (DEBUG_COND) {
                std::cout << "        applying speedControl" << std::endl;
            }
#endif
            accelACC = accelSpeedControl(vErr);
        } else {
            accelACC = accelGapControl(veh, gap2pred, speed, predSpeed, vErr);
        }

    }

    double newSpeed = speed + ACCEL2SPEED(accelACC);

#ifdef DEBUG_ACC
    if (DEBUG_COND) {
        std::cout << "        result: accel=" << accelACC << " newSpeed="  << newSpeed << std::endl;
    }
#endif

    return MAX2(0., newSpeed);
}


MSCFModel*
MSCFModel_ACC::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_ACC(vtype);
}
