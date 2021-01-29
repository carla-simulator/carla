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
/// @file    GUIMEVehicleControl.cpp
/// @author  Jakob Erdmann
/// @date    Okt 2012
///
// The class responsible for building and deletion of meso vehicles (gui-version)
/****************************************************************************/
#include <config.h>

#include <fx.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <gui/GUIGlobals.h>
#include <microsim/MSRouteHandler.h>
#include "GUIMEVehicleControl.h"
#include "GUIMEVehicle.h"


// ===========================================================================
// member method definitions
// ===========================================================================
GUIMEVehicleControl::GUIMEVehicleControl()
    : MEVehicleControl() {}


GUIMEVehicleControl::~GUIMEVehicleControl() {
    // just to quit cleanly on a failure
    if (myLock.locked()) {
        myLock.unlock();
    }
}


SUMOVehicle*
GUIMEVehicleControl::buildVehicle(SUMOVehicleParameter* defs,
                                  const MSRoute* route, MSVehicleType* type,
                                  const bool ignoreStopErrors, const bool fromRouteFile) {
    MSBaseVehicle* built = new GUIMEVehicle(defs, route, type, type->computeChosenSpeedDeviation(fromRouteFile ? MSRouteHandler::getParsingRNG() : nullptr));
    initVehicle(built, ignoreStopErrors);
    return built;
}



bool
GUIMEVehicleControl::addVehicle(const std::string& id, SUMOVehicle* v) {
    FXMutexLock locker(myLock);
    return MEVehicleControl::addVehicle(id, v);
}


void
GUIMEVehicleControl::deleteVehicle(SUMOVehicle* veh, bool discard) {
    FXMutexLock locker(myLock);
    MEVehicleControl::deleteVehicle(veh, discard);
}


void
GUIMEVehicleControl::insertVehicleIDs(std::vector<GUIGlID>& into) {
    FXMutexLock locker(myLock);
    into.reserve(myVehicleDict.size());
    for (VehicleDictType::iterator i = myVehicleDict.begin(); i != myVehicleDict.end(); ++i) {
        SUMOVehicle* veh = (*i).second;
        if (veh->isOnRoad()) {
            into.push_back(static_cast<GUIMEVehicle*>((*i).second)->getGlID());
        }
    }
}



void
GUIMEVehicleControl::secureVehicles() {
    myLock.lock();
}


void
GUIMEVehicleControl::releaseVehicles() {
    myLock.unlock();
}


/****************************************************************************/
