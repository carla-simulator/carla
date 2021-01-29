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
/// @file    Constants.h
/// @author  Martin Dippold
/// @author  Michael Behrisch
/// @date    July 2016
///
//
/****************************************************************************/


#ifndef PHEMlightCONSTANTS
#define PHEMlightCONSTANTS

#include <string>

#define FLEET

namespace PHEMlightdll {
    class Constants {
        //Calculation constant
    public:
        static const double GRAVITY_CONST;
        static const double AIR_DENSITY_CONST;
        static const double NORMALIZING_SPEED;
        static const double NORMALIZING_ACCELARATION;
        static const double SPEED_DCEL_MIN;
        static const double ZERO_SPEED_ACCURACY;
        static const double DRIVE_TRAIN_EFFICIENCY_All;
        static const double DRIVE_TRAIN_EFFICIENCY_CB;


        //Strings
        static const std::string HeavyVehicle;

        //Vehiclestrings
        static const std::string strPKW;
        static const std::string strLNF;
        static const std::string strLKW;
        static const std::string strLSZ;
        static const std::string strRB;
        static const std::string strLB;
        static const std::string strMR2;
        static const std::string strMR4;
        static const std::string strKKR;

        //Fuelstrings
        static const std::string strGasoline;
        static const std::string strDiesel;
        static const std::string strCNG;
        static const std::string strLPG;
        static const std::string strHybrid;
        static const std::string strBEV;

        //Euroclasses
        static const std::string strEU;

        //Sizeclasse
        static const std::string strSI;
        static const std::string strSII;
        static const std::string strSIII;


        //Drive train efficiency
        static double _DRIVE_TRAIN_EFFICIENCY;
        const static double&  getDRIVE_TRAIN_EFFICIENCY();
        static void setDRIVE_TRAIN_EFFICIENCY(const double&  value);

    };
}


#endif	//#ifndef PHEMlightCONSTANTS
