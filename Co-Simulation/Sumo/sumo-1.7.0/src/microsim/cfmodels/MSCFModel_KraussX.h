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
/// @file    MSCFModel_KraussX.h
/// @author  Jakob Erdmann
/// @date    27 Feb 2017
///
// Experimental extensions to the Krauss car-following model
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSCFModel_Krauss.h"
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/** @class MSCFModel_KraussX
 * @brief Krauss car-following model, changing accel and speed by slope
 * @see MSCFModel
 * @see MSCFModel_Krauss
 */
class MSCFModel_KraussX : public MSCFModel_Krauss {
public:
    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel_KraussX(const MSVehicleType* vtype);


    /// @brief Destructor
    ~MSCFModel_KraussX();


    /// @name Implementations of the MSCFModel interface
    /// @{
    /// @brief apply dawdling
    double patchSpeedBeforeLC(const MSVehicle* veh, double vMin, double vMax) const;


    /** @brief Returns the model's name
     * @return The model's name
     * @see MSCFModel::getModelName
     */
    int getModelID() const {
        return SUMO_TAG_CF_KRAUSSX;
    }
    /// @}


    /** @brief Duplicates the car-following model
     * @param[in] vtype The vehicle type this model belongs to (1:1)
     * @return A duplicate of this car-following model
     */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;

private:


    /** @brief Applies driver imperfection (dawdling / sigma)
     * @param[in] vOld The previous speed
     * @param[in] vMin The minimum speed (due to braking constraints)
     * @param[in] vMax The maximum speed that may be driven (all constraints)
     * @return The speed after dawdling
     *
     */
    double dawdleX(double vOld, double vMin, double vMax, std::mt19937* rng) const;

    /// @brief extension parameter nr1
    double myTmp1;
    double myTmp2;

};


