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
/// @file    GNEPersonStop.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2020
///
// Representation of person stops in NETEDIT
/****************************************************************************/
#include <cmath>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_EnableAttribute.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/vehicle/SUMORouteHandler.h>

#include "GNEPersonStop.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEPersonStop::GNEPersonStop(GNENet* net, GNEDemandElement* personParent, GNEAdditional* stoppingPlace, const SUMOVehicleParameter::Stop& stopParameter) :
    GNEDemandElement(personParent, net, GLO_PERSONSTOP, GNE_TAG_PERSONSTOP_BUSSTOP,
{}, {}, {}, {stoppingPlace}, {}, {}, {personParent}, {}),
SUMOVehicleParameter::Stop(stopParameter) {
}


GNEPersonStop::GNEPersonStop(GNENet* net, GNEDemandElement* personParent, GNEEdge* edge, const SUMOVehicleParameter::Stop& stopParameter) :
    GNEDemandElement(personParent, net, GLO_PERSONSTOP, GNE_TAG_PERSONSTOP_EDGE,
{}, {edge}, {}, {}, {}, {}, {personParent}, {}),
SUMOVehicleParameter::Stop(stopParameter) {
}


GNEPersonStop::~GNEPersonStop() {}


std::string
GNEPersonStop::getBegin() const {
    return "";
}


void
GNEPersonStop::writeDemandElement(OutputDevice& device) const {
    write(device);
}


bool
GNEPersonStop::isDemandElementValid() const {
    // only Stops placed over lanes can be invalid
    if (myTagProperty.getTag() != SUMO_TAG_STOP_LANE) {
        return true;
    } else if (friendlyPos) {
        // with friendly position enabled position are "always fixed"
        return true;
    } else {
        // obtain lane length
        double laneLength = getParentEdges().front()->getNBEdge()->getFinalLength() * getParentLanes().front()->getLengthGeometryFactor();
        // declare a copy of start and end positions
        double startPosCopy = startPos;
        double endPosCopy = endPos;
        // check if position has to be fixed
        if (startPosCopy < 0) {
            startPosCopy += laneLength;
        }
        if (endPosCopy < 0) {
            endPosCopy += laneLength;
        }
        // check values
        if (!(parametersSet & STOP_START_SET) && !(parametersSet & STOP_END_SET)) {
            return true;
        } else if (!(parametersSet & STOP_START_SET)) {
            return (endPosCopy <= getParentEdges().front()->getNBEdge()->getFinalLength());
        } else if (!(parametersSet & STOP_END_SET)) {
            return (startPosCopy >= 0);
        } else {
            return ((startPosCopy >= 0) && (endPosCopy <= getParentEdges().front()->getNBEdge()->getFinalLength()) && ((endPosCopy - startPosCopy) >= POSITION_EPS));
        }
    }
}


std::string
GNEPersonStop::getDemandElementProblem() const {
    // declare a copy of start and end positions
    double startPosCopy = startPos;
    double endPosCopy = endPos;
    // obtain lane length
    double laneLength = getParentEdges().front()->getNBEdge()->getFinalLength();
    // check if position has to be fixed
    if (startPosCopy < 0) {
        startPosCopy += laneLength;
    }
    if (endPosCopy < 0) {
        endPosCopy += laneLength;
    }
    // declare variables
    std::string errorStart, separator, errorEnd;
    // check positions over lane
    if (startPosCopy < 0) {
        errorStart = (toString(SUMO_ATTR_STARTPOS) + " < 0");
    } else if (startPosCopy > getParentEdges().front()->getNBEdge()->getFinalLength()) {
        errorStart = (toString(SUMO_ATTR_STARTPOS) + " > lanes's length");
    }
    if (endPosCopy < 0) {
        errorEnd = (toString(SUMO_ATTR_ENDPOS) + " < 0");
    } else if (endPosCopy > getParentEdges().front()->getNBEdge()->getFinalLength()) {
        errorEnd = (toString(SUMO_ATTR_ENDPOS) + " > lanes's length");
    }
    // check separator
    if ((errorStart.size() > 0) && (errorEnd.size() > 0)) {
        separator = " and ";
    }
    return errorStart + separator + errorEnd;
}


void
GNEPersonStop::fixDemandElementProblem() {
    //
}


SUMOVehicleClass
GNEPersonStop::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNEPersonStop::getColor() const {
    return myNet->getViewNet()->getVisualisationSettings().colorSettings.personStops;
}


void
GNEPersonStop::startGeometryMoving() {
    // only start geometry moving if stop is placed over a lane
    if (getParentLanes().size() > 0) {
        // always save original position over view
        myStopMove.originalViewPosition = getPositionInView();
        // save start and end position
        myStopMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_STARTPOS);
        myStopMove.secondOriginalPosition = getAttribute(SUMO_ATTR_ENDPOS);
        // save current centering boundary
        myStopMove.movingGeometryBoundary = getCenteringBoundary();
    }
}


void
GNEPersonStop::endGeometryMoving() {
    // check that stop is placed over a lane and endGeometryMoving was called only once
    if ((getParentLanes().size() > 0) && myStopMove.movingGeometryBoundary.isInitialised()) {
        // reset myMovingGeometryBoundary
        myStopMove.movingGeometryBoundary.reset();
    }
}


void
GNEPersonStop::moveGeometry(const Position& offset) {
    // only move if at leats start or end positions is defined
    if ((getParentLanes().size() > 0) && ((parametersSet & STOP_START_SET) || (parametersSet & STOP_END_SET))) {
        // Calculate new position using old position
        Position newPosition = myStopMove.originalViewPosition;
        newPosition.add(offset);
        // filtern position using snap to active grid
        newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
        double offsetLane = getParentLanes().front()->getLaneShape().nearest_offset_to_point2D(newPosition, false) - getParentLanes().front()->getLaneShape().nearest_offset_to_point2D(myStopMove.originalViewPosition, false);
        // check if both position has to be moved
        if ((parametersSet & STOP_START_SET) && (parametersSet & STOP_END_SET)) {
            // calculate stoppingPlace length and lane length (After apply geometry factor)
            double stoppingPlaceLength = fabs(parse<double>(myStopMove.secondOriginalPosition) - parse<double>(myStopMove.firstOriginalLanePosition));
            double laneLengt = getParentEdges().front()->getNBEdge()->getFinalLength() * getParentLanes().front()->getLengthGeometryFactor();
            // avoid changing stopping place's length
            if ((parse<double>(myStopMove.firstOriginalLanePosition) + offsetLane) < 0) {
                startPos = 0;
                endPos = stoppingPlaceLength;
            } else if ((parse<double>(myStopMove.secondOriginalPosition) + offsetLane) > laneLengt) {
                startPos = laneLengt - stoppingPlaceLength;
                endPos = laneLengt;
            } else {
                startPos = parse<double>(myStopMove.firstOriginalLanePosition) + offsetLane;
                endPos = parse<double>(myStopMove.secondOriginalPosition) + offsetLane;
            }
        } else {
            // check if start position must be moved
            if ((parametersSet & STOP_START_SET)) {
                startPos = parse<double>(myStopMove.firstOriginalLanePosition) + offsetLane;
            }
            // check if start position must be moved
            if ((parametersSet & STOP_END_SET)) {
                endPos = parse<double>(myStopMove.secondOriginalPosition) + offsetLane;
            }
        }
        // update geometry
        updateGeometry();
    }
}


void
GNEPersonStop::commitGeometryMoving(GNEUndoList* undoList) {
    // only commit geometry moving if at leats start or end positions is defined
    if ((getParentLanes().size() > 0) && ((parametersSet & STOP_START_SET) || (parametersSet & STOP_END_SET))) {
        undoList->p_begin("position of " + getTagStr());
        if (parametersSet & STOP_START_SET) {
            undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_STARTPOS, toString(startPos), myStopMove.firstOriginalLanePosition));
        }
        if (parametersSet & STOP_END_SET) {
            undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_ENDPOS, toString(endPos), myStopMove.secondOriginalPosition));
        }
        undoList->p_end();
    }
}


void
GNEPersonStop::updateGeometry() {
    // only update Stops over lanes, because other uses the geometry of stopping place parent
    if (getParentEdges().size() > 0) {
        /*
            // Cut shape using as delimitators fixed start position and fixed end position
            myDemandElementGeometry.updateGeometry(getParentLanes().front()->getLaneShape(), getStartGeometryPositionOverLane(), getEndGeometryPositionOverLane());
        */
    } else if (getParentAdditionals().size() > 0) {
        // use geometry of additional (busStop)
        myDemandElementGeometry.updateGeometry(getParentAdditionals().at(0));
    }
    // compute previous and next person plan
    GNEDemandElement* previousDemandElement = getParentDemandElements().front()->getPreviousChildDemandElement(this);
    if (previousDemandElement) {
        previousDemandElement->updateGeometry();
    }
    GNEDemandElement* nextDemandElement = getParentDemandElements().front()->getNextChildDemandElement(this);
    if (nextDemandElement) {
        nextDemandElement->updateGeometry();
    }
}


void
GNEPersonStop::computePath() {
    // nothing to compute
}


void
GNEPersonStop::invalidatePath() {
    // nothing to invalidate
}


Position
GNEPersonStop::getPositionInView() const {
    if (getParentLanes().size() > 0) {
        // calculate start and end positions as absolute values
        double start = fabs(parametersSet & STOP_START_SET ? startPos : 0);
        double end = fabs(parametersSet & STOP_END_SET ? endPos : getParentEdges().front()->getNBEdge()->getFinalLength());
        // obtain position in view depending if both positions are defined
        if (!(parametersSet & STOP_START_SET) && !(parametersSet & STOP_END_SET)) {
            return getParentLanes().front()->getLaneShape().positionAtOffset(getParentLanes().front()->getLaneShape().length() / 2);
        } else if (!(parametersSet & STOP_START_SET)) {
            return getParentLanes().front()->getLaneShape().positionAtOffset(end);
        } else if (!(parametersSet & STOP_END_SET)) {
            return getParentLanes().front()->getLaneShape().positionAtOffset(start);
        } else {
            return getParentLanes().front()->getLaneShape().positionAtOffset((start + end) / 2.0);
        }
    } else if (getParentDemandElements().size() > 0) {
        return getParentDemandElements().front()->getPositionInView();
    } else {
        throw ProcessError("Invalid Stop parent");
    }
}


std::string
GNEPersonStop::getParentName() const {
    if (getParentDemandElements().size() > 0) {
        return getParentDemandElements().front()->getID();
    } else if (getParentAdditionals().size() > 0) {
        return getParentAdditionals().front()->getID();
    } else if (getParentLanes().size() > 0) {
        return getParentLanes().front()->getID();
    } else {
        throw ProcessError("Invalid parent");
    }
}


Boundary
GNEPersonStop::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (getParentAdditionals().size() > 0) {
        return getParentAdditionals().at(0)->getCenteringBoundary();
    } else if (myStopMove.movingGeometryBoundary.isInitialised()) {
        return myStopMove.movingGeometryBoundary;
    } else if (myDemandElementGeometry.getShape().size() > 0) {
        Boundary b = myDemandElementGeometry.getShape().getBoxBoundary();
        b.grow(20);
        return b;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNEPersonStop::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEPersonStop::drawGL(const GUIVisualizationSettings& s) const {
    // declare flag to enable or disable draw person plan
    bool drawPersonPlan = false;
    if (myTagProperty.isPersonStop()) {
        if (myNet->getViewNet()->getNetworkViewOptions().showDemandElements() && myNet->getViewNet()->getDataViewOptions().showDemandElements() &&
                myNet->getViewNet()->getDemandViewOptions().showNonInspectedDemandElements(this)) {
            drawPersonPlan = true;
        }
    } else if (myNet->getViewNet()->getDemandViewOptions().showAllPersonPlans()) {
        drawPersonPlan = true;
    } else if (myNet->getViewNet()->getInspectedAttributeCarrier() == getParentDemandElements().front()) {
        drawPersonPlan = true;
    } else if (myNet->getViewNet()->getDemandViewOptions().getLockedPerson() == getParentDemandElements().front()) {
        drawPersonPlan = true;
    } else if (myNet->getViewNet()->getInspectedAttributeCarrier() && myNet->getViewNet()->getInspectedAttributeCarrier()->getTagProperty().isPersonPlan() &&
               (myNet->getViewNet()->getInspectedAttributeCarrier()->getAttribute(GNE_ATTR_PARENT) == getAttribute(GNE_ATTR_PARENT))) {
        drawPersonPlan = true;
    }
    // check if stop can be drawn
    if (drawPersonPlan) {
        // Obtain exaggeration of the draw
        const double exaggeration = s.addSize.getExaggeration(s, this);
        // declare value to save stop color
        RGBColor stopColor;
        // Set color
        if (drawUsingSelectColor()) {
            stopColor = s.colorSettings.selectedPersonPlanColor;
        } else {
            stopColor = s.colorSettings.stops;
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // set Color
        GLHelper::setColor(stopColor);
        // Start with the drawing of the area traslating matrix to origin
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
        // draw depending of details
        if (s.drawDetail(s.detailSettings.stopsDetails, exaggeration) && getParentLanes().size() > 0) {
            // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
            GLHelper::drawBoxLines(myDemandElementGeometry.getShape(), myDemandElementGeometry.getShapeRotations(), myDemandElementGeometry.getShapeLengths(), exaggeration * 0.1, 0,
                                   getParentEdges().front()->getNBEdge()->getLaneWidth(getParentLanes().front()->getIndex()) * 0.5);
            GLHelper::drawBoxLines(myDemandElementGeometry.getShape(), myDemandElementGeometry.getShapeRotations(), myDemandElementGeometry.getShapeLengths(), exaggeration * 0.1, 0,
                                   getParentEdges().front()->getNBEdge()->getLaneWidth(getParentLanes().front()->getIndex()) * -0.5);
            // pop draw matrix
            glPopMatrix();
            // Add a draw matrix
            glPushMatrix();
            // Start with the drawing of the area traslating matrix to origin
            myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
            // move to geometry front
            glTranslated(myDemandElementGeometry.getShape().back().x(), myDemandElementGeometry.getShape().back().y(), 0);
            glRotated(myDemandElementGeometry.getShapeRotations().back(), 0, 0, 1);
            // draw front of Stop depending if it's placed over a lane or over a stoppingPlace
            if (getParentLanes().size() > 0) {
                // draw front of Stop
                GLHelper::drawBoxLine(Position(0, 0), 0, exaggeration * 0.5,
                                      getParentEdges().front()->getNBEdge()->getLaneWidth(getParentLanes().front()->getIndex()) * 0.5);
            } else {
                // draw front of Stop
                GLHelper::drawBoxLine(Position(0, 0), 0, exaggeration * 0.5, exaggeration);
            }
            // move to "S" position
            glTranslated(0, 1, 0);
            // only draw text if isn't being drawn for selecting
            if (s.drawForRectangleSelection) {
                GLHelper::setColor(stopColor);
                GLHelper::drawBoxLine(Position(0, 1), 0, 2, 1);
            } else if (s.drawDetail(s.detailSettings.stopsText, exaggeration)) {
                // draw "S" symbol
                GLHelper::drawText("S", Position(), .1, 2.8, stopColor);
                // move to subtitle positin
                glTranslated(0, 1.4, 0);
                // draw subtitle depending of tag
                GLHelper::drawText("lane", Position(), .1, 1, stopColor, 180);
            }
            // pop draw matrix
            glPopMatrix();
            // Draw name if isn't being drawn for selecting
            drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
            // check if dotted contour has to be drawn
            if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
                // draw dooted contour depending if it's placed over a lane or over a stoppingPlace
                if (getParentLanes().size() > 0) {
                    // GLHelper::drawShapeDottedContourAroundShape(s, getType(), myDemandElementGeometry.getShape(),
                    //        getParentEdges().front()->getNBEdge()->getLaneWidth(getParentLanes().front()->getIndex()) * 0.5);
                } else {
                    // GLHelper::drawShapeDottedContourAroundShape(s, getType(), myDemandElementGeometry.getShape(), exaggeration);
                }
            }
        } else {
            // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
            GNEGeometry::drawGeometry(myNet->getViewNet(), myDemandElementGeometry, exaggeration * 0.8);
            // pop draw matrix
            glPopMatrix();
        }
        // Pop name
        glPopName();
        // draw person parent if this stop if their first person plan child
        if ((getParentDemandElements().size() == 1) && getParentDemandElements().front()->getChildDemandElements().front() == this) {
            getParentDemandElements().front()->drawGL(s);
        }
    }
}


void
GNEPersonStop::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*lane*/, const double /*offsetFront*/) const {
    // personStops don't use drawPartialGL
}


void
GNEPersonStop::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*fromLane*/, const GNELane* /*toLane*/, const double /*offsetFront*/) const {
    // personStops don't use drawPartialGL
}


std::string
GNEPersonStop::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getParentDemandElements().front()->getID();
        case SUMO_ATTR_DURATION:
            if (parametersSet & STOP_DURATION_SET) {
                return time2string(duration);
            } else {
                return "";
            }
        case SUMO_ATTR_UNTIL:
            if (parametersSet & STOP_UNTIL_SET) {
                return time2string(until);
            } else {
                return "";
            }
        case SUMO_ATTR_EXTENSION:
            if (parametersSet & STOP_EXTENSION_SET) {
                return time2string(extension);
            } else {
                return "";
            }
        case SUMO_ATTR_INDEX:
            if (index == STOP_INDEX_END) {
                return "end";
            } else if (index == STOP_INDEX_FIT) {
                return "fit";
            } else {
                return toString(index);
            }
        case SUMO_ATTR_TRIGGERED:
            // this is an special case
            if (parametersSet & STOP_TRIGGER_SET) {
                return "1";
            } else {
                return "0";
            }
        case SUMO_ATTR_CONTAINER_TRIGGERED:
            // this is an special case
            if (parametersSet & STOP_CONTAINER_TRIGGER_SET) {
                return "1";
            } else {
                return "0";
            }
        case SUMO_ATTR_EXPECTED:
            if (parametersSet & STOP_EXPECTED_SET) {
                return toString(awaitedPersons);
            } else {
                return "";
            }
        case SUMO_ATTR_EXPECTED_CONTAINERS:
            if (parametersSet & STOP_EXPECTED_CONTAINERS_SET) {
                return toString(awaitedContainers);
            } else {
                return "";
            }
        case SUMO_ATTR_PARKING:
            return toString(parking);
        case SUMO_ATTR_ACTTYPE:
            return actType;
        case SUMO_ATTR_TRIP_ID:
            if (parametersSet & STOP_TRIP_ID_SET) {
                return tripId;
            } else {
                return "";
            }
        // specific of Stops over stoppingPlaces
        case SUMO_ATTR_BUS_STOP:
        case SUMO_ATTR_CONTAINER_STOP:
        case SUMO_ATTR_CHARGING_STATION:
        case SUMO_ATTR_PARKING_AREA:
            return getParentAdditionals().front()->getID();
        // specific of stops over lanes
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        case SUMO_ATTR_STARTPOS:
            if (parametersSet & STOP_START_SET) {
                return toString(startPos);
            } else {
                return "";
            }
        case SUMO_ATTR_ENDPOS:
            if (parametersSet & STOP_END_SET) {
                return toString(endPos);
            } else {
                return "";
            }
        case SUMO_ATTR_FRIENDLY_POS:
            return toString(friendlyPos);
        //
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARENT:
            return getParentDemandElements().front()->getID();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEPersonStop::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_STARTPOS:
            if (parametersSet & STOP_START_SET) {
                return startPos;
            } else {
                return 0;
            }
        case SUMO_ATTR_ENDPOS:
            if (parametersSet & STOP_END_SET) {
                return endPos;
            } else {
                return getParentEdges().front()->getNBEdge()->getFinalLength();
            }
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPersonStop::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_DURATION:
        case SUMO_ATTR_UNTIL:
        case SUMO_ATTR_EXTENSION:
        case SUMO_ATTR_INDEX:
        case SUMO_ATTR_TRIGGERED:
        case SUMO_ATTR_CONTAINER_TRIGGERED:
        case SUMO_ATTR_EXPECTED:
        case SUMO_ATTR_EXPECTED_CONTAINERS:
        case SUMO_ATTR_PARKING:
        case SUMO_ATTR_ACTTYPE:
        case SUMO_ATTR_TRIP_ID:
        // specific of Stops over stoppingPlaces
        case SUMO_ATTR_BUS_STOP:
        case SUMO_ATTR_CONTAINER_STOP:
        case SUMO_ATTR_CHARGING_STATION:
        case SUMO_ATTR_PARKING_AREA:
        // specific of stops over lanes
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_STARTPOS:
        case SUMO_ATTR_ENDPOS:
        case SUMO_ATTR_FRIENDLY_POS:
        //
        case GNE_ATTR_SELECTED:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEPersonStop::isValid(SumoXMLAttr key, const std::string& value) {
    // declare string error
    std::string error;
    switch (key) {
        case SUMO_ATTR_DURATION:
        case SUMO_ATTR_UNTIL:
        case SUMO_ATTR_EXTENSION:
            if (canParse<SUMOTime>(value)) {
                return parse<SUMOTime>(value) >= 0;
            } else {
                return false;
            }
        case SUMO_ATTR_INDEX:
            if ((value == "fit") || (value == "end")) {
                return true;
            } else if (canParse<int>(value)) {
                return (parse<int>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_TRIGGERED:
            return canParse<bool>(value);
        case SUMO_ATTR_CONTAINER_TRIGGERED:
            return canParse<bool>(value);
        case SUMO_ATTR_EXPECTED:
        case SUMO_ATTR_EXPECTED_CONTAINERS:
            if (value.empty()) {
                return true;
            } else {
                std::vector<std::string> IDs = parse<std::vector<std::string>>(value);
                for (const auto& i : IDs) {
                    if (SUMOXMLDefinitions::isValidVehicleID(i) == false) {
                        return false;
                    }
                }
                return true;
            }
        case SUMO_ATTR_PARKING:
            return canParse<bool>(value);
        case SUMO_ATTR_ACTTYPE:
            return true;
        case SUMO_ATTR_TRIP_ID:
            return SUMOXMLDefinitions::isValidVehicleID(value);
        // specific of Stops over stoppingPlaces
        case SUMO_ATTR_BUS_STOP:
            return (myNet->retrieveAdditional(SUMO_TAG_BUS_STOP, value, false) != nullptr);
        case SUMO_ATTR_CONTAINER_STOP:
            return (myNet->retrieveAdditional(SUMO_TAG_CONTAINER_STOP, value, false) != nullptr);
        case SUMO_ATTR_CHARGING_STATION:
            return (myNet->retrieveAdditional(SUMO_TAG_CHARGING_STATION, value, false) != nullptr);
        case SUMO_ATTR_PARKING_AREA:
            return (myNet->retrieveAdditional(SUMO_TAG_PARKING_AREA, value, false) != nullptr);
        // specific of stops over lanes
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
                return SUMORouteHandler::isStopPosValid(parse<double>(value), endPos, getParentEdges().front()->getNBEdge()->getFinalLength(), POSITION_EPS, friendlyPos);
            } else {
                return false;
            }
        case SUMO_ATTR_ENDPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return SUMORouteHandler::isStopPosValid(startPos, parse<double>(value), getParentEdges().front()->getNBEdge()->getFinalLength(), POSITION_EPS, friendlyPos);
            } else {
                return false;
            }
        case SUMO_ATTR_FRIENDLY_POS:
            return canParse<bool>(value);
        //
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPersonStop::enableAttribute(SumoXMLAttr key, GNEUndoList* undoList) {
    // obtain a copy of parameter sets
    int newParametersSet = parametersSet;
    // modify parametersSetCopy depending of attr
    switch (key) {
        case SUMO_ATTR_STARTPOS:
            newParametersSet |= STOP_START_SET;
            break;
        case SUMO_ATTR_ENDPOS:
            newParametersSet |= STOP_END_SET;
            break;
        case SUMO_ATTR_DURATION:
            newParametersSet |= STOP_DURATION_SET;
            break;
        case SUMO_ATTR_UNTIL:
            newParametersSet |= STOP_UNTIL_SET;
            break;
        case SUMO_ATTR_EXTENSION:
            newParametersSet |= STOP_EXTENSION_SET;
            break;
        case SUMO_ATTR_EXPECTED:
            newParametersSet |= STOP_TRIGGER_SET;
            break;
        case SUMO_ATTR_EXPECTED_CONTAINERS:
            newParametersSet |= STOP_CONTAINER_TRIGGER_SET;
            break;
        case SUMO_ATTR_PARKING:
            newParametersSet |= STOP_PARKING_SET;
            break;
        default:
            break;
    }
    // add GNEChange_EnableAttribute
    undoList->add(new GNEChange_EnableAttribute(this, parametersSet, newParametersSet), true);
    // modify parametersSetCopy depending of attr
    switch (key) {
        case SUMO_ATTR_STARTPOS:
            if (parametersSet & STOP_END_SET) {
                undoList->p_add(new GNEChange_Attribute(this, key, toString(endPos - MIN_STOP_LENGTH)));
            } else {
                undoList->p_add(new GNEChange_Attribute(this, key, toString(getParentEdges().front()->getNBEdge()->getFinalLength() - MIN_STOP_LENGTH)));
            }
            break;
        case SUMO_ATTR_ENDPOS:
            undoList->p_add(new GNEChange_Attribute(this, key, toString(getParentEdges().front()->getNBEdge()->getFinalLength())));
            break;
        case SUMO_ATTR_DURATION:
            undoList->p_add(new GNEChange_Attribute(this, key, myTagProperty.getAttributeProperties(key).getDefaultValue()));
            break;
        case SUMO_ATTR_UNTIL:
        case SUMO_ATTR_EXTENSION:
            undoList->p_add(new GNEChange_Attribute(this, key, myTagProperty.getAttributeProperties(key).getDefaultValue()));
            break;
        default:
            break;
    }
}


void
GNEPersonStop::disableAttribute(SumoXMLAttr key, GNEUndoList* undoList) {
    // obtain a copy of parameter sets
    int newParametersSet = parametersSet;
    // modify parametersSetCopy depending of attr
    switch (key) {
        case SUMO_ATTR_STARTPOS:
            newParametersSet &= ~STOP_START_SET;
            break;
        case SUMO_ATTR_ENDPOS:
            newParametersSet &= ~STOP_END_SET;
            break;
        case SUMO_ATTR_DURATION:
            newParametersSet &= ~STOP_DURATION_SET;
            break;
        case SUMO_ATTR_UNTIL:
            newParametersSet &= ~STOP_UNTIL_SET;
            break;
        case SUMO_ATTR_EXTENSION:
            newParametersSet &= ~STOP_EXTENSION_SET;
            break;
        case SUMO_ATTR_EXPECTED:
            newParametersSet &= ~STOP_TRIGGER_SET;
            break;
        case SUMO_ATTR_EXPECTED_CONTAINERS:
            newParametersSet &= ~STOP_CONTAINER_TRIGGER_SET;
            break;
        case SUMO_ATTR_PARKING:
            newParametersSet &= ~STOP_PARKING_SET;
            break;
        default:
            break;
    }
    // add GNEChange_EnableAttribute
    undoList->add(new GNEChange_EnableAttribute(this, parametersSet, newParametersSet), true);
}


bool
GNEPersonStop::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        // Currently stops parents cannot be edited
        case SUMO_ATTR_BUS_STOP:
        case SUMO_ATTR_CONTAINER_STOP:
        case SUMO_ATTR_CHARGING_STATION:
        case SUMO_ATTR_PARKING_AREA:
            return false;
        case SUMO_ATTR_STARTPOS:
            return (parametersSet & STOP_START_SET) != 0;
        case SUMO_ATTR_ENDPOS:
            return (parametersSet & STOP_END_SET) != 0;
        case SUMO_ATTR_DURATION:
            return (parametersSet & STOP_DURATION_SET) != 0;
        case SUMO_ATTR_UNTIL:
            return (parametersSet & STOP_UNTIL_SET) != 0;
        case SUMO_ATTR_EXTENSION:
            return (parametersSet & STOP_EXTENSION_SET) != 0;
        case SUMO_ATTR_EXPECTED:
            return (parametersSet & STOP_TRIGGER_SET) != 0;
        case SUMO_ATTR_EXPECTED_CONTAINERS:
            return (parametersSet & STOP_CONTAINER_TRIGGER_SET) != 0;
        case SUMO_ATTR_PARKING:
            return (parametersSet & STOP_PARKING_SET) != 0;
        default:
            return true;
    }
}


std::string
GNEPersonStop::getPopUpID() const {
    return getTagStr();
}


std::string
GNEPersonStop::getHierarchyName() const {
    if (getParentAdditionals().size() > 0) {
        return "person stop: " + getParentAdditionals().front()->getTagStr();
    } else {
        return "person stop: lane";
    }
}


double
GNEPersonStop::getStartGeometryPositionOverLane() const {
    double fixedPos = 0;
    if (parametersSet & STOP_START_SET) {
        fixedPos = startPos;
    } else if (parametersSet & STOP_END_SET) {
        fixedPos = endPos - MIN_STOP_LENGTH;
    } else {
        fixedPos = getParentEdges().front()->getNBEdge()->getFinalLength() - MIN_STOP_LENGTH;
    }
    const double len = getParentEdges().front()->getNBEdge()->getFinalLength();
    if (fixedPos < 0) {
        fixedPos += len;
    }
    return fixedPos * getParentLanes().front()->getLengthGeometryFactor();
}


double
GNEPersonStop::getEndGeometryPositionOverLane() const {
    double fixedPos = 0;
    if (parametersSet & STOP_END_SET) {
        fixedPos = endPos;
    } else {
        fixedPos = getParentEdges().front()->getNBEdge()->getFinalLength();
    }
    const double len = getParentEdges().front()->getNBEdge()->getFinalLength();
    if (fixedPos < 0) {
        fixedPos += len;
    }
    return fixedPos * getParentLanes().front()->getLengthGeometryFactor();
}

// ===========================================================================
// private
// ===========================================================================

void
GNEPersonStop::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_DURATION:
            if (value.empty()) {
                parametersSet &= ~STOP_DURATION_SET;
            } else {
                duration = string2time(value);
                parametersSet |= STOP_DURATION_SET;
            }
            break;
        case SUMO_ATTR_UNTIL:
            if (value.empty()) {
                parametersSet &= ~STOP_UNTIL_SET;
            } else {
                until = string2time(value);
                parametersSet |= STOP_UNTIL_SET;
            }
            break;
        case SUMO_ATTR_EXTENSION:
            if (value.empty()) {
                parametersSet &= ~STOP_EXTENSION_SET;
            } else {
                extension = string2time(value);
                parametersSet |= STOP_EXTENSION_SET;
            }
            break;
        case SUMO_ATTR_INDEX:
            if (value == "fit") {
                index = STOP_INDEX_FIT;
            } else if (value == "end") {
                index = STOP_INDEX_END;
            } else {
                index = parse<int>(value);
            }
            break;
        case SUMO_ATTR_TRIGGERED:
            triggered = parse<bool>(value);
            // this is an special case: only if SUMO_ATTR_TRIGGERED is true, it will be written in XML
            if (triggered) {
                parametersSet |= STOP_TRIGGER_SET;
            } else {
                parametersSet &= ~STOP_TRIGGER_SET;
            }
            break;
        case SUMO_ATTR_CONTAINER_TRIGGERED:
            containerTriggered = parse<bool>(value);
            // this is an special case: only if SUMO_ATTR_CONTAINER_TRIGGERED is true, it will be written in XML
            if (containerTriggered) {
                parametersSet |= STOP_CONTAINER_TRIGGER_SET;
            } else {
                parametersSet &= ~STOP_CONTAINER_TRIGGER_SET;
            }
            break;
        case SUMO_ATTR_EXPECTED:
            if (value.empty()) {
                parametersSet &= ~STOP_EXPECTED_SET;
            } else {
                awaitedPersons = parse<std::set<std::string> >(value);
                parametersSet |= STOP_EXPECTED_SET;
            }
            break;
        case SUMO_ATTR_EXPECTED_CONTAINERS:
            if (value.empty()) {
                parametersSet &= ~STOP_EXPECTED_CONTAINERS_SET;
            } else {
                awaitedContainers = parse<std::set<std::string> >(value);
                parametersSet |= STOP_EXPECTED_CONTAINERS_SET;
            }
            break;
        case SUMO_ATTR_PARKING:
            parking = parse<bool>(value);
            break;
        case SUMO_ATTR_ACTTYPE:
            actType = value;
            break;
        case SUMO_ATTR_TRIP_ID:
            if (value.empty()) {
                parametersSet &= ~STOP_TRIP_ID_SET;
            } else {
                tripId = value;
                parametersSet |= STOP_TRIP_ID_SET;
            }
            break;
        // specific of Stops over stoppingPlaces
        case SUMO_ATTR_BUS_STOP:
        case SUMO_ATTR_CONTAINER_STOP:
        case SUMO_ATTR_CHARGING_STATION:
        case SUMO_ATTR_PARKING_AREA:
            replaceAdditionalParent(SUMO_TAG_BUS_STOP, value, 0);
            updateGeometry();
            break;
        // specific of Stops over lanes
        case SUMO_ATTR_LANE:
            replaceDemandParentLanes(value);
            updateGeometry();
            break;
        case SUMO_ATTR_STARTPOS:
            if (value.empty()) {
                parametersSet &= ~STOP_START_SET;
            } else {
                startPos = parse<double>(value);
                parametersSet |= STOP_START_SET;
            }
            updateGeometry();
            break;
        case SUMO_ATTR_ENDPOS:
            if (value.empty()) {
                parametersSet &= ~STOP_END_SET;
            } else {
                endPos = parse<double>(value);
                parametersSet |= STOP_END_SET;
            }
            updateGeometry();
            break;
        case SUMO_ATTR_FRIENDLY_POS:
            friendlyPos = parse<bool>(value);
            break;
        //
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPersonStop::setEnabledAttribute(const int enabledAttributes) {
    parametersSet = enabledAttributes;
}


/****************************************************************************/
