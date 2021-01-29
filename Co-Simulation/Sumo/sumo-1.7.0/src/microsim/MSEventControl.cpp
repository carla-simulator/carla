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
/// @file    MSEventControl.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Matthias Heppner
/// @date    Mon, 12 Mar 2001
///
// Stores time-dependant events and executes them at the proper time
/****************************************************************************/
#include <config.h>

#include <cassert>
#include "MSEventControl.h"
#include <utils/common/MsgHandler.h>
#include <utils/common/Command.h>
#include "MSNet.h"


// ===========================================================================
// member definitions
// ===========================================================================
MSEventControl::MSEventControl() :
    myEvents() {}


MSEventControl::~MSEventControl() {
    // delete the events
    while (!myEvents.empty()) {
        Event e = myEvents.top();
        delete e.first;
        myEvents.pop();
    }
}


void
MSEventControl::addEvent(Command* operation, SUMOTime execTimeStep) {
    myEvents.push(Event(operation, execTimeStep));
}


void
MSEventControl::execute(SUMOTime execTime) {
    // Execute all events that are scheduled for execTime.
    while (!myEvents.empty()) {
        Event currEvent = myEvents.top();
        if (currEvent.second < 0) {
            currEvent.second = execTime;
        }
        if (currEvent.second < execTime + DELTA_T) {
            Command* command = currEvent.first;
            myEvents.pop();
            SUMOTime time = 0;
            try {
                time = command->execute(execTime);
            } catch (...) {
                delete command;
                throw;
            }

            // Delete nonrecurring events, reinsert recurring ones
            // with new execution time = execTime + returned offset.
            if (time <= 0) {
                if (time < 0) {
                    WRITE_WARNING("Command returned negative repeat number; will be deleted.");
                }
                delete currEvent.first;
            } else {
                currEvent.second += time;
                myEvents.push(currEvent);
            }
        } else {
            break;
        }
    }
}


bool
MSEventControl::isEmpty() {
    return myEvents.empty();
}


void
MSEventControl::clearState(SUMOTime currentTime, SUMOTime newTime) {
    std::vector<Event> keep;
    while (!myEvents.empty()) {
        Event currEvent = myEvents.top();
        myEvents.pop();
        SUMOTime newExecTime = currEvent.first->shiftTime(currentTime, currEvent.second, newTime);
        if (newExecTime >= 0) {
            keep.push_back(std::make_pair(currEvent.first, newExecTime));
        } else {
            delete currEvent.first;
        }
    }
    for (Event e : keep) {
        myEvents.push(e);
    }
}

/****************************************************************************/
