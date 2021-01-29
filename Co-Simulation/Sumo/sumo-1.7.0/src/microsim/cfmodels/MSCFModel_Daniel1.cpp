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
/// @file    MSCFModel_Daniel1.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 05 Jun 2012
///
// The original Krauss (1998) car-following model and parameter
/****************************************************************************/
#include <config.h>

#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_Daniel1.h"
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <utils/common/RandHelper.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_Daniel1::MSCFModel_Daniel1(const MSVehicleType* vtype) :
    MSCFModel(vtype),
    myDawdle(vtype->getParameter().getCFParam(SUMO_ATTR_SIGMA, SUMOVTypeParameter::getDefaultImperfection(vtype->getParameter().vehicleClass))),
    myTauDecel(myDecel * myHeadwayTime),
    myTmp1(vtype->getParameter().getCFParam(SUMO_ATTR_TMP1, 1.0)),
    myTmp2(vtype->getParameter().getCFParam(SUMO_ATTR_TMP2, 1.0)),
    myTmp3(vtype->getParameter().getCFParam(SUMO_ATTR_TMP3, 1.0)),
    myTmp4(vtype->getParameter().getCFParam(SUMO_ATTR_TMP4, 1.0)),
    myTmp5(vtype->getParameter().getCFParam(SUMO_ATTR_TMP5, 1.0)) {
}


MSCFModel_Daniel1::~MSCFModel_Daniel1() {}


double
MSCFModel_Daniel1::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    const double oldV = veh->getSpeed(); // save old v for optional acceleration computation
    const double vSafe = MIN2(vPos, veh->processNextStop(vPos)); // process stops
    // we need the acceleration for emission computation;
    //  in this case, we neglect dawdling, nonetheless, using
    //  vSafe does not incorporate speed reduction due to interaction
    //  on lane changing
    const double vMin = getSpeedAfterMaxDecel(oldV);
    const double vMax = MIN3(veh->getLane()->getVehicleMaxSpeed(veh), maxNextSpeed(oldV, veh), vSafe);
#ifdef _DEBUG
    if (vMin > vMax) {
        WRITE_WARNING("Maximum speed of vehicle '" + veh->getID() + "' is lower than the minimum speed (min: " + toString(vMin) + ", max: " + toString(vMax) + ").");
    }
#endif
    return veh->getLaneChangeModel().patchSpeed(vMin, MAX2(vMin, dawdle(vMax, veh->getRNG())), vMax, *this);
}


double
MSCFModel_Daniel1::followSpeed(const MSVehicle* const veh, double speed, double gap, double predSpeed, double /*predMaxDecel*/, const MSVehicle* const /*pred*/) const {
    return MIN2(_vsafe(gap, predSpeed), maxNextSpeed(speed, veh));
}


double
MSCFModel_Daniel1::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    return MIN2(_vsafe(gap, 0), maxNextSpeed(speed, veh));
}


double
MSCFModel_Daniel1::dawdle(double speed, std::mt19937* rng) const {
    return MAX2(0., speed - ACCEL2SPEED(myDawdle * myAccel * RandHelper::rand(rng)));
}


/** Returns the SK-vsafe. */
double MSCFModel_Daniel1::_vsafe(double gap, double predSpeed) const {
    if (predSpeed == 0 && gap < 0.01) {
        return 0;
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
MSCFModel_Daniel1::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_Daniel1(vtype);
}
