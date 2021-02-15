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
/// @file    GNEVaporizer.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2016
///
//
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEVaporizer.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEVaporizer::GNEVaporizer(GNENet* net, GNEEdge* edge, SUMOTime begin, SUMOTime end, const std::string& name) :
    GNEAdditional(edge->getID(), net, GLO_VAPORIZER, SUMO_TAG_VAPORIZER, name, false,
{}, {edge}, {}, {}, {}, {}, {}, {}),
myBegin(begin),
myEnd(end) {
}


GNEVaporizer::~GNEVaporizer() {
}


void
GNEVaporizer::updateGeometry() {
    // calculate perpendicular line
    calculatePerpendicularLine(3);
}


Position
GNEVaporizer::getPositionInView() const {
    if (getParentEdges().front()->getLanes().front()->getLaneShape().length() < 2.5) {
        return getParentEdges().front()->getLanes().front()->getLaneShape().front();
    } else {
        Position A = getParentEdges().front()->getLanes().front()->getLaneShape().positionAtOffset(2.5);
        Position B = getParentEdges().front()->getLanes().back()->getLaneShape().positionAtOffset(2.5);
        // return Middle point
        return Position((A.x() + B.x()) / 2, (A.y() + B.y()) / 2);
    }
}


Boundary
GNEVaporizer::getCenteringBoundary() const {
    return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
}


void
GNEVaporizer::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEVaporizer::moveGeometry(const Position&) {
    // This additional cannot be moved
}


void
GNEVaporizer::commitGeometryMoving(GNEUndoList*) {
    // This additional cannot be moved
}


std::string
GNEVaporizer::getParentName() const {
    return getParentEdges().front()->getID();
}


void
GNEVaporizer::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double vaporizerExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(vaporizerExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // declare colors
        RGBColor vaporizerColor, centralLineColor;
        // set colors
        if (drawUsingSelectColor()) {
            vaporizerColor = s.colorSettings.selectedAdditionalColor;
            centralLineColor = vaporizerColor.changedBrightness(-32);
        } else {
            vaporizerColor = s.additionalSettings.vaporizerColor;
            centralLineColor = RGBColor::WHITE;
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add layer matrix matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_VAPORIZER);
        // set base color
        GLHelper::setColor(vaporizerColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GNEGeometry::drawGeometry(myNet->getViewNet(), myAdditionalGeometry, 0.3 * vaporizerExaggeration);
        // move to front
        glTranslated(0, 0, .1);
        // set central color
        GLHelper::setColor(centralLineColor);
        // Draw the area using shape, shapeRotations, shapeLengths and value of exaggeration
        GNEGeometry::drawGeometry(myNet->getViewNet(), myAdditionalGeometry, 0.05 * vaporizerExaggeration);
        // move to icon position and front
        glTranslated(myAdditionalGeometry.getShape().front().x(), myAdditionalGeometry.getShape().front().y(), .1);
        // rotate
        glRotated(myAdditionalGeometry.getShape().rotationDegreeAtOffset(0), 0, 0, -1);
        // Draw icon depending of Route Probe is selected and if isn't being drawn for selecting
        if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.laneTextures, vaporizerExaggeration)) {
            // set color
            glColor3d(1, 1, 1);
            // rotate texture
            glRotated(180, 0, 0, 1);
            // draw texture
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_VAPORIZERSELECTED), s.additionalSettings.vaporizerSize * vaporizerExaggeration);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_VAPORIZER), s.additionalSettings.vaporizerSize * vaporizerExaggeration);
            }
        } else {
            // set route probe color
            GLHelper::setColor(vaporizerColor);
            // just drawn a box
            GLHelper::drawBoxLine(Position(0, 0), 0, 2 * s.additionalSettings.vaporizerSize, s.additionalSettings.vaporizerSize * vaporizerExaggeration);
        }
        // pop layer matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(true, s, myAdditionalGeometry.getShape(), 0.3, vaporizerExaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedContourShape(false, s, myAdditionalGeometry.getShape(), 0.3, vaporizerExaggeration);
        }
    }
}


std::string
GNEVaporizer::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
            return getID();
        case SUMO_ATTR_BEGIN:
            return time2string(myBegin);
        case SUMO_ATTR_END:
            return time2string(myEnd);
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEVaporizer::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_BEGIN:
            return STEPS2TIME(myBegin);
        case SUMO_ATTR_END:
            return STEPS2TIME(myEnd);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNEVaporizer::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
        case SUMO_ATTR_BEGIN:
        case SUMO_ATTR_END:
        case SUMO_ATTR_NAME:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEVaporizer::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
            if (myNet->retrieveEdge(value, false) != nullptr) {
                return isValidAdditionalID(value);
            } else {
                return false;
            }
        case SUMO_ATTR_BEGIN:
            if (canParse<SUMOTime>(value)) {
                return (parse<SUMOTime>(value) <= myEnd);
            } else {
                return false;
            }
        case SUMO_ATTR_END:
            if (canParse<SUMOTime>(value)) {
                return (myBegin <= parse<SUMOTime>(value));
            } else {
                return false;
            }
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEVaporizer::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEVaporizer::getPopUpID() const {
    return getTagStr();
}


std::string
GNEVaporizer::getHierarchyName() const {
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_BEGIN) + " -> " + getAttribute(SUMO_ATTR_END);
}

// ===========================================================================
// private
// ===========================================================================

void
GNEVaporizer::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
            myNet->getAttributeCarriers()->updateID(this, value);
            replaceAdditionalParentEdges(value);
            break;
        case SUMO_ATTR_BEGIN:
            myBegin = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_END:
            myEnd = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
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
