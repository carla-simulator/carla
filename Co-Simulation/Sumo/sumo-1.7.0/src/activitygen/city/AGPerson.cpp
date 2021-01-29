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
/// @file    AGPerson.cpp
/// @author  Piotr Woznica
/// @author  Walter Bamberger
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    July 2010
///
// Parent object of every person, contains age and any natural characteristic
/****************************************************************************/
#include <config.h>

#include "AGPerson.h"
#include <utils/common/RandHelper.h>
#include <iostream>


// ===========================================================================
// method definitions
// ===========================================================================
AGPerson::AGPerson(int age) : age(age) {}


AGPerson::~AGPerson() {}


void
AGPerson::print() const {
    std::cout << "- Person: Age=" << age << std::endl;
}


int
AGPerson::getAge() const {
    return age;
}


bool
AGPerson::decide(double proba) const {
    return (RandHelper::rand(1000) < static_cast<int>(1000.0f * proba));
}


/****************************************************************************/
