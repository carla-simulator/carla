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
/// @file    MSDispatch_RouteExtension.cpp
/// @author  Michael Behrisch
/// @date    06.08.2020
///
// An algorithm that performs dispatch for the taxi device
/****************************************************************************/
#include <config.h>

#include <limits>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/transportables/MSTransportable.h>
#include "MSRoutingEngine.h"
#include "MSDispatch_RouteExtension.h"

//#define DEBUG_DISPATCH
//#define DEBUG_COND2(obj) (obj->getID() == "p0")
#define DEBUG_COND2(obj) (true)


// ===========================================================================
// method definitions
// ===========================================================================
void
MSDispatch_RouteExtension::findInsertionPoint(std::vector<const Reservation*>::iterator& resIt, EdgePosVector::iterator& edgeIt,
        const EdgePosVector::iterator& edgeEnd, ConstMSEdgeVector& route,
        const MSEdge* newEdge, const double newPos) const {
    for (const MSEdge* edge : route) {
        while (edgeIt != edgeEnd && edge == edgeIt->first) {
            if (edge == newEdge && edgeIt->second > newPos) {
                break;
            }
            resIt++;
            edgeIt++;
        }
        if (edge == newEdge) {
            break;
        }
    }
}


int
MSDispatch_RouteExtension::dispatch(MSDevice_Taxi* taxi, std::vector<Reservation*>::iterator& resIt, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, std::vector<Reservation*>& reservations) {
    const Reservation* const res = *resIt;
#ifdef DEBUG_DISPATCH
    if (DEBUG_COND2(person)) {
        std::cout << SIMTIME << " dispatch taxi=" << taxi->getHolder().getID() << " person=" << toString(res->persons) << "\n";
    }
#endif
    const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    int capacityLeft = taxi->getHolder().getVehicleType().getPersonCapacity() - (int)res->persons.size();
    std::vector<const Reservation*> sequence{ res, res };
    std::vector<const Reservation*> toRemove{ res };
    EdgePosVector posSequence({ std::make_pair(res->from, res->fromPos), std::make_pair(res->to, res->toPos) });
    const Reservation* first = sequence.front();
    const Reservation* last = sequence.back();
    ConstMSEdgeVector route;
    router.compute(first->from, first->fromPos, last->to, last->toPos, &taxi->getHolder(), MAX2(now, first->pickupTime), route);
    // check whether the ride can be shared
    for (auto it2 = resIt + 1; it2 != reservations.end();) {
        Reservation* const res2 = *it2;
        if (capacityLeft < (int)res2->persons.size()) {
            it2++;
            continue;
        }
        // check whether res2 picks up or gets picked up on the way
        ConstMSEdgeVector route2;
        // TODO It may be more efficient to check first whether from and to are already in the route
        router.compute(res2->from, res2->fromPos, res2->to, res2->toPos, &taxi->getHolder(), MAX2(now, res2->pickupTime), route2);
        const bool pickup = std::find(route.begin(), route.end(), res2->from) != route.end();
        const bool dropoff = std::find(route.begin(), route.end(), res2->to) != route.end();
#ifdef DEBUG_DISPATCH
        if (DEBUG_COND2(person)) std::cout << "  consider sharing ride with " << toString(res2->persons)
                                               << " pickup=" << pickup << " startFirst=" << (std::find(route2.begin(), route2.end(), first->from) != route2.end())
                                               << " dropoff=" << dropoff << " endLast=" << (std::find(route2.begin(), route2.end(), last->to) != route2.end())
                                               << "\n";
#endif
        if ((pickup || std::find(route2.begin(), route2.end(), first->from) != route2.end()) &&
                (dropoff || std::find(route2.begin(), route2.end(), last->to) != route2.end())) {
            std::vector<const Reservation*>::iterator resSeqIt = sequence.begin();
            EdgePosVector::iterator edgeIt = posSequence.begin();
            if (pickup) {
                // new reservation gets picked up
                findInsertionPoint(resSeqIt, edgeIt, posSequence.end(), route, res2->from, res2->fromPos);
            }
            resSeqIt = sequence.insert(resSeqIt, res2) + 1;
            edgeIt = posSequence.insert(edgeIt, std::make_pair(res2->from, res2->fromPos)) + 1;
            if (dropoff) {
                // new reservation drops off and route continues
                findInsertionPoint(resSeqIt, edgeIt, posSequence.end(), route, res2->to, res2->toPos);
                sequence.insert(resSeqIt, res2);
                posSequence.insert(edgeIt, std::make_pair(res2->from, res2->fromPos));
            } else {
                // new reservation ends last
                sequence.push_back(res2);
                posSequence.push_back(std::make_pair(res2->to, res2->toPos));
            }
            toRemove.push_back(res2);
            it2 = reservations.erase(it2); // (resIt before it2) stays valid
            // TODO we have more capacity if some pickup is after an earlier dropoff
            capacityLeft -= (int)res2->persons.size();
            if (capacityLeft == 0) {
                break;
            }
            route.clear();
            first = sequence.front();
            last = sequence.back();
            // TODO this is wrong for non linear networks! should be reusing the route snippets from above
            router.compute(first->from, first->fromPos, last->to, last->toPos, &taxi->getHolder(), MAX2(now, first->pickupTime), route);
        } else {
            it2++;
        }
    }
    if (sequence.size() > 2) {
        taxi->dispatchShared(sequence);
        if (myOutput != nullptr) {
            myOutput->writeXMLHeader("DispatchInfo_RouteExtension", "");
            myOutput->openTag("dispatchShared");
            myOutput->writeAttr("time", time2string(now));
            myOutput->writeAttr("id", taxi->getHolder().getID());
            myOutput->writeAttr("persons", toString(res->persons));
            myOutput->writeAttr("sharingPersons", toString(sequence));
            myOutput->writeAttr("type", "routeExtension");
            myOutput->closeTag();
        }
#ifdef DEBUG_DISPATCH
        if (DEBUG_COND2(person)) std::cout << "  sharing ride with " << toString(sequence)
                                               << "\n";
#endif
    } else {
        taxi->dispatch(*res);
    }
    for (const Reservation* r : toRemove) {
        servedReservation(r); // deleting r
    }
    resIt = reservations.erase(resIt);
    return (int)toRemove.size();
}


/****************************************************************************/
