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
/// @file    GNERouteProbe.cpp
/// @author  Pablo Alvarez Lopez
/// @date    May 2016
///
//
/****************************************************************************/
#include <config.h>

#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/gui/div/GLHelper.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNERouteProbe.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNERouteProbe::GNERouteProbe(const std::string& id, GNENet* net, GNEEdge* edge, const std::string& frequency,
                             const std::string& name, const std::string& filename, SUMOTime begin) :
    GNEAdditional(id, net, GLO_ROUTEPROBE, SUMO_TAG_ROUTEPROBE, name, false,
{}, {edge}, {}, {}, {}, {}, {}, {}),
myFrequency(frequency),
myFilename(filename),
myBegin(begin) {
}


GNERouteProbe::~GNERouteProbe() {
}


void
GNERouteProbe::updateGeometry() {
    // calculate perpendicular line
    calculatePerpendicularLine(1);
}


Position
GNERouteProbe::getPositionInView() const {
    if (getParentEdges().front()->getLanes().front()->getLaneShape().length() < 0.5) {
        return getParentEdges().front()->getLanes().front()->getLaneShape().front();
    } else {
        Position A = getParentEdges().front()->getLanes().front()->getLaneShape().positionAtOffset(0.5);
        Position B = getParentEdges().front()->getLanes().back()->getLaneShape().positionAtOffset(0.5);
        // return Middle point
        return Position((A.x() + B.x()) / 2, (A.y() + B.y()) / 2);
    }
}


Boundary
GNERouteProbe::getCenteringBoundary() const {
    return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
}


void
GNERouteProbe::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNERouteProbe::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNERouteProbe::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


std::string
GNERouteProbe::getParentName() const {
    return getParentEdges().front()->getID();
}


void
GNERouteProbe::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double routeProbeExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(routeProbeExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // declare colors
        RGBColor routeProbeColor, centralLineColor;
        // set colors
        if (drawUsingSelectColor()) {
            routeProbeColor = s.colorSettings.selectedAdditionalColor;
            centralLineColor = routeProbeColor.changedBrightness(-32);
        } else {
            routeProbeColor = s.additionalSettings.routeProbeColor;
            centralLineColor = RGBColor::WHITE;
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add layer matrix matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_ROUTEPROBE);
        // set base color
        GLHelper::setColor(routeProbeColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GNEGeometry::drawGeometry(myNet->getViewNet(), myAdditionalGeometry, 0.3 * routeProbeExaggeration);
        // move to front
        glTranslated(0, 0, .1);
        // set central color
        GLHelper::setColor(centralLineColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GNEGeometry::drawGeometry(myNet->getViewNet(), myAdditionalGeometry, 0.05 * routeProbeExaggeration);
        // move to icon position and front
        glTranslated(myAdditionalGeometry.getShape().front().x(), myAdditionalGeometry.getShape().front().y(), .1);
        // rotate
        glRotated(myAdditionalGeometry.getShape().rotationDegreeAtOffset(0), 0, 0, -1);
        // Draw icon depending of Route Probe is selected and if isn't being drawn for selecting
        if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.laneTextures, routeProbeExaggeration)) {
            // set color
            glColor3d(1, 1, 1);
            // rotate texture
            glRotated(180, 0, 0, 1);
            // draw texture
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_ROUTEPROBESELECTED), s.additionalSettings.routeProbeSize * routeProbeExaggeration);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_ROUTEPROBE), s.additionalSettings.routeProbeSize * routeProbeExaggeration);
            }
        } else {
            // set route probe color
            GLHelper::setColor(routeProbeColor);
            // just drawn a box
            GLHelper::drawBoxLine(Position(0, 0), 0, 2 * s.additionalSettings.routeProbeSize, s.additionalSettings.routeProbeSize * routeProbeExaggeration);
        }
        // pop layer matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(true, s, myAdditionalGeometry.getShape(), 0.3, routeProbeExaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(false, s, myAdditionalGeometry.getShape(), 0.3, routeProbeExaggeration);
        }
    }
}


std::string
GNERouteProbe::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_EDGE:
            return getParentEdges().front()->getID();
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_FILE:
            return myFilename;
        case SUMO_ATTR_FREQUENCY:
            return toString(myFrequency);
        case SUMO_ATTR_BEGIN:
            return time2string(myBegin);
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNERouteProbe::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_BEGIN:
            return STEPS2TIME(myBegin);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNERouteProbe::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_FILE:
        case SUMO_ATTR_FREQUENCY:
        case SUMO_ATTR_BEGIN:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNERouteProbe::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNERouteProbe::getPopUpID() const {
    return getTagStr();
}


std::string
GNERouteProbe::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_BEGIN);
}

// ===========================================================================
// private
// ===========================================================================

bool
GNERouteProbe::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_EDGE:
            if (myNet->retrieveEdge(value, false) != nullptr) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_FILE:
            return SUMOXMLDefinitions::isValidFilename(value);
        case SUMO_ATTR_FREQUENCY:
            if (value.empty()) {
                return true;
            } else {
                return canParse<SUMOTime>(value);
            }
        case SUMO_ATTR_BEGIN:
            return canParse<SUMOTime>(value);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNERouteProbe::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_EDGE:
            replaceAdditionalParentEdges(value);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_FILE:
            myFilename = value;
            break;
        case SUMO_ATTR_FREQUENCY:
            myFrequency = value;
            break;
        case SUMO_ATTR_BEGIN:
            myBegin = parse<SUMOTime>(value);
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
