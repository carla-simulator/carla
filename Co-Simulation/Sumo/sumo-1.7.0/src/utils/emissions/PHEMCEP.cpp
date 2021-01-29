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
/// @file    PHEMCEP.cpp
/// @author  Nikolaus Furian
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Marek Heinrich
/// @date    Thu, 13.06.2013
///
// Helper class for PHEM Light, holds a specific CEP for a PHEM emission class
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <string>
#include <utils/common/StdDefs.h>
#include <utils/common/StringBijection.h>
#include <utils/common/UtilExceptions.h>
#include "PHEMCEP.h"


// ===========================================================================
// method definitions
// ===========================================================================
PHEMCEP::PHEMCEP(bool heavyVehicle, SUMOEmissionClass emissionClass, const std::string& emissionClassIdentifier,
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
                 const std::vector<double>& idlingValuesPollutants) {
    _emissionClass = emissionClass;
    _resistanceF0 = f0;
    _resistanceF1 = f1;
    _resistanceF2 = f2;
    _resistanceF3 = f3;
    _resistanceF4 = f4;
    _cdValue = cdValue;
    _crossSectionalArea = crossArea;
    _massVehicle = vehicleMass;
    _vehicleLoading = vehicleLoading;
    _massRot = vehicleMassRot;
    _ratedPower = ratedPower;
    _vehicleFuelType = vehicleFuelType;

    _pNormV0 = pNormV0 / 3.6;
    _pNormP0 = pNormP0;
    _pNormV1 = pNormV1 / 3.6;
    _pNormP1 = pNormP1;

    _axleRatio = axleRatio;
    _engineIdlingSpeed = engineIdlingSpeed;
    _engineRatedSpeed = engineRatedSpeed;
    _effictiveWheelDiameter = effectiveWheelDiameter;

    _heavyVehicle = heavyVehicle;
    _idlingFC = idlingFC;

    std::vector<std::string> pollutantIdentifier;
    std::vector< std::vector<double> > pollutantMeasures;
    std::vector<std::vector<double> > normalizedPollutantMeasures;

    // init pollutant identifiers
    for (int i = 0; i < (int)headerLinePollutants.size(); i++) {
        pollutantIdentifier.push_back(headerLinePollutants[i]);
    } // end for

    // get size of powerPatterns
    _sizeOfPatternFC = (int)matrixFC.size();
    _sizeOfPatternPollutants = (int)matrixPollutants.size();

    // initialize measures
    for (int i = 0; i < (int)headerLinePollutants.size(); i++) {
        pollutantMeasures.push_back(std::vector<double>());
        normalizedPollutantMeasures.push_back(std::vector<double>());
    } // end for

    // looping through matrix and assigning values for speed rotational table
    _speedCurveRotational.clear();
    _speedPatternRotational.clear();
    _gearTransmissionCurve.clear();
    for (int i = 0; i < (int)matrixSpeedRotational.size(); i++) {
        if (matrixSpeedRotational[i].size() != 3) {
            throw InvalidArgument("Error loading vehicle file for: " + emissionClassIdentifier);
        }

        _speedPatternRotational.push_back(matrixSpeedRotational[i][0] / 3.6);
        _speedCurveRotational.push_back(matrixSpeedRotational[i][1]);
        _gearTransmissionCurve.push_back(matrixSpeedRotational[i][2]);
    } // end for

    // looping through matrix and assigning values for drag table
    _nNormTable.clear();
    _dragNormTable.clear();
    for (int i = 0; i < (int) normedDragTable.size(); i++) {
        if (normedDragTable[i].size() != 2) {
            return;
        }

        _nNormTable.push_back(normedDragTable[i][0]);
        _dragNormTable.push_back(normedDragTable[i][1]);

    } // end for

    // looping through matrix and assigning values for Fuel consumption
    _cepCurveFC.clear();
    _powerPatternFC.clear();
    _normalizedPowerPatternFC.clear();
    _normedCepCurveFC.clear();
    for (int i = 0; i < (int)matrixFC.size(); i++) {
        if (matrixFC[i].size() != 2) {
            throw InvalidArgument("Error loading vehicle file for: " + emissionClassIdentifier);
        }

        _powerPatternFC.push_back(matrixFC[i][0] * _ratedPower);
        _normalizedPowerPatternFC.push_back(matrixFC[i][0]);
        _cepCurveFC.push_back(matrixFC[i][1] * _ratedPower);
        _normedCepCurveFC.push_back(matrixFC[i][1]);

    } // end for

    _powerPatternPollutants.clear();
    double pollutantMultiplyer = 1;

    _drivingPower = _normalizingPower = CalcPower(NORMALIZING_SPEED, NORMALIZING_ACCELARATION, 0, vehicleLoading);

    // looping through matrix and assigning values for pollutants

    if (heavyVehicle) {
        _normalizingPower = _ratedPower;
        pollutantMultiplyer = _ratedPower;
        _normalizingType = RatedPower;
    } else {
        _normalizingPower = _drivingPower;
        _normalizingType = DrivingPower;
    } // end if

    const int headerCount = (int)headerLinePollutants.size();
    for (int i = 0; i < (int)matrixPollutants.size(); i++) {
        for (int j = 0; j < (int)matrixPollutants[i].size(); j++) {
            if ((int)matrixPollutants[i].size() != headerCount + 1) {
                return;
            }

            if (j == 0) {
                _normailzedPowerPatternPollutants.push_back(matrixPollutants[i][j]);
                _powerPatternPollutants.push_back(matrixPollutants[i][j] * _normalizingPower);
            } else {
                pollutantMeasures[j - 1].push_back(matrixPollutants[i][j] * pollutantMultiplyer);
                normalizedPollutantMeasures[j - 1].push_back(matrixPollutants[i][j]);
            } // end if
        } // end for
    } // end for

    for (int i = 0; i < (int) headerLinePollutants.size(); i++) {
        _cepCurvePollutants.insert(pollutantIdentifier[i], pollutantMeasures[i]);
        _normalizedCepCurvePollutants.insert(pollutantIdentifier[i], normalizedPollutantMeasures[i]);
        _idlingValuesPollutants.insert(pollutantIdentifier[i], idlingValuesPollutants[i] * pollutantMultiplyer);
    } // end for

    _idlingFC = idlingFC * _ratedPower;

} // end of Cep


PHEMCEP::~PHEMCEP() {
    // free power pattern
    _powerPatternFC.clear();
    _powerPatternPollutants.clear();
    _cepCurveFC.clear();
    _speedCurveRotational.clear();
    _speedPatternRotational.clear();
} // end of ~Cep


double
PHEMCEP::GetEmission(const std::string& pollutant, double power, double speed, bool normalized) const {
    std::vector<double> emissionCurve;
    std::vector<double> powerPattern;

    if (!normalized && fabs(speed) <= ZERO_SPEED_ACCURACY) {
        if (pollutant == "FC") {
            return _idlingFC;
        } else {
            return _idlingValuesPollutants.get(pollutant);
        }
    } // end if

    if (pollutant == "FC") {
        if (normalized) {
            emissionCurve = _normedCepCurveFC;
            powerPattern = _normalizedPowerPatternFC;
        } else {
            emissionCurve = _cepCurveFC;
            powerPattern = _powerPatternFC;
        }
    } else {
        if (!_cepCurvePollutants.hasString(pollutant)) {
            throw InvalidArgument("Emission pollutant " + pollutant + " not found!");
        }

        if (normalized) {
            emissionCurve = _normalizedCepCurvePollutants.get(pollutant);
            powerPattern = _normailzedPowerPatternPollutants;
        } else {
            emissionCurve = _cepCurvePollutants.get(pollutant);
            powerPattern = _powerPatternPollutants;
        }

    } // end if



    if (emissionCurve.size() == 0) {
        throw InvalidArgument("Empty emission curve for " + pollutant + " found!");
    }

    if (emissionCurve.size() == 1) {
        return emissionCurve[0];
    }

    // in case that the demanded power is smaller than the first entry (smallest) in the power pattern the first two entries are extrapolated
    if (power <= powerPattern.front()) {
        double calcEmission =  PHEMCEP::Interpolate(power, powerPattern[0], powerPattern[1], emissionCurve[0], emissionCurve[1]);

        if (calcEmission < 0) {
            return 0;
        } else {
            return calcEmission;
        }

    } // end if

    // if power bigger than all entries in power pattern the last two values are linearly extrapolated
    if (power >= powerPattern.back()) {
        return PHEMCEP::Interpolate(power, powerPattern[powerPattern.size() - 2], powerPattern.back(), emissionCurve[emissionCurve.size() - 2], emissionCurve.back());
    } // end if

    // bisection search to find correct position in power pattern
    int upperIndex;
    int lowerIndex;

    PHEMCEP::FindLowerUpperInPattern(lowerIndex, upperIndex, powerPattern, power);

    return PHEMCEP::Interpolate(power, powerPattern[lowerIndex], powerPattern[upperIndex], emissionCurve[lowerIndex], emissionCurve[upperIndex]);

} // end of GetEmission


double
PHEMCEP::Interpolate(double px, double p1, double p2, double e1, double e2) const {
    if (p2 == p1) {
        return e1;
    }
    return e1 + (px - p1) / (p2 - p1) * (e2 - e1);
} // end of Interpolate


double PHEMCEP::GetDecelCoast(double speed, double acc, double gradient, double /* vehicleLoading */) const {
    if (speed < SPEED_DCEL_MIN) {
        return speed / SPEED_DCEL_MIN * GetDecelCoast(SPEED_DCEL_MIN, acc, gradient, _vehicleLoading); // !!!vehicleLoading
    } // end if

    double rotCoeff = GetRotationalCoeffecient(speed);

    int upperIndex;
    int lowerIndex;

    double iGear = GetGearCoeffecient(speed);

    double iTot = iGear * _axleRatio;

    double n = (30 * speed * iTot) / ((_effictiveWheelDiameter / 2) * M_PI2);
    double nNorm = (n - _engineIdlingSpeed) / (_engineRatedSpeed - _engineIdlingSpeed);

    FindLowerUpperInPattern(lowerIndex, upperIndex, _nNormTable, nNorm);

    double fMot = 0;

    if (speed >= 10e-2) {
        fMot = (-GetDragCoeffecient(nNorm) * _ratedPower * 1000 / speed) / 0.9;
    } // end if

    double fRoll = (_resistanceF0
                    + _resistanceF1 * speed
                    + pow(_resistanceF2 * speed, 2)
                    + pow(_resistanceF3 * speed, 3)
                    + pow(_resistanceF4 * speed, 4)) * (_massVehicle + _vehicleLoading) * GRAVITY_CONST; // !!!vehicleLoading

    double fAir = _cdValue * _crossSectionalArea * 1.2 * 0.5 * pow(speed, 2);

    double fGrad = (_massVehicle + _vehicleLoading) * GRAVITY_CONST * gradient / 100; // !!!vehicleLoading

    return -(fMot + fRoll + fAir + fGrad) / ((_massVehicle + _vehicleLoading) * rotCoeff); // !!!vehicleLoading
} // end of GetDecelCoast


double
PHEMCEP::GetRotationalCoeffecient(double speed) const {
    int upperIndex;
    int lowerIndex;

    PHEMCEP::FindLowerUpperInPattern(lowerIndex, upperIndex, _speedPatternRotational, speed);

    return PHEMCEP::Interpolate(speed,
                                _speedPatternRotational[lowerIndex],
                                _speedPatternRotational[upperIndex],
                                _speedCurveRotational[lowerIndex],
                                _speedCurveRotational[upperIndex]);
} // end of GetRotationalCoeffecient

double PHEMCEP::GetGearCoeffecient(double speed) const {
    int upperIndex;
    int lowerIndex;

    FindLowerUpperInPattern(lowerIndex, upperIndex, _gearTransmissionCurve, speed);

    return Interpolate(speed,
                       _speedPatternRotational[lowerIndex],
                       _speedPatternRotational[upperIndex],
                       _gearTransmissionCurve[lowerIndex],
                       _gearTransmissionCurve[upperIndex]);
} // end of GetGearCoefficient

double PHEMCEP::GetDragCoeffecient(double nNorm)  const {
    int upperIndex;
    int lowerIndex;

    FindLowerUpperInPattern(lowerIndex, upperIndex, _dragNormTable, nNorm);

    return Interpolate(nNorm,
                       _nNormTable[lowerIndex],
                       _nNormTable[upperIndex],
                       _dragNormTable[lowerIndex],
                       _dragNormTable[upperIndex]);
} // end of GetGearCoefficient

void PHEMCEP::FindLowerUpperInPattern(int& lowerIndex, int& upperIndex, const std::vector<double>& pattern, double value) const {
    if (value <= pattern.front()) {
        lowerIndex = 0;
        upperIndex = 0;
        return;

    } // end if

    if (value >= pattern.back()) {
        lowerIndex = (int)pattern.size() - 1;
        upperIndex = (int)pattern.size() - 1;
        return;
    } // end if

    // bisection search to find correct position in power pattern
    int middleIndex = ((int)pattern.size() - 1) / 2;
    upperIndex = (int)pattern.size() - 1;
    lowerIndex = 0;

    while (upperIndex - lowerIndex > 1) {
        if (pattern[middleIndex] == value) {
            lowerIndex = middleIndex;
            upperIndex = middleIndex;
            return;
        } else if (pattern[middleIndex] < value) {
            lowerIndex = middleIndex;
            middleIndex = (upperIndex - lowerIndex) / 2 + lowerIndex;
        } else {
            upperIndex = middleIndex;
            middleIndex = (upperIndex - lowerIndex) / 2 + lowerIndex;
        } // end if
    } // end while

    if (pattern[lowerIndex] <= value && value < pattern[upperIndex]) {
        return;
    } else {
        throw ProcessError("Error during calculation of position in pattern!");
    }
} // end of FindLowerUpperInPattern


double
PHEMCEP::CalcPower(double v, double a, double slope, double /* vehicleLoading */) const {
    const double rotFactor = GetRotationalCoeffecient(v);
    double power = (_massVehicle + _vehicleLoading) * GRAVITY_CONST * (_resistanceF0 + _resistanceF1 * v + _resistanceF4 * pow(v, 4)) * v;
    power += (_crossSectionalArea * _cdValue * AIR_DENSITY_CONST / 2) * pow(v, 3);
    power += (_massVehicle * rotFactor + _massRot + _vehicleLoading) * a * v;
    power += (_massVehicle + _vehicleLoading) * slope * 0.01 * v;
    return power / 950.;
}


double
PHEMCEP::GetMaxAccel(double v, double a, double gradient, double /* vehicleLoading */) const {
    UNUSED_PARAMETER(a);
    double rotFactor = GetRotationalCoeffecient(v);
    const double pMaxForAcc = GetPMaxNorm(v) * _ratedPower - PHEMCEP::CalcPower(v, 0, gradient, _vehicleLoading); // !!!vehicleLoading
    return (pMaxForAcc * 1000) / ((_massVehicle * rotFactor + _massRot + _vehicleLoading) * v); // !!!vehicleLoading
}



double PHEMCEP::GetPMaxNorm(double speed) const {
    // Linear function between v0 and v1, constant elsewhere
    if (speed <= _pNormV0) {
        return _pNormP0;
    } else if (speed >= _pNormV1) {
        return _pNormP1;
    } else {
        return PHEMCEP::Interpolate(speed, _pNormV0, _pNormV1, _pNormP0, _pNormP1);
    }
} // end of GetPMaxNorm


/****************************************************************************/
