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
/// @file    MSVTypeProbe.cpp
/// @author  Tino Morenz
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 24.10.2007
///
// Writes positions of vehicles that have a certain (named) type
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/WrappingCommand.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSLane.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>

#include "MSVTypeProbe.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSVTypeProbe::MSVTypeProbe(const std::string& id,
                           const std::string& vType,
                           OutputDevice& od, SUMOTime frequency)
    : Named(id), myVType(vType), myOutputDevice(od), myFrequency(frequency) {
    MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(this);
    myOutputDevice.writeXMLHeader("vehicle-type-probes", "vtypeprobe_file.xsd");
}


MSVTypeProbe::~MSVTypeProbe() {
}


SUMOTime
MSVTypeProbe::execute(SUMOTime currentTime) {
    myOutputDevice.openTag(SUMO_TAG_TIMESTEP);
    myOutputDevice.writeAttr(SUMO_ATTR_TIME, time2string(currentTime));
    myOutputDevice.writeAttr(SUMO_ATTR_ID, getID());
    myOutputDevice.writeAttr("vType", myVType);
    MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
    for (MSVehicleControl::constVehIt it = vc.loadedVehBegin(); it != vc.loadedVehEnd(); ++it) {
        const SUMOVehicle* veh = it->second;
        const MSVehicle* microVeh = dynamic_cast<const MSVehicle*>(veh);
        if (myVType == "" || myVType == veh->getVehicleType().getID()) {
            if (veh->isOnRoad()) {
                Position pos = veh->getPosition();
                myOutputDevice.openTag(SUMO_TAG_VEHICLE);
                myOutputDevice.writeAttr(SUMO_ATTR_ID, veh->getID());
                if (microVeh != nullptr) {
                    myOutputDevice.writeAttr(SUMO_ATTR_LANE, microVeh->getLane()->getID());
                }
                myOutputDevice.writeAttr(SUMO_ATTR_POSITION, veh->getPositionOnLane());
                myOutputDevice.writeAttr(SUMO_ATTR_X, pos.x());
                myOutputDevice.writeAttr(SUMO_ATTR_Y, pos.y());
                if (MSNet::getInstance()->hasElevation()) {
                    myOutputDevice.writeAttr(SUMO_ATTR_Z, pos.z());
                }
                if (GeoConvHelper::getFinal().usingGeoProjection()) {
                    GeoConvHelper::getFinal().cartesian2geo(pos);
                    myOutputDevice.setPrecision(gPrecisionGeo);
                    myOutputDevice.writeAttr(SUMO_ATTR_LAT, pos.y());
                    myOutputDevice.writeAttr(SUMO_ATTR_LON, pos.x());
                    myOutputDevice.setPrecision();
                }
                myOutputDevice.writeAttr(SUMO_ATTR_SPEED, veh->getSpeed());
                myOutputDevice.closeTag();
            }
        }
    }
    myOutputDevice.closeTag();
    return myFrequency;
}


/****************************************************************************/
