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
/// @file    GNEPoly.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2017
///
// A class for visualizing and editing POIS in netedit (adapted from
// GUIPolygon and NLHandler)
/****************************************************************************/
#include <string>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>

#include "GNEPoly.h"


// ===========================================================================
// method definitions
// ===========================================================================
GNEPoly::GNEPoly(GNENet* net, const std::string& id, const std::string& type, const PositionVector& shape, bool geo, bool fill, double lineWidth,
                 const RGBColor& color, double layer, double angle, const std::string& imgFile, bool relativePath, bool movementBlocked, bool shapeBlocked) :
    GUIPolygon(id, type, color, shape, geo, fill, lineWidth, layer, angle, imgFile, relativePath),
    GNEShape(net, SUMO_TAG_POLY, movementBlocked,
{}, {}, {}, {}, {}, {}, {}, {}),
myBlockShape(shapeBlocked),
mySimplifiedShape(false) {
    // check if imgFile is valid
    if (!imgFile.empty() && GUITexturesHelper::getTextureID(imgFile) == -1) {
        setShapeImgFile("");
    }
    // set GEO shape
    myGeoShape = myShape;
    for (int i = 0; i < (int) myGeoShape.size(); i++) {
        GeoConvHelper::getFinal().cartesian2geo(myGeoShape[i]);
    }
    // update geometry
    updateGeometry();
}


GNEPoly::~GNEPoly() {}


const std::string&
GNEPoly::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNEPoly::getGUIGlObject() {
    return this;
}


std::string
GNEPoly::generateChildID(SumoXMLTag childTag) {
    int counter = (int)myNet->getAttributeCarriers()->getShapes().at(SUMO_TAG_POLY).size();
    while (myNet->retrieveShape(SUMO_TAG_POLY, getID() + toString(childTag) + toString(counter), false) != nullptr) {
        counter++;
    }
    return (getID() + toString(childTag) + toString(counter));
}


void
GNEPoly::setParameter(const std::string& key, const std::string& value) {
    Parameterised::setParameter(key, value);
}


void
GNEPoly::startPolyShapeGeometryMoving(const double shapeOffset) {
    // save current centering boundary
    myMovingGeometryBoundary = getCenteringBoundary();
    // start move shape depending of block shape
    if (myBlockShape) {
        startMoveShape(myShape, -1, myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.polygonGeometryPointRadius);
    } else {
        startMoveShape(myShape, shapeOffset, myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.polygonGeometryPointRadius);
    }
}


void
GNEPoly::endPolyShapeGeometryMoving() {
    // check that endGeometryMoving was called only once
    if (myMovingGeometryBoundary.isInitialised()) {
        // Remove object from net
        myNet->removeGLObjectFromGrid(this);
        // reset myMovingGeometryBoundary
        myMovingGeometryBoundary.reset();
        // add object into grid again (using the new centering boundary)
        myNet->addGLObjectIntoGrid(this);
    }
}


int
GNEPoly::getPolyVertexIndex(Position pos, const bool snapToGrid) const {
    // check if position has to be snapped to grid
    if (snapToGrid) {
        pos = myNet->getViewNet()->snapToActiveGrid(pos);
    }
    const double offset = myShape.nearest_offset_to_point2D(pos, true);
    if (offset == GeomHelper::INVALID_OFFSET) {
        return -1;
    }
    Position newPos = myShape.positionAtOffset2D(offset);
    // first check if vertex already exists in the inner geometry
    for (int i = 0; i < (int)myShape.size(); i++) {
        if (myShape[i].distanceTo2D(newPos) < myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.polygonGeometryPointRadius) {
            // index refers to inner geometry
            if (i == 0 || i == (int)(myShape.size() - 1)) {
                return -1;
            }
            return i;
        }
    }
    return -1;
}


void
GNEPoly::movePolyShape(const Position& offset) {
    // first obtain a copy of shapeBeforeMoving
    PositionVector newShape = getShapeBeforeMoving();
    if (moveEntireShape()) {
        // move entire shape
        newShape.add(offset);
    } else {
        int geometryPointIndex = getGeometryPointIndex();
        // if geometryPoint is -1, then we have to create a new geometry point
        if (geometryPointIndex == -1) {
            geometryPointIndex = newShape.insertAtClosest(getPosOverShapeBeforeMoving(), true);
        }
        // get last index
        const int lastIndex = (int)newShape.size() - 1;
        // check if we have to move first and last postion
        if ((newShape.size() > 2) && (newShape.front() == newShape.back()) &&
                ((geometryPointIndex == 0) || (geometryPointIndex == lastIndex))) {
            // move first geometry point
            newShape[0].add(offset);
            // snap to grid
            newShape[0] = myNet->getViewNet()->snapToActiveGrid(newShape[0]);
            // set end geometry point
            newShape[lastIndex] = newShape[0];
        } else {
            // move geometry point within newShape
            newShape[geometryPointIndex].add(offset);
            // snap to grid
            newShape[geometryPointIndex] = myNet->getViewNet()->snapToActiveGrid(newShape[geometryPointIndex]);
        }
    }
    // set new poly shape
    myShape = newShape;
    // update geometry
    updateGeometry();
}


void
GNEPoly::commitPolyShapeChange(GNEUndoList* undoList) {
    // get visualisation settings
    auto& s = myNet->getViewNet()->getVisualisationSettings();
    // restore original shape into shapeToCommit
    PositionVector shapeToCommit = myShape;
    // get geometryPoint radius
    const double geometryPointRadius = s.neteditSizeSettings.polygonGeometryPointRadius * s.polySize.getExaggeration(s, this);
    // remove double points
    shapeToCommit.removeDoublePoints(geometryPointRadius);
    // check if we have to merge start and end points
    if ((shapeToCommit.front() != shapeToCommit.back()) && (shapeToCommit.front().distanceTo2D(shapeToCommit.back()) < geometryPointRadius)) {
        shapeToCommit[0] = shapeToCommit.back();
    }
    // update geometry
    updateGeometry();
    // restore old geometry to allow change attribute (And restore shape if during movement a new point was created
    myShape = getShapeBeforeMoving();
    // finish geometry moving
    endPolyShapeGeometryMoving();
    // commit new shape
    undoList->p_begin("moving " + toString(SUMO_ATTR_SHAPE) + " of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_SHAPE, toString(shapeToCommit)));
    undoList->p_end();
}


void
GNEPoly::updateGeometry() {
    // just update geometry
    myPolygonGeometry.updateGeometry(myShape);
}


void
GNEPoly::writeShape(OutputDevice& device) {
    writeXML(device, myGEO);
}


Position
GNEPoly::getPositionInView() const {
    return myShape.getPolygonCenter();
}


Boundary
GNEPoly::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMovingGeometryBoundary.isInitialised()) {
        return myMovingGeometryBoundary;
    }  else {
        return GUIPolygon::getCenteringBoundary();
    }
}


GUIGlID
GNEPoly::getGlID() const {
    return GUIPolygon::getGlID();
}


std::string
GNEPoly::getParentName() const {
    return myNet->getMicrosimID();
}


GUIGLObjectPopupMenu*
GNEPoly::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    FXMenuCommand* simplifyShape = new FXMenuCommand(ret, "Simplify Shape\t\tReplace current shape with a rectangle", nullptr, &parent, MID_GNE_POLYGON_SIMPLIFY_SHAPE);
    // disable simplify shape if polygon was already simplified
    if (mySimplifiedShape || myShape.size() <= 2) {
        simplifyShape->disable();
    }
    if (myShape.isClosed()) {
        new FXMenuCommand(ret, "Open shape\t\tOpen polygon's shape", nullptr, &parent, MID_GNE_POLYGON_OPEN);
    } else {
        new FXMenuCommand(ret, "Close shape\t\tClose polygon's shape", nullptr, &parent, MID_GNE_POLYGON_CLOSE);
    }
    // create a extra FXMenuCommand if mouse is over a vertex
    int index = getVertexIndex(myNet->getViewNet()->getPositionInformation(), false);
    if (index != -1) {
        FXMenuCommand* removeGeometryPoint = new FXMenuCommand(ret, "Remove geometry point\t\tRemove geometry point under mouse", nullptr, &parent, MID_GNE_POLYGON_DELETE_GEOMETRY_POINT);
        FXMenuCommand* setFirstPoint = new FXMenuCommand(ret, "Set first geometry point\t\tSet", nullptr, &parent, MID_GNE_POLYGON_SET_FIRST_POINT);
        // disable setFirstPoint if shape only have three points
        if ((myShape.isClosed() && (myShape.size() <= 4)) || (!myShape.isClosed() && (myShape.size() <= 2))) {
            removeGeometryPoint->disable();
        }
        // disable setFirstPoint if mouse is over first point
        if (index == 0) {
            setFirstPoint->disable();
        }
    }
    return ret;
}


GUIParameterTableWindow*
GNEPoly::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    return GUIPolygon::getParameterWindow(app, parent);
}


void
GNEPoly::drawGL(const GUIVisualizationSettings& s) const {
    // check if boundary has to be drawn
    if (s.drawBoundaries) {
        GLHelper::drawBoundary(getCenteringBoundary());
    }
    // first check if poly can be drawn
    if (myNet->getViewNet()->getDemandViewOptions().showShapes() &&
            myNet->getViewNet()->getDataViewOptions().showShapes() &&
            checkDraw(s)) {
        // Obtain constants
        const double polyExaggeration = s.polySize.getExaggeration(s, this);
        const Position mousePosition = myNet->getViewNet()->getPositionInformation();
        // get colors
        const RGBColor color = isAttributeCarrierSelected() ? s.colorSettings.selectionColor : getShapeColor();
        const RGBColor invertedColor = color.invertedColor();
        const RGBColor darkerColor = color.changedBrightness(-32);
        // obtain scaled geometry
        GNEGeometry::Geometry scaledGeometry = myPolygonGeometry;
        if (polyExaggeration != 1) {
            scaledGeometry.scaleGeometry(polyExaggeration);
        }
        // push name (needed for getGUIGlObjectsUnderCursor(...)
        glPushName(getGlID());
        // push layer matrix
        glPushMatrix();
        // translate to front
        myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_POLYGON);
        // check if we're drawing a polygon or a polyline
        if (getFill()) {
            if (s.drawForPositionSelection) {
                // check if mouse is within geometry
                if (scaledGeometry.getShape().around(mousePosition)) {
                    // push matrix
                    glPushMatrix();
                    // move to mouse position
                    glTranslated(mousePosition.x(), mousePosition.y(), 0);
                    // set color
                    GLHelper::setColor(color);
                    // draw circle
                    GLHelper::drawFilledCircle(1, s.getCircleResolution());
                    // pop matrix
                    glPopMatrix();
                }
            } else {
                // draw inner polygon
                drawInnerPolygon(s, scaledGeometry.getShape(), 0, drawUsingSelectColor());
            }
        } else {
            // push matrix
            glPushMatrix();
            // set color
            GLHelper::setColor(color);
            // draw geometry (polyline)
            GNEGeometry::drawGeometry(myNet->getViewNet(), scaledGeometry, s.neteditSizeSettings.polylineWidth * polyExaggeration);
            // pop matrix
            glPopMatrix();
        }
        // draw contour if shape isn't blocked
        if (!myBlockShape) {
            // push contour matrix
            glPushMatrix();
            // translate to front
            glTranslated(0, 0, 0.1);
            // set color
            GLHelper::setColor(darkerColor);
            // draw polygon contour
            GNEGeometry::drawGeometry(myNet->getViewNet(), scaledGeometry, s.neteditSizeSettings.polygonContourWidth);
            // pop contour matrix
            glPopMatrix();
            // draw shape points only in Network supemode
            if (s.drawMovingGeometryPoint(polyExaggeration, s.neteditSizeSettings.polygonGeometryPointRadius) && myNet->getViewNet()->getEditModes().isCurrentSupermodeNetwork()) {
                // draw geometry points
                GNEGeometry::drawGeometryPoints(s, myNet->getViewNet(), scaledGeometry.getShape(), darkerColor, invertedColor, s.neteditSizeSettings.polygonGeometryPointRadius, polyExaggeration);
                // draw moving hint points
                if (myBlockMovement == false) {
                    GNEGeometry::drawMovingHint(s, myNet->getViewNet(), scaledGeometry.getShape(), invertedColor, s.neteditSizeSettings.polygonGeometryPointRadius, polyExaggeration);
                }
            }
        }
        // pop layer matrix
        glPopMatrix();
        // check if dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // draw depending if is closed
            if (getFill() || scaledGeometry.getShape().isClosed()) {
                GNEGeometry::drawDottedContourClosedShape(true, s, scaledGeometry.getShape(), 1);
            } else {
                GNEGeometry::drawDottedContourShape(true, s, scaledGeometry.getShape(), s.neteditSizeSettings.polylineWidth, polyExaggeration);
            }
        }
        // pop name
        glPopName();
    }
}


int
GNEPoly::getVertexIndex(Position pos, bool snapToGrid) {
    // check if position has to be snapped to grid
    if (snapToGrid) {
        pos = myNet->getViewNet()->snapToActiveGrid(pos);
    }
    // first check if vertex already exists
    for (const auto& shapePosition : myShape) {
        if (shapePosition.distanceTo2D(pos) < myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.polygonGeometryPointRadius) {
            return myShape.indexOfClosest(shapePosition);
        }
    }
    return -1;
}


void
GNEPoly::deleteGeometryPoint(const Position& pos, bool allowUndo) {
    if (myShape.size() > 1) {
        // obtain index
        PositionVector modifiedShape = myShape;
        int index = modifiedShape.indexOfClosest(pos);
        // remove point dependending of
        if (myShape.isClosed() && (index == 0 || index == (int)modifiedShape.size() - 1) && (myShape.size() > 2)) {
            modifiedShape.erase(modifiedShape.begin());
            modifiedShape.erase(modifiedShape.end() - 1);
            modifiedShape.push_back(modifiedShape.front());
        } else {
            modifiedShape.erase(modifiedShape.begin() + index);
        }
        // set new shape depending of allowUndo
        if (allowUndo) {
            myNet->getViewNet()->getUndoList()->p_begin("delete geometry point");
            setAttribute(SUMO_ATTR_SHAPE, toString(modifiedShape), myNet->getViewNet()->getUndoList());
            myNet->getViewNet()->getUndoList()->p_end();
        } else {
            // first remove object from grid due shape is used for boundary
            myNet->removeGLObjectFromGrid(this);
            // set new shape
            myShape = modifiedShape;
            // disable simplified shape flag
            mySimplifiedShape = false;
            // add object into grid again
            myNet->addGLObjectIntoGrid(this);
        }
    } else {
        WRITE_WARNING("Number of remaining points insufficient")
    }
}


bool
GNEPoly::isPolygonBlocked() const {
    return myBlockShape;
}


bool
GNEPoly::isPolygonClosed() const {
    return myShape.isClosed();
}


void
GNEPoly::openPolygon(bool allowUndo) {
    // only open if shape is closed
    if (myShape.isClosed()) {
        if (allowUndo) {
            myNet->getViewNet()->getUndoList()->p_begin("open polygon");
            setAttribute(GNE_ATTR_CLOSE_SHAPE, "false", myNet->getViewNet()->getUndoList());
            myNet->getViewNet()->getUndoList()->p_end();
        } else {
            myShape.pop_back();
            // disable simplified shape flag
            mySimplifiedShape = false;
            // update geometry to avoid grabbing Problems
            updateGeometry();
        }
    } else {
        WRITE_WARNING("Polygon already opened")
    }
}


void
GNEPoly::closePolygon(bool allowUndo) {
    // only close if shape is opened
    if (myShape.isClosed() == false) {
        if (allowUndo) {
            myNet->getViewNet()->getUndoList()->p_begin("close shape");
            setAttribute(GNE_ATTR_CLOSE_SHAPE, "true", myNet->getViewNet()->getUndoList());
            myNet->getViewNet()->getUndoList()->p_end();
        } else {
            myShape.closePolygon();
            // disable simplified shape flag
            mySimplifiedShape = false;
            // update geometry to avoid grabbing Problems
            updateGeometry();
        }
    } else {
        WRITE_WARNING("Polygon already closed")
    }
}


void
GNEPoly::changeFirstGeometryPoint(int oldIndex, bool allowUndo) {
    // check that old index is correct
    if (oldIndex >= (int)myShape.size()) {
        throw InvalidArgument("Invalid old Index");
    } else if (oldIndex == 0) {
        WRITE_WARNING("Selected point must be different of the first point")
    } else {
        // Configure new shape
        PositionVector newShape;
        for (int i = oldIndex; i < (int)myShape.size(); i++) {
            newShape.push_back(myShape[i]);
        }
        if (myShape.isClosed()) {
            for (int i = 1; i < oldIndex; i++) {
                newShape.push_back(myShape[i]);
            }
            newShape.push_back(newShape.front());
        } else {
            for (int i = 0; i < oldIndex; i++) {
                newShape.push_back(myShape[i]);
            }
        }
        // set new rotated shape
        if (allowUndo) {
            myNet->getViewNet()->getUndoList()->p_begin("change first geometry point");
            setAttribute(SUMO_ATTR_SHAPE, toString(newShape), myNet->getViewNet()->getUndoList());
            myNet->getViewNet()->getUndoList()->p_end();
        } else {
            // set new shape
            myShape = newShape;
            // disable simplified shape flag
            mySimplifiedShape = false;
            // update geometry to avoid grabbing Problems
            updateGeometry();
        }
    }
}


void
GNEPoly::simplifyShape(bool allowUndo) {
    if (!mySimplifiedShape && myShape.size() > 2) {
        const Boundary b =  myShape.getBoxBoundary();
        PositionVector simplifiedShape;
        if (myShape.isClosed()) {
            // create a square as simplified shape
            simplifiedShape.push_back(Position(b.xmin(), b.ymin()));
            simplifiedShape.push_back(Position(b.xmin(), b.ymax()));
            simplifiedShape.push_back(Position(b.xmax(), b.ymax()));
            simplifiedShape.push_back(Position(b.xmax(), b.ymin()));
            simplifiedShape.push_back(simplifiedShape[0]);
        } else {
            // create a line as simplified shape
            simplifiedShape.push_back(myShape.front());
            simplifiedShape.push_back(myShape.back());
        }
        // set new shape depending of allowUndo
        if (allowUndo) {
            myNet->getViewNet()->getUndoList()->p_begin("simplify shape");
            setAttribute(SUMO_ATTR_SHAPE, toString(simplifiedShape), myNet->getViewNet()->getUndoList());
            myNet->getViewNet()->getUndoList()->p_end();
        } else {
            // set new shape
            myShape = simplifiedShape;
            // update geometry to avoid grabbing Problems
            updateGeometry();
        }
        // change flag after setting simplified shape
        mySimplifiedShape = true;
    } else {
        WRITE_WARNING("Polygon already simplified")
    }
}


std::string
GNEPoly::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return myID;
        case SUMO_ATTR_SHAPE:
            return toString(myShape);
        case SUMO_ATTR_GEOSHAPE:
            return toString(myGeoShape, gPrecisionGeo);
        case SUMO_ATTR_COLOR:
            return toString(getShapeColor());
        case SUMO_ATTR_FILL:
            return toString(myFill);
        case SUMO_ATTR_LINEWIDTH:
            return toString(myLineWidth);
        case SUMO_ATTR_LAYER:
            if (getShapeLayer() == Shape::DEFAULT_LAYER) {
                return "default";
            } else {
                return toString(getShapeLayer());
            }
        case SUMO_ATTR_TYPE:
            return getShapeType();
        case SUMO_ATTR_IMGFILE:
            return getShapeImgFile();
        case SUMO_ATTR_RELATIVEPATH:
            return toString(getShapeRelativePath());
        case SUMO_ATTR_ANGLE:
            return toString(getShapeNaviDegree());
        case SUMO_ATTR_GEO:
            return toString(myGEO);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return toString(myBlockMovement);
        case GNE_ATTR_BLOCK_SHAPE:
            return toString(myBlockShape);
        case GNE_ATTR_CLOSE_SHAPE:
            return toString(myShape.isClosed());
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPoly::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_SHAPE:
        case SUMO_ATTR_GEOSHAPE:
        case SUMO_ATTR_COLOR:
        case SUMO_ATTR_FILL:
        case SUMO_ATTR_LINEWIDTH:
        case SUMO_ATTR_LAYER:
        case SUMO_ATTR_TYPE:
        case SUMO_ATTR_IMGFILE:
        case SUMO_ATTR_RELATIVEPATH:
        case SUMO_ATTR_ANGLE:
        case SUMO_ATTR_GEO:
        case GNE_ATTR_BLOCK_MOVEMENT:
        case GNE_ATTR_BLOCK_SHAPE:
        case GNE_ATTR_CLOSE_SHAPE:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEPoly::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return SUMOXMLDefinitions::isValidTypeID(value) && (myNet->retrieveShape(SUMO_TAG_POLY, value, false) == nullptr);
        case SUMO_ATTR_SHAPE:
        case SUMO_ATTR_GEOSHAPE:
            // empty shapes AREN'T allowed
            if (value.empty()) {
                return false;
            } else {
                return canParse<PositionVector>(value);
            }
        case SUMO_ATTR_COLOR:
            return canParse<RGBColor>(value);
        case SUMO_ATTR_FILL:
            return canParse<bool>(value);
        case SUMO_ATTR_LINEWIDTH:
            return canParse<double>(value) && (parse<double>(value) >= 0);
        case SUMO_ATTR_LAYER:
            if (value == "default") {
                return true;
            } else {
                return canParse<double>(value);
            }
        case SUMO_ATTR_TYPE:
            return true;
        case SUMO_ATTR_IMGFILE:
            if (value == "") {
                return true;
            } else {
                // check that image can be loaded
                return GUITexturesHelper::getTextureID(value) != -1;
            }
        case SUMO_ATTR_RELATIVEPATH:
            return canParse<bool>(value);
        case SUMO_ATTR_ANGLE:
            return canParse<double>(value);
        case SUMO_ATTR_GEO:
            return canParse<bool>(value);
        case GNE_ATTR_BLOCK_MOVEMENT:
            return canParse<bool>(value);
        case GNE_ATTR_BLOCK_SHAPE:
            return canParse<bool>(value);
        case GNE_ATTR_CLOSE_SHAPE:
            if (canParse<bool>(value)) {
                bool closePolygon = parse<bool>(value);
                if (closePolygon && (myShape.begin() == myShape.end())) {
                    // Polygon already closed, then invalid value
                    return false;
                } else if (!closePolygon && (myShape.begin() != myShape.end())) {
                    // Polygon already open, then invalid value
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

bool
GNEPoly::isAttributeEnabled(SumoXMLAttr /* key */) const {
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
GNEPoly::setAttribute(SumoXMLAttr key, const std::string& value) {
    // first remove object from grid due almost modificactions affects to boundary (but avoided for certain attributes)
    if ((key != SUMO_ATTR_ID) && (key != GNE_ATTR_PARAMETERS) && (key != GNE_ATTR_SELECTED)) {
        myNet->removeGLObjectFromGrid(this);
    }
    switch (key) {
        case SUMO_ATTR_ID: {
            // note: getAttributeCarriers().updateID doesn't change Microsim ID in GNEShapes
            myNet->getAttributeCarriers()->updateID(this, value);
            // set named ID
            myID = value;
            break;
        }
        case SUMO_ATTR_SHAPE: {
            // set new shape
            myShape = parse<PositionVector>(value);
            // set GEO shape
            myGeoShape = myShape;
            for (int i = 0; i < (int) myGeoShape.size(); i++) {
                GeoConvHelper::getFinal().cartesian2geo(myGeoShape[i]);
            }
            // disable simplified shape flag
            mySimplifiedShape = false;
            // update geometry
            updateGeometry();
            break;
        }
        case SUMO_ATTR_GEOSHAPE: {
            // set new GEO shape
            myGeoShape = parse<PositionVector>(value);
            // set shape
            myShape = myGeoShape ;
            for (int i = 0; i < (int) myShape.size(); i++) {
                GeoConvHelper::getFinal().x2cartesian_const(myShape[i]);
            }
            // disable simplified shape flag
            mySimplifiedShape = false;
            // update geometry
            updateGeometry();
            break;
        }
        case SUMO_ATTR_COLOR:
            setShapeColor(parse<RGBColor>(value));
            break;
        case SUMO_ATTR_FILL:
            myFill = parse<bool>(value);
            break;
        case SUMO_ATTR_LINEWIDTH:
            myLineWidth = parse<double>(value);
            break;
        case SUMO_ATTR_LAYER:
            if (value == "default") {
                setShapeLayer(Shape::DEFAULT_LAYER);
            } else {
                setShapeLayer(parse<double>(value));
            }
            break;
        case SUMO_ATTR_TYPE:
            setShapeType(value);
            break;
        case SUMO_ATTR_IMGFILE:
            setShapeImgFile(value);
            // all textures must be refresh
            GUITexturesHelper::clearTextures();
            break;
        case SUMO_ATTR_RELATIVEPATH:
            setShapeRelativePath(parse<bool>(value));
            break;
        case SUMO_ATTR_ANGLE:
            setShapeNaviDegree(parse<double>(value));
            break;
        case SUMO_ATTR_GEO:
            myGEO = parse<bool>(value);
            break;
        case GNE_ATTR_BLOCK_MOVEMENT:
            myBlockMovement = parse<bool>(value);
            break;
        case GNE_ATTR_BLOCK_SHAPE:
            myBlockShape = parse<bool>(value);
            break;
        case GNE_ATTR_CLOSE_SHAPE:
            if (parse<bool>(value)) {
                myShape.closePolygon();
                myGeoShape.closePolygon();
            } else {
                myShape.pop_back();
                myGeoShape.pop_back();
            }
            // disable simplified shape flag
            mySimplifiedShape = false;
            // update geometry
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
    // add object into grid again (but avoided for certain attributes)
    if ((key != SUMO_ATTR_ID) && (key != GNE_ATTR_PARAMETERS) && (key != GNE_ATTR_SELECTED)) {
        myNet->addGLObjectIntoGrid(this);
    }
}

/****************************************************************************/
