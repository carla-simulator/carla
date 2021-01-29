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
/// @file    MSQueueExport.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @date    2012-04-26
///
// Export the queueing length in front of a junction (very experimental!)
/****************************************************************************/
#include <config.h>

#include <microsim/MSEdgeControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSGlobals.h>
#include <utils/iodevices/OutputDevice.h>
#include "MSQueueExport.h"
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>


// ===========================================================================
// method definitions
// ===========================================================================
void
MSQueueExport::write(OutputDevice& of, SUMOTime timestep) {
    of.openTag("data").writeAttr("timestep", time2string(timestep));
    writeEdge(of);
    of.closeTag();
}


void
MSQueueExport::writeEdge(OutputDevice& of) {
    of.openTag("lanes");
    MSEdgeControl& ec = MSNet::getInstance()->getEdgeControl();
    const MSEdgeVector& edges = ec.getEdges();
    for (MSEdgeVector::const_iterator e = edges.begin(); e != edges.end(); ++e) {
        MSEdge& edge = **e;
        const std::vector<MSLane*>& lanes = edge.getLanes();
        for (std::vector<MSLane*>::const_iterator lane = lanes.begin(); lane != lanes.end(); ++lane) {
            writeLane(of, **lane);
        }
    }
    of.closeTag();
}


void
MSQueueExport::writeLane(OutputDevice& of, const MSLane& lane) {
    // maximum of all vehicle waiting times
    double queueing_time = 0.0;
    // back of last stopped vehicle (XXX does not check for continuous queue)
    double queueing_length = 0.0;
    // back of last slow vehicle (XXX does not check for continuous queue)
    double queueing_length2 = 0.0;
    const double threshold_velocity = 5 / 3.6; // slow

    if (!lane.empty()) {
        for (MSLane::VehCont::const_iterator it_veh = lane.myVehicles.begin(); it_veh != lane.myVehicles.end(); ++it_veh) {
            const MSVehicle& veh = **it_veh;
            if (!veh.isOnRoad()) {
                continue;
            }

            if (veh.getWaitingSeconds() > 0) {
                queueing_time = MAX2(veh.getWaitingSeconds(), queueing_time);
                const double veh_back_to_lane_end = (lane.getLength() - veh.getPositionOnLane()) + veh.getVehicleType().getLength();
                queueing_length = MAX2(veh_back_to_lane_end, queueing_length);
            }

            //Experimental
            if (veh.getSpeed() < (threshold_velocity) && (veh.getPositionOnLane() > (veh.getLane()->getLength()) * 0.25)) {
                const double veh_back_to_lane_end = (lane.getLength() - veh.getPositionOnLane()) + veh.getVehicleType().getLength();
                queueing_length2 = MAX2(veh_back_to_lane_end, queueing_length2);
            }
        }
    }

    //Output
    if (queueing_length > 1 || queueing_length2 > 1) {
        of.openTag("lane").writeAttr("id", lane.getID()).writeAttr("queueing_time", queueing_time).writeAttr("queueing_length", queueing_length);
        of.writeAttr("queueing_length_experimental", queueing_length2).closeTag();
    }
}


/****************************************************************************/
