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
/// @file    NIVissimConnection.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>

#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <utils/common/VectorHelper.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include "NIVissimExtendedEdgePoint.h"
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/GeomHelper.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBEdgeCont.h>
#include "NIVissimEdge.h"
#include "NIVissimClosedLanesVector.h"
#include "NIVissimNodeDef.h"
#include "NIVissimConnection.h"
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// static members
// ===========================================================================
NIVissimConnection::DictType NIVissimConnection::myDict;
int NIVissimConnection::myMaxID;


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimConnection::NIVissimConnection(int id,
                                       const std::string& name, const NIVissimExtendedEdgePoint& from_def,
                                       const NIVissimExtendedEdgePoint& to_def,
                                       const PositionVector& geom,
                                       const std::vector<int>& assignedVehicles, const NIVissimClosedLanesVector& clv)
    : NIVissimAbstractEdge(id, geom),
      myName(name), myFromDef(from_def), myToDef(to_def),
      myAssignedVehicles(assignedVehicles), myClosedLanes(clv) {}


NIVissimConnection::~NIVissimConnection() {
    for (NIVissimClosedLanesVector::iterator i = myClosedLanes.begin(); i != myClosedLanes.end(); i++) {
        delete (*i);
    }
    myClosedLanes.clear();
}


bool
NIVissimConnection::dictionary(int id, NIVissimConnection* o) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = o;
        return true;
    }
    return false;
}



NIVissimConnection*
NIVissimConnection::dictionary(int id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}


void
NIVissimConnection::buildNodeClusters() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimConnection* e = (*i).second;
        if (!e->clustered()) {
            assert(e->myBoundary != 0 && e->myBoundary->xmax() > e->myBoundary->xmin());
            std::vector<int> connections =
                NIVissimConnection::getWithin(*(e->myBoundary));
            NIVissimNodeCluster::dictionary(-1, -1, connections,
                                            std::vector<int>(), true); // 19.5.!!! should be on a single edge
        }
    }
}





std::vector<int>
NIVissimConnection::getWithin(const AbstractPoly& poly) {
    std::vector<int> ret;
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        if ((*i).second->crosses(poly)) {
            ret.push_back((*i).second->myID);
        }
    }
    return ret;
}


void
NIVissimConnection::computeBounding() {
    Boundary* bound = new Boundary();
    bound->add(myFromDef.getGeomPosition());
    bound->add(myToDef.getGeomPosition());
    assert(myBoundary == 0);
    myBoundary = bound;
}


std::vector<int>
NIVissimConnection::getForEdge(int edgeid, bool /*omitNodeAssigned*/) {
    std::vector<int> ret;
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        int connID = (*i).first;
        if ((*i).second->myFromDef.getEdgeID() == edgeid
                ||
                (*i).second->myToDef.getEdgeID() == edgeid) {
            if (!(*i).second->hasNodeCluster()) {
                ret.push_back(connID);
            }
        }
    }
    return ret;
}


int
NIVissimConnection::getFromEdgeID() const {
    return myFromDef.getEdgeID();
}


int
NIVissimConnection::getToEdgeID() const {
    return myToDef.getEdgeID();
}


double
NIVissimConnection::getFromPosition() const {
    return myFromDef.getPosition();
}


double
NIVissimConnection::getToPosition() const {
    return myToDef.getPosition();
}


Position
NIVissimConnection::getFromGeomPosition() const {
    return myFromDef.getGeomPosition();
}



Position
NIVissimConnection::getToGeomPosition() const {
    return myToDef.getGeomPosition();
}


void
NIVissimConnection::setNodeCluster(int nodeid) {
    assert(myNode == -1);
    myNode = nodeid;
}


void
NIVissimConnection::buildGeom() {
    if (myGeom.size() > 0) {
        return;
    }
    myGeom.push_back(myFromDef.getGeomPosition());
    myGeom.push_back(myToDef.getGeomPosition());
}


int
NIVissimConnection::buildEdgeConnections(NBEdgeCont& ec) {
    int unsetConnections = 0;
    // try to determine the connected edges
    NBEdge* fromEdge = nullptr;
    NBEdge* toEdge = nullptr;
    NIVissimEdge* vissimFrom = NIVissimEdge::dictionary(getFromEdgeID());
    if (vissimFrom->wasWithinAJunction()) {
        // this edge was not built, try to get one that approaches it
        vissimFrom = vissimFrom->getBestIncoming();
        if (vissimFrom != nullptr) {
            fromEdge = ec.retrievePossiblySplit(toString(vissimFrom->getID()), toString(getFromEdgeID()), true);
        }
    } else {
        // this edge was built, try to get the proper part
        fromEdge = ec.retrievePossiblySplit(toString(getFromEdgeID()), toString(getToEdgeID()), true);
    }
    NIVissimEdge* vissimTo = NIVissimEdge::dictionary(getToEdgeID());
    if (vissimTo->wasWithinAJunction()) {
        vissimTo = vissimTo->getBestOutgoing();
        if (vissimTo != nullptr) {
            toEdge = ec.retrievePossiblySplit(toString(vissimTo->getID()), toString(getToEdgeID()), true);
        }
    } else {
        toEdge = ec.retrievePossiblySplit(toString(getToEdgeID()), toString(getFromEdgeID()), false);
    }

    // try to get the edges the current connection connects
    /*
    NBEdge *fromEdge = ec.retrievePossiblySplit(toString(getFromEdgeID()), toString(getToEdgeID()), true);
    NBEdge *toEdge = ec.retrievePossiblySplit(toString(getToEdgeID()), toString(getFromEdgeID()), false);
    */
    if (fromEdge == nullptr || toEdge == nullptr) {
        WRITE_WARNING("Could not build connection between '" + toString(getFromEdgeID()) + "' and '" + toString(getToEdgeID()) + "'.");
        return 1; // !!! actually not 1
    }
    recheckLanes(fromEdge, toEdge);
    const std::vector<int>& fromLanes = getFromLanes();
    const std::vector<int>& toLanes = getToLanes();
    if (fromLanes.size() != toLanes.size()) {
        WRITE_WARNING("Lane sizes differ for connection '" + toString(getID()) + "'.");
    } else {
        for (int index = 0; index < (int)fromLanes.size(); ++index) {
            if (fromEdge->getNumLanes() <= fromLanes[index]) {
                WRITE_WARNING("Could not set connection between '" + fromEdge->getID() + "_" + toString(fromLanes[index]) + "' and '" + toEdge->getID() + "_" + toString(toLanes[index]) + "'.");
                ++unsetConnections;
            } else if (!fromEdge->addLane2LaneConnection(fromLanes[index], toEdge, toLanes[index], NBEdge::Lane2LaneInfoType::VALIDATED, true)) {
                WRITE_WARNING("Could not set connection between '" + fromEdge->getID() + "_" + toString(fromLanes[index]) + "' and '" + toEdge->getID() + "_" + toString(toLanes[index]) + "'.");
                ++unsetConnections;
            }
        }
    }
    return unsetConnections;
}


void
NIVissimConnection::dict_buildNBEdgeConnections(NBEdgeCont& ec) {
    int unsetConnections = 0;
    // go through connections
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        unsetConnections += (*i).second->buildEdgeConnections(ec);
    }
    if (unsetConnections != 0) {
        WRITE_WARNING(toString<int>(unsetConnections) + " of " + toString<int>((int)myDict.size()) + " connections could not be assigned.");
    }
}


const std::vector<int>&
NIVissimConnection::getFromLanes() const {
    return myFromDef.getLanes();
}


const std::vector<int>&
NIVissimConnection::getToLanes() const {
    return myToDef.getLanes();
}


void
NIVissimConnection::recheckLanes(const NBEdge* const fromEdge, const NBEdge* const toEdge) {
    myFromDef.recheckLanes(fromEdge);
    myToDef.recheckLanes(toEdge);
}


const Boundary&
NIVissimConnection::getBoundingBox() const {
    assert(myBoundary != 0 && myBoundary->xmax() >= myBoundary->xmin());
    return *myBoundary;
}


void
NIVissimConnection::dict_assignToEdges() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimConnection* c = (*i).second;
        NIVissimEdge::dictionary(c->getFromEdgeID())->addOutgoingConnection((*i).first);
        NIVissimEdge::dictionary(c->getToEdgeID())->addIncomingConnection((*i).first);
    }
}


int
NIVissimConnection::getMaxID() {
    return myMaxID;
}


/****************************************************************************/
