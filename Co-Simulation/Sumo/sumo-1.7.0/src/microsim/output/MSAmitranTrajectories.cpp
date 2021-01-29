/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSAmitranTrajectories.cpp
/// @author  Michael Behrisch
/// @date    13.03.2014
///
// Realises dumping the complete network state
/****************************************************************************/
#include <config.h>

#include <microsim/MSVehicleControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSGlobals.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/emissions/PollutantsInterface.h>
#include "MSAmitranTrajectories.h"

// ===========================================================================
// static member definitions
// ===========================================================================
std::set<std::string> MSAmitranTrajectories::myWrittenTypes;
std::map<std::string, int> MSAmitranTrajectories::myWrittenVehicles;


// ===========================================================================
// method definitions
// ===========================================================================
void
MSAmitranTrajectories::write(OutputDevice& of, const SUMOTime timestep) {
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    for (MSVehicleControl::constVehIt v = vc.loadedVehBegin(); v != vc.loadedVehEnd(); ++v) {
        writeVehicle(of, *v->second, timestep);
    }
}


void
MSAmitranTrajectories::writeVehicle(OutputDevice& of, const SUMOVehicle& veh, const SUMOTime timestep) {
    if (veh.isOnRoad()) {
        const std::string& type = veh.getVehicleType().getID();
        if (myWrittenTypes.count(type) == 0) {
            of.openTag(SUMO_TAG_ACTORCONFIG).writeAttr(SUMO_ATTR_ID, veh.getVehicleType().getNumericalID());
            const SUMOEmissionClass c = veh.getVehicleType().getEmissionClass();
            if (c != 0) {
                of.writeAttr(SUMO_ATTR_VEHICLECLASS, PollutantsInterface::getAmitranVehicleClass(c));
                of.writeAttr("fuel", PollutantsInterface::getFuel(c));
                of.writeAttr(SUMO_ATTR_EMISSIONCLASS, "Euro" + toString(PollutantsInterface::getEuroClass(c)));
                const double weight = PollutantsInterface::getWeight(c);
                if (weight > 0.) {
                    of.writeAttr(SUMO_ATTR_WEIGHT, int(weight / 10. + 0.5));
                }
            }
            of.writeAttr(SUMO_ATTR_REF, type).closeTag();
            myWrittenTypes.insert(type);
        }
        if (myWrittenVehicles.count(veh.getID()) == 0) {
            const int index = (int)myWrittenVehicles.size();
            of.openTag(SUMO_TAG_VEHICLE).writeAttr(SUMO_ATTR_ID, index)
            .writeAttr(SUMO_ATTR_ACTORCONFIG, veh.getVehicleType().getNumericalID())
            .writeAttr(SUMO_ATTR_STARTTIME, STEPS2MS(veh.getDeparture()));
            of.writeAttr(SUMO_ATTR_REF, veh.getID()).closeTag();
            myWrittenVehicles[veh.getID()] = index;
        }
        of.openTag(SUMO_TAG_MOTIONSTATE).writeAttr(SUMO_ATTR_VEHICLE, myWrittenVehicles[veh.getID()])
        .writeAttr(SUMO_ATTR_SPEED, int(100.*veh.getSpeed() + 0.5))
        .writeAttr(SUMO_ATTR_TIME, STEPS2MS(timestep))
        .writeAttr(SUMO_ATTR_ACCELERATION, int(1000.*veh.getAcceleration() + 0.5));
        of.closeTag();
    }
}


/****************************************************************************/
