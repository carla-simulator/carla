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
/// @file    RealisticEngineModel.h
/// @author  Michele Segata, Antonio Saverio Valente
/// @date    4 Feb 2015
///
// A detailed engine model
/****************************************************************************/

#pragma once

#include "GenericEngineModel.h"

#include <stdint.h>

#include "VehicleEngineHandler.h"
#include "EngineParameters.h"


/**
 * This class implements a detailed engine and braking model which takes into
 * account engine characteristics, gear ratios, external forces due to air
 * drag, etc.
 * This class uses a particular notation for variables to take into account
 * both name and unit, to avoid confusion and mistakes. We write the name in
 * camel case, then an underscore, then the unit of measure. For example,
 * engine force in newton (kg * m / s^2) we use
 *
 * double engineForce_kgmps2
 *
 * so units which are multiplied are just one next to the other (kgm), "p"
 * denotes "per", while the exponent is just a number attached to the unit
 * (s2). The same notation is used for functions that compute quantities
 */
class RealisticEngineModel : public GenericEngineModel {

protected:

    EngineParameters ep;
    //current gear
    int currentGear;
    //minimum speed to be considered for horsepower computation
    double minSpeed_mps;
    //simulation sampling time
    double dt_s;
    //xml file with engine parameters
    std::string xmlFile;
    //id of the vehicle parameters to load
    std::string vehicleType;

    /**
     * Given engine rotation in rpm, tracting wheels diameter, differential
     * ratio, and gear ratio, computes vehicle speed in mps
     *
     * @param[in] rpm engine rotation speed in rpm
     * @param[in] wheelDiameter_m tracting wheels diameter in m
     * @param[in] differentialRatio differential ratio
     * @param[in] gearRatio gear ratio
     * @return vehicle speed in meters per second
     */
    double rpmToSpeed_mps(double rpm, double wheelDiameter_m,
                          double differentialRatio, double gearRatio);
    /**
     * Computes vehicle speed in mps using stored values
     *
     * @param[in] rpm engine rotation speed in rpm
     * @return vehicle speed in meters per second
     */
    double rpmToSpeed_mps(double rpm);

    /**
     * Given vehicle speed in mps, tracting wheels diameter, differential
     * ratio, and gear ratio, computes engine rotation speed in rpm
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @param[in] wheelDiameter_m tracting wheels diameter in m
     * @param[in] differentialRatio differential ratio
     * @param[in] gearRatio gear ratio
     * @return engine rotation speed in rpm
     */
    double speed_mpsToRpm(double speed_mps, double wheelDiameter_m,
                          double differentialRatio, double gearRatio);

    /**
     * Computes engine rotation speed in rpm using stored values
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @return engine rotation speed in rpm
     */
    double speed_mpsToRpm(double speed_mps);

    /**
     * Computes engine rotation speed in rpm using stored values and given
     * gear ratio
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @param[in] gearRatio gear ratio
     * @return engine rotation speed in rpm
     */
    double speed_mpsToRpm(double speed_mps, double gearRatio);

    /**
     * Given an engine mapping computes the available power (hp) given
     * current engine speed in rpm
     *
     * @param[in] rpm engine speed in rpm
     * @param[in] engineMapping engine mapping from rpm to hp
     * @return available horsepower
     */
    double rpmToPower_hp(double rpm, const struct EngineParameters::PolynomialEngineModelRpmToHp* engineMapping);

    /**
     * Computes the available power (hp) using stored values
     *
     * @param[in] rpm engine speed in rpm
     * @return available horsepower
     */
    double rpmToPower_hp(double rpm);

    /**
     * Given vehicle speed in mps, and the rpm to hp mapping, computes
     * the available power at the current speed
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @param[in] engineMapping engine mapping from rpm to hp
     * @param[in] wheelDiameter_m tracting wheels diameter in m
     * @param[in] differentialRatio differential ratio
     * @param[in] gearRatio gear ratio
     * @return available horsepower
     */
    double speed_mpsToPower_hp(double speed_mps,
                               const struct EngineParameters::PolynomialEngineModelRpmToHp* engineMapping,
                               double wheelDiameter_m, double differentialRatio,
                               double gearRatio);

    /**
     * Computes the available power at the current speed using stored values
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @return available horsepower
     */
    double speed_mpsToPower_hp(double speed_mps);

    /**
     * Given vehicle speed in mps, and the rpm to hp mapping, computes
     * the available maximum thrust at current speed
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @param[in] engineMapping engine mapping from rpm to hp
     * @param[in] wheelDiameter_m tracting wheels diameter in m
     * @param[in] differentialRatio differential ratio
     * @param[in] gearRatio gear ratio
     * @param[in] engineEfficiency engine efficiency
     * @return available horsepower
     */
    double speed_mpsToThrust_N(double speed_mps,
                               const struct EngineParameters::PolynomialEngineModelRpmToHp* engineMapping,
                               double wheelDiameter_m, double differentialRatio,
                               double gearRatio, double engineEfficiency);

    /**
     * Computes the available maximum thrust at current speed using stored
     * values
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @return available maximum thrust
     */
    double speed_mpsToThrust_N(double speed_mps);

    /**
     * Computes air drag resistance
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @param[in] cAir drag coefficient
     * @param[in] a_m2 maximum vehicle section in square meters
     * @param[in] rho_kgpm3 air density in kilograms per cubic meter
     * @return air drag force in N
     */
    double airDrag_N(double speed_mps, double cAir, double a_m2, double rho_kgpm3);

    /**
     * Computes air drag resistance using stored values
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @return air drag force in N
     */
    double airDrag_N(double speed_mps);

    /**
     * Computes rolling resistance
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @param[in] mass_kg mass of the vehicle in kg
     * @param[in] cr1 tires parameter
     * @param[in] cr2 tires parameter
     * @return rolling resistance in N
     */
    double rollingResistance_N(double speed_mps, double mass_kg, double cr1, double cr2);
    /**
     * Computes rolling resistance using stored values
     *
     * @param[in] speed_mps vehicle speed in meters per second
     * @return rolling resistance in N
     */
    double rollingResistance_N(double speed_mps);

    /**
     * Computes gravitational force due to slope
     *
     * @param[in] mass_kg vehicle mass in kg
     * @param[in] slope road slope in degrees
     * @return gravitational force in N
     */
    double gravityForce_N(double mass_kg, double slope);
    /**
     * Computes gravitational force using stored values
     *
     * @return gravitational force in N
     */
    double gravityForce_N();

    /**
     * Computes total opposing force, i.e., air drag, rolling resistance, and
     * gravity
     *
     * @param[in] speed_mps speed in meters per second
     * @param[in] mass_kg vehicle mass in kg
     * @param[in] slope road slope in degrees
     * @param[in] cAir drag coefficient
     * @param[in] a_m2 maximum vehicle section in square meters
     * @param[in] rho_kgpm3 air density in kilograms per cubic meter
     * @param[in] cr1 tires parameter
     * @param[in] cr2 tires parameter
     * @return total opposing force in N
     */
    double opposingForce_N(double speed_mps, double mass_kg, double slope,
                           double cAir, double a_m2, double rho_kgpm3,
                           double cr1, double cr2);

    /**
     * Computes total opposing force, using stored values
     *
     * @param[in] speed_mps speed in meters per second
     * @return total opposing force in N
     */
    double opposingForce_N(double speed_mps);

    /**
     * Computes maximum acceleration without slipping tires
     *
     * @param[in] slope road slope in degrees
     * @param[in] frictionCoefficient tires friction coefficient
     * @return maximum acceleration without slipping tires in meters per squared second
     */
    double maxNoSlipAcceleration_mps2(double slope, double frictionCoefficient);

    /**
     * Computes maximum acceleration without slipping tires using stored values
     *
     * @return maximum acceleration without slipping tires in meters per squared second
     */
    double maxNoSlipAcceleration_mps2();

    /**
     * Computes the acceleration for a certain thrust, using stored values of mass
     *
     * @param[in] thrust_N thrust in N
     * @return the acceleration for the vehicle mass in meters per squared second
     */
    double thrust_NToAcceleration_mps2(double thrust_N);

    /**
     * Computes current gear according to gear shifting rules
     *
     * @param[in] speed_mps current speed in meters per second
     * @param[in] acceleration_mps2 current acceleration in meters per squared second
     * @return current gear (0 based)
     */
    int performGearShifting(double speed_mps, double acceleration_mps2);

    /**
     * Computes the engine actuation time constant as function of the rpm
     *
     * @param[in] rpm engine rpm
     * @return engine time constant in seconds
     */
    double getEngineTimeConstant_s(double rpm);

    /**
     * Computes maximum vehicle acceleration. This function computes the
     * maximum acceleration at the wheel, considering tires friction but
     * without considering air friction, rolling resistance, and road slope.
     *
     * @param[in] speed_mps current speed in meters per second
     * @return the maximum engine acceleration at the wheel, as provided by the
     * engine, if no tire slip occur
     */
    double maxEngineAcceleration_mps2(double speed_mps);

    /**
     * Computes real vehicle deceleration
     *
     * @param[in] speed_mps current speed in meters per second
     * @param[in] accel_mps2 current vehicle acceleration in meters per squared second
     * @param[in] reqAccel_mps2 the requested deceleration in meters per
     * squared second. Notice that this value must be negative
     * @param[in] the real deceleration in meters per squared second. Notice
     * that the returned value is an acceleration, you should not invert it
     */
    double getRealBrakingAcceleration(double speed_mps, double accel_mps2, double reqAccel_mps2, SUMOTime t);

public:
    RealisticEngineModel();
    virtual ~RealisticEngineModel();

    /**
     * Computes real vehicle acceleration given current speed, current acceleration,
     * and requested acceleration. If the requested acceleration is negative, then
     * the braking model is invoked
     *
     * @param[in] speed_mps current speed in meters per second
     * @param[in] accel_mps2 current acceleration in meters per squared second
     * @param[in] reqAccel_mps2 requested acceleration in meters per squared second
     * @param[in] timeStep current simulation timestep. needed for debugging purposes only
     * @return the real acceleration that the vehicle applies
     */
    virtual double getRealAcceleration(double speed_mps, double accel_mps2, double reqAccel_mps2, SUMOTime timeStep = 0);

    /**
     * Load model parameters. This method requires a map of strings to be as
     * flexible as possible, independently from the actual model implementation
     *
     * @param[in] parameters a map of strings (from parameter name to parameter
     * value) including configuration parameters
     */
    virtual void loadParameters(const ParMap& parameters);

    /**
     * Load parameters from xml file
     */
    void loadParameters();

    /**
     * Sets a single parameter value
     *
     * @param[in] parameter the name of the parameter
     * @param[in] value the value for the parameter
     */
    virtual void setParameter(const std::string parameter, const std::string& value);
    virtual void setParameter(const std::string parameter, double value);
    virtual void setParameter(const std::string parameter, int value);

    /**
     * Returns current engine RPM and gear
     *
     * @param[in] speed_mps current speed
     * @param[out] gear current gear
     * @param[out] rpm engine's rpm
     */
    void getEngineData(double speed_mps, int& gear, double& rpm);

#ifdef EE
private:
    //some things for the easter egg :)
    bool               initee;
    int                lastTimeStep;
    struct sockaddr_in serv_addr;
    int                socketfd;
#endif
};

