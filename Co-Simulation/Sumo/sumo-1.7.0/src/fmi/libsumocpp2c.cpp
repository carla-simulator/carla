/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2020-2020 German Aerospace Center (DLR) and others.
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
/// @file    libsumocpp2c.cpp
/// @author  Robert Hilbrich
/// @date    Mon, 17 Aug 2020
///
// Implementation of the libsumo c++ to c wrapper
/****************************************************************************/


#include <libsumo/Simulation.h>
#include <utils/geom/PositionVector.h>
#include <libsumo/Vehicle.h>

#include "libsumocpp2c.h"

void
libsumo_load() {
    std::vector<std::string> options = {"-c", "/Users/robert/Git-Repositories/SUMO/tools/game/grid6.sumocfg"};
    libsumo::Simulation::load(options);
}

int
libsumo_vehicle_getIDCount() {
    return libsumo::Vehicle::getIDCount();
}

void
libsumo_close() {
    libsumo::Simulation::close();
}

void
libsumo_step(double time) {
    libsumo::Simulation::step(time);
}
