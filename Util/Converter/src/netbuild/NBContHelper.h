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
/// @file    NBContHelper.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 17 Dec 2001
///
// Some methods for traversing lists of edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cassert>
#include "NBHelpers.h"
#include "NBCont.h"
#include "NBEdge.h"
#include "NBNode.h"
#include <utils/common/StdDefs.h>
#include <utils/geom/GeomHelper.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * NBContHelper
 * Some static helper methods that traverse a sorted list of edges in both
 * directions
 */
class NBContHelper {
public:
    /** Moves the given iterator clockwise within the given container
        of edges sorted clockwise */
    static void nextCW(const EdgeVector& edges,
                       EdgeVector::const_iterator& from);

    /** Moves the given iterator counter clockwise within the given container
        of edges sorted clockwise */
    static void nextCCW(const EdgeVector& edges,
                        EdgeVector::const_iterator& from);

    static double getMaxSpeed(const EdgeVector& edges);

    static double getMinSpeed(const EdgeVector& edges);

    /** writes the vector of bools to the given stream */
    static std::ostream& out(std::ostream& os, const std::vector<bool>& v);


    /**
     * relative_outgoing_edge_sorter
     * Class to sort edges by their angle in relation to the node the
     * edge using this class is incoming into. This is normally done to
     * sort edges outgoing from the node the using edge is incoming in
     * by their angle in relation to the using edge's angle (this angle
     * is the reference angle).
     */
    class relative_outgoing_edge_sorter {
    public:
        /// constructor
        explicit relative_outgoing_edge_sorter(NBEdge* e) : myAngle(e->getEndAngle()) {}
        /// constructor
        explicit relative_outgoing_edge_sorter(double angle) : myAngle(angle) {}

    public:
        /// comparing operation
        bool operator()(const NBEdge* e1, const NBEdge* e2) const;

    private:
        /// @brief the reference angle to compare edges agains
        double myAngle;
    };


    /**
     * relative_incoming_edge_sorter
     * Class to sort edges by their angle in relation to an outgoing edge.
     * This is normally done to sort edges incoming at the starting node of this edge
     * by their angle in relation to the using edge's angle (this angle
     * is the reference angle).
     */
    class relative_incoming_edge_sorter {
    public:
        /// constructor
        explicit relative_incoming_edge_sorter(NBEdge* e) : myAngle(e->getStartAngle()) {}
        /// constructor
        explicit relative_incoming_edge_sorter(double angle) : myAngle(angle) {}

    public:
        /// comparing operation
        bool operator()(const NBEdge* e1, const NBEdge* e2) const;

    private:
        /// @brief the reference angle to compare edges agains
        double myAngle;
    };


    /**
     * edge_by_priority_sorter
     * Class to sort edges by their priority
     */
    class edge_by_priority_sorter {
    public:
        /// comparing operator
        int operator()(NBEdge* e1, NBEdge* e2) const {
            if (e1->getPriority() != e2->getPriority()) {
                return e1->getPriority() > e2->getPriority();
            }
            if (e1->getSpeed() != e2->getSpeed()) {
                return e1->getSpeed() > e2->getSpeed();
            }
            return e1->getNumLanes() > e2->getNumLanes();
        }
    };

    // ---------------------------

    /**
     * @class edge_opposite_direction_sorter
     * @brief Class to sort edges by their angle in relation to the given edge
     *
     * The resulting sorted list has the edge in the most opposite direction
     *  to the given edge as her first entry.
     */
    class edge_opposite_direction_sorter {
    public:
        /** @brief Constructor
         * @param[in] e The edge to which the sorting relates
         * @param[in] n The node to consider
         */
        explicit edge_opposite_direction_sorter(const NBEdge* const e, const NBNode* const n, bool regardPriority) :
            myNode(n),
            myEdge(e),
            myRegardPriority(regardPriority) {
            myAngle = getEdgeAngleAt(e, n);
        }

        /** @brief Comparing operation
         * @param[in] e1 The first edge to compare
         * @param[in] e2 The second edge to compare
         * @return Which edge is more opposite to the related one
         */
        int operator()(NBEdge* e1, NBEdge* e2) const {
            if (!myRegardPriority || e1->getPriority() == e2->getPriority() || e1 == myEdge || e2 == myEdge) {
                return getDiff(e1) > getDiff(e2);
            } else {
                return e1->getPriority() > e2->getPriority();
            }
        }

    protected:
        /** @brief Computes the angle difference between the related and the given edge
         * @param[in] e The edge to compare the angle difference of
         * @return The angle difference
         */
        double getDiff(const NBEdge* const e) const {
            return fabs(GeomHelper::angleDiff(getEdgeAngleAt(e, myNode), myAngle));
        }

        /** @brief Returns the given edge's angle at the given node
         *
         * Please note that we always consider the "outgoing direction".
         * @param[in] e The edge to which the sorting relates
         * @param[in] n The node to consider
         */
        double getEdgeAngleAt(const NBEdge* const e, const NBNode* const n) const {
            if (e->getFromNode() == n) {
                return e->getGeometry().angleAt2D(0);
            } else {
                return e->getGeometry()[-1].angleTo2D(e->getGeometry()[-2]);
            }
        }

    private:

        /// @brief The related node
        const NBNode* const myNode;

        /// @brief the reference edge
        const NBEdge* const myEdge;

        /// @brief The angle of the related edge at the given node
        double myAngle;

        /// @brief Whether edge priority may override closer angles
        bool myRegardPriority;

    private:
        /// @brief Invalidated assignment operator
        edge_opposite_direction_sorter& operator=(const edge_opposite_direction_sorter& s);

    };

    // ---------------------------

    /**
     * edge_similar_direction_sorter
     * Class to sort edges by their angle in relation to the given edge
     * The resulting list should have the edge in the most similar direction
     * to the given edge as its first entry
     */
    class edge_similar_direction_sorter {
    public:
        /// constructor
        explicit edge_similar_direction_sorter(const NBEdge* const e, bool outgoing = true) :
            myCompareOutgoing(outgoing),
            myAngle(outgoing ? e->getShapeEndAngle() : e->getShapeStartAngle())
        {}

        /// comparing operation
        int operator()(const NBEdge* e1, const NBEdge* e2) const {
            const double d1 = angleDiff(myCompareOutgoing ? e1->getShapeStartAngle() : e1->getShapeEndAngle(), myAngle);
            const double d2 = angleDiff(myCompareOutgoing ? e2->getShapeStartAngle() : e2->getShapeEndAngle(), myAngle);
            if (fabs(fabs(d1) - fabs(d2)) < NUMERICAL_EPS) {
                if (fabs(d1 - d2) > NUMERICAL_EPS) {
                    return d1 < d2;
                } else {
                    return e1->getNumericalID() < e2->getNumericalID();
                }
            }
            return fabs(d1) < fabs(d2);
        }

    private:
        double angleDiff(const double angle1, const double angle2) const {
            double d = angle2 - angle1;
            while (d >= 180.) {
                d -= 360.;
            }
            while (d < -180.) {
                d += 360.;
            }
            return d;
        }


    private:
        /// the angle to find the edge with the opposite direction
        bool myCompareOutgoing;
        double myAngle;
    };


    /**
     * @class node_with_incoming_finder
     */
    class node_with_incoming_finder {
    public:
        /// constructor
        node_with_incoming_finder(const NBEdge* const e);

        bool operator()(const NBNode* const n) const;

    private:
        const NBEdge* const myEdge;

    private:
        /// @brief invalidated assignment operator
        node_with_incoming_finder& operator=(const node_with_incoming_finder& s);

    };


    /**
     * @class node_with_outgoing_finder
     */
    class node_with_outgoing_finder {
    public:
        /// constructor
        node_with_outgoing_finder(const NBEdge* const e);

        bool operator()(const NBNode* const n) const;

    private:
        const NBEdge* const myEdge;

    private:
        /// @brief invalidated assignment operator
        node_with_outgoing_finder& operator=(const node_with_outgoing_finder& s);

    };




    class edge_with_destination_finder {
    public:
        /// constructor
        edge_with_destination_finder(NBNode* dest);

        bool operator()(NBEdge* e) const;

    private:
        NBNode* myDestinationNode;

    private:
        /// @brief invalidated assignment operator
        edge_with_destination_finder& operator=(const edge_with_destination_finder& s);

    };


    /** Tries to return the first edge within the given container which
        connects both given nodes */
    static NBEdge* findConnectingEdge(const EdgeVector& edges,
                                      NBNode* from, NBNode* to);


    /** returns the maximum speed allowed on the edges */
    static double maxSpeed(const EdgeVector& ev);

    /**
     * same_connection_edge_sorter
     * This class is used to sort edges which connect the same nodes.
     * The edges are sorted in dependence to edges connecting them. The
     * rightmost will be the first in the list; the leftmost the last one.
     */
    class same_connection_edge_sorter {
    public:
        /// constructor
        explicit same_connection_edge_sorter() { }

        /// comparing operation
        int operator()(NBEdge* e1, NBEdge* e2) const {
            std::pair<double, double> mm1 = getMinMaxRelAngles(e1);
            std::pair<double, double> mm2 = getMinMaxRelAngles(e2);
            if (mm1.first == mm2.first && mm1.second == mm2.second) {
                // ok, let's simply sort them arbitrarily
                return e1->getID() < e2->getID();
            }

            assert(
                (mm1.first <= mm2.first && mm1.second <= mm2.second)
                ||
                (mm1.first >= mm2.first && mm1.second >= mm2.second));
            return (mm1.first >= mm2.first && mm1.second >= mm2.second);
        }

        /**
         *
         */
        std::pair<double, double> getMinMaxRelAngles(NBEdge* e) const {
            double min = 360;
            double max = 360;
            const EdgeVector& ev = e->getConnectedEdges();
            for (EdgeVector::const_iterator i = ev.begin(); i != ev.end(); ++i) {
                double angle = NBHelpers::normRelAngle(
                                   e->getTotalAngle(), (*i)->getTotalAngle());
                if (min == 360 || min > angle) {
                    min = angle;
                }
                if (max == 360 || max < angle) {
                    max = angle;
                }
            }
            return std::pair<double, double>(min, max);
        }
    };


    friend std::ostream& operator<<(std::ostream& os, const EdgeVector& ev);

    class opposite_finder {
    public:
        /// constructor
        opposite_finder(NBEdge* edge)
            : myReferenceEdge(edge) { }

        bool operator()(NBEdge* e) const {
            return e->isTurningDirectionAt(myReferenceEdge) ||
                   myReferenceEdge->isTurningDirectionAt(e);
        }

    private:
        NBEdge* myReferenceEdge;

    };

    /**
     * edge_by_angle_to_nodeShapeCentroid_sorter
     * Class to sort edges by their angle in relation to the node shape
     * */
    class edge_by_angle_to_nodeShapeCentroid_sorter {
    public:
        /// constructor
        explicit edge_by_angle_to_nodeShapeCentroid_sorter(const NBNode* n) : myNode(n) {}

    public:
        /// comparing operation
        bool operator()(const NBEdge* e1, const NBEdge* e2) const;

    private:
        /// the edge to compute the relative angle of
        const NBNode* myNode;
    };

};
