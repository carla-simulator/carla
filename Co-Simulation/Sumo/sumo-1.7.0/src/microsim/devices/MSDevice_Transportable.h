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
/// @file    MSDevice_Transportable.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @date    Fri, 30.01.2009
///
// A device which is used to keep track of persons and containers riding with a vehicle
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include "MSVehicleDevice.h"
#include <utils/common/SUMOTime.h>
#include <microsim/MSVehicle.h>
#include <utils/common/WrappingCommand.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_Transportable
 * @see MSDevice
 */
class MSDevice_Transportable : public MSVehicleDevice {
public:
    /** @brief Build devices for the given vehicle, if needed
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static MSDevice_Transportable* buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into, const bool isContainer);



public:
    /// @brief Destructor.
    ~MSDevice_Transportable();


    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Checks whether the vehicle is at a stop and transportable action is needed.
     * @param[in] veh The regarded vehicle
     * @param[in] oldPos Position before the move-micro-timestep.
     * @param[in] newPos Position after the move-micro-timestep.
     * @param[in] newSpeed The vehicle's current speed
     * @return false, if the vehicle is beyond the lane, true otherwise
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyMove
     */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos, double newSpeed);


    /** @brief Adds passengers on vehicle insertion
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return Always true
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Passengers leaving on arrival
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     *
     * @return True if it did not leave the net.
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos,
                     MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
    /// @}


    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return myAmContainer ? "container" : "person";
    }

    /// @brief try to retrieve the given parameter from this device. Throw exception for unsupported key
    std::string getParameter(const std::string& key) const;

    /** @brief Add a passenger
     *
     * @param[in] transportable The passenger / container to add.
     */
    void addTransportable(MSTransportable* transportable);

    /** @brief Remove a passenger (TraCI)
     *
     * @param[in] transportable The passenger / container to remove.
     */
    void removeTransportable(MSTransportable* transportable);

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

    /** @brief Return the number of passengers / containers
     * @return The number of stored transportables
     */
    int size() const {
        return (int)myTransportables.size();
    }


    /** @brief Returns the list of transportables using this vehicle
     * @return transportables within this vehicle
     */
    const std::vector<MSTransportable*>& getTransportables() const {
        return myTransportables;
    }

protected:
    /** @brief Internal notification about the vehicle moves, see MSMoveReminder::notifyMoveInternal()
     *
     */
    void notifyMoveInternal(const SUMOTrafficObject& veh,
                            const double frontOnLane,
                            const double timeOnLane,
                            const double meanSpeedFrontOnLane,
                            const double meanSpeedVehicleOnLane,
                            const double travelledDistanceFrontOnLane,
                            const double travelledDistanceVehicleOnLane,
                            const double /* meanLengthOnLane */);

private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_Transportable(SUMOVehicle& holder, const std::string& id, const bool isContainer);



private:
    /// @brief Whether it is a container device
    const bool myAmContainer;

    /// @brief The passengers of the vehicle
    std::vector<MSTransportable*> myTransportables;

    /// @brief Whether the vehicle is at a stop
    bool myStopped;


private:
    /// @brief Invalidated copy constructor.
    MSDevice_Transportable(const MSDevice_Transportable&);

    /// @brief Invalidated assignment operator.
    MSDevice_Transportable& operator=(const MSDevice_Transportable&);


};
