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
/// @file    CC_VehicleVariables.cpp
/// @author  Michele Segata
/// @date    Mon, 7 Mar 2016
///
/****************************************************************************/
#include "CC_VehicleVariables.h"

//initialize default L and K matrices
const int CC_VehicleVariables::defaultL[][MAX_N_CARS] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0},
    {1, 0, 1, 0, 0, 0, 0, 0},
    {1, 0, 0, 1, 0, 0, 0, 0},
    {1, 0, 0, 0, 1, 0, 0, 0},
    {1, 0, 0, 0, 0, 1, 0, 0},
    {1, 0, 0, 0, 0, 0, 1, 0}
};
const double CC_VehicleVariables::defaultK[][MAX_N_CARS] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {460, 0, 0, 0, 0, 0, 0, 0},
    {80, 860, 0, 0, 0, 0, 0, 0},
    {80, 0, 860, 0, 0, 0, 0, 0},
    {80, 0, 0, 860, 0, 0, 0, 0},
    {80, 0, 0, 0, 860, 0, 0, 0},
    {80, 0, 0, 0, 0, 860, 0, 0},
    {80, 0, 0, 0, 0, 0, 860, 0}
};
const double CC_VehicleVariables::defaultB[MAX_N_CARS] = {1800, 1800, 1800, 1800, 1800, 1800, 1800, 1800};
const double CC_VehicleVariables::defaultH[MAX_N_CARS] = {0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8};

CC_VehicleVariables::CC_VehicleVariables() :
    controllerAcceleration(0), frontSpeed(0), frontAcceleration(0),
    frontControllerAcceleration(0), frontDataReadTime(0), frontAngle(0), frontInitialized(false),
    autoFeed(false), leaderVehicle(0), frontVehicle(0),
    accHeadwayTime(1.5), accLambda(0.1),
    useControllerAcceleration(true), leaderSpeed(0),
    leaderAcceleration(0), leaderControllerAcceleration(0), leaderDataReadTime(0), leaderAngle(0),
    leaderInitialized(false), caccInitialized(false),
    useFixedAcceleration(0), fixedAcceleration(0),
    crashed(false),
    ccDesiredSpeed(14), ccKp(1), activeController(Plexe::DRIVER),
    nInitialized(0), position(-1), nCars(8),
    caccXi(-1), caccOmegaN(-1), caccC1(-1), caccAlpha1(-1), caccAlpha2(-1),
    caccAlpha3(-1), caccAlpha4(-1), caccAlpha5(-1), caccSpacing(5),
    engineTau(0.5),
    uMin(-1e6), uMax(1e6),
    ploegH(0.5), ploegKp(0.2), ploegKd(0.7),
    flatbedKa(2.4), flatbedKv(0.6), flatbedKp(12), flatbedD(5), flatbedH(4),
    engine(0), engineModel(CC_ENGINE_MODEL_FOLM),
    usePrediction(false),
    autoLaneChange(false) {
    fakeData.frontAcceleration = 0;
    fakeData.frontControllerAcceleration = 0;
    fakeData.frontDistance = 0;
    fakeData.frontSpeed = 0;
    fakeData.leaderAcceleration = 0;
    fakeData.leaderControllerAcceleration = 0;
    fakeData.leaderSpeed = 0;
    leaderPosition.set(0, 0);
    frontPosition.set(0, 0);
    //init L, K, b, and h with default values
    memcpy(L, defaultL, sizeof(int)*MAX_N_CARS * MAX_N_CARS);
    memcpy(K, defaultK, sizeof(double)*MAX_N_CARS * MAX_N_CARS);
    memcpy(b, defaultB, sizeof(double)*MAX_N_CARS);
    memcpy(h, defaultH, sizeof(double)*MAX_N_CARS);
    //no data about any vehicle has been set
    for (int i = 0; i < MAX_N_CARS; i++) {
        initialized[i] = false;
    }
}

CC_VehicleVariables::~CC_VehicleVariables() {
    if (engine) {
        delete engine;
    }
}
