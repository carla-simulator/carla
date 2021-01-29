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
/// @file    AGStreet.cpp
/// @author  Piotr Woznica
/// @author  Walter Bamberger
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    July 2010
///
// Represents a SUMO edge and contains people and work densities
/****************************************************************************/
#include <config.h>

#include "AGStreet.h"
#include "router/ROEdge.h"
#include <iostream>


// ===========================================================================
// method definitions
// ===========================================================================
AGStreet::AGStreet(const std::string& id, RONode* from, RONode* to, int index, const int priority) :
    ROEdge(id, from, to, index, priority), myPopulation(0.), myNumWorkplaces(0.) {
}


void
AGStreet::print() const {
    std::cout << "- AGStreet: Name=" << getID() << " Length=" << getLength() << " pop=" << myPopulation << " work=" << myNumWorkplaces << std::endl;
}


double
AGStreet::getPopulation() const {
    return myPopulation;
}


void
AGStreet::setPopulation(const double population) {
    myPopulation = population;
}


double
AGStreet::getWorkplaceNumber() const {
    return myNumWorkplaces;
}


void
AGStreet::setWorkplaceNumber(const double workPositions) {
    myNumWorkplaces = workPositions;
}


bool
AGStreet::allows(const SUMOVehicleClass vclass) const {
    return (getPermissions() & vclass) == vclass;
}


/****************************************************************************/
