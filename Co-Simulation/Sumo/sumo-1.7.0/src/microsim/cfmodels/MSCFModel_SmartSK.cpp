/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSCFModel_SmartSK.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Peter Wagner
/// @date    Tue, 05 Jun 2012
///
// A smarter SK
/****************************************************************************/
#include <config.h>

#include <map>
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_SmartSK.h"
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <utils/common/RandHelper.h>

//#define SmartSK_DEBUG

// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_SmartSK::MSCFModel_SmartSK(const MSVehicleType* vtype) :
// check whether setting these variables here with default values is ''good'' SUMO design
//        double tmp1=0.0, double tmp2=5.0, double tmp3=0.0, double tmp4, double tmp5)
    MSCFModel(vtype),
    myDawdle(vtype->getParameter().getCFParam(SUMO_ATTR_SIGMA, SUMOVTypeParameter::getDefaultImperfection(vtype->getParameter().vehicleClass))),
    myTauDecel(myDecel * myHeadwayTime),
    myTmp1(vtype->getParameter().getCFParam(SUMO_ATTR_TMP1, 1.0)),
    myTmp2(vtype->getParameter().getCFParam(SUMO_ATTR_TMP2, 1.0)),
    myTmp3(vtype->getParameter().getCFParam(SUMO_ATTR_TMP3, 1.0)),
    myTmp4(vtype->getParameter().getCFParam(SUMO_ATTR_TMP4, 1.0)),
    myTmp5(vtype->getParameter().getCFParam(SUMO_ATTR_TMP5, 1.0)) {
    // the variable tmp1 is the acceleration delay time, e.g. two seconds (or something like this).
    // for use in the upate process, a rule like if (v<myTmp1) vsafe = 0; is needed.
    // To have this, we have to transform myTmp1 (which is a time) into an equivalent speed. This is done by the
    // using the vsafe formula and computing:
    // v(t=myTmp1) = -myTauDecel + sqrt(myTauDecel*myTauDecel + accel*(accel + decel)*t*t + accel*decel*t*TS);
    double t = myTmp1;
    myS2Sspeed = -myTauDecel + sqrt(myTauDecel * myTauDecel + myAccel * (myAccel + myDecel) * t * t + myAccel * myDecel * t * TS);
#ifdef SmartSK_DEBUG
    std::cout << "# s2s-speed: " << myS2Sspeed << std::endl;
#endif
    if (myS2Sspeed > 5.0) {
        myS2Sspeed = 5.0;
    }
// double maxDeltaGap = -0.5*ACCEL2DIST(myDecel + myAccel);
    maxDeltaGap = -0.5 * (myDecel + myAccel) * TS * TS;
#ifdef SmartSK_DEBUG
    std::cout << "# maxDeltaGap = " << maxDeltaGap << std::endl;
#endif
    myTmp2 = TS / myTmp2;
    myTmp3 = sqrt(TS) * myTmp3;
}


MSCFModel_SmartSK::~MSCFModel_SmartSK() {}


double
MSCFModel_SmartSK::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    const double vNext = MSCFModel::finalizeSpeed(veh, vPos);
    updateMyHeadway(veh);
    SSKVehicleVariables* vars = (SSKVehicleVariables*)veh->getCarFollowVariables();
#ifdef SmartSK_DEBUG
    if (vars->ggOld.size() > 1) {
        std::cout << "# more than one entry in ggOld list. Speed is " << vPos << ", corresponding dist is " << vars->ggOld[(int) vPos] << "\n";
        for (std::map<int, double>::iterator I = vars->ggOld.begin(); I != vars->ggOld.end(); I++) {
            std::cout << "# " << (*I).first << ' ' << (*I).second << std::endl;
        }
    }
#endif
    vars->gOld = vars->ggOld[(int) vPos];
    vars->ggOld.clear();
    return vNext;
}

double
MSCFModel_SmartSK::followSpeed(const MSVehicle* const veh, double speed, double gap, double predSpeed, double /*predMaxDecel*/, const MSVehicle* const /*pred*/) const {
    SSKVehicleVariables* vars = (SSKVehicleVariables*)veh->getCarFollowVariables();

// if (((gap - vars->gOld) < maxDeltaGap) && (speed>=5.0) && gap>=5.0) {
    if ((gap - vars->gOld) < maxDeltaGap) {
        double tTauTest = gap / speed;
// allow  headway only to decrease only, never to increase. Increase is handled automatically by the headway dynamics in finalizeSpeed()!!!
        if ((tTauTest < vars->myHeadway) && (tTauTest > TS)) {
            vars->myHeadway = tTauTest;
        }
    }

    double vsafe = _vsafe(veh, gap, predSpeed);
    if ((speed <= 0.0) && (vsafe < myS2Sspeed)) {
        vsafe = 0;
    }

    double vNew = MAX2(getSpeedAfterMaxDecel(speed), MIN2(vsafe, maxNextSpeed(speed, veh)));
    // there must be a better place to do the following assignment!!!
    vars->gOld = gap;
    vars->ggOld[(int)vNew] = gap;
    return vNew;
}

double
MSCFModel_SmartSK::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    SSKVehicleVariables* vars = (SSKVehicleVariables*)veh->getCarFollowVariables();

// if (((gap - vars->gOld) < maxDeltaGap) && (speed>=5.0) && gap>=5.0) {
    if ((gap - vars->gOld) < maxDeltaGap) {
        double tTauTest = gap / speed;
// allow  headway only to decrease only, never to increase. Increase is handled automatically by the headway dynamics in finalizeSpeed()!!!
        if ((tTauTest < vars->myHeadway) && (tTauTest > TS)) {
            vars->myHeadway = tTauTest;
        }
    }

    return MAX2(getSpeedAfterMaxDecel(speed), MIN2(_vsafe(veh, gap, 0), maxNextSpeed(speed, veh)));
}

double
MSCFModel_SmartSK::patchSpeedBeforeLC(const MSVehicle* veh, double /*vMin*/, double /*vMax*/) const {
    return dawdle(veh->getSpeed(), veh->getRNG());
}

double
MSCFModel_SmartSK::dawdle(double speed, std::mt19937* rng) const {
    return MAX2(0., speed - ACCEL2SPEED(myDawdle * myAccel * RandHelper::rand(rng)));
}


/** Returns the SK-vsafe. */
double MSCFModel_SmartSK::_vsafe(const MSVehicle* const veh, double gap, double predSpeed) const {
    if (predSpeed == 0 && gap < 0.01) {
        return 0;
    }
    SSKVehicleVariables* vars = (SSKVehicleVariables*)veh->getCarFollowVariables();
    // this is the most obvious change to the normal SK: the model uses the variable vars->myHeadway instead of the constant
    // myHeadwayTime as the "desired headway" tau
    double bTau = myDecel * (vars->myHeadway);
    double vsafe = (double)(-1. * bTau
                            + sqrt(
                                bTau * bTau
                                + (predSpeed * predSpeed)
                                + (2. * myDecel * gap)
                            ));
    assert(vsafe >= 0);
    return vsafe;
}


MSCFModel*
MSCFModel_SmartSK::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_SmartSK(vtype);
}
