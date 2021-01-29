/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDispatch_TraCI.cpp
/// @author  Jakob Erdmann
/// @date    16.12.2019
///
// An algorithm that performs dispatch for the taxi device
/****************************************************************************/
#include <config.h>

#include <limits>
#include "MSDispatch_TraCI.h"

//#define DEBUG_RESERVATION
//#define DEBUG_DISPATCH
//#define DEBUG_SERVABLE
//#define DEBUG_TRAVELTIME
//#define DEBUG_DETOUR
//#define DEBUG_COND2(obj) (obj->getID() == "p0")
#define DEBUG_COND2(obj) (true)

// ===========================================================================
// MSDispatch_TraCI methods
// ===========================================================================

Reservation*
MSDispatch_TraCI::addReservation(MSTransportable* person,
                                 SUMOTime reservationTime,
                                 SUMOTime pickupTime,
                                 const MSEdge* from, double fromPos,
                                 const MSEdge* to, double toPos,
                                 const std::string& group) {
    Reservation* res = MSDispatch::addReservation(person, reservationTime, pickupTime, from, fromPos, to, toPos, group);
    if (!myReservationLookup.has(res)) {
        myReservationLookup.insert(toString(myReservationCount), res);
        myReservationCount++;
    }
    return res;
}

std::string
MSDispatch_TraCI::getReservationID(Reservation* res) {
    if (myReservationLookup.has(res)) {
        return myReservationLookup.getString(res);
    } else {
        throw InvalidArgument("Reservation is not known");
    }
}

void
MSDispatch_TraCI::interpretDispatch(MSDevice_Taxi* taxi, const std::vector<std::string>& reservationsIDs) {
    std::vector<const Reservation*> reservations;
    for (std::string resID : reservationsIDs) {
        if (myReservationLookup.hasString(resID)) {
            reservations.push_back(myReservationLookup.get(resID));
        } else {
            throw InvalidArgument("Reservation id '" + resID + "' is not known");
        }
    }
    if (reservations.size() == 1) {
        taxi->dispatch(*reservations.front());
    } else {
        taxi->dispatchShared(reservations);
    }
    // in case of ride sharing the same reservation may occur multiple times
    std::set<const Reservation*> unique(reservations.begin(), reservations.end());
    for (const Reservation* res : unique) {
        servedReservation(res);
    }
}


//
/****************************************************************************/
