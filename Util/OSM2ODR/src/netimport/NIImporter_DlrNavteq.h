/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIImporter_DlrNavteq.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in Elmar's format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/common/UtilExceptions.h>
#include <utils/importio/LineHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdgeCont;
class NBNetBuilder;
class NBNodeCont;
class NBTrafficLightLogicCont;
class NBTypeCont;
class OptionsCont;
class PositionVector;
class StringTokenizer;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_DlrNavteq
 * @brief Importer for networks stored in Elmar's format
 *
 */
class NIImporter_DlrNavteq {
public:
    /** @brief Loads content of the optionally given dlr-navteq (aka Elmar-fomat) folder
     *
     * If the option "dlr-navteq-prefix" is set, the file(s) stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "dlr-navteq-prefix" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);

    /// @brief scaling factor for geo coordinates (DLRNavteq format uses this to increase floating point precisions)
    static const std::string GEO_SCALE;

    /// @brief magic value for undefined stuff
    static const std::string UNDEFINED;

protected:
    /**
     * @class NodesHandler
     * @brief Importer of nodes stored in unsplit elmar format
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain node definitions
     * in DLRNavteq's unsplit format.
     */
    class NodesHandler : public LineHandler {
    public:
        /** @brief Constructor
         * @param[in, filled] nc The node control to insert loaded nodes into
         * @param[in] file The name of the parsed file
         * @param[in, geoms] geoms Storage for read edge geometries
         */
        NodesHandler(NBNodeCont& nc, const std::string& file,
                     std::map<std::string, PositionVector>& geoms);


        /// @brief Destructor
        ~NodesHandler();


        /** @brief Parsing method
         *
         * Implementation of the LineHandler-interface called by a LineReader;
         *  interprets the retrieved information and stores it into "myNodeCont".
         * Additionally, edge geometries are parsed and stored into "myGeoms".
         *
         * @param[in] result The read line
         * @return Whether the parsing shall continue
         * @exception ProcessError if something fails
         * @see LineHandler::report
         */
        bool report(const std::string& result);


    protected:
        // @brief The node container to store parsed nodes into
        NBNodeCont& myNodeCont;

        /// @brief A container for parsed geometries
        std::map<std::string, PositionVector>& myGeoms;


    private:
        /// @brief Invalidated copy constructor.
        NodesHandler(const NodesHandler&);

        /// @brief Invalidated assignment operator.
        NodesHandler& operator=(const NodesHandler&);

    };


    /**
     * @class EdgesHandler
     * @brief Importer of edges stored in unsplit elmar format
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain edge definitions
     * in DLRNavteq's unsplit format.
     */
    class EdgesHandler : public LineHandler {

    public:
        /** @brief Constructor
         * @param[in] nc The node control to retrieve nodes from
         * @param[in, filled] ec The edge control to insert loaded edges into
         * @param[in] tc The type control to retrieve types from
         * @param[in] file The name of the parsed file
         * @param[in] geoms The previously read edge geometries
         * @param[in] streetNames The previously read street names
         */
        EdgesHandler(NBNodeCont& nc, NBEdgeCont& ec, NBTypeCont& tc,
                     const std::string& file,
                     std::map<std::string, PositionVector>& geoms,
                     std::map<std::string, std::string>& streetNames);


        /// @brief Destructor
        ~EdgesHandler();


        /** @brief Parsing method
             *
             * Implementation of the LineHandler-interface called by a LineReader;
             * interprets the retrieved information and stores it into "myEdgeCont".
             * @param[in] result The read line
             * @return Whether the parsing shall continue
             * @exception ProcessError if something fails
             * @see LineHandler::report
         */
        bool report(const std::string& result);


    protected:
        /// @brief The node container to get the referenced nodes from
        NBNodeCont& myNodeCont;

        /// @brief The edge container to store loaded edges into
        NBEdgeCont& myEdgeCont;

        /// @brief The type container to retrieve type info from
        NBTypeCont& myTypeCont;

        /// @brief Previously read edge geometries (manipulated during use)
        std::map<std::string, PositionVector>& myGeoms;

        /// @brief Previously read streat names (non-const because operate[] is more convenient)
        std::map<std::string, std::string>& myStreetNames;

        /// @brief Whether node positions shall not be added to the edge's geometry
        bool myTryIgnoreNodePositions;

        /// @brief version number of current file
        double myVersion;

        /// @brief the version number of the edge file being parsed
        std::vector<int> myColumns;

        /// @brief the file being parsed
        const std::string myFile;

        static const int MISSING_COLUMN;

        enum ColumnName {
            LINK_ID = 0,
            NODE_ID_FROM,
            NODE_ID_TO,
            BETWEEN_NODE_ID,
            LENGTH,
            VEHICLE_TYPE,
            FORM_OF_WAY,
            BRUNNEL_TYPE,
            FUNCTIONAL_ROAD_CLASS,
            SPEED_CATEGORY,
            NUMBER_OF_LANES,
            SPEED_LIMIT,
            SPEED_RESTRICTION,
            NAME_ID1_REGIONAL,
            NAME_ID2_LOCAL,
            HOUSENUMBERS_RIGHT,
            HOUSENUMBERS_LEFT,
            ZIP_CODE,
            AREA_ID,
            SUBAREA_ID,
            THROUGH_TRAFFIC,
            SPECIAL_RESTRICTIONS,
            EXTENDED_NUMBER_OF_LANES,
            ISRAMP,
            CONNECTION
        };

        std::string getColumn(const StringTokenizer& st, ColumnName name, const std::string fallback = "");

    private:
        /// @brief build the street name for the given ids
        std::string getStreetNameFromIDs(const std::string& regionalID, const std::string& localID) const;


    private:
        /// @brief Invalidated copy constructor.
        EdgesHandler(const EdgesHandler&);

        /// @brief Invalidated assignment operator.
        EdgesHandler& operator=(const EdgesHandler&);

    };


    /**
     * @class TrafficlightsHandler
     * @brief Importer of traffic lights stored in DLRNavteq's (aka elmar) format
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain traffic light definitions
     * in DLRNavteq's format.
     */
    class TrafficlightsHandler : public LineHandler {
    public:
        /** @brief Constructor
         * @param[in] nc The node control to retrieve nodes from
         * @param[in, filled] tlc The traffic lights container to fill
         * @param[in] file The name of the parsed file
         */
        TrafficlightsHandler(NBNodeCont& nc, NBTrafficLightLogicCont& tlc,
                             NBEdgeCont& ne, const std::string& file);


        /// @brief Destructor
        ~TrafficlightsHandler();


        /** @brief Parsing method
         *
         * Implementation of the LineHandler-interface called by a LineReader;
         * interprets the retrieved information and alters the nodes.
         * @param[in] result The read line
         * @return Whether the parsing shall continue
         * @exception ProcessError if something fails
         * @see LineHandler::report
         */
        bool report(const std::string& result);


    protected:
        /// @brief The node container to get the referenced nodes from
        NBNodeCont& myNodeCont;

        /// @brief The traffic lights container to add built tls to
        NBTrafficLightLogicCont& myTLLogicCont;

        /// @brief The edge container to get the referenced edges from
        NBEdgeCont& myEdgeCont;


    private:
        /// @brief Invalidated copy constructor.
        TrafficlightsHandler(const TrafficlightsHandler&);

        /// @brief Invalidated assignment operator.
        TrafficlightsHandler& operator=(const TrafficlightsHandler&);

    };


    /**
     * @class NamesHandler
     * @brief Importer of street names in DLRNavteq's (aka elmar) format
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain name definitions
     * in DLRNavteq's format.
     */
    class NamesHandler : public LineHandler {
    public:
        /** @brief Constructor
         * @param[in] file The name of the parsed file
         * @param[filled] streetNames output container for read names
         */
        NamesHandler(const std::string& file, std::map<std::string, std::string>& streetNames);


        /// @brief Destructor
        ~NamesHandler();


        /** @brief Parsing method
         *
         * Implementation of the LineHandler-interface called by a LineReader;
         * interprets the retrieved information and stores the streetNames
         * @param[in] result The read line
         * @return Whether the parsing shall continue
         * @exception ProcessError if something fails
         * @see LineHandler::report
         */
        bool report(const std::string& result);


    protected:
        /// @brief The container for storing read names
        std::map<std::string, std::string>& myStreetNames;


    private:
        /// @brief Invalidated copy constructor.
        NamesHandler(const NamesHandler&);

        /// @brief Invalidated assignment operator.
        NamesHandler& operator=(const NamesHandler&);

    };


    /**
     * @class TimeRestrictionsHandler
     * @brief Importer of street names in DLRNavteq's (aka elmar) format
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain name definitions
     * in DLRNavteq's format.
     */
    class TimeRestrictionsHandler : public LineHandler {
    public:
        /** @brief Constructor
         * @param[in] file The name of the parsed file
         * @param[filled] streetNames output container for read names
         */
        TimeRestrictionsHandler(NBEdgeCont& ec, NBDistrictCont& dc, time_t constructionTime);


        /// @brief Destructor
        ~TimeRestrictionsHandler();


        /** @brief Parsing method
         *
         * Implementation of the LineHandler-interface called by a LineReader;
         * interprets the retrieved information and stores the streetNames
         * @param[in] result The read line
         * @return Whether the parsing shall continue
         * @exception ProcessError if something fails
         * @see LineHandler::report
         */
        bool report(const std::string& result);

        void printSummary();


    protected:
        /// @brief The edge container
        NBEdgeCont& myEdgeCont;
        NBDistrictCont& myDistrictCont;

        /// @brief The date for which to build the network (in case some edges are still under construction)
        time_t myConstructionTime;
        time_t myCS_min;
        time_t myCS_max;
        int myConstructionEntries;
        int myNotStarted;
        int myUnderConstruction;
        int myFinished;
        int myRemovedEdges; // only counts those not already removed through other options


    private:
        /// @brief Invalidated copy constructor.
        TimeRestrictionsHandler(const TimeRestrictionsHandler&);

        /// @brief Invalidated assignment operator.
        TimeRestrictionsHandler& operator=(const TimeRestrictionsHandler&);

    };


    /**
     * @class ProhibitionHandler
     * @brief Imports prohibitions regarding connectivity
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain prohibited manoeuver definitions
     * in DLRNavteq's format.
     */
    class ProhibitionHandler : public LineHandler {
    public:
        /** @brief Constructor
         * @param[in] file The name of the parsed file
         * @param[filled] streetNames output container for read names
         */
        ProhibitionHandler(NBEdgeCont& ne, const std::string& file, time_t constructionTime);


        /// @brief Destructor
        ~ProhibitionHandler();


        /** @brief Parsing method
         *
         * Implementation of the LineHandler-interface called by a LineReader;
         * interprets the retrieved information and stores the streetNames
         * @param[in] result The read line
         * @return Whether the parsing shall continue
         * @exception ProcessError if something fails
         * @see LineHandler::report
         */
        bool report(const std::string& result);


    protected:
        /// @brief The edge container to store loaded edges into
        NBEdgeCont& myEdgeCont;
        const std::string myFile;
        double myVersion;
        time_t myConstructionTime;


    private:
        /// @brief Invalidated copy constructor.
        ProhibitionHandler(const ProhibitionHandler&);

        /// @brief Invalidated assignment operator.
        ProhibitionHandler& operator=(const ProhibitionHandler&);

    };


    /**
     * @class ConnectedLanesHandler
     * @brief Imports prohibitions regarding connectivity
     *
     * Being a LineHandler, this class retrieves each line from a LineReader
     * and parses these information assuming they contain prohibited manoeuver definitions
     * in DLRNavteq's format.
     */
    class ConnectedLanesHandler : public LineHandler {
    public:
        /** @brief Constructor
         * @param[in] file The name of the parsed file
         * @param[filled] streetNames output container for read names
         */
        ConnectedLanesHandler(NBEdgeCont& ne);


        /// @brief Destructor
        ~ConnectedLanesHandler();


        /** @brief Parsing method
         *
         * Implementation of the LineHandler-interface called by a LineReader;
         * interprets the retrieved information and stores the streetNames
         * @param[in] result The read line
         * @return Whether the parsing shall continue
         * @exception ProcessError if something fails
         * @see LineHandler::report
         */
        bool report(const std::string& result);


    protected:
        /// @brief The edge container to store loaded edges into
        NBEdgeCont& myEdgeCont;


    private:
        /// @brief Invalidated copy constructor.
        ConnectedLanesHandler(const ConnectedLanesHandler&);

        /// @brief Invalidated assignment operator.
        ConnectedLanesHandler& operator=(const ConnectedLanesHandler&);

    };


    static double readVersion(const std::string& line, const std::string& file);
    static int readPrefixedInt(const std::string& s, const std::string& prefix, int fallBack = 0);
    static time_t readTimeRec(const std::string& start, const std::string& duration);
    static time_t readDate(const std::string& yyyymmdd);

};
