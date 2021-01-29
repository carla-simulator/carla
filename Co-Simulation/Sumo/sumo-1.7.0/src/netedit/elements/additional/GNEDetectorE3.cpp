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
/// @file    GNEDetectorE3.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
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

#include "GNEDetectorE3.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNEDetectorE3::GNEDetectorE3(const std::string& id, GNENet* net, Position pos, SUMOTime freq, const std::string& filename,
                             const std::string& vehicleTypes, const std::string& name, SUMOTime timeThreshold, double speedThreshold, bool blockMovement) :
    GNEAdditional(id, net, GLO_E3DETECTOR, SUMO_TAG_E3DETECTOR, name, blockMovement,
{}, {}, {}, {}, {}, {}, {}, {}),
myPosition(pos),
myFreq(freq),
myFilename(filename),
myVehicleTypes(vehicleTypes),
myTimeThreshold(timeThreshold),
mySpeedThreshold(speedThreshold) {
}


GNEDetectorE3::~GNEDetectorE3() {}


void
GNEDetectorE3::updateGeometry() {
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
GNEDetectorE3::getPositionInView() const {
    return myPosition;
}


Boundary
GNEDetectorE3::getCenteringBoundary() const {
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
GNEDetectorE3::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEDetectorE3::moveGeometry(const Position& offset) {
    // restore old position, apply offset and update Geometry
    myPosition = myMove.originalViewPosition;
    myPosition.add(offset);
    // filtern position using snap to active grid
    // filtern position using snap to active grid
    myPosition = myNet->getViewNet()->snapToActiveGrid(myPosition);
    updateGeometry();
}


void
GNEDetectorE3::commitGeometryMoving(GNEUndoList* undoList) {
    // commit new position allowing undo/redo
    undoList->p_begin("position of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPosition), toString(myMove.originalViewPosition)));
    undoList->p_end();
}


std::string
GNEDetectorE3::getParentName() const {
    return myNet->getMicrosimID();
}


void
GNEDetectorE3::drawGL(const GUIVisualizationSettings& s) const {
    // Obtain exaggeration of the draw
    const double E3Exaggeration = s.addSize.getExaggeration(s, this);
    // first check if additional has to be drawn
    if (s.drawAdditionals(E3Exaggeration) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // check if boundary has to be drawn
        if (s.drawBoundaries) {
            GLHelper::drawBoundary(getCenteringBoundary());
        }
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add layer matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_E3DETECTOR);
        // translate to position
        glTranslated(myPosition.x(), myPosition.y(), 0);
        // scale
        glScaled(E3Exaggeration, E3Exaggeration, 1);
        // set color
        glColor3d(1, 1, 1);
        // rotate
        glRotated(180, 0, 0, 1);
        // draw depending
        if (s.drawForRectangleSelection || !s.drawDetail(s.detailSettings.laneTextures, E3Exaggeration)) {
            // set color
            GLHelper::setColor(RGBColor::GREY);
            // just draw a box
            GLHelper::drawBoxLine(Position(0, s.detectorSettings.E3Size), 0, 2 * s.detectorSettings.E3Size, s.detectorSettings.E3Size);
        } else {
            // draw texture
            if (drawUsingSelectColor()) {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_E3SELECTED), s.detectorSettings.E3Size);
            } else {
                GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_E3), s.detectorSettings.E3Size);
            }
            // draw lock icon
            myBlockIcon.drawIcon(s, E3Exaggeration, 0.4);
        }
        // pop layer matrix
        glPopMatrix();
        // Draw name if isn't being drawn for selecting
        drawName(getPositionInView(), s.scale, s.addName);
        // Pop name
        glPopName();
        // push connection matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_E3DETECTOR, -0.1);
        // Draw child connections
        drawHierarchicalConnections(s, this, E3Exaggeration);
        // Pop connection matrix
        glPopMatrix();
        // draw additional name
        drawAdditionalName(s);
        // check if dotted contours has to be drawn
        if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
            GNEGeometry::drawDottedSquaredShape(true, s, myPosition, s.detectorSettings.E3Size, s.detectorSettings.E3Size, 0, 0, 0, E3Exaggeration);
        }
        if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
            GNEGeometry::drawDottedSquaredShape(false, s, myPosition, s.detectorSettings.E3Size, s.detectorSettings.E3Size, 0, 0, 0, E3Exaggeration);
        }
    }
}


std::string
GNEDetectorE3::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_POSITION:
            return toString(myPosition);
        case SUMO_ATTR_FREQUENCY:
            return time2string(myFreq);
        case SUMO_ATTR_NAME:
            return myAdditionalName;
        case SUMO_ATTR_FILE:
            return myFilename;
        case SUMO_ATTR_VTYPES:
            return myVehicleTypes;
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
            return time2string(myTimeThreshold);
        case SUMO_ATTR_HALTING_SPEED_THRESHOLD:
            return toString(mySpeedThreshold);
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
GNEDetectorE3::getAttributeDouble(SumoXMLAttr key) const {
    throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
}


void
GNEDetectorE3::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_FREQUENCY:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_NAME:
        case SUMO_ATTR_FILE:
        case SUMO_ATTR_VTYPES:
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
        case SUMO_ATTR_HALTING_SPEED_THRESHOLD:
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
GNEDetectorE3::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return isValidDetectorID(value);
        case SUMO_ATTR_POSITION:
            return canParse<Position>(value);
        case SUMO_ATTR_FREQUENCY:
            return canParse<SUMOTime>(value);
        case SUMO_ATTR_NAME:
            return SUMOXMLDefinitions::isValidAttribute(value);
        case SUMO_ATTR_FILE:
            return SUMOXMLDefinitions::isValidFilename(value);
        case SUMO_ATTR_VTYPES:
            if (value.empty()) {
                return true;
            } else {
                return SUMOXMLDefinitions::isValidListOfTypeID(value);
            }
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
            return canParse<SUMOTime>(value);
        case SUMO_ATTR_HALTING_SPEED_THRESHOLD:
            return canParse<double>(value) && (parse<double>(value) >= 0);
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
GNEDetectorE3::checkChildAdditionalRestriction() const {
    int numEntrys = 0;
    int numExits = 0;
    // iterate over additional chidls and obtain number of entrys and exits
    for (auto i : getChildAdditionals()) {
        if (i->getTagProperty().getTag() == SUMO_TAG_DET_ENTRY) {
            numEntrys++;
        } else if (i->getTagProperty().getTag() == SUMO_TAG_DET_EXIT) {
            numExits++;
        }
    }
    // write warnings
    if (numEntrys == 0) {
        WRITE_WARNING("An " + toString(SUMO_TAG_E3DETECTOR) + " need at least one " + toString(SUMO_TAG_DET_ENTRY) + " detector");
    }
    if (numExits == 0) {
        WRITE_WARNING("An " + toString(SUMO_TAG_E3DETECTOR) + " need at least one " + toString(SUMO_TAG_DET_EXIT) + " detector");
    }
    // return false depending of number of Entrys and Exits
    return ((numEntrys != 0) && (numExits != 0));
}


bool
GNEDetectorE3::isAttributeEnabled(SumoXMLAttr /* key */) const {
    return true;
}


std::string
GNEDetectorE3::getPopUpID() const {
    return getTagStr() + ":" + getID();
}


std::string
GNEDetectorE3::getHierarchyName() const {
    return getTagStr();
}

// ===========================================================================
// private
// ===========================================================================

void
GNEDetectorE3::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            // Change IDs of all Entry/Exits children
            for (const auto& entryExit : getChildAdditionals()) {
                entryExit->setMicrosimID(getID());
            }
            break;
        case SUMO_ATTR_POSITION:
            myNet->removeGLObjectFromGrid(this);
            myPosition = parse<Position>(value);
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_FREQUENCY:
            myFreq = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_NAME:
            myAdditionalName = value;
            break;
        case SUMO_ATTR_FILE:
            myFilename = value;
            break;
        case SUMO_ATTR_VTYPES:
            myVehicleTypes = value;
            break;
        case SUMO_ATTR_HALTING_TIME_THRESHOLD:
            myTimeThreshold = parse<SUMOTime>(value);
            break;
        case SUMO_ATTR_HALTING_SPEED_THRESHOLD:
            mySpeedThreshold = parse<double>(value);
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


/****************************************************************************/
