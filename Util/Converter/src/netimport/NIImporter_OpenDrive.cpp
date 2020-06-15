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
/// @file    NIImporter_OpenDrive.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in openDrive format
/****************************************************************************/
#include <config.h>
#include <string>
#include <cmath>
#include <iterator>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/shapes/SUMOPolygon.h>
#include <utils/shapes/PointOfInterest.h>
#include <utils/iodevices/OutputDevice.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBOwnTLDef.h>
#include <netbuild/NBTrafficLightLogicCont.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/geom/GeomConvHelper.h>
#include <foreign/eulerspiral/odrSpiral.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/geom/Boundary.h>
#include "NILoader.h"
#include "NIImporter_OpenDrive.h"

//#define DEBUG_VARIABLE_WIDTHS
//#define DEBUG_VARIABLE_SPEED
//#define DEBUG_CONNECTIONS
//#define DEBUG_SPIRAL
//#define DEBUG_INTERNALSHAPES

#define DEBUG_COND(road) ((road)->id == "18")
#define DEBUG_COND2(edgeID) (StringUtils::startsWith((edgeID), "-18."))
#define DEBUG_COND3(roadID) (roadID == "18")

// ===========================================================================
// definitions
// ===========================================================================

// ===========================================================================
// static variables
// ===========================================================================
StringBijection<int>::Entry NIImporter_OpenDrive::openDriveTags[] = {
    { "header",           NIImporter_OpenDrive::OPENDRIVE_TAG_HEADER },
    { "road",             NIImporter_OpenDrive::OPENDRIVE_TAG_ROAD },
    { "predecessor",      NIImporter_OpenDrive::OPENDRIVE_TAG_PREDECESSOR },
    { "successor",        NIImporter_OpenDrive::OPENDRIVE_TAG_SUCCESSOR },
    { "geometry",         NIImporter_OpenDrive::OPENDRIVE_TAG_GEOMETRY },
    { "line",             NIImporter_OpenDrive::OPENDRIVE_TAG_LINE },
    { "spiral",           NIImporter_OpenDrive::OPENDRIVE_TAG_SPIRAL },
    { "arc",              NIImporter_OpenDrive::OPENDRIVE_TAG_ARC },
    { "poly3",            NIImporter_OpenDrive::OPENDRIVE_TAG_POLY3 },
    { "paramPoly3",       NIImporter_OpenDrive::OPENDRIVE_TAG_PARAMPOLY3 },
    { "laneSection",      NIImporter_OpenDrive::OPENDRIVE_TAG_LANESECTION },
    { "laneOffset",       NIImporter_OpenDrive::OPENDRIVE_TAG_LANEOFFSET },
    { "left",             NIImporter_OpenDrive::OPENDRIVE_TAG_LEFT },
    { "center",           NIImporter_OpenDrive::OPENDRIVE_TAG_CENTER },
    { "right",            NIImporter_OpenDrive::OPENDRIVE_TAG_RIGHT },
    { "lane",             NIImporter_OpenDrive::OPENDRIVE_TAG_LANE },
    { "signal",           NIImporter_OpenDrive::OPENDRIVE_TAG_SIGNAL },
    { "signalReference",  NIImporter_OpenDrive::OPENDRIVE_TAG_SIGNALREFERENCE },
    { "validity",         NIImporter_OpenDrive::OPENDRIVE_TAG_VALIDITY },
    { "junction",         NIImporter_OpenDrive::OPENDRIVE_TAG_JUNCTION },
    { "connection",       NIImporter_OpenDrive::OPENDRIVE_TAG_CONNECTION },
    { "laneLink",         NIImporter_OpenDrive::OPENDRIVE_TAG_LANELINK },
    { "width",            NIImporter_OpenDrive::OPENDRIVE_TAG_WIDTH },
    { "speed",            NIImporter_OpenDrive::OPENDRIVE_TAG_SPEED },
    { "elevation",        NIImporter_OpenDrive::OPENDRIVE_TAG_ELEVATION },
    { "geoReference",     NIImporter_OpenDrive::OPENDRIVE_TAG_GEOREFERENCE },
    { "object",           NIImporter_OpenDrive::OPENDRIVE_TAG_OBJECT },
    { "repeat",           NIImporter_OpenDrive::OPENDRIVE_TAG_REPEAT },

    { "",                 NIImporter_OpenDrive::OPENDRIVE_TAG_NOTHING }
};


StringBijection<int>::Entry NIImporter_OpenDrive::openDriveAttrs[] = {
    { "revMajor",       NIImporter_OpenDrive::OPENDRIVE_ATTR_REVMAJOR },
    { "revMinor",       NIImporter_OpenDrive::OPENDRIVE_ATTR_REVMINOR },
    { "id",             NIImporter_OpenDrive::OPENDRIVE_ATTR_ID },
    { "length",         NIImporter_OpenDrive::OPENDRIVE_ATTR_LENGTH },
    { "width",          NIImporter_OpenDrive::OPENDRIVE_ATTR_WIDTH },
    { "radius",         NIImporter_OpenDrive::OPENDRIVE_ATTR_RADIUS },
    { "distance",       NIImporter_OpenDrive::OPENDRIVE_ATTR_DISTANCE },
    { "tStart",         NIImporter_OpenDrive::OPENDRIVE_ATTR_TSTART },
    { "tEnd",           NIImporter_OpenDrive::OPENDRIVE_ATTR_TEND },
    { "widthStart",     NIImporter_OpenDrive::OPENDRIVE_ATTR_WIDTHSTART },
    { "widthEnd",       NIImporter_OpenDrive::OPENDRIVE_ATTR_WIDTHEND },
    { "junction",       NIImporter_OpenDrive::OPENDRIVE_ATTR_JUNCTION },
    { "elementType",    NIImporter_OpenDrive::OPENDRIVE_ATTR_ELEMENTTYPE },
    { "elementId",      NIImporter_OpenDrive::OPENDRIVE_ATTR_ELEMENTID },
    { "contactPoint",   NIImporter_OpenDrive::OPENDRIVE_ATTR_CONTACTPOINT },
    { "s",              NIImporter_OpenDrive::OPENDRIVE_ATTR_S },
    { "t",              NIImporter_OpenDrive::OPENDRIVE_ATTR_T },
    { "x",              NIImporter_OpenDrive::OPENDRIVE_ATTR_X },
    { "y",              NIImporter_OpenDrive::OPENDRIVE_ATTR_Y },
    { "hdg",            NIImporter_OpenDrive::OPENDRIVE_ATTR_HDG },
    { "curvStart",      NIImporter_OpenDrive::OPENDRIVE_ATTR_CURVSTART },
    { "curvEnd",        NIImporter_OpenDrive::OPENDRIVE_ATTR_CURVEND },
    { "curvature",      NIImporter_OpenDrive::OPENDRIVE_ATTR_CURVATURE },
    { "a",              NIImporter_OpenDrive::OPENDRIVE_ATTR_A },
    { "b",              NIImporter_OpenDrive::OPENDRIVE_ATTR_B },
    { "c",              NIImporter_OpenDrive::OPENDRIVE_ATTR_C },
    { "d",              NIImporter_OpenDrive::OPENDRIVE_ATTR_D },
    { "aU",             NIImporter_OpenDrive::OPENDRIVE_ATTR_AU },
    { "bU",             NIImporter_OpenDrive::OPENDRIVE_ATTR_BU },
    { "cU",             NIImporter_OpenDrive::OPENDRIVE_ATTR_CU },
    { "dU",             NIImporter_OpenDrive::OPENDRIVE_ATTR_DU },
    { "aV",             NIImporter_OpenDrive::OPENDRIVE_ATTR_AV },
    { "bV",             NIImporter_OpenDrive::OPENDRIVE_ATTR_BV },
    { "cV",             NIImporter_OpenDrive::OPENDRIVE_ATTR_CV },
    { "dV",             NIImporter_OpenDrive::OPENDRIVE_ATTR_DV },
    { "pRange",         NIImporter_OpenDrive::OPENDRIVE_ATTR_PRANGE },
    { "type",           NIImporter_OpenDrive::OPENDRIVE_ATTR_TYPE },
    { "level",          NIImporter_OpenDrive::OPENDRIVE_ATTR_LEVEL },
    { "orientation",    NIImporter_OpenDrive::OPENDRIVE_ATTR_ORIENTATION },
    { "dynamic",        NIImporter_OpenDrive::OPENDRIVE_ATTR_DYNAMIC },
    { "incomingRoad",   NIImporter_OpenDrive::OPENDRIVE_ATTR_INCOMINGROAD },
    { "connectingRoad", NIImporter_OpenDrive::OPENDRIVE_ATTR_CONNECTINGROAD },
    { "from",           NIImporter_OpenDrive::OPENDRIVE_ATTR_FROM },
    { "to",             NIImporter_OpenDrive::OPENDRIVE_ATTR_TO },
    { "fromLane",       NIImporter_OpenDrive::OPENDRIVE_ATTR_FROMLANE },
    { "toLane",         NIImporter_OpenDrive::OPENDRIVE_ATTR_TOLANE },
    { "max",            NIImporter_OpenDrive::OPENDRIVE_ATTR_MAX },
    { "sOffset",        NIImporter_OpenDrive::OPENDRIVE_ATTR_SOFFSET },
    { "name",           NIImporter_OpenDrive::OPENDRIVE_ATTR_NAME },
    // towards xodr v1.4 speed:unit
    { "unit",           NIImporter_OpenDrive::OPENDRIVE_ATTR_UNIT },

    { "",               NIImporter_OpenDrive::OPENDRIVE_ATTR_NOTHING }
};


bool NIImporter_OpenDrive::myImportAllTypes;
bool NIImporter_OpenDrive::myImportWidths;
double NIImporter_OpenDrive::myMinWidth;
bool NIImporter_OpenDrive::myImportInternalShapes;

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods (interface in this case)
// ---------------------------------------------------------------------------
void
NIImporter_OpenDrive::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether the option is set (properly)
    if (!oc.isUsableFileList("opendrive-files")) {
        return;
    }
    // prepare types
    myImportAllTypes = oc.getBool("opendrive.import-all-lanes");
    myImportWidths = !oc.getBool("opendrive.ignore-widths");
    myMinWidth = oc.getFloat("opendrive.min-width");
    myImportInternalShapes = oc.getBool("opendrive.internal-shapes");
    NBTypeCont& tc = nb.getTypeCont();
    NBNodeCont& nc = nb.getNodeCont();
    // build the handler
    std::map<std::string, OpenDriveEdge*> edges;
    NIImporter_OpenDrive handler(nb.getTypeCont(), edges);
    // parse file(s)
    std::vector<std::string> files = oc.getStringVector("opendrive-files");
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        if (!FileHelpers::isReadable(*file)) {
            WRITE_ERROR("Could not open opendrive file '" + *file + "'.");
            return;
        }
        handler.setFileName(*file);
        PROGRESS_BEGIN_MESSAGE("Parsing opendrive from '" + *file + "'");
        XMLSubSys::runParser(handler, *file);
        PROGRESS_DONE_MESSAGE();
    }
    // apply signal reference information
    for (auto& item : edges) {
        for (OpenDriveSignal& signal : item.second->signals) {
            if (signal.type == "") {
                if (handler.getSignals().count(signal.id) == 0) {
                    WRITE_WARNING("Could not find signal reference '" + signal.id + "'.");
                } else {
                    const OpenDriveSignal& ref = handler.getSignals()[signal.id];
                    signal.type = ref.type;
                    signal.name = ref.name;
                    signal.dynamic = ref.dynamic;
                }
            }
        }
    }

    // split inner/outer edges
    std::map<std::string, OpenDriveEdge*> innerEdges, outerEdges;
    for (std::map<std::string, OpenDriveEdge*>::iterator i = edges.begin(); i != edges.end(); ++i) {
        if ((*i).second->isInner) {
            innerEdges[(*i).first] = (*i).second;
        } else {
            outerEdges[(*i).first] = (*i).second;
        }
    }

    // convert geometries into a discretised representation
    computeShapes(edges);
    // check whether lane sections are valid and whether further must be introduced
    revisitLaneSections(tc, edges);

    // -------------------------
    // node building
    // -------------------------
    // build nodes#1
    //  look at all links which belong to a node, collect their bounding boxes
    //  and place the node in the middle of this bounding box
    std::map<std::string, Boundary> posMap;
    std::map<std::string, std::string> edge2junction;
    std::vector<NodeSet> joinedNodeIDs;
    //   compute node positions
    for (std::map<std::string, OpenDriveEdge*>::iterator i = innerEdges.begin(); i != innerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        assert(e->junction != "-1" && e->junction != "");
        edge2junction[e->id] = e->junction;
        if (posMap.find(e->junction) == posMap.end()) {
            posMap[e->junction] = Boundary();
        }
        posMap[e->junction].add(e->geom.getBoxBoundary());
    }
    //   build nodes
    for (std::map<std::string, Boundary>::iterator i = posMap.begin(); i != posMap.end(); ++i) {
        //std::cout << " import node=" << (*i).first << " z=" << (*i).second.getCenter() << " boundary=" << (*i).second << "\n";
        if (!nb.getNodeCont().insert((*i).first, (*i).second.getCenter())) {
            throw ProcessError("Could not add node '" + (*i).first + "'.");
        }
    }
    //  assign built nodes
    for (std::map<std::string, OpenDriveEdge*>::iterator i = outerEdges.begin(); i != outerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        for (std::vector<OpenDriveLink>::iterator j = e->links.begin(); j != e->links.end(); ++j) {
            OpenDriveLink& l = *j;
            const std::string& nid = l.elementID;
            if (l.elementType != OPENDRIVE_ET_ROAD) {
                if (nb.getNodeCont().retrieve(nid) == nullptr) {
                    // not yet seen, build (possibly a junction without connections)
                    Position pos = l.linkType == OPENDRIVE_LT_SUCCESSOR ? e->geom[-1] : e->geom[0];
                    if (!nb.getNodeCont().insert(nid, pos)) {
                        throw ProcessError("Could not build node '" + nid + "'.");
                    }
                }
                // set node information
                setNodeSecure(nb.getNodeCont(), *e, l.elementID, l.linkType, joinedNodeIDs);
                continue;
            }
            if (edge2junction.find(l.elementID) != edge2junction.end()) {
                // set node information of an internal road
                setNodeSecure(nb.getNodeCont(), *e, edge2junction[l.elementID], l.linkType, joinedNodeIDs);
                continue;
            }
        }
    }
    //  we should now have all nodes set for links which are not outer edge-to-outer edge links


    // build nodes#2
    //  build nodes for all outer edge-to-outer edge connections
    for (std::map<std::string, OpenDriveEdge*>::iterator i = outerEdges.begin(); i != outerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        for (std::vector<OpenDriveLink>::iterator j = e->links.begin(); j != e->links.end(); ++j) {
            OpenDriveLink& l = *j;
            if (l.elementType != OPENDRIVE_ET_ROAD || edge2junction.find(l.elementID) != edge2junction.end()) {
                // is a connection to an internal edge, or a node, skip
                continue;
            }
            // we have a direct connection between to external edges
            std::string id1 = e->id;
            std::string id2 = l.elementID;
            if (id1 < id2) {
                std::swap(id1, id2);
            }
            std::string nid = id1 + "." + id2;
            if (nb.getNodeCont().retrieve(nid) == nullptr) {
                // not yet seen, build
                Position pos = l.linkType == OPENDRIVE_LT_SUCCESSOR ? e->geom[-1] : e->geom[0];
                if (!nb.getNodeCont().insert(nid, pos)) {
                    throw ProcessError("Could not build node '" + nid + "'.");
                }
            }
            /* debug-stuff
            else {
                Position pos = l.linkType==OPENDRIVE_LT_SUCCESSOR ? e.geom[e.geom.size()-1] : e.geom[0];
                cout << nid << " " << pos << " " << nb.getNodeCont().retrieve(nid)->getPosition() << endl;
            }
            */
            setNodeSecure(nb.getNodeCont(), *e, nid, l.linkType, joinedNodeIDs);
        }
    }
    // we should now have start/end nodes for all outer edge-to-outer edge connections


    // build nodes#3
    //  assign further nodes generated from inner-edges
    //  these nodes have not been assigned earlier, because the connections are referenced in inner-edges
    for (std::map<std::string, OpenDriveEdge*>::iterator i = outerEdges.begin(); i != outerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        if (e->to != nullptr && e->from != nullptr) {
            continue;
        }
        for (std::map<std::string, OpenDriveEdge*>::iterator j = innerEdges.begin(); j != innerEdges.end(); ++j) {
            OpenDriveEdge* ie = (*j).second;
            for (std::vector<OpenDriveLink>::iterator k = ie->links.begin(); k != ie->links.end(); ++k) {
                OpenDriveLink& il = *k;
                if (il.elementType != OPENDRIVE_ET_ROAD || il.elementID != e->id) {
                    // not conneted to the currently investigated outer edge
                    continue;
                }
                std::string nid = edge2junction[ie->id];
                if (il.contactPoint == OPENDRIVE_CP_START) {
                    setNodeSecure(nb.getNodeCont(), *e, nid, OPENDRIVE_LT_PREDECESSOR, joinedNodeIDs);
                } else {
                    setNodeSecure(nb.getNodeCont(), *e, nid, OPENDRIVE_LT_SUCCESSOR, joinedNodeIDs);
                }
            }
        }

    }

    // build start/end nodes which were not defined previously
    for (std::map<std::string, OpenDriveEdge*>::iterator i = outerEdges.begin(); i != outerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        if ((e->from == nullptr || e->to == nullptr) && e->geom.size() == 0) {
            continue;
        }
        if (e->from == nullptr) {
            const std::string nid = e->id + ".begin";
            e->from = getOrBuildNode(nid, e->geom.front(), nb.getNodeCont());
        }
        if (e->to == nullptr) {
            const std::string nid = e->id + ".end";
            e->to = getOrBuildNode(nid, e->geom.back(), nb.getNodeCont());
        }
    }

    std::map<NBNode*, NBNode*> joinedNodes;
    for (NodeSet& joined : joinedNodeIDs) {
        Position joinedPos(0, 0);
        for (NBNode* j : joined) {
            joinedPos = joinedPos + j->getPosition();
        }
        joinedPos = joinedPos * (1.0 / joined.size());
        const std::string joinedID = "cluster_" + joinNamedToString(joined, "_");
        if (!nc.insert(joinedID, joinedPos)) {
            throw ProcessError("Could not add node '" + joinedID + "'.");
        }
        NBNode* n = nc.retrieve(joinedID);
        for (NBNode* j : joined) {
            joinedNodes[j] = n;
        }
    }
    for (std::map<std::string, OpenDriveEdge*>::iterator i = outerEdges.begin(); i != outerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        if (joinedNodes.count(e->from) != 0) {
            nc.extract(e->from, true);
            e->from = joinedNodes[e->from];
        }
        if (joinedNodes.count(e->to) != 0) {
            nc.extract(e->to, true);
            e->to = joinedNodes[e->to];
        }
    }


    // -------------------------
    // edge building
    // -------------------------
    const double defaultSpeed = tc.getSpeed("");
    const bool saveOrigIDs = OptionsCont::getOptions().getBool("output.original-names");
    // lane-id-map sumoEdge,sumoLaneIndex->odrLaneIndex
    std::map<std::pair<NBEdge*, int>, int> laneIndexMap;
    // build edges
    for (std::map<std::string, OpenDriveEdge*>::iterator i = outerEdges.begin(); i != outerEdges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        if (e->geom.size() < 2) {
            WRITE_WARNING("Ignoring road '" + e->id + "' without geometry.");
            continue;
        }
        bool lanesBuilt = false;

        // go along the lane sections, build a node in between of each pair

        /// @todo: One could think of determining whether lane sections may be joined when being equal in SUMO's sense
        /// Their naming would have to be updated, too, also in TraCI

        /// @todo: probably, the lane offsets to the center are not right
        NBNode* sFrom = e->from;
        NBNode* sTo = e->to;
        int priorityR = e->getPriority(OPENDRIVE_TAG_RIGHT);
        int priorityL = e->getPriority(OPENDRIVE_TAG_LEFT);
        double sB = 0;
        double sE = e->length;
        // 0-length geometries are possible if only the inner points are represented
        PositionVector geomWithOffset = e->geom;
        if (e->laneOffsets.size() > 0) {
            try {
                geomWithOffset.move2side(e->laneOffsets);
                //std::cout << " e=" << e->id << " offsets=" << e->laneOffsets << " geom=" << e->geom << " geom2=" << geomWithOffset << "\n";
            } catch (InvalidArgument&) {
                WRITE_WARNING("Could not apply laneOffsets for edge '" + e->id + "'");
            }
        }
        const double length2D = geomWithOffset.length2D();
        double cF = length2D == 0 ? 1 : e->length / length2D;
        NBEdge* prevRight = nullptr;
        NBEdge* prevLeft = nullptr;

        // starting at the same node as ending, and no lane sections?
        if (sFrom == sTo && e->laneSections.size() == 1) {
            // --> loop, split!
            OpenDriveLaneSection ls = e->laneSections[0];
            ls.s = e->length / 2.;
            e->laneSections.push_back(ls);
            WRITE_WARNING("Edge '" + e->id + "' has to be split as it connects same junctions.")
        }
        sanitizeWidths(e);
        if (myMinWidth > 0) {
            const double minDist = oc.getFloat("opendrive.curve-resolution");
            splitMinWidths(e, tc, minDist);
        }

        // build along lane sections
        for (std::vector<OpenDriveLaneSection>::iterator j = e->laneSections.begin(); j != e->laneSections.end(); ++j) {
            // add internal node if needed
            if (j == e->laneSections.end() - 1) {
                sTo = e->to;
                sE = e->length / cF;
            } else {
                double nextS = (j + 1)->s;
                sTo = new NBNode(e->id + "." + toString(nextS), geomWithOffset.positionAtOffset(nextS));
                if (!nb.getNodeCont().insert(sTo)) {
                    throw ProcessError("Could not add node '" + sTo->getID() + "'.");
                }
                sE = nextS / cF;
            }
            PositionVector geom = geomWithOffset.getSubpart2D(sB, sE);
            std::string id = e->id;
            if (sFrom != e->from || sTo != e->to) {
                id = id + "." + toString((*j).s);
            } else if (e->laneSections.size() == 1) {
                id = id + ".0.00";
            }
#ifdef DEBUG_VARIABLE_WIDTHS
            if (DEBUG_COND(e)) {
                std::cout << " id=" << id << " sB=" << sB << " sE=" << sE << " geom=" << geom << "\n";
            }
#endif

            // build lanes to right
            NBEdge* currRight = nullptr;
            if ((*j).rightLaneNumber > 0) {
                currRight = new NBEdge("-" + id, sFrom, sTo, (*j).rightType, defaultSpeed, (*j).rightLaneNumber, priorityR,
                                       NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET, geom, e->streetName, "", LaneSpreadFunction::RIGHT, true);
                lanesBuilt = true;
                const std::vector<OpenDriveLane>& lanes = (*j).lanesByDir[OPENDRIVE_TAG_RIGHT];
                for (std::vector<OpenDriveLane>::const_iterator k = lanes.begin(); k != lanes.end(); ++k) {
                    std::map<int, int>::const_iterator lp = (*j).laneMap.find((*k).id);
                    if (lp != (*j).laneMap.end()) {
                        int sumoLaneIndex = lp->second;
                        setLaneAttributes(e, currRight->getLaneStruct(sumoLaneIndex), *k, saveOrigIDs, tc);
                        laneIndexMap[std::make_pair(currRight, sumoLaneIndex)] = (*k).id;
                    }
                }
                if (!nb.getEdgeCont().insert(currRight, myImportAllTypes)) {
                    throw ProcessError("Could not add edge '" + currRight->getID() + "'.");
                }
                if (nb.getEdgeCont().wasIgnored(id)) {
                    prevRight = nullptr;
                } else {
                    // connect lane sections
                    if (prevRight != nullptr) {
                        std::map<int, int> connections = (*j).getInnerConnections(OPENDRIVE_TAG_RIGHT, *(j - 1));
                        for (std::map<int, int>::const_iterator k = connections.begin(); k != connections.end(); ++k) {
#ifdef DEBUG_CONNECTIONS
                            if (DEBUG_COND(e)) {
                                std::cout << "addCon1 from=" << prevRight->getID() << "_" << (*k).first << " to=" << currRight->getID() << "_" << (*k).second << "\n";
                            }
#endif
                            prevRight->addLane2LaneConnection((*k).first, currRight, (*k).second, NBEdge::Lane2LaneInfoType::VALIDATED);
                        }
                    }
                    prevRight = currRight;
                }
            }

            // build lanes to left
            NBEdge* currLeft = nullptr;
            if ((*j).leftLaneNumber > 0) {
                currLeft = new NBEdge(id, sTo, sFrom, (*j).leftType, defaultSpeed, (*j).leftLaneNumber, priorityL,
                                      NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET, geom.reverse(), e->streetName, "", LaneSpreadFunction::RIGHT, true);
                lanesBuilt = true;
                const std::vector<OpenDriveLane>& lanes = (*j).lanesByDir[OPENDRIVE_TAG_LEFT];
                for (std::vector<OpenDriveLane>::const_iterator k = lanes.begin(); k != lanes.end(); ++k) {
                    std::map<int, int>::const_iterator lp = (*j).laneMap.find((*k).id);
                    if (lp != (*j).laneMap.end()) {
                        int sumoLaneIndex = lp->second;
                        setLaneAttributes(e, currLeft->getLaneStruct(sumoLaneIndex), *k, saveOrigIDs, tc);
                        laneIndexMap[std::make_pair(currLeft, sumoLaneIndex)] = (*k).id;
                    }
                }
                if (!nb.getEdgeCont().insert(currLeft, myImportAllTypes)) {
                    throw ProcessError("Could not add edge '" + currLeft->getID() + "'.");
                }
                if (nb.getEdgeCont().wasIgnored(id)) {
                    prevLeft = nullptr;
                } else {
                    // connect lane sections
                    if (prevLeft != nullptr) {
                        std::map<int, int> connections = (*j).getInnerConnections(OPENDRIVE_TAG_LEFT, *(j - 1));
                        for (std::map<int, int>::const_iterator k = connections.begin(); k != connections.end(); ++k) {
#ifdef DEBUG_CONNECTIONS
                            if (DEBUG_COND(e)) {
                                std::cout << "addCon2 from=" << currLeft->getID() << "_" << (*k).first << " to=" << prevLeft->getID() << "_" << (*k).second << "\n";
                            }
#endif
                            currLeft->addLane2LaneConnection((*k).first, prevLeft, (*k).second, NBEdge::Lane2LaneInfoType::VALIDATED);
                        }
                    }
                    prevLeft = currLeft;
                }
            }
            (*j).sumoID = id;


            sB = sE;
            sFrom = sTo;
        }
        // optionally write road objects
        if (oc.isSet("polygon-output")) {
            const bool writeGeo = GeoConvHelper::getLoaded().usingGeoProjection() && (
                                      oc.isDefault("proj.plain-geo") || oc.getBool("proj.plain-geo"));
            OutputDevice& dev = OutputDevice::getDevice(oc.getString("polygon-output"));
            dev.writeXMLHeader("additional", "additional_file.xsd");
            //SUMOPolygon poly("road_" + e->id, "road", RGBColor::BLUE, e->geom, true, false);
            //poly.writeXML(dev, false);
            for (auto& o : e->objects) {
                Position ref = e->geom.positionAtOffset2D(o.s, -o.t);
                if (o.radius >= 0) {
                    // cicrular shape
                    // GeoConvHelper::getFinal is not ready yet
                    GeoConvHelper::getLoaded().cartesian2geo(ref);
                    PointOfInterest poly(o.id, o.type, RGBColor::YELLOW, ref, true, "", -1, 0);
                    poly.setParameter("name", o.name);
                    poly.writeXML(dev, writeGeo);
                } else {
                    // rectangular shape
                    PositionVector centerLine;
                    centerLine.push_back(Position(-o.length / 2, 0));
                    centerLine.push_back(Position(o.length / 2, 0));
                    double roadHdg = e->geom.rotationAtOffset(o.s);
                    centerLine.rotate2D(roadHdg + o.hdg);
                    //PointOfInterest poiRef("ref_" + o.id, "", RGBColor::CYAN, ref, false, "", 0, 0, Shape::DEFAULT_LAYER + 2);
                    //poiRef.writeXML(dev, false);
                    centerLine.add(ref);
                    //SUMOPolygon polyCenter("center_" + o.id, "", RGBColor::MAGENTA, centerLine, true, false, Shape::DEFAULT_LAYER + 1);
                    //polyCenter.writeXML(dev, false);
                    centerLine.move2side(o.width / 2);
                    PositionVector shape = centerLine;
                    centerLine.move2side(-o.width);
                    shape.append(centerLine.reverse(), POSITION_EPS);
                    if (writeGeo) {
                        // GeoConvHelper::getFinal is not ready yet
                        for (int i = 0; i < (int) shape.size(); i++) {
                            GeoConvHelper::getLoaded().cartesian2geo(shape[i]);
                        }
                    }
                    SUMOPolygon poly(o.id, o.type, RGBColor::YELLOW, shape, true, true, 1);
                    poly.setParameter("name", o.name);
                    poly.writeXML(dev, writeGeo);
                }
            }
        }
        if (!lanesBuilt) {
            WRITE_WARNINGF("Edge '%' has no lanes.", e->id);
        }
    }

    // -------------------------
    // connections building
    // -------------------------
    // generate explicit lane-to-lane connections
    for (std::map<std::string, OpenDriveEdge*>::iterator i = edges.begin(); i != edges.end(); ++i) {
        setEdgeLinks2(*(*i).second, edges);
    }
    // compute connections across intersections, if any
    std::vector<Connection> connections2;
    for (std::map<std::string, OpenDriveEdge*>::iterator j = edges.begin(); j != edges.end(); ++j) {
        const std::set<Connection>& conns = (*j).second->connections;

        for (std::set<Connection>::const_iterator i = conns.begin(); i != conns.end(); ++i) {
            if (innerEdges.find((*i).fromEdge) != innerEdges.end()) {
                // connections starting at inner edges are processed by starting from outer edges
                continue;
            }
            if (innerEdges.find((*i).toEdge) != innerEdges.end()) {
                std::set<Connection> seen;
                buildConnectionsToOuter(*i, innerEdges, connections2, seen);
            } else {
                connections2.push_back(*i);
            }
        }
    }
    // set connections
    for (std::vector<Connection>::const_iterator i = connections2.begin(); i != connections2.end(); ++i) {
#ifdef DEBUG_CONNECTIONS
        std::cout << "connections2 " << (*i).getDescription() << "\n";
#endif
        std::string fromEdge = (*i).fromEdge;
        if (edges.find(fromEdge) == edges.end()) {
            WRITE_WARNING("While setting connections: from-edge '" + fromEdge + "' is not known.");
            continue;
        }
        OpenDriveEdge* odFrom = edges[fromEdge];
        int fromLane = (*i).fromLane;
        bool fromLast = ((*i).fromCP == OPENDRIVE_CP_END) && ((*i).fromLane < 0);
        fromEdge = fromLast ? odFrom->laneSections.back().sumoID : odFrom->laneSections[0].sumoID;

        std::string toEdge = (*i).toEdge;
        if (edges.find(toEdge) == edges.end()) {
            WRITE_WARNING("While setting connections: to-edge '" + toEdge + "' is not known.");
            continue;
        }

        OpenDriveEdge* odTo = edges[toEdge];
        int toLane = (*i).toLane;
        bool toLast = ((*i).toCP == OPENDRIVE_CP_END) || ((*i).toLane > 0);
        toEdge = toLast ? odTo->laneSections.back().sumoID : odTo->laneSections[0].sumoID;

        if (fromLane == UNSET_CONNECTION) {
            continue;
        }
        if (fromLane < 0) {
            fromEdge = revertID(fromEdge);
        }
        if (toLane == UNSET_CONNECTION) {
            continue;
        }
        if (toLane < 0) {
            toEdge = revertID(toEdge);
        }
        fromLane = fromLast ? odFrom->laneSections.back().laneMap[fromLane] : odFrom->laneSections[0].laneMap[fromLane];
        toLane = toLast ?  odTo->laneSections.back().laneMap[toLane] : odTo->laneSections[0].laneMap[toLane];
        NBEdge* from = nb.getEdgeCont().retrieve(fromEdge);
        NBEdge* to = nb.getEdgeCont().retrieve(toEdge);
        if (from == nullptr) {
            WRITE_WARNING("Could not find fromEdge representation of '" + fromEdge + "' in connection '" + (*i).origID + "'.");
        }
        if (to == nullptr) {
            WRITE_WARNING("Could not find fromEdge representation of '" + toEdge + "' in connection '" + (*i).origID + "'.");
        }
        if (from == nullptr || to == nullptr) {
            continue;
        }

#ifdef DEBUG_CONNECTIONS
        if (DEBUG_COND2(from->getID())) {
            std::cout << "addCon3 from=" << from->getID() << "_" << fromLane << " to=" << to->getID() << "_" << toLane << "\n";
        }
#endif
        from->addLane2LaneConnection(fromLane, to, toLane, NBEdge::Lane2LaneInfoType::USER, false, false, true,
                                     NBEdge::UNSPECIFIED_CONTPOS,
                                     NBEdge::UNSPECIFIED_VISIBILITY_DISTANCE,
                                     NBEdge::UNSPECIFIED_SPEED,
                                     NBEdge::UNSPECIFIED_LOADED_LENGTH,
                                     (*i).shape);

        if ((*i).origID != "" && saveOrigIDs) {
            // @todo: this is the most silly way to determine the connection
            std::vector<NBEdge::Connection>& cons = from->getConnections();
            for (std::vector<NBEdge::Connection>::iterator k = cons.begin(); k != cons.end(); ++k) {
                if ((*k).fromLane == fromLane && (*k).toEdge == to && (*k).toLane == toLane) {
                    (*k).setParameter(SUMO_PARAM_ORIGID, (*i).origID + "_" + toString((*i).origLane));
                    break;
                }
            }
        }
    }


    // -------------------------
    // traffic lights
    // -------------------------
    std::map<std::string, std::string> tlsControlled;
    for (std::map<std::string, OpenDriveEdge*>::iterator i = edges.begin(); i != edges.end(); ++i) {
        OpenDriveEdge* e = (*i).second;
        for (const OpenDriveSignal& signal : e->signals) {
            if (signal.type != "1000001") { // traffic_light (Section 6.11)
                continue;
            }
            if (e->laneSections.size() == 0) {
                WRITE_WARNING("Edge '" + e->id + "' has signals but no lane sections.");
                continue;
            }
            std::vector<OpenDriveLaneSection>::iterator k = e->laneSections.begin();
            bool found = false;
            for (; k != e->laneSections.end() - 1 && !found;) {
                if (signal.s > (*k).s && signal.s <= (*(k + 1)).s) {
                    found = true;
                } else {
                    ++k;
                }
            }

            std::string id = (*k).sumoID;
            if (id == "") {
                // traffic light on connecting road
                if (e->junction != "") {
                    //WRITE_WARNING("Found a traffic light signal on an internal edge; will not build it (original edge id='" + e->id + "').");
                    std::string fromID, toID;
                    for (std::vector<OpenDriveLink>::const_iterator l = e->links.begin(); l != e->links.end(); ++l) {
                        if ((*l).linkType == OPENDRIVE_LT_PREDECESSOR && (*l).elementType == OPENDRIVE_ET_ROAD) {
                            if (fromID != "") {
                                WRITE_WARNING("Ambigous start of connection.");
                            }
                            OpenDriveEdge* e = edges[(*l).elementID];
                            if ((*l).contactPoint == OPENDRIVE_CP_START) {
                                fromID = e->laneSections[0].sumoID;
                                if (signal.orientation < 0) {
                                    fromID = "-" + fromID;
                                }
                            } else {
                                fromID = e->laneSections.back().sumoID;
                                if (signal.orientation > 0) {
                                    fromID = "-" + fromID;
                                }
                            }
                        }
                        if ((*l).linkType == OPENDRIVE_LT_SUCCESSOR && (*l).elementType == OPENDRIVE_ET_ROAD) {
                            if (toID != "") {
                                WRITE_WARNING("Ambigous end of connection.");
                            }
                            OpenDriveEdge* e = edges[(*l).elementID];
                            toID = (*l).contactPoint == OPENDRIVE_CP_START ? e->laneSections[0].sumoID : e->laneSections.back().sumoID;
                        }
                    }
                    // figure out the correct combination of directions
                    NBEdge* from;
                    NBEdge* to;
                    auto fromTo = retrieveSignalEdges(nb, fromID, toID, e->junction);
                    from = fromTo.first;
                    to = fromTo.second;
                    if (from == nullptr) {
                        WRITE_WARNINGF("Could not find edge '%' for signal '%'.", fromID, signal.id);
                        continue;
                    }

                    // consider signal validity to determine direction
                    if (signal.maxLane != 0) {
                        bool fromForward = from->getID()[0] == '-';
                        bool lanesForward = signal.maxLane < 0;
                        if (fromForward != lanesForward) {
                            std::swap(fromID, toID);

                            auto fromTo = retrieveSignalEdges(nb, fromID, toID, e->junction);
                            from = fromTo.first;
                            to = fromTo.second;
                            if (from == nullptr) {
                                WRITE_WARNINGF("Could not find edge '%' for signal '%'.", fromID, signal.id);
                                continue;
                            }
                        }
                    }
                    for (NBEdge::Connection& c : from->getConnections()) {
                        if (c.toEdge == to) {
                            int odLane = laneIndexMap[std::make_pair(from, c.fromLane)];
                            //std::cout << "  fromLane=" << c.fromLane << " odLane=" << odLane << "\n";
                            if (signal.minLane == 0 || (signal.minLane <= odLane && signal.maxLane >= odLane)) {
                                c.setParameter("signalID", signal.id);
                            }
                        }
                    }
                    getTLSSecure(from, nb);
                    //std::cout << "odrEdge=" << e->id << " fromID=" << fromID << " toID=" << toID << " from=" << from->getID() << " to=" << to->getID()
                    //    << " signal=" << signal.id << " minLane=" << signal.minLane << " maxLane=" << signal.maxLane << "\n";
                } else {
                    WRITE_WARNINGF("Found a traffic light signal on an unknown edge (original edge id='%').", e->id);
                    continue;
                }
            } else {
                // traffic light on normal road
                if (signal.orientation == 1) {
                    // forward direction has negative lane indices and gets a negative prefix in sumo
                    id = "-" + id;
                }
                NBEdge* edge = nb.getEdgeCont().retrieve(id);
                if (edge == nullptr) {
                    WRITE_WARNINGF("Could not find edge '%' for signal '%'.", id, signal.id);
                    continue;
                }
                getTLSSecure(edge, nb);
                /// XXX consider lane validity
                for (NBEdge::Connection& c : edge->getConnections()) {
                    int odLane = laneIndexMap[std::make_pair(edge, c.fromLane)];
                    if (signal.minLane == 0 || (signal.minLane <= odLane && signal.maxLane >= odLane)) {
                        c.setParameter("signalID", signal.id);
                    }
                }
                //std::cout << "odrEdge=" << e->id << " sumoID=" << (*k).sumoID << " sumoEdge=" << edge->getID()
                //    << " signal=" << signal.id << " minLane=" << signal.minLane << " maxLane=" << signal.maxLane << "\n";
            }
            // @note: tls 'signalID' parameters are set via NBTrafficLightLogicCont::setOpenDriveSignalParameters
        }
    }

    // -------------------------
    // clean up
    // -------------------------
    for (std::map<std::string, OpenDriveEdge*>::iterator i = edges.begin(); i != edges.end(); ++i) {
        delete (*i).second;
    }
}

std::pair<NBEdge*, NBEdge*>
NIImporter_OpenDrive::retrieveSignalEdges(NBNetBuilder& nb, const std::string& fromID, const std::string& toID, const std::string& junction) {
    NBEdge* from;
    NBEdge* to;
    from = nb.getEdgeCont().retrieve(fromID);
    if (from == nullptr || from->getToNode()->getID() != junction) {
        from = nb.getEdgeCont().retrieve(fromID[0] == '-' ? fromID.substr(1) : "-" + fromID);
    }
    to = nb.getEdgeCont().retrieve(toID);
    if (to == nullptr || to->getFromNode()->getID() != junction) {
        to = nb.getEdgeCont().retrieve(toID[0] == '-' ? toID.substr(1) : "-" + toID);
    }
    return std::make_pair(from, to);
}


NBTrafficLightDefinition*
NIImporter_OpenDrive::getTLSSecure(NBEdge* inEdge, NBNetBuilder& nb) {
    NBNode* toNode = inEdge->getToNode();
    if (!toNode->isTLControlled()) {
        TrafficLightType type = SUMOXMLDefinitions::TrafficLightTypes.get(OptionsCont::getOptions().getString("tls.default-type"));
        NBOwnTLDef* tlDef = new NBOwnTLDef(toNode->getID(), toNode, 0, type);
        if (!nb.getTLLogicCont().insert(tlDef)) {
            // actually, nothing should fail here
            delete tlDef;
            throw ProcessError();
        }
        toNode->addTrafficLight(tlDef);
        //tlDef->setSinglePhase();
    }
    return *toNode->getControllingTLS().begin();
}

void
NIImporter_OpenDrive::setLaneAttributes(const OpenDriveEdge* e, NBEdge::Lane& sumoLane, const OpenDriveLane& odLane, bool saveOrigIDs, const NBTypeCont& tc) {
    if (saveOrigIDs) {
        sumoLane.setParameter(SUMO_PARAM_ORIGID, e->id + "_" + toString(odLane.id));
    }
    sumoLane.speed = odLane.speed != 0 ? odLane.speed : tc.getSpeed(odLane.type);
    sumoLane.permissions = tc.getPermissions(odLane.type);
    sumoLane.width = myImportWidths && odLane.width != NBEdge::UNSPECIFIED_WIDTH ? odLane.width : tc.getWidth(odLane.type);
    sumoLane.type = odLane.type;

    const double widthResolution = tc.getWidthResolution(odLane.type);
    const double maxWidth = tc.getMaxWidth(odLane.type);

    const bool forbiddenNarrow = (sumoLane.width < myMinWidth
                                  && (sumoLane.permissions & SVC_PASSENGER) != 0
                                  && sumoLane.width < tc.getWidth(odLane.type));

    if (sumoLane.width >= 0 && widthResolution > 0) {
        sumoLane.width = floor(sumoLane.width / widthResolution + 0.5) * widthResolution;
        if (forbiddenNarrow && sumoLane.width >= myMinWidth) {
            sumoLane.width -= widthResolution;
            if (sumoLane.width <= 0) {
                sumoLane.width = MAX2(POSITION_EPS, myMinWidth - POSITION_EPS);
            }
        } else if (sumoLane.width == 0) {
            // round up when close to 0
            sumoLane.width = widthResolution;
        }
    }
    if (maxWidth > 0) {
        sumoLane.width = MIN2(sumoLane.width, maxWidth);
    }
    if (forbiddenNarrow) {
        // avoid narrow passenger car lanes (especially at sections with varying width)
        sumoLane.permissions = SVC_EMERGENCY | SVC_AUTHORITY;
    }
}

void
NIImporter_OpenDrive::buildConnectionsToOuter(const Connection& c, const std::map<std::string, OpenDriveEdge*>& innerEdges, std::vector<Connection>& into, std::set<Connection>& seen) {

    OpenDriveEdge* dest = innerEdges.find(c.toEdge)->second;
#ifdef DEBUG_CONNECTIONS
    if (DEBUG_COND3(c.fromEdge)) {
        std::cout << "  buildConnectionsToOuter " << c.getDescription() << "\n";
        std::cout << "    dest=" << (dest == nullptr ? "NULL" : dest->id) << " seenlist=";
        for (std::set<Connection>::const_iterator i = seen.begin(); i != seen.end(); ++i) {
            std::cout << "    " << (*i).fromEdge << "," << (*i).toEdge << " ";
        }
        std::cout << "\n";
    }
#endif
    if (dest == nullptr) {
        /// !!! should not, look in all?
        return;
    }
    seen.insert(c);
    const std::set<Connection>& conts = dest->connections;
    for (std::set<Connection>::const_iterator i = conts.begin(); i != conts.end(); ++i) {
        auto innerEdgesIt = innerEdges.find((*i).toEdge);
#ifdef DEBUG_CONNECTIONS
        if (DEBUG_COND3(c.fromEdge)) {
            std::cout << "      toInner=" << (innerEdgesIt != innerEdges.end()) << " destCon " << (*i).getDescription() << "\n";
        }
#endif
        if (innerEdgesIt != innerEdges.end()) {
            std::vector<Connection> t;
            if (seen.count(*i) == 0) {
                buildConnectionsToOuter(*i, innerEdges, t, seen);
                for (std::vector<Connection>::const_iterator j = t.begin(); j != t.end(); ++j) {
                    // @todo this section is unverified
                    Connection cn = (*j);
                    cn.fromEdge = c.fromEdge;
                    cn.fromLane = c.fromLane;
                    cn.fromCP = c.fromCP;
                    cn.all = c.all; // @todo "all" is a hack trying to avoid the "from is zero" problem;
                    if (myImportInternalShapes) {
                        cn.shape = innerEdgesIt->second->geom + c.shape;
                    }
                    into.push_back(cn);
                }
            } else {
                WRITE_WARNING("Circular connections in junction including roads '" + c.fromEdge + "' and '" + c.toEdge + "', loop size " + toString(seen.size()));
            }
        } else {
            int in = c.toLane;
            int out = (*i).fromLane;
            if (c.toCP == OPENDRIVE_CP_END) {
                // inner edge runs in reverse direction
                std::swap(in, out);
            }
#ifdef DEBUG_CONNECTIONS
            if (DEBUG_COND3(c.fromEdge)) {
                std::cout << "        laneSectionsConnected dest=" << dest->id << " in=" << in << " out=" << out
                          << " connected=" <<  laneSectionsConnected(dest, in, out) << "\n";
            }
#endif

            if (laneSectionsConnected(dest, in, out)) {
                Connection cn = (*i);
                cn.fromEdge = c.fromEdge;
                cn.fromLane = c.fromLane;
                cn.fromCP = c.fromCP;
                cn.all = c.all;
                cn.origID = c.toEdge;
                cn.origLane = c.toLane;
                if (myImportInternalShapes) {
                    OpenDriveXMLTag lanesDir;
                    cn.shape = dest->geom;
                    // determine which lane of dest belongs to this connection
                    int referenceLane = 0;
                    int offsetFactor = 1;
                    if (c.toCP == OPENDRIVE_CP_END) {
                        offsetFactor = -1;
                        lanesDir = OPENDRIVE_TAG_LEFT;
                        for (const auto& destLane : dest->laneSections.front().lanesByDir[lanesDir]) {
                            if (destLane.successor == c.fromLane) {
                                referenceLane = destLane.id;
                                break;
                            }
                        }
                    } else {
                        lanesDir = OPENDRIVE_TAG_RIGHT;
                        for (const auto& destLane : dest->laneSections.front().lanesByDir[lanesDir]) {
                            if (destLane.predecessor == c.fromLane) {
                                referenceLane = destLane.id;
                                break;
                            }
                        }
                    }
                    // compute offsets
                    //if (cn.fromEdge == "1014000" && dest->id == "3001022") {
                    //    std::cout << "computeOffsets\n";
                    //}
                    std::vector<double> offsets(dest->geom.size(), 0);
                    if (dest->laneOffsets.size() > 0) {
                        offsets = dest->laneOffsets;
                    }
#ifdef DEBUG_INTERNALSHAPES
                    std::string destPred;
#endif
                    double s = 0;
                    int iShape = 0;
                    for (int laneSectionIndex = 0; laneSectionIndex < (int)dest->laneSections.size(); laneSectionIndex++) {
                        auto& laneSection = dest->laneSections[laneSectionIndex];
                        const double nextS = laneSectionIndex + 1 < (int)dest->laneSections.size() ? dest->laneSections[laneSectionIndex + 1].s : std::numeric_limits<double>::max();
                        int i = iShape; // shape index at the start of the current lane section
                        double sStart = s; // distance offset a the start of the current lane section
                        double finalS = s; // final distance value after processing this segment
                        int finalI = i;
                        for (const OpenDriveLane& destLane : laneSection.lanesByDir[lanesDir]) {
                            // each lane of the current segment repeats the same section of shape points and distance offsets
                            double sectionS = 0;
                            i = iShape;
                            s = sStart;
#ifdef DEBUG_INTERNALSHAPES
                            destPred += "  lane=" + toString(destLane.id)
                                        + " pred=" + toString(destLane.predecessor)
                                        + " succ=" + toString(destLane.successor)
                                        + " wStart=" + toString(destLane.widthData.front().computeAt(0))
                                        + " wEnd=" + toString(destLane.widthData.front().computeAt(cn.shape.length2D()))
                                        + " width=" + toString(destLane.width) + "\n";
#endif
                            if (abs(destLane.id) <= abs(referenceLane)) {
                                const double multiplier = offsetFactor * (destLane.id == referenceLane ? 0.5 : 1);
#ifdef DEBUG_INTERNALSHAPES
                                destPred += "     multiplier=" + toString(multiplier) + "\n";
#endif
                                int widthDataIndex = 0;
                                while (s < nextS && i < (int)cn.shape.size()) {
                                    if (i > 0) {
                                        const double dist = cn.shape[i - 1].distanceTo2D(cn.shape[i]);
                                        s += dist;
                                        sectionS += dist;

                                    }
                                    while (widthDataIndex + 1 < (int)destLane.widthData.size()
                                            && sectionS >= destLane.widthData[widthDataIndex + 1].s) {
                                        widthDataIndex++;
                                    }
                                    offsets[i] += destLane.widthData[widthDataIndex].computeAt(sectionS) * multiplier;
                                    //if (cn.fromEdge == "1014000" && dest->id == "3001022") {
                                    //    std::cout << " i=" << i << " s=" << s << " lane=" << destLane.id << " rlane=" << referenceLane /*<< " nextS=" << nextS << */ << " lsIndex=" << laneSectionIndex << " wI=" << widthDataIndex << " wSize=" << destLane.widthData.size() << " m=" << multiplier << " o=" << offsets[i] << "\n";
                                    //}
                                    i++;
                                }
                                finalS = s;
                                finalI = i;
                            } else if (finalS == s) {
                                // update finalS without changing offsets
                                while (s < nextS && i < (int)cn.shape.size()) {
                                    if (i > 0) {
                                        const double dist = cn.shape[i - 1].distanceTo2D(cn.shape[i]);
                                        s += dist;
                                        finalS += dist;

                                    }
                                    i++;
                                }
                                finalI = i;

                            }
                        }
                        // advance values for the next lane section
                        iShape = finalI;
                        s = finalS;
                    }
                    try {
                        cn.shape.move2side(offsets);
                    } catch (InvalidArgument&) {
                        WRITE_WARNING("Could not import internal lane shape from edge '" + c.fromEdge + "' to edge '" + c.toEdge);
                        cn.shape.clear();
                    }
#ifdef DEBUG_INTERNALSHAPES
                    std::cout << "internalShape "
                              << c.getDescription()
                              << " dest=" << dest->id
                              << " refLane=" << referenceLane
                              << " destPred\n" << destPred
                              << " offsets=" << offsets
                              << "\n shape=" << dest->geom
                              << "\n shape2=" << cn.shape
                              << "\n";
#endif
                    if (c.toCP == OPENDRIVE_CP_END) {
                        cn.shape = cn.shape.reverse();
                    }
                }
#ifdef DEBUG_CONNECTIONS
                if (DEBUG_COND3(c.fromEdge)) {
                    std::cout << "        added connection\n";
                }
#endif
                into.push_back(cn);
            }
        }
    }
}


bool
NIImporter_OpenDrive::laneSectionsConnected(OpenDriveEdge* edge, int in, int out) {
    if (edge->laneSections.size() == 1) {
        return in == out;
    } else {
        // there could be spacing lanes (type 'none') that lead to a shift in lane index
        for (auto it = edge->laneSections.begin(); it + 1 < edge->laneSections.end(); it++) {
            OpenDriveLaneSection& laneSection = *it;
            if (laneSection.lanesByDir.find(OPENDRIVE_TAG_RIGHT) != laneSection.lanesByDir.end()) {
                for (OpenDriveLane& lane : laneSection.lanesByDir.find(OPENDRIVE_TAG_RIGHT)->second) {
                    if (lane.id == in) {
                        in = lane.successor;
                        break;
                    }
                }
            }
            if (laneSection.lanesByDir.find(OPENDRIVE_TAG_LEFT) != laneSection.lanesByDir.end()) {
                for (OpenDriveLane& lane : laneSection.lanesByDir.find(OPENDRIVE_TAG_LEFT)->second) {
                    if (lane.id == in) {
                        in = lane.successor;
                        break;
                    }
                }
            }
        }
        return in == out;
    }
}


void
NIImporter_OpenDrive::setEdgeLinks2(OpenDriveEdge& e, const std::map<std::string, OpenDriveEdge*>& edges) {
    for (std::vector<OpenDriveLink>::iterator i = e.links.begin(); i != e.links.end(); ++i) {
        OpenDriveLink& l = *i;
        if (l.elementType != OPENDRIVE_ET_ROAD) {
            // we assume that links to nodes are later given as connections to edges
            continue;
        }
        // get the right direction of the connected edge
        std::string connectedEdge = l.elementID;
        std::string edgeID = e.id;

        OpenDriveLaneSection& laneSection = l.linkType == OPENDRIVE_LT_SUCCESSOR ? e.laneSections.back() : e.laneSections[0];
        const std::map<int, int>& laneMap = laneSection.laneMap;
#ifdef DEBUG_CONNECTIONS
        if (DEBUG_COND(&e)) {
            std::cout << "edge=" << e.id << " eType=" << l.elementType << " lType=" << l.linkType  << " connectedEdge=" << connectedEdge << " laneSection=" << laneSection.s << " map:\n";
            std::cout << joinToString(laneMap, "\n", ":") << "\n";
        }
#endif
        if (laneSection.lanesByDir.find(OPENDRIVE_TAG_RIGHT) != laneSection.lanesByDir.end()) {
            const std::vector<OpenDriveLane>& lanes = laneSection.lanesByDir.find(OPENDRIVE_TAG_RIGHT)->second;
            for (std::vector<OpenDriveLane>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
                if (!myImportAllTypes && laneMap.find((*j).id) == laneMap.end()) {
                    continue;
                }
                Connection c; // @todo: give Connection a new name and a constructor
                c.fromEdge = e.id;
                c.fromLane = (*j).id;
                c.fromCP = OPENDRIVE_CP_END;
                c.toLane = l.linkType == OPENDRIVE_LT_SUCCESSOR ? (*j).successor : (*j).predecessor;
                c.toEdge = connectedEdge;
                c.toCP = l.contactPoint;
                c.all = false;
                if (l.linkType != OPENDRIVE_LT_SUCCESSOR) {
                    std::swap(c.fromEdge, c.toEdge);
                    std::swap(c.fromLane, c.toLane);
                    std::swap(c.fromCP, c.toCP);
                }
                if (edges.find(c.fromEdge) == edges.end()) {
                    WRITE_ERROR("While setting connections: incoming road '" + c.fromEdge + "' is not known.");
                } else {
                    OpenDriveEdge* src = edges.find(c.fromEdge)->second;
                    src->connections.insert(c);
#ifdef DEBUG_CONNECTIONS
                    if (DEBUG_COND(src)) {
                        std::cout << "insertConRight from=" << src->id << "_" << c.fromLane << " to=" << c.toEdge << "_" << c.toLane << "\n";
                    }
#endif
                }
            }
        }
        if (laneSection.lanesByDir.find(OPENDRIVE_TAG_LEFT) != laneSection.lanesByDir.end()) {
            const std::vector<OpenDriveLane>& lanes = laneSection.lanesByDir.find(OPENDRIVE_TAG_LEFT)->second;
            for (std::vector<OpenDriveLane>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
                if (!myImportAllTypes && laneMap.find((*j).id) == laneMap.end()) {
                    continue;
                }
                Connection c;
                c.toEdge = e.id;
                c.toLane = (*j).id;
                c.toCP = OPENDRIVE_CP_END;
                c.fromLane = l.linkType == OPENDRIVE_LT_SUCCESSOR ? (*j).successor : (*j).predecessor;
                c.fromEdge = connectedEdge;
                c.fromCP = l.contactPoint;
                c.all = false;
                if (l.linkType != OPENDRIVE_LT_SUCCESSOR) {
                    std::swap(c.fromEdge, c.toEdge);
                    std::swap(c.fromLane, c.toLane);
                    std::swap(c.fromCP, c.toCP);
                }
                if (edges.find(c.fromEdge) == edges.end()) {
                    WRITE_ERROR("While setting connections: incoming road '" + c.fromEdge + "' is not known.");
                } else {
                    OpenDriveEdge* src = edges.find(c.fromEdge)->second;
                    src->connections.insert(c);
#ifdef DEBUG_CONNECTIONS
                    if (DEBUG_COND(src)) {
                        std::cout << "insertConLeft from=" << src->id << "_" << c.fromLane << " to=" << c.toEdge << "_" << c.toLane << "\n";
                    }
#endif
                }
            }
        }
    }
}


std::string NIImporter_OpenDrive::revertID(const std::string& id) {
    if (id[0] == '-') {
        return id.substr(1);
    }
    return "-" + id;
}


NBNode*
NIImporter_OpenDrive::getOrBuildNode(const std::string& id, const Position& pos,
                                     NBNodeCont& nc) {
    if (nc.retrieve(id) == nullptr) {
        // not yet built; build now
        if (!nc.insert(id, pos)) {
            // !!! clean up
            throw ProcessError("Could not add node '" + id + "'.");
        }
    }
    return nc.retrieve(id);
}


void
NIImporter_OpenDrive::setNodeSecure(NBNodeCont& nc, OpenDriveEdge& e,
                                    const std::string& nodeID, NIImporter_OpenDrive::LinkType lt, std::vector<NodeSet>& joinedNodeIDs) {
    NBNode* n = nc.retrieve(nodeID);
    if (n == nullptr) {
        throw ProcessError("Could not find node '" + nodeID + "'.");
    }
    NBNode* toJoin = nullptr;
    if (lt == OPENDRIVE_LT_SUCCESSOR) {
        if (e.to != nullptr && e.to != n) {
            toJoin = e.to;
        }
        e.to = n;
    } else {
        if (e.from != nullptr && e.from != n) {
            toJoin = e.from;
        }
        e.from = n;
    }
    if (toJoin != nullptr) {
        // join nodes
        NodeSet* set1 = nullptr;
        NodeSet* set2 = nullptr;
        for (NodeSet& joined : joinedNodeIDs) {
            if (joined.count(toJoin) != 0) {
                set1 = &joined;
            }
            if (joined.count(n) != 0) {
                set2 = &joined;
            }
        }
        if (set1 == nullptr && set2 == nullptr) {
            joinedNodeIDs.push_back(NodeSet());
            joinedNodeIDs.back().insert(n);
            joinedNodeIDs.back().insert(toJoin);
        } else if (set1 == nullptr && set2 != nullptr) {
            set2->insert(toJoin);
        } else if (set1 != nullptr && set2 == nullptr) {
            set1->insert(n);
        } else {
            set1->insert(set2->begin(), set2->end());
            joinedNodeIDs.erase(std::find(joinedNodeIDs.begin(), joinedNodeIDs.end(), *set2));
        }
    }
}

bool
NIImporter_OpenDrive::hasNonLinearElevation(OpenDriveEdge& e) {
    if (e.elevations.size() > 1) {
        return true;
    }
    for (OpenDriveElevation& el : e.elevations) {
        if (el.c != 0 || el.d != 0) {
            return true;
        }
    }
    return false;
}

void
NIImporter_OpenDrive::computeShapes(std::map<std::string, OpenDriveEdge*>& edges) {
    OptionsCont& oc = OptionsCont::getOptions();
    const double res = oc.getFloat("opendrive.curve-resolution");
    for (std::map<std::string, OpenDriveEdge*>::iterator i = edges.begin(); i != edges.end(); ++i) {
        OpenDriveEdge& e = *(*i).second;
        GeometryType prevType = OPENDRIVE_GT_UNKNOWN;
        const double lineRes = hasNonLinearElevation(e) ? res : -1;
        Position last;
        for (std::vector<OpenDriveGeometry>::iterator j = e.geometries.begin(); j != e.geometries.end(); ++j) {
            OpenDriveGeometry& g = *j;
            PositionVector geom;
            switch (g.type) {
                case OPENDRIVE_GT_UNKNOWN:
                    break;
                case OPENDRIVE_GT_LINE:
                    geom = geomFromLine(e, g, lineRes);
                    break;
                case OPENDRIVE_GT_SPIRAL:
                    geom = geomFromSpiral(e, g, res);
                    break;
                case OPENDRIVE_GT_ARC:
                    geom = geomFromArc(e, g, res);
                    break;
                case OPENDRIVE_GT_POLY3:
                    geom = geomFromPoly(e, g, res);
                    break;
                case OPENDRIVE_GT_PARAMPOLY3:
                    geom = geomFromParamPoly(e, g, res);
                    break;
                default:
                    break;
            }
            if (e.geom.size() > 0 && prevType == OPENDRIVE_GT_LINE) {
                // remove redundant end point of the previous geometry segment
                // (the start point of the current segment should have the same value)
                // this avoids geometry errors due to imprecision
                if (!e.geom.back().almostSame(geom.front())) {
                    const int index = (int)(j - e.geometries.begin());
                    WRITE_WARNING("Mismatched geometry for edge '" + e.id + "' between geometry segments " + toString(index - 1) + " and " + toString(index) + ".");
                }
                e.geom.pop_back();
            }
            //std::cout << " adding geometry to road=" << e.id << " old=" << e.geom << " new=" << geom << "\n";
            for (PositionVector::iterator k = geom.begin(); k != geom.end(); ++k) {
                last = *k;
                e.geom.push_back_noDoublePos(*k);
            }
            prevType = g.type;
        }
        if (e.geom.size() == 1 && e.geom.front() != last) {
            // avoid length-1 geometry due to almostSame check
            e.geom.push_back(last);
        }
        if (oc.exists("geometry.min-dist") && !oc.isDefault("geometry.min-dist")) {
            e.geom.removeDoublePoints(oc.getFloat("geometry.min-dist"), true);
        }
        if (!NBNetBuilder::transformCoordinates(e.geom)) {
            WRITE_ERROR("Unable to project coordinates for edge '" + e.id + "'.");
        }
        // add z-data
        int k = 0;
        double pos = 0;
        //std::cout << " edge=" << e.id << " geom.size=" << e.geom.size() << " geom.len=" << e.geom.length2D() << " ele.size=" << e.elevations.size() << "\n";
        for (std::vector<OpenDriveElevation>::iterator j = e.elevations.begin(); j != e.elevations.end(); ++j) {
            const OpenDriveElevation& el = *j;
            const double sNext = (j + 1) == e.elevations.end() ? std::numeric_limits<double>::max() : (*(j + 1)).s;
            while (k < (int)e.geom.size() && pos < sNext) {
                const double z = el.computeAt(pos);
                //std::cout << " edge=" << e.id << " k=" << k << " sNext=" << sNext << " pos=" << pos << " z=" << z << " el.s=" << el.s << " el.a=" << el.a << " el.b=" << el.b << " el.c=" << el.c << " el.d=" << el.d <<  "\n";
                e.geom[k].add(0, 0, z);
                k++;
                if (k < (int)e.geom.size()) {
                    // XXX pos understimates the actual position since the
                    // actual geometry between k-1 and k could be curved
                    pos += e.geom[k - 1].distanceTo2D(e.geom[k]);
                }
            }
        }
        // add laneoffset
        if (e.offsets.size() > 0) {
            // make sure there are intermediate points for each offset-section
            for (std::vector<OpenDriveLaneOffset>::iterator j = e.offsets.begin(); j != e.offsets.end(); ++j) {
                const OpenDriveLaneOffset& el = *j;
                // check wether we need to insert a new point at dist
                Position pS = e.geom.positionAtOffset2D(el.s);
                int iS = e.geom.indexOfClosest(pS);
                // prevent close spacing to reduce impact of rounding errors in z-axis
                if (pS.distanceTo2D(e.geom[iS]) > POSITION_EPS) {
                    e.geom.insertAtClosest(pS, false);
                    //std::cout << " edge=" << e.id << " inserting pos=" << pS << " s=" << el.s << " iS=" << iS << " dist=" << pS.distanceTo2D(e.geom[iS]) << "\n";
                }
            }
            // XXX add further points for sections with non-constant offset
            // shift each point orthogonally by the specified offset
            int k = 0;
            double pos = 0;
            for (std::vector<OpenDriveLaneOffset>::iterator j = e.offsets.begin(); j != e.offsets.end(); ++j) {
                const OpenDriveLaneOffset& el = *j;
                const double sNext = (j + 1) == e.offsets.end() ? std::numeric_limits<double>::max() : (*(j + 1)).s;
                while (k < (int)e.geom.size() && pos < sNext) {
                    const double offset = el.computeAt(pos);
                    e.laneOffsets.push_back(fabs(offset) > POSITION_EPS ? -offset : 0);
                    k++;
                    if (k < (int)e.geom.size()) {
                        // XXX pos understimates the actual position since the
                        // actual geometry between k-1 and k could be curved
                        pos += e.geom[k - 1].distanceTo2D(e.geom[k]);
                    }
                }
            }
        }
        //std::cout << " loaded geometry " << e.id << "=" << e.geom << "\n";
    }
}


void
NIImporter_OpenDrive::revisitLaneSections(const NBTypeCont& tc, std::map<std::string, OpenDriveEdge*>& edges) {
    for (std::map<std::string, OpenDriveEdge*>::iterator i = edges.begin(); i != edges.end(); ++i) {
        OpenDriveEdge& e = *(*i).second;
#ifdef DEBUG_VARIABLE_SPEED
        if (DEBUG_COND(&e)) {
            gDebugFlag1 = true;
            std::cout << "revisitLaneSections e=" << e.id << "\n";
        }
#endif
        std::vector<OpenDriveLaneSection>& laneSections = e.laneSections;
        // split by speed limits
        std::vector<OpenDriveLaneSection> newSections;
        for (std::vector<OpenDriveLaneSection>::iterator j = laneSections.begin(); j != laneSections.end(); ++j) {
            std::vector<OpenDriveLaneSection> splitSections;
            bool splitBySpeed = (*j).buildSpeedChanges(tc, splitSections);
            if (!splitBySpeed) {
                newSections.push_back(*j);
            } else {
                std::copy(splitSections.begin(), splitSections.end(), back_inserter(newSections));
            }
        }

        e.laneSections = newSections;
        laneSections = e.laneSections;
        double lastS = -1;
        // check whether the lane sections are in the right order
        bool sorted = true;
        for (std::vector<OpenDriveLaneSection>::const_iterator j = laneSections.begin(); j != laneSections.end() && sorted; ++j) {
            if ((*j).s <= lastS) {
                sorted = false;
            }
            lastS = (*j).s;
        }
        if (!sorted) {
            WRITE_WARNING("The sections of edge '" + e.id + "' are not sorted properly.");
            sort(e.laneSections.begin(), e.laneSections.end(), sections_by_s_sorter());
        }
        // check whether no duplicates of s-value occure
        lastS = -1;
        laneSections = e.laneSections;
        for (std::vector<OpenDriveLaneSection>::iterator j = laneSections.begin(); j != laneSections.end();) {
            bool simlarToLast = fabs((*j).s - lastS) < POSITION_EPS;
            lastS = (*j).s;
            // keep all lane sections for connecting roads because they are
            // needed to establish connectivity (laneSectionsConnected)
            if (simlarToLast && !e.isInner) {
                WRITE_WARNING("Almost duplicate s-value '" + toString(lastS) + "' for lane sections occurred at edge '" + e.id + "'; second entry was removed.");
                j = laneSections.erase(j);
            } else {
                ++j;
            }
        }
#ifdef DEBUG_VARIABLE_SPEED
        gDebugFlag1 = false;
#endif
    }
}


PositionVector
NIImporter_OpenDrive::geomFromLine(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution) {
    UNUSED_PARAMETER(e);
    PositionVector ret;
    Position start(g.x, g.y);
    Position end = calculateStraightEndPoint(g.hdg, g.length, start);
    if (resolution > 0 && g.length > 0) {
        const int numPoints = (int)ceil(g.length / resolution) + 1;
        double dx = (end.x() - start.x()) / (numPoints - 1);
        double dy = (end.y() - start.y()) / (numPoints - 1);
        for (int i = 0; i < numPoints; i++) {
            ret.push_back(Position(g.x + i * dx, g.y + i * dy));
        }
    } else {
        ret.push_back(start);
        ret.push_back(end);
    }
    return ret;
}


PositionVector
NIImporter_OpenDrive::geomFromSpiral(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution) {
    UNUSED_PARAMETER(e);
    PositionVector ret;
    double curveStart = g.params[0];
    double curveEnd = g.params[1];
    try {
        double cDot = (curveEnd - curveStart) / g.length;
        if (cDot == 0 || g.length == 0) {
            WRITE_WARNINGF("Could not compute spiral geometry for edge '%' (cDot=% length=%).", e.id, toString(cDot), toString(g.length));
            ret.push_back(Position(g.x, g.y));
            return ret;
        }
        double sStart = curveStart / cDot;
        double sEnd = curveEnd / cDot;
        double x = 0;
        double y = 0;
        double t = 0;
        double tStart = 0;
        double s;
        odrSpiral(sStart, cDot, &x, &y, &tStart);
        for (s = sStart; s <= sEnd; s += resolution) {
            odrSpiral(s, cDot, &x, &y, &t);
            ret.push_back(Position(x, y));
        }
        if (s != sEnd /*&& ret.size() == 1*/) {
            odrSpiral(sEnd, cDot, &x, &y, &t);
            ret.push_back(Position(x, y));
        }
        //if (s != sEnd && ret.size() > 2) {
        //    ret.pop_back();
        //}
        assert(ret.size() >= 2);
        assert(ret[0] != ret[1]);
        // shift start to coordinate origin
        PositionVector ret1 = ret;
        ret.add(ret.front() * -1);
        // rotate
        PositionVector ret2 = ret;
        ret.rotate2D(g.hdg - tStart);
#ifdef DEBUG_SPIRAL
        std::cout
                << std::setprecision(4)
                << "edge=" << e.id << " s=" << g.s
                << " cStart=" << curveStart
                << " cEnd=" << curveEnd
                << " cDot=" << cDot
                << " sStart=" << sStart
                << " sEnd=" << sEnd
                << " g.hdg=" << GeomHelper::naviDegree(g.hdg)
                << " tStart=" << GeomHelper::naviDegree(tStart)
                << "\n  beforeShift=" << ret1
                << "\n  beforeRot=" << ret2
                << "\n";
#endif
        // shift to geometry start
        ret.add(g.x, g.y, 0);
    } catch (const std::runtime_error& error) {
        WRITE_WARNING("Could not compute spiral geometry for edge '" + e.id + "' (" + error.what() + ").");
        ret.push_back(Position(g.x, g.y));
    }
    return ret.getSubpart2D(0, g.length);
}


PositionVector
NIImporter_OpenDrive::geomFromArc(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution) {
    UNUSED_PARAMETER(e);
    PositionVector ret;
    double dist = 0.0;
    double centerX = g.x;
    double centerY = g.y;
    // left: positive value
    double curvature = g.params[0];
    double radius = 1. / curvature;
    // center point
    calculateCurveCenter(&centerX, &centerY, radius, g.hdg);
    double endX = g.x;
    double endY = g.y;
    double startX = g.x;
    double startY = g.y;
    double geo_posS = g.s;
    double geo_posE = g.s;
    bool end = false;
    do {
        geo_posE += resolution;
        if (geo_posE - g.s > g.length) {
            geo_posE = g.s + g.length;
        }
        if (geo_posE - g.s > g.length) {
            geo_posE = g.s + g.length;
        }
        calcPointOnCurve(&endX, &endY, centerX, centerY, radius, geo_posE - geo_posS);

        dist += (geo_posE - geo_posS);
        //
        ret.push_back(Position(startX, startY));
        //
        startX = endX;
        startY = endY;
        geo_posS = geo_posE;

        if (geo_posE  - (g.s + g.length) < 0.001 && geo_posE  - (g.s + g.length) > -0.001) {
            end = true;
        }
    } while (!end);
    return ret.getSubpart2D(0, g.length);
}


PositionVector
NIImporter_OpenDrive::geomFromPoly(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution) {
    UNUSED_PARAMETER(e);
    const double s = sin(g.hdg);
    const double c = cos(g.hdg);
    PositionVector ret;
    for (double off = 0; off < g.length + 2.; off += resolution) {
        double x = off;
        double y = g.params[0] + g.params[1] * off + g.params[2] * pow(off, 2.) + g.params[3] * pow(off, 3.);
        double xnew = x * c - y * s;
        double ynew = x * s + y * c;
        ret.push_back(Position(g.x + xnew, g.y + ynew));
    }
    return ret.getSubpart2D(0, g.length);
}


PositionVector
NIImporter_OpenDrive::geomFromParamPoly(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution) {
    UNUSED_PARAMETER(e);
    const double s = sin(g.hdg);
    const double c = cos(g.hdg);
    const double pMax = g.params[8] <= 0 ? g.length : g.params[8];
    const double pStep = pMax / ceil(g.length / resolution);
    PositionVector ret;
    for (double p = 0; p <= pMax + pStep; p += pStep) {
        double x = g.params[0] + g.params[1] * p + g.params[2] * pow(p, 2.) + g.params[3] * pow(p, 3.);
        double y = g.params[4] + g.params[5] * p + g.params[6] * pow(p, 2.) + g.params[7] * pow(p, 3.);
        double xnew = x * c - y * s;
        double ynew = x * s + y * c;
        ret.push_back(Position(g.x + xnew, g.y + ynew));
    }
    return ret.getSubpart2D(0, g.length);
}


Position
NIImporter_OpenDrive::calculateStraightEndPoint(double hdg, double length, const Position& start) {
    double normx = 1.0f;
    double normy = 0.0f;
    double x2 = normx * cos(hdg) - normy * sin(hdg);
    double y2 = normx * sin(hdg) + normy * cos(hdg);
    normx = x2 * length;
    normy = y2 * length;
    return Position(start.x() + normx, start.y() + normy);
}


void
NIImporter_OpenDrive::calculateCurveCenter(double* ad_x, double* ad_y, double ad_radius, double ad_hdg) {
    double normX = 1.0;
    double normY = 0.0;
    double tmpX;
    double turn;
    if (ad_radius > 0) {
        turn = -1.0;
    } else {
        turn = 1.0;
    }

    tmpX = normX;
    normX = normX * cos(ad_hdg) + normY * sin(ad_hdg);
    normY = tmpX * sin(ad_hdg) + normY * cos(ad_hdg);

    tmpX = normX;
    normX = turn * normY;
    normY = -turn * tmpX;

    normX = fabs(ad_radius) * normX;
    normY = fabs(ad_radius) * normY;

    *ad_x += normX;
    *ad_y += normY;
}


void
NIImporter_OpenDrive::calcPointOnCurve(double* ad_x, double* ad_y, double ad_centerX, double ad_centerY,
                                       double ad_r, double ad_length) {
    double rotAngle = ad_length / fabs(ad_r);
    double vx = *ad_x - ad_centerX;
    double vy = *ad_y - ad_centerY;
    double tmpx;

    double turn;
    if (ad_r > 0) {
        turn = -1;    //left
    } else {
        turn = 1;    //right
    }
    tmpx = vx;
    vx = vx * cos(rotAngle) + turn * vy * sin(rotAngle);
    vy = -1 * turn * tmpx * sin(rotAngle) + vy * cos(rotAngle);
    *ad_x = vx + ad_centerX;
    *ad_y = vy + ad_centerY;
}


// ---------------------------------------------------------------------------
// section
// ---------------------------------------------------------------------------
NIImporter_OpenDrive::OpenDriveLaneSection::OpenDriveLaneSection(double sArg) : s(sArg), sOrig(sArg) {
    lanesByDir[OPENDRIVE_TAG_LEFT] = std::vector<OpenDriveLane>();
    lanesByDir[OPENDRIVE_TAG_RIGHT] = std::vector<OpenDriveLane>();
    lanesByDir[OPENDRIVE_TAG_CENTER] = std::vector<OpenDriveLane>();
}


void
NIImporter_OpenDrive::OpenDriveLaneSection::buildLaneMapping(const NBTypeCont& tc) {
    int sumoLane = 0;
    bool singleType = true;
    std::vector<std::string> types;
    const std::vector<OpenDriveLane>& dirLanesR = lanesByDir.find(OPENDRIVE_TAG_RIGHT)->second;
    for (std::vector<OpenDriveLane>::const_reverse_iterator i = dirLanesR.rbegin(); i != dirLanesR.rend(); ++i) {
        if (myImportAllTypes || (tc.knows((*i).type) && !tc.getShallBeDiscarded((*i).type))) {
            laneMap[(*i).id] = sumoLane++;
            types.push_back((*i).type);
            if (types.front() != types.back()) {
                singleType = false;
            }
        }
    }
    rightLaneNumber = sumoLane;
    rightType = sumoLane > 0 ? (singleType ? types.front() : joinToString(types, "|")) : "";
    sumoLane = 0;
    singleType = true;
    types.clear();
    const std::vector<OpenDriveLane>& dirLanesL = lanesByDir.find(OPENDRIVE_TAG_LEFT)->second;
    for (std::vector<OpenDriveLane>::const_iterator i = dirLanesL.begin(); i != dirLanesL.end(); ++i) {
        if (myImportAllTypes || (tc.knows((*i).type) && !tc.getShallBeDiscarded((*i).type))) {
            laneMap[(*i).id] = sumoLane++;
            types.push_back((*i).type);
            if (types.front() != types.back()) {
                singleType = false;
            }
        }
    }
    leftLaneNumber = sumoLane;
    leftType = sumoLane > 0 ? (singleType ? types.front() : joinToString(types, "|")) : "";
}


std::map<int, int>
NIImporter_OpenDrive::OpenDriveLaneSection::getInnerConnections(OpenDriveXMLTag dir, const OpenDriveLaneSection& prev) {
    std::map<int, int> ret;
    const std::vector<OpenDriveLane>& dirLanes = lanesByDir.find(dir)->second;
    for (std::vector<OpenDriveLane>::const_reverse_iterator i = dirLanes.rbegin(); i != dirLanes.rend(); ++i) {
        std::map<int, int>::const_iterator toP = laneMap.find((*i).id);
        if (toP == laneMap.end()) {
            // the current lane is not available in SUMO
            continue;
        }
        int to = (*toP).second;
        int from = UNSET_CONNECTION;
        if ((*i).predecessor != UNSET_CONNECTION) {
            from = (*i).predecessor;
        }
        if (from != UNSET_CONNECTION) {
            std::map<int, int>::const_iterator fromP = prev.laneMap.find(from);
            if (fromP != prev.laneMap.end()) {
                from = (*fromP).second;
            } else {
                from = UNSET_CONNECTION;
            }
        }
        if (from != UNSET_CONNECTION && to != UNSET_CONNECTION) {
            if (ret.find(from) != ret.end()) {
//        WRITE_WARNING("double connection");
            }
            if (dir == OPENDRIVE_TAG_LEFT) {
                std::swap(from, to);
            }
            ret[from] = to;
        } else {
//      WRITE_WARNING("missing connection");
        }
    }
    return ret;
}


NIImporter_OpenDrive::OpenDriveLaneSection
NIImporter_OpenDrive::OpenDriveLaneSection::buildLaneSection(double startPos) {
    OpenDriveLaneSection ret(*this);
    ret.s += startPos;
    for (int k = 0; k < (int)ret.lanesByDir[OPENDRIVE_TAG_RIGHT].size(); ++k) {
        OpenDriveLane& l = ret.lanesByDir[OPENDRIVE_TAG_RIGHT][k];
        l.speed = 0;
        std::vector<std::pair<double, double> >::const_iterator i = std::find_if(l.speeds.begin(), l.speeds.end(), same_position_finder(startPos));
        if (i != l.speeds.end()) {
            l.speed = (*i).second;
        }
    }
    for (int k = 0; k < (int)ret.lanesByDir[OPENDRIVE_TAG_LEFT].size(); ++k) {
        OpenDriveLane& l = ret.lanesByDir[OPENDRIVE_TAG_LEFT][k];
        std::vector<std::pair<double, double> >::const_iterator i = std::find_if(l.speeds.begin(), l.speeds.end(), same_position_finder(startPos));
        l.speed = 0;
        if (i != l.speeds.end()) {
            l.speed = (*i).second;
        }
    }
    return ret;
}


bool
NIImporter_OpenDrive::OpenDriveLaneSection::buildSpeedChanges(const NBTypeCont& tc, std::vector<OpenDriveLaneSection>& newSections) {
    std::set<double> speedChangePositions;
    // collect speed change positions and apply initial speed to the begin
    for (std::vector<OpenDriveLane>::iterator k = lanesByDir[OPENDRIVE_TAG_RIGHT].begin(); k != lanesByDir[OPENDRIVE_TAG_RIGHT].end(); ++k) {
        for (std::vector<std::pair<double, double> >::const_iterator l = (*k).speeds.begin(); l != (*k).speeds.end(); ++l) {
            speedChangePositions.insert((*l).first);
            if ((*l).first == 0) {
                (*k).speed = (*l).second;
            }
        }
    }
    for (std::vector<OpenDriveLane>::iterator k = lanesByDir[OPENDRIVE_TAG_LEFT].begin(); k != lanesByDir[OPENDRIVE_TAG_LEFT].end(); ++k) {
        for (std::vector<std::pair<double, double> >::const_iterator l = (*k).speeds.begin(); l != (*k).speeds.end(); ++l) {
            speedChangePositions.insert((*l).first);
            if ((*l).first == 0) {
                (*k).speed = (*l).second;
            }
        }
    }
    // do nothing if there is none
    if (speedChangePositions.size() == 0) {
        return false;
    }
    if (*speedChangePositions.begin() > 0) {
        speedChangePositions.insert(0);
    }
#ifdef DEBUG_VARIABLE_SPEED
    if (gDebugFlag1) std::cout
                << "  buildSpeedChanges sectionStart=" << s
                << " speedChangePositions=" << joinToString(speedChangePositions, ", ")
                << "\n";
#endif
    for (std::set<double>::iterator i = speedChangePositions.begin(); i != speedChangePositions.end(); ++i) {
        if (i == speedChangePositions.begin()) {
            newSections.push_back(*this);
        } else {
            newSections.push_back(buildLaneSection(*i));
        }
    }
    // propagate speeds
    for (int i = 0; i != (int)newSections.size(); ++i) {
        OpenDriveLaneSection& ls = newSections[i];
        std::map<OpenDriveXMLTag, std::vector<OpenDriveLane> >& lanesByDir = ls.lanesByDir;
        for (std::map<OpenDriveXMLTag, std::vector<OpenDriveLane> >::iterator k = lanesByDir.begin(); k != lanesByDir.end(); ++k) {
            std::vector<OpenDriveLane>& lanes = (*k).second;
            for (int j = 0; j != (int)lanes.size(); ++j) {
                OpenDriveLane& l = lanes[j];
                if (l.speed != 0) {
                    continue;
                }
                if (i > 0) {
                    l.speed = newSections[i - 1].lanesByDir[(*k).first][j].speed;
                } else {
                    tc.getSpeed(l.type);
                }
            }
        }
    }
    return true;
}



// ---------------------------------------------------------------------------
// edge
// ---------------------------------------------------------------------------
int
NIImporter_OpenDrive::OpenDriveEdge::getPriority(OpenDriveXMLTag dir) const {
    // for signal interpretations see https://de.wikipedia.org/wiki/Bildtafel_der_Verkehrszeichen_in_der_Bundesrepublik_Deutschland_seit_2013
    int prio = 1;
    for (std::vector<OpenDriveSignal>::const_iterator i = signals.begin(); i != signals.end(); ++i) {
        int tmp = 1;
        if ((*i).type == "301" || (*i).type == "306") { // priority road or local priority
            tmp = 2;
        }
        if ((*i).type == "205" /*|| (*i).type == "206"*/) { // yield or stop
            tmp = 0;
        }
        if (tmp != 1 && dir == OPENDRIVE_TAG_RIGHT && (*i).orientation > 0) {
            prio = tmp;
        }
        if (tmp != 1 && dir == OPENDRIVE_TAG_LEFT && (*i).orientation < 0) {
            prio = tmp;
        }

    }
    return prio;
}



// ---------------------------------------------------------------------------
// loader methods
// ---------------------------------------------------------------------------
NIImporter_OpenDrive::NIImporter_OpenDrive(const NBTypeCont& tc, std::map<std::string, OpenDriveEdge*>& edges)
    : GenericSAXHandler(openDriveTags, OPENDRIVE_TAG_NOTHING, openDriveAttrs, OPENDRIVE_ATTR_NOTHING, "opendrive"),
      myTypeContainer(tc), myCurrentEdge("", "", "", -1), myEdges(edges) {
}


NIImporter_OpenDrive::~NIImporter_OpenDrive() {
}


void
NIImporter_OpenDrive::myStartElement(int element,
                                     const SUMOSAXAttributes& attrs) {
    bool ok = true;
    switch (element) {
        case OPENDRIVE_TAG_HEADER: {
            /*
            int majorVersion = attrs.get<int>(OPENDRIVE_ATTR_REVMAJOR, nullptr, ok);
            int minorVersion = attrs.get<int>(OPENDRIVE_ATTR_REVMINOR, nullptr, ok);
            if (majorVersion != 1 || minorVersion != 2) {
                // TODO: leave note of exceptions
                WRITE_WARNING("Given openDrive file '" + getFileName() + "' uses version " + toString(majorVersion) + "." + toString(minorVersion) + ";\n Version 1.2 is supported.");
            }
            */
        }
        break;
        case OPENDRIVE_TAG_ROAD: {
            std::string id = attrs.get<std::string>(OPENDRIVE_ATTR_ID, nullptr, ok);
            std::string streetName = attrs.getOpt<std::string>(OPENDRIVE_ATTR_NAME, nullptr, ok, "", false);
            std::string junction = attrs.get<std::string>(OPENDRIVE_ATTR_JUNCTION, id.c_str(), ok);
            double length = attrs.get<double>(OPENDRIVE_ATTR_LENGTH, id.c_str(), ok);
            myCurrentEdge = OpenDriveEdge(id, streetName, junction, length);
        }
        break;
        case OPENDRIVE_TAG_PREDECESSOR: {
            if (myElementStack.size() >= 2 && myElementStack[myElementStack.size() - 2] == OPENDRIVE_TAG_ROAD) {
                std::string elementType = attrs.get<std::string>(OPENDRIVE_ATTR_ELEMENTTYPE, myCurrentEdge.id.c_str(), ok);
                std::string elementID = attrs.get<std::string>(OPENDRIVE_ATTR_ELEMENTID, myCurrentEdge.id.c_str(), ok);
                std::string contactPoint = attrs.hasAttribute(OPENDRIVE_ATTR_CONTACTPOINT)
                                           ? attrs.get<std::string>(OPENDRIVE_ATTR_CONTACTPOINT, myCurrentEdge.id.c_str(), ok)
                                           : "end";
                addLink(OPENDRIVE_LT_PREDECESSOR, elementType, elementID, contactPoint);
            }
            if (myElementStack.size() >= 2 && myElementStack[myElementStack.size() - 2] == OPENDRIVE_TAG_LANE) {
                int no = attrs.get<int>(OPENDRIVE_ATTR_ID, myCurrentEdge.id.c_str(), ok);
                OpenDriveLane& l = myCurrentEdge.laneSections.back().lanesByDir[myCurrentLaneDirection].back();
                l.predecessor = no;
            }
        }
        break;
        case OPENDRIVE_TAG_SUCCESSOR: {
            if (myElementStack.size() >= 2 && myElementStack[myElementStack.size() - 2] == OPENDRIVE_TAG_ROAD) {
                std::string elementType = attrs.get<std::string>(OPENDRIVE_ATTR_ELEMENTTYPE, myCurrentEdge.id.c_str(), ok);
                std::string elementID = attrs.get<std::string>(OPENDRIVE_ATTR_ELEMENTID, myCurrentEdge.id.c_str(), ok);
                std::string contactPoint = attrs.hasAttribute(OPENDRIVE_ATTR_CONTACTPOINT)
                                           ? attrs.get<std::string>(OPENDRIVE_ATTR_CONTACTPOINT, myCurrentEdge.id.c_str(), ok)
                                           : "start";
                addLink(OPENDRIVE_LT_SUCCESSOR, elementType, elementID, contactPoint);
            }
            if (myElementStack.size() >= 2 && myElementStack[myElementStack.size() - 2] == OPENDRIVE_TAG_LANE) {
                int no = attrs.get<int>(OPENDRIVE_ATTR_ID, myCurrentEdge.id.c_str(), ok);
                OpenDriveLane& l = myCurrentEdge.laneSections.back().lanesByDir[myCurrentLaneDirection].back();
                l.successor = no;
            }
        }
        break;
        case OPENDRIVE_TAG_GEOMETRY: {
            double length = attrs.get<double>(OPENDRIVE_ATTR_LENGTH, myCurrentEdge.id.c_str(), ok);
            double s = attrs.get<double>(OPENDRIVE_ATTR_S, myCurrentEdge.id.c_str(), ok);
            double x = attrs.get<double>(OPENDRIVE_ATTR_X, myCurrentEdge.id.c_str(), ok);
            double y = attrs.get<double>(OPENDRIVE_ATTR_Y, myCurrentEdge.id.c_str(), ok);
            double hdg = attrs.get<double>(OPENDRIVE_ATTR_HDG, myCurrentEdge.id.c_str(), ok);
            myCurrentEdge.geometries.push_back(OpenDriveGeometry(length, s, x, y, hdg));
        }
        break;
        case OPENDRIVE_TAG_ELEVATION: {
            double s = attrs.get<double>(OPENDRIVE_ATTR_S, myCurrentEdge.id.c_str(), ok);
            double a = attrs.get<double>(OPENDRIVE_ATTR_A, myCurrentEdge.id.c_str(), ok);
            double b = attrs.get<double>(OPENDRIVE_ATTR_B, myCurrentEdge.id.c_str(), ok);
            double c = attrs.get<double>(OPENDRIVE_ATTR_C, myCurrentEdge.id.c_str(), ok);
            double d = attrs.get<double>(OPENDRIVE_ATTR_D, myCurrentEdge.id.c_str(), ok);
            myCurrentEdge.elevations.push_back(OpenDriveElevation(s, a, b, c, d));
        }
        break;
        case OPENDRIVE_TAG_LINE: {
            if (myElementStack.size() > 0 && myElementStack.back() == OPENDRIVE_TAG_GEOMETRY) {
                std::vector<double> vals;
                addGeometryShape(OPENDRIVE_GT_LINE, vals);
            }
        }
        break;
        case OPENDRIVE_TAG_SPIRAL: {
            std::vector<double> vals;
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_CURVSTART, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_CURVEND, myCurrentEdge.id.c_str(), ok));
            addGeometryShape(OPENDRIVE_GT_SPIRAL, vals);
        }
        break;
        case OPENDRIVE_TAG_ARC: {
            std::vector<double> vals;
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_CURVATURE, myCurrentEdge.id.c_str(), ok));
            addGeometryShape(OPENDRIVE_GT_ARC, vals);
        }
        break;
        case OPENDRIVE_TAG_POLY3: {
            std::vector<double> vals;
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_A, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_B, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_C, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_D, myCurrentEdge.id.c_str(), ok));
            addGeometryShape(OPENDRIVE_GT_POLY3, vals);
        }
        break;
        case OPENDRIVE_TAG_PARAMPOLY3: {
            std::vector<double> vals;
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_AU, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_BU, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_CU, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_DU, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_AV, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_BV, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_CV, myCurrentEdge.id.c_str(), ok));
            vals.push_back(attrs.get<double>(OPENDRIVE_ATTR_DV, myCurrentEdge.id.c_str(), ok));
            const std::string pRange = attrs.getOpt<std::string>(OPENDRIVE_ATTR_PRANGE, myCurrentEdge.id.c_str(), ok, "normalized", false);
            if (pRange == "normalized") {
                vals.push_back(1.0);
            } else if (pRange == "arcLength") {
                vals.push_back(-1.0);
            } else {
                WRITE_WARNING("Ignoring invalid pRange value '" + pRange + "' for road '" + myCurrentEdge.id + "'.");
                vals.push_back(1.0);
            }
            addGeometryShape(OPENDRIVE_GT_PARAMPOLY3, vals);
        }
        break;
        case OPENDRIVE_TAG_LANESECTION: {
            double s = attrs.get<double>(OPENDRIVE_ATTR_S, myCurrentEdge.id.c_str(), ok);
            if (myCurrentEdge.laneSections.size() > 0) {
                myCurrentEdge.laneSections.back().length = s - myCurrentEdge.laneSections.back().s;
            }
            myCurrentEdge.laneSections.push_back(OpenDriveLaneSection(s));
            // possibly updated by the next laneSection
            myCurrentEdge.laneSections.back().length = myCurrentEdge.length - s;
        }
        break;
        case OPENDRIVE_TAG_LANEOFFSET: {
            double s = attrs.get<double>(OPENDRIVE_ATTR_S, myCurrentEdge.id.c_str(), ok);
            double a = attrs.get<double>(OPENDRIVE_ATTR_A, myCurrentEdge.id.c_str(), ok);
            double b = attrs.get<double>(OPENDRIVE_ATTR_B, myCurrentEdge.id.c_str(), ok);
            double c = attrs.get<double>(OPENDRIVE_ATTR_C, myCurrentEdge.id.c_str(), ok);
            double d = attrs.get<double>(OPENDRIVE_ATTR_D, myCurrentEdge.id.c_str(), ok);
            myCurrentEdge.offsets.push_back(OpenDriveLaneOffset(s, a, b, c, d));
        }
        break;
        case OPENDRIVE_TAG_LEFT:
            myCurrentLaneDirection = OPENDRIVE_TAG_LEFT;
            break;
        case OPENDRIVE_TAG_CENTER:
            myCurrentLaneDirection = OPENDRIVE_TAG_CENTER;
            break;
        case OPENDRIVE_TAG_RIGHT:
            myCurrentLaneDirection = OPENDRIVE_TAG_RIGHT;
            break;
        case OPENDRIVE_TAG_LANE: {
            std::string type = attrs.get<std::string>(OPENDRIVE_ATTR_TYPE, myCurrentEdge.id.c_str(), ok);
            int id = attrs.get<int>(OPENDRIVE_ATTR_ID, myCurrentEdge.id.c_str(), ok);
            std::string level = attrs.hasAttribute(OPENDRIVE_ATTR_LEVEL)
                                ? attrs.get<std::string>(OPENDRIVE_ATTR_LEVEL, myCurrentEdge.id.c_str(), ok)
                                : "";
            OpenDriveLaneSection& ls = myCurrentEdge.laneSections.back();
            ls.lanesByDir[myCurrentLaneDirection].push_back(OpenDriveLane(id, level, type));
        }
        break;
        case OPENDRIVE_TAG_SIGNAL: {
            std::string id = attrs.get<std::string>(OPENDRIVE_ATTR_ID, myCurrentEdge.id.c_str(), ok);
            std::string type = attrs.get<std::string>(OPENDRIVE_ATTR_TYPE, myCurrentEdge.id.c_str(), ok);
            std::string name = attrs.getOpt<std::string>(OPENDRIVE_ATTR_NAME, myCurrentEdge.id.c_str(), ok, "", false);
            const std::string orientation = attrs.get<std::string>(OPENDRIVE_ATTR_ORIENTATION, myCurrentEdge.id.c_str(), ok);
            int orientationCode = orientation == "-" ? -1 : orientation == "+" ? 1 : 0;
            double s = attrs.get<double>(OPENDRIVE_ATTR_S, myCurrentEdge.id.c_str(), ok);
            bool dynamic = attrs.get<std::string>(OPENDRIVE_ATTR_DYNAMIC, myCurrentEdge.id.c_str(), ok) == "no" ? false : true;
            OpenDriveSignal signal = OpenDriveSignal(id, type, name, orientationCode, dynamic, s);
            myCurrentEdge.signals.push_back(signal);
            mySignals[id] = signal;
        }
        break;
        case OPENDRIVE_TAG_SIGNALREFERENCE: {
            std::string id = attrs.get<std::string>(OPENDRIVE_ATTR_ID, myCurrentEdge.id.c_str(), ok);
            const std::string orientation = attrs.get<std::string>(OPENDRIVE_ATTR_ORIENTATION, myCurrentEdge.id.c_str(), ok);
            int orientationCode = orientation == "-" ? -1 : orientation == "+" ? 1 : 0;
            double s = attrs.get<double>(OPENDRIVE_ATTR_S, myCurrentEdge.id.c_str(), ok);
            OpenDriveSignal signal = OpenDriveSignal(id, "", "", orientationCode, false, s);
            myCurrentEdge.signals.push_back(signal);
        }
        break;
        case OPENDRIVE_TAG_VALIDITY: {
            int fromLane = attrs.get<int>(OPENDRIVE_ATTR_FROMLANE, myCurrentEdge.id.c_str(), ok);
            int toLane = attrs.get<int>(OPENDRIVE_ATTR_TOLANE, myCurrentEdge.id.c_str(), ok);
            myCurrentEdge.signals.back().minLane = fromLane;
            myCurrentEdge.signals.back().maxLane = toLane;
        }
        break;
        case OPENDRIVE_TAG_JUNCTION:
            myCurrentJunctionID = attrs.get<std::string>(OPENDRIVE_ATTR_ID, myCurrentJunctionID.c_str(), ok);
            break;
        case OPENDRIVE_TAG_CONNECTION: {
            std::string id = attrs.get<std::string>(OPENDRIVE_ATTR_ID, myCurrentJunctionID.c_str(), ok);
            myCurrentIncomingRoad = attrs.get<std::string>(OPENDRIVE_ATTR_INCOMINGROAD, myCurrentJunctionID.c_str(), ok);
            myCurrentConnectingRoad = attrs.get<std::string>(OPENDRIVE_ATTR_CONNECTINGROAD, myCurrentJunctionID.c_str(), ok);
            std::string cp = attrs.get<std::string>(OPENDRIVE_ATTR_CONTACTPOINT, myCurrentJunctionID.c_str(), ok);
            myCurrentContactPoint = cp == "start" ? OPENDRIVE_CP_START : OPENDRIVE_CP_END;
            myConnectionWasEmpty = true;
        }
        break;
        case OPENDRIVE_TAG_LANELINK: {
            int from = attrs.get<int>(OPENDRIVE_ATTR_FROM, myCurrentJunctionID.c_str(), ok);
            int to = attrs.get<int>(OPENDRIVE_ATTR_TO, myCurrentJunctionID.c_str(), ok);
            Connection c;
            c.fromEdge = myCurrentIncomingRoad;
            c.toEdge = myCurrentConnectingRoad;
            c.fromLane = from;
            c.toLane = to;
            c.fromCP = OPENDRIVE_CP_END;
            c.toCP = myCurrentContactPoint;
            c.all = false;
            if (myEdges.find(c.fromEdge) == myEdges.end()) {
                WRITE_ERROR("In laneLink-element: incoming road '" + c.fromEdge + "' is not known.");
            } else {
                OpenDriveEdge* e = myEdges.find(c.fromEdge)->second;
                e->connections.insert(c);
                myConnectionWasEmpty = false;
            }
        }
        break;
        case OPENDRIVE_TAG_WIDTH: {
            if (myElementStack.size() >= 2 && myElementStack[myElementStack.size() - 1] == OPENDRIVE_TAG_LANE) {
                const double s = attrs.get<double>(OPENDRIVE_ATTR_SOFFSET, myCurrentEdge.id.c_str(), ok);
                const double a = attrs.get<double>(OPENDRIVE_ATTR_A, myCurrentEdge.id.c_str(), ok);
                const double b = attrs.get<double>(OPENDRIVE_ATTR_B, myCurrentEdge.id.c_str(), ok);
                const double c = attrs.get<double>(OPENDRIVE_ATTR_C, myCurrentEdge.id.c_str(), ok);
                const double d = attrs.get<double>(OPENDRIVE_ATTR_D, myCurrentEdge.id.c_str(), ok);
                OpenDriveLane& l = myCurrentEdge.laneSections.back().lanesByDir[myCurrentLaneDirection].back();
                l.width = MAX2(l.width, a);
                l.widthData.push_back(OpenDriveWidth(s, a, b, c, d));
#ifdef DEBUG_VARIABLE_WIDTHS
                if (DEBUG_COND(&myCurrentEdge)) {
                    std::cout << " road=" << myCurrentEdge.id
                              << std::setprecision(gPrecision)
                              << " junction=" << myCurrentEdge.junction
                              << " section=" << myCurrentEdge.laneSections.size() - 1
                              << " dir=" << myCurrentLaneDirection << " lane=" << l.id
                              << " type=" << l.type
                              << " width=" << l.width
                              << " a=" << a
                              << " b=" << b
                              << " c=" << c
                              << " d=" << d
                              << " s=" << s
                              << " entries=" << l.widthData.size()
                              << "\n";
                }
#endif
            }
        }
        break;
        case OPENDRIVE_TAG_SPEED: {
            if (myElementStack.size() >= 2 && myElementStack[myElementStack.size() - 1] == OPENDRIVE_TAG_LANE) {
                double speed = attrs.get<double>(OPENDRIVE_ATTR_MAX, myCurrentEdge.id.c_str(), ok);
                double pos = attrs.get<double>(OPENDRIVE_ATTR_SOFFSET, myCurrentEdge.id.c_str(), ok);
                // required for xodr v1.4
                const std::string unit = attrs.getOpt<std::string>(OPENDRIVE_ATTR_UNIT, myCurrentEdge.id.c_str(), ok, "", false);
                // now convert the speed to reasonable default SI [m/s]
                if (!unit.empty()) {
                    // something to be done at all ?
                    if (unit == "km/h") {
                        speed /= 3.6;
                    }
                    if (unit == "mph") {
                        speed *= 1.609344 / 3.6;
                    }
                    // IGNORING unknown units.
                }
                myCurrentEdge.laneSections.back().lanesByDir[myCurrentLaneDirection].back().speeds.push_back(std::make_pair(pos, speed));
            }
        }
        break;
        case OPENDRIVE_TAG_OBJECT: {
            if (!attrs.hasAttribute(OPENDRIVE_ATTR_ID)) {
                WRITE_WARNING("Ignoring object without id at edge '" + toString(myCurrentEdge.id) + "'.");
                break;
            }
            OpenDriveObject o;
            o.id = attrs.get<std::string>(OPENDRIVE_ATTR_ID, 0, ok);
            o.type = attrs.getOpt<std::string>(OPENDRIVE_ATTR_TYPE, o.id.c_str(), ok, "", false);
            o.name = attrs.getOpt<std::string>(OPENDRIVE_ATTR_NAME, o.id.c_str(), ok, "", false);
            o.s = attrs.get<double>(OPENDRIVE_ATTR_S, o.id.c_str(), ok);
            o.t = attrs.get<double>(OPENDRIVE_ATTR_T, o.id.c_str(), ok);
            o.width = attrs.getOpt<double>(OPENDRIVE_ATTR_WIDTH, o.id.c_str(), ok, -1);
            o.length = attrs.getOpt<double>(OPENDRIVE_ATTR_LENGTH, o.id.c_str(), ok, -1);
            o.radius = attrs.getOpt<double>(OPENDRIVE_ATTR_RADIUS, o.id.c_str(), ok, -1);
            o.hdg = attrs.getOpt<double>(OPENDRIVE_ATTR_HDG, o.id.c_str(), ok, 0);
            myCurrentEdge.objects.push_back(o);
        }
        break;
        case OPENDRIVE_TAG_REPEAT: {
            if (myCurrentEdge.objects.empty()) {
                WRITE_ERROR("Repeat without object at edge '" + toString(myCurrentEdge.id) + "'.");
                ok = false;
            } else {
                OpenDriveObject o = myCurrentEdge.objects.back();
                const std::string baseID = o.id;
                double dist = attrs.get<double>(OPENDRIVE_ATTR_DISTANCE, o.id.c_str(), ok);
                if (dist == 0) {
                    // continuous feature. Split into parts (XXX exmport as a single polygon #5235)
                    dist = OptionsCont::getOptions().getFloat("opendrive.curve-resolution");
                }

                myCurrentEdge.objects.pop_back();
                const double length = attrs.get<double>(OPENDRIVE_ATTR_LENGTH, o.id.c_str(), ok);
                o.s = attrs.getOpt<double>(OPENDRIVE_ATTR_S, o.id.c_str(), ok, o.s);
                double wStart = attrs.getOpt<double>(OPENDRIVE_ATTR_WIDTHSTART, o.id.c_str(), ok, o.width);
                double wEnd = attrs.getOpt<double>(OPENDRIVE_ATTR_WIDTHEND, o.id.c_str(), ok, o.width);
                double tStart = attrs.getOpt<double>(OPENDRIVE_ATTR_TSTART, o.id.c_str(), ok, o.t);
                double tEnd = attrs.getOpt<double>(OPENDRIVE_ATTR_TEND, o.id.c_str(), ok, o.t);
                int index = 0;
                for (double x = 0; x <= length + NUMERICAL_EPS; x += dist) {
                    o.id = baseID + "#" + toString(index++);
                    const double a = x / length;
                    o.width = wStart * (1 - a) + wEnd * a;
                    o.t = tStart * (1 - a) + tEnd * a;
                    myCurrentEdge.objects.push_back(o);
                    o.s += dist;
                }
            }
        }
        break;
        default:
            break;
    }
    myElementStack.push_back(element);
}


void
NIImporter_OpenDrive::myCharacters(int element, const std::string& cdata) {
    if (element == OPENDRIVE_TAG_GEOREFERENCE) {
        size_t i = cdata.find("+proj");
        if (i != std::string::npos) {
            const std::string proj = cdata.substr(i);
            if (proj != "") {
                GeoConvHelper* result = nullptr;
                Boundary convBoundary;
                Boundary origBoundary;
                Position networkOffset(0, 0);
                // XXX read values from the header
                convBoundary.add(Position(0, 0));
                origBoundary.add(Position(0, 0));
                try {
                    result = new GeoConvHelper(proj, networkOffset, origBoundary, convBoundary);
                    GeoConvHelper::setLoaded(*result);
                } catch (ProcessError& e) {
                    WRITE_ERROR("Could not set projection. (" + std::string(e.what()) + ")");
                }
            }
        } else {
            WRITE_WARNING("geoReference format '" + cdata + "' currently not supported");
        }
    }
}


void
NIImporter_OpenDrive::myEndElement(int element) {
    myElementStack.pop_back();
    switch (element) {
        case OPENDRIVE_TAG_ROAD:
            myEdges[myCurrentEdge.id] = new OpenDriveEdge(myCurrentEdge);
            break;
        case OPENDRIVE_TAG_CONNECTION:
            if (myConnectionWasEmpty) {
                Connection c;
                c.fromEdge = myCurrentIncomingRoad;
                c.toEdge = myCurrentConnectingRoad;
                c.fromLane = 0;
                c.toLane = 0;
                c.fromCP = OPENDRIVE_CP_END;
                c.toCP = myCurrentContactPoint;
                c.all = true;
                if (myEdges.find(c.fromEdge) == myEdges.end()) {
                    WRITE_ERROR("In laneLink-element: incoming road '" + c.fromEdge + "' is not known.");
                } else {
                    OpenDriveEdge* e = myEdges.find(c.fromEdge)->second;
                    e->connections.insert(c);
                }
            }
            break;
        case OPENDRIVE_TAG_LANESECTION: {
            myCurrentEdge.laneSections.back().buildLaneMapping(myTypeContainer);
        }
        break;
        default:
            break;
    }
}



void
NIImporter_OpenDrive::addLink(LinkType lt, const std::string& elementType,
                              const std::string& elementID,
                              const std::string& contactPoint) {
    OpenDriveLink l(lt, elementID);
    // elementType
    if (elementType == "road") {
        l.elementType = OPENDRIVE_ET_ROAD;
    } else if (elementType == "junction") {
        l.elementType = OPENDRIVE_ET_JUNCTION;
    }
    // contact point
    if (contactPoint == "start") {
        l.contactPoint = OPENDRIVE_CP_START;
    } else if (contactPoint == "end") {
        l.contactPoint = OPENDRIVE_CP_END;
    }
    // add
    myCurrentEdge.links.push_back(l);
}


void
NIImporter_OpenDrive::addGeometryShape(GeometryType type, const std::vector<double>& vals) {
    // checks
    if (myCurrentEdge.geometries.size() == 0) {
        throw ProcessError("Mismatching paranthesis in geometry definition for road '" + myCurrentEdge.id + "'");
    }
    OpenDriveGeometry& last = myCurrentEdge.geometries.back();
    if (last.type != OPENDRIVE_GT_UNKNOWN) {
        throw ProcessError("Double geometry information for road '" + myCurrentEdge.id + "'");
    }
    // set
    last.type = type;
    last.params = vals;
}


bool
operator<(const NIImporter_OpenDrive::Connection& c1, const NIImporter_OpenDrive::Connection& c2) {
    if (c1.fromEdge != c2.fromEdge) {
        return c1.fromEdge   < c2.fromEdge;
    }
    if (c1.toEdge     != c2.toEdge) {
        return c1.toEdge     < c2.toEdge;
    }
    if (c1.fromLane != c2.fromLane) {
        return c1.fromLane < c2.fromLane;
    }
    return c1.toLane < c2.toLane;
}

void
NIImporter_OpenDrive::sanitizeWidths(OpenDriveEdge* e) {
#ifdef DEBUG_VARIABLE_WIDTHS
    if (DEBUG_COND(e)) {
        gDebugFlag1 = true;
        std::cout << "sanitizeWidths e=" << e->id << " sections=" << e->laneSections.size() << "\n";
    }
#endif
    for (OpenDriveLaneSection& sec : e->laneSections) {
        // filter widths within the current section (#5888).
        // @note, Short laneSections could also be worth filtering alltogether
        if (sec.rightLaneNumber > 0) {
            sanitizeWidths(sec.lanesByDir[OPENDRIVE_TAG_RIGHT], sec.length);
        }
        if (sec.leftLaneNumber > 0) {
            sanitizeWidths(sec.lanesByDir[OPENDRIVE_TAG_LEFT], sec.length);
        }
    }
}

void
NIImporter_OpenDrive::sanitizeWidths(std::vector<OpenDriveLane>& lanes, double length) {
    for (OpenDriveLane& l : lanes) {
        if (l.widthData.size() > 0) {
            auto& wd = l.widthData;
            const double threshold = POSITION_EPS;
            double maxNoShort = -std::numeric_limits<double>::max();
            double seen = 0;
            for (int i = 0; i < (int)wd.size(); i++) {
                const double wdLength = i < (int)wd.size() - 1 ? wd[i + 1].s - wd[i].s : length - seen;
                seen += wdLength;
                if (wdLength > threshold) {
                    maxNoShort = MAX2(maxNoShort, wd[i].a);
                }
            }
            if (maxNoShort > 0) {
                l.width = maxNoShort;
            }
        }
    }
}


void
NIImporter_OpenDrive::splitMinWidths(OpenDriveEdge* e, const NBTypeCont& tc, double minDist) {
    std::vector<OpenDriveLaneSection> newSections;
#ifdef DEBUG_VARIABLE_WIDTHS
    if (DEBUG_COND(e)) {
        gDebugFlag1 = true;
        std::cout << "splitMinWidths e=" << e->id << " sections=" << e->laneSections.size() << "\n";
    }
#endif
    for (std::vector<OpenDriveLaneSection>::iterator j = e->laneSections.begin(); j != e->laneSections.end(); ++j) {
        OpenDriveLaneSection& sec = *j;
        std::vector<double> splitPositions;
        const double sectionEnd = (j + 1) == e->laneSections.end() ? e->length : (*(j + 1)).s;
        const int section = (int)(j - e->laneSections.begin());
#ifdef DEBUG_VARIABLE_WIDTHS
        if (DEBUG_COND(e)) {
            std::cout << "  findWidthSplit section=" << section << " sectionStart=" << sec.s << " sectionOrigStart=" << sec.sOrig << " sectionEnd=" << sectionEnd << "\n";
        }
#endif
        if (sec.rightLaneNumber > 0) {
            findWidthSplit(tc, sec.lanesByDir[OPENDRIVE_TAG_RIGHT], section, sec.sOrig, sectionEnd, splitPositions);
        }
        if (sec.leftLaneNumber > 0) {
            findWidthSplit(tc, sec.lanesByDir[OPENDRIVE_TAG_LEFT], section, sec.sOrig, sectionEnd, splitPositions);
        }
        newSections.push_back(sec);
        std::sort(splitPositions.begin(), splitPositions.end());
        // filter out tiny splits
        double prevSplit = sec.s;
        for (std::vector<double>::iterator it = splitPositions.begin(); it != splitPositions.end();) {
            if ((*it) - prevSplit < minDist || sectionEnd - (*it) < minDist) {
                // avoid tiny (or duplicate) splits
#ifdef DEBUG_VARIABLE_WIDTHS
                if (DEBUG_COND(e)) {
                    std::cout << " skip close split=" << (*it) << " prevSplit=" << prevSplit << "\n";
                }
#endif
                it = splitPositions.erase(it);
            } else if ((*it) < sec.s) {
                // avoid splits for another section
#ifdef DEBUG_VARIABLE_WIDTHS
                if (DEBUG_COND(e)) {
                    std::cout << " skip early split=" << (*it) << " s=" << sec.s << "\n";
                }
#endif
                it = splitPositions.erase(it);
            } else {
                prevSplit = *it;
                it++;
            }
        }

        if (splitPositions.size() > 0) {
#ifdef DEBUG_VARIABLE_WIDTHS
            if (DEBUG_COND(e)) {
                std::cout << " road=" << e->id << " splitMinWidths section=" << section
                          << " start=" << sec.s
                          << " origStart=" << sec.sOrig
                          << " end=" << sectionEnd << " minDist=" << minDist
                          << " splitPositions=" << toString(splitPositions) << "\n";
            }
#endif
#ifdef DEBUG_VARIABLE_WIDTHS
            if (DEBUG_COND(e)) {
                std::cout << "first section...\n";
            }
#endif
            recomputeWidths(newSections.back(), sec.sOrig, splitPositions.front(), sec.sOrig, sectionEnd);
            for (std::vector<double>::iterator it = splitPositions.begin(); it != splitPositions.end(); ++it) {
                OpenDriveLaneSection secNew = sec;
                secNew.s = *it;
#ifdef DEBUG_VARIABLE_WIDTHS
                if (DEBUG_COND(e)) {
                    std::cout << "splitAt " << secNew.s << "\n";
                }
#endif
                newSections.push_back(secNew);
                if (secNew.rightLaneNumber > 0) {
                    setStraightConnections(newSections.back().lanesByDir[OPENDRIVE_TAG_RIGHT]);
                }
                if (secNew.leftLaneNumber > 0) {
                    setStraightConnections(newSections.back().lanesByDir[OPENDRIVE_TAG_LEFT]);
                }
                double end = (it + 1) == splitPositions.end() ? sectionEnd : *(it + 1);
                recomputeWidths(newSections.back(), secNew.s, end, sec.sOrig, sectionEnd);
            }
        }
    }
    gDebugFlag1 = false;
    e->laneSections = newSections;
}


void
NIImporter_OpenDrive::findWidthSplit(const NBTypeCont& tc, std::vector<OpenDriveLane>& lanes,
                                     int section, double sectionStart, double sectionEnd,
                                     std::vector<double>& splitPositions) {
    UNUSED_PARAMETER(section);
    for (std::vector<OpenDriveLane>::iterator k = lanes.begin(); k != lanes.end(); ++k) {
        OpenDriveLane& l = *k;
        SVCPermissions permissions = tc.getPermissions(l.type) & ~(SVC_PEDESTRIAN | SVC_BICYCLE);
        if (l.widthData.size() > 0 && tc.knows(l.type) && !tc.getShallBeDiscarded(l.type) && permissions != 0) {
            double sPrev = l.widthData.front().s;
            double wPrev = l.widthData.front().computeAt(sPrev);
            if (gDebugFlag1) std::cout
                        << "findWidthSplit section=" << section
                        << "   sectionStart=" << sectionStart
                        << "   sectionEnd=" << sectionEnd
                        << " lane=" << l.id
                        << " type=" << l.type
                        << " widthEntries=" << l.widthData.size() << "\n"
                        << "    s=" << sPrev
                        << " w=" << wPrev
                        << "\n";
            for (std::vector<OpenDriveWidth>::iterator it_w = l.widthData.begin(); it_w != l.widthData.end(); ++it_w) {
                double sEnd = (it_w + 1) != l.widthData.end() ? (*(it_w + 1)).s : sectionEnd - sectionStart;
                double w = (*it_w).computeAt(sEnd);
                if (gDebugFlag1) std::cout
                            << "    sEnd=" << sEnd
                            << " s=" << (*it_w).s
                            << " a=" << (*it_w).a << " b=" << (*it_w).b << " c=" << (*it_w).c << " d=" << (*it_w).d
                            << " w=" << w
                            << "\n";
                const double changeDist = fabs(myMinWidth - wPrev);
                if (((wPrev < myMinWidth) && (w > myMinWidth))
                        || ((wPrev > myMinWidth) && (w < myMinWidth))) {
                    double splitPos = sPrev + (sEnd - sPrev) / fabs(w - wPrev) * changeDist;
                    double wSplit = (*it_w).computeAt(splitPos);
                    if (gDebugFlag1) {
                        std::cout << "     candidate splitPos=" << splitPos << " w=" << wSplit << "\n";
                    }
                    // ensure that the thin part is actually thin enough
                    while (wSplit > myMinWidth) {
                        if (wPrev < myMinWidth) {
                            // getting wider
                            splitPos -= POSITION_EPS;
                            if (splitPos < sPrev) {
                                if (gDebugFlag1) {
                                    std::cout << "        aborting search splitPos=" << splitPos << " wSplit=" << wSplit << " sPrev=" << sPrev << " wPrev=" << wPrev << "\n";
                                }
                                splitPos = sPrev;
                                break;
                            }
                        } else {
                            // getting thinner
                            splitPos += POSITION_EPS;
                            if (splitPos > sEnd) {
                                if (gDebugFlag1) {
                                    std::cout << "        aborting search splitPos=" << splitPos << " wSplit=" << wSplit << " sEnd=" << sEnd << " w=" << w << "\n";
                                }
                                splitPos = sEnd;
                                break;
                            }
                        }
                        wSplit = (*it_w).computeAt(splitPos);
                        if (gDebugFlag1) {
                            std::cout << "        refined splitPos=" << splitPos << " w=" << wSplit << "\n";
                        }
                    }
                    splitPositions.push_back(sectionStart + splitPos);
                }
                //    //wPrev = wSplit;
                //} else if ((fabs(wPrev) < NUMERICAL_EPS && w > POSITION_EPS)
                //        || (wPrev > POSITION_EPS && fabs(w) < NUMERICAL_EPS)) {
                //    splitPositions.push_back(sectionStart + sPrev);
                //    if (gDebugFlag1) std::cout << "     laneDisappears candidate splitPos=" << sPrev << " wPrev=" << wPrev << " w=" << w<< "\n";
                //}
                wPrev = w;
                sPrev = sEnd;
            }
        }
    }
}


void
NIImporter_OpenDrive::setStraightConnections(std::vector<OpenDriveLane>& lanes) {
    for (std::vector<OpenDriveLane>::iterator k = lanes.begin(); k != lanes.end(); ++k) {
        (*k).predecessor = (*k).id;
    }
}


void
NIImporter_OpenDrive::recomputeWidths(OpenDriveLaneSection& sec, double start, double end, double sectionStart, double sectionEnd) {
    if (sec.rightLaneNumber > 0) {
        recomputeWidths(sec.lanesByDir[OPENDRIVE_TAG_RIGHT], start, end, sectionStart, sectionEnd);
    }
    if (sec.leftLaneNumber > 0) {
        recomputeWidths(sec.lanesByDir[OPENDRIVE_TAG_LEFT], start, end, sectionStart, sectionEnd);
    }
}


void
NIImporter_OpenDrive::recomputeWidths(std::vector<OpenDriveLane>& lanes, double start, double end, double sectionStart, double sectionEnd) {
    for (std::vector<OpenDriveLane>::iterator k = lanes.begin(); k != lanes.end(); ++k) {
        OpenDriveLane& l = *k;
        if (l.widthData.size() > 0) {
#ifdef DEBUG_VARIABLE_WIDTHS
            if (gDebugFlag1) std::cout
                        << "recomputeWidths lane=" << l.id
                        << " type=" << l.type
                        << " start=" << start
                        << " end=" << end
                        << " sectionStart=" << sectionStart
                        << " sectionEnd=" << sectionEnd
                        << " widthEntries=" << l.widthData.size() << "\n"
                        << "\n";
#endif
            l.width = 0;
            double sPrev = l.widthData.front().s;
            double sPrevAbs = sPrev + sectionStart;
            for (std::vector<OpenDriveWidth>::iterator it_w = l.widthData.begin(); it_w != l.widthData.end(); ++it_w) {
                double sEnd = (it_w + 1) != l.widthData.end() ? (*(it_w + 1)).s : sectionEnd - sectionStart;
                double sEndAbs = sEnd + sectionStart;
#ifdef DEBUG_VARIABLE_WIDTHS
                if (gDebugFlag1) std::cout
                            << " sPrev=" << sPrev << " sPrevAbs=" << sPrevAbs
                            << " sEnd=" << sEnd << " sEndAbs=" << sEndAbs
                            << " widthData s=" << (*it_w).s
                            << " a=" << (*it_w).a
                            << " b=" << (*it_w).b
                            << " c=" << (*it_w).c
                            << " d=" << (*it_w).d
                            << "\n";
#endif
                if (sPrevAbs <= start && sEndAbs >= start) {
#ifdef DEBUG_VARIABLE_WIDTHS
                    if (gDebugFlag1) {
                        std::cout << "   atStart=" << start << " pos=" << start - sectionStart << " w=" << (*it_w).computeAt(start - sectionStart) << "\n";
                    }
#endif
                    l.width = MAX2(l.width, (*it_w).computeAt(start - sectionStart));
                }
                if (sPrevAbs <= end && sEndAbs >= end) {
#ifdef DEBUG_VARIABLE_WIDTHS
                    if (gDebugFlag1) {
                        std::cout << "   atEnd=" << end << " pos=" << end - sectionStart << " w=" << (*it_w).computeAt(end - sectionStart) << "\n";
                    }
#endif
                    l.width = MAX2(l.width, (*it_w).computeAt(end - sectionStart));
                }
                if (start <= sPrevAbs && end >= sPrevAbs) {
#ifdef DEBUG_VARIABLE_WIDTHS
                    if (gDebugFlag1) {
                        std::cout << "   atSPrev=" << sPrev << " w=" << (*it_w).computeAt(sPrev) << "\n";
                    }
#endif
                    l.width = MAX2(l.width, (*it_w).computeAt(sPrev));
                }
                if (start <= sEndAbs && end >= sEndAbs) {
#ifdef DEBUG_VARIABLE_WIDTHS
                    if (gDebugFlag1) {
                        std::cout << "   atSEnd=" << sEnd << " w=" << (*it_w).computeAt(sEnd) << "\n";
                    }
#endif
                    l.width = MAX2(l.width, (*it_w).computeAt(sEnd));
                }
#ifdef DEBUG_VARIABLE_WIDTHS
                if (gDebugFlag1) {
                    std::cout << " sPrev=" << sPrev << " sEnd=" << sEnd << " l.width=" << l.width << "\n";
                }
#endif
                sPrev = sEnd;
                sPrevAbs = sEndAbs;
            }
        }
    }
}


/****************************************************************************/
