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
/// @file    NIVissimEdge.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    End of 2002
///
// A temporary storage for edges imported from Vissim
/****************************************************************************/
#pragma once
#include <config.h>

#include "NIVissimConnectionCluster.h"
#include <vector>
#include <string>
#include <map>
#include <netbuild/NBEdge.h>
#include <utils/geom/PositionVector.h>
#include <utils/common/UtilExceptions.h>
#include "NIVissimAbstractEdge.h"
#include "NIVissimClosedLanesVector.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NBDistrictCont;
class NIVissimDistrictConnection;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIVissimEdge
 * @brief A temporary storage for edges imported from Vissim
 */
class NIVissimEdge
    : public NIVissimAbstractEdge {
public:
    /// Constructor
    NIVissimEdge(int id, const std::string& name,
                 const std::string& type,
                 std::vector<double> laneWidths,
                 double zuschlag1,
                 double zuschlag2, double length,
                 const PositionVector& geom,
                 const NIVissimClosedLanesVector& clv);

    /// Destructor
    ~NIVissimEdge();

    void setNodeCluster(int nodeid);
    void buildGeom();

    /// Adds a connection where this edge is the destination
    void addIncomingConnection(int id);

    /// Adds a connection where this edge is the source
    void addOutgoingConnection(int id);

    /** @brief Returns the node at the given position
        As this may be ambigous, a second node not to return may be supplied */
    NBNode* getNodeAt(const Position& p, NBNode* other = 0);

    /** Returns the begin position of the edge */
    Position getBegin2D() const;

    /// Returns the end position of the edge
    Position getEnd2D() const;

    /// Returns the length of the node
    double getLength() const;

    void checkDistrictConnectionExistanceAt(double pos);

    void mergedInto(NIVissimConnectionCluster* old,
                    NIVissimConnectionCluster* act);

    void removeFromConnectionCluster(NIVissimConnectionCluster* c);
    void addToConnectionCluster(NIVissimConnectionCluster* c);
    void setSpeed(int lane, int speedDist);
    bool addToTreatAsSame(NIVissimEdge* e);

    NIVissimConnection* getConnectionTo(NIVissimEdge* e);
    const std::vector<NIVissimEdge*>& getToTreatAsSame() const;


    /** @brief Returns whether this edge was found to be within a junction
     * @return Whether this node is assumed to be within a junction
     */
    bool wasWithinAJunction() const {
        return myAmWithinJunction;
    }

    NIVissimEdge* getBestIncoming() const;
    NIVissimEdge* getBestOutgoing() const;

    friend class NIVissimNodeDef_Edges;
    friend class NIVissimNodeDef_Poly;

public:
    /** @brief Adds the described item to the dictionary
        Builds the edge first */
    static bool dictionary(int id, const std::string& name,
                           const std::string& type, int noLanes, double zuschlag1,
                           double zuschlag2, double length,
                           const PositionVector& geom,
                           const NIVissimClosedLanesVector& clv);

    /// Adds the edge to the dictionary
    static bool dictionary(int id, NIVissimEdge* o);

    /// Returns the named edge from the dictionary
    static NIVissimEdge* dictionary(int id);

    /** @brief Clusters connections of each edge
     *
     * For every edge stored in this container, its connections are collected and
     *  joined into "clusters" if they have the same "direction" (incoming/outgoing)
     *  and are not further than 10m away from each other.
     *
     * @todo Probably the distance (MAX_CLUSTER_DISTANCE=10m) should be made variable
     */
    static void buildConnectionClusters();

    /// Builds NBEdges from the VissimEdges within the dictionary
    static void dict_buildNBEdges(NBDistrictCont& dc, NBNodeCont& nc,
                                  NBEdgeCont& ec, double offset);

    static void dict_propagateSpeeds();

    static void dict_checkEdges2Join();


    /** @brief Writes edges with unset speeds to the warnings message log instance
     *
     * Vissim has no direct speed definition of edges; still, we try to propagate
     *  speed changes along the streets. If a lane is not covered by such, its id
     *  is put into the static container "myLanesWithMissingSpeeds".
     * If the option "vissim.report-unset-speeds" is set, all lane ids stored within
     *  this container are written.
     */
    static void reportUnsetSpeeds();


private:
    /// The definition for a container for connection clusters
    typedef std::vector<NIVissimConnectionCluster*> ConnectionClusters;

private:
    /** @brief Builds the NBEdge from this VissimEdge
     *
     * @param[in] dc The district container used if this edge must be split
     * @param[in] nc The node container used for (optionally) building this edge's nodes
     * @param[in] ec The edge control to add this edge to
     * @param[in] sameNodesOffset Offset used to discriminate nodes
     * @exception ProcessError If one of the built nodes or edges could not be added to the according container
     */
    void buildNBEdge(NBDistrictCont& dc, NBNodeCont& nc,
                     NBEdgeCont& ec, double sameNodesOffset);

    /// Returns the origin node
    std::pair<NIVissimConnectionCluster*, NBNode*>
    getFromNode(NBNodeCont& nc, ConnectionClusters& clusters);

    /// Returns the destination node
    std::pair<NIVissimConnectionCluster*, NBNode*>
    getToNode(NBNodeCont& nc, ConnectionClusters& clusters);

    /// Tries to resolve the problem that the same node has been returned as origin and destination node
    std::pair<NBNode*, NBNode*> resolveSameNode(NBNodeCont& nc,
            double offset, NBNode* prevFrom, NBNode* prevTo);

//    double recheckSpeedPatches();

    std::vector<NIVissimConnection*> getOutgoingConnected(int lane) const;

    void propagateSpeed(double speed, std::vector<int> forLanes);


    void setDistrictSpeed();
    double getRealSpeed(int distNo);
    void checkUnconnectedLaneSpeeds();
    void propagateOwn();



private:
    static NBNode* getNodeSecure(int nodeid, const Position& pos,
                                 const std::string& possibleName);

    std::pair<NBNode*, NBNode*>
    remapOneOfNodes(NBNodeCont& nc,
                    NIVissimDistrictConnection* d,
                    NBNode* fromNode, NBNode* toNode);

private:
    /**
     * Sorts connections the edge participates in by their position along
     * the given edge
     */
    class connection_position_sorter {
    public:
        /// constructor
        explicit connection_position_sorter(int edgeid);

        /// comparing operation
        int operator()(int c1id, int c2id) const;

    private:
        /// The id of the edge
        int myEdgeID;

    };


    /**
     * Sorts connection clusters the edge participates in by their
     * position along the given edge
     */
    class connection_cluster_position_sorter {
    public:
        /// constructor
        explicit connection_cluster_position_sorter(int edgeid);

        /// comparing operation
        int operator()(NIVissimConnectionCluster* cc1,
                       NIVissimConnectionCluster* cc2) const;

    private:
        /// The id of the edge
        int myEdgeID;

    };

private:
    /// The name of the edge
    std::string myName;

    /// The type of the edge
    std::string myType;

    /// The number of lanes the edge has
    int myNoLanes;
    std::vector<double> myLaneWidths;

    /// Additional load values for this edge
    double myZuschlag1, myZuschlag2;

    /// List of lanes closed on this edge
    NIVissimClosedLanesVector myClosedLanes;

    /// List of connection clusters along this edge
    ConnectionClusters myConnectionClusters;

    /// List of connections incoming to this edge
    std::vector<int> myIncomingConnections;

    /// List of connections outgoing from this edge
    std::vector<int> myOutgoingConnections;

    std::vector<double> myDistrictConnections;

    std::vector<int> myPatchedSpeeds;

    std::vector<double> myLaneSpeeds;

    std::vector<NIVissimEdge*> myToTreatAsSame;

    /// @brief Information whether this edge was not build due to being within a junction
    bool myAmWithinJunction;

private:
    /// @brief Definition of the dictionary type
    typedef std::map<int, NIVissimEdge*> DictType;

    /// @brief The dictionary
    static DictType myDict;

    /// @brief The current maximum id; needed for further id assignment
    static int myMaxID;

    static std::vector<std::string> myLanesWithMissingSpeeds;

};
