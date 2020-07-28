/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIXMLTrafficLightsHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2011-10-05
///
// Importer for traffic lights stored in XML
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <utils/common/StringTokenizer.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBOwnTLDef.h>
#include <netbuild/NBLoadedSUMOTLDef.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include "NIImporter_SUMO.h"
#include "NIXMLTrafficLightsHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIXMLTrafficLightsHandler::NIXMLTrafficLightsHandler(
    NBTrafficLightLogicCont& tlCont, NBEdgeCont& ec, bool ignoreUnknown) :
    SUMOSAXHandler("xml-tllogics"),
    myTLLCont(tlCont),
    myEdgeCont(ec),
    myCurrentTL(nullptr),
    myResetPhases(false),
    myIgnoreUnknown(ignoreUnknown)
{ }


NIXMLTrafficLightsHandler::~NIXMLTrafficLightsHandler() {}


void
NIXMLTrafficLightsHandler::myStartElement(
    int element, const SUMOSAXAttributes& attrs) {
    switch (element) {
        case SUMO_TAG_TLLOGIC:
            myCurrentTL = initTrafficLightLogic(attrs, myCurrentTL);
            break;
        case SUMO_TAG_PHASE:
            if (myCurrentTL != nullptr) {
                if (myResetPhases) {
                    myCurrentTL->getLogic()->resetPhases();
                    myResetPhases = false;
                }
                NIImporter_SUMO::addPhase(attrs, myCurrentTL);
                myCurrentTL->phasesLoaded();
            }
            break;
        case SUMO_TAG_CONNECTION:
            addTlConnection(attrs);
            break;
        case SUMO_TAG_DEL:
            removeTlConnection(attrs);
            break;
        case SUMO_TAG_PARAM:
            if (myCurrentTL != nullptr) {
                bool ok = true;
                const std::string key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                // circumventing empty string test
                const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
                myCurrentTL->setParameter(key, val);
            }
        default:
            break;
    }
}


void
NIXMLTrafficLightsHandler::myEndElement(int element) {
    switch (element) {
        case SUMO_TAG_TLLOGIC:
            myCurrentTL = nullptr;
            break;
        default:
            break;
    }
}


NBLoadedSUMOTLDef*
NIXMLTrafficLightsHandler::initTrafficLightLogic(const SUMOSAXAttributes& attrs, NBLoadedSUMOTLDef* currentTL) {
    if (currentTL) {
        WRITE_ERROR("Definition of tlLogic '" + currentTL->getID() + "' was not finished.");
        return nullptr;
    }
    bool ok = true;
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    std::string programID = attrs.getOpt<std::string>(SUMO_ATTR_PROGRAMID, id.c_str(), ok, "UNKNOWN_PROGRAM");
    SUMOTime offset = attrs.hasAttribute(SUMO_ATTR_OFFSET) ? TIME2STEPS(attrs.get<double>(SUMO_ATTR_OFFSET, id.c_str(), ok)) : 0;
    std::string typeS = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, nullptr, ok,
                        OptionsCont::getOptions().getString("tls.default-type"));
    TrafficLightType type;
    if (SUMOXMLDefinitions::TrafficLightTypes.hasString(typeS)) {
        type = SUMOXMLDefinitions::TrafficLightTypes.get(typeS);
    } else {
        WRITE_ERROR("Unknown traffic light type '" + typeS + "' for tlLogic '" + id + "'.");
        return nullptr;
    }
    // there are three scenarios to consider
    // 1) the tll.xml is loaded to update traffic lights defined in a net.xml:
    //   simply retrieve the loaded definitions and update them
    // 2) the tll.xml is loaded to define new traffic lights
    //   nod.xml will have triggered building of NBOwnTLDef. Replace it with NBLoadedSUMOTLDef
    // 3) the tll.xml is loaded to define new programs for a defined traffic light
    //   there should be a definition with the same id but different programID
    const std::map<std::string, NBTrafficLightDefinition*>& programs = myTLLCont.getPrograms(id);
    if (programs.size() == 0) {
        if (!myIgnoreUnknown) {
            WRITE_ERROR("Cannot load traffic light program for unknown id '" + id + "', programID '" + programID + "'.");
        }
        return nullptr;
    }
    const std::string existingProgram = programs.begin()->first; // arbitrary for our purpose
    NBLoadedSUMOTLDef* loadedDef = dynamic_cast<NBLoadedSUMOTLDef*>(myTLLCont.getDefinition(id, programID));
    if (loadedDef == nullptr) {
        NBLoadedSUMOTLDef* oldDef = dynamic_cast<NBLoadedSUMOTLDef*>(myTLLCont.getDefinition(id, existingProgram));
        if (oldDef == nullptr) {
            // case 2
            NBTrafficLightDefinition* newDef = dynamic_cast<NBOwnTLDef*>(myTLLCont.getDefinition(
                                                   id, NBTrafficLightDefinition::DefaultProgramID));
            bool deleteDefault = false;
            if (newDef == nullptr) {
                // the default program may have already been replaced with a loaded program
                newDef = dynamic_cast<NBLoadedSUMOTLDef*>(myTLLCont.getDefinition(
                             id, NBTrafficLightDefinition::DefaultProgramID));
                if (newDef == nullptr) {
                    WRITE_ERROR("Cannot load traffic light program for unknown id '" + id + "', programID '" + programID + "'.");
                    return nullptr;
                }
            } else {
                deleteDefault = true;
            }
            assert(newDef != 0);
            loadedDef = new NBLoadedSUMOTLDef(id, programID, offset, type);
            // copy nodes and controlled inner edges
            std::vector<NBNode*> nodes = newDef->getNodes();
            for (std::vector<NBNode*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
                loadedDef->addNode(*it);
            }
            loadedDef->addControlledInnerEdges(newDef->getControlledInnerEdges());
            if (deleteDefault) {
                // make a copy because the vector is modified in the loop
                const std::vector<NBNode*> nodes = newDef->getNodes();
                // replace default Program
                for (NBNode* const n : nodes) {
                    n->removeTrafficLight(newDef);
                }
                myTLLCont.removeProgram(id, NBTrafficLightDefinition::DefaultProgramID);
            }
            myTLLCont.insert(loadedDef);
        } else {
            // case 3
            NBTrafficLightLogic* oldLogic = oldDef->getLogic();
            NBTrafficLightLogic newLogic(id, programID, oldLogic->getNumLinks(), offset, type);
            loadedDef = new NBLoadedSUMOTLDef(*oldDef, newLogic);
            // copy nodes
            std::vector<NBNode*> nodes = oldDef->getNodes();
            for (std::vector<NBNode*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
                loadedDef->addNode(*it);
            }
            //std::cout << " case3 oldDef=" << oldDef->getDescription() << " loadedDef=" << loadedDef->getDescription() << "\n";
            myTLLCont.insert(loadedDef);
        }
    } else {
        // case 1
        if (attrs.hasAttribute(SUMO_ATTR_OFFSET)) {
            loadedDef->setOffset(offset);
        }
        if (attrs.hasAttribute(SUMO_ATTR_TYPE)) {
            loadedDef->setType(type);
        }
    }
    if (ok) {
        myResetPhases = true;
        return loadedDef;
    } else {
        return nullptr;
    }
}


void
NIXMLTrafficLightsHandler::addTlConnection(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    // parse identifying attributes
    NBEdge* from = retrieveEdge(attrs, SUMO_ATTR_FROM, ok);
    NBEdge* to = retrieveEdge(attrs, SUMO_ATTR_TO, ok);
    if (!ok) {
        return;
    }
    int fromLane = retrieveLaneIndex(attrs, SUMO_ATTR_FROM_LANE, from, ok);
    int toLane = retrieveLaneIndex(attrs, SUMO_ATTR_TO_LANE, to, ok);
    if (!ok) {
        return;
    }
    // retrieve connection
    const std::vector<NBEdge::Connection>& connections = from->getConnections();
    std::vector<NBEdge::Connection>::const_iterator con_it;
    con_it = find_if(connections.begin(), connections.end(),
                     NBEdge::connections_finder(fromLane, to, toLane));
    if (con_it == connections.end()) {
        WRITE_ERROR("Connection from=" + from->getID() + " to=" + to->getID() +
                    " fromLane=" + toString(fromLane) + " toLane=" + toString(toLane) + " not found");
        return;
    }
    NBEdge::Connection c = *con_it;
    // read other  attributes
    std::string tlID = attrs.getOpt<std::string>(SUMO_ATTR_TLID, nullptr, ok, "");
    if (tlID == "") {
        // we are updating an existing tl-controlled connection
        tlID = (*(from->getToNode()->getControllingTLS().begin()))->getID();
        assert(tlID != "");
    }
    int tlIndex = attrs.getOpt<int>(SUMO_ATTR_TLLINKINDEX, nullptr, ok, -1);
    if (tlIndex == -1) {
        // we are updating an existing tl-controlled connection
        tlIndex = c.tlLinkIndex;
    }
    int tlIndex2 = attrs.getOpt<int>(SUMO_ATTR_TLLINKINDEX2, nullptr, ok, -1);
    if (tlIndex2 == -1) {
        // we are updating an existing tl-controlled connection or index2 is not used
        tlIndex2 = c.tlLinkIndex2;
    }

    // register the connection with all definitions
    const std::map<std::string, NBTrafficLightDefinition*>& programs = myTLLCont.getPrograms(tlID);
    if (programs.size() > 0) {
        std::map<std::string, NBTrafficLightDefinition*>::const_iterator it;
        for (it = programs.begin(); it != programs.end(); it++) {
            NBLoadedSUMOTLDef* tlDef = dynamic_cast<NBLoadedSUMOTLDef*>(it->second);
            if (tlDef) {
                tlDef->addConnection(from, c.toEdge, c.fromLane, c.toLane, tlIndex, tlIndex2,  false);
            } else {
                throw ProcessError("Corrupt traffic light definition '"
                                   + tlID + "' (program '" + it->first + "')");
            }
        }
    } else {
        SumoXMLNodeType type = from->getToNode()->getType();
        if (type != SumoXMLNodeType::RAIL_CROSSING && type != SumoXMLNodeType::RAIL_SIGNAL) {
            WRITE_ERROR("The traffic light '" + tlID + "' is not known.");
        }
    }
}


void
NIXMLTrafficLightsHandler::removeTlConnection(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    std::string tlID = attrs.get<std::string>(SUMO_ATTR_TLID, nullptr, ok);
    // does the traffic light still exist?
    const std::map<std::string, NBTrafficLightDefinition*>& programs = myTLLCont.getPrograms(tlID);
    if (programs.size() > 0) {
        // parse identifying attributes
        NBEdge* from = retrieveEdge(attrs, SUMO_ATTR_FROM, ok);
        NBEdge* to = retrieveEdge(attrs, SUMO_ATTR_TO, ok);
        int fromLane = -1;
        int toLane = -1;
        if (ok) {
            fromLane = retrieveLaneIndex(attrs, SUMO_ATTR_FROM_LANE, from, ok, true);
            toLane = retrieveLaneIndex(attrs, SUMO_ATTR_TO_LANE, to, ok, true);
        }
        int tlIndex = attrs.get<int>(SUMO_ATTR_TLLINKINDEX, nullptr, ok);

        NBConnection conn(from, fromLane, to, toLane, tlIndex);
        // remove the connection from all definitions
        std::map<std::string, NBTrafficLightDefinition*>::const_iterator it;
        for (it = programs.begin(); it != programs.end(); it++) {
            NBLoadedSUMOTLDef* tlDef = dynamic_cast<NBLoadedSUMOTLDef*>(it->second);
            if (tlDef) {
                tlDef->removeConnection(conn, false);
            } else {
                throw ProcessError("Corrupt traffic light definition '"
                                   + tlID + "' (program '" + it->first + "')");
            }
        }
    }
}


NBEdge*
NIXMLTrafficLightsHandler::retrieveEdge(
    const SUMOSAXAttributes& attrs, SumoXMLAttr attr, bool& ok) {
    std::string edgeID = attrs.get<std::string>(attr, nullptr, ok);
    NBEdge* edge = myEdgeCont.retrieve(edgeID, true);
    if (edge == nullptr) {
        WRITE_ERROR("Unknown edge '" + edgeID + "' given in connection.");
        ok = false;
    }
    return edge;
}


int
NIXMLTrafficLightsHandler::retrieveLaneIndex(
    const SUMOSAXAttributes& attrs, SumoXMLAttr attr, NBEdge* edge, bool& ok, bool isDelete) {
    int laneIndex = attrs.get<int>(attr, nullptr, ok);
    if (edge->getNumLanes() <= laneIndex) {
        if (!isDelete) {
            WRITE_ERROR("Invalid lane index '" + toString(laneIndex) + "' for edge '" + edge->getID() + "'.");
        }
        ok = false;
    }
    return laneIndex;
}


/****************************************************************************/
