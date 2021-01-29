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
/// @file    NBParking.cpp
/// @author  Jakob Erdmann
/// @date    Tue, 14 Nov 2017
///
// The representation of an imported parking area
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include "NBParking.h"
#include "NBEdge.h"
#include "NBEdgeCont.h"


// ===========================================================================
// method definitions
// ===========================================================================
NBParking::NBParking(const std::string& id, const std::string& edgeID, const std::string& name) :
    Named(id),
    myEdgeID(edgeID),
    myName(name) {
}

void
NBParking::write(OutputDevice& device, NBEdgeCont& ec) const {
    const NBEdge* e = ec.retrieve(myEdgeID);
    if (e != nullptr) {
        if ((e->getPermissions() & SVC_PASSENGER) == 0) {
            WRITE_WARNINGF("Ignoring parking area on edge '%' due to invalid permissions.", e->getID());
            return;
        }
        // keep minimum distance of 5m to junction corners
        const int cornerDistance = 5;
        int capacity = (int)((e->getFinalLength() - 2 * cornerDistance) / 7.5);
        if (capacity <= 0) {
            WRITE_WARNINGF("Ignoring parking area on edge '%' due to insufficient space.", e->getID());
            return;
        }
        int lane = 0;
        for (; lane < e->getNumLanes(); ++lane) {
            if ((e->getPermissions(lane) & SVC_PASSENGER) != 0) {
                break;
            }
        }
        device.openTag(SUMO_TAG_PARKING_AREA);
        device.writeAttr(SUMO_ATTR_ID, getID());
        device.writeAttr(SUMO_ATTR_LANE, e->getLaneID(lane));
        device.writeAttr(SUMO_ATTR_STARTPOS, cornerDistance);
        device.writeAttr(SUMO_ATTR_ENDPOS, -cornerDistance);
        device.writeAttr(SUMO_ATTR_ROADSIDE_CAPACITY, capacity);
        if (!myName.empty()) {
            device.writeAttr(SUMO_ATTR_NAME, myName);
        }
        device.closeTag();
    } else {
        WRITE_WARNINGF("could not find edge for parkingArea '%s'", getID());
    }
    // XXX else: prevent edge merging via --geometry.remove
}


void
NBParkingCont::addEdges2Keep(const OptionsCont& oc, std::set<std::string>& into) {
    if (oc.isSet("parking-output")) {
        for (NBParking& p : *this) {
            into.insert(p.getEdgeID());
        }
    }
}
