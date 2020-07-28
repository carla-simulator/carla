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
/// @file    NIImporter_OpenStreetMap.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @author  Gregor Laemmel
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in OpenStreetMap format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/Parameterised.h>
#include <netbuild/NBPTPlatform.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBEdgeCont;
class NBNetBuilder;
class NBNode;
class NBNodeCont;
class NBTrafficLightLogicCont;
class NBTypeCont;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_OpenStreetMap
 * @brief Importer for networks stored in OpenStreetMap format
 *
 */
class NIImporter_OpenStreetMap {
public:
    /** @brief Loads content of the optionally given OSM file
     *
     * If the option "osm-files" is set, the file(s) stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "osm-files" is not set, this method simply returns.
     *
     * @param[in] oc The options to use
     * @param[in, out] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);

protected:
    /** @brief An internal representation of an OSM-node
     */
    struct NIOSMNode {
        NIOSMNode(long long int _id, double _lon, double _lat)
            :
            id(_id), lon(_lon), lat(_lat), ele(0),
            tlsControlled(false),
            railwayCrossing(false),
            railwaySignal(false),
            railwayBufferStop(false),
            ptStopPosition(false), ptStopLength(0), name(""),
            permissions(SVC_IGNORING),
            positionMeters(std::numeric_limits<double>::max()),
            node(0) { }

        /// @brief The node's id
        const long long int id;
        /// @brief The longitude the node is located at
        const double lon;
        /// @brief The latitude the node is located at
        const double lat;
        /// @brief The elevation of this node
        double ele;
        /// @brief Whether this is a tls controlled junction
        bool tlsControlled;
        /// @brief Whether this is a railway crossing
        bool railwayCrossing;
        /// @brief Whether this is a railway (main) signal
        bool railwaySignal;
        /// @brief Whether this is a railway buffer stop
        bool railwayBufferStop;
        /// @brief Whether this is a public transport stop position
        bool ptStopPosition;
        /// @brief The length of the pt stop
        double ptStopLength;
        /// @brief The name of the node
        std::string name;
        /// @brief type of pt stop
        SVCPermissions permissions;
        /// @brief kilometrage/mileage
        std::string position;
        /// @brief position converted to m (using highest precision available)
        double positionMeters;
        /// @brief the NBNode that was instantiated
        NBNode* node;

    private:
        /// invalidated assignment operator
        NIOSMNode& operator=(const NIOSMNode& s) = delete;


    };

public:
    /// @brief translate osm transport designations into sumo vehicle class
    static SUMOVehicleClass interpretTransportType(const std::string& type, NIOSMNode* toSet = nullptr);

protected:


    /** @enum CycleWayType
     * @brief details on the kind of cycleway along this road
     */
    enum WayType {
        WAY_NONE = 0,
        WAY_FORWARD = 1,
        WAY_BACKWARD = 2,
        WAY_BOTH = WAY_FORWARD | WAY_BACKWARD,
        WAY_UNKNOWN = 4
    };

    enum ParkingType {
        PARKING_NONE = 0,
        PARKING_LEFT = 1,
        PARKING_RIGHT = 2,
        PARKING_BOTH = WAY_FORWARD | WAY_BACKWARD,
        PARKING_UNKNOWN = 4,
        PARKING_FORBIDDEN = 8,
        PARKING_PERPENDICULAR = 16,
        PARKING_DIAGONAL = 32
    };


    /** @brief An internal definition of a loaded edge
     */
    struct Edge : public Parameterised {

        explicit Edge(long long int _id)
            :
            id(_id), myNoLanes(-1), myNoLanesForward(0),
            myMaxSpeed(MAXSPEED_UNGIVEN),
            myMaxSpeedBackward(MAXSPEED_UNGIVEN),
            myCyclewayType(WAY_UNKNOWN), // building of extra lane depends on bikelaneWidth of loaded typemap
            myBuswayType(WAY_NONE), // buslanes are always built when declared
            mySidewalkType(WAY_UNKNOWN), // building of extra lanes depends on sidewalkWidth of loaded typemap
            myRailDirection(WAY_UNKNOWN), // store direction(s) of railway usage
            myParkingType(PARKING_NONE), // parking areas exported optionally
            myLayer(0), // layer is non-zero only in conflict areas
            myCurrentIsRoad(false),
            myCurrentIsPlatform(false),
            myCurrentIsElectrified(false)
        { }


        /// @brief The edge's id
        const long long int id;
        /// @brief The edge's street name
        std::string streetName;
        /// @brief The edge's track name
        std::string ref;
        /// @brief number of lanes, or -1 if unknown
        int myNoLanes;
        /// @brief number of lanes in forward direction or 0 if unknown, negative if backwards lanes are meant
        int myNoLanesForward;
        /// @brief maximum speed in km/h, or MAXSPEED_UNGIVEN
        double myMaxSpeed;
        /// @brief maximum speed in km/h, or MAXSPEED_UNGIVEN
        double myMaxSpeedBackward;
        /// @brief The type, stored in "highway" key
        std::string myHighWayType;
        /// @brief Information whether this is an one-way road
        std::string myIsOneWay;
        /// @brief Information about the kind of cycleway along this road
        WayType myCyclewayType;
        /// @brief Information about the kind of busway along this road
        WayType myBuswayType;
        /// @brief Information about the kind of sidwalk along this road
        WayType mySidewalkType;
        /// @brief Information about the direction(s) of railway usage
        WayType myRailDirection;
        /// @brief Information about road-side parking
        int myParkingType;
        /// @brief Information about the relative z-ordering of ways
        int myLayer;
        /// @brief The list of nodes this edge is made of
        std::vector<long long int> myCurrentNodes;
        /// @brief Information whether this is a road
        bool myCurrentIsRoad;
        /// @brief Information whether this is a pt platform
        bool myCurrentIsPlatform;
        /// @brief Information whether this is railway is electrified
        bool myCurrentIsElectrified;

    private:
        /// invalidated assignment operator
        Edge& operator=(const Edge& s) = delete;


    };


    NIImporter_OpenStreetMap();

    ~NIImporter_OpenStreetMap();

    void load(const OptionsCont& oc, NBNetBuilder& nb);

private:
    /** @brief Functor which compares two NIOSMNodes according
     * to their coordinates
     */
    class CompareNodes {
    public:
        bool operator()(const NIOSMNode* n1, const NIOSMNode* n2) const {
            return (n1->lat > n2->lat) || (n1->lat == n2->lat && n1->lon > n2->lon);
        }
    };


    /// @brief The separator within newly created compound type names
    static const std::string compoundTypeSeparator;

    class CompareEdges;

    /** @brief the map from OSM node ids to actual nodes
     * @note: NIOSMNodes may appear multiple times due to substition
     */
    std::map<long long int, NIOSMNode*> myOSMNodes;

    /// @brief the set of unique nodes used in NodesHandler, used when freeing memory
    std::set<NIOSMNode*, CompareNodes> myUniqueNodes;


    /** @brief the map from OSM way ids to edge objects */
    std::map<long long int, Edge*> myEdges;

    /** @brief the map from OSM way ids to platform shapes */
    std::map<long long int, Edge*> myPlatformShapes;

    /** @brief the map from stop_area relations to member node */
    std::map<long long int, std::set<long long int> > myStopAreas;

    /// @brief The compounds types that do not contain known types
    std::set<std::string> myUnusableTypes;

    /// @brief The compound types that have already been mapped to other known types
    std::map<std::string, std::string> myKnownCompoundTypes;

    /** @brief Builds an NBNode
     *
     * If a node with the given id is already known, nothing is done.
     *  Otherwise, the position and other information of the node is retrieved from the
     *  given node map, the node is built and added to the given node container.
     * If the node is controlled by a tls, the according tls is built and added
     *  to the tls container.
     * @param[in] id The id of the node to build
     * @param[in] osmNodes Map of node ids to information about these
     * @param[in, out] nc The node container to add the built node to
     * @param[in, out] tlsc The traffic lights logic container to add the built tls to
     * @return The built/found node
     * @exception ProcessError If the tls could not be added to the container
     */
    NBNode* insertNodeChecking(long long int id, NBNodeCont& nc, NBTrafficLightLogicCont& tlsc);


    /** @brief Builds an NBEdge
     *
     * @param[in] e The definition of the edge
     * @param[in] index The index of the edge (in the case it is split along her nodes)
     * @param[in] from The origin node of the edge
     * @param[in] to The destination node of the edge
     * @param[in] passed The list of passed nodes (geometry information)
     * @param[in] osmNodes Container of node definitions for getting information about nodes from
     * @param[in, out] The NetBuilder instance
     * @return the new index if the edge is split
     * @exception ProcessError If the edge could not be added to the container
     */
    int insertEdge(Edge* e, int index, NBNode* from, NBNode* to,
                   const std::vector<long long int>& passed, NBNetBuilder& nb);

    /// @brief reconstruct elevation from layer info
    void reconstructLayerElevation(double layerElevation, NBNetBuilder& nb);

    /// @brief collect neighboring nodes with their road distance and maximum between-speed. Search does not continue beyond knownElevation-nodes
    std::map<NBNode*, std::pair<double, double> >
    getNeighboringNodes(NBNode* node, double maxDist, const std::set<NBNode*>& knownElevation);

    /// @brief check whether the type is known or consists of known type compounds. return empty string otherwise
    std::string usableType(const std::string& type, const std::string& id, NBTypeCont& tc);

    /// @brief extend kilometrage data for all nodes along railway
    void extendRailwayDistances(Edge* e, NBTypeCont& tc);

    /// @brief read distance value from node and return value in m
    static double interpretDistance(NIOSMNode* node);

protected:
    static const double MAXSPEED_UNGIVEN;
    static const long long int INVALID_ID;

    /**
     * @class NodesHandler
     * @brief A class which extracts OSM-nodes from a parsed OSM-file
     */
    friend class NodesHandler;
    class NodesHandler : public SUMOSAXHandler {
    public:
        /** @brief Contructor
         * @param[in, out] toFill The nodes container to fill
         * @param[in, out] uniqueNodes The nodes container for ensuring uniqueness
         * @param[in] options The options to use
         */
        NodesHandler(std::map<long long int, NIOSMNode*>& toFill, std::set<NIOSMNode*,
                     CompareNodes>& uniqueNodes,
                     const OptionsCont& cont);


        /// @brief Destructor
        ~NodesHandler() override;

        int getDuplicateNodes() const {
            return myDuplicateNodes;
        }

    protected:
        /// @name inherited from GenericSAXHandler
        //@{

        /** @brief Called on the opening of a tag;
         *
         * @param[in] element ID of the currently opened element
         * @param[in] attrs Attributes within the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myStartElement
         */
        void myStartElement(int element, const SUMOSAXAttributes& attrs) override;


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element) override;
        //@}


    private:

        /// @brief The nodes container to fill
        std::map<long long int, NIOSMNode*>& myToFill;

        /// @brief ID of the currently parsed node, for reporting mainly
        long long int myLastNodeID;

        /// @brief Hierarchy helper for parsing a node's tags
        bool myIsInValidNodeTag;

        /// @brief The current hierarchy level
        int myHierarchyLevel;

        /// @brief the set of unique nodes (used for duplicate detection/substitution)
        std::set<NIOSMNode*, CompareNodes>& myUniqueNodes;

        /// @brief whether elevation data should be imported
        const bool myImportElevation;

        /// @brief number of diplic
        int myDuplicateNodes;

        /// @brief the options
        const OptionsCont& myOptionsCont;


    private:
        /** @brief invalidated copy constructor */
        NodesHandler(const NodesHandler& s);

        /** @brief invalidated assignment operator */
        NodesHandler& operator=(const NodesHandler& s);

    };


    /**
     * @class EdgesHandler
     * @brief A class which extracts OSM-edges from a parsed OSM-file
     */
    class EdgesHandler : public SUMOSAXHandler {
    public:
        /** @brief Constructor
         *
         * @param[in] osmNodes The previously parsed (osm-)nodes
         * @param[in, out] toFill The edges container to fill with read edges
         */
        EdgesHandler(const std::map<long long int, NIOSMNode*>& osmNodes,
                     std::map<long long int, Edge*>& toFill, std::map<long long int, Edge*>& platformShapes);


        /// @brief Destructor
        ~EdgesHandler() override;


    protected:
        /// @name inherited from GenericSAXHandler
        //@{

        /** @brief Called on the opening of a tag;
         *
         * @param[in] element ID of the currently opened element
         * @param[in] attrs Attributes within the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myStartElement
         */
        void myStartElement(int element, const SUMOSAXAttributes& attrs) override;


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element) override;
        //@}

        double interpretSpeed(const std::string& key, std::string value);

    private:
        /// @brief The previously parsed nodes
        const std::map<long long int, NIOSMNode*>& myOSMNodes;

        /// @brief A map of built edges
        std::map<long long int, Edge*>& myEdgeMap;

        /// @brief A map of built edges
        std::map<long long int, Edge*>& myPlatformShapesMap;

        /// @brief The currently built edge
        Edge* myCurrentEdge;

        /// @brief The element stack
        std::vector<int> myParentElements;

        /// @brief A map of non-numeric speed descriptions to their numeric values
        std::map<std::string, double> mySpeedMap;

        /// @brief whether additional way attributes shall be added to the edge
        bool myAllAttributes;

    private:
        /** @brief invalidated copy constructor */
        EdgesHandler(const EdgesHandler& s);

        /** @brief invalidated assignment operator */
        EdgesHandler& operator=(const EdgesHandler& s);

    };

    /**
     * @class RelationHandler
     * @brief A class which extracts relevant relation information from a parsed OSM-file
     *   - turn restrictions
     */
    class RelationHandler : public SUMOSAXHandler {
    public:
        /** @brief Constructor
         *
         * @param[in] osmNodes The previously parsed OSM-nodes
         * @param[in] osmEdges The previously parse OSM-edges
         */
        RelationHandler(const std::map<long long int, NIOSMNode*>& osmNodes,
                        const std::map<long long int, Edge*>& osmEdges, NBPTStopCont* nbptStopCont,
                        const std::map<long long int, Edge*>& platfromShapes, NBPTLineCont* nbptLineCont,
                        const OptionsCont& oc);


        /// @brief Destructor
        ~RelationHandler() override;

    protected:
        /// @name inherited from GenericSAXHandler
        //@{

        /** @brief Called on the opening of a tag;
         *
         * @param[in] element ID of the currently opened element
         * @param[in] attrs Attributes within the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myStartElement
         */
        void myStartElement(int element, const SUMOSAXAttributes& attrs) override;


        /** @brief Called when a closing tag occurs
         *
         * @param[in] element ID of the currently opened element
         * @exception ProcessError If something fails
         * @see GenericSAXHandler::myEndElement
         */
        void myEndElement(int element) override;
        //@}


    private:
        /// @brief The previously parsed nodes
        const std::map<long long int, NIOSMNode*>& myOSMNodes;

        /// @brief The previously parsed edges
        const std::map<long long int, Edge*>& myOSMEdges;

        /// @brief The previously parsed platform shapes
        const std::map<long long int, Edge*>& myPlatformShapes;

        /// @brief The previously filled pt stop container
        NBPTStopCont* myNBPTStopCont;

        /// @brief PT Line container to be filled
        NBPTLineCont* myNBPTLineCont;

        /// @brief The currently parsed relation
        long long int myCurrentRelation;

        /// @brief The element stack
        std::vector<int> myParentElements;

        /// @brief whether the currently parsed relation is a restriction
        bool myIsRestriction;

        /// @brief the origination way for the current restriction
        long long int myFromWay;

        /// @brief the destination way for the current restriction
        long long int myToWay;

        /// @brief the via node/way for the current restriction
        long long int myViaNode;
        long long int myViaWay;


        /// @brief the options cont
        const OptionsCont& myOptionsCont;

        /** @enum RestrictionType
         * @brief whether the only allowed or the only forbidden connection is defined
         */
        enum RestrictionType {
            /// @brief The only valid connection is declared
            RESTRICTION_ONLY,
            /// @brief The only invalid connection is declared
            RESTRICTION_NO,
            /// @brief The relation tag was missing
            RESTRICTION_UNKNOWN
        };
        RestrictionType myRestrictionType;

        /// @brief reset members to their defaults for parsing a new relation
        void resetValues();

        /// @brief check whether a referenced way has a corresponding edge
        bool checkEdgeRef(long long int ref) const;

        /// @brief try to apply the parsed restriction and return whether successful
        bool applyRestriction() const;

        /// @brief try to find the way segment among candidates
        NBEdge* findEdgeRef(long long int wayRef, const std::vector<NBEdge*>& candidates) const;

    private:
        /** @brief invalidated copy constructor */
        RelationHandler(const RelationHandler& s);

        /** @brief invalidated assignment operator */
        RelationHandler& operator=(const RelationHandler& s);

        /// @brief bus stop references
        std::vector<long long int> myStops;


        struct NIIPTPlatform {
            long long int ref;
            bool isWay;
        };

        /// @brief bus stop platforms
        std::vector<NIIPTPlatform> myPlatforms;

        /// @brief ways in pt line references
        std::vector<long long int> myWays;

        /// @brief indicates whether current relation is a pt stop area
        bool myIsStopArea;

        /// @brief indicates whether current relation is a route
        bool myIsRoute;

        /// @brief indicates whether current relation is a pt route
        std::string myPTRouteType;

        /// @brief name of the relation
        std::string myName;

        /// @brief ref of the pt line
        std::string myRef;

        /// @brief service interval of the pt line in minutes
        int myInterval;

        /// @brief night service information of the pt line
        std::string myNightService;
    };

};
