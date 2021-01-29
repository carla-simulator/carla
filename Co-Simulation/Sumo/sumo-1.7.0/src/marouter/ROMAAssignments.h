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
/// @file    ROMAAssignments.h
/// @author  Yun-Pang Floetteroed
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Feb 2013
///
// Assignment methods
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/router/SUMOAbstractRouter.h>
#include <utils/common/SUMOTime.h>

// ===========================================================================
// class declarations
// ===========================================================================
class RONet;
class ODMatrix;
class Distribution_Points;
class ROEdge;
class ROMAEdge;
class ROVehicle;



// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROMAAssignments
 * @brief assignment methods
 *
 */
class ROMAAssignments {
public:
    /// Constructor
    ROMAAssignments(const SUMOTime begin, const SUMOTime end, const bool additiveTraffic,
                    const double adaptionFactor, const int maxAlternatives,
                    RONet& net, ODMatrix& matrix, SUMOAbstractRouter<ROEdge, ROVehicle>& router);

    /// Destructor
    ~ROMAAssignments();

    ROVehicle* getDefaultVehicle() {
        return myDefaultVehicle;
    }

    // @brief calculate edge capacity for the given edge
    static double getCapacity(const ROEdge* edge);

    // @brief calculate edge travel time for the given edge and number of vehicles per hour
    double capacityConstraintFunction(const ROEdge* edge, const double flow) const;

    // @brief clear effort storage
    void resetFlows();

    // @brief incremental method
    void incremental(const int numIter, const bool verbose);

    // @brief UE method
    void ue();

    // @brief SUE method
    void sue(const int maxOuterIteration, const int maxInnerIteration, const int kPaths, const double penalty, const double tolerance, const std::string routeChoiceMethod);

    /** @brief Returns the effort to pass an edge including penalties
     *
     * This method is given to the used router in order to obtain the efforts
     *  to pass an edge from the internal edge weights container.
     *
     * @param[in] e The edge for which the effort to be passed shall be returned
     * @param[in] v The (default) vehicle that is routed
     * @param[in] t The time for which the effort shall be returned
     * @return The effort (time to pass in this case) for an edge
     * @see DijkstraRouter_ByProxi
     */
    static double getPenalizedEffort(const ROEdge* const e, const ROVehicle* const v, double t);

    /** @brief Returns the traveltime on an edge including penalties
     *
     * This method is given to the used router in order to obtain the efforts
     *  to pass an edge from the internal edge weights container.
     *
     * @param[in] e The edge for which the effort to be passed shall be returned
     * @param[in] v The (default) vehicle that is routed
     * @param[in] t The time for which the effort shall be returned
     * @return The effort (time to pass in this case) for an edge
     * @see DijkstraRouter_ByProxi
     */
    static double getPenalizedTT(const ROEdge* const e, const ROVehicle* const v, double t);

    /** @brief Returns the traveltime on an edge without penalties
     *
     * This method is given to the used router in order to obtain the efforts
     *  to pass an edge from the internal edge weights container.
     *
     * @param[in] e The edge for which the effort to be passed shall be returned
     * @param[in] v The (default) vehicle that is routed
     * @param[in] t The time for which the effort shall be returned
     * @return The effort (time to pass in this case) for an edge
     * @see DijkstraRouter_ByProxi
     */
    static double getTravelTime(const ROEdge* const e, const ROVehicle* const v, double t);

private:
    /// @brief add a route and check for duplicates
    bool addRoute(const ConstROEdgeVector& edges, std::vector<RORoute*>& paths, std::string routeId, double prob);

    const ConstROEdgeVector computePath(ODCell* cell, const SUMOTime time = 0, const double probability = 0., SUMOAbstractRouter<ROEdge, ROVehicle>* router = nullptr);

    /// @brief get the k shortest paths
    void getKPaths(const int kPaths, const double penalty);

private:
    const SUMOTime myBegin;
    const SUMOTime myEnd;
    const bool myAdditiveTraffic;
    const double myAdaptionFactor;
    const int myMaxAlternatives;
    RONet& myNet;
    ODMatrix& myMatrix;
    SUMOAbstractRouter<ROEdge, ROVehicle>& myRouter;
    static std::map<const ROEdge* const, double> myPenalties;
    ROVehicle* myDefaultVehicle;

#ifdef HAVE_FOX
private:
    class RoutingTask : public FXWorkerThread::Task {
    public:
        RoutingTask(ROMAAssignments& assign, ODCell* c, const SUMOTime begin, const double linkFlow)
            : myAssign(assign), myCell(c), myBegin(begin), myLinkFlow(linkFlow) {}
        void run(FXWorkerThread* context);
    private:
        ROMAAssignments& myAssign;
        ODCell* const myCell;
        const SUMOTime myBegin;
        const double myLinkFlow;
    private:
        /// @brief Invalidated assignment operator.
        RoutingTask& operator=(const RoutingTask&);
    };
#endif


private:
    /// @brief Invalidated assignment operator
    ROMAAssignments& operator=(const ROMAAssignments& src);

};

