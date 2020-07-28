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
/// @file    NBOwnTLDef.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @date    Tue, 29.05.2005
///
// A traffic light logics which must be computed (only nodes/edges are given)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "NBTrafficLightDefinition.h"
#include "NBNode.h"


// ===========================================================================
// class declarations
// ===========================================================================


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBOwnTLDef
 * @brief A traffic light logics which must be computed (only nodes/edges are given)
 */
class NBOwnTLDef : public NBTrafficLightDefinition {
public:
    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] junctions Junctions controlled by this tls
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBOwnTLDef(const std::string& id,
               const std::vector<NBNode*>& junctions,
               SUMOTime offset,
               TrafficLightType type);


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] junction The junction controlled by this tls
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBOwnTLDef(const std::string& id, NBNode* junction, SUMOTime offset,
               TrafficLightType type);


    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] offset The offset of the plan
     * @param[in] type The algorithm type for the computed traffic light
     */
    NBOwnTLDef(const std::string& id, SUMOTime offset, TrafficLightType type);


    /// @brief Destructor
    ~NBOwnTLDef();


    /// @name Public methods from NBTrafficLightDefinition-interface
    /// @{

    /** @brief Replaces occurences of the removed edge in incoming/outgoing edges of all definitions
     * @param[in] removed The removed edge
     * @param[in] incoming The edges to use instead if an incoming edge was removed
     * @param[in] outgoing The edges to use instead if an outgoing edge was removed
     * @see NBTrafficLightDefinition::remapRemoved
     */
    void remapRemoved(NBEdge* removed,
                      const EdgeVector& incoming, const EdgeVector& outgoing);


    /** @brief Informs edges about being controlled by a tls
     * @see NBTrafficLightDefinition::setTLControllingInformation
     */
    void setTLControllingInformation() const;
    /// @}


    /** @brief Forces the definition not to compute an additional phase for left-movers
     */
    void setSinglePhase() {
        myHaveSinglePhase = true;
    }

    /// @brief add an additional pedestrian phase if there are crossings that did not get green yet
    static void addPedestrianScramble(NBTrafficLightLogic* logic, int noLinksAll,
                                      SUMOTime greenTime, SUMOTime yellowTime,
                                      const std::vector<NBNode::Crossing*>& crossings, const EdgeVector& fromEdges, const EdgeVector& toEdges);

    /// @brief add 1 or 2 phases depending on the presence of pedestrian crossings
    static std::string addPedestrianPhases(NBTrafficLightLogic* logic, SUMOTime greenTime,
                                           SUMOTime minDur, SUMOTime maxDur,
                                           std::string state, const std::vector<NBNode::Crossing*>& crossings, const EdgeVector& fromEdges, const EdgeVector& toEdges);

    /// @brief compute phase state in regard to pedestrian crossings
    static std::string patchStateForCrossings(const std::string& state,
            const std::vector<NBNode::Crossing*>& crossings, const EdgeVector& fromEdges, const EdgeVector& toEdges);

    /** @brief helper function for myCompute
     * @param[in] brakingTime Duration a vehicle needs for braking in front of the tls
     * @param[in] onlyConts whether the method is only called to compute myNeedsContRelation
     * @return The computed logic
     */
    NBTrafficLightLogic* computeLogicAndConts(int brakingTimeSeconds, bool onlyConts = false);

    /* initialize myNeedsContRelation and set myNeedsContRelationReady to true */
    void initNeedsContRelation() const;

    /* build optional all-red phase */
    void buildAllRedState(SUMOTime allRedTime, NBTrafficLightLogic* logic, const std::string& state);

    ///@brief Returns the maximum index controlled by this traffic light
    int getMaxIndex();

protected:
    /// @name Protected methods from NBTrafficLightDefinition-interface
    /// @{

    /** @brief Computes the traffic light logic finally in dependence to the type
     * @param[in] brakingTime Duration a vehicle needs for braking in front of the tls
     * @return The computed logic
     * @see NBTrafficLightDefinition::myCompute
     */
    NBTrafficLightLogic* myCompute(int brakingTimeSeconds);


    /** @brief Collects the links participating in this traffic light
     * @exception ProcessError If a link could not be found
     * @see NBTrafficLightDefinition::collectLinks
     */
    void collectLinks();


    /** @brief Replaces a removed edge/lane
     * @param[in] removed The edge to replace
     * @param[in] removedLane The lane of this edge to replace
     * @param[in] by The edge to insert instead
     * @param[in] byLane This edge's lane to insert instead
     * @see NBTrafficLightDefinition::replaceRemoved
     */
    void replaceRemoved(NBEdge* removed, int removedLane,
                        NBEdge* by, int byLane, bool incoming);
    /// @}


protected:

    /// @brief test whether a joined tls with layout 'opposites' would be built without dedicated left-turn phase
    bool corridorLike() const;

    /** @brief Returns the weight of a stream given its direction
     * @param[in] dir The direction of the stream
     * @return This stream's weight
     * @todo There are several magic numbers; describe
     */
    double getDirectionalWeight(LinkDirection dir);


    /** @brief Returns this edge's priority at the node it ends at
     * @param[in] e The edge to ask for his priority
     * @return The edge's priority at his destination node
     */
    int getToPrio(const NBEdge* const e);


    /** @brief Returns how many streams outgoing from the edges can pass the junction without being blocked
     * @param[in] e1 The first edge
     * @param[in] e2 The second edge
     * @todo There are several magic numbers; describe
     */
    double computeUnblockedWeightedStreamNumber(const NBEdge* const e1, const NBEdge* const e2);


    /** @brief Returns the combination of two edges from the given which has most unblocked streams
     * @param[in] edges The list of edges to include in the computation
     * @return The two edges for which the weighted number of unblocked streams is the highest
     */
    std::pair<NBEdge*, NBEdge*> getBestCombination(const EdgeVector& edges);


    /** @brief Returns the combination of two edges from the given which has most unblocked streams
     *
     * The chosen edges are removed from the given vector
     *
     * @param[in, changed] incoming The list of edges which are participating in the logic
     * @return The two edges for which the weighted number of unblocked streams is the highest
     */
    std::pair<NBEdge*, NBEdge*> getBestPair(EdgeVector& incoming);


    /// @brief compute whether the given connection is crossed by pedestrians
    static bool hasCrossing(const NBEdge* from, const NBEdge* to, const std::vector<NBNode::Crossing*>& crossings);

    /// @brief get edges that have connections
    static EdgeVector getConnectedOuterEdges(const EdgeVector& incoming);


    /// @brief allow connections that are compatible with the chosen edges
    std::string allowCompatible(std::string state, const EdgeVector& fromEdges, const EdgeVector& toEdges,
                                const std::vector<int>& fromLanes, const std::vector<int>& toLanes);

    std::string allowSingleEdge(std::string state, const EdgeVector& fromEdges);

    std::string allowFollowers(std::string state, const EdgeVector& fromEdges, const EdgeVector& toEdges);

    std::string allowPredecessors(std::string state, const EdgeVector& fromEdges, const EdgeVector& toEdges,
                                  const std::vector<int>& fromLanes, const std::vector<int>& toLanes);

    std::string allowUnrelated(std::string state, const EdgeVector& fromEdges, const EdgeVector& toEdges,
                               const std::vector<bool>& isTurnaround,
                               const std::vector<NBNode::Crossing*>& crossings);

    std::string allowByVClass(std::string state, const EdgeVector& fromEdges, const EdgeVector& toEdges, SVCPermissions perm);

    /// @brief whether the given index is forbidden by a green link in the current state
    bool forbidden(const std::string& state, int index, const EdgeVector& fromEdges, const EdgeVector& toEdges);

    /** @brief change 'G' to 'g' for conflicting connections
     * @param[in] state
     * @param[in] fromEdges
     * @param[in] toEdges
     * @param[in] isTurnaround
     * @param[in] fromLanes
     * @param[in] hadGreenMajor
     * @param[out] haveForbiddenLeftMover
     * @param[out] rightTurnConflicts
     * @return The corrected state
     */
    std::string correctConflicting(std::string state, const EdgeVector& fromEdges, const EdgeVector& toEdges,
                                   const std::vector<bool>& isTurnaround,
                                   const std::vector<int>& fromLanes,
                                   const std::vector<bool>& hadGreenMajor,
                                   bool& haveForbiddenLeftMover, std::vector<bool>& rightTurnConflicts);

    /// @brief fix states in regard to custom crossing indices
    void checkCustomCrossingIndices(NBTrafficLightLogic* logic) const;

    /** @class edge_by_incoming_priority_sorter
     * @brief Sorts edges by their priority within the node they end at
     */
    class edge_by_incoming_priority_sorter {
    public:
        /** @brief comparing operator
         * @param[in] e1 an edge
         * @param[in] e2 an edge
         */
        int operator()(const NBEdge* const e1, const NBEdge* const e2) const {
            if (e1->getJunctionPriority(e1->getToNode()) != e2->getJunctionPriority(e2->getToNode())) {
                return e1->getJunctionPriority(e1->getToNode()) > e2->getJunctionPriority(e2->getToNode());
            }
            return e1->getID() > e2->getID();
        }
    };


private:
    /// @brief Whether left-mover should not have an additional phase
    bool myHaveSinglePhase;

};
