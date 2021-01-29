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
/// @file    MSCFModel_KraussPS.cpp
/// @author  Tobias Mayer
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Mon, 04 Aug 2009
///
// Krauss car-following model, changing accel and speed by slope
/****************************************************************************/
#include <config.h>

#include <utils/geom/GeomHelper.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSLane.h>
#include "MSCFModel_KraussPS.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSCFModel_KraussPS::MSCFModel_KraussPS(const MSVehicleType* vtype) :
    MSCFModel_Krauss(vtype) {
}


MSCFModel_KraussPS::~MSCFModel_KraussPS() {}


double
MSCFModel_KraussPS::maxNextSpeed(double speed, const MSVehicle* const veh) const {
    const double gravity = 9.80665;
    const double aMax = MAX2(0., getMaxAccel() - gravity * sin(DEG2RAD(veh->getSlope())));
    // assuming drag force is proportional to the square of speed
    const double vMax = MAX2(
                            sqrt(aMax / getMaxAccel()) * myType->getMaxSpeed(),
                            // prevent emergency braking when inclination changes suddenly (momentum)
                            speed - ACCEL2SPEED(getMaxDecel()));
    return MAX2(
               // prevent stalling at low speed
               getMaxAccel() / 2,
               MIN2(speed + ACCEL2SPEED(aMax), vMax));
}


MSCFModel*
MSCFModel_KraussPS::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_KraussPS(vtype);
}


/****************************************************************************/
