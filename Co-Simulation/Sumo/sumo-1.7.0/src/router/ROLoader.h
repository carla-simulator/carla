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
/// @file    ROLoader.h
/// @author  Daniel Krajzewicz
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// Loader for networks and route imports
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>
#include <utils/common/ValueTimeLine.h>
#include <utils/vehicle/SUMORouteLoaderControl.h>
#include <utils/xml/SAXWeightsHandler.h>
#include "RORoutable.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class ROAbstractEdgeBuilder;
class RONet;
class ROVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROLoader
 * @brief The data loader.
 *
 * Loads the network and route descriptions using further classes.
 *
 * Is capable to either load all routes in one step or go through them step wise.
 */
class ROLoader {
public:
    /** @brief Constructor
     *
     * @param[in] oc The options to use
     * @param[in] emptyDestinationsAllowed Whether trips may be given without destinations
     * @todo Recheck usage of emptyDestinationsAllowed
     */
    ROLoader(OptionsCont& oc, const bool emptyDestinationsAllowed, const bool logSteps);


    /// @brief Destructor
    virtual ~ROLoader();

    /// Loads the network
    virtual void loadNet(RONet& toFill, ROAbstractEdgeBuilder& eb);

    /// Loads the net weights
    bool loadWeights(RONet& net, const std::string& optionName,
                     const std::string& measure, const bool useLanes, const bool boundariesOverride);

    /** @brief Builds and opens all route loaders */
    void openRoutes(RONet& net);

    /** @brief Loads routes from all previously build route loaders */
    void processRoutes(const SUMOTime start, const SUMOTime end, const SUMOTime increment,
                       RONet& net, const RORouterProvider& provider);

protected:
    /** @brief Opens route handler of the given type
     *
     * Checks whether the given option name is known, returns true if
     *  not (this means that everything's ok, though the according
     *  handler is not built).
     *
     * Checks then whether the given option name is set and his value is one
     *  or a set of valid (existing) files. This is done via a call to
     *  "OptionsCont::isUsableFileList" (which generates a proper error
     *  message).
     *
     * If the given files are valid, the proper instance(s) is built using
     *  "buildNamedHandler" and if this could be done, it is added to
     *  the list of route handlers to use ("myHandler")
     *
     * Returns whether the wished handler(s) could be built.
     *
     * @param[in] optionName The name of the option that refers to which handler and which files shall be used
     * @param[in] net The net to assign to the built handlers
     * @return Whether the wished handler(s) could be built
     */
    bool openTypedRoutes(const std::string& optionName, RONet& net, const bool readAll = false);


    /**
     * @class EdgeFloatTimeLineRetriever_EdgeWeight
     * @brief Obtains edge weights from a weights handler and stores them within the edges
     * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever
     */
    class EdgeFloatTimeLineRetriever_EdgeWeight : public SAXWeightsHandler::EdgeFloatTimeLineRetriever {
    public:
        /// @brief Constructor
        EdgeFloatTimeLineRetriever_EdgeWeight(RONet& net) : myNet(net) {}

        /// @brief Destructor
        ~EdgeFloatTimeLineRetriever_EdgeWeight() { }

        /** @brief Adds an effort for a given edge and time period
         *
         * @param[in] id The id of the object to add a weight for
         * @param[in] val The weight
         * @param[in] beg The begin of the interval the weight is valid for
         * @param[in] end The end of the interval the weight is valid for
         * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever::addEdgeWeight
         */
        void addEdgeWeight(const std::string& id,
                           double val, double beg, double end) const;

    private:
        /// @brief The network edges shall be obtained from
        RONet& myNet;

    };


    /**
     * @class EdgeFloatTimeLineRetriever_EdgeTravelTime
     * @brief Obtains edge travel times from a weights handler and stores them within the edges
     * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever
     */
    class EdgeFloatTimeLineRetriever_EdgeTravelTime : public SAXWeightsHandler::EdgeFloatTimeLineRetriever {
    public:
        /// @brief Constructor
        EdgeFloatTimeLineRetriever_EdgeTravelTime(RONet& net) : myNet(net) {}

        /// @brief Destructor
        ~EdgeFloatTimeLineRetriever_EdgeTravelTime() {}

        /** @brief Adds a travel time for a given edge and time period
         *
         * @param[in] id The id of the object to add a weight for
         * @param[in] val The travel time
         * @param[in] beg The begin of the interval the weight is valid for
         * @param[in] end The end of the interval the weight is valid for
         * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever::addEdgeWeight
         */
        void addEdgeWeight(const std::string& id,
                           double val, double beg, double end) const;

    private:
        /// @brief The network edges shall be obtained from
        RONet& myNet;

    };



protected:
    void writeStats(const SUMOTime time, const SUMOTime start, const SUMOTime absNo, bool endGiven);


private:
    /// @brief Options to use
    OptionsCont& myOptions;

    /// @brief Information whether empty destinations are allowed
    const bool myEmptyDestinationsAllowed;

    /// @brief Information whether the routing steps should be logged
    const bool myLogSteps;

    /// @brief List of route loaders
    SUMORouteLoaderControl myLoaders;


private:
    /// @brief Invalidated copy constructor
    ROLoader(const ROLoader& src);

    /// @brief Invalidated assignment operator
    ROLoader& operator=(const ROLoader& src);
};
