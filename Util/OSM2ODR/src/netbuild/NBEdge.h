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
/// @file    NBEdge.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// The representation of a single edge during network building
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <vector>
#include <string>
#include <set>
#include <cassert>
#include <utils/common/Named.h>
#include <utils/common/Parameterised.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/Bresenham.h>
#include <utils/geom/PositionVector.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "NBCont.h"
#include "NBHelpers.h"
#include "NBSign.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NBConnection;
class NBNodeCont;
class NBEdgeCont;
class OutputDevice;
class GNELane;
class NBVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBRouterEdge
 * @brief Superclass for NBEdge and NBEdge::Connection to initialize Router
 */
class NBRouterEdge {
public:
    virtual const std::string& getID() const = 0;
    virtual double getSpeed() const = 0;
    virtual double getLength() const = 0;
    virtual const NBRouterEdge* getBidiEdge() const = 0;
    virtual int getNumericalID() const = 0;
    virtual const ConstRouterEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const = 0;
    virtual bool isInternal() const {
        return false;
    }
    inline bool prohibits(const NBVehicle* const /*veh*/) const {
        return false;
    }
    inline bool restricts(const NBVehicle* const /*veh*/) const {
        return false;
    }


    static inline double getTravelTimeStatic(const NBRouterEdge* const edge, const NBVehicle* const /*veh*/, double /*time*/) {
        return edge->getLength() / edge->getSpeed();
    }
};


/**
 * @class NBEdge
 * @brief The representation of a single edge during network building
 */
class NBEdge : public Named, public Parameterised, public NBRouterEdge {
    friend class NBEdgeCont;

    /** used for visualization (NETEDIT) */
    friend class GNELane;
    friend class GNEEdge;
    friend class GNEJunction;

public:

    /** @enum EdgeBuildingStep
     * @brief Current state of the edge within the building process
     *
     * As the network is build in a cascaded way, considering loaded
     *  information, a counter holding the current step is needed. This is done
     *  by using this enumeration.
     */
    enum class EdgeBuildingStep {
        /// @brief The edge has been loaded and connections shall not be added
        INIT_REJECT_CONNECTIONS,
        /// @brief The edge has been loaded, nothing is computed yet
        INIT,
        /// @brief The relationships between edges are computed/loaded
        EDGE2EDGES,
        /// @brief Lanes to edges - relationships are computed/loaded
        LANES2EDGES,
        /// @brief Lanes to lanes - relationships are computed; should be recheked
        LANES2LANES_RECHECK,
        /// @brief Lanes to lanes - relationships are computed; no recheck is necessary/wished
        LANES2LANES_DONE,
        /// @brief Lanes to lanes - relationships are loaded; no recheck is necessary/wished
        LANES2LANES_USER
    };


    /** @enum Lane2LaneInfoType
    * @brief Modes of setting connections between lanes
    */
    enum class Lane2LaneInfoType {
        /// @brief The connection was computed
        COMPUTED,
        /// @brief The connection was given by the user
        USER,
        /// @brief The connection was computed and validated
        VALIDATED
    };


    /** @struct Lane
     * @brief An (internal) definition of a single lane of an edge
     */
    struct Lane final : public Parameterised {
        /// @brief constructor
        Lane(NBEdge* e, const std::string& _origID);

        /// @brief The lane's shape
        PositionVector shape;

        /// @brief The speed allowed on this lane
        double speed;

        /// @brief List of vehicle types that are allowed on this lane
        SVCPermissions permissions;

        /// @brief List of vehicle types that are preferred on this lane
        SVCPermissions preferred;

        /// @brief This lane's offset to the intersection begin
        double endOffset;

        /// @brief stopOffsets.second - The stop offset for vehicles stopping at the lane's end.
        ///        Applies if vClass is in in stopOffset.first bitset
        std::map<int, double> stopOffsets;

        /// @brief This lane's width
        double width;

        /// @brief An opposite lane ID, if given
        std::string oppositeID;

        /// @brief Whether this lane is an acceleration lane
        bool accelRamp;

        /// @brief Whether connection information for this lane is already completed
        // @note (see NIImporter_DlrNavteq::ConnectedLanesHandler)
        bool connectionsDone;

        /// @brief A custom shape for this lane set by the user
        PositionVector customShape;

        /// @brief the type of this lane
        std::string type;
    };


    /** @struct Connection
     * @brief A structure which describes a connection between edges or lanes
     */
    struct Connection final : public Parameterised, public NBRouterEdge {
        /** @brief Constructor
         * @param[in] fromLane_ The lane the connections starts at
         * @param[in] toEdge_ The edge the connections yields in
         * @param[in] toLane_ The lane the connections yields in
         */
        Connection(int fromLane_, NBEdge* toEdge_, int toLane_);

        /// @brief constructor with more parameters
        Connection(int fromLane_, NBEdge* toEdge_, int toLane_, bool mayDefinitelyPass_,
                   bool keepClear_ = true,
                   double contPos_ = UNSPECIFIED_CONTPOS,
                   double visibility_ = UNSPECIFIED_VISIBILITY_DISTANCE,
                   double speed_ = UNSPECIFIED_SPEED,
                   double length_ = myDefaultConnectionLength,
                   bool haveVia_ = false,
                   bool uncontrolled_ = false,
                   const PositionVector& customShape_ = PositionVector::EMPTY,
                   SVCPermissions permissions = SVC_UNSPECIFIED);

        /// @brief The lane the connections starts at
        int fromLane;

        /// @brief The edge the connections yields in
        NBEdge* toEdge;

        /// @brief The lane the connections yields in
        int toLane;

        /// @brief The id of the traffic light that controls this connection
        std::string tlID;

        /// @brief The index of this connection within the controlling traffic light
        int tlLinkIndex;

        /// @brief The index of the internal junction within the controlling traffic light (optional)
        int tlLinkIndex2;

        /// @brief Information about being definitely free to drive (on-ramps)
        bool mayDefinitelyPass;

        /// @brief whether the junction must be kept clear when using this connection
        bool keepClear;

        /// @brief custom position for internal junction on this connection
        double contPos;

        /// @brief custom foe visiblity for connection
        double visibility;

        /// @brief custom speed for connection
        double speed;

        /// @brief custom length for connection
        double customLength;

        /// @brief custom shape for connection
        PositionVector customShape;

        /// @brief List of vehicle types that are allowed on this connection
        SVCPermissions permissions;

        /// @brief id of Connection
        std::string id;

        /// @brief shape of Connection
        PositionVector shape;

        /// @brief maximum velocity
        double vmax;

        /// @brief check if Connection have a Via
        bool haveVia;

        /// @brief if Connection have a via, ID of it
        std::string viaID;

        /// @brief shape of via
        PositionVector viaShape;

        /// @brief the length of the via shape (maybe customized)
        double viaLength;

        /// @brief FOE Internal links
        std::vector<int> foeInternalLinks;

        /// @brief FOE Incomings lanes
        std::vector<std::string> foeIncomingLanes;

        /// @brief The lane index of this internal lane within the internal edge
        int internalLaneIndex;

        /// @brief check if Connection is uncontrolled
        bool uncontrolled;

        /// @brief get ID of internal lane
        std::string getInternalLaneID() const;

        /// @brief get string describing this connection
        std::string getDescription(const NBEdge* parent) const;

        /// @brief computed length (average of all internal lane shape lengths that share an internal edge)
        double length;

        /// @name NBRouterEdge interface
        /// @{
        static ConstRouterEdgePairVector myViaSuccessors; // always empty
        const std::string& getID() const {
            return id;
        }
        double getSpeed() const {
            return vmax;
        }
        double getLength() const {
            return shape.length() + viaShape.length();
        }
        int getNumericalID() const {
            throw ProcessError("NBEdge::Connection does not implement getNumericalID()");
        }
        const Connection* getBidiEdge() const {
            return nullptr;
        }
        bool isInternal() const {
            return true;
        }
        const ConstRouterEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const {
            UNUSED_PARAMETER(vClass);
            return myViaSuccessors;
        }
        /// }@
    };

    /// @brief Dummy edge to use when a reference must be supplied in the no-arguments constructor (FOX technicality)
    static NBEdge DummyEdge;

    /// @brief unspecified lane width
    static const double UNSPECIFIED_WIDTH;

    /// @brief unspecified lane offset
    static const double UNSPECIFIED_OFFSET;

    /// @brief unspecified lane speed
    static const double UNSPECIFIED_SPEED;

    /// @brief unspecified internal junction position
    static const double UNSPECIFIED_CONTPOS;

    /// @brief unspecified foe visibility for connections
    static const double UNSPECIFIED_VISIBILITY_DISTANCE;

    /// @brief no length override given
    static const double UNSPECIFIED_LOADED_LENGTH;

    /// @brief unspecified signal offset
    static const double UNSPECIFIED_SIGNAL_OFFSET;

    /// @brief the distance at which to take the default angle
    static const double ANGLE_LOOKAHEAD;

    /// @brief internal lane computation not yet done
    static const int UNSPECIFIED_INTERNAL_LANE_INDEX;

    /// @brief TLS-controlled despite its node controlled not specified.
    static const bool UNSPECIFIED_CONNECTION_UNCONTROLLED;

    /// @brief junction priority values set by setJunctionPriority
    enum JunctionPriority {
        MINOR_ROAD = 0,
        PRIORITY_ROAD = 1,
        ROUNDABOUT = 1000
    };

    static void setDefaultConnectionLength(double length) {
        myDefaultConnectionLength = length;
    }

public:
    /** @brief Constructor
     *
     * Use this if no edge geometry is given.
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (my be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] width This edge's lane width
     * @param[in] endOffset Additional offset to the destination node
     * @param[in] streetName The street name (need not be unique)
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @see init
     * @see LaneSpreadFunction
     */
    NBEdge(const std::string& id,
           NBNode* from, NBNode* to, std::string type,
           double speed, int nolanes, int priority,
           double width, double endOffset,
           const std::string& streetName = "",
           LaneSpreadFunction spread = LaneSpreadFunction::RIGHT);


    /** @brief Constructor
     *
     * Use this if the edge's geometry is given.
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (may be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] width This edge's lane width
     * @param[in] endOffset Additional offset to the destination node
     * @param[in] geom The edge's geomatry
     * @param[in] streetName The street name (need not be unique)
     * @param[in] origID The original ID in the source network (need not be unique)
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     * @see init
     * @see LaneSpreadFunction
     */
    NBEdge(const std::string& id,
           NBNode* from, NBNode* to, std::string type,
           double speed, int nolanes, int priority,
           double width, double endOffset,
           PositionVector geom,
           const std::string& streetName = "",
           const std::string& origID = "",
           LaneSpreadFunction spread = LaneSpreadFunction::RIGHT,
           bool tryIgnoreNodePositions = false);

    /** @brief Constructor
     *
     * Use this to copy attribuets from another edge
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] tpl The template edge to copy attributes from
     * @param[in] geom The geometry to use (may be empty)
     * @param[in] numLanes The number of lanes of the new edge (copy from tpl by default)
     */
    NBEdge(const std::string& id,
           NBNode* from, NBNode* to,
           const NBEdge* tpl,
           const PositionVector& geom = PositionVector(),
           int numLanes = -1);


    /// @brief Destructor
    ~NBEdge();


    /** @brief Resets initial values
     *
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     * @param[in] type The type of the edge (may be =="")
     * @param[in] speed The maximum velocity allowed on this edge
     * @param[in] nolanes The number of lanes this edge has
     * @param[in] priority This edge's priority
     * @param[in] geom The edge's geomatry
     * @param[in] width This edge's lane width
     * @param[in] endOffset Additional offset to the destination node
     * @param[in] streetName The street name (need not be unique)
     * @param[in] spread How the lateral offset of the lanes shall be computed
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     */
    void reinit(NBNode* from, NBNode* to, const std::string& type,
                double speed, int nolanes, int priority,
                PositionVector geom, double width, double endOffset,
                const std::string& streetName,
                LaneSpreadFunction spread = LaneSpreadFunction::RIGHT,
                bool tryIgnoreNodePositions = false);

    /** @brief Resets nodes but keeps all other values the same (used when joining)
     * @param[in] from The node the edge starts at
     * @param[in] to The node the edge ends at
     */
    void reinitNodes(NBNode* from, NBNode* to);

    /// @name Applying offset
    /// @{
    /** @brief Applies an offset to the edge
     * @param[in] xoff The x-offset to apply
     * @param[in] yoff The y-offset to apply
     */
    void reshiftPosition(double xoff, double yoff);

    /// @brief mirror coordinates along the x-axis
    void mirrorX();
    /// @}

    /// @name Atomar getter methods
    //@{

    /** @brief Returns the number of lanes
     * @returns This edge's number of lanes
     */
    int getNumLanes() const {
        return (int)myLanes.size();
    }

    /** @brief Returns the priority of the edge
     * @return This edge's priority
     */
    int getPriority() const {
        return myPriority;
    }

    /// @brief Sets the priority of the edge
    void setPriority(int priority) {
        myPriority = priority;
    }

    /** @brief Returns the origin node of the edge
     * @return The node this edge starts at
     */
    NBNode* getFromNode() const {
        return myFrom;
    }

    /** @brief Returns the destination node of the edge
     * @return The node this edge ends at
     */
    NBNode* getToNode() const {
        return myTo;
    }

    /** @brief Returns the angle at the start of the edge
     * (relative to the node shape center)
     * The angle is computed in computeAngle()
     * @return This edge's start angle
     */
    inline double getStartAngle() const {
        return myStartAngle;
    }

    /** @brief Returns the angle at the end of the edge
     * (relative to the node shape center)
     * The angle is computed in computeAngle()
     * @return This edge's end angle
     */
    inline double getEndAngle() const {
        return myEndAngle;
    }

    /** @brief Returns the angle at the start of the edge
     * @note only using edge shape
     * @return This edge's start angle
     */
    double getShapeStartAngle() const;


    /** @brief Returns the angle at the end of the edge
     * @note only using edge shape
     * @note The angle is computed in computeAngle()
     * @return This edge's end angle
     */
    double getShapeEndAngle() const;

    /** @brief Returns the angle at the start of the edge
     * @note The angle is computed in computeAngle()
     * @return This edge's angle
     */
    inline double getTotalAngle() const {
        return myTotalAngle;
    }

    /** @brief Returns the computed length of the edge
     * @return The edge's computed length
     */
    double getLength() const {
        return myLength;
    }


    /** @brief Returns the length was set explicitly or the computed length if it wasn't set
     * @todo consolidate use of myLength and myLoaded length
     * @return The edge's specified length
     */
    double getLoadedLength() const {
        return myLoadedLength > 0 ? myLoadedLength : myLength;
    }

    /// @brief get length that will be assigned to the lanes in the final network
    double getFinalLength() const;

    /** @brief Returns whether a length was set explicitly
     * @return Wether the edge's length was specified
     */
    bool hasLoadedLength() const {
        return myLoadedLength > 0;
    }

    /** @brief Returns the speed allowed on this edge
     * @return The maximum speed allowed on this edge
     */
    double getSpeed() const {
        return mySpeed;
    }

    /** @brief The building step of this edge
     * @return The current building step for this edge
     * @todo Recheck usage!
     * @see EdgeBuildingStep
     */
    EdgeBuildingStep getStep() const {
        return myStep;
    }

    /** @brief Returns the default width of lanes of this edge
     * @return The width of lanes of this edge
     */
    double getLaneWidth() const {
        return myLaneWidth;
    }

    /** @brief Returns the width of the lane of this edge
     * @return The width of the lane of this edge
     */
    double getLaneWidth(int lane) const;

    /// @brief Returns the combined width of all lanes of this edge
    double getTotalWidth() const;

    /// @brief Returns the street name of this edge
    const std::string& getStreetName() const {
        return myStreetName;
    }

    /// @brief sets the street name of this edge
    void setStreetName(const std::string& name) {
        myStreetName = name;
    }

    /** @brief Returns the offset to the destination node
     * @return The offset to the destination node
     */
    double getEndOffset() const {
        return myEndOffset;
    }

    double getDistance() const {
        return myDistance;
    }

    /** @brief Returns the stopOffset to the end of the edge
     * @return The offset to the end of the edge
     */
    const std::map<int, double>& getStopOffsets() const {
        return myStopOffsets;
    }

    /** @brief Returns the offset to the destination node a the specified lane
     * @return The offset to the destination node
     */
    double getEndOffset(int lane) const;

    /** @brief Returns the stop offset to the specified lane's end
     * @return The stop offset to the specified lane's end
     */
    const std::map<int, double>& getStopOffsets(int lane) const;

    /// @brief Returns the offset of a traffic signal from the end of this edge
    double getSignalOffset() const;

    /// @brief Returns the position of a traffic signal on this edge
    const Position& getSignalPosition() const {
        return mySignalPosition;
    }

    /// @brief Returns the node that (possibly) represents a traffic signal controlling at the end of this edge
    const NBNode* getSignalNode() const {
        return mySignalNode;
    }

    /// @brief sets the offset of a traffic signal from the end of this edge
    void setSignalPosition(const Position& pos, const NBNode* signalNode) {
        mySignalPosition = pos;
        mySignalNode = signalNode;
    }

    /** @brief Returns the lane definitions
     * @return The stored lane definitions
     */
    const std::vector<NBEdge::Lane>& getLanes() const {
        return myLanes;
    }
    //@}

    /** @brief return the first lane with permissions other than SVC_PEDESTRIAN and 0
     * @param[in] direction The direction in which the lanes shall be checked
     * @param[in] exclusive Whether lanes that allow pedestrians along with other classes shall be counted as non-pedestrian
     */
    int getFirstNonPedestrianLaneIndex(int direction, bool exclusive = false) const;

    /// @brief return index of the first lane that allows the given permissions
    int getSpecialLane(SVCPermissions permissions) const;

    /** @brief return the first lane that permits at least 1 vClass or the last lane if search direction of there is no such lane
     * @param[in] direction The direction in which the lanes shall be checked
     */
    int getFirstAllowedLaneIndex(int direction) const;

    /// @brif get first non-pedestrian lane
    NBEdge::Lane getFirstNonPedestrianLane(int direction) const;

    /// @brief return all permission variants within the specified lane range [iStart, iEnd[
    std::set<SVCPermissions> getPermissionVariants(int iStart, int iEnd) const;

    /// @brief get lane indices that allow the given permissions
    int getNumLanesThatAllow(SVCPermissions permissions) const;

    /// @brief return the angle for computing pedestrian crossings at the given node
    double getCrossingAngle(NBNode* node);

    /// @brief get the lane id for the canonical sidewalk lane
    std::string getSidewalkID();

    /// @name Edge geometry access and computation
    //@{
    /** @brief Returns the geometry of the edge
     * @return The edge's geometry
     */
    const PositionVector& getGeometry() const {
        return myGeom;
    }

    /// @brief Returns the geometry of the edge without the endpoints
    const PositionVector getInnerGeometry() const;

    /// @brief Returns whether the geometry consists only of the node positions
    bool hasDefaultGeometry() const;

    /** @brief Returns whether the geometry is terminated by the node positions
     * This default may be violated by initializing with
     * tryIgnoreNodePositions=true' or with setGeometry()
     * non-default endpoints are useful to control the generated node shape
     */
    bool hasDefaultGeometryEndpoints() const;

    /** @brief Returns whether the geometry is terminated by the node positions
     * This default may be violated by initializing with
     * tryIgnoreNodePositions=true' or with setGeometry()
     * non-default endpoints are useful to control the generated node shape
     */
    bool hasDefaultGeometryEndpointAtNode(const NBNode* node) const;

    Position getEndpointAtNode(const NBNode* node) const;

    /** @brief (Re)sets the edge's geometry
     *
     * Replaces the edge's prior geometry by the given. Then, computes
     *  the geometries of all lanes using computeLaneShapes.
     * Definitely not the best way to have it accessable from outside...
     * @param[in] g The edge's new geometry
     * @param[in] inner whether g should be interpreted as inner points
     * @todo Recheck usage, disallow access
     * @see computeLaneShapes
     */
    void setGeometry(const PositionVector& g, bool inner = false);

    /** @brief Adds a further geometry point
     *
     * Some importer do not know an edge's geometry when it is initialised.
     *  This method allows to insert further geometry points after the edge
     *  has been built.
     *
     * @param[in] index The position at which the point shall be added
     * @param[in] p The point to add
     */
    void addGeometryPoint(int index, const Position& p);

    /// @brief linearly extend the geometry at the given node
    void extendGeometryAtNode(const NBNode* node, double maxExtent);

    /// @brief linearly extend the geometry at the given node
    void shortenGeometryAtNode(const NBNode* node, double reduction);

    /// @brief shift geometry at the given node to avoid overlap
    void shiftPositionAtNode(NBNode* node, NBEdge* opposite);

    /// @brief return position taking into account loaded length
    Position geometryPositionAtOffset(double offset) const;

    /** @brief Recomputeds the lane shapes to terminate at the node shape
     * For every lane the intersection with the fromNode and toNode is
     * calculated and the lane shorted accordingly. The edge length is then set
     * to the average of all lane lengths (which may differ). This average length is used as the lane
     * length when writing the network.
     * @note All lanes of an edge in a sumo net must have the same nominal length
     *  but may differ in actual geomtric length.
     * @note Depends on previous call to NBNodeCont::computeNodeShapes
     */
    void computeEdgeShape(double smoothElevationThreshold = -1);

    /** @brief Returns the shape of the nth lane
     * @return The shape of the lane given by its index (counter from right)
     */
    const PositionVector& getLaneShape(int i) const;

    /** @brief (Re)sets how the lanes lateral offset shall be computed
     * @param[in] spread The type of lateral offset to apply
     * @see LaneSpreadFunction
     */
    void setLaneSpreadFunction(LaneSpreadFunction spread);

    /** @brief Returns how this edge's lanes' lateral offset is computed
     * @return The type of lateral offset that is applied on this edge
     * @see LaneSpreadFunction
     */
    LaneSpreadFunction getLaneSpreadFunction() const {
        return myLaneSpreadFunction;
    }

    /** @brief Removes points with a distance lesser than the given
     * @param[in] minDist The minimum distance between two position to keep the second
     */
    void reduceGeometry(const double minDist);

    /** @brief Check the angles of successive geometry segments
     * @param[in] maxAngle The maximum angle allowed
     * @param[in] minRadius The minimum turning radius allowed at the start and end
     * @param[in] fix Whether to prune geometry points to avoid sharp turns at start and end
     */
    void checkGeometry(const double maxAngle, const double minRadius, bool fix, bool silent);
    //@}

    /// @name Setting and getting connections
    /// @{
    /** @brief Adds a connection to another edge
     *
     * If the given edge does not start at the node this edge ends on, false is returned.
     *
     * All other cases return true. Though, a connection may not been added if this edge
     *  is in step "INIT_REJECT_CONNECTIONS". Also, this method assures that a connection
     *  to an edge is set only once, no multiple connections to next edge are stored.
     *
     * After a first connection to an edge was set, the process step is set to "EDGE2EDGES".
     * @note Passing 0 implicitly removes all existing connections
     *
     * @param[in] dest The connection's destination edge
     * @return Whether the connection was valid
     */
    bool addEdge2EdgeConnection(NBEdge* dest, bool overrideRemoval = false);

    /** @brief Adds a connection between the specified this edge's lane and an approached one
     *
     * If the given edge does not start at the node this edge ends on, false is returned.
     *
     * All other cases return true. Though, a connection may not been added if this edge
     *  is in step "INIT_REJECT_CONNECTIONS". Before the lane-to-lane connection is set,
     *  a connection between edges is established using "addEdge2EdgeConnection". Then,
     *  "setConnection" is called for inserting the lane-to-lane connection.
     *
     * @param[in] fromLane The connection's starting lane (of this edge)
     * @param[in] dest The connection's destination edge
     * @param[in] toLane The connection's destination lane
     * @param[in] type The connections's type
     * @param[in] mayUseSameDestination Whether this connection may be set though connecting an already connected lane
     * @param[in] mayDefinitelyPass Whether this connection is definitely undistrubed (special case for on-ramps)
     * @return Whether the connection was added / exists
     * @see addEdge2EdgeConnection
     * @see setConnection
     * @todo Check difference between "setConnection" and "addLane2LaneConnection"
     */
    bool addLane2LaneConnection(int fromLane, NBEdge* dest,
                                int toLane, Lane2LaneInfoType type,
                                bool mayUseSameDestination = false,
                                bool mayDefinitelyPass = false,
                                bool keepClear = true,
                                double contPos = UNSPECIFIED_CONTPOS,
                                double visibility = UNSPECIFIED_VISIBILITY_DISTANCE,
                                double speed = UNSPECIFIED_SPEED,
                                double length = myDefaultConnectionLength,
                                const PositionVector& customShape = PositionVector::EMPTY,
                                const bool uncontrolled = UNSPECIFIED_CONNECTION_UNCONTROLLED,
                                SVCPermissions = SVC_UNSPECIFIED,
                                bool postProcess = false);

    /** @brief Builds no connections starting at the given lanes
     *
     * If "invalidatePrevious" is true, a call to "invalidateConnections(true)" is done.
     * This method loops through the given connections to set, calling "addLane2LaneConnection"
     *  for each.
     *
     * @param[in] fromLane The first of the connections' starting lanes (of this edge)
     * @param[in] dest The connections' destination edge
     * @param[in] toLane The first of the connections' destination lanes
     * @param[in] no The number of connections to set
     * @param[in] type The connections' type
     * @param[in] invalidatePrevious Whether previously set connection shall be deleted
     * @param[in] mayDefinitelyPass Whether these connections are definitely undistrubed (special case for on-ramps)
     * @return Whether the connections were added / existed
     * @see addLane2LaneConnection
     * @see invalidateConnections
     */
    bool addLane2LaneConnections(int fromLane,
                                 NBEdge* dest, int toLane, int no,
                                 Lane2LaneInfoType type, bool invalidatePrevious = false,
                                 bool mayDefinitelyPass = false);

    /** @brief Adds a connection to a certain lane of a certain edge
     *
     * @param[in] lane The connection's starting lane (of this edge)
     * @param[in] destEdge The connection's destination edge
     * @param[in] destLane The connection's destination lane
     * @param[in] type The connections's type
     * @param[in] mayUseSameDestination Whether this connection may be set though connecting an already connected lane
     * @param[in] mayDefinitelyPass Whether this connection is definitely undistrubed (special case for on-ramps)
     * @todo Check difference between "setConnection" and "addLane2LaneConnection"
     */
    bool setConnection(int lane, NBEdge* destEdge,
                       int destLane,
                       Lane2LaneInfoType type,
                       bool mayUseSameDestination = false,
                       bool mayDefinitelyPass = false,
                       bool keepClear = true,
                       double contPos = UNSPECIFIED_CONTPOS,
                       double visibility = UNSPECIFIED_VISIBILITY_DISTANCE,
                       double speed = UNSPECIFIED_SPEED,
                       double length = myDefaultConnectionLength,
                       const PositionVector& customShape = PositionVector::EMPTY,
                       const bool uncontrolled = UNSPECIFIED_CONNECTION_UNCONTROLLED,
                       SVCPermissions permissions = SVC_UNSPECIFIED,
                       bool postProcess = false);

    /** @brief Returns connections from a given lane
     *
     * This method goes through "myConnections" and copies those which are
     *  starting at the given lane.
     * @param[in] lane The lane which connections shall be returned
     * @param[in] to The target Edge (ignore nullptr)
     * @param[in] toLane The target lane (ignore if > 0)
     * @return The connections from the given lane
     * @see NBEdge::Connection
     */
    std::vector<Connection> getConnectionsFromLane(int lane, NBEdge* to = nullptr, int toLane = -1) const;

    /** @brief Returns the specified connection
     * This method goes through "myConnections" and returns the specified one
     * @see NBEdge::Connection
     */
    Connection getConnection(int fromLane, const NBEdge* to, int toLane) const;

    /** @brief Returns reference to the specified connection
     * This method goes through "myConnections" and returns the specified one
     * @see NBEdge::Connection
     */
    Connection& getConnectionRef(int fromLane, const NBEdge* to, int toLane);

    /** @brief Retrieves info about a connection to a certain lane of a certain edge
     *
     * Turnaround edge is ignored!
     * @param[in] destEdge The connection's destination edge
     * @param[in] destLane The connection's destination lane
     * @param[in] fromLane If a value >= 0 is given, only return true if a connection from the given lane exists
     * @return whether a connection to the specified lane exists
     */
    bool hasConnectionTo(NBEdge* destEdge, int destLane, int fromLane = -1) const;

    /** @brief Returns the information whethe a connection to the given edge has been added (or computed)
     *
     * @param[in] e The destination edge
     * @param[in] ignoreTurnaround flag to ignore or not Turnaround
     * @return Whether a connection to the specified edge exists
     */
    bool isConnectedTo(const NBEdge* e, const bool ignoreTurnaround = false) const;

    /** @brief Returns the connections
     * @return This edge's connections to following edges
     */
    const std::vector<Connection>& getConnections() const {
        return myConnections;
    }

    /** @brief Returns the connections
     * @return This edge's connections to following edges
     */
    std::vector<Connection>& getConnections() {
        return myConnections;
    }

    /** @brief Returns the list of outgoing edges without the turnaround sorted in clockwise direction
     * @return Connected edges, sorted clockwise
     */
    const EdgeVector* getConnectedSorted();

    /** @brief Returns the list of outgoing edges unsorted
     * @return Connected edges
     */
    EdgeVector getConnectedEdges() const;

    /** @brief Returns the list of incoming edges unsorted
     * @return Connected predecessor edges
     */
    EdgeVector getIncomingEdges() const;

    /** @brief Returns the list of lanes that may be used to reach the given edge
     * @return Lanes approaching the given edge
     */
    std::vector<int> getConnectionLanes(NBEdge* currentOutgoing, bool withBikes = true) const;

    /// @brief sorts the outgoing connections by their angle relative to their junction
    void sortOutgoingConnectionsByAngle();

    /// @brief sorts the outgoing connections by their from-lane-index and their to-lane-index
    void sortOutgoingConnectionsByIndex();

    /** @brief Remaps the connection in a way that allows the removal of it
     *
     * This edge (which is a self loop edge, in fact) connections are spread over the valid incoming edges
     * @todo recheck!
     */
    void remapConnections(const EdgeVector& incoming);

    /** @brief Removes the specified connection(s)
     * @param[in] toEdge The destination edge
     * @param[in] fromLane The lane from which connections shall be removed; -1 means remove all
     * @param[in] toLane   The lane to which connections shall be removed; -1 means remove all
     * @param[in] tryLater If the connection does not exist, try again during recheckLanes()
     * @param[in] adaptToLaneRemoval we are in the process of removing a complete lane, adapt all connections accordingly
     */
    void removeFromConnections(NBEdge* toEdge, int fromLane = -1, int toLane = -1, bool tryLater = false, const bool adaptToLaneRemoval = false, const bool keepPossibleTurns = false);

    /// @brief remove an existent connection of edge
    bool removeFromConnections(const NBEdge::Connection& connectionToRemove);

    /// @brief invalidate current connections of edge
    void invalidateConnections(bool reallowSetting = false);

    /// @brief replace in current connections of edge
    void replaceInConnections(NBEdge* which, NBEdge* by, int laneOff);

    /// @brief replace in current connections of edge
    void replaceInConnections(NBEdge* which, const std::vector<NBEdge::Connection>& origConns);

    /// @brief copy connections from antoher edge
    void copyConnectionsFrom(NBEdge* src);

    /// @brief modifify the toLane for all connections to the given edge
    void shiftToLanesToEdge(NBEdge* to, int laneOff);
    /// @}

    /** @brief Returns whether the given edge is the opposite direction to this edge
     * @param[in] edge The edge which may be the turnaround direction
     * @return Whether the given edge is this edge's turnaround direction
     * (regardless of whether a connection exists)
     */
    bool isTurningDirectionAt(const NBEdge* const edge) const;

    /** @brief Sets the turing destination at the given edge
     * @param[in] e The turn destination
     * @param[in] onlyPossible If true, only sets myPossibleTurnDestination
     */
    void setTurningDestination(NBEdge* e, bool onlyPossible = false);

    /// @name Setting/getting special types
    /// @{
    /// @brief Marks this edge as a macroscopic connector
    void setAsMacroscopicConnector() {
        myAmMacroscopicConnector = true;
    }

    /** @brief Returns whether this edge was marked as a macroscopic connector
     * @return Whether this edge was marked as a macroscopic connector
     */
    bool isMacroscopicConnector() const {
        return myAmMacroscopicConnector;
    }

    /// @brief Marks this edge being within an intersection
    void setInsideTLS(bool inside) {
        myAmInTLS = inside;
    }

    /** @brief Returns whether this edge was marked as being within an intersection
     * @return Whether this edge was marked as being within an intersection
     */
    bool isInsideTLS() const {
        return myAmInTLS;
    }
    /// @}

    /** @brief Sets the junction priority of the edge
     * @param[in] node The node for which the edge's priority is given
     * @param[in] prio The edge's new priority at this node
     * @todo Maybe the edge priority whould be stored in the node
     */
    void setJunctionPriority(const NBNode* const node, int prio);

    /** @brief Returns the junction priority (normalised for the node currently build)
     *
     * If the given node is neither the edge's start nor the edge's ending node, the behaviour
     *  is undefined.
     *
     * @param[in] node The node for which the edge's priority shall be returned
     * @return The edge's priority at the given node
     * @todo Maybe the edge priority whould be stored in the node
     */
    int getJunctionPriority(const NBNode* const node) const;

    /// @brief set loaded length
    void setLoadedLength(double val);

    /// @brief patch average lane length in regard to the opposite edge
    void setAverageLengthWithOpposite(double val);

    /// @brief dimiss vehicle class information
    void dismissVehicleClassInformation();

    /// @brief get ID of type
    const std::string& getTypeID() const {
        return myType;
    }

    /// @brief whether at least one lane has values differing from the edges values
    bool needsLaneSpecificOutput() const;

    /// @brief whether at least one lane has restrictions
    bool hasPermissions() const;

    /// @brief whether lanes differ in allowed vehicle classes
    bool hasLaneSpecificPermissions() const;

    /// @brief whether lanes differ in speed
    bool hasLaneSpecificSpeed() const;

    /// @brief whether lanes differ in width
    bool hasLaneSpecificWidth() const;

    /// @brief whether lanes differ in type
    bool hasLaneSpecificType() const;

    /// @brief whether lanes differ in offset
    bool hasLaneSpecificEndOffset() const;

    /// @brief whether lanes differ in stopOffsets
    bool hasLaneSpecificStopOffsets() const;

    /// @brief whether one of the lanes is an acceleration lane
    bool hasAccelLane() const;

    /// @brief whether one of the lanes has a custom shape
    bool hasCustomLaneShape() const;

    /// @brief whether one of the lanes has parameters set
    bool hasLaneParams() const;

    /// @brief computes the edge (step1: computation of approached edges)
    bool computeEdge2Edges(bool noLeftMovers);

    /// @brief computes the edge, step2: computation of which lanes approach the edges)
    bool computeLanes2Edges();

    /// @brief recheck whether all lanes within the edge are all right and optimises the connections once again
    bool recheckLanes();

    /** @brief Add a connection to the previously computed turnaround, if wished
     * and a turning direction exists (myTurnDestination!=0)
     * @param[in] noTLSControlled Whether the turnaround shall not be connected if the edge is controlled by a tls
     * @param[in] noFringe Whether the turnaround shall not be connected if the junction is at the (outer) fringe
     * @param[in] onlyDeadends Whether the turnaround shall only be built at deadends
     * @param[in] onlyTurnlane Whether the turnaround shall only be built when there is an exclusive (left) turn lane
     * @param[in] noGeometryLike Whether the turnaround shall be built at geometry-like nodes
     */
    void appendTurnaround(bool noTLSControlled, bool noFringe, bool onlyDeadends, bool onlyTurnlane, bool noGeometryLike, bool checkPermissions);

    /** @brief Returns the node at the given edges length (using an epsilon)
        @note When no node is existing at the given position, 0 is returned
        The epsilon is a static member of NBEdge, should be setable via program options */
    NBNode* tryGetNodeAtPosition(double pos, double tolerance = 5.0) const;

    /// @brief get max lane offset
    double getMaxLaneOffset();

    /// @brief Check if lanes were assigned
    bool lanesWereAssigned() const;

    /// @brief return true if certain connection must be controlled by TLS
    bool mayBeTLSControlled(int fromLane, NBEdge* toEdge, int toLane) const;

    /// @brief Returns if the link could be set as to be controlled
    bool setControllingTLInformation(const NBConnection& c, const std::string& tlID);

    /// @brief clears tlID for all connections
    void clearControllingTLInformation();

    /// @brief add crossing points as incoming with given outgoing
    void addCrossingPointsAsIncomingWithGivenOutgoing(NBEdge* o, PositionVector& into);

    /// @brief get the outer boundary of this edge when going clock-wise around the given node
    PositionVector getCWBoundaryLine(const NBNode& n) const;

    /// @brief get the outer boundary of this edge when going counter-clock-wise around the given node
    PositionVector getCCWBoundaryLine(const NBNode& n) const;

    /// @brief Check if Node is expandable
    bool expandableBy(NBEdge* possContinuation, std::string& reason) const;

    /// @brief append another edge
    void append(NBEdge* continuation);

    /// @brief Check if edge has signalised connections
    bool hasSignalisedConnectionTo(const NBEdge* const e) const;

    /// @brief move outgoing connection
    void moveOutgoingConnectionsFrom(NBEdge* e, int laneOff);

    /* @brief return the turn destination if it exists
     * @param[in] possibleDestination Wether myPossibleTurnDestination should be returned if no turnaround connection
     * exists
     */
    NBEdge* getTurnDestination(bool possibleDestination = false) const;

    /// @brief get lane ID
    std::string getLaneID(int lane) const;

    /// @brief get lane speed
    double getLaneSpeed(int lane) const;

    /// @brief Check if edge is near enought to be joined to another edge
    bool isNearEnough2BeJoined2(NBEdge* e, double threshold) const;

    /** @brief Returns the angle of the edge's geometry at the given node
     *
     * The angle is signed, regards direction, and starts at 12 o'clock
     *  (north->south), proceeds positive clockwise.
     * @param[in] node The node for which the edge's angle shall be returned
     * @return This edge's angle at the given node
     */
    double getAngleAtNode(const NBNode* const node) const;

    /** @brief Returns the angle of from the node shape center to where the edge meets
     * the node shape
     *
     * The angle is signed, disregards direction, and starts at 12 o'clock
     *  (north->south), proceeds positive clockwise.
     * @param[in] node The node for which the edge's angle shall be returned
     * @return This edge's angle at the given node shape
     */
    double getAngleAtNodeToCenter(const NBNode* const node) const;

    /// @brief increment lane
    void incLaneNo(int by);

    /// @brief decrement lane
    void decLaneNo(int by);

    /// @brief delete lane
    void deleteLane(int index, bool recompute, bool shiftIndices);

    /// @brief add lane
    void addLane(int index, bool recomputeShape, bool recomputeConnections, bool shiftIndices);

    /// @brief mark edge as in lane to state lane
    void markAsInLane2LaneState();

    /// @brief add a pedestrian sidewalk of the given width and shift existing connctions
    void addSidewalk(double width);

    /// @brief restore an previously added sidewalk
    void restoreSidewalk(std::vector<NBEdge::Lane> oldLanes, PositionVector oldGeometry, std::vector<NBEdge::Connection> oldConnections);

    /// add a bicycle lane of the given width and shift existing connctions
    void addBikeLane(double width);

    /// @brief restore an previously added BikeLane
    void restoreBikelane(std::vector<NBEdge::Lane> oldLanes, PositionVector oldGeometry, std::vector<NBEdge::Connection> oldConnections);

    /// @brief add a lane of the given width, restricted to the given class and shift existing connections
    void addRestrictedLane(double width, SUMOVehicleClass vclass);

    /// @brief set allowed/disallowed classes for the given lane or for all lanes if -1 is given
    void setPermissions(SVCPermissions permissions, int lane = -1);

    /// @brief set preferred Vehicle Class
    void setPreferredVehicleClass(SVCPermissions permissions, int lane = -1);

    /// @brief set allowed class for the given lane or for all lanes if -1 is given
    void allowVehicleClass(int lane, SUMOVehicleClass vclass);

    /// @brief set disallowed class for the given lane or for all lanes if -1 is given
    void disallowVehicleClass(int lane, SUMOVehicleClass vclass);

    /// @brief prefer certain vehicle class
    void preferVehicleClass(int lane, SUMOVehicleClass vclass);

    /// @brief set lane specific width (negative lane implies set for all lanes)
    void setLaneWidth(int lane, double width);

    /// @brief set lane specific type (negative lane implies set for all lanes)
    void setLaneType(int lane, const std::string& type);

    /// @brief set lane specific end-offset (negative lane implies set for all lanes)
    void setEndOffset(int lane, double offset);

    /// @brief set lane specific speed (negative lane implies set for all lanes)
    void setSpeed(int lane, double speed);

    /// @brief set lane and vehicle class specific stopOffset (negative lane implies set for all lanes)
    /// @return Whether given stop offset was applied.
    bool setStopOffsets(int lane, std::map<int, double> offsets, bool overwrite = false);

    /// @brief marks one lane as acceleration lane
    void setAcceleration(int lane, bool accelRamp);

    /// @brief marks this edge has being an offRamp or leading to one (used for connection computation)
    void markOffRamp(bool isOffRamp) {
        myIsOffRamp = isOffRamp;
    }

    bool isOffRamp() const {
        return myIsOffRamp;
    }

    /// @brief sets a custom lane shape
    void setLaneShape(int lane, const PositionVector& shape);

    /// @brief get the union of allowed classes over all lanes or for a specific lane
    SVCPermissions getPermissions(int lane = -1) const;

    /// @brief set origID for all lanes
    void setOrigID(const std::string origID);

    /// @brief set lane specific speed (negative lane implies set for all lanes)
    void setDistance(double distance) {
        myDistance = distance;
    }

    /// @brief disable connections for TLS
    void disableConnection4TLS(int fromLane, NBEdge* toEdge, int toLane);

    // @brief returns a reference to the internal structure for the convenience of NETEDIT
    Lane& getLaneStruct(int lane) {
        assert(lane >= 0);
        assert(lane < (int)myLanes.size());
        return myLanes[lane];
    }

    // @brief returns a reference to the internal structure for the convenience of NETEDIT
    const Lane& getLaneStruct(int lane) const {
        assert(lane >= 0);
        assert(lane < (int)myLanes.size());
        return myLanes[lane];
    }

    /// @brief declares connections as fully loaded. This is needed to avoid recomputing connections if an edge has no connections intentionally.
    void declareConnectionsAsLoaded(EdgeBuildingStep step = EdgeBuildingStep::LANES2LANES_USER) {
        myStep = step;
    }

    /* @brief fill connection attributes shape, viaShape, ...
     *
     * @param[in,out] edgeIndex The number of connections already handled
     * @param[in,out] splitIndex The number of via edges already built
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     */
    void buildInnerEdges(const NBNode& n, int noInternalNoSplits, int& linkIndex, int& splitIndex);

    /// @brief get Signs
    inline const std::vector<NBSign>& getSigns() const {
        return mySigns;
    }

    /// @brief add Sign
    inline void addSign(NBSign sign) {
        mySigns.push_back(sign);
    }

    /// @brief cut shape at the intersection shapes
    PositionVector cutAtIntersection(const PositionVector& old) const;

    /// @brief Set Node border
    void setNodeBorder(const NBNode* node, const Position& p, const Position& p2, bool rectangularCut);
    const PositionVector& getNodeBorder(const NBNode* node);
    void resetNodeBorder(const NBNode* node);

    /// @brief whether this edge is part of a bidirectional railway
    bool isBidiRail(bool ignoreSpread = false) const;

    /// @brief whether this edge is a railway edge that does not continue
    bool isRailDeadEnd() const;

    /// @brief debugging helper to print all connections
    void debugPrintConnections(bool outgoing = true, bool incoming = false) const;

    /// @brief compute the first intersection point between the given lane geometries considering their rspective widths
    static double firstIntersection(const PositionVector& v1, const PositionVector& v2, double width2, const std::string& error = "");

    /** returns a modified version of laneShape which starts at the outside of startNode. laneShape may be shorted or extended
     * @note see [wiki:Developer/Network_Building_Process]
     */
    static PositionVector startShapeAt(const PositionVector& laneShape, const NBNode* startNode, PositionVector nodeShape);

    /// @name functions for router usage
    //@{

    static inline double getTravelTimeStatic(const NBEdge* const edge, const NBVehicle* const /*veh*/, double /*time*/) {
        return edge->getLength() / edge->getSpeed();
    }

    static int getLaneIndexFromLaneID(const std::string laneID);

    /// @brief sets the index of the edge in the list of all network edges
    void setNumericalID(int index) {
        myIndex = index;
    }

    /** @brief Returns the index (numeric id) of the edge
     * @note This is only used in the context of routing
     * @return This edge's numerical id
     */
    int getNumericalID() const {
        return myIndex;
    }

    const NBEdge* getBidiEdge() const {
        return isBidiRail() ? myPossibleTurnDestination : nullptr;
    }

    /** @brief Returns the following edges for the given vClass
     */
    const EdgeVector& getSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const;


    /** @brief Returns the following edges for the given vClass
     */
    const ConstRouterEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const;

    //@}
    const std::string& getID() const {
        return Named::getID();
    }

    /// @brief join adjacent lanes with the given permissions
    bool joinLanes(SVCPermissions perms);

    /// @brief reset lane shapes to what they would be before cutting with the junction shapes
    void resetLaneShapes();

    /// @brief return the straightest follower edge for the given permissions or nullptr (never returns turn-arounds)
    /// @note: this method is called before connections are built and simply goes by node graph topology
    NBEdge* getStraightContinuation(SVCPermissions permissions) const;

    /// @brief return the straightest predecessor edge for the given permissions or nullptr (never returns turn-arounds)
    /// @note: this method is called before connections are built and simply goes by node graph topology
    NBEdge* getStraightPredecessor(SVCPermissions permissions) const;

    /// @brief return only those edges that permit at least one of the give permissions
    static EdgeVector filterByPermissions(const EdgeVector& edges, SVCPermissions permissions);

private:
    /** @class ToEdgeConnectionsAdder
     * @brief A class that being a bresenham-callback assigns the incoming lanes to the edges
     */
    class ToEdgeConnectionsAdder : public Bresenham::BresenhamCallBack {
    private:
        /// @brief map of edges to this edge's lanes that reach them
        std::map<NBEdge*, std::vector<int> > myConnections;

        /// @brief the transition from the virtual lane to the edge it belongs to
        const EdgeVector& myTransitions;

    public:
        /// @brief constructor
        ToEdgeConnectionsAdder(const EdgeVector& transitions)
            : myTransitions(transitions) { }

        /// @brief destructor
        ~ToEdgeConnectionsAdder() { }

        /// @brief executes a bresenham - step
        void execute(const int lane, const int virtEdge);

        /// @brief get built connections
        const std::map<NBEdge*, std::vector<int> >& getBuiltConnections() const {
            return myConnections;
        }

    private:
        /// @brief Invalidated copy constructor.
        ToEdgeConnectionsAdder(const ToEdgeConnectionsAdder&);

        /// @brief Invalidated assignment operator.
        ToEdgeConnectionsAdder& operator=(const ToEdgeConnectionsAdder&);
    };


    /**
     * @class MainDirections
     * @brief Holds (- relative to the edge it is build from -!!!) the list of
     * main directions a vehicle that drives on this street may take on
     * the junction the edge ends in
     * The back direction is not regarded
     */
    class MainDirections {
    public:
        /// @brief enum of possible directions
        enum class Direction { 
            RIGHTMOST, 
            LEFTMOST, 
            FORWARD 
        };

    public:
        /// @brief constructor
        MainDirections(const EdgeVector& outgoing, NBEdge* parent, NBNode* to, const std::vector<int>& availableLanes);

        /// @brief destructor
        ~MainDirections();

        /// @brief returns the index of the straightmost among the given outgoing edges
        int getStraightest() const {
            return myStraightest;
        }

        /// @brief returns the information whether no following street has a higher priority
        bool empty() const;

        /// @brief returns the information whether the street in the given direction has a higher priority
        bool includes(Direction d) const;

    private:
        /// @brief the index of the straightmost among the given outgoing edges
        int myStraightest;

        /// @brief list of the main direction within the following junction relative to the edge
        std::vector<Direction> myDirs;

        /// @brief Invalidated copy constructor.
        MainDirections(const MainDirections&);

        /// @brief Invalidated assignment operator.
        MainDirections& operator=(const MainDirections&);
    };

    /// @brief Computes the shape for the given lane
    PositionVector computeLaneShape(int lane, double offset) const;

    /// @brief compute lane shapes
    void computeLaneShapes();

private:
    /** @brief Initialization routines common to all constructors
     *
     * Checks whether the number of lanes>0, whether the junction's from-
     *  and to-nodes are given (!=0) and whether they are distict. Throws
     *  a ProcessError if any of these checks fails.
     *
     * Adds the nodes positions to geometry if it shall not be ignored or
     *  if the geometry is empty.
     *
     * Computes the angle and length, and adds this edge to its node as
     *  outgoing/incoming. Builds lane informations.
     *
     * @param[in] noLanes The number of lanes this edge has
     * @param[in] tryIgnoreNodePositions Does not add node geometries if geom.size()>=2
     * @param[in] origID The original ID this edge had
     */
    void init(int noLanes, bool tryIgnoreNodePositions, const std::string& origID);

    /// @brief divides the lanes on the outgoing edges
    void divideOnEdges(const EdgeVector* outgoing);

    /// @brief divide selected lanes on edges
    void divideSelectedLanesOnEdges(const EdgeVector* outgoing, const std::vector<int>& availableLanes);

    /// @brief add some straight connections
    void addStraightConnections(const EdgeVector* outgoing, const std::vector<int>& availableLanes, const std::vector<int>& priorities);

    /// @brief recomputes the edge priorities and manipulates them for a distribution of lanes on edges which is more like in real-life
    const std::vector<int> prepareEdgePriorities(const EdgeVector* outgoing, const std::vector<int>& availableLanes);

    /// @name Setting and getting connections
    /// @{
    /** @briefmoves a connection one place to the left;
     * @note Attention! no checking for field validity
     */
    void moveConnectionToLeft(int lane);

    /** @briefmoves a connection one place to the right;
     * @noteAttention! no checking for field validity
     */
    void moveConnectionToRight(int lane);

    /// @brief whether the connection can originate on newFromLane
    bool canMoveConnection(const Connection& con, int newFromLane) const;
    /// @}

    /// @brief computes the angle of this edge and stores it in myAngle
    void computeAngle();

    /// @brief determine conflict between opposite left turns
    bool bothLeftIntersect(const NBNode& n, const PositionVector& shape, LinkDirection dir, NBEdge* otherFrom, const NBEdge::Connection& otherCon, int numPoints, double width2, int shapeFlag = 0) const;

    /// @brief returns whether any lane already allows the given vclass exclusively
    bool hasRestrictedLane(SUMOVehicleClass vclass) const;

    /// @brief restore a restricted lane
    void restoreRestrictedLane(SUMOVehicleClass vclass, std::vector<NBEdge::Lane> oldLanes, PositionVector oldGeometry, std::vector<NBEdge::Connection> oldConnections);

    /// @brief assign length to all lanes of an internal edge
    void assignInternalLaneLength(std::vector<Connection>::iterator i, int numLanes, double lengthSum);

private:
    /** @brief The building step
     * @see EdgeBuildingStep
     */
    EdgeBuildingStep myStep;

    /// @brief The type of the edge
    std::string myType;

    /// @brief The source and the destination node
    NBNode* myFrom, *myTo;

    /// @brief The length of the edge
    double myLength;

    /// @brief The angles of the edge
    /// @{
    double myStartAngle;
    double myEndAngle;
    double myTotalAngle;
    /// @}

    /// @brief The priority of the edge
    int myPriority;

    /// @brief The maximal speed
    double mySpeed;

    /// @brief The mileage/kilometrage at the start of this edge in a linear coordination system
    double myDistance;

    /** @brief List of connections to following edges
     * @see Connection
     */
    std::vector<Connection> myConnections;

    /// @brief List of connections marked for delayed removal
    std::vector<Connection> myConnectionsToDelete;

    /// @brief The turn destination edge (if a connection exists)
    NBEdge* myTurnDestination;

    /// @brief The edge that would be the turn destination if there was one
    NBEdge* myPossibleTurnDestination;

    /// @brief The priority normalised for the node the edge is outgoing of
    int myFromJunctionPriority;

    /// @brief The priority normalised for the node the edge is incoming in
    int myToJunctionPriority;

    /// @brief The geometry for the edge
    PositionVector myGeom;

    /// @brief The information about how to spread the lanes
    LaneSpreadFunction myLaneSpreadFunction;

    /// @brief This edges's offset to the intersection begin (will be applied to all lanes)
    double myEndOffset;

    /// @brief A vClass specific stop offset - assumed of length 0 (unspecified) or 1.
    ///        For the latter case the int is a bit set specifying the vClasses,
    ///        the offset applies to (see SUMOVehicleClass.h), and the double is the
    ///        stopping offset in meters from the lane end
    std::map<int, double> myStopOffsets;

    /// @brief This width of this edge's lanes
    double myLaneWidth;

    /** @brief Lane information
     * @see Lane
     */
    std::vector<Lane> myLanes;

    /// @brief An optional length to use (-1 if not valid)
    double myLoadedLength;

    /// @brief Information whether this is lies within a joined tls
    bool myAmInTLS;

    /// @brief Information whether this edge is a (macroscopic) connector
    bool myAmMacroscopicConnector;

    /// @brief The street name (or whatever arbitrary string you wish to attach)
    std::string myStreetName;

    /// @brief the street signs along this edge
    std::vector<NBSign> mySigns;

    /// @brief the position of a traffic light signal on this edge
    Position mySignalPosition;
    const NBNode* mySignalNode;

    /// @brief intersection borders (because the node shape might be invalid)
    /// @{
    PositionVector myFromBorder;
    PositionVector myToBorder;
    /// @}

    /// @brief whether this edge is an Off-Ramp or leads to one
    bool myIsOffRamp;

    /// @brief the index of the edge in the list of all edges. Set by NBEdgeCont and requires re-set whenever the list of edges changes
    int myIndex;

    // @brief a static list of successor edges. Set by NBEdgeCont and requires reset when the network changes
    mutable EdgeVector mySuccessors;

    // @brief a static list of successor edges. Set by NBEdgeCont and requires reset when the network changes
    mutable ConstRouterEdgePairVector myViaSuccessors;

    // @brief default length for overriding connection lengths
    static double myDefaultConnectionLength;

public:

    /// @class connections_toedge_finder
    class connections_toedge_finder {
    public:
        /// @brief constructor
        connections_toedge_finder(const NBEdge* const edge2find, bool hasFromLane = false) :
            myHasFromLane(hasFromLane),
            myEdge2Find(edge2find) { }

        /// @brief operator ()
        bool operator()(const Connection& c) const {
            return c.toEdge == myEdge2Find && (!myHasFromLane || c.fromLane != -1);
        }

    private:
        /// @brief check if has from lane
        const bool myHasFromLane;

        /// @brief edge to find
        const NBEdge* const myEdge2Find;

    private:
        /// @brief invalidated assignment operator
        connections_toedge_finder& operator=(const connections_toedge_finder& s);
    };

    /// @class connections_toedgelane_finder
    class connections_toedgelane_finder {
    public:
        /// @brief constructor
        connections_toedgelane_finder(NBEdge* const edge2find, int lane2find, int fromLane2find) :
            myEdge2Find(edge2find),
            myLane2Find(lane2find),
            myFromLane2Find(fromLane2find) { }

        /// @brief operator ()
        bool operator()(const Connection& c) const {
            return c.toEdge == myEdge2Find && c.toLane == myLane2Find && (myFromLane2Find < 0 || c.fromLane == myFromLane2Find);
        }

    private:
        /// @brief edge to find
        NBEdge* const myEdge2Find;

        /// @brief lane to find
        int myLane2Find;

        /// @brief from lane to find
        int myFromLane2Find;

    private:
        /// @brief invalidated assignment operator
        connections_toedgelane_finder& operator=(const connections_toedgelane_finder& s);

    };

    /// @class connections_finder
    class connections_finder {
    public:
        /// @brief constructor
        connections_finder(int fromLane, NBEdge* const edge2find, int lane2find, bool invertEdge2find = false) :
            myFromLane(fromLane), myEdge2Find(edge2find), myLane2Find(lane2find), myInvertEdge2find(invertEdge2find) { }

        /// @brief operator ()
        bool operator()(const Connection& c) const {
            return ((c.fromLane == myFromLane || myFromLane == -1)
                    && ((!myInvertEdge2find && c.toEdge == myEdge2Find) || (myInvertEdge2find && c.toEdge != myEdge2Find))
                    && (c.toLane == myLane2Find || myLane2Find == -1));
        }

    private:
        /// @brief index of from lane
        int myFromLane;

        /// @brief edge to find
        NBEdge* const myEdge2Find;

        /// @brief lane to find
        int myLane2Find;

        /// @brief invert edge to find
        bool myInvertEdge2find;

    private:
        /// @brief invalidated assignment operator
        connections_finder& operator=(const connections_finder& s);

    };

    /// @class connections_conflict_finder
    class connections_conflict_finder {
    public:
        /// @brief constructor
        connections_conflict_finder(int fromLane, NBEdge* const edge2find, bool checkRight) :
            myFromLane(fromLane), myEdge2Find(edge2find), myCheckRight(checkRight) { }

        /// @brief operator ()
        bool operator()(const Connection& c) const {
            return (((myCheckRight && c.fromLane < myFromLane) || (!myCheckRight && c.fromLane > myFromLane))
                    && c.fromLane >= 0 // already assigned
                    && c.toEdge == myEdge2Find);
        }

    private:
        /// @brief index of from lane
        int myFromLane;

        /// @brief edge to find
        NBEdge* const myEdge2Find;

        /// @brief check if is right
        bool myCheckRight;

    private:
        /// @brief invalidated assignment operator
        connections_conflict_finder& operator=(const connections_conflict_finder& s);

    };

    /// @class connections_fromlane_finder
    class connections_fromlane_finder {
    public:
        /// @briefconstructor
        connections_fromlane_finder(int lane2find) : myLane2Find(lane2find) { }

        /// @brief operator ()
        bool operator()(const Connection& c) const {
            return c.fromLane == myLane2Find;
        }

    private:
        /// @brief index of lane to find
        int myLane2Find;

    private:
        /// @brief invalidated assignment operator
        connections_fromlane_finder& operator=(const connections_fromlane_finder& s);

    };

    /// @brief connections_sorter sort by fromLane, toEdge and toLane
    static bool connections_sorter(const Connection& c1, const Connection& c2);

    /**
     * @class connections_relative_edgelane_sorter
     * @brief Class to sort edges by their angle
     */
    class connections_relative_edgelane_sorter {
    public:
        /// @brief constructor
        explicit connections_relative_edgelane_sorter(NBEdge* e) : myEdge(e) {}

    public:
        /// @brief comparing operation
        int operator()(const Connection& c1, const Connection& c2) const;

    private:
        /// @brief the edge to compute the relative angle of
        NBEdge* myEdge;
    };

private:
    /// @brief invalidated copy constructor
    NBEdge(const NBEdge& s);

    /// @brief invalidated assignment operator
    NBEdge& operator=(const NBEdge& s);

    /// @brief constructor for dummy edge
    NBEdge();

};
