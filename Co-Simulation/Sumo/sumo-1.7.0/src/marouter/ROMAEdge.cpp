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
/// @file    ROMAEdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christian Roessel
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A basic edge for routing applications
/****************************************************************************/
#include <config.h>

#include "ROMAEdge.h"


// ===========================================================================
// method definitions
// ===========================================================================
ROMAEdge::ROMAEdge(const std::string& id, RONode* from, RONode* to, int index, const int priority)
    : ROEdge(id, from, to, index, priority) {
}


ROMAEdge::~ROMAEdge() {
}


void
ROMAEdge::addSuccessor(ROEdge* s, ROEdge* via, std::string dir) {
    ROEdge::addSuccessor(s, via, dir);
    if (dir == "l" || dir == "L") {
        myLeftTurns.insert(static_cast<ROMAEdge*>(s));
    }
}


/****************************************************************************/
