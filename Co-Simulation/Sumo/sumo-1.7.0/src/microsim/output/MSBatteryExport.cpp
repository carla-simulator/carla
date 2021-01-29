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
/// @file    MSBatteryExport.cpp
/// @author  Mario Krumnow
/// @author  Tamas Kurczveil
/// @author  Pablo Alvarez Lopez
/// @date    20-12-13
///
// Realises dumping Battery Data
/****************************************************************************/
#include <config.h>

#include <microsim/MSEdgeControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <utils/iodevices/OutputDevice.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/devices/MSDevice_Battery.h>
#include "MSBatteryExport.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
MSBatteryExport::write(OutputDevice& of, SUMOTime timestep, int precision) {
    of.openTag(SUMO_TAG_TIMESTEP).writeAttr(SUMO_ATTR_TIME, time2string(timestep));
    of.setPrecision(precision);

    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    MSVehicleControl::constVehIt it = vc.loadedVehBegin();
    MSVehicleControl::constVehIt end = vc.loadedVehEnd();
    for (; it != end; ++it) {
        const MSVehicle* veh = static_cast<const MSVehicle*>((*it).second);

        if (!(veh->isOnRoad() || veh->isParking() || veh->isRemoteControlled())) {
            continue;
        }

        std::string fclass = veh->getVehicleType().getID();
        fclass = fclass.substr(0, fclass.find_first_of("@"));

        if (static_cast<MSDevice_Battery*>(veh->getDevice(typeid(MSDevice_Battery))) != nullptr) {
            MSDevice_Battery* batteryToExport = dynamic_cast<MSDevice_Battery*>(veh->getDevice(typeid(MSDevice_Battery)));
            if (batteryToExport->getMaximumBatteryCapacity() > 0) {
                // Open Row
                of.openTag(SUMO_TAG_VEHICLE);
                // Write ID
                of.writeAttr(SUMO_ATTR_ID, veh->getID());
                // Write consum
                of.writeAttr(SUMO_ATTR_ENERGYCONSUMED, batteryToExport->getConsum());
                // Write Actual battery capacity
                of.writeAttr(SUMO_ATTR_ACTUALBATTERYCAPACITY, batteryToExport->getActualBatteryCapacity());
                // Write Maximum battery capacity
                of.writeAttr(SUMO_ATTR_MAXIMUMBATTERYCAPACITY, batteryToExport->getMaximumBatteryCapacity());
                // Write Charging Station ID
                of.writeAttr(SUMO_ATTR_CHARGINGSTATIONID, batteryToExport->getChargingStationID());
                // Write Charge charged in the Battery
                of.writeAttr(SUMO_ATTR_ENERGYCHARGED, batteryToExport->getEnergyCharged());
                // Write ChargeInTransit
                if (batteryToExport->isChargingInTransit()) {
                    of.writeAttr(SUMO_ATTR_ENERGYCHARGEDINTRANSIT, batteryToExport->getEnergyCharged());
                } else {
                    of.writeAttr(SUMO_ATTR_ENERGYCHARGEDINTRANSIT, 0.00);
                }
                // Write ChargingStopped
                if (batteryToExport->isChargingStopped()) {
                    of.writeAttr(SUMO_ATTR_ENERGYCHARGEDSTOPPED, batteryToExport->getEnergyCharged());
                } else {
                    of.writeAttr(SUMO_ATTR_ENERGYCHARGEDSTOPPED, 0.00);
                }
                // Write Speed
                of.writeAttr(SUMO_ATTR_SPEED, veh->getSpeed());
                // Write Acceleration
                of.writeAttr(SUMO_ATTR_ACCELERATION, veh->getAcceleration());

                Position pos = veh->getPosition();
                of.writeAttr(SUMO_ATTR_X, veh->getPosition().x());
                of.writeAttr(SUMO_ATTR_Y, veh->getPosition().y());

                // Write Lane ID / edge ID
                if (MSGlobals::gUseMesoSim) {
                    of.writeAttr(SUMO_ATTR_EDGE, veh->getEdge()->getID());
                } else {
                    of.writeAttr(SUMO_ATTR_LANE, veh->getLane()->getID());
                }
                // Write vehicle position in the lane
                of.writeAttr(SUMO_ATTR_POSONLANE, veh->getPositionOnLane());
                // Write Time stopped (In all cases)
                of.writeAttr(SUMO_ATTR_TIMESTOPPED, batteryToExport->getVehicleStopped());
                // Close Row
                of.closeTag();
            }
        }
    }
    of.closeTag();
}
