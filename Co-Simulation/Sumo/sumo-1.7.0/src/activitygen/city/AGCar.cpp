/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    AGCar.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Cars owned by people of the city: included in households.
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <sstream>
#include <string>
#include "AGCar.h"
#include "AGAdult.h"


// ===========================================================================
// method definitions
// ===========================================================================
std::string
AGCar::createName(int idHH, int idCar) {
    std::ostringstream os;
    os << "h" << idHH << "c" << idCar;
    return os.str();
}

bool
AGCar::associateTo(AGAdult* pers) {
    if (currentUser == nullptr) {
        currentUser = pers;
        return true;
    }
    return false;
}

bool
AGCar::isAssociated() const {
    return (currentUser != nullptr);
}

std::string
AGCar::getName() const {
    return idName;
}


/****************************************************************************/
