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
/// @file    GNEDemandElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Dec 2018
///
// A abstract class for demand elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>

#include "GNEDemandElement.h"


// ===========================================================================
// static defintions
// ===========================================================================
const double GNEDemandElement::myPersonPlanArrivalPositionDiameter = SUMO_const_halfLaneWidth;

// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEDemandElement - methods
// ---------------------------------------------------------------------------

GNEDemandElement::GNEDemandElement(const std::string& id, GNENet* net, GUIGlObjectType type, SumoXMLTag tag,
                                   const std::vector<GNEJunction*>& junctionParents,
                                   const std::vector<GNEEdge*>& edgeParents,
                                   const std::vector<GNELane*>& laneParents,
                                   const std::vector<GNEAdditional*>& additionalParents,
                                   const std::vector<GNEShape*>& shapeParents,
                                   const std::vector<GNETAZElement*>& TAZElementParents,
                                   const std::vector<GNEDemandElement*>& demandElementParents,
                                   const std::vector<GNEGenericData*>& genericDataParents) :
    GUIGlObject(type, id),
    GNEHierarchicalElement(net, tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    GNEPathElements(this),
    myStackedLabelNumber(0) {
}


GNEDemandElement::GNEDemandElement(GNEDemandElement* demandElementParent, GNENet* net, GUIGlObjectType type, SumoXMLTag tag,
                                   const std::vector<GNEJunction*>& junctionParents,
                                   const std::vector<GNEEdge*>& edgeParents,
                                   const std::vector<GNELane*>& laneParents,
                                   const std::vector<GNEAdditional*>& additionalParents,
                                   const std::vector<GNEShape*>& shapeParents,
                                   const std::vector<GNETAZElement*>& TAZElementParents,
                                   const std::vector<GNEDemandElement*>& demandElementParents,
                                   const std::vector<GNEGenericData*>& genericDataParents) :
    GUIGlObject(type, demandElementParent->getID()),
    GNEHierarchicalElement(net, tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    GNEPathElements(this),
    myStackedLabelNumber(0) {
}


GNEDemandElement::~GNEDemandElement() {}


const std::string&
GNEDemandElement::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNEDemandElement::getGUIGlObject() {
    return this;
}


const GNEGeometry::Geometry&
GNEDemandElement::getDemandElementGeometry() {
    return myDemandElementGeometry;
}


const GNEGeometry::SegmentGeometry&
GNEDemandElement::getDemandElementSegmentGeometry() const {
    return myDemandElementSegmentGeometry;
}


GNEDemandElement*
GNEDemandElement::getPreviousChildDemandElement(const GNEDemandElement* demandElement) const {
    // find child demand element
    auto it = std::find(getChildDemandElements().begin(), getChildDemandElements().end(), demandElement);
    // return element or null depending of iterator
    if (it == getChildDemandElements().end()) {
        return nullptr;
    } else if (it == getChildDemandElements().begin()) {
        return nullptr;
    } else {
        return *(it - 1);
    }
}


GNEDemandElement*
GNEDemandElement::getNextChildDemandElement(const GNEDemandElement* demandElement) const {
    // find child demand element
    auto it = std::find(getChildDemandElements().begin(), getChildDemandElements().end(), demandElement);
    // return element or null depending of iterator
    if (it == getChildDemandElements().end()) {
        return nullptr;
    } else if (it == (getChildDemandElements().end() - 1)) {
        return nullptr;
    } else {
        return *(it + 1);
    }
}


std::vector<GNEEdge*>
GNEDemandElement::getViaEdges() const {
    std::vector<GNEEdge*> middleEdges;
    // there are only middle edges if there is more than two edges
    if (getParentEdges().size() > 2) {
        // reserve middleEdges
        middleEdges.reserve(getParentEdges().size() - 2);
        // iterate over second and previous last parent edge
        for (auto i = (getParentEdges().begin() + 1); i != (getParentEdges().end() - 1); i++) {
            middleEdges.push_back(*i);
        }
    }
    return middleEdges;
}


void
GNEDemandElement::updateDemandElementGeometry(const GNELane* lane, const double posOverLane) {
    myDemandElementGeometry.updateGeometry(lane, posOverLane);
}


void
GNEDemandElement::updateDemandElementStackLabel(const int stack) {
    myStackedLabelNumber = stack;
}


void
GNEDemandElement::updateDemandElementSpreadGeometry(const GNELane* lane, const double posOverLane) {
    mySpreadGeometry.updateGeometry(lane, posOverLane);
}


void
GNEDemandElement::updatePartialGeometry(const GNELane* lane) {
    // declare extreme geometry
    GNEGeometry::ExtremeGeometry extremeGeometry;
    // special case for stops
    if (myTagProperty.isVehicle()) {
        // use depart/arrival positions as lane extremes
        extremeGeometry.laneStartPosition = getAttributeDouble(SUMO_ATTR_DEPARTPOS);
        extremeGeometry.laneEndPosition = getAttributeDouble(SUMO_ATTR_ARRIVALPOS);
    } else if (myTagProperty.isPersonPlan()) {
        // calculate person plan start and end positions
        extremeGeometry = calculatePersonPlanLaneStartEndPos();
    }
    // update geometry path for the given lane
    GNEGeometry::updateGeometricPath(myDemandElementSegmentGeometry, lane, extremeGeometry);
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        i->updatePartialGeometry(lane);
    }
}


bool
GNEDemandElement::isDemandElementValid() const {
    return true;
}


std::string
GNEDemandElement::getDemandElementProblem() const {
    return "";
}


void
GNEDemandElement::fixDemandElementProblem() {
    throw InvalidArgument(getTagStr() + " cannot fix any problem");
}


void
GNEDemandElement::openDemandElementDialog() {
    throw InvalidArgument(getTagStr() + " doesn't have an demand element dialog");
}


std::string
GNEDemandElement::getBegin() const {
    throw InvalidArgument(getTagStr() + " doesn't have an begin time");
}


GUIGLObjectPopupMenu*
GNEDemandElement::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
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


GUIParameterTableWindow*
GNEDemandElement::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    // Create table
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // Iterate over attributes
    for (const auto& i : myTagProperty) {
        // Add attribute and set it dynamic if aren't unique
        if (i.isUnique()) {
            ret->mkItem(i.getAttrStr().c_str(), false, getAttribute(i.getAttr()));
        } else {
            ret->mkItem(i.getAttrStr().c_str(), true, getAttribute(i.getAttr()));
        }
    }
    // close building
    ret->closeBuilding();
    return ret;
}


bool
GNEDemandElement::isValidDemandElementID(const std::string& newID) const {
    if (SUMOXMLDefinitions::isValidVehicleID(newID) && (myNet->retrieveDemandElement(myTagProperty.getTag(), newID, false) == nullptr)) {
        return true;
    } else {
        return false;
    }
}


GNELane*
GNEDemandElement::getFirstAllowedVehicleLane() const {
    // first check if current demand element has parent edges
    if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        // use route edges
        return getParentDemandElements().at(1)->getParentEdges().front()->getLaneByAllowedVClass(getVClass());
    } else if ((myTagProperty.getTag() == SUMO_TAG_VEHICLE) || (myTagProperty.getTag() == GNE_TAG_FLOW_ROUTE)) {
        // check if vehicle use a embedded route
        if (getParentDemandElements().size() == 2) {
            return getParentDemandElements().at(1)->getParentEdges().front()->getLaneByAllowedVClass(getVClass());
        } else {
            return nullptr;
        }
    } else if ((myTagProperty.getTag() == GNE_TAG_VEHICLE_WITHROUTE) || (myTagProperty.getTag() == GNE_TAG_FLOW_WITHROUTE)) {
        if (getChildDemandElements().size() > 0) {
            return getChildDemandElements().front()->getParentEdges().front()->getLaneByAllowedVClass(getVClass());
        } else {
            return nullptr;
        }
    } else if (myTagProperty.personPlanStartBusStop()) {
        return getParentAdditionals().front()->getParentLanes().front();
    } else if (getParentEdges().size() > 0) {
        if (myTagProperty.hasAttribute(SUMO_ATTR_DEPARTLANE)) {
            // obtain Lane depending of attribute "departLane"
            std::string departLane = getAttribute(SUMO_ATTR_DEPARTLANE);
            //  check depart lane
            if ((departLane == "random") || (departLane == "free") || (departLane == "allowed") || (departLane == "best") || (departLane == "first")) {
                return getParentEdges().front()->getLaneByAllowedVClass(getVClass());
            }
            // obtain index
            const int departLaneIndex = parse<int>(getAttribute(SUMO_ATTR_DEPARTLANE));
            // if index is correct, return lane. In other case, return nullptr;
            if ((departLaneIndex >= 0) && (departLaneIndex < getParentEdges().front()->getNBEdge()->getNumLanes())) {
                return getParentEdges().front()->getLanes().at(departLaneIndex);
            } else {
                return nullptr;
            }
        } else if (myTagProperty.isRide()) {
            // special case for rides
            return getParentEdges().front()->getLaneByDisallowedVClass(getVClass());
        } else {
            // in other case, always return the first allowed
            return getParentEdges().front()->getLaneByAllowedVClass(getVClass());
        }
    } else {
        return nullptr;
    }
}


GNELane*
GNEDemandElement::getLastAllowedVehicleLane() const {
    // first check if current demand element has parent edges
    if (myTagProperty.getTag() == GNE_TAG_WALK_ROUTE) {
        // use route edges
        return getParentDemandElements().at(1)->getParentEdges().back()->getLaneByAllowedVClass(getVClass());
    } else if (myTagProperty.personPlanEndBusStop()) {
        // return busStop lane
        return getParentAdditionals().back()->getParentLanes().front();
    } else if (getParentEdges().size() > 0) {
        if (myTagProperty.hasAttribute(SUMO_ATTR_ARRIVALLANE)) {
            // obtain Lane depending of attribute "arrivalLane"
            std::string arrivalLane = getAttribute(SUMO_ATTR_ARRIVALLANE);
            //  check depart lane
            if (arrivalLane == "current") {
                return getParentEdges().back()->getLaneByAllowedVClass(getVClass());
            }
            // obtain index
            const int arrivalLaneIndex = parse<int>(getAttribute(SUMO_ATTR_ARRIVALLANE));
            // if index is correct, return lane. In other case, return nullptr;
            if ((arrivalLaneIndex >= 0) && (arrivalLaneIndex < getParentEdges().back()->getNBEdge()->getNumLanes())) {
                return getParentEdges().back()->getLanes().at(arrivalLaneIndex);
            } else {
                return nullptr;
            }
        } else if (myTagProperty.isRide()) {
            // special case for rides
            return getParentEdges().back()->getLaneByDisallowedVClass(getVClass());
        } else {
            // in other case, always return the first allowed
            return getParentEdges().back()->getLaneByAllowedVClass(getVClass());
        }
    } else {
        return nullptr;
    }
}


const GNEEdge*
GNEDemandElement::getFirstPersonPlanEdge() const {
    // set specific mode depending of tag
    switch (myTagProperty.getTag()) {
        // route
        case GNE_TAG_WALK_ROUTE:
            return getParentDemandElements().at(1)->getParentEdges().front();
        // edges
        case GNE_TAG_WALK_EDGES:
        // edge->edge
        case GNE_TAG_PERSONTRIP_EDGE_EDGE:
        case GNE_TAG_WALK_EDGE_EDGE:
        case GNE_TAG_RIDE_EDGE_EDGE:
        // edge->busStop
        case GNE_TAG_PERSONTRIP_EDGE_BUSSTOP:
        case GNE_TAG_WALK_EDGE_BUSSTOP:
        case GNE_TAG_RIDE_EDGE_BUSSTOP:
            return getParentEdges().front();
        // busStop->edge
        case GNE_TAG_PERSONTRIP_BUSSTOP_EDGE:
        case GNE_TAG_WALK_BUSSTOP_EDGE:
        case GNE_TAG_RIDE_BUSSTOP_EDGE:
        // busStop->busStop
        case GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP:
        case GNE_TAG_WALK_BUSSTOP_BUSSTOP:
        case GNE_TAG_RIDE_BUSSTOP_BUSSTOP:
            return getParentAdditionals().front()->getParentLanes().front()->getParentEdge();
        // stops
        case GNE_TAG_PERSONSTOP_BUSSTOP:
            return getParentAdditionals().front()->getParentLanes().front()->getParentEdge();
        case GNE_TAG_PERSONSTOP_EDGE:
            return getParentLanes().front()->getParentEdge();
        default:
            return nullptr;
    }
}


GNEGeometry::ExtremeGeometry
GNEDemandElement::calculatePersonPlanLaneStartEndPos() const {
    // declare extreme geometry
    GNEGeometry::ExtremeGeometry extremeGeometry;
    // get previous person Plan
    const GNEDemandElement* previousPersonPlan = getParentDemandElements().at(0)->getPreviousChildDemandElement(this);
    double endLanePosPreviousLane = 0;
    // obtain previous plan
    if (previousPersonPlan) {
        if (previousPersonPlan->getTagProperty().personPlanEndEdge()) {
            // get arrival position
            endLanePosPreviousLane = previousPersonPlan->getAttributeDouble(SUMO_ATTR_ARRIVALPOS);
        } else if (previousPersonPlan->getTagProperty().personPlanEndBusStop()) {
            // get arrival pos (end busStop shape)
            endLanePosPreviousLane = previousPersonPlan->getParentAdditionals().back()->getAttributeDouble(SUMO_ATTR_ENDPOS);
            extremeGeometry.viewStartPos = previousPersonPlan->getParentAdditionals().back()->getAdditionalGeometry().getShape().back();
        }
    }
    // set lane start position
    if (myTagProperty.personPlanStartEdge()) {
        if (previousPersonPlan) {
            // use as startLanePos the endLanePosPreviousLane
            extremeGeometry.laneStartPosition = endLanePosPreviousLane;
            // obtain last allowed vehicle lane of previous person plan
            const GNELane* lastAllowedLanePrevious = previousPersonPlan->getLastAllowedVehicleLane();
            // check if both plans start in the same lane
            if (lastAllowedLanePrevious) {
                extremeGeometry.viewStartPos = lastAllowedLanePrevious->getLaneShape().positionAtOffset(extremeGeometry.laneStartPosition);
            }
        } else {
            // return pedestrian departPos
            extremeGeometry.laneStartPosition = getParentDemandElements().front()->getAttributeDouble(SUMO_ATTR_DEPARTPOS);
        }
    } else if (myTagProperty.personPlanStartBusStop()) {
        // use as startLanePos the busStop end position
        extremeGeometry.laneStartPosition = getParentAdditionals().front()->getAttributeDouble(SUMO_ATTR_ENDPOS);
        // use as extraEndPosition the end of first busStop shape
        extremeGeometry.viewStartPos = getParentAdditionals().front()->getAdditionalGeometry().getShape().back();
    }
    // set lane end position
    if (myTagProperty.personPlanEndEdge()) {
        // use as endLane Position the arrival position
        extremeGeometry.laneEndPosition = getAttributeDouble(SUMO_ATTR_ARRIVALPOS);
    } else if (myTagProperty.personPlanEndBusStop()) {
        // use as endLanePosition the busStop start position
        extremeGeometry.laneEndPosition = getParentAdditionals().back()->getAttributeDouble(SUMO_ATTR_STARTPOS);
        // use as extraEndPosition the begin of last busStop shape
        extremeGeometry.viewEndPos = getParentAdditionals().back()->getAdditionalGeometry().getShape().front();
    }
    // return extreme geometry
    return extremeGeometry;
}


void
GNEDemandElement::drawPersonPlanPartialLane(const GUIVisualizationSettings& s, const GNELane* lane,
        const double offsetFront, const double personPlanWidth, const RGBColor& personPlanColor) const {
    // get inspected person plan
    const GNEAttributeCarrier* personPlanInspected = myNet->getViewNet()->getInspectedAttributeCarrier();
    const GNEDemandElement* personParent = getParentDemandElements().front();
    // declare flag to enable or disable draw person plan
    bool drawPersonPlan = false;
    if (myNet->getViewNet()->getDemandViewOptions().showAllPersonPlans()) {
        // all person plan has to be drawn
        drawPersonPlan = true;
    } else if (myNet->getViewNet()->getDemandViewOptions().getLockedPerson() == personParent) {
        // if person parent is locked, draw all their person plans children
        drawPersonPlan = true;
    } else if (personPlanInspected == personParent) {
        // draw if we're inspecting person parent
        drawPersonPlan = true;
    } else if (personPlanInspected == this) {
        // draw if we're inspecting this demand element
        drawPersonPlan = true;
    }
    // check if draw person plan elements can be drawn
    if (drawPersonPlan) {
        // flag to check if width must be duplicated
        const bool duplicateWidth = (personPlanInspected == this) || (personPlanInspected == personParent);
        // get segment widtsh
        const double segmentWidth = s.addSize.getExaggeration(s, lane) * personPlanWidth;
        // get color
        const RGBColor& color = drawUsingSelectColor() ? s.colorSettings.selectedPersonPlanColor : personPlanColor;
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType(), offsetFront);
        // iterate over segments
        for (const auto& segment : myDemandElementSegmentGeometry) {
            // draw partial segment
            if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                // Set person plan color
                GLHelper::setColor(color);
                // draw segment depending of duplicateWidth
                if (duplicateWidth) {
                    // draw segment geometry
                    GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, (2 * segmentWidth));
                    // check if shape dotted contour has to be drawn
                    if (personPlanInspected == this) {
                        GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, (2 * segmentWidth));
                    }
                } else {
                    // draw segment geometry
                    GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, segmentWidth);
                    // check if shape dotted contour has to be drawn
                    if (personPlanInspected == this) {
                        GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, segmentWidth);
                    }
                }
            }
        }
        // Pop last matrix
        glPopMatrix();
        // Draw name if isn't being drawn for selecting
        if (!s.drawForRectangleSelection) {
            drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
        }
        // Pop name
        glPopName();
        // check if person plan ArrivalPos attribute
        if (myTagProperty.hasAttribute(SUMO_ATTR_ARRIVALPOS)) {
            // obtain arrival position using last segment
            const Position& arrivalPos = getDemandElementSegmentGeometry().getLastPosition();
            // only draw arrival position point if isn't -1
            if (arrivalPos != Position::INVALID) {
                // obtain circle width
                const double circleRadius = (duplicateWidth ? myPersonPlanArrivalPositionDiameter : (myPersonPlanArrivalPositionDiameter / 2.0));
                const double circleWidth = circleRadius * MIN2((double)0.5, s.laneWidthExaggeration);
                const double circleWidthSquared = circleWidth * circleWidth;
                if (!s.drawForRectangleSelection || (myNet->getViewNet()->getPositionInformation().distanceSquaredTo2D(arrivalPos) <= (circleWidthSquared + 2))) {
                    // push draw matrix
                    glPushMatrix();
                    // Start with the drawing of the area traslating matrix to origin
                    myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
                    // translate to pos and move to upper using GLO_PERSONTRIP (to avoid overlapping)
                    glTranslated(arrivalPos.x(), arrivalPos.y(), 0);
                    // Set person plan color
                    GLHelper::setColor(color);
                    // resolution of drawn circle depending of the zoom (To improve smothness)
                    GLHelper::drawFilledCircle(circleWidth, s.getCircleResolution());
                    // pop draw matrix
                    glPopMatrix();
                }
            }
        }
        // check if shape dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // get first and last allowed lanes
            const GNELane* firstLane = getFirstAllowedVehicleLane();
            const GNELane* lastLane = getLastAllowedVehicleLane();
            // iterate over segments
            for (const auto& segment : myDemandElementSegmentGeometry) {
                if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                    // draw partial segment
                    if (firstLane == lane) {
                        // draw front dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), segmentWidth, true, false);
                    } else if (lastLane == lane) {
                        // draw back dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), segmentWidth, false, true);
                    } else {
                        // draw dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, lane->getDottedLaneGeometry(), segmentWidth, false, false);
                    }
                }
            }
        }
    }
    // draw person parent if this is the edge first edge and this is the first plan
    if ((getFirstPersonPlanEdge() == lane->getParentEdge()) && (personParent->getChildDemandElements().front() == this)) {
        personParent->drawGL(s);
    }
}


void
GNEDemandElement::drawPersonPlanPartialJunction(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane,
        const double offsetFront, const double personPlanWidth, const RGBColor& personPlanColor) const {
    // get inspected person plan
    const GNEAttributeCarrier* personPlanInspected = myNet->getViewNet()->getInspectedAttributeCarrier();
    const GNEDemandElement* personParent = getParentDemandElements().front();
    // declare flag to enable or disable draw person plan
    bool drawPersonPlan = false;
    if (myNet->getViewNet()->getDemandViewOptions().showAllPersonPlans()) {
        // all person plan has to be drawn
        drawPersonPlan = true;
    } else if (myNet->getViewNet()->getDemandViewOptions().getLockedPerson() == personParent) {
        // if person parent is locked, draw all their person plans children
        drawPersonPlan = true;
    } else if (personPlanInspected == personParent) {
        // draw if we're inspecting person parent
        drawPersonPlan = true;
    } else if (personPlanInspected == this) {
        // draw if we're inspecting this demand element
        drawPersonPlan = true;
    }
    // check if draw person plan elements can be drawn
    if (drawPersonPlan) {
        // flag to check if width must be duplicated
        const bool duplicateWidth = (personPlanInspected == this) || (personPlanInspected == personParent);
        // get segment widtsh
        const double segmentWidth = s.addSize.getExaggeration(s, fromLane) * personPlanWidth;
        // get color
        const RGBColor& color = drawUsingSelectColor() ? s.colorSettings.selectedPersonPlanColor : personPlanColor;
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // push a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType(), offsetFront);
        // check if draw lane2lane connection or a red line
        if (fromLane->getLane2laneConnections().exist(toLane)) {
            // obtain lane2lane geometry
            const GNEGeometry::Geometry& lane2laneGeometry = fromLane->getLane2laneConnections().getLane2laneGeometry(toLane);
            // Set person plan color
            GLHelper::setColor(color);
            // draw lane2lane
            if (duplicateWidth) {
                GNEGeometry::drawGeometry(myNet->getViewNet(), lane2laneGeometry, (2 * segmentWidth));
            } else {
                GNEGeometry::drawGeometry(myNet->getViewNet(), lane2laneGeometry, segmentWidth);
            }
        } else {
            // Set invalid person plan color
            GLHelper::setColor(RGBColor::RED);
            // draw line between end of first shape and first position of second shape
            GLHelper::drawBoxLines({fromLane->getLaneShape().back(), toLane->getLaneShape().front()}, (0.5 * segmentWidth));
        }
        // Pop last matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // check if shape dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // draw lane2lane dotted geometry
            if (fromLane->getLane2laneConnections().exist(toLane)) {
                GNEGeometry::drawDottedContourLane(true, s, fromLane->getLane2laneConnections().getLane2laneDottedGeometry(toLane), segmentWidth, false, false);
            }
        }
    }
}


void
GNEDemandElement::replaceDemandParentEdges(const std::string& value) {
    replaceParentElements(this, parse<std::vector<GNEEdge*> >(getNet(), value));
}


void
GNEDemandElement::replaceDemandParentLanes(const std::string& value) {
    replaceParentElements(this, parse<std::vector<GNELane*> >(getNet(), value));
}


void
GNEDemandElement::replaceFirstParentEdge(const std::string& value) {
    std::vector<GNEEdge*> parentEdges = getParentEdges();
    parentEdges[0] = myNet->retrieveEdge(value);
    // replace parent edges
    replaceParentElements(this, parentEdges);
}


void
GNEDemandElement::replaceMiddleParentEdges(const std::string& value, const bool updateChildReferences) {
    std::vector<GNEEdge*> middleEdges = parse<std::vector<GNEEdge*> >(getNet(), value);
    middleEdges.insert(middleEdges.begin(), getParentEdges().front());
    middleEdges.push_back(getParentEdges().back());
    // check if we have to update references in all childs, or simply update parent edges vector
    if (updateChildReferences) {
        // replace parent edges
        replaceParentElements(this, middleEdges);
    } else {
        myHierarchicalContainer.setParents<std::vector<GNEEdge*> >(middleEdges);
    }
}


void
GNEDemandElement::replaceLastParentEdge(const std::string& value) {
    std::vector<GNEEdge*> parentEdges = getParentEdges();
    parentEdges[(int)parentEdges.size() - 1] = myNet->retrieveEdge(value);
    // replace parent edges
    replaceParentElements(this, parentEdges);
}


void
GNEDemandElement::replaceAdditionalParent(SumoXMLTag tag, const std::string& value, const int parentIndex) {
    std::vector<GNEAdditional*> parentAdditionals = getParentAdditionals();
    parentAdditionals[parentIndex] = myNet->retrieveAdditional(tag, value);
    // replace parent additionals
    replaceParentElements(this, parentAdditionals);
}


void
GNEDemandElement::replaceDemandElementParent(SumoXMLTag tag, const std::string& value, const int parentIndex) {
    std::vector<GNEDemandElement*> parentDemandElements = getParentDemandElements();
    parentDemandElements[parentIndex] = myNet->retrieveDemandElement(tag, value);
    // replace parent demand elements
    replaceParentElements(this, parentDemandElements);
}


bool
GNEDemandElement::checkChildDemandElementRestriction() const {
    // throw exception because this function mus be implemented in child (see GNEE3Detector)
    throw ProcessError("Calling non-implemented function checkChildDemandElementRestriction during saving of " + getTagStr() + ". It muss be reimplemented in child class");
}

/****************************************************************************/
