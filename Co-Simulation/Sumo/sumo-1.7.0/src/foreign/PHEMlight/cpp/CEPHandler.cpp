/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
// PHEMlight module
// Copyright 2016 Technische Universitaet Graz, https://www.tugraz.at/
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
/// @file    CEPHandler.cpp
/// @author  Martin Dippold
/// @author  Michael Behrisch
/// @date    July 2016
///
//
/****************************************************************************/


#include <fstream>
#include <sstream>
#include "CEPHandler.h"
#include "CEP.h"
#include "Helpers.h"
#include "Constants.h"


namespace PHEMlightdll {

    CEPHandler::CEPHandler() {
        _ceps = std::map<std::string, CEP*>();
    }

    const std::map<std::string, CEP*>& CEPHandler::getCEPS() const {
        return _ceps;
    }

    bool CEPHandler::GetCEP(const std::vector<std::string>& DataPath, Helpers* Helper) {
        if (getCEPS().find(Helper->getgClass()) == getCEPS().end()) {
            if (!Load(DataPath, Helper)) {
                return false;
            }
        }
        return true;
    }

    bool CEPHandler::Load(const std::vector<std::string>& DataPath, Helpers* Helper) {
        //Deklaration
        // get string identifier for PHEM emission class
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to 'ToString':
        std::string emissionRep = Helper->getgClass();

        // to hold everything.
        std::vector<std::vector<double> > matrixSpeedInertiaTable;
        std::vector<std::vector<double> > normedTragTableSpeedInertiaTable;
        std::vector<std::vector<double> > matrixFC;
        std::vector<std::vector<double> > matrixPollutants;
        std::vector<double> idlingValuesFC;
        std::vector<double> idlingValuesPollutants;
        std::vector<std::string> headerFC;
        std::vector<std::string> headerPollutants;

        double vehicleMass;
        double vehicleLoading;
        double vehicleMassRot;
        double crosssectionalArea;
        double cwValue;
        double f0;
        double f1;
        double f2;
        double f3;
        double f4;
        double axleRatio;
        std::vector<double> transmissionGearRatios;
        double auxPower;
        double ratedPower;
        double engineIdlingSpeed;
        double engineRatedSpeed;
        double effectiveWhellDiameter;
        std::string vehicleMassType;
        std::string vehicleFuelType;
        double pNormV0;
        double pNormP0;
        double pNormV1;
        double pNormP1;

        if (!ReadVehicleFile(DataPath, emissionRep, Helper, vehicleMass, vehicleLoading, vehicleMassRot, crosssectionalArea, cwValue, f0, f1, f2, f3, f4, axleRatio, auxPower, ratedPower, engineIdlingSpeed, engineRatedSpeed, effectiveWhellDiameter, transmissionGearRatios, vehicleMassType, vehicleFuelType, pNormV0, pNormP0, pNormV1, pNormP1, matrixSpeedInertiaTable, normedTragTableSpeedInertiaTable)) {
            return false;
        }

        if (!ReadEmissionData(true, DataPath, emissionRep, Helper, headerFC, matrixFC, idlingValuesFC)) {
            return false;
        }

        if (!ReadEmissionData(false, DataPath, emissionRep, Helper, headerPollutants, matrixPollutants, idlingValuesPollutants)) {
            return false;
        }

        _ceps.insert(std::make_pair(Helper->getgClass(), new CEP(vehicleMassType == Constants::HeavyVehicle, vehicleMass, vehicleLoading, vehicleMassRot, crosssectionalArea, cwValue, f0, f1, f2, f3, f4, axleRatio, transmissionGearRatios, auxPower, ratedPower, engineIdlingSpeed, engineRatedSpeed, effectiveWhellDiameter, pNormV0, pNormP0, pNormV1, pNormP1, vehicleFuelType, matrixFC, headerPollutants, matrixPollutants, matrixSpeedInertiaTable, normedTragTableSpeedInertiaTable, idlingValuesFC.front(), idlingValuesPollutants)));

        return true;
    }

    bool CEPHandler::ReadVehicleFile(const std::vector<std::string>& DataPath, const std::string& emissionClass, Helpers* Helper, double& vehicleMass, double& vehicleLoading, double& vehicleMassRot, double& crossArea, double& cWValue, double& f0, double& f1, double& f2, double& f3, double& f4, double& axleRatio, double& auxPower, double& ratedPower, double& engineIdlingSpeed, double& engineRatedSpeed, double& effectiveWheelDiameter, std::vector<double>& transmissionGearRatios, std::string& vehicleMassType, std::string& vehicleFuelType, double& pNormV0, double& pNormP0, double& pNormV1, double& pNormP1, std::vector<std::vector<double> >& matrixSpeedInertiaTable, std::vector<std::vector<double> >& normedDragTable) {
        vehicleMass = 0;
        vehicleLoading = 0;
        vehicleMassRot = 0;
        crossArea = 0;
        cWValue = 0;
        f0 = 0;
        f1 = 0;
        f2 = 0;
        f3 = 0;
        f4 = 0;
        axleRatio = 0;
        ratedPower = 0;
        auxPower = 0;
        engineIdlingSpeed = 0;
        engineRatedSpeed = 0;
        effectiveWheelDiameter = 0;
        vehicleMassType = "";
        vehicleFuelType = "";
        pNormV0 = 0;
        pNormP0 = 0;
        pNormV1 = 0;
        pNormP1 = 0;
        transmissionGearRatios = std::vector<double>();
        matrixSpeedInertiaTable = std::vector<std::vector<double> >();
        normedDragTable = std::vector<std::vector<double> >();
        std::string line;
        std::string cell;
        int dataCount = 0;

        //Open file
        std::ifstream vehicleReader;
        for (std::vector<std::string>::const_iterator i = DataPath.begin(); i != DataPath.end(); i++) {
            vehicleReader.open(((*i) + emissionClass + ".PHEMLight.veh").c_str());
            if (vehicleReader.good()) {
                break;
            }
        }
        if (!vehicleReader.good()) {
            Helper->setErrMsg("File does not exist! (" + emissionClass + ".PHEMLight.veh)");
            return false;
        }

        // skip header
        ReadLine(vehicleReader);

        while ((line = ReadLine(vehicleReader)) != "" && dataCount <= 49) {
            if (line.substr(0, 1) == Helper->getCommentPrefix()) {
                continue;
            }
            else {
                dataCount++;
            }

            cell = split(line, ',')[0];

            // reading Mass
            if (dataCount == 1) {
                vehicleMass = todouble(cell);
            }

            // reading vehicle loading
            if (dataCount == 2) {
                vehicleLoading = todouble(cell);
            }

            // reading cWValue
            if (dataCount == 3) {
                cWValue = todouble(cell);
            }

            // reading crossectional area
            if (dataCount == 4) {
                crossArea = todouble(cell);
            }

            // reading vehicle mass rotational
            if (dataCount == 7) {
                vehicleMassRot = todouble(cell);
            }

            // reading rated power
            if (dataCount == 9) {
                auxPower = todouble(cell);
            }

            // reading rated power
            if (dataCount == 10) {
                ratedPower = todouble(cell);
            }

            // reading engine rated speed
            if (dataCount == 11) {
                engineRatedSpeed = todouble(cell);
            }

            // reading engine idling speed
            if (dataCount == 12) {
                engineIdlingSpeed = todouble(cell);
            }

            // reading f0
            if (dataCount == 14) {
                f0 = todouble(cell);
            }

            // reading f1
            if (dataCount == 15) {
                f1 = todouble(cell);
            }

            // reading f2
            if (dataCount == 16) {
                f2 = todouble(cell);
            }

            // reading f3
            if (dataCount == 17) {
                f3 = todouble(cell);
            }

            // reading f4
            if (dataCount == 18) {
                f4 = todouble(cell);
            }

            // reading axleRatio
            if (dataCount == 21) {
                axleRatio = todouble(cell);
            }

            // reading effective wheel diameter
            if (dataCount == 22) {
                effectiveWheelDiameter = todouble(cell);
            }

            if (dataCount >= 23 && dataCount <= 40) {
                transmissionGearRatios.push_back(todouble(cell));
            }

            // reading vehicleMassType
            if (dataCount == 45) {
                vehicleMassType = cell;
            }

            // reading vehicleFuelType
            if (dataCount == 46) {
                vehicleFuelType = cell;
            }

            // reading pNormV0
            if (dataCount == 47) {
                pNormV0 = todouble(cell);
            }

            // reading pNormP0
            if (dataCount == 48) {
                pNormP0 = todouble(cell);
            }

            // reading pNormV1
            if (dataCount == 49) {
                pNormV1 = todouble(cell);
            }

            // reading pNormP1
            if (dataCount == 50) {
                pNormP1 = todouble(cell);
            }
        }

        while ((line = ReadLine(vehicleReader)) != "" && line.substr(0, 1) != Helper->getCommentPrefix()) {
            if (line.substr(0, 1) == Helper->getCommentPrefix()) {
                continue;
            }

            matrixSpeedInertiaTable.push_back(todoubleList(split(line, ',')));
        }

        while ((line = ReadLine(vehicleReader)) != "") {
            if (line.substr(0, 1) == Helper->getCommentPrefix()) {
                continue;
            }

            normedDragTable.push_back(todoubleList(split(line, ',')));
        }

        return true;
    }

    bool CEPHandler::ReadEmissionData(bool readFC, const std::vector<std::string>& DataPath, const std::string& emissionClass, Helpers* Helper, std::vector<std::string>& header, std::vector<std::vector<double> >& matrix, std::vector<double>& idlingValues) {
        // declare file stream
        std::string line;
        header = std::vector<std::string>();
        matrix = std::vector<std::vector<double> >();
        idlingValues = std::vector<double>();

        std::string pollutantExtension = "";
        if (readFC) {
            pollutantExtension += std::string("_FC");
        }

        std::ifstream fileReader;
        for (std::vector<std::string>::const_iterator i = DataPath.begin(); i != DataPath.end(); i++) {
            fileReader.open(((*i) + emissionClass + pollutantExtension + ".csv").c_str());
            if (fileReader.good()) {
                break;
            }
        }
        if (!fileReader.good()) {
            Helper->setErrMsg("File does not exist! (" + emissionClass + pollutantExtension + ".csv)");
            return false;
        }

        // read header line for pollutant identifiers
        if ((line = ReadLine(fileReader)) != "") {
            std::vector<std::string> entries = split(line, ',');
            // skip first entry "Pe"
            for (int i = 1; i < (int)entries.size(); i++) {
                header.push_back(entries[i]);
            }
        }

        // skip units
        ReadLine(fileReader);

        // skip comment
        ReadLine(fileReader);

        //readIdlingValues
        line = ReadLine(fileReader);

        std::vector<std::string> stringIdlings = split(line, ',');
        stringIdlings.erase(stringIdlings.begin());

        idlingValues = todoubleList(stringIdlings);

        while ((line = ReadLine(fileReader)) != "") {
            matrix.push_back(todoubleList(split(line, ',')));
        }
        return true;
    }

    std::vector<std::string> CEPHandler::split(const std::string& s, char delim) {
        std::vector<std::string> elems;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }

    double CEPHandler::todouble(const std::string& s) {
        std::stringstream ss(s);
        double item;
        ss >> item;
        return item;
    }

    std::vector<double> CEPHandler::todoubleList(const std::vector<std::string>& s) {
        std::vector<double> result;
        for (std::vector<std::string>::const_iterator i = s.begin(); i != s.end(); ++i) {
            result.push_back(todouble(*i));
        }
        return result;
    }

    std::string CEPHandler::ReadLine(std::ifstream& s) {
        std::string line;
        std::getline(s, line);
        line.erase(line.find_last_not_of(" \n\r\t") + 1);
        return line;
    }
}
