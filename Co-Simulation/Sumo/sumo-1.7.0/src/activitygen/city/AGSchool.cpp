/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    AGSchool.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
///
// Correspond to given ages and referenced by children. Has a precise location.
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <string>
#include "AGSchool.h"
#include "AGPosition.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
AGSchool::print() const {
    std::cout << "- school: " << " placeNbr=" << capacity << " hours=[" << opening << ";" << closing << "] ages=[" << beginAge << ";" << endAge << "]" << std::endl;
}

int
AGSchool::getPlaces() {
    return capacity;
}

bool
AGSchool::addNewChild() {
    if (capacity > 0) {
        --capacity;
        return true;
    }
    return false;
}

bool
AGSchool::removeChild() {
    if (capacity < initCapacity) {
        ++capacity;
        return true;
    }
    return false;
}

bool
AGSchool::acceptThisAge(int age) {
    if (age <= endAge && age >= beginAge) {
        return true;
    }
    return false;
}

int
AGSchool::getBeginAge() {
    return beginAge;
}

int
AGSchool::getEndAge() {
    return endAge;
}

AGPosition
AGSchool::getPosition() {
    return location;
}

int
AGSchool::getClosingHour() {
    return closing;
}

int
AGSchool::getOpeningHour() {
    return opening;
}


/****************************************************************************/
