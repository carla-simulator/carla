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
/// @file    GNERide.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// A class for visualizing rides in Netedit
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>

#include "GNERide.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNERide::GNERide(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge,
                 double arrivalPosition, const std::vector<std::string>& lines) :
    GNEDemandElement(personParent, net, GLO_RIDE, GNE_TAG_RIDE_EDGE_EDGE,
{}, {fromEdge, toEdge}, {}, {}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myLines(lines) {
    // compute ride
    computePath();
}


GNERide::GNERide(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEAdditional* toBusStop,
                 double arrivalPosition, const std::vector<std::string>& lines) :
    GNEDemandElement(personParent, net, GLO_RIDE, GNE_TAG_RIDE_EDGE_BUSSTOP,
{}, {fromEdge}, {}, {toBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myLines(lines) {
    // compute ride
    computePath();
}

GNERide::GNERide(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEEdge* toEdge,
                 double arrivalPosition, const std::vector<std::string>& lines) :
    GNEDemandElement(personParent, net, GLO_RIDE, GNE_TAG_RIDE_BUSSTOP_EDGE,
{}, {toEdge}, {}, {fromBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myLines(lines) {
    // compute ride
    computePath();
}


GNERide::GNERide(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEAdditional* toBusStop,
                 double arrivalPosition, const std::vector<std::string>& lines) :
    GNEDemandElement(personParent, net, GLO_RIDE, GNE_TAG_RIDE_BUSSTOP_BUSSTOP,
{}, {}, {}, {fromBusStop, toBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myLines(lines) {
    // compute ride
    computePath();
}


GNERide::~GNERide() {}


GUIGLObjectPopupMenu*
GNERide::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    // build header
    buildPopupHeader(ret, app);
    // build menu command for center button and copy cursor position to clipboard
    buildCenterPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    // buld menu commands for names
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " name to clipboard").c_str(), nullptr, ret, MID_COPY_NAME);
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " typed name to clipboard").c_str(), nullptr, ret, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    // show option to open demand element dialog
    if (myTagProperty.hasDialog()) {
        new FXMenuCommand(ret, ("Open " + getTagStr() + " Dialog").c_str(), getIcon(), &parent, MID_OPEN_ADDITIONAL_DIALOG);
        new FXMenuSeparator(ret);
    }
    new FXMenuCommand(ret, ("Cursor position in view: " + toString(getPositionInView().x()) + "," + toString(getPositionInView().y())).c_str(), nullptr, nullptr, 0);
    return ret;
}


void
GNERide::writeDemandElement(OutputDevice& device) const {
    // open tag
    device.openTag(SUMO_TAG_RIDE);
    // check if we have to write "from" attributes
    if (getParentDemandElements().at(0)->getPreviousChildDemandElement(this) == nullptr) {
        // write "to" attributes depending of start and end
        if (myTagProperty.personPlanStartEdge()) {
            device.writeAttr(SUMO_ATTR_FROM, getParentEdges().front()->getID());
        } else if (myTagProperty.personPlanStartBusStop()) {
            device.writeAttr(SUMO_ATTR_FROM, getParentAdditionals().front()->getID());
        }
    }
    // write "to" attributes depending of start and end
    if (myTagProperty.personPlanStartEdge()) {
        device.writeAttr(SUMO_ATTR_TO, getParentEdges().back()->getID());
    } else if (myTagProperty.personPlanStartBusStop()) {
        device.writeAttr(SUMO_ATTR_BUS_STOP, getParentAdditionals().back()->getID());
    }
    // only write arrivalPos if is different of -1
    if (myArrivalPosition != -1) {
        device.writeAttr(SUMO_ATTR_ARRIVALPOS, myArrivalPosition);
    }
    // write lines
    if (myLines.empty()) {
        device.writeAttr(SUMO_ATTR_LINES, "ANY");
    } else {
        device.writeAttr(SUMO_ATTR_LINES, myLines);
    }
    // write parameters
    writeParams(device);
    // close tag
    device.closeTag();
}


bool
GNERide::isDemandElementValid() const {
    if ((getParentEdges().size() == 2) && (getParentEdges().at(0) == getParentEdges().at(1))) {
        // from and to are the same edges
        return true;
    } else if (getPath().size() > 0) {
        // if path edges isn't empty, then there is a valid route
        return true;
    } else {
        return false;
    }
}


std::string
GNERide::getDemandElementProblem() const {
    if (getParentEdges().size() == 0) {
        return ("A ride need at least one edge");
    } else {
        // check if exist at least a connection between every edge
        for (int i = 1; i < (int)getParentEdges().size(); i++) {
            if (myNet->getPathCalculator()->consecutiveEdgesConnected(getParentDemandElements().front()->getVClass(), getParentEdges().at((int)i - 1), getParentEdges().at(i)) == false) {
                return ("Edge '" + getParentEdges().at((int)i - 1)->getID() + "' and edge '" + getParentEdges().at(i)->getID() + "' aren't consecutives");
            }
        }
        // there is connections bewteen all edges, then all ok
        return "";
    }
}


void
GNERide::fixDemandElementProblem() {
    // currently the only solution is removing Ride
}


SUMOVehicleClass
GNERide::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNERide::getColor() const {
    return getParentDemandElements().front()->getColor();
}


void
GNERide::startGeometryMoving() {
    // only start geometry moving if arrival position isn't -1
    if (myArrivalPosition != -1) {
        // always save original position over view
        myRideMove.originalViewPosition = getPositionInView();
        // save arrival position
        myRideMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_ARRIVALPOS);
        // save current centering boundary
        myRideMove.movingGeometryBoundary = getCenteringBoundary();
    }
}


void
GNERide::endGeometryMoving() {
    // check that myArrivalPosition isn't -1 and endGeometryMoving was called only once
    if ((myArrivalPosition != -1) && myRideMove.movingGeometryBoundary.isInitialised()) {
        // reset myMovingGeometryBoundary
        myRideMove.movingGeometryBoundary.reset();
    }
}


void
GNERide::moveGeometry(const Position& offset) {
    // only move if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        // Calculate new position using old position
        Position newPosition = myRideMove.originalViewPosition;
        newPosition.add(offset);
        // filtern position using snap to active grid
        newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
        // obtain lane shape (to improve code legibility)
        const PositionVector& laneShape = getParentEdges().back()->getLanes().front()->getLaneShape();
        // calculate offset lane
        double offsetLane = laneShape.nearest_offset_to_point2D(newPosition, false) - laneShape.nearest_offset_to_point2D(myRideMove.originalViewPosition, false);
        // Update arrival Position
        myArrivalPosition = parse<double>(myRideMove.firstOriginalLanePosition) + offsetLane;
        // Update geometry
        updateGeometry();
    }
}


void
GNERide::commitGeometryMoving(GNEUndoList* undoList) {
    // only commit geometry moving if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        undoList->p_begin("arrivalPos of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_ARRIVALPOS, toString(myArrivalPosition), myRideMove.firstOriginalLanePosition));
        undoList->p_end();
    }
}


void
GNERide::updateGeometry() {
    // calculate person plan start and end positions
    GNEGeometry::ExtremeGeometry extremeGeometry = calculatePersonPlanLaneStartEndPos();
    // calculate edge geometry path using path
    GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getPath(), extremeGeometry);
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        i->updateGeometry();
    }
}


void
GNERide::computePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_EDGE) {
        calculatePathLanes(getVClass(), false,
                           getFirstAllowedVehicleLane(),
                           getLastAllowedVehicleLane(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_BUSSTOP) {
        calculatePathLanes(getVClass(), false,
                           getFirstAllowedVehicleLane(),
                           getParentAdditionals().back()->getParentLanes().front(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_EDGE) {
        calculatePathLanes(getVClass(), false,
                           getParentAdditionals().front()->getParentLanes().front(),
                           getLastAllowedVehicleLane(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_BUSSTOP) {
        calculatePathLanes(getVClass(), false,
                           getParentAdditionals().front()->getParentLanes().front(),
                           getParentAdditionals().back()->getParentLanes().front(),
                           {});
    }
    // update geometry
    updateGeometry();
}


void
GNERide::invalidatePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_EDGE) {
        resetPathLanes(getVClass(), false,
                       getFirstAllowedVehicleLane(),
                       getLastAllowedVehicleLane(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_BUSSTOP) {
        resetPathLanes(getVClass(), false,
                       getFirstAllowedVehicleLane(),
                       getParentAdditionals().back()->getParentLanes().front(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_EDGE) {
        resetPathLanes(getVClass(), false,
                       getParentAdditionals().front()->getParentLanes().front(),
                       getLastAllowedVehicleLane(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_BUSSTOP) {
        resetPathLanes(getVClass(), false,
                       getParentAdditionals().front()->getParentLanes().front(),
                       getParentAdditionals().back()->getParentLanes().front(),
                       {});
    }
    // update geometry
    updateGeometry();
}


Position
GNERide::getPositionInView() const {
    return Position();
}


std::string
GNERide::getParentName() const {
    return getParentDemandElements().front()->getID();
}


Boundary
GNERide::getCenteringBoundary() const {
    Boundary rideBoundary;
    // return the combination of all parent edges's boundaries
    for (const auto& i : getParentEdges()) {
        rideBoundary.add(i->getCenteringBoundary());
    }
    // check if is valid
    if (rideBoundary.isInitialised()) {
        return rideBoundary;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNERide::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNERide::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // Rides are drawn in drawPartialGL
}


void
GNERide::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    // draw person plan over lane
    drawPersonPlanPartialLane(s, lane, offsetFront, s.widthSettings.ride, s.colorSettings.ride);
}


void
GNERide::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const {
    // draw person plan over junction
    drawPersonPlanPartialJunction(s, fromLane, toLane, offsetFront, s.widthSettings.ride, s.colorSettings.ride);
}


std::string
GNERide::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_ID:
            return getParentDemandElements().front()->getID();
        case SUMO_ATTR_FROM:
            return getParentEdges().front()->getID();
        case SUMO_ATTR_TO:
            return getParentEdges().back()->getID();
        case GNE_ATTR_FROM_BUSSTOP:
            return getParentAdditionals().front()->getID();
        case GNE_ATTR_TO_BUSSTOP:
            return getParentAdditionals().back()->getID();
        // specific person plan attributes
        case SUMO_ATTR_LINES:
            return joinToString(myLines, " ");
        case SUMO_ATTR_ARRIVALPOS:
            if (myArrivalPosition == -1) {
                return "";
            } else {
                return toString(myArrivalPosition);
            }
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        case GNE_ATTR_PARENT:
            return getParentDemandElements().front()->getID();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNERide::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ARRIVALPOS:
            if (myArrivalPosition != -1) {
                return myArrivalPosition;
            } else {
                return (getLastAllowedVehicleLane()->getLaneShape().length() - POSITION_EPS);
            }
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNERide::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
        // specific person plan attributes
        case SUMO_ATTR_LINES:
        case SUMO_ATTR_ARRIVALPOS:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERide::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveEdge(value, false) != nullptr);
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
            return (myNet->retrieveAdditional(SUMO_TAG_BUS_STOP, value, false) != nullptr);
        // specific person plan attributes
        case SUMO_ATTR_LINES:
            return canParse<std::vector<std::string> >(value);
        case SUMO_ATTR_ARRIVALPOS:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                const double parsedValue = canParse<double>(value);
                if ((parsedValue < 0) || (parsedValue > getLastAllowedVehicleLane()->getLaneShape().length())) {
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNERide::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNERide::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNERide::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNERide::getPopUpID() const {
    return getTagStr();
}


std::string
GNERide::getHierarchyName() const {
    if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_EDGE) {
        return "ride: " + getParentEdges().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_BUSSTOP) {
        return "ride: " + getParentEdges().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_EDGE) {
        return "ride: " + getParentAdditionals().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_BUSSTOP) {
        return "ride: " + getParentAdditionals().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else {
        throw ("Invalid ride tag");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNERide::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
            // change first edge
            replaceFirstParentEdge(value);
            // compute person trip
            computePath();
            break;
        case SUMO_ATTR_TO:
            // change last edge
            replaceLastParentEdge(value);
            // compute person trip
            computePath();
            break;
        case GNE_ATTR_FROM_BUSSTOP:
            replaceAdditionalParent(SUMO_TAG_BUS_STOP, value, 0);
            // compute person trip
            computePath();
            break;
        case GNE_ATTR_TO_BUSSTOP:
            // -> check this
            if (getParentAdditionals().size() > 1) {
                replaceAdditionalParent(SUMO_TAG_BUS_STOP, value, 1);
            } else {
                replaceAdditionalParent(SUMO_TAG_BUS_STOP, value, 0);
            }
            // compute person trip
            computePath();
            break;
        // specific person plan attributes
        case SUMO_ATTR_LINES:
            myLines = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
            break;
        case SUMO_ATTR_ARRIVALPOS:
            if (value.empty()) {
                myArrivalPosition = -1;
            } else {
                myArrivalPosition = parse<double>(value);
            }
            updateGeometry();
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
GNERide::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
