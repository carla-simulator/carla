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
/// @file    GNERerouter.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Additional.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/dialogs/GNERerouterDialog.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNERerouter.h"
#include "GNERerouterSymbol.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNERerouter::GNERerouter(const std::string& id, GNENet* net, const Position& pos,
                         const std::string& name, const std::string& filename, double probability,
                         bool off, SUMOTime timeThreshold, const std::string& vTypes, bool blockMovement) :
    GNEAdditional(id, net, GLO_REROUTER, SUMO_TAG_REROUTER, name, blockMovement,
{}, {}, {}, {}, {}, {}, {}, {}),
myPosition(pos),
myFilename(filename),
myProbability(probability),
myOff(off),
myTimeThreshold(timeThreshold),
myVTypes(vTypes) {
}


GNERerouter::~GNERerouter() {
}


void
GNERerouter::updateGeometry() {
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
GNERerouter::getPositionInView() const {
    return myPosition;
}


Boundary
GNERerouter::getCenteringBoundary() const {
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
GNERerouter::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNERerouter::openAdditionalDialog() {
    // Open rerouter dialog
    GNERerouterDialog(this);
}


void
GNERerouter::moveGeometry(const Position& offset) {
    // restore old position, apply offset and update Geometry
    myPosition = myMove.originalViewPosition;
    myPosition.add(offset);
    // filtern position using snap to active grid
    myPosition = myNet->getViewNet()->snapToActiveGrid(myPosition);
    updateGeometry();
}


void
GNERerouter::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPosition), toString(myMove.originalViewPosition)));
    undoList->p_end();
}


std::string
GNERerouter::getParentName() const {
    return myNet->getMicrosimID();
}


void
GNERerouter::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double rerouterExaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(rerouterExaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // check if boundary has to be drawn
        if (s.drawBoundaries) {
            GLHelper::drawBoundary(getCenteringBoundary());
        }
        // push name
        glPushName(getGlID());
        // push layer matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_REROUTER);
        // translate to position
        glTranslated(myPosition.x(), myPosition.y(), 0);
        // scale
        glScaled(rerouterExaggeration, rerouterExaggeration, 1);
        // Draw icon depending of detector is selected and if isn't being drawn for selecting
        if (!s.drawForPositionSelection) {
            // set White color
            glColor3d(1, 1, 1);
            // rotate
            glRotated(180, 0, 0, 1);
            // draw texture depending of selection
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_REROUTERSELECTED), s.additionalSettings.rerouterSize);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_REROUTER), s.additionalSettings.rerouterSize);
            }
        } else {
            // set redcolor
            GLHelper::setColor(RGBColor::RED);
            // just draw a square
            GLHelper::drawBoxLine(Position(0, s.additionalSettings.rerouterSize), 0, 2 * s.additionalSettings.rerouterSize, s.additionalSettings.rerouterSize);
        }
        // draw Lock icon
        myBlockIcon.drawIcon(s, rerouterExaggeration, 0.4);
        // Pop layer matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // push connection matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_REROUTER, -0.1);
        // Draw child connections
        drawHierarchicalConnections(s, this, rerouterExaggeration);
        // Pop connection matrix
        glPopMatrix();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            GNEGeometry::drawDottedSquaredShape(true, s, myPosition, s.additionalSettings.rerouterSize, s.additionalSettings.rerouterSize, 0, 0, 0, rerouterExaggeration);
        }
        if (s.drawDottedContour() || (myNet->getViewNet()->getFrontAttributeCarrier() == this)) {
            GNEGeometry::drawDottedSquaredShape(false, s, myPosition, s.additionalSettings.rerouterSize, s.additionalSettings.rerouterSize, 0, 0, 0, rerouterExaggeration);
        }
    }
}


std::string
GNERerouter::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_EDGES: {
            std::vector<std::string> edges;
            for (const auto& rerouterSymbol : getChildAdditionals()) {
                if (rerouterSymbol->getTagProperty().isSymbol()) {
                    edges.push_back(rerouterSymbol->getAttribute(SUMO_ATTR_EDGE));
                }
            }
            return toString(edges);
        }
        case SUMO_ATTR_POSITION:
            return toString(myPosition);
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_FILE:
            return myFilename;
        case SUMO_ATTR_PROB:
            return toString(myProbability);
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
            return time2string(myTimeThreshold);
        case SUMO_ATTR_VTYPES:
            return myVTypes;
        case SUMO_ATTR_OFF:
            return toString(myOff);
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
GNERerouter::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_PROB:
            return myProbability;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNERerouter::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        // special case  for lanes due rerouter Symbols
        case SUMO_ATTR_EDGES:
            // rebuild rerouter Symbols
            rebuildRerouterSymbols(value, undoList);
            break;
        case SUMO_ATTR_ID:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_FILE:
        case SUMO_ATTR_PROB:
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
        case SUMO_ATTR_VTYPES:
        case SUMO_ATTR_OFF:
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
GNERerouter::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidAdditionalID(value);
        case SUMO_ATTR_EDGES:
            return canParse<std::vector<GNEEdge*> >(myNet, value, false);
        case SUMO_ATTR_POSITION:
            return canParse<Position>(value);
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_FILE:
            return SUMOXMLDefinitions::isValidFilename(value);
        case SUMO_ATTR_PROB:
            return canParse<double>(value) && (parse<double>(value) >= 0) && (parse<double>(value) <= 1);
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
            return canParse<SUMOTime>(value);
        case SUMO_ATTR_VTYPES:
            if (value.empty()) {
                return true;
            } else {
                return SUMOXMLDefinitions::isValidListOfTypeID(value);
            }
        case SUMO_ATTR_OFF:
            return canParse<bool>(value);
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
GNERerouter::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNERerouter::getPopUpID() const {
    return getTagStr() + ": " + getID();
}


std::string
GNERerouter::getHierarchyName() const {
    return getTagStr();
}

// ===========================================================================
// private
// ===========================================================================

void
GNERerouter::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_EDGES:
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
        case SUMO_ATTR_FILE:
            myFilename = value;
            break;
        case SUMO_ATTR_PROB:
            myProbability = parse<double>(value);
            break;
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
            myTimeThreshold = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_VTYPES:
            myVTypes = value;
            break;
        case SUMO_ATTR_OFF:
            myOff = parse<bool>(value);
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
GNERerouter::rebuildRerouterSymbols(const std::string& value, GNEUndoList* undoList) {
    undoList->p_begin(("change " + getTagStr() + " attribute").c_str());
    // drop all additional children
    while (getChildAdditionals().size() > 0) {
        undoList->add(new GNEChange_Additional(getChildAdditionals().front(), false), true);
    }
    // get edge vector
    const std::vector<GNEEdge*> edges = parse<std::vector<GNEEdge*> >(myNet, value);
    // create new VSS Symbols
    for (const auto& edge : edges) {
        // create VSS Symbol
        GNEAdditional* VSSSymbol = new GNERerouterSymbol(this, edge);
        // add it using GNEChange_Additional
        myNet->getViewNet()->getUndoList()->add(new GNEChange_Additional(VSSSymbol, true), true);
    }
    undoList->p_end();
}


/****************************************************************************/
