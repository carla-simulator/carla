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
/// @file    GNEWalk.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// A class for visualizing walks in Netedit
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>

#include "GNEWalk.h"
#include "GNERoute.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEWalk::GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge, double arrivalPosition) :
    GNEDemandElement(personParent, net, GLO_WALK, GNE_TAG_WALK_EDGE_EDGE,
{}, {fromEdge, toEdge}, {}, {}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition) {
    // compute walk
    computePath();
}


GNEWalk::GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEAdditional* toBusStop, double arrivalPosition) :
    GNEDemandElement(personParent, net, GLO_WALK, GNE_TAG_WALK_EDGE_BUSSTOP,
{}, {fromEdge}, {}, {toBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition) {
    // compute walk
    computePath();
}

GNEWalk::GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEEdge* toEdge, double arrivalPosition) :
    GNEDemandElement(personParent, net, GLO_WALK, GNE_TAG_WALK_BUSSTOP_EDGE,
{}, {toEdge}, {}, {fromBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition) {
    // compute walk
    computePath();
}


GNEWalk::GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEAdditional* toBusStop, double arrivalPosition) :
    GNEDemandElement(personParent, net, GLO_WALK, GNE_TAG_WALK_BUSSTOP_BUSSTOP,
{}, {}, {}, {fromBusStop, toBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition) {
    // compute walk
    computePath();
}


GNEWalk::GNEWalk(GNENet* net, GNEDemandElement* personParent, std::vector<GNEEdge*> edges, double arrivalPosition) :
    GNEDemandElement(personParent, net, GLO_WALK, GNE_TAG_WALK_EDGES,
{}, {edges}, {}, {}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition) {
    // compute walk
    computePath();
}


GNEWalk::GNEWalk(GNENet* net, GNEDemandElement* personParent, GNEDemandElement* route, double arrivalPosition) :
    GNEDemandElement(personParent, net, GLO_WALK, GNE_TAG_WALK_ROUTE,
{}, {}, {}, {}, {}, {}, {personParent, route}, {}),
myArrivalPosition(arrivalPosition) {
    // compute walk
    computePath();
}


GNEWalk::~GNEWalk() {}


GUIGLObjectPopupMenu*
GNEWalk::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
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
GNEWalk::writeDemandElement(OutputDevice& device) const {
    // open tag
    device.openTag(SUMO_TAG_WALK);
    // write attributes depending  of walk type
    if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        device.writeAttr(SUMO_ATTR_ROUTE, getParentDemandElements().at(1)->getID());
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGES) {
        device.writeAttr(SUMO_ATTR_EDGES, parseIDs(getParentEdges()));
    } else {
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
    }
    // only write arrivalPos if is different of -1
    if (myArrivalPosition != -1) {
        device.writeAttr(SUMO_ATTR_ARRIVALPOS, myArrivalPosition);
    }
    // write parameters
    writeParams(device);
    // close tag
    device.closeTag();
}


bool
GNEWalk::isDemandElementValid() const {
    if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        // check if route parent is valid
        return getParentDemandElements().at(1)->isDemandElementValid();
    } else if (getParentEdges().size() == 2) {
        if (getParentEdges().at(0) == getParentEdges().at(1)) {
            // from and to are the same edges, then path is valid
            return true;
        } else {
            // check if exist a route between parent edges
            return (myNet->getPathCalculator()->calculatePath(getParentDemandElements().at(0)->getVClass(), getParentEdges()).size() > 0);
        }
    } else if (getPath().size() > 0) {
        // if path edges isn't empty, then there is a valid route
        return true;
    } else {
        return false;
    }
}


std::string
GNEWalk::getDemandElementProblem() const {
    if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        return "";
    } else if (getParentEdges().size() == 0) {
        return ("A walk need at least one edge");
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
GNEWalk::fixDemandElementProblem() {
    // currently the only solution is removing Walk
}


SUMOVehicleClass
GNEWalk::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNEWalk::getColor() const {
    return getParentDemandElements().front()->getColor();
}


void
GNEWalk::startGeometryMoving() {
    // only start geometry moving if arrival position isn't -1
    if (myArrivalPosition != -1) {
        // always save original position over view
        myWalkMove.originalViewPosition = getPositionInView();
        // save arrival position
        myWalkMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_ARRIVALPOS);
        // save current centering boundary
        myWalkMove.movingGeometryBoundary = getCenteringBoundary();
    }
}


void
GNEWalk::endGeometryMoving() {
    // check that myArrivalPosition isn't -1 and endGeometryMoving was called only once
    if ((myArrivalPosition != -1) && myWalkMove.movingGeometryBoundary.isInitialised()) {
        // reset myMovingGeometryBoundary
        myWalkMove.movingGeometryBoundary.reset();
    }
}


void
GNEWalk::moveGeometry(const Position& offset) {
    // only move if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        // Calculate new position using old position
        Position newPosition = myWalkMove.originalViewPosition;
        newPosition.add(offset);
        // filtern position using snap to active grid
        newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
        // obtain lane shape (to improve code legibility)
        const PositionVector& laneShape = getParentEdges().back()->getLanes().front()->getLaneShape();
        // calculate offset lane
        double offsetLane = laneShape.nearest_offset_to_point2D(newPosition, false) - laneShape.nearest_offset_to_point2D(myWalkMove.originalViewPosition, false);
        // Update arrival Position
        myArrivalPosition = parse<double>(myWalkMove.firstOriginalLanePosition) + offsetLane;
        // Update geometry
        updateGeometry();
    }
}


void
GNEWalk::commitGeometryMoving(GNEUndoList* undoList) {
    // only commit geometry moving if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        undoList->p_begin("arrivalPos of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_ARRIVALPOS, toString(myArrivalPosition), myWalkMove.firstOriginalLanePosition));
        undoList->p_end();
    }
}


void
GNEWalk::updateGeometry() {
    // calculate person plan start and end positions
    GNEGeometry::ExtremeGeometry extremeGeometry = calculatePersonPlanLaneStartEndPos();
    // calculate geometry path depending if is a Walk over route
    if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        // calculate edge geometry path using parent route
        GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getParentDemandElements().at(1)->getPath(), extremeGeometry);
    } else {
        // calculate edge geometry path using path
        GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getPath(), extremeGeometry);
    }
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        i->updateGeometry();
    }
}


void
GNEWalk::computePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_WALK_EDGE_EDGE) {
        calculatePathLanes(getVClass(), true,
                           getFirstAllowedVehicleLane(),
                           getLastAllowedVehicleLane(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGE_BUSSTOP) {
        calculatePathLanes(getVClass(), true,
                           getFirstAllowedVehicleLane(),
                           getParentAdditionals().back()->getParentLanes().front(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_BUSSTOP_EDGE) {
        calculatePathLanes(getVClass(), true,
                           getParentAdditionals().front()->getParentLanes().front(),
                           getLastAllowedVehicleLane(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_BUSSTOP_BUSSTOP) {
        calculatePathLanes(getVClass(), true,
                           getParentAdditionals().front()->getParentLanes().front(),
                           getParentAdditionals().back()->getParentLanes().front(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGES) {
        // calculate consecutive path using parent edges
        calculateConsecutivePathLanes(getVClass(), true, getParentEdges());
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        calculatePathLanes(getVClass(), true,
                           getFirstAllowedVehicleLane(),
                           getLastAllowedVehicleLane(),
                           getParentDemandElements().back()->getParentEdges());
    }
    // update geometry
    updateGeometry();
}


void
GNEWalk::invalidatePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_WALK_EDGE_EDGE) {
        resetPathLanes(getVClass(), true,
                       getFirstAllowedVehicleLane(),
                       getLastAllowedVehicleLane(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGE_BUSSTOP) {
        resetPathLanes(getVClass(), true,
                       getFirstAllowedVehicleLane(),
                       getParentAdditionals().back()->getParentLanes().front(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_BUSSTOP_EDGE) {
        resetPathLanes(getVClass(), true,
                       getParentAdditionals().front()->getParentLanes().front(),
                       getLastAllowedVehicleLane(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_BUSSTOP_BUSSTOP) {
        resetPathLanes(getVClass(), true,
                       getParentAdditionals().front()->getParentLanes().front(),
                       getParentAdditionals().back()->getParentLanes().front(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGES) {
        // due walk edges don't need to calculate a dijkstra path, just calculate consecutive path lanes again
        calculateConsecutivePathLanes(getVClass(), true, getParentEdges());
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        resetPathLanes(getVClass(), true,
                       getFirstAllowedVehicleLane(),
                       getLastAllowedVehicleLane(),
                       getParentDemandElements().back()->getParentEdges());
    }
    // update geometry
    updateGeometry();
}


Position
GNEWalk::getPositionInView() const {
    return Position();
}


std::string
GNEWalk::getParentName() const {
    return getParentDemandElements().front()->getID();
}


Boundary
GNEWalk::getCenteringBoundary() const {
    Boundary walkBoundary;
    // return the combination of all parent edges's boundaries
    for (const auto& i : getParentEdges()) {
        walkBoundary.add(i->getCenteringBoundary());
    }
    // check if is valid
    if (walkBoundary.isInitialised()) {
        return walkBoundary;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNEWalk::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* originalElement, const GNENetworkElement* newElement, GNEUndoList* undoList) {
    // only split geometry of WalkEdges
    if (myTagProperty.getTag() == GNE_TAG_WALK_EDGES) {
        // obtain new list of walk edges
        std::string newWalkEdges = getNewListOfParents(originalElement, newElement);
        // update walk edges
        if (newWalkEdges.size() > 0) {
            setAttribute(SUMO_ATTR_EDGES, newWalkEdges, undoList);
        }
    }
}


void
GNEWalk::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // Walks are drawn in drawPartialGL
}


void
GNEWalk::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    // draw person plan over lane
    drawPersonPlanPartialLane(s, lane, offsetFront, s.widthSettings.walk, s.colorSettings.walk);
}


void
GNEWalk::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const {
    // draw person plan over junction
    drawPersonPlanPartialJunction(s, fromLane, toLane, offsetFront, s.widthSettings.walk, s.colorSettings.walk);
}


std::string
GNEWalk::getAttribute(SumoXMLAttr key) const {
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
        case SUMO_ATTR_EDGES:
            return parseIDs(getParentEdges());
        case SUMO_ATTR_ROUTE:
            return getParentDemandElements().at(1)->getID();
        // specific person plan attributes
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
GNEWalk::getAttributeDouble(SumoXMLAttr key) const {
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
GNEWalk::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
        case SUMO_ATTR_EDGES:
        case SUMO_ATTR_ROUTE:
        // specific person plan attributes
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
GNEWalk::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveEdge(value, false) != nullptr);
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
            return (myNet->retrieveAdditional(SUMO_TAG_BUS_STOP, value, false) != nullptr);
        case SUMO_ATTR_EDGES:
            if (canParse<std::vector<GNEEdge*> >(myNet, value, false)) {
                // all edges exist, then check if compounds a valid route
                return GNERoute::isRouteValid(parse<std::vector<GNEEdge*> >(myNet, value)).empty();
            } else {
                return false;
            }
        case SUMO_ATTR_ROUTE:
            return (myNet->retrieveDemandElement(SUMO_TAG_ROUTE, value, false) != nullptr);
        // specific person plan attributes
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
GNEWalk::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNEWalk::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNEWalk::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNEWalk::getPopUpID() const {
    return getTagStr();
}


std::string
GNEWalk::getHierarchyName() const {
    if (myTagProperty.getTag() == GNE_TAG_WALK_EDGE_EDGE) {
        return "walk: " + getParentEdges().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGE_BUSSTOP) {
        return "walk: " + getParentEdges().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_BUSSTOP_EDGE) {
        return "walk: " + getParentAdditionals().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_BUSSTOP_BUSSTOP) {
        return "walk: " + getParentAdditionals().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_EDGES) {
        return "walk: " + getParentEdges().front()->getID() + " ... " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        return "walk: " + getParentDemandElements().at(1)->getID();
    } else {
        throw ("Invalid walk tag");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEWalk::setAttribute(SumoXMLAttr key, const std::string& value) {
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
        case SUMO_ATTR_EDGES:
            replaceDemandParentEdges(value);
            updateGeometry();
            break;
        case SUMO_ATTR_ROUTE:
            replaceDemandElementParent(SUMO_TAG_ROUTE, value, 1);
            updateGeometry();
            break;
        // specific person plan attributes
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
GNEWalk::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
