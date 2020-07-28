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
/// @file    NBNodeShapeComputer.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    2004-01-12
///
// This class computes shapes of junctions
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/geom/PositionVector.h>


// ===========================================================================
// class definitions
// ===========================================================================
class NBNode;
class NBEdge;


// ===========================================================================
// class declarations
// ===========================================================================
/**
 * @class NBNodeShapeComputer
 * @brief This class computes shapes of junctions
 */
class NBNodeShapeComputer {
public:
    /// Constructor
    NBNodeShapeComputer(const NBNode& node);

    /// Destructor
    ~NBNodeShapeComputer();

    /// Computes the shape of the assigned junction
    PositionVector compute();

    /// @brief get computed radius for node
    double getRadius() const {
        return myRadius;
    }

private:
    typedef std::map<NBEdge*, PositionVector> GeomsMap;

    /** @brief Computes the node geometry
     * Edges with the same direction are grouped.
     * Then the node geometry is built from intersection between the borders
     * of adjacent edge groups
     */
    PositionVector computeNodeShapeDefault(bool simpleContinuation);

    /** @brief Computes the node geometry using normals
     *
     * In the case the other method does not work, this method computes the geometry
     *  of a node by adding points to the polygon which are computed by building
     *  the normals of participating edges' geometry boundaries (cw/ccw)
     *  at the node's height (the length of the edge the edge would cross the node
     *  point).
     *
     *  @note This usually gives a very small node shape, appropriate for
     *  dead-ends or turn-around-only situations
     */
    PositionVector computeNodeShapeSmall();

    /// @brief compute clockwise/counter-clockwise edge boundaries
    void computeEdgeBoundaries(const EdgeVector& edges,
                                GeomsMap& geomsCCW,
                                GeomsMap& geomsCW);

    /** @brief Joins edges and computes ccw/cw boundaries
     *
     * This method goes through all edges and stores each edge's ccw and cw
     *  boundary in geomsCCW/geomsCW. This boundary is extrapolated by 100m
     *  at the node's position.
     * In addition, "same" is filled so that this map contains a list of
     *  all edges within the value-vector which direction at the node differs
     *  less than 1 from the key-edge's direction.
     */
    void joinSameDirectionEdges(const EdgeVector& edges, std::map<NBEdge*, std::set<NBEdge*> >& same);

    /** @brief Joins edges
     *
     * This methods joins edges which are in marked as being "same" in the means
     *  as given by joinSameDirectionEdges. The result (list of so-to-say "directions"
     *  is returned;
     */
    EdgeVector computeUniqueDirectionList(
            const EdgeVector& all,
            std::map<NBEdge*, std::set<NBEdge*> >& same,
            GeomsMap& geomsCCW,
            GeomsMap& geomsCW);

    /** @brief Compute smoothed corner shape
     * @param[in] begShape
     * @param[in] endShape
     * @param[in] begPoint
     * @param[in] endPoint
     * @param[in] cornerDetail
     * @return shape to be appended between begPoint and endPoint
     */
    PositionVector getSmoothCorner(PositionVector begShape, PositionVector endShape,
                                   const Position& begPoint, const Position& endPoint, int cornerDetail);

    /** @brief Initialize neighbors and angles
     * @param[in] edges The list of edges sorted in clockwise direction
     * @param[in] current An iterator to the current edge
     * @param[in] geomsCW geometry map
     * @param[in] geomsCCW geometry map
     * @param[out] cwi An iterator to the clockwise neighbor
     * @param[out] ccwi An iterator to the counter-clockwise neighbor
     * @param[out] cad The angle difference to the clockwise neighbor
     * @param[out] ccad The angle difference to the counter-clockwise neighbor
     */
    static void initNeighbors(const EdgeVector& edges, const EdgeVector::const_iterator& current,
                              GeomsMap& geomsCW,
                              GeomsMap& geomsCCW,
                              EdgeVector::const_iterator& cwi,
                              EdgeVector::const_iterator& ccwi,
                              double& cad,
                              double& ccad);

    /// @return whether trying to intersect these edges would probably fail
    bool badIntersection(const NBEdge* e1, const NBEdge* e2, double distance);

    /// @brief return the intersection point closest to the given offset
    double closestIntersection(const PositionVector& geom1, const PositionVector& geom2, double offset);

    /// @brief whether the given edges (along with those in the same direction) requires a large turning radius
    bool needsLargeTurn(NBEdge* e1, NBEdge* e2,
            std::map<NBEdge*, std::set<NBEdge*> >& same) const;

    /// @brief determine the default radius appropriate for the current junction
    double getDefaultRadius(const OptionsCont& oc);

    /// @brief compute with of rightmost lanes that exlude the given permissions
    static double getExtraWidth(const NBEdge* e, SVCPermissions exclude);

private:
    /// The node to compute the geometry for
    const NBNode& myNode;

    /// @brief the computed node radius
    double myRadius;

    static const SVCPermissions SVC_LARGE_TURN;

private:
    /// @brief Invalidated assignment operator
    NBNodeShapeComputer& operator=(const NBNodeShapeComputer& s);

};
