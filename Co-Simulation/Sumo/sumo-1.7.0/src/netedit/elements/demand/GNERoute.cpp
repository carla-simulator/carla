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
/// @file    GNERoute.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2019
///
// A class for visualizing routes in Netedit
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/elements/network/GNEJunction.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNERoute.h"

// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GNERoute::GNERoutePopupMenu) GNERoutePopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND, MID_GNE_ROUTE_APPLY_DISTANCE,     GNERoute::GNERoutePopupMenu::onCmdApplyDistance),
};

// Object implementation
FXIMPLEMENT(GNERoute::GNERoutePopupMenu, GUIGLObjectPopupMenu, GNERoutePopupMenuMap, ARRAYNUMBER(GNERoutePopupMenuMap))

// ===========================================================================
// GNERoute::GNERoutePopupMenu - methods
// ===========================================================================

GNERoute::GNERoutePopupMenu::GNERoutePopupMenu(GUIMainWindow& app, GUISUMOAbstractView& parent, GUIGlObject& o) :
    GUIGLObjectPopupMenu(app, parent, o) {
}


GNERoute::GNERoutePopupMenu::~GNERoutePopupMenu() {}


long
GNERoute::GNERoutePopupMenu::onCmdApplyDistance(FXObject*, FXSelector, void*) {
    GNERoute* route = static_cast<GNERoute*>(myObject);
    GNEUndoList* undoList = route->myNet->getViewNet()->getUndoList();
    undoList->p_begin("apply distance along route");
    double dist = (route->getParentEdges().size() > 0) ? route->getParentEdges().front()->getNBEdge()->getDistance() : 0;
    for (GNEEdge* edge : route->getParentEdges()) {
        undoList->p_add(new GNEChange_Attribute(edge, SUMO_ATTR_DISTANCE, toString(dist), edge->getAttribute(SUMO_ATTR_DISTANCE)));
        dist += edge->getNBEdge()->getFinalLength();
    }
    undoList->p_end();
    return 1;
}

// ===========================================================================
// GNERoute - methods
// ===========================================================================

GNERoute::GNERoute(GNENet* net) :
    GNEDemandElement(net->generateDemandElementID(SUMO_TAG_ROUTE), net, GLO_ROUTE, SUMO_TAG_ROUTE,
{}, {}, {}, {}, {}, {}, {}, {}),
Parameterised(),
myColor(RGBColor::YELLOW),
myVClass(SVC_PASSENGER) {
    // compute route
    computePath();
}


GNERoute::GNERoute(GNENet* net, const GNERouteHandler::RouteParameter& routeParameters) :
    GNEDemandElement(routeParameters.routeID, net, GLO_ROUTE, SUMO_TAG_ROUTE,
{}, routeParameters.edges, {}, {}, {}, {}, {}, {}),
Parameterised(routeParameters.parameters),
myColor(routeParameters.color),
myVClass(routeParameters.vClass) {
    // compute route
    computePath();
}


GNERoute::GNERoute(GNENet* net, GNEDemandElement* vehicleParent, const GNERouteHandler::RouteParameter& routeParameters) :
    GNEDemandElement(vehicleParent, net, GLO_ROUTE, GNE_TAG_ROUTE_EMBEDDED,
{}, routeParameters.edges, {}, {}, {}, {}, {vehicleParent}, {}),
Parameterised(routeParameters.parameters),
myColor(routeParameters.color),
myVClass(routeParameters.vClass) {
    // compute route
    computePath();
}


GNERoute::GNERoute(GNEDemandElement* route) :
    GNEDemandElement(route, route->getNet(), GLO_ROUTE, SUMO_TAG_ROUTE,
{}, route->getParentEdges(), {}, {}, {}, {}, {}, {}),
Parameterised(),
myColor(route->getColor()),
myVClass(route->getVClass()) {
    // compute route
    computePath();
}


GNERoute::~GNERoute() {}


GUIGLObjectPopupMenu*
GNERoute::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GNERoutePopupMenu(app, parent, *this);
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
    new FXMenuSeparator(ret);
    new FXMenuCommand(ret, "Apply distance along route", nullptr, ret, MID_GNE_ROUTE_APPLY_DISTANCE);
    return ret;
}


void
GNERoute::writeDemandElement(OutputDevice& device) const {
    device.openTag(SUMO_TAG_ROUTE);
    device.writeAttr(SUMO_ATTR_EDGES, parseIDs(getParentEdges()));
    device.writeAttr(SUMO_ATTR_COLOR, toString(myColor));
    // write extra attributes depending if is an embedded route
    if (myTagProperty.getTag() == SUMO_TAG_ROUTE) {
        device.writeAttr(SUMO_ATTR_ID, getID());
        // write stops associated to this route
        for (const auto& stop : getChildDemandElements()) {
            if (stop->getTagProperty().isStop()) {
                stop->writeDemandElement(device);
            }
        }
    }
    // write parameters
    writeParams(device);
    // close tag
    device.closeTag();
}


bool
GNERoute::isDemandElementValid() const {
    if ((getParentEdges().size() == 2) && (getParentEdges().at(0) == getParentEdges().at(1))) {
        // from and to are the same edges, then return true
        return true;
    } else if (getParentEdges().size() > 0) {
        // check that exist a connection between every edge
        return isRouteValid(getParentEdges()).empty();
    } else {
        return false;
    }
}


std::string
GNERoute::getDemandElementProblem() const {
    // return string with the problem obtained from isRouteValid
    return isRouteValid(getParentEdges());
}


void
GNERoute::fixDemandElementProblem() {
    // currently the only solution is removing Route
}


SUMOVehicleClass
GNERoute::getVClass() const {
    return myVClass;
}


const RGBColor&
GNERoute::getColor() const {
    return myColor;
}


void
GNERoute::startGeometryMoving() {
    // Routes cannot be moved
}


void
GNERoute::endGeometryMoving() {
    // Routes cannot be moved
}


void
GNERoute::moveGeometry(const Position&) {
    // Routes cannot be moved
}


void
GNERoute::commitGeometryMoving(GNEUndoList*) {
    // Routes cannot be moved
}


void
GNERoute::updateGeometry() {
    // declare extreme geometry
    GNEGeometry::ExtremeGeometry extremeGeometry;
    // calculate edge geometry path using path
    GNEGeometry::calculateLaneGeometricPath(myDemandElementSegmentGeometry, getPath(), extremeGeometry);
    // update child demand elementss
    for (const auto& i : getChildDemandElements()) {
        if (!i->getTagProperty().isPersonStop() && !i->getTagProperty().isStop()) {
            i->updateGeometry();
        }
    }
}


void
GNERoute::computePath() {
    // calculate consecutive path using parent edges
    calculateConsecutivePathLanes(getVClass(), true, getParentEdges());
    // update geometry
    updateGeometry();
}


void
GNERoute::invalidatePath() {
    // due routes don't need to calculate a dijkstra path, just calculate consecutive path lanes again
    calculateConsecutivePathLanes(getVClass(), true, getParentEdges());
    // update geometry
    updateGeometry();
}


Position
GNERoute::getPositionInView() const {
    return Position();
}


std::string
GNERoute::getParentName() const {
    return getParentEdges().front()->getID();
}


Boundary
GNERoute::getCenteringBoundary() const {
    Boundary routeBoundary;
    // return the combination of all parent edges's boundaries
    for (const auto& i : getParentEdges()) {
        routeBoundary.add(i->getCenteringBoundary());
    }
    // check if is valid
    if (routeBoundary.isInitialised()) {
        return routeBoundary;
    } else {
        return Boundary(-0.1, -0.1, 0.1, 0.1);
    }
}


void
GNERoute::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* originalElement, const GNENetworkElement* newElement, GNEUndoList* undoList) {
    // obtain new list of route edges
    std::string newRouteEdges = getNewListOfParents(originalElement, newElement);
    // update route edges
    if (newRouteEdges.size() > 0) {
        setAttribute(SUMO_ATTR_EDGES, newRouteEdges, undoList);
    }
}


void
GNERoute::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // Routes are drawn in drawPartialGL
}


void
GNERoute::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    // check if route can be drawn
    if (myNet->getViewNet()->getNetworkViewOptions().showDemandElements() &&
            myNet->getViewNet()->getDataViewOptions().showDemandElements() &&
            myNet->getViewNet()->getDemandViewOptions().showNonInspectedDemandElements(this)) {
        // get route width
        const double routeWidth = s.vehicleSize.getExaggeration(s, this) * s.widthSettings.route;
        // obtain color
        const RGBColor routeColor = drawUsingSelectColor() ? s.colorSettings.selectedRouteColor : getColor();
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
                // Set route color (needed due drawShapeDottedContour)
                GLHelper::setColor(routeColor);
                // draw box lines
                GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, routeWidth);
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
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), routeWidth, true, false);
                    } else if (lastLane == lane) {
                        // draw back dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), routeWidth, false, true);
                    } else {
                        // draw dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, lane->getDottedLaneGeometry(), routeWidth, false, false);
                    }
                }
            }
        }
    }
}


void
GNERoute::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const {
    // only drawn in super mode demand
    if (myNet->getViewNet()->getNetworkViewOptions().showDemandElements() && myNet->getViewNet()->getDataViewOptions().showDemandElements() &&
            fromLane->getLane2laneConnections().exist(toLane) && myNet->getViewNet()->getDemandViewOptions().showNonInspectedDemandElements(this)) {
        // calculate width
        const double width = s.vehicleSize.getExaggeration(s, this) * s.widthSettings.route;
        // obtain lane2lane geometry
        const GNEGeometry::Geometry& lane2laneGeometry = fromLane->getLane2laneConnections().getLane2laneGeometry(toLane);
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType() + offsetFront);
        // Set color of the base
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedRouteColor);
        } else {
            GLHelper::setColor(getColor());
        }
        // draw lane2lane
        GNEGeometry::drawGeometry(myNet->getViewNet(), lane2laneGeometry, width);
        // Pop last matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // check if shape dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // draw lane2lane dotted geometry
            if (fromLane->getLane2laneConnections().exist(toLane)) {
                GNEGeometry::drawDottedContourLane(true, s, fromLane->getLane2laneConnections().getLane2laneDottedGeometry(toLane), width, false, false);
            }
        }
    }
}


std::string
GNERoute::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_EDGES:
            return parseIDs(getParentEdges());
        case SUMO_ATTR_COLOR:
            return toString(myColor);
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNERoute::getAttributeDouble(SumoXMLAttr /*key*/) const {
    return 0;
}


void
GNERoute::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGES:
        case SUMO_ATTR_COLOR:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERoute::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidDemandElementID(value);
        case SUMO_ATTR_EDGES:
            if (canParse<std::vector<GNEEdge*> >(myNet, value, false)) {
                // all edges exist, then check if compounds a valid route
                return isRouteValid(parse<std::vector<GNEEdge*> >(myNet, value)).empty();
            } else {
                return false;
            }
        case SUMO_ATTR_COLOR:
            return canParse<RGBColor>(value);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNERoute::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNERoute::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNERoute::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNERoute::getPopUpID() const {
    return getTagStr();
}


std::string
GNERoute::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) ;
}


std::string
GNERoute::isRouteValid(const std::vector<GNEEdge*>& edges) {
    if (edges.size() == 0) {
        // routes cannot be empty
        return ("list of route edges cannot be empty");
    } else if (edges.size() == 1) {
        // routes with a single edge are valid, then return an empty string
        return ("");
    } else {
        // iterate over edges to check that compounds a chain
        auto it = edges.begin();
        while (it != edges.end() - 1) {
            const GNEEdge* currentEdge = *it;
            const GNEEdge* nextEdge = *(it + 1);
            // same consecutive edges aren't allowed
            if (currentEdge->getID() == nextEdge->getID()) {
                return ("consecutive duplicated edges (" + currentEdge->getID() + ") aren't allowed in a route");
            }
            // obtain outgoing edges of currentEdge
            const std::vector<GNEEdge*>& outgoingEdges = currentEdge->getParentJunctions().back()->getGNEOutgoingEdges();
            // check if nextEdge is in outgoingEdges
            if (std::find(outgoingEdges.begin(), outgoingEdges.end(), nextEdge) == outgoingEdges.end()) {
                return ("Edges '" + currentEdge->getID() + "' and '" + nextEdge->getID() + "' aren't consecutives");
            }
            it++;
        }
        // all edges consecutives, then return an empty string
        return ("");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNERoute::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_EDGES:
            replaceDemandParentEdges(value);
            // compute route
            updateGeometry();
            break;
        case SUMO_ATTR_COLOR:
            myColor = parse<RGBColor>(value);
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
GNERoute::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
