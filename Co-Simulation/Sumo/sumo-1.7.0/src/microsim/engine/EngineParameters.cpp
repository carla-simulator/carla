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
/// @file    EngineParameters.cpp
/// @author  Michele Segata
/// @date    4 Feb 2015
///
/****************************************************************************/

#include <config.h>

#include <cmath>
#include <utils/geom/GeomHelper.h> // for M_PI
#include "EngineParameters.h"


EngineParameters::EngineParameters() : nGears(5), differentialRatio(3.714), wheelDiameter_m(0.94),
    mass_kg(1300), cAir(0.3), a_m2(2.7), rho_kgpm3(1.2), cr1(0.0136), cr2(5.18e-7),
    slope(0.), tiresFrictionCoefficient(0.7), engineEfficiency(0.8),
    massFactor(1.089), cylinders(4), dt(0.01), minRpm(1000), maxRpm(7000),
    brakesTau_s(0.2), tauEx_s(0.1), tauBurn_s(-1), fixedTauBurn(false) {
    id = "";
    //set default gear ratios
    gearRatios = new double[nGears];
    gearRatios[0] = 3.909;//4.5;
    gearRatios[1] = 2.238;//3.5;//2.6;
    gearRatios[2] = 1.520;//2.5;//1.5;
    gearRatios[3] = 1.156;//1.5;//1.0;
    gearRatios[4] = 0.971;//1.0;//0.8;
    //default engine mapping
    engineMapping.degree = 1;
    engineMapping.x[0] = -7.50084;
    engineMapping.x[1] = 0.02045;
    //default shifting rule
    shiftingRule.rpm = 6000;
    shiftingRule.deltaRpm = 100;
    //initialize precomputed coefficients
    computeCoefficients();

}

EngineParameters::EngineParameters(const EngineParameters& other) :
    nGears(other.nGears), differentialRatio(other.differentialRatio), wheelDiameter_m(other.wheelDiameter_m),
    mass_kg(other.mass_kg), cAir(other.cAir), a_m2(other.a_m2), rho_kgpm3(other.rho_kgpm3), cr1(other.cr1), cr2(other.cr2),
    slope(other.slope), tiresFrictionCoefficient(other.tiresFrictionCoefficient), engineEfficiency(other.engineEfficiency),
    massFactor(other.massFactor), cylinders(other.cylinders), dt(other.dt), minRpm(other.minRpm), maxRpm(other.maxRpm),
    brakesTau_s(other.brakesTau_s), tauEx_s(other.tauEx_s), tauBurn_s(other.tauBurn_s), fixedTauBurn(other.fixedTauBurn) {
    id = other.id;
    gearRatios = new double[nGears];
    for (int i = 0; i < nGears; i++) {
        gearRatios[i] = other.gearRatios[i];
    }
    engineMapping.degree = other.engineMapping.degree;
    for (int i = 0; i < engineMapping.degree; i++) {
        engineMapping.x[i] = other.engineMapping.x[i];
    }
    shiftingRule.rpm = other.shiftingRule.rpm;
    shiftingRule.deltaRpm = other.shiftingRule.deltaRpm;
    computeCoefficients();
}
EngineParameters& EngineParameters::operator =(const EngineParameters& other) {
    id = other.id;
    nGears = other.nGears;
    differentialRatio = other.differentialRatio;
    wheelDiameter_m = other.wheelDiameter_m;
    mass_kg = other.mass_kg;
    cAir = other.cAir;
    a_m2 = other.a_m2;
    rho_kgpm3 = other.rho_kgpm3;
    cr1 = other.cr1;
    cr2 = other.cr2;
    slope = other.slope;
    tiresFrictionCoefficient = other.tiresFrictionCoefficient;
    engineEfficiency = other.engineEfficiency;
    massFactor  = other.massFactor;
    cylinders = other.cylinders;
    dt = other.dt;
    minRpm = other.minRpm;
    maxRpm = other.maxRpm;
    delete [] gearRatios;
    gearRatios = new double[nGears];
    for (int i = 0; i < nGears; i++) {
        gearRatios[i] = other.gearRatios[i];
    }
    engineMapping.degree = other.engineMapping.degree;
    for (int i = 0; i < engineMapping.degree; i++) {
        engineMapping.x[i] = other.engineMapping.x[i];
    }
    shiftingRule.rpm = other.shiftingRule.rpm;
    shiftingRule.deltaRpm = other.shiftingRule.deltaRpm;
    brakesTau_s = other.brakesTau_s;
    tauBurn_s = other.tauBurn_s;
    tauEx_s = other.tauEx_s;
    fixedTauBurn = other.fixedTauBurn;
    computeCoefficients();
    return *this;
}

EngineParameters::~EngineParameters() {
    delete [] gearRatios;
}

void EngineParameters::computeCoefficients() {
    __airFrictionCoefficient = 0.5 * cAir * a_m2 * rho_kgpm3;
    __cr1 = mass_kg * massFactor * GRAVITY_MPS2 * cr1;
    __cr2 = mass_kg * massFactor * GRAVITY_MPS2 * cr2;
    __gravity = mass_kg * massFactor * GRAVITY_MPS2 * sin(slope / 180 * M_PI);
    __maxNoSlipAcceleration = tiresFrictionCoefficient * GRAVITY_MPS2 * cos(slope / 180 * M_PI);
    __rpmToSpeedCoefficient = wheelDiameter_m * M_PI / (differentialRatio * 60);
    __speedToRpmCoefficient = differentialRatio * 60 / (wheelDiameter_m * M_PI);
    __speedToThrustCoefficient = engineEfficiency * HP_TO_W;
    __maxAccelerationCoefficient = mass_kg * massFactor;
    __brakesAlpha = dt / (brakesTau_s + dt);
    __brakesOneMinusAlpha = 1 - __brakesAlpha;
    __engineTau1 = (420.0 * cylinders - 240.0) / (2.0 * cylinders);
    __engineTau2 = (120.0 * cylinders - 120.0) / cylinders;
    __engineTauDe_s = tauBurn_s + tauEx_s;
}

void EngineParameters::dumpParameters(std::ostream& out) {
    out << "ID: " << id.c_str() << std::endl;

    out << "Gearbox:\n";
    out << "\tGears number: " << (int)nGears << std::endl;
    for (int i = 0; i < nGears; i++) {
        out << std::setprecision(4) << "\tRatio of gear " << (i + 1) << ": " << gearRatios[i] << std::endl;
    }
    out << std::setprecision(4) << "\tFinal drive ratio: " << differentialRatio << std::endl;

    out << "Wheels:\n";
    out << std::setprecision(3) << "\tDiameter: " << wheelDiameter_m << " m\n";
    out << std::setprecision(3) << "\tFriction coefficient: " << tiresFrictionCoefficient << std::endl;
    out << std::setprecision(10) << "\tcr1: " << cr1 << std::endl;
    out << std::setprecision(10) << "\tcr2: " << cr2 << std::endl;

    out << "Mass:\n";
    out << std::setprecision(2) << "\tMass: " << mass_kg << " kg\n";
    out << std::setprecision(4) << "\tMass factor: " << massFactor << std::endl;

    out << "Air drag:\n";
    out << std::setprecision(4) << "\tDrag coefficient: " << cAir << std::endl;
    out << std::setprecision(3) << "\tMax section: " << a_m2 << " m^2\n";

    out << "Engine:\n";
    out << "\tEfficiency: " << engineEfficiency << std::endl;
    out << "\tCylinders: " << cylinders << std::endl;
    out << "\tMinimum rpm: " << minRpm << std::endl;
    out << "\tMaximum rpm: " << maxRpm << std::endl;
    out << "\tMapping (rpm to hp) degree: " << engineMapping.degree << std::endl;
    for (int i = 0; i < engineMapping.degree; i++) {
        out << "\t\tMapping coefficient x" << i << ": " << engineMapping.x[i] << std::endl;
    }
    out << "\tShifting rpm: " << shiftingRule.rpm << std::endl;
    out << "\tShifting delta: " << shiftingRule.deltaRpm << std::endl;

    out << "Brakes:\n";
    out << "\tTime constant (s): " << brakesTau_s << std::endl;

    out << "Vehicle unrelated parameters:\n";
    out << std::setprecision(4) << "\tAir density: " << rho_kgpm3 << " kg/m^3\n";
    out << "\tRoad slope: " << slope << " degrees\n";
    out << std::setprecision(3) << "\tSimulation sampling time: " << dt << " s\n";
}
