/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    NBAlgorithms_Ramps.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    29. March 2012
///
// Algorithms for highway on-/off-ramps computation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>


// ===========================================================================
// class declarations
// ===========================================================================
class NBNetBuilder;
class OptionsCont;
class NBNode;
class NBEdgeCont;
class NBDistrictCont;


// ===========================================================================
// class definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// NBAlgorithms_Ramps
// ---------------------------------------------------------------------------
/* @class NBRampsComputer
 * @brief Computes highway on-/off-ramps (if wished)
 */
class NBRampsComputer {
public:
    /** @brief Computes highway on-/off-ramps (if wished)
     * @param[in, changed] nb The network builder which contains the current network representation
     * @param[in] oc The options container
     */
    static void computeRamps(NBNetBuilder& nb, OptionsCont& oc);

    /// @brief suffix for newly generated on-ramp edges
    static const std::string ADDED_ON_RAMP_EDGE;

private:
    /** @brief Determines whether the given node may be an on-ramp begin
     * @param[in] cur The node to check
     * @param[in] minHighwaySpeed The minimum speed limit a highway must have for being a highway
     * @param[in] maxRampSpeed The maximum speed limit a ramp must have for being a ramp
     * @param[in] noramps Edges that shall not be treated as ramps
     * @param[in] minWeaveLength The minimum length for weaving areas
     * @return Whether the node is assumed to be an on-ramp begin
     */
    static bool mayNeedOnRamp(NBNode* cur, double minHighwaySpeed, double maxRampSpeed,
                              const std::set<std::string>& noramps, double minWeaveLength);


    /** @brief Determines whether the given node may be an off-ramp end
     * @param[in] cur The node to check
     * @param[in] minHighwaySpeed The minimum speed limit a highway must have for being a highway
     * @param[in] maxRampSpeed The maximum speed limit a ramp must have for being a ramp
     * @param[in] noramps Edges that shall not be treated as ramps
     * @return Whether the node is assumed to be an off-ramp end
     */
    static bool mayNeedOffRamp(NBNode* cur, double minHighwaySpeed, double maxRampSpeed,
                               const std::set<std::string>& noramps);


    /** @brief Builds an on-ramp starting at the given node
     * @param[in] cur The node at which the on-ramp shall begin
     * @param[in] nc The container of nodes
     * @param[in] ec The container of edges
     * @param[in] dc The container of districts
     * @param[in] rampLength The wished ramp length
     * @param[in] dontSplit Whether no edges shall be split
     * @param[in, filled] incremented The list of edges which lane number was already incremented
     */
    static void buildOnRamp(NBNode* cur, NBNodeCont& nc, NBEdgeCont& ec, NBDistrictCont& dc, double rampLength, bool dontSplit, bool addLanes);


    /** @brief Builds an off-ramp ending at the given node
     * @param[in] cur The node at which the off-ramp shall end
     * @param[in] nc The container of nodes
     * @param[in] ec The container of edges
     * @param[in] dc The container of districts
     * @param[in] rampLength The wished ramp length
     * @param[in] dontSplit Whether no edges shall be split
     * @param[in, filled] incremented The list of edges which lane number was already incremented
     */
    static void buildOffRamp(NBNode* cur, NBNodeCont& nc, NBEdgeCont& ec, NBDistrictCont& dc, double rampLength, bool dontSplit, bool addLanes,
                             const std::set<NBNode*, ComparatorIdLess>& potOnRamps);


    static void getOnRampEdges(NBNode* n, NBEdge** potHighway, NBEdge** potRamp, NBEdge** other);
    static void getOffRampEdges(NBNode* n, NBEdge** potHighway, NBEdge** potRamp, NBEdge** other);
    static bool determinedBySpeed(NBEdge** potHighway, NBEdge** potRamp);
    static bool determinedByLaneNumber(NBEdge** potHighway, NBEdge** potRamp);

    /** @brief Checks whether an on-/off-ramp can be bult here
     *
     * - none of the participating edges must be a macroscopic connector
     * - ramp+highways together must have more lanes than the continuation
     * - speeds must match the defined swells
     * @param[in] potHighway The highway part to check
     * @param[in] potRamp The ramp part to check
     * @param[in] other The successor/predecessor edge
     * @param[in] minHighwaySpeed The minimum speed limit a highway must have for being a highway
     * @param[in] maxRampSpeed The maximum speed limit a ramp must have for being a ramp
     * @param[in] noramps Edges that shall not be treated as ramps
     * @return Whether a ramp can be built here
     */
    static bool fulfillsRampConstraints(NBEdge* potHighway, NBEdge* potRamp, NBEdge* other, double minHighwaySpeed, double maxRampSpeed,
                                        const std::set<std::string>& noramps);


    /** @brief Moves the ramp to the right, as new lanes were added
     * @param[in] ramp The ramp to move
     * @param[in] addedLanes The number of added lanes
     */
    static void moveRampRight(NBEdge* ramp, int addedLanes);

    /// @brief whether the edge has a mode that does not indicate a ramp edge
    static bool hasWrongMode(NBEdge* edge);

    /// @brief shift ramp geometry to merge smoothly with the motorway
    static void patchRampGeometry(NBEdge* potRamp, NBEdge* first, NBEdge* potHighway, bool onRamp);
};
