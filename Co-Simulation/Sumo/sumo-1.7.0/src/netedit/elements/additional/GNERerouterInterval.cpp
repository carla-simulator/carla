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
/// @file    GNERerouterInterval.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2017
///
//
/****************************************************************************/
#include <config.h>

#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/dialogs/GNERerouterDialog.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>

#include "GNERerouterInterval.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNERerouterInterval::GNERerouterInterval(GNERerouterDialog* rerouterDialog) :
    GNEAdditional(rerouterDialog->getEditedAdditional()->getNet(), GLO_REROUTER, SUMO_TAG_INTERVAL, "", false,
{}, {}, {}, {rerouterDialog->getEditedAdditional()}, {}, {}, {}, {}),
myBegin(0),
myEnd(0) {
    // fill reroute interval with default values
    setDefaultValues();
}


GNERerouterInterval::GNERerouterInterval(GNEAdditional* rerouterParent, SUMOTime begin, SUMOTime end) :
    GNEAdditional(rerouterParent->getNet(), GLO_REROUTER, SUMO_TAG_INTERVAL, "", false,
{}, {}, {}, {rerouterParent}, {}, {}, {}, {}),
myBegin(begin),
myEnd(end) {
}


GNERerouterInterval::~GNERerouterInterval() {}

void
GNERerouterInterval::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNERerouterInterval::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


void
GNERerouterInterval::updateGeometry() {
    // This additional doesn't own a geometry
}


Position
GNERerouterInterval::getPositionInView() const {
    return getParentAdditionals().at(0)->getPositionInView();
}


Boundary
GNERerouterInterval::getCenteringBoundary() const {
    return getParentAdditionals().at(0)->getCenteringBoundary();
}


void
GNERerouterInterval::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


std::string
GNERerouterInterval::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNERerouterInterval::drawGL(const GUIVisualizationSettings&) const {
    // Currently This additional isn't drawn
}


std::string
GNERerouterInterval::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getParentAdditionals().front()->getID();
        case SUMO_ATTR_BEGIN:
            return time2string(myBegin);
        case SUMO_ATTR_END:
            return time2string(myEnd);
        case GNE_ATTR_PARENT:
            return getParentAdditionals().at(0)->getID();
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNERerouterInterval::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_BEGIN:
            return STEPS2TIME(myBegin);
        case SUMO_ATTR_END:
            return STEPS2TIME(myEnd);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNERerouterInterval::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_BEGIN:
        case SUMO_ATTR_END:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERerouterInterval::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_BEGIN:
            return canParse<SUMOTime>(value) && (parse<SUMOTime>(value) < myEnd);
        case SUMO_ATTR_END:
            return canParse<SUMOTime>(value) && (parse<SUMOTime>(value) > myBegin);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERerouterInterval::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNERerouterInterval::getPopUpID() const {
    return getTagStr();
}


std::string
GNERerouterInterval::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_BEGIN) + " -> " + getAttribute(SUMO_ATTR_END);
}

// ===========================================================================
// private
// ===========================================================================

void
GNERerouterInterval::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_BEGIN:
            myBegin = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_END:
            myEnd = parse<SUMOTime>(value);
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
