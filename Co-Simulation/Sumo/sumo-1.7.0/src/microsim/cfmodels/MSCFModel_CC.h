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
/// @file    MSCFModel_CC.h
/// @author  Michele Segata
/// @date    Wed, 18 Apr 2012
///
// A series of automatic Cruise Controllers (CC, ACC, CACC)
/****************************************************************************/
#pragma once
#include <config.h>

#include "CC_Const.h"
#include <microsim/cfmodels/MSCFModel.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleType.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <microsim/cfmodels/MSCFModel_Krauss.h>
#include <string.h>

#include <microsim/engine/GenericEngineModel.h>
#include <microsim/engine/FirstOrderLagModel.h>
#include <microsim/engine/RealisticEngineModel.h>

#include "CC_VehicleVariables.h"


// ===========================================================================
// class definitions
// ===========================================================================
/** @class MSCFModel_CC
 * @brief A set of automatic Cruise Controllers, including classic Cruise
 * Control (CC), Adaptive Cruise Control (ACC) and Cooperative Adaptive Cruise
 * Control (CACC). Take as references the chapters 5, 6 and 7 of the Rajamani's
 * book "Vehicle dynamics and control" (2011).
 * This model is meant to be used for simulation of platooning systems in mixed
 * scenarios, so with automatic and driver controlled vehicles.
 * The platooning manager is a distributed application implemented for veins
 * (so for omnet++) supported by a 802.11p based communication protocol, which
 * will determine the actions to be performed (such as switching on the
 * automatic controller, or the lane to move to) and communicate them to this
 * car following models via TraCI
 * @see MSCFModel
 */
class MSCFModel_CC : public MSCFModel {
public:

    /** @brief Constructor
     * @param[in] accel The maximum acceleration that controllers can output (def. 1.5 m/s^2)
     * @param[in] decel The maximum deceleration that ACC and CACC controllers can output (def. 6 m/s^2)
     * @param[in] ccDecel The maximum deceleration that the CC can output (def. 1.5 m/s^2)
     * @param[in] headwayTime the headway gap for ACC (be aware of instabilities) (def. 1.5 s)
     * @param[in] constantSpacing the constant gap for CACC (def. 5 m)
     * @param[in] kp design constant for CC (def. 1)
     * @param[in] lambda design constant for ACC (def. 0.1)
     * @param[in] c1 design constant for CACC (def. 0.5)
     * @param[in] xi design constant for CACC (def. 1)
     * @param[in] omegaN design constant for CACC (def. 0.2)
     * @param[in] tau engine time constant used for actuation lag (def. 0.5 s)
     * @param[in] lanesCount number of lanes of the highway
     * @param[in] ccAccel the maximum acceleration the CC can apply
     */
    MSCFModel_CC(const MSVehicleType* vtype);

    /// @brief Destructor
    ~MSCFModel_CC();


    /// @name Implementations of the MSCFModel interface
    /// @{

    /** @brief Applies interaction with stops and lane changing model influences
     * @param[in] veh The ego vehicle
     * @param[in] vPos The possible velocity
     * @return The velocity after applying interactions with stops and lane change model influences
     */
    virtual double finalizeSpeed(MSVehicle* const veh, double vPos) const;


    /** @brief Computes the vehicle's safe speed (no dawdling)
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @return EGO's safe speed
     * @see MSCFModel::ffeV
     */
    double followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const;

    /** @brief Overload base MSCFModel::insertionFollowSpeed method to inject
     * automated vehicles as soon as they are requested, without checking
     * for safe speed constraints
     *
     */
    virtual double insertionFollowSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const;


    /** @brief Computes the vehicle's safe speed for approaching a non-moving obstacle (no dawdling)
     * @param[in] veh The vehicle (EGO)
     * @param[in] gap2pred The (netto) distance to the the obstacle
     * @return EGO's safe speed for approaching a non-moving obstacle
     * @see MSCFModel::ffeS
     * @todo generic Interface, models can call for the values they need
     */
    double stopSpeed(const MSVehicle* const veh, const double speed, double gap2pred) const;

    /** @brief Computes the vehicle's safe speed without a leader
     *
     * Returns the velocity of the vehicle in dependence to the length of the free street and the target
     *  velocity at the end of the free range. If onInsertion is true, the vehicle may still brake
     *  before the next movement.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] seen The look ahead distance
     * @param[in] maxSpeed The maximum allowed speed
     * @param[in] onInsertion whether speed at insertion is asked for
     * @return EGO's safe speed
     */
    virtual double freeSpeed(const MSVehicle* const veh, double speed, double seen,
                             double maxSpeed, const bool onInsertion = false) const;

    virtual double maxNextSpeed(double speed, const MSVehicle* const veh) const;

    virtual double minNextSpeed(double speed, const MSVehicle* const veh = 0) const;


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
        return SUMO_TAG_CF_CC;
    }
    /// @}



    /** @brief Duplicates the car-following model
     * @param[in] vtype The vehicle type this model belongs to (1:1)
     * @return A duplicate of this car-following model
     */
    MSCFModel* duplicate(const MSVehicleType* vtype) const;

    VehicleVariables* createVehicleVariables() const;

    /**
     * @brief returns CACC desired constant spacing
     *
     * @param[in] veh the vehicle to get constant spacing of
     * @return spacing the spacing in meters
     */
    double getCACCConstantSpacing(const MSVehicle* veh) const;

    /**
     * @brief set the information about a generic car. This method should be invoked
     * by TraCI when a wireless message with such data is received. For testing, it might
     * be also invoked from SUMO source code
     *
     * @param[in] veh the vehicle for which the data must be saved
     * @param[in] speed the leader speed
     * @param[in] acceleration the leader acceleration
     * @param[in] position the position of the leader
     * @param[in] time the time at which this data was read from leader's sensors
     */
//    void setVehicleInformation(const MSVehicle* veh, double speed, double acceleration, Position position, double time)  const;

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

    /**
     * @brief get the information about a vehicle. This can be used by TraCI in order to
     * get speed and acceleration of the platoon leader before sending them to other
     * vehicles
     *
     * @param[in] veh the vehicle for which the data is requested
     * @param[out] speed where the speed is written
     * @param[out] acceleration where the acceleration is written
     * @param[out] controllerAcceleration the last acceleration value computed by
     * the controller will be written in this variable. This might be different from
     * acceleration because of actuation lag
     */
    void getVehicleInformation(const MSVehicle* veh, double& speed, double& acceleration, double& controllerAcceleration, Position& position, double& time) const;

    /**
     * @brief switch on the ACC, so disabling the human driver car control
     *
     * @param[in] veh the vehicle for which the ACC must be switched on
     * @param[in] ccDesiredSpeed the cruise control speed
     */
    void switchOnACC(const MSVehicle* veh, double ccDesiredSpeed) const;

    /**
     * @brief return the currently active controller
     *
     * @param[in] veh the vehicle for which the action is requested
     * @return the currently active controller
     */
    enum Plexe::ACTIVE_CONTROLLER getActiveController(const MSVehicle* veh) const;

    /**
     * @brief return the data that is currently being measured by the radar
     */
    void getRadarMeasurements(const MSVehicle* veh, double& distance, double& relativeSpeed) const;

    /**
     * @brief returns the ACC computed acceleration when the faked
     * CACC is controlling the car. This can be used to check for
     * vehicles in front
     */
    double getACCAcceleration(const MSVehicle* veh) const;

    /**
     * @brief returns the number of lanes set in the configuration file
     */
    int getMyLanesCount() const;

private:

    /**
     * @brief Recomputes controller related parameters after setting them
     */
    void recomputeParameters(const MSVehicle* veh) const;

    /**
     * @brief Resets the consensus controller. In particular, sets the
     * "initialized" vector all to false. This might be useful when changing
     * topology.
     */
    void resetConsensus(const MSVehicle* veh) const;

private:
    void performAutoLaneChange(MSVehicle* const veh) const;

    double _v(const MSVehicle* const veh, double gap2pred, double egoSpeed, double predSpeed) const;

    /** @brief controller for the CC which computes the acceleration to be applied. the value needs to be passed to the actuator
     *
     * @param[in] egoSpeed vehicle current speed
     * @param[in] desSpeed vehicle desired speed
     * @return the acceleration to be given to the actuator
     */
    double _cc(const MSVehicle* veh, double egoSpeed, double desSpeed) const;

    /** @brief controller for the ACC which computes the acceleration to be applied. the value needs to be passed to the actuator
     *
     * @param[in] egoSpeed vehicle current speed
     * @param[in] desSpeed vehicle desired speed
     * @param[in] gap2pred the distance to preceding vehicle
     * @param[in] headwayTime the headway time ACC should maintain
     * @return the acceleration to be given to the actuator
     */
    double _acc(const MSVehicle* veh, double egoSpeed, double predSpeed, double gap2pred, double headwayTime) const;

    /** @brief controller for the CACC which computes the acceleration to be applied. the value needs to be passed to the actuator
     *
     * @param[in] egoSpeed vehicle current speed
     * @param[in] desSpeed vehicle desired speed
     * @param[in] predAcceleration acceleration of preceding vehicle
     * @param[in] gap2pred the distance to preceding vehicle
     * @param[in] leaderSpeed the speed of the platoon leader
     * @param[in] leaderAcceleration the acceleration of the platoon leader
     * @param[in] spacing the spacing to be kept
     * @return the acceleration to be given to the actuator
     */
    double _cacc(const MSVehicle* veh, double egoSpeed, double predSpeed, double predAcceleration, double gap2pred, double leaderSpeed, double leaderAcceleration, double spacing) const;

    /** @brief controller for the Ploeg's CACC which computes the control input variation.
     * Opposed to other controllers, this method returns a value which needs to be summed
     * to the previous desired acceleration.
     *
     * @param[in] egoSpeed vehicle current speed
     * @param[in] predSpeed the speed of the front vehicle
     * @param[in] predAcceleration acceleration of preceding vehicle
     * @param[in] gap2pred the distance to preceding vehicle
     * @return the variation of desired acceleration
     */
    double _ploeg(const MSVehicle* veh, double egoSpeed, double predSpeed, double predAcceleration, double gap2pred) const;

    /** @brief controller based on consensus strategy
     *
     * @param[in] egoSpeed vehicle current speed
     * @param[in] egoPosition vehicle current position
     * @param[in] time current time
     * @return the acceleration to be given to the actuator
     */
    double _consensus(const MSVehicle* veh, double egoSpeed, Position egoPosition, double time) const;

    /** @brief computes the desired distance between vehicle i and vehicle j
     *
     * @param[in] vehicles data about all vehicles
     * @param[in] h vector of times headway
     * @param[in] i index of own vehicle
     * @param[in] j index of vehicle to compute distance from
     * @return the desired distance between vehicle i and j
     *
     */
    double d_i_j(const struct Plexe::VEHICLE_DATA* vehicles, const double h[MAX_N_CARS], int i, int j) const;

    /** @brief flatbed platoon towing model
     *
     * @param[in] egoAcceleration vehicle current acceleration
     * @param[in] egoSpeed vehicle current speed
     * @param[in] predSpeed front vehicle speed
     * @param[in] gap2pred distance to front vehicle
     * @param[in] leaderSpeed speed of leading vehicle
     */
    double _flatbed(const MSVehicle* veh, double egoAcceleration, double egoSpeed, double predSpeed,
                    double gap2pred, double leaderSpeed) const;


private:

    /// @brief the car following model which drives the car when automated cruising is disabled, i.e., the human driver
    MSCFModel* myHumanDriver;

    /// @brief The maximum deceleration that the CC can output
    const double myCcDecel;

    /// @brief The maximum acceleration that the CC can output
    const double myCcAccel;

    /// @brief the constant gap for CACC
    const double myConstantSpacing;

    /// @brief design constant for CC
    const double myKp;

    /// @brief design constant for ACC
    const double myLambda;

    /// @brief design constant for CACC
    const double myC1;

    /// @brief design constant for CACC
    const double myXi;

    /// @brief design constant for CACC
    const double myOmegaN;

    /// @brief engine time constant used for actuation lag
    const double myTau;

    /// @brief number of lanes in the highway, in the absence of on-/off-ramps. This is used
    /// to move to the correct lane even when a lane is added for on-/off-ramps
    const int myLanesCount;

    /// @brief Ploeg's CACC parameters
    const double myPloegH;
    const double myPloegKp;
    const double myPloegKd;

    /// @brief flatbed CACC parameters
    const double myFlatbedKa;
    const double myFlatbedKv;
    const double myFlatbedKp;
    const double myFlatbedH;
    const double myFlatbedD;

private:
    /// @brief Invalidated assignment operator.
    MSCFModel_CC& operator=(const MSCFModel_CC&) = delete;
};

