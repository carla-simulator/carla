/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSCFModel_W99.cpp
/// @author  Jakob Erdmann
/// @date    June 2019
///
// The psycho-physical model of Wiedemann (10-Parameter version from 1999)
// references:
// code adapted from https://github.com/glgh/w99-demo
// (MIT License, Copyright (c) 2016 glgh)
/****************************************************************************/
#include <config.h>

#include <cmath>
#include "MSCFModel_W99.h"
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include <utils/common/RandHelper.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// only used by DK2008
#define INTERACTION_GAP 150

// ===========================================================================
// DEBUG constants
// ===========================================================================
#define DEBUG_FOLLOW_SPEED
//#define DEBUG_COND (veh->getID() == "flow.0")
#define DEBUG_COND (veh->isSelected())

// ===========================================================================
// static members
// ===========================================================================


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_W99::MSCFModel_W99(const MSVehicleType* vtype) :
    MSCFModel(vtype),
    myCC1(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC1,  1.30)),
    myCC2(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC2,  8.00)),
    myCC3(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC3, -12.00)),
    myCC4(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC4, -0.25)),
    myCC5(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC5,  0.35)),
    myCC6(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC6,  6.00)),
    myCC7(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC7,  0.25)),
    myCC8(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC8,  2.00)),
    myCC9(vtype->getParameter().getCFParam(SUMO_ATTR_CF_W99_CC9,  1.50)) {
    // translate some values to make them show up correctly in the gui
    myHeadwayTime = myCC1;
    myAccel = myCC8;
    // W99 does not drive very precise and may violate minGap on occasion
    myCollisionMinGapFactor = vtype->getParameter().getCFParam(SUMO_ATTR_COLLISION_MINGAP_FACTOR, 0.1);
}


MSCFModel_W99::~MSCFModel_W99() {}


void
MSCFModel_W99::computeThresholds(double speed, double predSpeed, double leaderAccel, double rndVal,
                                 double& sdxc, double& sdxo, double& sdxv) const {

    const double dv = predSpeed - speed;
    sdxc = myType->getMinGap(); // cc0
    if (predSpeed > 0) {
        const double v_slower = (dv >= 0 || leaderAccel < 1) ? speed : predSpeed + dv * rndVal;
        sdxc += myCC1 * MAX2(0.0, v_slower);
    }
    sdxo = sdxc + myCC2; //maximum following distance (upper limit of car-following process)
    sdxv = sdxo + myCC3 * (dv - myCC4);
}


double
MSCFModel_W99::followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double /*predMaxDecel*/, const MSVehicle* const pred) const {
    const double dx = gap2pred + myType->getMinGap();
    const double dv = predSpeed - speed;

    const double cc0 = myType->getMinGap();

    const double leaderAccel = pred != nullptr ? pred->getAcceleration() : 0;
    const double oldAccel = veh->getAcceleration();

    const double rndVal = speed > 0 ? RandHelper::rand(veh->getRNG()) - 0.5 : 0.5;
    double sdxc, sdxo, sdxv;
    computeThresholds(speed, predSpeed, leaderAccel, rndVal, sdxc, sdxo, sdxv);

    const double sdv = myCC6 * dx * dx / 10000;
    const double sdvc = speed > 0 ? myCC4 - sdv : 0; //minimum closing dv
    const double sdvo = predSpeed > myCC5 ? sdv + myCC5 : sdv; //minimum opening dv

    double accel = 0; // resulting acceleration
    int status = 0;

    if (dv < sdvo && dx <= sdxc) {
        // 'Decelerate - Increase Distance';
        accel = 0;
        if (predSpeed > 0) {
            if (dv < 0) {
                if (dx > cc0) {
                    accel = MIN2(leaderAccel + dv * dv / (cc0 - dx), 0.0);
                    status = 0;
                } else {
                    accel = MIN2(leaderAccel + 0.5 * (dv - sdvo), 0.0);
                    status = 1;
                }
            }
            if (accel > -myCC7) {
                accel = -myCC7;
                status = 2;
            } else {
                accel = MAX2(accel, -10 + 0.5 * sqrt(speed));
                status = 3;
            }
        }

    } else if (dv < sdvc && dx < sdxv) {
        // 'Decelerate - Decrease Distance';
        accel = 0.5 * dv * dv / (sdxc - dx - 0.1);
        status = 4;
        // deceleration is capped by myEmergencyDecel in MSCFModel::finalizeSpeed
    } else if (dv < sdvo && dx < sdxo) {
        // 'Accelerate/Decelerate - Keep Distance';
        if (oldAccel <= 0) {
            accel = MIN2(oldAccel, -myCC7);
            status = 5;
        } else {
            accel = MAX2(oldAccel, myCC7);
            status = 6;
            // acceleration is capped at desired speed in MSCFModel::finalizeSpeed
        }
    } else {
        // 'Accelerate/Relax - Increase/Keep Speed';
        if (dx > sdxc) {
            //if (follower.status === 'w') {
            //    accel = cc7;
            //} else
            {
                const double accelMax = myCC8 + myCC9 * MIN2(speed, 80 / 3.6) + RandHelper::rand(veh->getRNG());
                if (dx < sdxo) {
                    accel = MIN2(dv * dv / (sdxo - dx), accelMax);
                    status = 7;
                } else {
                    accel = accelMax;
                    status = 8;
                }
            }
            // acceleration is capped at desired speed in MSCFModel::finalizeSpeed
        }
    }
    double vNew = speed + ACCEL2SPEED(accel);
#ifdef DEBUG_FOLLOW_SPEED
    if (DEBUG_COND) {
        std::cout << SIMTIME << " W99::fS veh=" << veh->getID() << " pred=" << Named::getIDSecure(pred)
                  << " v=" << speed << " pV=" << predSpeed << " g=" << gap2pred
                  << " dv=" << dv << " dx=" << dx
                  << " sdxc=" << sdxc << " sdxo=" << sdxo << " sdxv=" << sdxv << " sdv=" << sdv << " sdvo=" << sdvo << " sdvc=" << sdvc
                  << " st=" << status
                  << " a=" << accel << " V=" << vNew << "\n";
    }
#else
    UNUSED_PARAMETER(status);
#endif
    if (MSGlobals::gSemiImplicitEulerUpdate) {
        vNew = MAX2(0.0, vNew);
    }
    return vNew;
}



double
MSCFModel_W99::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    // see reasoning in MSCFModel_Wiedemann::stopSpeed
    return MIN2(maximumSafeStopSpeed(gap, speed, false, veh->getActionStepLengthSecs()), maxNextSpeed(speed, veh));
}


double
MSCFModel_W99::interactionGap(const MSVehicle* const, double vL) const {
    UNUSED_PARAMETER(vL);
    return INTERACTION_GAP;
}


MSCFModel*
MSCFModel_W99::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_W99(vtype);
}

