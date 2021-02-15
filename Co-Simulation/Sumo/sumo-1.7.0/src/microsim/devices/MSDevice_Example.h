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
/// @file    MSDevice_Example.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    11.06.2013
///
// A device which stands as an implementation example and which outputs movereminder calls
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSVehicleDevice.h"
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOTrafficObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_Example
 * @brief A device which collects info on the vehicle trip (mainly on departure and arrival)
 *
 * Each device collects departure time, lane and speed and the same for arrival.
 *
 * @see MSDevice
 */
class MSDevice_Example : public MSVehicleDevice {
public:
    /** @brief Inserts MSDevice_Example-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a example-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);

    /// @brief resets counters
    static void cleanup();

public:
    /// @brief Destructor.
    ~MSDevice_Example();



    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Checks for waiting steps when the vehicle moves
     *
     * @param[in] veh Vehicle that asks this reminder.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     *
     * @return True (always).
     */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos,
                    double newPos, double newSpeed);


    /** @brief Saves departure info on insertion
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return Always true
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Saves arrival info
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @return True if it did not leave the net.
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos,
                     MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
    /// @}


    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "example";
    }

    /// @brief try to retrieve the given parameter from this device. Throw exception for unsupported key
    std::string getParameter(const std::string& key) const;

    /// @brief try to set the given parameter for this device. Throw exception for unsupported key
    void setParameter(const std::string& key, const std::string& value);

    /** @brief Called on writing tripinfo output
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     * @see MSDevice::generateOutput
     */
    void generateOutput(OutputDevice* tripinfoOut) const;



private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_Example(SUMOVehicle& holder, const std::string& id, double customValue1,
                     double customValue2, double customValue3);



private:
    // private state members of the Example device

    /// @brief a value which is initialised based on a commandline/configuration option
    double myCustomValue1;

    /// @brief a value which is initialised based on a vehicle parameter
    double myCustomValue2;

    /// @brief a value which is initialised based on a vType parameter
    double myCustomValue3;



private:
    /// @brief Invalidated copy constructor.
    MSDevice_Example(const MSDevice_Example&);

    /// @brief Invalidated assignment operator.
    MSDevice_Example& operator=(const MSDevice_Example&);


};
