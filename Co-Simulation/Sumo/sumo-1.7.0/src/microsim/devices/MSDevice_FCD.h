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
/// @file    MSDevice_FCD.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    11.06.2013
///
// A device which records floating car data
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSVehicleDevice.h"
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;
class SUMOTrafficObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_FCD
 * @brief A device which collects info on the vehicle trip (mainly on departure and arrival)
 *
 * Each device collects departure time, lane and speed and the same for arrival.
 *
 * @see MSDevice
 */
class MSDevice_FCD : public MSVehicleDevice {
public:
    /** @brief Inserts MSDevice_FCD-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a FCD-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);

public:
    /// @brief Destructor.
    ~MSDevice_FCD();

    bool notifyEnter(SUMOTrafficObject& /*veh*/, MSMoveReminder::Notification /*reason*/, const MSLane* /*enteredLane*/) {
        return false;
    }

    void saveState(OutputDevice& /* out */) const {
    }

    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "fcd";
    }

    static const std::set<const MSEdge*>& getEdgeFilter() {
        return myEdgeFilter;
    }

    /// @brief resets the edge filter
    static void cleanup();

private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_FCD(SUMOVehicle& holder, const std::string& id);


    /// @brief spatial filter for FCD output
    static std::set<const MSEdge*> myEdgeFilter;
    static bool myEdgeFilterInitialized;

    /// @brief initialize edge filter (once)
    static void initEdgeFilter();

private:
    /// @brief Invalidated copy constructor.
    MSDevice_FCD(const MSDevice_FCD&);

    /// @brief Invalidated assignment operator.
    MSDevice_FCD& operator=(const MSDevice_FCD&);


};
