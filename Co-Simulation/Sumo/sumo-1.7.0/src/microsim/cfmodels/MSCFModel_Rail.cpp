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
/// @file    MSCFModel_Rail.cpp
/// @author  Gregor L\"ammel
/// @author  Leander Flamm
/// @date    Tue, 08 Feb 2017
///
// <description missing>
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/MsgHandler.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSVehicle.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include "MSCFModel_Rail.h"

#define G  9.80665

MSCFModel_Rail::MSCFModel_Rail(const MSVehicleType* vtype) :
    MSCFModel(vtype) {
    const std::string trainType = vtype->getParameter().getCFParamString(SUMO_ATTR_TRAIN_TYPE, "NGT400");
    if (trainType.compare("RB425") == 0) {
        myTrainParams = initRB425Params();
    } else if (trainType.compare("RB628") == 0) {
        myTrainParams = initRB628Params();
    } else if (trainType.compare("NGT400") == 0) {
        myTrainParams = initNGT400Params();
    } else if (trainType.compare("NGT400_16") == 0) {
        myTrainParams = initNGT400_16Params();
    } else if (trainType.compare("ICE1") == 0) {
        myTrainParams = initICE1Params();
    } else if (trainType.compare("REDosto7") == 0) {
        myTrainParams = initREDosto7Params();
    } else if (trainType.compare("Freight") == 0) {
        myTrainParams = initFreightParams();
    } else if (trainType.compare("ICE3") == 0) {
        myTrainParams = initICE3Params();
    } else {
        WRITE_ERROR("Unknown train type: " + trainType + ". Exiting!");
        throw ProcessError();
    }
    // override with user values
    if (vtype->wasSet(VTYPEPARS_MAXSPEED_SET)) {
        myTrainParams.vmax = vtype->getMaxSpeed();
    }
    if (vtype->wasSet(VTYPEPARS_LENGTH_SET)) {
        myTrainParams.length = vtype->getLength();
    }
    myTrainParams.decl = vtype->getParameter().getCFParam(SUMO_ATTR_DECEL, myTrainParams.decl);
    setMaxDecel(myTrainParams.decl);
    setEmergencyDecel(vtype->getParameter().getCFParam(SUMO_ATTR_EMERGENCYDECEL, myTrainParams.decl + 0.3));
    // update type parameters so they are shown correctly in the gui (if defaults from trainType are used)
    const_cast<MSVehicleType*>(vtype)->setMaxSpeed(myTrainParams.vmax);
    const_cast<MSVehicleType*>(vtype)->setLength(myTrainParams.length);

}

MSCFModel_Rail::~MSCFModel_Rail() { }

double MSCFModel_Rail::followSpeed(const MSVehicle* const veh, double speed, double gap,
                                   double /* predSpeed */, double /* predMaxDecel*/, const MSVehicle* const /*pred*/) const {

    // followSpeed module is used for the simulation of moving block operations. The safety gap is chosen similar to the existing german
    // system CIR-ELKE (based on LZB). Other implementations of moving block systems may differ, but for now no appropriate parameter
    // can be set (would be per lane, not per train) -> hard-coded

    // @note: default train minGap of 5 is already subtracted from gap
    if (speed >= 30 / 3.6) {
        // safety distance for higher speeds (>= 30 km/h)
        gap = MAX2(0.0, gap + veh->getVehicleType().getMinGap() - 50);
    }

    const double vsafe = maximumSafeStopSpeed(gap, speed, false, TS); // absolute breaking distance
    const double vmin = minNextSpeed(speed, veh);
    const double vmax = maxNextSpeed(speed, veh);

    if (MSGlobals::gSemiImplicitEulerUpdate) {
        return MIN2(vsafe, vmax);
    } else {
        // ballistic
        // XXX: the euler variant can break as strong as it wishes immediately! The ballistic cannot, refs. #2575.
        return MAX2(MIN2(vsafe, vmax), vmin);
    }
}

int
MSCFModel_Rail::getModelID() const {
    return SUMO_TAG_CF_RAIL;
}

MSCFModel*
MSCFModel_Rail::duplicate(const MSVehicleType* vtype) const {
    return new MSCFModel_Rail(vtype);
}

double MSCFModel_Rail::maxNextSpeed(double speed, const MSVehicle* const veh) const {

    if (speed >= myTrainParams.vmax) {
        return myTrainParams.vmax;
    }

    double targetSpeed = myTrainParams.vmax;

    double res = getInterpolatedValueFromLookUpMap(speed, &(myTrainParams.resistance)); // kN

    double slope = veh->getSlope();
    double gr = myTrainParams.weight * G * sin(DEG2RAD(slope)); //kN

    double totalRes = res + gr; //kN

    double trac = getInterpolatedValueFromLookUpMap(speed, &(myTrainParams.traction)); // kN

    double a;
    if (speed < targetSpeed) {
        a = (trac - totalRes) / myTrainParams.rotWeight; //kN/t == N/kg
    } else {
        a = 0.;
        if (totalRes > trac) {
            a = (trac - totalRes) / myTrainParams.rotWeight; //kN/t == N/kg
        }
    }

    double maxNextSpeed = speed + a * DELTA_T / 1000.;

//    std::cout << veh->getID() << " speed: " << (speed*3.6) << std::endl;

    return maxNextSpeed;
}

double MSCFModel_Rail::minNextSpeed(double speed, const MSVehicle* const veh) const {

    const double slope = veh->getSlope();
    const double gr = myTrainParams.weight * G * sin(DEG2RAD(slope)); //kN
    const double res = getInterpolatedValueFromLookUpMap(speed, &(myTrainParams.resistance)); // kN
    const double totalRes = res + gr; //kN
    const double a = myTrainParams.decl + totalRes / myTrainParams.rotWeight;
    const double vMin = speed - a * DELTA_T / 1000.;
    if (MSGlobals::gSemiImplicitEulerUpdate) {
        return MAX2(vMin, 0.);
    } else {
        // NOTE: ballistic update allows for negative speeds to indicate a stop within the next timestep
        return vMin;
    }

}


double
MSCFModel_Rail::minNextSpeedEmergency(double speed, const MSVehicle* const veh) const {
    return minNextSpeed(speed, veh);
}


double MSCFModel_Rail::getInterpolatedValueFromLookUpMap(double speed, const LookUpMap* lookUpMap) const {
    speed = speed * 3.6; // lookup values in km/h
    std::map<double, double>::const_iterator low, prev;
    low = lookUpMap->lower_bound(speed);

    if (low == lookUpMap->end()) { //speed > max speed
        return (lookUpMap->rbegin())->second;
    }

    if (low == lookUpMap->begin()) {
        return low->second;
    }

    prev = low;
    --prev;

    double range = low->first - prev->first;
    double dist = speed - prev->first;
    assert(range > 0);
    assert(dist > 0);

    double weight = dist / range;

    double res = (1 - weight) * prev->second + weight * low->second;

    return res;

}



//void
//MSCFModel_Rail::initVehicleVariables(const MSVehicle *const veh, MSCFModel_Rail::VehicleVariables *pVariables) const {
//
//    pVariables->setInitialized();
//
//}

double MSCFModel_Rail::getSpeedAfterMaxDecel(double /* speed */) const {

//    //TODO: slope not known here
//    double gr = 0; //trainParams.weight * 9.81 * edge.grade
//
//    double a = 0;//trainParams.decl - gr/trainParams.rotWeight;
//
//    return speed + a * DELTA_T / 1000.;
    WRITE_ERROR("function call not allowd for rail model. Exiting!");
    throw ProcessError();
}

MSCFModel::VehicleVariables* MSCFModel_Rail::createVehicleVariables() const {
    VehicleVariables* ret = new VehicleVariables();
    return ret;
}


double MSCFModel_Rail::finalizeSpeed(MSVehicle* const veh, double vPos) const {
    return MSCFModel::finalizeSpeed(veh, vPos);
}

double MSCFModel_Rail::freeSpeed(const MSVehicle* const /* veh */, double /* speed */, double dist, double targetSpeed,
                                 const bool onInsertion) const {

//    MSCFModel_Rail::VehicleVariables *vars = (MSCFModel_Rail::VehicleVariables *) veh->getCarFollowVariables();
//    if (vars->isNotYetInitialized()) {
//        initVehicleVariables(veh, vars);
//    }

    //TODO: signals, coasting, ...

    if (MSGlobals::gSemiImplicitEulerUpdate) {
        // adapt speed to succeeding lane, no reaction time is involved
        // when breaking for y steps the following distance g is covered
        // (drive with v in the final step)
        // g = (y^2 + y) * 0.5 * b + y * v
        // y = ((((sqrt((b + 2.0*v)*(b + 2.0*v) + 8.0*b*g)) - b)*0.5 - v)/b)
        const double v = SPEED2DIST(targetSpeed);
        if (dist < v) {
            return targetSpeed;
        }
        const double b = ACCEL2DIST(myDecel);
        const double y = MAX2(0.0, ((sqrt((b + 2.0 * v) * (b + 2.0 * v) + 8.0 * b * dist) - b) * 0.5 - v) / b);
        const double yFull = floor(y);
        const double exactGap = (yFull * yFull + yFull) * 0.5 * b + yFull * v + (y > yFull ? v : 0.0);
        const double fullSpeedGain = (yFull + (onInsertion ? 1. : 0.)) * ACCEL2SPEED(myTrainParams.decl);
        return DIST2SPEED(MAX2(0.0, dist - exactGap) / (yFull + 1)) + fullSpeedGain + targetSpeed;
    } else {
        WRITE_ERROR("Anything else than semi implicit euler update is not yet implemented. Exiting!");
        throw ProcessError();
    }
}

double MSCFModel_Rail::stopSpeed(const MSVehicle* const veh, const double speed, double gap) const {
    return MIN2(maximumSafeStopSpeed(gap, speed, false, TS), maxNextSpeed(speed, veh));
}
