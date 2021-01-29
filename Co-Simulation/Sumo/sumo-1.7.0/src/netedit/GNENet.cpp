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
/// @file    GNENet.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2011
///
// A visual container for GNE-network-components such as GNEEdge and GNEJunction.
// GNE components wrap netbuild-components and supply visualisation and editing
// capabilities (adapted from GUINet)
//
// WorkrouteFlow (rough draft)
//   use NILoader to fill
//   do netedit stuff
//   call compute to save results
//
/****************************************************************************/
#include <netbuild/NBAlgorithms.h>
#include <netbuild/NBNetBuilder.h>
#include <netedit/changes/GNEChange_Additional.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <netedit/changes/GNEChange_Connection.h>
#include <netedit/changes/GNEChange_Crossing.h>
#include <netedit/changes/GNEChange_DataSet.h>
#include <netedit/changes/GNEChange_DataInterval.h>
#include <netedit/changes/GNEChange_GenericData.h>
#include <netedit/changes/GNEChange_DemandElement.h>
#include <netedit/changes/GNEChange_TAZElement.h>
#include <netedit/changes/GNEChange_Edge.h>
#include <netedit/changes/GNEChange_Junction.h>
#include <netedit/changes/GNEChange_Lane.h>
#include <netedit/changes/GNEChange_Shape.h>
#include <netedit/dialogs/GNEFixAdditionalElements.h>
#include <netedit/dialogs/GNEFixDemandElements.h>
#include <netedit/elements/additional/GNEAdditionalHandler.h>
#include <netedit/elements/data/GNEDataInterval.h>
#include <netedit/elements/demand/GNERouteHandler.h>
#include <netedit/elements/network/GNEConnection.h>
#include <netedit/elements/network/GNECrossing.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/frames/common/GNEInspectorFrame.h>
#include <netwrite/NWFrame.h>
#include <netwrite/NWWriter_SUMO.h>
#include <netwrite/NWWriter_XML.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/globjects/GUIGlObjectStorage.h>
#include <utils/options/OptionsCont.h>
#include <utils/xml/XMLSubSys.h>

#include "GNEApplicationWindow.h"
#include "GNENet.h"
#include "GNEViewNet.h"
#include "GNEUndoList.h"
#include "GNEViewParent.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXIMPLEMENT_ABSTRACT(GNENetHelper::GNEChange_ReplaceEdgeInTLS, GNEChange, nullptr, 0)

// ===========================================================================
// static members
// ===========================================================================

const double GNENet::Z_INITIALIZED = 1;

// ===========================================================================
// member method definitions
// ===========================================================================

GNENet::GNENet(NBNetBuilder* netBuilder) :
    GUIGlObject(GLO_NETWORK, ""),
    myViewNet(nullptr),
    myNetBuilder(netBuilder),
    myAttributeCarriers(new GNENetHelper::AttributeCarriers(this)),
    myPathCalculator(new GNENetHelper::PathCalculator(this)),
    myEdgeIDSupplier("gneE", netBuilder->getEdgeCont().getAllNames()),
    myJunctionIDSupplier("gneJ", netBuilder->getNodeCont().getAllNames()),
    myNeedRecompute(true),
    myNetSaved(true),
    myAdditionalsSaved(true),
    myTLSProgramsSaved(true),
    myDemandElementsSaved(true),
    myDataElementsSaved(true),
    myUpdateGeometryEnabled(true) {
    // set net in gIDStorage
    GUIGlObjectStorage::gIDStorage.setNetObject(this);
    // Write GL debug information
    WRITE_GLDEBUG("initJunctionsAndEdges function called in GNENet constructor");
    // init junction and edges
    initJunctionsAndEdges();
    // check Z boundary
    if (myZBoundary.ymin() != Z_INITIALIZED) {
        myZBoundary.add(0, 0);
    }

}


GNENet::~GNENet() {
    // delete route calculator Instance
    delete myPathCalculator;
    // delete AttributeCarriers
    delete myAttributeCarriers;
    // show extra information for tests
    WRITE_DEBUG("Deleting net builder in GNENet destructor");
    delete myNetBuilder;
}


GNENetHelper::AttributeCarriers*
GNENet::getAttributeCarriers() const {
    return myAttributeCarriers;
}


GNENetHelper::PathCalculator*
GNENet::getPathCalculator() {
    return myPathCalculator;
}


const Boundary&
GNENet::getBoundary() const {
    // SUMORTree is also a Boundary
    return myGrid;
}


SUMORTree&
GNENet::getGrid() {
    return myGrid;
}


GUIGLObjectPopupMenu*
GNENet::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    buildPopupHeader(ret, app);
    buildCenterPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    return ret;
}


GUIParameterTableWindow*
GNENet::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    // Nets lanes don't have attributes
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // close building
    ret->closeBuilding();
    return ret;
}


void
GNENet::drawGL(const GUIVisualizationSettings& /*s*/) const {
    // nothing to drawn
}


Boundary
GNENet::getCenteringBoundary() const {
    return getBoundary();
}


void
GNENet::expandBoundary(const Boundary& newBoundary) {
    myGrid.add(newBoundary);
}


const Boundary&
GNENet::getZBoundary() const {
    return myZBoundary;
}


void
GNENet::addZValueInBoundary(const double z) {
    // @todo let Boundary class track z-coordinate natively
    if (z != 0) {
        myZBoundary.add(z, Z_INITIALIZED);
    }
}


GNEJunction*
GNENet::createJunction(const Position& pos, GNEUndoList* undoList) {
    std::string id = myJunctionIDSupplier.getNext();
    // create new NBNode
    NBNode* nbn = new NBNode(id, pos);
    // create GNEJunciton
    GNEJunction* junction = new GNEJunction(this, nbn);
    undoList->add(new GNEChange_Junction(junction, true), true);
    return junction;
}


GNEEdge*
GNENet::createEdge(
    GNEJunction* src, GNEJunction* dest, GNEEdge* tpl, GNEUndoList* undoList,
    const std::string& suggestedName,
    bool wasSplit,
    bool allowDuplicateGeom,
    bool recomputeConnections) {
    // prevent duplicate edge (same geometry)
    const EdgeVector& outgoing = src->getNBNode()->getOutgoingEdges();
    for (EdgeVector::const_iterator it = outgoing.begin(); it != outgoing.end(); it++) {
        if ((*it)->getToNode() == dest->getNBNode() && (*it)->getGeometry().size() == 2) {
            if (!allowDuplicateGeom) {
                return nullptr;
            }
        }
    }

    std::string id;
    if (suggestedName != "" && !retrieveEdge(suggestedName, false)) {
        id = suggestedName;
        reserveEdgeID(id);
    } else {
        id = myEdgeIDSupplier.getNext();
    }

    GNEEdge* edge;
    if (tpl) {
        NBEdge* nbeTpl = tpl->getNBEdge();
        NBEdge* nbe = new NBEdge(id, src->getNBNode(), dest->getNBNode(), nbeTpl);
        edge = new GNEEdge(this, nbe, wasSplit);
    } else {
        // default if no template is given
        const OptionsCont& oc = OptionsCont::getOptions();
        double defaultSpeed = oc.getFloat("default.speed");
        std::string defaultType = oc.getString("default.type");
        int defaultNrLanes = oc.getInt("default.lanenumber");
        int defaultPriority = oc.getInt("default.priority");
        double defaultWidth = NBEdge::UNSPECIFIED_WIDTH;
        double defaultOffset = NBEdge::UNSPECIFIED_OFFSET;
        NBEdge* nbe = new NBEdge(id, src->getNBNode(), dest->getNBNode(),
                                 defaultType, defaultSpeed,
                                 defaultNrLanes, defaultPriority,
                                 defaultWidth,
                                 defaultOffset);
        edge = new GNEEdge(this, nbe, wasSplit);
    }
    undoList->p_begin("create " + toString(SUMO_TAG_EDGE));
    undoList->add(new GNEChange_Edge(edge, true), true);
    if (recomputeConnections) {
        src->setLogicValid(false, undoList);
        dest->setLogicValid(false, undoList);
    }
    requireRecompute();
    undoList->p_end();
    return edge;
}


void
GNENet::deleteJunction(GNEJunction* junction, GNEUndoList* undoList) {
    // we have to delete all incident edges because they cannot exist without that junction
    // all deletions must be undone/redone together so we start a new command group
    // @todo if any of those edges are dead-ends should we remove their orphan junctions as well?
    undoList->p_begin("delete " + toString(SUMO_TAG_JUNCTION));
    // invalidate path elements
    junction->invalidatePathElements();
    // delete all crossings vinculated with junction
    while (junction->getGNECrossings().size() > 0) {
        deleteCrossing(junction->getGNECrossings().front(), undoList);
    }
    // find all crossings of neightbour junctions that shares an edge of this junction
    std::vector<GNECrossing*> crossingsToRemove;
    std::vector<GNEJunction*> junctionNeighbours = junction->getJunctionNeighbours();
    for (const auto& junction : junctionNeighbours) {
        // iterate over crossing of neighbour juntion
        for (const auto& crossing : junction->getGNECrossings()) {
            // if at least one of the edges of junction to remove belongs to a crossing of the neighbour junction, delete it
            if (crossing->checkEdgeBelong(junction->getChildEdges())) {
                crossingsToRemove.push_back(crossing);
            }
        }
    }
    // delete crossings top remove
    for (const auto& crossing : crossingsToRemove) {
        deleteCrossing(crossing, undoList);
    }
    // deleting edges changes in the underlying EdgeVector so we have to make a copy
    const EdgeVector incidentEdges = junction->getNBNode()->getEdges();
    for (const auto& edge : incidentEdges) {
        deleteEdge(myAttributeCarriers->getEdges().at(edge->getID()), undoList, true);
    }
    // remove any traffic lights from the traffic light container (avoids lots of warnings)
    junction->setAttribute(SUMO_ATTR_TYPE, toString(SumoXMLNodeType::PRIORITY), undoList);
    // delete edge
    undoList->add(new GNEChange_Junction(junction, false), true);
    undoList->p_end();
}


void
GNENet::deleteEdge(GNEEdge* edge, GNEUndoList* undoList, bool recomputeConnections) {
    undoList->p_begin("delete " + toString(SUMO_TAG_EDGE));
    // iterate over lanes
    for (const auto& lane : edge->getLanes()) {
        // invalidate path elements
        lane->invalidatePathElements();
        // delete lane additionals
        while (lane->getChildAdditionals().size() > 0) {
            deleteAdditional(lane->getChildAdditionals().front(), undoList);
        }
        // delete lane shapes
        while (lane->getChildShapes().size() > 0) {
            deleteShape(lane->getChildShapes().front(), undoList);
        }
        // delete lane demand elements
        while (lane->getChildDemandElements().size() > 0) {
            deleteDemandElement(lane->getChildDemandElements().front(), undoList);
        }
        // delete lane generic data elements
        while (lane->getChildGenericDatas().size() > 0) {
            deleteGenericData(lane->getChildGenericDatas().front(), undoList);
        }
    }
    // delete edge child additionals
    while (edge->getChildAdditionals().size() > 0) {
        deleteAdditional(edge->getChildAdditionals().front(), undoList);
    }
    // delete edge child shapes
    while (edge->getChildShapes().size() > 0) {
        deleteShape(edge->getChildShapes().front(), undoList);
    }
    // delete edge child demand elements
    while (edge->getChildDemandElements().size() > 0) {
        deleteDemandElement(edge->getChildDemandElements().front(), undoList);
    }
    // delete edge child generic datas
    while (edge->getChildGenericDatas().size() > 0) {
        deleteGenericData(edge->getChildGenericDatas().front(), undoList);
    }
    // remove edge from crossings related with this edge
    edge->getParentJunctions().front()->removeEdgeFromCrossings(edge, undoList);
    edge->getParentJunctions().back()->removeEdgeFromCrossings(edge, undoList);
    // update affected connections
    if (recomputeConnections) {
        edge->getParentJunctions().front()->setLogicValid(false, undoList);
        edge->getParentJunctions().back()->setLogicValid(false, undoList);
    } else {
        edge->getParentJunctions().front()->removeConnectionsTo(edge, undoList, true);
        edge->getParentJunctions().front()->removeConnectionsFrom(edge, undoList, true);
    }
    // if junction source is a TLS and after deletion will have only an edge, remove TLS
    if (edge->getParentJunctions().front()->getNBNode()->isTLControlled() && (edge->getParentJunctions().front()->getGNEOutgoingEdges().size() <= 1)) {
        edge->getParentJunctions().front()->setAttribute(SUMO_ATTR_TYPE, toString(SumoXMLNodeType::PRIORITY), undoList);
    }
    // if junction destiny is a TLS and after deletion will have only an edge, remove TLS
    if (edge->getParentJunctions().back()->getNBNode()->isTLControlled() && (edge->getParentJunctions().back()->getGNEIncomingEdges().size() <= 1)) {
        edge->getParentJunctions().back()->setAttribute(SUMO_ATTR_TYPE, toString(SumoXMLNodeType::PRIORITY), undoList);
    }
    // Delete edge
    undoList->add(new GNEChange_Edge(edge, false), true);
    // remove edge requires always a recompute (due geometry and connections)
    requireRecompute();
    // finish delete edge
    undoList->p_end();
}


void
GNENet::replaceIncomingEdge(GNEEdge* which, GNEEdge* by, GNEUndoList* undoList) {
    undoList->p_begin("replace " + toString(SUMO_TAG_EDGE));
    undoList->p_add(new GNEChange_Attribute(by, SUMO_ATTR_TO, which->getAttribute(SUMO_ATTR_TO)));
    // iterate over lane
    for (const auto& lane : which->getLanes()) {
        // replace in additionals
        std::vector<GNEAdditional*> copyOfLaneAdditionals = lane->getChildAdditionals();
        for (const auto& additional : copyOfLaneAdditionals) {
            undoList->p_add(new GNEChange_Attribute(additional, SUMO_ATTR_LANE, by->getNBEdge()->getLaneID(lane->getIndex())));
        }
        // replace in shapes
        std::vector<GNEShape*> copyOfLaneShapes = lane->getChildShapes();
        for (const auto& shape : copyOfLaneShapes) {
            undoList->p_add(new GNEChange_Attribute(shape, SUMO_ATTR_LANE, by->getNBEdge()->getLaneID(lane->getIndex())));
        }
        // replace in demand elements
        std::vector<GNEDemandElement*> copyOfLaneDemandElements = lane->getChildDemandElements();
        for (const auto& demandElement : copyOfLaneDemandElements) {
            undoList->p_add(new GNEChange_Attribute(demandElement, SUMO_ATTR_LANE, by->getNBEdge()->getLaneID(lane->getIndex())));
        }
        // replace in generic datas
        std::vector<GNEGenericData*> copyOfLaneGenericDatas = lane->getChildGenericDatas();
        for (const auto& demandElement : copyOfLaneGenericDatas) {
            undoList->p_add(new GNEChange_Attribute(demandElement, SUMO_ATTR_LANE, by->getNBEdge()->getLaneID(lane->getIndex())));
        }
    }
    // replace in edge additionals children
    while (which->getChildAdditionals().size() > 0) {
        undoList->p_add(new GNEChange_Attribute(which->getChildAdditionals().front(), SUMO_ATTR_EDGE, by->getID()));
    }
    // replace in edge shapes children
    while (which->getChildShapes().size() > 0) {
        undoList->p_add(new GNEChange_Attribute(which->getChildShapes().front(), SUMO_ATTR_EDGE, by->getID()));
    }
    // replace in edge demand elements children
    while (which->getChildDemandElements().size() > 0) {
        undoList->p_add(new GNEChange_Attribute(which->getChildDemandElements().front(), SUMO_ATTR_EDGE, by->getID()));
    }
    // replace in edge demand elements children
    while (which->getChildGenericDatas().size() > 0) {
        undoList->p_add(new GNEChange_Attribute(which->getChildGenericDatas().front(), SUMO_ATTR_EDGE, by->getID()));
    }
    // replace in rerouters
    for (const auto& rerouter : which->getParentAdditionals()) {
        replaceInListAttribute(rerouter, SUMO_ATTR_EDGES, which->getID(), by->getID(), undoList);
    }
    // replace in crossings
    for (const auto& crossing : which->getParentJunctions().back()->getGNECrossings()) {
        // if at least one of the edges of junction to remove belongs to a crossing of the source junction, delete it
        replaceInListAttribute(crossing, SUMO_ATTR_EDGES, which->getID(), by->getID(), undoList);
    }
    // fix connections (make a copy because they will be modified
    std::vector<NBEdge::Connection> NBConnections = which->getNBEdge()->getConnections();
    for (const auto& NBConnection : NBConnections) {
        undoList->add(new GNEChange_Connection(which, NBConnection, false, false), true);
        undoList->add(new GNEChange_Connection(by, NBConnection, false, true), true);
    }
    undoList->add(new GNENetHelper::GNEChange_ReplaceEdgeInTLS(getTLLogicCont(), which->getNBEdge(), by->getNBEdge()), true);
    // Delete edge
    undoList->add(new GNEChange_Edge(which, false), true);
    // finish replace edge
    undoList->p_end();
}


void
GNENet::deleteLane(GNELane* lane, GNEUndoList* undoList, bool recomputeConnections) {
    GNEEdge* edge = lane->getParentEdge();
    if (edge->getNBEdge()->getNumLanes() == 1) {
        // remove the whole edge instead
        deleteEdge(edge, undoList, recomputeConnections);
    } else {
        undoList->p_begin("delete " + toString(SUMO_TAG_LANE));
        // invalidate path elements
        lane->invalidatePathElements();
        // delete lane additional children
        while (lane->getChildAdditionals().size() > 0) {
            deleteAdditional(lane->getChildAdditionals().front(), undoList);
        }
        // delete lane shape children
        while (lane->getChildShapes().size() > 0) {
            deleteShape(lane->getChildShapes().front(), undoList);
        }
        // delete lane demand element children
        while (lane->getChildDemandElements().size() > 0) {
            deleteDemandElement(lane->getChildDemandElements().front(), undoList);
        }
        // delete lane generic data children
        while (lane->getChildGenericDatas().size() > 0) {
            deleteGenericData(lane->getChildGenericDatas().front(), undoList);
        }
        // update affected connections
        if (recomputeConnections) {
            edge->getParentJunctions().front()->setLogicValid(false, undoList);
            edge->getParentJunctions().back()->setLogicValid(false, undoList);
        } else {
            edge->getParentJunctions().front()->removeConnectionsTo(edge, undoList, true, lane->getIndex());
            edge->getParentJunctions().front()->removeConnectionsFrom(edge, undoList, true, lane->getIndex());
        }
        // delete lane
        const NBEdge::Lane& laneAttrs = edge->getNBEdge()->getLaneStruct(lane->getIndex());
        undoList->add(new GNEChange_Lane(edge, lane, laneAttrs, false, recomputeConnections), true);
        // remove lane requires always a recompute (due geometry and connections)
        requireRecompute();
        undoList->p_end();
    }
}


void
GNENet::deleteConnection(GNEConnection* connection, GNEUndoList* undoList) {
    undoList->p_begin("delete " + toString(SUMO_TAG_CONNECTION));
    // obtain NBConnection to remove
    NBConnection deleted = connection->getNBConnection();
    GNEJunction* junctionDestiny = connection->getEdgeFrom()->getParentJunctions().back();
    junctionDestiny->markAsModified(undoList);
    undoList->add(new GNEChange_Connection(connection->getEdgeFrom(), connection->getNBEdgeConnection(), connection->isAttributeCarrierSelected(), false), true);
    junctionDestiny->invalidateTLS(undoList, deleted);
    // remove connection requires always a recompute (due geometry and connections)
    requireRecompute();
    undoList->p_end();
}


void
GNENet::deleteCrossing(GNECrossing* crossing, GNEUndoList* undoList) {
    undoList->p_begin("delete crossing");
    // remove it using GNEChange_Crossing
    undoList->add(new GNEChange_Crossing(
                      crossing->getParentJunction(), crossing->getNBCrossing()->edges,
                      crossing->getNBCrossing()->width, crossing->getNBCrossing()->priority,
                      crossing->getNBCrossing()->customTLIndex,
                      crossing->getNBCrossing()->customTLIndex2,
                      crossing->getNBCrossing()->customShape,
                      crossing->isAttributeCarrierSelected(),
                      false), true);
    // remove crossing requires always a recompute (due geometry and connections)
    requireRecompute();
    undoList->p_end();
}


void
GNENet::deleteAdditional(GNEAdditional* additional, GNEUndoList* undoList) {
    undoList->p_begin("delete " + additional->getTagStr());
    // remove all demand element children of this additional deleteDemandElement this function recursively
    while (additional->getChildDemandElements().size() > 0) {
        deleteDemandElement(additional->getChildDemandElements().front(), undoList);
    }
    // remove all generic data children of this additional deleteGenericData this function recursively
    while (additional->getChildGenericDatas().size() > 0) {
        deleteGenericData(additional->getChildGenericDatas().front(), undoList);
    }
    // remove all additional children of this additional calling this function recursively
    while (additional->getChildAdditionals().size() > 0) {
        deleteAdditional(additional->getChildAdditionals().front(), undoList);
    }
    // remove additional
    undoList->add(new GNEChange_Additional(additional, false), true);
    undoList->p_end();
}


void
GNENet::deleteShape(GNEShape* shape, GNEUndoList* undoList) {
    undoList->p_begin("delete " + shape->getTagStr());
    // delete shape
    undoList->add(new GNEChange_Shape(shape, false), true);
    undoList->p_end();
}


void
GNENet::deleteTAZElement(GNETAZElement* TAZElement, GNEUndoList* undoList) {
    undoList->p_begin("delete " + TAZElement->getTagStr());
    // remove all demand element children of this TAZElement deleteDemandElement this function recursively
    while (TAZElement->getChildDemandElements().size() > 0) {
        deleteDemandElement(TAZElement->getChildDemandElements().front(), undoList);
    }
    // remove all generic data children of this TAZElement deleteGenericData this function recursively
    while (TAZElement->getChildGenericDatas().size() > 0) {
        deleteGenericData(TAZElement->getChildGenericDatas().front(), undoList);
    }
    // remove all TAZElement children of this TAZElement calling this function recursively
    while (TAZElement->getChildTAZElements().size() > 0) {
        deleteTAZElement(TAZElement->getChildTAZElements().front(), undoList);
    }
    // remove TAZElement
    undoList->add(new GNEChange_TAZElement(TAZElement, false), true);
    undoList->p_end();
}


void
GNENet::deleteDemandElement(GNEDemandElement* demandElement, GNEUndoList* undoList) {
    // check that default VTypes aren't removed
    if ((demandElement->getTagProperty().getTag() == SUMO_TAG_VTYPE) && (GNEAttributeCarrier::parse<bool>(demandElement->getAttribute(GNE_ATTR_DEFAULT_VTYPE)))) {
        throw ProcessError("Trying to delete a default Vehicle Type");
    } else {
        // check if currently is being inspected
        if (myViewNet->getInspectedAttributeCarrier() == demandElement) {
            myViewNet->getViewParent()->getInspectorFrame()->clearInspectedAC();
        }
        undoList->p_begin("delete " + demandElement->getTagStr());
        // remove all child demand elements of this demandElement calling this function recursively
        while (demandElement->getChildDemandElements().size() > 0) {
            deleteDemandElement(demandElement->getChildDemandElements().front(), undoList);
        }
        // remove all generic data children of this additional deleteGenericData this function recursively
        while (demandElement->getChildGenericDatas().size() > 0) {
            deleteGenericData(demandElement->getChildGenericDatas().front(), undoList);
        }
        // remove demandElement
        undoList->add(new GNEChange_DemandElement(demandElement, false), true);
        undoList->p_end();
    }
}


void
GNENet::deleteDataSet(GNEDataSet* dataSet, GNEUndoList* undoList) {
    undoList->p_begin("delete " + dataSet->getTagStr());
    // make a copy of all generic data children
    auto copyOfDataIntervalChildren = dataSet->getDataIntervalChildren();
    // clear all data intervals (this will be delete also the dataSet)
    for (const auto& dataInterval : copyOfDataIntervalChildren) {
        deleteDataInterval(dataInterval.second, undoList);
    }
    undoList->p_end();
}


void
GNENet::deleteDataInterval(GNEDataInterval* dataInterval, GNEUndoList* undoList) {
    undoList->p_begin("delete " + dataInterval->getTagStr());
    // make a copy of all generic data children
    auto copyOfGenericDataChildren = dataInterval->getGenericDataChildren();
    // clear all generic datas (this will be delete also the data intervals)
    for (const auto& genericData : copyOfGenericDataChildren) {
        deleteGenericData(genericData, undoList);
    }
    undoList->p_end();
}


void
GNENet::deleteGenericData(GNEGenericData* genericData, GNEUndoList* undoList) {
    undoList->p_begin("delete " + genericData->getTagStr());
    // remove all child demand elements of this demandElement calling this function recursively
    while (genericData->getChildDemandElements().size() > 0) {
        deleteDemandElement(genericData->getChildDemandElements().front(), undoList);
    }
    // remove all generic data children of this additional deleteGenericData this function recursively
    while (genericData->getChildGenericDatas().size() > 0) {
        deleteGenericData(genericData->getChildGenericDatas().front(), undoList);
    }
    // get pointer to dataInterval and dataSet
    GNEDataInterval* dataInterval = genericData->getDataIntervalParent();
    GNEDataSet* dataSet = dataInterval->getDataSetParent();
    // remove generic data
    undoList->add(new GNEChange_GenericData(genericData, false), true);
    // check if data interval is empty
    if (dataInterval->getGenericDataChildren().empty()) {
        // remove data interval
        undoList->add(new GNEChange_DataInterval(genericData->getDataIntervalParent(), false), true);
        // now check if data set is empty
        if (dataSet->getDataIntervalChildren().empty()) {
            // remove data set
            undoList->add(new GNEChange_DataSet(genericData->getDataIntervalParent()->getDataSetParent(), false), true);
        }
    }
    undoList->p_end();
}


void
GNENet::duplicateLane(GNELane* lane, GNEUndoList* undoList, bool recomputeConnections) {
    undoList->p_begin("duplicate " + toString(SUMO_TAG_LANE));
    GNEEdge* edge = lane->getParentEdge();
    const NBEdge::Lane& laneAttrs = edge->getNBEdge()->getLaneStruct(lane->getIndex());
    if (recomputeConnections) {
        edge->getParentJunctions().front()->setLogicValid(false, undoList);
        edge->getParentJunctions().front()->setLogicValid(false, undoList);
    }
    GNELane* newLane = new GNELane(edge, lane->getIndex());
    undoList->add(new GNEChange_Lane(edge, newLane, laneAttrs, true, recomputeConnections), true);
    requireRecompute();
    undoList->p_end();
}


bool
GNENet::restrictLane(SUMOVehicleClass vclass, GNELane* lane, GNEUndoList* undoList) {
    bool addRestriction = true;
    if (vclass == SVC_PEDESTRIAN) {
        GNEEdge* edge = lane->getParentEdge();
        for (const auto& edgeLane : edge->getLanes()) {
            if (edgeLane->isRestricted(SVC_PEDESTRIAN)) {
                // prevent adding a 2nd sidewalk
                addRestriction = false;
            } else {
                // ensure that the sidewalk is used exclusively
                const SVCPermissions allOldWithoutPeds = edge->getNBEdge()->getPermissions(edgeLane->getIndex()) & ~SVC_PEDESTRIAN;
                edgeLane->setAttribute(SUMO_ATTR_ALLOW, getVehicleClassNames(allOldWithoutPeds), undoList);
            }
        }
    }
    // restrict the lane
    if (addRestriction) {
        const double width = (vclass == SVC_PEDESTRIAN || vclass == SVC_BICYCLE
                              ? OptionsCont::getOptions().getFloat("default.sidewalk-width")
                              : OptionsCont::getOptions().getFloat("default.lanewidth"));
        lane->setAttribute(SUMO_ATTR_ALLOW, toString(vclass), undoList);
        lane->setAttribute(SUMO_ATTR_WIDTH, toString(width), undoList);
        return true;
    } else {
        return false;
    }
}


bool
GNENet::addRestrictedLane(SUMOVehicleClass vclass, GNEEdge* edge, int index, GNEUndoList* undoList) {
    // First check that edge don't have a restricted lane of the given vclass
    for (const auto& lane : edge->getLanes()) {
        if (lane->isRestricted(vclass)) {
            return false;
        }
    }
    // check that index is correct (index == size adds to the left of the leftmost lane)
    const int numLanes = (int)edge->getLanes().size();
    if (index > numLanes) {
        return false;
    }
    if (index < 0) {
        // guess index from vclass
        if (vclass == SVC_PEDESTRIAN) {
            index = 0;
        } else if (vclass == SVC_BICYCLE) {
            // add bikelanes to the left of an existing sidewalk
            index = edge->getLanes()[0]->isRestricted(SVC_PEDESTRIAN) ? 1 : 0;
        } else if (vclass == SVC_IGNORING || vclass == SVC_BUS) {
            // add greenVerge to the left of an existing sidewalk or bikeLane
            // add busLane to the left of an existing sidewalk, bikeLane or greenVerge
            index = 0;
            while (index < numLanes && (edge->getNBEdge()->getPermissions(index) & ~(SVC_PEDESTRIAN | SVC_BICYCLE)) == 0) {
                index++;
            }
        }
    }
    // duplicate selected lane
    duplicateLane(edge->getLanes().at(MIN2(index, numLanes - 1)), undoList, true);
    // transform the created lane
    return restrictLane(vclass, edge->getLanes().at(index), undoList);
}


bool
GNENet::removeRestrictedLane(SUMOVehicleClass vclass, GNEEdge* edge, GNEUndoList* undoList) {
    // iterate over lanes of edge
    for (const auto& lane : edge->getLanes()) {
        if (lane->isRestricted(vclass)) {
            // Delete lane
            deleteLane(lane, undoList, true);
            return true;
        }
    }
    return false;
}


GNEJunction*
GNENet::splitEdge(GNEEdge* edge, const Position& pos, GNEUndoList* undoList, GNEJunction* newJunction) {
    // begin undo list
    undoList->p_begin("split " + toString(SUMO_TAG_EDGE));
    // check if we have to create a new edge
    if (newJunction == nullptr) {
        newJunction = createJunction(pos, undoList);
    }
    // obtain edge geometry and split position
    const PositionVector& oldEdgeGeometry = edge->getNBEdge()->getGeometry();
    const double edgeSplitPosition = oldEdgeGeometry.nearest_offset_to_point2D(pos, false);
    // obtain lane geometry and split position (needed for adjust additional and demand childs)
    const PositionVector& oldLaneGeometry = edge->getLanes().front()->getLaneShape();
    const double laneSplitPosition = oldLaneGeometry.nearest_offset_to_point2D(pos, false);
    // split edge geometry in two new geometries using edgeSplitPosition
    std::pair<PositionVector, PositionVector> newGeoms = oldEdgeGeometry.splitAt(edgeSplitPosition);
    // get shape end
    const std::string shapeEnd = edge->getAttribute(GNE_ATTR_SHAPE_END);
    // figure out the new name
    int posBase = 0;
    // set baseName
    std::string baseName = edge->getMicrosimID();
    if (edge->wasSplit()) {
        const std::string::size_type sep_index = baseName.rfind('.');
        // edge may have been renamed in between
        if (sep_index != std::string::npos) {
            std::string posString = baseName.substr(sep_index + 1);
            if (GNEAttributeCarrier::canParse<int>(posString.c_str())) {
                ;
                posBase = GNEAttributeCarrier::parse<int>(posString.c_str());
                baseName = baseName.substr(0, sep_index); // includes the .
            }
        }
    }
    baseName += '.';
    // create a new edge from the new junction to the previous destination
    GNEEdge* secondPart = createEdge(newJunction, edge->getParentJunctions().back(), edge,
                                     undoList, baseName + toString(posBase + (int)edgeSplitPosition), true, false, false);
    // fix connections from the split edge (must happen before changing SUMO_ATTR_TO)
    edge->getParentJunctions().back()->replaceIncomingConnections(edge, secondPart, undoList);
    // remove affected crossings from junction (must happen before changing SUMO_ATTR_TO)
    std::vector<NBNode::Crossing> affectedCrossings;
    for (GNECrossing* crossing : edge->getParentJunctions().back()->getGNECrossings()) {
        if (crossing->checkEdgeBelong(edge)) {
            NBNode::Crossing nbC = *crossing->getNBCrossing();
            undoList->add(new GNEChange_Crossing(edge->getParentJunctions().back(), nbC, false), true);
            EdgeVector newEdges;
            for (NBEdge* nbEdge : nbC.edges) {
                if (nbEdge == edge->getNBEdge()) {
                    newEdges.push_back(secondPart->getNBEdge());
                } else {
                    newEdges.push_back(nbEdge);
                }
            }
            nbC.edges = newEdges;
            affectedCrossings.push_back(nbC);
        }
    }
    // modify the edge so that it ends at the new junction (and all incoming connections are preserved
    undoList->p_add(new GNEChange_Attribute(edge, SUMO_ATTR_TO, newJunction->getID()));
    // set first part of geometry
    newGeoms.first.pop_back();
    newGeoms.first.erase(newGeoms.first.begin());
    edge->setAttribute(GNE_ATTR_SHAPE_END, "", undoList);
    edge->setAttribute(SUMO_ATTR_SHAPE, toString(newGeoms.first), undoList);
    // set second part of geometry
    secondPart->setAttribute(GNE_ATTR_SHAPE_END, shapeEnd, undoList);
    newGeoms.second.pop_back();
    newGeoms.second.erase(newGeoms.second.begin());
    secondPart->setAttribute(SUMO_ATTR_SHAPE, toString(newGeoms.second), undoList);
    // reconnect across the split
    for (int i = 0; i < (int)edge->getLanes().size(); ++i) {
        undoList->add(new GNEChange_Connection(edge, NBEdge::Connection(i, secondPart->getNBEdge(), i), false, true), true);
    }
    // re-add modified crossings
    for (const auto& nbC : affectedCrossings) {
        undoList->add(new GNEChange_Crossing(secondPart->getParentJunctions().back(), nbC, true), true);
    }
    // Split geometry of all child additional
    for (const auto& additional : edge->getChildAdditionals()) {
        additional->splitEdgeGeometry(edgeSplitPosition, edge, secondPart, undoList);
    }
    // Split geometry of all child lane additional
    for (int i = 0; i < (int)edge->getLanes().size(); i++) {
        for (const auto& additional : edge->getLanes().at(i)->getChildAdditionals()) {
            additional->splitEdgeGeometry(laneSplitPosition, edge->getLanes().at(i), secondPart->getLanes().at(i), undoList);
        }
    }
    // Split geometry of all child demand elements
    for (const auto& demandElement : edge->getChildDemandElements()) {
        demandElement->splitEdgeGeometry(edgeSplitPosition, edge, secondPart, undoList);
    }
    // Split geometry of all child lane demand elements
    for (int i = 0; i < (int)edge->getLanes().size(); i++) {
        for (const auto& demandElement : edge->getLanes().at(i)->getChildDemandElements()) {
            demandElement->splitEdgeGeometry(laneSplitPosition, edge->getLanes().at(i), secondPart->getLanes().at(i), undoList);
        }
    }
    // finish undo list
    undoList->p_end();
    // return new junction
    return newJunction;
}


void
GNENet::splitEdgesBidi(GNEEdge* edge, GNEEdge* oppositeEdge, const Position& pos, GNEUndoList* undoList) {
    GNEJunction* newJunction = nullptr;
    undoList->p_begin("split " + toString(SUMO_TAG_EDGE) + "s");
    // split edge and save created junction
    newJunction = splitEdge(edge, pos, undoList, newJunction);
    // split second edge
    splitEdge(oppositeEdge, pos, undoList, newJunction);
    undoList->p_end();
}


void
GNENet::reverseEdge(GNEEdge* edge, GNEUndoList* undoList) {
    undoList->p_begin("reverse " + toString(SUMO_TAG_EDGE));
    deleteEdge(edge, undoList, false); // still exists. we delete it so we can reuse the name in case of resplit
    GNEEdge* reversed = createEdge(edge->getParentJunctions().back(), edge->getParentJunctions().front(), edge, undoList, edge->getID(), false, true);
    assert(reversed != 0);
    reversed->setAttribute(SUMO_ATTR_SHAPE, toString(edge->getNBEdge()->getInnerGeometry().reverse()), undoList);
    reversed->setAttribute(GNE_ATTR_SHAPE_START, edge->getAttribute(GNE_ATTR_SHAPE_END), undoList);
    reversed->setAttribute(GNE_ATTR_SHAPE_END, edge->getAttribute(GNE_ATTR_SHAPE_START), undoList);
    undoList->p_end();
}


GNEEdge*
GNENet::addReversedEdge(GNEEdge* edge, GNEUndoList* undoList) {
    undoList->p_begin("add reversed " + toString(SUMO_TAG_EDGE));
    GNEEdge* reversed = nullptr;
    if (edge->getNBEdge()->getLaneSpreadFunction() == LaneSpreadFunction::RIGHT || isRailway(edge->getNBEdge()->getPermissions())) {
        // for rail edges, we assume bi-directional tracks are wanted
        reversed = createEdge(edge->getParentJunctions().back(), edge->getParentJunctions().front(), edge, undoList, "-" + edge->getID(), false, true);
        assert(reversed != 0);
        reversed->setAttribute(SUMO_ATTR_SHAPE, toString(edge->getNBEdge()->getInnerGeometry().reverse()), undoList);
        reversed->setAttribute(GNE_ATTR_SHAPE_START, edge->getAttribute(GNE_ATTR_SHAPE_END), undoList);
        reversed->setAttribute(GNE_ATTR_SHAPE_END, edge->getAttribute(GNE_ATTR_SHAPE_START), undoList);
    } else {
        // if the edge is centered it should probably connect somewhere else
        // make it easy to move and reconnect it
        PositionVector orig = edge->getNBEdge()->getGeometry();
        PositionVector origInner = edge->getNBEdge()->getInnerGeometry();
        const double tentativeShift = edge->getNBEdge()->getTotalWidth() + 2;
        orig.move2side(-tentativeShift);
        origInner.move2side(-tentativeShift);
        GNEJunction* src = createJunction(orig.back(), undoList);
        GNEJunction* dest = createJunction(orig.front(), undoList);
        reversed = createEdge(src, dest, edge, undoList, "-" + edge->getID(), false, true);
        assert(reversed != 0);
        reversed->setAttribute(SUMO_ATTR_SHAPE, toString(origInner.reverse()), undoList);
        reversed->setAttribute(SUMO_ATTR_SHAPE, toString(origInner.reverse()), undoList);
        // select the new edge and its nodes
        reversed->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
        src->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
        dest->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
    }
    undoList->p_end();
    return reversed;
}


void
GNENet::mergeJunctions(GNEJunction* moved, GNEJunction* target, GNEUndoList* undoList) {
    undoList->p_begin("merge " + toString(SUMO_TAG_JUNCTION) + "s");
    // place moved junction in the same position of target junction
    moved->setAttribute(SUMO_ATTR_POSITION, target->getAttribute(SUMO_ATTR_POSITION), undoList);
    // deleting edges changes in the underlying EdgeVector so we have to make a copy
    const EdgeVector incomingNBEdges = moved->getNBNode()->getIncomingEdges();
    for (const auto& incomingNBEdge : incomingNBEdges) {
        // delete edges between the merged junctions
        GNEEdge* edge = myAttributeCarriers->getEdges().at(incomingNBEdge->getID());
        if (edge->getParentJunctions().front() == target) {
            deleteEdge(edge, undoList, false);
        } else {
            undoList->p_add(new GNEChange_Attribute(edge, SUMO_ATTR_TO, target->getID()));
        }
    }
    // deleting edges changes in the underlying EdgeVector so we have to make a copy
    const EdgeVector outgoingNBEdges = moved->getNBNode()->getOutgoingEdges();
    for (const auto& outgoingNBEdge : outgoingNBEdges) {
        // delete edges between the merged junctions
        GNEEdge* edge = myAttributeCarriers->getEdges().at(outgoingNBEdge->getID());
        if (edge->getParentJunctions().back() == target) {
            deleteEdge(edge, undoList, false);
        } else {
            undoList->p_add(new GNEChange_Attribute(edge, SUMO_ATTR_FROM, target->getID()));
        }
    }
    // deleted moved junction
    deleteJunction(moved, undoList);
    undoList->p_end();
}


void
GNENet::selectRoundabout(GNEJunction* junction, GNEUndoList* undoList) {
    for (const EdgeSet& roundabout : myNetBuilder->getEdgeCont().getRoundabouts()) {
        for (NBEdge* edge : roundabout) {
            if (edge->getFromNode() == junction->getNBNode()) {
                undoList->p_begin("select roundabout");
                for (NBEdge* roundaboutEdge : roundabout) {
                    GNEEdge* e = retrieveEdge(roundaboutEdge->getID());
                    e->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                    e->getParentJunctions().back()->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                }
                undoList->p_end();
                return;
            }
        }
    }
}


void
GNENet::createRoundabout(GNEJunction* junction, GNEUndoList* undoList) {
    undoList->p_begin("create roundabout");
    junction->getNBNode()->updateSurroundingGeometry();
    double radius = junction->getNBNode()->getRadius();
    if (radius == NBNode::UNSPECIFIED_RADIUS) {
        radius = OptionsCont::getOptions().getFloat("default.junctions.radius");
    }
    std::vector<GNEEdge*> edges;
    // use clockwise sorting
    for (NBEdge* nbEdge : junction->getNBNode()->getEdges()) {
        edges.push_back(retrieveEdge(nbEdge->getID()));
    }
    const bool lefthand = OptionsCont::getOptions().getBool("lefthand");
    const double lefthandSign = lefthand ? -1 : 1;
    std::vector<GNEJunction*> newJunctions;
    GNEEdge* prevOpposite = nullptr;
    // split incoming/outgoing edges
    for (GNEEdge* edge : edges) {
        GNEJunction* newJunction = nullptr;
        if (edge == prevOpposite) {
            newJunction = newJunctions.back();
        }
        //std::cout << " edge=" << edge->getID() << " prevOpposite=" << Named::getIDSecure(prevOpposite) << " newJunction=" << Named::getIDSecure(newJunction) << "\n";
        prevOpposite = edge->getOppositeEdge();
        const double geomLength = edge->getNBEdge()->getGeometry().length2D();
        const double splitOffset = (edge->getParentJunctions().back() == junction
                                    ? MAX2(POSITION_EPS, geomLength - radius)
                                    : MIN2(geomLength - POSITION_EPS, radius));
        Position pos = edge->getNBEdge()->getGeometry().positionAtOffset2D(splitOffset);
        newJunction = splitEdge(edge, pos, undoList, newJunction);
        if (newJunctions.empty() || newJunction != newJunctions.back()) {
            newJunctions.push_back(newJunction);
        }
    }
    Position center = junction->getPositionInView();
    deleteJunction(junction, undoList);
    // create new edges to connect roundabout junctions (counter-clockwise)
    GNEEdge* tpl = myViewNet->getViewParent()->getInspectorFrame()->getTemplateEditor()->getEdgeTemplate();
    const double resolution = OptionsCont::getOptions().getFloat("opendrive.curve-resolution") * 3;
    for (int i = 0; i < (int)newJunctions.size(); i++) {
        GNEJunction* from = newJunctions[(i + 1) % newJunctions.size()];
        GNEJunction* to = newJunctions[i];
        GNEEdge* newEdge = createEdge(from, to, tpl, undoList);
        const double angle1 = center.angleTo2D(from->getPositionInView());
        const double angle2 = center.angleTo2D(to->getPositionInView());
        // insert geometry points every resolution meters
        const double angleDiff = fabs(GeomHelper::angleDiff(angle2, angle1));
        // circumference = 2 * M_PI * radius, angularFraction = angleDiff / 2 * M_PI
        int numSegments = MAX2(2, (int)ceil(angleDiff * radius / resolution));
        PositionVector innerGeom;
        for (int j = 1; j < numSegments; j++) {
            const double angle = angle1 + lefthandSign * j * angleDiff / numSegments;
            innerGeom.push_back(center + Position(cos(angle) * radius, sin(angle) * radius));
        }
        //std::cout << " newEdge=" << newEdge->getID() << " angle1=" << angle1 << " angle2=" << angle2 << " angleDiff=" << angleDiff
        //    << " numSegments=" << numSegments << " innerGeom=" << innerGeom << "\n";
        newEdge->setAttribute(SUMO_ATTR_SHAPE, toString(innerGeom), undoList);
    }
    undoList->p_end();
}


bool
GNENet::checkJunctionPosition(const Position& pos) {
    // Check that there isn't another junction in the same position as Pos
    for (auto i : myAttributeCarriers->getJunctions()) {
        if (i.second->getPositionInView() == pos) {
            return false;
        }
    }
    return true;
}


void
GNENet::requireSaveNet(bool value) {
    if (myNetSaved == true) {
        WRITE_DEBUG("net has to be saved");
        std::string additionalsSaved = (myAdditionalsSaved ? "saved" : "unsaved");
        std::string demandElementsSaved = (myDemandElementsSaved ? "saved" : "unsaved");
        std::string dataSetsSaved = (myDataElementsSaved ? "saved" : "unsaved");
        WRITE_DEBUG("Current saving Status: net unsaved, additionals " + additionalsSaved +
                    ", demand elements " + demandElementsSaved + ", data sets " + dataSetsSaved);
    }
    myNetSaved = !value;
}


bool
GNENet::isNetSaved() const {
    return myNetSaved;
}


void
GNENet::save(OptionsCont& oc) {
    // compute without volatile options and update network
    computeAndUpdate(oc, false);
    // write network
    NWFrame::writeNetwork(oc, *myNetBuilder);
    myNetSaved = true;
}


void
GNENet::savePlain(OptionsCont& oc) {
    // compute without volatile options
    computeAndUpdate(oc, false);
    NWWriter_XML::writeNetwork(oc, *myNetBuilder);
}


void
GNENet::saveJoined(OptionsCont& oc) {
    // compute without volatile options
    computeAndUpdate(oc, false);
    NWWriter_XML::writeJoinedJunctions(oc, myNetBuilder->getNodeCont());
}


void
GNENet::setViewNet(GNEViewNet* viewNet) {
    // set view net
    myViewNet = viewNet;
    // add default vTypes
    myAttributeCarriers->addDefaultVTypes();
    // update geometry of all lanes (needed  for dotted geometry)
    for (const auto& edge : myAttributeCarriers->getEdges()) {
        for (const auto& lane : edge.second->getLanes()) {
            lane->updateGeometry();
        }
    }
}


GNEJunction*
GNENet::retrieveJunction(const std::string& id, bool failHard) const {
    if (myAttributeCarriers->getJunctions().count(id)) {
        return myAttributeCarriers->getJunctions().at(id);
    } else if (failHard) {
        // If junction wasn't found, throw exception
        throw UnknownElement("Junction " + id);
    } else {
        return nullptr;
    }
}


GNEEdge*
GNENet::retrieveEdge(const std::string& id, bool failHard) const {
    auto i = myAttributeCarriers->getEdges().find(id);
    // If edge was found
    if (i != myAttributeCarriers->getEdges().end()) {
        return i->second;
    } else if (failHard) {
        // If edge wasn't found, throw exception
        throw UnknownElement("Edge " + id);
    } else {
        return nullptr;
    }
}


GNEEdge*
GNENet::retrieveEdge(GNEJunction* from, GNEJunction* to, bool failHard) const {
    if ((from != nullptr) && (to != nullptr)) {
        // iterate over Junctions of net
        for (const auto& edge : myAttributeCarriers->getEdges()) {
            if ((edge.second->getParentJunctions().front() == from) && (edge.second->getParentJunctions().back() == to)) {
                return edge.second;
            }
        }
        // if edge wasn't found, throw exception or return nullptr
        if (failHard) {
            throw UnknownElement("Edge with from='" + from->getID() + "' and to='" + to->getID() + "'");
        } else {
            return nullptr;
        }
    } else {
        throw UnknownElement("Junctions cannot be nullptr");
    }
}


GNEConnection*
GNENet::retrieveConnection(const std::string& id, bool failHard) const {
    // iterate over junctions
    for (auto i : myAttributeCarriers->getJunctions()) {
        // iterate over connections
        for (auto j : i.second->getGNEConnections()) {
            if (j->getID() == id) {
                return j;
            }
        }
    }
    if (failHard) {
        // If POI wasn't found, throw exception
        throw UnknownElement("Connection " + id);
    } else {
        return nullptr;
    }
}


std::vector<GNEConnection*>
GNENet::retrieveConnections(bool onlySelected) const {
    std::vector<GNEConnection*> result;
    // iterate over junctions
    for (auto i : myAttributeCarriers->getJunctions()) {
        // iterate over connections
        for (auto j : i.second->getGNEConnections()) {
            if (!onlySelected || j->isAttributeCarrierSelected()) {
                result.push_back(j);
            }
        }
    }
    return result;
}


GNECrossing*
GNENet::retrieveCrossing(const std::string& id, bool failHard) const {
    // iterate over junctions
    for (auto i : myAttributeCarriers->getJunctions()) {
        // iterate over crossings
        for (auto j : i.second->getGNECrossings()) {
            if (j->getID() == id) {
                return j;
            }
        }
    }
    if (failHard) {
        // If POI wasn't found, throw exception
        throw UnknownElement("Crossing " + id);
    } else {
        return nullptr;
    }
}


std::vector<GNECrossing*>
GNENet::retrieveCrossings(bool onlySelected) const {
    std::vector<GNECrossing*> result;
    // iterate over junctions
    for (auto i : myAttributeCarriers->getJunctions()) {
        // iterate over crossings
        for (auto j : i.second->getGNECrossings()) {
            if (!onlySelected || j->isAttributeCarrierSelected()) {
                result.push_back(j);
            }
        }
    }
    return result;
}


std::vector<GNEEdge*>
GNENet::retrieveEdges(bool onlySelected) {
    std::vector<GNEEdge*> result;
    // returns edges depending of selection
    for (auto i : myAttributeCarriers->getEdges()) {
        if (!onlySelected || i.second->isAttributeCarrierSelected()) {
            result.push_back(i.second);
        }
    }
    return result;
}


std::vector<GNELane*>
GNENet::retrieveLanes(bool onlySelected) {
    std::vector<GNELane*> result;
    // returns lanes depending of selection
    for (auto i : myAttributeCarriers->getEdges()) {
        for (auto j : i.second->getLanes()) {
            if (!onlySelected || j->isAttributeCarrierSelected()) {
                result.push_back(j);
            }
        }
    }
    return result;
}


GNELane*
GNENet::retrieveLane(const std::string& id, bool failHard, bool checkVolatileChange) {
    const std::string edge_id = SUMOXMLDefinitions::getEdgeIDFromLane(id);
    GNEEdge* edge = retrieveEdge(edge_id, failHard);
    if (edge != nullptr) {
        GNELane* lane = nullptr;
        // search  lane in lane's edges
        for (auto it : edge->getLanes()) {
            if (it->getID() == id) {
                lane = it;
            }
        }
        // throw exception or return nullptr if lane wasn't found
        if (lane == nullptr) {
            if (failHard) {
                // Throw exception if failHard is enabled
                throw UnknownElement(toString(SUMO_TAG_LANE) + " " + id);
            }
        } else {
            // check if the recomputing with volatile option has changed the number of lanes (needed for additionals and demand elements)
            if (checkVolatileChange && (myEdgesAndNumberOfLanes.count(edge_id) == 1) && myEdgesAndNumberOfLanes[edge_id] != (int)edge->getLanes().size()) {
                return edge->getLanes().at(lane->getIndex() + 1);
            }
            return lane;
        }
    } else if (failHard) {
        // Throw exception if failHard is enabled
        throw UnknownElement(toString(SUMO_TAG_EDGE) + " " + edge_id);
    }
    return nullptr;
}


std::vector<GNEJunction*>
GNENet::retrieveJunctions(bool onlySelected) {
    std::vector<GNEJunction*> result;
    // returns junctions depending of selection
    for (auto i : myAttributeCarriers->getJunctions()) {
        if (!onlySelected || i.second->isAttributeCarrierSelected()) {
            result.push_back(i.second);
        }
    }
    return result;
}


std::vector<GNEShape*>
GNENet::retrieveShapes(SumoXMLTag shapeTag, bool onlySelected) {
    std::vector<GNEShape*> result;
    // return all polys depending of onlySelected
    for (const auto& shape : myAttributeCarriers->getShapes().at(shapeTag)) {
        if (!onlySelected || shape.second->isAttributeCarrierSelected()) {
            result.push_back(shape.second);
        }
    }
    return result;
}


std::vector<GNEShape*>
GNENet::retrieveShapes(bool onlySelected) {
    std::vector<GNEShape*> result;
    // return all polygons and POIs
    for (const auto& shapeTag : myAttributeCarriers->getShapes()) {
        for (const auto& shape : shapeTag.second) {
            if (!onlySelected || shape.second->isAttributeCarrierSelected()) {
                result.push_back(shape.second);
            }
        }
    }
    return result;
}


void
GNENet::addGLObjectIntoGrid(GNEAttributeCarrier* AC) {
    // first check if given object has an associated GUIGlObject
    if (AC->getGUIGlObject()) {
        // check if object must be inserted in RTREE
        if (AC->getTagProperty().isPlacedInRTree()) {
            myGrid.addAdditionalGLObject(AC->getGUIGlObject());
        }
    }
}


void
GNENet::removeGLObjectFromGrid(GNEAttributeCarrier* AC) {
    // first check if given object has an associated GUIGlObject
    if (AC->getGUIGlObject()) {
        // check if object must be inserted in RTREE
        if (AC->getTagProperty().isPlacedInRTree()) {
            myGrid.removeAdditionalGLObject(AC->getGUIGlObject());
        }
    }
}


GNEAttributeCarrier*
GNENet::retrieveAttributeCarrier(const GUIGlID id, bool failHard) const {
    // obtain blocked GUIGlObject
    GUIGlObject* object = GUIGlObjectStorage::gIDStorage.getObjectBlocking(id);
    // Make sure that object exists
    if (object != nullptr) {
        // unblock and try to parse to AtributeCarrier
        GUIGlObjectStorage::gIDStorage.unblockObject(id);
        GNEAttributeCarrier* ac = dynamic_cast<GNEAttributeCarrier*>(object);
        // If was sucesfully parsed, return it
        if (ac == nullptr) {
            throw ProcessError("GUIGlObject does not match the declared type");
        } else {
            return ac;
        }
    } else if (failHard) {
        throw ProcessError("Attempted to retrieve non-existant GUIGlObject");
    } else {
        return nullptr;
    }
}


std::vector<GNEAttributeCarrier*>
GNENet::retrieveAttributeCarriers(SumoXMLTag type) {
    std::vector<GNEAttributeCarrier*> result;
    if (type == SUMO_TAG_NOTHING) {
        // return all elements
        for (const auto& junction : myAttributeCarriers->getJunctions()) {
            result.push_back(junction.second);
            for (const auto& crossing : junction.second->getGNECrossings()) {
                result.push_back(crossing);
            }
        }
        for (const auto& edge : myAttributeCarriers->getEdges()) {
            result.push_back(edge.second);
            for (const auto& lane : edge.second->getLanes()) {
                result.push_back(lane);
            }
            for (const auto& connection : edge.second->getGNEConnections()) {
                result.push_back(connection);
            }
        }
        for (const auto& additionalSet : myAttributeCarriers->getAdditionals()) {
            for (const auto& additional : additionalSet.second) {
                result.push_back(additional.second);
            }
        }
        for (const auto& shapeSet : myAttributeCarriers->getShapes()) {
            for (const auto& shape : shapeSet.second) {
                result.push_back(shape.second);
            }
        }
        for (const auto& TAZSet : myAttributeCarriers->getTAZElements()) {
            for (const auto& TAZElement : TAZSet.second) {
                result.push_back(TAZElement.second);
            }
        }
        for (const auto& demandElementSet : myAttributeCarriers->getDemandElements()) {
            for (const auto& demandElement : demandElementSet.second) {
                result.push_back(demandElement.second);
            }
        }
        for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
            result.push_back(dataSet.second);
            for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
                result.push_back(dataInterval.second);
                for (const auto& genericData : dataInterval.second->getGenericDataChildren()) {
                    result.push_back(genericData);
                }
            }
        }
    } else if (GNEAttributeCarrier::getTagProperties(type).isAdditionalElement()) {
        // iterate over all additionals
        for (const auto& additionalTag : myAttributeCarriers->getAdditionals()) {
            for (const auto& additional : additionalTag.second) {
                if (additional.second->getTagProperty().getTag() == type) {
                    result.push_back(additional.second);
                } else {
                    // check additional children
                    for (const auto& additionalChild : additional.second->getChildAdditionals()) {
                        if (additionalChild->getTagProperty().getTag() == type) {
                            result.push_back(additionalChild);
                        }
                    }
                }
            }
        }
    } else if (GNEAttributeCarrier::getTagProperties(type).isShape()) {
        // only returns shapes of a certain type.
        for (const auto& shape : myAttributeCarriers->getShapes().at(type)) {
            result.push_back(shape.second);
        }
    } else if (GNEAttributeCarrier::getTagProperties(type).isTAZElement()) {
        // only returns TAZ of a certain type.
        for (const auto& TAZElement : myAttributeCarriers->getTAZElements().at(type)) {
            result.push_back(TAZElement.second);
        }
    } else if (GNEAttributeCarrier::getTagProperties(type).isDemandElement()) {
        // only returns demand elements of a certain type.
        for (const auto& demandElemet : myAttributeCarriers->getDemandElements().at(type)) {
            result.push_back(demandElemet.second);
        }
    } else if (GNEAttributeCarrier::getTagProperties(type).isGenericData()) {
        for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
            for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
                for (const auto& genericData : dataInterval.second->getGenericDataChildren()) {
                    if (genericData->getTagProperty().getTag() == type) {
                        result.push_back(genericData);
                    }
                }
            }
        }
    } else {
        // return only a part of elements, depending of type
        switch (type) {
            case SUMO_TAG_JUNCTION:
                for (const auto& junction : myAttributeCarriers->getJunctions()) {
                    result.push_back(junction.second);
                }
                break;
            case SUMO_TAG_EDGE:
                for (const auto& edge : myAttributeCarriers->getEdges()) {
                    result.push_back(edge.second);
                }
                break;
            case SUMO_TAG_LANE:
                for (const auto& edge : myAttributeCarriers->getEdges()) {
                    for (const auto& lane : edge.second->getLanes()) {
                        result.push_back(lane);
                    }
                }
                break;
            case SUMO_TAG_CONNECTION:
                for (const auto& edge : myAttributeCarriers->getEdges()) {
                    for (const auto& connection : edge.second->getGNEConnections()) {
                        result.push_back(connection);
                    }
                }
                break;
            case SUMO_TAG_CROSSING:
                for (const auto& junction : myAttributeCarriers->getJunctions()) {
                    for (const auto& crossing : junction.second->getGNECrossings()) {
                        result.push_back(crossing);
                    }
                }
                break;
            case SUMO_TAG_DATASET:
                for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
                    result.push_back(dataSet.second);
                }
                break;
            case SUMO_TAG_DATAINTERVAL:
                for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
                    for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
                        result.push_back(dataInterval.second);
                    }
                }
                break;
            default:
                // return nothing
                break;
        }
    }
    return result;
}


void
GNENet::computeNetwork(GNEApplicationWindow* window, bool force, bool volatileOptions, std::string additionalPath, std::string demandPath, std::string dataPath) {
    if (!myNeedRecompute) {
        if (force) {
            if (volatileOptions) {
                window->setStatusBarText("Forced computing junctions with volatile options ...");
            } else {
                window->setStatusBarText("Forced computing junctions ...");
            }
        } else {
            return;
        }
    } else {
        if (volatileOptions) {
            window->setStatusBarText("Computing junctions with volatile options ...");
        } else {
            window->setStatusBarText("Computing junctions  ...");
        }
    }
    // save current number of lanes for every edge if recomputing is with volatile options
    if (volatileOptions) {
        for (auto it : myAttributeCarriers->getEdges()) {
            myEdgesAndNumberOfLanes[it.second->getID()] = (int)it.second->getLanes().size();
        }
    }
    // compute and update
    OptionsCont& oc = OptionsCont::getOptions();
    computeAndUpdate(oc, volatileOptions);
    // load additionals if was recomputed with volatile options
    if (additionalPath != "") {
        // Create additional handler
        GNEAdditionalHandler additionalHandler(additionalPath, this);
        // Run parser
        if (!XMLSubSys::runParser(additionalHandler, additionalPath, false)) {
            WRITE_MESSAGE("Loading of " + additionalPath + " failed.");
        }
        // clear myEdgesAndNumberOfLanes after reload additionals
        myEdgesAndNumberOfLanes.clear();
    }
    // load demand elements if was recomputed with volatile options
    if (demandPath != "") {
        // Create demandElement handler
        GNERouteHandler demandElementHandler(demandPath, this, false);
        // Run parser
        if (!XMLSubSys::runParser(demandElementHandler, demandPath, false)) {
            WRITE_MESSAGE("Loading of " + demandPath + " failed.");
        }
        // clear myEdgesAndNumberOfLanes after reload demandElements
        myEdgesAndNumberOfLanes.clear();
    }
    UNUSED_PARAMETER(dataPath);
    window->getApp()->endWaitCursor();
    window->setStatusBarText("Finished computing junctions.");
}


void
GNENet::computeDemandElements(GNEApplicationWindow* window) {
    window->setStatusBarText("Computing demand elements ...");
    // if we aren't in Demand mode, update path calculator
    if (!myViewNet->getEditModes().isCurrentSupermodeDemand())  {
        myPathCalculator->updatePathCalculator();
    }
    // iterate over all demand elements and compute
    for (const auto& i : myAttributeCarriers->getDemandElements()) {
        for (const auto& j : i.second) {
            j.second->computePath();
        }
    }
    window->setStatusBarText("Finished computing demand elements.");
}


void
GNENet::computeDataElements(GNEApplicationWindow* window) {
    window->setStatusBarText("Computing data elements ...");
    /*
    // iterate over all demand elements and compute
    for (const auto& i : myAttributeCarriers->getDemandElements()) {
        for (const auto& j : i.second) {
            j.second->computePath();
        }
    }
    */
    window->setStatusBarText("Finished computing data elements.");
}


void
GNENet::computeJunction(GNEJunction* junction) {
    // recompute tl-logics
    OptionsCont& oc = OptionsCont::getOptions();
    NBTrafficLightLogicCont& tllCont = getTLLogicCont();
    // iterate over traffic lights definitions. Make a copy because invalid
    // definitions will be removed (and would otherwise destroy the iterator)
    const std::set<NBTrafficLightDefinition*> tlsDefs = junction->getNBNode()->getControllingTLS();
    for (auto it : tlsDefs) {
        it->setParticipantsInformation();
        it->setTLControllingInformation();
        tllCont.computeSingleLogic(oc, it);
    }

    // @todo compute connections etc...
}


void
GNENet::requireRecompute() {
    myNeedRecompute = true;
}


bool
GNENet::isNetRecomputed() const {
    return (myNeedRecompute == false);
}


bool
GNENet::netHasGNECrossings() const {
    for (auto n : myAttributeCarriers->getJunctions()) {
        if (n.second->getGNECrossings().size() > 0) {
            return true;
        }
    }
    return false;
}


FXApp*
GNENet::getApp() {
    return myViewNet->getApp();
}


NBNetBuilder*
GNENet::getNetBuilder() const {
    return myNetBuilder;
}


bool
GNENet::joinSelectedJunctions(GNEUndoList* undoList) {
    std::vector<GNEJunction*> selectedJunctions = retrieveJunctions(true);
    if (selectedJunctions.size() < 2) {
        return false;
    }
    EdgeVector allIncoming;
    EdgeVector allOutgoing;
    std::set<NBNode*, ComparatorIdLess> cluster;
    for (auto it : selectedJunctions) {
        cluster.insert(it->getNBNode());
        const EdgeVector& incoming = it->getNBNode()->getIncomingEdges();
        allIncoming.insert(allIncoming.end(), incoming.begin(), incoming.end());
        const EdgeVector& outgoing = it->getNBNode()->getOutgoingEdges();
        allOutgoing.insert(allOutgoing.end(), outgoing.begin(), outgoing.end());
    }
    // create new junction
    Position pos;
    Position oldPos;
    bool setTL;
    std::string id = "cluster";
    TrafficLightType type;
    SumoXMLNodeType nodeType = SumoXMLNodeType::UNKNOWN;
    myNetBuilder->getNodeCont().analyzeCluster(cluster, id, pos, setTL, type, nodeType);
    // save position
    oldPos = pos;

    // Check that there isn't another junction in the same position as Pos but doesn't belong to cluster
    for (auto i : myAttributeCarriers->getJunctions()) {
        if ((i.second->getPositionInView() == pos) && (cluster.find(i.second->getNBNode()) == cluster.end())) {
            // show warning in gui testing debug mode
            WRITE_DEBUG("Opening FXMessageBox 'Join non-selected junction'");
            // Ask confirmation to user
            FXuint answer = FXMessageBox::question(getApp(), MBOX_YES_NO,
                                                   ("Position of joined " + toString(SUMO_TAG_JUNCTION)).c_str(), "%s",
                                                   ("There is another unselected " + toString(SUMO_TAG_JUNCTION) + " in the same position of joined " + toString(SUMO_TAG_JUNCTION) +
                                                    + ".\nIt will be joined with the other selected " + toString(SUMO_TAG_JUNCTION) + "s. Continue?").c_str());
            if (answer != 1) { // 1:yes, 2:no, 4:esc
                // write warning if netedit is running in testing mode
                if (answer == 2) {
                    WRITE_DEBUG("Closed FXMessageBox 'Join non-selected junction' with 'No'");
                } else if (answer == 4) {
                    WRITE_DEBUG("Closed FXMessageBox 'Join non-selected junction' with 'ESC'");
                }
                return false;
            } else {
                // write warning if netedit is running in testing mode
                WRITE_DEBUG("Closed FXMessageBox 'Join non-selected junction' with 'Yes'");
                // select conflicted junction an join all again
                i.second->setAttribute(GNE_ATTR_SELECTED, "true", undoList);
                return joinSelectedJunctions(undoList);
            }
        }
    }

    // use checkJunctionPosition to avoid conflicts with junction in the same position as others
    while (checkJunctionPosition(pos) == false) {
        pos.setx(pos.x() + 0.1);
        pos.sety(pos.y() + 0.1);
    }

    // start with the join selected junctions
    undoList->p_begin("Join selected " + toString(SUMO_TAG_JUNCTION) + "s");
    GNEJunction* joined = createJunction(pos, undoList);
    joined->setAttribute(SUMO_ATTR_TYPE, toString(nodeType), undoList); // i.e. rail crossing
    if (setTL) {
        joined->setAttribute(SUMO_ATTR_TLTYPE, toString(type), undoList);
    }

    // #3128 this is not undone when calling 'undo'
    myNetBuilder->getNodeCont().registerJoinedCluster(cluster);

    // first remove all crossing of the involved junctions and edges
    // (otherwise edge removal will trigger discarding)
    std::vector<NBNode::Crossing> oldCrossings;
    for (auto i : selectedJunctions) {
        while (i->getGNECrossings().size() > 0) {
            GNECrossing* crossing = i->getGNECrossings().front();
            oldCrossings.push_back(*crossing->getNBCrossing());
            deleteCrossing(crossing, undoList);
        }
    }

    // preserve old connections
    for (auto it : selectedJunctions) {
        it->setLogicValid(false, undoList);
    }
    // remap edges
    for (auto it : allIncoming) {
        undoList->p_add(new GNEChange_Attribute(myAttributeCarriers->getEdges().at(it->getID()), SUMO_ATTR_TO, joined->getID()));
    }

    EdgeSet edgesWithin;
    for (auto it : allOutgoing) {
        // delete edges within the cluster
        GNEEdge* edge = myAttributeCarriers->getEdges().at(it->getID());
        if (edge->getParentJunctions().back() == joined) {
            edgesWithin.insert(it);
            deleteEdge(edge, undoList, false);
        } else {
            undoList->p_add(new GNEChange_Attribute(myAttributeCarriers->getEdges().at(it->getID()), SUMO_ATTR_FROM, joined->getID()));
        }
    }

    // remap all crossing of the involved junctions and edges
    for (auto nbc : oldCrossings) {
        bool keep = true;
        for (NBEdge* e : nbc.edges) {
            if (edgesWithin.count(e) != 0) {
                keep = false;
                break;
            }
        };
        if (keep) {
            undoList->add(new GNEChange_Crossing(joined, nbc.edges, nbc.width,
                                                 nbc.priority || joined->getNBNode()->isTLControlled(),
                                                 nbc.customTLIndex, nbc.customTLIndex2, nbc.customShape,
                                                 false, true), true);
        }
    }

    // delete original junctions
    for (auto it : selectedJunctions) {
        deleteJunction(it, undoList);
    }
    joined->setAttribute(SUMO_ATTR_ID, id, undoList);


    // check if joined junction had to change their original position to avoid errors
    if (pos != oldPos) {
        joined->setAttribute(SUMO_ATTR_POSITION, toString(oldPos), undoList);
    }
    undoList->p_end();
    return true;
}


bool
GNENet::cleanInvalidCrossings(GNEUndoList* undoList) {
    // obtain current net's crossings
    std::vector<GNECrossing*> myNetCrossings;
    for (auto it : myAttributeCarriers->getJunctions()) {
        myNetCrossings.reserve(myNetCrossings.size() + it.second->getGNECrossings().size());
        myNetCrossings.insert(myNetCrossings.end(), it.second->getGNECrossings().begin(), it.second->getGNECrossings().end());
    }
    // obtain invalid crossigns
    std::vector<GNECrossing*> myInvalidCrossings;
    for (auto i = myNetCrossings.begin(); i != myNetCrossings.end(); i++) {
        if ((*i)->getNBCrossing()->valid == false) {
            myInvalidCrossings.push_back(*i);
        }
    }

    if (myInvalidCrossings.empty()) {
        // show warning in gui testing debug mode
        WRITE_DEBUG("Opening FXMessageBox 'No crossing to remove'");
        // open a dialog informing that there isn't crossing to remove
        FXMessageBox::warning(getApp(), MBOX_OK,
                              ("Clear " + toString(SUMO_TAG_CROSSING) + "s").c_str(), "%s",
                              ("There is no invalid " + toString(SUMO_TAG_CROSSING) + "s to remove").c_str());
        // show warning in gui testing debug mode
        WRITE_DEBUG("Closed FXMessageBox 'No crossing to remove' with 'OK'");
    } else {
        std::string plural = myInvalidCrossings.size() == 1 ? ("") : ("s");
        // show warning in gui testing debug mode
        WRITE_DEBUG("Opening FXMessageBox 'clear crossings'");
        // Ask confirmation to user
        FXuint answer = FXMessageBox::question(getApp(), MBOX_YES_NO,
                                               ("Clear " + toString(SUMO_TAG_CROSSING) + "s").c_str(), "%s",
                                               ("Clear " + toString(SUMO_TAG_CROSSING) + plural + " will be removed. Continue?").c_str());
        if (answer != 1) { // 1:yes, 2:no, 4:esc
            // write warning if netedit is running in testing mode
            if (answer == 2) {
                WRITE_DEBUG("Closed FXMessageBox 'clear crossings' with 'No'");
            } else if (answer == 4) {
                WRITE_DEBUG("Closed FXMessageBox 'clear crossings' with 'ESC'");
            }
        } else {
            undoList->p_begin("Clean " + toString(SUMO_TAG_CROSSING) + "s");
            for (auto i = myInvalidCrossings.begin(); i != myInvalidCrossings.end(); i++) {
                deleteCrossing((*i), undoList);
            }
            undoList->p_end();
        }
    }
    return 1;
}


void
GNENet::removeSolitaryJunctions(GNEUndoList* undoList) {
    undoList->p_begin("Clean " + toString(SUMO_TAG_JUNCTION) + "s");
    std::vector<GNEJunction*> toRemove;
    for (auto it : myAttributeCarriers->getJunctions()) {
        GNEJunction* junction = it.second;
        if (junction->getNBNode()->getEdges().size() == 0) {
            toRemove.push_back(junction);
        }
    }
    for (auto it : toRemove) {
        deleteJunction(it, undoList);
    }
    undoList->p_end();
}


void
GNENet::cleanUnusedRoutes(GNEUndoList* undoList) {
    // first declare a vector to save all routes without children
    std::vector<GNEDemandElement*> routesWithoutChildren;
    routesWithoutChildren.reserve(myAttributeCarriers->getDemandElements().at(SUMO_TAG_ROUTE).size());
    // iterate over routes
    for (const auto& i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_ROUTE)) {
        if (i.second->getChildDemandElements().empty()) {
            routesWithoutChildren.push_back(i.second);
        }
    }
    // finally remove all routesWithoutChildren
    if (routesWithoutChildren.size() > 0) {
        // begin undo list
        undoList->p_begin("clean unused routes");
        // iterate over routesWithoutChildren
        for (const auto& i : routesWithoutChildren) {
            // due route doesn't have children, simply call GNEChange_DemandElement
            undoList->add(new GNEChange_DemandElement(i, false), true);
        }
        // end undo list
        undoList->p_end();
    }
}


void
GNENet::joinRoutes(GNEUndoList* undoList) {
    // first declare a sorted set of sorted route's edges in string format
    std::set<std::pair<std::string, GNEDemandElement*> > mySortedRoutes;
    // iterate over routes and save it in mySortedRoutes  (only if it doesn't have Stop Children)
    for (const auto& i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_ROUTE)) {
        // first check route has stops
        bool hasStops = false;
        for (const auto& j : i.second->getChildDemandElements()) {
            if (j->getTagProperty().isStop()) {
                hasStops = true;
            }
        }
        if (!hasStops) {
            mySortedRoutes.insert(std::make_pair(GNEAttributeCarrier::parseIDs(i.second->getParentEdges()), i.second));
        }
    }
    // now declare a matrix in which organice routes to be merged
    std::vector<std::vector<GNEDemandElement*> > routesToMerge;
    auto index = mySortedRoutes.begin();
    // iterate over mySortedRoutes
    for (auto i = mySortedRoutes.begin(); i != mySortedRoutes.end(); i++) {
        if (routesToMerge.empty()) {
            routesToMerge.push_back({i->second});
        } else {
            if (index->first == i->first) {
                routesToMerge.back().push_back(i->second);
            } else {
                routesToMerge.push_back({i->second});
                index = i;
            }
        }
    }
    // now check if there is routes to merge
    bool thereIsRoutesToMerge = false;
    for (const auto& i : routesToMerge) {
        if (i.size() > 1) {
            thereIsRoutesToMerge = true;
        }
    }
    // if exist
    if (thereIsRoutesToMerge) {
        // begin undo list
        undoList->p_begin("merge routes");
        // iterate over route to edges
        for (const auto& i : routesToMerge) {
            if (i.size() > 1) {
                // iterate over duplicated routes
                for (int j = 1; j < (int)i.size(); j++) {
                    // move all vehicles of every duplicated route
                    while (i.at(j)->getChildDemandElements().size() > 0) {
                        i.at(j)->getChildDemandElements().front()->setAttribute(SUMO_ATTR_ROUTE, i.at(0)->getID(), undoList);
                    }
                    // finally remove route
                    undoList->add(new GNEChange_DemandElement(i.at(j), false), true);
                }
            }
        }
        // end undo list
        undoList->p_end();
    }
}


void
GNENet::cleanInvalidDemandElements(GNEUndoList* undoList) {
    // first declare a vector to save all invalid demand elements
    std::vector<GNEDemandElement*> invalidDemandElements;
    invalidDemandElements.reserve(myAttributeCarriers->getDemandElements().at(SUMO_TAG_ROUTE).size() +
                                  myAttributeCarriers->getDemandElements().at(SUMO_TAG_FLOW).size() +
                                  myAttributeCarriers->getDemandElements().at(SUMO_TAG_TRIP).size());
    // iterate over routes
    for (const auto& i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_ROUTE)) {
        if (!i.second->isDemandElementValid()) {
            invalidDemandElements.push_back(i.second);
        }
    }
    // iterate over flows
    for (const auto& i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_FLOW)) {
        if (!i.second->isDemandElementValid()) {
            invalidDemandElements.push_back(i.second);
        }
    }
    // iterate over trip
    for (const auto& i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_TRIP)) {
        if (!i.second->isDemandElementValid()) {
            invalidDemandElements.push_back(i.second);
        }
    }
    // continue if there is invalidDemandElements to remove
    if (invalidDemandElements.size() > 0) {
        // begin undo list
        undoList->p_begin("remove invalid demand elements");
        // iterate over invalidDemandElements
        for (const auto& i : invalidDemandElements) {
            // simply call GNEChange_DemandElement
            undoList->add(new GNEChange_DemandElement(i, false), true);
        }
        // end undo list
        undoList->p_end();
    }
}

void
GNENet::replaceJunctionByGeometry(GNEJunction* junction, GNEUndoList* undoList) {
    if (junction->getNBNode()->checkIsRemovable()) {
        // start operation
        undoList->p_begin("Replace junction by geometry");
        // obtain Edges to join
        std::vector<std::pair<NBEdge*, NBEdge*> > toJoin = junction->getNBNode()->getEdgesToJoin();
        // clear connections of junction to replace
        clearJunctionConnections(junction, undoList);
        // iterate over NBEdges to join
        for (auto j : toJoin) {
            // obtain GNEEdges
            GNEEdge* begin = myAttributeCarriers->getEdges().at(j.first->getID());
            GNEEdge* continuation = myAttributeCarriers->getEdges().at(j.second->getID());
            // remove connections between the edges
            std::vector<NBEdge::Connection> connections = begin->getNBEdge()->getConnections();
            for (auto con : connections) {
                undoList->add(new GNEChange_Connection(begin, con, false, false), true);
            }
            // fix shape of replaced edge
            PositionVector newShape = begin->getNBEdge()->getInnerGeometry();
            if (begin->getNBEdge()->hasDefaultGeometryEndpointAtNode(begin->getNBEdge()->getToNode())) {
                newShape.push_back(junction->getNBNode()->getPosition());
            } else {
                newShape.push_back(begin->getNBEdge()->getGeometry()[-1]);
            }
            if (continuation->getNBEdge()->hasDefaultGeometryEndpointAtNode(begin->getNBEdge()->getToNode())) {
                newShape.push_back_noDoublePos(junction->getNBNode()->getPosition());
            } else {
                newShape.push_back_noDoublePos(continuation->getNBEdge()->getGeometry()[0]);
            }
            // replace incoming edge
            replaceIncomingEdge(continuation, begin, undoList);

            newShape.append(continuation->getNBEdge()->getInnerGeometry());
            begin->setAttribute(GNE_ATTR_SHAPE_END, continuation->getAttribute(GNE_ATTR_SHAPE_END), undoList);
            begin->setAttribute(SUMO_ATTR_ENDOFFSET, continuation->getAttribute(SUMO_ATTR_ENDOFFSET), undoList);
            begin->setAttribute(SUMO_ATTR_SHAPE, toString(newShape), undoList);
            begin->getNBEdge()->resetNodeBorder(begin->getNBEdge()->getToNode());
        }
        //delete replaced junction
        deleteJunction(junction, undoList);
        // finish operation
        undoList->p_end();
    } else {
        throw ProcessError("Junction isn't removable");
    }
}


void
GNENet::splitJunction(GNEJunction* junction, bool reconnect, GNEUndoList* undoList) {
    std::vector<std::pair<Position, std::string> > endpoints = junction->getNBNode()->getEndPoints();
    if (endpoints.size() < 2) {
        return;
    }
    // start operation
    undoList->p_begin("Split junction");
    // record connections
    std::map<GNEEdge*, std::vector<NBEdge::Connection>> straightConnections;
    for (GNEEdge* e : junction->getGNEIncomingEdges()) {
        for (const auto& c : e->getNBEdge()->getConnections()) {
            if (c.fromLane >= 0 && junction->getNBNode()->getDirection(e->getNBEdge(), c.toEdge) == LinkDirection::STRAIGHT) {
                straightConnections[e].push_back(c);
            }
        };
    }
    //std::cout << "split junction at endpoints:\n";

    junction->setLogicValid(false, undoList);
    for (const auto& pair : endpoints) {
        const Position& pos = pair.first;
        const std::string& origID = pair.second;
        GNEJunction* newJunction = createJunction(pos, undoList);
        std::string newID = origID != "" ? origID : newJunction->getID();
        // make a copy because the original vectors are modified during iteration
        const std::vector<GNEEdge*> incoming = junction->getGNEIncomingEdges();
        const std::vector<GNEEdge*> outgoing = junction->getGNEOutgoingEdges();
        //std::cout << "  checkEndpoint " << pair.first << " " << pair.second << " newID=" << newID << "\n";
        for (GNEEdge* e : incoming) {
            //std::cout << "   incoming " << e->getID() << " pos=" << pos << " origTo=" << e->getNBEdge()->getParameter("origTo") << " newID=" << newID << "\n";
            if (e->getNBEdge()->getGeometry().back().almostSame(pos) || e->getNBEdge()->getParameter("origTo") == newID) {
                //std::cout << "     match\n";
                undoList->p_add(new GNEChange_Attribute(e, SUMO_ATTR_TO, newJunction->getID()));
            }
        }
        for (GNEEdge* e : outgoing) {
            //std::cout << "   outgoing " << e->getID() << " pos=" << pos << " origFrom=" << e->getNBEdge()->getParameter("origFrom") << " newID=" << newID << "\n";
            if (e->getNBEdge()->getGeometry().front().almostSame(pos) || e->getNBEdge()->getParameter("origFrom") == newID) {
                //std::cout << "     match\n";
                undoList->p_add(new GNEChange_Attribute(e, SUMO_ATTR_FROM, newJunction->getID()));
            }
        }
        if (newID != newJunction->getID()) {
            if (newJunction->isValid(SUMO_ATTR_ID, newID)) {
                undoList->p_add(new GNEChange_Attribute(newJunction, SUMO_ATTR_ID, newID));
            } else {
                WRITE_WARNING("Could not rename split node to '" + newID + "'");
            }
        }
    }
    // recreate edges from straightConnections
    if (reconnect) {
        for (const auto& item : straightConnections) {
            GNEEdge* in = item.first;
            std::map<NBEdge*, GNEEdge*> newEdges;
            for (auto& c : item.second) {
                GNEEdge* out = retrieveEdge(c.toEdge->getID());
                GNEEdge* newEdge = nullptr;
                if (in->getParentJunctions().back() == out->getParentJunctions().front()) {
                    continue;
                }
                if (newEdges.count(c.toEdge) == 0) {
                    newEdge = createEdge(in->getParentJunctions().back(), out->getParentJunctions().front(), in, undoList);
                    newEdges[c.toEdge] = newEdge;
                    newEdge->setAttribute(SUMO_ATTR_NUMLANES, "1", undoList);
                } else {
                    newEdge = newEdges[c.toEdge];
                    duplicateLane(newEdge->getLanes().back(), undoList, true);
                }
                // copy permissions
                newEdge->getLanes().back()->setAttribute(SUMO_ATTR_ALLOW,
                        in->getLanes()[c.fromLane]-> getAttribute(SUMO_ATTR_ALLOW), undoList);
            }
        }
    }

    deleteJunction(junction, undoList);
    // finish operation
    undoList->p_end();
}



void
GNENet::clearJunctionConnections(GNEJunction* junction, GNEUndoList* undoList) {
    undoList->p_begin("clear junction connections");
    std::vector<GNEConnection*> connections = junction->getGNEConnections();
    // Iterate over all connections and clear it
    for (auto i : connections) {
        deleteConnection(i, undoList);
    }
    undoList->p_end();
}


void
GNENet::resetJunctionConnections(GNEJunction* junction, GNEUndoList* undoList) {
    undoList->p_begin("reset junction connections");
    // first clear connections
    clearJunctionConnections(junction, undoList);
    // invalidate logic to create new connections in the next recomputing
    junction->setLogicValid(false, undoList);
    undoList->p_end();
}


void
GNENet::changeEdgeEndpoints(GNEEdge* edge, const std::string& newSource, const std::string& newDest) {
    NBNode* from = retrieveJunction(newSource)->getNBNode();
    NBNode* to = retrieveJunction(newDest)->getNBNode();
    edge->getNBEdge()->reinitNodes(from, to);
    requireRecompute();
}


GNEViewNet*
GNENet::getViewNet() const {
    return myViewNet;
}


std::vector<GNEAttributeCarrier*>
GNENet::getSelectedAttributeCarriers(bool ignoreCurrentSupermode) {
    // declare vector to save result
    std::vector<GNEAttributeCarrier*> result;
    result.reserve(gSelected.getSelected().size());
    // iterate over all elements of global selection
    for (const auto& glID : gSelected.getSelected()) {
        // obtain AC
        GNEAttributeCarrier* AC = retrieveAttributeCarrier(glID, false);
        // check if attribute carrier exist and is selected
        if (AC && AC->isAttributeCarrierSelected()) {
            bool insert = false;
            if (ignoreCurrentSupermode) {
                insert = true;
            } else if (myViewNet->getEditModes().isCurrentSupermodeNetwork() && (AC->getTagProperty().isNetworkElement() ||
                       AC->getTagProperty().isAdditionalElement() || AC->getTagProperty().isShape() || AC->getTagProperty().isTAZElement())) {
                insert = true;
            } else if (myViewNet->getEditModes().isCurrentSupermodeDemand() && AC->getTagProperty().isDemandElement()) {
                insert = true;
            } else if (myViewNet->getEditModes().isCurrentSupermodeData() && AC->getTagProperty().isDataElement()) {
                insert = true;
            }
            if (insert) {
                result.push_back(AC);
            }
        }
    }
    return result;
}


NBTrafficLightLogicCont&
GNENet::getTLLogicCont() {
    return myNetBuilder->getTLLogicCont();
}


NBEdgeCont&
GNENet::getEdgeCont() {
    return myNetBuilder->getEdgeCont();
}


void
GNENet::addExplicitTurnaround(std::string id) {
    myExplicitTurnarounds.insert(id);
}


void
GNENet::removeExplicitTurnaround(std::string id) {
    myExplicitTurnarounds.erase(id);
}


GNEAdditional*
GNENet::retrieveAdditional(SumoXMLTag type, const std::string& id, bool hardFail) const {
    if ((myAttributeCarriers->getAdditionals().count(type) > 0) && (myAttributeCarriers->getAdditionals().at(type).count(id) != 0)) {
        return myAttributeCarriers->getAdditionals().at(type).at(id);
    } else if (hardFail) {
        throw ProcessError("Attempted to retrieve non-existant additional");
    } else {
        return nullptr;
    }
}


std::vector<GNEAdditional*>
GNENet::retrieveAdditionals(bool onlySelected) const {
    std::vector<GNEAdditional*> result;
    // returns additionals depending of selection
    for (const auto& additionalTag : myAttributeCarriers->getAdditionals()) {
        for (const auto& additional : additionalTag.second) {
            if (!onlySelected || additional.second->isAttributeCarrierSelected()) {
                result.push_back(additional.second);
            }
            // iterate over children
            for (const auto& additionalChild : additional.second->getChildAdditionals()) {
                if (!onlySelected || additionalChild->isAttributeCarrierSelected()) {
                    result.push_back(additionalChild);
                }
            }
        }
    }
    return result;
}


int
GNENet::getNumberOfAdditionals(SumoXMLTag type) const {
    int counter = 0;
    for (auto i : myAttributeCarriers->getAdditionals()) {
        if ((type == SUMO_TAG_NOTHING) || (type == i.first)) {
            counter += (int)i.second.size();
        }
    }
    return counter;
}


void
GNENet::requireSaveAdditionals(bool value) {
    if (myAdditionalsSaved) {
        WRITE_DEBUG("Additionals has to be saved");
        std::string netSaved = (myNetSaved ? "saved" : "unsaved");
        std::string demandElementsSaved = (myDemandElementsSaved ? "saved" : "unsaved");
        std::string dataSetSaved = (myDataElementsSaved ? "saved" : "unsaved");
        WRITE_DEBUG("Current saving Status: net " + netSaved + ", additionals unsaved, demand elements " +
                    demandElementsSaved + ", data sets " + dataSetSaved);
    }
    myAdditionalsSaved = !value;
    if (myViewNet != nullptr) {
        if (myAdditionalsSaved) {
            myViewNet->getViewParent()->getGNEAppWindows()->disableSaveAdditionalsMenu();
        } else {
            myViewNet->getViewParent()->getGNEAppWindows()->enableSaveAdditionalsMenu();
        }
    }
}


void
GNENet::saveAdditionals(const std::string& filename) {
    // obtain invalid additionals depending of number of their parent lanes
    std::vector<GNEAdditional*> invalidSingleLaneAdditionals;
    std::vector<GNEAdditional*> invalidMultiLaneAdditionals;
    // iterate over additionals and obtain invalids
    for (const auto& additionalPair : myAttributeCarriers->getAdditionals()) {
        for (const auto& addditional : additionalPair.second) {
            // check if has to be fixed
            if (addditional.second->getTagProperty().hasAttribute(SUMO_ATTR_LANE) && !addditional.second->isAdditionalValid()) {
                invalidSingleLaneAdditionals.push_back(addditional.second);
            } else if (addditional.second->getTagProperty().hasAttribute(SUMO_ATTR_LANES) && !addditional.second->isAdditionalValid()) {
                invalidMultiLaneAdditionals.push_back(addditional.second);
            }
        }
    }
    // if there are invalid StoppingPlaces or detectors, open GNEFixAdditionalElements
    if (invalidSingleLaneAdditionals.size() > 0 || invalidMultiLaneAdditionals.size() > 0) {
        // 0 -> Canceled Saving, with or whithout selecting invalid stopping places and E2
        // 1 -> Invalid stoppingPlaces and E2 fixed, friendlyPos enabled, or saved with invalid positions
        GNEFixAdditionalElements fixAdditionalElementsDialog(myViewNet, invalidSingleLaneAdditionals, invalidMultiLaneAdditionals);
        if (fixAdditionalElementsDialog.execute() == 0) {
            // show debug information
            WRITE_DEBUG("Additionals saving aborted");
        } else {
            saveAdditionalsConfirmed(filename);
            // change value of flag
            myAdditionalsSaved = true;
            // show debug information
            WRITE_DEBUG("Additionals saved after dialog");
        }
        // update view
        myViewNet->updateViewNet();
        // set focus again in net
        myViewNet->setFocus();
    } else {
        saveAdditionalsConfirmed(filename);
        // change value of flag
        myAdditionalsSaved = true;
        // show debug information
        WRITE_DEBUG("Additionals saved");
    }
}


bool
GNENet::isAdditionalsSaved() const {
    return myAdditionalsSaved;
}


std::string
GNENet::generateAdditionalID(SumoXMLTag type) const {
    int counter = 0;
    while (myAttributeCarriers->getAdditionals().at(type).count(toString(type) + "_" + toString(counter)) != 0) {
        counter++;
    }
    return (toString(type) + "_" + toString(counter));
}


GNEDemandElement*
GNENet::retrieveDemandElement(SumoXMLTag type, const std::string& id, bool hardFail) const {
    if ((myAttributeCarriers->getDemandElements().count(type) > 0) && (myAttributeCarriers->getDemandElements().at(type).count(id) != 0)) {
        return myAttributeCarriers->getDemandElements().at(type).at(id);
    } else if (hardFail) {
        throw ProcessError("Attempted to retrieve non-existant demand element");
    } else {
        return nullptr;
    }
}


std::vector<GNEDemandElement*>
GNENet::retrieveDemandElements(bool onlySelected) const {
    std::vector<GNEDemandElement*> result;
    // returns demand elements depending of selection
    for (auto i : myAttributeCarriers->getDemandElements()) {
        for (auto j : i.second) {
            if (!onlySelected || j.second->isAttributeCarrierSelected()) {
                result.push_back(j.second);
            }
        }
    }
    return result;
}


int
GNENet::getNumberOfDemandElements(SumoXMLTag type) const {
    int counter = 0;
    for (auto i : myAttributeCarriers->getDemandElements()) {
        if ((type == SUMO_TAG_NOTHING) || (type == i.first)) {
            counter += (int)i.second.size();
        }
    }
    return counter;
}


void
GNENet::requireSaveDemandElements(bool value) {
    if (myDemandElementsSaved == true) {
        WRITE_DEBUG("DemandElements has to be saved");
        std::string netSaved = (myNetSaved ? "saved" : "unsaved");
        std::string additionalsSaved = (myAdditionalsSaved ? "saved" : "unsaved");
        std::string dataSetsSaved = (myDemandElementsSaved ? "saved" : "unsaved");
        WRITE_DEBUG("Current saving Status: net " + netSaved + ", additionals " + additionalsSaved +
                    ", demand elements unsaved, data sets " + dataSetsSaved);
    }
    myDemandElementsSaved = !value;
    if (myViewNet != nullptr) {
        if (myDemandElementsSaved) {
            myViewNet->getViewParent()->getGNEAppWindows()->disableSaveDemandElementsMenu();
        } else {
            myViewNet->getViewParent()->getGNEAppWindows()->enableSaveDemandElementsMenu();
        }
    }
}


void
GNENet::saveDemandElements(const std::string& filename) {
    // first recompute demand elements
    computeDemandElements(myViewNet->getViewParent()->getGNEAppWindows());
    // obtain invalid demandElements depending of number of their parent lanes
    std::vector<GNEDemandElement*> invalidSingleLaneDemandElements;
    // iterate over demandElements and obtain invalids
    for (const auto& demandElementSet : myAttributeCarriers->getDemandElements()) {
        for (const auto& demandElement : demandElementSet.second) {
            // compute before check if demand element is valid
            demandElement.second->computePath();
            // check if has to be fixed
            if (!demandElement.second->isDemandElementValid()) {
                invalidSingleLaneDemandElements.push_back(demandElement.second);
            }
        }
    }
    // if there are invalid demand elements, open GNEFixDemandElements
    if (invalidSingleLaneDemandElements.size() > 0) {
        // 0 -> Canceled Saving, with or whithout selecting invalid demand elements
        // 1 -> Invalid demand elements fixed, friendlyPos enabled, or saved with invalid positions
        GNEFixDemandElements fixDemandElementsDialog(myViewNet, invalidSingleLaneDemandElements);
        if (fixDemandElementsDialog.execute() == 0) {
            // show debug information
            WRITE_DEBUG("demand elements saving aborted");
        } else {
            saveDemandElementsConfirmed(filename);
            // change value of flag
            myDemandElementsSaved = true;
            // show debug information
            WRITE_DEBUG("demand elements saved after dialog");
        }
        // update view
        myViewNet->updateViewNet();
        // set focus again in net
        myViewNet->setFocus();
    } else {
        saveDemandElementsConfirmed(filename);
        // change value of flag
        myDemandElementsSaved = true;
        // show debug information
        WRITE_DEBUG("demand elements saved");
    }
}


bool
GNENet::isDemandElementsSaved() const {
    return myDemandElementsSaved;
}


std::string
GNENet::generateDemandElementID(SumoXMLTag tag) const {
    // get references to vehicle maps
    const std::map<std::string, GNEDemandElement*>& vehicles = myAttributeCarriers->getDemandElements().at(SUMO_TAG_VEHICLE);
    const std::map<std::string, GNEDemandElement*>& trips = myAttributeCarriers->getDemandElements().at(SUMO_TAG_TRIP);
    const std::map<std::string, GNEDemandElement*>& vehiclesEmbebbed = myAttributeCarriers->getDemandElements().at(GNE_TAG_VEHICLE_WITHROUTE);
    const std::map<std::string, GNEDemandElement*>& routeFlows = myAttributeCarriers->getDemandElements().at(GNE_TAG_FLOW_ROUTE);
    const std::map<std::string, GNEDemandElement*>& flows = myAttributeCarriers->getDemandElements().at(SUMO_TAG_FLOW);
    const std::map<std::string, GNEDemandElement*>& flowsEmbebbed = myAttributeCarriers->getDemandElements().at(GNE_TAG_FLOW_WITHROUTE);
    // get references to persons maps
    const std::map<std::string, GNEDemandElement*>& persons = myAttributeCarriers->getDemandElements().at(SUMO_TAG_PERSON);
    const std::map<std::string, GNEDemandElement*>& personFlows = myAttributeCarriers->getDemandElements().at(SUMO_TAG_PERSONFLOW);
    // declare flags
    const bool isVehicle = ((tag == SUMO_TAG_VEHICLE) || (tag == SUMO_TAG_TRIP) || (tag == GNE_TAG_VEHICLE_WITHROUTE));
    const bool isFlow = ((tag == GNE_TAG_FLOW_ROUTE) || (tag == SUMO_TAG_FLOW) || (tag == GNE_TAG_FLOW_WITHROUTE));
    const bool isPerson = ((tag == SUMO_TAG_PERSON) || (tag == SUMO_TAG_PERSONFLOW));
    // declare counter
    int counter = 0;
    if (isVehicle || isFlow) {
        // declare tag
        const std::string tagStr = isVehicle ? toString(SUMO_TAG_VEHICLE) : toString(SUMO_TAG_FLOW);
        // special case for vehicles (Vehicles, Flows, Trips and routeFlows share nameSpaces)
        while ((vehicles.count(tagStr + "_" + toString(counter)) != 0) ||
                (trips.count(tagStr + "_" + toString(counter)) != 0) ||
                (vehiclesEmbebbed.count(tagStr + "_" + toString(counter)) != 0) ||
                (routeFlows.count(tagStr + "_" + toString(counter)) != 0) ||
                (flows.count(tagStr + "_" + toString(counter)) != 0) ||
                (flowsEmbebbed.count(tagStr + "_" + toString(counter)) != 0) ||
                (vehicles.count(tagStr + "_" + toString(counter)) != 0)) {
            counter++;
        }
        // return new vehicle ID
        return (tagStr + "_" + toString(counter));
    } else if (isPerson) {
        // special case for persons (person and personFlows share nameSpaces)
        while ((persons.count(toString(tag) + "_" + toString(counter)) != 0) ||
                (personFlows.count(toString(tag) + "_" + toString(counter)) != 0)) {
            counter++;
        }
        // return new person ID
        return (toString(tag) + "_" + toString(counter));
    } else {
        while (myAttributeCarriers->getDemandElements().at(tag).count(toString(tag) + "_" + toString(counter)) != 0) {
            counter++;
        }
        // return new element ID
        return (toString(tag) + "_" + toString(counter));
    }
}


GNEDataSet*
GNENet::retrieveDataSet(const std::string& id, bool hardFail) const {
    if (myAttributeCarriers->getDataSets().count(id) > 0) {
        return myAttributeCarriers->getDataSets().at(id);
    } else if (hardFail) {
        throw ProcessError("Attempted to retrieve non-existant data set");
    } else {
        return nullptr;
    }
}


std::vector<GNEDataSet*>
GNENet::retrieveDataSets() const {
    std::vector<GNEDataSet*> result;
    result.reserve(myAttributeCarriers->getDataSets().size());
    // returns data sets
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        result.push_back(dataSet.second);
    }
    return result;
}


std::vector<GNEGenericData*>
GNENet::retrieveGenericDatas(bool onlySelected) const {
    std::vector<GNEGenericData*> result;
    size_t numGenericDatas = 0;
    // first reserve
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
            numGenericDatas += dataInterval.second->getGenericDataChildren().size();
        }
    }
    result.reserve(numGenericDatas);
    // returns generic datas depending of selection
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        for (const auto& dataInterval : dataSet.second->getDataIntervalChildren()) {
            for (const auto& genericData : dataInterval.second->getGenericDataChildren()) {
                if (!onlySelected || genericData->isAttributeCarrierSelected()) {
                    result.push_back(genericData);
                }
            }
        }
    }
    return result;
}


int
GNENet::getNumberOfDataSets() const {
    return (int)myAttributeCarriers->getDataSets().size();
}


void
GNENet::requireSaveDataElements(bool value) {
    if (myDataElementsSaved == true) {
        WRITE_DEBUG("DataSets has to be saved");
        std::string netSaved = (myNetSaved ? "saved" : "unsaved");
        std::string additionalsSaved = (myAdditionalsSaved ? "saved" : "unsaved");
        std::string demandEleementsSaved = (myDemandElementsSaved ? "saved" : "unsaved");
        WRITE_DEBUG("Current saving Status: net " + netSaved + ", additionals " + additionalsSaved +
                    ", demand elements " + demandEleementsSaved + ", data sets unsaved");
    }
    myDataElementsSaved = !value;
    if (myViewNet != nullptr) {
        if (myDataElementsSaved) {
            myViewNet->getViewParent()->getGNEAppWindows()->disableSaveDataElementsMenu();
        } else {
            myViewNet->getViewParent()->getGNEAppWindows()->enableSaveDataElementsMenu();
        }
    }
}


void
GNENet::saveDataElements(const std::string& filename) {
    // first recompute data sets
    computeDataElements(myViewNet->getViewParent()->getGNEAppWindows());
    // save data elements
    saveDataElementsConfirmed(filename);
    // change value of flag
    myDataElementsSaved = true;
    // show debug information
    WRITE_DEBUG("data sets saved");
}


bool
GNENet::isDataElementsSaved() const {
    return myDataElementsSaved;
}


std::string
GNENet::generateDataSetID(const std::string& prefix) const {
    const std::string dataSetTagStr = toString(SUMO_TAG_DATASET);
    int counter = 0;
    while (myAttributeCarriers->getDataSets().count(prefix + dataSetTagStr + "_" + toString(counter)) != 0) {
        counter++;
    }
    return (prefix + dataSetTagStr + "_" + toString(counter));
}


std::set<std::string>
GNENet::retrieveGenericDataParameters(const std::string& genericDataTag, const double begin, const double end) const {
    // declare solution
    std::set<std::string> attributesSolution;
    // declare generic data vector
    std::vector<GNEGenericData*> genericDatas;
    // iterate over all data sets
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        for (const auto& interval : dataSet.second->getDataIntervalChildren()) {
            // check interval
            if ((interval.second->getAttributeDouble(SUMO_ATTR_BEGIN) >= begin) && (interval.second->getAttributeDouble(SUMO_ATTR_END) <= end)) {
                // iterate over generic datas
                for (const auto& genericData : interval.second->getGenericDataChildren()) {
                    if (genericDataTag.empty() || (genericData->getTagProperty().getTagStr() == genericDataTag)) {
                        genericDatas.push_back(genericData);
                    }
                }
            }
        }
    }
    // iterate over generic datas
    for (const auto& genericData : genericDatas) {
        for (const auto& attribute : genericData->getParametersMap()) {
            attributesSolution.insert(attribute.first);
        }
    }
    return attributesSolution;
}


std::set<std::string>
GNENet::retrieveGenericDataParameters(const std::string& dataSetID, const std::string& genericDataTag,
                                      const std::string& beginStr, const std::string& endStr) const {
    // declare solution
    std::set<std::string> attributesSolution;
    // vector of data sets and intervals
    std::vector<GNEDataSet*> dataSets;
    std::vector<GNEDataInterval*> dataIntervals;
    // if dataSetID is empty, return all parameters
    if (dataSetID.empty()) {
        // add all data sets
        dataSets.reserve(myAttributeCarriers->getDataSets().size());
        for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
            dataSets.push_back(dataSet.second);
        }
    } else if (myAttributeCarriers->getDataSets().count(dataSetID) > 0) {
        dataSets.push_back(myAttributeCarriers->getDataSets().at(dataSetID));
    } else {
        return attributesSolution;
    }
    // now continue with data intervals
    int numberOfIntervals = 0;
    for (const auto& dataSet : dataSets) {
        numberOfIntervals += (int)dataSet->getDataIntervalChildren().size();
    }
    // resize dataIntervals
    dataIntervals.reserve(numberOfIntervals);
    // add intervals
    for (const auto& dataSet : dataSets) {
        for (const auto& dataInterval : dataSet->getDataIntervalChildren()) {
            // continue depending of begin and end
            if (beginStr.empty() && endStr.empty()) {
                dataIntervals.push_back(dataInterval.second);
            } else if (endStr.empty()) {
                // parse begin
                const double begin = GNEAttributeCarrier::parse<double>(beginStr);
                if (dataInterval.second->getAttributeDouble(SUMO_ATTR_BEGIN) >= begin) {
                    dataIntervals.push_back(dataInterval.second);
                }
            } else if (beginStr.empty()) {
                // parse end
                const double end = GNEAttributeCarrier::parse<double>(endStr);
                if (dataInterval.second->getAttributeDouble(SUMO_ATTR_END) <= end) {
                    dataIntervals.push_back(dataInterval.second);
                }
            } else {
                // parse both begin end
                const double begin = GNEAttributeCarrier::parse<double>(beginStr);
                const double end = GNEAttributeCarrier::parse<double>(endStr);
                if ((dataInterval.second->getAttributeDouble(SUMO_ATTR_BEGIN) >= begin) &&
                        (dataInterval.second->getAttributeDouble(SUMO_ATTR_END) <= end)) {
                    dataIntervals.push_back(dataInterval.second);
                }
            }
        }
    }
    // finally iterate over intervals and get attributes
    for (const auto& dataInterval : dataIntervals) {
        for (const auto& genericData : dataInterval->getGenericDataChildren()) {
            // check generic data tag
            if (genericDataTag.empty() || (genericData->getTagProperty().getTagStr() == genericDataTag)) {
                for (const auto& attribute : genericData->getParametersMap()) {
                    attributesSolution.insert(attribute.first);
                }
            }
        }
    }
    return attributesSolution;
}


double
GNENet::getDataSetIntervalMinimumBegin() const {
    double minimumBegin = 0;
    // update with first minimum (if exist)
    if ((myAttributeCarriers->getDataSets().size() > 0) && (myAttributeCarriers->getDataSets().begin()->second->getDataIntervalChildren().size() > 0)) {
        minimumBegin = myAttributeCarriers->getDataSets().begin()->second->getDataIntervalChildren().begin()->second->getAttributeDouble(SUMO_ATTR_BEGIN);
    }
    // iterate over all data sets
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        // iterate over interval
        for (const auto& interval : dataSet.second->getDataIntervalChildren()) {
            if (interval.second->getAttributeDouble(SUMO_ATTR_BEGIN) < minimumBegin) {
                minimumBegin = interval.second->getAttributeDouble(SUMO_ATTR_BEGIN);
            }
        }
    }
    return minimumBegin;
}


double
GNENet::getDataSetIntervalMaximumEnd() const {
    double maximumEnd = 0;
    // update with first maximum (if exist)
    if ((myAttributeCarriers->getDataSets().size() > 0) && (myAttributeCarriers->getDataSets().begin()->second->getDataIntervalChildren().size() > 0)) {
        maximumEnd = myAttributeCarriers->getDataSets().begin()->second->getDataIntervalChildren().begin()->second->getAttributeDouble(SUMO_ATTR_END);
    }
    // iterate over all data sets
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        // iterate over interval
        for (const auto& interval : dataSet.second->getDataIntervalChildren()) {
            if (interval.second->getAttributeDouble(SUMO_ATTR_END) > maximumEnd) {
                maximumEnd = interval.second->getAttributeDouble(SUMO_ATTR_END);
            }
        }
    }
    return maximumEnd;
}


void
GNENet::saveAdditionalsConfirmed(const std::string& filename) {
    OutputDevice& device = OutputDevice::getDevice(filename);
    device.writeXMLHeader("additional", "additional_file.xsd");
    // now write all route probes (see Ticket #4058)
    for (const auto& additionalPair : myAttributeCarriers->getAdditionals()) {
        if (additionalPair.first == SUMO_TAG_ROUTEPROBE) {
            for (const auto& additional : additionalPair.second) {
                additional.second->writeAdditional(device);
            }
        }
    }
    // now write all stoppingPlaces
    for (const auto& additionalPair : myAttributeCarriers->getAdditionals()) {
        if (GNEAttributeCarrier::getTagProperties(additionalPair.first).isStoppingPlace()) {
            for (const auto& additional : additionalPair.second) {
                // only save stoppingPlaces that doesn't have Additional parents, because they are automatically writed by writeAdditional(...) parent's function
                if (additional.second->getParentAdditionals().empty()) {
                    additional.second->writeAdditional(device);
                }
            }
        }
    }
    // now write all detectors
    for (const auto& additionalPair : myAttributeCarriers->getAdditionals()) {
        if (GNEAttributeCarrier::getTagProperties(additionalPair.first).isDetector()) {
            for (const auto& additional : additionalPair.second) {
                // only save Detectors that doesn't have Additional parents, because they are automatically writed by writeAdditional(...) parent's function
                if (additional.second->getParentAdditionals().empty()) {
                    additional.second->writeAdditional(device);
                }
            }
        }
    }
    // now write rest of additionals
    for (const auto& additionalPair : myAttributeCarriers->getAdditionals()) {
        const auto& tagValue = GNEAttributeCarrier::getTagProperties(additionalPair.first);
        if (!tagValue.isStoppingPlace() && !tagValue.isDetector() && (additionalPair.first != SUMO_TAG_ROUTEPROBE) && (additionalPair.first != SUMO_TAG_VTYPE) && (additionalPair.first != SUMO_TAG_ROUTE)) {
            for (const auto& additional : additionalPair.second) {
                // only save additionals that doesn't have Additional parents, because they are automatically writed by writeAdditional(...) parent's function
                if (additional.second->getParentAdditionals().empty()) {
                    additional.second->writeAdditional(device);
                }
            }
        }
    }
    // write TAZs
    for (const auto& TAZ : myAttributeCarriers->getTAZElements().at(SUMO_TAG_TAZ)) {
        TAZ.second->writeTAZElement(device);
    }
    // write Polygons
    for (const auto& poly : myAttributeCarriers->getShapes().at(SUMO_TAG_POLY)) {
        poly.second->writeShape(device);
    }
    // write POIs
    for (const auto& shape : myAttributeCarriers->getShapes()) {
        for (const auto& POI : shape.second) {
            if (POI.second->getTagProperty().getTag() != SUMO_TAG_POLY) {
                POI.second->writeShape(device);
            }
        }
    }
    device.close();
}


void
GNENet::saveDemandElementsConfirmed(const std::string& filename) {
    OutputDevice& device = OutputDevice::getDevice(filename);
    device.writeXMLHeader("routes", "routes_file.xsd");
    // first  write all vehicle types
    for (auto i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_VTYPE)) {
        i.second->writeDemandElement(device);
    }
    // first  write all person types
    for (auto i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_PTYPE)) {
        i.second->writeDemandElement(device);
    }
    // now write all routes (and their associated stops)
    for (auto i : myAttributeCarriers->getDemandElements().at(SUMO_TAG_ROUTE)) {
        i.second->writeDemandElement(device);
    }
    // sort vehicles/persons by depart
    std::map<double, std::vector<GNEDemandElement*> > vehiclesSortedByDepart;
    for (const auto& demandElementTag : myAttributeCarriers->getDemandElements()) {
        for (const auto& demandElement : demandElementTag.second) {
            if (demandElement.second->getTagProperty().isPerson() || demandElement.second->getTagProperty().isVehicle()) {
                // save it in myVehiclesSortedByDepart
                vehiclesSortedByDepart[GNEAttributeCarrier::parse<double>(demandElement.second->getBegin())].push_back(demandElement.second);
            }
        }
    }
    // finally write all vehicles and persons sorted by depart time (and their associated stops, personPlans, etc.)
    for (const auto& vehicleTag : vehiclesSortedByDepart) {
        for (const auto& vehicle : vehicleTag.second) {
            vehicle->writeDemandElement(device);
        }
    }
    device.close();
}


void
GNENet::saveDataElementsConfirmed(const std::string& filename) {
    OutputDevice& device = OutputDevice::getDevice(filename);
    device.writeXMLHeader("meandata", "meandata_file.xsd");
    // write all data sets
    for (const auto& dataSet : myAttributeCarriers->getDataSets()) {
        dataSet.second->writeDataSet(device);
    }
    // close device
    device.close();
}


GNEShape*
GNENet::retrieveShape(SumoXMLTag type, const std::string& id, bool hardFail) const {
    if ((myAttributeCarriers->getShapes().count(type) > 0) && (myAttributeCarriers->getShapes().at(type).count(id) != 0)) {
        return myAttributeCarriers->getShapes().at(type).at(id);
    } else if (hardFail) {
        throw ProcessError("Attempted to retrieve non-existant shape");
    } else {
        return nullptr;
    }
}


std::vector<GNEShape*>
GNENet::retrieveShapes(bool onlySelected) const {
    std::vector<GNEShape*> result;
    // returns shapes depending of selection
    for (auto i : myAttributeCarriers->getShapes()) {
        for (auto j : i.second) {
            if (!onlySelected || j.second->isAttributeCarrierSelected()) {
                result.push_back(j.second);
            }
        }
    }
    return result;
}


std::string
GNENet::generateShapeID(SumoXMLTag tag) const {
    int counter = 0;
    // generate tag depending of shape type
    if (tag == SUMO_TAG_POLY) {
        while (myAttributeCarriers->getShapes().at(tag).count(toString(tag) + "_" + toString(counter)) != 0) {
            counter++;
        }
        return (toString(tag) + "_" + toString(counter));
    } else {
        while (myAttributeCarriers->getShapes().at(tag).count(toString(SUMO_TAG_POI) + "_" + toString(counter)) != 0) {
            counter++;
        }
        return (toString(SUMO_TAG_POI) + "_" + toString(counter));
    }
}


int
GNENet::getNumberOfShapes(SumoXMLTag type) const {
    int counter = 0;
    for (auto i : myAttributeCarriers->getShapes()) {
        if ((type == SUMO_TAG_NOTHING) || (type == i.first)) {
            counter += (int)i.second.size();
        }
    }
    return counter;
}


GNETAZElement*
GNENet::retrieveTAZElement(SumoXMLTag type, const std::string& id, bool hardFail) const {
    if ((myAttributeCarriers->getTAZElements().count(type) > 0) && (myAttributeCarriers->getTAZElements().at(type).count(id) != 0)) {
        return myAttributeCarriers->getTAZElements().at(type).at(id);
    } else if (hardFail) {
        throw ProcessError("Attempted to retrieve non-existant TAZElement");
    } else {
        return nullptr;
    }
}


std::vector<GNETAZElement*>
GNENet::retrieveTAZElements(bool onlySelected) const {
    std::vector<GNETAZElement*> result;
    // returns TAZElements depending of selection
    for (auto i : myAttributeCarriers->getTAZElements()) {
        for (auto j : i.second) {
            if (!onlySelected || j.second->isAttributeCarrierSelected()) {
                result.push_back(j.second);
            }
        }
    }
    return result;
}


std::string
GNENet::generateTAZElementID(SumoXMLTag tag) const {
    int counter = 0;
    while (myAttributeCarriers->getTAZElements().at(tag).count(toString(tag) + "_" + toString(counter)) != 0) {
        counter++;
    }
    return (toString(tag) + "_" + toString(counter));
}


int
GNENet::getNumberOfTAZElements(SumoXMLTag type) const {
    int counter = 0;
    for (auto i : myAttributeCarriers->getTAZElements()) {
        if ((type == SUMO_TAG_NOTHING) || (type == i.first)) {
            counter += (int)i.second.size();
        }
    }
    return counter;
}


void
GNENet::requireSaveTLSPrograms() {
    if (myTLSProgramsSaved == true) {
        WRITE_DEBUG("TLSPrograms has to be saved");
    }
    myTLSProgramsSaved = false;
    myViewNet->getViewParent()->getGNEAppWindows()->enableSaveTLSProgramsMenu();
}


void
GNENet::saveTLSPrograms(const std::string& filename) {
    // open output device
    OutputDevice& device = OutputDevice::getDevice(filename);
    device.openTag("additionals");
    // write traffic lights using NWWriter
    NWWriter_SUMO::writeTrafficLights(device, getTLLogicCont());
    device.close();
    // change flag to true
    myTLSProgramsSaved = true;
    // show debug information
    WRITE_DEBUG("TLSPrograms saved");
}


int
GNENet::getNumberOfTLSPrograms() const {
    return -1;
}

void
GNENet::enableUpdateGeometry() {
    myUpdateGeometryEnabled = true;
}


void
GNENet::disableUpdateGeometry() {
    myUpdateGeometryEnabled = false;
}


bool
GNENet::isUpdateGeometryEnabled() const {
    return myUpdateGeometryEnabled;
}

// ===========================================================================
// private
// ===========================================================================

void
GNENet::initJunctionsAndEdges() {
    // init junctions (by default Crossing and walking areas aren't created)
    NBNodeCont& nodeContainer = myNetBuilder->getNodeCont();
    for (auto name_it : nodeContainer.getAllNames()) {
        NBNode* nbn = nodeContainer.retrieve(name_it);
        myAttributeCarriers->registerJunction(new GNEJunction(this, nbn, true));
    }
    // init edges
    NBEdgeCont& ec = myNetBuilder->getEdgeCont();
    for (auto name_it : ec.getAllNames()) {
        // create edge using NBEdge
        GNEEdge* edge = new GNEEdge(this, ec.retrieve(name_it), false, true);
        // register edge
        myAttributeCarriers->registerEdge(edge);
        // add manually child references due initJunctionsAndEdges doesn't use undo-redo
        edge->getParentJunctions().front()->addChildElement(edge);
        edge->getParentJunctions().back()->addChildElement(edge);
        // check grid
        if (myGrid.getWidth() > 10e16 || myGrid.getHeight() > 10e16) {
            throw ProcessError("Network size exceeds 1 Lightyear. Please reconsider your inputs.\n");
        }
    }
    // make sure myGrid is initialized even for an empty net
    if (myAttributeCarriers->getEdges().size() == 0) {
        myGrid.add(Boundary(0, 0, 100, 100));
    }
    // recalculate all lane2lane connections
    for (const auto& edge : myAttributeCarriers->getEdges()) {
        for (const auto& lane : edge.second->getLanes()) {
            lane->updateGeometry();
        }
    }
    // sort nodes edges so that arrows can be drawn correctly
    NBNodesEdgesSorter::sortNodesEdges(nodeContainer);
}


void
GNENet::reserveEdgeID(const std::string& id) {
    myEdgeIDSupplier.avoid(id);
}


void
GNENet::reserveJunctionID(const std::string& id) {
    myJunctionIDSupplier.avoid(id);
}


void
GNENet::initGNEConnections() {
    for (const auto& edge : myAttributeCarriers->getEdges()) {
        // remake connections
        edge.second->remakeGNEConnections();
        // update geometry of connections
        for (const auto& connection : edge.second->getGNEConnections()) {
            connection->updateGeometry();
        }
    }
}


void
GNENet::computeAndUpdate(OptionsCont& oc, bool volatileOptions) {
    // make sure we only add turn arounds to edges which currently exist within the network
    std::set<std::string> liveExplicitTurnarounds;
    for (const auto& explicitTurnarounds : myExplicitTurnarounds) {
        if (myAttributeCarriers->getEdges().count(explicitTurnarounds) > 0) {
            liveExplicitTurnarounds.insert(explicitTurnarounds);
        }
    }
    // removes all junctions of grid
    WRITE_GLDEBUG("Removing junctions during recomputing");
    for (const auto& it : myAttributeCarriers->getJunctions()) {
        myGrid.removeAdditionalGLObject(it.second);
    }
    // remove all edges from grid
    WRITE_GLDEBUG("Removing edges during recomputing");
    for (const auto& it : myAttributeCarriers->getEdges()) {
        myGrid.removeAdditionalGLObject(it.second);
    }
    // compute using NetBuilder
    myNetBuilder->compute(oc, liveExplicitTurnarounds, volatileOptions);
    // remap ids if necessary
    if (oc.getBool("numerical-ids") || oc.isSet("reserved-ids")) {
        myAttributeCarriers->remapJunctionAndEdgeIds();
    }
    // update rtree if necessary
    if (!oc.getBool("offset.disable-normalization")) {
        for (const auto& edge : myAttributeCarriers->getEdges()) {
            // refresh edge geometry
            edge.second->updateGeometry();
        }
    }
    // Clear current inspected ACs in inspectorFrame if a previous net was loaded
    if (myViewNet != nullptr) {
        myViewNet->getViewParent()->getInspectorFrame()->clearInspectedAC();
    }
    // Reset Grid
    myGrid.reset();
    myGrid.add(GeoConvHelper::getFinal().getConvBoundary());
    // if volatile options are true
    if (volatileOptions) {
        // check that net exist
        if (myViewNet == nullptr) {
            throw ProcessError("ViewNet doesn't exist");
        }
        // disable update geometry before clear undo list
        myUpdateGeometryEnabled = false;
        // clear undo list (This will be remove additionals and shapes)
        myViewNet->getUndoList()->p_clear();
        // clear all elements (it will also removed from grid)
        myAttributeCarriers->clearJunctions();
        myAttributeCarriers->clearEdges();
        myAttributeCarriers->clearAdditionals();
        myAttributeCarriers->clearShapes();
        myAttributeCarriers->clearTAZElements();
        myAttributeCarriers->clearDemandElements();
        // enable update geometry again
        myUpdateGeometryEnabled = true;
        // Write GL debug information
        WRITE_GLDEBUG("initJunctionsAndEdges function called in computeAndUpdate(...) due recomputing with volatile options");
        // init again junction an edges (Additionals and shapes will be loaded after the end of this function)
        initJunctionsAndEdges();
    } else {
        // insert all junctions of grid again
        WRITE_GLDEBUG("Add junctions during recomputing after calling myNetBuilder->compute(...)");
        for (const auto& junction : myAttributeCarriers->getJunctions()) {
            myGrid.addAdditionalGLObject(junction.second);
        }
        // insert all edges from grid again
        WRITE_GLDEBUG("Add edges during recomputing after calling myNetBuilder->compute(...)");
        for (const auto& edge : myAttributeCarriers->getEdges()) {
            myGrid.addAdditionalGLObject(edge.second);
        }
        // remake connections
        for (const auto& connection : myAttributeCarriers->getEdges()) {
            connection.second->remakeGNEConnections();
        }
        // iterate over junctions of net
        for (const auto& junction : myAttributeCarriers->getJunctions()) {
            // undolist may not yet exist but is also not needed when just marking junctions as valid
            junction.second->setLogicValid(true, nullptr);
            // updated geometry
            junction.second->updateGeometryAfterNetbuild();
        }
        // iterate over all edges of net
        for (const auto& edge : myAttributeCarriers->getEdges()) {
            // update geometry
            edge.second->updateGeometry();
        }
    }
    // net recomputed, then return false;
    myNeedRecompute = false;
}


void
GNENet::replaceInListAttribute(GNEAttributeCarrier* ac, SumoXMLAttr key, const std::string& which, const std::string& by, GNEUndoList* undoList) {
    assert(ac->getTagProperty().getAttributeProperties(key).isList());
    std::vector<std::string> values = GNEAttributeCarrier::parse<std::vector<std::string> >(ac->getAttribute(key));
    std::vector<std::string> newValues;
    for (auto v : values) {
        newValues.push_back(v == which ? by : v);
    }
    ac->setAttribute(key, toString(newValues), undoList);
}


/****************************************************************************/
