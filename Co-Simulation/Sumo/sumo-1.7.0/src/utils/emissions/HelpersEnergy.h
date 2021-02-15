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
/// @file    HelpersEnergy.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
///
// Helper methods for HBEFA-based emission computation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <limits>
#include <cmath>
#include <utils/common/StdDefs.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/SUMOVehicleClass.h>
#include "PollutantsInterface.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class HelpersEnergy
 * @brief Helper methods for energy-based electricity consumption computation based on the battery device
 */
class HelpersEnergy : public PollutantsInterface::Helper {
private:
    static const int ENERGY_BASE = 4 << 16;

public:
    /** @brief Constructor (initializes myEmissionClassStrings)
     */
    HelpersEnergy();


    /** @brief Computes the emitted pollutant amount using the given speed and acceleration
     *
     * Returns only valid values for electricity all other types give 0.
     *
     * @param[in] c emission class for the function parameters to use
     * @param[in] e the type of emission (CO, CO2, ...), only electricity gives valid results
     * @param[in] v The vehicle's current velocity
     * @param[in] a The vehicle's current acceleration
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return The amount emitted by the given emission class when moving with the given velocity and acceleration [mg/s or ml/s]
     */
    double compute(const SUMOEmissionClass c, const PollutantsInterface::EmissionType e, const double v, const double a, const double slope, const std::map<int, double>* param) const;

    /** @brief Computes the achievable acceleartion using the given speed and amount of consumed electric power
     *
     * @param[in] c emission class for the function parameters to use
     * @param[in] e the type of emission (CO, CO2, ...), only electricity gives valid results
     * @param[in] v The vehicle's current velocity
     * @param[in] P The vehicle's current power consumption
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return The amount emitted by the given emission class when moving with the given velocity and acceleration [mg/s or ml/s]
     */
    double acceleration(const SUMOEmissionClass c, const PollutantsInterface::EmissionType e, const double v, const double P, const double slope, const std::map<int, double>* param) const;

    double getDefaultParam(int paramKey) const {
        return myDefaultParameter.find(paramKey)->second;
    }


private:
    /// @brief The default parameter
    std::map<int, double> myDefaultParameter;
};
