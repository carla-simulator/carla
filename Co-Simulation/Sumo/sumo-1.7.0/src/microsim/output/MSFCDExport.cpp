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
/// @file    MSFCDExport.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @date    2012-04-26
///
// Realises dumping Floating Car Data (FCD) Data
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/geom/GeomHelper.h>
#include <libsumo/Helper.h>
#include <microsim/devices/MSDevice_FCD.h>
#include <microsim/devices/MSTransportableDevice_FCD.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSVehicleControl.h>
#include "MSFCDExport.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
MSFCDExport::write(OutputDevice& of, SUMOTime timestep, bool elevation) {
    const bool useGeo = OptionsCont::getOptions().getBool("fcd-output.geo");
    const bool signals = OptionsCont::getOptions().getBool("fcd-output.signals");
    const bool writeAccel = OptionsCont::getOptions().getBool("fcd-output.acceleration");
    const bool writeDistance = OptionsCont::getOptions().getBool("fcd-output.distance");
    std::vector<std::string> params = OptionsCont::getOptions().getStringVector("fcd-output.params");
    const SUMOTime period = string2time(OptionsCont::getOptions().getString("device.fcd.period"));
    const SUMOTime begin = string2time(OptionsCont::getOptions().getString("begin"));
    if (period > 0 && (timestep - begin) % period != 0) {
        return;
    }
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    const double radius = OptionsCont::getOptions().getFloat("device.fcd.radius");
    const bool filter = MSDevice_FCD::getEdgeFilter().size() > 0;
    std::set<const Named*> inRadius;
    if (radius > 0) {
        // collect all vehicles in radius around equipped vehicles
        for (MSVehicleControl::constVehIt it = vc.loadedVehBegin(); it != vc.loadedVehEnd(); ++it) {
            const SUMOVehicle* veh = it->second;
            MSDevice_FCD* fcdDevice = (MSDevice_FCD*)veh->getDevice(typeid(MSDevice_FCD));
            if (fcdDevice != nullptr
                    && (veh->isOnRoad() || veh->isParking() || veh->isRemoteControlled())
                    && (!filter || MSDevice_FCD::getEdgeFilter().count(veh->getEdge()) > 0)) {
                PositionVector shape;
                shape.push_back(veh->getPosition());
                libsumo::Helper::collectObjectsInRange(libsumo::CMD_GET_VEHICLE_VARIABLE, shape, radius, inRadius);
                libsumo::Helper::collectObjectsInRange(libsumo::CMD_GET_PERSON_VARIABLE, shape, radius, inRadius);
            }
        }
    }

    of.openTag("timestep").writeAttr(SUMO_ATTR_TIME, time2string(timestep));
    for (MSVehicleControl::constVehIt it = vc.loadedVehBegin(); it != vc.loadedVehEnd(); ++it) {
        const SUMOVehicle* veh = it->second;
        const MSVehicle* microVeh = dynamic_cast<const MSVehicle*>(veh);
        if ((veh->isOnRoad() || veh->isParking() || veh->isRemoteControlled())
                // only filter on normal edges
                && (!filter || MSDevice_FCD::getEdgeFilter().count(veh->getEdge()) > 0)
                && (veh->getDevice(typeid(MSDevice_FCD)) != nullptr || (radius > 0 && inRadius.count(veh) > 0))) {
            Position pos = veh->getPosition();
            if (useGeo) {
                of.setPrecision(gPrecisionGeo);
                GeoConvHelper::getFinal().cartesian2geo(pos);
            }
            of.openTag(SUMO_TAG_VEHICLE);
            of.writeAttr(SUMO_ATTR_ID, veh->getID());
            of.writeAttr(SUMO_ATTR_X, pos.x());
            of.writeAttr(SUMO_ATTR_Y, pos.y());
            if (elevation) {
                of.writeAttr(SUMO_ATTR_Z, pos.z());
            }
            of.writeAttr(SUMO_ATTR_ANGLE, GeomHelper::naviDegree(veh->getAngle()));
            of.writeAttr(SUMO_ATTR_TYPE, veh->getVehicleType().getID());
            of.writeAttr(SUMO_ATTR_SPEED, veh->getSpeed());
            of.writeAttr(SUMO_ATTR_POSITION, veh->getPositionOnLane());
            if (microVeh != nullptr) {
                of.writeAttr(SUMO_ATTR_LANE, microVeh->getLane()->getID());
            } else {
                of.writeAttr(SUMO_ATTR_EDGE, veh->getEdge()->getID());
            }
            of.writeAttr(SUMO_ATTR_SLOPE, veh->getSlope());
            if (microVeh != nullptr) {
                if (signals) {
                    of.writeAttr("signals", toString(microVeh->getSignals()));
                }
                if (writeAccel) {
                    of.writeAttr("acceleration", toString(microVeh->getAcceleration()));
                    if (MSGlobals::gSublane) {
                        of.writeAttr("accelerationLat", microVeh->getLaneChangeModel().getAccelerationLat());
                    }
                }
            }
            if (writeDistance) {
                double distance = veh->getEdge()->getDistance();
                if (microVeh != nullptr) {
                    if (microVeh->getLane()->isInternal()) {
                        distance += microVeh->getRoute().getDistanceBetween(0, microVeh->getPositionOnLane(),
                                    microVeh->getEdge(), &microVeh->getLane()->getEdge(), true, microVeh->getRoutePosition());
                    } else {
                        distance += microVeh->getPositionOnLane();
                    }
                } else {
                    distance += veh->getPositionOnLane();
                }
                // if the kilometrage runs counter to the edge direction edge->getDistance() is negative
                of.writeAttr("distance", fabs(distance));
            }
            for (const std::string& key : params) {
                const std::string value = veh->getParameter().getParameter(key);
                if (value != "") {
                    of.writeAttr(StringUtils::escapeXML(key), StringUtils::escapeXML(value));
                }
            }
            of.closeTag();
            // write persons and containers
            const MSEdge* edge = microVeh == nullptr ? veh->getEdge() : &veh->getLane()->getEdge();

            const std::vector<MSTransportable*>& persons = veh->getPersons();
            for (MSTransportable* person : persons) {
                writeTransportable(of, edge, person, true, SUMO_TAG_PERSON, useGeo, elevation);
            }
            const std::vector<MSTransportable*>& containers = veh->getContainers();
            for (MSTransportable* container : containers) {
                writeTransportable(of, edge, container, true, SUMO_TAG_CONTAINER, useGeo, elevation);
            }
        }
    }
    if (MSNet::getInstance()->getPersonControl().hasTransportables()) {
        // write persons
        MSEdgeControl& ec = MSNet::getInstance()->getEdgeControl();
        const MSEdgeVector& edges = ec.getEdges();
        for (MSEdgeVector::const_iterator e = edges.begin(); e != edges.end(); ++e) {
            if (filter && MSDevice_FCD::getEdgeFilter().count(*e) == 0) {
                continue;
            }
            const std::vector<MSTransportable*>& persons = (*e)->getSortedPersons(timestep);
            for (MSTransportable* person : persons) {
                writeTransportable(of, *e, person, inRadius.count(person) > 0, SUMO_TAG_PERSON, useGeo, elevation);
            }
        }
    }
    if (MSNet::getInstance()->getContainerControl().hasTransportables()) {
        // write containers
        MSEdgeControl& ec = MSNet::getInstance()->getEdgeControl();
        const std::vector<MSEdge*>& edges = ec.getEdges();
        for (std::vector<MSEdge*>::const_iterator e = edges.begin(); e != edges.end(); ++e) {
            if (filter && MSDevice_FCD::getEdgeFilter().count(*e) == 0) {
                continue;
            }
            const std::vector<MSTransportable*>& containers = (*e)->getSortedContainers(timestep);
            for (MSTransportable* container : containers) {
                writeTransportable(of, *e, container, inRadius.count(container) > 0, SUMO_TAG_CONTAINER, useGeo, elevation);
            }
        }
    }
    of.closeTag();
}


void
MSFCDExport::writeTransportable(OutputDevice& of, const MSEdge* e, MSTransportable* p, bool inRadius, SumoXMLTag tag, bool useGeo, bool elevation) {
    if (p->getDevice(typeid(MSTransportableDevice_FCD)) == nullptr && !inRadius) {
        return;
    }
    Position pos = p->getPosition();
    if (useGeo) {
        of.setPrecision(gPrecisionGeo);
        GeoConvHelper::getFinal().cartesian2geo(pos);
    }
    of.openTag(tag);
    of.writeAttr(SUMO_ATTR_ID, p->getID());
    of.writeAttr(SUMO_ATTR_X, pos.x());
    of.writeAttr(SUMO_ATTR_Y, pos.y());
    if (elevation) {
        of.writeAttr(SUMO_ATTR_Z, pos.z());
    }
    of.writeAttr(SUMO_ATTR_ANGLE, GeomHelper::naviDegree(p->getAngle()));
    of.writeAttr(SUMO_ATTR_SPEED, p->getSpeed());
    of.writeAttr(SUMO_ATTR_POSITION, p->getEdgePos());
    of.writeAttr(SUMO_ATTR_EDGE, e->getID());
    of.writeAttr(SUMO_ATTR_SLOPE, e->getLanes()[0]->getShape().slopeDegreeAtOffset(p->getEdgePos()));
    of.closeTag();
}


/****************************************************************************/
