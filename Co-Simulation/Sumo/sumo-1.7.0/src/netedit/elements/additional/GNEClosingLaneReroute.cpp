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
/// @file    GNEClosingLaneReroute.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2017
///
//
/****************************************************************************/
#include <config.h>

#include "GNEClosingLaneReroute.h"
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/dialogs/GNERerouterIntervalDialog.h>

#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>


// ===========================================================================
// member method definitions
// ===========================================================================

GNEClosingLaneReroute::GNEClosingLaneReroute(GNEAdditional* rerouterIntervalParent, GNELane* closedLane, SVCPermissions permissions) :
    GNEAdditional(rerouterIntervalParent->getNet(), GLO_REROUTER, SUMO_TAG_CLOSING_LANE_REROUTE, "", false,
{}, {}, {}, {rerouterIntervalParent}, {}, {}, {}, {}),
myClosedLane(closedLane),
myPermissions(permissions) {
}


GNEClosingLaneReroute::~GNEClosingLaneReroute() {}


void
GNEClosingLaneReroute::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNEClosingLaneReroute::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


void
GNEClosingLaneReroute::updateGeometry() {
    // This additional doesn't own a geometry
}


Position
GNEClosingLaneReroute::getPositionInView() const {
    return getParentAdditionals().at(0)->getPositionInView();
}


Boundary
GNEClosingLaneReroute::getCenteringBoundary() const {
    return myClosedLane->getCenteringBoundary();
}


void
GNEClosingLaneReroute::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


std::string
GNEClosingLaneReroute::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNEClosingLaneReroute::drawGL(const GUIVisualizationSettings& /* s */) const {
    // Currently this additional isn't drawn
}


std::string
GNEClosingLaneReroute::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_LANE:
            return myClosedLane->getID();
        case SUMO_ATTR_ALLOW:
            return getVehicleClassNames(myPermissions);
        case SUMO_ATTR_DISALLOW:
            return getVehicleClassNames(invertPermissions(myPermissions));
        case GNE_ATTR_PARENT:
            return getParentAdditionals().at(0)->getID();
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEClosingLaneReroute::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEClosingLaneReroute::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_ALLOW:
        case SUMO_ATTR_DISALLOW:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEClosingLaneReroute::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_LANE:
            return (myNet->retrieveLane(value, false) != nullptr);
        case SUMO_ATTR_ALLOW:
        case SUMO_ATTR_DISALLOW:
            return canParseVehicleClasses(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEClosingLaneReroute::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEClosingLaneReroute::getPopUpID() const {
    return getTagStr();
}


std::string
GNEClosingLaneReroute::getHierarchyName() const {
    return getTagStr() + ": " + myClosedLane->getID();
}

// ===========================================================================
// private
// ===========================================================================

void
GNEClosingLaneReroute::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_LANE:
            myClosedLane = myNet->retrieveLane(value);
            break;
        case SUMO_ATTR_ALLOW:
            myPermissions = parseVehicleClasses(value);
            break;
        case SUMO_ATTR_DISALLOW:
            myPermissions = invertPermissions(parseVehicleClasses(value));
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
