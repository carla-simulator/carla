/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSCFModel_PWag2009.cpp
/// @author  Peter Wagner
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    03.04.2010
///
// Scalable model based on Krauss by Peter Wagner
/****************************************************************************/
#include <config.h>

#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_PWag2009.h"
#include <utils/common/RandHelper.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_PWag2009::MSCFModel_PWag2009(const MSVehicleType* vtype) :
    MSCFModel(vtype),
    myDawdle(vtype->getParameter().getCFParam(SUMO_ATTR_SIGMA, SUMOVTypeParameter::getDefaultImperfection(vtype->getParameter().vehicleClass))),
    myTauDecel(myDecel * myHeadwayTime),
    myDecelDivTau(myDecel / myHeadwayTime),
    myTauLastDecel(myDecel * vtype->getParameter().getCFParam(SUMO_ATTR_CF_PWAGNER2009_TAULAST, 0.3)),
    myActionPointProbability(vtype->getParameter().getCFParam(SUMO_ATTR_CF_PWAGNER2009_APPROB, 0.5)) {
    // PWag2009 does not drive very precise and may violate minGap on occasion
    myCollisionMinGapFactor = vtype->getParameter().getCFParam(SUMO_ATTR_COLLISION_MINGAP_FACTOR, 0.1);
}


MSCFModel_PWag2009::~MSCFModel_PWag2009() {}


double
MSCFModel_PWag2009::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    const double vNext = MSCFModel::finalizeSpeed(veh, vPos);
    VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
    double apref = SPEED2ACCEL(vNext - veh->getSpeed());
    vars->aOld = apref;
    return vNext;
}

double
MSCFModel_PWag2009::patchSpeedBeforeLC(const MSVehicle* veh, double vMin, double vMax) const {
    UNUSED_PARAMETER(veh);
    UNUSED_PARAMETER(vMin);
    return vMax;
}

// in addition, the parameters myTauLast, probAP, and sigmaAcc are needed; sigmaAcc can use myDawdle
// myTauLast might use the current time-step size, but this yields eventually an extreme model, I would be
// more careful and set it to something around 0.3 or 0.4, which are among the shortest headways I have
// seen so far in data ...

double
MSCFModel_PWag2009::followSpeed(const MSVehicle* const veh, double speed, double gap, double predSpeed, double /*predMaxDecel*/, const MSVehicle* const /*pred*/) const {
    if (predSpeed == 0 && gap < 0.01) {
        return 0;
    }
    const double vsafe = -myTauLastDecel + sqrt(myTauLastDecel * myTauLastDecel + predSpeed * predSpeed + 2.0 * myDecel * gap);
    const double asafe = SPEED2ACCEL(vsafe - speed);
    VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
    double apref = vars->aOld;
    if (apref <= asafe && RandHelper::rand(veh->getRNG()) <= myActionPointProbability * TS) {
        apref = myDecelDivTau * (gap + (predSpeed - speed) * myHeadwayTime - speed * myHeadwayTime) / (speed + myTauDecel);
        apref = MIN2(apref, myAccel);
        apref = MAX2(apref, -myDecel);
        apref += myDawdle * RandHelper::rand((double) - 1., (double)1., veh->getRNG());
    }
    if (apref > asafe) {
        apref = asafe;
    }
    return MAX2(0., speed + ACCEL2SPEED(apref));
}

// uses the safe speed and preferred acceleration with the same NORMAL tau to compute stopSpeed
double
MSCFModel_PWag2009::stopSpeed(const MSVehicle* const /* veh */, const double speed, double gap) const {
    if (gap < 0.01) {
        return 0.;
    }
    const double vsafe = -myTauDecel + sqrt(myTauDecel * myTauDecel +  2.0 * myDecel * gap);
    const double asafe = SPEED2ACCEL(vsafe - speed);
//    VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
    double apref = myDecelDivTau * (gap  - 2 * speed * myHeadwayTime) / (speed + myTauDecel);
    if (apref <= asafe) {
        apref = MIN2(apref, myAccel);
        apref = MAX2(apref, -myDecel);
    } else {
        apref = asafe;
    }
    return MAX2(0., vsafe + ACCEL2SPEED(apref));
}


MSCFModel*
MSCFModel_PWag2009::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_PWag2009(vtype);
}
