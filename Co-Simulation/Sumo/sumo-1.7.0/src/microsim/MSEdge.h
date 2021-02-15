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
/// @file    MSEdge.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 12 Mar 2001
///
// A road/street connecting two junctions
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#ifdef HAVE_FOX
#include <fx.h>
#endif
#include <utils/common/Named.h>
#include <utils/common/Parameterised.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/geom/Boundary.h>
#include <utils/router/ReversedEdge.h>
#include <utils/router/RailEdge.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/vehicle/SUMOTrafficObject.h>
#include "MSNet.h"


// ===========================================================================
// class declarations
// ===========================================================================
class Boundary;
class OutputDevice;
class SUMOVehicle;
class SUMOVehicleParameter;
class MSVehicle;
class MSLane;
class MSLaneChanger;
class MSPerson;
class MSJunction;
class MSEdge;
class MSTransportable;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSEdge
 * @brief A road/street connecting two junctions
 *
 * A single connection between two junctions.
 * Holds lanes which are reponsible for vehicle movements.
 */

typedef std::vector<MSEdge*> MSEdgeVector;
typedef std::vector<const MSEdge*> ConstMSEdgeVector;
typedef std::vector<std::pair<const MSEdge*, const MSEdge*> > MSConstEdgePairVector;

class MSEdge : public Named, public Parameterised {
private:
    /** @brief "Map" from vehicle class to allowed lanes */
    typedef std::vector<std::pair<SVCPermissions, std::shared_ptr<const std::vector<MSLane*> > > > AllowedLanesCont;

    /** @brief Succeeding edges (keys) and allowed lanes to reach these edges (values). */
    typedef std::map<const MSEdge*, AllowedLanesCont> AllowedLanesByTarget;


public:
    /** @brief Constructor.
     *
     * After calling this constructor, the edge is not yet initialised
     *  completely. A call to "initialize" with proper values is needed
     *  for this.
     *
     * @param[in] id The id of the edge
     * @param[in] numericalID The numerical id (index) of the edge
     * @param[in] function A basic type of the edge
     * @param[in] streetName The street name for that edge
     */
    MSEdge(const std::string& id, int numericalID, const SumoXMLEdgeFunc function,
           const std::string& streetName, const std::string& edgeType, int priority,
           double distance);


    /// @brief Destructor.
    virtual ~MSEdge();


    /** @brief Initialize the edge.
     *
     * @param[in] allowed Information which edges may be reached from which lanes
     * @param[in] lanes List of this edge's lanes
     */
    void initialize(const std::vector<MSLane*>* lanes);


    /** @brief Recalculates the cached values
     */
    void recalcCache();


    /// @todo Has to be called after all edges were built and all connections were set...; Still, is not very nice
    virtual void closeBuilding();

    /// Has to be called after all sucessors and predecessors have been set (after closeBuilding())
    void buildLaneChanger();

    /* @brief returns whether initizliaing a lane change is permitted on this edge
     * @note Has to be called after all sucessors and predecessors have been set (after closeBuilding())
     */
    bool allowsLaneChanging() const;

    /// @name Access to the edge's lanes
    /// @{

    /** @brief Returns the lane left to the one given, 0 if the given lane is leftmost
     *
     * @param[in] lane The lane right to the one to be returned
     * @return The lane left to the given, 0 if no such lane exists
     * @todo This method searches for the given in the container; probably, this could be done faster
     */
    MSLane* leftLane(const MSLane* const lane) const;


    /** @brief Returns the lane right to the one given, 0 if the given lane is rightmost
     *
     * @param[in] lane The lane left to the one to be returned
     * @return The lane right to the given, 0 if no such lane exists
     * @todo This method searches for the given in the container; probably, this could be done faster
     */
    MSLane* rightLane(const MSLane* const lane) const;


    /** @brief Returns the lane with the given offset parallel to the given lane one or 0 if it does not exist
     *
     * @param[in] lane The base lane
     * @param[in] offset The offset of the result lane
     * @param[in] includeOpposte Whether an opposite direction lane may be returned
     * @todo This method searches for the given in the container; probably, this could be done faster
     */
    MSLane* parallelLane(const MSLane* const lane, int offset, bool includeOpposite = true) const;


    /** @brief Returns this edge's lanes
     *
     * @return This edge's lanes
     */
    inline const std::vector<MSLane*>& getLanes() const {
        return *myLanes;
    }

    /// @brief return total number of vehicles on this edges lanes or segments
    int getVehicleNumber() const;

    /// @brief return vehicles on this edges lanes or segments
    std::vector<const SUMOVehicle*> getVehicles() const;

    double getBruttoOccupancy() const;

    /// @brief return flow based on meanSpead @note: may produced incorrect results when jammed
    double getFlow() const;

    /// @brief return accumated waiting time for all vehicles on this edges lanes or segments
    double getWaitingSeconds() const;

    /// @brief return mean occupancy on this edges lanes or segments
    double getOccupancy() const;

    /** @brief Returns this edge's persons set.
     *  @brief Avoids the creation of new vector as in getSortedPersons
     *
     * @return This edge's persons.
     */
    inline const std::set<MSTransportable*>& getPersons() const {
        return myPersons;
    }

    /** @brief Returns this edge's persons sorted by pos
     *
     * @return This edge's persons sorted by pos
     */
    std::vector<MSTransportable*> getSortedPersons(SUMOTime timestep, bool includeRiding = false) const;


    /** @brief Returns this edge's containers sorted by pos
     *
     * @return This edge's containers sorted by pos
     */
    std::vector<MSTransportable*> getSortedContainers(SUMOTime timestep, bool includeRiding = false) const;

    /** @brief Get the allowed lanes to reach the destination-edge.
     *
     * If there is no such edge, return nullptr. Then you are on the wrong edge.
     *
     * @param[in] destination The edge to reach
     * @param[in] vclass The vehicle class for which this information shall be returned
     * @return The lanes that may be used to reach the given edge, nullptr if no such lanes exist
     */
    const std::vector<MSLane*>* allowedLanes(const MSEdge& destination,
            SUMOVehicleClass vclass = SVC_IGNORING) const;



    /** @brief Get the allowed lanes for the given vehicle class.
     *
     * If there is no such edge, return nullptr. Then you are on the wrong edge.
     *
     * @param[in] vclass The vehicle class for which this information shall be returned
     * @return The lanes that may be used by the given vclass
     */
    const std::vector<MSLane*>* allowedLanes(SUMOVehicleClass vclass = SVC_IGNORING) const;

    inline bool isConnectedTo(const MSEdge& destination, SUMOVehicleClass vclass) const {
        const std::vector<MSLane*>* const lanes = allowedLanes(destination, vclass);
        return lanes != nullptr && !lanes->empty();
    }
    /// @}



    /// @name Access to other edge attributes
    /// @{

    /** @brief Returns the edge type (SumoXMLEdgeFunc)
     * @return This edge's SumoXMLEdgeFunc
     * @see SumoXMLEdgeFunc
     */
    inline SumoXMLEdgeFunc getFunction() const {
        return myFunction;
    }

    /// @brief return whether this edge is an internal edge
    inline bool isNormal() const {
        return myFunction == SumoXMLEdgeFunc::NORMAL;
    }

    /// @brief return whether this edge is an internal edge
    inline bool isInternal() const {
        return myFunction == SumoXMLEdgeFunc::INTERNAL;
    }

    /// @brief return whether this edge is a pedestrian crossing
    inline bool isCrossing() const {
        return myFunction == SumoXMLEdgeFunc::CROSSING;
    }


    /// @brief check and register the opposite superposable edge if any
    void checkAndRegisterBiDirEdge(const std::string& bidiID = "");

    /// @brief return opposite superposable/congruent edge, if it exist and 0 else
    inline const MSEdge* getBidiEdge() const {
        return myBidiEdge;
    }

    /// @brief return whether this edge is walking area
    inline bool isWalkingArea() const {
        return myFunction == SumoXMLEdgeFunc::WALKINGAREA;
    }

    inline bool isTazConnector() const {
        return myFunction == SumoXMLEdgeFunc::CONNECTOR;
    }

    void setOtherTazConnector(const MSEdge* edge) {
        myOtherTazConnector = edge;
    }

    const MSEdge* getOtherTazConnector() const {
        return myOtherTazConnector;
    }

    /** @brief Returns the numerical id of the edge
     * @return This edge's numerical id
     */
    inline int getNumericalID() const {
        return myNumericalID;
    }


    /** @brief Returns the street name of the edge
     */
    const std::string& getStreetName() const {
        return myStreetName;
    }

    /** @brief Returns the type of the edge
     */
    const std::string& getEdgeType() const {
        return myEdgeType;
    }

    /** @brief Returns the priority of the edge
     */
    int getPriority() const {
        return myPriority;
    }

    /** @brief Returns the kilometrage/mileage at the start of the edge
     */
    double getDistance() const {
        return myDistance;
    }
    /// @}

    /**@brief Sets the crossed edge ids for a crossing edge
     *
     */
    void setCrossingEdges(const std::vector<std::string>& crossingEdges)		{
        myCrossingEdges.clear();
        myCrossingEdges.insert(myCrossingEdges.begin(), crossingEdges.begin(), crossingEdges.end());
    }

    /**@brief Gets the crossed edge ids
     *@return The list of crossed edge ids in a crossing edge or an empty vector
     */
    const std::vector<std::string>& getCrossingEdges() const {
        return myCrossingEdges;
    }


    /// @name Access to succeeding/predecessing edges
    /// @{

    /** @brief Adds an edge to the list of edges which may be reached from this edge and to the incoming of the other edge
     *
     * This is mainly used by the taz (district) parsing
     * @param[in] edge The edge to add
     */
    void addSuccessor(MSEdge* edge, const MSEdge* via = nullptr);

    /** @brief Returns the number of edges that may be reached from this edge
     * @return The number of following edges
     */
    int getNumSuccessors() const {
        return (int) mySuccessors.size();
    }


    /** @brief Returns the following edges, restricted by vClass
     * @param[in] vClass The vClass for which to restrict the successors
     * @return The eligible following edges
     */
    const MSEdgeVector& getSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const;

    /** @brief Returns the following edges with internal vias, restricted by vClass
     * @param[in] vClass The vClass for which to restrict the successors
     * @return The eligible following edges
     */
    const MSConstEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const;


    /** @brief Returns the number of edges this edge is connected to
     *
     * @return The number of edges following this edge
     */
    int getNumPredecessors() const {
        return (int) myPredecessors.size();
    }


    /** @brief
     * @return
     */
    const MSEdgeVector& getPredecessors() const {
        return myPredecessors;
    }


    const MSJunction* getFromJunction() const {
        return myFromJunction;
    }

    const MSJunction* getToJunction() const {
        return myToJunction;
    }


    void setJunctions(MSJunction* from, MSJunction* to);
    /// @}



    /// @name Access to vaporizing interface
    /// @{

    /** @brief Returns whether vehicles on this edge shall be vaporized
     * @return Whether no vehicle shall be on this edge
     */
    bool isVaporizing() const {
        return myVaporizationRequests > 0;
    }


    /** @brief Enables vaporization
     *
     * The internal vaporization counter is increased enabling the
     *  vaporization.
     * Called from the event handler.
     * @param[in] t The current time (unused)
     * @return Time to next call (always 0)
     * @exception ProcessError not thrown by this method, just derived
     */
    SUMOTime incVaporization(SUMOTime t);


    /** @brief Disables vaporization
     *
     * The internal vaporization counter is decreased what disables
     *  the vaporization if it was only once enabled.
     * Called from the event handler.
     * @param[in] t The current time (unused)
     * @return Time to next call (always 0)
     * @exception ProcessError not thrown by this method, just derived
     */
    SUMOTime decVaporization(SUMOTime t);
    /// @}


    /** @brief Computes and returns the current travel time for this edge
     *
     * The mean speed of all lanes is used to compute the travel time.
     * To avoid infinite travel times, the given minimum speed is used.
     *
     * @param[in] minSpeed The minimumSpeed to assume if traffic on this edge is stopped
     * @return The current effort (travel time) to pass the edge
     */
    double getCurrentTravelTime(const double minSpeed = NUMERICAL_EPS) const;


    /// @brief returns the minimum travel time for the given vehicle
    inline double getMinimumTravelTime(const SUMOVehicle* const veh) const {
        if (myFunction == SumoXMLEdgeFunc::CONNECTOR) {
            return 0;
        } else if (veh != 0) {
            return getLength() / getVehicleMaxSpeed(veh) + myTimePenalty;
        } else {
            return myEmptyTraveltime;
        }
    }


    /** @brief Returns the travel time for the given edge
     *
     * @param[in] edge The edge for which the travel time shall be retrieved
     * @param[in] veh The vehicle for which the travel time on this edge shall be retrieved
     * @param[in] time The time for which the travel time shall be returned [s]
     * @return The traveltime needed by the given vehicle to pass the edge at the given time
     */
    static inline double getTravelTimeStatic(const MSEdge* const edge, const SUMOVehicle* const veh, double time) {
        return MSNet::getInstance()->getTravelTime(edge, veh, time);
    }

    static double getTravelTimeAggregated(const MSEdge* const edge, const SUMOVehicle* const veh, double time);

    /** @brief Returns the averaged speed used by the routing device
     */
    double getRoutingSpeed() const;


    /// @name Methods releated to vehicle insertion
    /// @{

    /** @brief Tries to insert the given vehicle into the network
     *
     * The procedure for choosing the proper lane is determined, first.
     *  In dependance to this, the proper lane is chosen.
     *
     * Insertion itself is done by calling the chose lane's "insertVehicle"
     *  method but only if the checkOnly argument is false. The check needs
     *  to be certain only in the negative case (if false is returned, there
     *  is no way this vehicle would be inserted).
     *
     * @param[in] v The vehicle to insert
     * @param[in] time The current simulation time
     * @param[in] checkOnly Whether we perform only the check without actually inserting
     * @param[in] forceCheck Whether the full insertion check should be run for each pending vehicle
     *            or whether insertion on lanes for which an insertion has already a failed should be ignored
     *            in the current time step.
     * @return Whether the vehicle could be inserted
     * @see MSLane::insertVehicle
     */
    bool insertVehicle(SUMOVehicle& v, SUMOTime time, const bool checkOnly = false, const bool forceCheck = false) const;

    /// @brief check whether the given departSpeed is valid for this edge
    bool validateDepartSpeed(SUMOVehicle& v) const;

    /** @brief Finds the emptiest lane allowing the vehicle class
     *
     * The emptiest lane is the one which vehicle insertion is most likely to succeed.
     *
     * If there are no vehicles before departPos, then the lane with the largest
     * gap between departPos and the last vehicle is
     * Otheriwise the lane with lowes occupancy is selected
     * If there is more than one, the first according to its
     *  index in the lane container is chosen.
     *
     * If allowed==0, the lanes allowed for the given vehicle class
     *  will be used.
     *
     * @param[in] allowed The lanes to choose from
     * @param[in] vclass The vehicle class to look for
     * @param[in] departPos An upper bound on vehicle depart position
     * @return the least occupied lane
     * @see allowedLanes
     */
    MSLane* getFreeLane(const std::vector<MSLane*>* allowed, const SUMOVehicleClass vclass, double departPos) const;


    /** @brief Finds a depart lane for the given vehicle parameters
     *
     * Depending on the depart lane procedure a depart lane is chosen.
     *  Repeated calls with the same vehicle may return different results
     *  if the procedure is "random" or "free". In case no appropriate
     *  lane was found, 0 is returned.
     *
     * @param[in] veh The vehicle to get the depart lane for
     * @return a possible/chosen depart lane, 0 if no lane can be used
     */
    MSLane* getDepartLane(MSVehicle& veh) const;


    /** @brief Returns the last time a vehicle could not be inserted
     * @return The current value
     */
    inline SUMOTime getLastFailedInsertionTime() const {
        return myLastFailedInsertionTime;
    }


    /** @brief Sets the last time a vehicle could not be inserted
     * @param[in] time the new value
     */
    inline void setLastFailedInsertionTime(SUMOTime time) const {
        myLastFailedInsertionTime = time;
    }
    /// @}


    /** @brief Performs lane changing on this edge */
    void changeLanes(SUMOTime t) const;


    /// @todo extension: inner junctions are not filled
    const MSEdge* getInternalFollowingEdge(const MSEdge* followerAfterInternal) const;


    /// @brief returns the length of all internal edges on the junction until reaching the non-internal edge followerAfterInternal.
    double getInternalFollowingLengthTo(const MSEdge* followerAfterInternal) const;

    /// @brief if this edge is an internal edge, return its first normal predecessor, otherwise the edge itself
    const MSEdge* getNormalBefore() const;

    /// @brief Returns whether the vehicle (class) is not allowed on the edge
    inline bool prohibits(const SUMOVehicle* const vehicle) const {
        if (vehicle == nullptr) {
            return false;
        }
        const SUMOVehicleClass svc = vehicle->getVClass();
        return (myCombinedPermissions & svc) != svc;
    }

    /** @brief Returns whether this edge has restriction parameters forbidding the given vehicle to pass it
     * The restriction mechanism is not implemented yet for the microsim, so it always returns false.
     * @param[in] vehicle The vehicle for which the information has to be returned
     * @return Whether the vehicle must not enter this edge
     */
    inline bool restricts(const SUMOVehicle* const /* vehicle */) const {
        return false;
    }

    /// @brief Returns the combined permissions of all lanes of this edge
    inline SVCPermissions getPermissions() const {
        return myCombinedPermissions;
    }

    /** @brief Returns the edges's width (sum over all lanes)
     * @return This edges's width
     */
    double getWidth() const {
        return myWidth;
    }

    /// @brief Returns the right side offsets of this edge's sublanes
    const std::vector<double> getSubLaneSides() const {
        return mySublaneSides;
    }

    void rebuildAllowedLanes();

    void rebuildAllowedTargets(const bool updateVehicles = true);


    /** @brief optimistic air distance heuristic for use in routing
     * @param[in] other The edge to which the distance shall be returned
     * @param[in] doBoundaryEstimate whether the distance should be estimated by looking at the distance of the bounding boxes
     * @return The distance to the other edge
     */
    double getDistanceTo(const MSEdge* other, const bool doBoundaryEstimate = false) const;


    /// @brief return the coordinates of the center of the given stop
    static const Position getStopPosition(const SUMOVehicleParameter::Stop& stop);


    /** @brief return the length of the edge
     * @return The edge's length
     */
    inline double getLength() const {
        return myLength;
    }


    /** @brief Returns the speed limit of the edge
     * @caution The speed limit of the first lane is retured; should probably be the fastest edge
     * @return The maximum speed allowed on this edge
     */
    double getSpeedLimit() const;

    /// @brief return shape.length() / myLength
    double getLengthGeometryFactor() const;

    /** @brief Sets a new maximum speed for all lanes (used by TraCI and MSCalibrator)
     * @param[in] val the new speed in m/s
     */
    void setMaxSpeed(double val) const;

    /** @brief Returns the maximum speed the vehicle may use on this edge
     *
     * @caution Only the first lane is considered
     * @return The maximum velocity on this edge for the given vehicle
     */
    double getVehicleMaxSpeed(const SUMOTrafficObject* const veh) const;


    virtual void addPerson(MSTransportable* p) const;

    virtual void removePerson(MSTransportable* p) const;

    /// @brief Add a container to myContainers
    virtual void addContainer(MSTransportable* container) const {
        myContainers.insert(container);
    }

    /// @brief Remove container from myContainers
    virtual void removeContainer(MSTransportable* container) const {
        std::set<MSTransportable*>::iterator i = myContainers.find(container);
        if (i != myContainers.end()) {
            myContainers.erase(i);
        }
    }

    inline bool isRoundabout() const {
        return myAmRoundabout;
    }

    void markAsRoundabout() {
        myAmRoundabout = true;
    }

    void markDelayed() const {
        myAmDelayed = true;
    }

    // return whether there have been vehicles on this edge at least once
    inline bool isDelayed() const {
        return myAmDelayed || myBidiEdge == nullptr || myBidiEdge->myAmDelayed;
    }

    bool hasLaneChanger() const {
        return myLaneChanger != 0;
    }

    /// @brief whether this edge allows changing to the opposite direction edge
    bool canChangeToOpposite();

    /// @brief Returns the opposite direction edge if on exists else a nullptr
    const MSEdge* getOppositeEdge() const;

    /// @brief get the mean speed
    double getMeanSpeed() const;

    /// @brief get the mean speed of all bicycles on this edge
    double getMeanSpeedBike() const;

    /// @brief whether any lane has a minor link
    bool hasMinorLink() const;

    /// @brief return whether this edge is at the fringe of the network
    bool isFringe() const {
        return myAmFringe;
    }

    /// @brief whether this lane is selected in the GUI
    virtual bool isSelected() const {
        return false;
    }

    /// @brief grant exclusive access to the mesoscopic state
    virtual void lock() const {}

    /// @brief release exclusive access to the mesoscopic state
    virtual void unlock() const {};

    /// @brief Adds a vehicle to the list of waiting vehicles
    void addWaiting(SUMOVehicle* vehicle) const;

    /// @brief Removes a vehicle from the list of waiting vehicles
    void removeWaiting(const SUMOVehicle* vehicle) const;

    /* @brief returns a vehicle that is waiting for a for a person or a container at this edge at the given position
     * @param[in] transportable The person or container that wants to ride
     * @param[in] position The vehicle shall be positioned in the interval [position - t, position + t], where t is some tolerance
     */
    SUMOVehicle* getWaitingVehicle(MSTransportable* transportable, const double position) const;

    /** @brief Inserts edge into the static dictionary
        Returns true if the key id isn't already in the dictionary. Otherwise
        returns false. */
    static bool dictionary(const std::string& id, MSEdge* edge);

    /** @brief Returns the MSEdge associated to the key id if exists, otherwise returns 0. */
    static MSEdge* dictionary(const std::string& id);

    /// @brief Returns the number of edges
    static int dictSize();

    /// @brief Returns all edges with a numerical id
    static const MSEdgeVector& getAllEdges();

    /** @brief Clears the dictionary */
    static void clear();

    /** @brief Inserts IDs of all known edges into the given vector */
    static void insertIDs(std::vector<std::string>& into);


public:
    /// @name Static parser helper
    /// @{

    /** @brief Parses the given string assuming it contains a list of edge ids divided by spaces
     *
     * Splits the string at spaces, uses polymorph method to generate edge vector.
     * @param[in] desc The string containing space-separated edge ids
     * @param[out] into The vector to fill
     * @param[in] rid The id of the route these description belongs to; used for error message generation
     * @exception ProcessError If one of the strings contained is not a known edge id
     */
    static void parseEdgesList(const std::string& desc, ConstMSEdgeVector& into,
                               const std::string& rid);


    /** @brief Parses the given string vector assuming it edge ids
     * @param[in] desc The string vector containing edge ids
     * @param[out] into The vector to fill
     * @param[in] rid The id of the route these description belongs to; used for error message generation
     * @exception ProcessError If one of the strings contained is not a known edge id
     */
    static void parseEdgesList(const std::vector<std::string>& desc, ConstMSEdgeVector& into,
                               const std::string& rid);
    /// @}


    ReversedEdge<MSEdge, SUMOVehicle>* getReversedRoutingEdge() const {
        if (myReversedRoutingEdge == nullptr) {
            myReversedRoutingEdge = new ReversedEdge<MSEdge, SUMOVehicle>(this);
        }
        return myReversedRoutingEdge;
    }

    RailEdge<MSEdge, SUMOVehicle>* getRailwayRoutingEdge() const {
        if (myRailwayRoutingEdge == nullptr) {
            myRailwayRoutingEdge = new RailEdge<MSEdge, SUMOVehicle>(this);
        }
        return myRailwayRoutingEdge;
    }

protected:
    /** @class by_id_sorter
     * @brief Sorts edges by their ids
     */
    class by_id_sorter {
    public:
        /// @brief constructor
        explicit by_id_sorter() { }

        /// @brief comparing operator
        int operator()(const MSEdge* const e1, const MSEdge* const e2) const {
            return e1->getNumericalID() < e2->getNumericalID();
        }

    };

    /** @class transportable_by_position_sorter
     * @brief Sorts transportables by their positions
     */
    class transportable_by_position_sorter {
    public:
        /// @brief constructor
        explicit transportable_by_position_sorter(SUMOTime timestep): myTime(timestep) { }

        /// @brief comparing operator
        int operator()(const MSTransportable* const c1, const MSTransportable* const c2) const;
    private:
        SUMOTime myTime;
    };


    /// @brief return upper bound for the depart position on this edge
    double getDepartPosBound(const MSVehicle& veh, bool upper = true) const;

protected:
    /// @brief This edge's numerical id
    const int myNumericalID;

    /// @brief Container for the edge's lane; should be sorted: (right-hand-traffic) the more left the lane, the higher the container-index
    std::shared_ptr<const std::vector<MSLane*> > myLanes;

    /// @brief This member will do the lane-change
    MSLaneChanger* myLaneChanger;

    /// @brief the purpose of the edge
    const SumoXMLEdgeFunc myFunction;

    /// @brief Vaporizer counter
    int myVaporizationRequests;

    /// @brief The time of last insertion failure
    mutable SUMOTime myLastFailedInsertionTime;

    /// @brief A cache for the rejected insertion attempts. Used to assure that no
    ///        further insertion attempts are made on a lane where an attempt has
    ///        already failed in the current time step if MSInsertionControl::myEagerInsertionCheck is off.
    mutable std::set<int> myFailedInsertionMemory;

    /// @brief The crossed edges id for a crossing edge. On not crossing edges it is empty
    std::vector<std::string> myCrossingEdges;

    /// @brief The succeeding edges
    MSEdgeVector mySuccessors;

    MSConstEdgePairVector myViaSuccessors;

    /// @brief The preceeding edges
    MSEdgeVector myPredecessors;

    /// @brief the junctions for this edge
    MSJunction* myFromJunction;
    MSJunction* myToJunction;

    /// @brief Persons on the edge for drawing and pushbutton
    mutable std::set<MSTransportable*> myPersons;

    /// @brief Containers on the edge
    mutable std::set<MSTransportable*> myContainers;

    /// @name Storages for allowed lanes (depending on vehicle classes)
    /// @{

    /// @brief Associative container from vehicle class to allowed-lanes.
    AllowedLanesCont myAllowed;

    /// @brief From target edge to lanes allowed to be used to reach it
    AllowedLanesByTarget myAllowedTargets;

    /// @brief The intersection of lane permissions for this edge
    SVCPermissions myMinimumPermissions = SVCAll;
    /// @brief The union of lane permissions for this edge
    SVCPermissions myCombinedPermissions = 0;
    /// @}

    /// @brief the other taz-connector if this edge isTazConnector, otherwise nullptr
    const MSEdge* myOtherTazConnector;

    /// @brief the real-world name of this edge (need not be unique)
    std::string myStreetName;

    /// @brief the type of the edge (optionally used during network creation)
    std::string myEdgeType;

    /// @brief the priority of the edge (used during network creation)
    const int myPriority;

    /// @brief the kilometrage/mileage at the start of the edge
    const double myDistance;

    /// Edge width [m]
    double myWidth;

    /// @brief the length of the edge (cached value for speedup)
    double myLength;

    /// @brief the traveltime on the empty edge (cached value for speedup)
    double myEmptyTraveltime;

    /// @brief flat penalty when computing traveltime
    double myTimePenalty;

    /// @brief whether this edge had a vehicle with less than max speed on it
    mutable bool myAmDelayed;

    /// @brief whether this edge belongs to a roundabout
    bool myAmRoundabout;

    /// @brief whether this edge is at the network fringe
    bool myAmFringe;

    /// @brief the right side for each sublane on this edge
    std::vector<double> mySublaneSides;

    /// @name Static edge container
    /// @{

    /// @brief definition of the static dictionary type
    typedef std::map< std::string, MSEdge* > DictType;

    /** @brief Static dictionary to associate string-ids with objects.
     * @deprecated Move to MSEdgeControl, make non-static
     */
    static DictType myDict;

    /** @brief Static list of edges
     * @deprecated Move to MSEdgeControl, make non-static
     */
    static MSEdgeVector myEdges;
    /// @}


    /// @brief The successors available for a given vClass
    mutable std::map<SUMOVehicleClass, MSEdgeVector> myClassesSuccessorMap;

    /// @brief The successors available for a given vClass
    mutable std::map<SUMOVehicleClass, MSConstEdgePairVector> myClassesViaSuccessorMap;

    /// @brief The bounding rectangle of end nodes incoming or outgoing edges for taz connectors or of my own start and end node for normal edges
    Boundary myBoundary;

    /// @brief List of waiting vehicles
    mutable std::vector<SUMOVehicle*> myWaiting;

#ifdef HAVE_FOX
    /// @brief Mutex for accessing waiting vehicles
    mutable FXMutex myWaitingMutex;

    /// @brief Mutex for accessing successor edges
    mutable FXMutex mySuccessorMutex;
#endif

private:

    /// @brief the oppositing superposable edge
    const MSEdge* myBidiEdge;

    /// @brief a reversed version for backward routing
    mutable ReversedEdge<MSEdge, SUMOVehicle>* myReversedRoutingEdge = nullptr;
    mutable RailEdge<MSEdge, SUMOVehicle>* myRailwayRoutingEdge = nullptr;

    /// @brief Invalidated copy constructor.
    MSEdge(const MSEdge&);

    /// @brief assignment operator.
    MSEdge& operator=(const MSEdge&) = delete;

    bool isSuperposable(const MSEdge* other);

    void addToAllowed(const SVCPermissions permissions, std::shared_ptr<const std::vector<MSLane*> > allowedLanes, AllowedLanesCont& laneCont) const;
};
