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
/// @file    HelpersPHEMlight.h
/// @author  Daniel Krajzewicz
/// @author  Nikolaus Furian
/// @date    Sat, 20.04.2013
///
// Helper methods for PHEMlight-based emission computation
/****************************************************************************/
#pragma once
#include <config.h>

#define INTERNAL_PHEM

#include <vector>
#include <limits>
#include <cmath>
#ifdef INTERNAL_PHEM
#include "PHEMCEPHandler.h"
#endif
#include <foreign/PHEMlight/cpp/CEP.h>
#include <foreign/PHEMlight/cpp/CEPHandler.h>
#include <foreign/PHEMlight/cpp/Helpers.h>
#include <utils/common/StdDefs.h>
#include "PollutantsInterface.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class HelpersPHEMlight
 * @brief Helper methods for PHEMlight-based emission computation
 */
class HelpersPHEMlight : public PollutantsInterface::Helper {
private:
    static const int PHEMLIGHT_BASE = 3 << 16;

public:
    /** @brief Constructor
     */
    HelpersPHEMlight();

    /** @brief Checks whether the string describes a known vehicle class
     * @param[in] eClass The string describing the vehicle emission class
     * @return whether it describes a valid emission class
     */
    SUMOEmissionClass getClassByName(const std::string& eClass, const SUMOVehicleClass vc);

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

    /** @brief Returns a reference weight in kg described by this emission class as described in the Amitran interface
     * This implementation returns only meaningful values for Solo_LKW (truck without trailer) and LNF (light duty vehicles).
     * @param[in] c the emission class
     * @return a reference weight
     */
    double getWeight(const SUMOEmissionClass c) const;

    /** @brief Returns the amount of emitted pollutant given the vehicle type and state (in mg/s or in ml/s for fuel)
     * @param[in] c The vehicle emission class
     * @param[in] v The vehicle's current velocity
     * @param[in] a The vehicle's current acceleration
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return The amount of the pollutant emitted by the given emission class when moving with the given velocity and acceleration [mg/s or ml/s]
     */
    double compute(const SUMOEmissionClass c, const PollutantsInterface::EmissionType e, const double v, const double a, const double slope, const std::map<int, double>* param) const;

    /** @brief Returns the adapted acceleration value, useful for comparing with external PHEMlight references.
     * @param[in] c the emission class
     * @param[in] v the speed value
     * @param[in] a the acceleration value
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return the modified acceleration
     */
    double getModifiedAccel(const SUMOEmissionClass c, const double v, const double a, const double slope) const;

private:
    /** @brief Returns the amount of emitted pollutant given the vehicle type and state (in mg/s or in ml/s for fuel)
    * @param[in] currCep The vehicle emission class
    * @param[in] e The emission type
    * @param[in] p The vehicle's current power
    * @param[in] v The vehicle's current velocity
    * @return The amount of the pollutant emitted by the given emission class when moving with the given velocity and acceleration [mg/s or ml/s]
    */
    double getEmission(const PHEMCEP* oldCep, PHEMlightdll::CEP* currCep, const std::string& e, const double p, const double v) const;

    /// @brief the index of the next class
    int myIndex;
    PHEMlightdll::CEPHandler myCEPHandler;
    mutable PHEMlightdll::Helpers myHelper;
    std::map<SUMOEmissionClass, PHEMlightdll::CEP*> myCEPs;
};
