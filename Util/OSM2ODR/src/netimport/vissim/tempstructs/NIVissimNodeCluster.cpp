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
/// @file    NIVissimNodeCluster.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>


#include <map>
#include <algorithm>
#include <cassert>
#include <utils/common/VectorHelper.h>
#include <utils/common/ToString.h>
#include <utils/geom/PositionVector.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include "NIVissimTL.h"
#include "NIVissimDisturbance.h"
#include "NIVissimConnection.h"
#include "NIVissimNodeCluster.h"


// ===========================================================================
// static member variables
// ===========================================================================
NIVissimNodeCluster::DictType NIVissimNodeCluster::myDict;
int NIVissimNodeCluster::myCurrentID = 1;


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimNodeCluster::NIVissimNodeCluster(int id, int nodeid, int tlid,
        const std::vector<int>& connectors,
        const std::vector<int>& disturbances,
        bool amEdgeSplitOnly)
    : myID(id), myNodeID(nodeid), myTLID(tlid),
      myConnectors(connectors), myDisturbances(disturbances),
      myNBNode(nullptr), myAmEdgeSplit(amEdgeSplitOnly) {}


NIVissimNodeCluster::~NIVissimNodeCluster() {}




bool
NIVissimNodeCluster::dictionary(int id, NIVissimNodeCluster* o) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = o;
        return true;
    }
    assert(false);
    return false;
}


int
NIVissimNodeCluster::dictionary(int nodeid, int tlid,
                                const std::vector<int>& connectors,
                                const std::vector<int>& disturbances,
                                bool amEdgeSplitOnly) {
    int id = nodeid;
    if (nodeid < 0) {
        id = myCurrentID++;
    }
    NIVissimNodeCluster* o = new NIVissimNodeCluster(id,
            nodeid, tlid, connectors, disturbances, amEdgeSplitOnly);
    dictionary(id, o);
    return id;
}


NIVissimNodeCluster*
NIVissimNodeCluster::dictionary(int id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}



int
NIVissimNodeCluster::contSize() {
    return (int)myDict.size();
}



std::string
NIVissimNodeCluster::getNodeName() const {
    if (myTLID == -1) {
        return toString<int>(myID);
    } else {
        return toString<int>(myID) + "LSA " + toString<int>(myTLID);
    }
}


void
NIVissimNodeCluster::buildNBNode(NBNodeCont& nc) {
    if (myConnectors.size() == 0) {
        return; // !!! Check, whether this can happen
    }

    // compute the position
    PositionVector crossings;
    std::vector<int>::iterator i, j;
    // check whether this is a split of an edge only
    if (myAmEdgeSplit) {
// !!! should be        assert(myTLID==-1);
        for (i = myConnectors.begin(); i != myConnectors.end(); i++) {
            NIVissimConnection* c1 = NIVissimConnection::dictionary(*i);
            crossings.push_back_noDoublePos(c1->getFromGeomPosition());
        }
    } else {
        // compute the places the connections cross
        for (i = myConnectors.begin(); i != myConnectors.end(); i++) {
            NIVissimAbstractEdge* c1 = NIVissimAbstractEdge::dictionary(*i);
            c1->buildGeom();
            for (j = i + 1; j != myConnectors.end(); j++) {
                NIVissimAbstractEdge* c2 = NIVissimAbstractEdge::dictionary(*j);
                c2->buildGeom();
                if (c1->crossesEdge(c2)) {
                    crossings.push_back_noDoublePos(c1->crossesEdgeAtPoint(c2));
                }
            }
        }
        // alternative way: compute via positions of crossings
        if (crossings.size() == 0) {
            for (i = myConnectors.begin(); i != myConnectors.end(); i++) {
                NIVissimConnection* c1 = NIVissimConnection::dictionary(*i);
                crossings.push_back_noDoublePos(c1->getFromGeomPosition());
                crossings.push_back_noDoublePos(c1->getToGeomPosition());
            }
        }
    }
    // get the position (center)
    Position pos = crossings.getPolygonCenter();
    // build the node
    /*    if(myTLID!=-1) {
     !!!        NIVissimTL *tl = NIVissimTL::dictionary(myTLID);
            if(tl->getType()=="festzeit") {
                node = new NBNode(getNodeName(), pos.x(), pos.y(),
                    "traffic_light");
            } else {
                node = new NBNode(getNodeName(), pos.x(), pos.y(),
                    "actuated_traffic_light");
            }
        }*/
    NBNode* node = new NBNode(getNodeName(), pos, SumoXMLNodeType::PRIORITY);
    if (!nc.insert(node)) {
        delete node;
        throw 1;
    }
    myNBNode = node;
}


void
NIVissimNodeCluster::buildNBNodes(NBNodeCont& nc) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        (*i).second->buildNBNode(nc);
    }
}



void
NIVissimNodeCluster::dict_recheckEdgeChanges() {
    return;
}


int
NIVissimNodeCluster::getFromNode(int edgeid) {
    int ret = -1;
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimNodeCluster* c = (*i).second;
        for (std::vector<int>::iterator j = c->myConnectors.begin(); j != c->myConnectors.end(); j++) {
            NIVissimConnection* conn = NIVissimConnection::dictionary(*j);
            if (conn != nullptr && conn->getToEdgeID() == edgeid) {
//                return (*i).first;
                if (ret != -1 && (*i).first != ret) {
//                     "NIVissimNodeCluster:DoubleNode:" << ret << endl;
                    throw 1; // an edge should not outgo from two different nodes
// but actually, a joined cluster may posess a connections more than once
                }
                ret = (*i).first;
            }
        }
    }
    return ret;
}


int
NIVissimNodeCluster::getToNode(int edgeid) {
    int ret = -1;
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimNodeCluster* c = (*i).second;
        for (std::vector<int>::iterator j = c->myConnectors.begin(); j != c->myConnectors.end(); j++) {
            NIVissimConnection* conn = NIVissimConnection::dictionary(*j);
            if (conn != nullptr && conn->getFromEdgeID() == edgeid) {
//                return (*i).first;
                if (ret != -1 && ret != (*i).first) {
//                  << "NIVissimNodeCluster: multiple to-nodes" << endl;
                    throw 1; // an edge should not outgo from two different nodes
// but actually, a joined cluster may posess a connections more than once

                }
                ret = (*i).first;
            }
        }
    }
    return ret;
}


void
NIVissimNodeCluster::_debugOut(std::ostream& into) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimNodeCluster* c = (*i).second;
        into << std::endl << c->myID << ":";
        for (std::vector<int>::iterator j = c->myConnectors.begin(); j != c->myConnectors.end(); j++) {
            if (j != c->myConnectors.begin()) {
                into << ", ";
            }
            into << (*j);
        }
    }
    into << "=======================" << std::endl;
}



NBNode*
NIVissimNodeCluster::getNBNode() const {
    return myNBNode;
}


Position
NIVissimNodeCluster::getPos() const {
    return myPosition;
}


void
NIVissimNodeCluster::dict_addDisturbances(NBDistrictCont& dc,
        NBNodeCont& nc, NBEdgeCont& ec) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        const std::vector<int>& disturbances = (*i).second->myDisturbances;
        NBNode* node = nc.retrieve((*i).second->getNodeName());
        for (std::vector<int>::const_iterator j = disturbances.begin(); j != disturbances.end(); j++) {
            NIVissimDisturbance* disturbance = NIVissimDisturbance::dictionary(*j);
            disturbance->addToNode(node, dc, nc, ec);
        }
    }
    NIVissimDisturbance::reportRefused();
}


void
NIVissimNodeCluster::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


void
NIVissimNodeCluster::setCurrentVirtID(int id) {
    myCurrentID = id;
}


/****************************************************************************/
