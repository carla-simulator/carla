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
/// @file    MSCFModel_ACC.h
/// @author  Kallirroi Porfyri
/// @date    Feb 2018
///
// ACC car-following model based on [1], [2].
// [1] Milanes, V., and S. E. Shladover. Handling Cut-In Vehicles in Strings
//    of Cooperative Adaptive Cruise Control Vehicles. Journal of Intelligent
//     Transportation Systems, Vol. 20, No. 2, 2015, pp. 178-191.
// [2] Xiao, L., M. Wang and B. van Arem. Realistic Car-Following Models for
//    Microscopic Simulation of Adaptive and Cooperative Adaptive Cruise
//     Control Vehicles. Transportation Research Record: Journal of the
//     Transportation Research Board, No. 2623, 2017. (DOI: 10.3141/2623-01).
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSCFModel.h"
#include <utils/xml/SUMOXMLDefinitions.h>

// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;
class MSVehicleType;
class MSCFModel_CACC;

// ===========================================================================
// class definitions
// ===========================================================================
/** @class MSCFModel_ACC
* @brief The ACC car-following model
* @see MSCFModel
*/
class MSCFModel_ACC : public MSCFModel {
public:
    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel_ACC(const MSVehicleType* vtype);

    /// @brief Destructor
    ~MSCFModel_ACC();


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
    * @param[in] gap2pred The (netto) distance to the the obstacle
    * @return EGO's safe speed for approaching a non-moving obstacle
    * @see MSCFModel::ffeS
    * @todo generic Interface, models can call for the values they need
    */
    double stopSpeed(const MSVehicle* const veh, const double speed, double gap2pred) const;

    /** @brief Returns the a gap such that the gap mode acceleration of the follower is zero
     * @param[in] veh The vehicle itself, for obtaining other values
     * @param[in] speed EGO's speed
     * @param[in] leaderSpeed LEADER's speed
     * @param[in] leaderMaxDecel LEADER's max. deceleration rate
     */
    double getSecureGap(const MSVehicle* const veh, const MSVehicle* const pred, const double speed, const double leaderSpeed, const double leaderMaxDecel) const;

    /** @brief Computes the vehicle's acceptable speed at insertion
     * @param[in] veh The vehicle (EGO)
     * @param[in] pred The leader vehicle, for obtaining other values
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @return EGO's safe speed
     */
    double insertionFollowSpeed(const MSVehicle* const v, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const;


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
        return SUMO_TAG_CF_ACC;
    }
    /// @}



    /** @brief Duplicates the car-following model
    * @param[in] vtype The vehicle type this model belongs to (1:1)
    * @return A duplicate of this car-following model
    */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;

    VehicleVariables* createVehicleVariables() const {
        ACCVehicleVariables* ret = new ACCVehicleVariables();
        ret->ACC_ControlMode = 0;
        ret->lastUpdateTime = 0;
        return ret;
    }

    friend class MSCFModel_CACC;

private:
    class ACCVehicleVariables : public MSCFModel::VehicleVariables {
    public:
        ACCVehicleVariables() : ACC_ControlMode(0) {}
        /// @brief The vehicle's ACC control mode. 0 for speed control and 1 for gap control
        int ACC_ControlMode;
        SUMOTime lastUpdateTime;
    };


private:
    double _v(const MSVehicle* const veh, const double gap2pred, const double mySpeed,
              const double predSpeed, const double desSpeed, const bool respectMinGap = true) const;

    double accelSpeedControl(double vErr) const;
    double accelGapControl(const MSVehicle* const veh, const double gap2pred, const double speed, const double predSpeed, double vErr) const;


private:
    double mySpeedControlGain;
    double myGapClosingControlGainSpeed;
    double myGapClosingControlGainSpace;
    double myGapControlGainSpeed;
    double myGapControlGainSpace;
    double myCollisionAvoidanceGainSpeed;
    double myCollisionAvoidanceGainSpace;

private:
    /// @brief Invalidated assignment operator
    MSCFModel_ACC& operator=(const MSCFModel_ACC& s);
};

