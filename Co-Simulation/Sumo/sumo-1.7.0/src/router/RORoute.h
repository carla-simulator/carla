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
/// @file    RORoute.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Yun-Pang Floetteroed
/// @date    Sept 2002
///
// A complete router's route
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/common/Named.h>
#include <utils/common/RGBColor.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/vehicle/SUMOVehicleParameter.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class ROVehicle;
class OutputDevice;

typedef std::vector<const ROEdge*> ConstROEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORoute
 * @brief A complete router's route
 *
 * This class represents a single and complete vehicle route after being
 *  computed/imported.
 */
class RORoute : public Named {
public:
    /** @brief Constructor
     *
     * @param[in] id The route's id
     * @param[in] costs The route's costs
     * @param[in] prob The route's probability
     * @param[in] route The list of edges the route is made of
     * @param[in] color The (optional) color of this route
     *
     * @todo Are costs/prob really mandatory?
     */
    RORoute(const std::string& id, double costs, double prob,
            const ConstROEdgeVector& route, const RGBColor* const color,
            const std::vector<SUMOVehicleParameter::Stop>& stops);


    /** @brief Constructor
     *
     * @param[in] id The route's id
     * @param[in] route The list of edges the route is made of
     */
    RORoute(const std::string& id, const ConstROEdgeVector& route);

    /** @brief Copy constructor
     *
     * @param[in] src The route to copy
     */
    RORoute(const RORoute& src);


    /// @brief Destructor
    ~RORoute();


    /** @brief Returns the first edge in the route
     *
     * @return The route's first edge
     */
    const ROEdge* getFirst() const {
        return myRoute[0];
    }


    /** @brief Returns the last edge in the route
     *
     * @return The route's last edge
     */
    const ROEdge* getLast() const {
        return myRoute.back();
    }


    /** @brief Returns the costs of the route
     *
     * @return The route's costs (normally the time needed to pass it)
     * @todo Recheck why the costs are stored in a route
     */
    double getCosts() const {
        return myCosts;
    }


    /** @brief Returns the probability the driver will take this route with
     *
     * @return The probability to choose the route
     * @todo Recheck why the probability is stored in a route
     */
    double getProbability() const {
        return myProbability;
    }


    /** @brief Sets the costs of the route
     *
     * @todo Recheck why the costs are stored in a route
     */
    void setCosts(double costs);


    /** @brief Sets the probability of the route
     *
     * @todo Recheck why the probability is stored in a route
     */
    void setProbability(double prob);


    /** @brief Returns the number of edges in this route
     *
     * @return The number of edges the route is made of
     */
    int size() const {
        return (int) myRoute.size();
    }


    /** @brief Returns the list of edges this route consists of
     *
     * @return The edges this route consists of
     */
    const ConstROEdgeVector& getEdgeVector() const {
        return myRoute;
    }

    /** @brief Returns this route's color
     *
     * @return This route's color
     */
    const RGBColor* getColor() const {
        return myColor;
    }


    /** @brief Checks whether this route contains loops and removes such
     */
    void recheckForLoops(const ConstROEdgeVector& mandatory);

    OutputDevice&
    writeXMLDefinition(OutputDevice& dev, const ROVehicle* const veh,
                       const bool withCosts, const bool withExitTimes) const;

    /** @brief add additional vehicles/probability
     */
    void addProbability(double prob);

    /** @brief Returns the list of stops this route contains
     *
     * @return list of stops
     */
    const std::vector<SUMOVehicleParameter::Stop>& getStops() const {
        return myStops;
    }

    /** @brief Adapts the until time of all stops by the given offset
     */
    void addStopOffset(const SUMOTime offset) {
        for (std::vector<SUMOVehicleParameter::Stop>::iterator stop = myStops.begin(); stop != myStops.end(); ++stop) {
            if (stop->until >= 0) {
                stop->until += offset;
            }
        }
    }

private:
    /// @brief The costs of the route
    double myCosts;

    /// @brief The probability the driver will take this route with
    double myProbability;

    /// @brief The edges the route consists of
    ConstROEdgeVector myRoute;

    /// @brief The color of the route
    const RGBColor* myColor;

    /// @brief List of the stops on the parsed route
    std::vector<SUMOVehicleParameter::Stop> myStops;


private:
    /// @brief Invalidated assignment operator
    RORoute& operator=(const RORoute& src);

};
