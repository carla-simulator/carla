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
/// @file    GNERouteProbReroute.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2017
///
//
/****************************************************************************/
#include <config.h>

#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/dialogs/GNERerouterIntervalDialog.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>

#include "GNERouteProbReroute.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNERouteProbReroute::GNERouteProbReroute(GNEAdditional* rerouterIntervalParent, const std::string& newRouteId, double probability) :
    GNEAdditional(rerouterIntervalParent->getNet(), GLO_REROUTER, SUMO_TAG_ROUTE_PROB_REROUTE, "", false,
{}, {}, {}, {rerouterIntervalParent}, {}, {}, {}, {}),
myNewRouteId(newRouteId),
myProbability(probability) {
}


GNERouteProbReroute::~GNERouteProbReroute() {}


void
GNERouteProbReroute::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNERouteProbReroute::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


void
GNERouteProbReroute::updateGeometry() {
    // This additional doesn't own a geometry
}


Position
GNERouteProbReroute::getPositionInView() const {
    return getParentAdditionals().at(0)->getPositionInView();
}


Boundary
GNERouteProbReroute::getCenteringBoundary() const {
    return getParentAdditionals().at(0)->getCenteringBoundary();
}


void
GNERouteProbReroute::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


std::string
GNERouteProbReroute::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNERouteProbReroute::drawGL(const GUIVisualizationSettings&) const {
    // Currently This additional isn't drawn
}


std::string
GNERouteProbReroute::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_ROUTE:
            return myNewRouteId;
        case SUMO_ATTR_PROB:
            return toString(myProbability);
        case GNE_ATTR_PARENT:
            return getParentAdditionals().at(0)->getID();
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNERouteProbReroute::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNERouteProbReroute::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_ROUTE:
        case SUMO_ATTR_PROB:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERouteProbReroute::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_ROUTE:
            return SUMOXMLDefinitions::isValidVehicleID(value);
        case SUMO_ATTR_PROB:
            return canParse<double>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERouteProbReroute::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNERouteProbReroute::getPopUpID() const {
    return getTagStr();
}


std::string
GNERouteProbReroute::getHierarchyName() const {
    return getTagStr() + ": " + myNewRouteId;
}

// ===========================================================================
// private
// ===========================================================================

void
GNERouteProbReroute::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_ROUTE:
            myNewRouteId = value;
            break;
        case SUMO_ATTR_PROB:
            myProbability = parse<double>(value);
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
