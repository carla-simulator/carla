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
/// @file    GNEVariableSpeedSign.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#include <config.h>

#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/gui/div/GLHelper.h>
#include <netedit/changes/GNEChange_Additional.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/dialogs/GNEVariableSpeedSignDialog.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNENet.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEVariableSpeedSign.h"
#include "GNEVariableSpeedSignSymbol.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEVariableSpeedSign::GNEVariableSpeedSign(const std::string& id, GNENet* net, const Position& pos, const std::string& name, bool blockMovement) :
    GNEAdditional(id, net, GLO_VSS, SUMO_TAG_VSS, name, blockMovement,
{}, {}, {}, {}, {}, {}, {}, {}),
myPosition(pos) {
}


GNEVariableSpeedSign::~GNEVariableSpeedSign() {
}


void
GNEVariableSpeedSign::updateGeometry() {
    // update additional geometry
    myAdditionalGeometry.updateGeometry(myPosition, 0);

    // update block icon position
    myBlockIcon.updatePositionAndRotation();

    // Set block icon offset
    myBlockIcon.setOffset(-0.5, -0.5);

    // Update Hierarchical connections geometry
    myHierarchicalConnections.update();
}


Position
GNEVariableSpeedSign::getPositionInView() const {
    return myPosition;
}


Boundary
GNEVariableSpeedSign::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMove.movingGeometryBoundary.isInitialised()) {
        return myMove.movingGeometryBoundary;
    } else {
        Boundary b;
        b.add(myPosition);
        b.grow(5);
        return b;
    }
}


void
GNEVariableSpeedSign::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEVariableSpeedSign::openAdditionalDialog() {
    // Open VSS dialog
    GNEVariableSpeedSignDialog(this);
}


void
GNEVariableSpeedSign::moveGeometry(const Position& offset) {
    // restore old position, apply offset and update Geometry
    myPosition = myMove.originalViewPosition;
    myPosition.add(offset);
    // filtern position using snap to active grid
    myPosition = myNet->getViewNet()->snapToActiveGrid(myPosition);
    updateGeometry();
}


void
GNEVariableSpeedSign::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPosition), toString(myMove.originalViewPosition)));
    undoList->p_end();
}


std::string
GNEVariableSpeedSign::getParentName() const {
    return myNet->getMicrosimID();
}


void
GNEVariableSpeedSign::drawGL(const GUIVisualizationSettings& s) const {
    // obtain exaggeration
    const double VSSExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(VSSExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // check if boundary has to be drawn
        if (s.drawBoundaries) {
            GLHelper::drawBoundary(getCenteringBoundary());
        }
        // push name
        glPushName(getGlID());
        // push layer matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_VSS);
        // translate to position
        glTranslated(myPosition.x(), myPosition.y(), 0);
        // scale
        glScaled(VSSExaggeration, VSSExaggeration, 1);
        // Draw icon depending of variable speed sign is or if isn't being drawn for selecting
        if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.laneTextures, VSSExaggeration)) {
            // set white color
            glColor3d(1, 1, 1);
            // rotate
            glRotated(180, 0, 0, 1);
            // draw texture
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_VARIABLESPEEDSIGNSELECTED), s.additionalSettings.VSSSize);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_VARIABLESPEEDSIGN), s.additionalSettings.VSSSize);
            }
        } else {
            // set white color
            GLHelper::setColor(RGBColor::WHITE);
            // just draw a withe square
            GLHelper::drawBoxLine(Position(0, s.additionalSettings.VSSSize), 0, 2 * s.additionalSettings.VSSSize, s.additionalSettings.VSSSize);
        }
        // Show Lock icon
        myBlockIcon.drawIcon(s, VSSExaggeration, 0.4);
        // Pop layer matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // push connection matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_VSS, -0.1);
        // Draw child connections
        drawHierarchicalConnections(s, this, VSSExaggeration);
        // Pop connection matrix
        glPopMatrix();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            GNEGeometry::drawDottedSquaredShape(true, s, myPosition, s.additionalSettings.VSSSize, s.additionalSettings.VSSSize, 0, 0, 0, VSSExaggeration);
        }
        if (s.drawDottedContour() || (myNet->getViewNet()->getFrontAttributeCarrier() == this)) {
            GNEGeometry::drawDottedSquaredShape(false, s, myPosition, s.additionalSettings.VSSSize, s.additionalSettings.VSSSize, 0, 0, 0, VSSExaggeration);
        }
    }
}


std::string
GNEVariableSpeedSign::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_LANES: {
            std::vector<std::string> lanes;
            for (const auto& VSSSymbol : getChildAdditionals()) {
                if (VSSSymbol->getTagProperty().isSymbol()) {
                    lanes.push_back(VSSSymbol->getAttribute(SUMO_ATTR_LANE));
                }
            }
            return toString(lanes);
        }
        case SUMO_ATTR_POSITION:
            return toString(myPosition);
        case SUMO_ATTR_NAME:
            return myAdditionalName;
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


double
GNEVariableSpeedSign::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEVariableSpeedSign::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        // special case  for lanes due VSS Symbols
        case SUMO_ATTR_LANES:
            // rebuild VSS Symbols
            rebuildVSSSymbols(value, undoList);
            break;
        case SUMO_ATTR_ID:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_NAME:
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
GNEVariableSpeedSign::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_POSITION:
            return canParse<Position>(value);
        case SUMO_ATTR_LANES:
            return canParse<std::vector<GNELane*> >(myNet, value, false);
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
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


bool
GNEVariableSpeedSign::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEVariableSpeedSign::getPopUpID() const {
    return getTagStr() + ": " + getID();
}


std::string
GNEVariableSpeedSign::getHierarchyName() const {
    return getTagStr();
}

// ===========================================================================
// private
// ===========================================================================

void
GNEVariableSpeedSign::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_LANES:
            throw InvalidArgument(getTagStr() + " cannot be edited");
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_POSITION:
            myNet->removeGLObjectFromGrid(this);
            myPosition = parse<Position>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
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
GNEVariableSpeedSign::rebuildVSSSymbols(const std::string& value, GNEUndoList* undoList) {
    undoList->p_begin(("change " + getTagStr() + " attribute").c_str());
    // drop all additional children
    while (getChildAdditionals().size() > 0) {
        undoList->add(new GNEChange_Additional(getChildAdditionals().front(), false), true);
    }
    // get lane vector
    const std::vector<GNELane*> lanes = parse<std::vector<GNELane*> >(myNet, value);
    // create new VSS Symbols
    for (const auto& lane : lanes) {
        // create VSS Symbol
        GNEAdditional* VSSSymbol = new GNEVariableSpeedSignSymbol(this, lane);
        // add it using GNEChange_Additional
        myNet->getViewNet()->getUndoList()->add(new GNEChange_Additional(VSSSymbol, true), true);
    }
    undoList->p_end();
}

/****************************************************************************/
