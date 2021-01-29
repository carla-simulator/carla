/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_BTsender.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    14.08.2013
///
// A BT sender
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <string>
#include "MSVehicleDevice.h"
#include <utils/common/SUMOTime.h>
#include <utils/geom/Boundary.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;
class SUMOTrafficObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_BTsender
 * @brief A BT sender
 *
 * @see MSDevice
 */
class MSDevice_BTsender : public MSVehicleDevice {
public:
    /** @brief Inserts MSDevice_BTsender-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a bt-sender-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);


    /** @brief removes remaining vehicleInformation in sVehicles
     */
    static void cleanup();


    /// for accessing the maps of running/arrived vehicles
    friend class MSDevice_BTreceiver;



public:
    /// @brief Destructor.
    ~MSDevice_BTsender();



    /// @name Methods inherited from MSMoveReminder.
    /// @{

    /** @brief Adds the vehicle to running vehicles if it (re-) enters the network
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return Always true
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, Notification reason, const MSLane* enteredLane = 0);


    /** @brief Checks whether the reminder still has to be notified about the vehicle moves
     *
     * Indicator if the reminders is still active for the passed
     * vehicle/parameters. If false, the vehicle will erase this reminder
     * from it's reminder-container.
     *
     * @param[in] veh Vehicle that asks this reminder.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     *
     * @return True if vehicle hasn't passed the reminder completely.
     */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos, double newSpeed);


    /** @brief Moves (the known) vehicle from running to arrived vehicles' list
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyLeave
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos, Notification reason, const MSLane* enteredLane = 0);
    /// @}

    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "btsender";
    }


    /** @class VehicleState
     * @brief A single movement state of the vehicle
     */
    class VehicleState {
    public:
        /** @brief Constructor
         * @param[in] _speed The speed of the vehicle
         * @param[in] _position The position of the vehicle
         * @param[in] _laneID The id of the lane the vehicle is located at
         * @param[in] _lanePos The position of the vehicle along the lane
         */
        VehicleState(const double _speed, const Position& _position, const std::string& _laneID,
                     const double _lanePos, const int _routePos)
            : speed(_speed), position(_position), laneID(_laneID), lanePos(_lanePos), routePos(_routePos) {}

        /// @brief Destructor
        ~VehicleState() {}

        /// @brief The speed of the vehicle
        double speed;
        /// @brief The position of the vehicle
        Position position;
        /// @brief The lane the vehicle was at
        std::string laneID;
        /// @brief The position at the lane of the vehicle
        double lanePos;
        /// @brief The position in the route of the vehicle
        int routePos;

    };



    /** @class VehicleInformation
     * @brief Stores the information of a vehicle
     */
    class VehicleInformation : public Named {
    public:
        /** @brief Constructor
         * @param[in] id The id of the vehicle
         */
        VehicleInformation(const std::string& id) : Named(id), amOnNet(true), haveArrived(false)  {}

        /// @brief Destructor
        virtual ~VehicleInformation() {}

        /** @brief Returns the boundary of passed positions
         * @return The positions boundary
         */
        Boundary getBoxBoundary() const {
            Boundary ret;
            for (std::vector<VehicleState>::const_iterator i = updates.begin(); i != updates.end(); ++i) {
                ret.add((*i).position);
            }
            return ret;
        }

        /// @brief List of position updates during last step
        std::vector<VehicleState> updates;

        /// @brief Whether the vehicle is within the simulated network
        bool amOnNet;

        /// @brief Whether the vehicle was removed from the simulation
        bool haveArrived;

        /// @brief List of edges travelled
        ConstMSEdgeVector route;

    };



private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_BTsender(SUMOVehicle& holder, const std::string& id);



protected:
    /// @brief The list of arrived senders
    static std::map<std::string, VehicleInformation*> sVehicles;



private:
    /// @brief Invalidated copy constructor.
    MSDevice_BTsender(const MSDevice_BTsender&);

    /// @brief Invalidated assignment operator.
    MSDevice_BTsender& operator=(const MSDevice_BTsender&);


};
