/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSVehicleDevice.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Tue, 04 Dec 2007
///
// Abstract in-vehicle device
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <random>
#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/MSMoveReminder.h>
#include "MSDevice.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicleDevice
 * @brief Abstract in-vehicle device
 *
 * The MSVehicleDevice-interface brings the following interfaces to a vehicle that
 *  may be overwritten by real devices:
 * @arg Retrieval of the vehicle that holds the device
 * @arg Building and retrieval of a device id
 * @arg Methods called on vehicle movement / state change
 *
 * The "methods called on vehicle movement / state change" are called for each
 *  device within the corresponding vehicle methods. MSVehicleDevice brings already
 *  an empty (nothing doing) implementation of these.
 */
class MSVehicleDevice : public MSMoveReminder, public MSDevice {
public:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSVehicleDevice(SUMOVehicle& holder, const std::string& id) :
        MSMoveReminder(id), MSDevice(id), myHolder(holder) {
    }


    /// @brief Destructor
    virtual ~MSVehicleDevice() { }


    /** @brief Returns the vehicle that holds this device
     *
     * @return The vehicle that holds this device
     */
    inline SUMOVehicle& getHolder() const {
        return myHolder;
    }

    inline SUMOVehicle::NumericalID getNumericalID() const {
        return myHolder.getNumericalID();
    }

    /// @brief Function-object for stable sorting of objects with numerical ids
    struct ComparatorNumericalVehicleIdLess {
        bool operator()(const MSVehicleDevice* const a, const MSVehicleDevice* const b) const {
            return a->myHolder.getNumericalID() < b->myHolder.getNumericalID();
        }
    };


protected:
    /// @brief The vehicle that stores the device
    SUMOVehicle& myHolder;

private:
    /// @brief Invalidated copy constructor.
    MSVehicleDevice(const MSVehicleDevice&);

    /// @brief Invalidated assignment operator.
    MSVehicleDevice& operator=(const MSVehicleDevice&);

};
