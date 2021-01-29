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
/// @file    EngineParameters.h
/// @author  Michele Segata
/// @date    4 Feb 2015
///
/****************************************************************************/

#pragma once

#include <iostream>
#include <iomanip>

//gravity force
#define GRAVITY_MPS2    9.81
//horsepower to watts conversion factor
#define HP_TO_W         745.699872
//maximum engine time constant
#define TAU_MAX         0.5
//maximum degree for the engine polynomial
#define MAX_POLY_DEGREE 9

class EngineParameters {

public:

    /**
     * Defines the polynomial coefficients of the engine model mapping engine
     * speed to horsepower.
     * The model is of type
     * power(rpm) [hp] = x0 + x1*hp + x2*hp^2 + ...
     */
    struct PolynomialEngineModelRpmToHp {
        int degree;
        double x[MAX_POLY_DEGREE];
    };

    /**
     * Defines the rules for gear shifting. The structure contains the rpm
     * at which shifting should occur and the delta used for up and down
     * shifting. So to shift up, the rpm should be at least rpm + delta,
     * while to shift down the should be lower than rpm - delta. this
     * is needed to avoid oscillations when running aroud rpm
     */
    struct GearShiftingRules {
        double rpm;
        double deltaRpm;
    };

    //id of the type of vehicle
    std::string id;
    //number of gears
    int nGears;
    //gear ratios
    double* gearRatios;
    //differential ratio
    double differentialRatio;
    //wheel diameter in meters
    double wheelDiameter_m;
    //mass of vehicle
    double mass_kg;
    //air friction parameters
    double cAir, a_m2, rho_kgpm3;
    //precomputed air friction coefficient
    double __airFrictionCoefficient;
    //tires parameters
    double cr1, cr2;
    //precomputed rolling resistance parameters
    double __cr1, __cr2;
    //road slope in degrees
    double slope;
    //precomputed gravitational force
    double __gravity;
    //tires friction coefficient
    double tiresFrictionCoefficient;
    //precomputed maximum acceleration with no slip
    double __maxNoSlipAcceleration;
    //engine mapping
    struct PolynomialEngineModelRpmToHp engineMapping;
    //gear shifting rule
    struct GearShiftingRules shiftingRule;
    //engine efficiency
    double engineEfficiency;
    //factor taking into account inertia of rotating masses, such as flywheel
    double massFactor;
    //number of cylinders of the engine
    int cylinders;
    //simulation timestep
    double dt;
    //min and max rpm
    double minRpm, maxRpm;
    //brakes actuation time constant in second
    double brakesTau_s;
    //engine time constants for exhaust and burning
    double tauEx_s, tauBurn_s;
    //whether tauBurn_s is fixed or dynamic
    bool fixedTauBurn;
    //other precomputed coefficients
    double __rpmToSpeedCoefficient;
    double __speedToRpmCoefficient;
    double __speedToThrustCoefficient;
    double __maxAccelerationCoefficient;
    double __engineTau1;
    double __engineTau2;
    double __engineTauDe_s;
    double __brakesAlpha;
    double __brakesOneMinusAlpha;
    //todo: finish
    double minSpeed;

    /**
     * Computes the precomputed coefficients used to speedup the model
     */
    void computeCoefficients();

    /**
     * Writes the parameter to an output stream, for debugging purposes
     */
    void dumpParameters(std::ostream& out);

    EngineParameters();
    EngineParameters(const EngineParameters& other);
    EngineParameters& operator = (const EngineParameters& other);
    virtual ~EngineParameters();

};

