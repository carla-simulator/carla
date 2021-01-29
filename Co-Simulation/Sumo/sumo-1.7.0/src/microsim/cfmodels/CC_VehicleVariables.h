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
/// @file    CC_VehicleVariables.h
/// @author  Michele Segata
/// @date    Mon, 7 Mar 2016
///
/****************************************************************************/
#pragma once
#include <config.h>

#include "CC_Const.h"
#include <microsim/cfmodels/MSCFModel.h>
#include <utils/geom/Position.h>
#include <string.h>
#include <string>
#include <map>

#include <microsim/engine/GenericEngineModel.h>
#include <microsim/engine/FirstOrderLagModel.h>
#include <microsim/engine/RealisticEngineModel.h>

class CC_VehicleVariables : public MSCFModel::VehicleVariables {
public:

    /**
     * @struct FAKE_CONTROLLER_DATA
     * @brief represent the set of fake data which is sent to the controller in
     * order to automatically make the car move to a precise position before
     * joining the platoon.
     * we expect to get from the upper application the data that the CACC needs, i.e.:
     * - front distance, front speed and front vehicle acceleration: this information
     *   regards the car that the vehicle joining the platoon will have directly in
     *   front. this data might be real or might be fake: for example, if the platoon
     *   management algorithm decides to set the vehicle as the new leader, there won't
     *   be a car in front, and the fake data will be used only for positioning. in the
     *   case of fake data, acceleration must be set to 0
     * - leader front speed and acceleration: this information is the same as previously
     *   described for vehicle in front, but regards the leader. again, if the vehicle
     *   is being set as the new leader, this data might be fake data
     */
    struct FAKE_CONTROLLER_DATA {
        double frontDistance;
        double frontSpeed;
        double frontAcceleration;
        double frontControllerAcceleration;
        double leaderSpeed;
        double leaderAcceleration;
        double leaderControllerAcceleration;
    };

    /**
     * Topology matrix L for the consensus controller
     */
    const static int defaultL[MAX_N_CARS][MAX_N_CARS];

    /**
     * Gains matrix K for the consensus controller
     */
    const static double defaultK[MAX_N_CARS][MAX_N_CARS];

    /**
     * Default damping ratios vector b for the consensus controller
     */
    const static double defaultB[];

    /**
     * Default time headways vector h for the consensus controller
     */
    const static double defaultH[];

    CC_VehicleVariables();
    ~CC_VehicleVariables();

    /// @brief acceleration as computed by the controller, to be sent to other vehicles
    double controllerAcceleration;

    /// @brief current front vehicle speed
    double frontSpeed;
    /// @brief current front vehicle acceleration (used by CACC)
    double frontAcceleration;
    /// @brief front vehicle controller acceleration (used by CACC)
    double frontControllerAcceleration;
    /// @brief current front vehicle position
    Position frontPosition;
    /// @brief when front vehicle data has been readed from GPS
    double frontDataReadTime;
    /// @brief front vehicle velocity vector
    Position frontVelocity;
    /// @brief front vehicle angle in radians
    double frontAngle;
    /// @did we receive at least one packet?
    bool frontInitialized;

    /// @brief determines whether CACC should automatically fetch data about other vehicles
    bool autoFeed;
    /// @brief leader vehicle, used for auto feeding
    MSVehicle* leaderVehicle;
    /// @brief front sumo id, used for auto feeding
    MSVehicle* frontVehicle;

    /// @brief headway time for ACC
    double accHeadwayTime;
    double accLambda;

    /// @brief determines whether PATH's CACC should use the real vehicle
    /// acceleration or the controller computed one
    bool useControllerAcceleration;
    /// @brief platoon's leader speed (used by CACC)
    double leaderSpeed;
    /// @brief platoon's leader acceleration (used by CACC)
    double leaderAcceleration;
    /// @brief platoon's leader controller acceleration (used by CACC)
    double leaderControllerAcceleration;
    /// @brief platoon's leader position
    Position leaderPosition;
    /// @brief when leader data has been readed from GPS
    double leaderDataReadTime;
    /// @brief platoon's leader velocity vector
    Position leaderVelocity;
    /// @brief platoon's leader angle in radians
    double leaderAngle;
    /// @did we receive at least one packet?
    bool leaderInitialized;
    bool caccInitialized;

    //enable/disable the use of a constant, user defined acceleration instead of the one computed by the controller
    int useFixedAcceleration;
    //fixed acceleration to use
    double fixedAcceleration;

    //car collided in the last timestep
    bool crashed;

    /// @brief CC desired speed
    double ccDesiredSpeed;
    double ccKp;
    /// @brief currently active controller
    enum Plexe::ACTIVE_CONTROLLER activeController;

    /// @brief fake controller data. @see FAKE_CONTROLLER_DATA
    struct FAKE_CONTROLLER_DATA fakeData;

    /// @brief L matrix
    int L[MAX_N_CARS][MAX_N_CARS];
    /// @brief K matrix
    double K[MAX_N_CARS][MAX_N_CARS];
    /// @brief vector of damping ratios b
    double b[MAX_N_CARS];
    /// @brief vector of time headways h
    double h[MAX_N_CARS];

    /// @brief data about vehicles in the platoon
    struct Plexe::VEHICLE_DATA vehicles[MAX_N_CARS];
    /// @brief tells whether data about a certain vehicle has been initialized
    bool initialized[MAX_N_CARS];
    /// @brief count of initialized vehicles
    int nInitialized;
    /// @brief my position within the platoon (0 = first car)
    int position;
    /// @brief number of cars in the platoon
    int nCars;

    /// @brief controller related parameters
    double caccXi;
    double caccOmegaN;
    double caccC1;
    double caccAlpha1, caccAlpha2, caccAlpha3, caccAlpha4, caccAlpha5;
    /// @brief fixed spacing for CACC
    double caccSpacing;
    double engineTau;
    /// @brief limits for u
    double uMin, uMax;
    double ploegH;
    double ploegKp;
    double ploegKd;
    double flatbedKa;
    double flatbedKv;
    double flatbedKp;
    double flatbedD;
    double flatbedH;

    /// @brief engine model employed by this car
    GenericEngineModel* engine;
    /// @brief numeric value indicating the employed model
    int engineModel;

    /// @brief enable/disable data prediction (interpolation) for missing data
    bool usePrediction;

    /// @brief list of members belonging to my platoon
    std::map<int, std::string> members;

    /// @brief automatic whole platoon lane change
    bool autoLaneChange;
};

