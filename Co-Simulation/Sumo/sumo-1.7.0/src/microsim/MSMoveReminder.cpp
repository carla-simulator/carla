/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSMoveReminder.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2008-10-27
///
// Something on a lane to be noticed about vehicle movement
/****************************************************************************/
#include <config.h>

#include <string>
#include "MSLane.h"
#include "MSMoveReminder.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSMoveReminder::MSMoveReminder(const std::string& description, MSLane* const lane, const bool doAdd) :
    myLane(lane),
    myDescription(description)
#ifdef HAVE_FOX
    , myNotificationMutex(true)
#endif
{
    if (myLane != nullptr && doAdd) {
        // add reminder to lane
        myLane->addMoveReminder(this);
    }
}


void
MSMoveReminder::updateDetector(SUMOTrafficObject& veh, double entryPos, double leavePos,
                               SUMOTime entryTime, SUMOTime currentTime, SUMOTime leaveTime,
                               bool cleanUp) {
    // each vehicle is tracked linearly across its segment. For each vehicle,
    // the time and position of the previous call are maintained and only
    // the increments are sent to notifyMoveInternal
    if (entryTime > currentTime) {
        return; // calibrator may insert vehicles a tiny bit into the future; ignore those
    }
    auto j = myLastVehicleUpdateValues.find(&veh);
    if (j != myLastVehicleUpdateValues.end()) {
        // the vehicle already has reported its values before; use these
        // however, if this was called from prepareDetectorForWriting the time
        // only has a resolution of DELTA_T and might be invalid
        const SUMOTime previousEntryTime = j->second.first;
        if (previousEntryTime <= currentTime) {
            entryTime = previousEntryTime;
            entryPos = j->second.second;
        }
    }
    assert(entryTime <= currentTime);
    if ((entryTime < leaveTime) && (entryPos <= leavePos)) {
        const double timeOnLane = STEPS2TIME(currentTime - entryTime);
        const double speed = (leavePos - entryPos) / STEPS2TIME(leaveTime - entryTime);
        myLastVehicleUpdateValues[&veh] = std::pair<SUMOTime, double>(currentTime, entryPos + speed * timeOnLane);
        assert(timeOnLane >= 0);
        notifyMoveInternal(veh, timeOnLane, timeOnLane, speed, speed, speed * timeOnLane, speed * timeOnLane, 0.);
    } else {
        // it would be natrual to
        // assert(entryTime == leaveTime);
        // assert(entryPos == leavePos);
        // However, in the presence of calibrators, vehicles may jump a bit
        myLastVehicleUpdateValues[&veh] = std::pair<SUMOTime, double>(leaveTime, leavePos);
    }
    if (cleanUp) {
        // clean up after the vehicle has left the area of this reminder
        removeFromVehicleUpdateValues(veh);
    }
}


void
MSMoveReminder::removeFromVehicleUpdateValues(SUMOTrafficObject& veh) {
    myLastVehicleUpdateValues.erase(&veh);
}


/****************************************************************************/
