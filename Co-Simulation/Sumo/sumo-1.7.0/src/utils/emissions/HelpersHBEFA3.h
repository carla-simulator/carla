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
/// @file    HelpersHBEFA3.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
///
// Helper methods for HBEFA3-based emission computation
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
 * @class HelpersHBEFA3
 * @brief Helper methods for HBEFA3-based emission computation
 *
 * The parameter are stored per vehicle class; 6*6 parameter are used, sorted by
 *  the pollutant (CO2, CO, HC, fuel, NOx, PMx), and the function part
 *  (c0, cav1, cav2, c1, c2, c3).
 */
class HelpersHBEFA3 : public PollutantsInterface::Helper {
private:
    static const int HBEFA3_BASE = 2 << 16;

public:
    /** @brief Constructor (initializes myEmissionClassStrings)
     */
    HelpersHBEFA3();

    /** @brief Returns the emission class described by the given parameters.
     * @param[in] base the base class giving the default
     * @param[in] vClass the vehicle class as described in the Amitran interface (Passenger, ...)
     * @param[in] fuel the fuel type as described in the Amitran interface (Gasoline, Diesel, ...)
     * @param[in] eClass the emission class as described in the Amitran interface (Euro0, ...)
     * @param[in] weight the vehicle weight in kg as described in the Amitran interface
     * @return the class described by the parameters
     */
    SUMOEmissionClass getClass(const SUMOEmissionClass base, const std::string& vClass, const std::string& fuel, const std::string& eClass, const double weight) const;

    /** @brief Returns the vehicle class described by this emission class as described in the Amitran interface (Passenger, ...)
     * @param[in] c the emission class
     * @return the name of the vehicle class
     */
    std::string getAmitranVehicleClass(const SUMOEmissionClass c) const;

    /** @brief Returns the fuel type described by this emission class as described in the Amitran interface (Gasoline, Diesel, ...)
     * @param[in] c the emission class
     * @return the fuel type
     */
    std::string getFuel(const SUMOEmissionClass c) const;

    /** @brief Returns the Euro emission class described by this emission class as described in the Amitran interface (0, ..., 6)
     * @param[in] c the emission class
     * @return the Euro class
     */
    int getEuroClass(const SUMOEmissionClass c) const;


    /** @brief Computes the emitted pollutant amount using the given speed and acceleration
     *
     * As the functions are defining emissions in g/hour, the function's result is normed
     *  by 3.6 (seconds in an hour/1000) yielding mg/s. For fuel ml/s is returned.
     *  Negative acceleration results directly in zero emission.
     *
     * @param[in] c emission class for the function parameters to use
     * @param[in] e the type of emission (CO, CO2, ...)
     * @param[in] v The vehicle's current velocity
     * @param[in] a The vehicle's current acceleration
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return The amount emitted by the given emission class when moving with the given velocity and acceleration [mg/s or ml/s]
     */
    inline double compute(const SUMOEmissionClass c, const PollutantsInterface::EmissionType e, const double v, const double a, const double slope, const std::map<int, double>* param) const {
        UNUSED_PARAMETER(slope);
        UNUSED_PARAMETER(param);
        if (a < 0. || e == PollutantsInterface::ELEC) {
            return 0.;
        }
        const int index = (c & ~PollutantsInterface::HEAVY_BIT) - HBEFA3_BASE;
        double scale = 3.6;
        if (e == PollutantsInterface::FUEL) {
            if (getFuel(c) == "Diesel") {
                scale *= 836.;
            } else {
                scale *= 742.;
            }
        }
        const double* f = myFunctionParameter[index][e];
        return (double) MAX2((f[0] + f[1] * a * v + f[2] * a * a * v + f[3] * v + f[4] * v * v + f[5] * v * v * v) / scale, 0.);
    }


private:
    /// @brief The function parameter
    static double myFunctionParameter[45][6][6];

};
