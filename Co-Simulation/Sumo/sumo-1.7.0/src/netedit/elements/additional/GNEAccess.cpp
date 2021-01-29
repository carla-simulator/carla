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
/// @file    GNEAccess.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2018
///
//
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEAccess.h"
#include "GNEAdditionalHandler.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNEAccess::GNEAccess(GNEAdditional* busStop, GNELane* lane, GNENet* net, double pos, const std::string& length, bool friendlyPos, bool blockMovement) :
    GNEAdditional(net, GLO_ACCESS, SUMO_TAG_ACCESS, "", blockMovement,
{}, {}, {lane}, {busStop}, {}, {}, {}, {}),
myPositionOverLane(pos),
myLength(length),
myFriendlyPosition(friendlyPos) {
}


GNEAccess::~GNEAccess() {
}


void
GNEAccess::moveGeometry(const Position& offset) {
    // Calculate new position using old position
    Position newPosition = myMove.originalViewPosition;
    newPosition.add(offset);
    // filtern position using snap to active grid
    newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
    myPositionOverLane = getParentLanes().front()->getLaneShape().nearest_offset_to_point2D(newPosition, false);
    // Update geometry
    updateGeometry();
}


void
GNEAccess::commitGeometryMoving(GNEUndoList* undoList) {
    if (!myBlockMovement) {
        // commit new position allowing undo/redo
        undoList->p_begin("position of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPositionOverLane), myMove.firstOriginalLanePosition));
        undoList->p_end();
    }
}


void
GNEAccess::updateGeometry() {
    // set start position
    double fixedPositionOverLane;
    if (myPositionOverLane == -1) {
        fixedPositionOverLane = getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
    } else if (myPositionOverLane < 0) {
        fixedPositionOverLane = 0;
    } else if (myPositionOverLane > getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength()) {
        fixedPositionOverLane = getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
    } else {
        fixedPositionOverLane = myPositionOverLane;
    }
    // update geometry
    myAdditionalGeometry.updateGeometry(getParentLanes().front(), fixedPositionOverLane * getParentLanes().front()->getLengthGeometryFactor());
    // update block icon position
    myBlockIcon.updatePositionAndRotation();
}


Position
GNEAccess::getPositionInView() const {
    if (myPositionOverLane == -1) {
        return getParentLanes().front()->getLaneShape().front();
    } else {
        if (myPositionOverLane < 0) {
            return getParentLanes().front()->getLaneShape().front();
        } else if (myPositionOverLane > getParentLanes().front()->getLaneShape().length()) {
            return getParentLanes().front()->getLaneShape().back();
        } else {
            return getParentLanes().front()->getLaneShape().positionAtOffset(myPositionOverLane);
        }
    }
}


Boundary
GNEAccess::getCenteringBoundary() const {
    return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
}


void
GNEAccess::splitEdgeGeometry(const double splitPosition, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* newElement, GNEUndoList* undoList) {
    if (splitPosition < myPositionOverLane) {
        // change lane
        setAttribute(SUMO_ATTR_LANE, newElement->getID(), undoList);
        // now adjust start position
        setAttribute(SUMO_ATTR_POSITION, toString(myPositionOverLane - splitPosition), undoList);
    }
}


bool
GNEAccess::isAccessPositionFixed() const {
    // with friendly position enabled position are "always fixed"
    if (myFriendlyPosition) {
        return true;
    } else {
        if (myPositionOverLane != -1) {
            return (myPositionOverLane >= 0) && (myPositionOverLane <= getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength());
        } else {
            return false;
        }
    }
}


GNEEdge*
GNEAccess::getEdge() const {
    return getParentLanes().front()->getParentEdge();
}


std::string
GNEAccess::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNEAccess::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration
    const double accessExaggeration = s.addSize.getExaggeration(s, this);
    // declare width
    const double radius = 0.5;
    // first check if additional has to be drawn
    if (s.drawAdditionals(accessExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // push matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_ACCESS);
        // set color depending of selection
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else {
            GLHelper::setColor(s.stoppingPlaceSettings.busStopColor);
        }
        // translate to geometry position
        glTranslated(myAdditionalGeometry.getPosition().x(), myAdditionalGeometry.getPosition().y(), 0);
        // draw circle
        if (s.drawForRectangleSelection) {
            GLHelper::drawFilledCircle(radius * accessExaggeration, 8);
        } else {
            GLHelper::drawFilledCircle(radius * accessExaggeration, 16);
        }
        // pop matrix
        glPopMatrix();
        // pop gl identficador
        glPopName();
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourCircle(true, s, myAdditionalGeometry.getPosition(), 0.5, accessExaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourCircle(false, s, myAdditionalGeometry.getPosition(), 0.5, accessExaggeration);
        }
    }
}


std::string
GNEAccess::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getParentAdditionals().front()->getID();
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        case SUMO_ATTR_POSITION:
            return toString(myPositionOverLane);
        case SUMO_ATTR_LENGTH:
            return toString(myLength);
        case SUMO_ATTR_FRIENDLY_POS:
            return toString(myFriendlyPosition);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return toString(myBlockMovement);
        case GNE_ATTR_PARENT:
            return getParentAdditionals().at(0)->getID();
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEAccess::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEAccess::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_LENGTH:
        case SUMO_ATTR_FRIENDLY_POS:
        case GNE_ATTR_BLOCK_MOVEMENT:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEAccess::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_LANE: {
            GNELane* lane = myNet->retrieveLane(value, false);
            if (lane != nullptr) {
                if (getParentLanes().front()->getParentEdge()->getID() != lane->getParentEdge()->getID()) {
                    return GNEAdditionalHandler::accessCanBeCreated(getParentAdditionals().at(0), lane->getParentEdge());
                } else {
                    return true;
                }
            } else {
                return false;
            }
        }
        case SUMO_ATTR_POSITION:
            if (value.empty()) {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_LENGTH:
            if (value.empty()) {
                return true;
            } else {
                return (canParse<double>(value) && (parse<double>(value) >= 0));
            }
        case SUMO_ATTR_FRIENDLY_POS:
            return canParse<bool>(value);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return canParse<bool>(value);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEAccess::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEAccess::getPopUpID() const {
    return getTagStr();
}


std::string
GNEAccess::getHierarchyName() const {
    return getTagStr() + ": " + getParentLanes().front()->getParentEdge()->getID();
}

// ===========================================================================
// private
// ===========================================================================

void
GNEAccess::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_LANE:
            replaceAdditionalParentLanes(value);
            break;
        case SUMO_ATTR_POSITION:
            myPositionOverLane = parse<double>(value);
            break;
        case SUMO_ATTR_LENGTH:
            myLength = value;
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            myFriendlyPosition = parse<bool>(value);
            break;
        case GNE_ATTR_BLOCK_MOVEMENT:
            myBlockMovement = parse<bool>(value);
            break;
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


/****************************************************************************/
