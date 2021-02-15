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
/// @file    MSDispatch_GreedyShared.cpp
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
#include "MSDispatch_GreedyShared.h"

//#define DEBUG_DISPATCH
//#define DEBUG_COND2(obj) (obj->getID() == "p0")
#define DEBUG_COND2(obj) (true)


// ===========================================================================
// MSDispatch_GreedyShared methods
// ===========================================================================

int
MSDispatch_GreedyShared::dispatch(MSDevice_Taxi* taxi, std::vector<Reservation*>::iterator& resIt, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, std::vector<Reservation*>& reservations) {
    const Reservation* const res = *resIt;
#ifdef DEBUG_DISPATCH
    if (DEBUG_COND2(person)) {
        std::cout << SIMTIME << " dispatch taxi=" << taxi->getHolder().getID() << " person=" << toString(res->persons) << "\n";
    }
#endif
    const int capacityLeft = taxi->getHolder().getVehicleType().getPersonCapacity() - (int)res->persons.size();
    const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    // check whether the ride can be shared
    int shareCase = 0;
    Reservation* res2 = nullptr;
    double absLoss = 0;
    double relLoss = 0;
    double absLoss2 = 0;
    double relLoss2 = 0;
    double directTime = -1; // only computed once for res
    double directTime2 = -1;
    for (auto it2 = resIt + 1; it2 != reservations.end(); it2++) {
        res2 = *it2;
        if (capacityLeft < (int)res2->persons.size()) {
            continue;
        }
        // res picks up res2 on the way
        directTime2 = -1; // reset for new candidate
        const SUMOTime startPickup = MAX2(now, res->pickupTime);
        const double detourTime = computeDetourTime(startPickup, res2->pickupTime, taxi,
                                  res->from, res->fromPos, res2->from, res2->fromPos, res->to, res->toPos, router, directTime);
        const double absLossPickup = detourTime - directTime;
        const double relLossPickup = absLossPickup / directTime;

#ifdef DEBUG_DISPATCH
        if (DEBUG_COND2(person)) std::cout << "  consider sharing ride with " << toString(res2->persons)
                                               << " absLossPickup=" << absLossPickup
                                               << " relLossPickup=" << relLossPickup
                                               << "\n";
#endif
        if (absLossPickup < myAbsoluteLossThreshold && relLossPickup < myRelativeLossThreshold) {
            const SUMOTime startDropOff = MAX2(now, res2->pickupTime);
            double directTimeTmp = -1; // direct time from picking up res2 to dropping of res
            // case 1: res2 is dropped off before res (more detour for res)
            double detourTime2 = computeDetourTime(startDropOff, startDropOff, taxi,
                                                   res2->from, res2->fromPos, res2->to, res2->toPos, res->to, res->toPos, router, directTimeTmp);
            const double absLoss_c1 = absLossPickup + (detourTime2 - directTimeTmp);
            const double relLoss_c1 = absLoss_c1 / directTime;

            // case 2: res2 is dropped off after res (detour for res2)
            double detourTime3 = computeDetourTime(startDropOff, startDropOff, taxi,
                                                   res2->from, res2->fromPos, res->to, res->toPos, res2->to, res2->toPos, router, directTime2);
            const double absLoss_c2 = detourTime3 - directTime2;
            const double relLoss_c2 = absLoss_c2 / directTime2;

            if (absLoss_c2 <= absLoss_c1 && absLoss_c2 < myAbsoluteLossThreshold && relLoss_c2 < myRelativeLossThreshold) {
                shareCase = 2;
                taxi->dispatchShared({res, res2, res, res2});
                absLoss = absLossPickup;
                relLoss = relLossPickup;
                absLoss2 = absLoss_c2;
                relLoss2 = relLoss_c2;
            } else if (absLoss_c1 < myAbsoluteLossThreshold && relLoss_c1 < myRelativeLossThreshold) {
                shareCase = 1;
                taxi->dispatchShared({res, res2, res2, res});
                absLoss = absLoss_c1;
                relLoss = relLoss_c1;
                absLoss2 = 0;
                relLoss2 = 0;
            }
            if (shareCase != 0) {
                reservations.erase(it2); // (it before it2) stays valid
                break;
            } else {
#ifdef DEBUG_DISPATCH
                if (DEBUG_COND2(person)) std::cout << "     rejected:"
                                                       << " absLoss_c1=" << absLoss_c1
                                                       << " relLoss_c1=" << relLoss_c1
                                                       << " absLoss_c2=" << absLoss_c2
                                                       << " relLoss_c2=" << relLoss_c2
                                                       << "\n";
#endif
            }
        }
    }
    if (shareCase != 0) {
        if (myOutput != nullptr) {
            myOutput->writeXMLHeader("DispatchInfo_GreedyShared", "");
            myOutput->openTag("dispatchShared");
            myOutput->writeAttr("time", time2string(now));
            myOutput->writeAttr("id", taxi->getHolder().getID());
            myOutput->writeAttr("persons", toString(res->persons));
            myOutput->writeAttr("sharingPersons", toString(res2->persons));
            myOutput->writeAttr("type", shareCase);
            myOutput->writeAttr("absLoss", absLoss);
            myOutput->writeAttr("relLoss", relLoss);
            myOutput->writeAttr("absLoss2", absLoss2);
            myOutput->writeAttr("relLoss2", relLoss2);
            myOutput->closeTag();
        }
#ifdef DEBUG_DISPATCH
        if (DEBUG_COND2(person)) std::cout << "  sharing ride with " << toString(res2->persons)
                                               << " type=" << shareCase
                                               << " absLoss=" << absLoss << " relLoss=" << relLoss
                                               << " absLoss2=" << absLoss2 << " relLoss2=" << relLoss2
                                               << "\n";
#endif
        servedReservation(res2); // deleting res2
    } else {
        taxi->dispatch(*res);
    }
    servedReservation(res); // deleting res
    resIt = reservations.erase(resIt);
    return shareCase == 0 ? 1 : 2;
}


/****************************************************************************/
