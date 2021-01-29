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
/// @file    GNEVariableSpeedSignSymbol.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jul 2020
///
//
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEVariableSpeedSignSymbol.h"
#include "GNEAdditionalHandler.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNEVariableSpeedSignSymbol::GNEVariableSpeedSignSymbol(GNEAdditional* VSSParent, GNELane* lane) :
    GNEAdditional(VSSParent->getNet(), GLO_VSS, GNE_TAG_VSS_SYMBOL, "", false,
{}, {}, {lane}, {VSSParent}, {}, {}, {}, {}) {
}


GNEVariableSpeedSignSymbol::~GNEVariableSpeedSignSymbol() {
}


void
GNEVariableSpeedSignSymbol::moveGeometry(const Position& /*offset*/) {
    // Nothing to do
}


void
GNEVariableSpeedSignSymbol::commitGeometryMoving(GNEUndoList* /*undoList*/) {
    // Nothing to do
}


void
GNEVariableSpeedSignSymbol::updateGeometry() {
    myAdditionalGeometry.updateGeometry(getParentLanes().front(), 1.5);
    // update connections
    getParentAdditionals().front()->updateHierarchicalConnections();
}


Position
GNEVariableSpeedSignSymbol::getPositionInView() const {
    return myAdditionalGeometry.getPosition();
}


Boundary
GNEVariableSpeedSignSymbol::getCenteringBoundary() const {
    return myAdditionalGeometry.getShape().getBoxBoundary().grow(10);
}


void
GNEVariableSpeedSignSymbol::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/,
        const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // nothing to split
}


std::string
GNEVariableSpeedSignSymbol::getParentName() const {
    return getParentAdditionals().at(0)->getID();
}


void
GNEVariableSpeedSignSymbol::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double VSSExaggeration = s.addSize.getExaggeration(s, getParentAdditionals().front());
    // first check if additional has to be drawn
    if (s.drawAdditionals(VSSExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // Start drawing adding an gl identificator (except in Move mode)
        if (myNet->getViewNet()->getEditModes().networkEditMode != NetworkEditMode::NETWORK_MOVE) {
            glPushName(getParentAdditionals().front()->getGlID());
        }
        // start drawing symbol
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(getParentAdditionals().front(), GLO_VSS);
        // translate to position
        glTranslated(myAdditionalGeometry.getPosition().x(), myAdditionalGeometry.getPosition().y(), 0);
        // rotate
        glRotated(-1 * myAdditionalGeometry.getRotation(), 0, 0, 1);
        // scale
        glScaled(VSSExaggeration, VSSExaggeration, 1);
        // set color
        if (getParentAdditionals().front()->isAttributeCarrierSelected()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else {
            GLHelper::setColor(RGBColor::RED);
        }
        // draw circle
        GLHelper::drawFilledCircle((double) 1.3, s.getCircleResolution());
        // draw details
        if (!s.forceDrawForPositionSelection && (s.scale >= 5)) {
            // move to front
            glTranslated(0, 0, 0.1);
            // set color
            if (getParentAdditionals().front()->isAttributeCarrierSelected()) {
                GLHelper::setColor(s.colorSettings.selectedAdditionalColor.changedBrightness(-32));
            } else {
                GLHelper::setColor(RGBColor::BLACK);
            }
            // draw another circle
            GLHelper::drawFilledCircle((double) 1.1, s.getCircleResolution());
            // move to front
            glTranslated(0, 0, 0.1);
            // draw speed
            if (getParentAdditionals().front()->isAttributeCarrierSelected()) {
                GLHelper::drawText("S", Position(0, 0), .1, 1.2, s.colorSettings.selectedAdditionalColor, 180);
            } else {
                GLHelper::drawText("S", Position(0, 0), .1, 1.2, RGBColor::YELLOW, 180);
            }
        }
        // Pop symbol matrix
        glPopMatrix();
        // Pop VSS name
        if (myNet->getViewNet()->getEditModes().networkEditMode != NetworkEditMode::NETWORK_MOVE) {
            glPopName();
        }
        // check if dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == getParentAdditionals().front())) {
            GNEGeometry::drawDottedContourCircle(true, s, myAdditionalGeometry.getPosition(), 1.3, VSSExaggeration);
        }
        if (s.drawDottedContour() || (myNet->getViewNet()->getFrontAttributeCarrier() == getParentAdditionals().front())) {
            GNEGeometry::drawDottedContourCircle(false, s, myAdditionalGeometry.getPosition(), 1.3, VSSExaggeration);
        }
    }
}


std::string
GNEVariableSpeedSignSymbol::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_LANE:
            return getParentLanes().front()->getID();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEVariableSpeedSignSymbol::getAttributeDouble(SumoXMLAttr /*key*/) const {
    throw InvalidArgument("Symbols cannot be edited");
}


void
GNEVariableSpeedSignSymbol::setAttribute(SumoXMLAttr /*key*/, const std::string& /*value*/, GNEUndoList* /*undoList*/) {
    throw InvalidArgument("Symbols cannot be edited");
}


bool
GNEVariableSpeedSignSymbol::isValid(SumoXMLAttr /*key*/, const std::string& /*value*/) {
    throw InvalidArgument("Symbols cannot be edited");
}


bool
GNEVariableSpeedSignSymbol::isAttributeEnabled(SumoXMLAttr /*key*/) const {
    return true;
}


std::string
GNEVariableSpeedSignSymbol::getPopUpID() const {
    return getParentAdditionals().at(0)->getPopUpID();
}


std::string
GNEVariableSpeedSignSymbol::getHierarchyName() const {
    return getParentAdditionals().at(0)->getHierarchyName();
}

// ===========================================================================
// private
// ===========================================================================

void
GNEVariableSpeedSignSymbol::setAttribute(SumoXMLAttr /*key*/, const std::string& /*value*/) {
    throw InvalidArgument("Symbols cannot be edited");
}


/****************************************************************************/
