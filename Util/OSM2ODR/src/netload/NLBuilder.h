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
/// @file    NLBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// The main interface for loading a microsim
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <utils/xml/SAXWeightsHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class NLContainer;
class MSJunctionLogic;
class MSDetectorControl;
class OptionsCont;
class NLHandler;
class NLEdgeControlBuilder;
class NLJunctionControlBuilder;
class NLDetectorBuilder;
class NLTriggerBuilder;
class SUMORouteLoader;
class SUMORouteLoaderControl;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NLBuilder
 * @brief The main interface for loading a microsim
 *
 * It is a black-box where only the options and factories must be supplied
 *  on the constructor call. An (empty) instance of the network must be
 *  supplied, too, and is filled during loading.
 */
class NLBuilder {
public:
    /** @brief Constructor
     *
     * @param[in] oc The options to use
     * @param[in, out] net The network to fill
     * @param[in] eb The builder of edges to use
     * @param[in] jb The builder of junctions to use
     * @param[in] db The detector builder to use
     * @param[in] tb The trigger builder to use
     * @param[in] xmlHandler The xml handler to use
     */
    NLBuilder(OptionsCont& oc, MSNet& net,
              NLEdgeControlBuilder& eb, NLJunctionControlBuilder& jb,
              NLDetectorBuilder& db,
              NLHandler& xmlHandler);


    /// @brief Destructor
    virtual ~NLBuilder();


    /** @brief Builds and initialises the simulation
     *
     * At first, the network is loaded and the built using "buildNet".
     *  If this could be done, additional information is loaded (state dump,
     *  weight files, route files, and additional files).
     * If everything could be done, true is returned, otherwise false.
     *
     * @see buildNet
     * @exception ProcessError If something fails on network building
     * @todo Again, both returning a bool and throwing an exception; quite inconsistent
     */
    virtual bool build();

    /**
    * loads the net, additional routes and the detectors
    */
    static MSNet* init(const bool isLibsumo = false);

    /// @brief initializes all RNGs
    static void initRandomness();


protected:
    /** @brief Loads a described subpart form the given list of files
     *
     * Assuming the given string to be an option name behind which a list of files
     *  is stored, this method invokes an XML reader on all the files set for this option.
     * @param[in] mmlWhat The option to get the file list from
     * @param[in] isNet   whether a network gets loaded
     * @return Whether loading of all files was successfull
     */
    bool load(const std::string& mmlWhat, const bool isNet = false);


    /** @brief Closes the net building process
     *
     * Builds the microsim-structures which belong to a MSNet using the factories
     *  filled while loading. Initialises the network using these structures by calling
     *  MSNet::closeBuilding.
     * If an error occurs, all built structures are deleted and a ProcessError is thrown.
     * @exception ProcessError If the loaded structures could not be built
     */
    void buildNet();


    /** @brief Builds the route loader control
     *
     * Goes through the list of route files to open defined in the option
     *  "route-files" and builds loaders reading these files
     * @param[in] oc The options to read the list of route files to open from
     * @return The built route loader control
     * @exception ProcessError If an error occurred
     */
    SUMORouteLoaderControl* buildRouteLoaderControl(const OptionsCont& oc);




    /**
     * @class EdgeFloatTimeLineRetriever_EdgeTravelTime
     * @brief Obtains edge efforts from a weights handler and stores them within the edges
     * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever
     */
    class EdgeFloatTimeLineRetriever_EdgeEffort : public SAXWeightsHandler::EdgeFloatTimeLineRetriever {
    public:
        /// @brief Constructor
        EdgeFloatTimeLineRetriever_EdgeEffort(MSNet& net) : myNet(net) {}

        /// @brief Destructor
        ~EdgeFloatTimeLineRetriever_EdgeEffort() { }

        /** @brief Adds an effort for a given edge and time period
         *
         * @param[in] id The id of the object to add a weight for
         * @param[in] val The effort
         * @param[in] beg The begin of the interval the weight is valid for
         * @param[in] end The end of the interval the weight is valid for
         * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever::addEdgeWeight
         */
        void addEdgeWeight(const std::string& id,
                           double val, double beg, double end) const;

    private:
        /// @brief The network edges shall be obtained from
        MSNet& myNet;

    };


    /**
     * @class EdgeFloatTimeLineRetriever_EdgeTravelTime
     * @brief Obtains edge travel times from a weights handler and stores them within the edges
     * @see SAXWeightsHandler::EdgeFloatTimeLineRetriever
     */
    class EdgeFloatTimeLineRetriever_EdgeTravelTime : public SAXWeightsHandler::EdgeFloatTimeLineRetriever {
    public:
        /// @brief Constructor
        EdgeFloatTimeLineRetriever_EdgeTravelTime(MSNet& net) : myNet(net) {}

        /// @brief Destructor
        ~EdgeFloatTimeLineRetriever_EdgeTravelTime() { }

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
        MSNet& myNet;

    };


protected:
    /// @brief The options to get the names of the files to load and further information from
    OptionsCont& myOptions;

    /// @brief The edge control builder to use
    NLEdgeControlBuilder& myEdgeBuilder;

    /// @brief The junction control builder to use
    NLJunctionControlBuilder& myJunctionBuilder;

    /// @brief The detector control builder to use
    NLDetectorBuilder& myDetectorBuilder;

    /// @brief The net to fill
    MSNet& myNet;

    /// @brief The handler used to parse the net
    NLHandler& myXMLHandler;


private:
    /// @brief invalidated copy operator
    NLBuilder(const NLBuilder& s);

    /// @brief invalidated assignment operator
    NLBuilder& operator=(const NLBuilder& s);

};
