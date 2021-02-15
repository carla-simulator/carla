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
/// @file    MSCFModel_Wiedemann.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    June 2011
///
// The psycho-physical model of Wiedemann
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
/** @class MSCFModel_Wiedemann
 * @brief The Wiedemann Model car-following model
 * @see MSCFModel
 */
// XXX: which Wiedemann is this? There are several versions... Below it is stated that it is modified it with Krauss vsafe... (Leo)
class MSCFModel_Wiedemann : public MSCFModel {
public:

    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel_Wiedemann(const MSVehicleType* vtype);


    /// @brief Destructor
    ~MSCFModel_Wiedemann();


    /// @name Implementations of the MSCFModel interface
    /// @{

    /** @brief Applies interaction with stops and lane changing model influences
     * @param[in] veh The ego vehicle
     * @param[in] vPos The possible velocity
     * @return The velocity after applying interactions with stops and lane change model influences
     */
    double finalizeSpeed(MSVehicle* const veh, double vPos) const;


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

    /** @brief Returns the minimum gap to reserve if the leader is braking at maximum (>=0)
     * @param[in] veh The vehicle itself, for obtaining other values
     * @param[in] pred The leader vehicle, for obtaining other values
     * @param[in] speed EGO's speed
     * @param[in] leaderSpeed LEADER's speed
     * @param[in] leaderMaxDecel LEADER's max. deceleration rate
     */
    double getSecureGap(const MSVehicle* const veh, const MSVehicle* const pred, const double speed, const double leaderSpeed, const double leaderMaxDecel) const;

    /** @brief Returns the model's name
     * @return The model's name
     * @see MSCFModel::getModelName
     */
    int getModelID() const {
        return SUMO_TAG_CF_WIEDEMANN;
    }


    /** @brief Duplicates the car-following model
     * @param[in] vtype The vehicle type this model belongs to (1:1)
     * @return A duplicate of this car-following model
     */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;


    VehicleVariables* createVehicleVariables() const {
        return new VehicleVariables();
    }
    /// @}


private:
    class VehicleVariables : public MSCFModel::VehicleVariables {
    public:
        VehicleVariables() : accelSign(1) {}
        /// @brief state variable for remembering the drift direction
        double accelSign;
    };


private:
    /* @brief the main enty point for the speed computation
     * @param[in] gap The netto gap (front bumper of ego to back bumper of leader)
     */
    double _v(const MSVehicle* veh, double predSpeed, double gap) const;

    /// @name acceleration based on the 'driving regime'
    /// @{
    double fullspeed(double v, double vpref, double dx, double bx) const; // also 'WUNSCH'
    double following(double sign) const; // also 'FOLGEN'
    double approaching(double dv, double dx, double abx) const;  // also 'BREMSBX'
    double emergency(double dv, double dx) const; // also 'BREMSAX'
    /// @}

private:
    /// @name model parameter
    /// @{

    /// @brief The driver's security parameter // also 'ZF1'
    const double mySecurity;

    /// @brief The driver's estimation parameter // also 'ZF2'
    const double myEstimation;

    /// @brief the minimum front-bumper to front-bumper distance when standing
    const double myAX;

    /// @brief perception threshold modifier
    const double myCX;

    /// @brief The vehicle's minimum acceleration [m/s^2] // also b_null
    const double myMinAccel;

    /// @brief free-flow distance in m
    static const double D_MAX;
    /// @}

private:
    /// @brief Invalidated assignment operator
    MSCFModel_Wiedemann& operator=(const MSCFModel_Wiedemann& s);
};

