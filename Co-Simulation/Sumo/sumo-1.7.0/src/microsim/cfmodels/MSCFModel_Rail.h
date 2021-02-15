/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSCFModel_Rail.h
/// @author  Gregor L\"ammel
/// @date    Tue, 08 Feb 2017
///
// <description missing>
/****************************************************************************/
#pragma once


#include "MSCFModel.h"



class MSCFModel_Rail : public MSCFModel {

public:
    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel_Rail(const MSVehicleType* vtype);

    double followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed,
                       double predMaxDecel, const MSVehicle* const pred = 0) const;

    virtual int getModelID() const;

    virtual MSCFModel* duplicate(const MSVehicleType* vtype) const;



    virtual ~MSCFModel_Rail();

    virtual double maxNextSpeed(double speed, const MSVehicle* const veh) const;

    virtual double minNextSpeed(double speed, const MSVehicle* const veh) const;

    virtual double minNextSpeedEmergency(double speed, const MSVehicle* const veh = 0) const;

    double getSpeedAfterMaxDecel(double v) const;

    MSCFModel::VehicleVariables* createVehicleVariables() const;

    double finalizeSpeed(MSVehicle* const veh, double vPos) const;

    double freeSpeed(const MSVehicle* const veh, double speed, double seen, double maxSpeed,
                     const bool onInsertion) const;

private:


    typedef std::map<double, double> LookUpMap;

    struct TrainParams {
        double weight;
        double mf;
        double length;
        double decl;
        double vmax;
        double recovery;
        double rotWeight;
        LookUpMap traction;
        LookUpMap resistance;
    };

    double getInterpolatedValueFromLookUpMap(double speed, const LookUpMap* lookUpMap) const;


public:
    double stopSpeed(const MSVehicle* const veh, const double speed, double gap) const;

//    class VehicleVariables : public MSCFModel::VehicleVariables {
//
//    public:
//
//        const std::string getTrainType() const { return myTrainType; };
//
//        void setTrainType(std::string trainType) { myTrainType = trainType; }
//
//        bool isNotYetInitialized() {
//            return notYetInitialized;
//        }
//
//        void setInitialized() {
//            notYetInitialized = false;
//        }
//
//    private:
//        std::string myTrainType;
//        bool notYetInitialized = true;
//
//
//    };

private:


    TrainParams myTrainParams;

    LookUpMap initNGT400Traction() const { // early version of NGT 400
        LookUpMap map;
        map[0] = 716.0;
        map[10] = 700.0;
        map[20] = 684.0;
        map[30] = 669.0;
        map[40] = 653.0;
        map[50] = 637.0;
        map[60] = 622.0;
        map[70] = 606.0;
        map[80] = 590.0;
        map[90] = 574.0;
        map[100] = 559.0;
        map[110] = 543.0;
        map[120] = 527.0;
        map[130] = 507.0;
        map[140] = 471.0;
        map[150] = 439.0;
        map[160] = 412.0;
        map[170] = 388.0;
        map[180] = 366.0;
        map[190] = 347.0;
        map[200] = 329.0;
        map[210] = 314.0;
        map[220] = 299.0;
        map[230] = 286.0;
        map[240] = 275.0;
        map[250] = 264.0;
        map[260] = 253.0;
        map[270] = 244.0;
        map[280] = 235.0;
        map[290] = 227.0;
        map[300] = 220.0;
        map[310] = 213.0;
        map[320] = 206.0;
        map[330] = 200.0;
        map[340] = 194.0;
        map[350] = 188.0;
        map[360] = 183.0;
        map[370] = 178.0;
        map[380] = 173.0;
        map[390] = 169.0;
        map[400] = 165.0;
        map[410] = 160.6;
        map[420] = 156.8;
        map[430] = 153.2;
        map[440] = 149.7;
        map[450] = 146.4;
        map[460] = 143.2;
        map[470] = 140.1;
        map[480] = 137.2;
        return map;
    }

    LookUpMap initNGT400Resistance() const { // early version of NGT 400
        LookUpMap map;
        map[0] = 1.9;
        map[10] = 2.1;
        map[20] = 2.4;
        map[30] = 2.9;
        map[40] = 3.5;
        map[50] = 4.2;
        map[60] = 5.0;
        map[70] = 6.0;
        map[80] = 7.0;
        map[90] = 8.2;
        map[100] = 9.6;
        map[110] = 11.0;
        map[120] = 12.6;
        map[130] = 14.3;
        map[140] = 16.1;
        map[150] = 18.0;
        map[160] = 20.1;
        map[170] = 22.3;
        map[180] = 24.6;
        map[190] = 27.0;
        map[200] = 29.5;
        map[210] = 32.2;
        map[220] = 35.0;
        map[230] = 37.9;
        map[240] = 40.9;
        map[250] = 44.1;
        map[260] = 47.4;
        map[270] = 50.8;
        map[280] = 54.3;
        map[290] = 58.0;
        map[300] = 61.7;
        map[310] = 65.6;
        map[320] = 69.7;
        map[330] = 73.8;
        map[340] = 78.1;
        map[350] = 82.4;
        map[360] = 86.9;
        map[370] = 91.6;
        map[380] = 96.3;
        map[390] = 101.2;
        map[400] = 106.2;
        map[410] = 111.3;
        map[420] = 116.6;
        map[430] = 121.9;
        map[440] = 127.4;
        map[450] = 133.0;
        map[460] = 138.7;
        map[470] = 144.6;
        map[480] = 150.6;
        return map;
    }

    TrainParams initNGT400Params() const {
        TrainParams params;
        params.weight = 384;
        params.mf = 1.04;
        params.length = 200;
        params.decl = 0.9;
        params.vmax = 500 / 3.6;
        params.recovery = 0.6;
        params.rotWeight = params.weight * params.mf;
        params.traction = initNGT400Traction();
        params.resistance = initNGT400Resistance();
        return params;
    }

    LookUpMap initNGT400_16Traction() const {
        LookUpMap map;
        map[0] = 274.5;
        map[10] = 274.5;
        map[20] = 274.5;
        map[30] = 274.5;
        map[40] = 274.5;
        map[50] = 274.5;
        map[60] = 274.5;
        map[70] = 274.5;
        map[80] = 274.5;
        map[90] = 274.5;
        map[100] = 274.5;
        map[110] = 274.5;
        map[120] = 274.5;
        map[130] = 274.5;
        map[140] = 274.5;
        map[150] = 274.5;
        map[160] = 274.5;
        map[170] = 274.5;
        map[180] = 274.5;
        map[190] = 274.5;
        map[200] = 274.5;
        map[210] = 273;
        map[220] = 262.3;
        map[230] = 250;
        map[240] = 239;
        map[250] = 229;
        map[260] = 222;
        map[270] = 213;
        map[280] = 205;
        map[290] = 198;
        map[300] = 191;
        map[310] = 186;
        map[320] = 180;
        map[330] = 174;
        map[340] = 169;
        map[350] = 165;
        map[360] = 160;
        map[370] = 155.5;
        map[380] = 151;
        map[390] = 148;
        map[400] = 144;
        map[410] = 140;
        map[420] = 136;
        map[430] = 134;
        map[440] = 131;
        map[450] = 128;
        map[460] = 125;
        map[470] = 123;
        return map;
    }

    LookUpMap initNGT400_16Resistance() const {
        LookUpMap map;
        map[0] = 5.71;
        map[10] = 6.01;
        map[20] = 6.4;
        map[30] = 7.0;
        map[40] = 7.51;
        map[50] = 8.34;
        map[60] = 9.2;
        map[70] = 10.4;
        map[80] = 11.3;
        map[90] = 12.58;
        map[100] = 14.05;
        map[110] = 15.5;
        map[120] = 16.9;
        map[130] = 18.75;
        map[140] = 20.77;
        map[150] = 22.9;
        map[160] = 25.0;
        map[170] = 26.9;
        map[180] = 29.38;
        map[190] = 32.0;
        map[200] = 34.5;
        map[210] = 37.0;
        map[220] = 39.91;
        map[230] = 43.11;
        map[240] = 46.4;
        map[250] = 49.3;
        map[260] = 52.4;
        map[270] = 56.09;
        map[280] = 59.8;
        map[290] = 63.7;
        map[300] = 66.8;
        map[310] = 71.0;
        map[320] = 75.39;
        map[330] = 79.9;
        map[340] = 84.1;
        map[350] = 87.9;
        map[360] = 92.7;
        map[370] = 97.6;
        map[380] = 102.0;
        map[390] = 107.0;
        map[400] = 112.0;
        map[410] = 117.5;
        map[420] = 123.0;
        map[430] = 128.3;
        map[440] = 133.3;
        map[450] = 139.2;
        map[460] = 145.5;
        map[470] = 150.0;
        return map;
    }

    TrainParams initNGT400_16Params() const {
        TrainParams params;
        params.weight = 384;
        params.mf = 1.04;
        params.length = 200;
        params.decl = 0.9;
        params.vmax = 430 / 3.6;
        params.recovery = 0.6;
        params.rotWeight = params.weight * params.mf;
        params.traction = initNGT400_16Traction();
        params.resistance = initNGT400_16Resistance();
        return params;
    }

    LookUpMap initICE1Traction() const {
        LookUpMap map;
        map[0] = 400;
        map[10] = 394;
        map[20] = 388;
        map[30] = 382;
        map[40] = 377;
        map[50] = 372;
        map[60] = 369;
        map[70] = 366;
        map[80] = 363;
        map[90] = 361;
        map[100] = 349;
        map[110] = 317;
        map[120] = 290;
        map[130] = 268;
        map[140] = 249;
        map[150] = 232;
        map[160] = 217;
        map[170] = 205;
        map[180] = 193;
        map[190] = 183;
        map[200] = 174;
        map[210] = 165;
        map[220] = 158;
        map[230] = 151;
        map[240] = 145;
        map[250] = 139;
        return map;
    }

    LookUpMap initICE1Resistance() const {
        LookUpMap map;
        map[0] = 10.7;
        map[10] = 12.3;
        map[20] = 14.2;
        map[30] = 16.4;
        map[40] = 18.7;
        map[50] = 21.3;
        map[60] = 24.2;
        map[70] = 27.3;
        map[80] = 30.6;
        map[90] = 34.1;
        map[100] = 37.9;
        map[110] = 41.9;
        map[120] = 46.2;
        map[130] = 50.6;
        map[140] = 55.4;
        map[150] = 60.4;
        map[160] = 65.6;
        map[170] = 71.1;
        map[180] = 76.7;
        map[190] = 82.6;
        map[200] = 88.8;
        map[210] = 95.2;
        map[220] = 101.8;
        map[230] = 108.7;
        map[240] = 115.8;
        map[250] = 123.1;
        return map;
    }

    TrainParams initICE1Params() const {
        TrainParams params;
        params.weight = 876;
        params.mf = 1.1;
        params.length = 358;
        params.decl = 0.5;
        params.vmax = 250 / 3.6;
        params.recovery = 0.1;
        params.rotWeight = params.weight * params.mf;
        params.traction = initICE1Traction();
        params.resistance = initICE1Resistance();
        return params;
    }

    TrainParams initICE3Params() const {
        TrainParams params;
        params.weight = 420;
        params.mf = 1.04;
        params.length = 200;
        params.decl = 0.5;
        params.vmax = 300 / 3.6;
        params.recovery = 0.6;
        params.rotWeight = params.weight * params.mf;
        params.traction = initICE3Traction();
        params.resistance = initICE3Resistance();
        return params;
    }

    LookUpMap initICE3Traction() const {
        LookUpMap map;
        map[0] = 300;
        map[10] = 298;
        map[20] = 297;
        map[30] = 295;
        map[40] = 293;
        map[50] = 292;
        map[60] = 290;
        map[70] = 288;
        map[80] = 286.5;
        map[90] = 285;
        map[100] = 283;
        map[110] = 262;
        map[120] = 240;
        map[130] = 221;
        map[140] = 206;
        map[150] = 192;
        map[160] = 180;
        map[170] = 169;
        map[180] = 160;
        map[190] = 152;
        map[200] = 144;
        map[210] = 137;
        map[220] = 131;
        map[230] = 125;
        map[240] = 120;
        map[250] = 115;
        map[260] = 111;
        map[270] = 107;
        map[280] = 103;
        map[290] = 99;
        map[300] = 96;
        return map;
    }

    LookUpMap initICE3Resistance() const {
        LookUpMap map;
        map[0] = 7.4;
        map[10] = 7.6;
        map[20] = 8.0;
        map[30] = 8.4;
        map[40] = 9.1;
        map[50] = 9.8;
        map[60] = 10.7;
        map[70] = 11.7;
        map[80] = 12.8;
        map[90] = 14.1;
        map[100] = 15.5;
        map[110] = 17.1;
        map[120] = 18.8;
        map[130] = 20.6;
        map[140] = 22.6;
        map[150] = 24.6;
        map[160] = 26.9;
        map[170] = 29.2;
        map[180] = 31.7;
        map[190] = 34.3;
        map[200] = 37.1;
        map[210] = 40.0;
        map[220] = 43.1;
        map[230] = 46.2;
        map[240] = 49.6;
        map[250] = 53.0;
        map[260] = 56.6;
        map[270] = 60.3;
        map[280] = 64.1;
        map[290] = 68.1;
        map[300] = 71.8;
        return map;
    }

    TrainParams initREDosto7Params() const {
        TrainParams params;
        params.weight = 425.5;
        params.mf = 1.1;
        params.length = 207;
        params.decl = 0.5;
        params.vmax = 160 / 3.6;
        params.recovery = 0.1;
        params.rotWeight = params.weight * params.mf;
        params.traction = initREDosto7Traction();
        params.resistance = initREDosto7Resistance();
        return params;
    }

    LookUpMap initREDosto7Traction() const {
        LookUpMap map;
        map[0] = 300;
        map[10] = 300;
        map[20] = 300;
        map[30] = 300;
        map[40] = 300;
        map[50] = 300;
        map[60] = 300;
        map[70] = 289;
        map[80] = 253;
        map[90] = 224;
        map[100] = 202;
        map[110] = 183;
        map[120] = 168;
        map[130] = 155;
        map[140] = 144;
        map[150] = 134;
        map[160] = 125;
        return map;
    }

    LookUpMap initREDosto7Resistance() const {
        LookUpMap map;
        map[0] = 8.5;
        map[10] = 8.9;
        map[20] = 9.5;
        map[30] = 10.4;
        map[40] = 11.4;
        map[50] = 12.7;
        map[60] = 14.1;
        map[70] = 15.8;
        map[80] = 17.7;
        map[90] = 19.8;
        map[100] = 22.6;
        map[110] = 24.6;
        map[120] = 27.3;
        map[130] = 30.2;
        map[140] = 33.3;
        map[150] = 36.6;
        map[160] = 40.2;
        return map;
    }

    TrainParams initRB628Params() const {
        TrainParams params;
        params.weight = 72.2;
        params.mf = 1.04;
        params.length = 46;
        params.decl = 0.5;
        params.vmax = 120 / 3.6;
        params.recovery = 0;
        params.rotWeight = params.weight * params.mf;
        params.traction = initRB628Traction();
        params.resistance = initRB628Resistance();
        return params;
    }

    LookUpMap initRB628Traction() const {
        LookUpMap map;
        map[0] = 60;
        map[10] = 53.8;
        map[20] = 47.6;
        map[30] = 36.9;
        map[40] = 28.7;
        map[50] = 23.5;
        map[60] = 20.0;
        map[70] = 17.5;
        map[80] = 15.2;
        map[90] = 13.9;
        map[100] = 12.8;
        map[110] = 11.7;
        map[120] = 10.8;
        return map;
    }

    LookUpMap initRB628Resistance() const {
        LookUpMap map;
        map[0] = 1.29;
        map[10] = 1.46;
        map[20] = 1.73;
        map[30] = 2.08;
        map[40] = 2.52;
        map[50] = 3.05;
        map[60] = 3.66;
        map[70] = 4.36;
        map[80] = 5.16;
        map[90] = 6.03;
        map[100] = 7.00;
        map[110] = 8.06;
        map[120] = 9.2;
        return map;
    }

    TrainParams initFreightParams() const {
        TrainParams params;
        params.weight = 1440;
        params.mf = 1.06;
        params.length = 512;
        params.decl = 0.3;
        params.vmax = 120 / 3.6;
        params.recovery = 0.05;
        params.rotWeight = params.weight * params.mf;
        params.traction = initFreightTraction();
        params.resistance = initFreightResistance();
        return params;
    }

    LookUpMap initFreightTraction() const {
        LookUpMap map;
        map[0] = 300;
        map[10] = 296;
        map[20] = 293;
        map[30] = 289;
        map[40] = 286;
        map[50] = 282;
        map[60] = 279;
        map[70] = 275;
        map[80] = 272;
        map[90] = 255;
        map[100] = 230;
        map[110] = 209;
        map[120] = 190;//guessed value
        return map;
    }

    LookUpMap initFreightResistance() const {
        LookUpMap map;
        map[0] = 1.9;
        map[10] = 4.3;
        map[20] = 8.5;
        map[30] = 14.3;
        map[40] = 21.7;
        map[50] = 30.8;
        map[60] = 41.5;
        map[70] = 53.8;
        map[80] = 67.8;
        map[90] = 83.5;
        map[100] = 110.7;
        map[110] = 119.6;
        map[120] = 140.2;
        return map;
    }

    LookUpMap initRB425Traction() const {
        LookUpMap map;
        map[0] = 150;
        map[10] = 150;
        map[20] = 150;
        map[30] = 150;
        map[40] = 150;
        map[50] = 150;
        map[60] = 140;
        map[70] = 120;
        map[80] = 105;
        map[90] = 93;
        map[100] = 84;
        map[110] = 75;
        map[120] = 70;
        map[130] = 56;
        map[140] = 52;
        map[150] = 46;
        map[160] = 40;
        return map;
    }

    LookUpMap initRB425Resistance() const {
        LookUpMap map;
        map[0] = 2.6;
        map[10] = 2.9;
        map[20] = 3.3;
        map[30] = 3.7;
        map[40] = 4.3;
        map[50] = 4.9;
        map[60] = 5.7;
        map[70] = 6.6;
        map[80] = 7.5;
        map[90] = 8.6;
        map[100] = 9.7;
        map[110] = 11.0;
        map[120] = 12.3;
        map[130] = 13.8;
        map[140] = 15.3;
        map[150] = 16.9;
        map[160] = 18.7;
        return map;
    }

    TrainParams initRB425Params() const {
        TrainParams params;
        params.weight = 138;
        params.mf = 1.04;
        params.length = 67.5;
        params.decl = 1.0;
        params.vmax = 160 / 3.6;
        params.recovery = 0.6;
        params.rotWeight = params.weight * params.mf;
        params.traction = initRB425Traction();
        params.resistance = initRB425Resistance();
        return params;
    }

//    void initVehicleVariables(const MSVehicle *const pVehicle, MSCFModel_Rail::VehicleVariables *pVariables)const;

};


