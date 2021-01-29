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
/// @file    GNEParkingSpace.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// A lane area vehicles can halt at (GNE version)
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEParkingSpace.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEParkingSpace::GNEParkingSpace(GNENet* net, GNEAdditional* parkingAreaParent, const Position& pos, double width, double length, double angle, bool blockMovement) :
    GNEAdditional(net, GLO_PARKING_SPACE, SUMO_TAG_PARKING_SPACE, "", blockMovement,
{}, {}, {}, {parkingAreaParent}, {}, {}, {}, {}),
myPosition(pos),
myWidth(width),
myLength(length),
myAngle(angle) {
}


GNEParkingSpace::~GNEParkingSpace() {}


void
GNEParkingSpace::moveGeometry(const Position& offset) {
    // restore old position, apply offset and update Geometry
    myPosition = myMove.originalViewPosition;
    myPosition.add(offset);
    // filtern position using snap to active grid
    myPosition = myNet->getViewNet()->snapToActiveGrid(myPosition);
    updateGeometry();
}


void
GNEParkingSpace::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPosition), toString(myMove.originalViewPosition)));
    undoList->p_end();
}


void
GNEParkingSpace::updateGeometry() {
    // Nothing to update
}


Position
GNEParkingSpace::getPositionInView() const {
    return myPosition;
}


Boundary
GNEParkingSpace::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMove.movingGeometryBoundary.isInitialised()) {
        return myMove.movingGeometryBoundary;
    } else {
        // calculate shape using a Position vector as reference
        PositionVector boundaryShape({
            {-(myWidth / 2), 0},
            { (myWidth / 2), 0},
            { (myWidth / 2), myLength},
            {-(myWidth / 2), myLength},
        });
        // rotate position vector (note: convert from degree to rads
        boundaryShape.rotate2D(myAngle * PI / 180.0);
        // move to space position
        boundaryShape.add(myPosition);
        // return boundary associated to boundaryShape
        return boundaryShape.getBoxBoundary().grow(5);
    }
}


void
GNEParkingSpace::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


std::string
GNEParkingSpace::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNEParkingSpace::drawGL(const GUIVisualizationSettings& s) const {
    // Set initial values
    const double parkingAreaExaggeration = s.addSize.getExaggeration(s, this);
    // obtain values with exaggeration
    const double widthExaggeration = myWidth * parkingAreaExaggeration;
    const double lengthExaggeration = myLength * parkingAreaExaggeration;
    // first check if additional has to be drawn
    if (s.drawAdditionals(parkingAreaExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // check if boundary has to be drawn
        if (s.drawBoundaries) {
            GLHelper::drawBoundary(getCenteringBoundary());
        }
        // push name
        glPushName(getGlID());
        // push later matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_PARKING_SPACE);
        // translate to position
        glTranslated(myPosition.x(), myPosition.y(), 0);
        // rotate
        glRotated(myAngle, 0, 0, 1);
        // only drawn small box if isn't being drawn for selecting
        if (!s.drawForRectangleSelection) {
            // Set Color depending of selection
            if (drawUsingSelectColor()) {
                GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
            } else {
                GLHelper::setColor(s.stoppingPlaceSettings.parkingSpaceColorContour);
            }
            GLHelper::drawBoxLine(Position(0, lengthExaggeration + 0.05), 0, lengthExaggeration + 0.1, (widthExaggeration * 0.5) + 0.05);
        }
        // Traslate matrix and draw blue innen
        glTranslated(0, 0, 0.1);
        // Set Color depending of selection
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else {
            GLHelper::setColor(s.stoppingPlaceSettings.parkingSpaceColor);
        }
        GLHelper::drawBoxLine(Position(0, lengthExaggeration), 0, lengthExaggeration, widthExaggeration * 0.5);
        // Traslate matrix and draw lock icon if isn't being drawn for selecting
        glTranslated(0, lengthExaggeration * 0.5, 0.1);
        // draw lock icon
        myBlockIcon.drawIcon(s, parkingAreaExaggeration);
        // pop layer matrix
        glPopMatrix();
        // pop name
        glPopName();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            // draw using drawDottedContourClosedShape
            GNEGeometry::drawDottedSquaredShape(true, s, myPosition, lengthExaggeration * 0.5, widthExaggeration * 0.5, lengthExaggeration * 0.5, 0, myAngle, 1);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            // draw using drawDottedContourClosedShape
            GNEGeometry::drawDottedSquaredShape(false, s, myPosition, lengthExaggeration * 0.5, widthExaggeration * 0.5, lengthExaggeration * 0.5, 0, myAngle, 1);
        }
    }
}


std::string
GNEParkingSpace::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_POSITION:
            return toString(myPosition);
        case SUMO_ATTR_WIDTH:
            return toString(myWidth);
        case SUMO_ATTR_LENGTH:
            return toString(myLength);
        case SUMO_ATTR_ANGLE:
            return toString(myAngle);
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
GNEParkingSpace::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEParkingSpace::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_WIDTH:
        case SUMO_ATTR_LENGTH:
        case SUMO_ATTR_ANGLE:
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
GNEParkingSpace::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_POSITION:
            return canParse<Position>(value);
        case SUMO_ATTR_WIDTH:
            return canParse<double>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_LENGTH:
            return canParse<double>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_ANGLE:
            return canParse<double>(value);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return canParse<bool>(value);
        case GNE_ATTR_PARENT:
            return (myNet->retrieveAdditional(SUMO_TAG_PARKING_AREA, value, false) != nullptr);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEParkingSpace::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEParkingSpace::getPopUpID() const {
    return getTagStr();
}


std::string
GNEParkingSpace::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_POSITION);
}

// ===========================================================================
// private
// ===========================================================================

void
GNEParkingSpace::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_POSITION:
            myNet->removeGLObjectFromGrid(this);
            myPosition = parse<Position>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_WIDTH:
            myNet->removeGLObjectFromGrid(this);
            myWidth = parse<double>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_LENGTH:
            myNet->removeGLObjectFromGrid(this);
            myLength = parse<double>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_ANGLE:
            myNet->removeGLObjectFromGrid(this);
            myAngle = parse<double>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case GNE_ATTR_BLOCK_MOVEMENT:
            myBlockMovement = parse<bool>(value);
            break;
        case GNE_ATTR_PARENT:
            replaceAdditionalParent(SUMO_TAG_PARKING_AREA, value, 0);
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
