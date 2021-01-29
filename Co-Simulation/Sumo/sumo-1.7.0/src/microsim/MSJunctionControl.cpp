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
/// @file    MSJunctionControl.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Tue, 06 Mar 2001
///
// Container for junctions; performs operations on all stored junctions
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include "MSInternalJunction.h"
#include "MSJunctionControl.h"


// ===========================================================================
// member method definitions
// ===========================================================================
MSJunctionControl::MSJunctionControl() {
}


MSJunctionControl::~MSJunctionControl() {
}


void
MSJunctionControl::postloadInitContainer() {
    // initialize normal junctions before internal junctions
    // (to allow calling getIndex() during initialization of internal junction links)
    for (const auto& i : *this) {
        if (i.second->getType() != SumoXMLNodeType::INTERNAL) {
            i.second->postloadInit();
        }
    }
    for (const auto& i : *this) {
        if (i.second->getType() == SumoXMLNodeType::INTERNAL) {
            i.second->postloadInit();
        }
    }
}


/****************************************************************************/
