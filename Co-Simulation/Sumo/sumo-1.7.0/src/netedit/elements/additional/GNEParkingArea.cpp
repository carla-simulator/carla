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
/// @file    GNEParkingArea.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// A lane area vehicles can park at (GNE version)
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/vehicle/SUMORouteHandler.h>

#include "GNEParkingArea.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEParkingArea::GNEParkingArea(const std::string& id, GNELane* lane, GNENet* net, const double startPos, const double endPos, const int parametersSet,
                               const std::string& name, bool friendlyPosition, int roadSideCapacity, bool onRoad, double width, const std::string& length, double angle, bool blockMovement) :
    GNEStoppingPlace(id, net, GLO_PARKING_AREA, SUMO_TAG_PARKING_AREA, lane, startPos, endPos, parametersSet, name, friendlyPosition, blockMovement),
    myRoadSideCapacity(roadSideCapacity),
    myOnRoad(onRoad),
    myWidth(width),
    myLength(length),
    myAngle(angle) {
}


GNEParkingArea::~GNEParkingArea() {}


void
GNEParkingArea::updateGeometry() {
    // first check if object has to be removed from grid (SUMOTree)
    if (!myMove.movingGeometryBoundary.isInitialised()) {
        myNet->removeGLObjectFromGrid(this);
    }

    // Get value of option "lefthand"
    double offsetSign = OptionsCont::getOptions().getBool("lefthand") ? -1 : 1;

    // Update common geometry of stopping place
    setStoppingPlaceGeometry(getParentLanes().front()->getParentEdge()->getNBEdge()->getLaneWidth(getParentLanes().front()->getIndex()) / 2 + myWidth);

    // Obtain a copy of the shape
    PositionVector tmpShape = myAdditionalGeometry.getShape();

    // Move shape to side
    tmpShape.move2side(1.5 * offsetSign);

    // Get position of the sign
    mySignPos = tmpShape.getLineCenter();

    // update block icon position
    myBlockIcon.updatePositionAndRotation();

    // last step is to check if object has to be added into grid (SUMOTree) again
    if (!myMove.movingGeometryBoundary.isInitialised()) {
        myNet->addGLObjectIntoGrid(this);
    }
}


Boundary
GNEParkingArea::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMove.movingGeometryBoundary.isInitialised()) {
        return myMove.movingGeometryBoundary;
    } else if (myAdditionalGeometry.getShape().size() > 0) {
        Boundary b = myAdditionalGeometry.getShape().getBoxBoundary();
        b.grow(myWidth + 1);
        return b;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNEParkingArea::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEParkingArea::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double parkingAreaExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(parkingAreaExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // declare colors
        RGBColor baseColor, signColor;
        // set colors
        if (mySpecialColor) {
            baseColor = *mySpecialColor;
            signColor = baseColor.changedBrightness(-32);
        } else if (drawUsingSelectColor()) {
            baseColor = s.colorSettings.selectedAdditionalColor;
            signColor = baseColor.changedBrightness(-32);
        } else {
            baseColor = s.stoppingPlaceSettings.parkingAreaColor;
            signColor = s.stoppingPlaceSettings.parkingAreaColorSign;
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_PARKING_AREA);
        // set base color
        GLHelper::setColor(baseColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GNEGeometry::drawGeometry(myNet->getViewNet(), myAdditionalGeometry, myWidth * parkingAreaExaggeration);
        // draw detail
        if (s.drawDetail(s.detailSettings.stoppingPlaceDetails, parkingAreaExaggeration)) {
            // draw sign
            drawSign(s, parkingAreaExaggeration, baseColor, signColor, "H");
            // draw lock icon
            myBlockIcon.drawIcon(s, parkingAreaExaggeration);
        }
        // pop draw matrix
        glPopMatrix();
        // Draw name if isn't being drawn for selecting
        drawName(getPositionInView(), s.scale, s.addName);
        // Pop name
        glPopName();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(true, s, myAdditionalGeometry.getShape(), myWidth, parkingAreaExaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(false, s, myAdditionalGeometry.getShape(), myWidth, parkingAreaExaggeration);
        }
        // draw child demand elements
        for (const auto& demandElement : getChildDemandElements()) {
            if (!demandElement->getTagProperty().isPlacedInRTree()) {
                demandElement->drawGL(s);
            }
        }
    }
}


std::string
GNEParkingArea::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        case SUMO_ATTR_STARTPOS:
            if (myParametersSet & STOPPINGPLACE_STARTPOS_SET) {
                return toString(myStartPosition);
            } else {
                return "";
            }
        case SUMO_ATTR_ENDPOS:
            if (myParametersSet & STOPPINGPLACE_ENDPOS_SET) {
                return toString(myEndPosition);
            } else {
                return "";
            }
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_FRIENDLY_POS:
            return toString(myFriendlyPosition);
        case SUMO_ATTR_ROADSIDE_CAPACITY:
            return toString(myRoadSideCapacity);
        case SUMO_ATTR_ONROAD:
            return toString(myOnRoad);
        case SUMO_ATTR_WIDTH:
            return toString(myWidth);
        case SUMO_ATTR_LENGTH:
            return myLength;
        case SUMO_ATTR_ANGLE:
            return toString(myAngle);
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
GNEParkingArea::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_STARTPOS:
        case SUMO_ATTR_ENDPOS:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_FRIENDLY_POS:
        case SUMO_ATTR_ROADSIDE_CAPACITY:
        case SUMO_ATTR_ONROAD:
        case SUMO_ATTR_WIDTH:
        case SUMO_ATTR_LENGTH:
        case SUMO_ATTR_ANGLE:
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
GNEParkingArea::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_LANE:
            if (myNet->retrieveLane(value, false) != nullptr) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_STARTPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return SUMORouteHandler::isStopPosValid(parse<double>(value), myEndPosition, getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength(), POSITION_EPS, myFriendlyPosition);
            } else {
                return false;
            }
        case SUMO_ATTR_ENDPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return SUMORouteHandler::isStopPosValid(myStartPosition, parse<double>(value), getParentLanes().front()->getParentEdge()->getNBEdge()->getFinalLength(), POSITION_EPS, myFriendlyPosition);
            } else {
                return false;
            }
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_FRIENDLY_POS:
            return canParse<bool>(value);
        case SUMO_ATTR_ROADSIDE_CAPACITY:
            return canParse<double>(value) && (parse<double>(value) >= 0);
        case SUMO_ATTR_ONROAD:
            return canParse<bool>(value);
        case SUMO_ATTR_WIDTH:
            return canParse<double>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_LENGTH:
            if (value.empty()) {
                return true;
            } else {
                return canParse<double>(value) && (parse<double>(value) > 0);
            }
        case SUMO_ATTR_ANGLE:
            return canParse<double>(value);
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

// ===========================================================================
// private
// ===========================================================================

void
GNEParkingArea::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            // Change IDs of all parking Spaces
            for (const auto& parkingSpace : getChildAdditionals()) {
                parkingSpace->setMicrosimID(getID());
            }
            break;
        case SUMO_ATTR_LANE:
            replaceAdditionalParentLanes(value);
            break;
        case SUMO_ATTR_STARTPOS:
            myNet->removeGLObjectFromGrid(this);
            if (!value.empty()) {
                myStartPosition = parse<double>(value);
                myParametersSet |= STOPPINGPLACE_STARTPOS_SET;
            } else {
                myParametersSet &= ~STOPPINGPLACE_STARTPOS_SET;
            }
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_ENDPOS:
            myNet->removeGLObjectFromGrid(this);
            if (!value.empty()) {
                myEndPosition = parse<double>(value);
                myParametersSet |= STOPPINGPLACE_ENDPOS_SET;
            } else {
                myParametersSet &= ~STOPPINGPLACE_ENDPOS_SET;
            }
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            myFriendlyPosition = parse<bool>(value);
            break;
        case SUMO_ATTR_ROADSIDE_CAPACITY:
            myRoadSideCapacity = parse<int>(value);
            break;
        case SUMO_ATTR_ONROAD:
            myOnRoad = parse<bool>(value);
            break;
        case SUMO_ATTR_WIDTH:
            myNet->removeGLObjectFromGrid(this);
            myWidth = parse<double>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_LENGTH:
            myLength = value;
            break;
        case SUMO_ATTR_ANGLE:
            myAngle = parse<double>(value);
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
