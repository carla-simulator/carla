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
/// @file    MSCFModel_KraussOrig1.cpp
/// @author  Tobias Mayer
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Mon, 04 Aug 2009
///
// The original Krauss (1998) car-following model and parameter
/****************************************************************************/
#include <config.h>

#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_KraussOrig1.h"
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <utils/common/RandHelper.h>
#include <microsim/MSGlobals.h>

// ===========================================================================
// DEBUG constants
// ===========================================================================
//#define DEBUG_COND (veh->getID()=="disabled")

// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_KraussOrig1::MSCFModel_KraussOrig1(const MSVehicleType* vtype) :
    MSCFModel(vtype),
    myDawdle(vtype->getParameter().getCFParam(SUMO_ATTR_SIGMA, SUMOVTypeParameter::getDefaultImperfection(vtype->getParameter().vehicleClass))),
    myTauDecel(myDecel * myHeadwayTime) {
}


MSCFModel_KraussOrig1::~MSCFModel_KraussOrig1() {}

double
MSCFModel_KraussOrig1::patchSpeedBeforeLC(const MSVehicle* veh, double vMin, double vMax) const {
    UNUSED_PARAMETER(veh);
    const double vDawdle = MAX2(vMin, dawdle(vMax, veh->getRNG()));
    return vDawdle;
}


double
MSCFModel_KraussOrig1::followSpeed(const MSVehicle* const veh, double speed, double gap, double predSpeed, double predMaxDecel, const MSVehicle* const /*pred*/) const {
    if (MSGlobals::gSemiImplicitEulerUpdate) {
        return MIN2(vsafe(gap, predSpeed, predMaxDecel), maxNextSpeed(speed, veh)); // XXX: and why not cap with minNextSpeed!? (Leo)
    } else {
        return MAX2(MIN2(maximumSafeFollowSpeed(gap, speed, predSpeed, predMaxDecel), maxNextSpeed(speed, veh)), minNextSpeed(speed));
    }
}


double
MSCFModel_KraussOrig1::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    if (MSGlobals::gSemiImplicitEulerUpdate) {
        return MIN2(vsafe(gap, 0., 0.), maxNextSpeed(speed, veh));
    } else {
        // XXX: using this here is probably in the spirit of Krauss, but we should consider,
        // if the original vsafe should be kept instead (Leo), refs. #2575
        return MIN2(maximumSafeStopSpeedBallistic(gap, speed), maxNextSpeed(speed, veh));
    }
}


double
MSCFModel_KraussOrig1::dawdle(double speed, std::mt19937* rng) const {
    if (!MSGlobals::gSemiImplicitEulerUpdate) {
        // in case of the ballistic update, negative speeds indicate
        // a desired stop before the completion of the next timestep.
        // We do not allow dawdling to overwrite this indication
        if (speed < 0) {
            return speed;
        }
    }
    return MAX2(0., speed - ACCEL2SPEED(myDawdle * myAccel * RandHelper::rand(rng)));
}


/** Returns the SK-vsafe. */
double MSCFModel_KraussOrig1::vsafe(double gap, double predSpeed, double /* predMaxDecel */) const {
    if (predSpeed == 0 && gap < 0.01) {
        return 0;
    } else if (predSpeed == 0 &&  gap <= ACCEL2SPEED(myDecel)) {
        // workaround for #2310
        return MIN2(ACCEL2SPEED(myDecel), DIST2SPEED(gap));
    }
    double vsafe = (double)(-1. * myTauDecel
                            + sqrt(
                                myTauDecel * myTauDecel
                                + (predSpeed * predSpeed)
                                + (2. * myDecel * gap)
                            ));
    assert(vsafe >= 0);
    return vsafe;
}


MSCFModel*
MSCFModel_KraussOrig1::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_KraussOrig1(vtype);
}


/****************************************************************************/
