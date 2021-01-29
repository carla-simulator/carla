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
/// @file    GNEBusStop.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
// A lane area vehicles can halt at (GNE version)
/****************************************************************************/
#include <foreign/fontstash/fontstash.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/options/OptionsCont.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/vehicle/SUMORouteHandler.h>

#include "GNEBusStop.h"

// ===========================================================================
// method definitions
// ===========================================================================

GNEBusStop::GNEBusStop(const std::string& id, GNELane* lane, GNENet* net, const double startPos, const double endPos, const int parametersSet,
                       const std::string& name, const std::vector<std::string>& lines, int personCapacity, double parkingLength, bool friendlyPosition, bool blockMovement) :
    GNEStoppingPlace(id, net, GLO_BUS_STOP, SUMO_TAG_BUS_STOP, lane, startPos, endPos, parametersSet, name, friendlyPosition, blockMovement),
    myLines(lines),
    myPersonCapacity(personCapacity),
    myParkingLength(parkingLength)
{ }


GNEBusStop::~GNEBusStop() {}


void
GNEBusStop::updateGeometry() {
    // Get value of option "lefthand"
    double offsetSign = OptionsCont::getOptions().getBool("lefthand") ? -1 : 1;

    // Update common geometry of stopping place
    setStoppingPlaceGeometry(getParentLanes().front()->getParentEdge()->getNBEdge()->getLaneWidth(getParentLanes().front()->getIndex()) * 0.5);

    // Obtain a copy of the shape
    PositionVector tmpShape = myAdditionalGeometry.getShape();

    // Move shape to side
    tmpShape.move2side(myNet->getViewNet()->getVisualisationSettings().stoppingPlaceSettings.stoppingPlaceSignOffset * offsetSign);

    // Get position of the sign
    mySignPos = tmpShape.getLineCenter();

    // update block icon position
    myBlockIcon.updatePositionAndRotation();

    // update child demand elements geometry
    for (const auto& i : getChildDemandElements()) {
        // special case for person trips
        if (i->getTagProperty().isPersonTrip()) {
            // update previous and next person plan
            GNEDemandElement* previousDemandElement = i->getParentDemandElements().front()->getPreviousChildDemandElement(i);
            if (previousDemandElement) {
                previousDemandElement->updatePartialGeometry(getParentLanes().front());
            }
            GNEDemandElement* nextDemandElement = i->getParentDemandElements().front()->getNextChildDemandElement(i);
            if (nextDemandElement) {
                nextDemandElement->updatePartialGeometry(getParentLanes().front());
            }
        }
        i->updatePartialGeometry(getParentLanes().front());
    }
}


Boundary
GNEBusStop::getCenteringBoundary() const {
    return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
}


void
GNEBusStop::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double busStopExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(busStopExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
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
            baseColor = s.stoppingPlaceSettings.busStopColor;
            signColor = s.stoppingPlaceSettings.busStopColorSign;
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_BUS_STOP);
        // set base color
        GLHelper::setColor(baseColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GNEGeometry::drawGeometry(myNet->getViewNet(), myAdditionalGeometry, s.stoppingPlaceSettings.busStopWidth * busStopExaggeration);
        // draw detail
        if (s.drawDetail(s.detailSettings.stoppingPlaceDetails, busStopExaggeration)) {
            // draw lines
            drawLines(s, myLines, baseColor);
            // draw sign
            drawSign(s, busStopExaggeration, baseColor, signColor, "H");
            // draw lock icon
            myBlockIcon.drawIcon(s, busStopExaggeration);
        }
        // pop draw matrix
        glPopMatrix();
        // Draw name if isn't being drawn for selecting
        drawName(getPositionInView(), s.scale, s.addName);
        // Pop name
        glPopName();
        // draw connection betwen access
        drawConnectionAccess(s, baseColor);
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(true, s, myAdditionalGeometry.getShape(), s.stoppingPlaceSettings.busStopWidth, busStopExaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(false, s, myAdditionalGeometry.getShape(), s.stoppingPlaceSettings.busStopWidth, busStopExaggeration);
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
GNEBusStop::getAttribute(SumoXMLAttr key) const {
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
        case SUMO_ATTR_LINES:
            return joinToString(myLines, " ");
        case SUMO_ATTR_PERSON_CAPACITY:
            return toString(myPersonCapacity);
        case SUMO_ATTR_PARKING_LENGTH:
            return toString(myParkingLength);
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
GNEBusStop::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
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
        case SUMO_ATTR_LINES:
        case SUMO_ATTR_PERSON_CAPACITY:
        case SUMO_ATTR_PARKING_LENGTH:
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
GNEBusStop::isValid(SumoXMLAttr key, const std::string& value) {
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
        case SUMO_ATTR_LINES:
            return canParse<std::vector<std::string> >(value);
        case SUMO_ATTR_PERSON_CAPACITY:
            return canParse<int>(value) && (parse<int>(value) > 0 || parse<int>(value) == -1);
        case SUMO_ATTR_PARKING_LENGTH:
            return canParse<double>(value) && (parse<double>(value) >= 0);
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
GNEBusStop::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            // Change IDs of all access children
            for (const auto& access : getChildAdditionals()) {
                access->setMicrosimID(getID());
            }
            break;
        case SUMO_ATTR_LANE:
            replaceAdditionalParentLanes(value);
            break;
        case SUMO_ATTR_STARTPOS:
            if (!value.empty()) {
                myStartPosition = parse<double>(value);
                myParametersSet |= STOPPINGPLACE_STARTPOS_SET;
            } else {
                myParametersSet &= ~STOPPINGPLACE_STARTPOS_SET;
            }
            break;
        case SUMO_ATTR_ENDPOS:
            if (!value.empty()) {
                myEndPosition = parse<double>(value);
                myParametersSet |= STOPPINGPLACE_ENDPOS_SET;
            } else {
                myParametersSet &= ~STOPPINGPLACE_ENDPOS_SET;
            }
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            myFriendlyPosition = parse<bool>(value);
            break;
        case SUMO_ATTR_LINES:
            myLines = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
            break;
        case SUMO_ATTR_PERSON_CAPACITY:
            myPersonCapacity = GNEAttributeCarrier::parse<int>(value);
            break;
        case SUMO_ATTR_PARKING_LENGTH:
            myParkingLength = GNEAttributeCarrier::parse<double>(value);
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


void
GNEBusStop::drawConnectionAccess(const GUIVisualizationSettings& s, const RGBColor& color) const {
    if (!s.drawForPositionSelection && !s.drawForRectangleSelection) {
        // Add a draw matrix for details
        glPushMatrix();
        // move to GLO_BUS_STOP
        glTranslated(0, 0, GLO_BUS_STOP);
        // set color
        GLHelper::setColor(color);
        // draw lines between BusStops and Access
        for (const auto& access : getChildAdditionals()) {
            GLHelper::drawBoxLine(access->getAdditionalGeometry().getPosition(),
                                  RAD2DEG(myBlockIcon.getPosition().angleTo2D(access->getAdditionalGeometry().getPosition())) - 90,
                                  myBlockIcon.getPosition().distanceTo2D(access->getAdditionalGeometry().getPosition()), .05);
        }
        // pop draw matrix
        glPopMatrix();
    }
}

/****************************************************************************/
