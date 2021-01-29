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
/// @file    ROJTRRouter.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Computes routes using junction turning percentages
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/router/SUMOAbstractRouter.h>
#include <router/RORoutable.h>


// ===========================================================================
// class declarations
// ===========================================================================
class RONet;
class ROEdge;
class ROJTREdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROJTRRouter
 * @brief Computes routes using junction turning percentages
 */
class ROJTRRouter : public SUMOAbstractRouter<ROEdge, ROVehicle> {
public:
    /** @brief Constructor
     * @param[in] unbuildIsWarningOnly Whether not closed routes shall not yield in an error
     * @param[in] acceptAllDestinations If false, only sinks will be used as final edges
     * @param[in] maxEdges The maximum number of edges a route may have
     * @param[in] ignoreClasses Whether routing shall be done without regarding vehicle classes
     * @param[in] allowLoops Whether a vehicle may reuse a road
     * @param[in] discountSources Whether upstream flow shall be discounted from source flows
     */
    ROJTRRouter(bool unbuildIsWarningOnly,
                bool acceptAllDestinations, int maxEdges, bool ignoreClasses,
                bool allowLoops,
                bool discountSources);


    /// @brief Destructor
    ~ROJTRRouter();

    virtual SUMOAbstractRouter<ROEdge, ROVehicle>* clone() {
        return new ROJTRRouter(myUnbuildIsWarningOnly, myAcceptAllDestination, myMaxEdges, myIgnoreClasses, myAllowLoops, myDiscountSources);
    }

    /// @name Implementatios of SUMOAbstractRouter
    /// @{

    /** @brief Computes a route
     *
     * The description how routes are computed is given in the user documentation
     * @param[in] from The edge the vehicle starts at
     * @param[in] to The destination edge - invalid here
     * @param[in] vehicle The vehicle to compute the route for
     * @param[in] time The departure time of the vehicle
     * @param[filled] into The list of edges to store the route into
     */
    bool compute(const ROEdge* from, const ROEdge* to, const ROVehicle* const vehicle,
                 SUMOTime time, ConstROEdgeVector& into, bool silent = false);


    /** @brief Recomputes the costs of a route
     * @param[in] edges The route
     * @param[in] v The vehicle that belongs to the route
     * @param[in] msTime The departure time of the vehicle
     * @return The route costs
     */
    double recomputeCosts(const ConstROEdgeVector& edges, const ROVehicle* const v, SUMOTime msTime) const;
    /// @}

private:
    /// @brief Whether unbuildable routes shall be reported as warniings, not errors
    const bool myUnbuildIsWarningOnly;

    /// @brief Whether all edges may be used as route end
    const bool myAcceptAllDestination;

    /// @brief The maximum number of edges a route may have
    const int myMaxEdges;

    /// @brief Whether vehicle class information shall be ignored
    const bool myIgnoreClasses;

    /// @brief Whether a vehicle may reuse a road
    const bool myAllowLoops;

    /// @brief Whether upstream flows shall be discounted from source flows
    const bool myDiscountSources;
};
