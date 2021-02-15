/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
// PHEMlight module
// Copyright (C) 2016-2017 Technische Universitaet Graz, https://www.tugraz.at/
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
/// @file    CEP.cpp
/// @author  Martin Dippold
/// @author  Michael Behrisch
/// @date    July 2016
///
//
/****************************************************************************/


#include "CEP.h"
#include "Constants.h"
#include "Helpers.h"


namespace PHEMlightdll {

    CEP::CEP(bool heavyVehicle, double vehicleMass, double vehicleLoading, double vehicleMassRot, double crossArea, double cWValue, double f0, double f1, double f2, double f3, double f4, double axleRatio, std::vector<double>& transmissionGearRatios, double auxPower, double ratedPower, double engineIdlingSpeed, double engineRatedSpeed, double effictiveWheelDiameter, double pNormV0, double pNormP0, double pNormV1, double pNormP1, const std::string& vehicelFuelType, std::vector<std::vector<double> >& matrixFC, std::vector<std::string>& headerLinePollutants, std::vector<std::vector<double> >& matrixPollutants, std::vector<std::vector<double> >& matrixSpeedRotational, std::vector<std::vector<double> >& normedDragTable, double idlingFC, std::vector<double>& idlingPollutants) {
        (void)transmissionGearRatios; // just to make the compiler happy about the unused parameter
        InitializeInstanceFields();
        _resistanceF0 = f0;
        _resistanceF1 = f1;
        _resistanceF2 = f2;
        _resistanceF3 = f3;
        _resistanceF4 = f4;
        _cWValue = cWValue;
        _crossSectionalArea = crossArea;
        _massVehicle = vehicleMass;
        _vehicleLoading = vehicleLoading;
        _vehicleMassRot = vehicleMassRot;
        _ratedPower = ratedPower;
        _engineIdlingSpeed = engineIdlingSpeed;
        _engineRatedSpeed = engineRatedSpeed;
        _effectiveWheelDiameter = effictiveWheelDiameter;
        _heavyVehicle = heavyVehicle;
        _fuelType = vehicelFuelType;
        _axleRatio = axleRatio;
        _auxPower = auxPower;

        _pNormV0 = pNormV0 / 3.6;
        _pNormP0 = pNormP0;
        _pNormV1 = pNormV1 / 3.6;
        _pNormP1 = pNormP1;

        std::vector<std::string> pollutantIdentifier;
        std::vector<std::vector<double> > pollutantMeasures;
        std::vector<std::vector<double> > normalizedPollutantMeasures;

        // init pollutant identifiers
        for (int i = 0; i < (int)headerLinePollutants.size(); i++) {
            pollutantIdentifier.push_back(headerLinePollutants[i]);
        }

        // initialize measures
        for (int i = 0; i < (int)headerLinePollutants.size(); i++) {
            pollutantMeasures.push_back(std::vector<double>());
            normalizedPollutantMeasures.push_back(std::vector<double>());
        }

        // looping through matrix and assigning values for speed rotational table
        _speedCurveRotational = std::vector<double>();
        _speedPatternRotational = std::vector<double>();
        _gearTransmissionCurve = std::vector<double>();
        for (int i = 0; i < (int)matrixSpeedRotational.size(); i++) {
            if (matrixSpeedRotational[i].size() != 3) {
                return;
            }

            _speedPatternRotational.push_back(matrixSpeedRotational[i][0] / 3.6);
            _gearTransmissionCurve.push_back(matrixSpeedRotational[i][1]);
            _speedCurveRotational.push_back(matrixSpeedRotational[i][2]);
        }

        // looping through matrix and assigning values for drag table
        _nNormTable = std::vector<double>();
        _dragNormTable = std::vector<double>();
        for (int i = 0; i < (int)normedDragTable.size(); i++) {
            if (normedDragTable[i].size() != 2) {
                return;
            }

            _nNormTable.push_back(normedDragTable[i][0]);
            _dragNormTable.push_back(normedDragTable[i][1]);
        }

        // looping through matrix and assigning values for Fuel consumption
        _cepCurveFC = std::vector<double>();
        _normedCepCurveFC = std::vector<double>();
        _powerPatternFC = std::vector<double>();
        _normalizedPowerPatternFC = std::vector<double>();
        for (int i = 0; i < (int)matrixFC.size(); i++) {
            if (matrixFC[i].size() != 2) {
                return;
            }

            _powerPatternFC.push_back(matrixFC[i][0] * _ratedPower);
            _normalizedPowerPatternFC.push_back(matrixFC[i][0]);
            _cepCurveFC.push_back(matrixFC[i][1] * _ratedPower);
            _normedCepCurveFC.push_back(matrixFC[i][1]);

        }

        _powerPatternPollutants = std::vector<double>();

        double pollutantMultiplyer = 1;

        _drivingPower = _normalizingPower = CalcPower(Constants::NORMALIZING_SPEED, Constants::NORMALIZING_ACCELARATION, 0);

        // looping through matrix and assigning values for pollutants
        if (heavyVehicle) {
            _normalizingPower = _ratedPower;
            _normalizingType = NormalizingType_RatedPower;
            pollutantMultiplyer = _ratedPower;
        }
        else {
            _normalizingPower = _drivingPower;
            _normalizingType = NormalizingType_DrivingPower;
        }

        _normailzedPowerPatternPollutants = std::vector<double>();

        _cepNormalizedCurvePollutants = std::map<std::string, std::vector<double> >();

        int headerCount = (int)headerLinePollutants.size();
        for (int i = 0; i < (int)matrixPollutants.size(); i++) {
            for (int j = 0; j < (int)matrixPollutants[i].size(); j++) {
                if ((int)matrixPollutants[i].size() != headerCount + 1) {
                    return;
                }

                if (j == 0) {
                    _normailzedPowerPatternPollutants.push_back(matrixPollutants[i][j]);
                    _powerPatternPollutants.push_back(matrixPollutants[i][j] * getNormalizingPower());
                }
                else {
                    pollutantMeasures[j - 1].push_back(matrixPollutants[i][j] * pollutantMultiplyer);
                    normalizedPollutantMeasures[j - 1].push_back(matrixPollutants[i][j]);
                }
            }
        }

        _cepCurvePollutants = std::map<std::string, std::vector<double> >();
        _idlingValuesPollutants = std::map<std::string, double>();

        for (int i = 0; i < (int)headerLinePollutants.size(); i++) {
            _cepCurvePollutants.insert(std::make_pair(pollutantIdentifier[i], pollutantMeasures[i]));
            _cepNormalizedCurvePollutants.insert(std::make_pair(pollutantIdentifier[i], normalizedPollutantMeasures[i]));
            _idlingValuesPollutants.insert(std::make_pair(pollutantIdentifier[i], idlingPollutants[i] * pollutantMultiplyer));
        }

        _idlingValueFC = idlingFC * _ratedPower;
    }

    const bool& CEP::getHeavyVehicle() const {
        return _heavyVehicle;
    }

    const std::string& CEP::getFuelType() const {
        return _fuelType;
    }

    const CEP::NormalizingType& CEP::getNormalizingTypeX() const {
        return _normalizingType;
    }

    const double& CEP::getRatedPower() const {
        return _ratedPower;
    }

    void CEP::setRatedPower(const double& value) {
        _ratedPower = value;
    }

    const double& CEP::getNormalizingPower() const {
        return _normalizingPower;
    }

    const double& CEP::getDrivingPower() const {
        return _drivingPower;
    }

    void CEP::setDrivingPower(const double& value) {
        _drivingPower = value;
    }

    double CEP::CalcPower(double speed, double acc, double gradient) {
        //Declaration
        double power = 0;
        double rotFactor = GetRotationalCoeffecient(speed);
        double powerAux = (_auxPower * _ratedPower);

        //Calculate the power
        power += (_massVehicle + _vehicleLoading) * Constants::GRAVITY_CONST * (_resistanceF0 + _resistanceF1 * speed + _resistanceF4 * std::pow(speed, 4)) * speed;
        power += (_crossSectionalArea * _cWValue * Constants::AIR_DENSITY_CONST / 2) * std::pow(speed, 3);
        power += (_massVehicle * rotFactor + _vehicleMassRot + _vehicleLoading) * acc * speed;
        power += (_massVehicle + _vehicleLoading) * Constants::GRAVITY_CONST * gradient * 0.01 * speed;
        power /= 1000;
        power /= Constants::_DRIVE_TRAIN_EFFICIENCY;
        power += powerAux;

        //Return result
        return power;
    }

    double CEP::CalcEngPower(double power) {
        if (power < _powerPatternFC.front()) {
            return _powerPatternFC.front();
        }
        if (power > _powerPatternFC.back()) {
            return _powerPatternFC.back();
        }

        return power;
    }

    double CEP::GetEmission(const std::string& pollutant, double power, double speed, Helpers* VehicleClass) {
        //Declaration
        std::vector<double> emissionCurve;
        std::vector<double> powerPattern;

        // bisection search to find correct position in power pattern	
        int upperIndex;
        int lowerIndex;

        if (_fuelType != Constants::strBEV) {
            if (std::abs(speed) <= Constants::ZERO_SPEED_ACCURACY) {
                if (pollutant == "FC") {
                    return _idlingValueFC;
                }
                else {
                    if (_cepCurvePollutants.find(pollutant) == _cepCurvePollutants.end()) {
                        VehicleClass->setErrMsg(std::string("Emission pollutant ") + pollutant + std::string(" not found!"));
                        return 0;
                    }

                    return _idlingValuesPollutants[pollutant];
                }
            }
        }

        if (pollutant == "FC") {
            emissionCurve = _cepCurveFC;
            powerPattern = _powerPatternFC;
        }
        else {
            if (_cepCurvePollutants.find(pollutant) == _cepCurvePollutants.end()) {
                VehicleClass->setErrMsg(std::string("Emission pollutant ") + pollutant + std::string(" not found!"));
                return 0;
            }

            emissionCurve = _cepCurvePollutants[pollutant];
            powerPattern = _powerPatternPollutants;
        }

        if (emissionCurve.empty()) {
            VehicleClass->setErrMsg(std::string("Empty emission curve for ") + pollutant + std::string(" found!"));
            return 0;
        }
        if (emissionCurve.size() == 1) {
            return emissionCurve[0];
        }

        // in case that the demanded power is smaller than the first entry (smallest) in the power pattern the first is returned (should never happen)
        if (power <= powerPattern.front()) {
            return emissionCurve[0];
        }

        // if power bigger than all entries in power pattern return the last (should never happen)
        if (power >= powerPattern.back()) {
            return emissionCurve.back();
        }

        FindLowerUpperInPattern(lowerIndex, upperIndex, powerPattern, power);
        return Interpolate(power, powerPattern[lowerIndex], powerPattern[upperIndex], emissionCurve[lowerIndex], emissionCurve[upperIndex]);
    }

    double CEP::GetCO2Emission(double _FC, double _CO, double _HC, Helpers* VehicleClass) {
        //Declaration
        double fCBr;
        double fCHC = 0.866;
        double fCCO = 0.429;
        double fCCO2 = 0.273;

//C# TO C++ CONVERTER NOTE: The following 'switch' operated on a string variable and was converted to C++ 'if-else' logic:
//        switch (_fuelType)
//ORIGINAL LINE: case Constants.strGasoline:
        if (_fuelType == Constants::strGasoline) {
                fCBr = 0.865;
        }
//ORIGINAL LINE: case Constants.strDiesel:
        else if (_fuelType == Constants::strDiesel) {
                fCBr = 0.863;
        }
//ORIGINAL LINE: case Constants.strCNG:
        else if (_fuelType == Constants::strCNG) {
                fCBr = 0.693;
                fCHC = 0.803;
        }
//ORIGINAL LINE: case Constants.strLPG:
        else if (_fuelType == Constants::strLPG) {
                fCBr = 0.825;
                fCHC = 0.825;
        }
        else {
                VehicleClass->setErrMsg(std::string("The propolsion type is not known! (") + _fuelType + std::string(")"));
                return 0;
        }

        return (_FC * fCBr - _CO * fCCO - _HC * fCHC) / fCCO2;
    }

    double CEP::GetDecelCoast(double speed, double acc, double gradient) {
        //Declaration
        int upperIndex;
        int lowerIndex;

        if (speed < Constants::SPEED_DCEL_MIN) {
            return speed / Constants::SPEED_DCEL_MIN * GetDecelCoast(Constants::SPEED_DCEL_MIN, acc, gradient);
        }

        double rotCoeff = GetRotationalCoeffecient(speed);
        FindLowerUpperInPattern(lowerIndex, upperIndex, _speedPatternRotational, speed);
        double iGear = Interpolate(speed, _speedPatternRotational[lowerIndex], _speedPatternRotational[upperIndex], _gearTransmissionCurve[lowerIndex], _gearTransmissionCurve[upperIndex]);

        double iTot = iGear * _axleRatio;

        double n = (30 * speed * iTot) / ((_effectiveWheelDiameter / 2) * M_PI);
        double nNorm = (n - _engineIdlingSpeed) / (_engineRatedSpeed - _engineIdlingSpeed);

        FindLowerUpperInPattern(lowerIndex, upperIndex, _nNormTable, nNorm);

        double fMot = 0;

        if (speed >= 10e-2) {
            fMot = (-Interpolate(nNorm, _nNormTable[lowerIndex], _nNormTable[upperIndex], _dragNormTable[lowerIndex], _dragNormTable[upperIndex]) * _ratedPower * 1000 / speed) / 0.9;
        }

        double fRoll = (_resistanceF0 + _resistanceF1 * speed + std::pow(_resistanceF2 * speed, 2) + std::pow(_resistanceF3 * speed, 3) + std::pow(_resistanceF4 * speed, 4)) * (_massVehicle + _vehicleLoading) * Constants::GRAVITY_CONST;

        double fAir = _cWValue * _crossSectionalArea * 1.2 * 0.5 * std::pow(speed, 2);

        double fGrad = (_massVehicle + _vehicleLoading) * Constants::GRAVITY_CONST * gradient / 100;

        return -(fMot + fRoll + fAir + fGrad) / ((_massVehicle + _vehicleLoading) * rotCoeff);
    }

    double CEP::GetRotationalCoeffecient(double speed) {
        //Declaration
        int upperIndex;
        int lowerIndex;

        FindLowerUpperInPattern(lowerIndex, upperIndex, _speedPatternRotational, speed);
        return Interpolate(speed, _speedPatternRotational[lowerIndex], _speedPatternRotational[upperIndex], _speedCurveRotational[lowerIndex], _speedCurveRotational[upperIndex]);
    }

    void CEP::FindLowerUpperInPattern(int& lowerIndex, int& upperIndex, std::vector<double>& pattern, double value) {
        lowerIndex = 0;
        upperIndex = 0;

        if (value <= pattern.front()) {
            lowerIndex = 0;
            upperIndex = 0;
            return;
        }

        if (value >= pattern.back()) {
            lowerIndex = (int)pattern.size() - 1;
            upperIndex = (int)pattern.size() - 1;
            return;
        }

        // bisection search to find correct position in power pattern	
        int middleIndex = ((int)pattern.size() - 1) / 2;
        upperIndex = (int)pattern.size() - 1;
        lowerIndex = 0;

        while (upperIndex - lowerIndex > 1) {
            if (pattern[middleIndex] == value) {
                lowerIndex = middleIndex;
                upperIndex = middleIndex;
                return;
            }
            else if (pattern[middleIndex] < value) {
                lowerIndex = middleIndex;
                middleIndex = (upperIndex - lowerIndex) / 2 + lowerIndex;
            }
            else {
                upperIndex = middleIndex;
                middleIndex = (upperIndex - lowerIndex) / 2 + lowerIndex;
            }
        }

        if (pattern[lowerIndex] <= value && value < pattern[upperIndex]) {
            return;
        }
    }

    double CEP::Interpolate(double px, double p1, double p2, double e1, double e2) {
        if (p2 == p1) {
            return e1;
        }

        return e1 + (px - p1) / (p2 - p1) * (e2 - e1);
    }

    double CEP::GetMaxAccel(double speed, double gradient) {
        double rotFactor = GetRotationalCoeffecient(speed);
        double pMaxForAcc = GetPMaxNorm(speed) * _ratedPower - CalcPower(speed, 0, gradient);

        return (pMaxForAcc * 1000) / ((_massVehicle * rotFactor + _vehicleMassRot + _vehicleLoading) * speed);
    }

    double CEP::GetPMaxNorm(double speed) {
        // Linear function between v0 and v1, constant elsewhere
        if (speed <= _pNormV0) {
            return _pNormP0;
        }
        else if (speed >= _pNormV1) {
            return _pNormP1;
        }
        else {
            return Interpolate(speed, _pNormV0, _pNormV1, _pNormP0, _pNormP1);
        }
    }

    void CEP::InitializeInstanceFields() {
        _heavyVehicle = false;
        _normalizingType = static_cast<NormalizingType>(0);
        _ratedPower = 0;
        _normalizingPower = 0;
        _drivingPower = 0;
        _massVehicle = 0;
        _vehicleLoading = 0;
        _vehicleMassRot = 0;
        _crossSectionalArea = 0;
        _cWValue = 0;
        _resistanceF0 = 0;
        _resistanceF1 = 0;
        _resistanceF2 = 0;
        _resistanceF3 = 0;
        _resistanceF4 = 0;
        _axleRatio = 0;
        _auxPower = 0;
        _pNormV0 = 0;
        _pNormP0 = 0;
        _pNormV1 = 0;
        _pNormP1 = 0;
        _engineRatedSpeed = 0;
        _engineIdlingSpeed = 0;
        _effectiveWheelDiameter = 0;
        _idlingValueFC = 0;
    }
}
