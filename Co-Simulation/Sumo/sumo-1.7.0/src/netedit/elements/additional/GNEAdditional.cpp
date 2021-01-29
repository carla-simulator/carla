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
/// @file    GNEAdditional.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Dec 2015
///
// A abstract class for representation of additional elements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/options/OptionsCont.h>

#include "GNEAdditional.h"

// ===========================================================================
// member method definitions
// ===========================================================================

GNEAdditional::GNEAdditional(const std::string& id, GNENet* net, GUIGlObjectType type, SumoXMLTag tag, std::string additionalName, bool blockMovement,
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
    myAdditionalName(additionalName),
    myBlockMovement(blockMovement),
    myBlockIcon(this),
    mySpecialColor(nullptr) {
}


GNEAdditional::GNEAdditional(GNENet* net, GUIGlObjectType type, SumoXMLTag tag, std::string additionalName, bool blockMovement,
                             const std::vector<GNEJunction*>& junctionParents,
                             const std::vector<GNEEdge*>& edgeParents,
                             const std::vector<GNELane*>& laneParents,
                             const std::vector<GNEAdditional*>& additionalParents,
                             const std::vector<GNEShape*>& shapeParents,
                             const std::vector<GNETAZElement*>& TAZElementParents,
                             const std::vector<GNEDemandElement*>& demandElementParents,
                             const std::vector<GNEGenericData*>& genericDataParents) :
    GUIGlObject(type, additionalParents.front()->getID()),
    GNEHierarchicalElement(net, tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    GNEPathElements(this),
    myAdditionalName(additionalName),
    myBlockMovement(blockMovement),
    myBlockIcon(this),
    mySpecialColor(nullptr) {
}


GNEAdditional::~GNEAdditional() {}


const std::string&
GNEAdditional::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNEAdditional::getGUIGlObject() {
    return this;
}


const GNEGeometry::Geometry&
GNEAdditional::getAdditionalGeometry() const {
    return myAdditionalGeometry;
}


const GNEGeometry::SegmentGeometry&
GNEAdditional::getAdditionalSegmentGeometry() const {
    return myAdditionalSegmentGeometry;
}


void
GNEAdditional::setSpecialColor(const RGBColor* color) {
    mySpecialColor = color;
}


void
GNEAdditional::writeAdditional(OutputDevice& device) const {
    // first check if minimum number of children is correct
    if ((myTagProperty.hasMinimumNumberOfChildren() || myTagProperty.hasMinimumNumberOfChildren()) && !checkChildAdditionalRestriction()) {
        WRITE_WARNING(getTagStr() + " with ID='" + getID() + "' cannot be written");
    } else {
        // Open Tag or synonym Tag
        if (myTagProperty.hasTagSynonym()) {
            device.openTag(myTagProperty.getTagSynonym());
        } else {
            device.openTag(myTagProperty.getTag());
        }
        // iterate over attribute properties
        for (const auto& attrProperty : myTagProperty) {
            // obtain attribute value
            const std::string attributeValue = getAttribute(attrProperty.getAttr());
            //  first check if attribute is optional but not vehicle classes
            if (attrProperty.isOptional() && !attrProperty.isVClasses()) {
                // Only write attributes with default value if is different from original
                if (attrProperty.getDefaultValue() != attributeValue) {
                    // check if attribute must be written using a synonim
                    if (attrProperty.hasAttrSynonym()) {
                        device.writeAttr(attrProperty.getAttrSynonym(), attributeValue);
                    } else {
                        // SVC permissions uses their own writting function
                        if (attrProperty.isSVCPermission()) {
                            // disallow attribute musn't be written
                            if (attrProperty.getAttr() != SUMO_ATTR_DISALLOW) {
                                writePermissions(device, parseVehicleClasses(attributeValue));
                            }
                        } else if (myTagProperty.canMaskXYZPositions() && (attrProperty.getAttr() == SUMO_ATTR_POSITION)) {
                            // get position attribute and write it separate
                            Position pos = parse<Position>(getAttribute(SUMO_ATTR_POSITION));
                            device.writeAttr(SUMO_ATTR_X, toString(pos.x()));
                            device.writeAttr(SUMO_ATTR_Y, toString(pos.y()));
                            // write 0 only if is different from 0 (the default value)
                            if (pos.z() != 0) {
                                device.writeAttr(SUMO_ATTR_Z, toString(pos.z()));
                            }
                        } else {
                            device.writeAttr(attrProperty.getAttr(), attributeValue);
                        }
                    }
                }
            } else {
                // Attributes without default values are always writted
                if (attrProperty.hasAttrSynonym()) {
                    device.writeAttr(attrProperty.getAttrSynonym(), attributeValue);
                } else {
                    // SVC permissions uses their own writting function
                    if (attrProperty.isSVCPermission()) {
                        // disallow attribute musn't be written
                        if (attrProperty.getAttr() != SUMO_ATTR_DISALLOW) {
                            writePermissions(device, parseVehicleClasses(attributeValue));
                        }
                    } else if (myTagProperty.canMaskXYZPositions() && (attrProperty.getAttr() == SUMO_ATTR_POSITION)) {
                        // get position attribute and write it separate
                        Position pos = parse<Position>(getAttribute(SUMO_ATTR_POSITION));
                        device.writeAttr(SUMO_ATTR_X, toString(pos.x()));
                        device.writeAttr(SUMO_ATTR_Y, toString(pos.y()));
                        // write 0 only if is different from 0 (the default value)
                        if (pos.z() != 0) {
                            device.writeAttr(SUMO_ATTR_Z, toString(pos.z()));
                        }
                    } else {
                        device.writeAttr(attrProperty.getAttr(), attributeValue);
                    }
                }
            }
        }
        // iterate over children and write it in XML (or in a different file)
        if (myTagProperty.canWriteChildrenSeparate() && myTagProperty.hasAttribute(SUMO_ATTR_FILE) && !getAttribute(SUMO_ATTR_FILE).empty()) {
            // we assume that rerouter values files is placed in the same folder as the additional file
            OutputDevice& deviceChildren = OutputDevice::getDevice(FileHelpers::getFilePath(OptionsCont::getOptions().getString("additional-files")) + getAttribute(SUMO_ATTR_FILE));
            deviceChildren.writeXMLHeader("rerouterValue", "additional_file.xsd");
            // save children in a different filename
            for (const auto& additionalChild : getChildAdditionals()) {
                // avoid to write two times additionals that haben two parents (Only write as child of first parent)
                if (additionalChild->getParentAdditionals().size() < 1) {
                    additionalChild->writeAdditional(deviceChildren);
                } else if (myTagProperty.getTag() == additionalChild->getTagProperty().getMasterTags().front()) {
                    additionalChild->writeAdditional(deviceChildren);
                }
            }
            deviceChildren.close();
        } else {
            for (const auto& additionalChild : getChildAdditionals()) {
                // avoid write symbols
                if (!additionalChild->getTagProperty().isSymbol()) {
                    // avoid to write two times additionals that haben two parents (Only write as child of first parent)
                    if (additionalChild->getParentAdditionals().size() < 2) {
                        additionalChild->writeAdditional(device);
                    } else if (myTagProperty.getTag() == additionalChild->getTagProperty().getMasterTags().front()) {
                        additionalChild->writeAdditional(device);
                    }
                }
            }
        }
        // write parameters (Always after children to avoid problems with additionals.xsd)
        writeParams(device);
        // Close tag
        device.closeTag();
    }
}


bool
GNEAdditional::isAdditionalValid() const {
    return true;
}


std::string
GNEAdditional::getAdditionalProblem() const {
    return "";
}


void
GNEAdditional::fixAdditionalProblem() {
    throw InvalidArgument(getTagStr() + " cannot fix any problem");
}


void
GNEAdditional::openAdditionalDialog() {
    throw InvalidArgument(getTagStr() + " doesn't have an additional dialog");
}


void
GNEAdditional::startGeometryMoving() {
    // only move if additional is drawable
    if (myTagProperty.isDrawable()) {
        // always save original position over view
        myMove.originalViewPosition = getPositionInView();
        // check if position over lane or lanes has to be saved
        if (myTagProperty.hasAttribute(SUMO_ATTR_LANE)) {
            if (myTagProperty.canMaskStartEndPos()) {
                // obtain start and end position
                myMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_STARTPOS);
                myMove.secondOriginalPosition = getAttribute(SUMO_ATTR_ENDPOS);
            } else {
                // obtain position attribute
                myMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_POSITION);
            }
        } else if (myTagProperty.hasAttribute(SUMO_ATTR_LANES) &&
                   myTagProperty.hasAttribute(SUMO_ATTR_POSITION) &&
                   myTagProperty.hasAttribute(SUMO_ATTR_ENDPOS)) {
            // obtain start and end position
            myMove.firstOriginalLanePosition = getAttribute(SUMO_ATTR_POSITION);
            myMove.secondOriginalPosition = getAttribute(SUMO_ATTR_ENDPOS);
        }
        // save current centering boundary if element is placed in RTree
        if (myTagProperty.isPlacedInRTree()) {
            myMove.movingGeometryBoundary = getCenteringBoundary();
        }
        // start geometry in all children
        for (const auto& i : getChildDemandElements()) {
            i->startGeometryMoving();
        }
    }
}


void
GNEAdditional::endGeometryMoving() {
    // check that endGeometryMoving was called only once
    if (myTagProperty.isDrawable()) {
        // check if object must be placed in RTREE
        if (myTagProperty.isPlacedInRTree()) {
            // Remove object from net
            myNet->removeGLObjectFromGrid(this);
            // reset myMovingGeometryBoundary
            myMove.movingGeometryBoundary.reset();
            // add object into grid again (using the new centering boundary)
            myNet->addGLObjectIntoGrid(this);
        }
        // end geometry in all children
        for (const auto& i : getChildDemandElements()) {
            i->endGeometryMoving();
        }
    }
}


bool
GNEAdditional::isAdditionalBlocked() const {
    return myBlockMovement;
}


void
GNEAdditional::updatePartialGeometry(const GNELane* lane) {
    // currently only for E2 Multilane Detectors
    if (myTagProperty.getTag() == SUMO_TAG_E2DETECTOR_MULTILANE) {
        // declare extreme geometry
        GNEGeometry::ExtremeGeometry extremeGeometry;
        // get extremes
        extremeGeometry.laneStartPosition = GNEAttributeCarrier::parse<double>(getAttribute(SUMO_ATTR_POSITION));
        extremeGeometry.laneEndPosition = GNEAttributeCarrier::parse<double>(getAttribute(SUMO_ATTR_ENDPOS));
        // update geometry path for the given lane
        GNEGeometry::updateGeometricPath(myAdditionalSegmentGeometry, lane, extremeGeometry);
    }
}


GUIGLObjectPopupMenu*
GNEAdditional::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
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
    // show option to open additional dialog
    if (myTagProperty.hasDialog()) {
        new FXMenuCommand(ret, ("Open " + getTagStr() + " Dialog").c_str(), getIcon(), &parent, MID_OPEN_ADDITIONAL_DIALOG);
        new FXMenuSeparator(ret);
    }
    // Show position parameters
    if (myTagProperty.hasAttribute(SUMO_ATTR_LANE) && (myAdditionalGeometry.getShape().size() > 1)) {
        const GNELane* lane = myNet->retrieveLane(getAttribute(SUMO_ATTR_LANE));
        // Show menu command inner position
        const double innerPos = myAdditionalGeometry.getShape().nearest_offset_to_point2D(parent.getPositionInformation());
        new FXMenuCommand(ret, ("Cursor position over additional shape: " + toString(innerPos)).c_str(), nullptr, nullptr, 0);
        // If shape isn't empty, show menu command lane position
        if (myAdditionalGeometry.getShape().size() > 0) {
            const double lanePos = lane->getLaneShape().nearest_offset_to_point2D(myAdditionalGeometry.getShape().front());
            new FXMenuCommand(ret, ("Cursor position over " + toString(SUMO_TAG_LANE) + ": " + toString(innerPos + lanePos)).c_str(), nullptr, nullptr, 0);
        }
    } else if (myTagProperty.hasAttribute(SUMO_ATTR_EDGE) && (myAdditionalGeometry.getShape().size() > 1)) {
        const GNEEdge* edge = myNet->retrieveEdge(getAttribute(SUMO_ATTR_EDGE));
        // Show menu command inner position
        const double innerPos = myAdditionalGeometry.getShape().nearest_offset_to_point2D(parent.getPositionInformation());
        new FXMenuCommand(ret, ("Cursor position over additional shape: " + toString(innerPos)).c_str(), nullptr, nullptr, 0);
        // If shape isn't empty, show menu command edge position
        if (myAdditionalGeometry.getShape().size() > 0) {
            const double edgePos = edge->getLanes().at(0)->getLaneShape().nearest_offset_to_point2D(myAdditionalGeometry.getShape().front());
            new FXMenuCommand(ret, ("Mouse position over " + toString(SUMO_TAG_EDGE) + ": " + toString(innerPos + edgePos)).c_str(), nullptr, nullptr, 0);
        }
    } else {
        new FXMenuCommand(ret, ("Cursor position in view: " + toString(getPositionInView().x()) + "," + toString(getPositionInView().y())).c_str(), nullptr, nullptr, 0);
    }
    return ret;
}


GUIParameterTableWindow*
GNEAdditional::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
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


const std::string&
GNEAdditional::getOptionalAdditionalName() const {
    return myAdditionalName;
}


void
GNEAdditional::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* lane, const double offsetFront) const {
    // calculate E2Detector width
    const double E2DetectorWidth = s.addSize.getExaggeration(s, lane);
    // check if E2 can be drawn
    if (s.drawAdditionals(E2DetectorWidth) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // obtain color
        const RGBColor routeColor = drawUsingSelectColor() ? s.colorSettings.selectedAdditionalColor : s.detectorSettings.E2Color;
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType() + offsetFront);
        // iterate over segments
        for (const auto& segment : myAdditionalSegmentGeometry) {
            // draw partial segment
            if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                // Set route color (needed due drawShapeDottedContour)
                GLHelper::setColor(routeColor);
                // draw box lines
                GNEGeometry::drawSegmentGeometry(myNet->getViewNet(), segment, E2DetectorWidth);
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
            // iterate over segments
            for (const auto& segment : myAdditionalSegmentGeometry) {
                if (segment.isLaneSegment() && (segment.getLane() == lane)) {
                    // draw partial segment
                    if (getParentLanes().front() == lane) {
                        // draw front dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), E2DetectorWidth, true, false);
                    } else if (getParentLanes().back() == lane) {
                        // draw back dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, GNEGeometry::DottedGeometry(s, segment.getShape(), false), E2DetectorWidth, false, true);
                    } else {
                        // draw dotted contour
                        GNEGeometry::drawDottedContourLane(true, s, lane->getDottedLaneGeometry(), E2DetectorWidth, false, false);
                    }
                }
            }
        }
    }
}


void
GNEAdditional::drawPartialGL(const GUIVisualizationSettings& s, const GNELane* fromLane, const GNELane* toLane, const double offsetFront) const {
    // calculate E2Detector width
    const double E2DetectorWidth = s.addSize.getExaggeration(s, fromLane);
    // check if E2 can be drawn
    if (s.drawAdditionals(E2DetectorWidth) && myNet->getViewNet()->getDataViewOptions().showAdditionals()) {
        // Start drawing adding an gl identificator
        glPushName(getGlID());
        // Add a draw matrix
        glPushMatrix();
        // Start with the drawing of the area traslating matrix to origin
        glTranslated(0, 0, getType() + offsetFront);
        // Set color of the base
        if (drawUsingSelectColor()) {
            GLHelper::setColor(s.colorSettings.selectedAdditionalColor);
        } else {
            GLHelper::setColor(s.detectorSettings.E2Color);
        }
        // draw lane2lane
        if (fromLane->getLane2laneConnections().exist(toLane)) {
            GNEGeometry::drawGeometry(myNet->getViewNet(), fromLane->getLane2laneConnections().getLane2laneGeometry(toLane), E2DetectorWidth);
        } else {
            // Set invalid person plan color
            GLHelper::setColor(RGBColor::RED);
            // draw line between end of first shape and first position of second shape
            GLHelper::drawBoxLines({fromLane->getLaneShape().back(), toLane->getLaneShape().front()}, (0.5 * E2DetectorWidth));
        }
        // Pop last matrix
        glPopMatrix();
        // Pop name
        glPopName();
        // check if shape dotted contour has to be drawn
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            // draw lane2lane dotted geometry
            if (fromLane->getLane2laneConnections().exist(toLane)) {
                GNEGeometry::drawDottedContourLane(true, s, fromLane->getLane2laneConnections().getLane2laneDottedGeometry(toLane), E2DetectorWidth, false, false);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// GNEAdditional::BlockIcon - methods
// ---------------------------------------------------------------------------

GNEAdditional::BlockIcon::BlockIcon(GNEAdditional* additional) :
    myAdditional(additional),
    myRotation(0) {}


void
GNEAdditional::BlockIcon::updatePositionAndRotation() {
    if (myAdditional->getAdditionalGeometry().getShape().size() > 1) {
        const double middlePos = myAdditional->getAdditionalGeometry().getShape().length2D() * 0.5;
        // calculate position and rotation
        myPosition = myAdditional->getAdditionalGeometry().getShape().positionAtOffset2D(middlePos);
        myRotation = myAdditional->getAdditionalGeometry().getShape().rotationDegreeAtOffset(middlePos);
    } else {
        // get position and rotation of additional geometry
        myPosition = myAdditional->getAdditionalGeometry().getPosition();
        myRotation = myAdditional->getAdditionalGeometry().getRotation();
    }
}


void
GNEAdditional::BlockIcon::setOffset(const double x, const double y) {
    myOffset.setx(x);
    myOffset.sety(y);
}


void
GNEAdditional::BlockIcon::drawIcon(const GUIVisualizationSettings& s, const double exaggeration, const double size) const {
    // check if block icon can be draw
    if ((myPosition != Position::INVALID) &&
            !s.drawForPositionSelection && !s.drawForRectangleSelection &&
            s.drawDetail(s.detailSettings.lockIcon, exaggeration) &&
            myAdditional->myNet->getViewNet()->showLockIcon()) {
        // get texture
        GUIGlID lockTexture = 0;
        // Draw icon depending of the state of additional
        if (myAdditional->drawUsingSelectColor()) {
            if (!myAdditional->getTagProperty().canBlockMovement()) {
                // Draw not movable texture if additional isn't movable and is selected
                lockTexture = GUITextureSubSys::getTexture(GNETEXTURE_NOTMOVINGSELECTED);
            } else if (myAdditional->myBlockMovement) {
                // Draw lock texture if additional is movable, is blocked and is selected
                lockTexture = GUITextureSubSys::getTexture(GNETEXTURE_LOCKSELECTED);
            } else {
                // Draw empty texture if additional is movable, isn't blocked and is selected
                lockTexture = GUITextureSubSys::getTexture(GNETEXTURE_EMPTYSELECTED);
            }
        } else {
            if (!myAdditional->getTagProperty().canBlockMovement()) {
                // Draw not movable texture if additional isn't movable
                lockTexture = GUITextureSubSys::getTexture(GNETEXTURE_NOTMOVING);
            } else if (myAdditional->myBlockMovement) {
                // Draw lock texture if additional is movable and is blocked
                lockTexture = GUITextureSubSys::getTexture(GNETEXTURE_LOCK);
            } else {
                // Draw empty texture if additional is movable and isn't blocked
                lockTexture = GUITextureSubSys::getTexture(GNETEXTURE_EMPTY);
            }
        }
        // Start pushing matrix
        glPushMatrix();
        // Traslate to middle of shape
        glTranslated(myPosition.x(), myPosition.y(), 0.1);
        // Set draw color
        glColor3d(1, 1, 1);
        // Rotate depending of rotation
        glRotated((myRotation * -1) + 90, 0, 0, 1);
        // Traslate depending of the offset
        glTranslated(myOffset.x(), myOffset.y(), 0);
        // Rotate again
        glRotated(180, 0, 0, 1);
        // Draw lock icon
        GUITexturesHelper::drawTexturedBox(lockTexture, size);
        // Pop matrix
        glPopMatrix();
    }
}


const Position&
GNEAdditional::BlockIcon::getPosition() const {
    return myPosition;
}


double
GNEAdditional::BlockIcon::getRotation() const {
    return myRotation;
}

// ---------------------------------------------------------------------------
// GNEAdditional - protected methods
// ---------------------------------------------------------------------------

void
GNEAdditional::setDefaultValues() {
    // iterate over attributes and set default value
    for (const auto& i : myTagProperty) {
        if (i.hasStaticDefaultValue()) {
            setAttribute(i.getAttr(), i.getDefaultValue());
        }
    }
}


bool
GNEAdditional::isValidAdditionalID(const std::string& newID) const {
    if (SUMOXMLDefinitions::isValidAdditionalID(newID) && (myNet->retrieveAdditional(myTagProperty.getTag(), newID, false) == nullptr)) {
        return true;
    } else {
        return false;
    }
}


bool
GNEAdditional::isValidDetectorID(const std::string& newID) const {
    if (SUMOXMLDefinitions::isValidDetectorID(newID) && (myNet->retrieveAdditional(myTagProperty.getTag(), newID, false) == nullptr)) {
        return true;
    } else {
        return false;
    }
}


void
GNEAdditional::drawAdditionalName(const GUIVisualizationSettings& s) const {
    if (s.addFullName.show && (myAdditionalName != "") && !s.drawForRectangleSelection && !s.drawForPositionSelection) {
        GLHelper::drawText(myAdditionalName, myBlockIcon.getPosition(), GLO_MAX - getType(), s.addFullName.scaledSize(s.scale), s.addFullName.color, myBlockIcon.getRotation());
    }
}


void
GNEAdditional::replaceAdditionalParentEdges(const std::string& value) {
    replaceParentElements(this, parse<std::vector<GNEEdge*> >(getNet(), value));
}


void
GNEAdditional::replaceAdditionalParentLanes(const std::string& value) {
    replaceParentElements(this, parse<std::vector<GNELane*> >(getNet(), value));
}


void
GNEAdditional::replaceAdditionalChildEdges(const std::string& value) {
    replaceChildElements(this, parse<std::vector<GNEEdge*> >(getNet(), value));
}


void
GNEAdditional::replaceAdditionalChildLanes(const std::string& value) {
    replaceChildElements(this, parse<std::vector<GNELane*> >(getNet(), value));
}


void
GNEAdditional::replaceAdditionalParent(SumoXMLTag tag, const std::string& value, const int parentIndex) {
    std::vector<GNEAdditional*> parentAdditionals = getParentAdditionals();
    parentAdditionals[parentIndex] = myNet->retrieveAdditional(tag, value);
    // replace parent additionals
    replaceParentElements(this, parentAdditionals);
}


void
GNEAdditional::replaceDemandElementParent(SumoXMLTag tag, const std::string& value, const int parentIndex) {
    std::vector<GNEDemandElement*> parentDemandElements = getParentDemandElements();
    parentDemandElements[parentIndex] = myNet->retrieveDemandElement(tag, value);
    // replace parent demand elements
    replaceParentElements(this, parentDemandElements);
}


void
GNEAdditional::calculatePerpendicularLine(const double endLaneposition) {
    if (getParentEdges().empty()) {
        throw ProcessError("Invalid number of edges");
    } else {
        // get lanes
        const GNELane* firstLane = getParentEdges().front()->getLanes().front();
        const GNELane* lastLane = getParentEdges().front()->getLanes().back();
        // get first and back lane shapes
        PositionVector firstLaneShape = firstLane->getLaneShape();
        PositionVector lastLaneShape = lastLane->getLaneShape();
        // move shapes
        firstLaneShape.move2side((firstLane->getParentEdge()->getNBEdge()->getLaneWidth(firstLane->getIndex()) * 0.5) + 1);
        lastLaneShape.move2side(lastLane->getParentEdge()->getNBEdge()->getLaneWidth(lastLane->getIndex()) * -0.5);
        // calculate lane postion
        const double lanePosition = firstLaneShape.length2D() >= endLaneposition ? endLaneposition : firstLaneShape.length2D();
        // update geometry
        myAdditionalGeometry.updateGeometry({firstLaneShape.positionAtOffset2D(lanePosition), lastLaneShape.positionAtOffset2D(lanePosition)});
    }
}


void
GNEAdditional::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNEAdditional::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


bool
GNEAdditional::checkChildAdditionalRestriction() const {
    // throw exception because this function mus be implemented in child (see GNEE3Detector)
    throw ProcessError("Calling non-implemented function checkChildAdditionalRestriction during saving of " + getTagStr() + ". It muss be reimplemented in child class");
}


void
GNEAdditional::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
