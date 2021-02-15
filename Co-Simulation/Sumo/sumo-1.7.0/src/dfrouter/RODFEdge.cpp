/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    RODFEdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Yun-Pang Floetteroed
/// @date    Thu, 16.03.2006
///
// An edge within the DFROUTER
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <utils/common/MsgHandler.h>
#include "RODFEdge.h"


// ===========================================================================
// method definitions
// ===========================================================================
RODFEdge::RODFEdge(const std::string& id, RONode* from, RONode* to, int index, const int priority)
    : ROEdge(id, from, to, index, priority) {}


RODFEdge::~RODFEdge() {}


void
RODFEdge::setFlows(const std::vector<FlowDef>& flows) {
    myFlows = flows;
}


const std::vector<FlowDef>&
RODFEdge::getFlows() const {
    return myFlows;
}


/****************************************************************************/
