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
/// @file    MSCFModel_Krauss.h
/// @author  Tobias Mayer
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 28 Jul 2009
///
// Krauss car-following model, with acceleration decrease and faster start
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSCFModel_KraussOrig1.h"
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/** @class MSCFModel_Krauss
 * @brief Krauss car-following model, with acceleration decrease and faster start
 * @see MSCFModel
 */
class MSCFModel_Krauss : public MSCFModel_KraussOrig1 {
public:
    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel_Krauss(const MSVehicleType* vtype);


    /// @brief Destructor
    ~MSCFModel_Krauss();

    /// @name Implementations of the MSCFModel interface
    /// @{
    /// @brief apply dawdling
    double patchSpeedBeforeLC(const MSVehicle* veh, double vMin, double vMax) const;

    /** @brief Computes the vehicle's safe speed for approaching a non-moving obstacle (no dawdling)
     * this uses the maximumSafeStopSpeed
     * @param[in] veh The vehicle (EGO)
     * @param[in] gap2pred The (netto) distance to the the obstacle
     * @return EGO's safe speed for approaching a non-moving obstacle
     * @todo generic Interface, models can call for the values they need
     */
    double stopSpeed(const MSVehicle* const veh, const double speed, double gap2pred) const;


    /** @brief Computes the vehicle's safe speed (no dawdling)
     * this uses the maximumSafeFollowSpeed
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @param[in] pred The leading vehicle (LEADER)
     * @return EGO's safe speed
     */
    double followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const;


    /** @brief Returns the model's name
     * @return The model's name
     * @see MSCFModel::getModelName
     */
    int getModelID() const {
        return SUMO_TAG_CF_KRAUSS;
    }
    /// @}


    /** @brief Duplicates the car-following model
     * @param[in] vtype The vehicle type this model belongs to (1:1)
     * @return A duplicate of this car-following model
     */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;


protected:

    /** @brief Applies driver imperfection (dawdling / sigma)
     * @param[in] speed The speed with no dawdling
     * @param[in] sigma The sigma value to use
     * @return The speed after dawdling
     */
    double dawdle2(double speed, double sigma, std::mt19937* rng) const;

};


