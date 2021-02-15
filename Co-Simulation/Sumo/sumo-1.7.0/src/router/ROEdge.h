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
/// @file    ROEdge.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @author  Melanie Knocke
/// @author  Yun-Pang Floetteroed
/// @date    Sept 2002
///
// A basic edge for routing applications
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <utils/common/Named.h>
#include <utils/common/StdDefs.h>
#include <utils/common/ValueTimeLine.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/RandHelper.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/geom/Boundary.h>
#ifdef HAVE_FOX
#include <fx.h>
#endif
#include <utils/vehicle/SUMOVTypeParameter.h>
#include "RONode.h"
#include "ROVehicle.h"


// ===========================================================================
// class declarations
// ===========================================================================
class ROLane;
class ROEdge;

typedef std::vector<ROEdge*> ROEdgeVector;
typedef std::vector<const ROEdge*> ConstROEdgeVector;
typedef std::vector<std::pair<const ROEdge*, const ROEdge*> > ROConstEdgePairVector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROEdge
 * @brief A basic edge for routing applications
 *
 * The edge contains two time lines, one for the travel time and one for a second
 *  measure which may be used for computing the costs of a route. After loading
 *  the weights, it is needed to call "buildTimeLines" in order to initialise
 *  these time lines.
 */
class ROEdge : public Named, public Parameterised {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] index The numeric id of the edge
     */
    ROEdge(const std::string& id, RONode* from, RONode* to, int index, const int priority);


    /// Destructor
    virtual ~ROEdge();


    /// @name Set-up methods
    //@{

    /** @brief Adds a lane to the edge while loading
     *
     * The lane's length is adapted. Additionally, the information about allowed/disallowed
     *  vehicle classes is patched using the information stored in the lane.
     *
     * @param[in] lane The lane to add
     * @todo What about vehicle-type aware connections?
     */
    virtual void addLane(ROLane* lane);


    /** @brief Adds information about a connected edge
     *
     * The edge s is added to "myFollowingEdges" and this edge is added as predecessor to s.
     * @param[in] s The edge to add
     * @todo What about vehicle-type aware connections?
     */
    virtual void addSuccessor(ROEdge* s, ROEdge* via = nullptr, std::string dir = "");


    /** @brief Sets the function of the edge
    * @param[in] func The new function for the edge
    */
    inline void setFunction(SumoXMLEdgeFunc func) {
        myFunction = func;
    }


    /** @brief Sets whether the edge is a source
    * @param[in] func The new source functionality for the edge
    */
    inline void setSource(const bool isSource = true) {
        myAmSource = isSource;
    }


    /** @brief Sets whether the edge is a sink
    * @param[in] func The new sink functionality for the edge
    */
    inline void setSink(const bool isSink = true) {
        myAmSink = isSink;
    }


    /** @brief Sets the vehicle class specific speed limits of the edge
     * @param[in] restrictions The restrictions for the edge
     */
    inline void setRestrictions(const std::map<SUMOVehicleClass, double>* restrictions) {
        myRestrictions = restrictions;
    }

    inline void setTimePenalty(double value) {
        myTimePenalty = value;
    }

    /// @brief return whether this edge is an internal edge
    inline bool isInternal() const {
        return myFunction == SumoXMLEdgeFunc::INTERNAL;
    }

    /// @brief return whether this edge is a pedestrian crossing
    inline bool isCrossing() const {
        return myFunction == SumoXMLEdgeFunc::CROSSING;
    }

    /// @brief return whether this edge is walking area
    inline bool isWalkingArea() const {
        return myFunction == SumoXMLEdgeFunc::WALKINGAREA;
    }

    inline bool isTazConnector() const {
        return myFunction == SumoXMLEdgeFunc::CONNECTOR;
    }

    void setOtherTazConnector(const ROEdge* edge) {
        myOtherTazConnector = edge;
    }

    const ROEdge* getOtherTazConnector() const {
        return myOtherTazConnector;
    }

    /** @brief Builds the internal representation of the travel time/effort
     *
     * Should be called after weights / travel times have been loaded.
     *
     * In the case "weight-attribute" is one of "CO", "CO2", "HC", "NOx", "PMx", "fuel", or "electricity"
     *  the proper value (departs/s) is computed and multiplied with the travel time.
     *
     * @param[in] measure The name of the measure to use.
     */
    void buildTimeLines(const std::string& measure, const bool boundariesOverride);

    void cacheParamRestrictions(const std::vector<std::string>& restrictionKeys);
    //@}



    /// @name Getter methods
    //@{

    /** @brief Returns the function of the edge
    * @return This edge's basic function
    * @see SumoXMLEdgeFunc
    */
    inline SumoXMLEdgeFunc getFunction() const {
        return myFunction;
    }


    /** @brief Returns whether the edge acts as a sink
    * @return whether the edge is a sink
    */
    inline bool isSink() const {
        return myAmSink;
    }


    /** @brief Returns the length of the edge
     * @return This edge's length
     */
    double getLength() const {
        return myLength;
    }

    /** @brief Returns the index (numeric id) of the edge
     * @return This edge's numerical id
     */
    int getNumericalID() const {
        return myIndex;
    }


    /** @brief Returns the speed allowed on this edge
     * @return The speed allowed on this edge
     */
    double getSpeedLimit() const {
        return mySpeed;
    }

    /// @brief return a lower bound on shape.length() / myLength that is
    // sufficient for the astar air-distance heuristic
    double getLengthGeometryFactor() const;

    /** @brief Returns the lane's maximum speed, given a vehicle's speed limit adaptation
     * @param[in] The vehicle to return the adapted speed limit for
     * @return This lane's resulting max. speed
     */
    inline double getVClassMaxSpeed(SUMOVehicleClass vclass) const {
        if (myRestrictions != 0) {
            std::map<SUMOVehicleClass, double>::const_iterator r = myRestrictions->find(vclass);
            if (r != myRestrictions->end()) {
                return r->second;
            }
        }
        return mySpeed;
    }


    /** @brief Returns the number of lanes this edge has
     * @return This edge's number of lanes
     */
    int getNumLanes() const {
        return (int) myLanes.size();
    }


    /** @brief returns the information whether this edge is directly connected to the given
     *
     * @param[in] e The edge which may be connected
     * @param[in] vClass The vehicle class for which the connectivity is checked
     * @return Whether the given edge is a direct successor to this one
     */
    bool isConnectedTo(const ROEdge& e, const SUMOVehicleClass vClass) const;


    /** @brief Returns whether this edge prohibits the given vehicle to pass it
     * @param[in] vehicle The vehicle for which the information has to be returned
     * @return Whether the vehicle must not enter this edge
     */
    inline bool prohibits(const ROVehicle* const vehicle) const {
        const SUMOVehicleClass vclass = vehicle->getVClass();
        return (myCombinedPermissions & vclass) != vclass;
    }

    inline SVCPermissions getPermissions() const {
        return myCombinedPermissions;
    }

    /** @brief Returns whether this edge has restriction parameters forbidding the given vehicle to pass it
     * @param[in] vehicle The vehicle for which the information has to be returned
     * @return Whether the vehicle must not enter this edge
     */
    inline bool restricts(const ROVehicle* const vehicle) const {
        const std::vector<double>& vTypeRestrictions = vehicle->getType()->paramRestrictions;
        assert(vTypeRestrictions.size() == myParamRestrictions.size());
        for (int i = 0; i < (int)vTypeRestrictions.size(); i++) {
            if (vTypeRestrictions[i] > myParamRestrictions[i]) {
                return true;
            }
        }
        return false;
    }


    /** @brief Returns whether this edge succeeding edges prohibit the given vehicle to pass them
     * @param[in] vehicle The vehicle for which the information has to be returned
     * @return Whether the vehicle may continue its route on any of the following edges
     */
    bool allFollowersProhibit(const ROVehicle* const vehicle) const;
    //@}



    /// @name Methods for getting/setting travel time and cost information
    //@{

    /** @brief Adds a weight value
     *
     * @param[in] value The value to add
     * @param[in] timeBegin The begin time of the interval the given value is valid for [s]
     * @param[in] timeEnd The end time of the interval the given value is valid for [s]
     */
    void addEffort(double value, double timeBegin, double timeEnd);


    /** @brief Adds a travel time value
     *
     * @param[in] value The value to add
     * @param[in] timeBegin The begin time of the interval the given value is valid for [s]
     * @param[in] timeEnd The end time of the interval the given value is valid for [s]
     */
    void addTravelTime(double value, double timeBegin, double timeEnd);


    /** @brief Returns the number of edges this edge is connected to
     *
     * If this edge's type is set to "sink", 0 is returned, otherwise
     *  the number of edges stored in "myFollowingEdges".
     *
     * @return The number of edges following this edge
     */
    int getNumSuccessors() const;


    /** @brief Returns the following edges, restricted by vClass
    * @param[in] vClass The vClass for which to restrict the successors
    * @return The eligible following edges
    */
    const ROEdgeVector& getSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const;

    /** @brief Returns the following edges including vias, restricted by vClass
    * @param[in] vClass The vClass for which to restrict the successors
    * @return The eligible following edges
    */
    const ROConstEdgePairVector& getViaSuccessors(SUMOVehicleClass vClass = SVC_IGNORING) const;


    /** @brief Returns the number of edges connected to this edge
     *
     * If this edge's type is set to "source", 0 is returned, otherwise
     *  the number of edges stored in "myApproachingEdges".
     *
     * @return The number of edges reaching into this edge
     */
    int getNumPredecessors() const;


    /** @brief Returns the edge at the given position from the list of incoming edges
     * @param[in] pos The position of the list within the list of incoming
     * @return The incoming edge, stored at position pos
     */
    const ROEdgeVector& getPredecessors() const {
        return myApproachingEdges;
    }

    /// @brief if this edge is an internal edge, return its first normal predecessor, otherwise the edge itself
    const ROEdge* getNormalBefore() const;

    /// @brief if this edge is an internal edge, return its first normal successor, otherwise the edge itself
    const ROEdge* getNormalAfter() const;

    /** @brief Returns the effort for this edge
     *
     * @param[in] veh The vehicle for which the effort on this edge shall be retrieved
     * @param[in] time The tim for which the effort shall be returned [s]
     * @return The effort needed by the given vehicle to pass the edge at the given time
     * @todo Recheck whether the vehicle's maximum speed is considered
     */
    double getEffort(const ROVehicle* const veh, double time) const;


    /** @brief Returns whether a travel time for this edge was loaded
     *
     * @param[in] time The time for which the travel time shall be returned [s]
     * @return whether a value was loaded
     */
    bool hasLoadedTravelTime(double time) const;


    /** @brief Returns the travel time for this edge
     *
     * @param[in] veh The vehicle for which the travel time on this edge shall be retrieved
     * @param[in] time The time for which the travel time shall be returned [s]
     * @return The travel time needed by the given vehicle to pass the edge at the given time
     */
    double getTravelTime(const ROVehicle* const veh, double time) const;


    /** @brief Returns the effort for the given edge
     *
     * @param[in] edge The edge for which the effort shall be retrieved
     * @param[in] veh The vehicle for which the effort on this edge shall be retrieved
     * @param[in] time The time for which the effort shall be returned [s]
     * @return The effort needed by the given vehicle to pass the edge at the given time
     * @todo Recheck whether the vehicle's maximum speed is considered
     */
    static inline double getEffortStatic(const ROEdge* const edge, const ROVehicle* const veh, double time) {
        return edge->getEffort(veh, time);
    }


    /** @brief Returns the travel time for the given edge
     *
     * @param[in] edge The edge for which the travel time shall be retrieved
     * @param[in] veh The vehicle for which the travel time on this edge shall be retrieved
     * @param[in] time The time for which the travel time shall be returned [s]
     * @return The traveltime needed by the given vehicle to pass the edge at the given time
     */
    static inline double getTravelTimeStatic(const ROEdge* const edge, const ROVehicle* const veh, double time) {
        return edge->getTravelTime(veh, time);
    }

    static inline double getTravelTimeStaticRandomized(const ROEdge* const edge, const ROVehicle* const veh, double time) {
        return edge->getTravelTime(veh, time) * RandHelper::rand(1., gWeightsRandomFactor);
    }

    /// @brief Alias for getTravelTimeStatic (there is no routing device to provide aggregated travel times)
    static inline double getTravelTimeAggregated(const ROEdge* const edge, const ROVehicle* const veh, double time) {
        return edge->getTravelTime(veh, time);
    }

    /// @brief Return traveltime weighted by edge priority (scaled penalty for low-priority edges)
    static inline double getTravelTimeStaticPriorityFactor(const ROEdge* const edge, const ROVehicle* const veh, double time) {
        double result = edge->getTravelTime(veh, time);
        // lower priority should result in higher effort (and the edge with
        // minimum priority receives a factor of myPriorityFactor
        const double relativeInversePrio = 1 - ((edge->getPriority() - myMinEdgePriority) / myEdgePriorityRange);
        result *= 1 + relativeInversePrio * myPriorityFactor;
        return result;
    }

    /** @brief Returns a lower bound for the travel time on this edge without using any stored timeLine
     *
     * @param[in] veh The vehicle for which the effort on this edge shall be retrieved
     * @param[in] time The time for which the effort shall be returned [s]
     */
    inline double getMinimumTravelTime(const ROVehicle* const veh) const {
        if (isTazConnector()) {
            return 0;
        } else if (veh != 0) {
            return myLength / MIN2(veh->getType()->maxSpeed, veh->getChosenSpeedFactor() * mySpeed);
        } else {
            return myLength / mySpeed;
        }
    }


    template<PollutantsInterface::EmissionType ET>
    static double getEmissionEffort(const ROEdge* const edge, const ROVehicle* const veh, double time) {
        double ret = 0;
        if (!edge->getStoredEffort(time, ret)) {
            const SUMOVTypeParameter* const type = veh->getType();
            const double vMax = MIN2(type->maxSpeed, edge->mySpeed);
            const double accel = type->getCFParam(SUMO_ATTR_ACCEL, SUMOVTypeParameter::getDefaultAccel(type->vehicleClass)) * type->getCFParam(SUMO_ATTR_SIGMA, SUMOVTypeParameter::getDefaultImperfection(type->vehicleClass)) / 2.;
            ret = PollutantsInterface::computeDefault(type->emissionClass, ET, vMax, accel, 0, edge->getTravelTime(veh, time)); // @todo: give correct slope
        }
        return ret;
    }


    static double getNoiseEffort(const ROEdge* const edge, const ROVehicle* const veh, double time);

    static double getStoredEffort(const ROEdge* const edge, const ROVehicle* const /*veh*/, double time) {
        double ret = 0;
        edge->getStoredEffort(time, ret);
        return ret;
    }
    //@}


    /// @brief optimistic distance heuristic for use in routing
    double getDistanceTo(const ROEdge* other, const bool doBoundaryEstimate = false) const;


    /** @brief Returns all ROEdges */
    static const ROEdgeVector& getAllEdges();

    /// @brief Returns the number of edges
    static int dictSize() {
        return (int)myEdges.size();
    };

    static void setGlobalOptions(const bool interpolate) {
        myInterpolate = interpolate;
    }

    /// @brief return the coordinates of the center of the given stop
    static const Position getStopPosition(const SUMOVehicleParameter::Stop& stop);

    /// @brief get edge priority (road class)
    int getPriority() const {
        return myPriority;
    }

    const RONode* getFromJunction() const {
        return myFromJunction;
    }

    const RONode* getToJunction() const {
        return myToJunction;
    }

    /** @brief Returns this edge's lanes
     *
     * @return This edge's lanes
     */
    const std::vector<ROLane*>& getLanes() const {
        return myLanes;
    }

    /// @brief return opposite superposable/congruent edge, if it exist and 0 else
    inline const ROEdge* getBidiEdge() const {
        return myBidiEdge;
    }

    /// @brief set opposite superposable/congruent edge
    inline void setBidiEdge(const ROEdge* bidiEdge) {
        myBidiEdge = bidiEdge;
    }

    ReversedEdge<ROEdge, ROVehicle>* getReversedRoutingEdge() const {
        if (myReversedRoutingEdge == nullptr) {
            myReversedRoutingEdge = new ReversedEdge<ROEdge, ROVehicle>(this);
        }
        return myReversedRoutingEdge;
    }

    RailEdge<ROEdge, ROVehicle>* getRailwayRoutingEdge() const {
        if (myRailwayRoutingEdge == nullptr) {
            myRailwayRoutingEdge = new RailEdge<ROEdge, ROVehicle>(this);
        }
        return myRailwayRoutingEdge;
    }

    /// @brief whether effort data was loaded for this edge
    bool hasStoredEffort() const {
        return myUsingETimeLine;
    }

    /// @brief initialize priority factor range
    static bool initPriorityFactor(double priorityFactor);

protected:
    /** @brief Retrieves the stored effort
     *
     * @param[in] veh The vehicle for which the effort on this edge shall be retrieved
     * @param[in] time The tim for which the effort shall be returned
     * @return Whether the effort is given
     */
    bool getStoredEffort(double time, double& ret) const;



protected:
    /// @brief the junctions for this edge
    RONode* myFromJunction;
    RONode* myToJunction;

    /// @brief The index (numeric id) of the edge
    const int myIndex;

    /// @brief The edge priority (road class)
    const int myPriority;

    /// @brief The maximum speed allowed on this edge
    double mySpeed;

    /// @brief The length of the edge
    double myLength;

    /// @brief whether the edge is a source or a sink
    bool myAmSink, myAmSource;
    /// @brief Container storing passing time varying over time for the edge
    mutable ValueTimeLine<double> myTravelTimes;
    /// @brief Information whether the time line shall be used instead of the length value
    bool myUsingTTTimeLine;

    /// @brief Container storing passing time varying over time for the edge
    mutable ValueTimeLine<double> myEfforts;
    /// @brief Information whether the time line shall be used instead of the length value
    bool myUsingETimeLine;

    /// @brief Information whether to interpolate at interval boundaries
    static bool myInterpolate;

    /// @brief Information whether the edge has reported missing weights
    static bool myHaveEWarned;
    /// @brief Information whether the edge has reported missing weights
    static bool myHaveTTWarned;

    /// @brief List of edges that may be approached from this edge
    ROEdgeVector myFollowingEdges;

    ROConstEdgePairVector myFollowingViaEdges;

    /// @brief List of edges that approached this edge
    ROEdgeVector myApproachingEdges;

    /// @brief The function of the edge
    SumoXMLEdgeFunc myFunction;

    /// The vClass speed restrictions for this edge
    const std::map<SUMOVehicleClass, double>* myRestrictions;

    /// @brief This edge's lanes
    std::vector<ROLane*> myLanes;

    /// @brief The list of allowed vehicle classes combined across lanes
    SVCPermissions myCombinedPermissions;

    /// @brief the other taz-connector if this edge isTazConnector, otherwise nullptr
    const ROEdge* myOtherTazConnector;

    /// @brief the bidirectional rail edge or nullpr
    const ROEdge* myBidiEdge;

    /// @brief The bounding rectangle of end nodes incoming or outgoing edges for taz connectors or of my own start and end node for normal edges
    Boundary myBoundary;

    /// @brief flat penalty when computing traveltime
    double myTimePenalty;

    /// @brief cached value of parameters which may restrict access
    std::vector<double> myParamRestrictions;

    static ROEdgeVector myEdges;

    /// @brief Coefficient for factoring edge priority into routing weight
    static double myPriorityFactor;
    /// @brief Minimum priority for all edges
    static double myMinEdgePriority;
    /// @brief the difference between maximum and minimum priority for all edges
    static double myEdgePriorityRange;

    /// @brief The successors available for a given vClass
    mutable std::map<SUMOVehicleClass, ROEdgeVector> myClassesSuccessorMap;

    /// @brief The successors with vias available for a given vClass
    mutable std::map<SUMOVehicleClass, ROConstEdgePairVector> myClassesViaSuccessorMap;

    /// @brief a reversed version for backward routing
    mutable ReversedEdge<ROEdge, ROVehicle>* myReversedRoutingEdge = nullptr;
    mutable RailEdge<ROEdge, ROVehicle>* myRailwayRoutingEdge = nullptr;

#ifdef HAVE_FOX
    /// The mutex used to avoid concurrent updates of myClassesSuccessorMap
    mutable FXMutex myLock;
#endif

private:
    /// @brief Invalidated copy constructor
    ROEdge(const ROEdge& src);

    /// @brief Invalidated assignment operator
    ROEdge& operator=(const ROEdge& src);

};
