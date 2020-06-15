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
/// @file    NBLoadedSUMOTLDef.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mar 2011
///
// A complete traffic light logic loaded from a sumo-net. (opted to reimplement
// since NBLoadedTLDef is quite vissim specific)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <set>
#include "NBNode.h"
#include "NBEdgeCont.h"
#include "NBTrafficLightDefinition.h"
#include "NBTrafficLightLogic.h"
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBLoadedSUMOTLDef
 * @brief A loaded (complete) traffic light logic
 */
class NBLoadedSUMOTLDef : public NBTrafficLightDefinition {
public:

    /** @brief Constructor
     * @param[in] id The id of the tls
     * @param[in] programID The programID for the computed logic
     * @param[in] offset The offset for the computed logic
     * @param[in] type The algorithm type for the computed logic
     */
    NBLoadedSUMOTLDef(const std::string& id, const std::string& programID, SUMOTime offset, TrafficLightType type);

    /** @brief Constructor that copies from an existing definition and its computed logic (used by NETEDIT)
     * @param[in] def The definition to copy
     * @param[in] logic The computed logic of the given def
     */
    NBLoadedSUMOTLDef(const NBTrafficLightDefinition& def, const NBTrafficLightLogic& logic);


    /// @brief Destructor
    ~NBLoadedSUMOTLDef();

    /** @brief Sets the programID
     * @param[in] programID The new ID of the program (subID)
     */
    void setProgramID(const std::string& programID);

    /** @brief Informs edges about being controlled by a tls
     */
    void setTLControllingInformation() const;

    /** @brief Replaces occurences of the removed edge in incoming/outgoing edges of all definitions
     * @param[in] removed The removed edge
     * @param[in] incoming The edges to use instead if an incoming edge was removed
     * @param[in] outgoing The edges to use instead if an outgoing edge was removed
     */
    void remapRemoved(NBEdge* removed,
                      const EdgeVector& incoming, const EdgeVector& outgoing);


    /** @brief Replaces a removed edge/lane
     * @param[in] removed The edge to replace
     * @param[in] removedLane The lane of this edge to replace
     * @param[in] by The edge to insert instead
     * @param[in] byLane This edge's lane to insert instead
     */
    void replaceRemoved(NBEdge* removed, int removedLane,
                        NBEdge* by, int byLane, bool incoming);

    /** @brief patches signal plans by modifying lane indices
     * with the given offset, only indices with a value above threshold are modified
     */
    void shiftTLConnectionLaneIndex(NBEdge* edge, int offset, int threshold = -1);

    /** @brief Adds a phase to the logic
     * the new phase is inserted at the end of the list of already added phases
     * @param[in] duration The duration of the phase to add
     * @param[in] state The state definition of a tls phase
     * @param[in] minDur The minimum duration of the phase to add
     * @param[in] maxDur The maximum duration of the phase to add
     */
    void addPhase(SUMOTime duration, const std::string& state, SUMOTime minDur, SUMOTime maxDur, const std::vector<int>& next, const std::string& name);

    /// @brief mark phases as load
    void phasesLoaded() {
        myPhasesLoaded = true;
    }

    /** @brief Adds a connection and immediately informs the edges
     */
    void addConnection(NBEdge* from, NBEdge* to, int fromLane, int toLane, int linkIndex, int linkIndex2, bool reconstruct = true);


    /** @brief removes the given connection from the traffic light
     * if recontruct=true, reconstructs the logic and informs the edges for immediate use in NETEDIT
     * @note: tlIndex is not necessarily unique. we need the whole connection data here
     */
    void removeConnection(const NBConnection& conn, bool reconstruct = true);

    /// @brief register changes that necessitate recomputation
    void registerModifications(bool addedConnections, bool removedConnections);

    /** @brief Returns the internal logic
     */
    NBTrafficLightLogic* getLogic() {
        return myTLLogic;
    }

    /** @brief Sets the offset of this tls
     * @param[in] offset The offset of this cycle
     */
    void setOffset(SUMOTime offset);

    /** @brief Sets the algorithm type of this tls
     * @param[in] offset The algorithm type of this tls
     */
    void setType(TrafficLightType type);

    /// @brief whether the given index must yield to the foeIndex while turing right on a red light
    bool rightOnRedConflict(int index, int foeIndex) const;

    /* @brief shortens phase states to remove states that are not referenced by
     * any controlled link and returns whether states were shortened
    */
    bool cleanupStates();

    /// @brief whether this definition uses signal group (multiple connections with the same link index)
    bool usingSignalGroups() const;

    /// @brief join nodes and states from the given logic (append red state)
    void joinLogic(NBTrafficLightDefinition* def);

    /// @brief heuristically add minDur and maxDur when switching from tlType fixed to actuated
    void guessMinMaxDuration();

    /// @brief let connections with the same state use the same link index
    void groupSignals();

    /// @brief let all connections use a distinct link index
    void ungroupSignals();

    /// @brief copy the assignment of link indices to connections from the given definition and rebuilt the states to match
    // Note: Issues a warning when the grouping of def is incompatible with the current states
    void copyIndices(NBTrafficLightDefinition* def);

protected:
    /** @brief Collects the links participating in this traffic light
     *    (only if not previously loaded)
     */
    void collectLinks();

    /** @brief Build the list of participating edges
     */
    void collectEdges();

    /** @brief Computes the traffic light logic finally in dependence to the type
     * @param[in] brakingTime Duration a vehicle needs for braking in front of the tls in seconds
     * @return The computed logic
     */
    NBTrafficLightLogic* myCompute(int brakingTimeSeconds);

    bool amInvalid() const;

    /* initialize myNeedsContRelation and set myNeedsContRelationReady to true */
    void initNeedsContRelation() const;

    /// @brief return the highest known tls link index used by any controlled connection or crossing
    int getMaxIndex();

    ///@brief Returns the maximum index controlled by this traffic light
    int getMaxValidIndex();

    /// @brief get all states for the given link index
    std::string getStates(int index);

    /// @brief return whether the given link index is used by any connectons
    bool isUsed(int index);

    /// @brief replace the given link index in all connections
    void replaceIndex(int oldIndex, int newIndex);

    /// brief retrieve all edges with connections that use the given traffic light index
    std::set<const NBEdge*> getEdgesUsingIndex(int index) const;

private:

    /** @brief phases are added directly to myTLLogic which is then returned in myCompute() */
    NBTrafficLightLogic* myTLLogic;

    /// @brief repair the plan if controlled nodes received pedestrian crossings
    void patchIfCrossingsAdded();

    /// @brief set of edges with shifted lane indices (to avoid shifting twice)
    std::set<NBEdge*> myShifted;

    /// @brief whether the logic must be reconstructed
    bool myReconstructAddedConnections;
    bool myReconstructRemovedConnections;
    bool myPhasesLoaded;

    /** @brief Collects the edges for each tlIndex
     * @param[out] fromEdges The from-edge for each controlled tlIndex
     * @param[out] toEdges The to-edge for each controlled tlIndex
     * @param[out] fromLanes The from-lanes for each controlled tlIndex
     */
    void collectEdgeVectors(EdgeVector& fromEdges, EdgeVector& toEdges, std::vector<int>& fromLanes) const;

    /// @brief adapt to removal or addition of connections
    void reconstructLogic();

    /// @brief return whether all tls link indices are valid
    bool hasValidIndices() const;

private:
    /// @brief class for identifying connections
    class connection_equal {
    public:
        /// constructor
        connection_equal(const NBConnection& c) : myC(c) {}

        bool operator()(const NBConnection& c) const {
            return c.getFrom() == myC.getFrom() && c.getTo() == myC.getTo() &&
                   c.getFromLane() == myC.getFromLane() && c.getToLane() == myC.getToLane();
        }
    private:
        const NBConnection& myC;
    private:
        /// @brief invalidated assignment operator
        connection_equal& operator=(const connection_equal& s);

    };

};
