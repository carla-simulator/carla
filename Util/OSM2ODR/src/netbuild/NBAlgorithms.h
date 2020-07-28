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
/// @file    NBAlgorithms.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    02. March 2012
///
// Algorithms for network computation
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include "NBEdgeCont.h"
#include "NBNodeCont.h"

// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBNode;

// ===========================================================================
// class definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// NBTurningDirectionsComputer
// ---------------------------------------------------------------------------
/* @class NBTurningDirectionsComputer
 * @brief Computes turnaround destinations for all edges (if exist)
 */
class NBTurningDirectionsComputer {
public:
    /** @brief Computes turnaround destinations for all edges (if exist)
     * @param[in] nc The container of nodes to loop along
     * @param[in] warn Whether warnings shall be issued
     */
    static void computeTurnDirections(NBNodeCont& nc, bool warn = true);

    /** @brief Computes turnaround destinations for all incoming edges of the given nodes (if any)
     * @param[in] node The node for which to compute turnaround destinations
     * @param[in] warn Whether warnings shall be issued
     * @note: This is needed by NETEDIT
     */
    static void computeTurnDirectionsForNode(NBNode* node, bool warn);

private:
    /** @struct Combination
     * @brief Stores the information about the angle between an incoming ("from") and an outgoing ("to") edge
     *
     * Note that the angle is increased by 360 if the edges connect the same two nodes in
     *  opposite direction.
     */
    struct Combination {
        NBEdge* from;
        NBEdge* to;
        double angle;
    };


    /** @class combination_by_angle_sorter
     * @brief Sorts "Combination"s by decreasing angle
     */
    class combination_by_angle_sorter {
    public:
        explicit combination_by_angle_sorter() { }
        int operator()(const Combination& c1, const Combination& c2) const {
            if (c1.angle != c2.angle) {
                return c1.angle > c2.angle;
            }
            if (c1.from != c2.from) {
                return c1.from->getID() < c2.from->getID();
            }
            return c1.to->getID() < c2.to->getID();
        }
    };
};



// ---------------------------------------------------------------------------
// NBNodesEdgesSorter
// ---------------------------------------------------------------------------
/* @class NBNodesEdgesSorter
 * @brief Sorts a node's edges clockwise regarding driving direction
 */
class NBNodesEdgesSorter {
public:
    /** @brief Sorts a node's edges clockwise regarding driving direction
     * @param[in] nc The container of nodes to loop along
     * @param[in] useNodeShape Whether to sort based on the node shape (instead of only the edge angle)
     */
    static void sortNodesEdges(NBNodeCont& nc, bool useNodeShape = false);

    /** @class crossing_by_junction_angle_sorter
     * @brief Sorts crossings by minimum clockwise clockwise edge angle. Use the
     * ordering found in myAllEdges of the given node
     */
    class crossing_by_junction_angle_sorter {
    public:
        explicit crossing_by_junction_angle_sorter(const NBNode* node, const EdgeVector& ordering);

        int operator()(const std::unique_ptr<NBNode::Crossing>& c1, const std::unique_ptr<NBNode::Crossing>& c2) const {
            const int r1 = getMinRank(c1->edges);
            const int r2 = getMinRank(c2->edges);
            if (r1 == r2) {
                return c1->edges.size() > c2->edges.size();
            } else {
                return (int)(r1 < r2);
            }
        }

    private:
        /// @brief retrieves the minimum index in myAllEdges
        int getMinRank(const EdgeVector& e) const {
            int result = (int)myOrdering.size();
            for (EdgeVector::const_iterator it = e.begin(); it != e.end(); ++it) {
                int rank = (int)std::distance(myOrdering.begin(), std::find(myOrdering.begin(), myOrdering.end(), *it));
                result = MIN2(result, rank);
            }
            return result;
        }

    private:
        EdgeVector myOrdering;

    private:
        /// @brief invalidated assignment operator
        crossing_by_junction_angle_sorter& operator=(const crossing_by_junction_angle_sorter& s);

    };
    /** @brief Assures correct order for same-angle opposite-direction edges
     * @param[in] n The currently processed node
     * @param[in] i1 Pointer to first edge
     * @param[in] i2 Pointer to second edge
     */
    static void swapWhenReversed(const NBNode* const n,
                                 const std::vector<NBEdge*>::iterator& i1,
                                 const std::vector<NBEdge*>::iterator& i2);


    /** @class edge_by_junction_angle_sorter
     * @brief Sorts incoming and outgoing edges clockwise around the given node
     */
    class edge_by_junction_angle_sorter {
    public:
        explicit edge_by_junction_angle_sorter(NBNode* n) : myNode(n) {}
        int operator()(NBEdge* e1, NBEdge* e2) const {
            return getConvAngle(e1) < getConvAngle(e2);
        }

    protected:
        /// @brief Converts the angle of the edge if it is an incoming edge
        double getConvAngle(NBEdge* e) const {
            double angle = e->getAngleAtNode(myNode);
            if (angle < 0.) {
                angle = 360. + angle;
            }
            // convert angle if the edge is an outgoing edge
            if (e->getFromNode() == myNode) {
                angle += (double) 180.;
                if (angle >= (double) 360.) {
                    angle -= (double) 360.;
                }
            }
            if (angle < 0.1 || angle > 359.9) {
                angle = (double) 0.;
            }
            assert(angle >= 0 && angle < (double)360);
            return angle;
        }

    private:
        /// @brief The node to compute the relative angle of
        NBNode* myNode;

    };

};



// ---------------------------------------------------------------------------
// NBNodeTypeComputer
// ---------------------------------------------------------------------------
/* @class NBNodeTypeComputer
 * @brief Computes node types
 */
class NBNodeTypeComputer {
public:
    /** @brief Computes node types
     * @param[in] nc The container of nodes to loop along
     */
    static void computeNodeTypes(NBNodeCont& nc, NBTrafficLightLogicCont& tlc);

    /** @brief Checks rail_crossing for validity
     * @param[in] nc The container of nodes to loop along
     */
    static void validateRailCrossings(NBNodeCont& nc, NBTrafficLightLogicCont& tlc);

    /// @brief whether the given node only has rail edges
    static bool isRailwayNode(const NBNode* n);
};



// ---------------------------------------------------------------------------
// NBEdgePriorityComputer
// ---------------------------------------------------------------------------
/* @class NBEdgePriorityComputer
 * @brief Computes edge priorities within a node
 */
class NBEdgePriorityComputer {
public:
    /** @brief Computes edge priorities within a node
     * @param[in] nc The container of nodes to loop along
     */
    static void computeEdgePriorities(NBNodeCont& nc);

private:
    /** @brief Sets the priorites in case of a priority junction
     * @param[in] n The node to set edges' priorities
     */
    static void setPriorityJunctionPriorities(NBNode& n);

    /// @brief set priority for edges that are parallel to the best edges
    static void markBestParallel(const NBNode& n, NBEdge* bestFirst, NBEdge* bestSecond);

    /** @brief Sets the priorites in case of a priority junction
     * @param[in] n The node to set edges' priorities
     * @param[in] s The vector of edges to get and mark the first from
     * @param[in] prio The priority to assign
     * @return The vector's first edge
     */
    static NBEdge* extractAndMarkFirst(NBNode& n, std::vector<NBEdge*>& s, int prio = 1);

    /** @brief Returns whether both edges have the same priority
     * @param[in] e1 The first edge
     * @param[in] e2 The second edge
     * Whether both edges have the same priority
     */
    static bool samePriority(const NBEdge* const e1, const NBEdge* const e2);

    /// @brief return whether the priorite attribute can be used to distinguish the edges
    static bool hasDifferentPriorities(const EdgeVector& edges, const NBEdge* excluded);

};
