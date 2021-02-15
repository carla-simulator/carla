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
/// @file    MEVehicleControl.h
/// @author  Daniel Krajzewicz
/// @date    Wed, 10. Dec 2003
///
// The class responsible for building and deletion of vehicles (meso-version)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <microsim/MSVehicleControl.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MEVehicleControl
 * @brief The class responsible for building and deletion of vehicles (gui-version)
 *
 * Builds GUIVehicle instances instead of MSVehicle.
 *
 * @see MSVehicleControl
 * @todo This is partially unsecure due to concurrent access...
 * @todo Recheck vehicle deletion
 */
class MEVehicleControl : public MSVehicleControl {
public:
    /// @brief Constructor
    MEVehicleControl();


    /// @brief Destructor
    ~MEVehicleControl();


    /// @name Vehicle creation
    /// @{

    /** @brief Builds a vehicle, increases the number of built vehicles
     *
     * Instead of a MSVehicle, a GUIVehicle is built
     *
     * @param[in] defs The parameter defining the vehicle
     * @param[in] route The route of this vehicle
     * @param[in] type The type of this vehicle
     * @param[in] ignoreStopErrors whether invalid stops trigger a warning only
     * @param[in] fromRouteFile whether we are just reading the route file or creating via trigger, traci, ...
     * @return The built vehicle (MEVehicle instance)
     * @see MSVehicleControl::buildVehicle
     */
    virtual SUMOVehicle* buildVehicle(SUMOVehicleParameter* defs,
                                      const MSRoute* route, MSVehicleType* type,
                                      const bool ignoreStopErrors, const bool fromRouteFile = true);
    /// @}


private:
    /// @brief invalidated copy constructor
    MEVehicleControl(const MEVehicleControl& s);

    /// @brief invalidated assignment operator
    MEVehicleControl& operator=(const MEVehicleControl& s);


};
