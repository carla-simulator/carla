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
/// @file    MSDispatch.cpp
/// @author  Jakob Erdmann
/// @date    16.12.2019
///
// An algorithm that performs dispatch for the taxi device
/****************************************************************************/
#include <config.h>

#include <limits>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/transportables/MSTransportable.h>
#include "MSRoutingEngine.h"
#include "MSDispatch.h"

//#define DEBUG_RESERVATION
//#define DEBUG_DISPATCH
//#define DEBUG_SERVABLE
//#define DEBUG_TRAVELTIME
//#define DEBUG_DETOUR
//#define DEBUG_COND2(obj) (obj->getID() == "p0")
#define DEBUG_COND2(obj) (true)


// ===========================================================================
// Reservation methods
// ===========================================================================

std::string
Reservation::getID() const {
    return toString(persons);
}

// ===========================================================================
// MSDispatch methods
// ===========================================================================

MSDispatch::MSDispatch(const std::map<std::string, std::string>& params) :
    Parameterised(params),
    myOutput(nullptr) {
    const std::string outFile = OptionsCont::getOptions().getString("device.taxi.dispatch-algorithm.output");
    if (outFile != "") {
        myOutput = &OutputDevice::getDevice(outFile);
        myOutput->writeXMLHeader("DispatchInfo", "");
    }
}


Reservation*
MSDispatch::addReservation(MSTransportable* person,
                           SUMOTime reservationTime,
                           SUMOTime pickupTime,
                           const MSEdge* from, double fromPos,
                           const MSEdge* to, double toPos,
                           const std::string& group) {
    // no new reservation nedded if the person can be added to an existing group
    Reservation* result = nullptr;
    bool added = false;
    auto it = myGroupReservations.find(group);
    if (it != myGroupReservations.end()) {
        // try to add to existing reservation
        for (Reservation* res : it->second) {
            if (res->persons.count(person) == 0
                    && res->from == from
                    && res->to == to
                    && res->fromPos == fromPos
                    && res->toPos == toPos) {
                res->persons.insert(person);
                result = res;
                added = true;
                break;
            }
        }
    }
    if (!added) {
        Reservation* newRes = new Reservation({person}, reservationTime, pickupTime, from, fromPos, to, toPos, group);
        myGroupReservations[group].push_back(newRes);
        result = newRes;
    }
    myHasServableReservations = true;
#ifdef DEBUG_RESERVATION
    if (DEBUG_COND2(person)) std::cout << SIMTIME
                                           << " addReservation p=" << person->getID()
                                           << " rT=" << time2string(reservationTime)
                                           << " pT=" << time2string(pickupTime)
                                           << " from=" << from->getID() << " fromPos=" << fromPos
                                           << " to=" << to->getID() << " toPos=" << toPos
                                           << " group=" << group
                                           << "\n";
#endif
    return result;
}

std::vector<Reservation*>
MSDispatch::getReservations() {
    std::vector<Reservation*> reservations;
    for (const auto& it : myGroupReservations) {
        reservations.insert(reservations.end(), it.second.begin(), it.second.end());
    }
    return reservations;
}


void
MSDispatch::servedReservation(const Reservation* res) {
    auto it = myGroupReservations.find(res->group);
    if (it == myGroupReservations.end()) {
        throw ProcessError("Inconsistent group reservations.");
    }
    auto it2 = std::find(it->second.begin(), it->second.end(), res);
    if (it2 == it->second.end()) {
        throw ProcessError("Inconsistent group reservations (2).");
    }
    delete *it2;
    it->second.erase(it2);
    if (it->second.empty()) {
        myGroupReservations.erase(it);
    }
}


SUMOTime
MSDispatch::computePickupTime(SUMOTime t, const MSDevice_Taxi* taxi, const Reservation& res, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router) {
    ConstMSEdgeVector edges;
    router.compute(taxi->getHolder().getEdge(), taxi->getHolder().getPositionOnLane(),
                   res.from, res.fromPos, &taxi->getHolder(), t, edges);
    return TIME2STEPS(router.recomputeCosts(edges, &taxi->getHolder(), t));
}


double
MSDispatch::computeDetourTime(SUMOTime t, SUMOTime viaTime, const MSDevice_Taxi* taxi,
                              const MSEdge* from, double fromPos,
                              const MSEdge* via, double viaPos,
                              const MSEdge* to, double toPos,
                              SUMOAbstractRouter<MSEdge, SUMOVehicle>& router,
                              double& timeDirect) {
    ConstMSEdgeVector edges;
    if (timeDirect < 0) {
        router.compute(from, fromPos, to, toPos, &taxi->getHolder(), t, edges);
        timeDirect = router.recomputeCosts(edges, &taxi->getHolder(), fromPos, toPos, t);
        edges.clear();
    }

    router.compute(from, fromPos, via, viaPos, &taxi->getHolder(), t, edges);
    const double start = STEPS2TIME(t);
    const double leg1 = router.recomputeCosts(edges, &taxi->getHolder(), fromPos, viaPos, t);
#ifdef DEBUG_DETOUR
    std::cout << "        leg1=" << toString(edges) << " startPos=" << fromPos << " toPos=" << viaPos << " time=" << leg1 << "\n";
#endif
    const double wait = MAX2(0.0, STEPS2TIME(viaTime) - (start + leg1));
    edges.clear();
    const SUMOTime timeContinue = TIME2STEPS(start + leg1 + wait);
    router.compute(via, viaPos, to, toPos, &taxi->getHolder(), timeContinue, edges);
    const double leg2 = router.recomputeCosts(edges, &taxi->getHolder(), viaPos, toPos, timeContinue);
    const double timeDetour = leg1 + wait + leg2;
#ifdef DEBUG_DETOUR
    std::cout << "        leg2=" << toString(edges) << " startPos=" << viaPos << " toPos=" << toPos << " time=" << leg2 << "\n";
    std::cout << "    t=" << STEPS2TIME(t) << " vt=" << STEPS2TIME(viaTime)
              << " from=" << from->getID() << " to=" << to->getID() << " via=" << via->getID()
              << " direct=" << timeDirect << " detour=" << timeDetour << " wait=" << wait << "\n";
#endif
    return timeDetour;
}

/****************************************************************************/
