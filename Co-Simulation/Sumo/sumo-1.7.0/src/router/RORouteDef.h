/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    RORouteDef.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// Base class for a vehicle's route definition
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <iostream>
#include <utils/common/Named.h>
#include <utils/router/SUMOAbstractRouter.h>
#include "RORoute.h"


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class OptionsCont;
class ROVehicle;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORouteDef
 * @brief Base class for a vehicle's route definition
 *
 * This class resembles what a vehicle knows about his route when being loaded
 *  into a router. Whether it is just the origin and the destination, the whole
 *  route through the network or even a route with alternatives depends on
 *  the derived class.
 */
class RORouteDef : public Named {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the route
     * @param[in] color The color of the route
     */
    RORouteDef(const std::string& id, const int lastUsed,
               const bool tryRepair, const bool mayBeDisconnected);


    /// @brief Destructor
    virtual ~RORouteDef();


    /** @brief Adds a single alternative loaded from the file
        An alternative may also be generated during DUA */
    void addLoadedAlternative(RORoute* alternative);

    /** @brief Adds an alternative loaded from the file */
    void addAlternativeDef(const RORouteDef* alternative);

    /** @brief Triggers building of the complete route (via
     * preComputeCurrentRoute) or returns precomputed route */
    RORoute* buildCurrentRoute(SUMOAbstractRouter<ROEdge, ROVehicle>& router, SUMOTime begin,
                               const ROVehicle& veh) const;

    /** @brief Builds the complete route
     * (or chooses her from the list of alternatives, when existing) */
    void preComputeCurrentRoute(SUMOAbstractRouter<ROEdge, ROVehicle>& router, SUMOTime begin,
                                const ROVehicle& veh) const;

    /** @brief Builds the complete route
     * (or chooses her from the list of alternatives, when existing) */
    bool repairCurrentRoute(SUMOAbstractRouter<ROEdge, ROVehicle>& router, SUMOTime begin,
                            const ROVehicle& veh, ConstROEdgeVector oldEdges, ConstROEdgeVector& newEdges) const;

    /** @brief Adds an alternative to the list of routes
    *
     * (This may be the new route) */
    void addAlternative(SUMOAbstractRouter<ROEdge, ROVehicle>& router,
                        const ROVehicle* const, RORoute* current, SUMOTime begin);

    const ROEdge* getDestination() const;

    const RORoute* getFirstRoute() const {
        if (myAlternatives.empty()) {
            return 0;
        }
        return myAlternatives.front();
    }

    /** @brief Saves the built route / route alternatives
     *
     * Writes the route into the given stream.
     *
     * @param[in] dev The device to write the route into
     * @param[in] asAlternatives Whether the route shall be saved as route alternatives
     * @return The same device for further usage
     */
    OutputDevice& writeXMLDefinition(OutputDevice& dev, const ROVehicle* const veh,
                                     bool asAlternatives, bool withExitTimes) const;

    /** @brief Returns a origin-destination copy of the route definition.
     *
     * The resulting route definition contains only a single route with
     * origin and destination edge copied from this one
     *
     * @param[in] id The id for the new route definition
     * @return the new route definition
     */
    RORouteDef* copyOrigDest(const std::string& id) const;

    /** @brief Returns a deep copy of the route definition.
     *
     * The resulting route definition contains copies of all
     * routes contained in this one
     *
     * @param[in] id The id for the new route definition
     * @param[in] stopOffset The offset time for "until"-stops defined in the original route
     * @return the new route definition
     */
    RORouteDef* copy(const std::string& id, const SUMOTime stopOffset) const;

    /** @brief Returns the sum of the probablities of the contained routes */
    double getOverallProb() const;


    /// @brief whether this route shall be silently discarded
    bool discardSilent() const {
        return myDiscardSilent;
    }


    static void setUsingJTRR() {
        myUsingJTRR = true;
    }

protected:
    /// @brief precomputed route for out-of-order computation
    mutable RORoute* myPrecomputed;

    /// @brief Index of the route used within the last step
    mutable int myLastUsed;

    /// @brief The alternatives
    std::vector<RORoute*> myAlternatives;

    /// @brief Routes which are deleted someplace else
    std::set<RORoute*> myRouteRefs;

    /// @brief Information whether a new route was generated
    mutable bool myNewRoute;

    const bool myTryRepair;
    const bool myMayBeDisconnected;

    /// @brief Whether this route should be silently discarded
    mutable bool myDiscardSilent;

    static bool myUsingJTRR;

private:
    /** Function-object for sorting routes from highest to lowest probability. */
    struct ComparatorProbability {
        bool operator()(const RORoute* const a, const RORoute* const b) {
            return a->getProbability() > b->getProbability();
        }
    };

private:
    /// @brief Invalidated copy constructor
    RORouteDef(const RORouteDef& src);

    /// @brief Invalidated assignment operator
    RORouteDef& operator=(const RORouteDef& src);

};
