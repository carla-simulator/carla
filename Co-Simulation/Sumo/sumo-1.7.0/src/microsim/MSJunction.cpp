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
/// @file    MSJunction.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 12 Dez 2001
///
// The base class for an intersection
/****************************************************************************/
#include <config.h>

#include "MSVehicle.h"
#include "MSEdge.h"
#include "MSJunction.h"






// ===========================================================================
// debug constants
// ===========================================================================
//#define DEBUG_LINKLEADER
//#define DEBUG_COND (ego->isSelected())
//#define DEBUG_COND (true)



// ===========================================================================
// class declarations
// ===========================================================================
class MSLink;

// ===========================================================================
// static member definitions
// ===========================================================================

// ===========================================================================
// member method definition
// ===========================================================================
MSJunction::MSJunction(const std::string& id, SumoXMLNodeType type, const Position& position,
                       const PositionVector& shape, const std::string& name) :
    Named(id),
    myType(type),
    myPosition(position),
    myShape(shape),
    myName(name) {
}


MSJunction::~MSJunction() {}


const Position&
MSJunction::getPosition() const {
    return myPosition;
}


void
MSJunction::postloadInit() {}


int
MSJunction::getNrOfIncomingLanes() const {
    int nr = 0;
    for (const MSEdge* e : myIncoming) {
        nr += (int)e->getLanes().size();
    }
    return nr;
}


/****************************************************************************/
