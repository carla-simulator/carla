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
/// @file    MSChargingStation.cpp
/// @author  Daniel Krajzewicz
/// @author  Tamas Kurczveil
/// @author  Pablo Alvarez Lopez
/// @date    20-12-13
///
// Chargin Station for Electric vehicles
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utils/common/StringUtils.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <microsim/MSVehicleType.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/devices/MSDevice_Battery.h>
#include <microsim/MSNet.h>
#include "MSChargingStation.h"
#include "MSTrigger.h"


// ===========================================================================
// member method definitions
// ===========================================================================

MSChargingStation::MSChargingStation(const std::string& chargingStationID, MSLane& lane, double startPos, double endPos,
                                     const std::string& name,
                                     double chargingPower, double efficency, bool chargeInTransit, double chargeDelay) :
    MSStoppingPlace(chargingStationID, std::vector<std::string>(), lane, startPos, endPos, name),
    myChargingPower(0),
    myEfficiency(0),
    myChargeInTransit(chargeInTransit),
    myChargeDelay(0),
    myChargingVehicle(false),
    myTotalCharge(0) {
    if (chargingPower < 0)
        WRITE_WARNING("Parameter " + toString(SUMO_ATTR_CHARGINGPOWER) + " for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " is invalid (" + toString(getChargingPower()) + ").")
        else {
            myChargingPower = chargingPower;
        }

    if (efficency < 0 || efficency > 1) {
        WRITE_WARNING("Parameter " + toString(SUMO_ATTR_EFFICIENCY) + " for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " is invalid (" + toString(getEfficency()) + ").")
    } else {
        myEfficiency = efficency;
    }

    if (chargeDelay < 0) {
        WRITE_WARNING("Parameter " + toString(SUMO_ATTR_CHARGEDELAY) + " for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " is invalid (" + toString(getEfficency()) + ").")
    } else {
        myChargeDelay = chargeDelay;
    }

    if (getBeginLanePosition() > getEndLanePosition()) {
        WRITE_WARNING(toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " doesn't have a valid range (" + toString(getBeginLanePosition()) + " < " + toString(getEndLanePosition()) + ").");
    }
}


MSChargingStation::~MSChargingStation() {
}


double
MSChargingStation::getChargingPower() const {
    return myChargingPower;
}


double
MSChargingStation::getEfficency() const {
    return myEfficiency;
}


bool
MSChargingStation::getChargeInTransit() const {
    return myChargeInTransit;
}


double
MSChargingStation::getChargeDelay() const {
    return myChargeDelay;
}


void
MSChargingStation::setChargingPower(double chargingPower) {
    if (chargingPower < 0) {
        WRITE_WARNING("New " + toString(SUMO_ATTR_CHARGINGPOWER) + " for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " isn't valid (" + toString(chargingPower) + ").")
    } else {
        myChargingPower = chargingPower;
    }
}


void
MSChargingStation::setEfficency(double efficency) {
    if (efficency < 0 || efficency > 1) {
        WRITE_WARNING("New " + toString(SUMO_ATTR_EFFICIENCY) + " for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " isn't valid (" + toString(efficency) + ").")
    } else {
        myEfficiency = efficency;
    }
}


void
MSChargingStation::setChargeInTransit(bool chargeInTransit) {
    myChargeInTransit = chargeInTransit;
}


void
MSChargingStation::setChargeDelay(double chargeDelay) {
    if (chargeDelay < 0) {
        WRITE_WARNING("New " + toString(SUMO_ATTR_CHARGEDELAY) + " for " + toString(SUMO_TAG_CHARGING_STATION) + " with ID = " + getID() + " isn't valid (" + toString(chargeDelay) + ").")
    } else {
        myChargeDelay = chargeDelay;
    }
}


void
MSChargingStation::setChargingVehicle(bool value) {
    myChargingVehicle = value;
}


bool
MSChargingStation::vehicleIsInside(const double position) const {
    if ((position >= getBeginLanePosition()) && (position <= getEndLanePosition())) {
        return true;
    } else {
        return false;
    }
}


bool
MSChargingStation::isCharging() const {
    return myChargingVehicle;
}


void
MSChargingStation::addChargeValueForOutput(double WCharged, MSDevice_Battery* battery) {
    std::string status = "";
    if (battery->getChargingStartTime() > myChargeDelay) {
        if (battery->getHolder().getSpeed() < battery->getStoppingTreshold()) {
            status = "chargingStopped";
        } else if (myChargeInTransit == true) {
            status = "chargingInTransit";
        } else {
            status = "noCharging";
        }
    } else {
        if (myChargeInTransit == true) {
            status = "waitingChargeInTransit";
        } else if (battery->getHolder().getSpeed() < battery->getStoppingTreshold()) {
            status = "waitingChargeStopped";
        } else {
            status = "noWaitingCharge";
        }
    }
    // update total charge
    myTotalCharge += WCharged;
    // create charge row and insert it in myChargeValues
    charge C(MSNet::getInstance()->getCurrentTimeStep(), battery->getHolder().getID(), battery->getHolder().getVehicleType().getID(),
             status, WCharged, battery->getActualBatteryCapacity(), battery->getMaximumBatteryCapacity(),
             myChargingPower, myEfficiency, myTotalCharge);
    myChargeValues.push_back(C);
}


void
MSChargingStation::writeChargingStationOutput(OutputDevice& output) {
    output.openTag(SUMO_TAG_CHARGING_STATION);
    output.writeAttr(SUMO_ATTR_ID, myID);
    output.writeAttr(SUMO_ATTR_TOTALENERGYCHARGED, myTotalCharge);
    output.writeAttr(SUMO_ATTR_CHARGINGSTEPS, myChargeValues.size());
    // start writting
    if (myChargeValues.size() > 0) {
        // First calculate charge for every vehicle
        std::vector<double> charge;
        std::vector<std::pair<SUMOTime, SUMOTime> > vectorBeginEndCharge;
        SUMOTime firsTimeStep = myChargeValues.at(0).timeStep;
        // set first value
        charge.push_back(0);
        vectorBeginEndCharge.push_back(std::pair<SUMOTime, SUMOTime>(firsTimeStep, 0));
        // iterate over charging values
        for (std::vector<MSChargingStation::charge>::const_iterator i = myChargeValues.begin(); i != myChargeValues.end(); i++) {
            // update chargue
            charge.back() += i->WCharged;
            // update end time
            vectorBeginEndCharge.back().second = i->timeStep;
            // update timestep of charge
            firsTimeStep += 1000;
            // check if charge is continuous. If not, open a new vehicle tag
            if (((i + 1) != myChargeValues.end()) && (((i + 1)->timeStep) != firsTimeStep)) {
                // set new firsTimeStep of charge
                firsTimeStep = (i + 1)->timeStep;
                charge.push_back(0);
                vectorBeginEndCharge.push_back(std::pair<SUMOTime, SUMOTime>(firsTimeStep, 0));
            }
        }
        // now write values
        firsTimeStep = myChargeValues.at(0).timeStep;
        int vehicleCounter = 0;
        // open tag for first vehicle and write id and type of vehicle
        output.openTag(SUMO_TAG_VEHICLE);
        output.writeAttr(SUMO_ATTR_ID, myChargeValues.at(0).vehicleID);
        output.writeAttr(SUMO_ATTR_TYPE, myChargeValues.at(0).vehicleType);
        output.writeAttr(SUMO_ATTR_TOTALENERGYCHARGED_VEHICLE, charge.at(0));
        output.writeAttr(SUMO_ATTR_CHARGINGBEGIN, time2string(vectorBeginEndCharge.at(0).first));
        output.writeAttr(SUMO_ATTR_CHARGINGEND, time2string(vectorBeginEndCharge.at(0).second));
        // iterate over charging values
        for (std::vector<MSChargingStation::charge>::const_iterator i = myChargeValues.begin(); i != myChargeValues.end(); i++) {
            // open tag for timestep and write all parameters
            output.openTag(SUMO_TAG_STEP);
            output.writeAttr(SUMO_ATTR_TIME, time2string(i->timeStep));
            // charge values
            output.writeAttr(SUMO_ATTR_CHARGING_STATUS, i->status);
            output.writeAttr(SUMO_ATTR_ENERGYCHARGED, i->WCharged);
            output.writeAttr(SUMO_ATTR_PARTIALCHARGE, i->totalEnergyCharged);
            // charging values of charging station in this timestep
            output.writeAttr(SUMO_ATTR_CHARGINGPOWER, i->chargingPower);
            output.writeAttr(SUMO_ATTR_EFFICIENCY, i->chargingEfficiency);
            // battery status of vehicle
            output.writeAttr(SUMO_ATTR_ACTUALBATTERYCAPACITY, i->actualBatteryCapacity);
            output.writeAttr(SUMO_ATTR_MAXIMUMBATTERYCAPACITY, i->maxBatteryCapacity);
            // close tag timestep
            output.closeTag();
            // update timestep of charge
            firsTimeStep += 1000;
            // check if charge is continuous. If not, open a new vehicle tag
            if (((i + 1) != myChargeValues.end()) && (((i + 1)->timeStep) != firsTimeStep)) {
                // set new firsTimeStep of charge
                firsTimeStep = (i + 1)->timeStep;
                // update counter
                vehicleCounter++;
                // close previous vehicle tag
                output.closeTag();
                // open tag for new vehicle and write id and type of vehicle
                output.openTag(SUMO_TAG_VEHICLE);
                output.writeAttr(SUMO_ATTR_ID, (i + 1)->vehicleID);
                output.writeAttr(SUMO_ATTR_TYPE, (i + 1)->vehicleType);
                output.writeAttr(SUMO_ATTR_TOTALENERGYCHARGED_VEHICLE, charge.at(vehicleCounter));
                output.writeAttr(SUMO_ATTR_CHARGINGBEGIN, vectorBeginEndCharge.at(vehicleCounter).first);
                output.writeAttr(SUMO_ATTR_CHARGINGEND, vectorBeginEndCharge.at(vehicleCounter).second);
            }
        }
        // close vehicle tag
        output.closeTag();
    }
    // close charging station tag
    output.closeTag();
}


/****************************************************************************/
