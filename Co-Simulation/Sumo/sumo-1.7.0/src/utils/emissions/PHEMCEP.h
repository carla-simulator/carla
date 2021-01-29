/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    PHEMCEP.h
/// @author  Nikolaus Furian
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Marek Heinrich
/// @date    Thu, 13.06.2013
///
// Helper class for PHEM Light, holds a specific CEP for a PHEM emission class
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/StringBijection.h>
#include "PHEMCEP.h"
#include "PHEMConstants.h"



// ===========================================================================
// enumerations
// ===========================================================================
enum NormalizingType {
    RatedPower,
    DrivingPower
};


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PHEMCEP
 * @brief Data Handler for a single CEP emission data set
 */
class PHEMCEP {
public:
    /*** @brief Constructor
     * @param[in] emissionClass PHEM emission class of vehicle
     * @param[in] vehicleMass vehicle mass
     * @param[in] vehicleLoading vehicle loading
     * @param[in] vehicleMassRot rotational mass of vehicle
     * @param[in] crossArea crosssectional area of vehicle
     * @param[in] cWValue cw-value
     * @param[in] f0 Rolling resistance f0
     * @param[in] f1 Rolling resistance f1
     * @param[in] f2 Rolling resistance f2
     * @param[in] f3 Rolling resistance f3
     * @param[in] f4 Rolling resistance f4
     * @param[in] ratedPower rated power of vehicle
     * @param[in] pNormV0 out variable for step function to get maximum normalized rated power over speed
     * @param[in] pNormP0 out variable for step function to get maximum normalized rated power over speed
     * @param[in] pNormV1 out variable for step function to get maximum normalized rated power over speed
     * @param[in] pNormP1 out variable for step function to get maximum normalized rated power over speed
     * @param[in] vehicleFuelType out variable for fuel type (D, G) of vehicle, needed for density of fuel
     * @param[in] matrixFC Coefficients of the fuel consumption
     * @param[in] headerLine Definition of covered pollutants
     * @param[in] headerLinePollutants Coefficients of the pollutants
     * @param[in] matrixPollutants Coefficients of the pollutants
     * @param[in] matrixSpeedRotational Table for rotational coefficients over speed
     */
    PHEMCEP(bool heavyVehicel, SUMOEmissionClass emissionClass, const std::string& emissionClassIdentifier,
            double vehicleMass, double vehicleLoading, double vehicleMassRot,
            double crossArea, double cdValue,
            double f0, double f1, double f2, double f3, double f4,
            double ratedPower, double pNormV0, double pNormP0, double pNormV1, double pNormP1,
            double axleRatio, double engineIdlingSpeed, double engineRatedSpeed, double effectiveWheelDiameter,
            double idlingFC,
            const std::string& vehicleFuelType,
            const std::vector< std::vector<double> >& matrixFC,
            const std::vector<std::string>& headerLinePollutants,
            const std::vector< std::vector<double> >& matrixPollutants,
            const std::vector< std::vector<double> >& matrixSpeedRotational,
            const std::vector< std::vector<double> >& normedDragTable,
            const std::vector<double>& idlingValuesPollutants);

    /// @brief Destructor
    ~PHEMCEP();

    /** @brief Returns the power of used for a vehicle at state v,a, slope and loading
         * @param[in] v The vehicle's average velocity
         * @param[in] a The vehicle's average acceleration
         * @param[in] slope The road's slope at vehicle's position [deg]
    	 * @param{in] vehicleCep vehicles CEP data
         * @param{in] loading vehicle loading [kg]
         * @return The power demand for desired state [kW]
         */
    double CalcPower(double v, double a, double slope, double vehicleLoading = 0) const;


    /**	 @brief Returns the maximum accelaration for a vehicle at state v,a, slope and loading
         * @param[in] v The vehicle's average velocity
         * @param[in] a The vehicle's average acceleration
         * @param[in] slope The road's slope at vehicle's position [deg]
    	 * @param{in] vehicleCep vehicles CEP data
         * @param{in] loading vehicle loading [kg]
         * @return The maximum accelaration for desired state [kW]
         */
    double GetMaxAccel(double v, double a, double gradient, double vehicleLoading = 0) const;

    /** @brief Returns a emission measure for power[kW] level
     * @param[in] pollutantIdentifier Desired pollutant, e.g. NOx
     * @param[in] power in [kW]
     * @return emission in [g/h]
     */
    double GetEmission(const std::string& pollutantIdentifier, double power, double speed, bool normalized = false) const;
    double GetDecelCoast(double speed, double acc, double gradient, double vehicleLoading) const;


    /** @brief Getter function to recieve vehicle data from CEP
     * @return PHEM emission class of vehicle
     */
    SUMOEmissionClass GetEmissionClass() const {
        return _emissionClass;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return Rolling resistance f0
     */
    double GetResistanceF0() const {
        return _resistanceF0;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return Rolling resistance f1
     */
    double GetResistanceF1() const {
        return _resistanceF1;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return Rolling resistance f2
     */
    double GetResistanceF2() const {
        return _resistanceF2;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return Rolling resistance f3
     */
    double GetResistanceF3() const {
        return _resistanceF3;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return Rolling resistance f4
     */
    double GetResistanceF4() const {
        return _resistanceF4;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return Cw value
     * @todo: Why is it named "cdValue", here?
     */
    double GetCdValue() const {
        return _cdValue;
    }

    /** @brief Getter function to recieve vehicle data from CEP
     * @return crosssectional area of vehicle
     */
    double GetCrossSectionalArea() const {
        return _crossSectionalArea;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return vehicle mass
     */
    double GetMassVehicle() const {
        return _massVehicle;
    }

    /** @brief Getter function to recieve vehicle data from CEP
     * @return vehicle loading
     */
    double GetVehicleLoading() const {
        return _vehicleLoading;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return rotational mass of vehicle
     */
    double GetMassRot() const {
        return _massRot;
    }


    /** @brief Getter function to recieve vehicle data from CEP
     * @return rated power of vehicle
     */
    double GetRatedPower() const {
        return _ratedPower;
    }

    /** @brief Getter function to recieve vehicle data from CEP
     * @return fuel type of vehicle
     */
    const std::string& GetVehicleFuelType() const {
        return _vehicleFuelType;
    }

private:
    /** @brief Interpolates emission linearly between two known power-emission pairs
     * @param[in] px power-value to interpolate
     * @param[in] p1 first known power value
     * @param[in] p2 second known power value
     * @param[in] e1 emission value for p1
     * @param[in] e2 emission value for p2
     * @return emission value for px
     */
    double Interpolate(double px, double p1, double p2, double e1, double e2) const;

    /** @brief Finds bounding upper and lower index in pattern for value
     * @param[out] lowerIndex out variable for lower index
     * @param[out] upperIndex out variable for lower index
     * @param[in] pattern to search
     * @param[in] value to search
     */
    void FindLowerUpperInPattern(int& lowerIndex, int& upperIndex, const std::vector<double>& pattern, double value) const;

    /** @brief Calculates rotational index for speed
     * @param[in] speed desired speed
     */
    double GetRotationalCoeffecient(double speed) const;
    double GetGearCoeffecient(double speed) const;
    double GetDragCoeffecient(double nNorm) const;

    /** @brief Calculates maximum available rated power for speed
     * @param[in] speed desired speed
     */
    double GetPMaxNorm(double speed) const;

private:
    /// @brief PHEM emission class of vehicle
    SUMOEmissionClass _emissionClass;
    NormalizingType _normalizingType;
    /// @brief Rolling resistance f0
    double _resistanceF0;
    /// @brief Rolling resistance f1
    double _resistanceF1;
    /// @brief Rolling resistance f2
    double _resistanceF2;
    /// @brief Rolling resistance f3
    double _resistanceF3;
    /// @brief Rolling resistance f4
    double _resistanceF4;
    /// @brief Cw value
    double _cdValue;
    /// @brief crosssectional area of vehicle
    double _crossSectionalArea;
    /// @brief vehicle mass
    double _massVehicle;
    /// @brief vehicle loading
    double _vehicleLoading;
    /// @brief rotational mass of vehicle
    double _massRot;
    /// @brief rated power of vehicle
    double _ratedPower;
    /// @brief Step functions parameter for maximum rated power
    double _pNormV0;
    /// @brief Step functions parameter for maximum rated power
    double _pNormP0;
    /// @brief Step functions parameter for maximum rated power
    double _pNormV1;
    /// @brief Step functions parameter for maximum rated power
    double _pNormP1;
    double _axleRatio;
    double _engineIdlingSpeed;
    double _engineRatedSpeed;
    double _effictiveWheelDiameter;
    double _idlingFC;
    std::string _vehicleFuelType;
    int _sizeOfPatternFC;
    /// @todo describe
    int _sizeOfPatternPollutants;
    double _normalizingPower;
    double _drivingPower;
    bool _heavyVehicle;
    std::vector<double> _speedPatternRotational;
    /// @todo describe
    std::vector<double> _powerPatternFC;
    /// @todo describe
    std::vector<double> _powerPatternPollutants;
    std::vector<double> _normalizedPowerPatternFC;
    std::vector<double> _normailzedPowerPatternPollutants;
    /// @todo describe
    std::vector<double> _cepCurveFC;
    /// @todo describe
    std::vector<double> _normedCepCurveFC;
    std::vector<double> _speedCurveRotational;
    std::vector<double> _gearTransmissionCurve;
    std::vector<double> _nNormTable;
    std::vector<double> _dragNormTable;
    StringBijection< std::vector<double> > _cepCurvePollutants;
    StringBijection<std::vector<double> > _normalizedCepCurvePollutants;
    StringBijection<double> _idlingValuesPollutants;

};
