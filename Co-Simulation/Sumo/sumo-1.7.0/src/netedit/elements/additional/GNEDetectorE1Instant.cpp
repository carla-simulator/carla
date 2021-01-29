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
/// @file    GNEDetectorE1Instant.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2018
///
//
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEDetectorE1Instant.h"
#include "GNEAdditionalHandler.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEDetectorE1Instant::GNEDetectorE1Instant(const std::string& id, GNELane* lane, GNENet* net, double pos, const std::string& filename, const std::string& vehicleTypes, const std::string& name, bool friendlyPos, bool blockMovement) :
    GNEDetector(id, net, GLO_E1DETECTOR_INSTANT, SUMO_TAG_INSTANT_INDUCTION_LOOP, pos, "", filename, vehicleTypes, name, friendlyPos, blockMovement, {
    lane
}) {
}


GNEDetectorE1Instant::~GNEDetectorE1Instant() {
}


bool
GNEDetectorE1Instant::isAdditionalValid() const {
    // with friendly position enabled position are "always fixed"
    if (myFriendlyPosition) {
        return true;
    } else {
        return fabs(myPositionOverLane) <= getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
    }
}


std::string
GNEDetectorE1Instant::getAdditionalProblem() const {
    // declare variable for error position
    std::string errorPosition;
    const double len = getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
    // check positions over lane
    if (myPositionOverLane < -len) {
        errorPosition = (toString(SUMO_ATTR_POSITION) + " < 0");
    }
    if (myPositionOverLane > len) {
        errorPosition = (toString(SUMO_ATTR_POSITION) + " > lanes's length");
    }
    return errorPosition;
}


void
GNEDetectorE1Instant::fixAdditionalProblem() {
    // declare new position
    double newPositionOverLane = myPositionOverLane;
    // fix pos and length  checkAndFixDetectorPosition
    GNEAdditionalHandler::checkAndFixDetectorPosition(newPositionOverLane, getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength(), true);
    // set new position
    setAttribute(SUMO_ATTR_POSITION, toString(newPositionOverLane), myNet->getViewNet()->getUndoList());
}


void
GNEDetectorE1Instant::moveGeometry(const Position& offset) {
    // Calculate new position using old position
    Position newPosition = myMove.originalViewPosition;
    newPosition.add(offset);
    // filtern position using snap to active grid
    newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
    const bool storeNegative = myPositionOverLane < 0;
    myPositionOverLane = getParentLanes().front()->getLaneShape().nearest_offset_to_point2D(newPosition, false);
    if (storeNegative) {
        myPositionOverLane -= getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
    }
    // Update geometry
    updateGeometry();
}


void
GNEDetectorE1Instant::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPositionOverLane), myMove.firstOriginalLanePosition));
    undoList->p_end();
}


void
GNEDetectorE1Instant::updateGeometry() {
    // update geometry
    myAdditionalGeometry.updateGeometry(getParentLanes().front(), getGeometryPositionOverLane());

    // update block icon position
    myBlockIcon.updatePositionAndRotation();

    // Set offset of the block icon
    myBlockIcon.setOffset(1, 0);
}


void
GNEDetectorE1Instant::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double E1InstantExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(E1InstantExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // obtain scaledSize
        const double scaledWidth = s.detectorSettings.E1InstantWidth * 0.5 * s.scale;
        // declare colors
        RGBColor mainColor, secondColor, textColor;
        // set color
        if (drawUsingSelectColor()) {
            mainColor = s.colorSettings.selectedAdditionalColor;
            secondColor = mainColor.changedBrightness(-32);
            textColor = mainColor.changedBrightness(32);
        } else {
            mainColor = s.detectorSettings.E1InstantColor;
            secondColor = RGBColor::WHITE;
            textColor = RGBColor::BLACK;
        }
        // start drawing
        glPushName(getGlID());
        // push layer matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_E1DETECTOR_INSTANT);
        // draw E1Instant shape
        drawE1Shape(s, E1InstantExaggeration, scaledWidth, mainColor, secondColor);
        // Check if the distance is enought to draw details
        if (s.drawDetail(s.detailSettings.detectorDetails, E1InstantExaggeration)) {
            // draw E1 Logo
            drawDetectorLogo(s, E1InstantExaggeration, "E1", textColor);
            // Show Lock icon depending of the Edit mode
            myBlockIcon.drawIcon(s, E1InstantExaggeration);
        }
        // pop layer matrix
        glPopMatrix();
        // Draw name if isn't being drawn for selecting
        drawName(getPositionInView(), s.scale, s.addName);
        // Pop name
        glPopName();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedSquaredShape(true, s, myAdditionalGeometry.getPosition(), 2, 1, 0, 0, myAdditionalGeometry.getRotation(), E1InstantExaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedSquaredShape(false, s, myAdditionalGeometry.getPosition(), 2, 1, 0, 0, myAdditionalGeometry.getRotation(), E1InstantExaggeration);
        }
    }
}


std::string
GNEDetectorE1Instant::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        case SUMO_ATTR_POSITION:
            return toString(myPositionOverLane);
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_FILE:
            return myFilename;
        case SUMO_ATTR_VTYPES:
            return myVehicleTypes;
        case SUMO_ATTR_FRIENDLY_POS:
            return toString(myFriendlyPosition);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return toString(myBlockMovement);
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEDetectorE1Instant::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_FILE:
        case SUMO_ATTR_VTYPES:
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
GNEDetectorE1Instant::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidDetectorID(value);
        case SUMO_ATTR_LANE:
            if (myNet->retrieveLane(value, false) != nullptr) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_POSITION:
            return canParse<double>(value) && fabs(parse<double>(value)) < getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_FILE:
            return SUMOXMLDefinitions::isValidFilename(value);
        case SUMO_ATTR_VTYPES:
            if (value.empty()) {
                return true;
            } else {
                return SUMOXMLDefinitions::isValidListOfTypeID(value);
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
GNEDetectorE1Instant::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}

// ===========================================================================
// private
// ===========================================================================

void
GNEDetectorE1Instant::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_LANE:
            replaceAdditionalParentLanes(value);
            break;
        case SUMO_ATTR_POSITION:
            myPositionOverLane = parse<double>(value);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_FILE:
            myFilename = value;
            break;
        case SUMO_ATTR_VTYPES:
            myVehicleTypes = value;
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
