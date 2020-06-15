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
/// @file    NIVissimEdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A temporary storage for edges imported from Vissim
/****************************************************************************/
#include <config.h>

#include <string>
#include <algorithm>
#include <map>
#include <cassert>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iterator>
#include <utils/common/ToString.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/GeomHelper.h>
#include <utils/distribution/DistributionCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <utils/options/OptionsCont.h>
#include "NIVissimNodeCluster.h"
#include "NIVissimDistrictConnection.h"
#include "NIVissimClosedLanesVector.h"
#include "NIVissimConnection.h"
#include "NIVissimDisturbance.h"
#include "NIVissimEdge.h"
#include <utils/common/MsgHandler.h>


// ===========================================================================
// static members
// ===========================================================================
NIVissimEdge::DictType NIVissimEdge::myDict;
int NIVissimEdge::myMaxID = 0;
std::vector<std::string> NIVissimEdge::myLanesWithMissingSpeeds;


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimEdge::connection_position_sorter::connection_position_sorter(int edgeid)
    : myEdgeID(edgeid) {}


int
NIVissimEdge::connection_position_sorter::operator()(int c1id,
        int c2id) const {
    NIVissimConnection* c1 = NIVissimConnection::dictionary(c1id);
    NIVissimConnection* c2 = NIVissimConnection::dictionary(c2id);
    double pos1 =
        c1->getFromEdgeID() == myEdgeID
        ? c1->getFromPosition() : c1->getToPosition();
    double pos2 =
        c2->getFromEdgeID() == myEdgeID
        ? c2->getFromPosition() : c2->getToPosition();
    return pos1 < pos2;
}








NIVissimEdge::connection_cluster_position_sorter::connection_cluster_position_sorter(int edgeid)
    : myEdgeID(edgeid) {}


int
NIVissimEdge::connection_cluster_position_sorter::operator()(
    NIVissimConnectionCluster* cc1,
    NIVissimConnectionCluster* cc2) const {
    double pos1 = cc1->getPositionForEdge(myEdgeID);
    double pos2 = cc2->getPositionForEdge(myEdgeID);
    if (pos2 < 0 || pos1 < 0) {
        cc1->getPositionForEdge(myEdgeID);
        cc2->getPositionForEdge(myEdgeID);
    }
    assert(pos1 >= 0 && pos2 >= 0);
    return pos1 < pos2;
}




NIVissimEdge::NIVissimEdge(int id, const std::string& name,
                           const std::string& type,
                           std::vector<double> laneWidths,
                           double zuschlag1, double zuschlag2,
                           double /*length*/, const PositionVector& geom,
                           const NIVissimClosedLanesVector& clv) :
    NIVissimAbstractEdge(id, geom),
    myName(name), myType(type), myNoLanes((int)laneWidths.size()),
    myLaneWidths(laneWidths),
    myZuschlag1(zuschlag1), myZuschlag2(zuschlag2),
    myClosedLanes(clv),
    myLaneSpeeds(myNoLanes, -1),
    myAmWithinJunction(false)
    //, mySpeed(-1)
{
    assert(myNoLanes >= 0);
    if (myMaxID < myID) {
        myMaxID = myID;
    }
}


NIVissimEdge::~NIVissimEdge() {
    for (NIVissimClosedLanesVector::iterator i = myClosedLanes.begin(); i != myClosedLanes.end(); i++) {
        delete (*i);
    }
    myClosedLanes.clear();
}


bool
NIVissimEdge::dictionary(int id, const std::string& name,
                         const std::string& type, int noLanes,
                         double zuschlag1, double zuschlag2, double length,
                         const PositionVector& geom,
                         const NIVissimClosedLanesVector& clv) {
    NIVissimEdge* o = new NIVissimEdge(id, name, type, std::vector<double>(noLanes, NBEdge::UNSPECIFIED_WIDTH),
                                       zuschlag1, zuschlag2, length, geom, clv);
    if (!dictionary(id, o)) {
        delete o;
        return false;
    }
    return true;
}



bool
NIVissimEdge::dictionary(int id, NIVissimEdge* o) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = o;
        return true;
    }
    return false;
}



NIVissimEdge*
NIVissimEdge::dictionary(int id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}


void
NIVissimEdge::buildConnectionClusters() {
    const double MAX_CLUSTER_DISTANCE = 10;
    // build clusters for all edges made up from not previously assigne
    //  connections
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        int edgeid = (*i).first;
        NIVissimEdge* edge = (*i).second;
        // get all connectors using this edge
        std::vector<int> connectors = edge->myIncomingConnections;
        copy(edge->myOutgoingConnections.begin(), edge->myOutgoingConnections.end(), back_inserter(connectors));
        if (connectors.size() == 0) {
            continue;
        }
        // sort the connectors by the place on the edge
        sort(connectors.begin(), connectors.end(), connection_position_sorter(edgeid));
        // try to cluster the connections participating within the current edge
        std::vector<int> currentCluster;
        std::vector<int>::iterator j = connectors.begin();
        bool outgoing = NIVissimConnection::dictionary(*j)->getFromEdgeID() == (*i).first;
        double position = outgoing
                          ? NIVissimConnection::dictionary(*j)->getFromPosition()
                          : NIVissimConnection::dictionary(*j)->getToPosition();

        // skip connections already in a cluster
        // !!! (?)
        while (j != connectors.end() && NIVissimConnection::dictionary(*j)->hasNodeCluster()) {
            ++j;
        }
        if (j == connectors.end()) {
            continue;
        }
        currentCluster.push_back(*j);
        do {
            if (j + 1 != connectors.end() && !NIVissimConnection::dictionary(*j)->hasNodeCluster()) {
                bool n_outgoing = NIVissimConnection::dictionary(*(j + 1))->getFromEdgeID() == edgeid;
                double n_position = n_outgoing
                                    ? NIVissimConnection::dictionary(*(j + 1))->getFromPosition()
                                    : NIVissimConnection::dictionary(*(j + 1))->getToPosition();
                if (n_outgoing == outgoing && fabs(n_position - position) < MAX_CLUSTER_DISTANCE) {
                    // ok, in same cluster as prior
                    currentCluster.push_back(*(j + 1));
                } else {
                    // start new cluster
                    VectorHelper<int>::removeDouble(currentCluster);
                    edge->myConnectionClusters.push_back(new NIVissimConnectionCluster(currentCluster, -1, edgeid));
                    currentCluster.clear();
                    currentCluster.push_back(*(j + 1));
                }
                outgoing = n_outgoing;
                position = n_position;
            }
            j++;
        } while (j != connectors.end());
        // add last connection
        if (currentCluster.size() > 0) {
            VectorHelper<int>::removeDouble(currentCluster);
            edge->myConnectionClusters.push_back(new NIVissimConnectionCluster(currentCluster, -1, edgeid));
        }
    }
}


void
NIVissimEdge::dict_buildNBEdges(NBDistrictCont& dc, NBNodeCont& nc,
                                NBEdgeCont& ec, double offset) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimEdge* edge = (*i).second;
        edge->buildNBEdge(dc, nc, ec, offset);
    }
}


void
NIVissimEdge::dict_propagateSpeeds() {
    DictType::iterator i;
    for (i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimEdge* edge = (*i).second;
        edge->setDistrictSpeed();
    }
    for (i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimEdge* edge = (*i).second;
        edge->propagateSpeed(-1, std::vector<int>());
    }
    for (int j = 0; j < 3; j++) {
        for (i = myDict.begin(); i != myDict.end(); i++) {
            NIVissimEdge* edge = (*i).second;
            edge->propagateOwn();
        }
        for (i = myDict.begin(); i != myDict.end(); i++) {
            NIVissimEdge* edge = (*i).second;
            edge->checkUnconnectedLaneSpeeds();
        }
    }
}


void
NIVissimEdge::checkUnconnectedLaneSpeeds() {
    for (int i = 0; i < (int) myLaneSpeeds.size(); i++) {
        if (myLaneSpeeds[i] == -1) {
            double speed = -1;
            int j1 = i - 1; // !!! recheck - j1 may become negative?
            int j2 = i;
            while (j2 != (int) myLaneSpeeds.size() && myLaneSpeeds[j2] == -1) {
                j2++;
            }
            if (j1 < 0) {
                if (j2 < (int) myLaneSpeeds.size()) {
                    speed = myLaneSpeeds[j2];
                }
            } else {
                if (j2 >= (int) myLaneSpeeds.size()) {
                    speed = myLaneSpeeds[j1];
                } else {
                    speed = (myLaneSpeeds[j1] + myLaneSpeeds[j2]) / (double) 2.0;
                }
            }
            if (speed == -1) {
                continue;
            }
            myLaneSpeeds[i] = speed;
            std::vector<NIVissimConnection*> connected = getOutgoingConnected(i);
            for (std::vector<NIVissimConnection*>::iterator j = connected.begin(); j != connected.end(); j++) {
                NIVissimConnection* c = *j;
                NIVissimEdge* e = NIVissimEdge::dictionary(c->getToEdgeID());
                // propagate
                e->propagateSpeed(/*dc, */speed, c->getToLanes());
            }
        }
    }
}


void
NIVissimEdge::propagateOwn() {
    for (int i = 0; i < (int) myLaneSpeeds.size(); i++) {
        if (myLaneSpeeds[i] == -1) {
            continue;
        }
        std::vector<NIVissimConnection*> connected = getOutgoingConnected(i);
        for (std::vector<NIVissimConnection*>::iterator j = connected.begin(); j != connected.end(); j++) {
            NIVissimConnection* c = *j;
            NIVissimEdge* e = NIVissimEdge::dictionary(c->getToEdgeID());
            // propagate
            e->propagateSpeed(/*dc, */myLaneSpeeds[i], c->getToLanes());
        }
    }
}


void
NIVissimEdge::propagateSpeed(double speed, std::vector<int> forLanes) {
    // if no lane is given, all set be set
    if (forLanes.size() == 0) {
        for (int i = 0; i < myNoLanes; i++) {
            forLanes.push_back((int) i);
        }
    }
    // for the case of a first call
    // go through the lanes
    for (std::vector<int>::const_iterator i = forLanes.begin(); i < forLanes.end(); i++) {
        // check whether a speed was set before
        if (myLaneSpeeds[*i] != -1) {
            // do not reset it from incoming
            continue;
        }
        // check whether the lane has a new speed to set
        if ((int) myPatchedSpeeds.size() > *i && myPatchedSpeeds[*i] != -1) {
            // use it
            speed = getRealSpeed(/*dc, */myPatchedSpeeds[*i]);
        }
        // check whether a speed is given
        if (speed == -1) {
            // do nothing if not
            continue;
        }
        // set the lane's speed to the given
        myLaneSpeeds[*i] = speed;
        // propagate the speed further
        // get the list of connected edges
        std::vector<NIVissimConnection*> connected = getOutgoingConnected(*i);
        // go throught the list
        for (std::vector<NIVissimConnection*>::iterator j = connected.begin(); j != connected.end(); j++) {
            NIVissimConnection* c = *j;
            NIVissimEdge* e = NIVissimEdge::dictionary(c->getToEdgeID());
            // propagate
            e->propagateSpeed(/*dc, */speed, c->getToLanes());
        }
    }
}



void
NIVissimEdge::setDistrictSpeed() {
    if (myDistrictConnections.size() > 0) {
        double pos = *(myDistrictConnections.begin());
        if (pos < getLength() - pos) {
            NIVissimDistrictConnection* d =
                NIVissimDistrictConnection::dict_findForEdge(myID);
            if (d != nullptr) {
                double speed = d->getMeanSpeed(/*dc*/);
                if (speed == -1) {
                    return;
                }
                for (int i = 0; i < myNoLanes; i++) {
                    myLaneSpeeds[i] = speed;
                    // propagate the speed further
                    // get the list of connected edges
                    std::vector<NIVissimConnection*> connected = getOutgoingConnected(i);
                    // go throught the list
                    for (std::vector<NIVissimConnection*>::iterator j = connected.begin(); j != connected.end(); j++) {
                        NIVissimConnection* c = *j;
                        NIVissimEdge* e = NIVissimEdge::dictionary(c->getToEdgeID());
                        // propagate
                        e->propagateSpeed(/*dc, */speed, c->getToLanes());
                    }
                }
            }
        }
    }
}


std::vector<NIVissimConnection*>
NIVissimEdge::getOutgoingConnected(int lane) const {
    std::vector<NIVissimConnection*> ret;
    for (std::vector<int>::const_iterator i = myOutgoingConnections.begin(); i != myOutgoingConnections.end(); i++) {
        NIVissimConnection* c = NIVissimConnection::dictionary(*i);
        const std::vector<int>& lanes = c->getFromLanes();
        if (find(lanes.begin(), lanes.end(), lane) != lanes.end()) {
            NIVissimEdge* e = NIVissimEdge::dictionary(c->getToEdgeID());
            if (e != nullptr) {
                ret.push_back(c);
            }
        }
    }
    return ret;
}


void
NIVissimEdge::buildNBEdge(NBDistrictCont& dc, NBNodeCont& nc, NBEdgeCont& ec,
                          double sameNodesOffset) {
    // build the edge
    std::pair<NIVissimConnectionCluster*, NBNode*> fromInf, toInf;
    NBNode* fromNode, *toNode;
    fromNode = toNode = nullptr;
    sort(myConnectionClusters.begin(), myConnectionClusters.end(), connection_cluster_position_sorter(myID));
    sort(myDistrictConnections.begin(), myDistrictConnections.end());
    ConnectionClusters tmpClusters = myConnectionClusters;
    if (tmpClusters.size() != 0) {
        sort(tmpClusters.begin(), tmpClusters.end(), connection_cluster_position_sorter(myID));
        // get or build the from-node
        //  A node may have to be build when the edge starts or ends at
        //  a parking place or something like this
        fromInf = getFromNode(nc, tmpClusters);
        fromNode = fromInf.second;
        // get or build the to-node
        //if(tmpClusters.size()>0) {
        toInf = getToNode(nc, tmpClusters);
        toNode = toInf.second;
        if (fromInf.first != 0 && toNode != nullptr && fromInf.first->around(toNode->getPosition())) {
            WRITE_WARNING("Will not build edge '" + toString(myID) + "'.");
            myAmWithinJunction = true;
            return;
        }
        //}
        // if both nodes are the same, resolve the problem otherwise
        if (fromNode == toNode) {
            std::pair<NBNode*, NBNode*> tmp = resolveSameNode(nc, sameNodesOffset, fromNode, toNode);
            if (fromNode != tmp.first) {
                fromInf.first = 0;
            }
            if (toNode != tmp.second) {
                toInf.first = 0;
            }
            fromNode = tmp.first;
            toNode = tmp.second;
        }
    }

    //
    if (fromNode == nullptr) {
        fromInf.first = 0;
        Position pos = myGeom[0];
        fromNode = new NBNode(toString<int>(myID) + "-SourceNode", pos, SumoXMLNodeType::NOJUNCTION);
        if (!nc.insert(fromNode)) {
            throw ProcessError("Could not insert node '" + fromNode->getID() + "' to nodes container.");
        }
    }
    if (toNode == nullptr) {
        toInf.first = 0;
        Position pos = myGeom[-1];
        toNode = new NBNode(toString<int>(myID) + "-DestinationNode", pos, SumoXMLNodeType::NOJUNCTION);
        if (!nc.insert(toNode)) {
            throw ProcessError("Could not insert node '" + toNode->getID() + "' to nodes container.");
        }
    }

    // build the edge
    double avgSpeed = 0;
    for (int i = 0; i < myNoLanes; i++) {
        if ((int)myLaneSpeeds.size() <= i || myLaneSpeeds[i] == -1) {
            myLanesWithMissingSpeeds.push_back(toString(myID) + "_" + toString(i));
            avgSpeed += OptionsCont::getOptions().getFloat("vissim.default-speed");
        } else {
            avgSpeed += myLaneSpeeds[i];
        }
    }
    avgSpeed /= (double) myLaneSpeeds.size();
    avgSpeed *= OptionsCont::getOptions().getFloat("vissim.speed-norm");

    if (fromNode == toNode) {
        WRITE_WARNING("Could not build edge '" + toString(myID) + "'; would connect same node.");
        return;
    }

    NBEdge* buildEdge = new NBEdge(toString<int>(myID), fromNode, toNode, myType,
                                   avgSpeed / (double) 3.6, myNoLanes, -1,
                                   NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET,
                                   myGeom, myName, "", LaneSpreadFunction::CENTER, true);
    for (int i = 0; i < myNoLanes; i++) {
        buildEdge->setLaneWidth(i, myLaneWidths[i]);
        if ((int) myLaneSpeeds.size() <= i || myLaneSpeeds[i] == -1) {
            buildEdge->setSpeed(i, OptionsCont::getOptions().getFloat("vissim.default-speed") / (double) 3.6);
        } else {
            buildEdge->setSpeed(i, myLaneSpeeds[i] / (double) 3.6);
        }
    }
    ec.insert(buildEdge);
    // check whether the edge contains any other clusters
    if (tmpClusters.size() > 0) {
        bool cont = true;
        for (ConnectionClusters::iterator j = tmpClusters.begin(); cont && j != tmpClusters.end(); ++j) {
            // split the edge at the previously build node
            std::string nextID = buildEdge->getID() + "[1]";
            cont = ec.splitAt(dc, buildEdge, (*j)->getNBNode());
            // !!! what to do if the edge could not be split?
            buildEdge = ec.retrieve(nextID);
        }
    }
}


double
NIVissimEdge::getRealSpeed(int distNo) {
    std::string id = toString<int>(distNo);
    Distribution* dist = DistributionCont::dictionary("speed", id);
    if (dist == nullptr) {
        WRITE_WARNING("The referenced speed distribution '" + id + "' is not known.");
        return -1;
    }
    assert(dist != 0);
    double speed = dist->getMax();
    if (speed < 0 || speed > 1000) {
        WRITE_WARNING("What about distribution '" + toString<int>(distNo) + "' ");
    }
    return speed;
}

/*
bool
NIVissimEdge::recheckSpeedPatches()
{
//    int speed_idx = -1;
    // check set speeds
    if(myPatchedSpeeds.size()!=0) {
        std::vector<double>::iterator i =
            std::find(myPatchedSpeeds.begin(), myPatchedSpeeds.end(), -1);
        if(myPatchedSpeeds.size()!=myNoLanes||i!=myPatchedSpeeds.end()) {
            cot << "Warning! Not all lanes are patched! (edge:" << myID << ")." << endl;
        }
        //
        if(std::vector<double>Helper::maxValue(myPatchedSpeeds)!=std::vector<double>Helper::minValue(myPatchedSpeeds)) {
            cot << "Warning! Not all lanes have the same speed!! (edge:" << myID << ")." << endl;
        }
        //
/        // !!! ist natuerlich Quatsch - erst recht, wenn Edges zusammengefasst werden
        speed = std::vector<double>Helper::sum(myPatchedSpeeds);
        speed /= (double) myPatchedSpeeds.size();*/
/*        return true;
    }
    if(myDistrictConnections.size()>0) {
        double pos = *(myDistrictConnections.begin());
//        if(pos<10) {
            NIVissimDistrictConnection *d =
                NIVissimDistrictConnection::dict_findForEdge(myID);
            if(d!=0) {
                return true;
//                speed = d->getMeanSpeed();
            }
//        }
//        return true;
    }
    return false;
}
*/

std::pair<NIVissimConnectionCluster*, NBNode*>
NIVissimEdge::getFromNode(NBNodeCont& nc, ConnectionClusters& clusters) {
    // changed MAX_DISTANCE from 10 to 3.5, because 3.5 is the default lane width in VISSIM
    const double MAX_DISTANCE = 3.5;
    assert(clusters.size() >= 1);
    const Position& beg = myGeom.front();
    NIVissimConnectionCluster* c = *(clusters.begin());
    // check whether the edge starts within a already build node
    if (c->around(beg, MAX_DISTANCE)) {
        clusters.erase(clusters.begin());
        return std::pair<NIVissimConnectionCluster*, NBNode*>
               (c, c->getNBNode());
    }
    // check for a parking place at the begin
    if (myDistrictConnections.size() > 0) {
        double pos = *(myDistrictConnections.begin());
        if (pos < 10) {
            NBNode* node = new NBNode(toString<int>(myID) + "-begin", beg, SumoXMLNodeType::NOJUNCTION);
            if (!nc.insert(node)) {
                throw 1;
            }
            while (myDistrictConnections.size() > 0 && *(myDistrictConnections.begin()) < 10) {
                myDistrictConnections.erase(myDistrictConnections.begin());
            }
            return std::pair<NIVissimConnectionCluster*, NBNode*>(static_cast<NIVissimConnectionCluster*>(nullptr), node);
        }
    }
    // build a new node for the edge's begin otherwise
    NBNode* node = new NBNode(toString<int>(myID) + "-begin", beg, SumoXMLNodeType::NOJUNCTION);
    if (!nc.insert(node)) {
        throw 1;
    }
    return std::pair<NIVissimConnectionCluster*, NBNode*>(static_cast<NIVissimConnectionCluster*>(nullptr), node);
}


std::pair<NIVissimConnectionCluster*, NBNode*>
NIVissimEdge::getToNode(NBNodeCont& nc, ConnectionClusters& clusters) {
    const Position& end = myGeom.back();
    if (clusters.size() > 0) {
        const double MAX_DISTANCE = 10.;
        assert(clusters.size() >= 1);
        NIVissimConnectionCluster* c = *(clusters.end() - 1);
        // check whether the edge ends within a already build node
        if (c->around(end, MAX_DISTANCE)) {
            clusters.erase(clusters.end() - 1);
            return std::pair<NIVissimConnectionCluster*, NBNode*>(c, c->getNBNode());
        }
    }
    // check for a parking place at the end
    if (myDistrictConnections.size() > 0) {
        double pos = *(myDistrictConnections.end() - 1);
        if (pos > myGeom.length() - 10) {
            NBNode* node = new NBNode(toString<int>(myID) + "-end", end, SumoXMLNodeType::NOJUNCTION);
            if (!nc.insert(node)) {
                throw 1;
            }
            while (myDistrictConnections.size() > 0 && *(myDistrictConnections.end() - 1) < myGeom.length() - 10) {
                myDistrictConnections.erase(myDistrictConnections.end() - 1);
            }
            return std::pair<NIVissimConnectionCluster*, NBNode*>(static_cast<NIVissimConnectionCluster*>(nullptr), node);
        }
    }

    // build a new node for the edge's end otherwise
    NBNode* node = new NBNode(toString<int>(myID) + "-end", end, SumoXMLNodeType::NOJUNCTION);
    if (!nc.insert(node)) {
        throw 1;
    }
    return std::pair<NIVissimConnectionCluster*, NBNode*>(static_cast<NIVissimConnectionCluster*>(nullptr), node);
    /*
    if (clusters.size()>0) {
    NIVissimConnectionCluster *c = *(clusters.end()-1);
    clusters.erase(clusters.end()-1);
    return std::pair<NIVissimConnectionCluster*, NBNode*>(c, c->getNBNode());
    } else {
    // !!! self-loop edge?!
    return std::pair<NIVissimConnectionCluster*, NBNode*>(static_cast<NIVissimConnectionCluster*>(0), (*(myConnectionClusters.begin()))->getNBNode());
    }
    */
}


std::pair<NBNode*, NBNode*>
NIVissimEdge::remapOneOfNodes(NBNodeCont& nc,
                              NIVissimDistrictConnection* d,
                              NBNode* fromNode, NBNode* toNode) {
    std::string nid = "ParkingPlace" + toString<int>(d->getID());
    if (d->geomPosition().distanceTo(fromNode->getPosition())
            <
            d->geomPosition().distanceTo(toNode->getPosition())) {

        NBNode* newNode = new NBNode(nid,
                                     fromNode->getPosition(),
                                     SumoXMLNodeType::NOJUNCTION);
        nc.erase(fromNode);
        nc.insert(newNode);
        return std::pair<NBNode*, NBNode*>(newNode, toNode);
    } else {
        NBNode* newNode = new NBNode(nid,
                                     toNode->getPosition(),
                                     SumoXMLNodeType::NOJUNCTION);
        nc.erase(toNode);
        nc.insert(newNode);
        return std::pair<NBNode*, NBNode*>(fromNode, newNode);
    }
}



std::pair<NBNode*, NBNode*>
NIVissimEdge::resolveSameNode(NBNodeCont& nc, double offset,
                              NBNode* prevFrom, NBNode* prevTo) {
    // check whether the edge is connected to a district
    //  use it if so
    NIVissimDistrictConnection* d =
        NIVissimDistrictConnection::dict_findForEdge(myID);
    if (d != nullptr) {
        Position pos = d->geomPosition();
        double position = d->getPosition();
        // the district is at the begin of the edge
        if (myGeom.length() - position > position) {
            std::string nid = "ParkingPlace" + toString<int>(d->getID());
            NBNode* node = nc.retrieve(nid);
            if (node == nullptr) {
                node = new NBNode(nid,
                                  pos, SumoXMLNodeType::NOJUNCTION);
                if (!nc.insert(node)) {
                    throw 1;
                }
            }
            return std::pair<NBNode*, NBNode*>(node, prevTo);
        }
        // the district is at the end of the edge
        else {
            std::string nid = "ParkingPlace" + toString<int>(d->getID());
            NBNode* node = nc.retrieve(nid);
            if (node == nullptr) {
                node = new NBNode(nid, pos, SumoXMLNodeType::NOJUNCTION);
                if (!nc.insert(node)) {
                    throw 1;
                }
            }
            assert(node != 0);
            return std::pair<NBNode*, NBNode*>(prevFrom, node);
        }
    }
    // otherwise, check whether the edge is some kind of
    //  a dead end...
    // check which end is nearer to the node centre
    if (myConnectionClusters.size() == 1) {
        NBNode* node = prevFrom; // it is the same as getToNode()

        NIVissimConnectionCluster* c = *(myConnectionClusters.begin());
        // no end node given
        if (c->around(myGeom.front(), offset) && !c->around(myGeom.back(), offset)) {
            NBNode* end = new NBNode(
                toString<int>(myID) + "-End",
                myGeom.back(),
                SumoXMLNodeType::NOJUNCTION);
            if (!nc.insert(end)) {
                throw 1;
            }
            return std::pair<NBNode*, NBNode*>(node, end);
        }

        // no begin node given
        if (!c->around(myGeom.front(), offset) && c->around(myGeom.back(), offset)) {
            NBNode* beg = new NBNode(
                toString<int>(myID) + "-Begin",
                myGeom.front(),
                SumoXMLNodeType::NOJUNCTION);
            if (!nc.insert(beg)) {
                std::cout << "nope, NIVissimDisturbance" << std::endl;
                throw 1;
            }
            return std::pair<NBNode*, NBNode*>(beg, node);
        }

        // self-loop edge - both points lie within the same cluster
        if (c->around(myGeom.front()) && c->around(myGeom.back())) {
            return std::pair<NBNode*, NBNode*>(node, node);
        }
    }
    // what to do in other cases?
    //  It simply is a self-looping edge....
    return std::pair<NBNode*, NBNode*>(prevFrom, prevTo);
}




void
NIVissimEdge::setNodeCluster(int nodeid) {
    myNode = nodeid;
}


void
NIVissimEdge::buildGeom() {}


void
NIVissimEdge::addIncomingConnection(int id) {
    myIncomingConnections.push_back(id);
}


void
NIVissimEdge::addOutgoingConnection(int id) {
    myOutgoingConnections.push_back(id);
}



void
NIVissimEdge::mergedInto(NIVissimConnectionCluster* old,
                         NIVissimConnectionCluster* act) {
    ConnectionClusters::iterator i =
        std::find(myConnectionClusters.begin(), myConnectionClusters.end(), old);
    if (i != myConnectionClusters.end()) {
        myConnectionClusters.erase(i);
    }
    i = std::find(myConnectionClusters.begin(), myConnectionClusters.end(), act);
    if (i == myConnectionClusters.end()) {
        myConnectionClusters.push_back(act);
    }
}



void
NIVissimEdge::removeFromConnectionCluster(NIVissimConnectionCluster* c) {
    ConnectionClusters::iterator i =
        std::find(myConnectionClusters.begin(), myConnectionClusters.end(), c);
    assert(i != myConnectionClusters.end());
    myConnectionClusters.erase(i);
}


void
NIVissimEdge::addToConnectionCluster(NIVissimConnectionCluster* c) {
    ConnectionClusters::iterator i =
        std::find(myConnectionClusters.begin(), myConnectionClusters.end(), c);
    if (i == myConnectionClusters.end()) {
        myConnectionClusters.push_back(c);
    }
}


Position // !!! reference?
NIVissimEdge::getBegin2D() const {
    return myGeom[0];
}


Position // !!! reference?
NIVissimEdge::getEnd2D() const {
    return myGeom[-1];
}


double
NIVissimEdge::getLength() const {
    return myGeom.length();
}


void
NIVissimEdge::checkDistrictConnectionExistanceAt(double pos) {
    if (find(myDistrictConnections.begin(), myDistrictConnections.end(), pos) == myDistrictConnections.end()) {
        myDistrictConnections.push_back(pos);
        /*        int id = NIVissimConnection::getMaxID() + 1;
                std::vector<int> currentCluster;
                currentCluster.push_back(id);
                myConnectionClusters.push_back(
                    new NIVissimConnectionCluster(currentCluster, -1, myID));*/
    }
}


void
NIVissimEdge::setSpeed(int lane, int speedDist) {
    while ((int)myPatchedSpeeds.size() <= lane) {
        myPatchedSpeeds.push_back(-1);
    }
    myPatchedSpeeds[lane] = speedDist;
}


void
NIVissimEdge::dict_checkEdges2Join() {
    // go through the edges
    for (DictType::iterator i1 = myDict.begin(); i1 != myDict.end(); i1++) {
        // retrieve needed values from the first edge
        NIVissimEdge* e1 = (*i1).second;
        const PositionVector& g1 = e1->getGeometry();
        // check all other edges
        DictType::iterator i2 = i1;
        i2++;
        for (; i2 != myDict.end(); i2++) {
            // retrieve needed values from the second edge
            NIVissimEdge* e2 = (*i2).second;
            const PositionVector& g2 = e2->getGeometry();
            // get the connection description
            NIVissimConnection* c = e1->getConnectionTo(e2);
            if (c == nullptr) {
                c = e2->getConnectionTo(e1);
            }
            // the edge must not be a direct contiuation of the other
            if (c != nullptr) {
                if ((c->getFromEdgeID() == e1->getID() && fabs(c->getFromPosition() - e1->getGeometry().length()) < 5)
                        ||
                        (c->getFromEdgeID() == e2->getID() && fabs(c->getFromPosition() - e2->getGeometry().length()) < 5)) {

                    continue;
                }
            }
            // only parallel edges which do end at the same node
            //  should be joined
            // check for parallelity
            //  !!! the usage of an explicit value is not very fine
            if (fabs(GeomHelper::angleDiff(g1.beginEndAngle(), g2.beginEndAngle())) > DEG2RAD(2.0)) {
                // continue if the lines are not parallel
                continue;
            }

            // check whether the same node is approached
            //  (the distance between the ends should not be too large)
            //  !!! the usage of an explicit value is not very fine
            if (g1.back().distanceTo(g2.back()) > 10) {
                // continue if the lines do not end at the same length
                continue;
            }
            // ok, seem to be different lanes for the same edge
            //  mark as possibly joined later
            e1->addToTreatAsSame(e2);
            e2->addToTreatAsSame(e1);
        }
    }
}


bool
NIVissimEdge::addToTreatAsSame(NIVissimEdge* e) {
    if (e == this) {
        return false;
    }
    // check whether this edge already knows about the other
    if (find(myToTreatAsSame.begin(), myToTreatAsSame.end(), e) == myToTreatAsSame.end()) {
        myToTreatAsSame.push_back(e);
        return true;
    } else {
        return false; // !!! check this
    }
    /*
    //
    std::vector<NIVissimEdge*>::iterator i;
    // add to all other that shall be treated as same
    bool changed = true;
    while (changed) {
        changed = false;
        for (i = myToTreatAsSame.begin(); !changed && i != myToTreatAsSame.end(); i++) {
            changed |= (*i)->addToTreatAsSame(e);
        }
        for (i = myToTreatAsSame.begin(); !changed && i != myToTreatAsSame.end(); i++) {
            changed |= e->addToTreatAsSame(*i);
        }
    }
    */
}

NIVissimConnection*
NIVissimEdge::getConnectionTo(NIVissimEdge* e) {
    std::vector<int>::iterator i;
    for (i = myIncomingConnections.begin(); i != myIncomingConnections.end(); i++) {
        NIVissimConnection* c = NIVissimConnection::dictionary(*i);
        if (c->getFromEdgeID() == e->getID()) {
            return c;
        }
    }
    for (i = myOutgoingConnections.begin(); i != myOutgoingConnections.end(); i++) {
        NIVissimConnection* c = NIVissimConnection::dictionary(*i);
        if (c->getToEdgeID() == e->getID()) {
            return c;
        }
    }
    return nullptr;
}


const std::vector<NIVissimEdge*>&
NIVissimEdge::getToTreatAsSame() const {
    return myToTreatAsSame;
}


void
NIVissimEdge::reportUnsetSpeeds() {
    if (myLanesWithMissingSpeeds.size() == 0) {
        return;
    }
    std::ostringstream str;
    str << "The following lanes have no explicit speed information:\n  ";
    for (std::vector<std::string>::iterator i = myLanesWithMissingSpeeds.begin(); i != myLanesWithMissingSpeeds.end(); ++i) {
        if (i != myLanesWithMissingSpeeds.begin()) {
            str << ", ";
        }
        str << *i;
    }
    WRITE_WARNING(str.str());
}


NIVissimEdge*
NIVissimEdge::getBestIncoming() const {
    // @todo seems as this would have been a hard hack!
    /*
    for (std::vector<int>::const_iterator i = myIncomingConnections.begin(); i != myIncomingConnections.end(); ++i) {
        NIVissimConnection* c = NIVissimConnection::dictionary(*i);
        return NIVissimEdge::dictionary(c->getFromEdgeID());
    }
    return 0;
    */
    if (myIncomingConnections.size() != 0) {
        return NIVissimEdge::dictionary(NIVissimConnection::dictionary(myIncomingConnections.front())->getFromEdgeID());
    }
    return nullptr;
}


NIVissimEdge*
NIVissimEdge::getBestOutgoing() const {
    // @todo seems as this would have been a hard hack!
    /*
    for (std::vector<int>::const_iterator i = myOutgoingConnections.begin(); i != myOutgoingConnections.end(); ++i) {
        NIVissimConnection* c = NIVissimConnection::dictionary(*i);
        return NIVissimEdge::dictionary(c->getToEdgeID());
    }
    return 0;
    */
    if (myOutgoingConnections.size() != 0) {
        return NIVissimEdge::dictionary(NIVissimConnection::dictionary(myOutgoingConnections.front())->getToEdgeID());
    }
    return nullptr;
}


/****************************************************************************/
