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
/// @file    GUIEdgeControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Derivation of NLEdgeControlBuilder which build gui-edges
/****************************************************************************/
#include <config.h>

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <guisim/GUIEdge.h>
#include <guisim/GUINet.h>
#include <guisim/GUILane.h>
#include <microsim/MSJunction.h>
#include <netload/NLBuilder.h>
#include "GUIEdgeControlBuilder.h"
#include <gui/GUIGlobals.h>


// ===========================================================================
// method definitions
// ===========================================================================
GUIEdgeControlBuilder::GUIEdgeControlBuilder()
    : NLEdgeControlBuilder() {}


GUIEdgeControlBuilder::~GUIEdgeControlBuilder() {}


MSLane*
GUIEdgeControlBuilder::addLane(const std::string& id,
                               double maxSpeed, double length,
                               const PositionVector& shape,
                               double width,
                               SVCPermissions permissions,
                               int index, bool isRampAccel,
                               const std::string& type) {
    MSLane* lane = new GUILane(id, maxSpeed, length, myActiveEdge, myCurrentNumericalLaneID++, shape, width, permissions, index, isRampAccel, type);
    myLaneStorage->push_back(lane);
    myCurrentLaneIndex = index;
    return lane;
}



MSEdge*
GUIEdgeControlBuilder::buildEdge(const std::string& id, const SumoXMLEdgeFunc function,
                                 const std::string& streetName, const std::string& edgeType, const int priority, const double distance) {
    return new GUIEdge(id, myCurrentNumericalEdgeID++, function, streetName, edgeType, priority, distance);
}


/****************************************************************************/
