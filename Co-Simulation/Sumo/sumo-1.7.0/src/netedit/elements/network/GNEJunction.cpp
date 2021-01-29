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
/// @file    GNEJunction.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// A class for visualizing and editing junctions in netedit (adapted from
// GUIJunctionWrapper)
/****************************************************************************/
#include <config.h>

#include <netbuild/NBAlgorithms.h>
#include <netbuild/NBLoadedSUMOTLDef.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBOwnTLDef.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/changes/GNEChange_Connection.h>
#include <netedit/changes/GNEChange_TLS.h>
#include <netedit/elements/additional/GNEAdditional.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/images/GUITextureSubSys.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/options/OptionsCont.h>

#include "GNEEdge.h"
#include "GNELane.h"
#include "GNEConnection.h"
#include "GNEJunction.h"
#include "GNECrossing.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNEJunction::GNEJunction(GNENet* net, NBNode* nbn, bool loaded) :
    GNENetworkElement(net, nbn->getID(), GLO_JUNCTION, SUMO_TAG_JUNCTION,
{}, {}, {}, {}, {}, {}, {}, {}),
myNBNode(nbn),
myMaxDrawingSize(1),
myAmCreateEdgeSource(false),
myLogicStatus(loaded ? FEATURE_LOADED : FEATURE_GUESSED),
myAmResponsible(false),
myHasValidLogic(loaded),
myAmTLSSelected(false),
myColorForMissingConnections(false) {
}


GNEJunction::~GNEJunction() {
    // delete all GNECrossing
    for (const auto& crossing : myGNECrossings) {
        crossing->decRef();
        if (crossing->unreferenced()) {
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + crossing->getTagStr() + " '" + crossing->getID() + "' in GNEJunction destructor");
            delete crossing;
        }
    }

    if (myAmResponsible) {
        // show extra information for tests
        WRITE_DEBUG("Deleting NBNode of '" + getID() + "' in GNEJunction destructor");
        delete myNBNode;
    }
}


const PositionVector&
GNEJunction::getJunctionShape() const {
    return myNBNode->getShape();
}


void
GNEJunction::updateGeometry() {
    updateGeometryAfterNetbuild(true);
}


void
GNEJunction::updateGeometryAfterNetbuild(bool rebuildNBNodeCrossings) {
    // recalc max drawing size
    myMaxDrawingSize = MAX2(getCenteringBoundary().getWidth(), getCenteringBoundary().getHeight());
    rebuildGNECrossings(rebuildNBNodeCrossings);
    checkMissingConnections();
}


Position
GNEJunction::getPositionInView() const {
    return myNBNode->getPosition();
}


void
GNEJunction::startJunctionShapeGeometryMoving(const double shapeOffset) {
    // save current centering boundary
    myMovingGeometryBoundary = getCenteringBoundary();
    // start move shape depending of block shape
    startMoveShape(myNBNode->getShape(), shapeOffset, myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.junctionGeometryPointRadius);
}


void
GNEJunction::endJunctionShapeGeometryMoving() {
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
GNEJunction::getJunctionShapeVertexIndex(Position pos, const bool snapToGrid) const {
    // get shape
    const PositionVector shape = myNBNode->getShape();
    // check shape size
    if (shape.size() == 0) {
        return -1;
    }
    // check if position has to be snapped to grid
    if (snapToGrid) {
        pos = myNet->getViewNet()->snapToActiveGrid(pos);
    }
    const double offset = shape.nearest_offset_to_point2D(pos, true);
    if (offset == GeomHelper::INVALID_OFFSET) {
        // check if we clicked over start or end position
        if (shape.front().distanceTo2D(pos) < myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.junctionGeometryPointRadius) {
            return 0;
        } else if (shape.back().distanceTo2D(pos) < myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.junctionGeometryPointRadius) {
            return (int)shape.size() - 1;
        } else {
            return -1;
        }
    }
    Position newPos = shape.positionAtOffset2D(offset);
    // first check if vertex already exists in the inner geometry
    for (int i = 0; i < (int)shape.size(); i++) {
        if (shape[i].distanceTo2D(newPos) < myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.junctionGeometryPointRadius) {
            return i;
        }
    }
    return -1;
}


void
GNEJunction::moveJunctionShape(const Position& offset) {
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
    // set new shape
    myNBNode->setCustomShape(newShape);
    // update geometry
    updateGeometry();
}


void
GNEJunction::commitJunctionShapeChange(GNEUndoList* undoList) {
    // get visualisation settings
    auto& s = myNet->getViewNet()->getVisualisationSettings();
    // restore original shape into shapeToCommit
    PositionVector shapeToCommit = parse<PositionVector>(getAttribute(SUMO_ATTR_SHAPE));
    // get geometryPoint radius
    const double geometryPointRadius = s.neteditSizeSettings.junctionGeometryPointRadius * s.polySize.getExaggeration(s, this);
    // remove double points
    shapeToCommit.removeDoublePoints(geometryPointRadius);
    // check if we have to merge start and end points
    if ((shapeToCommit.front() != shapeToCommit.back()) && (shapeToCommit.front().distanceTo2D(shapeToCommit.back()) < geometryPointRadius)) {
        shapeToCommit[0] = shapeToCommit.back();
    }
    // update geometry
    updateGeometry();
    // restore old geometry to allow change attribute (And restore shape if during movement a new point was created
    myNBNode->setCustomShape(getShapeBeforeMoving());
    // finish geometry moving
    endJunctionShapeGeometryMoving();
    // commit new shape
    undoList->p_begin("moving " + toString(SUMO_ATTR_SHAPE) + " of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_SHAPE, toString(shapeToCommit)));
    undoList->p_end();
}


void
GNEJunction::deleteJunctionShapeGeometryPoint(const Position& mousePosition, GNEUndoList* undoList) {
    // get a copy of shape
    PositionVector newShape = parse<PositionVector>(getAttribute(SUMO_ATTR_SHAPE));
    // obtain index
    const int index = getJunctionShapeVertexIndex(mousePosition, false);
    // check index
    if ((index != -1) && (newShape.size() > 3)) {
        // remove geometry point
        newShape.erase(newShape.begin() + index);
        // set new shape
        undoList->p_begin("delete geometry point of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_SHAPE, toString(newShape)));
        undoList->p_end();
    }
}


void
GNEJunction::rebuildGNECrossings(bool rebuildNBNodeCrossings) {
    // rebuild GNECrossings only if create crossings and walkingAreas in net is enabled
    if (myNet->getNetBuilder()->haveNetworkCrossings()) {
        if (rebuildNBNodeCrossings) {
            // build new NBNode::Crossings and walking areas
            mirrorXLeftHand();
            myNBNode->buildCrossingsAndWalkingAreas();
            mirrorXLeftHand();
        }
        // create a vector to keep retrieved and created crossings
        std::vector<GNECrossing*> retrievedCrossings;
        // iterate over NBNode::Crossings of GNEJunction
        for (const auto& crossing : myNBNode->getCrossingsIncludingInvalid()) {
            // retrieve existent GNECrossing, or create it
            GNECrossing* retrievedGNECrossing = retrieveGNECrossing(crossing.get());
            retrievedCrossings.push_back(retrievedGNECrossing);
            // check if previously this GNECrossings exists, and if true, remove it from myGNECrossings and insert in tree again
            std::vector<GNECrossing*>::iterator retrievedExists = std::find(myGNECrossings.begin(), myGNECrossings.end(), retrievedGNECrossing);
            if (retrievedExists != myGNECrossings.end()) {
                myGNECrossings.erase(retrievedExists);
                // update geometry of retrieved crossing
                retrievedGNECrossing->updateGeometry();
            } else {
                // include reference to created GNECrossing
                retrievedGNECrossing->incRef();
            }
        }
        // delete non retrieved GNECrossings (we don't need to extract if from Tree two times)
        for (const auto& crossing : myGNECrossings) {
            crossing->decRef();
            // check if crossing is selected
            if (crossing->isAttributeCarrierSelected()) {
                crossing->unselectAttributeCarrier();
            }
            if (crossing->unreferenced()) {
                // remove it from net
                myNet->removeGLObjectFromGrid(crossing);
                // show extra information for tests
                WRITE_DEBUG("Deleting unreferenced " + crossing->getTagStr() + " in rebuildGNECrossings()");
                delete crossing;
            }
        }
        // copy retrieved (existent and created) GNECrossigns to myGNECrossings
        myGNECrossings = retrievedCrossings;
    }
}

void
GNEJunction::mirrorXLeftHand() {
    if (OptionsCont::getOptions().getBool("lefthand")) {
        myNBNode->mirrorX();
        for (NBEdge* e : myNBNode->getEdges()) {
            e->mirrorX();

        }
    }
}


GUIGLObjectPopupMenu*
GNEJunction::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    // check if we're in supermode network
    if (myNet->getViewNet()->getEditModes().isCurrentSupermodeNetwork()) {
        //if (parent.getVisualisationSettings()->editMode != GNE_MODE_CONNECT) {
        //    // XXX if joinable
        //    new FXMenuCommand(ret, "Join adjacent edges", 0, &parent, MID_GNE_JOIN_EDGES);
        //}
        const int numEndpoints = (int)myNBNode->getEndPoints().size();
        // check if we're handling a selection
        bool handlingSelection = isAttributeCarrierSelected() && (myNet->retrieveJunctions(true).size() > 1);
        // check if menu commands has to be disabled
        const bool wrongMode = (myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_CONNECT) ||
                               (myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_TLS) ||
                               (myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_CREATE_EDGE);
        // create menu commands
        FXMenuCommand* mcCustomShape = new FXMenuCommand(ret, "Set custom junction shape", nullptr, &parent, MID_GNE_JUNCTION_EDIT_SHAPE);
        FXMenuCommand* mcResetCustomShape = new FXMenuCommand(ret, "Reset junction shape", nullptr, &parent, MID_GNE_JUNCTION_RESET_SHAPE);
        FXMenuCommand* mcReplace = new FXMenuCommand(ret, "Replace junction by geometry point", nullptr, &parent, MID_GNE_JUNCTION_REPLACE);
        FXMenuCommand* mcSplit = new FXMenuCommand(ret, ("Split junction (" + toString(numEndpoints) + " end points)").c_str(), nullptr, &parent, MID_GNE_JUNCTION_SPLIT);
        FXMenuCommand* mcSplitReconnect = new FXMenuCommand(ret, "Split junction and reconnect", nullptr, &parent, MID_GNE_JUNCTION_SPLIT_RECONNECT);
        if (myNBNode->isRoundabout()) {
            new FXMenuCommand(ret, "Select roundabout", nullptr, &parent, MID_GNE_JUNCTION_SELECT_ROUNDABOUT);
        } else {
            double radius = myNBNode->getRadius();
            if (radius == NBNode::UNSPECIFIED_RADIUS) {
                radius = OptionsCont::getOptions().getFloat("default.junctions.radius");
            }
            const std::string menuEntry = "Convert to roundabout (using junction radius " + toString(radius) + ")";
            FXMenuCommand* mcRoundabout = new FXMenuCommand(ret, menuEntry.c_str(), nullptr, &parent, MID_GNE_JUNCTION_CONVERT_ROUNDABOUT);
            if (getChildEdges().size() < 2 ||
                    (myGNEIncomingEdges.size() == 1
                     && myGNEOutgoingEdges.size() == 1
                     && myGNEIncomingEdges[0]->getParentJunctions().front() == myGNEOutgoingEdges[0]->getParentJunctions().back())) {
                mcRoundabout->disable();
            }
        }
        FXMenuCommand* mcClearConnections = new FXMenuCommand(ret, "Clear connections", nullptr, &parent, MID_GNE_JUNCTION_CLEAR_CONNECTIONS);
        FXMenuCommand* mcResetConnections = new FXMenuCommand(ret, "Reset connections", nullptr, &parent, MID_GNE_JUNCTION_RESET_CONNECTIONS);
        // check if current mode  is correct
        if (wrongMode) {
            mcCustomShape->disable();
            mcClearConnections->disable();
            mcResetConnections->disable();
        }
        // check if we're handling a selection
        if (handlingSelection) {
            mcResetCustomShape->setText("Reset junction shapes");
        }
        // disable mcClearConnections if juction hasn't connections
        if (getGNEConnections().empty()) {
            mcClearConnections->disable();
        }
        // disable mcResetCustomShape if junction doesn't have a custom shape
        if (myNBNode->getShape().size() == 0) {
            mcResetCustomShape->disable();
        }
        // checkIsRemovable requiers turnarounds to be computed. This is ugly
        if ((myNBNode->getIncomingEdges().size() == 2) && (myNBNode->getOutgoingEdges().size() == 2)) {
            NBTurningDirectionsComputer::computeTurnDirectionsForNode(myNBNode, false);
        }
        std::string reason = "wrong edit mode";
        if (wrongMode || !myNBNode->checkIsRemovableReporting(reason)) {
            mcReplace->setText(mcReplace->getText() + " (" + reason.c_str() + ")");
            mcReplace->disable();
        }
        if (numEndpoints == 1) {
            mcSplit->disable();
            mcSplitReconnect->disable();
        }
    }
    return ret;
}


Boundary
GNEJunction::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMovingGeometryBoundary.isInitialised()) {
        return myMovingGeometryBoundary;
    } else if (myNBNode->getShape().size() > 0) {
        Boundary b = myNBNode->getShape().getBoxBoundary();
        b.grow(10);
        return b;
    } else {
        // calculate boundary using EXTENT as size
        const double EXTENT = 2;
        Boundary b(myNBNode->getPosition().x() - EXTENT, myNBNode->getPosition().y() - EXTENT,
                   myNBNode->getPosition().x() + EXTENT, myNBNode->getPosition().y() + EXTENT);
        b.grow(10);
        return b;
    }
}


void
GNEJunction::drawGL(const GUIVisualizationSettings& s) const {
    // check if boundary has to be drawn
    if (s.drawBoundaries) {
        GLHelper::drawBoundary(getCenteringBoundary());
    }
    // declare variables
    const Position mousePosition = myNet->getViewNet()->getPositionInformation();
    const double junctionExaggeration = s.junctionSize.getExaggeration(s, this, 4);
    const double bubbleRadius = s.neteditSizeSettings.junctionBubbleRadius * junctionExaggeration;
    // declare draw shape flag
    const bool drawShape = (myNBNode->getShape().size() > 0) && s.drawJunctionShape;
    // declare draw bubble flag
    bool drawBubble = true;
    if (!s.drawJunctionShape) {
        // don't draw bubble if it was disabled in GUIVisualizationSettings
        drawBubble = false;
    }
    if (myNBNode->getShape().area() > 4) {
        // don't draw if shape area is greather than 4
        drawBubble = false;
    }
    if (!myNet->getViewNet()->getEditModes().isCurrentSupermodeNetwork()) {
        // only draw bubbles in network mode
        drawBubble = false;
    }
    if (myNet->getViewNet()->showJunctionAsBubbles()) {
        // force draw bubbles if we enabled option in checkbox of viewNet
        drawBubble = true;
    }
    // only continue if exaggeration is greather than 0
    if (junctionExaggeration > 0) {
        // push junction name
        glPushName(getGlID());
        // push layer matrix
        glPushMatrix();
        // translate to front
        if (myAmCreateEdgeSource) {
            glTranslated(0, 0, GLO_TEMPORALSHAPE);
        } else {
            myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, GLO_JUNCTION);
        }
        // push name
        if (s.scale * junctionExaggeration * myMaxDrawingSize < 1.) {
            // draw something simple so that selection still works
            GLHelper::drawBoxLine(myNBNode->getPosition(), 0, 1, 1);
        } else {
            // check if shape has to be drawn
            if (drawShape) {
                // set shape color
                const RGBColor junctionShapeColor = setColor(s, false);
                // recognize full transparency and simply don't draw
                if (junctionShapeColor.alpha() != 0) {
                    // set color
                    GLHelper::setColor(junctionShapeColor);
                    // obtain junction Shape
                    PositionVector junctionClosedShape = myNBNode->getShape();
                    // close junction shape
                    junctionClosedShape.closePolygon();
                    // adjust shape to exaggeration
                    if (junctionExaggeration > 1) {
                        junctionClosedShape.scaleRelative(junctionExaggeration);
                    }
                    // first check if inner junction polygon can be drawn
                    if (s.drawForPositionSelection) {
                        // only draw a point if mouse is around shape
                        if (junctionClosedShape.around(mousePosition)) {
                            // push matrix
                            glPushMatrix();
                            // move to mouse position
                            glTranslated(mousePosition.x(), mousePosition.y(), 0.1);
                            // draw a simple circle
                            GLHelper::drawFilledCircle(1, s.getCircleResolution());
                            // pop matrix
                            glPopMatrix();
                        }
                    } else if ((s.scale * junctionExaggeration * myMaxDrawingSize) < 40.) {
                        // draw shape
                        GLHelper::drawFilledPoly(junctionClosedShape, true);
                    } else {
                        // draw shape with high detail
                        GLHelper::drawFilledPolyTesselated(junctionClosedShape, true);
                    }
                    // draw shape points only in Network supemode
                    if (myShapeEdited && s.drawMovingGeometryPoint(junctionExaggeration, s.neteditSizeSettings.junctionGeometryPointRadius) && myNet->getViewNet()->getEditModes().isCurrentSupermodeNetwork()) {
                        // set color
                        const RGBColor darkerColor = junctionShapeColor.changedBrightness(-32);
                        // calculate geometry
                        GNEGeometry::Geometry junctionGeometry;
                        // obtain junction Shape
                        PositionVector junctionOpenShape = myNBNode->getShape();
                        // adjust shape to exaggeration
                        if (junctionExaggeration > 1) {
                            junctionOpenShape.scaleRelative(junctionExaggeration);
                        }
                        // update geometry
                        junctionGeometry.updateGeometry(junctionOpenShape);
                        // set color
                        GLHelper::setColor(darkerColor);
                        // draw shape
                        GNEGeometry::drawGeometry(myNet->getViewNet(), junctionGeometry, s.neteditSizeSettings.junctionGeometryPointRadius * 0.5);
                        // draw geometry points
                        GNEGeometry::drawGeometryPoints(s, myNet->getViewNet(), junctionOpenShape, darkerColor, darkerColor, s.neteditSizeSettings.junctionGeometryPointRadius, junctionExaggeration);
                        // draw moving hint
                        GNEGeometry::drawMovingHint(s, myNet->getViewNet(), junctionOpenShape, darkerColor, s.neteditSizeSettings.junctionGeometryPointRadius, junctionExaggeration);
                    }
                }
            }
            // check if bubble has to be drawn
            if (drawBubble) {
                // set bubble color
                const RGBColor bubbleColor = setColor(s, true);
                // recognize full transparency and simply don't draw
                if (bubbleColor.alpha() != 0) {
                    // check if mouse is in bubble
                    const bool mouseInBubble = (mousePosition.distanceSquaredTo2D(myNBNode->getPosition()) <= (bubbleRadius * bubbleRadius));
                    // only draw filled circle if we aren't in draw for selecting mode, or if distance to center is enough)
                    if (!s.drawForPositionSelection || mouseInBubble) {
                        // push matrix
                        glPushMatrix();
                        // set color
                        GLHelper::setColor(bubbleColor);
                        // move matrix junction center
                        glTranslated(myNBNode->getPosition().x(), myNBNode->getPosition().y(), 0.1);
                        // draw filled circle
                        GLHelper::drawFilledCircle(bubbleRadius, s.getCircleResolution());
                        // pop matrix
                        glPopMatrix();
                    }
                }
            }
            // draw TLS
            drawTLSIcon(s);
            // draw elevation
            if (!s.drawForRectangleSelection && myNet->getViewNet()->getNetworkViewOptions().editingElevation()) {
                glPushMatrix();
                // Translate to center of junction
                glTranslated(myNBNode->getPosition().x(), myNBNode->getPosition().y(), 0.1);
                // draw Z value
                GLHelper::drawText(toString(myNBNode->getPosition().z()), Position(), GLO_MAX - 5, s.junctionID.scaledSize(s.scale), s.junctionID.color);
                glPopMatrix();
            }
            // pop layer Matrix
            glPopMatrix();
            // pop junction name
            glPopName();
            // draw name and ID
            if (!s.drawForRectangleSelection) {
                drawName(myNBNode->getPosition(), s.scale, s.junctionID);
                if (s.junctionName.show && myNBNode->getName() != "") {
                    GLHelper::drawTextSettings(s.junctionName, myNBNode->getName(), myNBNode->getPosition(), s.scale, s.angle);
                }
            }
            // draw Junction childs
            drawJunctionChildren(s);
            // check if dotted contour has to be drawn
            if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
                if (drawShape) {
                    GNEGeometry::drawDottedContourClosedShape(true, s, myNBNode->getShape(), junctionExaggeration);
                }
                if (drawBubble) {
                    GNEGeometry::drawDottedContourCircle(true, s, myNBNode->getCenter(), s.neteditSizeSettings.junctionBubbleRadius, junctionExaggeration);
                }
            }
            // check if dotted contour has to be drawn
            if (s.drawDottedContour() || (myNet->getViewNet()->getFrontAttributeCarrier() == this)) {
                if (drawShape) {
                    GNEGeometry::drawDottedContourClosedShape(false, s, myNBNode->getShape(), junctionExaggeration);
                }
                if (drawBubble) {
                    GNEGeometry::drawDottedContourCircle(false, s, myNBNode->getCenter(), s.neteditSizeSettings.junctionBubbleRadius, junctionExaggeration);
                }
            }
        }
    }
}


NBNode*
GNEJunction::getNBNode() const {
    return myNBNode;
}


std::vector<GNEJunction*>
GNEJunction::getJunctionNeighbours() const {
    // use set to avoid duplicates junctions
    std::set<GNEJunction*> junctions;
    for (const auto& i : myGNEIncomingEdges) {
        junctions.insert(i->getParentJunctions().front());
    }
    for (const auto& i : myGNEOutgoingEdges) {
        junctions.insert(i->getParentJunctions().back());
    }
    return std::vector<GNEJunction*>(junctions.begin(), junctions.end());
}


void
GNEJunction::addIncomingGNEEdge(GNEEdge* edge) {
    // Check if incoming edge was already inserted
    std::vector<GNEEdge*>::iterator i = std::find(myGNEIncomingEdges.begin(), myGNEIncomingEdges.end(), edge);
    if (i != myGNEIncomingEdges.end()) {
        throw InvalidArgument("Incoming " + toString(SUMO_TAG_EDGE) + " with ID '" + edge->getID() + "' was already inserted into " + getTagStr() + " with ID " + getID() + "'");
    } else {
        // Add edge into containers
        myGNEIncomingEdges.push_back(edge);
    }
}



void
GNEJunction::addOutgoingGNEEdge(GNEEdge* edge) {
    // Check if outgoing edge was already inserted
    std::vector<GNEEdge*>::iterator i = std::find(myGNEOutgoingEdges.begin(), myGNEOutgoingEdges.end(), edge);
    if (i != myGNEOutgoingEdges.end()) {
        throw InvalidArgument("Outgoing " + toString(SUMO_TAG_EDGE) + " with ID '" + edge->getID() + "' was already inserted into " + getTagStr() + " with ID " + getID() + "'");
    } else {
        // Add edge into containers
        myGNEOutgoingEdges.push_back(edge);
    }
}


void
GNEJunction::removeIncomingGNEEdge(GNEEdge* edge) {
    // Check if incoming edge was already inserted
    std::vector<GNEEdge*>::iterator i = std::find(myGNEIncomingEdges.begin(), myGNEIncomingEdges.end(), edge);
    if (i == myGNEIncomingEdges.end()) {
        throw InvalidArgument("Incoming " + toString(SUMO_TAG_EDGE) + " with ID '" + edge->getID() + "' doesn't found into " + getTagStr() + " with ID " + getID() + "'");
    } else {
        // remove edge from containers
        myGNEIncomingEdges.erase(i);
    }
}


void
GNEJunction::removeOutgoingGNEEdge(GNEEdge* edge) {
    // Check if outgoing edge was already inserted
    std::vector<GNEEdge*>::iterator i = std::find(myGNEOutgoingEdges.begin(), myGNEOutgoingEdges.end(), edge);
    if (i == myGNEOutgoingEdges.end()) {
        throw InvalidArgument("Outgoing " + toString(SUMO_TAG_EDGE) + " with ID '" + edge->getID() + "' doesn't found into " + getTagStr() + " with ID " + getID() + "'");
    } else {
        // remove edge from containers
        myGNEOutgoingEdges.erase(i);
    }
}


const std::vector<GNEEdge*>&
GNEJunction::getGNEIncomingEdges() const {
    return myGNEIncomingEdges;
}


const std::vector<GNEEdge*>&
GNEJunction::getGNEOutgoingEdges() const {
    return myGNEOutgoingEdges;
}


const std::vector<GNECrossing*>&
GNEJunction::getGNECrossings() const {
    return myGNECrossings;
}


std::vector<GNEConnection*>
GNEJunction::getGNEConnections() const {
    std::vector<GNEConnection*> connections;
    for (const auto& i : myGNEIncomingEdges) {
        for (const auto& j : i->getGNEConnections()) {
            connections.push_back(j);
        }
    }
    return connections;
}


void
GNEJunction::markAsCreateEdgeSource() {
    myAmCreateEdgeSource = true;
}


void
GNEJunction::unMarkAsCreateEdgeSource() {
    myAmCreateEdgeSource = false;
}


void
GNEJunction::selectTLS(bool selected) {
    myAmTLSSelected = selected;
}


void
GNEJunction::startGeometryMoving(bool extendToNeighbors) {
    // save current centering boundary
    myMovingGeometryBoundary = getCenteringBoundary();
    // save position
    myMovingPosition = myNBNode->getPosition();
    // First declare three sets with all affected GNEJunctions, GNEEdges and GNEConnections
    std::set<GNEJunction*> affectedJunctions;
    std::set<GNEEdge*> affectedEdges;
    // Iterate over GNEEdges
    for (const auto& edge : getChildEdges()) {
        // Add source and destiny junctions
        affectedJunctions.insert(edge->getParentJunctions().front());
        affectedJunctions.insert(edge->getParentJunctions().back());
        // Obtain neighbors of Junction source
        for (const auto& junctionSourceEdge : edge->getParentJunctions().front()->getChildEdges()) {
            affectedEdges.insert(junctionSourceEdge);
        }
        // Obtain neighbors of Junction destiny
        for (const auto& junctionDestinyEdge : edge->getParentJunctions().back()->getChildEdges()) {
            affectedEdges.insert(junctionDestinyEdge);
        }
    }
    // Iterate over affected Junctions only if extendToNeighbors is enabled
    if (extendToNeighbors) {
        for (const auto& affectedJunction : affectedJunctions) {
            // don't include this junction (to avoid start more than one times)
            if (affectedJunction != this) {
                // start geometry moving in edges
                affectedJunction->startGeometryMoving(false);
            }
        }
    }
    // Iterate over affected Edges
    for (const auto& edge : affectedEdges) {
        // start geometry moving in edges
        edge->startEdgeGeometryMoving(-1, false);
    }
}


void
GNEJunction::endGeometryMoving(bool extendToNeighbors) {
    // check that endGeometryMoving was called only once
    if (myMovingGeometryBoundary.isInitialised()) {
        // Remove object from net
        myNet->removeGLObjectFromGrid(this);
        // reset myMovingGeometryBoundary
        myMovingGeometryBoundary.reset();
        // First declare three sets with all affected GNEJunctions, GNEEdges and GNEConnections
        std::set<GNEJunction*> affectedJunctions;
        std::set<GNEEdge*> affectedEdges;
        // Iterate over GNEEdges
        for (const auto& edge : getChildEdges()) {
            // Add source and destiny junctions
            affectedJunctions.insert(edge->getParentJunctions().front());
            affectedJunctions.insert(edge->getParentJunctions().back());
            // Obtain neighbors of Junction source
            for (const auto& junctionSourceEdge : edge->getParentJunctions().front()->getChildEdges()) {
                affectedEdges.insert(junctionSourceEdge);
            }
            // Obtain neighbors of Junction destiny
            for (const auto& junctionDestinyEdge : edge->getParentJunctions().back()->getChildEdges()) {
                affectedEdges.insert(junctionDestinyEdge);
            }
        }
        // Iterate over affected Junctions
        if (extendToNeighbors) {
            for (const auto& junction : affectedJunctions) {
                // don't include this junction (to avoid end it more than one times)
                if (junction != this) {
                    // end geometry moving in edges
                    junction->endGeometryMoving(false);
                }
            }
        }
        // Iterate over affected Edges
        for (const auto& affectedEdge : affectedEdges) {
            if (!affectedEdge->isAttributeCarrierSelected()) {
                // end geometry moving in edges
                affectedEdge->endEdgeGeometryMoving();
            }
        }
        // add object into grid again (using the new centering boundary)
        myNet->addGLObjectIntoGrid(this);
        // update geometry of affected junctions
        if (extendToNeighbors) {
            for (const auto& junction : affectedJunctions) {
                junction->updateGeometry();
            }
        }
    }
}


void
GNEJunction::moveGeometry(const Position& offset) {
    // calculate new position
    Position newPosition = myMovingPosition;
    newPosition.add(offset);
    // filtern position using snap to active grid
    newPosition = myNet->getViewNet()->snapToActiveGrid(newPosition, offset.z() == 0);
    // move junction geometry without updating grid
    moveJunctionGeometry(newPosition);
}


void
GNEJunction::commitGeometryMoving(GNEUndoList* undoList) {
    // first end geometry point
    endGeometryMoving();
    if (isValid(SUMO_ATTR_POSITION, toString(myNBNode->getPosition()))) {
        undoList->p_begin("position of " + getTagStr());
        undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_POSITION, toString(myNBNode->getPosition()), toString(myMovingPosition)));
        undoList->p_end();
    } else {
        // tried to set an invalid position, revert back to the previous one
        moveJunctionGeometry(myMovingPosition);
    }
}


void
GNEJunction::invalidateShape() {
    if (!myNBNode->hasCustomShape()) {
        if (myNBNode->myPoly.size() > 0) {
            // write GL Debug
            WRITE_GLDEBUG("<-- Invalidating shape of junction '" + getID() + "' -->");
            // remove Juntion from grid
            myNet->removeGLObjectFromGrid(this);
            // clear poly
            myNBNode->myPoly.clear();
            // add Juntion into grid
            myNet->addGLObjectIntoGrid(this);
        }
        myNet->requireRecompute();
    }
}


void
GNEJunction::setLogicValid(bool valid, GNEUndoList* undoList, const std::string& status) {
    myHasValidLogic = valid;
    if (!valid) {
        assert(undoList != 0);
        assert(undoList->hasCommandGroup());
        NBTurningDirectionsComputer::computeTurnDirectionsForNode(myNBNode, false);
        EdgeVector incoming = myNBNode->getIncomingEdges();
        for (EdgeVector::iterator it = incoming.begin(); it != incoming.end(); it++) {
            GNEEdge* srcEdge = myNet->retrieveEdge((*it)->getID());
            removeConnectionsFrom(srcEdge, undoList, false); // false, because the whole tls will be invalidated at the end
            undoList->add(new GNEChange_Attribute(srcEdge, GNE_ATTR_MODIFICATION_STATUS, status), true);
        }
        undoList->add(new GNEChange_Attribute(this, GNE_ATTR_MODIFICATION_STATUS, status), true);
        invalidateTLS(undoList);
    } else {
        // logic valed, then rebuild GNECrossings to adapt it to the new logic
        // (but don't rebuild the crossings in NBNode because they are already finished)
        rebuildGNECrossings(false);
    }
}


void
GNEJunction::removeConnectionsFrom(GNEEdge* edge, GNEUndoList* undoList, bool updateTLS, int lane) {
    NBEdge* srcNBE = edge->getNBEdge();
    NBEdge* turnEdge = srcNBE->getTurnDestination();
    // Make a copy of connections
    std::vector<NBEdge::Connection> connections = srcNBE->getConnections();
    // delete in reverse so that undoing will add connections in the original order
    for (std::vector<NBEdge::Connection>::reverse_iterator con_it = connections.rbegin(); con_it != connections.rend(); con_it++) {
        if (lane >= 0 && (*con_it).fromLane != lane) {
            continue;
        }
        bool hasTurn = con_it->toEdge == turnEdge;
        undoList->add(new GNEChange_Connection(edge, *con_it, false, false), true);
        // needs to come after GNEChange_Connection
        // XXX bug: this code path will not be used on a redo!
        if (hasTurn) {
            myNet->addExplicitTurnaround(srcNBE->getID());
        }
    }
    if (updateTLS) {
        std::vector<NBConnection> removeConnections;
        for (NBEdge::Connection con : connections) {
            removeConnections.push_back(NBConnection(srcNBE, con.fromLane, con.toEdge, con.toLane));
        }
        removeTLSConnections(removeConnections, undoList);
    }
}


void
GNEJunction::removeConnectionsTo(GNEEdge* edge, GNEUndoList* undoList, bool updateTLS, int lane) {
    NBEdge* destNBE = edge->getNBEdge();
    std::vector<NBConnection> removeConnections;
    for (NBEdge* srcNBE : myNBNode->getIncomingEdges()) {
        GNEEdge* srcEdge = myNet->retrieveEdge(srcNBE->getID());
        std::vector<NBEdge::Connection> connections = srcNBE->getConnections();
        for (std::vector<NBEdge::Connection>::reverse_iterator con_it = connections.rbegin(); con_it != connections.rend(); con_it++) {
            if ((*con_it).toEdge == destNBE) {
                if (lane >= 0 && (*con_it).toLane != lane) {
                    continue;
                }
                bool hasTurn = srcNBE->getTurnDestination() == destNBE;
                undoList->add(new GNEChange_Connection(srcEdge, *con_it, false, false), true);
                // needs to come after GNEChange_Connection
                // XXX bug: this code path will not be used on a redo!
                if (hasTurn) {
                    myNet->addExplicitTurnaround(srcNBE->getID());
                }
                removeConnections.push_back(NBConnection(srcNBE, (*con_it).fromLane, destNBE, (*con_it).toLane));
            }
        }
    }
    if (updateTLS) {
        removeTLSConnections(removeConnections, undoList);
    }
}


void
GNEJunction::removeTLSConnections(std::vector<NBConnection>& connections, GNEUndoList* undoList) {
    if (connections.size() > 0) {
        const std::set<NBTrafficLightDefinition*> coypOfTls = myNBNode->getControllingTLS(); // make a copy!
        for (const auto& TLS : coypOfTls) {
            NBLoadedSUMOTLDef* tlDef = dynamic_cast<NBLoadedSUMOTLDef*>(TLS);
            // guessed TLS (NBOwnTLDef) do not need to be updated
            if (tlDef != nullptr) {
                std::string newID = tlDef->getID();
                // create replacement before deleting the original because deletion will mess up saving original nodes
                NBLoadedSUMOTLDef* replacementDef = new NBLoadedSUMOTLDef(*tlDef, *tlDef->getLogic());
                for (NBConnection& con : connections) {
                    replacementDef->removeConnection(con);
                }
                undoList->add(new GNEChange_TLS(this, tlDef, false), true);
                undoList->add(new GNEChange_TLS(this, replacementDef, true, false, newID), true);
                // the removed traffic light may have controlled more than one junction. These too have become invalid now
                const std::vector<NBNode*> copyOfNodes = tlDef->getNodes(); // make a copy!
                for (const auto& node : copyOfNodes) {
                    GNEJunction* sharing = myNet->retrieveJunction(node->getID());
                    undoList->add(new GNEChange_TLS(sharing, tlDef, false), true);
                    undoList->add(new GNEChange_TLS(sharing, replacementDef, true, false, newID), true);
                }
            }
        }
    }
}


void
GNEJunction::replaceIncomingConnections(GNEEdge* which, GNEEdge* by, GNEUndoList* undoList) {
    // remap connections of the edge
    assert(which->getLanes().size() == by->getLanes().size());
    std::vector<NBEdge::Connection> connections = which->getNBEdge()->getConnections();
    for (NBEdge::Connection& c : connections) {
        undoList->add(new GNEChange_Connection(which, c, false, false), true);
        undoList->add(new GNEChange_Connection(by, c, false, true), true);
    }
    // also remap tls connections
    const std::set<NBTrafficLightDefinition*> coypOfTls = myNBNode->getControllingTLS(); // make a copy!
    for (const auto& TLS : coypOfTls) {
        NBLoadedSUMOTLDef* tlDef = dynamic_cast<NBLoadedSUMOTLDef*>(TLS);
        // guessed TLS (NBOwnTLDef) do not need to be updated
        if (tlDef != nullptr) {
            std::string newID = tlDef->getID();
            // create replacement before deleting the original because deletion will mess up saving original nodes
            NBLoadedSUMOTLDef* replacementDef = new NBLoadedSUMOTLDef(*tlDef, *tlDef->getLogic());
            for (int i = 0; i < (int)which->getLanes().size(); ++i) {
                replacementDef->replaceRemoved(which->getNBEdge(), i, by->getNBEdge(), i, true);
            }
            undoList->add(new GNEChange_TLS(this, tlDef, false), true);
            undoList->add(new GNEChange_TLS(this, replacementDef, true, false, newID), true);
            // the removed traffic light may have controlled more than one junction. These too have become invalid now
            const std::vector<NBNode*> copyOfNodes = tlDef->getNodes(); // make a copy!
            for (const auto& node : copyOfNodes) {
                GNEJunction* sharing = myNet->retrieveJunction(node->getID());
                undoList->add(new GNEChange_TLS(sharing, tlDef, false), true);
                undoList->add(new GNEChange_TLS(sharing, replacementDef, true, false, newID), true);
            }
        }
    }
}


void
GNEJunction::markAsModified(GNEUndoList* undoList) {
    EdgeVector incoming = myNBNode->getIncomingEdges();
    for (EdgeVector::iterator it = incoming.begin(); it != incoming.end(); it++) {
        NBEdge* srcNBE = *it;
        GNEEdge* srcEdge = myNet->retrieveEdge(srcNBE->getID());
        undoList->add(new GNEChange_Attribute(srcEdge, GNE_ATTR_MODIFICATION_STATUS, FEATURE_MODIFIED), true);
    }
}


void
GNEJunction::invalidateTLS(GNEUndoList* undoList, const NBConnection& deletedConnection, const NBConnection& addedConnection) {
    assert(undoList->hasCommandGroup());
    // NBLoadedSUMOTLDef becomes invalid, replace with NBOwnTLDef which will be dynamically recomputed
    const std::set<NBTrafficLightDefinition*> coypOfTls = myNBNode->getControllingTLS(); // make a copy!
    for (const auto& TLS : coypOfTls) {
        NBLoadedSUMOTLDef* tlDef = dynamic_cast<NBLoadedSUMOTLDef*>(TLS);
        if (tlDef != nullptr) {
            NBTrafficLightDefinition* replacementDef = nullptr;
            std::string newID = tlDef->getID(); // + "_reguessed"; // changes due to reguessing will be visible in diff
            if (deletedConnection != NBConnection::InvalidConnection) {
                // create replacement before deleting the original because deletion will mess up saving original nodes
                NBLoadedSUMOTLDef* repl = new NBLoadedSUMOTLDef(*tlDef, *tlDef->getLogic());
                repl->removeConnection(deletedConnection);
                replacementDef = repl;
            } else if (addedConnection != NBConnection::InvalidConnection) {
                if (addedConnection.getTLIndex() == NBConnection::InvalidTlIndex) {
                    // custom tl indices of crossings might become invalid upon recomputation so we must save them
                    // however, the could remain valud so we register a change but keep them at their old value
                    for (GNECrossing* c : myGNECrossings) {
                        const std::string oldValue = c->getAttribute(SUMO_ATTR_TLLINKINDEX);
                        undoList->add(new GNEChange_Attribute(c, SUMO_ATTR_TLLINKINDEX, toString(NBConnection::InvalidTlIndex)), true);
                        undoList->add(new GNEChange_Attribute(c, SUMO_ATTR_TLLINKINDEX, oldValue), true);
                        const std::string oldValue2 = c->getAttribute(SUMO_ATTR_TLLINKINDEX);
                        undoList->add(new GNEChange_Attribute(c, SUMO_ATTR_TLLINKINDEX2, toString(NBConnection::InvalidTlIndex)), true);
                        undoList->add(new GNEChange_Attribute(c, SUMO_ATTR_TLLINKINDEX2, oldValue2), true);
                    }
                }
                NBLoadedSUMOTLDef* repl = new NBLoadedSUMOTLDef(*tlDef, *tlDef->getLogic());
                repl->addConnection(addedConnection.getFrom(), addedConnection.getTo(),
                                    addedConnection.getFromLane(), addedConnection.getToLane(), addedConnection.getTLIndex(), addedConnection.getTLIndex2());
                replacementDef = repl;
            } else {
                replacementDef = new NBOwnTLDef(newID, tlDef->getOffset(), tlDef->getType());
                replacementDef->setProgramID(tlDef->getProgramID());
            }
            undoList->add(new GNEChange_TLS(this, tlDef, false), true);
            undoList->add(new GNEChange_TLS(this, replacementDef, true, false, newID), true);
            // the removed traffic light may have controlled more than one junction. These too have become invalid now
            const std::vector<NBNode*> copyOfNodes = tlDef->getNodes(); // make a copy!
            for (const auto& node : copyOfNodes) {
                GNEJunction* sharing = myNet->retrieveJunction(node->getID());
                undoList->add(new GNEChange_TLS(sharing, tlDef, false), true);
                undoList->add(new GNEChange_TLS(sharing, replacementDef, true, false, newID), true);
            }
        }
    }
}

void
GNEJunction::removeEdgeFromCrossings(GNEEdge* edge, GNEUndoList* undoList) {
    // obtain a copy of GNECrossing of junctions
    auto copyOfGNECrossings = myGNECrossings;
    // iterate over copy of GNECrossings
    for (int i = 0; i < (int)myGNECrossings.size(); i++) {
        auto c = myGNECrossings.at(i);
        // obtain the set of edges vinculated with the crossing (due it works as ID)
        EdgeSet edgeSet(c->getCrossingEdges().begin(), c->getCrossingEdges().end());
        // If this edge is part of the set of edges of crossing
        if (edgeSet.count(edge->getNBEdge()) == 1) {
            // delete crossing if this is their last edge
            if ((c->getCrossingEdges().size() == 1) && (c->getCrossingEdges().front() == edge->getNBEdge())) {
                myNet->deleteCrossing(c, undoList);
                i = 0;
            } else {
                // remove this edge of the edge's attribute of crossing (note: This can invalidate the crossing)
                std::vector<std::string> edges = GNEAttributeCarrier::parse<std::vector<std::string>>(c->getAttribute(SUMO_ATTR_EDGES));
                edges.erase(std::find(edges.begin(), edges.end(), edge->getID()));
                c->setAttribute(SUMO_ATTR_EDGES, joinToString(edges, " "), undoList);
            }
        }
    }
}


bool
GNEJunction::isLogicValid() {
    return myHasValidLogic;
}


GNECrossing*
GNEJunction::retrieveGNECrossing(NBNode::Crossing* NBNodeCrossing, bool createIfNoExist) {
    // iterate over all crossing
    for (const auto& crossing : myGNECrossings) {
        // if found, return it
        if (crossing->getCrossingEdges() == NBNodeCrossing->edges) {
            return crossing;
        }
    }
    if (createIfNoExist) {
        // create new GNECrossing
        GNECrossing* createdGNECrossing = new GNECrossing(this, NBNodeCrossing->edges);
        // show extra information for tests
        WRITE_DEBUG("Created " + createdGNECrossing->getTagStr() + " '" + createdGNECrossing->getID() + "' in retrieveGNECrossing()");
        // update geometry after creating
        createdGNECrossing->updateGeometry();
        // add it in Network
        myNet->addGLObjectIntoGrid(createdGNECrossing);
        return createdGNECrossing;
    } else {
        return nullptr;
    }
}


void
GNEJunction::markConnectionsDeprecated(bool includingNeighbours) {
    // only it's needed to mark the connections of incoming edges
    for (const auto& i : myGNEIncomingEdges) {
        for (const auto& j : i->getGNEConnections()) {
            j->markConnectionGeometryDeprecated();
        }
        if (includingNeighbours) {
            i->getParentJunctions().front()->markConnectionsDeprecated(false);
        }
    }
}


void
GNEJunction::addPathAdditionalElement(GNEAdditional* additionalElement) {
    // get tag
    SumoXMLTag tag = additionalElement->getTagProperty().getTag();
    // avoid insert duplicated path element childs
    if (std::find(myPathAdditionalElements[tag].begin(), myPathAdditionalElements[tag].end(), additionalElement) == myPathAdditionalElements[tag].end()) {
        myPathAdditionalElements[tag].push_back(additionalElement);
    }
}


void
GNEJunction::removePathAdditionalElement(GNEAdditional* additionalElement) {
    // get tag
    SumoXMLTag tag = additionalElement->getTagProperty().getTag();
    // search and remove pathElementChild
    auto it = std::find(myPathAdditionalElements[tag].begin(), myPathAdditionalElements[tag].end(), additionalElement);
    if (it != myPathAdditionalElements[tag].end()) {
        myPathAdditionalElements[tag].erase(it);
    }
}


void
GNEJunction::addPathDemandElement(GNEDemandElement* demandElement) {
    // get tag
    SumoXMLTag tag = demandElement->getTagProperty().getTag();
    // avoid insert duplicated path element childs
    if (std::find(myPathDemandElements[tag].begin(), myPathDemandElements[tag].end(), demandElement) == myPathDemandElements[tag].end()) {
        myPathDemandElements[tag].push_back(demandElement);
    }
}


void
GNEJunction::removePathDemandElement(GNEDemandElement* demandElement) {
    // get tag
    SumoXMLTag tag = demandElement->getTagProperty().getTag();
    // search and remove pathElementChild
    auto it = std::find(myPathDemandElements[tag].begin(), myPathDemandElements[tag].end(), demandElement);
    if (it != myPathDemandElements[tag].end()) {
        myPathDemandElements[tag].erase(it);
    }
}


void
GNEJunction::addPathGenericData(GNEGenericData* genericData) {
    // get tag
    SumoXMLTag tag = genericData->getTagProperty().getTag();
    // avoid insert duplicated path element childs
    if (std::find(myPathGenericDatas[tag].begin(), myPathGenericDatas[tag].end(), genericData) == myPathGenericDatas[tag].end()) {
        myPathGenericDatas[tag].push_back(genericData);
    }
}


void
GNEJunction::removePathGenericData(GNEGenericData* genericData) {
    // get tag
    SumoXMLTag tag = genericData->getTagProperty().getTag();
    // search and remove pathElementChild
    auto it = std::find(myPathGenericDatas[tag].begin(), myPathGenericDatas[tag].end(), genericData);
    if (it != myPathGenericDatas[tag].end()) {
        myPathGenericDatas[tag].erase(it);
    }
}


void
GNEJunction::invalidatePathElements() {
    // make a copy of myPathAdditionalElements
    auto copyOfPathAdditionalElements = myPathAdditionalElements;
    for (const auto& tag : copyOfPathAdditionalElements) {
        for (const auto& additionalElement : tag.second) {
            // note: currently additional elements don't use compute/invalidate paths
            additionalElement->updateGeometry();
        }
    }
    // make a copy of myPathDemandElements
    auto copyOfPathDemandElements = myPathDemandElements;
    for (const auto& tag : copyOfPathDemandElements) {
        for (const auto& demandElement : tag.second) {
            demandElement->invalidatePath();
        }
    }
    // make a copy of myPathGenericDatas
    auto copyOfPathGenericDatas = myPathGenericDatas;
    for (const auto& tag : copyOfPathGenericDatas) {
        for (const auto& genericData : tag.second) {
            // note: currently generic datas don't use compute/invalidate paths
            genericData->updateGeometry();
        }
    }
}


std::string
GNEJunction::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_POSITION:
            return toString(myNBNode->getPosition());
        case SUMO_ATTR_TYPE:
            return toString(myNBNode->getType());
        case GNE_ATTR_MODIFICATION_STATUS:
            return myLogicStatus;
        case SUMO_ATTR_SHAPE:
            return toString(myNBNode->getShape());
        case SUMO_ATTR_RADIUS:
            return toString(myNBNode->getRadius());
        case SUMO_ATTR_TLTYPE:
            if (isAttributeEnabled(SUMO_ATTR_TLTYPE)) {
                // @todo this causes problems if the node were to have multiple programs of different type (plausible)
                return toString((*myNBNode->getControllingTLS().begin())->getType());
            } else {
                return "No TLS";
            }
        case SUMO_ATTR_TLID:
            if (isAttributeEnabled(SUMO_ATTR_TLID)) {
                return toString((*myNBNode->getControllingTLS().begin())->getID());
            } else {
                return "No TLS";
            }
        case SUMO_ATTR_KEEP_CLEAR:
            // keep clear is only used as a convenience feature in plain xml
            // input. When saving to .net.xml the status is saved only for the connections
            // to show the correct state we must check all connections
            for (const auto& i : myGNEIncomingEdges) {
                for (const auto& j : i->getGNEConnections()) {
                    if (j->getNBEdgeConnection().keepClear) {
                        return toString(true);
                    }
                }
            }
            return toString(false);
        case SUMO_ATTR_RIGHT_OF_WAY:
            return SUMOXMLDefinitions::RightOfWayValues.getString(myNBNode->getRightOfWay());
        case SUMO_ATTR_FRINGE:
            return SUMOXMLDefinitions::FringeTypeValues.getString(myNBNode->getFringeType());
        case SUMO_ATTR_NAME:
            return myNBNode->getName();
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return myNBNode->getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEJunction::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_POSITION:
        case GNE_ATTR_MODIFICATION_STATUS:
        case SUMO_ATTR_SHAPE:
        case SUMO_ATTR_RADIUS:
        case SUMO_ATTR_RIGHT_OF_WAY:
        case SUMO_ATTR_FRINGE:
        case SUMO_ATTR_NAME:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->add(new GNEChange_Attribute(this, key, value), true);
            break;
        case SUMO_ATTR_KEEP_CLEAR:
            // change Keep Clear attribute in all connections
            undoList->p_begin("change keepClear for whole junction");
            for (const auto& i : myGNEIncomingEdges) {
                for (const auto& j : i->getGNEConnections()) {
                    undoList->add(new GNEChange_Attribute(j, key, value), true);
                }
            }
            undoList->p_end();
            break;
        case SUMO_ATTR_TYPE: {
            undoList->p_begin("change " + getTagStr() + " type");
            if (NBNode::isTrafficLight(SUMOXMLDefinitions::NodeTypes.get(value))) {
                if (getNBNode()->isTLControlled() &&
                        // if switching changing from or to traffic_light_right_on_red we need to remove the old plan
                        (getNBNode()->getType() == SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED
                         || SUMOXMLDefinitions::NodeTypes.get(value) == SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED)
                   ) {
                    // make a copy because we will modify the original
                    const std::set<NBTrafficLightDefinition*> copyOfTls = myNBNode->getControllingTLS();
                    for (const auto& TLS : copyOfTls) {
                        undoList->add(new GNEChange_TLS(this, TLS, false), true);
                    }
                }
                if (!getNBNode()->isTLControlled()) {
                    // create new traffic light
                    undoList->add(new GNEChange_TLS(this, nullptr, true), true);
                }
            } else if (getNBNode()->isTLControlled()) {
                // delete old traffic light
                // make a copy because we will modify the original
                const std::set<NBTrafficLightDefinition*> copyOfTls = myNBNode->getControllingTLS();
                for (const auto& TLS : copyOfTls) {
                    undoList->add(new GNEChange_TLS(this, TLS, false, false), true);
                }
            }
            // must be the final step, otherwise we do not know which traffic lights to remove via GNEChange_TLS
            undoList->add(new GNEChange_Attribute(this, key, value), true);
            for (const auto& crossing : myGNECrossings) {
                undoList->add(new GNEChange_Attribute(crossing, SUMO_ATTR_TLLINKINDEX, "-1"), true);
                undoList->add(new GNEChange_Attribute(crossing, SUMO_ATTR_TLLINKINDEX2, "-1"), true);
            }
            undoList->p_end();
            break;
        }
        case SUMO_ATTR_TLTYPE: {
            undoList->p_begin("change " + getTagStr() + " tl-type");
            // make a copy because we will modify the original
            const std::set<NBTrafficLightDefinition*> copyOfTls = myNBNode->getControllingTLS();
            for (const auto& TLS : copyOfTls) {
                NBLoadedSUMOTLDef* oldLoaded = dynamic_cast<NBLoadedSUMOTLDef*>(TLS);
                if (oldLoaded != nullptr) {
                    NBLoadedSUMOTLDef* newDef = new NBLoadedSUMOTLDef(*oldLoaded, *oldLoaded->getLogic());
                    newDef->guessMinMaxDuration();
                    std::vector<NBNode*> nodes = TLS->getNodes();
                    for (const auto& node : nodes) {
                        GNEJunction* junction = myNet->retrieveJunction(node->getID());
                        undoList->add(new GNEChange_TLS(junction, TLS, false), true);
                        undoList->add(new GNEChange_TLS(junction, newDef, true), true);
                    }
                }
            }
            undoList->add(new GNEChange_Attribute(this, key, value), true);
            undoList->p_end();
            break;
        }
        case SUMO_ATTR_TLID: {
            undoList->p_begin("change " + toString(SUMO_TAG_TRAFFIC_LIGHT) + " id");
            const std::set<NBTrafficLightDefinition*> copyOfTls = myNBNode->getControllingTLS();
            assert(copyOfTls.size() > 0);
            NBTrafficLightDefinition* currentTLS = *copyOfTls.begin();
            NBTrafficLightDefinition* currentTLSCopy = nullptr;
            const bool currentIsSingle = currentTLS->getNodes().size() == 1;
            const bool currentIsLoaded = dynamic_cast<NBLoadedSUMOTLDef*>(currentTLS) != nullptr;
            if (currentIsLoaded) {
                currentTLSCopy = new NBLoadedSUMOTLDef(*currentTLS,
                                                       *dynamic_cast<NBLoadedSUMOTLDef*>(currentTLS)->getLogic());
            }
            // remove from previous tls
            for (const auto& TLS : copyOfTls) {
                undoList->add(new GNEChange_TLS(this, TLS, false), true);
            }
            NBTrafficLightLogicCont& tlCont = myNet->getTLLogicCont();
            // programs to which the current node shall be added
            const std::map<std::string, NBTrafficLightDefinition*> programs = tlCont.getPrograms(value);
            if (programs.size() > 0) {
                for (const auto& TLSProgram : programs) {
                    NBTrafficLightDefinition* oldTLS = TLSProgram.second;
                    if (dynamic_cast<NBOwnTLDef*>(oldTLS) != nullptr) {
                        undoList->add(new GNEChange_TLS(this, oldTLS, true), true);
                    } else {
                        // delete and re-create the definition because the loaded phases are now invalid
                        if (dynamic_cast<NBLoadedSUMOTLDef*>(oldTLS) != nullptr &&
                                dynamic_cast<NBLoadedSUMOTLDef*>(oldTLS)->usingSignalGroups()) {
                            // keep the old program and add all-red state for the added links
                            NBLoadedSUMOTLDef* newTLSJoined = new NBLoadedSUMOTLDef(*oldTLS, *dynamic_cast<NBLoadedSUMOTLDef*>(oldTLS)->getLogic());
                            newTLSJoined->joinLogic(currentTLSCopy);
                            undoList->add(new GNEChange_TLS(this, newTLSJoined, true, true), true);
                        } else {
                            undoList->add(new GNEChange_TLS(this, nullptr, true, false, value), true);
                        }
                        NBTrafficLightDefinition* newTLS = *myNBNode->getControllingTLS().begin();
                        // switch from old to new definition
                        std::vector<NBNode*> copyOfNodes = oldTLS->getNodes();
                        for (const auto& node : copyOfNodes) {
                            GNEJunction* oldJunction = myNet->retrieveJunction(node->getID());
                            undoList->add(new GNEChange_TLS(oldJunction, oldTLS, false), true);
                            undoList->add(new GNEChange_TLS(oldJunction, newTLS, true), true);
                        }
                    }
                }
            } else {
                if (currentIsSingle && currentIsLoaded) {
                    // rename the traffic light but keep everything else
                    NBTrafficLightLogic* renamedLogic = dynamic_cast<NBLoadedSUMOTLDef*>(currentTLSCopy)->getLogic();
                    renamedLogic->setID(value);
                    NBLoadedSUMOTLDef* renamedTLS = new NBLoadedSUMOTLDef(*currentTLSCopy, *renamedLogic);
                    renamedTLS->setID(value);
                    undoList->add(new GNEChange_TLS(this, renamedTLS, true, true), true);
                } else {
                    // create new traffic light
                    undoList->add(new GNEChange_TLS(this, nullptr, true, false, value), true);
                }
            }
            delete currentTLSCopy;
            undoList->p_end();
            break;
        }
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEJunction::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveJunction(value, false) == nullptr);
        case SUMO_ATTR_TYPE:
            return SUMOXMLDefinitions::NodeTypes.hasString(value);
        case SUMO_ATTR_POSITION:
            return canParse<Position>(value);
        case SUMO_ATTR_SHAPE:
            // empty shapes are allowed
            return canParse<PositionVector>(value);
        case SUMO_ATTR_RADIUS:
            return canParse<double>(value) && (parse<double>(value) >= -1);
        case SUMO_ATTR_TLTYPE:
            return myNBNode->isTLControlled() && SUMOXMLDefinitions::TrafficLightTypes.hasString(value);
        case SUMO_ATTR_TLID:
            return myNBNode->isTLControlled() && (value != "");
        case SUMO_ATTR_KEEP_CLEAR:
            return canParse<bool>(value);
        case SUMO_ATTR_RIGHT_OF_WAY:
            return SUMOXMLDefinitions::RightOfWayValues.hasString(value);
        case SUMO_ATTR_FRINGE:
            return SUMOXMLDefinitions::FringeTypeValues.hasString(value);
        case SUMO_ATTR_NAME:
            return true;
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEJunction::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_TLTYPE:
        case SUMO_ATTR_TLID:
            return myNBNode->isTLControlled();
        case SUMO_ATTR_KEEP_CLEAR: {
            // check if at least there is an incoming connection
            for (const auto& i : myGNEIncomingEdges) {
                if (i->getGNEConnections().size() > 0) {
                    return true;
                }
            }
            return false;
        }
        default:
            return true;
    }
}


void
GNEJunction::setResponsible(bool newVal) {
    myAmResponsible = newVal;
}

// ===========================================================================
// private
// ===========================================================================

void
GNEJunction::drawTLSIcon(const GUIVisualizationSettings& s) const {
    // draw TLS icon if isn't being drawn for selecting
    if ((myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_TLS) &&
            (myNBNode->isTLControlled()) && !myAmTLSSelected && !s.drawForRectangleSelection) {
        glPushMatrix();
        Position pos = myNBNode->getPosition();
        glTranslated(pos.x(), pos.y(), 0.1);
        glColor3d(1, 1, 1);
        const double halfWidth = 32 / s.scale;
        const double halfHeight = 64 / s.scale;
        GUITexturesHelper::drawTexturedBox(GUITextureSubSys::getTexture(GNETEXTURE_TLS), -halfWidth, -halfHeight, halfWidth, halfHeight);
        glPopMatrix();
    }
}


void
GNEJunction::drawJunctionChildren(const GUIVisualizationSettings& s) const {
    // draw crossings only if junction isn't being moved
    if (!myMovingGeometryBoundary.isInitialised()) {
        for (const auto& crossing : myGNECrossings) {
            crossing->drawGL(s);
        }
    }
    // draw connections and route elements connections (Only for incoming edges)
    for (const auto& incomingEdge : myGNEIncomingEdges) {
        for (const auto& connection : incomingEdge->getGNEConnections()) {
            connection->drawGL(s);
        }
    }
    // draw child demand elements
    for (const auto& demandElement : getChildDemandElements()) {
        if (!demandElement->getTagProperty().isPlacedInRTree()) {
            demandElement->drawGL(s);
        }
    }
    // draw child path additionals
    for (const auto& tag : myPathAdditionalElements) {
        for (const auto& element : tag.second) {
            element->drawJunctionPathChildren(s, this, 0);
        }
    }
    // draw child path demand elements
    for (const auto& tag : myPathDemandElements) {
        for (const GNEDemandElement* const element : tag.second) {
            element->drawJunctionPathChildren(s, this, 0);
        }
    }
    // draw child path generic datas
    for (const auto& tag : myPathGenericDatas) {
        for (const GNEGenericData* const element : tag.second) {
            element->drawJunctionPathChildren(s, this, 0);
        }
    }
}


void
GNEJunction::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_KEEP_CLEAR: {
            throw InvalidArgument(toString(key) + " cannot be edited");
        }
        case SUMO_ATTR_ID: {
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        }
        case SUMO_ATTR_TYPE: {
            SumoXMLNodeType type = SUMOXMLDefinitions::NodeTypes.get(value);
            if (myNBNode->getType() == SumoXMLNodeType::PRIORITY && type == SumoXMLNodeType::RIGHT_BEFORE_LEFT) {
                myNet->getNetBuilder()->getEdgeCont().removeRoundabout(myNBNode);
            }
            myNBNode->reinit(myNBNode->getPosition(), type);
            break;
        }
        case SUMO_ATTR_POSITION: {
            // start geometry moving (because new position affect all junction children)
            startGeometryMoving();
            // set new position in NBNode without updating grid
            moveJunctionGeometry(parse<Position>(value));
            // end geometry moving
            endGeometryMoving();
            // mark this connections and all of the junction's Neighbours as deprecated
            markConnectionsDeprecated(true);
            break;
        }
        case GNE_ATTR_MODIFICATION_STATUS:
            if (myLogicStatus == FEATURE_GUESSED && value != FEATURE_GUESSED) {
                // clear guessed connections. previous connections will be restored
                myNBNode->invalidateIncomingConnections();
                // Clear GNEConnections of incoming edges
                for (const auto& i : myGNEIncomingEdges) {
                    i->clearGNEConnections();
                }
            }
            myLogicStatus = value;
            break;
        case SUMO_ATTR_SHAPE: {
            // start geometry moving (because new position affect all junction children)
            startGeometryMoving();
            // set new shape (without updating grid)
            const PositionVector shape = parse<PositionVector>(value);
            myNBNode->setCustomShape(shape);
            // end geometry moving
            endGeometryMoving();
            // mark this connections and all of the junction's Neighbours as deprecated
            markConnectionsDeprecated(true);
            break;
        }
        case SUMO_ATTR_RADIUS: {
            myNBNode->setRadius(parse<double>(value));
            break;
        }
        case SUMO_ATTR_TLTYPE: {
            // we need to make a copy of controlling TLS (because original will be updated)
            const std::set<NBTrafficLightDefinition*> copyOfTls = myNBNode->getControllingTLS();
            for (const auto& TLS : copyOfTls) {
                TLS->setType(SUMOXMLDefinitions::TrafficLightTypes.get(value));
            }
            break;
        }
        case SUMO_ATTR_RIGHT_OF_WAY:
            myNBNode->setRightOfWay(SUMOXMLDefinitions::RightOfWayValues.get(value));
            break;
        case SUMO_ATTR_FRINGE:
            myNBNode->setFringeType(SUMOXMLDefinitions::FringeTypeValues.get(value));
            break;
        case SUMO_ATTR_NAME:
            myNBNode->setName(value);
            break;
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            myNBNode->setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEJunction::getColorValue(const GUIVisualizationSettings& /* s */, int activeScheme) const {
    switch (activeScheme) {
        case 0:
            if (myColorForMissingConnections) {
                return 3;
            } else {
                return 0;
            }
        case 1:
            return isAttributeCarrierSelected();
        case 2:
            switch (myNBNode->getType()) {
                case SumoXMLNodeType::TRAFFIC_LIGHT:
                    return 0;
                case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
                    return 1;
                case SumoXMLNodeType::PRIORITY:
                    return 2;
                case SumoXMLNodeType::PRIORITY_STOP:
                    return 3;
                case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
                    return 4;
                case SumoXMLNodeType::ALLWAY_STOP:
                    return 5;
                case SumoXMLNodeType::DISTRICT:
                    return 6;
                case SumoXMLNodeType::NOJUNCTION:
                    return 7;
                case SumoXMLNodeType::DEAD_END:
                case SumoXMLNodeType::DEAD_END_DEPRECATED:
                    return 8;
                case SumoXMLNodeType::UNKNOWN:
                    return 8; // may happen before first network computation
                case SumoXMLNodeType::INTERNAL:
                    assert(false);
                    return 8;
                case SumoXMLNodeType::RAIL_SIGNAL:
                    return 9;
                case SumoXMLNodeType::ZIPPER:
                    return 10;
                case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED:
                    return 11;
                case SumoXMLNodeType::RAIL_CROSSING:
                    return 12;
            }
        case 3:
            return myNBNode->getPosition().z();
        default:
            assert(false);
            return 0;
    }
}

void
GNEJunction::checkMissingConnections() {
    for (auto edge : myGNEIncomingEdges) {
        if (edge->getGNEConnections().size() > 0) {
            myColorForMissingConnections = false;
            return;
        }
    }
    // no connections. Use normal color for border edges and cul-de-sac
    if (myGNEIncomingEdges.size() == 0 || myGNEOutgoingEdges.size() == 0) {
        myColorForMissingConnections = false;
        return;
    } else if (myGNEIncomingEdges.size() == 1 && myGNEOutgoingEdges.size() == 1) {
        NBEdge* in = myGNEIncomingEdges[0]->getNBEdge();
        NBEdge* out = myGNEOutgoingEdges[0]->getNBEdge();
        if (in->isTurningDirectionAt(out)) {
            myColorForMissingConnections = false;
            return;
        }
    }
    myColorForMissingConnections = true;
}


void
GNEJunction::moveJunctionGeometry(const Position& pos) {
    // obtain NBNode position
    const Position orig = myNBNode->getPosition();
    // reinit NBNode
    myNBNode->reinit(pos, myNBNode->getType());
    // set new position of adjacent edges
    for (const auto& edge : getNBNode()->getEdges()) {
        myNet->retrieveEdge(edge->getID())->updateJunctionPosition(this, orig);
    }
    // declare three sets with all affected GNEJunctions, GNEEdges and GNEConnections
    std::set<GNEJunction*> affectedJunctions;
    std::set<GNEEdge*> affectedEdges;
    // Iterate over GNEEdges
    for (const auto& edge : getChildEdges()) {
        // Add source and destiny junctions
        affectedJunctions.insert(edge->getParentJunctions().front());
        affectedJunctions.insert(edge->getParentJunctions().back());
        // Obtain neighbors of Junction source
        for (const auto& junctionSourceEdge : edge->getParentJunctions().front()->getChildEdges()) {
            affectedEdges.insert(junctionSourceEdge);
        }
        // Obtain neighbors of Junction destiny
        for (const auto& junctionDestinyEdge : edge->getParentJunctions().back()->getChildEdges()) {
            affectedEdges.insert(junctionDestinyEdge);
        }
    }
    // Iterate over affected Edges
    for (const auto& affectedEdge : affectedEdges) {
        // Update edge geometry
        affectedEdge->updateGeometry();
    }
}


RGBColor
GNEJunction::setColor(const GUIVisualizationSettings& s, bool bubble) const {
    // get active scheme
    const int scheme = s.junctionColorer.getActive();
    // first check if we're editing shape
    if (myShapeEdited) {
        return s.colorSettings.editShape;
    }
    // set default color
    RGBColor color = s.junctionColorer.getScheme().getColor(getColorValue(s, scheme));
    // set special bubble color
    if (bubble && (scheme == 0) && !myColorForMissingConnections) {
        color = s.junctionColorer.getScheme().getColor(1);
    }
    // override with special colors (unless the color scheme is based on selection)
    if (drawUsingSelectColor() && scheme != 1) {
        color = s.colorSettings.selectionColor;
    }
    // set special color if we're creating a new edge
    if (myAmCreateEdgeSource) {
        color = RGBColor::GREEN;
    }
    // overwritte color if we're in data mode
    if (myNet->getViewNet()->getEditModes().isCurrentSupermodeData()) {
        color = s.junctionColorer.getScheme().getColor(6);
    }
    // return color
    return color;
}


void
GNEJunction::addTrafficLight(NBTrafficLightDefinition* tlDef, bool forceInsert) {
    NBTrafficLightLogicCont& tlCont = myNet->getTLLogicCont();
    tlCont.insert(tlDef, forceInsert); // may return false for tlDef which controls multiple junctions
    tlDef->addNode(myNBNode);
}


void
GNEJunction::removeTrafficLight(NBTrafficLightDefinition* tlDef) {
    NBTrafficLightLogicCont& tlCont = myNet->getTLLogicCont();
    if (tlDef->getNodes().size() == 1) {
        tlCont.extract(tlDef);
    }
    myNBNode->removeTrafficLight(tlDef);
}


/****************************************************************************/
