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
/// @file    NBTrafficLightDefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The base class for traffic light logic definitions
/****************************************************************************/
#include <config.h>

#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <iterator>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "NBTrafficLightDefinition.h"
#include <utils/options/OptionsCont.h>
#include "NBLinkPossibilityMatrix.h"
#include "NBTrafficLightLogic.h"
#include "NBOwnTLDef.h"
#include "NBContHelper.h"

//#define DEBUG_RIGHT_OF_WAY
#define DEBUGCOND true

// ===========================================================================
// static members
// ===========================================================================
const std::string NBTrafficLightDefinition::DefaultProgramID = "0";
const std::string NBTrafficLightDefinition::DummyID = "dummy";
const SUMOTime NBTrafficLightDefinition::UNSPECIFIED_DURATION(-1);

// ===========================================================================
// method definitions
// ===========================================================================
NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string& id,
        const std::vector<NBNode*>& junctions, const std::string& programID,
        SUMOTime offset, TrafficLightType type) :
    Named(id),
    myControlledNodes(junctions),
    mySubID(programID), myOffset(offset),
    myType(type),
    myNeedsContRelationReady(false),
    myRightOnRedConflictsReady(false) {
    std::vector<NBNode*>::iterator i = myControlledNodes.begin();
    while (i != myControlledNodes.end()) {
        for (std::vector<NBNode*>::iterator j = i + 1; j != myControlledNodes.end();) {
            if (*i == *j) {
                j = myControlledNodes.erase(j);
            } else {
                j++;
            }
        }
        i++;
    }
    std::sort(myControlledNodes.begin(), myControlledNodes.end(), NBNode::nodes_by_id_sorter());
    for (std::vector<NBNode*>::const_iterator i = junctions.begin(); i != junctions.end(); i++) {
        (*i)->addTrafficLight(this);
    }
}


NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string& id,
        NBNode* junction, const std::string& programID, SUMOTime offset, TrafficLightType type) :
    Named(id),
    mySubID(programID),
    myOffset(offset),
    myType(type),
    myNeedsContRelationReady(false),
    myRightOnRedConflictsReady(false) {
    addNode(junction);
}


NBTrafficLightDefinition::NBTrafficLightDefinition(const std::string& id, const std::string& programID,
        SUMOTime offset, TrafficLightType type) :
    Named(id),
    mySubID(programID),
    myOffset(offset),
    myType(type),
    myNeedsContRelationReady(false),
    myRightOnRedConflictsReady(false) {
}


NBTrafficLightDefinition::~NBTrafficLightDefinition() {}


NBTrafficLightLogic*
NBTrafficLightDefinition::compute(OptionsCont& oc) {
    // it is not really a traffic light if no incoming edge exists
    if (amInvalid()) {
        // make a copy of myControlledNodes because it will be modified;
        std::vector<NBNode*> nodes = myControlledNodes;
        for (auto it : nodes) {
            it->removeTrafficLight(this);
        }
        WRITE_WARNING("The traffic light '" + getID() + "' does not control any links; it will not be build.");
        return nullptr;
    }
    // compute the time needed to brake
    int brakingTime = computeBrakingTime(oc.getFloat("tls.yellow.min-decel"));
    // perform the computation depending on whether the traffic light
    //  definition was loaded or shall be computed new completely
    if (!oc.isDefault("tls.yellow.time")) {
        brakingTime = oc.getInt("tls.yellow.time");
    }
    NBTrafficLightLogic* ret = myCompute(brakingTime);
    ret->updateParameters(getParametersMap());
    return ret;
}


bool
NBTrafficLightDefinition::amInvalid() const {
    return myControlledLinks.size() == 0;
}


int
NBTrafficLightDefinition::computeBrakingTime(double minDecel) const {
    if (myIncomingEdges.size() == 0) {
        // don't crash
        return 3;
    }
    double vmax = NBContHelper::maxSpeed(myIncomingEdges);
    if (vmax < 71 / 3.6) {
        // up to 50kmh: 3 seconds , 60km/h: 4, 70kmh: 5
        // @note: these are German regulations, other countries may differ
        return 3 + (int)MAX2(0.0, (floor((vmax - 50 / 3.6) * 0.37)));
    } else {
        // above 70km/h we use a function that grows according to the "natural"
        // formula (vmax / 2 * minDecel) but continues smoothly where the german
        // rules leave of
        return (int)(1.8 + vmax / 2 / minDecel);
    }
}


void
NBTrafficLightDefinition::setParticipantsInformation() {
    // collect the information about participating edges and links
    collectEdges();
    collectLinks();
}

std::set<NBEdge*>
NBTrafficLightDefinition::collectReachable(EdgeVector outer, const EdgeVector& within, bool checkControlled) {
    std::set<NBEdge*> reachable;
    while (outer.size() > 0) {
        NBEdge* from = outer.back();
        outer.pop_back();
        std::vector<NBEdge::Connection>& cons = from->getConnections();
        for (std::vector<NBEdge::Connection>::iterator k = cons.begin(); k != cons.end(); k++) {
            NBEdge* to = (*k).toEdge;
            if (reachable.count(to) == 0 &&
                    (find(within.begin(), within.end(), to) != within.end()) &&
                    (!checkControlled || from->mayBeTLSControlled((*k).fromLane, to, (*k).toLane))) {
                reachable.insert(to);
                outer.push_back(to);
            }
        }
    }
    return reachable;
}


void
NBTrafficLightDefinition::collectEdges() {
    myIncomingEdges.clear();
    myEdgesWithin.clear();
    EdgeVector myOutgoing;
    // collect the edges from the participating nodes
    for (std::vector<NBNode*>::iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
        const EdgeVector& incoming = (*i)->getIncomingEdges();
        copy(incoming.begin(), incoming.end(), back_inserter(myIncomingEdges));
        const EdgeVector& outgoing = (*i)->getOutgoingEdges();
        copy(outgoing.begin(), outgoing.end(), back_inserter(myOutgoing));
    }
    EdgeVector outer;
    // check which of the edges are completely within the junction
    //  add them to the list of edges lying within the node
    for (NBEdge* edge : myIncomingEdges) {
        edge->setInsideTLS(false); // reset
        // an edge lies within the logic if it is outgoing as well as incoming
        EdgeVector::iterator k = std::find(myOutgoing.begin(), myOutgoing.end(), edge);
        if (k != myOutgoing.end()) {
            myEdgesWithin.push_back(edge);
        } else  {
            outer.push_back(edge);
        }
    }
    // collect edges that are reachable from the outside via controlled connections
    std::set<NBEdge*> reachable = collectReachable(outer, myEdgesWithin, true);
    // collect edges that are reachable from the outside regardless of controllability
    std::set<NBEdge*> reachable2 = collectReachable(outer, myEdgesWithin, false);

    const bool uncontrolledWithin = OptionsCont::getOptions().getBool("tls.uncontrolled-within");
    for (EdgeVector::iterator j = myEdgesWithin.begin(); j != myEdgesWithin.end(); ++j) {
        NBEdge* edge = *j;
        // edges that are marked as 'inner' will not get their own phase when
        // computing traffic light logics (unless they cannot be reached from the outside at all)
        if (reachable.count(edge) == 1) {
            edge->setInsideTLS(true);
            // legacy behavior
            if (uncontrolledWithin && myControlledInnerEdges.count(edge->getID()) == 0) {
                myIncomingEdges.erase(find(myIncomingEdges.begin(), myIncomingEdges.end(), edge));
            }
        }
        if (reachable2.count(edge) == 0 && edge->getFirstNonPedestrianLaneIndex(NBNode::FORWARD, true) >= 0
                && getID() != DummyID) {
            WRITE_WARNING("Unreachable edge '" + edge->getID() + "' within tlLogic '" + getID() + "'");
        }
    }
}


bool
NBTrafficLightDefinition::mustBrake(const NBEdge* const from, const NBEdge* const to) const {
    std::vector<NBNode*>::const_iterator i =
        find_if(myControlledNodes.begin(), myControlledNodes.end(),
                NBContHelper::node_with_incoming_finder(from));
    assert(i != myControlledNodes.end());
    NBNode* node = *i;
    if (!node->hasOutgoing(to)) {
        return true; // !!!
    }
    // @todo recheck relevance of lane indices
    return node->mustBrake(from, to, -1, -1, true);
}


bool
NBTrafficLightDefinition::mustBrake(const NBEdge* const possProhibitedFrom,
                                    const NBEdge* const possProhibitedTo,
                                    const NBEdge* const possProhibitorFrom,
                                    const NBEdge* const possProhibitorTo,
                                    bool regardNonSignalisedLowerPriority) const {
    return forbids(possProhibitorFrom, possProhibitorTo,
                   possProhibitedFrom, possProhibitedTo,
                   regardNonSignalisedLowerPriority);
}


bool
NBTrafficLightDefinition::mustBrake(const NBConnection& possProhibited,
                                    const NBConnection& possProhibitor,
                                    bool regardNonSignalisedLowerPriority) const {
    return forbids(possProhibitor.getFrom(), possProhibitor.getTo(),
                   possProhibited.getFrom(), possProhibited.getTo(),
                   regardNonSignalisedLowerPriority);
}


bool
NBTrafficLightDefinition::forbids(const NBEdge* const possProhibitorFrom,
                                  const NBEdge* const possProhibitorTo,
                                  const NBEdge* const possProhibitedFrom,
                                  const NBEdge* const possProhibitedTo,
                                  bool regardNonSignalisedLowerPriority,
                                  bool sameNodeOnly) const {
    if (possProhibitorFrom == nullptr || possProhibitorTo == nullptr || possProhibitedFrom == nullptr || possProhibitedTo == nullptr) {
        return false;
    }
    // retrieve both nodes
    std::vector<NBNode*>::const_iterator incoming =
        find_if(myControlledNodes.begin(), myControlledNodes.end(), NBContHelper::node_with_incoming_finder(possProhibitorFrom));
    std::vector<NBNode*>::const_iterator outgoing =
        find_if(myControlledNodes.begin(), myControlledNodes.end(), NBContHelper::node_with_outgoing_finder(possProhibitedTo));
    assert(incoming != myControlledNodes.end());
    NBNode* incnode = *incoming;
    NBNode* outnode = *outgoing;
    EdgeVector::const_iterator i;

#ifdef DEBUG_RIGHT_OF_WAY
    if (DEBUGCOND) {
        std::cout << "foribds tls=" << getID() << " from=" << possProhibitedFrom->getID() << " to=" << possProhibitedTo->getID() << " foeFrom=" << possProhibitorFrom->getID() << " foeTo=" << possProhibitorTo->getID() << " rnslp=" << regardNonSignalisedLowerPriority << " sameNodeOnly=" << sameNodeOnly;
    }
#endif
    if (incnode != outnode) {
        if (sameNodeOnly) {
#ifdef DEBUG_RIGHT_OF_WAY
            if (DEBUGCOND) {
                std::cout << "   differentNodes: allows (no check)\n";
            }
#endif
            return false;
        }
        // the links are located at different nodes
        const EdgeVector& ev1 = possProhibitedTo->getConnectedEdges();
        // go through the following edge,
        //  check whether one of these connections is prohibited
        for (i = ev1.begin(); i != ev1.end(); ++i) {
            std::vector<NBNode*>::const_iterator outgoing2 =
                find_if(myControlledNodes.begin(), myControlledNodes.end(), NBContHelper::node_with_outgoing_finder(*i));
            if (outgoing2 == myControlledNodes.end()) {
                continue;
            }
            NBNode* outnode2 = *outgoing2;
            if (incnode != outnode2) {
                continue;
            }
            if (incnode->getDirection(possProhibitedTo, *i) != LinkDirection::STRAIGHT) {
                continue;
            }
            bool ret1 = incnode->foes(possProhibitorFrom, possProhibitorTo,
                                      possProhibitedTo, *i);
            bool ret2 = incnode->forbids(possProhibitorFrom, possProhibitorTo,
                                         possProhibitedTo, *i,
                                         regardNonSignalisedLowerPriority);
            bool ret = ret1 || ret2;
            if (ret) {
#ifdef DEBUG_RIGHT_OF_WAY
                if (DEBUGCOND) {
                    std::cout << "   differentNodes: forbids\n";
                }
#endif
                return true;
            }
        }

        const EdgeVector& ev2 = possProhibitorTo->getConnectedEdges();
        // go through the following edge,
        //  check whether one of these connections is prohibited
        for (i = ev2.begin(); i != ev2.end(); ++i) {
            std::vector<NBNode*>::const_iterator incoming2 =
                find_if(myControlledNodes.begin(), myControlledNodes.end(), NBContHelper::node_with_incoming_finder(possProhibitorTo));
            if (incoming2 == myControlledNodes.end()) {
                continue;
            }
            NBNode* incnode2 = *incoming2;
            if (incnode2 != outnode) {
                continue;
            }
            if (incnode2->getDirection(possProhibitorTo, *i) != LinkDirection::STRAIGHT) {
                continue;
            }
            bool ret1 = incnode2->foes(possProhibitorTo, *i,
                                       possProhibitedFrom, possProhibitedTo);
            bool ret2 = incnode2->forbids(possProhibitorTo, *i,
                                          possProhibitedFrom, possProhibitedTo,
                                          regardNonSignalisedLowerPriority);
            bool ret = ret1 || ret2;
            if (ret) {
#ifdef DEBUG_RIGHT_OF_WAY
                if (DEBUGCOND) {
                    std::cout << "   differentNodes: forbids (2)\n";
                }
#endif
                return true;
            }
        }
#ifdef DEBUG_RIGHT_OF_WAY
        if (DEBUGCOND) {
            std::cout << "   differentNodes: allows\n";
        }
#endif
        return false;
    }
    // both links are located at the same node
    //  check using this node's information
    const bool result = incnode->forbids(possProhibitorFrom, possProhibitorTo,
                                         possProhibitedFrom, possProhibitedTo,
                                         regardNonSignalisedLowerPriority);
#ifdef DEBUG_RIGHT_OF_WAY
    if (DEBUGCOND) {
        std::cout << "   sameNodes: " << (result ? "forbids" : "allows") << "\n";
    }
#endif
    return result;
}


bool
NBTrafficLightDefinition::foes(const NBEdge* const from1, const NBEdge* const to1,
                               const NBEdge* const from2, const NBEdge* const to2) const {
    if (to1 == nullptr || to2 == nullptr) {
        return false;
    }
    // retrieve both nodes (it is possible that a connection
    std::vector<NBNode*>::const_iterator incoming =
        find_if(myControlledNodes.begin(), myControlledNodes.end(),
                NBContHelper::node_with_incoming_finder(from1));
    std::vector<NBNode*>::const_iterator outgoing =
        find_if(myControlledNodes.begin(), myControlledNodes.end(),
                NBContHelper::node_with_outgoing_finder(to1));
    assert(incoming != myControlledNodes.end());
    NBNode* incnode = *incoming;
    NBNode* outnode = *outgoing;
    if (incnode != outnode) {
        return false;
    }
    return incnode->foes(from1, to1, from2, to2);
}


void
NBTrafficLightDefinition::addNode(NBNode* node) {
    if (std::find(myControlledNodes.begin(), myControlledNodes.end(), node) == myControlledNodes.end()) {
        myControlledNodes.push_back(node);
        std::sort(myControlledNodes.begin(), myControlledNodes.end(), NBNode::nodes_by_id_sorter());
    }
    node->addTrafficLight(this);
}


void
NBTrafficLightDefinition::removeNode(NBNode* node) {
    std::vector<NBNode*>::iterator i = std::find(myControlledNodes.begin(), myControlledNodes.end(), node);
    if (i != myControlledNodes.end()) {
        myControlledNodes.erase(i);
    }
    // !!! remove in node?
}


void
NBTrafficLightDefinition::addControlledInnerEdges(const std::vector<std::string>& edges) {
    myControlledInnerEdges.insert(edges.begin(), edges.end());
}


std::vector<std::string>
NBTrafficLightDefinition::getControlledInnerEdges() const {
    return std::vector<std::string>(myControlledInnerEdges.begin(), myControlledInnerEdges.end());
}


const EdgeVector&
NBTrafficLightDefinition::getIncomingEdges() const {
    return myIncomingEdges;
}


void
NBTrafficLightDefinition::collectAllLinks(NBConnectionVector& into) {
    int tlIndex = 0;
    // build the list of links which are controled by the traffic light
    for (EdgeVector::iterator i = myIncomingEdges.begin(); i != myIncomingEdges.end(); i++) {
        NBEdge* incoming = *i;
        int noLanes = incoming->getNumLanes();
        for (int j = 0; j < noLanes; j++) {
            std::vector<NBEdge::Connection> connected = incoming->getConnectionsFromLane(j);
            for (std::vector<NBEdge::Connection>::iterator k = connected.begin(); k != connected.end(); k++) {
                const NBEdge::Connection& el = *k;
                if (incoming->mayBeTLSControlled(el.fromLane, el.toEdge, el.toLane)) {
                    if (el.toEdge != nullptr && el.toLane >= (int) el.toEdge->getNumLanes()) {
                        throw ProcessError("Connection '" + incoming->getID() + "_" + toString(j) + "->" + el.toEdge->getID() + "_" + toString(el.toLane) + "' yields in a not existing lane.");
                    }
                    if (incoming->getToNode()->getType() == SumoXMLNodeType::RAIL_CROSSING
                            && isRailway(incoming->getPermissions())) {
                        // railways stay uncontrolled at rail crossing but they
                        // must be registered in MSRailCrossing
                        into.push_back(NBConnection(incoming, el.fromLane, el.toEdge, el.toLane, -1));
                    } else if (incoming->getToNode()->getType() == SumoXMLNodeType::RAIL_SIGNAL
                               && incoming->getToNode()->getDirection(incoming, el.toEdge) == LinkDirection::TURN) {
                        // turnarounds stay uncontrolled at rail signal
                    } else {
                        into.push_back(NBConnection(incoming, el.fromLane, el.toEdge, el.toLane, tlIndex++));
                    }
                }
            }
        }
    }
    if (into.size() > 0 && tlIndex == 0) {
        WRITE_WARNINGF("The rail crossing '%' does not have any roads.", getID());
    }
}


bool
NBTrafficLightDefinition::needsCont(const NBEdge* fromE, const NBEdge* toE, const NBEdge* otherFromE, const NBEdge* otherToE) const {
    if (!myNeedsContRelationReady) {
        initNeedsContRelation();
        assert(myNeedsContRelationReady);
    }
    return std::find(myNeedsContRelation.begin(), myNeedsContRelation.end(),
                     StreamPair(fromE, toE, otherFromE, otherToE)) != myNeedsContRelation.end();
}


void
NBTrafficLightDefinition::initNeedsContRelation() const {
    if (!amInvalid()) {
        NBOwnTLDef dummy(DummyID, myControlledNodes, 0, TrafficLightType::STATIC);
        dummy.initNeedsContRelation();
        myNeedsContRelation = dummy.myNeedsContRelation;
        for (std::vector<NBNode*>::const_iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
            (*i)->removeTrafficLight(&dummy);
        }
    }
    myNeedsContRelationReady = true;
}


bool
NBTrafficLightDefinition::rightOnRedConflict(int index, int foeIndex) const {
    if (!myRightOnRedConflictsReady) {
        NBOwnTLDef dummy(DummyID, myControlledNodes, 0, TrafficLightType::STATIC);
        dummy.setParticipantsInformation();
        NBTrafficLightLogic* tllDummy = dummy.computeLogicAndConts(0, true);
        delete tllDummy;
        myRightOnRedConflicts = dummy.myRightOnRedConflicts;
        for (std::vector<NBNode*>::const_iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
            (*i)->removeTrafficLight(&dummy);
        }
        myRightOnRedConflictsReady = true;
        //std::cout << " rightOnRedConflicts tls=" << getID() << " pro=" << getProgramID() << "\n";
        //for (RightOnRedConflicts::const_iterator it = myRightOnRedConflicts.begin(); it != myRightOnRedConflicts.end(); ++it) {
        //    std::cout << "   " << it->first << ", " << it->second << "\n";
        //}
    }
    return std::find(myRightOnRedConflicts.begin(), myRightOnRedConflicts.end(), std::make_pair(index, foeIndex)) != myRightOnRedConflicts.end();
}

std::string
NBTrafficLightDefinition::getDescription() const {
    return getID() + ':' + getProgramID() + '@' + toString(this);
}


/****************************************************************************/
