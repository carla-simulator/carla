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
/// @file    NIVissimConnectionCluster.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>


#include <iostream>
#include <vector>
#include <utils/geom/Boundary.h>
#include <utils/geom/GeomHelper.h>
#include "NIVissimConnection.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NIVissimEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIVissimConnectionCluster
 * This class holds a list of connections either all outgoing or all
 * incoming within an edge, which do lies close together.
 * This shall be the connections which belong to a single node.
 * It still are not all of the node's connections, as other edges
 * may participate to this node, too.
 */
class NIVissimConnectionCluster {
public:
    /** @brief Constructor
        Build the boundary; The boundary includes both incoming and outgoing nodes */
    NIVissimConnectionCluster(const std::vector<int>& connections, int nodeCluster,
                              int edgeid);

    NIVissimConnectionCluster(const std::vector<int>& connections,
                              const Boundary& boundary, int nodeCluster, const std::vector<int>& edges);

    /// Destructor
    ~NIVissimConnectionCluster();

    /// Returns the information whether the given cluster overlaps the current
    bool overlapsWith(NIVissimConnectionCluster* c, double offset = 0) const;

    bool hasNodeCluster() const;

    NBNode* getNBNode() const;

    bool around(const Position& p, double offset = 0) const;

    double getPositionForEdge(int edgeid) const;

    friend class NIVissimEdge; // !!! debug

    const std::vector<int>& getConnections() const {
        return myConnections;
    }

    PositionVector getIncomingContinuationGeometry(NIVissimEdge* e) const;
    PositionVector getOutgoingContinuationGeometry(NIVissimEdge* e) const;
    NIVissimConnection* getIncomingContinuation(NIVissimEdge* e) const;
    NIVissimConnection* getOutgoingContinuation(NIVissimEdge* e) const;


public:
    /** @brief Tries to joind clusters participating within a node
        This is done by joining clusters which overlap */
    static void joinBySameEdges(double offset);

    static void joinByDisturbances(double offset);

    static void buildNodeClusters();

    static void _debugOut(std::ostream& into);

    static int dictSize();


    static int getNextFreeNodeID();

    static void clearDict();

private:
    class NodeSubCluster {
    public:
        NodeSubCluster(NIVissimConnection* c);
        ~NodeSubCluster();
        void add(NIVissimConnection* c);
        void add(const NodeSubCluster& c);
        int size() const;
        bool overlapsWith(const NodeSubCluster& c, double offset = 0);
        std::vector<int> getConnectionIDs() const;
        friend class NIVissimConnectionCluster;
    public:
        Boundary myBoundary;
        typedef std::vector<NIVissimConnection*> ConnectionCont;
        ConnectionCont myConnections;
    };

    class same_direction_sorter {
    private:
        double myAngle;

    public:
        /// constructor
        explicit same_direction_sorter(double angle)
            : myAngle(angle) { }

    public:
        /// comparing operation
        int operator()(NIVissimConnection* c1, NIVissimConnection* c2) const {
            return fabs(GeomHelper::angleDiff(c1->getGeometry().beginEndAngle(), myAngle))
                   <
                   fabs(GeomHelper::angleDiff(c2->getGeometry().beginEndAngle(), myAngle));
        }
    };



private:
    /// Adds the second cluster
    void add(NIVissimConnectionCluster* c);

    void removeConnections(const NodeSubCluster& c);

    void recomputeBoundary();

    void recheckEdges();

    bool joinable(NIVissimConnectionCluster* c2, double offset);


    std::vector<int> getDisturbanceParticipators();

    std::vector<int> extendByToTreatAsSame(const std::vector<int>& iv1,
                                           const std::vector<int>& iv2) const;

    bool isWeakDistrictConnRealisation(NIVissimConnectionCluster* c2);

    bool liesOnSameEdgesEnd(NIVissimConnectionCluster* cc2);



private:
    /// List of connection-ids which participate within this cluster
    std::vector<int> myConnections;

    /// The boundary of the cluster
    Boundary myBoundary;

    /// The node the cluster is assigned to
    int myNodeCluster;

    // The edge which holds the cluster
    std::vector<int> myEdges;

    std::vector<int> myNodes;

    std::vector<int> myTLs;

    std::vector<int> myOutgoingEdges, myIncomingEdges;

private:
    typedef std::vector<NIVissimConnectionCluster*> ContType;
    static ContType myClusters;
    static int myFirstFreeID;
    static int myStaticBlaID;
    int myBlaID;
};
