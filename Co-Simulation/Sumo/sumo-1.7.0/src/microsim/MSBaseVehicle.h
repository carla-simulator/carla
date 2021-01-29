/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSBaseVehicle.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 8 Nov 2010
///
// A base class for vehicle implementations
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <vector>
#include <set>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/common/StdDefs.h>
#include "MSRoute.h"
#include "MSMoveReminder.h"
#include "MSVehicleType.h"

// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class MSDevice_Transportable;
class MSVehicleDevice;
class MSEdgeWeightsStorage;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSBaseVehicle
 * @brief The base class for microscopic and mesoscopic vehicles
 */
class MSBaseVehicle : public SUMOVehicle {
public:
    // XXX: This definition was introduced to make the MSVehicle's previousSpeed
    //      available in the context of MSMoveReminder::notifyMove(). Another solution
    //      would be to modify notifyMove()'s interface to work with MSVehicle instead
    //      of SUMOVehicle (it is only called with MSVehicles!). Refs. #2579
    /** @brief Returns the vehicle's previous speed
     * @return The vehicle's speed
     */
    double getPreviousSpeed() const;

    friend class GUIBaseVehicle;

    /** @enum RouteValidity
     */
    enum RouteValidity {
        ROUTE_VALID = 0,
        ROUTE_UNCHECKED = 1 << 0,
        /// route was checked and is valid
        ROUTE_INVALID = 1 << 1,
        // starting edge permissions invalid (could change)
        ROUTE_START_INVALID_PERMISSIONS = 1 << 2,
        // insertion lane does not exist
        ROUTE_START_INVALID_LANE = 1 << 3
    };

    /** @brief Constructor
     * @param[in] pars The vehicle description
     * @param[in] route The vehicle's route
     * @param[in] type The vehicle's type
     * @param[in] speedFactor The factor for driven lane's speed limits
     * @exception ProcessError If a value is wrong
     */
    MSBaseVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
                  MSVehicleType* type, const double speedFactor);


    /// @brief Destructor
    virtual ~MSBaseVehicle();

    virtual void initDevices();

    bool isVehicle() const {
        return true;
    }

    /// @brief set the id (inherited from Named but forbidden for vehicles)
    void setID(const std::string& newID);

    /** @brief Returns the vehicle's parameter (including departure definition)
     *
     * @return The vehicle's parameter
     */
    const SUMOVehicleParameter& getParameter() const;

    /// @brief retrieve parameters of devices, models and the vehicle itself
    std::string getPrefixedParameter(const std::string& key, std::string& error) const;

    /** @brief Returns the vehicle's emission model parameter
     *
     * @return The vehicle's emission parameters
     */
    const std::map<int, double>* getEmissionParameters() const;

    /// @brief replace the vehicle parameter (deleting the old one)
    void replaceParameter(const SUMOVehicleParameter* newParameter);

    /// @brief check whether the vehicle is equiped with a device of the given type
    bool hasDevice(const std::string& deviceName) const;

    /// @brief create device of the given type
    void createDevice(const std::string& deviceName);

    /// @brief try to retrieve the given parameter from any of the vehicles devices, raise InvalidArgument if no device parameter by that name exists
    std::string getDeviceParameter(const std::string& deviceName, const std::string& key) const;

    /// @brief try to set the given parameter from any of the vehicles devices, raise InvalidArgument if no device parameter by that name exists
    void setDeviceParameter(const std::string& deviceName, const std::string& key, const std::string& value);

    /** @brief Returns the current route
     * @return The route the vehicle uses
     */
    inline const MSRoute& getRoute() const {
        return *myRoute;
    }


    /** @brief Returns the vehicle's type definition
     * @return The vehicle's type definition
     */
    inline const MSVehicleType& getVehicleType() const  {
        return *myType;
    }


    /** @brief Returns the vehicle's access class
     * @return The vehicle's access class
     */
    inline SUMOVehicleClass getVClass() const {
        return myType->getParameter().vehicleClass;
    }

    /** @brief Returns the maximum speed
     * @return The vehicle's maximum speed
     */
    double getMaxSpeed() const;


    /** @brief Returns the nSuccs'th successor of edge the vehicle is currently at
     *
     * If the rest of the route (counted from the current edge) has less than nSuccs edges,
     *  0 is returned.
     * @param[in] nSuccs The number of edge to look forward
     * @return The nSuccs'th following edge in the vehicle's route
     */
    const MSEdge* succEdge(int nSuccs) const;

    /** @brief Returns the edge the vehicle is currently at
     *
     * @return The current edge in the vehicle's route
     */
    const MSEdge* getEdge() const;


    /** @brief Returns the information whether the vehicle is on a road (is simulated)
     * @return Whether the vehicle is simulated
     */
    virtual bool isOnRoad() const {
        return true;
    }

    /** @brief Returns the information whether the vehicle is fully controlled
     * via TraCI
     * @return Whether the vehicle is remote-controlled
     */
    virtual bool isRemoteControlled() const {
        return false;
    }

    virtual bool wasRemoteControlled(SUMOTime lookBack = DELTA_T) const {
        UNUSED_PARAMETER(lookBack);
        return false;
    }

    /** @brief Returns the information whether the front of the vehhicle is on the given lane
     * @return Whether the vehicle's front is on that lane
     */
    virtual bool isFrontOnLane(const MSLane*) const {
        return true;
    }

    /** @brief Get the vehicle's lateral position on the lane
     * @return The lateral position of the vehicle (in m relative to the
     * centerline of the lane)
     */
    virtual double getLateralPositionOnLane() const {
        return 0;
    }

    /** @brief Returns the starting point for reroutes (usually the current edge)
     *
     * This differs from *myCurrEdge only if the vehicle is on an internal edge
     * @return The rerouting start point
     */
    virtual const MSEdge* getRerouteOrigin() const {
        return *myCurrEdge;
    }


    /** @brief Returns an iterator pointing to the current edge in this vehicles route
     * @return The current route pointer
     */
    const MSRouteIterator& getCurrentRouteEdge() const {
        return myCurrEdge;
    }


    /** @brief Performs a rerouting using the given router
     *
     * Tries to find a new route between the current edge and the destination edge, first.
     * Tries to replace the current route by the new one using replaceRoute.
     *
     * @param[in] t The time for which the route is computed
     * @param[in] router The router to use
     * @see replaceRoute
     */
    void reroute(SUMOTime t, const std::string& info, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, const bool onInit = false, const bool withTaz = false, const bool silent = false);


    /** @brief Replaces the current route by the given edges
     *
     * It is possible that the new route is not accepted, if a) it does not
     *  contain the vehicle's current edge, or b) something fails on insertion
     *  into the routes container (see in-line comments).
     *
     * @param[in] edges The new list of edges to pass
     * @param[in] onInit Whether the vehicle starts with this route
     * @param[in] check Whether the route should be checked for validity
     * @param[in] removeStops Whether stops should be removed if they do not fit onto the new route
     * @return Whether the new route was accepted
     */
    bool replaceRouteEdges(ConstMSEdgeVector& edges, double cost, double savings, const std::string& info, bool onInit = false, bool check = false, bool removeStops = true);


    /** @brief Returns the vehicle's acceleration
     *
     * This default implementation returns always 0.
     * @return The acceleration
     */
    virtual double getAcceleration() const;

    /** @brief Returns the slope of the road at vehicle's position
     *
     * This default implementation returns always 0.
     * @return The slope
     */
    virtual double getSlope() const;

    /** @brief Called when the vehicle is inserted into the network
     *
     * Sets optional information about departure time, informs the vehicle
     *  control about a further running vehicle.
     */
    void onDepart();

    /** @brief Returns this vehicle's real departure time
     * @return This vehicle's real departure time
     */
    inline SUMOTime getDeparture() const {
        return myDeparture;
    }

    /** @brief Returns the depart delay */
    SUMOTime getDepartDelay() const {
        return getDeparture() - getParameter().depart;
    }

    /** @brief Returns the public transport stop delay in seconds
     */
    virtual double getStopDelay() const {
        /// @todo implement for meso
        return -1;
    }

    /** @brief Returns this vehicle's real departure position
     * @return This vehicle's real departure position
     */
    inline double getDepartPos() const {
        return myDepartPos;
    }

    /** @brief Returns this vehicle's desired arrivalPos for its current route
     * (may change on reroute)
     * @return This vehicle's real arrivalPos
     */
    virtual double getArrivalPos() const {
        return myArrivalPos;
    }

    /** @brief Sets this vehicle's desired arrivalPos for its current route
     */
    virtual void setArrivalPos(double arrivalPos) {
        myArrivalPos = arrivalPos;
    }

    /** @brief Called when the vehicle is removed from the network.
     *
     * Moves along work reminders and
     *  informs all devices about quitting. Calls "leaveLane" then.
     *
     * @param[in] reason why the vehicle leaves (reached its destination, parking, teleport)
     */
    virtual void onRemovalFromNet(const MSMoveReminder::Notification /*reason*/) {}

    /** @brief Returns whether this vehicle has already departed
     */
    bool hasDeparted() const;

    /** @brief Returns whether this vehicle has already arived
     * (by default this is true if the vehicle has reached its final edge)
     */
    virtual bool hasArrived() const;

    /// @brief return index of edge within route
    int getRoutePosition() const;

    /// @brief reset index of edge within route
    void resetRoutePosition(int index, DepartLaneDefinition departLaneProcedure);

    /** @brief Returns the distance that was already driven by this vehicle
     * @return the distance driven [m]
     */
    double getOdometer() const;

    /** @brief Returns the number of new routes this vehicle got
     * @return the number of new routes this vehicle got
     */
    inline int getNumberReroutes() const {
        return myNumberReroutes;
    }

    /// @brief Returns this vehicles impatience
    double getImpatience() const;

    /** @brief Returns the number of persons
     * @return The number of passengers on-board
     */
    int getPersonNumber() const;

    /** @brief Returns the list of persons
     * @return The list of passengers on-board
     */
    std::vector<std::string> getPersonIDList() const;

    /** @brief Returns the number of containers
     * @return The number of contaiers on-board
     */
    int getContainerNumber() const;


    /** @brief Returns this vehicle's devices
     * @return This vehicle's devices
     */
    inline const std::vector<MSVehicleDevice*>& getDevices() const {
        return myDevices;
    }

    /// @brief whether the given transportable is allowed to board this vehicle
    bool allowsBoarding(MSTransportable* t) const;

    /** @brief Adds a person or container to this vehicle
     *
     * @param[in] transportable The person/container to add
     */
    virtual void addTransportable(MSTransportable* transportable);

    /// @brief removes a person or container
    void removeTransportable(MSTransportable* t);

    /// @brief retrieve riding persons
    const std::vector<MSTransportable*>& getPersons() const;

    /// @brief retrieve riding containers
    const std::vector<MSTransportable*>& getContainers() const;

    /// @brief returns whether the vehicle serves a public transport line that serves the given stop
    bool isLineStop(double position) const;

    /** @brief Validates the current or given route
     * @param[out] msg Description why the route is not valid (if it is the case)
     * @param[in] route The route to check (or 0 if the current route shall be checked)
     * @return Whether the vehicle's current route is valid
     */
    bool hasValidRoute(std::string& msg, const MSRoute* route = 0) const;

    /// @brief checks wether the vehicle can depart on the first edge
    virtual bool hasValidRouteStart(std::string& msg);

    /// @brief check for route validity at first insertion attempt
    int getRouteValidity(bool update = true, bool silent = false);

    /** @brief Adds a MoveReminder dynamically
     *
     * @param[in] rem the reminder to add
     * @see MSMoveReminder
     */
    void addReminder(MSMoveReminder* rem);

    /** @brief Removes a MoveReminder dynamically
     *
     * @param[in] rem the reminder to remove
     * @see MSMoveReminder
     */
    void removeReminder(MSMoveReminder* rem);

    /** @brief "Activates" all current move reminder
     *
     * For all move reminder stored in "myMoveReminders", their method
     *  "MSMoveReminder::notifyEnter" is called.
     *
     * @param[in] reason The reason for changing the reminders' states
     * @param[in] enteredLane The lane, which is entered (if applicable)
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    virtual void activateReminders(const MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Returns the vehicle's length
     * @return vehicle's length
     */
    inline double getLength() const {
        return myType->getLength();
    }


    /** @brief Returns the vehicle's width
     * @return vehicle's width
     */
    inline double getWidth() const {
        return myType->getWidth();
    }


    /** @brief Returns the precomputed factor by which the driver wants to be faster than the speed limit
     * @return Speed limit factor
     */
    inline double getChosenSpeedFactor() const {
        return myChosenSpeedFactor;
    }

    /** @brief Returns the precomputed factor by which the driver wants to be faster than the speed limit
     * @return Speed limit factor
     */
    inline void setChosenSpeedFactor(const double factor) {
        myChosenSpeedFactor = factor;
    }

    /// @brief Returns a device of the given type if it exists or 0
    MSVehicleDevice* getDevice(const std::type_info& type) const;


    /** @brief Replaces the current vehicle type by the one given
     *
     * If the currently used vehicle type is marked as being used by this vehicle
     *  only, it is deleted, first. The new, given type is then assigned to
     *  "myType".
     * @param[in] type The new vehicle type
     * @see MSBaseVehicle::myType
     */
    void replaceVehicleType(MSVehicleType* type);


    /** @brief Replaces the current vehicle type with a new one used by this vehicle only
     *
     * If the currently used vehicle type is already marked as being used by this vehicle
     *  only, no new type is created.
     * @return The new modifiable vehicle type
     * @see MSBaseVehicle::myType
     */
    MSVehicleType& getSingularType();

    /// @name state io
    //@{

    /// Saves the (common) state of a vehicle
    virtual void saveState(OutputDevice& out);

    //@}

    /** @brief Adds stops to the built vehicle
     *
     * This code needs to be separated from the MSBaseVehicle constructor
     *  since it is not allowed to call virtual functions from a constructor
     *
     * @param[in] ignoreStopErrors whether invalid stops trigger a warning only
     */
    void addStops(const bool ignoreStopErrors, MSRouteIterator* searchStart = 0);

    /// @brief whether this vehicle is selected in the GUI
    virtual bool isSelected() const {
        return false;
    }

    /// @brief @return The index of the vehicle's associated RNG
    int getRNGIndex() const;

    /// @brief @return The vehicle's associated RNG
    std::mt19937* getRNG() const;

    inline NumericalID getNumericalID() const {
        return myNumericalID;
    }

    const MSDevice_Transportable* getPersonDevice() const {
        return myPersonDevice;
    }

    const MSDevice_Transportable* getContainerDevice() const {
        return myContainerDevice;
    }


    /// @name Emission retrieval
    //@{

    /** @brief Returns CO2 emission of the current state
     * @return The current CO2 emission
     */
    double getCO2Emissions() const;


    /** @brief Returns CO emission of the current state
     * @return The current CO emission
     */
    double getCOEmissions() const;


    /** @brief Returns HC emission of the current state
     * @return The current HC emission
     */
    double getHCEmissions() const;


    /** @brief Returns NOx emission of the current state
     * @return The current NOx emission
     */
    double getNOxEmissions() const;


    /** @brief Returns PMx emission of the current state
     * @return The current PMx emission
     */
    double getPMxEmissions() const;


    /** @brief Returns fuel consumption of the current state
    * @return The current fuel consumption
    */
    double getFuelConsumption() const;


    /** @brief Returns electricity consumption of the current state
    * @return The current electricity consumption
    */
    double getElectricityConsumption() const;

    /** @brief Returns actual state of charge of battery (Wh)
    * RICE_CHECK: This may be a misnomer, SOC is typically percentage of the maximum battery capacity.
    * @return The actual battery state of charge
    */
    double getStateOfCharge() const;

    /** @brief Returns actual current (A) of ElecHybrid device
    * RICE_CHECK: Is this the current consumed from the overhead wire or the current driving the poweertrain of the vehicle?
    * @return The current of ElecHybrid device
    */
    double getElecHybridCurrent() const;

    /** @brief Returns noise emissions of the current state
     * @return The noise produced
     */
    double getHarmonoise_NoiseEmissions() const;
    //@}

    /** @class Influencer
      * @brief Changes the wished vehicle speed / lanes
      *
      * The class is used for passing velocities or velocity profiles obtained via TraCI to the vehicle.
      * The speed adaptation is controlled by the stored speedTimeLine
      * Additionally, the variables myConsiderSafeVelocity, myConsiderMaxAcceleration, and myConsiderMaxDeceleration
      * control whether the safe velocity, the maximum acceleration, and the maximum deceleration
      * have to be regarded.
      *
      * Furthermore this class is used to affect lane changing decisions according to
      * LaneChangeMode and any given laneTimeLine
      */
    class BaseInfluencer {
    public:
        /// @brief Constructor
        BaseInfluencer();

        /// @brief Destructor
        virtual ~BaseInfluencer() {}

        /// @brief Static initalization
        static void init();
        /// @brief Static cleanup
        static void cleanup();


        /// @brief return the current routing mode
        int getRoutingMode() const {
            return myRoutingMode;
        }

        /** @brief Sets routing behavior
         * @param[in] value an enum value controlling the different modes
         */
        void setRoutingMode(int value) {
            myRoutingMode = value;
        }


        SUMOAbstractRouter<MSEdge, SUMOVehicle>& getRouterTT(const int rngIndex, SUMOVehicleClass svc) const;

    protected:
        ///@brief routing mode (see TraCIConstants.h)
        int myRoutingMode;

    };



    /** @brief Returns the velocity/lane influencer
     *
     * If no influencer was existing before, one is built, first
     * @return Reference to this vehicle's speed influencer
     */
    virtual BaseInfluencer& getBaseInfluencer() = 0;

    virtual const BaseInfluencer* getBaseInfluencer() const = 0;

    virtual bool hasInfluencer() const  = 0;


    /** @brief Returns the vehicle's internal edge travel times/efforts container
     *
     * If the vehicle does not have such a container, it is built.
     * @return The vehicle's knowledge about edge weights
     */
    const MSEdgeWeightsStorage& getWeightsStorage() const;
    MSEdgeWeightsStorage& getWeightsStorage();

    /** @brief Returns the leader of the vehicle looking for a fixed distance.
     *
     * If the distance is not given it is calculated from the brake gap.
     * The gap returned does not include the minGap.
     * @param dist    up to which distance to look at least for a leader
     * @return The leading vehicle together with the gap; (0, -1) if no leader was found.
     */
    virtual std::pair<const MSVehicle* const, double> getLeader(double dist = 0) const {
        UNUSED_PARAMETER(dist);
        WRITE_WARNING("getLeader not yet implemented for meso");
        return std::make_pair(nullptr, -1);
    }

    /** @brief Returns the follower of the vehicle looking for a fixed distance.
     *
     * If the distance is not given it is set to the value of MSCFModel::brakeGap(2*roadSpeed, 4.5, 0)
     * The gap returned does not include the minGap.
     * If there are multiple followers, the one that maximizes the term (getSecureGap - gap) is returned.
     * @param dist    up to which distance to look at least for a leader
     * @return The leading vehicle together with the gap; (0, -1) if no leader was found.
     */
    virtual std::pair<const MSVehicle* const, double> getFollower(double dist = 0) const {
        UNUSED_PARAMETER(dist);
        WRITE_WARNING("getFollwer not yet implemented for meso");
        return std::make_pair(nullptr, -1);
    }

protected:
    /** @brief (Re-)Calculates the arrival position and lane from the vehicle parameters
     */
    void calculateArrivalParams();

    /** @brief Returns the list of still pending stop edges
     */
    virtual const ConstMSEdgeVector getStopEdges(double& firstPos, double& lastPos) const = 0;

protected:
    /// @brief This vehicle's parameter.
    const SUMOVehicleParameter* myParameter;

    /// @brief This vehicle's route.
    const MSRoute* myRoute;

    /// @brief This vehicle's type.
    MSVehicleType* myType;

    /// @brief Iterator to current route-edge
    MSRouteIterator myCurrEdge;

    /// @brief A precomputed factor by which the driver wants to be faster than the speed limit
    double myChosenSpeedFactor;


    /// @name Move reminder structures
    /// @{

    /// @brief Definition of a move reminder container
    //         The double value holds the relative position offset, i.e.,
    //         offset + vehicle-position - moveReminder-position = distance,
    //         i.e. the offset is counted up when the vehicle continues to a
    //         succeeding lane.
    typedef std::vector< std::pair<MSMoveReminder*, double> > MoveReminderCont;

    /// @brief Currently relevant move reminders
    MoveReminderCont myMoveReminders;
    /// @}

    /// @brief The devices this vehicle has
    std::vector<MSVehicleDevice*> myDevices;

    /// @brief The passengers this vehicle may have
    MSDevice_Transportable* myPersonDevice;

    /// @brief The containers this vehicle may have
    MSDevice_Transportable* myContainerDevice;

    /// @brief The real departure time
    SUMOTime myDeparture;

    /// @brief The real depart position
    double myDepartPos;

    /// @brief The position on the destination lane where the vehicle stops
    double myArrivalPos;

    /// @brief The destination lane where the vehicle stops
    int myArrivalLane;

    /// @brief The number of reroutings
    int myNumberReroutes;

    /// @brief The offset when adding route stops with 'until' on route replacement
    SUMOTime myStopUntilOffset;

    /// @brief A simple odometer to keep track of the length of the route already driven
    double myOdometer;

    /// @brief status of the current vehicle route
    int myRouteValidity;

    /* @brief magic value for undeparted vehicles
     * @note: in previous versions this was -1
     */
    static const SUMOTime NOT_YET_DEPARTED;

    static std::vector<MSTransportable*> myEmptyTransportableVector;

private:
    const NumericalID myNumericalID;

    /* @brief The vehicle's knowledge about edge efforts/travel times; @see MSEdgeWeightsStorage
     * @note member is initialized on first access */
    mutable MSEdgeWeightsStorage* myEdgeWeights;

    MSEdgeWeightsStorage& _getWeightsStorage() const;

    static NumericalID myCurrentNumericalIndex;

private:
    /// invalidated assignment operator
    MSBaseVehicle& operator=(const MSBaseVehicle& s) = delete;

#ifdef _DEBUG
public:
    static void initMoveReminderOutput(const OptionsCont& oc);

protected:
    /// @brief optionally generate movereminder-output for this vehicle
    void traceMoveReminder(const std::string& type, MSMoveReminder* rem, double pos, bool keep) const;

    /// @brief whether this vehicle shall trace its moveReminders
    const bool myTraceMoveReminders;
private:
    /// @brief vehicles which shall trace their move reminders
    static std::set<std::string> myShallTraceMoveReminders;
#endif


};
