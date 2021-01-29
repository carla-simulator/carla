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
/// @file    GNEDetectorEntryExit.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEDetectorEntryExit.h"
#include "GNEAdditionalHandler.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEDetectorEntryExit::GNEDetectorEntryExit(SumoXMLTag entryExitTag, GNENet* net, GNEAdditional* parent, GNELane* lane, double pos, bool friendlyPos, bool blockMovement) :
    GNEDetector(parent, net, GLO_DET_ENTRY, entryExitTag, pos, "", "", "", friendlyPos, blockMovement, {
    lane
}) {
    //check that this is a TAZ Source OR a TAZ Sink
    if ((entryExitTag != SUMO_TAG_DET_ENTRY) && (entryExitTag != SUMO_TAG_DET_EXIT)) {
        throw InvalidArgument("Invalid E3 Child Tag");
    }
}


GNEDetectorEntryExit::~GNEDetectorEntryExit() {}


bool
GNEDetectorEntryExit::isAdditionalValid() const {
    // with friendly position enabled position are "always fixed"
    if (myFriendlyPosition) {
        return true;
    } else {
        return fabs(myPositionOverLane) <= getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
    }
}


std::string
GNEDetectorEntryExit::getAdditionalProblem() const {
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
GNEDetectorEntryExit::fixAdditionalProblem() {
    // declare new position
    double newPositionOverLane = myPositionOverLane;
    // fix pos and length  checkAndFixDetectorPosition
    GNEAdditionalHandler::checkAndFixDetectorPosition(newPositionOverLane, getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength(), true);
    // set new position
    setAttribute(SUMO_ATTR_POSITION, toString(newPositionOverLane), myNet->getViewNet()->getUndoList());
}


void
GNEDetectorEntryExit::moveGeometry(const Position& offset) {
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
GNEDetectorEntryExit::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPositionOverLane), myMove.firstOriginalLanePosition));
    undoList->p_end();
}


void
GNEDetectorEntryExit::updateGeometry() {
    // update geometry
    myAdditionalGeometry.updateGeometry(getParentLanes().front(), getGeometryPositionOverLane());

    // update block icon position
    myBlockIcon.updatePositionAndRotation();

    // Set offset of the block icon
    myBlockIcon.setOffset(1, 0);

    // update E3 parent children
    getParentAdditionals().at(0)->updateHierarchicalConnections();
}


void
GNEDetectorEntryExit::drawGL(const GUIVisualizationSettings& s) const {
    // Set initial values
    const double entryExitExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(entryExitExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // Start drawing adding gl identificator
        glPushName(getGlID());
        // Push detector matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_DET_ENTRY);
        // Set color
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else if (myTagProperty.getTag() == SUMO_TAG_DET_ENTRY) {
            GLHelper::setColor(s.detectorSettings.E3EntryColor);
        } else if (myTagProperty.getTag() == SUMO_TAG_DET_EXIT) {
            GLHelper::setColor(s.detectorSettings.E3ExitColor);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // Push polygon matrix
        glPushMatrix();
        glTranslated(myAdditionalGeometry.getPosition().x(), myAdditionalGeometry.getPosition().y(), 0);
        glRotated((myBlockIcon.getRotation() * -1), 0, 0, 1);
        glScaled(entryExitExaggeration, entryExitExaggeration, 1);
        // draw details if isn't being drawn for selecting
        if (!s.drawForRectangleSelection) {
            // Draw polygon
            glBegin(GL_LINES);
            glVertex2d(1.7, 0);
            glVertex2d(-1.7, 0);
            glEnd();
            glBegin(GL_QUADS);
            glVertex2d(-1.7, .5);
            glVertex2d(-1.7, -.5);
            glVertex2d(1.7, -.5);
            glVertex2d(1.7, .5);
            glEnd();
            // first Arrow
            glTranslated(1.5, 0, 0);
            GLHelper::drawBoxLine(Position(0, 4), 0, 2, .05);
            GLHelper::drawTriangleAtEnd(Position(0, 4), Position(0, 1), (double) 1, (double) .25);
            // second Arrow
            glTranslated(-3, 0, 0);
            GLHelper::drawBoxLine(Position(0, 4), 0, 2, .05);
            GLHelper::drawTriangleAtEnd(Position(0, 4), Position(0, 1), (double) 1, (double) .25);
        } else {
            // Draw square in drawy for selecting mode
            glBegin(GL_QUADS);
            glVertex2d(-1.7, 4.3);
            glVertex2d(-1.7, -.5);
            glVertex2d(1.7, -.5);
            glVertex2d(1.7, 4.3);
            glEnd();
        }
        // Pop polygon matrix
        glPopMatrix();
        // Pop detector matrix
        glPopMatrix();
        // Check if the distance is enought to draw details
        if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.detectorDetails, entryExitExaggeration)) {
            // Push matrix
            glPushMatrix();
            // Traslate to center of detector
            glTranslated(myAdditionalGeometry.getShape().getLineCenter().x(), myAdditionalGeometry.getShape().getLineCenter().y(), getType() + 0.1);
            // Rotate depending of myBlockIcon.rotation
            glRotated(myBlockIcon.getRotation(), 0, 0, -1);
            //move to logo position
            glTranslated(1.9, 0, 0);
            // scale
            glScaled(entryExitExaggeration, entryExitExaggeration, 1);
            // draw Entry or Exit logo if isn't being drawn for selecting
            if (s.drawForRectangleSelection || s.drawForPositionSelection) {
                GLHelper::setColor(s.detectorSettings.E3EntryColor);
                GLHelper::drawBoxLine(Position(0, 1), 0, 2, 1);
            } else if (drawUsingSelectColor()) {
                GLHelper::drawText("E3", Position(), .1, 2.8, s.colorSettings.selectedAdditionalColor);
            } else if (myTagProperty.getTag() == SUMO_TAG_DET_ENTRY) {
                GLHelper::drawText("E3", Position(), .1, 2.8, s.detectorSettings.E3EntryColor);
            } else if (myTagProperty.getTag() == SUMO_TAG_DET_EXIT) {
                GLHelper::drawText("E3", Position(), .1, 2.8, s.detectorSettings.E3ExitColor);
            }
            //move to logo position
            glTranslated(1.7, 0, 0);
            // Rotate depending of myBlockIcon.rotation
            glRotated(90, 0, 0, 1);
            // draw Entry or Exit text if isn't being drawn for selecting
            if (s.drawForRectangleSelection || s.drawForPositionSelection) {
                GLHelper::setColor(s.detectorSettings.E3EntryColor);
                GLHelper::drawBoxLine(Position(0, 1), 0, 2, 1);
            } else if (drawUsingSelectColor()) {
                if (myTagProperty.getTag() == SUMO_TAG_DET_ENTRY) {
                    GLHelper::drawText("Entry", Position(), .1, 1, s.colorSettings.selectedAdditionalColor);
                } else if (myTagProperty.getTag() == SUMO_TAG_DET_EXIT) {
                    GLHelper::drawText("Exit", Position(), .1, 1, s.colorSettings.selectedAdditionalColor);
                }
            } else {
                if (myTagProperty.getTag() == SUMO_TAG_DET_ENTRY) {
                    GLHelper::drawText("Entry", Position(), .1, 1, s.detectorSettings.E3EntryColor);
                } else if (myTagProperty.getTag() == SUMO_TAG_DET_EXIT) {
                    GLHelper::drawText("Exit", Position(), .1, 1, s.detectorSettings.E3ExitColor);
                }
            }
            // pop matrix
            glPopMatrix();
            // draw lock icon
            myBlockIcon.drawIcon(s, entryExitExaggeration, 0.4);
        }
        // Draw name if isn't being drawn for selecting
        if (!s.drawForRectangleSelection) {
            drawName(getPositionInView(), s.scale, s.addName);
        }
        // check if dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // GNEGeometry::drawShapeDottedContour(s, getType(), entryExitExaggeration, myDottedGeometry);
        }
        // pop gl identificator
        glPopName();
    }
}


std::string
GNEDetectorEntryExit::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getParentAdditionals().front()->getID();
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        case SUMO_ATTR_POSITION:
            return toString(myPositionOverLane);
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


void
GNEDetectorEntryExit::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_FRIENDLY_POS:
        case GNE_ATTR_BLOCK_MOVEMENT:
        case GNE_ATTR_PARENT:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEDetectorEntryExit::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_LANE:
            return (myNet->retrieveLane(value, false) != nullptr);
        case SUMO_ATTR_POSITION:
            return canParse<double>(value) && fabs(parse<double>(value)) < getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength();
        case SUMO_ATTR_FRIENDLY_POS:
            return canParse<bool>(value);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return canParse<bool>(value);
        case GNE_ATTR_PARENT:
            return (myNet->retrieveAdditional(SUMO_TAG_E3DETECTOR, value, false) != nullptr);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEDetectorEntryExit::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


void
GNEDetectorEntryExit::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_LANE:
            replaceAdditionalParentLanes(value);
            break;
        case SUMO_ATTR_POSITION:
            myPositionOverLane = parse<double>(value);
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            myFriendlyPosition = parse<bool>(value);
            break;
        case GNE_ATTR_BLOCK_MOVEMENT:
            myBlockMovement = parse<bool>(value);
            break;
        case GNE_ATTR_PARENT:
            replaceAdditionalParent(SUMO_TAG_E3DETECTOR, value, 0);
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
