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
/// @file    ROJTRRouter.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Computes routes using junction turning percentages
/****************************************************************************/
#include <config.h>

#include <router/RONet.h>
#include "ROJTRRouter.h"
#include "ROJTREdge.h"
#include <utils/common/MsgHandler.h>


// ===========================================================================
// method definitions
// ===========================================================================
ROJTRRouter::ROJTRRouter(bool unbuildIsWarningOnly, bool acceptAllDestinations,
                         int maxEdges, bool ignoreClasses,
                         bool allowLoops,
                         bool discountSources) :
    SUMOAbstractRouter<ROEdge, ROVehicle>("JTRRouter", unbuildIsWarningOnly, &ROEdge::getTravelTimeStatic, nullptr, false, false),
    myUnbuildIsWarningOnly(unbuildIsWarningOnly),
    myAcceptAllDestination(acceptAllDestinations), myMaxEdges(maxEdges),
    myIgnoreClasses(ignoreClasses),
    myAllowLoops(allowLoops),
    myDiscountSources(discountSources)
{ }


ROJTRRouter::~ROJTRRouter() {}


bool
ROJTRRouter::compute(const ROEdge* from, const ROEdge* to,
                     const ROVehicle* const vehicle,
                     SUMOTime time, ConstROEdgeVector& into, bool silent) {
    const ROJTREdge* current = static_cast<const ROJTREdge*>(from);
    if (myDiscountSources && current->getSourceFlow() <= 0) {
        return true;
    }
    double timeS = STEPS2TIME(time);
    std::set<const ROEdge*> avoidEdges;
    // route until a sinks has been found
    while (current != nullptr && current != to &&
            (!current->isSink() || current == from || current->getSourceFlow() > 0) &&
            (int)into.size() < myMaxEdges) {
        into.push_back(current);
        const_cast<ROJTREdge*>(current)->changeSourceFlow(-1);
        if (!myAllowLoops) {
            avoidEdges.insert(current);
        }
        timeS += current->getTravelTime(vehicle, timeS);
        current = current->chooseNext(myIgnoreClasses ? nullptr : vehicle, timeS, avoidEdges);
        assert(myIgnoreClasses || current == 0 || !current->prohibits(vehicle));
    }
    // check whether no valid ending edge was found
    if (current == nullptr || (int) into.size() >= myMaxEdges) {
        if (myAcceptAllDestination) {
            return true;
        } else {
            if (!silent) {
                MsgHandler* mh = myUnbuildIsWarningOnly ? MsgHandler::getWarningInstance() : MsgHandler::getErrorInstance();
                mh->inform("The route starting at edge '" + from->getID() + "' could not be closed.");
            }
            return false;
        }
    }
    // append the sink
    if (current != nullptr) {
        into.push_back(current);
    }
    return true;
}


/****************************************************************************/
