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
/// @file    GNEPersonTrip.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2019
///
// A class for visualizing person trips in Netedit
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>

#include "GNEPersonTrip.h"
#include "GNERouteHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEEdge* toEdge,
                             double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(personParent, net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_EDGE_EDGE,
{}, {fromEdge, toEdge}, {}, {}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myVTypes(types),
myModes(modes) {
    // compute person trip
    computePath();
}


GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEEdge* fromEdge, GNEAdditional* toBusStop,
                             double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(personParent, net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_EDGE_BUSSTOP,
{}, {fromEdge}, {}, {toBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myVTypes(types),
myModes(modes) {
    // compute person trip
    computePath();
}

GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEEdge* toEdge,
                             double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(personParent, net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_BUSSTOP_EDGE,
{}, {toEdge}, {}, {fromBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myVTypes(types),
myModes(modes) {
    // compute person trip
    computePath();
}


GNEPersonTrip::GNEPersonTrip(GNENet* net, GNEDemandElement* personParent, GNEAdditional* fromBusStop, GNEAdditional* toBusStop,
                             double arrivalPosition, const std::vector<std::string>& types, const std::vector<std::string>& modes) :
    GNEDemandElement(personParent, net, GLO_PERSONTRIP, GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP,
{}, {}, {}, {fromBusStop, toBusStop}, {}, {}, {personParent}, {}),
myArrivalPosition(arrivalPosition),
myVTypes(types),
myModes(modes) {
    // compute person trip
    computePath();
}


GNEPersonTrip::~GNEPersonTrip() {}


GUIGLObjectPopupMenu*
GNEPersonTrip::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
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
GNEPersonTrip::writeDemandElement(OutputDevice& device) const {
    // open tag
    device.openTag(SUMO_TAG_PERSONTRIP);
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
    // write modes
    if (myModes.size() > 0) {
        device.writeAttr(SUMO_ATTR_MODES, myModes);
    }
    // write vTypes
    if (myVTypes.size() > 0) {
        device.writeAttr(SUMO_ATTR_VTYPES, myVTypes);
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
GNEPersonTrip::isDemandElementValid() const {
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
GNEPersonTrip::getDemandElementProblem() const {
    if (getParentEdges().size() == 0) {
        return ("A person trip need at least one edge");
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
GNEPersonTrip::fixDemandElementProblem() {
    // currently the only solution is removing PersonTrip
}


SUMOVehicleClass
GNEPersonTrip::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNEPersonTrip::getColor() const {
    return getParentDemandElements().front()->getColor();
}


void
GNEPersonTrip::startGeometryMoving() {
    // only start geometry moving if arrival position isn't -1
    if (myArrivalPosition != -1) {
        // always save original position over view
        myPersonTripMove.originalViewPosition = getPositionInView();
        // save arrival position
        myPersonTripMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_ARRIVALPOS);
        // save current centering boundary
        myPersonTripMove.movingGeometryBoundary = getCenteringBoundary();
    }
}


void
GNEPersonTrip::endGeometryMoving() {
    // check that myArrivalPosition isn't -1 and endGeometryMoving was called only once
    if ((myArrivalPosition != -1) && myPersonTripMove.movingGeometryBoundary.isInitialised()) {
        // reset myMovingGeometryBoundary
        myPersonTripMove.movingGeometryBoundary.reset();
    }
}


void
GNEPersonTrip::moveGeometry(const Position& offset) {
    // only move if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        // Calculate new position using old position
        Position newPosition = myPersonTripMove.originalViewPosition;
        newPosition.add(offset);
        // filtern position using snap to active grid
        newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
        // obtain lane shape (to improve code legibility)
        const PositionVector& laneShape = getParentEdges().back()->getLanes().front()->getLaneShape();
        // calculate offset lane
        double offsetLane = laneShape.nearest_offset_to_point2D(newPosition, false) - laneShape.nearest_offset_to_point2D(myPersonTripMove.originalViewPosition, false);
        // Update arrival Position
        myArrivalPosition = parse<double>(myPersonTripMove.firstOriginalLanePosition) + offsetLane;
        // Update geometry
        updateGeometry();
    }
}


void
GNEPersonTrip::commitGeometryMoving(GNEUndoList* undoList) {
    // only commit geometry moving if myArrivalPosition isn't -1
    if (myArrivalPosition != -1) {
        undoList->p_begin("arrivalPos of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_ARRIVALPOS, toString(myArrivalPosition), myPersonTripMove.firstOriginalLanePosition));
        undoList->p_end();
    }
}


void
GNEPersonTrip::updateGeometry() {
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
GNEPersonTrip::computePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_EDGE) {
        calculatePathLanes(getVClass(), true,
                           getFirstAllowedVehicleLane(),
                           getLastAllowedVehicleLane(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) {
        calculatePathLanes(getVClass(), true,
                           getFirstAllowedVehicleLane(),
                           getParentAdditionals().back()->getParentLanes().front(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_EDGE) {
        calculatePathLanes(getVClass(), true,
                           getParentAdditionals().front()->getParentLanes().front(),
                           getLastAllowedVehicleLane(),
                           {});
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP) {
        calculatePathLanes(getVClass(), true,
                           getParentAdditionals().front()->getParentLanes().front(),
                           getParentAdditionals().back()->getParentLanes().front(),
                           {});
    }
    // update geometry
    updateGeometry();
}


void
GNEPersonTrip::invalidatePath() {
    // update lanes depending of walk tag
    if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_EDGE) {
        resetPathLanes(getVClass(), true,
                       getFirstAllowedVehicleLane(),
                       getLastAllowedVehicleLane(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_EDGE_BUSSTOP) {
        resetPathLanes(getVClass(), true,
                       getFirstAllowedVehicleLane(),
                       getParentAdditionals().back()->getParentLanes().front(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_EDGE) {
        resetPathLanes(getVClass(), true,
                       getParentAdditionals().front()->getParentLanes().front(),
                       getLastAllowedVehicleLane(),
                       {});
    } else if (myTagProperty.getTag() == GNE_TAG_RIDE_BUSSTOP_BUSSTOP) {
        resetPathLanes(getVClass(), true,
                       getParentAdditionals().front()->getParentLanes().front(),
                       getParentAdditionals().back()->getParentLanes().front(),
                       {});
    }
    // update geometry
    updateGeometry();
}


Position
GNEPersonTrip::getPositionInView() const {
    return Position();
}


std::string
GNEPersonTrip::getParentName() const {
    return getParentDemandElements().front()->getID();
}


Boundary
GNEPersonTrip::getCenteringBoundary() const {
    Boundary personTripBoundary;
    // return the combination of all parent edges's boundaries
    for (const auto& i : getParentEdges()) {
        personTripBoundary.add(i->getCenteringBoundary());
    }
    // check if is valid
    if (personTripBoundary.isInitialised()) {
        return personTripBoundary;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNEPersonTrip::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEPersonTrip::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // PersonTrips are drawn in drawPartialGL
}


void
GNEPersonTrip::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    // draw person plan over lane
    drawPersonPlanPartialLane(s, lane, offsetFront, s.widthSettings.personTrip, s.colorSettings.personTrip);
}


void
GNEPersonTrip::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const {
    // draw person plan over junction
    drawPersonPlanPartialJunction(s, fromLane, toLane, offsetFront, s.widthSettings.personTrip, s.colorSettings.personTrip);
}


std::string
GNEPersonTrip::getAttribute(SumoXMLAttr key) const {
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
        case SUMO_ATTR_MODES:
            return joinToString(myModes, " ");
        case SUMO_ATTR_VTYPES:
            return joinToString(myVTypes, " ");
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
GNEPersonTrip::getAttributeDouble(SumoXMLAttr key) const {
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
GNEPersonTrip::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
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
        case SUMO_ATTR_MODES:
        case SUMO_ATTR_VTYPES:
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
GNEPersonTrip::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        // Common person plan attributes
        case SUMO_ATTR_FROM:
        case SUMO_ATTR_TO:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveEdge(value, false) != nullptr);
        case GNE_ATTR_FROM_BUSSTOP:
        case GNE_ATTR_TO_BUSSTOP:
            return (myNet->retrieveAdditional(SUMO_TAG_BUS_STOP, value, false) != nullptr);
        // specific person plan attributes
        case SUMO_ATTR_MODES: {
            SVCPermissions dummyModeSet;
            std::string dummyError;
            return SUMOVehicleParameter::parsePersonModes(value, myTagProperty.getTagStr(), getID(), dummyModeSet, dummyError);
        }
        case SUMO_ATTR_VTYPES:
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
GNEPersonTrip::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNEPersonTrip::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNEPersonTrip::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNEPersonTrip::getPopUpID() const {
    return getTagStr();
}


std::string
GNEPersonTrip::getHierarchyName() const {
    if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_EDGE) {
        return "personTrip: " + getParentEdges().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_EDGE_BUSSTOP) {
        return "personTrip: " + getParentEdges().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_EDGE) {
        return "personTrip: " + getParentAdditionals().front()->getID() + " -> " + getParentEdges().back()->getID();
    } else if (myTagProperty.getTag() == GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP) {
        return "personTrip: " + getParentAdditionals().front()->getID() + " -> " + getParentAdditionals().back()->getID();
    } else {
        throw ("Invalid personTrip tag");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEPersonTrip::setAttribute(SumoXMLAttr key, const std::string& value) {
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
        case SUMO_ATTR_MODES:
            myModes = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
            break;
        case SUMO_ATTR_VTYPES:
            myVTypes = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
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
GNEPersonTrip::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
