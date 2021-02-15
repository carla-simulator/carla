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
/// @file    MEVehicleControl.cpp
/// @author  Daniel Krajzewicz
/// @date    Wed, 10. Dec 2003
///
// The class responsible for building and deletion of vehicles (meso-version)
/****************************************************************************/
#include <config.h>

#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/iodevices/OutputDevice.h>
#include <microsim/MSNet.h>
#include <microsim/MSRouteHandler.h>
#include <microsim/MSVehicleType.h>
#include "MESegment.h"
#include "MEVehicle.h"
#include "MEVehicleControl.h"


// ===========================================================================
// member method definitions
// ===========================================================================
MEVehicleControl::MEVehicleControl()
    : MSVehicleControl() {}


MEVehicleControl::~MEVehicleControl() {}


SUMOVehicle*
MEVehicleControl::buildVehicle(SUMOVehicleParameter* defs,
                               const MSRoute* route, MSVehicleType* type,
                               const bool ignoreStopErrors, const bool fromRouteFile) {
    MEVehicle* built = new MEVehicle(defs, route, type, type->computeChosenSpeedDeviation(fromRouteFile ? MSRouteHandler::getParsingRNG() : nullptr));
    initVehicle(built, ignoreStopErrors);
    return built;
}


/****************************************************************************/
