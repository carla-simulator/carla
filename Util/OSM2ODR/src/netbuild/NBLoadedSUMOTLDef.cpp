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
/// @file    NBLoadedSUMOTLDef.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mar 2011
///
// A complete traffic light logic loaded from a sumo-net. (opted to reimplement
// since NBLoadedTLDef is quite vissim specific)
/****************************************************************************/
#include <config.h>

#include <vector>
#include <set>
#include <cassert>
#include <iterator>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include "NBTrafficLightLogic.h"
#include "NBOwnTLDef.h"
#include "NBTrafficLightDefinition.h"
#include "NBLoadedSUMOTLDef.h"
#include "NBNetBuilder.h"
#include "NBOwnTLDef.h"
#include "NBNode.h"

//#define DEBUG_RECONSTRUCTION

// ===========================================================================
// method definitions
// ===========================================================================

NBLoadedSUMOTLDef::NBLoadedSUMOTLDef(const std::string& id, const std::string& programID,
                                     SUMOTime offset, TrafficLightType type) :
    NBTrafficLightDefinition(id, programID, offset, type),
    myTLLogic(nullptr),
    myReconstructAddedConnections(false),
    myReconstructRemovedConnections(false),
    myPhasesLoaded(false) {
    myTLLogic = new NBTrafficLightLogic(id, programID, 0, offset, type);
}


NBLoadedSUMOTLDef::NBLoadedSUMOTLDef(const NBTrafficLightDefinition& def, const NBTrafficLightLogic& logic) :
    // allow for adding a new program for the same def: take the offset and programID from the new logic
    NBTrafficLightDefinition(def.getID(), logic.getProgramID(), logic.getOffset(), def.getType()),
    myTLLogic(new NBTrafficLightLogic(logic)),
    myReconstructAddedConnections(false),
    myReconstructRemovedConnections(false),
    myPhasesLoaded(false) {
    assert(def.getType() == logic.getType());
    myControlledLinks = def.getControlledLinks();
    myControlledNodes = def.getNodes();
    const NBLoadedSUMOTLDef* sumoDef = dynamic_cast<const NBLoadedSUMOTLDef*>(&def);
    updateParameters(def.getParametersMap());
    if (sumoDef != nullptr) {
        myReconstructAddedConnections = sumoDef->myReconstructAddedConnections;
        myReconstructRemovedConnections = sumoDef->myReconstructRemovedConnections;
    }
}


NBLoadedSUMOTLDef::~NBLoadedSUMOTLDef() {
    delete myTLLogic;
}


NBTrafficLightLogic*
NBLoadedSUMOTLDef::myCompute(int brakingTimeSeconds) {
    // @todo what to do with those parameters?
    UNUSED_PARAMETER(brakingTimeSeconds);
    reconstructLogic();
    myTLLogic->closeBuilding(false);
    patchIfCrossingsAdded();
    myTLLogic->closeBuilding();
    return new NBTrafficLightLogic(myTLLogic);
}


void
NBLoadedSUMOTLDef::addConnection(NBEdge* from, NBEdge* to, int fromLane, int toLane, int linkIndex, int linkIndex2, bool reconstruct) {
    assert(myTLLogic->getNumLinks() > 0); // logic should be loaded by now
    if (linkIndex >= (int)myTLLogic->getNumLinks()) {
        throw ProcessError("Invalid linkIndex " + toString(linkIndex) + " for traffic light '" + getID() +
                           "' with " + toString(myTLLogic->getNumLinks()) + " links.");
    }
    if (linkIndex2 >= (int)myTLLogic->getNumLinks()) {
        throw ProcessError("Invalid linkIndex2 " + toString(linkIndex2) + " for traffic light '" + getID() +
                           "' with " + toString(myTLLogic->getNumLinks()) + " links.");
    }
    NBConnection conn(from, fromLane, to, toLane, linkIndex, linkIndex2);
    // avoid duplicates
    auto newEnd = remove_if(myControlledLinks.begin(), myControlledLinks.end(), connection_equal(conn));
    // remove_if does not remove, only re-order
    myControlledLinks.erase(newEnd, myControlledLinks.end());
    myControlledLinks.push_back(conn);
    addNode(from->getToNode());
    addNode(to->getFromNode());
    // added connections are definitely controlled. make sure none are removed because they lie within the tl
    // myControlledInnerEdges.insert(from->getID()); // @todo recheck: this appears to be obsolete
    // set this information now so that it can be used while loading diffs
    from->setControllingTLInformation(conn, getID());
    myReconstructAddedConnections |= reconstruct;
}

void
NBLoadedSUMOTLDef::setProgramID(const std::string& programID) {
    NBTrafficLightDefinition::setProgramID(programID);
    myTLLogic->setProgramID(programID);
}

void
NBLoadedSUMOTLDef::setTLControllingInformation() const {
    if (myReconstructAddedConnections) {
        NBOwnTLDef dummy(DummyID, myControlledNodes, 0, getType());
        dummy.setParticipantsInformation();
        dummy.setTLControllingInformation();
        for (std::vector<NBNode*>::const_iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
            (*i)->removeTrafficLight(&dummy);
        }
    }
    if (myReconstructRemovedConnections) {
        return; // will be called again in reconstructLogic()
    }
    // if nodes have been removed our links may have been invalidated as well
    // since no logic will be built anyway there is no reason to inform any edges
    if (amInvalid()) {
        return;
    }
    // set the information about the link's positions within the tl into the
    //  edges the links are starting at, respectively
    for (NBConnectionVector::const_iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); it++) {
        const NBConnection& c = *it;
        if (c.getTLIndex() >= (int)myTLLogic->getNumLinks()) {
            throw ProcessError("Invalid linkIndex " + toString(c.getTLIndex()) + " for traffic light '" + getID() +
                               "' with " + toString(myTLLogic->getNumLinks()) + " links.");
        }
        NBEdge* edge = c.getFrom();
        if (edge != nullptr && edge->getNumLanes() > c.getFromLane()) {
            // logic may have yet to be reconstructed
            edge->setControllingTLInformation(c, getID());
        }
    }
}


void
NBLoadedSUMOTLDef::remapRemoved(NBEdge*, const EdgeVector&, const EdgeVector&) {}


void
NBLoadedSUMOTLDef::replaceRemoved(NBEdge* removed, int removedLane, NBEdge* by, int byLane, bool incoming) {
    for (NBConnectionVector::iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); ++it) {
        if (incoming) {
            (*it).replaceFrom(removed, removedLane, by, byLane);
        } else {
            (*it).replaceTo(removed, removedLane, by, byLane);
        }
    }
}


void
NBLoadedSUMOTLDef::addPhase(SUMOTime duration, const std::string& state, SUMOTime minDur, SUMOTime maxDur, const std::vector<int>& next, const std::string& name) {
    myTLLogic->addStep(duration, state, minDur, maxDur, next, name);
}


bool
NBLoadedSUMOTLDef::amInvalid() const {
    if (myControlledLinks.size() == 0) {
        return true;
    }
    if (myIncomingEdges.size() == 0) {
        return true;
    }
    return false;
}


void
NBLoadedSUMOTLDef::removeConnection(const NBConnection& conn, bool reconstruct) {
    for (auto it = myControlledLinks.begin(); it != myControlledLinks.end();) {
        if ((it->getFrom() == conn.getFrom() &&
                it->getTo() == conn.getTo() &&
                it->getFromLane() == conn.getFromLane() &&
                it->getToLane() == conn.getToLane())
                || (it->getTLIndex() == conn.getTLIndex() &&
                    conn.getTLIndex() != conn.InvalidTlIndex &&
                    (it->getFrom() == nullptr || it->getTo() == nullptr))) {
            if (reconstruct) {
                myReconstructRemovedConnections = true;
                it++;
            } else {
                it = myControlledLinks.erase(it);
            }
        } else {
            it++;
        }
    }
}


void
NBLoadedSUMOTLDef::setOffset(SUMOTime offset) {
    myOffset = offset;
    myTLLogic->setOffset(offset);
}


void
NBLoadedSUMOTLDef::setType(TrafficLightType type) {
    myType = type;
    myTLLogic->setType(type);
}


void
NBLoadedSUMOTLDef::collectEdges() {
    if (myControlledLinks.size() == 0) {
        NBTrafficLightDefinition::collectEdges();
    }
    myIncomingEdges.clear();
    EdgeVector myOutgoing;
    // collect the edges from the participating nodes
    for (std::vector<NBNode*>::iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
        const EdgeVector& incoming = (*i)->getIncomingEdges();
        copy(incoming.begin(), incoming.end(), back_inserter(myIncomingEdges));
        const EdgeVector& outgoing = (*i)->getOutgoingEdges();
        copy(outgoing.begin(), outgoing.end(), back_inserter(myOutgoing));
    }
    // check which of the edges are completely within the junction
    // and which are uncontrolled as well (we already know myControlledLinks)
    for (EdgeVector::iterator j = myIncomingEdges.begin(); j != myIncomingEdges.end();) {
        NBEdge* edge = *j;
        edge->setInsideTLS(false); // reset
        // an edge lies within the logic if it is outgoing as well as incoming
        EdgeVector::iterator k = std::find(myOutgoing.begin(), myOutgoing.end(), edge);
        if (k != myOutgoing.end()) {
            if (myControlledInnerEdges.count(edge->getID()) == 0) {
                bool controlled = false;
                for (NBConnectionVector::iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); it++) {
                    if ((*it).getFrom() == edge) {
                        controlled = true;
                        break;
                    }
                }
                if (controlled) {
                    myControlledInnerEdges.insert(edge->getID());
                } else {
                    myEdgesWithin.push_back(edge);
                    edge->setInsideTLS(true);
                    ++j; //j = myIncomingEdges.erase(j);
                    continue;
                }
            }
        }
        ++j;
    }
}


void
NBLoadedSUMOTLDef::collectLinks() {
    if (myControlledLinks.size() == 0) {
        // maybe we only loaded a different program for a default traffic light.
        // Try to build links now.
        collectAllLinks(myControlledLinks);
    }
}


/// @brief patches signal plans by modifying lane indices
void
NBLoadedSUMOTLDef::shiftTLConnectionLaneIndex(NBEdge* edge, int offset, int threshold) {
    // avoid shifting twice if the edge is incoming and outgoing to a joined TLS
    if (myShifted.count(edge) == 0) {
        /// XXX what if an edge should really be shifted twice?
        myShifted.insert(edge);
        for (NBConnectionVector::iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); it++) {
            (*it).shiftLaneIndex(edge, offset, threshold);
        }
    }
}

void
NBLoadedSUMOTLDef::patchIfCrossingsAdded() {
    const int size = myTLLogic->getNumLinks();
    int noLinksAll = 0;
    for (NBConnectionVector::const_iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); it++) {
        const NBConnection& c = *it;
        if (c.getTLIndex() != NBConnection::InvalidTlIndex) {
            noLinksAll = MAX2(noLinksAll, (int)c.getTLIndex() + 1);
        }
    }
    const int numNormalLinks = noLinksAll;
    int oldCrossings = 0;
    // collect crossings
    bool customIndex = false;
    std::vector<NBNode::Crossing*> crossings;
    for (std::vector<NBNode*>::iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
        const std::vector<NBNode::Crossing*>& c = (*i)->getCrossings();
        // set tl indices for crossings
        customIndex |= (*i)->setCrossingTLIndices(getID(), noLinksAll);
        copy(c.begin(), c.end(), std::back_inserter(crossings));
        noLinksAll += (int)c.size();
        oldCrossings += (*i)->numCrossingsFromSumoNet();
    }
    if ((int)crossings.size() != oldCrossings) {
        std::vector<NBTrafficLightLogic::PhaseDefinition> phases = myTLLogic->getPhases();
        // do not rebuilt crossing states there are custom indices and the state string is long enough
        if (phases.size() > 0 && (
                    (int)(phases.front().state.size()) < noLinksAll ||
                    ((int)(phases.front().state.size()) > noLinksAll && !customIndex))) {
            // collect edges
            EdgeVector fromEdges(size, (NBEdge*)nullptr);
            EdgeVector toEdges(size, (NBEdge*)nullptr);
            std::vector<int> fromLanes(size, 0);
            collectEdgeVectors(fromEdges, toEdges, fromLanes);
            const std::string crossingDefaultState(crossings.size(), 'r');

            // rebuild the logic (see NBOwnTLDef.cpp::myCompute)
            NBTrafficLightLogic* newLogic = new NBTrafficLightLogic(getID(), getProgramID(), 0, myOffset, myType);
            SUMOTime brakingTime = TIME2STEPS(computeBrakingTime(OptionsCont::getOptions().getFloat("tls.yellow.min-decel")));
            //std::cout << "patchIfCrossingsAdded for " << getID() << " numPhases=" << phases.size() << "\n";
            for (std::vector<NBTrafficLightLogic::PhaseDefinition>::const_iterator it = phases.begin(); it != phases.end(); it++) {
                const std::string state = it->state.substr(0, numNormalLinks) + crossingDefaultState;
                NBOwnTLDef::addPedestrianPhases(newLogic, it->duration, it->minDur, it->maxDur, state, crossings, fromEdges, toEdges);
            }
            NBOwnTLDef::addPedestrianScramble(newLogic, noLinksAll, TIME2STEPS(10), brakingTime, crossings, fromEdges, toEdges);

            delete myTLLogic;
            myTLLogic = newLogic;
        } else if (phases.size() == 0) {
            WRITE_WARNING("Could not patch tlLogic '" + getID() + "' for changed crossings");
        }
    }
}


void
NBLoadedSUMOTLDef::collectEdgeVectors(EdgeVector& fromEdges, EdgeVector& toEdges, std::vector<int>& fromLanes) const {
    assert(fromEdges.size() > 0);
    assert(fromEdges.size() == toEdges.size());
    const int size = (int)fromEdges.size();

    for (NBConnectionVector::const_iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); it++) {
        const NBConnection& c = *it;
        if (c.getTLIndex() != NBConnection::InvalidTlIndex) {
            if (c.getTLIndex() >= size) {
                throw ProcessError("Invalid linkIndex " + toString(c.getTLIndex()) + " for traffic light '" + getID() +
                                   "' with " + toString(size) + " links.");
            }
            fromEdges[c.getTLIndex()] = c.getFrom();
            toEdges[c.getTLIndex()] = c.getTo();
            fromLanes[c.getTLIndex()] = c.getFromLane();
        }
    }
}


void
NBLoadedSUMOTLDef::initNeedsContRelation() const {
    if (!amInvalid() && !myNeedsContRelationReady) {
        myNeedsContRelation.clear();
        myRightOnRedConflicts.clear();
        const bool controlledWithin = !OptionsCont::getOptions().getBool("tls.uncontrolled-within");
        const std::vector<NBTrafficLightLogic::PhaseDefinition> phases = myTLLogic->getPhases();
        for (std::vector<NBTrafficLightLogic::PhaseDefinition>::const_iterator it = phases.begin(); it != phases.end(); it++) {
            const std::string state = (*it).state;
            for (NBConnectionVector::const_iterator it1 = myControlledLinks.begin(); it1 != myControlledLinks.end(); it1++) {
                const NBConnection& c1 = *it1;
                const int i1 = c1.getTLIndex();
                if (i1 == NBConnection::InvalidTlIndex || (state[i1] != 'g' && state[i1] != 's') || c1.getFrom() == nullptr || c1.getTo() == nullptr) {
                    continue;
                }
                for (NBConnectionVector::const_iterator it2 = myControlledLinks.begin(); it2 != myControlledLinks.end(); it2++) {
                    const NBConnection& c2 = *it2;
                    const int i2 = c2.getTLIndex();
                    if (i2 != NBConnection::InvalidTlIndex
                            && i2 != i1
                            && (state[i2] == 'G' || state[i2] == 'g')
                            && c2.getFrom() != nullptr && c2.getTo() != nullptr) {
                        const bool rightTurnConflict = NBNode::rightTurnConflict(
                                                           c1.getFrom(), c1.getTo(), c1.getFromLane(), c2.getFrom(), c2.getTo(), c2.getFromLane());
                        const bool forbidden = forbids(c2.getFrom(), c2.getTo(), c1.getFrom(), c1.getTo(), true, controlledWithin);
                        const bool isFoes = foes(c2.getFrom(), c2.getTo(), c1.getFrom(), c1.getTo()) && !c2.getFrom()->isTurningDirectionAt(c2.getTo());
                        if (forbidden || rightTurnConflict) {
                            myNeedsContRelation.insert(StreamPair(c1.getFrom(), c1.getTo(), c2.getFrom(), c2.getTo()));
                        }
                        if (isFoes && state[i1] == 's') {
                            myRightOnRedConflicts.insert(std::make_pair(i1, i2));
                            //std::cout << getID() << " prog=" << getProgramID() << " phase=" << (it - phases.begin()) << " rightOnRedConflict i1=" << i1 << " i2=" << i2 << "\n";
                        }
                        //std::cout << getID() << " i1=" << i1 << " i2=" << i2 << " rightTurnConflict=" << rightTurnConflict << " forbidden=" << forbidden << " isFoes=" << isFoes << "\n";
                    }
                }
            }
        }
    }
    myNeedsContRelationReady = true;
    myRightOnRedConflictsReady = true;
}


bool
NBLoadedSUMOTLDef::rightOnRedConflict(int index, int foeIndex) const {
    if (amInvalid()) {
        return false;
    }
    if (!myRightOnRedConflictsReady) {
        initNeedsContRelation();
        assert(myRightOnRedConflictsReady);
    }
    return std::find(myRightOnRedConflicts.begin(), myRightOnRedConflicts.end(), std::make_pair(index, foeIndex)) != myRightOnRedConflicts.end();
}


void
NBLoadedSUMOTLDef::registerModifications(bool addedConnections, bool removedConnections) {
    myReconstructAddedConnections |= addedConnections;
    myReconstructRemovedConnections |= removedConnections;
}

void
NBLoadedSUMOTLDef::reconstructLogic() {
    const bool netedit = NBNetBuilder::runningNetedit();
#ifdef DEBUG_RECONSTRUCTION
    bool debugPrintModified = myReconstructAddedConnections || myReconstructRemovedConnections;
    std::cout << getID() << " reconstructLogic added=" << myReconstructAddedConnections
        << " removed=" << myReconstructRemovedConnections
        << " valid=" << hasValidIndices()
        << " phasesLoaded=" << myPhasesLoaded
        << " oldLinks:\n";
    for (NBConnectionVector::iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); ++it) {
        std::cout << "    " << *it << "\n";
    }
#endif
    if (myReconstructAddedConnections) {
        myReconstructAddedConnections = false;
        // do not rebuild the logic when running netedit and all links are already covered by the program
        if (!myPhasesLoaded && !(netedit && hasValidIndices())) {
            // rebuild the logic from scratch
            // XXX if a connection with the same from- and to-edge already exisits, its states could be copied instead
            NBOwnTLDef dummy(DummyID, myControlledNodes, 0, getType());
            dummy.setParticipantsInformation();
            dummy.setProgramID(getProgramID());
            dummy.setTLControllingInformation();
            NBTrafficLightLogic* newLogic = dummy.compute(OptionsCont::getOptions());
            myIncomingEdges = dummy.getIncomingEdges();
            myControlledLinks = dummy.getControlledLinks();
            for (std::vector<NBNode*>::const_iterator i = myControlledNodes.begin(); i != myControlledNodes.end(); i++) {
                (*i)->removeTrafficLight(&dummy);
            }
            delete myTLLogic;
            myTLLogic = newLogic;
            if (newLogic != nullptr) {
                newLogic->setID(getID());
                newLogic->setType(getType());
                newLogic->setOffset(getOffset());
                setTLControllingInformation();
                // reset crossing custom indices
                for (NBNode* n : myControlledNodes) {
                    for (NBNode::Crossing* c : n->getCrossings()) {
                        c->customTLIndex = NBConnection::InvalidTlIndex;
                    }
                }

            }
        } else {
            setTLControllingInformation();
        }
    }
    if (myReconstructRemovedConnections) {
        myReconstructRemovedConnections = false;
        // for each connection, check whether it is still valid
        for (NBConnectionVector::iterator it = myControlledLinks.begin(); it != myControlledLinks.end();) {
            const NBConnection con = (*it);
            if (// edge still exists
                std::find(myIncomingEdges.begin(), myIncomingEdges.end(), con.getFrom()) != myIncomingEdges.end()
                // connection still exists
                && con.getFrom()->hasConnectionTo(con.getTo(), con.getToLane(), con.getFromLane())
                // connection is still set to be controlled
                && con.getFrom()->mayBeTLSControlled(con.getFromLane(), con.getTo(), con.getToLane())) {
                it++;
            } else {
                // remove connection
                const int removed = con.getTLIndex();
                it = myControlledLinks.erase(it);
                // no automatic modificaions when running netedit
                if (!myPhasesLoaded && !(netedit && hasValidIndices())) {
                    // shift index off successive connections and remove entry from all phases if the tlIndex was only used by this connection
                    bool exclusive = true;
                    for (NBConnection& other : myControlledLinks) {
                        if (other != con && other.getTLIndex() == removed) {
                            exclusive = false;
                            break;
                        }
                    }
                    if (exclusive) {
                        // shift indices above the removed index downward
                        for (NBConnection& other : myControlledLinks) {
                            if (other.getTLIndex() > removed) {
                                other.setTLIndex(other.getTLIndex() - 1);
                            }
                        }
                        // shift crossing custom indices above the removed index downward
                        for (NBNode* n : myControlledNodes) {
                            for (NBNode::Crossing* c : n->getCrossings()) {
                                if (c->customTLIndex > removed) {
                                    c->customTLIndex--;
                                }
                            }
                        }
                        // rebuild the logic
                        NBTrafficLightLogic* newLogic = new NBTrafficLightLogic(getID(), getProgramID(), 0, myOffset, myType);
                        for (const NBTrafficLightLogic::PhaseDefinition& phase : myTLLogic->getPhases()) {
                            std::string newState = phase.state;
                            newState.erase(newState.begin() + removed);
                            newLogic->addStep(phase.duration, newState);
                        }
                        delete myTLLogic;
                        myTLLogic = newLogic;
                    }
                }
            }
        }
        setTLControllingInformation();
    }
#ifdef DEBUG_RECONSTRUCTION
    if (debugPrintModified) {
        std::cout << " newLinks:\n";
        for (NBConnectionVector::iterator it = myControlledLinks.begin(); it != myControlledLinks.end(); ++it) {
            std::cout << "    " << *it << "\n";
        }
    }
#endif
}


int
NBLoadedSUMOTLDef::getMaxIndex() {
    int maxIndex = -1;
    for (const NBConnection& c : myControlledLinks) {
        maxIndex = MAX2(maxIndex, c.getTLIndex());
        maxIndex = MAX2(maxIndex, c.getTLIndex2());
    }
    for (NBNode* n : myControlledNodes) {
        for (NBNode::Crossing* c : n->getCrossings()) {
            maxIndex = MAX2(maxIndex, c->tlLinkIndex);
            maxIndex = MAX2(maxIndex, c->tlLinkIndex2);
        }
    }
    return maxIndex;
}


int
NBLoadedSUMOTLDef::getMaxValidIndex() {
    return myTLLogic->getNumLinks() - 1;
}


bool
NBLoadedSUMOTLDef::hasValidIndices() const {
    for (const NBConnection& c : myControlledLinks) {
        if (c.getTLIndex() == NBConnection::InvalidTlIndex) {
            return false;
        }
    }
    for (NBNode* n : myControlledNodes) {
        for (NBNode::Crossing* c : n->getCrossings()) {
            if (c->tlLinkIndex == NBConnection::InvalidTlIndex) {
                return false;
            }
        }
    }
    // method getMaxIndex() is const but cannot be declare as such due to inheritance
    return const_cast<NBLoadedSUMOTLDef*>(this)->getMaxIndex() < myTLLogic->getNumLinks();
}


std::string
NBLoadedSUMOTLDef::getStates(int index) {
    assert(index >= 0);
    assert(index <= getMaxIndex());
    std::string result;
    for (auto& pd : myTLLogic->getPhases()) {
        result += pd.state[index];
    }
    return result;
}

bool
NBLoadedSUMOTLDef::isUsed(int index) {
    for (const NBConnection& c : myControlledLinks) {
        if (c.getTLIndex() == index || c.getTLIndex2() == index) {
            return true;
        }
    }
    for (NBNode* n : myControlledNodes) {
        for (NBNode::Crossing* c : n->getCrossings()) {
            if (c->tlLinkIndex == index || c->tlLinkIndex2 == index) {
                return true;
            }
        }
    }
    return false;
}

std::set<const NBEdge*>
NBLoadedSUMOTLDef::getEdgesUsingIndex(int index) const {
    std::set<const NBEdge*> result;
    for (const NBConnection& c : myControlledLinks) {
        if (c.getTLIndex() == index || c.getTLIndex2() == index) {
            result.insert(c.getFrom());
        }
    }
    return result;
}


void
NBLoadedSUMOTLDef::replaceIndex(int oldIndex, int newIndex) {
    if (oldIndex == newIndex) {
        return;
    }
    for (NBConnection& c : myControlledLinks) {
        if (c.getTLIndex() == oldIndex) {
            c.setTLIndex(newIndex);
        }
        if (c.getTLIndex2() == oldIndex) {
            c.setTLIndex2(newIndex);
        }
    }
    for (NBNode* n : myControlledNodes) {
        for (NBNode::Crossing* c : n->getCrossings()) {
            if (c->tlLinkIndex == oldIndex) {
                c->tlLinkIndex = newIndex;
            }
            if (c->tlLinkIndex2 == oldIndex) {
                c->tlLinkIndex2 = newIndex;
            }
        }
    }
}

void
NBLoadedSUMOTLDef::groupSignals() {
    const int maxIndex = getMaxIndex();
    std::vector<int> unusedIndices;
    for (int i = 0; i <= maxIndex; i++) {
        if (isUsed(i)) {
            std::set<const NBEdge*> edges = getEdgesUsingIndex(i);
            // compactify
            replaceIndex(i, i - (int)unusedIndices.size());
            if (edges.size() == 0) {
                // do not group pedestrian crossing signals
                continue;
            }
            std::string states = getStates(i);
            for (int j = i + 1; j <= maxIndex; j++) {
                // only group signals from the same edges as is commonly done by
                // traffic engineers
                if (states == getStates(j) && edges == getEdgesUsingIndex(j)) {
                    replaceIndex(j, i - (int)unusedIndices.size());
                }
            }
        } else {
            unusedIndices.push_back(i);
        }
    }
    for (int i = (int)unusedIndices.size() - 1; i >= 0; i--) {
        myTLLogic->deleteStateIndex(unusedIndices[i]);
    }
    cleanupStates();
    //std::cout << "oldMaxIndex=" << maxIndex << " newMaxIndex=" << getMaxIndex() << " unused=" << toString(unusedIndices) << "\n";
    setTLControllingInformation();
}

void
NBLoadedSUMOTLDef::ungroupSignals() {
    NBConnectionVector defaultOrdering;
    collectAllLinks(defaultOrdering);
    myTLLogic->setStateLength((int)myControlledLinks.size());
    std::vector<std::string> states; // organized per link rather than phase
    int index = 0;
    for (NBConnection& c : defaultOrdering) {
        NBConnection& c2 = *find_if(myControlledLinks.begin(), myControlledLinks.end(), connection_equal(c));
        states.push_back(getStates(c2.getTLIndex()));
        c2.setTLIndex(index++);
    }
    for (NBNode* n : myControlledNodes) {
        for (NBNode::Crossing* c : n->getCrossings()) {
            states.push_back(getStates(c->tlLinkIndex));
            c->tlLinkIndex = index++;
            if (c->tlLinkIndex2 != NBConnection::InvalidTlIndex) {
                states.push_back(getStates(c->tlLinkIndex2));
                c->tlLinkIndex2 = index++;
            }
        }
    }
    for (int i = 0; i < (int)states.size(); i++) {
        for (int p = 0; p < (int)states[i].size(); p++) {
            myTLLogic->setPhaseState(p, i, (LinkState)states[i][p]);
        }
    }
    setTLControllingInformation();
}


void
NBLoadedSUMOTLDef::copyIndices(NBTrafficLightDefinition* def) {
    std::map<int, std::string> oldStates; // organized per link index rather than phase
    std::map<int, std::string> newStates; // organized per link index rather than phase
    for (NBConnection& c : def->getControlledLinks()) {
        NBConnection& c2 = *find_if(myControlledLinks.begin(), myControlledLinks.end(), connection_equal(c));
        const int oldIndex = c2.getTLIndex();
        const int newIndex = c.getTLIndex();
        std::string states = getStates(oldIndex);
        oldStates[oldIndex] = states;
        if (newStates.count(newIndex) != 0 && newStates[newIndex] != states) {
            WRITE_WARNING("Signal groups from program '" + def->getProgramID() + "' are incompatible with the states of program '" + getProgramID() + "' at tlLogic '" + getID()
                          + "'. Possibly unsafe program.");
        } else {
            newStates[newIndex] = states;
        }
        c2.setTLIndex(newIndex);
    }
    const int maxIndex = getMaxIndex();
    myTLLogic->setStateLength(maxIndex + 1);
    for (int i = 0; i < (int)newStates.size(); i++) {
        for (int p = 0; p < (int)newStates[i].size(); p++) {
            myTLLogic->setPhaseState(p, i, (LinkState)newStates[i][p]);
        }
    }
    setTLControllingInformation();
}


bool
NBLoadedSUMOTLDef::cleanupStates() {
    const int maxIndex = getMaxIndex();
    std::vector<int> unusedIndices;
    for (int i = 0; i <= maxIndex; i++) {
        if (isUsed(i)) {
            if (unusedIndices.size() > 0) {
                replaceIndex(i, i - (int)unusedIndices.size());
            }
        } else {
            unusedIndices.push_back(i);
        }
    }
    for (int i = (int)unusedIndices.size() - 1; i >= 0; i--) {
        myTLLogic->deleteStateIndex(unusedIndices[i]);
    }
    if (unusedIndices.size() > 0) {
        myTLLogic->setStateLength(maxIndex + 1 - (int)unusedIndices.size());
        setTLControllingInformation();
        return true;
    } else {
        return false;
    }
}


void
NBLoadedSUMOTLDef::joinLogic(NBTrafficLightDefinition* def) {
    def->setParticipantsInformation();
    def->compute(OptionsCont::getOptions());
    const int maxIndex = MAX2(getMaxIndex(), def->getMaxIndex());
    myTLLogic->setStateLength(maxIndex + 1);
    myControlledLinks.insert(myControlledLinks.end(), def->getControlledLinks().begin(), def->getControlledLinks().end());
}

bool
NBLoadedSUMOTLDef::usingSignalGroups() const {
    // count how often each index is used
    std::map<int, int> indexUsage;
    for (const NBConnection& c : myControlledLinks) {
        indexUsage[c.getTLIndex()]++;
    }
    for (NBNode* n : myControlledNodes) {
        for (NBNode::Crossing* c : n->getCrossings()) {
            indexUsage[c->tlLinkIndex]++;
            indexUsage[c->tlLinkIndex2]++;
        }
    }
    for (auto it : indexUsage) {
        if (it.first >= 0 && it.second > 1) {
            return true;
        }
    }
    return false;
}

void
NBLoadedSUMOTLDef::guessMinMaxDuration() {
    bool hasMinMaxDur = false;
    for (auto phase : myTLLogic->getPhases()) {
        if (phase.maxDur != UNSPECIFIED_DURATION) {
            //std::cout << " phase=" << phase.state << " maxDur=" << phase.maxDur << "\n";
            hasMinMaxDur = true;
        }
    }
    if (!hasMinMaxDur) {
        const SUMOTime minMinDur = TIME2STEPS(OptionsCont::getOptions().getInt("tls.min-dur"));
        const SUMOTime maxDur = TIME2STEPS(OptionsCont::getOptions().getInt("tls.max-dur"));
        std::set<int> yellowIndices;
        for (auto phase : myTLLogic->getPhases()) {
            for (int i = 0; i < (int)phase.state.size(); i++) {
                if (phase.state[i] == 'y' || phase.state[i] == 'Y') {
                    yellowIndices.insert(i);
                }
            }
        }
        for (int ip = 0; ip < (int)myTLLogic->getPhases().size(); ip++) {
            bool needMinMaxDur = false;
            auto phase = myTLLogic->getPhases()[ip];
            std::set<int> greenIndices;
            if (phase.state.find_first_of("yY") != std::string::npos) {
                continue;
            }
            for (int i = 0; i < (int)phase.state.size(); i++) {
                if (yellowIndices.count(i) != 0 && phase.state[i] == 'G') {
                    needMinMaxDur = true;
                    greenIndices.insert(i);
                }
            }
            if (needMinMaxDur) {
                double maxSpeed = 0;
                for (NBConnection& c : myControlledLinks) {
                    if (greenIndices.count(c.getTLIndex()) != 0) {
                        maxSpeed = MAX2(maxSpeed, c.getFrom()->getLaneSpeed(c.getFromLane()));
                    }
                }
                // 5s at 50km/h, 10s at 80km/h, rounded to full seconds
                const double minDurBySpeed = maxSpeed * 3.6 / 6 - 3.3;
                SUMOTime minDur = MAX2(minMinDur, TIME2STEPS(floor(minDurBySpeed + 0.5)));
                myTLLogic->setPhaseMinDuration(ip, minDur);
                myTLLogic->setPhaseMaxDuration(ip, maxDur);
            }
        }
    }
}


/****************************************************************************/
