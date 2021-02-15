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
/// @file    MSFullExport.cpp
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2012-04-26
///
// Dumping a hugh List of Parameters available in the Simulation
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/emissions/HelpersHarmonoise.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include "MSFullExport.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
MSFullExport::write(OutputDevice& of, SUMOTime timestep) {
    of.openTag("data") << " timestep=\"" << time2string(timestep) << "\"";
    //Vehicles
    writeVehicles(of);
    //Edges
    writeEdge(of);
    //TrafficLights
    writeTLS(of, timestep);
    of.closeTag();
}


void
MSFullExport::writeVehicles(OutputDevice& of) {
    of.openTag("vehicles");
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    for (MSVehicleControl::constVehIt it = vc.loadedVehBegin(); it != vc.loadedVehEnd(); ++it) {
        const SUMOVehicle* veh = it->second;
        const MSVehicle* microVeh = dynamic_cast<const MSVehicle*>(veh);
        if (veh->isOnRoad()) {
            std::string fclass = veh->getVehicleType().getID();
            fclass = fclass.substr(0, fclass.find_first_of("@"));
            PollutantsInterface::Emissions emiss = PollutantsInterface::computeAll(
                    veh->getVehicleType().getEmissionClass(), veh->getSpeed(),
                    veh->getAcceleration(), veh->getSlope(),
                    veh->getEmissionParameters());
            of.openTag("vehicle").writeAttr("id", veh->getID()).writeAttr("eclass", PollutantsInterface::getName(veh->getVehicleType().getEmissionClass()));
            of.writeAttr("CO2", emiss.CO2).writeAttr("CO", emiss.CO).writeAttr("HC", emiss.HC).writeAttr("NOx", emiss.NOx);
            of.writeAttr("PMx", emiss.PMx).writeAttr("fuel", emiss.fuel).writeAttr("electricity", emiss.electricity);
            of.writeAttr("noise", HelpersHarmonoise::computeNoise(veh->getVehicleType().getEmissionClass(), veh->getSpeed(), veh->getAcceleration()));
            of.writeAttr("route", veh->getRoute().getID()).writeAttr("type", fclass);
            if (microVeh != nullptr) {
                of.writeAttr("waiting", microVeh->getWaitingSeconds());
                of.writeAttr("lane", microVeh->getLane()->getID());
            }
            of.writeAttr("pos", veh->getPositionOnLane()).writeAttr("speed", veh->getSpeed());
            of.writeAttr("angle", GeomHelper::naviDegree(veh->getAngle())).writeAttr("x", veh->getPosition().x()).writeAttr("y", veh->getPosition().y());
            of.closeTag();
        }
    }
    of.closeTag();
}

void
MSFullExport::writeEdge(OutputDevice& of) {
    of.openTag("edges");
    MSEdgeControl& ec = MSNet::getInstance()->getEdgeControl();
    const MSEdgeVector& edges = ec.getEdges();
    for (MSEdgeVector::const_iterator e = edges.begin(); e != edges.end(); ++e) {
        MSEdge& edge = **e;
        if (!MSGlobals::gUsingInternalLanes && !edge.isNormal()) {
            continue;
        }
        of.openTag("edge").writeAttr("id", edge.getID()).writeAttr("traveltime", edge.getCurrentTravelTime());
        const std::vector<MSLane*>& lanes = edge.getLanes();
        for (std::vector<MSLane*>::const_iterator lane = lanes.begin(); lane != lanes.end(); ++lane) {
            writeLane(of, **lane);
        }
        of.closeTag();
    }
    of.closeTag();
}


void
MSFullExport::writeLane(OutputDevice& of, const MSLane& lane) {

    of.openTag("lane").writeAttr("id", lane.getID()).writeAttr("CO", lane.getCOEmissions()).writeAttr("CO2", lane.getCO2Emissions());
    of.writeAttr("NOx", lane.getNOxEmissions()).writeAttr("PMx", lane.getPMxEmissions()).writeAttr("HC", lane.getHCEmissions());
    of.writeAttr("noise", lane.getHarmonoise_NoiseEmissions()).writeAttr("fuel", lane.getFuelConsumption());
    of.writeAttr("electricity", lane.getElectricityConsumption()).writeAttr("maxspeed", lane.getSpeedLimit());
    of.writeAttr("meanspeed", lane.getMeanSpeed()).writeAttr("occupancy", lane.getNettoOccupancy()).writeAttr("vehicle_count", lane.getVehicleNumber());
    of.closeTag();
}


void
MSFullExport::writeTLS(OutputDevice& of, SUMOTime /* timestep */) {
    of.openTag("tls");
    MSTLLogicControl& vc = MSNet::getInstance()->getTLSControl();
    std::vector<std::string> ids = vc.getAllTLIds();
    for (std::vector<std::string>::const_iterator id_it = ids.begin(); id_it != ids.end(); ++id_it) {
        MSTLLogicControl::TLSLogicVariants& vars = MSNet::getInstance()->getTLSControl().get(*id_it);
        const MSTrafficLightLogic::LaneVectorVector& lanes = vars.getActive()->getLaneVectors();

        std::vector<std::string> laneIDs;
        for (MSTrafficLightLogic::LaneVectorVector::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
            const MSTrafficLightLogic::LaneVector& llanes = (*i);
            for (MSTrafficLightLogic::LaneVector::const_iterator j = llanes.begin(); j != llanes.end(); ++j) {
                laneIDs.push_back((*j)->getID());
            }
        }

        std::string lane_output = "";
        for (int i1 = 0; i1 < (int)laneIDs.size(); ++i1) {
            lane_output += laneIDs[i1] + " ";
        }

        std::string state = vars.getActive()->getCurrentPhaseDef().getState();
        of.openTag("trafficlight").writeAttr("id", *id_it).writeAttr("state", state).closeTag();
    }
    of.closeTag();
}

