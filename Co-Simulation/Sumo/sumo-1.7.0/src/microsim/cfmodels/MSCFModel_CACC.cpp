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
/// @file    MSCFModel_CACC.cpp
/// @author  Kallirroi Porfyri
/// @date    Nov 2018
///
// CACC car-following model based on [1], [2].
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

#include "MSCFModel_CACC.h"
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/StringUtils.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <math.h>
#include <microsim/MSNet.h>

// ===========================================================================
// debug flags
// ===========================================================================
#define DEBUG_CACC 0
#define DEBUG_CACC_INSERTION_FOLLOW_SPEED 0
#define DEBUG_CACC_SECURE_GAP 0
#define DEBUG_COND (veh->isSelected())
//#define DEBUG_COND (veh->getID() == "flow.0")
//#define DEBUG_COND (veh->getID() == "CVflowToC2.11")


// ===========================================================================
// defaults
// ===========================================================================
#define DEFAULT_SC_GAIN_CACC -0.4
#define DEFAULT_GCC_GAIN_GAP_CACC 0.005
#define DEFAULT_GCC_GAIN_GAP_DOT_CACC 0.05
#define DEFAULT_GC_GAIN_GAP_CACC 0.45
#define DEFAULT_GC_GAIN_GAP_DOT_CACC 0.0125
#define DEFAULT_CA_GAIN_GAP_CACC 0.45
#define DEFAULT_CA_GAIN_GAP_DOT_CACC 0.05
#define DEFAULT_HEADWAYTIME_ACC 1.0

// override followSpeed when deemed unsafe by the given margin (the value was selected to reduce the number of necessary interventions)
#define DEFAULT_EMERGENCY_OVERRIDE_THRESHOLD 2.0

std::map<std::string, MSCFModel_CACC::CommunicationsOverrideMode> MSCFModel_CACC::CommunicationsOverrideModeMap = {
    {"0", CACC_NO_OVERRIDE},
    {"1", CACC_MODE_NO_LEADER},
    {"2", CACC_MODE_LEADER_NO_CAV},
    {"3", CACC_MODE_LEADER_CAV}
};

std::map<MSCFModel_CACC::VehicleMode, std::string> MSCFModel_CACC::VehicleModeNames = {
    {ACC_MODE, "ACC"},
    {CACC_MODE, "CACC"}
};

// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_CACC::MSCFModel_CACC(const MSVehicleType* vtype) :
    MSCFModel(vtype), acc_CFM(MSCFModel_ACC(vtype)),
    mySpeedControlGain(vtype->getParameter().getCFParam(SUMO_ATTR_SC_GAIN_CACC, DEFAULT_SC_GAIN_CACC)),
    myGapClosingControlGainGap(vtype->getParameter().getCFParam(SUMO_ATTR_GCC_GAIN_GAP_CACC, DEFAULT_GCC_GAIN_GAP_CACC)),
    myGapClosingControlGainGapDot(vtype->getParameter().getCFParam(SUMO_ATTR_GCC_GAIN_GAP_DOT_CACC, DEFAULT_GCC_GAIN_GAP_DOT_CACC)),
    myGapControlGainGap(vtype->getParameter().getCFParam(SUMO_ATTR_GC_GAIN_GAP_CACC, DEFAULT_GC_GAIN_GAP_CACC)),
    myGapControlGainGapDot(vtype->getParameter().getCFParam(SUMO_ATTR_GC_GAIN_GAP_DOT_CACC, DEFAULT_GC_GAIN_GAP_DOT_CACC)),
    myCollisionAvoidanceGainGap(vtype->getParameter().getCFParam(SUMO_ATTR_CA_GAIN_GAP_CACC, DEFAULT_CA_GAIN_GAP_CACC)),
    myCollisionAvoidanceGainGapDot(vtype->getParameter().getCFParam(SUMO_ATTR_CA_GAIN_GAP_DOT_CACC, DEFAULT_CA_GAIN_GAP_DOT_CACC)),
    myHeadwayTimeACC(vtype->getParameter().getCFParam(SUMO_ATTR_HEADWAY_TIME_CACC_TO_ACC, DEFAULT_HEADWAYTIME_ACC)) {
    myCollisionMinGapFactor = vtype->getParameter().getCFParam(SUMO_ATTR_COLLISION_MINGAP_FACTOR, 0.1);
    acc_CFM.setHeadwayTime(myHeadwayTimeACC);
}

MSCFModel_CACC::~MSCFModel_CACC() {}

double
MSCFModel_CACC::freeSpeed(const MSVehicle* const veh, double speed, double seen, double maxSpeed, const bool onInsertion) const {
    // set "caccVehicleMode" parameter to default value
    if (!MSGlobals::gComputeLC) {
        const_cast<SUMOVehicleParameter&>(veh->getParameter()).setParameter("caccVehicleMode", VehicleModeNames[CACC_MODE]);
    }
    return MSCFModel::freeSpeed(veh, speed, seen, maxSpeed, onInsertion);
}

double
MSCFModel_CACC::followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred) const {

    const double desSpeed = veh->getLane()->getVehicleMaxSpeed(veh);
    const double vCACC = _v(veh, pred, gap2pred, speed, predSpeed, desSpeed, true);
    //gDebugFlag1 = DEBUG_COND;
    const double vSafe = maximumSafeFollowSpeed(gap2pred, speed, predSpeed, predMaxDecel);
    //gDebugFlag1 = false;
#if DEBUG_CACC == 1
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << veh->getID() << " pred=" << Named::getIDSecure(pred)
                  << " v=" << speed << " vL=" << predSpeed << " gap=" << gap2pred
                  << " predDecel=" << predMaxDecel << " vCACC=" << vCACC << " vSafe=" << vSafe << "\n";
    }
#else
    UNUSED_PARAMETER(pred);
#endif
    const double speedOverride = MIN2(DEFAULT_EMERGENCY_OVERRIDE_THRESHOLD, gap2pred);
    if (vSafe + speedOverride < vCACC) {
#if DEBUG_CACC == 1
        if (DEBUG_COND) {
            std::cout << "Apply Safe speed, override=" << speedOverride << "\n";
        }
#endif
        return vSafe + speedOverride;
    }
    return vCACC;
}

double
MSCFModel_CACC::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {

    // NOTE: This allows return of smaller values than minNextSpeed().
    // Only relevant for the ballistic update: We give the argument headway=TS, to assure that
    // the stopping position is approached with a uniform deceleration also for tau!=TS.
    return MIN2(maximumSafeStopSpeed(gap, speed, false, veh->getActionStepLengthSecs()), maxNextSpeed(speed, veh));
}

double
MSCFModel_CACC::getSecureGap(const MSVehicle* const veh, const MSVehicle* const pred, const double speed, const double leaderSpeed, const double leaderMaxDecel) const {
    // Accel in gap mode should vanish:
    double desSpacing;
    if (pred->getCarFollowModel().getModelID() != SUMO_TAG_CF_CACC) {
        //      0 = myGapControlGainSpeed * (leaderSpeed - speed) + myGapControlGainSpace * (g - myHeadwayTime * speed);
        // <=>  myGapControlGainSpace * g = - myGapControlGainSpeed * (leaderSpeed - speed) + myGapControlGainSpace * myHeadwayTime * speed;
        // <=>  g = - myGapControlGainSpeed * (leaderSpeed - speed) / myGapControlGainSpace + myHeadwayTime * speed;
        desSpacing = acc_CFM.myGapControlGainSpeed * (speed - leaderSpeed) / acc_CFM.myGapControlGainSpace + myHeadwayTimeACC * speed; // MSCFModel_ACC::accelGapControl
    } else {
        desSpacing = myHeadwayTime * speed; // speedGapControl
    };
    const double desSpacingDefault = MSCFModel::getSecureGap(veh, pred, speed, leaderSpeed, leaderMaxDecel);
#if DEBUG_CACC_SECURE_GAP == 1
    std::cout << SIMTIME << "MSCFModel_ACC::getSecureGap speed=" << speed << " leaderSpeed=" << leaderSpeed
              << " desSpacing=" << desSpacing << " desSpacingDefault=" << desSpacingDefault << "\n";
#endif
    return MAX2(desSpacing, desSpacingDefault);
}


double
MSCFModel_CACC::insertionFollowSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred) const {
#if DEBUG_CACC_INSERTION_FOLLOW_SPEED == 1
    if (DEBUG_COND) {
        std::cout << "MSCFModel_ACC::insertionFollowSpeed(), speed=" << speed << " gap2pred=" << gap2pred << " predSpeed=" << predSpeed << "\n";
    }
#endif
    // iterate to find a stationary value for
    //    speed = followSpeed(v, speed, gap2pred, predSpeed, predMaxDecel, nullptr)
    const int max_iter = 50;
    int n_iter = 0;
    const double tol = 0.1;
    double damping = 0.8;

    double res = speed;
    while (n_iter < max_iter) {
        // proposed acceleration
        const double vCACC = _v(veh, pred, gap2pred, res, predSpeed, speed, true);
        const double vSafe = maximumSafeFollowSpeed(gap2pred, res, predSpeed, predMaxDecel, true);
        const double a = MIN2(vCACC, vSafe) - res;
        res = res + damping * a;
#if DEBUG_CACC_INSERTION_FOLLOW_SPEED == 1
        if (DEBUG_COND) {
            std::cout << "   n_iter=" << n_iter << " vSafe=" << vSafe << " vCACC=" << vCACC << " a=" << a << " damping=" << damping << " res=" << res << std::endl;
        }
#endif
        damping *= 0.9;
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
MSCFModel_CACC::interactionGap(const MSVehicle* const /* veh */, double /* vL */) const {
    /*maximum radar range is CACC is enabled*/
    return 250;
}


std::string
MSCFModel_CACC::getParameter(const MSVehicle* veh, const std::string& key) const {
    CACCVehicleVariables* vars = (CACCVehicleVariables*) veh->getCarFollowVariables();

    if (key.compare("caccCommunicationsOverrideMode") == 0) {
        return toString(vars->CACC_CommunicationsOverrideMode);
    }

    return "";
}


void
MSCFModel_CACC::setParameter(MSVehicle* veh, const std::string& key, const std::string& value) const {
    CACCVehicleVariables* vars = (CACCVehicleVariables*) veh->getCarFollowVariables();

    try {
        if (key.compare("caccCommunicationsOverrideMode") == 0) {
            vars->CACC_CommunicationsOverrideMode = CommunicationsOverrideModeMap[value];
        }
    } catch (NumberFormatException&) {
        throw InvalidArgument("Invalid value '" + value + "' for parameter '" + key + "' for vehicle '" + veh->getID() + "'");
    }
}


double MSCFModel_CACC::speedSpeedControl(const double speed, double vErr, VehicleMode& vehMode) const {
    // Speed control law
    vehMode = CACC_MODE;
    double sclAccel = mySpeedControlGain * vErr;
    double newSpeed = speed + ACCEL2SPEED(sclAccel);
    return newSpeed;
}

double MSCFModel_CACC::speedGapControl(const MSVehicle* const veh, const double gap2pred,
                                       const double speed, const double predSpeed, const double desSpeed, double vErr,
                                       const MSVehicle* const pred, VehicleMode& vehMode) const {
    // Gap control law
    double newSpeed = 0.0;

    CACCVehicleVariables* vars = (CACCVehicleVariables*) veh->getCarFollowVariables();
    CommunicationsOverrideMode commMode = vars->CACC_CommunicationsOverrideMode;

#if DEBUG_CACC == 1
    if (pred == nullptr) {
        std::cout << "CACC_ControlModeOverride for : " << veh->getID() << " is " << mode << std::endl;
    } else {
        std::cout << "CACC_ControlModeOverride for : " << veh->getID() << " is " << mode << ". Leader is : " << pred->getID() << std::endl;
    }
#endif

    if (commMode == CACC_NO_OVERRIDE) {  // old CACC logic
        if (pred != nullptr) {
            if (pred->getCarFollowModel().getModelID() != SUMO_TAG_CF_CACC) {
                vehMode = ACC_MODE;
                newSpeed = acc_CFM._v(veh, gap2pred, speed, predSpeed, desSpeed, true);
#if DEBUG_CACC == 1
                if (DEBUG_COND) {
                    std::cout << "        acc control mode" << std::endl;
                }
#endif
            } else {
                vehMode = CACC_MODE;
#if DEBUG_CACC == 1
                if (DEBUG_COND) {
                    std::cout << "        CACC control mode" << std::endl;
                }
#endif
                double desSpacing = myHeadwayTime * speed;
                double gap = gap2pred - veh->getVehicleType().getMinGap();
                double spacingErr = gap - desSpacing;
                double accel = veh->getAcceleration();
                double spacingErr1 = predSpeed - speed + myHeadwayTime * accel;

                if ((spacingErr > 0 && spacingErr < 0.2) && (vErr < 0.1)) {
                    // gap mode
                    //newSpeed = speed + 0.45 * spacingErr + 0.0125 *spacingErr1;
#if DEBUG_CACC == 1
                    if (DEBUG_COND) {
                        std::cout << "        applying gap control" << std::endl;
                    }
#endif
                    newSpeed = speed + myGapControlGainGap * spacingErr + myGapControlGainGapDot * spacingErr1;
                } else if (spacingErr < 0) {
                    // collision avoidance mode
                    //newSpeed = speed + 0.45 * spacingErr + 0.05 *spacingErr1;
#if DEBUG_CACC == 1
                    if (DEBUG_COND) {
                        std::cout << "        applying collision avoidance err=" << spacingErr << " err1=" << spacingErr1 << "\n";
                    }
#endif
                    newSpeed = speed + myCollisionAvoidanceGainGap * spacingErr + myCollisionAvoidanceGainGapDot * spacingErr1;
                } else {
                    // gap closing mode
#if DEBUG_CACC == 1
                    if (DEBUG_COND) {
                        std::cout << "        applying gap closing" << std::endl;
                    }
#endif
                    newSpeed = speed + myGapClosingControlGainGap * spacingErr + myGapClosingControlGainGapDot * spacingErr1;
                }
            }
        } else {
            /* no leader */
#if DEBUG_CACC == 1
            if (DEBUG_COND) {
                std::cout << "        no leader" << std::endl;
            }
#endif
            newSpeed = speedSpeedControl(speed, vErr, vehMode);
        }
    } else if (commMode == CACC_MODE_NO_LEADER) {  // no leader
        newSpeed = speedSpeedControl(speed, vErr, vehMode);
    } else if (commMode == CACC_MODE_LEADER_NO_CAV) {  // leader is not CAV or CV or crucialCAV
        vehMode = ACC_MODE;
        newSpeed = acc_CFM._v(veh, gap2pred, speed, predSpeed, desSpeed, true);
    } else if (commMode == CACC_MODE_LEADER_CAV) {  // leader is CAV or CV or crucialCAV
        vehMode = CACC_MODE;

        double desSpacing = myHeadwayTime * speed;
        double gap = gap2pred - veh->getVehicleType().getMinGap();
        double spacingErr = gap - desSpacing;
        double accel = veh->getAcceleration();
        double spacingErr1 = predSpeed - speed + myHeadwayTime * accel;

        if ((spacingErr > 0 && spacingErr < 0.2) && (vErr < 0.1)) {
            // gap mode
            //newSpeed = speed + 0.45 * spacingErr + 0.0125 *spacingErr1;
            newSpeed = speed + myGapControlGainGap * spacingErr + myGapControlGainGapDot * spacingErr1;
        } else if (spacingErr < 0) {
            // collision avoidance mode
            //newSpeed = speed + 0.45 * spacingErr + 0.05 *spacingErr1;
            newSpeed = speed + myCollisionAvoidanceGainGap * spacingErr + myCollisionAvoidanceGainGapDot * spacingErr1;
        } else {
            // gap closing mode
            newSpeed = speed + myGapClosingControlGainGap * spacingErr + myGapClosingControlGainGapDot * spacingErr1;
        }
    }

    return newSpeed;
}

double
MSCFModel_CACC::_v(const MSVehicle* const veh, const MSVehicle* const pred, const double gap2pred, const double speed,
                   const double predSpeed, const double desSpeed, const bool /* respectMinGap */) const {
    double newSpeed = 0.0;
    VehicleMode vehMode = CACC_MODE;

#if DEBUG_CACC == 1
    if (DEBUG_COND) {
        std::cout << SIMTIME << " MSCFModel_CACC::_v() for veh '" << veh->getID()
                  << " gap=" << gap2pred << " speed="  << speed << " predSpeed=" << predSpeed
                  << " desSpeed=" << desSpeed << std::endl;
    }
#endif

    /* Velocity error */
    double vErr = speed - desSpeed;
    bool setControlMode = false;
    CACCVehicleVariables* vars = (CACCVehicleVariables*)veh->getCarFollowVariables();
    if (vars->lastUpdateTime != MSNet::getInstance()->getCurrentTimeStep()) {
        vars->lastUpdateTime = MSNet::getInstance()->getCurrentTimeStep();
        setControlMode = true;
    }

    double time_gap = gap2pred / speed;
    if (time_gap > 2) {
#if DEBUG_CACC == 1
        if (DEBUG_COND) {
            std::cout << "        applying speedControl" << std::endl;
        }
#endif
        // Find acceleration - Speed control law
        newSpeed = speedSpeedControl(speed, vErr, vehMode);
        // Set cl to vehicle parameters
        if (setControlMode) {
            vars->CACC_ControlMode = 0;
            if (!MSGlobals::gComputeLC) {
                const_cast<SUMOVehicleParameter&>(veh->getParameter()).setParameter("caccVehicleMode", VehicleModeNames[vehMode]);
            }
        }
    } else if (time_gap < 1.5) {
        // Find acceleration - Gap control law
#if DEBUG_CACC == 1
        if (DEBUG_COND) {
            std::cout << "        speedGapControl" << std::endl;
        }
#endif

        newSpeed = speedGapControl(veh, gap2pred, speed, predSpeed, desSpeed, vErr, pred, vehMode);
        // Set cl to vehicle parameters
        if (setControlMode) {
            CommunicationsOverrideMode commMode = vars->CACC_CommunicationsOverrideMode;

            if (commMode == CACC_NO_OVERRIDE) {  // old code
                vars->CACC_ControlMode = 1;
            } else if (commMode == CACC_MODE_NO_LEADER) {
                vars->CACC_ControlMode = 1;
            } else if (commMode == CACC_MODE_LEADER_NO_CAV) {
                vars->CACC_ControlMode = 0;
            } else if (commMode == CACC_MODE_LEADER_CAV) {
                vars->CACC_ControlMode = 1;
            }

            if (!MSGlobals::gComputeLC) {
                const_cast<SUMOVehicleParameter&>(veh->getParameter()).setParameter("caccVehicleMode", VehicleModeNames[vehMode]);
            }
        }
    } else {
        // Follow previous applied law
        int cm = vars->CACC_ControlMode;
        if (!cm) {  // CACC_ControlMode = speed control

#if DEBUG_CACC == 1
            if (DEBUG_COND) {
                std::cout << "        applying speedControl (previous)" << std::endl;
            }
#endif
            newSpeed = speedSpeedControl(speed, vErr, vehMode);
        } else {  // CACC_ControlMode = gap control
#if DEBUG_CACC == 1
            if (DEBUG_COND) {
                std::cout << "        previous speedGapControl (previous)" << std::endl;
            }
#endif
            newSpeed = speedGapControl(veh, gap2pred, speed, predSpeed, desSpeed, vErr, pred, vehMode);
        }

        if (setControlMode && !MSGlobals::gComputeLC) {
            const_cast<SUMOVehicleParameter&>(veh->getParameter()).setParameter("caccVehicleMode", VehicleModeNames[vehMode]);
        }
    }

#if DEBUG_CACC == 1
    if (DEBUG_COND) std::cout << "        result: accel=" << SPEED2ACCEL(newSpeed - speed) {
                << " newSpeed="  << newSpeed << std::endl;
    }
#endif

    return MAX2(0., newSpeed);
}



MSCFModel*
MSCFModel_CACC::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_CACC(vtype);
}

