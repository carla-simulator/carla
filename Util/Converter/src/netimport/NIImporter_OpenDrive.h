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
/// @file    NIImporter_OpenDrive.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in openDrive format
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/xml/GenericSAXHandler.h>
#include <utils/geom/PositionVector.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBNetBuilder;
class NBEdge;
class OptionsCont;
class NBNode;
class NBNodeCont;


#define UNSET_CONNECTION 100000

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIImporter_OpenDrive
 * @brief Importer for networks stored in openDrive format
 *
 */
class NIImporter_OpenDrive : public GenericSAXHandler {
public:
    /** @brief Loads content of the optionally given SUMO file
     *
     * If the option "opendrive-files" is set, the file stored therein is read and
     *  the network definition stored therein is stored within the given network
     *  builder.
     *
     * If the option "opendrive-files" is not set, this method simply returns.
     *
     * The loading is done by parsing the network definition as an XML file
     *  using the SAXinterface and handling the incoming data via this class'
     *  methods.
     *
     * @param[in] oc The options to use
     * @param[in] nb The network builder to fill
     */
    static void loadNetwork(const OptionsCont& oc, NBNetBuilder& nb);


protected:

    /**
     * @enum OpenDriveXMLTag
     * @brief Numbers representing openDrive-XML - element names
     * @see GenericSAXHandler
     */
    enum OpenDriveXMLTag {
        OPENDRIVE_TAG_NOTHING,
        OPENDRIVE_TAG_HEADER,
        OPENDRIVE_TAG_ROAD,
        OPENDRIVE_TAG_PREDECESSOR,
        OPENDRIVE_TAG_SUCCESSOR,
        /// @todo OPENDRIVE_TAG_NEIGHBOR,
        /// @todo OPENDRIVE_TAG_TYPE,
        OPENDRIVE_TAG_GEOMETRY,
        OPENDRIVE_TAG_LINE,
        OPENDRIVE_TAG_SPIRAL,
        OPENDRIVE_TAG_ARC,
        OPENDRIVE_TAG_POLY3,
        OPENDRIVE_TAG_PARAMPOLY3,
        OPENDRIVE_TAG_LANESECTION,
        OPENDRIVE_TAG_LANEOFFSET,
        OPENDRIVE_TAG_LEFT,
        OPENDRIVE_TAG_CENTER,
        OPENDRIVE_TAG_RIGHT,
        OPENDRIVE_TAG_LANE,
        OPENDRIVE_TAG_SIGNAL,
        OPENDRIVE_TAG_SIGNALREFERENCE,
        OPENDRIVE_TAG_VALIDITY,
        OPENDRIVE_TAG_JUNCTION,
        OPENDRIVE_TAG_CONNECTION,
        OPENDRIVE_TAG_LANELINK,
        OPENDRIVE_TAG_WIDTH,
        OPENDRIVE_TAG_SPEED,
        OPENDRIVE_TAG_ELEVATION,
        OPENDRIVE_TAG_GEOREFERENCE,
        OPENDRIVE_TAG_OBJECT,
        OPENDRIVE_TAG_REPEAT
    };


    /**
     * @enum OpenDriveXMLAttr
     * @brief Numbers representing openDrive-XML - attributes
     * @see GenericSAXHandler
     */
    enum OpenDriveXMLAttr {
        OPENDRIVE_ATTR_NOTHING,
        OPENDRIVE_ATTR_REVMAJOR,
        OPENDRIVE_ATTR_REVMINOR,
        OPENDRIVE_ATTR_ID,
        OPENDRIVE_ATTR_LENGTH,
        OPENDRIVE_ATTR_WIDTH,
        OPENDRIVE_ATTR_RADIUS,
        OPENDRIVE_ATTR_DISTANCE,
        OPENDRIVE_ATTR_TSTART,
        OPENDRIVE_ATTR_TEND,
        OPENDRIVE_ATTR_WIDTHSTART,
        OPENDRIVE_ATTR_WIDTHEND,
        OPENDRIVE_ATTR_JUNCTION,
        OPENDRIVE_ATTR_ELEMENTTYPE,
        OPENDRIVE_ATTR_ELEMENTID,
        OPENDRIVE_ATTR_CONTACTPOINT,
        OPENDRIVE_ATTR_S,
        OPENDRIVE_ATTR_T,
        OPENDRIVE_ATTR_X,
        OPENDRIVE_ATTR_Y,
        OPENDRIVE_ATTR_HDG,
        OPENDRIVE_ATTR_CURVSTART,
        OPENDRIVE_ATTR_CURVEND,
        OPENDRIVE_ATTR_CURVATURE,
        OPENDRIVE_ATTR_A,
        OPENDRIVE_ATTR_B,
        OPENDRIVE_ATTR_C,
        OPENDRIVE_ATTR_D,
        OPENDRIVE_ATTR_AU,
        OPENDRIVE_ATTR_BU,
        OPENDRIVE_ATTR_CU,
        OPENDRIVE_ATTR_DU,
        OPENDRIVE_ATTR_AV,
        OPENDRIVE_ATTR_BV,
        OPENDRIVE_ATTR_CV,
        OPENDRIVE_ATTR_DV,
        OPENDRIVE_ATTR_PRANGE,
        OPENDRIVE_ATTR_TYPE,
        OPENDRIVE_ATTR_LEVEL,
        OPENDRIVE_ATTR_ORIENTATION,
        OPENDRIVE_ATTR_DYNAMIC,
        OPENDRIVE_ATTR_INCOMINGROAD,
        OPENDRIVE_ATTR_CONNECTINGROAD,
        OPENDRIVE_ATTR_FROM,
        OPENDRIVE_ATTR_TO,
        OPENDRIVE_ATTR_FROMLANE,
        OPENDRIVE_ATTR_TOLANE,
        OPENDRIVE_ATTR_MAX,
        OPENDRIVE_ATTR_SOFFSET,
        OPENDRIVE_ATTR_NAME,
        OPENDRIVE_ATTR_UNIT    // xodr v1.4
    };


    /** @brief OpenDrive link type enumeration
     */
    enum LinkType {
        OPENDRIVE_LT_SUCCESSOR,
        OPENDRIVE_LT_PREDECESSOR
    };


    /** @brief OpenDrive element type enumeration
     */
    enum ElementType {
        OPENDRIVE_ET_UNKNOWN,
        OPENDRIVE_ET_ROAD,
        OPENDRIVE_ET_JUNCTION
    };


    /** @brief OpenDrive contact type enumeration
     */
    enum ContactPoint {
        OPENDRIVE_CP_UNKNOWN,
        OPENDRIVE_CP_START,
        OPENDRIVE_CP_END
    };

    /** @brief OpenDrive geometry type enumeration
     */
    enum GeometryType {
        OPENDRIVE_GT_UNKNOWN,
        OPENDRIVE_GT_LINE,
        OPENDRIVE_GT_SPIRAL,
        OPENDRIVE_GT_ARC,
        OPENDRIVE_GT_POLY3,
        OPENDRIVE_GT_PARAMPOLY3 // rev 1.4
    };



    /**
     * @struct OpenDriveLink
     * @brief Representation of an OpenDrive link
     */
    struct OpenDriveLink {
        /** @brief Constructor
         * @param[in] linkTypeArg The link type
         * @param[in] elementIDArg The element id
         */
        OpenDriveLink(LinkType linkTypeArg, const std::string& elementIDArg)
            : linkType(linkTypeArg), elementID(elementIDArg),
              elementType(OPENDRIVE_ET_UNKNOWN), contactPoint(OPENDRIVE_CP_UNKNOWN) { }

        LinkType linkType;
        std::string elementID;
        ElementType elementType;
        ContactPoint contactPoint;
    };


    /**
     * @struct OpenDriveGeometry
     * @brief Representation of an OpenDrive geometry part
     */
    struct OpenDriveGeometry {
        /** @brief Constructor
         * @param[in] lengthArg The length of this geometry part
         * @param[in] sArg The offset from the start, counted from the begin
         * @param[in] xArg x-position at this part's begin
         * @param[in] yArg y-position at this part's begin
         * @param[in] hdgArg heading at this part's begin
         */
        OpenDriveGeometry(double lengthArg, double sArg, double xArg, double yArg, double hdgArg)
            : length(lengthArg), s(sArg), x(xArg), y(yArg), hdg(hdgArg),
              type(OPENDRIVE_GT_UNKNOWN) { }

        double length;
        double s;
        double x;
        double y;
        double hdg;
        GeometryType type;
        std::vector<double> params;
    };

    /**
     * @struct OpenDriveElevation
     * @brief Coefficients of an elevation profile (3rd degree polynomial)
     */
    struct Poly3 {
        /** @brief Constructor
         * @param[in] s The start offset
         * @param[in] a constant
         * @param[in] b first order
         * @param[in] c second order
         * @param[in] d third order
         */
        Poly3(double _s, double _a, double _b, double _c, double _d) :
            s(_s), a(_a), b(_b), c(_c), d(_d) {}

        double computeAt(double pos) const {
            const double ds = pos - s;
            return a + b * ds + c * ds * ds + d * ds * ds * ds;
        }

        double s;
        double a;
        double b;
        double c;
        double d;
    };

    /// LaneOffset has the same fields as Elevation
    typedef Poly3 OpenDriveElevation;
    typedef Poly3 OpenDriveLaneOffset;
    typedef Poly3 OpenDriveWidth;


    /**
     * @struct OpenDriveLane
     * @brief Representation of a lane
     */
    struct OpenDriveLane {
        /** @brief Constructor
         * @param[in] idArg The OpenDrive id of the lane
         * @param[in] levelArg The level
         * @param[in] typeArg type of the lane
         */
        OpenDriveLane(int idArg, const std::string& levelArg, const std::string& typeArg) :
            id(idArg), level(levelArg), type(typeArg), successor(UNSET_CONNECTION), predecessor(UNSET_CONNECTION),
            speed(0), width(NBEdge::UNSPECIFIED_WIDTH) { }

        int id; //!< The lane's id
        std::string level; //!< The lane's level (not used)
        std::string type; //!< The lane's type
        int successor; //!< The lane's successor lane
        int predecessor; //!< The lane's predecessor lane
        std::vector<std::pair<double, double> > speeds; //!< List of positions/speeds of speed changes
        double speed; //!< The lane's speed (set in post-processing)
        double width; //The lane's maximum width
        std::vector<OpenDriveWidth> widthData;
    };


    /**
     * @struct OpenDriveLaneSection
     * @brief Representation of a lane section
     */
    struct OpenDriveLaneSection {
        /** @brief Constructor
         * @param[in] sArg The offset from the start, counted from the begin
         */
        OpenDriveLaneSection(double sArg);


        /** @brief Build the mapping from OpenDrive to SUMO lanes
         *
         * Not all lanes are converted to SUMO-lanes; the mapping includes only those
         * which are included in the SUMO network.
         * @param[in] tc The type container needed to determine whether a lane shall be imported by using the lane's type
         */
        void buildLaneMapping(const NBTypeCont& tc);


        /** @brief Returns the links from the previous to this lane section
         * @param[in] dir The OpenDrive-direction of drive
         * @param[in] pre The previous lane section
         * @return which lane is approached from which lane of the given previous lane section
         */
        std::map<int, int> getInnerConnections(OpenDriveXMLTag dir, const OpenDriveLaneSection& prev);


        bool buildSpeedChanges(const NBTypeCont& tc, std::vector<OpenDriveLaneSection>& newSections);
        OpenDriveLaneSection buildLaneSection(double startPos);

        /// @brief The starting offset of this lane section
        double s;
        /// @brief The original starting offset of this lane section (differs from s if the section had to be split)
        double sOrig;
        /// @brief The length of this lane section
        double length;
        /// @brief A mapping from OpenDrive to SUMO-index (the first is signed, the second unsigned)
        std::map<int, int> laneMap;
        /// @brief The lanes, sorted by their direction
        std::map<OpenDriveXMLTag, std::vector<OpenDriveLane> > lanesByDir;
        /// @brief The id (generic, without the optionally leading '-') of the edge generated for this section
        std::string sumoID;
        /// @brief The number of lanes on the right and on the left side, respectively
        int rightLaneNumber, leftLaneNumber;
        /// @brief the composite type built from all used lane types
        std::string rightType;
        std::string leftType;
    };



    /**
     * @struct OpenDriveSignal
     * @brief Representation of a signal
     */
    struct OpenDriveSignal {
        /** @brief Constructor
         * @param[in] idArg The OpenDrive id of the signal
         * @param[in] typeArg The type of the signal
         * @param[in] nameArg The type of the signal
         * @param[in] orientationArg The direction the signal belongs to
         * @param[in] dynamicArg Whether the signal is dynamic
         * @param[in] sArg The offset from the start, counted from the begin
         */
        OpenDriveSignal(const std::string& idArg, const std::string typeArg, const std::string nameArg,
                        int orientationArg, bool dynamicArg, double sArg) :
            id(idArg), type(typeArg), name(nameArg),
            orientation(orientationArg), dynamic(dynamicArg), s(sArg),
            minLane(0), maxLane(0)
        { }

        /// dummy constructor for use in maps
        OpenDriveSignal() {}

        std::string id;
        std::string type;
        std::string name;
        int orientation;
        bool dynamic;
        double s;
        /// @brief signal validity range
        int minLane;
        int maxLane;
    };


    /**
     * @struct Connection
     * @brief A connection between two roads
     */
    struct Connection {
        std::string fromEdge;
        std::string toEdge;
        int fromLane;
        int toLane;
        ContactPoint fromCP;
        ContactPoint toCP;
        bool all;
        std::string origID;
        int origLane;
        PositionVector shape;

        std::string getDescription() const {
            return "Connection from=" + fromEdge + "_" + toString(fromLane)
                   + " to=" + toEdge + "_" + toString(toLane)
                   + " fromCP=" + (fromCP == OPENDRIVE_CP_START ? "start" : fromCP == OPENDRIVE_CP_END ? "end" : "unknown")
                   + " toCP=" + (toCP == OPENDRIVE_CP_START ? "start" : toCP == OPENDRIVE_CP_END ? "end" : "unknown")
                   + " all=" + toString(all);
            //+ " origID=" + origID + " origLane=" + toString(origLane);
        }
    };

    /**
     * @struct Object
     * @brief A road object (e.g. parkingSpace)
     */
    struct OpenDriveObject {
        std::string type;
        std::string name;
        std::string id;
        double s;
        double t;
        double zOffset;
        double length;
        double width;
        double height;
        double radius;
        double hdg;
        double pitch;
        double roll;
    };

    /**
     * @struct OpenDriveEdge
     * @brief Representation of an openDrive "link"
     */
    struct OpenDriveEdge {
        OpenDriveEdge(const std::string& idArg, const std::string& streetNameArg, const std::string& junctionArg, double lengthArg) :
            id(idArg),
            streetName(streetNameArg),
            junction(junctionArg),
            length(lengthArg),
            from(0),
            to(0) {
            isInner = junction != "" && junction != "-1";
        }


        /** @brief Returns the edge's priority, regarding the direction
         *
         * The priority is determined by evaluating the signs located at the road
         * @param[in] dir The direction which priority shall be returned
         * @return The priority of the given direction
         */
        int getPriority(OpenDriveXMLTag dir) const;


        /// @brief The id of the edge
        std::string id;
        /// @brief The road name of the edge
        std::string streetName;
        /// @brief The id of the junction the edge belongs to
        std::string junction;
        /// @brief The length of the edge
        double length;
        std::vector<OpenDriveLink> links;
        std::vector<OpenDriveGeometry> geometries;
        std::vector<OpenDriveElevation> elevations;
        std::vector<OpenDriveLaneOffset> offsets;
        NBNode* from;
        NBNode* to;
        PositionVector geom;
        std::vector<double> laneOffsets;
        std::vector<OpenDriveLaneSection> laneSections;
        std::vector<OpenDriveSignal> signals;
        std::set<Connection> connections;
        std::vector<OpenDriveObject> objects;
        bool isInner;
    };


    /** @brief A class for sorting lane sections by their s-value */
    class sections_by_s_sorter {
    public:
        /// @brief Constructor
        explicit sections_by_s_sorter() { }

        /// @brief Sorting function; compares OpenDriveLaneSection::s
        int operator()(const OpenDriveLaneSection& ls1, const OpenDriveLaneSection& ls2) {
            return ls1.s < ls2.s;
        }
    };

    /* @brief A class for search in position/speed tuple vectors for the given position */
    class same_position_finder {
    public:
        /** @brief constructor */
        explicit same_position_finder(double pos) : myPosition(pos) { }

        /** @brief the comparing function */
        bool operator()(const std::pair<double, double>& ps) {
            return ps.first == myPosition;
        }

    private:
        same_position_finder& operator=(const same_position_finder&); // just to avoid a compiler warning
    private:
        /// @brief The position to search for
        double myPosition;

    };

protected:
    /** @brief Constructor
     * @param[in] tc The type container used to determine whether a lane shall kept
     * @param[in] nc The edge map to fill
     */
    NIImporter_OpenDrive(const NBTypeCont& tc, std::map<std::string, OpenDriveEdge*>& edges);


    /// @brief Destructor
    ~NIImporter_OpenDrive();



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
    void myStartElement(int element, const SUMOSAXAttributes& attrs);

    /**
     * @brief Callback method for characters to implement by derived classes
     *
     * Called by "endElement" (see there).
     * @param[in] element The opened element, given as a int
     * @param[in] chars The complete embedded character string
     * @exceptions ProcessError These method may throw a ProcessError if something fails
     */
    void myCharacters(int element, const std::string& chars);


    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);
    //@}

    std::map<std::string, OpenDriveSignal>& getSignals() {
        return mySignals;
    }

private:
    void addLink(LinkType lt, const std::string& elementType, const std::string& elementID,
                 const std::string& contactPoint);
    void addGeometryShape(GeometryType type, const std::vector<double>& vals);
    static void setEdgeLinks2(OpenDriveEdge& e, const std::map<std::string, OpenDriveEdge*>& edges);
    static void buildConnectionsToOuter(const Connection& c, const std::map<std::string, OpenDriveEdge*>& innerEdges, std::vector<Connection>& into, std::set<Connection>& seen);
    static bool laneSectionsConnected(OpenDriveEdge* edge, int in, int out);
    friend bool operator<(const Connection& c1, const Connection& c2);
    static std::string revertID(const std::string& id);
    const NBTypeCont& myTypeContainer;
    OpenDriveEdge myCurrentEdge;

    std::map<std::string, OpenDriveEdge*>& myEdges;
    std::vector<int> myElementStack;
    OpenDriveXMLTag myCurrentLaneDirection;
    std::string myCurrentJunctionID;
    std::string myCurrentIncomingRoad;
    std::string myCurrentConnectingRoad;
    ContactPoint myCurrentContactPoint;
    bool myConnectionWasEmpty;
    std::map<std::string, OpenDriveSignal> mySignals;

    static bool myImportAllTypes;
    static bool myImportWidths;
    static double myMinWidth;
    static bool myImportInternalShapes;


protected:
    /** @brief Builds a node or returns the already built
     *
     * If the node is already known, it is returned. Otherwise, the
     *  node is built. If the newly built node can not be added to
     *  the container, a ProcessError is thrown.
     *  Otherwise this node is returned.
     *
     * @param[in] id The id of the node to build/get
     * @param[in, changed] pos The position of the node to build/get
     * @param[filled] nc The node container to retrieve/add the node to
     * @return The retrieved/built node
     * @exception ProcessError If the node could not be built/retrieved
     */
    static NBNode* getOrBuildNode(const std::string& id, const Position& pos, NBNodeCont& nc);


    static PositionVector geomFromLine(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution);
    static PositionVector geomFromSpiral(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution);
    static PositionVector geomFromArc(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution);
    static PositionVector geomFromPoly(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution);
    static PositionVector geomFromParamPoly(const OpenDriveEdge& e, const OpenDriveGeometry& g, double resolution);
    static Position calculateStraightEndPoint(double hdg, double length, const Position& start);
    static void calculateCurveCenter(double* ad_x, double* ad_y, double ad_radius, double ad_hdg);
    static void calcPointOnCurve(double* ad_x, double* ad_y, double ad_centerX, double ad_centerY,
                                 double ad_r, double ad_length);


    /** @brief Computes a polygon representation of each edge's geometry
     * @param[in] edges The edges which geometries shall be converted
     */
    static void computeShapes(std::map<std::string, OpenDriveEdge*>& edges);

    static bool hasNonLinearElevation(OpenDriveEdge& e);

    /** @brief Rechecks lane sections of the given edges
     *
     *
     * @param[in] edges The edges which lane sections shall be reviewed
     */
    static void revisitLaneSections(const NBTypeCont& tc, std::map<std::string, OpenDriveEdge*>& edges);

    static void setNodeSecure(NBNodeCont& nc, OpenDriveEdge& e,
                              const std::string& nodeID, NIImporter_OpenDrive::LinkType lt, std::vector<NodeSet>& joinedNodeIDs);

    static NBTrafficLightDefinition* getTLSSecure(NBEdge* inEdge, NBNetBuilder& nb);


    static std::pair<NBEdge*, NBEdge*> retrieveSignalEdges(NBNetBuilder& nb, const std::string& fromID, const std::string& toID, const std::string& junction);

    static void splitMinWidths(OpenDriveEdge* e, const NBTypeCont& tc, double minDist);

    static void findWidthSplit(const NBTypeCont& tc, std::vector<OpenDriveLane>& lanes,
                               int section, double sectionStart, double sectionEnd,
                               std::vector<double>& splitPositions);

    static void sanitizeWidths(OpenDriveEdge* e);
    static void sanitizeWidths(std::vector<OpenDriveLane>& lanes, double length);

    static void setStraightConnections(std::vector<OpenDriveLane>& lanes);
    static void recomputeWidths(OpenDriveLaneSection& sec, double start, double end, double sectionStart, double sectionEnd);
    static void recomputeWidths(std::vector<OpenDriveLane>& lanes, double start, double end, double sectionStart, double sectionEnd);
    static void setLaneAttributes(const OpenDriveEdge* e, NBEdge::Lane& sumoLane, const OpenDriveLane& odLane, bool saveOrigIDs, const NBTypeCont& tc);

    /// The names of openDrive-XML elements (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry openDriveTags[];

    /// The names of openDrive-XML attributes (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry openDriveAttrs[];



};
