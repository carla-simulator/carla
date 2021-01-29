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
/// @file    MSCFModel_CACC.h
/// @author  Kallirroi Porfyri
/// @date    Nov 2018
///
// CACC car-following model based on [1], [2].
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
#include "MSCFModel_ACC.h"
#include <utils/xml/SUMOXMLDefinitions.h>

// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;
class MSVehicleType;

// ===========================================================================
// class definitions
// ===========================================================================
/** @class MSCFModel_CACC
* @brief The CACC car-following model
* @see MSCFModel
*/
class MSCFModel_CACC : public MSCFModel {
public:
    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */

    MSCFModel_CACC(const MSVehicleType* vtype);

    /// @brief Destructor
    ~MSCFModel_CACC();


    /// @name Implementations of the MSCFModel interface
    /// @{

    /** @brief Computes the vehicle's safe speed without a leader
     *
     * Returns the velocity of the vehicle in dependence to the length of the free street and the target
     *  velocity at the end of the free range. If onInsertion is true, the vehicle may still brake
     *  before the next movement.
     * XXX: Currently only needed to (re-)set "caccVehicleMode" parameter to default value.
     *
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] seen The look ahead distance
     * @param[in] maxSpeed The maximum allowed speed
     * @param[in] onInsertion whether speed at insertion is asked for
     * @return EGO's safe speed
     */
    double freeSpeed(const MSVehicle* const veh, double speed, double seen, double maxSpeed, const bool onInsertion = false) const;

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
     * @param[in] pred The leader vehicle, for obtaining other values
     * @param[in] speed EGO's speed
     * @param[in] leaderSpeed LEADER's speed
     * @param[in] leaderMaxDecel LEADER's max. deceleration rate
     */
    double getSecureGap(const MSVehicle* const veh, const MSVehicle* const pred, const double speed, const double leaderSpeed, const double leaderMaxDecel) const;

    /** @brief Computes the vehicle's acceptable speed at insertion
     * @param[in] veh The vehicle (EGO)
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


    /**
     * @brief try to get the given parameter for this carFollowingModel
     *
     * @param[in] veh the vehicle from which the parameter must be retrieved
     * @param[in] key the key of the parameter
     * @return the value of the requested parameter
     */
    virtual std::string getParameter(const MSVehicle* veh, const std::string& key) const;


    /**
     * @brief try to set the given parameter for this carFollowingModel
     *
     * @param[in] veh the vehicle for which the parameter must be set
     * @param[in] key the key of the parameter
     * @param[in] value the value to be set for the given parameter
     */
    virtual void setParameter(MSVehicle* veh, const std::string& key, const std::string& value) const;


    /** @brief Returns the model's name
    * @return The model's name
    * @see MSCFModel::getModelName
    */
    int getModelID() const {
        return SUMO_TAG_CF_CACC;
    }
    /// @}



    /** @brief Duplicates the car-following model
    * @param[in] vtype The vehicle type this model belongs to (1:1)
    * @return A duplicate of this car-following model
    */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;

    virtual MSCFModel::VehicleVariables* createVehicleVariables() const {
        CACCVehicleVariables* ret = new CACCVehicleVariables();
        ret->CACC_ControlMode = 0;
        ret->CACC_CommunicationsOverrideMode = CACC_NO_OVERRIDE;
        ret->lastUpdateTime = 0;
        return ret;
    }


private:
    enum CommunicationsOverrideMode {
        CACC_NO_OVERRIDE = 0,
        CACC_MODE_NO_LEADER = 1,
        CACC_MODE_LEADER_NO_CAV = 2,
        CACC_MODE_LEADER_CAV = 3
    };

    static std::map<std::string, CommunicationsOverrideMode> CommunicationsOverrideModeMap;

    /// @brief Vehicle mode (default is CACC)
    /// Switch to ACC mode if CACC_ControlMode = 1 (gap control mode) _and_ leader's CFModel != CACC
    enum VehicleMode {
        ACC_MODE = 0,
        CACC_MODE
    };

    /// @brief Vehicle mode name map
    static std::map<VehicleMode, std::string> VehicleModeNames;

    class CACCVehicleVariables : public MSCFModel::VehicleVariables {
    public:
        CACCVehicleVariables() : CACC_ControlMode(0), CACC_CommunicationsOverrideMode(CACC_NO_OVERRIDE) {}
        /// @brief The vehicle's CACC  precious time step gap error
        int    CACC_ControlMode;
        CommunicationsOverrideMode CACC_CommunicationsOverrideMode;
        SUMOTime lastUpdateTime;
    };

private:
    double _v(const MSVehicle* const veh, const MSVehicle* const pred, const double gap2pred, const double mySpeed,
              const double predSpeed, const double desSpeed, const bool respectMinGap = true) const;

    double speedSpeedControl(const double speed, double vErr, VehicleMode& vehMode) const;
    double speedGapControl(const MSVehicle* const veh, const double gap2pred,
                           const double speed, const double predSpeed, const double desSpeed, double vErr,
                           const MSVehicle* const pred, VehicleMode& vehMode) const;

private:
    MSCFModel_ACC acc_CFM;
    double mySpeedControlGain;
    double myGapClosingControlGainGap;
    double myGapClosingControlGainGapDot;
    double myGapControlGainGap;
    double myGapControlGainGapDot;
    double myCollisionAvoidanceGainGap;
    double myCollisionAvoidanceGainGapDot;
    double myHeadwayTimeACC;

private:
    /// @brief Invalidated assignment operator
    MSCFModel_CACC& operator=(const MSCFModel_CACC& s);
};

