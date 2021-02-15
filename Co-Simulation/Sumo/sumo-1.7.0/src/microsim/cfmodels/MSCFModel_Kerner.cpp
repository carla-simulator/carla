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
/// @file    MSCFModel_Kerner.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    03.04.2010
///
// car-following model by B. Kerner
/****************************************************************************/
#include <config.h>

#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_Kerner.h"
#include <utils/common/RandHelper.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_Kerner::MSCFModel_Kerner(const MSVehicleType* vtype) :
    MSCFModel(vtype),
    myK(vtype->getParameter().getCFParam(SUMO_ATTR_K, 0.5)),
    myPhi(vtype->getParameter().getCFParam(SUMO_ATTR_CF_KERNER_PHI, 5.0)),
    myTauDecel(myDecel * myHeadwayTime) {
    // Kerner does not drive very precise and may violate minGap on occasion
    myCollisionMinGapFactor = vtype->getParameter().getCFParam(SUMO_ATTR_COLLISION_MINGAP_FACTOR, 0.1);
}


MSCFModel_Kerner::~MSCFModel_Kerner() {}


double
MSCFModel_Kerner::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    const double vNext = MSCFModel::finalizeSpeed(veh, vPos);
    VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
    vars->rand = RandHelper::rand(veh->getRNG());
    return vNext;
}


double
MSCFModel_Kerner::followSpeed(const MSVehicle* const veh, double speed, double gap, double predSpeed, double /*predMaxDecel*/, const MSVehicle* const /*pred*/) const {
    return MIN2(_v(veh, speed, maxNextSpeed(speed, veh), gap, predSpeed), maxNextSpeed(speed, veh));
}


double
MSCFModel_Kerner::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    return MIN2(_v(veh, speed, maxNextSpeed(speed, veh), gap, 0), maxNextSpeed(speed, veh));
}


MSCFModel::VehicleVariables*
MSCFModel_Kerner::createVehicleVariables() const {
    VehicleVariables* ret = new VehicleVariables();
    /// XXX should use egoVehicle->getRNG()
    ret->rand = RandHelper::rand();
    return ret;
}


double
MSCFModel_Kerner::_v(const MSVehicle* const veh, double speed, double vfree, double gap, double predSpeed) const {
    if (predSpeed == 0 && gap < 0.01) {
        return 0;
    }
    // !!! in the following, the prior step is not considered!!!
    double G = MAX2((double) 0, (double)(SPEED2DIST(myK * speed) + myPhi / myAccel * speed * (speed - predSpeed)));
    double vcond = gap > G ? speed + ACCEL2SPEED(myAccel) : speed + MAX2(ACCEL2SPEED(-myDecel), MIN2(ACCEL2SPEED(myAccel), predSpeed - speed));
    double vsafe = (double)(-1. * myTauDecel + sqrt(myTauDecel * myTauDecel + (predSpeed * predSpeed) + (2. * myDecel * gap)));
    VehicleVariables* vars = (VehicleVariables*)veh->getCarFollowVariables();
    double va = MAX2((double) 0, MIN3(vfree, vsafe, vcond)) + vars->rand;
    //std::cout << SIMTIME << " veh=" << veh->getID() << " speed=" << speed << " gap=" << gap << " G=" << G << " predSpeed=" << predSpeed << " vfree=" << vfree << " vsafe=" << vsafe << " vcond=" << vcond << " rand=" << vars->rand << "\n";
    double v = MAX2((double) 0, MIN4(vfree, va, speed + ACCEL2SPEED(myAccel), vsafe));
    return v;
}


MSCFModel*
MSCFModel_Kerner::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_Kerner(vtype);
}
