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
/// @file    NBRequest.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Tue, 20 Nov 2001
///
// This class computes the logic of a junction
/****************************************************************************/
#include <config.h>

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <bitset>
#include <sstream>
#include <map>
#include <cassert>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include "NBEdge.h"
#include "NBContHelper.h"
#include "NBNode.h"
#include "NBRequest.h"

//#define DEBUG_RESPONSE
//#define DEBUG_SETBLOCKING
#define DEBUGCOND (myJunction->getID() == "F")

// ===========================================================================
// static member variables
// ===========================================================================
int NBRequest::myGoodBuilds = 0;
int NBRequest::myNotBuild = 0;


// ===========================================================================
// method definitions
// ===========================================================================
NBRequest::NBRequest(const NBEdgeCont& ec,
                     NBNode* junction,
                     const EdgeVector& all,
                     const EdgeVector& incoming,
                     const EdgeVector& outgoing,
                     const NBConnectionProhibits& loadedProhibits) :
    myJunction(junction),
    myAll(all),
    myIncoming(incoming),
    myOutgoing(outgoing) {
    const int variations = numLinks();
    // build maps with information which forbidding connection were
    //  computed and what's in there
    myForbids.reserve(variations);
    myDone.reserve(variations);
    for (int i = 0; i < variations; i++) {
        myForbids.push_back(LinkInfoCont(variations, false));
        myDone.push_back(LinkInfoCont(variations, false));
    }
    // insert loaded prohibits
    for (NBConnectionProhibits::const_iterator j = loadedProhibits.begin(); j != loadedProhibits.end(); j++) {
        NBConnection prohibited = (*j).first;
        bool ok1 = prohibited.check(ec);
        if (find(myIncoming.begin(), myIncoming.end(), prohibited.getFrom()) == myIncoming.end()) {
            ok1 = false;
        }
        if (find(myOutgoing.begin(), myOutgoing.end(), prohibited.getTo()) == myOutgoing.end()) {
            ok1 = false;
        }
        int idx1 = 0;
        if (ok1) {
            idx1 = getIndex(prohibited.getFrom(), prohibited.getTo());
            if (idx1 < 0) {
                ok1 = false;
            }
        }
        const NBConnectionVector& prohibiting = (*j).second;
        for (NBConnectionVector::const_iterator k = prohibiting.begin(); k != prohibiting.end(); k++) {
            NBConnection sprohibiting = *k;
            bool ok2 = sprohibiting.check(ec);
            if (find(myIncoming.begin(), myIncoming.end(), sprohibiting.getFrom()) == myIncoming.end()) {
                ok2 = false;
            }
            if (find(myOutgoing.begin(), myOutgoing.end(), sprohibiting.getTo()) == myOutgoing.end()) {
                ok2 = false;
            }
            if (ok1 && ok2) {
                int idx2 = getIndex(sprohibiting.getFrom(), sprohibiting.getTo());
                if (idx2 < 0) {
                    ok2 = false;
                } else {
                    myForbids[idx2][idx1] = true;
                    myDone[idx2][idx1] = true;
                    myDone[idx1][idx2] = true;
                    myGoodBuilds++;
                }
            } else {
                std::string pfID = prohibited.getFrom() != nullptr ? prohibited.getFrom()->getID() : "UNKNOWN";
                std::string ptID = prohibited.getTo() != nullptr ? prohibited.getTo()->getID() : "UNKNOWN";
                std::string bfID = sprohibiting.getFrom() != nullptr ? sprohibiting.getFrom()->getID() : "UNKNOWN";
                std::string btID = sprohibiting.getTo() != nullptr ? sprohibiting.getTo()->getID() : "UNKNOWN";
                WRITE_WARNING("could not prohibit " + pfID + "->" + ptID + " by " + bfID + "->" + btID);
                myNotBuild++;
            }
        }
    }
    // ok, check whether someone has prohibited two links vice versa
    //  (this happens also in some Vissim-networks, when edges are joined)
    for (int s1 = 0; s1 < variations; s1++) {
        for (int s2 = s1 + 1; s2 < variations; s2++) {
            // not set, yet
            if (!myDone[s1][s2]) {
                continue;
            }
            // check whether both prohibit vice versa
            if (myForbids[s1][s2] && myForbids[s2][s1]) {
                // mark unset - let our algorithm fix it later
                myDone[s1][s2] = false;
                myDone[s2][s1] = false;
            }
        }
    }
}


NBRequest::~NBRequest() {}


void
NBRequest::buildBitfieldLogic() {
    EdgeVector::const_iterator i, j;
    for (i = myIncoming.begin(); i != myIncoming.end(); i++) {
        for (j = myOutgoing.begin(); j != myOutgoing.end(); j++) {
            computeRightOutgoingLinkCrossings(*i, *j);
            computeLeftOutgoingLinkCrossings(*i, *j);
        }
    }
    // reset signalised/non-signalised dependencies
    resetSignalised();
    // reset foes it the number of lanes matches (or exceeds) the number of incoming connections
    resetCooperating();
}


void
NBRequest::computeRightOutgoingLinkCrossings(NBEdge* from, NBEdge* to) {
    EdgeVector::const_iterator pfrom = std::find(myAll.begin(), myAll.end(), from);
    while (*pfrom != to) {
        NBContHelper::nextCCW(myAll, pfrom);
        if ((*pfrom)->getToNode() == myJunction) {
            EdgeVector::const_iterator pto = std::find(myAll.begin(), myAll.end(), to);
            while (*pto != from) {
                if (!((*pto)->getToNode() == myJunction)) {
                    setBlocking(from, to, *pfrom, *pto);
                }
                NBContHelper::nextCCW(myAll, pto);
            }
        }
    }
}


void
NBRequest::computeLeftOutgoingLinkCrossings(NBEdge* from, NBEdge* to) {
    EdgeVector::const_iterator pfrom = std::find(myAll.begin(), myAll.end(), from);
    while (*pfrom != to) {
        NBContHelper::nextCW(myAll, pfrom);
        if ((*pfrom)->getToNode() == myJunction) {
            EdgeVector::const_iterator pto = std::find(myAll.begin(), myAll.end(), to);
            while (*pto != from) {
                if (!((*pto)->getToNode() == myJunction)) {
                    setBlocking(from, to, *pfrom, *pto);
                }
                NBContHelper::nextCW(myAll, pto);
            }
        }
    }
}


void
NBRequest::setBlocking(NBEdge* from1, NBEdge* to1,
                       NBEdge* from2, NBEdge* to2) {
    // check whether one of the links has a dead end
    if (to1 == nullptr || to2 == nullptr) {
        return;
    }
    // get the indices of both links
    int idx1 = getIndex(from1, to1);
    int idx2 = getIndex(from2, to2);
    if (idx1 < 0 || idx2 < 0) {
        return; // !!! error output? did not happend, yet
    }
    // check whether the link crossing has already been checked
    assert(idx1 < (int)(myIncoming.size() * myOutgoing.size()));
    if (myDone[idx1][idx2]) {
        return;
    }
    // mark the crossings as done
    myDone[idx1][idx2] = true;
    myDone[idx2][idx1] = true;
    // special case all-way stop
    if (myJunction->getType() == SumoXMLNodeType::ALLWAY_STOP) {
        // all ways forbid each other. Conflict resolution happens via arrival time
        myForbids[idx1][idx2] = true;
        myForbids[idx2][idx1] = true;
        return;
    }
    // check if one of the links is a turn; this link is always not priorised
    //  true for right-before-left and priority
    if (from1->isTurningDirectionAt(to1)) {
        myForbids[idx2][idx1] = true;
        return;
    }
    if (from2->isTurningDirectionAt(to2)) {
        myForbids[idx1][idx2] = true;
        return;
    }
    // if  there are no connections, there are no prohibitions
    if (from1->isConnectedTo(to1)) {
        if (!from2->isConnectedTo(to2)) {
            myForbids[idx1][idx2] = true;
            myForbids[idx2][idx1] = false;
            return;
        }
    } else {
        if (!from2->isConnectedTo(to2)) {
            myForbids[idx1][idx2] = false;
            myForbids[idx2][idx1] = false;
            return;
        } else {
            myForbids[idx1][idx2] = false;
            myForbids[idx2][idx1] = true;
            return;
        }
    }
#ifdef DEBUG_SETBLOCKING
    if (DEBUGCOND) std::cout << "setBlocking"
                                 << " 1:" << from1->getID() << "->" << to1->getID()
                                 << " 2:" << from2->getID() << "->" << to2->getID() << "\n";
#endif
    // check the priorities if required by node type
    if (myJunction->getType() != SumoXMLNodeType::RIGHT_BEFORE_LEFT) {
        int from1p = from1->getJunctionPriority(myJunction);
        int from2p = from2->getJunctionPriority(myJunction);
#ifdef DEBUG_SETBLOCKING
        if (DEBUGCOND) std::cout << "setBlocking"
                                     << " 1:" << from1->getID() << "->" << to1->getID()
                                     << " 2:" << from2->getID() << "->" << to2->getID()
                                     << " p1=" << from1p << " p2=" << from2p << "\n";
#endif
        // check if one of the connections is higher priorised when incoming into
        //  the junction, the connection road will yield
        if (from1p > from2p) {
            myForbids[idx1][idx2] = true;
            return;
        }
        if (from2p > from1p) {
            myForbids[idx2][idx1] = true;
            return;
        }
    }
    // straight connections prohibit turning connections if the priorities are equal
    // (unless the junction is a bent priority junction)
    if (myJunction->getType() != SumoXMLNodeType::RIGHT_BEFORE_LEFT && !myJunction->isBentPriority()) {
        LinkDirection ld1 = myJunction->getDirection(from1, to1);
        LinkDirection ld2 = myJunction->getDirection(from2, to2);
#ifdef DEBUG_SETBLOCKING
        if (DEBUGCOND) std::cout << "setBlocking"
                                     << " 1:" << from1->getID() << "->" << to1->getID()
                                     << " 2:" << from2->getID() << "->" << to2->getID()
                                     << " dir1=" << toString(ld1) << " dir2=" << toString(ld2) << "\n";
#endif
        if (ld1 == LinkDirection::STRAIGHT) {
            if (ld2 != LinkDirection::STRAIGHT) {
                myForbids[idx1][idx2] = true;
                myForbids[idx2][idx1] = false;
                return;
            }
        } else {
            if (ld2 == LinkDirection::STRAIGHT) {
                myForbids[idx1][idx2] = false;
                myForbids[idx2][idx1] = true;
                return;
            }
        }
    }

    // check whether one of the connections is higher priorised on
    //  the outgoing edge when both roads are high priorised
    //  the connection with the lower priorised outgoing edge will lead
    // should be valid for priority junctions only
    /*
    if (from1p > 0 && from2p > 0) {
        assert(myJunction->getType() != SumoXMLNodeType::RIGHT_BEFORE_LEFT);
        int to1p = to1->getJunctionPriority(myJunction);
        int to2p = to2->getJunctionPriority(myJunction);
        if (to1p > to2p) {
            myForbids[idx1][idx2] = true;
            return;
        }
        if (to2p > to1p) {
            myForbids[idx2][idx1] = true;
            return;
        }
    }
    */

    // compute the yielding due to the right-before-left rule
    // get the position of the incoming lanes in the junction-wheel
    EdgeVector::const_iterator c1 = std::find(myAll.begin(), myAll.end(), from1);
    NBContHelper::nextCW(myAll, c1);
    // go through next edges clockwise...
    while (*c1 != from1 && *c1 != from2) {
        if (*c1 == to2) {
            // if we encounter to2 the second one prohibits the first
            myForbids[idx2][idx1] = true;
            return;
        }
        NBContHelper::nextCW(myAll, c1);
    }
    // get the position of the incoming lanes in the junction-wheel
    EdgeVector::const_iterator c2 = std::find(myAll.begin(), myAll.end(), from2);
    NBContHelper::nextCW(myAll, c2);
    // go through next edges clockwise...
    while (*c2 != from2 && *c2 != from1) {
        if (*c2 == to1) {
            // if we encounter to1 the second one prohibits the first
            myForbids[idx1][idx2] = true;
            return;
        }
        NBContHelper::nextCW(myAll, c2);
    }
#ifdef DEBUG_SETBLOCKING
    if (DEBUGCOND) std::cout << "setBlocking"
                                 << " 1:" << from1->getID() << "->" << to1->getID()
                                 << " 2:" << from2->getID() << "->" << to2->getID()
                                 << " noDecision\n";
#endif
}


int
NBRequest::distanceCounterClockwise(NBEdge* from, NBEdge* to) {
    EdgeVector::const_iterator p = std::find(myAll.begin(), myAll.end(), from);
    int ret = 0;
    do {
        ret++;
        if (p == myAll.begin()) {
            p = myAll.end();
        }
        p--;
    } while (*p != to);
    return ret;
}

const std::string&
NBRequest::getFoes(int linkIndex) const {
    assert(linkIndex >= 0);
    assert(linkIndex < (int)myFoes.size());
    return myFoes[linkIndex];
}


const std::string&
NBRequest::getResponse(int linkIndex) const {
    assert(linkIndex >= 0);
    assert(linkIndex < (int)myResponse.size());
    return myResponse[linkIndex];
}


void
NBRequest::writeLogic(OutputDevice& into) const {
    int numLinks = (int)myResponse.size();
    assert((int)myFoes.size() == numLinks);
    assert((int)myHaveVia.size() == numLinks);
    const bool padding = numLinks > 10;
    for (int i = 0; i < numLinks; i++) {
        into.openTag(SUMO_TAG_REQUEST);
        into.writeAttr(SUMO_ATTR_INDEX, i);
        if (padding && i < 10) {
            into.writePadding(" ");
        }
        into.writeAttr(SUMO_ATTR_RESPONSE, myResponse[i]);
        into.writeAttr(SUMO_ATTR_FOES, myFoes[i]);
        if (!OptionsCont::getOptions().getBool("no-internal-links")) {
            into.writeAttr(SUMO_ATTR_CONT, myHaveVia[i]);
        }
        into.closeTag();
    }
}


void
NBRequest::computeLogic(const bool checkLaneFoes) {
    myResponse.clear();
    myFoes.clear();
    myHaveVia.clear();
    int pos = 0;
    EdgeVector::const_iterator i;
    // normal connections
    for (i = myIncoming.begin(); i != myIncoming.end(); i++) {
        int noLanes = (*i)->getNumLanes();
        for (int k = 0; k < noLanes; k++) {
            pos = computeLaneResponse(*i, k, pos, checkLaneFoes || myJunction->getType() == SumoXMLNodeType::ZIPPER);
        }
    }
    // crossings
    auto crossings = myJunction->getCrossings();
    for (auto c : crossings) {
        pos = computeCrossingResponse(*c, pos);
    }
}

void
NBRequest::resetSignalised() {
    // go through possible prohibitions
    for (EdgeVector::const_iterator i11 = myIncoming.begin(); i11 != myIncoming.end(); i11++) {
        int noLanesEdge1 = (*i11)->getNumLanes();
        for (int j1 = 0; j1 < noLanesEdge1; j1++) {
            std::vector<NBEdge::Connection> el1 = (*i11)->getConnectionsFromLane(j1);
            for (std::vector<NBEdge::Connection>::iterator i12 = el1.begin(); i12 != el1.end(); ++i12) {
                int idx1 = getIndex((*i11), (*i12).toEdge);
                if (idx1 < 0) {
                    continue;
                }
                // go through possibly prohibited
                for (EdgeVector::const_iterator i21 = myIncoming.begin(); i21 != myIncoming.end(); i21++) {
                    int noLanesEdge2 = (*i21)->getNumLanes();
                    for (int j2 = 0; j2 < noLanesEdge2; j2++) {
                        std::vector<NBEdge::Connection> el2 = (*i21)->getConnectionsFromLane(j2);
                        for (std::vector<NBEdge::Connection>::iterator i22 = el2.begin(); i22 != el2.end(); i22++) {
                            int idx2 = getIndex((*i21), (*i22).toEdge);
                            if (idx2 < 0) {
                                continue;
                            }
                            // check
                            // same incoming connections do not prohibit each other
                            if ((*i11) == (*i21)) {
                                myForbids[idx1][idx2] = false;
                                myForbids[idx2][idx1] = false;
                                continue;
                            }
                            // check other
                            // if both are non-signalised or both are signalised
                            if (((*i12).tlID == "" && (*i22).tlID == "")
                                    ||
                                    ((*i12).tlID != "" && (*i22).tlID != "")) {
                                // do nothing
                                continue;
                            }
                            // supposing, we don not have to
                            //  brake if we are no foes
                            if (!foes(*i11, (*i12).toEdge, *i21, (*i22).toEdge)) {
                                continue;
                            }
                            // otherwise:
                            //  the non-signalised must break
                            if ((*i12).tlID != "") {
                                myForbids[idx1][idx2] = true;
                                myForbids[idx2][idx1] = false;
                            } else {
                                myForbids[idx1][idx2] = false;
                                myForbids[idx2][idx1] = true;
                            }
                        }
                    }
                }
            }
        }
    }
}


std::pair<int, int>
NBRequest::getSizes() const {
    int noLanes = 0;
    int noLinks = 0;
    for (EdgeVector::const_iterator i = myIncoming.begin();
            i != myIncoming.end(); i++) {
        int noLanesEdge = (*i)->getNumLanes();
        for (int j = 0; j < noLanesEdge; j++) {
            int numConnections = (int)(*i)->getConnectionsFromLane(j).size();
            noLinks += numConnections;
            if (numConnections > 0) {
                noLanes++;
            }
        }
    }
    return std::make_pair(noLanes, noLinks);
}


bool
NBRequest::foes(const NBEdge* const from1, const NBEdge* const to1,
                const NBEdge* const from2, const NBEdge* const to2) const {
    // unconnected edges do not forbid other edges
    if (to1 == nullptr || to2 == nullptr) {
        return false;
    }
    // get the indices
    int idx1 = getIndex(from1, to1);
    int idx2 = getIndex(from2, to2);
    if (idx1 < 0 || idx2 < 0) {
        return false; // sure? (The connection does not exist within this junction)
    }
    assert(idx1 < (int)(myIncoming.size() * myOutgoing.size()));
    assert(idx2 < (int)(myIncoming.size()*myOutgoing.size()));
    return myForbids[idx1][idx2] || myForbids[idx2][idx1];
}


bool
NBRequest::forbids(const NBEdge* const possProhibitorFrom, const NBEdge* const possProhibitorTo,
                   const NBEdge* const possProhibitedFrom, const NBEdge* const possProhibitedTo,
                   bool regardNonSignalisedLowerPriority) const {
    // unconnected edges do not forbid other edges
    if (possProhibitorTo == nullptr || possProhibitedTo == nullptr) {
        return false;
    }
    // get the indices
    int possProhibitorIdx = getIndex(possProhibitorFrom, possProhibitorTo);
    int possProhibitedIdx = getIndex(possProhibitedFrom, possProhibitedTo);
    if (possProhibitorIdx < 0 || possProhibitedIdx < 0) {
        return false; // sure? (The connection does not exist within this junction)
    }
    assert(possProhibitorIdx < (int)(myIncoming.size() * myOutgoing.size()));
    assert(possProhibitedIdx < (int)(myIncoming.size() * myOutgoing.size()));
    // check simple right-of-way-rules
    if (!regardNonSignalisedLowerPriority) {
        return myForbids[possProhibitorIdx][possProhibitedIdx];
    }
    // if its not forbidden, report
    if (!myForbids[possProhibitorIdx][possProhibitedIdx]) {
        return false;
    }
    // do not forbid a signalised stream by a non-signalised
    if (!possProhibitorFrom->hasSignalisedConnectionTo(possProhibitorTo)) {
        return false;
    }
    return true;
}

int
NBRequest::computeLaneResponse(NBEdge* from, int fromLane, int pos, const bool checkLaneFoes) {
    for (const NBEdge::Connection& c : from->getConnectionsFromLane(fromLane)) {
        assert(c.toEdge != 0);
        pos++;
        const std::string foes = getFoesString(from, c.toEdge, fromLane, c.toLane, checkLaneFoes);
        const std::string response = myJunction->getType() == SumoXMLNodeType::ZIPPER ? foes : getResponseString(from, c, checkLaneFoes);
        myFoes.push_back(foes);
        myResponse.push_back(response);
        myHaveVia.push_back(c.haveVia);
    }
    return pos;
}


int
NBRequest::computeCrossingResponse(const NBNode::Crossing& crossing, int pos) {
    std::string foes(myJunction->getCrossings().size(), '0');
    std::string response(myJunction->getCrossings().size(), '0');
    // conflicts with normal connections
    for (EdgeVector::const_reverse_iterator i = myIncoming.rbegin(); i != myIncoming.rend(); i++) {
        //const std::vector<NBEdge::Connection> &allConnections = (*i)->getConnections();
        const NBEdge* from = *i;
        int noLanes = from->getNumLanes();
        for (int j = noLanes; j-- > 0;) {
            std::vector<NBEdge::Connection> connected = from->getConnectionsFromLane(j);
            int size = (int) connected.size();
            for (int k = size; k-- > 0;) {
                const NBEdge* to = connected[k].toEdge;
                bool foe = false;
                for (EdgeVector::const_iterator it_e = crossing.edges.begin(); it_e != crossing.edges.end(); ++it_e) {
                    if ((*it_e) == from || (*it_e) == to) {
                        foe = true;
                        break;
                    }
                }
                foes += foe ? '1' : '0';
                response += mustBrakeForCrossing(myJunction, from, to, crossing) || !foe ? '0' : '1';
            }
        }
    }
    pos++;
    myResponse.push_back(response);
    myFoes.push_back(foes);
    myHaveVia.push_back(false);
    return pos;
}


std::string
NBRequest::getResponseString(const NBEdge* const from, const NBEdge::Connection& c, const bool checkLaneFoes) const {
    const NBEdge* const to = c.toEdge;
    const int fromLane = c.fromLane;
    const int toLane = c.toLane;
    int idx = 0;
    if (to != nullptr) {
        idx = getIndex(from, to);
    }
    std::string result;
    // crossings
    auto crossings = myJunction->getCrossings();
    for (std::vector<NBNode::Crossing*>::const_reverse_iterator i = crossings.rbegin(); i != crossings.rend(); i++) {
        result += mustBrakeForCrossing(myJunction, from, to, **i) ? '1' : '0';
    }
    NBEdge::Connection queryCon = from->getConnection(fromLane, to, toLane);
    // normal connections
    for (EdgeVector::const_reverse_iterator i = myIncoming.rbegin(); i != myIncoming.rend(); i++) {
        //const std::vector<NBEdge::Connection> &allConnections = (*i)->getConnections();
        int noLanes = (*i)->getNumLanes();
        for (int j = noLanes; j-- > 0;) {
            std::vector<NBEdge::Connection> connected = (*i)->getConnectionsFromLane(j);
            int size = (int) connected.size();
            for (int k = size; k-- > 0;) {
                if (c.mayDefinitelyPass) {
                    result += '0';
                } else if ((*i) == from && fromLane == j) {
                    // do not prohibit a connection by others from same lane
                    result += '0';
                } else {
                    assert(connected[k].toEdge != 0);
                    const int idx2 = getIndex(*i, connected[k].toEdge);
                    assert(k < (int) connected.size());
                    assert(idx < (int)(myIncoming.size() * myOutgoing.size()));
                    assert(idx2 < (int)(myIncoming.size() * myOutgoing.size()));
                    // check whether the connection is prohibited by another one
#ifdef DEBUG_RESPONSE
                    if (DEBUGCOND) {
                        std::cout << " c=" << queryCon.getDescription(from) << " prohibitC=" << connected[k].getDescription(*i)
                                  << " f=" << myForbids[idx2][idx]
                                  << " clf=" << checkLaneFoes
                                  << " clfbc=" << checkLaneFoesByClass(queryCon, *i, connected[k])
                                  << " clfbcoop=" << checkLaneFoesByCooperation(from, queryCon, *i, connected[k])
                                  << " lc=" << laneConflict(from, to, toLane, *i, connected[k].toEdge, connected[k].toLane)
                                  << " rtc=" << NBNode::rightTurnConflict(from, to, fromLane, *i, connected[k].toEdge, connected[k].fromLane)
                                  << " rtc2=" << rightTurnConflict(from, queryCon, *i, connected[k])
                                  << " mc=" << mergeConflict(from, queryCon, *i, connected[k], false)
                                  << " oltc=" << oppositeLeftTurnConflict(from, queryCon, *i, connected[k], false)
                                  << " rorc=" << myJunction->rightOnRedConflict(c.tlLinkIndex, connected[k].tlLinkIndex)
                                  << " tlscc=" << myJunction->tlsContConflict(from, c, *i, connected[k])
                                  << "\n";
                    }
#endif
                    const bool hasLaneConflict = (!(checkLaneFoes || checkLaneFoesByClass(queryCon, *i, connected[k])
                                                    || checkLaneFoesByCooperation(from, queryCon, *i, connected[k]))
                                                  || laneConflict(from, to, toLane, *i, connected[k].toEdge, connected[k].toLane));
                    if ((myForbids[idx2][idx] && hasLaneConflict)
                            || rightTurnConflict(from, queryCon, *i, connected[k])
                            || mergeConflict(from, queryCon, *i, connected[k], false)
                            || oppositeLeftTurnConflict(from, queryCon, *i, connected[k], false)
                            || myJunction->rightOnRedConflict(c.tlLinkIndex, connected[k].tlLinkIndex)
                            || (myJunction->tlsContConflict(from, c, *i, connected[k]) && hasLaneConflict
                                && !OptionsCont::getOptions().getBool("tls.ignore-internal-junction-jam"))
                       ) {
                        result += '1';
                    } else {
                        result += '0';
                    }
                }
            }
        }
    }
    return result;
}


std::string
NBRequest::getFoesString(NBEdge* from, NBEdge* to, int fromLane, int toLane, const bool checkLaneFoes) const {
    const bool lefthand = OptionsCont::getOptions().getBool("lefthand");
    // remember the case when the lane is a "dead end" in the meaning that
    // vehicles must choose another lane to move over the following
    // junction
    // !!! move to forbidden
    std::string result;
    // crossings
    auto crossings = myJunction->getCrossings();
    for (std::vector<NBNode::Crossing*>::const_reverse_iterator i = crossings.rbegin(); i != crossings.rend(); i++) {
        bool foes = false;
        for (EdgeVector::const_iterator it_e = (**i).edges.begin(); it_e != (**i).edges.end(); ++it_e) {
            if ((*it_e) == from || (*it_e) == to) {
                foes = true;
                break;
            }
        }
        result += foes ? '1' : '0';
    }
    NBEdge::Connection queryCon = from->getConnection(fromLane, to, toLane);
    // normal connections
    for (EdgeVector::const_reverse_iterator i = myIncoming.rbegin();
            i != myIncoming.rend(); i++) {

        for (int j = (int)(*i)->getNumLanes() - 1; j >= 0; --j) {
            std::vector<NBEdge::Connection> connected = (*i)->getConnectionsFromLane(j);
            int size = (int) connected.size();
            for (int k = size; k-- > 0;) {
                const bool hasLaneConflict = (!(checkLaneFoes || checkLaneFoesByClass(queryCon, *i, connected[k])
                                                || checkLaneFoesByCooperation(from, queryCon, *i, connected[k]))
                                              || laneConflict(from, to, toLane, *i, connected[k].toEdge, connected[k].toLane));
                if ((foes(from, to, (*i), connected[k].toEdge) && hasLaneConflict)
                        || rightTurnConflict(from, queryCon, *i, connected[k])
                        || myJunction->turnFoes(from, to, fromLane, *i, connected[k].toEdge, connected[k].fromLane, lefthand)
                        || mergeConflict(from, queryCon, *i, connected[k], true)
                        || oppositeLeftTurnConflict(from, queryCon, *i, connected[k], true)
                   ) {
                    result += '1';
                } else {
                    result += '0';
                }
            }
        }
    }
    return result;
}


bool
NBRequest::rightTurnConflict(const NBEdge* from, const NBEdge::Connection& con,
                             const NBEdge* prohibitorFrom, const NBEdge::Connection& prohibitorCon) const {
    return (!con.mayDefinitelyPass &&
            (NBNode::rightTurnConflict(from, con.toEdge, con.fromLane, prohibitorFrom, prohibitorCon.toEdge, prohibitorCon.fromLane)
             // reverse conflicht (override)
             || (prohibitorCon.mayDefinitelyPass &&
                 NBNode::rightTurnConflict(prohibitorFrom, prohibitorCon.toEdge, prohibitorCon.fromLane, from, con.toEdge, con.fromLane))));


}


bool
NBRequest::mergeConflict(const NBEdge* from, const NBEdge::Connection& con,
                         const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon, bool foes) const {
    if (from == prohibitorFrom
            && con.toEdge == prohibitorCon.toEdge
            && con.toLane == prohibitorCon.toLane
            && con.fromLane != prohibitorCon.fromLane
            && !myJunction->isConstantWidthTransition()) {
        if (foes) {
            return true;
        }
        if (prohibitorCon.mayDefinitelyPass) {
            return true;
        }
        if (con.mayDefinitelyPass) {
            return false;
        }
        const bool bike = from->getPermissions(con.fromLane) == SVC_BICYCLE;
        const bool prohibitorBike = prohibitorFrom->getPermissions(prohibitorCon.fromLane) == SVC_BICYCLE;
        if (myOutgoing.size() == 1) {
            // at on-ramp like situations, right lane should yield
            return bike || (con.fromLane < prohibitorCon.fromLane && !prohibitorBike);
        } else if (myIncoming.size() == 1) {
            // at off-ramp like situations, right lane should pass unless it's a bicycle lane
            return bike || (con.fromLane > prohibitorCon.fromLane && !prohibitorBike);
        } else {
            // priority depends on direction:
            // for right turns the rightmost lane gets priority
            // otherwise the left lane
            LinkDirection dir = myJunction->getDirection(from, con.toEdge);
            if (dir == LinkDirection::RIGHT || dir == LinkDirection::PARTRIGHT) {
                return con.fromLane > prohibitorCon.fromLane;
            } else {
                return con.fromLane < prohibitorCon.fromLane;
            }
        }

    } else {
        return false;
    }
}


bool
NBRequest::oppositeLeftTurnConflict(const NBEdge* from, const NBEdge::Connection& con,
                                    const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon, bool foes) const {
    LinkDirection dir = myJunction->getDirection(from, con.toEdge);
    // XXX lefthand issue (solve via #4256)
    if (dir != LinkDirection::LEFT && dir != LinkDirection::PARTLEFT) {
        return false;
    }
    dir = myJunction->getDirection(prohibitorFrom, prohibitorCon.toEdge);
    if (dir != LinkDirection::LEFT && dir != LinkDirection::PARTLEFT) {
        return false;
    }
    if (from == prohibitorFrom || NBRequest::foes(from, con.toEdge, prohibitorFrom, prohibitorCon.toEdge)) {
        // not an opposite pair
        return false;
    };

    double width2 = prohibitorCon.toEdge->getLaneWidth(prohibitorCon.toLane) / 2;
    PositionVector shape = con.shape;
    shape.append(con.viaShape);
    PositionVector otherShape = prohibitorCon.shape;
    otherShape.append(prohibitorCon.viaShape);
    if (shape.size() == 0 || otherShape.size() == 0) {
        // no internal lanes built
        return false;
    }
    const double minDV = NBEdge::firstIntersection(shape, otherShape, width2);
    if (minDV < shape.length() - POSITION_EPS && minDV > POSITION_EPS) {
        // break symmetry using edge id
        return foes || from->getID() < prohibitorFrom->getID();
    } else {
        return false;
    }
}

bool
NBRequest::checkLaneFoesByClass(const NBEdge::Connection& con,
                                const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon) const {
    if (con.toEdge != prohibitorCon.toEdge) {
        return false;
    }
    SVCPermissions svc = con.toEdge->getPermissions(con.toLane);
    SVCPermissions svc2 = prohibitorFrom->getPermissions(prohibitorCon.fromLane) & prohibitorCon.toEdge->getPermissions(prohibitorCon.toLane);
    // check for lane level conflict if the only common classes are bicycles or pedestrians
    return (svc & svc2 & ~(SVC_BICYCLE | SVC_PEDESTRIAN)) == 0;
}


bool
NBRequest::checkLaneFoesByCooperation(const NBEdge* from, const NBEdge::Connection& con,
                                      const NBEdge* prohibitorFrom,  const NBEdge::Connection& prohibitorCon) const {
    if (con.toEdge != prohibitorCon.toEdge) {
        return false;
    }
    // if from and prohibitorFrom target distinct lanes for all their
    // connections to the common target edge, cooperation is possible
    // (and should always happen unless the connections cross for some byzantine reason)

    std::set<int> fromTargetLanes;
    for (const auto& c : from->getConnections()) {
        if (c.toEdge == con.toEdge) {
            fromTargetLanes.insert(c.toLane);
        }
    }
    for (const auto& c : prohibitorFrom->getConnections()) {
        if (c.toEdge == con.toEdge && fromTargetLanes.count(c.toLane) != 0) {
            //std::cout << " con=" << con->getDescription(from) << " foe=" << prohibitorCon.getDescription(prohibitorFrom)
            //    << " no cooperation (targets=" << joinToString(fromTargetLanes, ' ') << " index=" << c.toLane << "\n";
            return false;
        }
    }
    return true;
}


bool
NBRequest::laneConflict(const NBEdge* from, const NBEdge* to, int toLane,
                        const NBEdge* prohibitorFrom, const NBEdge* prohibitorTo, int prohibitorToLane) const {
    if (to != prohibitorTo) {
        return true;
    }
    // since we know that the edge2edge connections are in conflict, the only
    // situation in which the lane2lane connections can be conflict-free is, if
    // they target the same edge but do not cross each other
    double angle = NBHelpers::relAngle(
                       from->getAngleAtNode(from->getToNode()), to->getAngleAtNode(to->getFromNode()));
    if (angle == 180) {
        angle = -180; // turnarounds are left turns
    }
    const double prohibitorAngle = NBHelpers::relAngle(
                                       prohibitorFrom->getAngleAtNode(prohibitorFrom->getToNode()), to->getAngleAtNode(to->getFromNode()));
    const bool rightOfProhibitor = prohibitorFrom->isTurningDirectionAt(to)
                                   || (angle > prohibitorAngle && !from->isTurningDirectionAt(to));
    return rightOfProhibitor ? toLane >= prohibitorToLane : toLane <= prohibitorToLane;
}

int
NBRequest::getIndex(const NBEdge* const from, const NBEdge* const to) const {
    EdgeVector::const_iterator fp = std::find(myIncoming.begin(), myIncoming.end(), from);
    EdgeVector::const_iterator tp = std::find(myOutgoing.begin(), myOutgoing.end(), to);
    if (fp == myIncoming.end() || tp == myOutgoing.end()) {
        return -1;
    }
    // compute the index
    return (int)(distance(myIncoming.begin(), fp) * myOutgoing.size() + distance(myOutgoing.begin(), tp));
}


std::ostream&
operator<<(std::ostream& os, const NBRequest& r) {
    int variations = r.numLinks();
    for (int i = 0; i < variations; i++) {
        os << i << ' ';
        for (int j = 0; j < variations; j++) {
            if (r.myForbids[i][j]) {
                os << '1';
            } else {
                os << '0';
            }
        }
        os << std::endl;
    }
    os << std::endl;
    return os;
}


bool
NBRequest::mustBrake(const NBEdge* const from, const NBEdge* const to, int fromLane, int toLane, bool includePedCrossings) const {
    NBEdge::Connection con(fromLane, const_cast<NBEdge*>(to), toLane);
    const int linkIndex = myJunction->getConnectionIndex(from, con);
    if (linkIndex >= 0 && (int)myResponse.size() > linkIndex) {
        std::string response = getResponse(linkIndex);
        if (!includePedCrossings) {
            response = response.substr(0, response.size() - myJunction->getCrossings().size());
        }
        if (response.find_first_of("1") == std::string::npos) {
            return false;
        } else if (!myJunction->isTLControlled()) {
            return true;
        }
        // if the link must respond it could also be due to a tlsConflict. This
        // must not carry over the the off-state response so we continue with
        // the regular check
    }
    // get the indices
    int idx2 = getIndex(from, to);
    if (idx2 == -1) {
        return false;
    }
    // go through all (existing) connections;
    //  check whether any of these forbids the one to determine
    assert(idx2 < (int)(myIncoming.size()*myOutgoing.size()));
    for (int idx1 = 0; idx1 < numLinks(); idx1++) {
        //assert(myDone[idx1][idx2]);
        if (myDone[idx1][idx2] && myForbids[idx1][idx2]) {
            return true;
        }
    }
    // maybe we need to brake for a pedestrian crossing
    if (includePedCrossings) {
        auto crossings = myJunction->getCrossings();
        for (std::vector<NBNode::Crossing*>::const_reverse_iterator i = crossings.rbegin(); i != crossings.rend(); i++) {
            if (mustBrakeForCrossing(myJunction, from, to, **i)) {
                return true;
            }
        }
    }
    // maybe we need to brake due to a right-turn conflict with straight-going
    // bicycles
    NBEdge::Connection queryCon = from->getConnection(fromLane, to, toLane);
    LinkDirection dir = myJunction->getDirection(from, to);
    if (dir == LinkDirection::RIGHT || dir == LinkDirection::PARTRIGHT) {
        for (const NBEdge::Connection& con : from->getConnections()) {
            if (rightTurnConflict(from, queryCon, from, con)) {
                return true;
            }
        }
    }
    // maybe we need to brake due to a merge conflict
    for (EdgeVector::const_reverse_iterator i = myIncoming.rbegin(); i != myIncoming.rend(); i++) {
        int noLanes = (*i)->getNumLanes();
        for (int j = noLanes; j-- > 0;) {
            std::vector<NBEdge::Connection> connected = (*i)->getConnectionsFromLane(j);
            const int size = (int) connected.size();
            for (int k = size; k-- > 0;) {
                if ((*i) == from && fromLane != j
                        && mergeConflict(from, queryCon, *i, connected[k], myJunction->getType() == SumoXMLNodeType::ZIPPER)) {
                    return true;
                }
            }
        }
    }
    // maybe we need to brake due to a zipper conflict
    if (myJunction->getType() == SumoXMLNodeType::ZIPPER) {
        for (int idx1 = 0; idx1 < numLinks(); idx1++) {
            //assert(myDone[idx1][idx2]);
            if (myDone[idx1][idx2] && myForbids[idx2][idx1]) {
                return true;
            }
        }
    }
    return false;
}


bool
NBRequest::mustBrakeForCrossing(const NBNode* node, const NBEdge* const from, const NBEdge* const to, const NBNode::Crossing& crossing) {
    const LinkDirection dir = node->getDirection(from, to);
    const bool mustYield = dir == LinkDirection::LEFT || dir == LinkDirection::RIGHT;
    if (crossing.priority || mustYield) {
        for (EdgeVector::const_iterator it_e = crossing.edges.begin(); it_e != crossing.edges.end(); ++it_e) {
            // left and right turns must yield to unprioritized crossings only on their destination edge
            if (((*it_e) == from && crossing.priority) || (*it_e) == to) {
                return true;
            }
        }
    }
    return false;
}


bool
NBRequest::mustBrake(const NBEdge* const possProhibitorFrom, const NBEdge* const possProhibitorTo,
                     const NBEdge* const possProhibitedFrom, const NBEdge* const possProhibitedTo) const {
    // get the indices
    int idx1 = getIndex(possProhibitorFrom, possProhibitorTo);
    int idx2 = getIndex(possProhibitedFrom, possProhibitedTo);
    return (myForbids[idx2][idx1]);
}


void
NBRequest::reportWarnings() {
    // check if any errors occurred on build the link prohibitions
    if (myNotBuild != 0) {
        WRITE_WARNING(toString(myNotBuild) + " of " + toString(myNotBuild + myGoodBuilds) + " prohibitions were not build.");
    }
}


void
NBRequest::resetCooperating() {
    // map from edge to number of incoming connections
    std::map<NBEdge*, int> incomingCount; // initialized to 0
    // map from edge to indices of approached lanes
    std::map<NBEdge*, std::set<int> > approachedLanes;
    // map from edge to list of incoming edges
    std::map<NBEdge*, EdgeVector> incomingEdges;
    for (EdgeVector::const_iterator it_e = myIncoming.begin(); it_e != myIncoming.end(); it_e++) {
        const std::vector<NBEdge::Connection> connections = (*it_e)->getConnections();
        for (std::vector<NBEdge::Connection>::const_iterator it_c = connections.begin(); it_c != connections.end(); ++it_c) {
            incomingCount[it_c->toEdge]++;
            approachedLanes[it_c->toEdge].insert(it_c->toLane);
            incomingEdges[it_c->toEdge].push_back(*it_e);
        }
    }
    for (std::map<NBEdge*, int>::iterator it = incomingCount.begin(); it != incomingCount.end(); ++it) {
        NBEdge* to = it->first;
        // we cannot test against to->getNumLanes() since not all lanes may be used
        if ((int)approachedLanes[to].size() >= it->second) {
            EdgeVector& incoming = incomingEdges[to];
            // make these connections mutually unconflicting
            for (EdgeVector::iterator it_e1 = incoming.begin(); it_e1 != incoming.end(); ++it_e1) {
                for (EdgeVector::iterator it_e2 = incoming.begin(); it_e2 != incoming.end(); ++it_e2) {
                    myForbids[getIndex(*it_e1, to)][getIndex(*it_e2, to)] = false;
                }
            }
        }
    }
}


bool
NBRequest::hasConflict() const {
    for (std::string foes : myFoes) {
        if (foes.find_first_of("1") != std::string::npos) {
            return true;
        }
    }
    return false;
}


int
NBRequest::numLinks() const {
    return (int)(myIncoming.size() * myOutgoing.size() + myJunction->getCrossings().size());
}


/****************************************************************************/
