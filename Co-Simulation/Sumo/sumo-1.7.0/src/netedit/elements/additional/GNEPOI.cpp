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
/// @file    GNEPOI.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2017
///
// A class for visualizing and editing POIS in netedit (adapted from
// GUIPointOfInterest and NLHandler)
/****************************************************************************/
#include <string>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <utils/gui/globjects/GLIncludes.h>

#include "GNEPOI.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEPOI::GNEPOI(GNENet* net, const std::string& id, const std::string& type, const RGBColor& color,
               const Position& pos, bool geo, double layer, double angle, const std::string& imgFile,
               bool relativePath, double width, double height, bool movementBlocked) :
    GUIPointOfInterest(id, type, color, pos, geo, "", 0, 0, layer, angle, imgFile, relativePath, width, height),
    GNEShape(net, SUMO_TAG_POI, movementBlocked,
{}, {}, {}, {}, {}, {}, {}, {}) {
    // set GEO Position
    myGEOPosition = pos;
    GeoConvHelper::getFinal().cartesian2geo(myGEOPosition);
}


GNEPOI::GNEPOI(GNENet* net, const std::string& id, const std::string& type, const RGBColor& color,
               double layer, double angle, const std::string& imgFile, bool relativePath, GNELane* lane, double posOverLane, double posLat,
               double width, double height, bool movementBlocked) :
    GUIPointOfInterest(id, type, color, Position(), false, lane->getID(), posOverLane, posLat, layer, angle, imgFile, relativePath, width, height),
    GNEShape(net, SUMO_TAG_POILANE, movementBlocked,
{}, {}, {lane}, {}, {}, {}, {}, {}) {
}


GNEPOI::~GNEPOI() {}


const std::string&
GNEPOI::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNEPOI::getGUIGlObject() {
    return this;
}


std::string
GNEPOI::generateChildID(SumoXMLTag childTag) {
    int counter = (int)myNet->getAttributeCarriers()->getShapes().at(SUMO_TAG_POI).size();
    while ((myNet->retrieveShape(SUMO_TAG_POI, getID() + toString(childTag) + toString(counter), false) != nullptr) &&
            (myNet->retrieveShape(SUMO_TAG_POILANE, getID() + toString(childTag) + toString(counter), false) != nullptr)) {
        counter++;
    }
    return (getID() + toString(childTag) + toString(counter));
}


void
GNEPOI::setParameter(const std::string& key, const std::string& value) {
    Parameterised::setParameter(key, value);
}


void
GNEPOI::startPOIGeometryMoving() {
    myPositionBeforeMoving = *this;
}


void
GNEPOI::endPOIGeometryMoving() {
}


void
GNEPOI::writeShape(OutputDevice& device) {
    if (getParentLanes().size() > 0) {
        // obtain fixed position over lane
        double fixedPositionOverLane = myPosOverLane > getParentLanes().at(0)->getLaneShape().length() ? getParentLanes().at(0)->getLaneShape().length() : myPosOverLane < 0 ? 0 : myPosOverLane;
        // write POILane using POI::writeXML
        writeXML(device, false, 0, getParentLanes().at(0)->getID(), fixedPositionOverLane, myPosLat);
    } else {
        writeXML(device, myGeo);
    }
}


void
GNEPOI::movePOIGeometry(const Position& offset) {
    if (!myBlockMovement) {
        // Calculate new position using old position
        Position newPosition = myPositionBeforeMoving;
        newPosition.add(offset);
        // filtern position using snap to active grid
        newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition);
        // set position depending of POI Type
        if (getParentLanes().size() > 0) {
            myPosOverLane = getParentLanes().at(0)->getLaneShape().nearest_offset_to_point2D(newPosition, false);
        } else {
            set(newPosition);
        }
        // Update geometry
        updateGeometry();
    }
}


void
GNEPOI::commitPOIGeometryMoving(GNEUndoList* undoList) {
    if (!myBlockMovement) {
        // restore original Position before moving (to avoid problems in GL Tree)
        Position myNewPosition(*this);
        set(myPositionBeforeMoving);
        // commit new position allowing undo/redo
        if (getParentLanes().size() > 0) {
            // restore old position before commit new position
            double originalPosOverLane = getParentLanes().at(0)->getLaneShape().nearest_offset_to_point2D(myPositionBeforeMoving, false);
            undoList->p_begin("position of " + getTagStr());
            undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myPosOverLane), toString(originalPosOverLane)));
            undoList->p_end();
        } else {
            undoList->p_begin("position of " + getTagStr());
            undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myNewPosition), toString(myPositionBeforeMoving)));
            undoList->p_end();
        }
    }
}


void
GNEPOI::updateGeometry() {
    if (getParentLanes().size() > 0) {
        // obtain fixed position over lane
        double fixedPositionOverLane = myPosOverLane > getParentLanes().at(0)->getLaneShapeLength() ? getParentLanes().at(0)->getLaneShapeLength() : myPosOverLane < 0 ? 0 : myPosOverLane;
        // set new position regarding to lane
        set(getParentLanes().at(0)->getLaneShape().positionAtOffset(fixedPositionOverLane * getParentLanes().at(0)->getLengthGeometryFactor(), -myPosLat));
    }
}


Position
GNEPOI::getPositionInView() const {
    return Position(x(), y());
}


Boundary
GNEPOI::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMovingGeometryBoundary.isInitialised()) {
        return myMovingGeometryBoundary;
    }  else {
        return GUIPointOfInterest::getCenteringBoundary();
    }
}


GUIGlID
GNEPOI::getGlID() const {
    return GUIPointOfInterest::getGlID();
}


std::string
GNEPOI::getParentName() const {
    if (getParentLanes().size() > 0) {
        return getParentLanes().front()->getID();
    } else {
        return myNet->getMicrosimID();
    }
}


GUIGLObjectPopupMenu*
GNEPOI::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    if (getParentLanes().size() > 0) {
        // build shape header
        buildShapePopupOptions(app, ret, getShapeType());
        // add option for convert to GNEPOI
        new FXMenuCommand(ret, ("Release from " + toString(SUMO_TAG_LANE)).c_str(), GUIIconSubSys::getIcon(GUIIcon::LANE), &parent, MID_GNE_POI_TRANSFORM);
        return ret;
    } else {
        // build shape header
        buildShapePopupOptions(app, ret, getShapeType());
        // add option for convert to GNEPOI
        new FXMenuCommand(ret, ("Attach to nearest " + toString(SUMO_TAG_LANE)).c_str(), GUIIconSubSys::getIcon(GUIIcon::LANE), &parent, MID_GNE_POI_TRANSFORM);
    }
    return ret;
}


GUIParameterTableWindow*
GNEPOI::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    return GUIPointOfInterest::getParameterWindow(app, parent);
}


void
GNEPOI::drawGL(const GUIVisualizationSettings& s) const {
    // first check if POI can be drawn
    if (myNet->getViewNet()->getDemandViewOptions().showShapes() && myNet->getViewNet()->getDataViewOptions().showShapes()) {
        // check if boundary has to be drawn
        if (s.drawBoundaries) {
            GLHelper::drawBoundary(getCenteringBoundary());
        }
        // check if POI can be drawn
        if (checkDraw(s)) {
            // obtain POIExaggeration
            const double POIExaggeration = s.poiSize.getExaggeration(s, this);
            // push name (needed for getGUIGlObjectsUnderCursor(...)
            glPushName(getGlID());
            // draw inner polygon
            if (myNet->getViewNet()->getFrontAttributeCarrier() == this) {
                drawInnerPOI(s, drawUsingSelectColor(), GLO_DOTTEDCONTOUR_FRONT);
            } else {
                drawInnerPOI(s, drawUsingSelectColor(), getShapeLayer());
            }
            // draw an orange square mode if there is an image(see #4036)
            if (!getShapeImgFile().empty() && myNet->getViewNet()->getTestingMode().isTestingEnabled()) {
                // Add a draw matrix for drawing logo
                glPushMatrix();
                glTranslated(x(), y(), getType() + 0.01);
                GLHelper::setColor(RGBColor::ORANGE);
                GLHelper::drawBoxLine(Position(0, 1), 0, 2, 1);
                glPopMatrix();
            }
            // check if dotted contour has to be drawn
            if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
                if (getShapeImgFile().empty()) {
                    GNEGeometry::drawDottedContourCircle(true, s, *this, 1.3, POIExaggeration);
                } else {
                    GNEGeometry::drawDottedSquaredShape(true, s, *this, getWidth(), getHeight(), 0, 0, getShapeNaviDegree(), POIExaggeration);
                }
            }
            // pop name
            glPopName();
        }
    }
}


std::string
GNEPOI::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return myID;
        case SUMO_ATTR_COLOR:
            return toString(getShapeColor());
        case SUMO_ATTR_LANE:
            return myLane;
        case SUMO_ATTR_POSITION:
            if (getParentLanes().size() > 0) {
                return toString(myPosOverLane);
            } else {
                return toString(*this);
            }
        case SUMO_ATTR_POSITION_LAT:
            return toString(myPosLat);
        case SUMO_ATTR_GEOPOSITION:
            return toString(myGEOPosition, gPrecisionGeo);
        case SUMO_ATTR_GEO:
            return toString(myGeo);
        case SUMO_ATTR_TYPE:
            return getShapeType();
        case SUMO_ATTR_LAYER:
            if (getShapeLayer() == Shape::DEFAULT_LAYER_POI) {
                return "default";
            } else {
                return toString(getShapeLayer());
            }
        case SUMO_ATTR_IMGFILE:
            return getShapeImgFile();
        case SUMO_ATTR_RELATIVEPATH:
            return toString(getShapeRelativePath());
        case SUMO_ATTR_WIDTH:
            return toString(getWidth());
        case SUMO_ATTR_HEIGHT:
            return toString(getHeight());
        case SUMO_ATTR_ANGLE:
            return toString(getShapeNaviDegree());
        case GNE_ATTR_BLOCK_MOVEMENT:
            return toString(myBlockMovement);
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " attribute '" + toString(key) + "' not allowed");
    }
}


void
GNEPOI::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_COLOR:
        case SUMO_ATTR_LANE:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_POSITION_LAT:
        case SUMO_ATTR_GEOPOSITION:
        case SUMO_ATTR_GEO:
        case SUMO_ATTR_TYPE:
        case SUMO_ATTR_LAYER:
        case SUMO_ATTR_IMGFILE:
        case SUMO_ATTR_RELATIVEPATH:
        case SUMO_ATTR_WIDTH:
        case SUMO_ATTR_HEIGHT:
        case SUMO_ATTR_ANGLE:
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
GNEPOI::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return SUMOXMLDefinitions::isValidTypeID(value) &&
                   (myNet->retrieveShape(SUMO_TAG_POI, value, false) == nullptr) &&
                   (myNet->retrieveShape(SUMO_TAG_POILANE, value, false) == nullptr);
        case SUMO_ATTR_COLOR:
            return canParse<RGBColor>(value);
        case SUMO_ATTR_LANE:
            return (myNet->retrieveLane(value, false) != nullptr);
        case SUMO_ATTR_POSITION:
            if (getParentLanes().size() > 0) {
                return canParse<double>(value);
            } else {
                return canParse<Position>(value);
            }
        case SUMO_ATTR_POSITION_LAT:
            return canParse<double>(value);
        case SUMO_ATTR_GEOPOSITION: {
            return canParse<Position>(value);
        }
        case SUMO_ATTR_GEO:
            return canParse<bool>(value);
        case SUMO_ATTR_TYPE:
            return true;
        case SUMO_ATTR_LAYER:
            if (value == "default") {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_IMGFILE:
            if (value == "") {
                return true;
            } else {
                // check that image can be loaded
                return GUITexturesHelper::getTextureID(value) != -1;
            }
        case SUMO_ATTR_RELATIVEPATH:
            return canParse<bool>(value);
        case SUMO_ATTR_WIDTH:
            return canParse<double>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_HEIGHT:
            return canParse<double>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_ANGLE:
            return canParse<double>(value);
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
GNEPOI::isAttributeEnabled(SumoXMLAttr /* key */) const {
    // check if we're in supermode Network
    if (myNet->getViewNet()->getEditModes().isCurrentSupermodeNetwork()) {
        return true;
    } else {
        return false;
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNEPOI::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID: {
            // note: getAttributeCarriers().updateID doesn't change Microsim ID in GNEShapes
            myNet->getAttributeCarriers()->updateID(this, value);
            // set named ID
            myID = value;
            break;
        }
        case SUMO_ATTR_COLOR:
            setShapeColor(parse<RGBColor>(value));
            break;
        case SUMO_ATTR_LANE:
            myLane = value;
            replaceShapeParentLanes(value);
            break;
        case SUMO_ATTR_POSITION: {
            if (getParentLanes().size() > 0) {
                myPosOverLane = parse<double>(value);
            } else {
                // first remove object from grid due position is used for boundary
                myNet->removeGLObjectFromGrid(this);
                // set position
                set(parse<Position>(value));
                // set GEO Position
                myGEOPosition.setx(this->x());
                myGEOPosition.sety(this->y());
                myGEOPosition.setz(this->z());
                GeoConvHelper::getFinal().cartesian2geo(myGEOPosition);
                // add object into grid again
                myNet->addGLObjectIntoGrid(this);
            }
            break;
        }
        case SUMO_ATTR_POSITION_LAT:
            myPosLat = parse<double>(value);
            break;
        case SUMO_ATTR_GEOPOSITION: {
            // first remove object from grid due position is used for boundary
            myNet->removeGLObjectFromGrid(this);
            // set new position
            myGEOPosition = parse<Position>(value);
            // set cartesian Position
            set(myGEOPosition);
            GeoConvHelper::getFinal().x2cartesian_const(*this);
            // add object into grid again
            myNet->addGLObjectIntoGrid(this);
            break;
        }
        case SUMO_ATTR_GEO:
            myGeo = parse<bool>(value);
            break;
        case SUMO_ATTR_TYPE:
            setShapeType(value);
            break;
        case SUMO_ATTR_LAYER:
            if (value == "default") {
                setShapeLayer(Shape::DEFAULT_LAYER_POI);
            } else {
                setShapeLayer(parse<double>(value));
            }
            break;
        case SUMO_ATTR_IMGFILE:
            // first remove object from grid due img file affect to boundary
            myNet->removeGLObjectFromGrid(this);
            setShapeImgFile(value);
            // all textures must be refresh
            GUITexturesHelper::clearTextures();
            // add object into grid again
            myNet->addGLObjectIntoGrid(this);
            break;
        case SUMO_ATTR_RELATIVEPATH:
            setShapeRelativePath(parse<bool>(value));
            break;
        case SUMO_ATTR_WIDTH:
            if (getParentLanes().size() > 0) {
                // set new width
                setWidth(parse<double>(value));
            } else {
                // first remove object from grid due position is used for boundary
                myNet->removeGLObjectFromGrid(this);
                // set new width
                setWidth(parse<double>(value));
                // add object into grid again
                myNet->addGLObjectIntoGrid(this);
            }
            break;
        case SUMO_ATTR_HEIGHT:
            if (getParentLanes().size() > 0) {
                // set new height
                setHeight(parse<double>(value));
            } else {
                // first remove object from grid due position is used for boundary
                myNet->removeGLObjectFromGrid(this);
                // set new height
                setHeight(parse<double>(value));
                // add object into grid again
                myNet->addGLObjectIntoGrid(this);
            }
            break;
        case SUMO_ATTR_ANGLE:
            setShapeNaviDegree(parse<double>(value));
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
            throw InvalidArgument(getTagStr() + " attribute '" + toString(key) + "' not allowed");
    }
}

/****************************************************************************/
