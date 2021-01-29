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
/// @file    MSEmissionExport.cpp
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2012-04-26
///
// Realises dumping Emission Data
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/emissions/HelpersHarmonoise.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/devices/MSDevice_Emissions.h>
#include <mesosim/MEVehicle.h>
#include <microsim/MSVehicleControl.h>
#include "MSEmissionExport.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
MSEmissionExport::write(OutputDevice& of, SUMOTime timestep, int precision) {
    const SUMOTime period = string2time(OptionsCont::getOptions().getString("device.emissions.period"));
    const SUMOTime begin = string2time(OptionsCont::getOptions().getString("begin"));
    if (period > 0 && (timestep - begin) % period != 0) {
        return;
    }
    of.openTag("timestep").writeAttr("time", time2string(timestep));
    of.setPrecision(precision);
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    for (MSVehicleControl::constVehIt it = vc.loadedVehBegin(); it != vc.loadedVehEnd(); ++it) {
        const SUMOVehicle* veh = it->second;
        MSDevice_Emissions* emissionsDevice = (MSDevice_Emissions*)veh->getDevice(typeid(MSDevice_Emissions));
        if (emissionsDevice != nullptr && (veh->isOnRoad() || veh->isIdling())) {
            std::string fclass = veh->getVehicleType().getID();
            fclass = fclass.substr(0, fclass.find_first_of("@"));
            PollutantsInterface::Emissions emiss = PollutantsInterface::computeAll(
                    veh->getVehicleType().getEmissionClass(),
                    veh->getSpeed(), veh->getAcceleration(), veh->getSlope(),
                    veh->getEmissionParameters());
            of.openTag("vehicle").writeAttr("id", veh->getID()).writeAttr("eclass", PollutantsInterface::getName(veh->getVehicleType().getEmissionClass()));
            of.writeAttr("CO2", emiss.CO2).writeAttr("CO", emiss.CO).writeAttr("HC", emiss.HC).writeAttr("NOx", emiss.NOx);
            of.writeAttr("PMx", emiss.PMx).writeAttr("fuel", emiss.fuel).writeAttr("electricity", emiss.electricity);
            of.writeAttr("noise", HelpersHarmonoise::computeNoise(veh->getVehicleType().getEmissionClass(), veh->getSpeed(), veh->getAcceleration()));
            of.writeAttr("route", veh->getRoute().getID()).writeAttr("type", fclass);
            if (MSGlobals::gUseMesoSim) {
                const MEVehicle* mesoVeh = dynamic_cast<const MEVehicle*>(veh);
                of.writeAttr("waiting", mesoVeh->getWaitingSeconds());
                of.writeAttr("edge", veh->getEdge()->getID());
            } else {
                const MSVehicle* microVeh = dynamic_cast<const MSVehicle*>(veh);
                of.writeAttr("waiting", microVeh->getWaitingSeconds());
                of.writeAttr("lane", microVeh->getLane()->getID());
            }
            of.writeAttr("pos", veh->getPositionOnLane()).writeAttr("speed", veh->getSpeed());
            of.writeAttr("angle", GeomHelper::naviDegree(veh->getAngle())).writeAttr("x", veh->getPosition().x()).writeAttr("y", veh->getPosition().y());
            of.closeTag();
        }
    }
    of.setPrecision(gPrecision);
    of.closeTag();
}
