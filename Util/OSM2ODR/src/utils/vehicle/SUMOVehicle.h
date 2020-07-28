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
/// @file    SUMOVehicle.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Tue, 17 Feb 2009
///
// Abstract base class for vehicle representations
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <typeinfo>
#include <utils/common/SUMOTime.h>
#include <utils/common/Named.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/vehicle/SUMOTrafficObject.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSRoute;
class MSEdge;
class MSLane;
class MSPerson;
class MSTransportable;
class MSParkingArea;
class MSStoppingPlace;
class MSVehicleDevice;
class SUMOSAXAttributes;

typedef std::vector<const MSEdge*> ConstMSEdgeVector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class SUMOVehicle
 * @brief Representation of a vehicle
 */
class SUMOVehicle : public SUMOTrafficObject {
public:
    typedef long long int NumericalID;

    /// @brief Constructor
    SUMOVehicle(const std::string& id) : SUMOTrafficObject(id) {}

    /// @brief Destructor
    virtual ~SUMOVehicle() {}

    /** @brief Get the vehicle's lateral position on the lane
     * @return The lateral position of the vehicle (in m relative to the
     * centerline of the lane)
     */
    virtual double getLateralPositionOnLane() const = 0;

    /** @brief Get the vehicle's angle
     * @return The angle of the vehicle (in degree)
     */
    virtual double getAngle() const = 0;

    /** @brief Returns the lane the vehicle is on
    * @return The vehicle's current lane
    */
    virtual MSLane* getLane() const = 0;


    /// Returns the current route
    virtual const MSRoute& getRoute() const = 0;

    /// @brief return index of edge within route
    virtual int getRoutePosition() const = 0;

    /** @brief Returns the nSuccs'th successor of edge the vehicle is currently at
     *
     * If the rest of the route (counted from the current edge) than nSuccs,
     *  0 is returned.
     * @param[in] nSuccs The number of edge to look forward
     * @return The nSuccs'th following edge in the vehicle's route
     */
    virtual const MSEdge* succEdge(int nSuccs) const = 0;

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
    virtual bool replaceRouteEdges(ConstMSEdgeVector& edges, double cost, double savings, const std::string& info, bool onInit = false, bool check = false, bool removeStops = true) = 0;

    /// Replaces the current route by the given one
    virtual bool replaceRoute(const MSRoute* route, const std::string& info, bool onInit = false, int offset = 0, bool addStops = true, bool removeStops = true) = 0;

    /** @brief Performs a rerouting using the given router
     *
     * Tries to find a new route between the current edge and the destination edge, first.
     * Tries to replace the current route by the new one using replaceRoute.
     *
     * @param[in] t The time for which the route is computed
     * @param[in] router The router to use
     * @see replaceRoute
     */
    virtual void reroute(SUMOTime t, const std::string& info, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, const bool onInit = false, const bool withTaz = false, const bool silent = false) = 0;

    /** @brief Validates the current or given route
     * @param[out] msg Description why the route is not valid (if it is the case)
     * @param[in] route The route to check (or 0 if the current route shall be checked)
     * @return Whether the vehicle's current route is valid
     */
    virtual bool hasValidRoute(std::string& msg, const MSRoute* route = 0) const = 0;
    /// @brief checks wether the vehicle can depart on the first edge
    virtual bool hasValidRouteStart(std::string& msg) = 0;

    /// @brief computes validity attributes for the current route
    virtual int getRouteValidity(bool update = true, bool silent = false) = 0;

    /** @brief Returns an iterator pointing to the current edge in this vehicles route
     * @return The current route pointer
     */
    virtual const ConstMSEdgeVector::const_iterator& getCurrentRouteEdge() const = 0;

    /** @brief Returns the vehicle's parameter (including departure definition)
     *
     * @return The vehicle's parameter
     */
    virtual const SUMOVehicleParameter& getParameter() const = 0;

    /** @brief Returns the vehicle's emission model parameter
     *
     * @return The vehicle's emission parameters
     */
    virtual const std::map<int, double>* getEmissionParameters() const = 0;

    /** @brief Replaces the vehicle's parameter
     */
    virtual void replaceParameter(const SUMOVehicleParameter* newParameter) = 0;

    /** @brief Called when the vehicle is inserted into the network
     *
     * Sets optional information about departure time, informs the vehicle
     *  control about a further running vehicle.
     */
    virtual void onDepart() = 0;

    /** @brief Returns the information whether the vehicle is on a road (is simulated)
     * @return Whether the vehicle is simulated
     */
    virtual bool isOnRoad() const = 0;

    /** @brief Returns whether the vehicle is idling (waiting to re-enter the net
     * @return true if the vehicle is waiting to enter the net (eg after parking)
    */
    virtual bool isIdling() const = 0;

    /** @brief Returns the information whether the front of the vehhicle is on the given lane
     * @return Whether the vehicle's front is on that lane
     */
    virtual bool isFrontOnLane(const MSLane*) const = 0;

    /** @brief Returns the information whether the vehicle is parked
     * @return Whether the vehicle is parked
     */
    virtual bool isParking() const = 0;

    /** @brief Returns the information whether the vehicle is fully controlled
     * via TraCI
     * @return Whether the vehicle is remote-controlled
     */
    virtual bool isRemoteControlled() const = 0;

    /** @brief Returns the information whether the vehicle is fully controlled
     * via TraCI
     * @return Whether the vehicle was remote-controlled within the given time range
     */
    virtual bool wasRemoteControlled(SUMOTime lookBack = DELTA_T) const = 0;

    /** @brief Returns this vehicle's real departure time
     * @return This vehicle's real departure time
     */
    virtual SUMOTime getDeparture() const = 0;

    /** @brief Returns this vehicle's real departure position
     * @return This vehicle's real departure position
     */
    virtual double getDepartPos() const = 0;

    /** @brief Returns this vehicle's desired arrivalPos for its current route
     * (may change on reroute)
     * @return This vehicle's real arrivalPos
     */
    virtual double getArrivalPos() const = 0;

    /** @brief Sets this vehicle's desired arrivalPos for its current route
     */
    virtual void setArrivalPos(double arrivalPos) = 0;

    /** @brief Returns whether this vehicle has departed
     */
    virtual bool hasDeparted() const = 0;

    /** @brief Returns the number of new routes this vehicle got
     * @return the number of new routes this vehicle got
     */
    virtual int getNumberReroutes() const = 0;

    /// @brief whether the given transportable is allowed to board this vehicle
    virtual bool allowsBoarding(MSTransportable* t) const = 0;

    /** @brief Adds a person or container to this vehicle
     *
     * @param[in] transportable The person/container to add
     */
    virtual void addTransportable(MSTransportable* transportable) = 0;

    /** @brief Returns the number of persons
     * @return The number of passengers on-board
     */
    virtual int getPersonNumber() const = 0;

    /** @brief Returns the list of persons
     * @return The list of passengers on-board
     */
    virtual std::vector<std::string> getPersonIDList() const = 0;

    /** @brief Returns the number of containers
     * @return The number of contaiers on-board
     */
    virtual int getContainerNumber() const = 0;

    /// @brief removes a person or container
    virtual void removeTransportable(MSTransportable* t) = 0;

    /// @brief retrieve riding persons
    virtual const std::vector<MSTransportable*>& getPersons() const = 0;

    /// @brief retrieve riding containers
    virtual const std::vector<MSTransportable*>& getContainers() const = 0;

    /** @brief Adds a stop
     *
     * The stop is put into the sorted list.
     * @param[in] stop The stop to add
     * @return Whether the stop could be added
     */
    virtual bool addStop(const SUMOVehicleParameter::Stop& stopPar, std::string& errorMsg, SUMOTime untilOffset = 0, bool collision = false,
                         ConstMSEdgeVector::const_iterator* searchStart = 0) = 0;

    /// @brief return list of route indices and stop positions for the remaining stops
    virtual std::vector<std::pair<int, double> > getStopIndices() const = 0;

    /// @brief returns whether the vehicle serves a public transport line that serves the given stop
    virtual bool isLineStop(double position) const = 0;


    /**
    * returns the next imminent stop in the stop queue
    * @return the upcoming stop
    */
    virtual MSParkingArea* getNextParkingArea() = 0;

    /** @brief Replaces a stop
      *
      * The stop replace the next stop into the sorted list.
      * @param[in] stop The stop to add
      * @return Whether the stop could be added
      */
    virtual bool replaceParkingArea(MSParkingArea* parkingArea, std::string& errorMsg) = 0;

    /// @brief Returns the remaining stop duration for a stopped vehicle or 0
    virtual SUMOTime remainingStopDuration() const = 0;

    /** @brief Returns whether the vehicle is at a stop and waiting for a person or container to continue
     */
    virtual bool isStoppedTriggered() const = 0;

    /** @brief Returns whether the vehicle is stopped in the range of the given position */
    virtual bool isStoppedInRange(const double pos, const double tolerance) const = 0;

    /** @brief Returns whether the vehicle stops at the given stopping place */
    virtual bool stopsAt(MSStoppingPlace* stop) const = 0;

    /** @brief Returns whether the vehicle stops at the given edge */
    virtual bool stopsAtEdge(const MSEdge* edge) const = 0;

    /** @brief Returns parameters of the next stop or nullptr **/
    virtual const SUMOVehicleParameter::Stop* getNextStopParameter() const = 0;

    virtual void setChosenSpeedFactor(const double factor) = 0;

    virtual SUMOTime getAccumulatedWaitingTime() const = 0;

    virtual SUMOTime getDepartDelay() const = 0;

    virtual SUMOTime getTimeLoss() const = 0;

    /// @brief get distance for coming to a stop (used for rerouting checks)
    virtual double getBrakeGap() const = 0;

    /// @brief Returns this vehicles impatience
    virtual double getImpatience() const = 0;

    /** @brief Returns this vehicle's devices
     * @return This vehicle's devices
     */
    virtual const std::vector<MSVehicleDevice*>& getDevices() const = 0;

    /// @brief Returns a device of the given type if it exists or 0
    virtual MSVehicleDevice* getDevice(const std::type_info& type) const = 0;

    /// @brief whether this vehicle is selected in the GUI
    virtual bool isSelected() const = 0;

    /// @brief @return The index of the vehicle's associated RNG
    virtual int getRNGIndex() const = 0;

    /** @brief Returns the associated RNG for this vehicle
    * @return The vehicle's associated RNG
    */
    virtual std::mt19937* getRNG() const = 0;

    /// @brief return the numerical ID which is only for internal usage
    //  (especially fast comparison in maps which need vehicles as keys)
    virtual NumericalID getNumericalID() const = 0;

    /// @brief Returns the vehicles's length
    virtual double getLength() const = 0;

    /// @name state io
    //@{

    /// Saves the states of a vehicle
    virtual void saveState(OutputDevice& out) = 0;

    /** @brief Loads the state of this vehicle from the given description
     */
    virtual void loadState(const SUMOSAXAttributes& attrs, const SUMOTime offset) = 0;
    //@}
};
