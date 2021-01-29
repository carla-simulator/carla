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
/// @file    MSStage.h
/// @author  Michael Behrisch
/// @date    Tue, 21 Apr 2015
///
// The common superclass for modelling transportable objects like persons and containers
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <cassert>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/vehicle/SUMOTrafficObject.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLane;
class MSNet;
class MSStoppingPlace;
class MSVehicleType;
class OutputDevice;
class SUMOVehicleParameter;
class SUMOVehicle;
class MSTransportableDevice;
class MSTransportable;
class MSTransportableStateAdapter;

typedef std::vector<const MSEdge*> ConstMSEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
enum class MSStageType {
    WAITING_FOR_DEPART = 0,
    WAITING = 1,
    WALKING = 2, // only for persons
    DRIVING = 3,
    ACCESS = 4,
    TRIP = 5,
    TRANSHIP = 6
};

/**
* The "abstract" class for a single stage of a movement
* Contains the destination of the current movement step
*/
class MSStage {
public:
    /// constructor
    MSStage(const MSEdge* destination, MSStoppingPlace* toStop, const double arrivalPos, MSStageType type, const std::string& group = "");

    /// destructor
    virtual ~MSStage();

    /// returns the destination edge
    const MSEdge* getDestination() const;

    /// returns the destination stop (if any)
    MSStoppingPlace* getDestinationStop() const {
        return myDestinationStop;
    }

    /// returns the origin stop (if any). only needed for MSStageTrip
    virtual const MSStoppingPlace* getOriginStop() const {
        return nullptr;
    }

    virtual double getArrivalPos() const {
        return myArrivalPos;
    }

    void setArrivalPos(double arrivalPos) {
        myArrivalPos = arrivalPos;
    }

    /// Returns the current edge
    virtual const MSEdge* getEdge() const;
    virtual const MSEdge* getFromEdge() const;
    virtual double getEdgePos(SUMOTime now) const;

    /// returns the position of the transportable
    virtual Position getPosition(SUMOTime now) const = 0;

    /// returns the angle of the transportable
    virtual double getAngle(SUMOTime now) const = 0;

    /// Returns the current lane (if applicable)
    virtual const MSLane* getLane() const {
        return nullptr;
    }

    ///
    MSStageType getStageType() const {
        return myType;
    }

    /// @brief return the id of the group of transportables traveling together
    const std::string& getGroup() const {
        return myGroup;
    }

    /// @brief return (brief) string representation of the current stage
    virtual std::string getStageDescription(const bool isPerson) const = 0;

    /// @brief return string summary of the current stage
    virtual std::string getStageSummary(const bool isPerson) const = 0;

    /// proceeds to this stage
    virtual void proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous) = 0;

    /// abort this stage (TraCI)
    virtual void abort(MSTransportable*) {};

    /// sets the walking speed (ignored in other stages)
    virtual void setSpeed(double) {};

    /// get departure time of stage
    SUMOTime getDeparted() const;

    /// get arrival time of stage
    SUMOTime getArrived() const;

    /// logs end of the step
    void setDeparted(SUMOTime now);

    /// logs end of the step
    virtual const std::string setArrived(MSNet* net, MSTransportable* transportable, SUMOTime now, const bool vehicleArrived);

    /// Whether the transportable waits for the given vehicle
    virtual bool isWaitingFor(const SUMOVehicle* vehicle) const;

    /// @brief Whether the transportable waits for a vehicle
    virtual bool isWaiting4Vehicle() const {
        return false;
    }

    /// @brief Whether the transportable waits for a vehicle
    virtual SUMOVehicle* getVehicle() const {
        return nullptr;
    }

    /// @brief the time this transportable spent waiting
    virtual SUMOTime getWaitingTime(SUMOTime now) const;

    /// @brief the speed of the transportable
    virtual double getSpeed() const;

    /// @brief the edges of the current stage
    virtual ConstMSEdgeVector getEdges() const;

    /// @brief get position on edge e at length at with orthogonal offset
    Position getEdgePosition(const MSEdge* e, double at, double offset) const;

    /// @brief get position on lane at length at with orthogonal offset
    Position getLanePosition(const MSLane* lane, double at, double offset) const;

    /// @brief get angle of the edge at a certain position
    double getEdgeAngle(const MSEdge* e, double at) const;

    void setDestination(const MSEdge* newDestination, MSStoppingPlace* newDestStop);

    /// @brief get travel distance in this stage
    virtual double getDistance() const = 0;

    /** @brief Called on writing tripinfo output
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     */
    virtual void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const = 0;

    /** @brief Called on writing vehroute output
     * @param[in] isPerson Whether we are writing person or container info
     * @param[in] os The stream to write the information into
     * @param[in] withRouteLength whether route length shall be written
     * @param[in] previous The previous stage for additional info such as from edge
     * @exception IOError not yet implemented
     */
    virtual void routeOutput(const bool isPerson, OutputDevice& os, const bool withRouteLength, const MSStage* const previous) const = 0;

    virtual MSStage* clone() const = 0;

    /** @brief Saves the current state into the given stream, standard implementation does nothing
     */
    virtual void saveState(std::ostringstream& out) {
        UNUSED_PARAMETER(out);
    }

    /** @brief Reconstructs the current state, standard implementation does nothing
     */
    virtual void loadState(MSTransportable* transportable, std::istringstream& state) {
        UNUSED_PARAMETER(transportable);
        UNUSED_PARAMETER(state);
    }

protected:
    /// the next edge to reach by getting transported
    const MSEdge* myDestination;

    /// the stop to reach by getting transported (if any)
    MSStoppingPlace* myDestinationStop;

    /// the position at which we want to arrive
    double myArrivalPos;

    /// the time at which this stage started
    SUMOTime myDeparted;

    /// the time at which this stage ended
    SUMOTime myArrived;

    /// The type of this stage
    MSStageType myType;

    /// The id of the group of transportables traveling together
    const std::string myGroup;

    /// @brief the offset for computing positions when standing at an edge
    static const double ROADSIDE_OFFSET;

private:
    /// @brief Invalidated copy constructor.
    MSStage(const MSStage&);

    /// @brief Invalidated assignment operator.
    MSStage& operator=(const MSStage&) = delete;

};


/**
* A "placeholder" stage storing routing info which will result in real stages when routed
*/
class MSStageTrip : public MSStage {
public:
    /// constructor
    MSStageTrip(const MSEdge* origin, MSStoppingPlace* fromStop,
                const MSEdge* destination, MSStoppingPlace* toStop,
                const SUMOTime duration, const SVCPermissions modeSet,
                const std::string& vTypes, const double speed, const double walkFactor,
                const double departPosLat, const bool hasArrivalPos, const double arrivalPos);

    /// destructor
    virtual ~MSStageTrip();

    MSStage* clone() const;

    const MSEdge* getEdge() const;

    const MSStoppingPlace* getOriginStop() const {
        return myOriginStop;
    }

    double getEdgePos(SUMOTime now) const;

    Position getPosition(SUMOTime now) const;

    double getAngle(SUMOTime now) const;

    double getDistance() const {
        // invalid
        return -1;
    }

    std::string getStageDescription(const bool isPerson) const {
        UNUSED_PARAMETER(isPerson);
        return "trip";
    }

    std::string getStageSummary(const bool isPerson) const;

    /// logs end of the step
    const std::string setArrived(MSNet* net, MSTransportable* transportable, SUMOTime now, const bool vehicleArrived);

    /// change origin for parking area rerouting
    void setOrigin(const MSEdge* origin) {
        myOrigin = origin;
    }

    /// proceeds to the next step
    void proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous);

    /** @brief Called on writing tripinfo output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const {
        UNUSED_PARAMETER(os);
        UNUSED_PARAMETER(transportable);
    }

    /** @brief Called on writing vehroute output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    void routeOutput(const bool isPerson, OutputDevice& os, const bool withRouteLength, const MSStage* const previous) const {
        UNUSED_PARAMETER(isPerson);
        UNUSED_PARAMETER(os);
        UNUSED_PARAMETER(withRouteLength);
        UNUSED_PARAMETER(previous);
    }

private:
    /// the origin edge
    const MSEdge* myOrigin;

    /// the origin edge
    const MSStoppingPlace* myOriginStop;

    /// the time the trip should take (applies to only walking)
    SUMOTime myDuration;

    /// @brief The allowed modes of transportation
    const SVCPermissions myModeSet;

    /// @brief The possible vehicles to use
    const std::string myVTypes;

    /// @brief The walking speed
    const double mySpeed;

    /// @brief The factor to apply to walking durations
    const double myWalkFactor;

    /// @brief The depart position
    double myDepartPos;

    /// @brief The lateral depart position
    const double myDepartPosLat;

    /// @brief whether an arrivalPos was in the input
    const bool myHaveArrivalPos;

private:
    /// @brief Invalidated copy constructor.
    MSStageTrip(const MSStageTrip&);

    /// @brief Invalidated assignment operator.
    MSStageTrip& operator=(const MSStageTrip&);

};


/**
* A "real" stage performing a waiting over the specified time
*/
class MSStageWaiting : public MSStage {
public:
    /// constructor
    MSStageWaiting(const MSEdge* destination, MSStoppingPlace* toStop, SUMOTime duration, SUMOTime until,
                   double pos, const std::string& actType, const bool initial);

    /// destructor
    virtual ~MSStageWaiting();

    MSStage* clone() const;

    /// abort this stage (TraCI)
    void abort(MSTransportable*);

    SUMOTime getUntil() const;

    ///
    Position getPosition(SUMOTime now) const;

    double getAngle(SUMOTime now) const;

    /// @brief get travel distance in this stage
    double getDistance() const {
        return 0;
    }

    SUMOTime getWaitingTime(SUMOTime now) const;

    std::string getStageDescription(const bool isPerson) const {
        UNUSED_PARAMETER(isPerson);
        return "waiting (" + myActType + ")";
    }

    std::string getStageSummary(const bool isPerson) const;

    /// proceeds to the next step
    void proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous);

    /** @brief Called on writing tripinfo output
    *
    * @param[in] os The stream to write the information into
    * @exception IOError not yet implemented
    */
    void tripInfoOutput(OutputDevice& os, const MSTransportable* const transportable) const;

    /** @brief Called on writing vehroute output
     * @param[in] isPerson Whether we are writing person or container info
     * @param[in] os The stream to write the information into
     * @param[in] withRouteLength whether route length shall be written
     * @param[in] previous The previous stage for additional info such as from edge
     * @exception IOError not yet implemented
     */
    void routeOutput(const bool isPerson, OutputDevice& os, const bool withRouteLength, const MSStage* const previous) const;

private:
    /// the time the person is waiting
    SUMOTime myWaitingDuration;

    /// the time until the person is waiting
    SUMOTime myWaitingUntil;

    /// @brief The type of activity
    std::string myActType;

private:
    /// @brief Invalidated copy constructor.
    MSStageWaiting(const MSStageWaiting&);

    /// @brief Invalidated assignment operator.
    MSStageWaiting& operator=(const MSStageWaiting&) = delete;

};


/**
* An abstract stage providing additional interface for the movement models
*/
class MSStageMoving : public MSStage {
public:
    /// constructor
    MSStageMoving(const std::vector<const MSEdge*>& route, MSStoppingPlace* toStop, const double speed,
                  const double departPos, const double arrivalPos, const double departPosLat, MSStageType type) :
        MSStage(route.back(), toStop, arrivalPos, type),
        myState(nullptr), myRoute(route), mySpeed(speed), myDepartPos(departPos), myDepartPosLat(departPosLat) {}

    /// destructor
    virtual ~MSStageMoving();

    virtual const MSEdge* getNextRouteEdge() const = 0;

    virtual MSTransportableStateAdapter* getState() const {
        return myState;
    }

    /// Returns the current edge
    const MSEdge* getEdge() const;

    /// Returns the current lane
    const MSLane* getLane() const;

    /// Returns first edge of the containers route
    const MSEdge* getFromEdge() const;

    /// @brief the edges of the current stage
    ConstMSEdgeVector getEdges() const;

    /// Returns the offset from the start of the current edge measured in its natural direction
    double getEdgePos(SUMOTime now) const;

    /// Returns the position of the container
    Position getPosition(SUMOTime now) const;

    /// Returns the angle of the container
    double getAngle(SUMOTime now) const;

    /// Returns the time the container spent waiting
    SUMOTime getWaitingTime(SUMOTime now) const;

    /// Returns the speed of the container
    double getSpeed() const;

    /// @brief the maximum speed of the transportable
    virtual double getMaxSpeed(const MSTransportable* const transportable = nullptr) const = 0;

    /// @brief move forward and return whether the transportable arrived
    virtual bool moveToNextEdge(MSTransportable* transportable, SUMOTime currentTime, MSEdge* nextInternal = 0) = 0;

    /// @brief place transportable on a previously passed edge
    virtual void setRouteIndex(MSTransportable* const transportable, int routeOffset);

    virtual void replaceRoute(MSTransportable* const transportable, const ConstMSEdgeVector& edges, int routeOffset);

    inline const std::vector<const MSEdge*>& getRoute() const {
        return myRoute;
    }

    inline const std::vector<const MSEdge*>::iterator getRouteStep() const {
        return myRouteStep;
    }

    inline double getDepartPos() const {
        return myDepartPos;
    }

    inline double getDepartPosLat() const {
        return myDepartPosLat;
    }

protected:
    /// @brief state that is to be manipulated by MSPModel
    MSTransportableStateAdapter* myState;

    /// @brief The route of the container
    std::vector<const MSEdge*> myRoute;

    /// @brief current step
    std::vector<const MSEdge*>::iterator myRouteStep;

    /// @brief The current internal edge this transportable is on or nullptr
    MSEdge* myCurrentInternalEdge = nullptr;

    /// @brief the speed of the transportable
    double mySpeed;

    /// @brief the depart position
    double myDepartPos;

    /// @brief the lateral depart position
    double myDepartPosLat;
};
