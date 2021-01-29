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
/// @file    GNEVariableSpeedSignStep.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Apr 2017
///
//
/****************************************************************************/
#include <config.h>

#include <netedit/dialogs/GNEVariableSpeedSignDialog.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>

#include "GNEVariableSpeedSignStep.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEVariableSpeedSignStep::GNEVariableSpeedSignStep(GNEAdditional* variableSpeedSignParent, double time, double speed) :
    GNEAdditional(variableSpeedSignParent->getNet(), GLO_VSS, SUMO_TAG_STEP, "", false,
{}, {}, {}, {variableSpeedSignParent}, {}, {}, {}, {}),
myTime(time),
mySpeed(speed) {
}


GNEVariableSpeedSignStep::~GNEVariableSpeedSignStep() {}


double
GNEVariableSpeedSignStep::getTime() const {
    return myTime;
}


void
GNEVariableSpeedSignStep::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNEVariableSpeedSignStep::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


void
GNEVariableSpeedSignStep::updateGeometry() {
    // This additional doesn't own a geometry
}


Position
GNEVariableSpeedSignStep::getPositionInView() const {
    return getParentAdditionals().at(0)->getPositionInView();
}


Boundary
GNEVariableSpeedSignStep::getCenteringBoundary() const {
    return getParentAdditionals().at(0)->getCenteringBoundary();
}


void
GNEVariableSpeedSignStep::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


std::string
GNEVariableSpeedSignStep::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNEVariableSpeedSignStep::drawGL(const GUIVisualizationSettings&) const {
    // Currently This additional isn't drawn
}


std::string
GNEVariableSpeedSignStep::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_TIME:
            return toString(myTime);
        case SUMO_ATTR_SPEED:
            return toString(mySpeed);
        case GNE_ATTR_PARENT:
            return getParentAdditionals().at(0)->getID();
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEVariableSpeedSignStep::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_TIME:
            return myTime;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNEVariableSpeedSignStep::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_TIME:
        case SUMO_ATTR_SPEED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEVariableSpeedSignStep::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_TIME:
            if (canParse<double>(value)) {
                // Check that
                double newTime = parse<double>(value);
                // Only allowed positiv times
                if (newTime < 0) {
                    return false;
                }
                // check that there isn't duplicate times
                int counter = 0;
                for (const auto& VSSChild : getParentAdditionals().at(0)->getChildAdditionals()) {
                    if (!VSSChild->getTagProperty().isSymbol() && VSSChild->getAttributeDouble(SUMO_ATTR_TIME) == newTime) {
                        counter++;
                    }
                }
                return (counter <= 1);
            } else {
                return false;
            }
        case SUMO_ATTR_SPEED:
            return canParse<double>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEVariableSpeedSignStep::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEVariableSpeedSignStep::getPopUpID() const {
    return getTagStr();
}


std::string
GNEVariableSpeedSignStep::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_TIME);
}

// ===========================================================================
// private
// ===========================================================================

void
GNEVariableSpeedSignStep::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_TIME:
            myTime = parse<double>(value);
            break;
        case SUMO_ATTR_SPEED:
            mySpeed = parse<double>(value);
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
