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
/// @file    AGChild.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Person in age to go to school: linked to a school object
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <vector>
#include <limits>
#include "AGChild.h"
#include "AGSchool.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
AGChild::print() const {
    std::cout << "- Child: Age=" << age << " School=" << mySchool << std::endl;
}

bool
AGChild::setSchool(AGSchool* school) {
    if (school == nullptr) {
        return false;
    }
    bool enoughPlace = school->addNewChild();
    if (enoughPlace) {
        mySchool = school;
    }
    return enoughPlace;
}

bool
AGChild::allocateASchool(std::list<AGSchool>* schools, AGPosition housePos) {
    double minDist = std::numeric_limits<double>::infinity();
    AGSchool* sch = nullptr;
    if (schools->size() == 0) {
        return false;
    }
    std::list<AGSchool>::iterator it;

    for (it = schools->begin(); it != schools->end(); ++it) {
        if (it->acceptThisAge(age) && it->getPlaces() > 0 && housePos.distanceTo(it->getPosition()) < minDist) {
            minDist = housePos.distanceTo(it->getPosition());
            sch = &(*it);
        }
    }
    return setSchool(sch);
}

bool
AGChild::leaveSchool() {
    if (mySchool != nullptr)
        if (!mySchool->removeChild()) {
            return false;
        }
    mySchool = nullptr;
    return true;
}

bool
AGChild::haveASchool() const {
    return (mySchool != nullptr);
}

AGPosition
AGChild::getSchoolLocation() const {
    return mySchool->getPosition();
}

int
AGChild::getSchoolClosing() const {
    return mySchool->getClosingHour();
}

int
AGChild::getSchoolOpening() const {
    return mySchool->getOpeningHour();
}


/****************************************************************************/
