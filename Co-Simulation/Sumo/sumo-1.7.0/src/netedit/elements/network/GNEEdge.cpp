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
/// @file    GNEEdge.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// A road/street connecting two junctions (netedit-version, adapted from GUIEdge)
// Basically a container for an NBEdge with drawing and editing capabilities
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/elements/additional/GNEDetectorE2.h>
#include <netedit/elements/additional/GNERouteProbe.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/changes/GNEChange_Lane.h>
#include <netedit/elements/demand/GNERoute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/options/OptionsCont.h>

#include "GNEConnection.h"
#include "GNECrossing.h"
#include "GNEJunction.h"
#include "GNELane.h"
#include "GNEEdge.h"

//#define DEBUG_SMOOTH_GEOM
//#define DEBUGCOND(obj) (true)
#define VEHICLE_GAP 1

// ===========================================================================
// static
// ===========================================================================
const double GNEEdge::SNAP_RADIUS = SUMO_const_halfLaneWidth;

// ===========================================================================
// members methods
// ===========================================================================

GNEEdge::GNEEdge(GNENet* net, NBEdge* nbe, bool wasSplit, bool loaded):
    GNENetworkElement(net, nbe->getID(), GLO_EDGE, SUMO_TAG_EDGE, {
    net->retrieveJunction(nbe->getFromNode()->getID()), net->retrieveJunction(nbe->getToNode()->getID())
},
{}, {}, {}, {}, {}, {}, {}),
myNBEdge(nbe),
myLanes(0),
myAmResponsible(false),
myWasSplit(wasSplit),
myConnectionStatus(loaded ? FEATURE_LOADED : FEATURE_GUESSED),
myUpdateGeometry(true) {
    // Create lanes
    int numLanes = myNBEdge->getNumLanes();
    myLanes.reserve(numLanes);
    for (int i = 0; i < numLanes; i++) {
        myLanes.push_back(new GNELane(this, i));
        myLanes.back()->incRef("GNEEdge::GNEEdge");
    }
    // update Lane geometries
    for (const auto& i : myLanes) {
        i->updateGeometry();
    }
}


GNEEdge::~GNEEdge() {
    // Delete references to this eddge in lanes
    for (const auto& lane : myLanes) {
        lane->decRef("GNEEdge::~GNEEdge");
        if (lane->unreferenced()) {
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + lane->getTagStr() + " '" + lane->getID() + "' in GNEEdge destructor");
            delete lane;
        }
    }
    // delete references to this eddge in connections
    for (const auto& connection : myGNEConnections) {
        connection->decRef("GNEEdge::~GNEEdge");
        if (connection->unreferenced()) {
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + connection->getTagStr() + " '" + connection->getID() + "' in GNEEdge destructor");
            delete connection;
        }
    }
    if (myAmResponsible) {
        delete myNBEdge;
    }
}


void
GNEEdge::updateGeometry() {
    // first check if myUpdateGeometry flag is enabled
    if (myUpdateGeometry) {
        // Update geometry of lanes
        for (const auto& lane : myLanes) {
            lane->updateGeometry();
        }
        // Update geometry of connections (Only if updateGrid is enabled, because in move mode connections are hidden
        // (note: only the previous marked as deprecated will be updated)
        if (!myMovingGeometryBoundary.isInitialised()) {
            for (const auto& connection : myGNEConnections) {
                connection->updateGeometry();
            }
        }
        // Update geometry of additionals children vinculated to this edge
        for (const auto& childAdditionals : getChildAdditionals()) {
            childAdditionals->updateGeometry();
        }
        // Update geometry of additionals demand elements vinculated to this edge
        for (const auto& childDemandElements : getChildDemandElements()) {
            childDemandElements->computePath();
            childDemandElements->updateGeometry();
        }
        // Update geometry of additionals generic datas vinculated to this edge
        for (const auto& childGenericData : getChildGenericDatas()) {
            childGenericData->updateGeometry();
        }
    }
    // update vehicle geometry
    updateVehicleSpreadGeometries();
    // update vehicle stack labels
    updateVehicleStackLabels();
}


Position
GNEEdge::getPositionInView() const {
    // currently unused
    return Position(0, 0);
}


bool
GNEEdge::clickedOverShapeStart(const Position& pos) {
    if (myNBEdge->getGeometry().front() != getParentJunctions().front()->getPositionInView()) {
        return (myNBEdge->getGeometry().front().distanceTo2D(pos) < SNAP_RADIUS);
    } else {
        return false;
    }
}


bool
GNEEdge::clickedOverShapeEnd(const Position& pos) {
    if (myNBEdge->getGeometry().back() != getParentJunctions().back()->getPositionInView()) {
        return (myNBEdge->getGeometry().back().distanceTo2D(pos) < SNAP_RADIUS);
    } else {
        return false;
    }
}


void
GNEEdge::startShapeBegin() {
    myPositionBeforeMoving = myNBEdge->getGeometry().front();
    // save current centering boundary
    myMovingGeometryBoundary = getCenteringBoundary();
}


void
GNEEdge::startShapeEnd() {
    myPositionBeforeMoving = myNBEdge->getGeometry().back();
    // save current centering boundary
    myMovingGeometryBoundary = getCenteringBoundary();
}


void
GNEEdge::moveShapeBegin(const Position& offset) {
    // change shape startPosition using oldPosition and offset
    Position shapeStartEdited = myPositionBeforeMoving;
    shapeStartEdited.add(offset);
    // snap to active grid
    shapeStartEdited = myNet->getViewNet()->snapToActiveGrid(shapeStartEdited, offset.z() == 0);
    // make sure that start and end position are different
    if (shapeStartEdited != myNBEdge->getGeometry().back()) {
        // set shape start position without updating grid
        setShapeStartPos(shapeStartEdited);
        updateGeometry();
    }
}


void
GNEEdge::moveShapeEnd(const Position& offset) {
    // change shape endPosition using oldPosition and offset
    Position shapeEndEdited = myPositionBeforeMoving;
    shapeEndEdited.add(offset);
    // snap to active grid
    shapeEndEdited = myNet->getViewNet()->snapToActiveGrid(shapeEndEdited, offset.z() == 0);
    // make sure that start and end position are different
    if (shapeEndEdited != myNBEdge->getGeometry().front()) {
        // set shape end position without updating grid
        setShapeEndPos(shapeEndEdited);
        updateGeometry();
    }
}


void
GNEEdge::commitShapeChangeBegin(GNEUndoList* undoList) {
    // first save current shape start position
    Position modifiedShapeStartPos = myNBEdge->getGeometry().front();
    // restore old shape start position
    setShapeStartPos(myPositionBeforeMoving);
    // end geometry moving
    endEdgeGeometryMoving();
    // set attribute using undolist
    undoList->p_begin("shape start of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, GNE_ATTR_SHAPE_START, toString(modifiedShapeStartPos), toString(myPositionBeforeMoving)));
    undoList->p_end();
}


void
GNEEdge::commitShapeChangeEnd(GNEUndoList* undoList) {
    // first save current shape end position
    Position modifiedShapeEndPos = myNBEdge->getGeometry().back();
    // restore old shape end position
    setShapeEndPos(myPositionBeforeMoving);
    // end geometry moving
    endEdgeGeometryMoving();
    // set attribute using undolist
    undoList->p_begin("shape end of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, GNE_ATTR_SHAPE_END, toString(modifiedShapeEndPos), toString(myPositionBeforeMoving)));
    undoList->p_end();
}


int
GNEEdge::getEdgeVertexIndex(Position pos, const bool snapToGrid) const {
    // check if position has to be snapped to grid
    if (snapToGrid) {
        pos = myNet->getViewNet()->snapToActiveGrid(pos);
    }
    const double offset = myNBEdge->getGeometry().nearest_offset_to_point2D(pos, true);
    if (offset == GeomHelper::INVALID_OFFSET) {
        return -1;
    }
    Position newPos = myNBEdge->getGeometry().positionAtOffset2D(offset);
    // first check if vertex already exists in the inner geometry
    for (int i = 0; i < (int)myNBEdge->getGeometry().size(); i++) {
        if (myNBEdge->getGeometry()[i].distanceTo2D(newPos) < SNAP_RADIUS) {
            // index refers to inner geometry
            if (i == 0 || i == (int)(myNBEdge->getGeometry().size() - 1)) {
                return -1;
            }
            return i;
        }
    }
    return -1;
}


void
GNEEdge::startEdgeGeometryMoving(const double shapeOffset, const bool invertOffset) {
    // save current centering boundary
    myMovingGeometryBoundary = getCenteringBoundary();
    // start move shape
    if (invertOffset) {
        startMoveShape(myNBEdge->getGeometry(), myNBEdge->getGeometry().length() - shapeOffset, SNAP_RADIUS);
    } else {
        startMoveShape(myNBEdge->getGeometry(), shapeOffset, SNAP_RADIUS);
    }
    // Save current centering boundary of lanes (and their children)
    for (const auto& lane : myLanes) {
        lane->startGeometryMoving();
    }
    // Save current centering boundary of additionals children vinculated to this edge
    for (const auto& additional : getChildAdditionals()) {
        additional->startGeometryMoving();
    }
    // Save current centering boundary of parent additionals that have this edge as parent
    for (const auto& additional : getParentAdditionals()) {
        additional->startGeometryMoving();
    }
    // Save current centering boundary of demand elements children vinculated to this edge
    for (const auto& demandElement : getChildDemandElements()) {
        demandElement->startGeometryMoving();
    }
    // Save current centering boundary of demand elements parents that have this edge as parent
    for (const auto& demandElement : getParentDemandElements()) {
        demandElement->startGeometryMoving();
    }
}


void
GNEEdge::moveEdgeShape(const Position& offset) {
    // first make a copy of myMovingShape
    PositionVector newShape = getShapeBeforeMoving();
    // move entire shap if this edge and their junctions is selected
    const bool allSelected = mySelected && getParentJunctions().front()->isAttributeCarrierSelected() && getParentJunctions().back()->isAttributeCarrierSelected();
    if (moveEntireShape() || allSelected) {
        // move entire shape
        newShape.add(offset);
    } else {
        int geometryPointIndex = getGeometryPointIndex();
        // if geometryPoint is -1, then we have to create a new geometry point
        if (geometryPointIndex == -1) {
            geometryPointIndex = newShape.insertAtClosest(getPosOverShapeBeforeMoving(), true);
        }
        // move geometry point within newShape
        newShape[geometryPointIndex].add(offset);
        // snap to grid
        newShape[geometryPointIndex] = myNet->getViewNet()->snapToActiveGrid(newShape[geometryPointIndex]);
        // check if edge is selected
        if (isAttributeCarrierSelected()) {
            // move more geometry points, depending if junctions are selected
            if (getParentJunctions().front()->isAttributeCarrierSelected()) {
                for (int i = 1; i < geometryPointIndex; i++) {
                    newShape[i].add(offset);
                }
            }
            if (getParentJunctions().back()->isAttributeCarrierSelected()) {
                for (int i = (geometryPointIndex + 1); i < (int)newShape.size(); i++) {
                    newShape[i].add(offset);
                }
            }
        }
    }
    // pop front and back
    newShape.pop_front();
    newShape.pop_back();
    // set new inner shape
    setGeometry(newShape, true);
}


void
GNEEdge::endEdgeGeometryMoving() {
    // check that endGeometryMoving was called only once
    if (myMovingGeometryBoundary.isInitialised()) {
        // Remove object from net
        myNet->removeGLObjectFromGrid(this);
        // reset myMovingGeometryBoundary
        myMovingGeometryBoundary.reset();
        // Restore centering boundary of lanes (and their children)
        for (const auto& lane : myLanes) {
            lane->endGeometryMoving();
        }
        // Restore centering boundary of additionals children vinculated to this edge
        for (const auto& additional : getChildAdditionals()) {
            additional->endGeometryMoving();
        }
        // Restore centering boundary of parent additionals that have this edge as parent
        for (const auto& additional : getParentAdditionals()) {
            additional->endGeometryMoving();
        }
        // Restore centering boundary of demand elements children vinculated to this edge
        for (const auto& demandElement : getChildDemandElements()) {
            demandElement->endGeometryMoving();
        }
        // Restore centering boundary of demand elements parents that have this edge as parent
        for (const auto& demandElement : getParentDemandElements()) {
            demandElement->endGeometryMoving();
        }
        // add object into grid again (using the new centering boundary)
        myNet->addGLObjectIntoGrid(this);
    }
}


void
GNEEdge::commitEdgeShapeChange(GNEUndoList* undoList) {
    // restore original shape into shapeToCommit
    PositionVector innerShapeToCommit = myNBEdge->getInnerGeometry();
    // first check if second and penultimate isn't in Junction's buubles
    double buubleRadius = myNet->getViewNet()->getVisualisationSettings().neteditSizeSettings.junctionBubbleRadius *
                          myNet->getViewNet()->getVisualisationSettings().junctionSize.exaggeration;
    if (myNBEdge->getGeometry().size() > 2 && myNBEdge->getGeometry()[0].distanceTo2D(myNBEdge->getGeometry()[1]) < buubleRadius) {
        innerShapeToCommit.removeClosest(innerShapeToCommit[0]);
    }
    if (myNBEdge->getGeometry().size() > 2 && myNBEdge->getGeometry()[(int)myNBEdge->getGeometry().size() - 2].distanceTo2D(myNBEdge->getGeometry()[(int)myNBEdge->getGeometry().size() - 1]) < buubleRadius) {
        innerShapeToCommit.removeClosest(innerShapeToCommit[(int)innerShapeToCommit.size() - 1]);
    }
    // second check if double points has to be removed
    innerShapeToCommit.removeDoublePoints(SNAP_RADIUS);
    // show warning if some of edge's shape was merged
    if (innerShapeToCommit.size() != myNBEdge->getInnerGeometry().size()) {
        WRITE_WARNING("Merged shape's point")
    }

    updateGeometry();
    // restore old geometry to allow change attribute (And restore shape if during movement a new point was created
    setGeometry(getShapeBeforeMoving(), false);
    // finish geometry moving
    endEdgeGeometryMoving();
    // commit new shape
    undoList->p_begin("moving " + toString(SUMO_ATTR_SHAPE) + " of " + getTagStr());
    undoList->p_add(new GNEChange_Attribute(this, SUMO_ATTR_SHAPE, toString(innerShapeToCommit)));
    undoList->p_end();
}


void
GNEEdge::deleteEdgeGeometryPoint(const Position& pos, bool allowUndo) {
    // obtain index and remove point
    PositionVector modifiedShape = myNBEdge->getInnerGeometry();
    int index = modifiedShape.indexOfClosest(pos);
    modifiedShape.erase(modifiedShape.begin() + index);
    // set new shape depending of allowUndo
    if (allowUndo) {
        myNet->getViewNet()->getUndoList()->p_begin("delete geometry point");
        setAttribute(SUMO_ATTR_SHAPE, toString(modifiedShape), myNet->getViewNet()->getUndoList());
        myNet->getViewNet()->getUndoList()->p_end();
    } else {
        // set new shape
        setGeometry(modifiedShape, true);
    }
}


void
GNEEdge::updateJunctionPosition(GNEJunction* junction, const Position& origPos) {
    Position delta = junction->getNBNode()->getPosition() - origPos;
    PositionVector geom = myNBEdge->getGeometry();
    // geometry endpoint need not equal junction position hence we modify it with delta
    if (junction == getParentJunctions().front()) {
        geom[0].add(delta);
    } else {
        geom[-1].add(delta);
    }
    setGeometry(geom, false);
}


Boundary
GNEEdge::getCenteringBoundary() const {
    // Return Boundary depending if myMovingGeometryBoundary is initialised (important for move geometry)
    if (myMovingGeometryBoundary.isInitialised()) {
        return myMovingGeometryBoundary;
    }  else {
        Boundary b;
        for (const auto& i : myLanes) {
            b.add(i->getCenteringBoundary());
        }
        // ensure that geometry points are selectable even if the lane geometry is strange
        for (const Position& pos : myNBEdge->getGeometry()) {
            b.add(pos);
        }
        b.grow(10);
        return b;
    }
}

const std::string
GNEEdge::getOptionalName() const {
    return myNBEdge->getStreetName();
}

GUIGLObjectPopupMenu*
GNEEdge::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildNameCopyPopupEntry(ret);
    // build selection and show parameters menu
    myNet->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    // build position copy entry
    buildPositionCopyEntry(ret, false);
    return ret;
}


GNEEdge*
GNEEdge::getOppositeEdge() const {
    return myNet->retrieveEdge(getParentJunctions().back(), getParentJunctions().front(), false);
}


void
GNEEdge::drawGL(const GUIVisualizationSettings& s) const {
    // check if boundary has to be drawn
    if (s.drawBoundaries) {
        GLHelper::drawBoundary(getCenteringBoundary());
    }
    // draw lanes
    for (const auto& lane : myLanes) {
        lane->drawGL(s);
    }
    // draw child additional
    for (const auto& additional : getChildAdditionals()) {
        additional->drawGL(s);
    }
    // draw person stops
    if (myNet->getViewNet()->getNetworkViewOptions().showDemandElements() && myNet->getViewNet()->getDataViewOptions().showDemandElements()) {
        for (const auto& personStopEdge : getChildDemandElements()) {
            if (personStopEdge->getTagProperty().getTag() == GNE_TAG_PERSONSTOP_EDGE) {
                personStopEdge->drawGL(s);
            }
        }
    }
    // draw vehicles
    const std::map<const GNELane*, std::vector<GNEDemandElement*> > vehiclesMap = getVehiclesOverEdgeMap();
    for (const auto& vehicleMap : vehiclesMap) {
        for (const auto& vehicle : vehicleMap.second) {
            vehicle->drawGL(s);
        }
    }
    // draw name if isn't being drawn for selecting
    drawEdgeName(s);
    // draw dotted contours
    if (myLanes.size() > 1) {
        if (s.drawDottedContour() || (myNet->getViewNet()->getInspectedAttributeCarrier() == this)) {
            GNEGeometry::drawDottedContourEdge(true, s, this, true, true);
        }
        if (s.drawDottedContour() || (myNet->getViewNet()->getFrontAttributeCarrier() == this)) {
            GNEGeometry::drawDottedContourEdge(false, s, this, true, true);
        }
    }
}


NBEdge*
GNEEdge::getNBEdge() const {
    return myNBEdge;
}


Position
GNEEdge::getSplitPos(const Position& clickPos) {
    const PositionVector& geom = myNBEdge->getGeometry();
    int index = geom.indexOfClosest(clickPos);
    if (geom[index].distanceTo2D(clickPos) < SNAP_RADIUS) {
        // split at existing geometry point
        return geom[index];
    } else {
        // split straight between the next two points
        return geom.positionAtOffset(geom.nearest_offset_to_point2D(clickPos));
    }
}


void
GNEEdge::editEndpoint(Position pos, GNEUndoList* undoList) {
    if ((myNBEdge->getGeometry().front() != getParentJunctions().front()->getPositionInView()) && (myNBEdge->getGeometry().front().distanceTo2D(pos) < SNAP_RADIUS)) {
        undoList->p_begin("remove endpoint");
        setAttribute(GNE_ATTR_SHAPE_START, "", undoList);
        undoList->p_end();
    } else if ((myNBEdge->getGeometry().back() != getParentJunctions().back()->getPositionInView()) && (myNBEdge->getGeometry().back().distanceTo2D(pos) < SNAP_RADIUS)) {
        undoList->p_begin("remove endpoint");
        setAttribute(GNE_ATTR_SHAPE_END, "", undoList);
        undoList->p_end();
    } else {
        // we need to create new Start/End position over Edge shape, not over clicked position
        double offset = myNBEdge->getGeometry().nearest_offset_to_point2D(myNet->getViewNet()->snapToActiveGrid(pos), true);
        if (offset != GeomHelper::INVALID_OFFSET) {
            PositionVector geom = myNBEdge->getGeometry();
            // calculate position over edge shape relative to clicked positino
            Position newPos = geom.positionAtOffset2D(offset);
            // snap new position to grid
            newPos = myNet->getViewNet()->snapToActiveGrid(newPos);
            undoList->p_begin("set endpoint");
            int index = geom.indexOfClosest(pos);
            // check if snap to existing geometry
            if (geom[index].distanceTo2D(pos) < SNAP_RADIUS) {
                pos = geom[index];
            }
            Position destPos = getParentJunctions().back()->getNBNode()->getPosition();
            Position sourcePos = getParentJunctions().front()->getNBNode()->getPosition();
            if (pos.distanceTo2D(destPos) < pos.distanceTo2D(sourcePos)) {
                setAttribute(GNE_ATTR_SHAPE_END, toString(newPos), undoList);
                getParentJunctions().back()->invalidateShape();
            } else {
                setAttribute(GNE_ATTR_SHAPE_START, toString(newPos), undoList);
                getParentJunctions().front()->invalidateShape();
            }
            // possibly existing inner point is no longer needed
            if (myNBEdge->getInnerGeometry().size() > 0 && getEdgeVertexIndex(pos, false) != -1) {
                deleteEdgeGeometryPoint(pos, false);
            }
            undoList->p_end();
        }
    }
}


void
GNEEdge::resetEndpoint(const Position& pos, GNEUndoList* undoList) {
    Position destPos = getParentJunctions().back()->getNBNode()->getPosition();
    Position sourcePos = getParentJunctions().front()->getNBNode()->getPosition();
    if (pos.distanceTo2D(destPos) < pos.distanceTo2D(sourcePos)) {
        setAttribute(GNE_ATTR_SHAPE_END, toString(destPos), undoList);
        getParentJunctions().back()->invalidateShape();
    } else {
        setAttribute(GNE_ATTR_SHAPE_START, toString(sourcePos), undoList);
        getParentJunctions().front()->invalidateShape();
    }
}


void
GNEEdge::setGeometry(PositionVector geom, bool inner) {
    // set new geometry
    const bool lefthand = OptionsCont::getOptions().getBool("lefthand");
    if (lefthand) {
        geom.mirrorX();
        myNBEdge->mirrorX();
    }
    myNBEdge->setGeometry(geom, inner);
    if (lefthand) {
        myNBEdge->mirrorX();
    }
    // update geometry
    updateGeometry();
    // invalidate junction source shape
    getParentJunctions().front()->invalidateShape();
    // iterate over first parent junction edges and update geometry
    for (const auto& edge : getParentJunctions().front()->getGNEIncomingEdges()) {
        edge->updateGeometry();
    }
    for (const auto& edge : getParentJunctions().front()->getGNEOutgoingEdges()) {
        edge->updateGeometry();
    }
    // invalidate junction destiny shape
    getParentJunctions().back()->invalidateShape();
    // iterate over second parent junction edges and update geometry
    for (const auto& edge : getParentJunctions().front()->getGNEIncomingEdges()) {
        edge->updateGeometry();
    }
    for (const auto& edge : getParentJunctions().back()->getGNEOutgoingEdges()) {
        edge->updateGeometry();
    }
}


const Position
GNEEdge::getFrontUpShapePosition() const {
    PositionVector laneShape = myLanes.front()->getLaneShape();
    laneShape.move2side(myLanes.front()->getParentEdge()->getNBEdge()->getLaneWidth(myLanes.front()->getIndex()) / 2);
    return laneShape.front();
}


const Position
GNEEdge::getFrontDownShapePosition() const {
    PositionVector laneShape = myLanes.back()->getLaneShape();
    laneShape.move2side(-1 * myLanes.back()->getParentEdge()->getNBEdge()->getLaneWidth(myLanes.back()->getIndex()) / 2);
    return laneShape.front();
}


const Position
GNEEdge::getBackUpShapePosition() const {
    PositionVector laneShape = myLanes.front()->getLaneShape();
    laneShape.move2side(myLanes.front()->getParentEdge()->getNBEdge()->getLaneWidth(myLanes.front()->getIndex()) / 2);
    return laneShape.back();
}


const Position
GNEEdge::getBackDownShapePosition() const {
    PositionVector laneShape = myLanes.back()->getLaneShape();
    laneShape.move2side(-1 * myLanes.back()->getParentEdge()->getNBEdge()->getLaneWidth(myLanes.back()->getIndex()) / 2);
    return laneShape.back();
}

void
GNEEdge::remakeGNEConnections() {
    // create new and removed unused GNEConnectinos
    const std::vector<NBEdge::Connection>& connections = myNBEdge->getConnections();
    // create a vector to keep retrieved and created connections
    std::vector<GNEConnection*> retrievedConnections;
    // iterate over NBEdge::Connections of GNEEdge
    for (const auto& connection : connections) {
        // retrieve existent GNEConnection, or create it
        GNEConnection* retrievedGNEConnection = retrieveGNEConnection(connection.fromLane, connection.toEdge, connection.toLane);
        retrievedGNEConnection->updateLinkState();
        retrievedConnections.push_back(retrievedGNEConnection);
        // check if previously this GNEConnections exists, and if true, remove it from myGNEConnections
        std::vector<GNEConnection*>::iterator retrievedExists = std::find(myGNEConnections.begin(), myGNEConnections.end(), retrievedGNEConnection);
        if (retrievedExists != myGNEConnections.end()) {
            myGNEConnections.erase(retrievedExists);
        } else {
            // include reference to created GNEConnection
            retrievedGNEConnection->incRef("GNEEdge::remakeGNEConnections");
        }
        // mark it as deprecated
        retrievedGNEConnection->markConnectionGeometryDeprecated();
    }
    // delete non retrieved GNEConnections
    for (const auto& connection : myGNEConnections) {
        // decrease reference
        connection->decRef();
        // delete GNEConnection if is unreferenced
        if (connection->unreferenced()) {
            // remove it from network
            myNet->addGLObjectIntoGrid(connection);
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + connection->getTagStr() + " '" + connection->getID() + "' in rebuildGNEConnections()");
            delete connection;
        }
    }
    // copy retrieved (existent and created) GNECrossigns to myGNEConnections
    myGNEConnections = retrievedConnections;
}


void
GNEEdge::clearGNEConnections() {
    // Drop all existents connections that aren't referenced anymore
    for (auto i : myGNEConnections) {
        // check if connection is selected
        if (i->isAttributeCarrierSelected()) {
            i->unselectAttributeCarrier();
        }
        // Dec reference of connection
        i->decRef("GNEEdge::clearGNEConnections");
        // Delete GNEConnectionToErase if is unreferenced
        if (i->unreferenced()) {
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + i->getTagStr() + " '" + i->getID() + "' in clearGNEConnections()");
            delete i;
        }
    }
    myGNEConnections.clear();
}


int
GNEEdge::getRouteProbeRelativePosition(GNERouteProbe* routeProbe) const {
    std::vector<GNEAdditional*> routeProbes;
    for (auto i : getChildAdditionals()) {
        if (i->getTagProperty().getTag() == routeProbe->getTagProperty().getTag()) {
            routeProbes.push_back(i);
        }
    }
    // return index of routeProbe in routeProbes vector
    auto it = std::find(routeProbes.begin(), routeProbes.end(), routeProbe);
    if (it == routeProbes.end()) {
        return -1;
    } else {
        return (int)(it - routeProbes.begin());
    }
}


std::vector<GNECrossing*>
GNEEdge::getGNECrossings() {
    std::vector<GNECrossing*> crossings;
    for (auto i : getParentJunctions().front()->getGNECrossings()) {
        if (i->checkEdgeBelong(this)) {
            crossings.push_back(i);
        }
    }
    for (auto i : getParentJunctions().back()->getGNECrossings()) {
        if (i->checkEdgeBelong(this)) {
            crossings.push_back(i);
        }
    }
    return crossings;
}


void
GNEEdge::copyTemplate(GNEEdge* tpl, GNEUndoList* undoList) {
    // begin undo list
    undoList->p_begin("copy template");
    // copy edge-specific attributes
    setAttribute(SUMO_ATTR_NUMLANES,   tpl->getAttribute(SUMO_ATTR_NUMLANES), undoList);
    setAttribute(SUMO_ATTR_TYPE,       tpl->getAttribute(SUMO_ATTR_TYPE), undoList);
    setAttribute(SUMO_ATTR_PRIORITY,   tpl->getAttribute(SUMO_ATTR_PRIORITY), undoList);
    setAttribute(SUMO_ATTR_SPREADTYPE, tpl->getAttribute(SUMO_ATTR_SPREADTYPE), undoList);
    // copy raw values for lane-specific attributes
    setAttribute(SUMO_ATTR_SPEED,      toString(myNBEdge->getSpeed()), undoList);
    setAttribute(SUMO_ATTR_WIDTH,      toString(myNBEdge->getLaneWidth()), undoList);
    setAttribute(SUMO_ATTR_ENDOFFSET,  toString(myNBEdge->getEndOffset()), undoList);
    // copy lane attributes as well
    for (int i = 0; i < (int)myLanes.size(); i++) {
        myLanes[i]->setAttribute(SUMO_ATTR_ALLOW, tpl->myLanes[i]->getAttribute(SUMO_ATTR_ALLOW), undoList);
        myLanes[i]->setAttribute(SUMO_ATTR_SPEED, tpl->myLanes[i]->getAttribute(SUMO_ATTR_SPEED), undoList);
        myLanes[i]->setAttribute(SUMO_ATTR_WIDTH, tpl->myLanes[i]->getAttribute(SUMO_ATTR_WIDTH), undoList);
        myLanes[i]->setAttribute(SUMO_ATTR_ENDOFFSET, tpl->myLanes[i]->getAttribute(SUMO_ATTR_ENDOFFSET), undoList);
    }
    // end undo list
    undoList->p_end();
}


std::set<GUIGlID>
GNEEdge::getLaneGlIDs() const {
    std::set<GUIGlID> result;
    for (auto i : myLanes) {
        result.insert(i->getGlID());
    }
    return result;
}


const std::vector<GNELane*>&
GNEEdge::getLanes() const {
    return myLanes;
}


const std::vector<GNEConnection*>&
GNEEdge::getGNEConnections() const {
    return myGNEConnections;
}


bool
GNEEdge::wasSplit() {
    return myWasSplit;
}


std::string
GNEEdge::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_FROM:
            return getParentJunctions().front()->getID();
        case SUMO_ATTR_TO:
            return getParentJunctions().back()->getID();
        case SUMO_ATTR_NUMLANES:
            return toString(myNBEdge->getNumLanes());
        case SUMO_ATTR_PRIORITY:
            return toString(myNBEdge->getPriority());
        case SUMO_ATTR_LENGTH:
            return toString(myNBEdge->getFinalLength());
        case SUMO_ATTR_TYPE:
            return myNBEdge->getTypeID();
        case SUMO_ATTR_SHAPE:
            return toString(myNBEdge->getInnerGeometry());
        case SUMO_ATTR_SPREADTYPE:
            return toString(myNBEdge->getLaneSpreadFunction());
        case SUMO_ATTR_NAME:
            return myNBEdge->getStreetName();
        case SUMO_ATTR_ALLOW:
            return (getVehicleClassNames(myNBEdge->getPermissions()) + (myNBEdge->hasLaneSpecificPermissions() ? " (combined!)" : ""));
        case SUMO_ATTR_DISALLOW: {
            return (getVehicleClassNames(invertPermissions(myNBEdge->getPermissions())) + (myNBEdge->hasLaneSpecificPermissions() ? " (combined!)" : ""));
        }
        case SUMO_ATTR_SPEED:
            if (myNBEdge->hasLaneSpecificSpeed()) {
                return "lane specific";
            } else {
                return toString(myNBEdge->getSpeed());
            }
        case SUMO_ATTR_WIDTH:
            if (myNBEdge->hasLaneSpecificWidth()) {
                return "lane specific";
            } else {
                return toString(myNBEdge->getLaneWidth());
            }
        case SUMO_ATTR_ENDOFFSET:
            if (myNBEdge->hasLaneSpecificEndOffset()) {
                return "lane specific";
            } else {
                return toString(myNBEdge->getEndOffset());
            }
        case SUMO_ATTR_DISTANCE:
            return toString(myNBEdge->getDistance());
        case GNE_ATTR_MODIFICATION_STATUS:
            return myConnectionStatus;
        case GNE_ATTR_SHAPE_START:
            if (myNBEdge->getGeometry().front() == getParentJunctions().front()->getPositionInView()) {
                return "";
            } else {
                return toString(myNBEdge->getGeometry().front());
            }
        case GNE_ATTR_SHAPE_END:
            if (myNBEdge->getGeometry().back() == getParentJunctions().back()->getPositionInView()) {
                return "";
            } else {
                return toString(myNBEdge->getGeometry().back());
            }
        case GNE_ATTR_BIDIR:
            return toString(myNBEdge->isBidiRail());
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return myNBEdge->getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


std::string
GNEEdge::getAttributeForSelection(SumoXMLAttr key) const {
    std::string result = getAttribute(key);
    if ((key == SUMO_ATTR_ALLOW || key == SUMO_ATTR_DISALLOW) && result.find("all") != std::string::npos) {
        result += " " + getVehicleClassNames(SVCAll, true);
    }
    return result;
}


void
GNEEdge::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    switch (key) {
        case SUMO_ATTR_WIDTH:
        case SUMO_ATTR_ENDOFFSET:
        case SUMO_ATTR_SPEED:
        case SUMO_ATTR_ALLOW:
        case SUMO_ATTR_DISALLOW: {
            undoList->p_begin("change " + getTagStr() + " attribute");
            const std::string origValue = myLanes.at(0)->getAttribute(key); // will have intermediate value of "lane specific"
            // lane specific attributes need to be changed via lanes to allow undo
            for (auto it : myLanes) {
                it->setAttribute(key, value, undoList);
            }
            // ensure that the edge value is also changed. Actually this sets the lane attributes again but it does not matter
            undoList->p_add(new GNEChange_Attribute(this, key, value, origValue));
            undoList->p_end();
            break;
        }
        case SUMO_ATTR_FROM: {
            undoList->p_begin("change  " + getTagStr() + "  attribute");
            // Remove edge from crossings of junction source
            removeEdgeFromCrossings(getParentJunctions().front(), undoList);
            // continue changing from junction
            GNEJunction* originalFirstParentJunction = getParentJunctions().front();
            getParentJunctions().front()->setLogicValid(false, undoList);
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            getParentJunctions().front()->setLogicValid(false, undoList);
            myNet->retrieveJunction(value)->setLogicValid(false, undoList);
            setAttribute(GNE_ATTR_SHAPE_START, toString(getParentJunctions().front()->getNBNode()->getPosition()), undoList);
            getParentJunctions().front()->invalidateShape();
            undoList->p_end();
            // update geometries of all implicated junctions
            originalFirstParentJunction->updateGeometry();
            getParentJunctions().front()->updateGeometry();
            getParentJunctions().back()->updateGeometry();
            break;
        }
        case SUMO_ATTR_TO: {
            undoList->p_begin("change  " + getTagStr() + "  attribute");
            // Remove edge from crossings of junction destiny
            removeEdgeFromCrossings(getParentJunctions().back(), undoList);
            // continue changing destiny junction
            GNEJunction* originalSecondParentJunction = getParentJunctions().back();
            getParentJunctions().back()->setLogicValid(false, undoList);
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            getParentJunctions().back()->setLogicValid(false, undoList);
            myNet->retrieveJunction(value)->setLogicValid(false, undoList);
            setAttribute(GNE_ATTR_SHAPE_END, toString(getParentJunctions().back()->getNBNode()->getPosition()), undoList);
            getParentJunctions().back()->invalidateShape();
            undoList->p_end();
            // update geometries of all implicated junctions
            originalSecondParentJunction->updateGeometry();
            getParentJunctions().back()->updateGeometry();
            getParentJunctions().front()->updateGeometry();
            break;
        }
        case SUMO_ATTR_ID:
        case SUMO_ATTR_PRIORITY:
        case SUMO_ATTR_LENGTH:
        case SUMO_ATTR_TYPE:
        case SUMO_ATTR_SPREADTYPE:
        case SUMO_ATTR_DISTANCE:
        case GNE_ATTR_MODIFICATION_STATUS:
        case GNE_ATTR_SHAPE_START:
        case GNE_ATTR_SHAPE_END:
        case GNE_ATTR_SELECTED:
        case GNE_ATTR_PARAMETERS:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        case SUMO_ATTR_NAME:
            // user cares about street names. Make sure they appear in the output
            OptionsCont::getOptions().resetWritable();
            OptionsCont::getOptions().set("output.street-names", "true");
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        case SUMO_ATTR_NUMLANES:
            if (value != getAttribute(key)) {
                // Remove edge from crossings of junction source
                removeEdgeFromCrossings(getParentJunctions().front(), undoList);
                // Remove edge from crossings of junction destiny
                removeEdgeFromCrossings(getParentJunctions().back(), undoList);
                // set num lanes
                setNumLanes(parse<int>(value), undoList);
            }
            break;
        case SUMO_ATTR_SHAPE:
            // @note: assumes value of inner geometry!
            // actually the geometry is already updated (incrementally
            // during mouse movement). We set the restore point to the end
            // of the last change-set
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        case GNE_ATTR_BIDIR:
            throw InvalidArgument("Attribute of '" + toString(key) + "' cannot be modified");
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEEdge::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            return SUMOXMLDefinitions::isValidNetID(value) && (myNet->retrieveEdge(value, false) == nullptr);
        case SUMO_ATTR_FROM: {
            // check that is a valid ID and is different of ID of junction destiny
            if (SUMOXMLDefinitions::isValidNetID(value) && (value != getParentJunctions().back()->getID())) {
                GNEJunction* junctionFrom = myNet->retrieveJunction(value, false);
                // check that there isn't already another edge with the same From and To Edge
                if ((junctionFrom != nullptr) && (myNet->retrieveEdge(junctionFrom, getParentJunctions().back(), false) == nullptr)) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        case SUMO_ATTR_TO: {
            // check that is a valid ID and is different of ID of junction Source
            if (SUMOXMLDefinitions::isValidNetID(value) && (value != getParentJunctions().front()->getID())) {
                GNEJunction* junctionTo = myNet->retrieveJunction(value, false);
                // check that there isn't already another edge with the same From and To Edge
                if ((junctionTo != nullptr) && (myNet->retrieveEdge(getParentJunctions().front(), junctionTo, false) == nullptr)) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
        case SUMO_ATTR_SPEED:
            return canParse<double>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_NUMLANES:
            return canParse<int>(value) && (parse<double>(value) > 0);
        case SUMO_ATTR_PRIORITY:
            return canParse<int>(value);
        case SUMO_ATTR_LENGTH:
            return canParse<double>(value) && ((parse<double>(value) > 0) || (parse<double>(value) == NBEdge::UNSPECIFIED_LOADED_LENGTH));
        case SUMO_ATTR_ALLOW:
        case SUMO_ATTR_DISALLOW:
            return canParseVehicleClasses(value);
        case SUMO_ATTR_TYPE:
            return true;
        case SUMO_ATTR_SHAPE:
            // empty shapes are allowed
            return canParse<PositionVector>(value);
        case SUMO_ATTR_SPREADTYPE:
            return SUMOXMLDefinitions::LaneSpreadFunctions.hasString(value);
        case SUMO_ATTR_NAME:
            return true;
        case SUMO_ATTR_WIDTH:
            return canParse<double>(value) && ((parse<double>(value) >= -1) || (parse<double>(value) == NBEdge::UNSPECIFIED_WIDTH));
        case SUMO_ATTR_ENDOFFSET:
            return canParse<double>(value) && parse<double>(value) >= 0 && parse<double>(value) < myNBEdge->getLoadedLength();
        case SUMO_ATTR_DISTANCE:
            return canParse<double>(value);
        case GNE_ATTR_SHAPE_START: {
            if (value.empty()) {
                return true;
            } else if (canParse<Position>(value)) {
                Position shapeStart = parse<Position>(value);
                return (shapeStart != myNBEdge->getGeometry()[-1]);
            } else {
                return false;
            }
        }
        case GNE_ATTR_SHAPE_END: {
            if (value.empty()) {
                return true;
            } else if (canParse<Position>(value)) {
                Position shapeEnd = parse<Position>(value);
                return (shapeEnd != myNBEdge->getGeometry()[0]);
            } else {
                return false;
            }
        }
        case GNE_ATTR_BIDIR:
            return false;
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEEdge::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case GNE_ATTR_BIDIR:
            return false;
        default:
            return true;
    }
}


void
GNEEdge::setResponsible(bool newVal) {
    myAmResponsible = newVal;
}


GNELane*
GNEEdge::getLaneByAllowedVClass(const SUMOVehicleClass vClass) const {
    // iterate over all NBEdge lanes
    for (int i = 0; i < (int)myNBEdge->getLanes().size(); i++) {
        // if given VClass is in permissions, return lane
        if (myNBEdge->getLanes().at(i).permissions & vClass) {
            // return GNELane
            return myLanes.at(i);
        }
    }
    // return first lane
    return myLanes.front();
}


GNELane*
GNEEdge::getLaneByDisallowedVClass(const SUMOVehicleClass vClass) const {
    // iterate over all NBEdge lanes
    for (int i = 0; i < (int)myNBEdge->getLanes().size(); i++) {
        // if given VClass isn't in permissions, return lane
        if (~(myNBEdge->getLanes().at(i).permissions) & vClass) {
            // return GNELane
            return myLanes.at(i);
        }
    }
    // return first lane
    return myLanes.front();
}


void
GNEEdge::updateVehicleSpreadGeometries() {
    // get lane vehicles map
    const std::map<const GNELane*, std::vector<GNEDemandElement*> > laneVehiclesMap = getVehiclesOverEdgeMap();
    // iterate over every lane
    for (const auto& laneVehicle : laneVehiclesMap) {
        // obtain total lenght
        double totalLength = 0;
        for (const auto& vehicle : laneVehicle.second) {
            totalLength += vehicle->getAttributeDouble(SUMO_ATTR_LENGTH) + VEHICLE_GAP;
        }
        // calculate multiplier for vehicle positions
        double multiplier = 1;
        const double laneShapeLenght = laneVehicle.first->getLaneShape().length();
        if (laneShapeLenght == 0) {
            multiplier = 0;
        } else if (totalLength > laneShapeLenght) {
            multiplier = (laneShapeLenght / totalLength);
        }
        // declare current lenght
        double lenght = 0;
        // iterate over vehicles to calculate position and rotations
        for (const auto& vehicle : laneVehicle.second) {
            vehicle->updateDemandElementSpreadGeometry(laneVehicle.first, lenght * multiplier);
            // update lenght
            lenght += vehicle->getAttributeDouble(SUMO_ATTR_LENGTH) + VEHICLE_GAP;
        }
    }
}


void
GNEEdge::updateVehicleStackLabels() {
    // get lane vehicles map
    const std::map<const GNELane*, std::vector<GNEDemandElement*> > laneVehiclesMap = getVehiclesOverEdgeMap();
    // iterate over laneVehiclesMap and obtain a vector with
    for (const auto& vehicleMap : laneVehiclesMap) {
        // declare map for sprt vehicles using their departpos+lenght position (StackPosition)
        std::vector<std::pair<GNEEdge::StackPosition, GNEDemandElement*> > departPosVehicles;
        // declare vector of stack demand elements
        std::vector<GNEEdge::StackDemandElements> stackedVehicles;
        // iterate over vehicles
        for (const auto& vehicle : vehicleMap.second) {
            // get vehicle's depart pos and lenght
            const double departPos = vehicle->getAttributeDouble(SUMO_ATTR_DEPARTPOS);
            const double length = vehicle->getAttributeDouble(SUMO_ATTR_LENGTH);
            double posOverLane = vehicle->getAttributeDouble(SUMO_ATTR_DEPARTPOS);
            // check if we have to adapt posOverLane
            if (posOverLane < 0) {
                posOverLane += vehicleMap.first->getLaneShape().length();
            }
            // make a stack position using departPos and length
            departPosVehicles.push_back(std::make_pair(StackPosition(departPos, length), vehicle));
            // update depart element geometry
            vehicle->updateDemandElementGeometry(vehicleMap.first, posOverLane);
            // reset vehicle stack label
            vehicle->updateDemandElementStackLabel(0);
        }

        // sort departPosVehicles
        std::sort(departPosVehicles.begin(), departPosVehicles.end());
        // iterate over departPosVehicles
        for (const auto& departPosVehicle : departPosVehicles) {
            // obtain stack position and vehicle
            const GNEEdge::StackPosition& vehicleStackPosition = departPosVehicle.first;
            GNEDemandElement* vehicle = departPosVehicle.second;
            // if stackedVehicles is empty, add a new StackDemandElements
            if (stackedVehicles.empty()) {
                stackedVehicles.push_back(GNEEdge::StackDemandElements(vehicleStackPosition, vehicle));
            } else if (areStackPositionOverlapped(vehicleStackPosition, stackedVehicles.back().getStackPosition())) {
                // add new vehicle to last inserted stackDemandElements
                stackedVehicles[stackedVehicles.size() - 1].addDemandElements(vehicle);
            } else {
                // No overlapping, then add a new StackDemandElements
                stackedVehicles.push_back(GNEEdge::StackDemandElements(vehicleStackPosition, vehicle));
            }
        }
        // iterate over stackedVehicles
        for (const auto& vehicle : stackedVehicles) {
            // only update vehicles with one or more stack
            if (vehicle.getDemandElements().size() > 1) {
                // set stack labels
                vehicle.getDemandElements().front()->updateDemandElementStackLabel((int)vehicle.getDemandElements().size());
            }
        }
    }
}


void
GNEEdge::drawEdgeGeometryPoints(const GUIVisualizationSettings& s, const GNELane* lane) const {
    // first check conditions
    if ((lane == myLanes.back()) && (s.scale > 8.0) && !myNet->getViewNet()->getEditModes().isCurrentSupermodeDemand()) {
        // Obtain exaggeration of the draw
        const double exaggeration = s.addSize.getExaggeration(s, this);
        // obtain circle width
        bool drawBig = (myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_MOVE ||
                        myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_DELETE);
        double circleWidth = drawBig ? SNAP_RADIUS * MIN2((double)1, s.laneWidthExaggeration) : 0.5;
        double circleWidthSquared = circleWidth * circleWidth;
        // obtain color
        RGBColor color = s.junctionColorer.getSchemes()[0].getColor(2);
        if (drawUsingSelectColor() && s.laneColorer.getActive() != 1) {
            // override with special colors (unless the color scheme is based on selection)
            color = s.colorSettings.selectedEdgeColor.changedBrightness(-20);
        }
        GLHelper::setColor(color);
        // recognize full transparency and simply don't draw
        if (color.alpha() > 0) {
            // add edge layer matrix
            glPushMatrix();
            // translate to front
            glTranslated(0, 0, 1);
            // draw geometry points expect initial and final
            for (int i = 1; i < (int)myNBEdge->getGeometry().size() - 1; i++) {
                Position pos = myNBEdge->getGeometry()[i];
                if (!s.drawForRectangleSelection || (myNet->getViewNet()->getPositionInformation().distanceSquaredTo2D(pos) <= (circleWidthSquared + 2))) {
                    glPushMatrix();
                    glTranslated(pos.x(), pos.y(), 0.1);
                    // resolution of drawn circle depending of the zoom (To improve smothness)
                    GLHelper::drawFilledCircle(circleWidth, s.getCircleResolution());
                    glPopMatrix();
                    // draw elevation or special symbols (Start, End and Block)
                    if (!s.drawForRectangleSelection && myNet->getViewNet()->getNetworkViewOptions().editingElevation()) {
                        glPushMatrix();
                        // Translate to geometry point
                        glTranslated(pos.x(), pos.y(), 0.2);
                        // draw Z value
                        GLHelper::drawText(toString(pos.z()), Position(), GLO_MAX - 5, s.edgeValue.scaledSize(s.scale) / 2, s.edgeValue.color);
                        glPopMatrix();
                    }
                }
            }
            // draw line geometry, start and end points if shapeStart or shape end is edited, and depending of drawForRectangleSelection
            if (myNet->getViewNet()->getEditModes().networkEditMode == NetworkEditMode::NETWORK_MOVE) {
                if ((myNBEdge->getGeometry().front() != getParentJunctions().front()->getPositionInView()) &&
                        (!s.drawForRectangleSelection || (myNet->getViewNet()->getPositionInformation().distanceSquaredTo2D(myNBEdge->getGeometry().front()) <= (circleWidthSquared + 2)))) {
                    glPushMatrix();
                    glTranslated(myNBEdge->getGeometry().front().x(), myNBEdge->getGeometry().front().y(), 0.1);
                    // resolution of drawn circle depending of the zoom (To improve smothness)
                    GLHelper::drawFilledCircle(circleWidth, s.getCircleResolution());
                    glPopMatrix();
                    // draw a "s" over last point depending of drawForRectangleSelection
                    if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.geometryPointsText, exaggeration)) {
                        glPushMatrix();
                        glTranslated(myNBEdge->getGeometry().front().x(), myNBEdge->getGeometry().front().y(), 0.2);
                        GLHelper::drawText("S", Position(), 0, circleWidth, RGBColor::WHITE);
                        glPopMatrix();
                        // draw line between Junction and point
                        glPushMatrix();
                        glTranslated(0, 0, 0.1);
                        glLineWidth(4);
                        GLHelper::drawLine(myNBEdge->getGeometry().front(), getParentJunctions().front()->getPositionInView());
                        // draw line between begin point of last lane shape and the first edge shape point
                        GLHelper::drawLine(myNBEdge->getGeometry().front(), myNBEdge->getLanes().back().shape.front());
                        glPopMatrix();
                    }
                }
                if ((myNBEdge->getGeometry().back() != getParentJunctions().back()->getPositionInView()) &&
                        (!s.drawForRectangleSelection || (myNet->getViewNet()->getPositionInformation().distanceSquaredTo2D(myNBEdge->getGeometry().back()) <= (circleWidthSquared + 2)))) {
                    glPushMatrix();
                    glTranslated(myNBEdge->getGeometry().back().x(), myNBEdge->getGeometry().back().y(), 0.1);
                    // resolution of drawn circle depending of the zoom (To improve smothness)
                    GLHelper::drawFilledCircle(circleWidth, s.getCircleResolution());
                    glPopMatrix();
                    // draw a "e" over last point depending of drawForRectangleSelection
                    if (!s.drawForRectangleSelection && s.drawDetail(s.detailSettings.geometryPointsText, exaggeration)) {
                        glPushMatrix();
                        glTranslated(myNBEdge->getGeometry().back().x(), myNBEdge->getGeometry().back().y(), 0.2);
                        GLHelper::drawText("E", Position(), 0, circleWidth, RGBColor::WHITE);
                        glPopMatrix();
                        // draw line between Junction and point
                        glPushMatrix();
                        glTranslated(0, 0, 0.1);
                        glLineWidth(4);
                        GLHelper::drawLine(myNBEdge->getGeometry().back(), getParentJunctions().back()->getPositionInView());
                        // draw line between last point of first lane shape and the last edge shape point
                        GLHelper::drawLine(myNBEdge->getGeometry().back(), myNBEdge->getLanes().back().shape.back());
                        glPopMatrix();
                    }
                }
            }
            // pop edge layer matrix
            glPopMatrix();
        }
    }
}

// ===========================================================================
// private
// ===========================================================================

GNEEdge::StackPosition::StackPosition(const double departPos, const double length) :
    tuple(departPos, departPos + length) {
}


double
GNEEdge::StackPosition::beginPosition() const {
    return std::get<0>(*this);
}


double
GNEEdge::StackPosition::endPosition() const {
    return std::get<1>(*this);
}


GNEEdge::StackDemandElements::StackDemandElements(const StackPosition stackedPosition, GNEDemandElement* demandElement) :
    tuple(stackedPosition, {
    demandElement
}) {
}


void
GNEEdge::StackDemandElements::addDemandElements(GNEDemandElement* demandElement) {
    std::get<1>(*this).push_back(demandElement);
}


const GNEEdge::StackPosition&
GNEEdge::StackDemandElements::getStackPosition() const {
    return std::get<0>(*this);
}


const std::vector<GNEDemandElement*>&
GNEEdge::StackDemandElements::getDemandElements() const {
    return std::get<1>(*this);
}


void
GNEEdge::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            break;
        case SUMO_ATTR_FROM:
            myNet->changeEdgeEndpoints(this, value, getParentJunctions().back()->getID());
            // update this edge of list of outgoings edges of the old first parent junction
            getParentJunctions().front()->removeOutgoingGNEEdge(this);
            // update first parent junction
            updateFirstParentJunction(value);
            // update this edge of list of outgoings edges of the new first parent junction
            getParentJunctions().front()->addOutgoingGNEEdge(this);
            break;
        case SUMO_ATTR_TO:
            myNet->changeEdgeEndpoints(this, getParentJunctions().front()->getID(), value);
            // update this edge of list of incomings edges of the old second parent junction
            getParentJunctions().back()->removeIncomingGNEEdge(this);
            // update second parent junction
            updateSecondParentJunction(value);
            // update this edge of list of incomings edges of the new second parent junction
            getParentJunctions().back()->addIncomingGNEEdge(this);
            break;
        case SUMO_ATTR_NUMLANES:
            throw InvalidArgument("GNEEdge::setAttribute (private) called for attr SUMO_ATTR_NUMLANES. This should never happen");
            break;
        case SUMO_ATTR_PRIORITY:
            myNBEdge->myPriority = parse<int>(value);
            break;
        case SUMO_ATTR_LENGTH:
            myNBEdge->setLoadedLength(parse<double>(value));
            break;
        case SUMO_ATTR_TYPE:
            myNBEdge->myType = value;
            break;
        case SUMO_ATTR_SHAPE:
            // start geometry moving (because a new shape affect all child edges)
            startEdgeGeometryMoving(-1, false);
            // set new geometry
            setGeometry(parse<PositionVector>(value), true);
            // start geometry moving (because a new shape affect all child edges)
            endEdgeGeometryMoving();
            break;
        case SUMO_ATTR_SPREADTYPE:
            myNBEdge->setLaneSpreadFunction(SUMOXMLDefinitions::LaneSpreadFunctions.get(value));
            break;
        case SUMO_ATTR_NAME:
            myNBEdge->setStreetName(value);
            break;
        case SUMO_ATTR_SPEED:
            myNBEdge->setSpeed(-1, parse<double>(value));
            break;
        case SUMO_ATTR_WIDTH:
            myNBEdge->setLaneWidth(-1, parse<double>(value));
            break;
        case SUMO_ATTR_ENDOFFSET:
            myNBEdge->setEndOffset(-1, parse<double>(value));
            break;
        case SUMO_ATTR_ALLOW:
            break;  // no edge value
        case SUMO_ATTR_DISALLOW:
            break; // no edge value
        case SUMO_ATTR_DISTANCE:
            myNBEdge->setDistance(parse<double>(value));
            break;
        case GNE_ATTR_MODIFICATION_STATUS:
            myConnectionStatus = value;
            if (value == FEATURE_GUESSED) {
                WRITE_DEBUG("invalidating (removing) connections of edge '" + getID() + "' due it were guessed");
                myNBEdge->invalidateConnections(true);
                clearGNEConnections();
            } else if (value != FEATURE_GUESSED) {
                WRITE_DEBUG("declaring connections of edge '" + getID() + "' as loaded (It will not be removed)");
                myNBEdge->declareConnectionsAsLoaded();
            }
            break;
        case GNE_ATTR_SHAPE_START: {
            // get geometry of NBEdge, remove FIRST element with the new value (or with the Junction Source position) and set it back to edge
            Position newShapeStart;
            if (value == "") {
                newShapeStart = getParentJunctions().front()->getPositionInView();
            } else {
                newShapeStart = parse<Position>(value);
            }
            // Remove object from net
            myNet->removeGLObjectFromGrid(this);
            // set shape start position
            setShapeStartPos(newShapeStart);
            // add object from net
            myNet->addGLObjectIntoGrid(this);
            break;
        }
        case GNE_ATTR_SHAPE_END: {
            // get geometry of NBEdge, remove LAST element with the new value (or with the Junction Destiny position) and set it back to edge
            Position newShapeEnd;
            if (value == "") {
                newShapeEnd = getParentJunctions().back()->getPositionInView();
            } else {
                newShapeEnd = parse<Position>(value);
            }
            // Remove object from net
            myNet->removeGLObjectFromGrid(this);
            // set shape end position
            setShapeEndPos(newShapeEnd);
            // add object from net
            myNet->addGLObjectIntoGrid(this);
            break;
        }
        case GNE_ATTR_BIDIR:
            throw InvalidArgument("Attribute of '" + toString(key) + "' cannot be modified");
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            myNBEdge->setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEEdge::setNumLanes(int numLanes, GNEUndoList* undoList) {
    // begin undo list
    undoList->p_begin("change number of " + toString(SUMO_TAG_LANE) +  "s");
    // invalidate logic of source/destiny edges
    getParentJunctions().front()->setLogicValid(false, undoList);
    getParentJunctions().back()->setLogicValid(false, undoList);
    // disable update geometry (see #6336)
    myUpdateGeometry = false;
    // remove edge of RTREE
    myNet->removeGLObjectFromGrid(this);
    const int oldNumLanes = (int)myLanes.size();
    for (int i = oldNumLanes; i < numLanes; i++) {
        // since the GNELane does not exist yet, it cannot have yet been referenced so we only pass a zero-pointer
        undoList->add(new GNEChange_Lane(this, myNBEdge->getLaneStruct(oldNumLanes - 1)), true);
    }
    for (int i = (oldNumLanes - 1); i > (numLanes - 1); i--) {
        // delete leftmost lane
        undoList->add(new GNEChange_Lane(this, myLanes[i], myNBEdge->getLaneStruct(i), false), true);
    }
    // enable updateGeometry again
    myUpdateGeometry = true;
    // insert edge in RTREE again
    myNet->addGLObjectIntoGrid(this);
    // update geometry of entire edge
    updateGeometry();
    // end undo list
    undoList->p_end();
}


void
GNEEdge::updateFirstParentJunction(const std::string& value) {
    std::vector<GNEJunction*> parentJunctions = getParentJunctions();
    parentJunctions[0] = myNet->retrieveJunction(value);
    // replace parent junctions
    replaceParentElements(this, parentJunctions);
}


void
GNEEdge::updateSecondParentJunction(const std::string& value) {
    std::vector<GNEJunction*> parentJunctions = getParentJunctions();
    parentJunctions[1] = myNet->retrieveJunction(value);
    // replace parent junctions
    replaceParentElements(this, parentJunctions);
}


void
GNEEdge::addLane(GNELane* lane, const NBEdge::Lane& laneAttrs, bool recomputeConnections) {
    // boundary of edge depends of number of lanes. We need to extract if before add or remove lane
    if (myUpdateGeometry) {
        myNet->removeGLObjectFromGrid(this);
    }
    const int index = lane ? lane->getIndex() : myNBEdge->getNumLanes();
    // the laneStruct must be created first to ensure we have some geometry
    // unless the connections are fully recomputed, existing indices must be shifted
    myNBEdge->addLane(index, true, recomputeConnections, !recomputeConnections);
    if (lane) {
        // restore a previously deleted lane
        myLanes.insert(myLanes.begin() + index, lane);

    } else {
        // create a new lane by copying leftmost lane
        lane = new GNELane(this, index);
        myLanes.push_back(lane);
    }
    lane->incRef("GNEEdge::addLane");
    // check if lane is selected
    if (lane->isAttributeCarrierSelected()) {
        lane->selectAttributeCarrier();
    }
    // we copy all attributes except shape since this is recomputed from edge shape
    myNBEdge->setSpeed(lane->getIndex(), laneAttrs.speed);
    myNBEdge->setPermissions(laneAttrs.permissions, lane->getIndex());
    myNBEdge->setPreferredVehicleClass(laneAttrs.preferred, lane->getIndex());
    myNBEdge->setEndOffset(lane->getIndex(), laneAttrs.endOffset);
    myNBEdge->setLaneWidth(lane->getIndex(), laneAttrs.width);
    // udate indices
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        myLanes[i]->setIndex(i);
    }
    /* while technically correct, this looks ugly
    getParentJunctions().front()->invalidateShape();
    getParentJunctions().back()->invalidateShape();
    */
    // Remake connections for this edge and all edges that target this lane
    remakeGNEConnections();
    // remake connections of all edges of junction source and destiny
    for (auto i : getParentJunctions().front()->getChildEdges()) {
        i->remakeGNEConnections();
    }
    // remake connections of all edges of junction source and destiny
    for (auto i : getParentJunctions().back()->getChildEdges()) {
        i->remakeGNEConnections();
    }
    // add object again
    if (myUpdateGeometry) {
        myNet->addGLObjectIntoGrid(this);
    }
    // Update geometry with the new lane
    updateGeometry();
}


void
GNEEdge::removeLane(GNELane* lane, bool recomputeConnections) {
    // boundary of edge depends of number of lanes. We need to extract if before add or remove lane
    if (myUpdateGeometry) {
        myNet->removeGLObjectFromGrid(this);
    }
    if (myLanes.size() == 0) {
        throw ProcessError("Should not remove the last " + toString(SUMO_TAG_LANE) + " from an " + getTagStr());
    }
    if (lane == nullptr) {
        lane = myLanes.back();
    }
    // check if lane is selected
    if (lane->isAttributeCarrierSelected()) {
        lane->unselectAttributeCarrier();
    }
    // Delete lane of edge's container
    // unless the connections are fully recomputed, existing indices must be shifted
    myNBEdge->deleteLane(lane->getIndex(), recomputeConnections, !recomputeConnections);
    lane->decRef("GNEEdge::removeLane");
    myLanes.erase(myLanes.begin() + lane->getIndex());
    // Delete lane if is unreferenced
    if (lane->unreferenced()) {
        // show extra information for tests
        WRITE_DEBUG("Deleting unreferenced " + lane->getTagStr() + " '" + lane->getID() + "' in removeLane()");
        delete lane;
    }
    // udate indices
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        myLanes[i]->setIndex(i);
    }
    /* while technically correct, this looks ugly
    getParentJunctions().front()->invalidateShape();
    getParentJunctions().back()->invalidateShape();
    */
    // Remake connections of this edge
    remakeGNEConnections();
    // remake connections of all edges of junction source and destiny
    for (auto i : getParentJunctions().front()->getChildEdges()) {
        i->remakeGNEConnections();
    }
    // remake connections of all edges of junction source and destiny
    for (auto i : getParentJunctions().back()->getChildEdges()) {
        i->remakeGNEConnections();
    }
    // add object again
    if (myUpdateGeometry) {
        myNet->addGLObjectIntoGrid(this);
    }
    // Update element
    updateGeometry();
}


void
GNEEdge::addConnection(NBEdge::Connection nbCon, bool selectAfterCreation) {
    // If a new connection was sucesfully created
    if (myNBEdge->setConnection(nbCon.fromLane, nbCon.toEdge, nbCon.toLane, NBEdge::Lane2LaneInfoType::USER, true, nbCon.mayDefinitelyPass,
                                nbCon.keepClear, nbCon.contPos, nbCon.visibility,
                                nbCon.speed, nbCon.customLength, nbCon.customShape, nbCon.uncontrolled)) {
        // Create  or retrieve existent GNEConection
        GNEConnection* con = retrieveGNEConnection(nbCon.fromLane, nbCon.toEdge, nbCon.toLane);
        // add it to GNEConnection container
        myGNEConnections.push_back(con);
        // Add reference
        myGNEConnections.back()->incRef("GNEEdge::addConnection");
        // select GNEConnection if needed
        if (selectAfterCreation) {
            con->selectAttributeCarrier();
        }
        // update geometry
        con->updateGeometry();
    }
    // actually we only do this to force a redraw
    updateGeometry();
}


void
GNEEdge::removeConnection(NBEdge::Connection nbCon) {
    // check if is a explicit turnaround
    if (nbCon.toEdge == myNBEdge->getTurnDestination()) {
        myNet->removeExplicitTurnaround(getID());
    }
    // remove NBEdge::connection from NBEdge
    myNBEdge->removeFromConnections(nbCon);
    // remove their associated GNEConnection
    GNEConnection* con = retrieveGNEConnection(nbCon.fromLane, nbCon.toEdge, nbCon.toLane, false);
    if (con != nullptr) {
        con->decRef("GNEEdge::removeConnection");
        myGNEConnections.erase(std::find(myGNEConnections.begin(), myGNEConnections.end(), con));
        // check if connection is selected
        if (con->isAttributeCarrierSelected()) {
            con->unselectAttributeCarrier();
        }
        if (con->unreferenced()) {
            // show extra information for tests
            WRITE_DEBUG("Deleting unreferenced " + con->getTagStr() + " '" + con->getID() + "' in removeConnection()");
            delete con;
            // actually we only do this to force a redraw
            updateGeometry();
        }
    }
}


GNEConnection*
GNEEdge::retrieveGNEConnection(int fromLane, NBEdge* to, int toLane, bool createIfNoExist) {
    for (auto i : myGNEConnections) {
        if ((i->getFromLaneIndex() == fromLane) && (i->getEdgeTo()->getNBEdge() == to) && (i->getToLaneIndex() == toLane)) {
            return i;
        }
    }
    if (createIfNoExist) {
        // create new connection. Will be added to the rTree on first geometry computation
        GNEConnection* createdConnection = new GNEConnection(myLanes[fromLane], myNet->retrieveEdge(to->getID())->getLanes()[toLane]);
        // show extra information for tests
        WRITE_DEBUG("Created " + createdConnection->getTagStr() + " '" + createdConnection->getID() + "' in retrieveGNEConnection()");
        // add it into network
        myNet->addGLObjectIntoGrid(createdConnection);
        return createdConnection;
    } else {
        return nullptr;
    }
}



void
GNEEdge::setMicrosimID(const std::string& newID) {
    GUIGlObject::setMicrosimID(newID);
    for (auto i : myLanes) {
        i->setMicrosimID(getNBEdge()->getLaneID(i->getIndex()));
    }
}


bool
GNEEdge::hasRestrictedLane(SUMOVehicleClass vclass) const {
    for (auto i : myLanes) {
        if (i->isRestricted(vclass)) {
            return true;
        }
    }
    return false;
}


void
GNEEdge::removeEdgeFromCrossings(GNEJunction* junction, GNEUndoList* undoList) {
    // Remove all crossings that contain this edge in parameter "edges"
    for (GNECrossing* const i : junction->getGNECrossings()) {
        if (i->checkEdgeBelong(this)) {
            myNet->deleteCrossing(i, undoList);
        }
    }
}


void
GNEEdge::straightenElevation(GNEUndoList* undoList) {
    PositionVector modifiedShape = myNBEdge->getGeometry().interpolateZ(
                                       myNBEdge->getFromNode()->getPosition().z(),
                                       myNBEdge->getToNode()->getPosition().z());
    PositionVector innerShape(modifiedShape.begin() + 1, modifiedShape.end() - 1);
    setAttribute(SUMO_ATTR_SHAPE, toString(innerShape), undoList);
}


PositionVector
GNEEdge::smoothShape(const PositionVector& old, bool forElevation) {
    const OptionsCont& oc = OptionsCont::getOptions();
    // distinguish 3 cases:
    // a) if the edge has exactly 3 or 4 points, use these as control points
    // b) if the edge has more than 4 points, use the first 2 and the last 2 as control points
    // c) if the edge is straight and both nodes are geometry-like nodes, use geometry of the continuation edges as control points
    PositionVector init;
#ifdef DEBUG_SMOOTH_GEOM
    if (DEBUGCOND(this)) std::cout << getID()
                                       << " forElevation=" << forElevation
                                       << " fromGeometryLike=" << myNBEdge->getFromNode()->geometryLike()
                                       << " toGeometryLike=" << myNBEdge->getToNode()->geometryLike()
                                       << " smoothShape old=" << old << "\n";
#endif
    if (old.size() == 3 || old.size() == 4) {
        init = old;
    } else if (old.size() > 4 && !forElevation) {
        // for elevation, the initial segments are not useful
        init.push_back(old[0]);
        init.push_back(old[1]);
        init.push_back(old[-2]);
        init.push_back(old[-1]);
    } else if (myNBEdge->getFromNode()->geometryLike() && myNBEdge->getToNode()->geometryLike()) {
        PositionVector begShape;
        PositionVector endShape;
        const EdgeVector& incoming = myNBEdge->getFromNode()->getIncomingEdges();
        const EdgeVector& outgoing = myNBEdge->getToNode()->getOutgoingEdges();
        if (incoming.size() == 1) {
            begShape = incoming[0]->getGeometry();
        } else {
            assert(incoming.size() == 2);
            begShape = myNBEdge->isTurningDirectionAt(incoming[0]) ? incoming[1]->getGeometry() : incoming[0]->getGeometry();
        }
        if (outgoing.size() == 1) {
            endShape = outgoing[0]->getGeometry();
        } else {
            assert(outgoing.size() == 2);
            endShape = myNBEdge->isTurningDirectionAt(outgoing[0]) ? outgoing[1]->getGeometry() : outgoing[0]->getGeometry();
        }
        const double dist = MIN2(old.length2D(), MAX2(old.length2D() / 8, fabs(old[0].z() - old[-1].z()) * OptionsCont::getOptions().getFloat("geometry.max-grade") / 3));
        if (forElevation) {
            // initialize control point elevation for smooth continuation
            init.push_back(old[0]);
            init.push_back(old.positionAtOffset2D(dist));
            init.push_back(old.positionAtOffset2D(old.length2D() - dist));
            init.push_back(old[-1]);
            double begZ = begShape.positionAtOffset2D(MAX2(0.0, begShape.length2D() - dist)).z();
            double endZ = endShape.positionAtOffset2D(MIN2(begShape.length2D(), dist)).z();
            // continue incline
            init[1].setz(2 * init[0].z() - begZ);
            init[2].setz(2 * init[-1].z() - endZ);
        } else {
            bool ok = true;
            const double straightThresh = DEG2RAD(oc.getFloat("opendrive-output.straight-threshold"));
            init = NBNode::bezierControlPoints(begShape, endShape, false, dist, dist, ok, nullptr, straightThresh);
        }
#ifdef DEBUG_SMOOTH_GEOM
        if (DEBUGCOND(this)) {
            std::cout << "   begShape=" << begShape << " endShape=" << endShape << " forElevation=" << forElevation << " dist=" << dist << " ok=" << ok << " init=" << init << "\n";
        }
#endif
    }
    if (init.size() == 0) {
        return PositionVector::EMPTY;
    } else {
        const int numPoints = MAX2(oc.getInt("junctions.internal-link-detail"),
                                   int(old.length2D() / oc.getFloat("opendrive.curve-resolution")));
        return init.bezier(numPoints);
    }
}


void
GNEEdge::smooth(GNEUndoList* undoList) {
    PositionVector modifiedShape = smoothShape(myNBEdge->getGeometry(), false);
    if (modifiedShape.size() < 2) {
        WRITE_WARNING("Could not compute smooth shape for edge '" + getID() + "'");
    } else {
        PositionVector innerShape(modifiedShape.begin() + 1, modifiedShape.end() - 1);
        setAttribute(SUMO_ATTR_SHAPE, toString(innerShape), undoList);
    }
}


void
GNEEdge::smoothElevation(GNEUndoList* undoList) {
    PositionVector elevationBase;
    for (const Position& pos : myNBEdge->getGeometry()) {
        if (elevationBase.size() == 0 || elevationBase[-1].z() != pos.z()) {
            elevationBase.push_back(pos);
        }
    }
    PositionVector elevation = smoothShape(elevationBase, true);
    if (elevation.size() <= 2) {
        WRITE_WARNING("Could not compute smooth elevation for edge '" + getID() + "'");
    } else {
        PositionVector modifiedShape = myNBEdge->getGeometry();
        if (modifiedShape.size() < 5) {
            modifiedShape = modifiedShape.resample(OptionsCont::getOptions().getFloat("opendrive.curve-resolution"), false);
        }
        const double scale = elevation.length2D() / modifiedShape.length2D();
        //std::cout << "   elevation=" << elevation << "\n mod1=" << modifiedShape << " scale=" << scale << "\n";
        double seen = 0;
        for (int i = 1; i < (int)modifiedShape.size(); ++i) {
            seen += modifiedShape[i - 1].distanceTo2D(modifiedShape[i]);
            modifiedShape[i].setz(elevation.positionAtOffset2D(seen * scale).z());
        }
        //std::cout << "   mod2=" << modifiedShape << "\n";
        PositionVector innerShape(modifiedShape.begin() + 1, modifiedShape.end() - 1);
        setAttribute(SUMO_ATTR_SHAPE, toString(innerShape), undoList);
    }
}


void
GNEEdge::setShapeStartPos(const Position& pos) {
    // remove start position and add it the new position
    PositionVector geom = myNBEdge->getGeometry();
    geom.pop_front();
    geom.push_front(pos);
    // restore modified shape
    setGeometry(geom, false);
}


void
GNEEdge::setShapeEndPos(const Position& pos) {
    // remove end position and add it the new position
    PositionVector geom = myNBEdge->getGeometry();
    geom.pop_back();
    geom.push_back(pos);
    // restore modified shape
    setGeometry(geom, false);
}


const std::map<const GNELane*, std::vector<GNEDemandElement*> >
GNEEdge::getVehiclesOverEdgeMap() const {
    // declare vehicles over edge vector
    std::vector<GNEDemandElement*> vehiclesOverEdge;
    // declare solution map
    std::map<const GNELane*, std::vector<GNEDemandElement*> > vehiclesOverEdgeMap;
    // declare a set of vehicles (to avoid duplicates)
    std::set<std::pair<double, GNEDemandElement*> > vehicles;
    // first obtain all vehicles of this edge
    for (const auto& edgeChild : getChildDemandElements()) {
        if (((edgeChild->getTagProperty().getTag() == SUMO_TAG_TRIP) || (edgeChild->getTagProperty().getTag() == SUMO_TAG_FLOW)) &&
                (edgeChild->getParentEdges().front() == this)) {
            vehicles.insert(std::make_pair(edgeChild->getAttributeDouble(SUMO_ATTR_DEPART), edgeChild));
            vehicles.insert(std::make_pair(edgeChild->getAttributeDouble(SUMO_ATTR_DEPART), edgeChild));
        } else if ((edgeChild->getTagProperty().getTag() == SUMO_TAG_ROUTE) && (edgeChild->getParentEdges().front() == this)) {
            for (const auto& routeChild : edgeChild->getChildDemandElements()) {
                if ((routeChild->getTagProperty().getTag() == SUMO_TAG_VEHICLE) || (routeChild->getTagProperty().getTag() == GNE_TAG_FLOW_ROUTE)) {
                    vehicles.insert(std::make_pair(routeChild->getAttributeDouble(SUMO_ATTR_DEPART), routeChild));
                }
            }
        } else if ((edgeChild->getTagProperty().getTag() == GNE_TAG_ROUTE_EMBEDDED) && (edgeChild->getParentEdges().front() == this)) {
            vehicles.insert(std::make_pair(edgeChild->getParentDemandElements().front()->getAttributeDouble(SUMO_ATTR_DEPART), edgeChild->getParentDemandElements().front()));
        }
    }
    // reserve
    vehiclesOverEdge.reserve(vehicles.size());
    // iterate over vehicles
    for (const auto& vehicle : vehicles) {
        // add it over vehiclesOverEdge;
        vehiclesOverEdge.push_back(vehicle.second);
    }
    // now split vehicles by lanes
    for (const auto& vehicle : vehiclesOverEdge) {
        const GNELane* vehicleLane = vehicle->getFirstAllowedVehicleLane();
        if (vehicleLane) {
            vehiclesOverEdgeMap[vehicleLane].push_back(vehicle);
        }
    }
    return vehiclesOverEdgeMap;
}


void
GNEEdge::drawEdgeName(const GUIVisualizationSettings& s) const {
    // check  if we can draw it
    if (!s.drawForPositionSelection && !s.drawForRectangleSelection) {
        // draw the name and/or the street name
        const bool drawStreetName = s.streetName.show && (myNBEdge->getStreetName() != "");
        const bool spreadSuperposed = s.spreadSuperposed && myLanes.back()->drawAsRailway(s) && myNBEdge->isBidiRail();
        // check conditions
        if (s.edgeName.show || drawStreetName || s.edgeValue.show) {
            // get first and last lanes
            const GNELane* firstLane = myLanes[0];
            const GNELane* lastLane = myLanes[myLanes.size() - 1];
            // calculate draw position
            Position drawPosition = firstLane->getLaneShape().positionAtOffset(firstLane->getLaneShape().length() / (double) 2.);
            drawPosition.add(lastLane->getLaneShape().positionAtOffset(lastLane->getLaneShape().length() / (double) 2.));
            drawPosition.mul(.5);
            if (spreadSuperposed) {
                // move name to the right of the edge and towards its beginning
                const double dist = 0.6 * s.edgeName.scaledSize(s.scale);
                const double shiftA = firstLane->getLaneShape().rotationAtOffset(firstLane->getLaneShape().length() / (double) 2.) - DEG2RAD(135);
                const Position shift(dist * cos(shiftA), dist * sin(shiftA));
                drawPosition.add(shift);
            }
            // calculate drawing angle
            double drawAngle = firstLane->getLaneShape().rotationDegreeAtOffset(firstLane->getLaneShape().length() / (double) 2.);
            drawAngle += 90;
            // avoid draw inverted text
            if (drawAngle > 90 && drawAngle < 270) {
                drawAngle -= 180;
            }
            // draw edge name
            if (s.edgeName.show) {
                drawName(drawPosition, s.scale, s.edgeName, drawAngle);
            }
            // draw street name
            if (drawStreetName) {
                GLHelper::drawTextSettings(s.streetName, myNBEdge->getStreetName(), drawPosition, s.scale, drawAngle);
            }
            // draw edge values
            if (s.edgeValue.show) {
                // get current scheme
                const int activeScheme = s.laneColorer.getActive();
                // calculate value depending of active scheme
                std::string value;
                if (activeScheme == 12) {
                    // edge param, could be non-numerical
                    value = getNBEdge()->getParameter(s.edgeParam, "");
                } else if (activeScheme == 13) {
                    // lane param, could be non-numerical
                    value = getNBEdge()->getLaneStruct(lastLane->getIndex()).getParameter(s.laneParam, "");
                } else {
                    // use numerical value value of leftmost lane to hopefully avoid sidewalks, bikelanes etc
                    const double doubleValue = lastLane->getColorValue(s, activeScheme);
                    const RGBColor color = s.laneColorer.getScheme().getColor(doubleValue);
                    value = color.alpha() == 0 ? "" : toString(doubleValue);
                }
                // check if value is empty
                if (value != "") {
                    GLHelper::drawTextSettings(s.edgeValue, value, drawPosition, s.scale, drawAngle);
                }
            }
        }
    }
}


bool
GNEEdge::areStackPositionOverlapped(const GNEEdge::StackPosition& vehicleA, const GNEEdge::StackPosition& vehicleB) const {
    if ((vehicleA.beginPosition() == vehicleB.beginPosition()) && (vehicleA.endPosition() == vehicleB.endPosition())) {
        return true;
    } else if ((vehicleA.beginPosition() < vehicleB.beginPosition()) && (vehicleA.endPosition() > vehicleB.endPosition())) {
        return true;
    } else if ((vehicleA.beginPosition() < vehicleB.beginPosition()) && (vehicleA.endPosition() > vehicleB.beginPosition())) {
        return true;
    } else if ((vehicleA.beginPosition() < vehicleB.endPosition()) && (vehicleA.endPosition() > vehicleB.endPosition())) {
        return true;
    } else {
        return false;
    }
}


/****************************************************************************/
