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
/// @file    MSCFModel_W99.h
/// @author  Jakob Erdmann
/// @date    June 2019
///
// The psycho-physical model of Wiedemann (10-Parameter version from 1999)
// code adapted from https://github.com/glgh/w99-demo
// (MIT License, Copyright (c) 2016 glgh)
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSCFModel.h"
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleType.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/** @class MSCFModel_W99
 * @brief The W99 Model car-following model
 * @see MSCFModel
 */
// XXX: which W99 is this? There are several versions... Below it is stated that it is modified it with Krauss vsafe... (Leo)
class MSCFModel_W99 : public MSCFModel {
public:

    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel_W99(const MSVehicleType* vtype);


    /// @brief Destructor
    ~MSCFModel_W99();


    /// @name Implementations of the MSCFModel interface
    /// @{


    /** @brief Computes the vehicle's safe speed (no dawdling)
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @return EGO's safe speed
     * @see MSCFModel::ffeV
     */
    double followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const;


    /** @brief Computes the vehicle's safe speed for approaching a non-moving obstacle (no dawdling)
     * @param[in] veh The vehicle (EGO)
     * @param[in] gap The (netto) distance to the the obstacle
     * @return EGO's safe speed for approaching a non-moving obstacle
     * @see MSCFModel::ffeS
     * @todo generic Interface, models can call for the values they need
     */
    double stopSpeed(const MSVehicle* const veh, const double speed, double gap) const;


    /** @brief Returns the maximum gap at which an interaction between both vehicles occurs
     *
     * "interaction" means that the LEADER influences EGO's speed.
     * @param[in] veh The EGO vehicle
     * @param[in] vL LEADER's speed
     * @return The interaction gap
     * @todo evaluate signature
     * @see MSCFModel::interactionGap
     */
    double interactionGap(const MSVehicle* const, double vL) const;

    /** @brief Returns the model's name
     * @return The model's name
     * @see MSCFModel::getModelName
     */
    int getModelID() const {
        return SUMO_TAG_CF_W99;
    }


    /** @brief Duplicates the car-following model
     * @param[in] vtype The vehicle type this model belongs to (1:1)
     * @return A duplicate of this car-following model
     */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;


    /// @}

    /*
    enum Status {
        DECEL1,
        DECEL2,
        FOLLOW,
        FREE_FLOW,
    }

    private:
    class VehicleVariables : public MSCFModel::VehicleVariables {
    public:
        VehicleVariables() : lastStatus(FREE_FLOW) {}
        /// @brief state variable for remembering the previous state
        Status lastStatus;
    };
    */


private:
    /// @name model parameter
    /// @{

    //const double myCC0; // StandStill Distance - (using minGap)
    const double myCC1;   // Spacing Time - s
    const double myCC2;   // Following Variation ("max drift") - m
    const double myCC3;   // Threshold for Entering 'Following' - s
    const double myCC4;   // Negative 'Following' Threshold - m/s
    const double myCC5;   // Positive 'Following' Threshold - m/s
    const double myCC6;   // Speed Dependency of Oscillation - 10^-4 rad/s
    const double myCC7;   // Oscillation Acceleration - m/s^2
    const double myCC8;   // Standstill Acceleration - m/s^2
    const double myCC9;   // Acceleration at 80km/h - m/s^2
    /// @}


    void computeThresholds(double speed, double predSpeed, double leaderAccel, double rndVal,
                           double& sdxc, double& sdxo, double& sdxv) const;

private:
    /// @brief Invalidated assignment operator
    MSCFModel_W99& operator=(const MSCFModel_W99& s);
};

