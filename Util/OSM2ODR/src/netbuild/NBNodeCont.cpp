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
/// @file    NBNodeCont.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Yun-Pang Floetteroed
/// @author  Walter Bamberger
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Sascha Krieg
/// @date    Tue, 20 Nov 2001
///
// Container for nodes during the netbuilding process
/****************************************************************************/
#include <config.h>

#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include <utils/options/OptionsCont.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StdDefs.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/common/IDSupplier.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/iodevices/OutputDevice.h>
#include "NBHelpers.h"
#include "NBAlgorithms.h"
#include "NBDistrict.h"
#include "NBEdgeCont.h"
#include "NBTrafficLightLogicCont.h"
#include "NBOwnTLDef.h"
#include "NBNodeCont.h"
#include "NBPTStopCont.h"
#include "NBPTLineCont.h"
#include "NBParking.h"

// ===========================================================================
// Algorithm constants
// ===========================================================================
#define MAX_SLIPLANE_LENGTH 1000

// ===========================================================================
// Debug Flags
// ===========================================================================

//#define DEBUG_JOINJUNCTIONS
//#define DEBUG_GUESSSIGNALS
#define DEBUGNODEID "3513423881"
#define DEBUGNODEID2 ""
//#define DEBUGNODEID "5548037023"
#define DEBUGCOND(obj) ((obj) != 0 && ((obj)->getID() == DEBUGNODEID || (obj)->getID() == DEBUGNODEID2))
//#define DEBUGCOND(obj) (true)


// ===========================================================================
// method definitions
// ===========================================================================
NBNodeCont::NBNodeCont()
    : myInternalID(1) {
}


NBNodeCont::~NBNodeCont() {
    clear();
}


// ----------- Insertion/removal/retrieval of nodes
bool
NBNodeCont::insert(const std::string& id, const Position& position,
                   NBDistrict* district) {
    NodeCont::iterator i = myNodes.find(id);
    if (i != myNodes.end()) {
        return false;
    }
    NBNode* node = new NBNode(id, position, district);
    myNodes[id] = node;
    const float pos[2] = {(float)position.x(), (float)position.y()};
    myRTree.Insert(pos, pos, node);
    return true;
}


bool
NBNodeCont::insert(NBNode* node) {
    std::string id = node->getID();
    NodeCont::iterator i = myNodes.find(id);
    if (i != myNodes.end()) {
        return false;
    }
    myNodes[id] = node;
    const float pos[2] = {(float)node->getPosition().x(), (float)node->getPosition().y()};
    myRTree.Insert(pos, pos, node);
    return true;
}


NBNode*
NBNodeCont::retrieve(const std::string& id) const {
    NodeCont::const_iterator i = myNodes.find(id);
    if (i == myNodes.end()) {
        return nullptr;
    }
    return (*i).second;
}


NBNode*
NBNodeCont::retrieve(const Position& position, const double offset) const {
    const double extOffset = offset + POSITION_EPS;
    const float cmin[2] = {(float)(position.x() - extOffset), (float)(position.y() - extOffset)};
    const float cmax[2] = {(float)(position.x() + extOffset), (float)(position.y() + extOffset)};
    std::set<const Named*> into;
    Named::StoringVisitor sv(into);
    myRTree.Search(cmin, cmax, sv);
    for (const Named* namedNode : into) {
        NBNode* node = const_cast<NBNode*>(dynamic_cast<const NBNode*>(namedNode));
        if (fabs(node->getPosition().x() - position.x()) <= offset
                &&
                fabs(node->getPosition().y() - position.y()) <= offset) {
            return node;
        }
    }
    return nullptr;
}


bool
NBNodeCont::erase(NBNode* node) {
    if (extract(node)) {
        delete node;
        return true;
    } else {
        return false;
    }
}


bool
NBNodeCont::extract(NBNode* node, bool remember) {
    NodeCont::iterator i = myNodes.find(node->getID());
    if (i == myNodes.end()) {
        return false;
    }
    myNodes.erase(i);
    const float pos[2] = {(float)node->getPosition().x(), (float)node->getPosition().y()};
    myRTree.Remove(pos, pos, node);
    node->removeTrafficLights();
    if (remember) {
        myExtractedNodes[node->getID()] = node;
    }
    return true;
}


// ----------- Adapting the input
void
NBNodeCont::removeSelfLoops(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tc) {
    int no = 0;
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        no += (*i).second->removeSelfLoops(dc, ec, tc);
    }
    if (no != 0) {
        WRITE_WARNING(toString(no) + " self-looping edge(s) removed.");
    }
}


void
NBNodeCont::joinSimilarEdges(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    // magic values
    const double distanceThreshold = 7.; // don't merge edges further apart
    const double lengthThreshold = 0.10; // don't merge edges with higher relative length-difference

    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        // count the edges to other nodes outgoing from the current node
        std::map<NBNode*, EdgeVector> connectionCount;
        const EdgeVector& outgoing = (*i).second->getOutgoingEdges();
        for (EdgeVector::const_iterator j = outgoing.begin(); j != outgoing.end(); j++) {
            connectionCount[(*j)->getToNode()].push_back(*j);
        }
        // check whether more than a single edge connect another node and join them
        std::map<NBNode*, EdgeVector>::iterator k;
        for (k = connectionCount.begin(); k != connectionCount.end(); k++) {
            // possibly we do not have anything to join...
            if ((*k).second.size() < 2) {
                continue;
            }
            // for the edges that seem to be a single street,
            //  check whether the geometry is similar
            const EdgeVector& ev = (*k).second;
            const NBEdge* const first = ev.front();
            EdgeVector::const_iterator jci; // join candidate iterator
            for (jci = ev.begin() + 1; jci != ev.end(); ++jci) {
                const double relativeLengthDifference = fabs(first->getLoadedLength() - (*jci)->getLoadedLength()) / first->getLoadedLength();
                if ((!first->isNearEnough2BeJoined2(*jci, distanceThreshold)) ||
                        (relativeLengthDifference > lengthThreshold) ||
                        (fabs(first->getSpeed() - (*jci)->getSpeed()) >= 0.01) || // output accuracy
                        (first->getPermissions() != (*jci)->getPermissions())
                   ) {
                    break;
                }
            }
            // @bug If there are 3 edges of which 2 can be joined, no joining will
            //   take place with the current implementation
            if (jci == ev.end()) {
                ec.joinSameNodeConnectingEdges(dc, tlc, ev);
            }
        }
    }
}


void
NBNodeCont::removeIsolatedRoads(NBDistrictCont& dc, NBEdgeCont& ec) {
    // Warn of isolated edges, i.e. a single edge with no connection to another edge
    const std::vector<std::string>& edgeNames = ec.getAllNames();
    for (std::vector<std::string>::const_iterator it = edgeNames.begin(); it != edgeNames.end(); ++it) {
        // Test whether this node starts at a dead end, i.e. it has only one adjacent node
        // to which an edge exists and from which an edge may come.
        NBEdge* e = ec.retrieve(*it);
        if (e == nullptr) {
            continue;
        }
        NBNode* from = e->getFromNode();
        const EdgeVector& outgoingEdges = from->getOutgoingEdges();
        if (outgoingEdges.size() != 1) {
            // At this node, several edges or no edge start; so, this node is no dead end.
            continue;
        }
        const EdgeVector& incomingEdges = from->getIncomingEdges();
        if (incomingEdges.size() > 1) {
            // At this node, several edges end; so, this node is no dead end.
            continue;
        } else if (incomingEdges.size() == 1) {
            NBNode* fromNodeOfIncomingEdge = incomingEdges[0]->getFromNode();
            NBNode* toNodeOfOutgoingEdge = outgoingEdges[0]->getToNode();
            if (fromNodeOfIncomingEdge != toNodeOfOutgoingEdge) {
                // At this node, an edge ends which is not the inverse direction of
                // the starting node.
                continue;
            }
        }
        // Now we know that the edge e starts a dead end.
        // Next we test if the dead end is isolated, i.e. does not lead to a junction
        bool hasJunction = false;
        EdgeVector road;
        NBEdge* eOld = nullptr;
        NBNode* to;
        NodeSet adjacentNodes;
        do {
            road.push_back(e);
            eOld = e;
            from = e->getFromNode();
            to = e->getToNode();
            const EdgeVector& outgoingEdgesOfToNode = to->getOutgoingEdges();
            const EdgeVector& incomingEdgesOfToNode = to->getIncomingEdges();
            adjacentNodes.clear();
            for (EdgeVector::const_iterator itOfOutgoings = outgoingEdgesOfToNode.begin(); itOfOutgoings != outgoingEdgesOfToNode.end(); ++itOfOutgoings) {
                if ((*itOfOutgoings)->getToNode() != from        // The back path
                        && (*itOfOutgoings)->getToNode() != to   // A loop / dummy edge
                   ) {
                    e = *itOfOutgoings; // Probably the next edge
                }
                adjacentNodes.insert((*itOfOutgoings)->getToNode());
            }
            for (EdgeVector::const_iterator itOfIncomings = incomingEdgesOfToNode.begin(); itOfIncomings != incomingEdgesOfToNode.end(); ++itOfIncomings) {
                adjacentNodes.insert((*itOfIncomings)->getFromNode());
            }
            adjacentNodes.erase(to);  // Omit loops
            if (adjacentNodes.size() > 2) {
                hasJunction = true;
            }
        } while (!hasJunction && eOld != e);
        if (!hasJunction) {
            std::string warningString;
            for (EdgeVector::iterator roadIt = road.begin(); roadIt != road.end(); ++roadIt) {
                if (roadIt == road.begin()) {
                    warningString += (*roadIt)->getID();
                } else {
                    warningString += "," + (*roadIt)->getID();
                }

                NBNode* fromNode = (*roadIt)->getFromNode();
                NBNode* toNode = (*roadIt)->getToNode();
                ec.erase(dc, *roadIt);
                if (fromNode->getIncomingEdges().size() == 0 && fromNode->getOutgoingEdges().size() == 0) {
                    // Node is empty; can be removed
                    erase(fromNode);
                }
                if (toNode->getIncomingEdges().size() == 0 && toNode->getOutgoingEdges().size() == 0) {
                    // Node is empty; can be removed
                    erase(toNode);
                }
            }
            WRITE_WARNINGF("Removed a road without junctions: %.", warningString);
        }
    }
}


void
NBNodeCont::removeComponents(NBDistrictCont& dc, NBEdgeCont& ec, const int numKeep) {
    std::vector<std::set<NBEdge*> > components;
    // need to use ids here to have the same ordering on all platforms
    std::set<std::string> edgesLeft;
    for (std::map<std::string, NBEdge*>::const_iterator edgeIt = ec.begin(); edgeIt != ec.end(); ++edgeIt) {
        edgesLeft.insert(edgeIt->first);
    }
    EdgeVector queue;
    std::set<NBEdge*> toRemove;
    while (!edgesLeft.empty()) {
        queue.push_back(ec.getByID(*edgesLeft.begin()));
        std::set<NBEdge*> component;
        while (!queue.empty()) {
            NBEdge* const e = queue.back();
            queue.pop_back();
            component.insert(e);
            std::vector<EdgeVector> edgeLists;
            edgeLists.push_back(e->getFromNode()->getOutgoingEdges());
            edgeLists.push_back(e->getFromNode()->getIncomingEdges());
            edgeLists.push_back(e->getToNode()->getOutgoingEdges());
            edgeLists.push_back(e->getToNode()->getIncomingEdges());
            for (std::vector<EdgeVector>::const_iterator listIt = edgeLists.begin(); listIt != edgeLists.end(); ++listIt) {
                for (EdgeVector::const_iterator edgeIt = listIt->begin(); edgeIt != listIt->end(); ++edgeIt) {
                    std::set<std::string>::iterator leftIt = edgesLeft.find((*edgeIt)->getID());
                    if (leftIt != edgesLeft.end()) {
                        queue.push_back(*edgeIt);
                        edgesLeft.erase(leftIt);
                    }
                }
            }
        }
        std::vector<std::set<NBEdge*> >::iterator cIt;
        for (cIt = components.begin(); cIt != components.end(); ++cIt) {
            if (cIt->size() < component.size()) {
                break;
            }
        }
        components.insert(cIt, component);
        if ((int)components.size() > numKeep) {
            toRemove.insert(components.back().begin(), components.back().end());
            components.pop_back();
        }
    }
    for (std::set<NBEdge*>::iterator edgeIt = toRemove.begin(); edgeIt != toRemove.end(); ++edgeIt) {
        NBNode* const fromNode = (*edgeIt)->getFromNode();
        NBNode* const toNode = (*edgeIt)->getToNode();
        ec.erase(dc, *edgeIt);
        if (fromNode->getIncomingEdges().size() == 0 && fromNode->getOutgoingEdges().size() == 0) {
            erase(fromNode);
        }
        if (toNode->getIncomingEdges().size() == 0 && toNode->getOutgoingEdges().size() == 0) {
            erase(toNode);
        }
    }
}


int
NBNodeCont::removeUnwishedNodes(NBDistrictCont& dc, NBEdgeCont& ec,
                                NBTrafficLightLogicCont& tlc, NBPTStopCont& sc, NBPTLineCont& lc,
                                NBParkingCont& pc,
                                bool removeGeometryNodes) {
    // load edges that shall not be modified
    std::set<std::string> edges2keep;
    if (removeGeometryNodes) {
        const OptionsCont& oc = OptionsCont::getOptions();
        if (oc.isSet("geometry.remove.keep-edges.input-file")) {
            NBHelpers::loadEdgesFromFile(oc.getString("geometry.remove.keep-edges.input-file"), edges2keep);
        }
        if (oc.isSet("geometry.remove.keep-edges.explicit")) {
            const std::vector<std::string> edges = oc.getStringVector("geometry.remove.keep-edges.explicit");
            edges2keep.insert(edges.begin(), edges.end());
        }
        sc.addEdges2Keep(oc, edges2keep);
        UNUSED_PARAMETER(lc); // no need to keep all route edges. They are validated again before writing
        pc.addEdges2Keep(oc, edges2keep);
    }
    int no = 0;
    std::vector<NBNode*> toRemove;
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        NBNode* current = (*i).second;
        bool remove = false;
        std::vector<std::pair<NBEdge*, NBEdge*> > toJoin;
        // check for completely empty nodes
        if (current->getOutgoingEdges().size() == 0 && current->getIncomingEdges().size() == 0) {
            // remove if empty
            remove = true;
        }
        // check for nodes which are only geometry nodes
        if (removeGeometryNodes && mySplit.count(current) == 0) {
            if ((current->getOutgoingEdges().size() == 1 && current->getIncomingEdges().size() == 1)
                    ||
                    (current->getOutgoingEdges().size() == 2 && current->getIncomingEdges().size() == 2)) {
                // ok, one in, one out or two in, two out
                //  -> ask the node whether to join
                remove = current->checkIsRemovable();
                // check whether any of the edges must be kept
                for (EdgeVector::const_iterator it_edge = current->getEdges().begin(); it_edge != current->getEdges().end(); ++it_edge) {
                    if (edges2keep.find((*it_edge)->getID()) != edges2keep.end()) {
                        remove = false;
                        break;
                    }
                }
                if (remove) {
                    toJoin = current->getEdgesToJoin();
                }
            }
        }
        // remove the node and join the geometries when wished
        if (!remove) {
            continue;
        }
        for (std::vector<std::pair<NBEdge*, NBEdge*> >::iterator j = toJoin.begin(); j != toJoin.end(); j++) {
            NBEdge* begin = (*j).first;
            NBEdge* continuation = (*j).second;
            begin->append(continuation);
            continuation->getToNode()->replaceIncoming(continuation, begin, 0);
            tlc.replaceRemoved(continuation, -1, begin, -1, true);
            ec.extract(dc, continuation, true);
        }
        toRemove.push_back(current);
        no++;
    }
    // erase all
    for (std::vector<NBNode*>::iterator j = toRemove.begin(); j != toRemove.end(); ++j) {
        extract(*j, true);
    }
    return no;
}


void
NBNodeCont::avoidOverlap() {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->avoidOverlap();
    }
}

// ----------- (Helper) methods for joining nodes
void
NBNodeCont::generateNodeClusters(double maxDist, NodeClusters& into) const {
    std::set<NBNode*> visited;
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        std::vector<NodeAndDist> toProc;
        if (visited.find((*i).second) != visited.end()) {
            continue;
        }
        toProc.push_back(std::make_pair((*i).second, 0));
        NodeSet c;
        while (!toProc.empty()) {
            NodeAndDist nodeAndDist = toProc.back();
            NBNode* n = nodeAndDist.first;
            double dist = nodeAndDist.second;
            toProc.pop_back();
            if (visited.find(n) != visited.end()) {
                continue;
            }
            visited.insert(n);
            bool pureRail = true;
            bool railAndPeds = true;
            for (NBEdge* e : n->getEdges()) {
                if ((e->getPermissions() & ~(SVC_RAIL_CLASSES | SVC_PEDESTRIAN)) != 0) {
                    railAndPeds = false;
                    pureRail = false;
                    break;
                }
                if ((e->getPermissions() & ~(SVC_RAIL_CLASSES)) != 0) {
                    pureRail = false;
                }
            }
            if (pureRail) {
                // do not join pure rail nodes
                continue;
            }
            c.insert(n);
            for (NBEdge* e : n->getEdges()) {
                NBNode* s = n->hasIncoming(e) ? e->getFromNode() : e->getToNode();
                const double length = e->getLoadedLength();
#ifdef DEBUG_JOINJUNCTIONS
                if (DEBUGCOND(s)) {
                    std::cout << "generateNodeClusters: consider s=" << s->getID()
                              << " clusterNode=" << n->getID() << " edge=" << e->getID() << " length=" << length << " with cluster " << joinNamedToString(c, ' ') << "\n";
                }
#endif
                if (railAndPeds && n->getType() != SumoXMLNodeType::RAIL_CROSSING) {
                    bool railAndPeds2 = true;
                    for (NBEdge* e : n->getEdges()) {
                        if ((e->getPermissions() & ~(SVC_RAIL_CLASSES | SVC_PEDESTRIAN)) != 0) {
                            railAndPeds2 = false;
                            break;
                        }
                    }
                    if (railAndPeds2 && s->getType() != SumoXMLNodeType::RAIL_CROSSING) {
                        // do not join rail/ped nodes unless at a rail crossing
                        // (neither nodes nor the traffic lights)
                        continue;
                    }
                }
                const bool bothCrossing = n->getType() == SumoXMLNodeType::RAIL_CROSSING && s->getType() == SumoXMLNodeType::RAIL_CROSSING;
                const bool joinPedCrossings = bothCrossing && e->getPermissions() == SVC_PEDESTRIAN;
                if ( // never join pedestrian stuff (unless at a rail crossing
                    !joinPedCrossings && (
                        e->getPermissions() == SVC_PEDESTRIAN
                        // only join edges for regular passenger traffic or edges that are extremely short
                        || (length > 3 * POSITION_EPS
                            && (e->getPermissions() & (SVC_PASSENGER | SVC_TRAM)) == 0
                            && n->getPosition().distanceTo2D(s->getPosition()) > SUMO_const_laneWidth))) {
                    continue;
                }
                // never join rail_crossings with other node types unless the crossing is only for tram
                if ((n->getType() == SumoXMLNodeType::RAIL_CROSSING && s->getType() != SumoXMLNodeType::RAIL_CROSSING)
                        || (n->getType() != SumoXMLNodeType::RAIL_CROSSING && s->getType() == SumoXMLNodeType::RAIL_CROSSING)) {
                    const SVCPermissions railNoTram = (SVC_RAIL_CLASSES & ~SVC_TRAM);
                    bool foundRail = false;
                    NBNode* crossingNode = n->getType() == SumoXMLNodeType::RAIL_CROSSING ? n : s;
                    for (NBEdge* e2 : crossingNode->getIncomingEdges()) {
                        if ((e2->getPermissions() & railNoTram) != 0) {
                            foundRail = true;
                            break;
                        }
                    }
                    if (foundRail) {
                        continue;
                    }
                }
                // never join rail_crossings via a rail edge
                if (bothCrossing && (e->getPermissions() & ~SVC_RAIL_CLASSES) == 0) {
                    continue;
                }
                if (visited.find(s) != visited.end()) {
                    continue;
                }
                if (length + dist < maxDist) {
                    if (s->geometryLike()) {
                        toProc.push_back(std::make_pair(s, dist + length));
                    } else {
                        toProc.push_back(std::make_pair(s, 0));
                    }
                }
            }
        }
        if (c.size() < 2) {
            continue;
        }
#ifdef DEBUG_JOINJUNCTIONS
        std::cout << " DEBUG: consider cluster " << joinNamedToString(c, ' ') << "\n";
#endif
        into.push_back(c);
    }
}


void
NBNodeCont::addJoinExclusion(const std::vector<std::string>& ids, bool check) {
    for (std::vector<std::string>::const_iterator it = ids.begin(); it != ids.end(); it++) {
        // error handling has to take place here since joinExclusions could be
        // loaded from multiple files / command line
        if (myJoined.count(*it) > 0) {
            WRITE_WARNINGF("Ignoring join exclusion for junction '%' since it already occurred in a list of nodes to be joined.", *it);
        } else if (check && retrieve(*it) == nullptr) {
            WRITE_WARNINGF("Ignoring join exclusion for unknown junction '%'.", *it);
        } else {
            myJoinExclusions.insert(*it);
        }
    }
}


void
NBNodeCont::addCluster2Join(std::set<std::string> cluster, NBNode* node) {
    // error handling has to take place here since joins could be loaded from multiple files
    std::set<std::string> validCluster;
    for (std::string nodeID : cluster) {
        if (myJoinExclusions.count(nodeID) > 0) {
            WRITE_WARNINGF("Ignoring join-cluster because junction '%' was already excluded from joining.", nodeID);
            return;
        } else if (myJoined.count(nodeID) > 0) {
            WRITE_WARNINGF("Ignoring join-cluster because junction '%' already occurred in another join-cluster.", nodeID);
            return;
        } else {
            NBNode* const node = retrieve(nodeID);
            if (node != nullptr) {
                validCluster.insert(nodeID);
            } else {
                if (StringUtils::startsWith(nodeID, "cluster_")) {
                    // assume join directive came from a pre-processed network. try to use component IDs
                    std::set<std::string> subIDs;
                    for (std::string nID : StringTokenizer(nodeID.substr(8), "_").getVector()) {
                        if (retrieve(nID) != nullptr) {
                            validCluster.insert(nID);
                        } else {
                            WRITE_ERROR("Unknown junction '" + nodeID + "' in join-cluster (componentID).");
                        }
                    }
                } else {
                    WRITE_ERROR("Unknown junction '" + nodeID + "' in join-cluster.");
                }
            }
        }
    }
    if (validCluster.size() > 1) {
        myJoined.insert(validCluster.begin(), validCluster.end());
        myClusters2Join.push_back(std::make_pair(validCluster, node));
    } else {
        WRITE_WARNINGF("Ignoring join-cluster '%s' because it has size '%'.", node->getID(), validCluster.size());
    }
}


int
NBNodeCont::joinLoadedClusters(NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    int numJoined = 0;
    for (auto& item : myClusters2Join) {
        // verify loaded cluster
        NodeSet cluster;
        for (std::string nodeID : item.first) {
            NBNode* node = retrieve(nodeID);
            if (node == nullptr) {
                WRITE_ERROR("unknown junction '" + nodeID + "' while joining.");
            } else {
                cluster.insert(node);
            }
        }
        if (cluster.size() > 1) {
            joinNodeCluster(cluster, dc, ec, tlc, item.second);
            numJoined++;
            myJoinExclusions.insert(item.second->getID());
        }
    }
    myClusters2Join.clear(); // make save for recompute
    return numJoined;
}


int
NBNodeCont::joinJunctions(double maxDist, NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc, NBPTStopCont& sc) {
#ifdef DEBUG_JOINJUNCTIONS
    std::cout << "joinJunctions...\n";
#endif
    NodeClusters cands;
    NodeClusters clusters;
    generateNodeClusters(maxDist, cands);
    for (NodeClusters::iterator i = cands.begin(); i != cands.end(); ++i) {
        NodeSet cluster = (*i);
#ifdef DEBUG_JOINJUNCTIONS
        gDebugFlag1 = false;
        for (NBNode* n : cluster) {
            if (DEBUGCOND(n)) {
                gDebugFlag1 = true;
            }
        }
#endif
        // remove join exclusions
        for (NodeSet::iterator j = cluster.begin(); j != cluster.end();) {
            NodeSet::iterator check = j;
            ++j;
            if (myJoinExclusions.count((*check)->getID()) > 0) {
                cluster.erase(check);
            }
        }
        // remove nodes that can be eliminated by geometry.remove
        pruneClusterFringe(cluster);
        // avoid removal of long edges (must have been added via an alternative path).
        pruneLongEdges(cluster, maxDist);
        // remove nodes that are part of a bypass lane (typically for turning right without waiting at a traffic light)
        pruneSlipLaneNodes(cluster);
        if (cluster.size() < 2) {
            continue;
        }
        std::string reason;
        std::string origReason;
        std::string origCluster;
        bool feasible = feasibleCluster(cluster, ec, sc, origReason);
        if (!feasible) {
#ifdef DEBUG_JOINJUNCTIONS
            if (gDebugFlag1) {
                std::cout << "   try to reduce to 4-circle nodes=" << joinNamedToString(cluster, ',') << "\n";
            }
#endif
            origCluster = joinNamedToString(cluster, ',');
            if (reduceToCircle(cluster, 4, cluster)) {
                feasible = feasibleCluster(cluster, ec, sc, reason);
                if (feasible) {
                    WRITE_WARNINGF("Reducing junction cluster % (%).", origCluster, origReason);
                }
            }
        }
        if (!feasible) {
#ifdef DEBUG_JOINJUNCTIONS
            if (gDebugFlag1) {
                std::cout << "   try to reduce to 2-circle nodes=" << joinNamedToString(cluster, ',') << "\n";
            }
#endif
            origCluster = joinNamedToString(cluster, ',');
            if (reduceToCircle(cluster, 2, cluster)) {
                feasible = feasibleCluster(cluster, ec, sc, reason);
                if (feasible) {
                    WRITE_WARNINGF("Reducing junction cluster % (%).", origCluster, origReason);
                }
            }
        }
        if (!feasible) {
            WRITE_WARNINGF("Not joining junctions % (%).", origCluster, origReason);
            continue;
        }
        // compute all connected components of this cluster
        // (may be more than 1 if intermediate nodes were removed)
        NodeClusters components;
        for (NBNode* current : cluster) {
            // merge all connected components into newComp
            NodeSet newComp;
            //std::cout << "checking connectivity for " << current->getID() << "\n";
            newComp.insert(current);
            for (NodeClusters::iterator it_comp = components.begin(); it_comp != components.end();) {
                NodeClusters::iterator check = it_comp;
                //std::cout << "   connected with " << toString(*check) << "?\n";
                bool connected = false;
                for (NBNode* k : *check) {
                    if (current->getConnectionTo(k) != nullptr || k->getConnectionTo(current) != nullptr) {
                        //std::cout << "joining with connected component " << toString(*check) << "\n";
                        newComp.insert((*check).begin(), (*check).end());
                        it_comp = components.erase(check);
                        connected = true;
                        break;
                    }
                }
                if (!connected) {
                    it_comp++;
                }
            }
            //std::cout << "adding new component " << toString(newComp) << "\n";
            components.push_back(newComp);
        }
        for (NodeClusters::iterator it_comp = components.begin(); it_comp != components.end(); ++it_comp) {
            if ((*it_comp).size() > 1) {
                //std::cout << "adding cluster " << toString(*it_comp) << "\n";
                clusters.push_back(*it_comp);
            }
        }
#ifdef DEBUG_JOINJUNCTIONS
        gDebugFlag1 = false;
#endif
    }
    joinNodeClusters(clusters, dc, ec, tlc);
    return (int)clusters.size();
}


void
NBNodeCont::pruneClusterFringe(NodeSet& cluster) const {
#ifdef DEBUG_JOINJUNCTIONS
    if (gDebugFlag1) {
        std::cout << "pruning cluster=" << joinNamedToString(cluster, ' ') << "\n";
    }
#endif
    // iteratively remove the fringe
    bool pruneFringe = true;
    // collect nodes that shall be joined due to distance but are not connected
    // to the cluster for passenger traffic
    while (pruneFringe) {
        pruneFringe = false;
        for (NodeSet::iterator j = cluster.begin(); j != cluster.end();) {
            NodeSet::iterator check = j;
            NBNode* n = *check;
            ++j;

            // compute clusterDist for node (length of shortest edge which connects this node to the cluster)
            double clusterDist = std::numeric_limits<double>::max();
            bool touchingCluster = false;
            for (EdgeVector::const_iterator it_edge = n->getOutgoingEdges().begin(); it_edge != n->getOutgoingEdges().end(); ++it_edge) {
                NBNode* neighbor = (*it_edge)->getToNode();
                if (cluster.count(neighbor) != 0) {
                    clusterDist = MIN2(clusterDist, (*it_edge)->getLoadedLength());
                    touchingCluster |= n->getPosition().distanceTo2D(neighbor->getPosition()) <= SUMO_const_laneWidth;
                }
            }
            for (EdgeVector::const_iterator it_edge = n->getIncomingEdges().begin(); it_edge != n->getIncomingEdges().end(); ++it_edge) {
                NBNode* neighbor = (*it_edge)->getFromNode();
                if (cluster.count(neighbor) != 0) {
                    clusterDist = MIN2(clusterDist, (*it_edge)->getLoadedLength());
                    touchingCluster |= n->getPosition().distanceTo2D(neighbor->getPosition()) <= SUMO_const_laneWidth;
                }
            }
            // remove geometry-like nodes at fringe of the cluster
            // (they have 1 neighbor in the cluster and at most 1 neighbor outside the cluster)
            std::set<NBNode*> outsideNeighbors;
            std::set<NBNode*> clusterNeighbors;
            const double pedestrianFringeThreshold = 0.3;
            for (NBEdge* e : n->getEdges()) {
                NBNode* neighbor = e->getFromNode() == n ? e->getToNode() : e->getFromNode();
                if (cluster.count(neighbor) == 0) {
                    if ((e->getPermissions() & SVC_PASSENGER) != 0
                            || isRailway(e->getPermissions()) // join railway crossings
                            || clusterDist <= pedestrianFringeThreshold
                            || touchingCluster) {
                        outsideNeighbors.insert(neighbor);
                    }
                } else {
                    clusterNeighbors.insert(neighbor);
                }
            }
#ifdef DEBUG_JOINJUNCTIONS
            if (gDebugFlag1) std::cout << "  check n=" << n->getID()
                                           << " clusterDist=" << clusterDist
                                           << " cd<th=" << (clusterDist <= pedestrianFringeThreshold)
                                           << " touching=" << touchingCluster
                                           << " out=" << joinNamedToString(outsideNeighbors, ',')
                                           << " in=" << joinNamedToString(clusterNeighbors, ',')
                                           << "\n";
#endif
            if (clusterNeighbors.size() == 0
                    || (outsideNeighbors.size() <= 1
                        && clusterNeighbors.size() == 1
                        && !n->isTLControlled())) {
                cluster.erase(check);
                pruneFringe = true; // other nodes could belong to the fringe now
#ifdef DEBUG_JOINJUNCTIONS
                if (gDebugFlag1) {
                    std::cout << "  pruned n=" << n->getID() << "\n";
                }
#endif
            }
        }
    }
}


void
NBNodeCont::pruneLongEdges(NodeSet& cluster, double maxDist) {
    std::set<NBNode*> toRemove;
    int maxPassengerLanes = 0;
    for (NBNode* n : cluster) {
        for (NBEdge* edge : n->getEdges()) {
            maxPassengerLanes = MAX2(maxPassengerLanes, edge->getNumLanesThatAllow(SVC_PASSENGER));
        }
    }
    for (NBNode* n : cluster) {
        for (NBEdge* edge : n->getOutgoingEdges()) {
            // we must track the edge length accross geometry like nodes
            // Also, intersecions that are geometry-like
            // from the perspective of passenger traffic should be tracked accross
            std::vector<NBNode*> passed;
            double length = 0;
            NBEdge* cur = edge;
            NBNode* to = edge->getToNode();
            while (cluster.count(to) != 0) {
                length += cur->getLoadedLength();
                bool goStraight = (std::find(passed.begin(), passed.end(), to) == passed.end()
                                   && (edge->getPermissions() & SVC_PASSENGER) != 0
                                   && to->geometryLike(
                                       NBEdge::filterByPermissions(to->getIncomingEdges(), SVC_PASSENGER),
                                       NBEdge::filterByPermissions(to->getOutgoingEdges(), SVC_PASSENGER)));
                passed.push_back(to);
                if (goStraight) {
                    cur = cur->getStraightContinuation(SVC_PASSENGER);
                    if (cur != nullptr) {
                        to = cur->getToNode();
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            // allow higher threshold at larger junctions
            double longThreshold = maxDist + SUMO_const_laneWidth * MAX2(0, maxPassengerLanes - 1);
#ifdef DEBUG_JOINJUNCTIONS
            if (gDebugFlag1) {
                std::cout << "check edge length " << edge->getID() << " (" << length << ", passed=" << passed.size() << ", max=" << longThreshold << ")\n";
            }
#endif
            if (length > longThreshold) {
                // we found an edge that should not be removed. Maybe we can
                // still keep the start or end in the cluster
                // (keep the start if the end can be removed and vice versa)
                const bool keepStart = getClusterNeighbors(passed.back(), cluster).size() == 1;
                const bool keepEnd = !keepStart && getClusterNeighbors(n, cluster).size() == 1;
#ifdef DEBUG_JOINJUNCTIONS
                if (gDebugFlag1) {
                    std::cout << "node=" << n->getID() << " long edge " << edge->getID() << " (" << length << ", passed=" << toString(passed) << ", max=" << longThreshold << ") keepStart=" << keepStart << " keepEnd=" << keepEnd << "\n";
                }
#endif
                if (!keepStart) {
                    toRemove.insert(n);
                }
                toRemove.insert(passed.begin(), passed.end() - 1);
                if (!keepEnd) {
                    toRemove.insert(passed.back());
                }

            }
        }
    }
    for (std::set<NBNode*>::iterator j = toRemove.begin(); j != toRemove.end(); ++j) {
        cluster.erase(*j);
    }
}


NodeSet
NBNodeCont::getClusterNeighbors(const NBNode* n, NodeSet& cluster) {
    NodeSet result;
    for (NBEdge* e : n->getEdges()) {
        NBNode* neighbor = e->getFromNode() == n ? e->getToNode() : e->getFromNode();
        if (cluster.count(neighbor) != 0) {
            result.insert(neighbor);
        }
    }
    return result;
}


void
NBNodeCont::pruneSlipLaneNodes(NodeSet& cluster) const {
#ifdef DEBUG_JOINJUNCTIONS
    if (gDebugFlag1) {
        std::cout << "pruning slip-lanes at cluster=" << joinNamedToString(cluster, ' ') << "\n";
    }
#endif
    // fringe has already been removed
    if (cluster.size() <= 2) {
        return;
    }
    NodeSet toRemove;
    for (NBNode* n : cluster) {
        EdgeVector outgoing;
        double inAngle;
        // find slip lanes where the start is part of the cluster
        if (maybeSlipLaneStart(n, outgoing, inAngle)) {
            // potential slip lane start but we don't know which of the outgoing edges it is
#ifdef DEBUG_JOINJUNCTIONS
            if (gDebugFlag1) {
                std::cout << "   candidate slip-lane start=" << n->getID() << " outgoing=" << toString(outgoing) << "\n";
            }
#endif
            for (NBEdge* contEdge : outgoing) {
                if ((contEdge->getPermissions() & SVC_PASSENGER) == 0) {
                    continue;
                }
                double slipLength = contEdge->getLength();
                NBNode* cont = contEdge->getToNode();
                NodeSet cands;
                cands.insert(n);
                while (cont->getIncomingEdges().size() == 1 && cont->getOutgoingEdges().size() == 1 && slipLength < MAX_SLIPLANE_LENGTH) {
                    if (cands.count(cont) != 0) {
                        break; // circle, should not happen
                    }
                    cands.insert(cont);
#ifdef DEBUG_JOINJUNCTIONS
                    if (gDebugFlag1) {
                        std::cout << "   candidate slip-lane cont=" << cont->getID() << "\n";
                    }
#endif
                    NBEdge* next = cont->getOutgoingEdges().front();
                    slipLength += next->getLength();
                    cont = next->getToNode();
                }
#ifdef DEBUG_JOINJUNCTIONS
                if (gDebugFlag1) {
                    std::cout << "   candidate slip-lane end=" << cont->getID() << " slipLength=" << slipLength << "\n";
                }
#endif
                if (cont->getIncomingEdges().size() >= 2 && cont->getOutgoingEdges().size() == 1 &&
                        // slip lanes are for turning so there needs to be a sufficient angle
                        abs(NBHelpers::relAngle(inAngle, cont->getOutgoingEdges().front()->getAngleAtNode(cont))) > 45) {
                    // check whether the other continuation at n is also connected to the sliplane end
                    NBEdge* otherEdge = (contEdge == outgoing.front() ? outgoing.back() : outgoing.front());
                    double otherLength = otherEdge->getLength();
                    NBNode* cont2 = otherEdge->getToNode();

                    NodeSet visited;
                    visited.insert(n);
                    std::vector<NodeAndDist> toProc;
                    toProc.push_back(std::make_pair(cont2, otherLength));
                    bool found = false;
                    while (!toProc.empty()) {
                        NodeAndDist nodeAndDist = toProc.back();
                        NBNode* cont2 = nodeAndDist.first;
                        double dist = nodeAndDist.second;
#ifdef DEBUG_JOINJUNCTIONS
                        if (gDebugFlag1) {
                            std::cout << "   search alternative cont2=" << cont2->getID() << " dist=" << dist << "\n";
                        }
#endif
                        toProc.pop_back();
                        if (visited.find(cont2) != visited.end()) {
                            continue;
                        }
                        visited.insert(cont2);
                        if (cont2 == cont) {
                            found = true;
                            break;
                        }
                        for (NBEdge* e : cont2->getOutgoingEdges()) {
                            const double dist2 = dist + e->getLength();
                            if (dist2 < slipLength * 2 && (e->getPermissions() & SVC_PASSENGER) != 0) {
                                toProc.push_back(std::make_pair(e->getToNode(), dist2));
                            }
                        }
                    }
                    if (found) {
                        // found slip lane
                        cands.insert(cont);
                        toRemove.insert(cands.begin(), cands.end());
#ifdef DEBUG_JOINJUNCTIONS
                        if (gDebugFlag1) {
                            std::cout << "   found slip-lane with nodes=" << joinNamedToString(cands, ' ') << "\n";
                        }
#endif
                    }
                }
            }
        }

        EdgeVector incoming;
        double outAngle;
        // find slip lanes where the end is part of the cluster
        if (maybeSlipLaneEnd(n, incoming, outAngle)) {
            // potential slip lane end but we don't know which of the incoming edges it is
#ifdef DEBUG_JOINJUNCTIONS
            if (gDebugFlag1) {
                std::cout << "   candidate slip-lane end=" << n->getID() << " incoming=" << toString(incoming) << "\n";
            }
#endif
            for (NBEdge* contEdge : incoming) {
                if ((contEdge->getPermissions() & SVC_PASSENGER) == 0) {
                    continue;
                }
                double slipLength = contEdge->getLength();
                NBNode* cont = contEdge->getFromNode();
                NodeSet cands;
                cands.insert(n);
                while (cont->getIncomingEdges().size() == 1 && cont->getOutgoingEdges().size() == 1 && slipLength < MAX_SLIPLANE_LENGTH) {
                    if (cands.count(cont) != 0) {
                        break; // circle, should not happen
                    }
                    cands.insert(cont);
#ifdef DEBUG_JOINJUNCTIONS
                    if (gDebugFlag1) {
                        std::cout << "   candidate slip-lane cont=" << cont->getID() << "\n";
                    }
#endif
                    NBEdge* next = cont->getIncomingEdges().front();
                    slipLength += next->getLength();
                    cont = next->getFromNode();
                }
#ifdef DEBUG_JOINJUNCTIONS
                if (gDebugFlag1) {
                    std::cout << "   candidate slip-lane start=" << cont->getID() << " slipLength=" << slipLength << "\n";
                }
#endif
                if (cont->getOutgoingEdges().size() >= 2 && cont->getIncomingEdges().size() == 1 &&
                        // slip lanes are for turning so there needs to be a sufficient angle
                        abs(NBHelpers::relAngle(outAngle, cont->getIncomingEdges().front()->getAngleAtNode(cont))) > 45) {
                    // check whether the other continuation at n is also connected to the sliplane end
                    NBEdge* otherEdge = (contEdge == incoming.front() ? incoming.back() : incoming.front());
                    double otherLength = otherEdge->getLength();
                    NBNode* cont2 = otherEdge->getFromNode();

                    NodeSet visited;
                    visited.insert(n);
                    std::vector<NodeAndDist> toProc;
                    toProc.push_back(std::make_pair(cont2, otherLength));
                    bool found = false;
                    while (!toProc.empty()) {
                        NodeAndDist nodeAndDist = toProc.back();
                        NBNode* cont2 = nodeAndDist.first;
                        double dist = nodeAndDist.second;
#ifdef DEBUG_JOINJUNCTIONS
                        if (gDebugFlag1) {
                            std::cout << "   search alternative cont2=" << cont2->getID() << " dist=" << dist << "\n";
                        }
#endif
                        toProc.pop_back();
                        if (visited.find(cont2) != visited.end()) {
                            continue;
                        }
                        visited.insert(cont2);
                        if (cont2 == cont) {
                            found = true;
                            break;
                        }
                        for (NBEdge* e : cont2->getIncomingEdges()) {
                            const double dist2 = dist + e->getLength();
                            if (dist2 < slipLength * 2 && (e->getPermissions() & SVC_PASSENGER) != 0) {
                                toProc.push_back(std::make_pair(e->getFromNode(), dist2));
                            }
                        }
                    }
                    if (found) {
                        // found slip lane
                        cands.insert(cont);
                        toRemove.insert(cands.begin(), cands.end());
#ifdef DEBUG_JOINJUNCTIONS
                        if (gDebugFlag1) {
                            std::cout << "   found slip-lane start with nodes=" << joinNamedToString(cands, ' ') << "\n";
                        }
#endif
                    }
                }
            }
        }



    }
    int numRemoved = 0;
    for (NBNode* n : toRemove) {
        numRemoved += (int)cluster.erase(n);
    }
    if (numRemoved > 0) {
#ifdef DEBUG_JOINJUNCTIONS
        if (gDebugFlag1) {
            std::cout << "   removed " << numRemoved << " nodes from cluster: " << joinNamedToString(toRemove, ' ') << "\n";
        }
#endif
        pruneClusterFringe(cluster);
    }
}


bool
NBNodeCont::maybeSlipLaneStart(const NBNode* n, EdgeVector& outgoing, double& inAngle) const {
    if (n->getIncomingEdges().size() == 1 && n->getOutgoingEdges().size() == 2) {
        outgoing.insert(outgoing.begin(), n->getOutgoingEdges().begin(), n->getOutgoingEdges().end());
        inAngle = n->getIncomingEdges().front()->getAngleAtNode(n);
        return true;
    } else if (n->getIncomingEdges().size() >= 2 && n->getOutgoingEdges().size() == 3) {
        // check if the incoming edges are going in opposite directions and then
        // use the incoming edge that has 2 almost-straight outgoing edges
        const double inRelAngle = fabs(NBHelpers::relAngle(n->getIncomingEdges().front()->getAngleAtNode(n), n->getIncomingEdges().back()->getAngleAtNode(n)));
        //std::cout << "n=" << n->getID() << " inRelAngle=" << inRelAngle << "\n";
        if (inRelAngle < 135) {
            return false; // not opposite incoming
        }
        for (NBEdge* in : n->getIncomingEdges()) {
            EdgeVector straight;
            int numReverse = 0;
            for (NBEdge* out : n->getOutgoingEdges()) {
                const double outRelAngle = fabs(NBHelpers::relAngle(in->getAngleAtNode(n), out->getAngleAtNode(n)));
                if (outRelAngle <= 45) {
                    straight.push_back(out);
                } else if (outRelAngle >= 135) {
                    numReverse++;
                }
            }
            if (straight.size() == 2 && numReverse == 1) {
                outgoing.insert(outgoing.begin(), straight.begin(), straight.end());
                inAngle = in->getAngleAtNode(n);
                return true;
            }
        }
    }
    return false;
}


bool
NBNodeCont::maybeSlipLaneEnd(const NBNode* n, EdgeVector& incoming, double& outAngle) const {
    if (n->getIncomingEdges().size() == 2 && n->getOutgoingEdges().size() == 1) {
        incoming.insert(incoming.begin(), n->getIncomingEdges().begin(), n->getIncomingEdges().end());
        outAngle = n->getOutgoingEdges().front()->getAngleAtNode(n);
        return true;
    } else if (n->getIncomingEdges().size() == 3 && n->getOutgoingEdges().size() >= 2) {
        // check if the outgoing edges are going in opposite directions and then
        // use the outgoing edge that has 2 almost-straight incoming edges
        const double outRelAngle = fabs(NBHelpers::relAngle(n->getOutgoingEdges().front()->getAngleAtNode(n), n->getOutgoingEdges().back()->getAngleAtNode(n)));
        //std::cout << "n=" << n->getID() << " outRelAngle=" << outRelAngle << "\n";
        if (outRelAngle < 135) {
            return false; // not opposite outgoing
        }
        for (NBEdge* out : n->getOutgoingEdges()) {
            EdgeVector straight;
            int numReverse = 0;
            for (NBEdge* in : n->getIncomingEdges()) {
                const double inRelAngle = fabs(NBHelpers::relAngle(in->getAngleAtNode(n), out->getAngleAtNode(n)));
                if (inRelAngle <= 45) {
                    straight.push_back(in);
                } else if (inRelAngle >= 135) {
                    numReverse++;
                }
            }
            if (straight.size() == 2 && numReverse == 1) {
                incoming.insert(incoming.begin(), straight.begin(), straight.end());
                outAngle = out->getAngleAtNode(n);
                return true;
            }
        }
    }
    return false;
}

bool
NBNodeCont::feasibleCluster(const NodeSet& cluster, const NBEdgeCont& ec, const NBPTStopCont& sc, std::string& reason) const {
    // check for clusters which are to complex and probably won't work very well
    // we count the incoming edges of the final junction
    std::map<std::string, double> finalIncomingAngles;
    std::map<std::string, double> finalOutgoingAngles;
    for (NodeSet::const_iterator j = cluster.begin(); j != cluster.end(); ++j) {
        for (EdgeVector::const_iterator it_edge = (*j)->getIncomingEdges().begin(); it_edge != (*j)->getIncomingEdges().end(); ++it_edge) {
            NBEdge* edge = *it_edge;
            if (cluster.count(edge->getFromNode()) == 0 && (edge->getPermissions() & SVC_PASSENGER) != 0) {
                // incoming edge, does not originate in the cluster
                finalIncomingAngles[edge->getID()] = edge->getAngleAtNode(edge->getToNode());
            }
        }
        for (EdgeVector::const_iterator it_edge = (*j)->getOutgoingEdges().begin(); it_edge != (*j)->getOutgoingEdges().end(); ++it_edge) {
            NBEdge* edge = *it_edge;
            if (cluster.count(edge->getToNode()) == 0 && (edge->getPermissions() & SVC_PASSENGER) != 0) {
                // outgoing edge, does not end in the cluster
                finalOutgoingAngles[edge->getID()] = edge->getAngleAtNode(edge->getFromNode());
            }
        }

    }
#ifdef DEBUG_JOINJUNCTIONS
    for (NBNode* n : cluster) {
        if (DEBUGCOND(n)) {
            std::cout << "feasibleCluster c=" << joinNamedToString(cluster, ',')
                      << "\n inAngles=" << joinToString(finalIncomingAngles, ' ', ':')
                      << "\n outAngles=" << joinToString(finalOutgoingAngles, ' ', ':')
                      << "\n";
        }
    }
#endif
    if (finalIncomingAngles.size() > 4) {
        reason = toString(finalIncomingAngles.size()) + " incoming edges";
        return false;
    }
    // check for incoming parallel edges
    const double PARALLEL_INCOMING_THRESHOLD = 10.0;
    bool foundParallel = false;
    for (std::map<std::string, double>::const_iterator j = finalIncomingAngles.begin(); j != finalIncomingAngles.end() && !foundParallel; ++j) {
        std::map<std::string, double>::const_iterator k = j;
        for (++k; k != finalIncomingAngles.end() && !foundParallel; ++k) {
            if (fabs(j->second - k->second) < PARALLEL_INCOMING_THRESHOLD) {
                reason = "parallel incoming " + j->first + "," + k->first;
                return false;
            }
        }
    }
    // check for outgoing parallel edges
    for (std::map<std::string, double>::const_iterator j = finalOutgoingAngles.begin(); j != finalOutgoingAngles.end() && !foundParallel; ++j) {
        std::map<std::string, double>::const_iterator k = j;
        for (++k; k != finalOutgoingAngles.end() && !foundParallel; ++k) {
            if (fabs(j->second - k->second) < PARALLEL_INCOMING_THRESHOLD) {
                reason = "parallel outgoing " + j->first + "," + k->first;
                return false;
            }
        }
    }
    // check for stop edges within the cluster
    if (OptionsCont::getOptions().isSet("ptstop-output")) {
        for (auto it = sc.begin(); it != sc.end(); it++) {
            NBEdge* edge = ec.retrieve(it->second->getEdgeId());
            if (edge != nullptr && cluster.count(edge->getFromNode()) != 0 && cluster.count(edge->getToNode()) != 0) {
                reason = "it contains stop '" + it->first + "'";
                return false;
            }
        }
    }
    int numTLS = 0;
    for (NBNode* n : cluster) {
        if (n->isTLControlled()) {
            numTLS++;
        };
    }
    const bool hasTLS = numTLS > 0;
    // prevent removal of long edges unless there is weak circle or a traffic light
    if (cluster.size() > 2) {
        // find the nodes with the biggests physical distance between them
        double maxDist = -1;
        NBEdge* maxEdge = nullptr;
        for (NBNode* n1 : cluster) {
            for (NBNode* n2 : cluster) {
                NBEdge* e1 = n1->getConnectionTo(n2);
                NBEdge* e2 = n2->getConnectionTo(n1);
                if (e1 != nullptr && e1->getLoadedLength() > maxDist) {
                    maxDist = e1->getLoadedLength();
                    maxEdge = e1;
                }
                if (e2 != nullptr && e2->getLoadedLength() > maxDist) {
                    maxDist = e2->getLoadedLength();
                    maxEdge = e2;
                }
            }
        }
#ifdef DEBUG_JOINJUNCTIONS
        for (NBNode* n : cluster) {
            if (DEBUGCOND(n)) {
                std::cout << "feasible hasTLS=" << hasTLS << " maxDist=" << maxDist << " maxEdge=" << maxEdge->getID() << "\n";
            }
        }
#endif
        if (!hasTLS && maxDist > 5) {
            // find a weak circle within cluster that does not use maxEdge
            std::vector<NBNode*> toCheck;
            std::set<NBNode*> visited;
            toCheck.push_back(maxEdge->getToNode());
            bool foundCircle = false;
            while (!toCheck.empty()) {
                NBNode* n = toCheck.back();
                if (n == maxEdge->getFromNode()) {
                    foundCircle = true;
                    break;
                }
                toCheck.pop_back();
                visited.insert(n);
                for (NBEdge* e : n->getEdges()) {
                    if (e != maxEdge) {
                        NBNode* cand = e->getFromNode() == n ? e->getToNode() : e->getFromNode();
                        if (visited.count(cand) == 0 && cluster.count(cand) != 0) {
                            toCheck.push_back(cand);
                        }
                    }
                }
            }
            if (!foundCircle) {
                reason = "not compact (maxEdge=" + maxEdge->getID() + " length=" + toString(maxDist) + ")";
                return false;
            }
        }
    }
    // prevent joining of simple merging/spreading structures
    if (!hasTLS && cluster.size() >= 2) {
        int entryNodes = 0;
        int exitNodes = 0;
        int outsideIncoming = 0;
        int outsideOutgoing = 0;
        int edgesWithin = 0;
        for (NBNode* n : cluster) {
            bool foundOutsideIncoming = false;
            for (NBEdge* e : n->getIncomingEdges()) {
                if (cluster.count(e->getFromNode()) == 0) {
                    // edge entering from outside the cluster
                    outsideIncoming++;
                    foundOutsideIncoming = true;
                } else {
                    edgesWithin++;
                }
            }
            if (foundOutsideIncoming) {
                entryNodes++;
            }
            bool foundOutsideOutgoing = false;
            for (NBEdge* e : n->getOutgoingEdges()) {
                if (cluster.count(e->getToNode()) == 0) {
                    // edge leaving cluster
                    outsideOutgoing++;
                    foundOutsideOutgoing = true;
                }
            }
            if (foundOutsideOutgoing) {
                exitNodes++;
            }
        }
        if (entryNodes < 2) {
            reason = "only 1 entry node";
            return false;
        }
        if (exitNodes < 2) {
            reason = "only 1 exit node";
            return false;
        }
        if (cluster.size() == 2) {
            if (edgesWithin == 1 && outsideIncoming < 3 && outsideOutgoing < 3) {
                reason = "only 1 edge within and no cross-traffic";
                return false;
            }
        }
    }
    return true;
}


bool
NBNodeCont::reduceToCircle(NodeSet& cluster, int circleSize, NodeSet startNodes, std::vector<NBNode*> cands) const {
    //std::cout << "reduceToCircle  cs=" << circleSize << " cands=" << toString(cands, ',') << " startNodes=" << joinNamedToString(startNodes, ',') << "\n";
    assert(circleSize >= 2);
    if ((int)cands.size() == circleSize) {
        if (cands.back()->getConnectionTo(cands.front()) != nullptr) {
            // cluster found
            NodeSet candCluster;
            candCluster.insert(cands.begin(), cands.end());
            pruneClusterFringe(candCluster);
            const bool feasible = (int)candCluster.size() == circleSize;
            if (feasible) {
                cluster.clear();
                cluster.insert(cands.begin(), cands.end());
            }
            return feasible;
        } else {
            return false;
        }
    }
    if ((int)cluster.size() <= circleSize || startNodes.size() == 0) {
        // no reduction possible
        //std::cout << "    abort\n";
        return false;
    }
    if (cands.size() == 0) {
        // try to find a circle starting from another start node
        NBEdge* e = shortestEdge(cluster, startNodes, cands);
        if (e != nullptr) {
            cands.push_back(e->getFromNode());
            startNodes.erase(e->getFromNode());
            if (reduceToCircle(cluster, circleSize, startNodes, cands)) {
                return true;
            } else {
                // try another start node
                return reduceToCircle(cluster, circleSize, startNodes);
            }
        }
    } else {
        NodeSet singleStart;
        singleStart.insert(cands.back());
        NBEdge* e = shortestEdge(cluster, singleStart, cands);
        if (e != nullptr) {
            std::vector<NBNode*> cands2(cands);
            cands2.push_back(e->getToNode());
            if (reduceToCircle(cluster, circleSize, startNodes, cands2)) {
                return true;
            }
        }
    }
    //std::cout << "    abort2\n";
    return false;
}


NBEdge*
NBNodeCont::shortestEdge(const NodeSet& cluster, const NodeSet& startNodes, const std::vector<NBNode*>& exclude) const {
    double minDist = std::numeric_limits<double>::max();
    NBEdge* result = nullptr;
    for (NBNode* n : startNodes) {
        for (NBEdge* e : n->getOutgoingEdges()) {
            NBNode* neigh = e->getToNode();
            if (cluster.count(neigh) != 0 && std::find(exclude.begin(), exclude.end(), neigh) == exclude.end()) {
                const double dist = n->getPosition().distanceTo2D(neigh->getPosition());
                //std::cout << "    e=" << e->getID() << " dist=" << dist << " minD=" << minDist << "\n";
                if (dist < minDist) {
                    minDist = dist;
                    result = e;
                }
            }
        }
    }
    //std::cout << "closestNeighbor startNodes=" << toString(startNodes) << " result=" << Named::getIDSecure(result) << "\n";
    return result;
}

void
NBNodeCont::joinNodeClusters(NodeClusters clusters,
                             NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc) {
    for (NodeSet cluster : clusters) {
        joinNodeCluster(cluster, dc, ec, tlc);
    }
}


void
NBNodeCont::joinNodeCluster(NodeSet cluster, NBDistrictCont& dc, NBEdgeCont& ec, NBTrafficLightLogicCont& tlc, NBNode* predefined) {
    const bool origNames = OptionsCont::getOptions().getBool("output.original-names");
    assert(cluster.size() > 1);
    Position pos;
    bool setTL;
    std::string id = "cluster";
    TrafficLightType type;
    SumoXMLNodeType nodeType = SumoXMLNodeType::UNKNOWN;
    analyzeCluster(cluster, id, pos, setTL, type, nodeType);
    NBNode* newNode = nullptr;
    if (predefined != nullptr) {
        newNode = predefined;
    } else {
        if (!insert(id, pos)) {
            // should not fail
            WRITE_WARNINGF("Could not join junctions %.", id);
            return;
        }
        newNode = retrieve(id);
    }
    std::string tlID = id;
    if (predefined != nullptr) {
        if (predefined->getType() != SumoXMLNodeType::UNKNOWN) {
            nodeType = predefined->getType();
        }
        Position ppos = predefined->getPosition();
        if (ppos.x() != Position::INVALID.x()) {
            pos.setx(ppos.x());
        }
        if (ppos.y() != Position::INVALID.y()) {
            pos.sety(ppos.y());
        }
        if (ppos.z() != Position::INVALID.z()) {
            pos.setz(ppos.z());
        }
    }
    newNode->reinit(pos, nodeType);
    if (setTL && !newNode->isTLControlled()) {
        NBTrafficLightDefinition* tlDef = new NBOwnTLDef(tlID, newNode, 0, type);
        if (!tlc.insert(tlDef)) {
            // actually, nothing should fail here
            delete tlDef;
            throw ProcessError("Could not allocate tls '" + id + "'.");
        }
    }
    // collect edges
    EdgeSet allEdges;
    for (NBNode* n : cluster) {
        const EdgeVector& edges = n->getEdges();
        allEdges.insert(edges.begin(), edges.end());
    }
    // determine edges with are incoming or fully inside
    EdgeSet clusterIncoming;
    EdgeSet inside;
    for (NBEdge* e : allEdges) {
        if (cluster.count(e->getToNode()) > 0) {
            if (cluster.count(e->getFromNode()) > 0) {
                inside.insert(e);
            } else {
                clusterIncoming.insert(e);
            }
        }
    }
#ifdef DEBUG_JOINJUNCTIONS
    std::cout << "joining cluster " << joinNamedToString(cluster, ' ') << "\n"
              << "  incoming=" << toString(clusterIncoming) << "\n"
              << "  inside=" << toString(inside) << "\n";
#endif

    // determine possible connectivity from outside edges
    std::map<NBEdge*, EdgeSet> reachable;
    for (NBEdge* e : clusterIncoming) {
        EdgeVector open;
        EdgeSet seen;
        open.push_back(e);
        while (open.size() > 0) {
            NBEdge* cur = open.back();
            //std::cout << "   e=" << e->getID() << " cur=" << cur->getID() << " open=" << toString(open) << "\n";
            seen.insert(cur);
            open.pop_back();
            if (cluster.count(cur->getToNode()) == 0) {
                //std::cout << "      continue\n";
                continue;
            }
            const auto& cons = cur->getConnections();
            if (cons.size() == 0 || ec.hasPostProcessConnection(cur->getID()) || cur->getStep() == NBEdge::EdgeBuildingStep::INIT) {
                // check permissions to determine reachability
                for (NBEdge* out : cur->getToNode()->getOutgoingEdges()) {
                    if (seen.count(out) == 0
                            && allEdges.count(out) != 0
                            && (out->getPermissions() & cur->getPermissions() & ~SVC_PEDESTRIAN) != 0) {
                        open.push_back(out);
                    }
                }
            } else {
                // check existing connections
                for (const auto& con : cons) {
                    if (con.toEdge != nullptr
                            && seen.count(con.toEdge) == 0
                            && allEdges.count(con.toEdge) != 0) {
                        open.push_back(con.toEdge);
                    }
                }
            }
        }
        seen.erase(e);
        for (NBEdge* reached : seen) {
            // filter out inside edges from reached
            if (inside.count(reached) == 0) {
                reachable[e].insert(reached);
            }
        }
#ifdef DEBUG_JOINJUNCTIONS
        std::cout << " reachable e=" << e->getID() << " seen=" << toString(seen) << " reachable=" << toString(reachable[e]) << "\n";
#endif
    }

    // remap and remove edges which are completely within the new intersection
    for (NBEdge* e : inside) {
        for (NBEdge* e2 : allEdges) {
            if (e != e2) {
                e2->replaceInConnections(e, e->getConnections());
            }
        }
        ec.extract(dc, e, true);
        allEdges.erase(e);
    }

    // remap edges which are incoming / outgoing
    for (NBEdge* e : allEdges) {
        std::vector<NBEdge::Connection> conns = e->getConnections();
        const bool outgoing = cluster.count(e->getFromNode()) > 0;
        NBNode* from = outgoing ? newNode : e->getFromNode();
        NBNode* to   = outgoing ? e->getToNode() : newNode;
        if (origNames) {
            if (outgoing) {
                e->setParameter("origFrom", e->getFromNode()->getID());
            } else {
                e->setParameter("origTo", e->getToNode()->getID());
            }
        }
        e->reinitNodes(from, to);
        // re-add connections which previously existed and may still valid.
        // connections to removed edges will be ignored
        for (std::vector<NBEdge::Connection>::iterator k = conns.begin(); k != conns.end(); ++k) {
            e->addLane2LaneConnection((*k).fromLane, (*k).toEdge, (*k).toLane, NBEdge::Lane2LaneInfoType::USER, false, (*k).mayDefinitelyPass);
            if ((*k).fromLane >= 0 && (*k).fromLane < e->getNumLanes() && e->getLaneStruct((*k).fromLane).connectionsDone) {
                // @note (see NIImporter_DlrNavteq::ConnectedLanesHandler)
                e->declareConnectionsAsLoaded(NBEdge::EdgeBuildingStep::INIT);
            }
        }
    }
    // disable connections that were impossible with the old topology
    for (NBEdge* in : newNode->getIncomingEdges()) {
        for (NBEdge* out : newNode->getOutgoingEdges()) {
            if (reachable[in].count(out) == 0 && !ec.hasPostProcessConnection(in->getID(), out->getID())) {
                //std::cout << " removeUnreachable in=" << in->getID() << " out=" << out->getID() << "\n";
                in->removeFromConnections(out, -1, -1, true, false, true);
            }
        }
    }

    // remove original nodes
    registerJoinedCluster(cluster);
    for (NBNode* n : cluster) {
        erase(n);
    }
}


void
NBNodeCont::registerJoinedCluster(const NodeSet& cluster) {
    std::set<std::string> ids;
    for (NBNode* n : cluster) {
        ids.insert(n->getID());
    }
    myJoinedClusters.push_back(ids);
}


void
NBNodeCont::analyzeCluster(NodeSet cluster, std::string& id, Position& pos,
                           bool& hasTLS, TrafficLightType& type, SumoXMLNodeType& nodeType) {
    id += "_" + joinNamedToString(cluster, '_');
    hasTLS = false;
    bool ambiguousType = false;
    for (NBNode* j : cluster) {
        pos.add(j->getPosition());
        // add a traffic light if any of the cluster members was controlled
        if (j->isTLControlled()) {
            if (!hasTLS) {
                // init type
                type = (*j->getControllingTLS().begin())->getType();
            } else if (type != (*j->getControllingTLS().begin())->getType()) {
                ambiguousType = true;
            }
            hasTLS = true;
        }
        SumoXMLNodeType otherType = j->getType();
        if (nodeType == SumoXMLNodeType::UNKNOWN) {
            nodeType = otherType;
        } else if (nodeType != otherType) {
            if (hasTLS) {
                nodeType = SumoXMLNodeType::TRAFFIC_LIGHT;
            } else {
                if ((nodeType != SumoXMLNodeType::PRIORITY && (nodeType != SumoXMLNodeType::NOJUNCTION || otherType != SumoXMLNodeType::PRIORITY))
                        || (otherType != SumoXMLNodeType::NOJUNCTION && otherType != SumoXMLNodeType::UNKNOWN && otherType != SumoXMLNodeType::PRIORITY)) {
                    WRITE_WARNINGF("Ambiguous node type for node cluster '%' (%,%), setting to '" + toString(SumoXMLNodeType::PRIORITY) + "'.", id, toString(nodeType), toString(otherType));
                }
                nodeType = SumoXMLNodeType::PRIORITY;
            }
        }
    }
    pos.mul(1.0 / cluster.size());
    if (ambiguousType) {
        type = SUMOXMLDefinitions::TrafficLightTypes.get(OptionsCont::getOptions().getString("tls.default-type"));
        WRITE_WARNINGF("Ambiguous traffic light type for node cluster '%', setting to '%'.", id, toString(type));
    }
}


// ----------- (Helper) methods for guessing/computing traffic lights
bool
NBNodeCont::shouldBeTLSControlled(const NodeSet& c, double laneSpeedThreshold) const {
    bool tooFast = false;
    double laneSpeedSum = 0;
    std::set<NBEdge*> seen;
    for (NBNode* j : c) {
        const EdgeVector& edges = j->getEdges();
        for (EdgeVector::const_iterator k = edges.begin(); k != edges.end(); ++k) {
            if (c.find((*k)->getFromNode()) != c.end() && c.find((*k)->getToNode()) != c.end()) {
                continue;
            }
            if (j->hasIncoming(*k)) {
                laneSpeedSum += (double)(*k)->getNumLanes() * (*k)->getLaneSpeed(0);
            }
            if ((*k)->getLaneSpeed(0) * 3.6 > 79) {
                tooFast = true;
            }
        }
    }
    //std::cout << " c=" << joinNamedToString(c, ' ') << " f=" << f << " size=" << c.size() << " thresh=" << laneSpeedThreshold << " tooFast=" << tooFast << "\n";
    return !tooFast && laneSpeedSum >= laneSpeedThreshold && c.size() != 0;
}

bool
NBNodeCont::onlyCrossings(const NodeSet& c) const {
    // check whether all component nodes are solely pedestrian crossings
    // (these work fine without joining)
    for (NBNode* node : c) {
        EdgeVector nonPedIncoming;
        EdgeVector nonPedOutgoing;
        for (NBEdge* e : node->getIncomingEdges()) {
            if (e->getPermissions() != SVC_PEDESTRIAN) {
                nonPedIncoming.push_back(e);
            }
        }
        for (NBEdge* e : node->getOutgoingEdges()) {
            if (e->getPermissions() != SVC_PEDESTRIAN) {
                nonPedOutgoing.push_back(e);
            }
        }
        if (!node->geometryLike(nonPedIncoming, nonPedOutgoing)) {
            //for (NBNode* node : c) {
            //    if (node->getID() == "2480337678") {
            //        std::cout << " node=" << node->getID() << " nonPedIncoming=" << toString(nonPedIncoming) << " nonPedOutgoing=" << toString(nonPedOutgoing) << "\n";
            //    }
            //}
            return false;
        }
    }
    return true;
}


bool
NBNodeCont::customTLID(const NodeSet& c) const {
    for (NBNode* node : c) {
        if (node->isTLControlled()) {
            const std::string tlID = (*node->getControllingTLS().begin())->getID();
            if (tlID != node->getID()
                    && !StringUtils::startsWith(tlID, "joinedS_")
                    && !StringUtils::startsWith(tlID, "joinedG_")
                    && !StringUtils::startsWith(tlID, "GS")) {
                return true;
            }
        }
    }
    return false;
}


void
NBNodeCont::guessTLs(OptionsCont& oc, NBTrafficLightLogicCont& tlc) {
    myGuessedTLS.clear();
    // build list of definitely not tls-controlled junctions
    const double laneSpeedThreshold = oc.getFloat("tls.guess.threshold");
    std::vector<NBNode*> ncontrolled;
    if (oc.isSet("tls.unset")) {
        std::vector<std::string> notTLControlledNodes = oc.getStringVector("tls.unset");
        for (std::vector<std::string>::const_iterator i = notTLControlledNodes.begin(); i != notTLControlledNodes.end(); ++i) {
            NBNode* n = NBNodeCont::retrieve(*i);
            if (n == nullptr) {
                throw ProcessError(" The junction '" + *i + "' to set as not-controlled is not known.");
            }
            std::set<NBTrafficLightDefinition*> tls = n->getControllingTLS();
            for (std::set<NBTrafficLightDefinition*>::const_iterator j = tls.begin(); j != tls.end(); ++j) {
                (*j)->removeNode(n);
            }
            n->removeTrafficLights();
            ncontrolled.push_back(n);
        }
    }

    TrafficLightType type = SUMOXMLDefinitions::TrafficLightTypes.get(OptionsCont::getOptions().getString("tls.default-type"));
    // loop#1 checking whether the node shall be tls controlled,
    //  because it is assigned to a district
    if (oc.exists("tls.taz-nodes") && oc.getBool("tls.taz-nodes")) {
        for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
            NBNode* cur = (*i).second;
            if (cur->isNearDistrict() && std::find(ncontrolled.begin(), ncontrolled.end(), cur) == ncontrolled.end()) {
                setAsTLControlled(cur, tlc, type);
            }
        }
    }

    // figure out which nodes mark the locations of TLS signals
    // This assumes nodes are already joined
    if (oc.exists("tls.guess-signals") && oc.getBool("tls.guess-signals")) {
        // prepare candidate edges
        const double signalDist = oc.getFloat("tls.guess-signals.dist");
        for (const auto& item : myNodes) {
            const NBNode* node = item.second;
            if (node->isTLControlled() && (node->getIncomingEdges().size() == 1 || node->geometryLike())) {
#ifdef DEBUG_GUESSSIGNALS
                if (DEBUGCOND(node) || true) {
                    std::cout << " propagate TLS from " << node->getID() << " downstream\n";
                }
#endif
                for (NBEdge* edge : node->getOutgoingEdges()) {
                    // do not overwrite closer signals
                    if (edge->getSignalOffset() == NBEdge::UNSPECIFIED_SIGNAL_OFFSET) {
                        edge->setSignalPosition(node->getPosition(), node);
                    }
                }
            }
        }
        std::set<NBEdge*> seen;
        std::set<NBEdge*> check;
        for (const auto& item : myNodes) {
            for (NBEdge* edge : item.second->getOutgoingEdges()) {
                if (edge->getSignalPosition() != Position::INVALID) {
                    check.insert(edge);
                    seen.insert(edge);
#ifdef DEBUG_GUESSSIGNALS
                    if (DEBUGCOND(edge->getSignalNode()) || true) {
                        std::cout << "   primary signalPosition edge=" << edge->getID() << " pos=" << edge->getSignalPosition() << "\n";
                    }
#endif
                }
            }
        }
        // propagate signal position until the next real intersection
        while (check.size() > 0) {
            NBEdge* const edge = *check.begin();
            check.erase(check.begin());
            seen.insert(edge);
            NBNode* const nextNode = edge->getToNode();
            if (nextNode->geometryLike() && !nextNode->isTLControlled()) {
                for (NBEdge* const outEdge : nextNode->getOutgoingEdges()) {
                    if (seen.count(outEdge) == 0) {
                        outEdge->setSignalPosition(edge->getSignalPosition(), edge->getSignalNode());
#ifdef DEBUG_GUESSSIGNALS
                        if (DEBUGCOND(edge->getSignalNode()) || true) {
                            std::cout << "   setSignalPosition edge=" << outEdge->getID() << " pos=" << edge->getSignalPosition() << "\n";
                        }
#endif
                        check.insert(outEdge);
                    }
                }
            }
        }

        // check which nodes should be controlled
        for (std::map<std::string, NBNode*>::const_iterator i = myNodes.begin(); i != myNodes.end(); ++i) {
            NBNode* node = i->second;
            if (find(ncontrolled.begin(), ncontrolled.end(), node) != ncontrolled.end()) {
                continue;
            }
            const EdgeVector& incoming = node->getIncomingEdges();
            const EdgeVector& outgoing = node->getOutgoingEdges();
            if (!node->isTLControlled() && incoming.size() > 1 && !node->geometryLike()
                    && !NBNodeTypeComputer::isRailwayNode(node)
                    && node->getType() != SumoXMLNodeType::RAIL_CROSSING) {
                std::vector<const NBNode*> signals;
                bool isTLS = true;
                // check if there is a signal at every incoming edge
                for (EdgeVector::const_iterator it_i = incoming.begin(); it_i != incoming.end(); ++it_i) {
                    const NBEdge* inEdge = *it_i;
                    if (inEdge->getSignalOffset() == NBEdge::UNSPECIFIED_SIGNAL_OFFSET && inEdge->getPermissions() != SVC_TRAM) {
#ifdef DEBUG_GUESSSIGNALS
                        if (DEBUGCOND(node)) {
                            std::cout << " noTLS, edge=" << inEdge->getID() << "\n";
                        }
#endif
                        isTLS = false;
                        break;
                    }
                }
                if (isTLS) {
                    node->updateSurroundingGeometry();
                    // check if all signals are within the required distance
                    // (requires detailed geometry computation)
                    for (EdgeVector::const_iterator it_i = incoming.begin(); it_i != incoming.end(); ++it_i) {
                        const NBEdge* inEdge = *it_i;
                        if ((inEdge->getSignalOffset() == NBEdge::UNSPECIFIED_SIGNAL_OFFSET || inEdge->getSignalOffset() > signalDist)
                                && inEdge->getPermissions() != SVC_TRAM) {
#ifdef DEBUG_GUESSSIGNALS
                            if (DEBUGCOND(node)) {
                                std::cout << " noTLS, edge=" << inEdge->getID() << " offset=" << inEdge->getSignalOffset() << " tlsPos=" << inEdge->getSignalPosition() << "\n";
                            }
#endif
                            isTLS = false;
                            break;
                        }
                        const NBNode* signal = inEdge->getSignalNode();
                        if (signal != nullptr) {
                            signals.push_back(signal);
                        }
                    }
                    // outgoing edges may be tagged with pedestrian crossings. These
                    // should also be merged into the main TLS
                    for (const NBEdge* outEdge : outgoing) {
                        NBNode* cand = outEdge->getToNode();
                        if (cand->isTLControlled() && cand->geometryLike() && outEdge->getLength() <= signalDist) {
#ifdef DEBUG_GUESSSIGNALS
                            if (DEBUGCOND(node)) {
                                std::cout << " node=" << node->getID() << " outEdge=" << outEdge->getID() << " signalNode=" << cand->getID() << " len=" << outEdge->getLength() << "\n";
                            }
#endif
                            signals.push_back(cand);
                        }
                    }
                }
                if (isTLS) {
                    for (const NBNode* s : signals) {
                        std::set<NBTrafficLightDefinition*> tls = s->getControllingTLS();
                        const_cast<NBNode*>(s)->reinit(s->getPosition(), SumoXMLNodeType::PRIORITY);
                        for (std::set<NBTrafficLightDefinition*>::iterator k = tls.begin(); k != tls.end(); ++k) {
                            tlc.removeFully(s->getID());
                        }
                    }
                    //if (true) std::cout << " node=" << node->getID() << " signals=" << toString(signals) << "\n";
                    NBTrafficLightDefinition* tlDef = new NBOwnTLDef("GS_" + node->getID(), node, 0, type);
                    // @todo patch endOffset for all incoming lanes according to the signal positions
                    if (!tlc.insert(tlDef)) {
                        // actually, nothing should fail here
                        WRITE_WARNINGF("Could not build joined tls '%'.", node->getID());
                        delete tlDef;
                        return;
                    }
                }
            }
        }
    }

    // guess joined tls first, if wished
    if (oc.getBool("tls.guess.joining")) {
        // get node clusters
        NodeClusters cands;
        generateNodeClusters(oc.getFloat("tls.join-dist"), cands);
        // check these candidates (clusters) whether they should be controlled by a tls
        for (NodeClusters::iterator i = cands.begin(); i != cands.end();) {
            NodeSet& c = (*i);
            // regard only junctions which are not yet controlled and are not
            //  forbidden to be controlled
            for (NodeSet::iterator j = c.begin(); j != c.end();) {
                if ((*j)->isTLControlled() || std::find(ncontrolled.begin(), ncontrolled.end(), *j) != ncontrolled.end()) {
                    c.erase(j++);
                } else {
                    ++j;
                }
            }
            // check whether the cluster should be controlled
            // to avoid gigantic clusters, assume that at most 4 nodes should be needed for a guessed-joined-tls
            if (c.size() == 0 || !shouldBeTLSControlled(c, laneSpeedThreshold * c.size() / MIN2((int)c.size(), 4))) {
                i = cands.erase(i);
            } else {
                ++i;
            }
        }
        // cands now only contain sets of junctions that shall be joined into being tls-controlled
        int index = 0;
        for (auto nodeSet : cands) {
            std::vector<NBNode*> nodes;
            for (NBNode* node : nodeSet) {
                nodes.push_back(node);
                myGuessedTLS.insert(node);
            }
            std::string id = "joinedG_" + toString(index++);
            NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, nodes, 0, type);
            if (!tlc.insert(tlDef)) {
                // actually, nothing should fail here
                WRITE_WARNING("Could not build guessed, joined tls.");
                delete tlDef;
                return;
            }
        }
    }

    // guess single tls
    if (oc.getBool("tls.guess")) {
        for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
            NBNode* cur = (*i).second;
            //  do nothing if already is tl-controlled
            if (cur->isTLControlled()) {
                continue;
            }
            // do nothing if in the list of explicit non-controlled junctions
            if (find(ncontrolled.begin(), ncontrolled.end(), cur) != ncontrolled.end()) {
                continue;
            }
            NodeSet c;
            c.insert(cur);
            if (!shouldBeTLSControlled(c, laneSpeedThreshold) || cur->geometryLike()) {
                continue;
            }
            setAsTLControlled(cur, tlc, type);
            myGuessedTLS.insert(cur);
        }
    }
}

void NBNodeCont::recheckGuessedTLS(NBTrafficLightLogicCont& tlc) {
    std::set<NBTrafficLightDefinition*> recompute;
    for (NBNode* node : myGuessedTLS) {
        if (!node->hasConflict()) {
            const std::set<NBTrafficLightDefinition*>& tlDefs = node->getControllingTLS();
            recompute.insert(tlDefs.begin(), tlDefs.end());
            node->removeTrafficLights(true);
            for (NBEdge* edge : node->getIncomingEdges()) {
                edge->clearControllingTLInformation();
            }
        }
    }
    for (NBTrafficLightDefinition* def : recompute) {
        if (def->getNodes().size() == 0) {
            tlc.removeFully(def->getID());
        } else {
            def->setParticipantsInformation();
            def->setTLControllingInformation();
            tlc.computeSingleLogic(OptionsCont::getOptions(), def);
        }
    }
}


void
NBNodeCont::joinTLS(NBTrafficLightLogicCont& tlc, double maxdist) {
    NodeClusters cands;
    generateNodeClusters(maxdist, cands);
    IDSupplier idSupplier("joinedS_");
    for (NodeSet& c : cands) {
        for (NodeSet::iterator j = c.begin(); j != c.end();) {
            if (!(*j)->isTLControlled()) {
                c.erase(j++);
            } else {
                ++j;
            }
        }
        if (c.size() < 2 || onlyCrossings(c) || customTLID(c)) {
            continue;
        }
        // figure out type of the joined TLS
        Position dummyPos;
        bool dummySetTL;
        std::string id = "joined"; // prefix (see #3871)
        TrafficLightType type;
        SumoXMLNodeType nodeType = SumoXMLNodeType::UNKNOWN;
        analyzeCluster(c, id, dummyPos, dummySetTL, type, nodeType);
        for (NBNode* j : c) {
            std::set<NBTrafficLightDefinition*> tls = j->getControllingTLS();
            j->removeTrafficLights();
            for (std::set<NBTrafficLightDefinition*>::iterator k = tls.begin(); k != tls.end(); ++k) {
                tlc.removeFully(j->getID());
            }
        }
        std::vector<NBNode*> nodes;
        for (NBNode* j : c) {
            nodes.push_back(j);
        }
        id = idSupplier.getNext();
        while (tlc.getPrograms(id).size() > 0) {
            id = idSupplier.getNext();
        }
        NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, nodes, 0, type);
        if (!tlc.insert(tlDef)) {
            // actually, nothing should fail here
            WRITE_WARNING("Could not build a joined tls.");
            delete tlDef;
            return;
        }
    }
}


void
NBNodeCont::setAsTLControlled(NBNode* node, NBTrafficLightLogicCont& tlc,
                              TrafficLightType type, std::string id) {
    if (id == "") {
        id = node->getID();
    }
    NBTrafficLightDefinition* tlDef = new NBOwnTLDef(id, node, 0, type);
    if (!tlc.insert(tlDef)) {
        // actually, nothing should fail here
        WRITE_WARNINGF("Building a tl-logic for junction '%' twice is not possible.", id);
        delete tlDef;
        return;
    }
}


// -----------
void
NBNodeCont::computeLanes2Lanes() {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->computeLanes2Lanes();
    }
}


// computes the "wheel" of incoming and outgoing edges for every node
void
NBNodeCont::computeLogics(const NBEdgeCont& ec) {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->computeLogic(ec);
    }
}


void
NBNodeCont::computeLogics2(const NBEdgeCont& ec, OptionsCont& oc) {
    std::set<NBNode*> roundaboutNodes;
    const bool checkLaneFoesAll = oc.getBool("check-lane-foes.all");
    const bool checkLaneFoesRoundabout = !checkLaneFoesAll && oc.getBool("check-lane-foes.roundabout");
    if (checkLaneFoesRoundabout) {
        const std::set<EdgeSet>& roundabouts = ec.getRoundabouts();
        for (std::set<EdgeSet>::const_iterator i = roundabouts.begin(); i != roundabouts.end(); ++i) {
            for (EdgeSet::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
                roundaboutNodes.insert((*j)->getToNode());
            }
        }
    }
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        const bool checkLaneFoes = checkLaneFoesAll || (checkLaneFoesRoundabout && roundaboutNodes.count((*i).second) > 0);
        (*i).second->computeLogic2(checkLaneFoes);
    }
}


void
NBNodeCont::clear() {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        delete ((*i).second);
    }
    myNodes.clear();
    for (auto& item : myExtractedNodes) {
        delete item.second;
    }
    myExtractedNodes.clear();
}


std::string
NBNodeCont::getFreeID() {
    int counter = 0;
    std::string freeID = "SUMOGenerated" + toString<int>(counter);
    // While there is a node with id equal to freeID
    while (retrieve(freeID) != nullptr) {
        // update counter and generate a new freeID
        counter++;
        freeID = "SUMOGenerated" + toString<int>(counter);
    }
    return freeID;
}


void
NBNodeCont::computeNodeShapes(double mismatchThreshold) {
    for (NodeCont::iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        (*i).second->computeNodeShape(mismatchThreshold);
    }
}


void
NBNodeCont::printBuiltNodesStatistics() const {
    int numUnregulatedJunctions = 0;
    int numDeadEndJunctions = 0;
    int numTrafficLightJunctions = 0;
    int numPriorityJunctions = 0;
    int numRightBeforeLeftJunctions = 0;
    int numAllWayStopJunctions = 0;
    int numZipperJunctions = 0;
    int numDistrictJunctions = 0;
    int numRailCrossing = 0;
    int numRailSignals = 0;
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); i++) {
        switch ((*i).second->getType()) {
            case SumoXMLNodeType::NOJUNCTION:
                ++numUnregulatedJunctions;
                break;
            case SumoXMLNodeType::DEAD_END:
                ++numDeadEndJunctions;
                break;
            case SumoXMLNodeType::TRAFFIC_LIGHT:
            case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED:
            case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
                ++numTrafficLightJunctions;
                break;
            case SumoXMLNodeType::PRIORITY:
            case SumoXMLNodeType::PRIORITY_STOP:
                ++numPriorityJunctions;
                break;
            case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
                ++numRightBeforeLeftJunctions;
                break;
            case SumoXMLNodeType::ALLWAY_STOP:
                ++numAllWayStopJunctions;
                break;
            case SumoXMLNodeType::ZIPPER:
                ++numZipperJunctions;
                break;
            case SumoXMLNodeType::DISTRICT:
                ++numDistrictJunctions;
                break;
            case SumoXMLNodeType::RAIL_CROSSING:
                ++numRailCrossing;
                break;
            case SumoXMLNodeType::RAIL_SIGNAL:
                ++numRailSignals;
                break;
            case SumoXMLNodeType::UNKNOWN:
                // should not happen
                break;
            default:
                break;
        }
    }
    WRITE_MESSAGE(" Node type statistics:");
    WRITE_MESSAGE("  Unregulated junctions       : " + toString(numUnregulatedJunctions));
    if (numDeadEndJunctions > 0) {
        WRITE_MESSAGE("  Dead-end junctions          : " + toString(numDeadEndJunctions));
    }
    WRITE_MESSAGE("  Priority junctions          : " + toString(numPriorityJunctions));
    WRITE_MESSAGE("  Right-before-left junctions : " + toString(numRightBeforeLeftJunctions));
    if (numTrafficLightJunctions > 0) {
        WRITE_MESSAGE("  Traffic light junctions      : " + toString(numTrafficLightJunctions));
    }
    if (numAllWayStopJunctions > 0) {
        WRITE_MESSAGE("  All-way stop junctions      : " + toString(numAllWayStopJunctions));
    }
    if (numZipperJunctions > 0) {
        WRITE_MESSAGE("  Zipper-merge junctions      : " + toString(numZipperJunctions));
    }
    if (numRailCrossing > 0) {
        WRITE_MESSAGE("  Rail crossing junctions      : " + toString(numRailCrossing));
    }
    if (numRailSignals > 0) {
        WRITE_MESSAGE("  Rail signal junctions      : " + toString(numRailSignals));
    }
    if (numDistrictJunctions > 0) {
        WRITE_MESSAGE("  District junctions      : " + toString(numDistrictJunctions));
    }
}


std::vector<std::string>
NBNodeCont::getAllNames() const {
    std::vector<std::string> ret;
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); ++i) {
        ret.push_back((*i).first);
    }
    return ret;
}


void
NBNodeCont::rename(NBNode* node, const std::string& newID) {
    if (myNodes.count(newID) != 0) {
        throw ProcessError("Attempt to rename node using existing id '" + newID + "'");
    }
    myNodes.erase(node->getID());
    node->setID(newID);
    myNodes[newID] = node;
}


void
NBNodeCont::discardTrafficLights(NBTrafficLightLogicCont& tlc, bool geometryLike, bool guessSignals) {
    for (NodeCont::const_iterator i = myNodes.begin(); i != myNodes.end(); ++i) {
        NBNode* node = i->second;
        if (node->isTLControlled() && (!geometryLike || node->geometryLike())) {
            // make a copy of tldefs
            const std::set<NBTrafficLightDefinition*> tldefs = node->getControllingTLS();
            if (geometryLike && (*tldefs.begin())->getNodes().size() > 1) {
                // do not remove joined tls when only removing geometry-like tls
                continue;
            }
            if (guessSignals && node->isTLControlled() && node->geometryLike()) {
                // record signal location
                for (NBEdge* edge : node->getOutgoingEdges()) {
                    edge->setSignalPosition(node->getPosition(), nullptr);
#ifdef DEBUG_GUESSSIGNALS
                    std::cout << "   discard-simple " << node->getID() << "  edge=" << edge->getID() << " pos=" << edge->getSignalPosition() << "\n";
#endif
                }
            }
            for (std::set<NBTrafficLightDefinition*>::const_iterator it = tldefs.begin(); it != tldefs.end(); ++it) {
                NBTrafficLightDefinition* tlDef = *it;
                node->removeTrafficLight(tlDef);
                tlc.extract(tlDef);
            }
            SumoXMLNodeType newType = NBNodeTypeComputer::isRailwayNode(node) ? SumoXMLNodeType::RAIL_SIGNAL : SumoXMLNodeType::UNKNOWN;
            node->reinit(node->getPosition(), newType);
        }
    }
}


void
NBNodeCont::discardRailSignals() {
    for (auto& item : myNodes) {
        NBNode* node = item.second;
        if (node->getType() == SumoXMLNodeType::RAIL_SIGNAL) {
            node->reinit(node->getPosition(), SumoXMLNodeType::PRIORITY);
        }
    }
}


int
NBNodeCont::remapIDs(bool numericaIDs, bool reservedIDs, const std::string& prefix) {
    bool startGiven = !OptionsCont::getOptions().isDefault("numerical-ids.node-start");
    std::vector<std::string> avoid;
    if (startGiven) {
        avoid.push_back(toString(OptionsCont::getOptions().getInt("numerical-ids.node-start") - 1));
    } else {
        avoid = getAllNames();
    }
    std::set<std::string> reserve;
    if (reservedIDs) {
        NBHelpers::loadPrefixedIDsFomFile(OptionsCont::getOptions().getString("reserved-ids"), "node:", reserve); // backward compatibility
        NBHelpers::loadPrefixedIDsFomFile(OptionsCont::getOptions().getString("reserved-ids"), "junction:", reserve); // selection format
        avoid.insert(avoid.end(), reserve.begin(), reserve.end());
    }
    IDSupplier idSupplier("", avoid);
    NodeSet toChange;
    for (NodeCont::iterator it = myNodes.begin(); it != myNodes.end(); it++) {
        if (startGiven) {
            toChange.insert(it->second);
            continue;
        }
        if (numericaIDs) {
            try {
                StringUtils::toLong(it->first);
            } catch (NumberFormatException&) {
                toChange.insert(it->second);
            }
        }
        if (reservedIDs && reserve.count(it->first) > 0) {
            toChange.insert(it->second);
        }
    }
    const bool origNames = OptionsCont::getOptions().getBool("output.original-names");
    for (NBNode* node : toChange) {
        myNodes.erase(node->getID());
    }
    for (NBNode* node : toChange) {
        if (origNames) {
            node->setParameter(SUMO_PARAM_ORIGID, node->getID());
        }
        node->setID(idSupplier.getNext());
        myNodes[node->getID()] = node;
    }
    if (prefix.empty()) {
        return (int)toChange.size();
    } else {
        int renamed = 0;
        // make a copy because we will modify the map
        auto oldNodes = myNodes;
        for (auto item : oldNodes) {
            if (!StringUtils::startsWith(item.first, prefix)) {
                rename(item.second, prefix + item.first);
                renamed++;
            }
        }
        return renamed;
    }
}


int
NBNodeCont::guessFringe() {
    NodeSet topRightFront;
    NodeSet topLeftFront;
    NodeSet bottomRightFront;
    NodeSet bottomLeftFront;
    for (const auto& item : myNodes) {
        paretoCheck(item.second, topRightFront, 1, 1);
        paretoCheck(item.second, topLeftFront, -1, 1);
        paretoCheck(item.second, bottomRightFront, 1, -1);
        paretoCheck(item.second, bottomLeftFront, -1, -1);
    }
    NodeSet front;
    front.insert(topRightFront.begin(), topRightFront.end());
    front.insert(topLeftFront.begin(), topLeftFront.end());
    front.insert(bottomRightFront.begin(), bottomRightFront.end());
    front.insert(bottomLeftFront.begin(), bottomLeftFront.end());
    int numFringe = 0;
    for (NBNode* n : front) {
        const int in = (int)n->getIncomingEdges().size();
        const int out = (int)n->getOutgoingEdges().size();
        if ((in <= 1 && out <= 1) &&
                (in == 0 || out == 0
                 || n->getIncomingEdges().front()->isTurningDirectionAt(n->getOutgoingEdges().front()))) {
            n->setFringeType(FringeType::OUTER);
            numFringe++;
        }
    }
    return numFringe;
}


void
NBNodeCont::paretoCheck(NBNode* node, NodeSet& frontier, int xSign, int ySign) {
    const double x = node->getPosition().x() * xSign;
    const double y = node->getPosition().y() * ySign;
    std::vector<NBNode*> dominated;
    for (NBNode* fn : frontier) {
        const double x2 = fn->getPosition().x() * xSign;
        const double y2 = fn->getPosition().y() * ySign;
        if (x2 >= x && y2 >= y) {
            return;
        } else if (x2 <= x && y2 <= y) {
            dominated.push_back(fn);
        }
    }
    frontier.insert(node);
    for (NBNode* r : dominated) {
        frontier.erase(r);
    }
}


/****************************************************************************/
