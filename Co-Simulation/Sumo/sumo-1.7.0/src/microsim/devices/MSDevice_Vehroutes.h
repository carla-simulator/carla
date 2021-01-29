/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_Vehroutes.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Fri, 30.01.2009
///
// A device which collects info on the vehicle trip
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSVehicleDevice.h"
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <utils/common/SUMOTime.h>
#include <utils/iodevices/OutputDevice_String.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSRoute;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_Vehroutes
 * @brief A device which collects info on the vehicle trip (mainly on departure and arrival)
 *
 * Each device collects departure time, lane and speed and the same for arrival.
 *
 * @see MSDevice
 */
class MSDevice_Vehroutes : public MSVehicleDevice {
public:
    /** @brief Static intialization
     */
    static void init();


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a vehroutes-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static MSDevice_Vehroutes* buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into, int maxRoutes = std::numeric_limits<int>::max());


    /// @brief generate vehroute output for vehicles which are still in the network
    static void generateOutputForUnfinished();


public:
    /// @brief Destructor.
    ~MSDevice_Vehroutes();



    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Does nothing, returns true only if exit times should be collected
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return True, if exit times are to be collected.
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Saves exit times if needed
     *
     * The exit time is collected on all occasions except for lane change.
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] reason how the vehicle leaves the lane
     * @see MSMoveReminder::notifyLeave
     *
     * @return True, if exit times are to be collected.
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos, Notification reason, const MSLane* enteredLane = 0);
    /// @}

    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "vehroute";
    }

    void stopEnded(const SUMOVehicleParameter::Stop& stop);

    /** @brief Called on writing vehroutes output
     *
     * @exception IOError not yet implemented
     */
    void generateOutput(OutputDevice* tripinfoOut) const;


    /** @brief Called on route retrieval
     *
     * @param[in] index The index of the route to retrieve
     * @return the route at the index
     */
    const MSRoute* getRoute(int index) const;


    /** @brief Saves the state of the device
    *
    * @param[in] out The OutputDevice to write the information into
    */
    void saveState(OutputDevice& out) const;

    /** @brief Loads the state of the device from the given description
    *
    * @param[in] attrs XML attributes describing the current state
    */
    void loadState(const SUMOSAXAttributes& attrs);


private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_Vehroutes(SUMOVehicle& holder, const std::string& id, int maxRoutes);


    /** @brief Called on route output
     *
     * @param[in] os the device to write into
     * @param[in] index The index of the route to write (-1 writes the current route)
     */
    void writeXMLRoute(OutputDevice& os, int index = -1) const;


    /** @brief Called on writing vehroutes output
    *
    * @param[in] hasArrived whether the vehicle has valid arrival information
    * @exception IOError not yet implemented
    */
    void writeOutput(const bool hasArrived) const;


    /** @brief Called on route change
     */
    void addRoute(const std::string& info);



private:
    /// @brief A shortcut for the Option "vehroute-output.exit-times"
    static bool mySaveExits;

    /// @brief A shortcut for the Option "vehroute-output.last-route"
    static bool myLastRouteOnly;

    /// @brief A shortcut for the Option "vehroute-output.dua"
    static bool myDUAStyle;

    /// @brief A shortcut for the Option "vehroute-output.costs"
    static bool myWriteCosts;

    /// @brief A shortcut for the Option "vehroute-output.sorted"
    static bool mySorted;

    /// @brief A shortcut for the Option "vehroute-output.intended-depart"
    static bool myIntendedDepart;

    /// @brief A shortcut for the Option "vehroute-output.route-length"
    static bool myRouteLength;

    /// @brief A shortcut for the Option "vehroute-output.skip-ptlines"
    static bool mySkipPTLines;

    /// @brief A shortcut for the Option "vehroute-output.incomplete"
    static bool myIncludeIncomplete;

    /// @brief A shortcut for the Option "vehroute-output.stop-edges"
    static bool myWriteStopPriorEdges;

    /** @class StateListener
     * @brief A class that is notified about reroutings
     */
    class StateListener : public MSNet::VehicleStateListener {
    public:
        /// @brief Destructor
        ~StateListener() {}

        /** @brief Called if a vehicle changes its state
         * @param[in] vehicle The vehicle which changed its state
         * @param[in] to The state the vehicle has changed to
         */
        void vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& info = "");

        /// @brief A map for internal notification
        std::map<const SUMOVehicle*, MSDevice_Vehroutes*, ComparatorNumericalIdLess> myDevices;

    };


    /// @brief A class that is notified about reroutings
    static StateListener myStateListener;

    /// @brief Map needed to sort vehicles by departure time
    static std::map<const SUMOTime, int> myDepartureCounts;

    /// @todo: describe
    static std::map<const SUMOTime, std::map<const std::string, std::string> > myRouteInfos;


    /**
     * @class RouteReplaceInfo
     * @brief Information about a replaced route
     *
     * Generated optionally and stored in a vector within the device
     *  this structure contains information about a replaced route: the edge
     *  the route was replaced at by a new one, the time this was done, and
     *  the previous route.
     */
    class RouteReplaceInfo {
    public:
        /** @brief Constructor
         * @param[in] edge_ The edge the route was replaced at
         * @param[in] time_ The time the route was replaced
         * @param[in] route_ The prior route
         */
        RouteReplaceInfo(const MSEdge* const edge_, const SUMOTime time_, const MSRoute* const route_, const std::string& info_)
            : edge(edge_), time(time_), route(route_), info(info_) {}

        /// @brief Destructor
        ~RouteReplaceInfo() { }

        /// @brief The edge the vehicle was on when the route was replaced
        const MSEdge* edge;

        /// @brief The time the route was replaced
        SUMOTime time;

        /// @brief The prior route
        const MSRoute* route;

        /// @brief Information regarding rerouting
        std::string info;

    };

    /// @brief The currently used route
    const MSRoute* myCurrentRoute;

    /// @brief Prior routes
    std::vector<RouteReplaceInfo> myReplacedRoutes;

    /// @brief The times the vehicle exites an edge
    std::vector<SUMOTime> myExits;

    /// @brief The maximum number of routes to report
    const int myMaxRoutes;

    /// @brief The last edge the exit time was saved for
    const MSEdge* myLastSavedAt;

    /// @brief The lane the vehicle departed at
    int myDepartLane;

    /// @brief The lane the vehicle departed at
    double myDepartPos;

    /// @brief The speed on departure
    double myDepartSpeed;

    /// @brief The lateral depart position
    double myDepartPosLat;

    /// @brief the edges that were passed before the current stop
    std::vector<const MSEdge*> myPriorEdges;

    OutputDevice_String myStopOut;

private:
    /// @brief Invalidated copy constructor.
    MSDevice_Vehroutes(const MSDevice_Vehroutes&);

    /// @brief Invalidated assignment operator.
    MSDevice_Vehroutes& operator=(const MSDevice_Vehroutes&);


};
