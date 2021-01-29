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
/// @file    MSLogicJunction.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Wed, 12 Dez 2001
///
// with one ore more logics.
/****************************************************************************/
#include <config.h>

#include "MSLinkCont.h"
#include "MSLogicJunction.h"
#include "MSLane.h"


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * methods from MSLogicJunction
 * ----------------------------------------------------------------------- */
MSLogicJunction::MSLogicJunction(const std::string& id,
                                 SumoXMLNodeType type,
                                 const Position& position,
                                 const PositionVector& shape,
                                 const std::string& name,
                                 std::vector<MSLane*> incoming,
                                 std::vector<MSLane*> internal):
    MSJunction(id, type, position, shape, name),
    myIncomingLanes(incoming),
    myInternalLanes(internal) {
}


MSLogicJunction::~MSLogicJunction() {}


void
MSLogicJunction::postloadInit() {
    /*
    if(getID()=="1565") {
        int bla = 0;
    }
    // inform links where they have to report approaching vehicles to
    int requestPos = 0;
    std::vector<MSLane*>::iterator i;
    // going through the incoming lanes...
    for(i=myIncomingLanes.begin(); i!=myIncomingLanes.end(); ++i) {
        const MSLinkCont &links = (*i)->getLinkCont();
        // ... set information for every link
        for(MSLinkCont::const_iterator j=links.begin(); j!=links.end(); j++) {
            (*j)->setRequestInformation(&myRequest, requestPos,
                &myRespond, requestPos/, clearInfo/);
            requestPos++;
        }
    }
    // set information for the internal lanes
    requestPos = 0;
    for(i=myInternalLanes.begin(); i!=myInternalLanes.end(); ++i) {
        // ... set information about participation
        static_cast<MSInternalLane*>(*i)->setParentJunctionInformation(
            &myInnerState, requestPos++);
    }
    */
}

const std::vector<MSLane*>
MSLogicJunction::getInternalLanes() const {
    // Besides the lanes im myInternal lanes, which are only the last parts of the connections,
    // this collects all lanes on the junction
    std::vector<MSLane*> allInternalLanes;
    for (std::vector<MSLane*>::const_iterator i = myInternalLanes.begin(); i != myInternalLanes.end(); ++i) {
        MSLane* l = *i;
        while (l != nullptr) {
            allInternalLanes.push_back(l);
            const std::vector<MSLane::IncomingLaneInfo> incoming = l->getIncomingLanes();
            if (incoming.size() == 0) {
                break;
            }
            assert(l->getIncomingLanes().size() == 1);
            l = l->getIncomingLanes()[0].lane;
            if (!l->isInternal()) {
                break;
            }
        }
    }
    return allInternalLanes;
}


/****************************************************************************/
