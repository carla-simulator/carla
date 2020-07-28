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
/// @file    NIImporter_SUMO.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in SUMO format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/common/Parameterised.h>
#include <netbuild/NBLoadedSUMOTLDef.h>
#include "NIXMLTypesHandler.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNetBuilder;
class NBEdge;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_SUMO
 * @brief Importer for networks stored in SUMO format
 *
 */
class NIImporter_SUMO : public SUMOSAXHandler {
public:
    /** @brief Loads content of the optionally given SUMO file
     *
     * If the option "sumo-net-file" is set, the file stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "sumo-net-file" is not set, this method simply returns.
     *
     * The loading is done by parsing the network definition as an XML file
     *  using the SAXinterface and handling the incoming data via this class'
     *  methods.
     *
     * @param[in,out] oc The options to use (option no-internal-links may be modified)
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(OptionsCont& oc, NBNetBuilder& nb);

    /// begins the reading of a traffic lights logic
    static NBLoadedSUMOTLDef* initTrafficLightLogic(const SUMOSAXAttributes& attrs, NBLoadedSUMOTLDef* currentTL);

    /// adds a phase to the traffic lights logic currently build
    static void addPhase(const SUMOSAXAttributes& attrs, NBLoadedSUMOTLDef* currentTL);

    /// Parses network location description and registers it with GeoConveHelper::setLoaded
    static GeoConvHelper* loadLocation(const SUMOSAXAttributes& attrs);

protected:
    /** @brief Constructor
     * @param[in] nc The network builder to fill
     */
    NIImporter_SUMO(NBNetBuilder& nb);

    /// @brief Destructor
    ~NIImporter_SUMO();

    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * In dependence to the obtained type, an appropriate parsing
     *  method is called ("addEdge" if an edge encounters, f.e.).
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element,
                        const SUMOSAXAttributes& attrs);


    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);
    //@}


private:
    /// @brief load the network
    void _loadNetwork(OptionsCont& oc);

    /// @name Object instance parsing methods
    //@{

    /** @brief Parses an edge and stores the values in "myCurrentEdge"
     * @param[in] attrs The attributes to get the edge's values from
     */
    void addEdge(const SUMOSAXAttributes& attrs);


    /** @brief Parses a lane and stores the values in "myCurrentLane"
     * @param[in] attrs The attributes to get the lane's values from
     */
    void addLane(const SUMOSAXAttributes& attrs);

    /** @brief parses stop offsets for the current lane or edge
     * @param[in] attrs The attributes to get the stop offset sepcifics from
     */
    void addStopOffsets(const SUMOSAXAttributes& attrs, bool& ok);

    /** @brief Parses a junction and saves it in the node control
     * @param[in] attrs The attributes to get the junction's values from
     */
    void addJunction(const SUMOSAXAttributes& attrs);


    /** @brief Parses a reques and saves selected attributes in myCurrentJunction
     * @param[in] attrs The attributes to get the junction's values from
     */
    void addRequest(const SUMOSAXAttributes& attrs);


    /** @brief Parses a connection and saves it
     *    into the lane's definition stored in "myCurrentLane"
     * @param[in] attrs The attributes to get the connection from
     */
    void addConnection(const SUMOSAXAttributes& attrs);

    /** @brief Parses a prohibition and saves it
     * @param[in] attrs The attributes to get the connection from
     */
    void addProhibition(const SUMOSAXAttributes& attrs);

    /** @brief Parses a roundabout and stores it in myEdgeCont.
     * @param[in] attrs The attributes to get the roundabouts values from
     */
    void addRoundabout(const SUMOSAXAttributes& attrs);

    //@}



private:
    /**
     * @struct Connection
     * @brief A connection description.
     */
    struct Connection : public Parameterised {
        /// @brief The id of the target edge
        std::string toEdgeID;
        /// @brief The index of the target lane
        int toLaneIdx;
        /// @brief The id of the traffic light that controls this connection
        std::string tlID;
        /// @brief The index of this connection within the controlling traffic light
        int tlLinkIndex;
        int tlLinkIndex2;
        /// @brief Information about being definitely free to drive (on-ramps)
        bool mayDefinitelyPass;
        /// @brief Whether the junction must be kept clear coming from this connection
        bool keepClear;
        /// @brief custom position for internal junction on this connection
        double contPos;
        /// @brief custom foe visibility for connection
        double visibility;
        /// @brief custom permissions for connection
        SVCPermissions permissions;
        /// @brief custom speed for connection
        double speed;
        /// @brief custom length for connection
        double customLength;
        /// @brief custom shape connection
        PositionVector customShape;
        /// @brief if set to true, This connection will not be TLS-controlled despite its node being controlled.
        bool uncontrolled;
    };


    /** @struct LaneAttrs
     * @brief Describes the values found in a lane's definition
     */
    struct LaneAttrs : public Parameterised {
        /// @brief The maximum velocity allowed on this lane
        double maxSpeed;
        /// @brief This lane's shape (may be custom)
        PositionVector shape;
        /// @brief This lane's connections
        std::vector<Connection> connections;
        /// @brief This lane's allowed vehicle classes
        std::string allow;
        /// @brief This lane's disallowed vehicle classes
        std::string disallow;
        /// @brief The width of this lane
        double width;
        /// @brief This lane's offset from the intersection
        double endOffset;
        /// @brief This lane's vehicle specific stop offsets
        std::map<SVCPermissions, double> stopOffsets;
        /// @brief Whether this lane is an acceleration lane
        bool accelRamp;
        /// @brief This lane's opposite lane
        std::string oppositeID;
        /// @brief Whether this lane has a custom shape
        bool customShape;
        /// @brief the type of this lane
        std::string type;
    };


    /** @struct EdgeAttrs
     * @brief Describes the values found in an edge's definition and this edge's lanes
     */
    struct EdgeAttrs : public Parameterised {
        /// @brief This edge's id
        std::string id;
        /// @brief This edge's street name
        std::string streetName;
        /// @brief This edge's type
        std::string type;
        /// @brief This edge's function
        SumoXMLEdgeFunc func;
        /// @brief The node this edge starts at
        std::string fromNode;
        /// @brief The node this edge ends at
        std::string toNode;
        /// @brief This edges's shape
        PositionVector shape;
        /// @brief The length of the edge if set explicitly
        double length;
        /// @brief This edge's priority
        int priority;
        /// @brief The maximum velocity allowed on this edge (!!!)
        double maxSpeed;
        /// @brief This edge's lanes
        std::vector<LaneAttrs*> lanes;
        /// @brief The built edge
        NBEdge* builtEdge;
        /// @brief The lane spread function
        LaneSpreadFunction lsf;
        /// @brief This edge's vehicle specific stop offsets (used for lanes, that do not have a specified stopOffset)
        std::map<SVCPermissions, double> stopOffsets;
        /// @brief The position at the start of this edge (kilometrage/mileage)
        double distance;
    };


    /** @struct Prohibition
     * @brief Describes the values found in a prohibition
     */
    struct Prohibition {
        std::string prohibitorFrom;
        std::string prohibitorTo;
        std::string prohibitedFrom;
        std::string prohibitedTo;
    };

    /** @struct Crossing
     * @brief Describes a pedestrian crossing
     */
    struct Crossing {
        Crossing(const std::string& _edgeID) :
            edgeID(_edgeID), customTLIndex(-1), customTLIndex2(-1) {}

        std::string edgeID;
        std::vector<std::string> crossingEdges;
        double width;
        bool priority;
        PositionVector customShape;
        int customTLIndex;
        int customTLIndex2;
    };

    /** @struct WalkingAreaParsedCustomShape
     * @brief Describes custom shape for a walking area during parsing
     */
    struct WalkingAreaParsedCustomShape {
        PositionVector shape;
        std::vector<std::string> fromEdges;
        std::vector<std::string> toEdges;
        std::vector<std::string> fromCrossed;
        std::vector<std::string> toCrossed;
    };

    /** @struct JunctionAttrs
     * @brief Describes the values found in a junction
     */
    struct JunctionAttrs {
        NBNode* node;
        // @the list of internal lanes corresponding to each link
        std::vector<std::string> intLanes;
        // @brief the complete response definition for all links
        std::vector<std::string> response;
    };

    /// @brief Loaded edge definitions
    std::map<std::string, EdgeAttrs*> myEdges;

    /// @brief Loaded prohibitions
    std::vector<Prohibition> myProhibitions;

    /// @brief The network builder to fill
    NBNetBuilder& myNetBuilder;

    /// @brief The node container to fill
    NBNodeCont& myNodeCont;

    /// @brief The node container to fill
    NBTrafficLightLogicCont& myTLLCont;

    /// @brief The handler for parsing edge types and restrictions
    NIXMLTypesHandler myTypesHandler;

    /// @brief The currently parsed edge's definition (to add loaded lanes to)
    EdgeAttrs* myCurrentEdge;

    /// @brief The currently parsed junction definition to help in reconstructing crossings
    JunctionAttrs myCurrentJunction;

    /// @brief The currently parsed lanes's definition (to add the shape to)
    LaneAttrs* myCurrentLane;

    /// @brief The currently parsed traffic light
    NBLoadedSUMOTLDef* myCurrentTL;

    /// @brief The coordinate transformation which was used to build the loaded network.
    GeoConvHelper* myLocation;

    /// @brief The pedestrian crossings found in the network
    std::map<std::string, std::vector<Crossing> > myPedestrianCrossings;

    /// @brief Map from walkingArea edge IDs to custom shapes
    std::map<std::string, WalkingAreaParsedCustomShape> myWACustomShapes;

    /// @brief element to receive parameters
    std::vector<Parameterised*> myLastParameterised;

    /// @brief the loaded network version
    double myNetworkVersion;

    /// @brief whether the loaded network contains internal lanes
    bool myHaveSeenInternalEdge;

    /// @brief whether the loaded network was built for lefthand traffic
    bool myAmLefthand;

    /// @brief the level of corner detail in the loaded network
    int myCornerDetail;

    /// @brief the level of geometry detail for internal lanes in the loaded network
    int myLinkDetail;

    /// @brief whether all lanes of an edge should have the same stop line
    bool myRectLaneCut;

    /// @brief whether walkingareas must be built
    bool myWalkingAreas;

    /// @brief whether turning speed was limited in the network
    double myLimitTurnSpeed;

    /// @brief whether foe-relationships where checked at lane-level
    bool myCheckLaneFoesAll;
    bool myCheckLaneFoesRoundabout;
    /// @brief whether some right-of-way checks at traffic light junctions should be disabled
    bool myTlsIgnoreInternalJunctionJam;
    /// @brief default spreadType defined in the network
    std::string myDefaultSpreadType;
    /// @brief overlap option for loaded network
    bool myGeomAvoidOverlap;

    /// @brief loaded roundabout edges
    std::vector<std::vector<std::string> > myRoundabouts;

    /// @brief list of node id with rail signals (no NBTrafficLightDefinition exists)
    std::set<std::string> myRailSignals;

    /// @brief list of parameter keys to discard
    std::set<std::string> myDiscardableParams;

private:
    /** @brief Parses lane index from lane ID an retrieve lane from EdgeAttrs
     * @param[in] edge The EdgeAttrs* which should contain the lane
     * @param[in] lane_id The ID of the lane
     */
    LaneAttrs* getLaneAttrsFromID(EdgeAttrs* edge, std::string lane_id);

    /// @brief read position from the given attributes, attribute errors to id
    static Position readPosition(const SUMOSAXAttributes& attrs, const std::string& id, bool& ok);

    /** @brief parses connection string of a prohibition (very old school)
     * @param[in] attr The connection attribute
     * @param[out] from ID of the source edge
     * @param[out] to ID of the destination edge
     * @param[out] ok Whether parsing completed successfully
     */
    void parseProhibitionConnection(const std::string& attr, std::string& from, std::string& to, bool& ok);
};
