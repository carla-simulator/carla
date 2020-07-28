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
/// @file    NIXMLNodesHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Importer for network nodes stored in XML
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/StringTokenizer.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeoConvHelper.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include <netbuild/NBOwnTLDef.h>
#include <netbuild/NBNetBuilder.h>
#include "NIXMLNodesHandler.h"
#include "NIImporter_SUMO.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIXMLNodesHandler::NIXMLNodesHandler(NBNodeCont& nc, NBEdgeCont& ec,
                                     NBTrafficLightLogicCont& tlc,
                                     OptionsCont& options) :
    SUMOSAXHandler("xml-nodes - file"),
    myOptions(options),
    myNodeCont(nc),
    myEdgeCont(ec),
    myTLLogicCont(tlc),
    myLocation(nullptr),
    myLastParameterised(nullptr) {
}


NIXMLNodesHandler::~NIXMLNodesHandler() {
    delete myLocation;
}


void
NIXMLNodesHandler::myStartElement(int element,
                                  const SUMOSAXAttributes& attrs) {
    switch (element) {
        case SUMO_TAG_LOCATION:
            myLocation = NIImporter_SUMO::loadLocation(attrs);
            break;
        case SUMO_TAG_NODE:
            addNode(attrs);
            break;
        case SUMO_TAG_JOIN:
            addJoinCluster(attrs);
            break;
        case SUMO_TAG_JOINEXCLUDE:
            addJoinExclusion(attrs);
            break;
        case SUMO_TAG_DEL:
            deleteNode(attrs);
            break;
        case SUMO_TAG_PARAM:
            if (myLastParameterised != nullptr) {
                bool ok = true;
                const std::string key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                // circumventing empty string test
                const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
                myLastParameterised->setParameter(key, val);
            }
        default:
            break;
    }
}


void
NIXMLNodesHandler::myEndElement(int element) {
    switch (element) {
        case SUMO_TAG_NODE:
            myLastParameterised = nullptr;
            break;
        default:
            break;
    }
}


void
NIXMLNodesHandler::addNode(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    // get the id, report a warning if not given or empty...
    myID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        return;
    }
    NBNode* node = myNodeCont.retrieve(myID);
    // retrieve the position of the node
    bool xOk = false;
    bool yOk = false;
    bool needConversion = true;
    if (node != nullptr) {
        myPosition = node->getPosition();
        xOk = yOk = true;
        needConversion = false;
    } else {
        myPosition.set(0, 0, 0); // better to reset than to reuse the previous (z)-value
    }
    if (attrs.hasAttribute(SUMO_ATTR_X)) {
        myPosition.set(attrs.get<double>(SUMO_ATTR_X, myID.c_str(), ok), myPosition.y());
        xOk = true;
        needConversion = true;
    }
    if (attrs.hasAttribute(SUMO_ATTR_Y)) {
        myPosition.set(myPosition.x(), attrs.get<double>(SUMO_ATTR_Y, myID.c_str(), ok));
        yOk = true;
        needConversion = true;
    }
    if (attrs.hasAttribute(SUMO_ATTR_Z)) {
        myPosition.set(myPosition.x(), myPosition.y(), attrs.get<double>(SUMO_ATTR_Z, myID.c_str(), ok));
    }
    if (xOk && yOk) {
        if (needConversion && !NBNetBuilder::transformCoordinate(myPosition, true, myLocation)) {
            WRITE_ERROR("Unable to project coordinates for node '" + myID + "'.");
        }
    } else {
        WRITE_ERROR("Missing position (at node ID='" + myID + "').");
    }
    bool updateEdgeGeometries = node != nullptr && myPosition != node->getPosition();
    node = processNodeType(attrs, node, myID, myPosition, updateEdgeGeometries, myNodeCont, myEdgeCont, myTLLogicCont);
    myLastParameterised = node;
}


NBNode*
NIXMLNodesHandler::processNodeType(const SUMOSAXAttributes& attrs, NBNode* node, const std::string& nodeID, const Position& position,
                                   bool updateEdgeGeometries,
                                   NBNodeCont& nc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    bool ok = true;
    // get the type
    SumoXMLNodeType type = SumoXMLNodeType::UNKNOWN;
    if (node != nullptr) {
        type = node->getType();
    }
    std::string typeS = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, nodeID.c_str(), ok, "");
    if (SUMOXMLDefinitions::NodeTypes.hasString(typeS)) {
        type = SUMOXMLDefinitions::NodeTypes.get(typeS);
        if (type == SumoXMLNodeType::DEAD_END_DEPRECATED || type == SumoXMLNodeType::DEAD_END) {
            // dead end is a computed status. Reset this to unknown so it will
            // be corrected if additional connections are loaded
            type = SumoXMLNodeType::UNKNOWN;
        }
    }
    std::set<NBTrafficLightDefinition*> oldTLS;
    // check whether a prior node shall be modified
    if (node == nullptr) {
        node = new NBNode(nodeID, position, type);
        if (!nc.insert(node)) {
            throw ProcessError("Could not insert node though checked this before (id='" + nodeID + "').");
        }
    } else {
        // patch information
        oldTLS = node->getControllingTLS();
        if (node->getType() == SumoXMLNodeType::PRIORITY && type == SumoXMLNodeType::RIGHT_BEFORE_LEFT) {
            ec.removeRoundabout(node);
        }
        node->reinit(position, type, updateEdgeGeometries);
    }
    // process traffic light definition
    if (NBNode::isTrafficLight(type)) {
        processTrafficLightDefinitions(attrs, node, tlc);
    }
    // remove previously set tls if this node is not controlled by them
    for (std::set<NBTrafficLightDefinition*>::iterator i = oldTLS.begin(); i != oldTLS.end(); ++i) {
        if ((*i)->getNodes().size() == 0) {
            tlc.removeFully((*i)->getID());
        }
    }

    // set optional shape
    PositionVector shape;
    if (attrs.hasAttribute(SUMO_ATTR_SHAPE)) {
        shape = attrs.getOpt<PositionVector>(SUMO_ATTR_SHAPE, nodeID.c_str(), ok, PositionVector());
        if (!NBNetBuilder::transformCoordinates(shape)) {
            WRITE_ERROR("Unable to project node shape at node '" + node->getID() + "'.");
        }
        if (shape.size() > 2) {
            shape.closePolygon();
        }
        node->setCustomShape(shape);
    }
    // set optional radius
    if (attrs.hasAttribute(SUMO_ATTR_RADIUS)) {
        node->setRadius(attrs.get<double>(SUMO_ATTR_RADIUS, nodeID.c_str(), ok));
    }
    // set optional keepClear flag
    if (attrs.hasAttribute(SUMO_ATTR_KEEP_CLEAR)) {
        node->setKeepClear(attrs.get<bool>(SUMO_ATTR_KEEP_CLEAR, nodeID.c_str(), ok));
    }

    // set optional right-of-way hint
    if (attrs.hasAttribute(SUMO_ATTR_RIGHT_OF_WAY)) {
        node->setRightOfWay(attrs.getRightOfWay(ok));
    }

    // set optional fringe type
    if (attrs.hasAttribute(SUMO_ATTR_FRINGE)) {
        node->setFringeType(attrs.getFringeType(ok));
    }
    // set optional name
    if (attrs.hasAttribute(SUMO_ATTR_NAME)) {
        node->setName(attrs.get<std::string>(SUMO_ATTR_NAME, nodeID.c_str(), ok));
    }
    return node;
}


void
NIXMLNodesHandler::deleteNode(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    // get the id, report a warning if not given or empty...
    myID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        return;
    }
    NBNode* node = myNodeCont.retrieve(myID);
    if (node == nullptr) {
        WRITE_WARNING("Ignoring tag '" + toString(SUMO_TAG_DEL) + "' for unknown node '" +
                      myID + "'");
        return;
    } else {
        myNodeCont.extract(node, true);
    }
}


void
NIXMLNodesHandler::addJoinCluster(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    const std::string clusterString = attrs.get<std::string>(SUMO_ATTR_NODES, nullptr, ok);
    std::vector<std::string> ids = StringTokenizer(clusterString).getVector();
    std::sort(ids.begin(), ids.end());

    myID = attrs.getOpt<std::string>(SUMO_ATTR_ID, nullptr, ok, "cluster_" + joinToString(ids, "_"));

    Position myPosition = Position::INVALID;
    if (attrs.hasAttribute(SUMO_ATTR_X)) {
        myPosition.setx(attrs.get<double>(SUMO_ATTR_X, myID.c_str(), ok));
    }
    if (attrs.hasAttribute(SUMO_ATTR_Y)) {
        myPosition.sety(attrs.get<double>(SUMO_ATTR_Y, myID.c_str(), ok));
    }
    if (attrs.hasAttribute(SUMO_ATTR_Z)) {
        myPosition.setz(attrs.get<double>(SUMO_ATTR_Z, myID.c_str(), ok));
    }

    NBNode* node = processNodeType(attrs, nullptr, myID, myPosition, false, myNodeCont, myEdgeCont, myTLLogicCont);
    if (ok) {
        myNodeCont.addCluster2Join(std::set<std::string>(ids.begin(), ids.end()), node);
    }
}


void
NIXMLNodesHandler::addJoinExclusion(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    const std::vector<std::string> ids = StringTokenizer(
            attrs.get<std::string>(SUMO_ATTR_NODES, nullptr, ok)).getVector();
    if (ok) {
        myNodeCont.addJoinExclusion(ids);
    }
}


void
NIXMLNodesHandler::processTrafficLightDefinitions(const SUMOSAXAttributes& attrs,
        NBNode* currentNode, NBTrafficLightLogicCont& tlc) {
    // try to get the tl-id
    // if a tl-id is given, we will look whether this tl already exists
    //  if so, we will add the node to it (and to all programs with this id), otherwise allocate a new one with this id
    // if no tl-id exists, we will build a tl with the node's id
    std::set<NBTrafficLightDefinition*> tlDefs;
    bool ok = true;

    std::string oldTlID = "";
    std::string oldTypeS = OptionsCont::getOptions().getString("tls.default-type");

    if (currentNode->isTLControlled()) {
        NBTrafficLightDefinition* oldDef = *(currentNode->getControllingTLS().begin());
        oldTlID = oldDef->getID();
        oldTypeS = toString(oldDef->getType());
    }
    std::string tlID = attrs.getOpt<std::string>(SUMO_ATTR_TLID, nullptr, ok, oldTlID);
    std::string typeS = attrs.getOpt<std::string>(SUMO_ATTR_TLTYPE, nullptr, ok, oldTypeS);
    if (tlID != oldTlID || typeS != oldTypeS) {
        currentNode->removeTrafficLights();
    }
    TrafficLightType type;
    if (SUMOXMLDefinitions::TrafficLightTypes.hasString(typeS)) {
        type = SUMOXMLDefinitions::TrafficLightTypes.get(typeS);
    } else {
        WRITE_ERROR("Unknown traffic light type '" + typeS + "' for node '" + currentNode->getID() + "'.");
        return;
    }
    if (tlID != "" && tlc.getPrograms(tlID).size() > 0) {
        // we already have definitions for this tlID
        const std::map<std::string, NBTrafficLightDefinition*>& programs = tlc.getPrograms(tlID);
        std::map<std::string, NBTrafficLightDefinition*>::const_iterator it;
        for (it = programs.begin(); it != programs.end(); it++) {
            if (it->second->getType() != type) {
                WRITE_ERROR("Mismatched traffic light type '" + typeS + "' for tl '" + tlID + "'.");
                ok = false;
            } else {
                tlDefs.insert(it->second);
                it->second->addNode(currentNode);
            }
        }
    } else {
        // we need to add a new defition
        tlID = (tlID == "" ? currentNode->getID() : tlID);
        NBTrafficLightDefinition* tlDef = new NBOwnTLDef(tlID, currentNode, 0, type);
        if (!tlc.insert(tlDef)) {
            // actually, nothing should fail here
            delete tlDef;
            throw ProcessError("Could not allocate tls '" + currentNode->getID() + "'.");
        }
        tlDefs.insert(tlDef);
    }
    // process inner edges which shall be controlled
    const std::vector<std::string>& controlledInner = attrs.getOptStringVector(SUMO_ATTR_CONTROLLED_INNER, nullptr, ok);
    if (controlledInner.size() != 0) {
        for (std::set<NBTrafficLightDefinition*>::iterator it = tlDefs.begin(); it != tlDefs.end(); it++) {
            (*it)->addControlledInnerEdges(controlledInner);
        }
    }
}


/****************************************************************************/
