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
/// @file    MSRailSignalConstraint.cpp
/// @author  Jakob Erdmann
/// @date    August 2020
///
// A constraint on rail signal switching
/****************************************************************************/
#include <config.h>
#include <cassert>
#include <utility>

#include <microsim/MSLane.h>
#include <microsim/MSLink.h>
#include "MSRailSignal.h"
#include "MSRailSignalConstraint.h"

// ===========================================================================
// static value definitions
// ===========================================================================
std::map<const MSLink*, MSRailSignalConstraint_Predecessor::PassedTracker*> MSRailSignalConstraint_Predecessor::myTrackerLookup;

// ===========================================================================
// MSRailSignalConstraint method definitions
// ===========================================================================
void
MSRailSignalConstraint::cleanup() {
    MSRailSignalConstraint_Predecessor::cleanup();
}

// ===========================================================================
// MSRailSignalConstraint_Predecessor method definitions
// ===========================================================================
MSRailSignalConstraint_Predecessor::MSRailSignalConstraint_Predecessor(const MSRailSignal* signal, const std::string& tripId, int limit) :
    myTripId(tripId),
    myLimit(limit) {
    for (const auto& lv : signal->getLinks()) {
        for (const MSLink* link : lv) {
            PassedTracker* pt = nullptr;
            if (myTrackerLookup.count(link) == 0) {
                pt = new PassedTracker(link);
                myTrackerLookup[link] = pt;
            } else {
                pt = myTrackerLookup[link];
            }
            pt->raiseLimit(limit);
            myTrackers.push_back(pt);
        }
    }

}

void
MSRailSignalConstraint_Predecessor::cleanup() {
    for (auto item : myTrackerLookup) {
        delete item.second;
    }
    myTrackerLookup.clear();
}

bool
MSRailSignalConstraint_Predecessor::cleared() const {
    for (PassedTracker* pt : myTrackers) {
        if (pt->hasPassed(myTripId, myLimit)) {
            return true;
        }
    }
    return false;
}

MSRailSignalConstraint_Predecessor::PassedTracker::PassedTracker(const MSLink* link) :
    MSMoveReminder("PassedTracker_" + link->getViaLaneOrLane()->getID(), link->getViaLaneOrLane(), true),
    myPassed(1, ""),
    myLastIndex(0)
{ }

bool
MSRailSignalConstraint_Predecessor::PassedTracker::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification /*reason*/, const MSLane* /*enteredLane*/) {
    myLastIndex = (myLastIndex + 1) % myPassed.size();
    myPassed[myLastIndex] = veh.getParameter().getParameter("tripId", veh.getID());
    return true;
}

void
MSRailSignalConstraint_Predecessor::PassedTracker::raiseLimit(int limit) {
    while (limit > (int)myPassed.size()) {
        myPassed.insert(myPassed.begin() + myLastIndex + 1, "");
    }
}

bool
MSRailSignalConstraint_Predecessor::PassedTracker::hasPassed(const std::string& tripId, int limit) const {
    int i = myLastIndex;
    while (limit > 0) {
        if (myPassed[i] == tripId) {
            return true;
        }
        if (i == 0) {
            i = (int)myPassed.size() - 1;
        } else {
            i--;
        }
        limit--;
    }
    return false;
}


/****************************************************************************/
