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
/// @file    MSInternalJunction.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Wed, 12 Dez 2001
///
// junction.
/****************************************************************************/
#include <config.h>

#include "MSInternalJunction.h"
#include "MSRightOfWayJunction.h"
#include "MSLane.h"
#include "MSEdge.h"
#include "MSJunctionLogic.h"
#include <algorithm>
#include <cassert>
#include <cmath>


// ===========================================================================
// method definitions
// ===========================================================================
MSInternalJunction::MSInternalJunction(const std::string& id,
                                       SumoXMLNodeType type,
                                       const Position& position,
                                       const PositionVector& shape,
                                       std::vector<MSLane*> incoming,
                                       std::vector<MSLane*> internal)
    : MSLogicJunction(id, type, position, shape, "", incoming, internal) {}



MSInternalJunction::~MSInternalJunction() {}


void
MSInternalJunction::postloadInit() {
    if (myIncomingLanes.size() == 0) {
        throw ProcessError("Internal junction " + getID() + " has no incoming lanes");
    }
    // the first lane in the list of incoming lanes is special. It defines the
    // link that needs to do all the checking for this internal junction
    const MSLane* specialLane = myIncomingLanes[0];
    assert(specialLane->getLinkCont().size() == 1);
    MSLink* thisLink = specialLane->getLinkCont()[0];
    const MSRightOfWayJunction* parent = dynamic_cast<const MSRightOfWayJunction*>(specialLane->getEdge().getToJunction());
    if (parent == nullptr) {
        // parent has type traffic_light_unregulated
        return;
    }
    const int ownLinkIndex = specialLane->getIncomingLanes()[0].viaLink->getIndex();
    const MSLogicJunction::LinkBits& response = parent->getLogic()->getResponseFor(ownLinkIndex);
    // inform links where they have to report approaching vehicles to
    //std::cout << " special=" << specialLane->getID() << " incoming=" << toString(myIncomingLanes) << " internal=" << toString(myInternalLanes) << "\n";
    for (std::vector<MSLane*>::iterator i = myInternalLanes.begin(); i != myInternalLanes.end(); ++i) {
        const MSLinkCont& lc = (*i)->getLinkCont();
        for (MSLinkCont::const_iterator q = lc.begin(); q != lc.end(); ++q) {
            if ((*q)->getViaLane() != nullptr) {
                const int foeIndex = (*i)->getIncomingLanes()[0].viaLink->getIndex();
                //std::cout << "       response=" << response << " index=" << ownLinkIndex << " foeIndex=" << foeIndex << " ibct=" << indirectBicycleTurn(specialLane, thisLink, *i, *q) << "\n";
                if (response.test(foeIndex) || indirectBicycleTurn(specialLane, thisLink, *i, *q)) {
                    // only respect vehicles before internal junctions if they
                    // have priority (see the analogous foeLinks.test() when
                    // initializing myLinkFoeInternalLanes in MSRightOfWayJunction
                    // Indirect left turns for bicycles are a special case
                    // because they both intersect on their second part with the first part of the other one
                    // and only one of the has priority
                    myInternalLaneFoes.push_back(*i);
                }
                myInternalLaneFoes.push_back((*q)->getViaLane());
            } else {
                myInternalLaneFoes.push_back(*i);
            }
            //std::cout << "  i=" << (*i)->getID() << " qLane=" << (*q)->getLane()->getID() << " qVia=" << Named::getIDSecure((*q)->getViaLane()) << " foes=" << toString(myInternalLaneFoes) << "\n";
        }

    }
    for (std::vector<MSLane*>::const_iterator i = myIncomingLanes.begin() + 1; i != myIncomingLanes.end(); ++i) {
        MSLane* l = *i;
        const MSLinkCont& lc = l->getLinkCont();
        for (MSLinkCont::const_iterator j = lc.begin(); j != lc.end(); ++j) {
            MSLane* via = (*j)->getViaLane();
            if (std::find(myInternalLanes.begin(), myInternalLanes.end(), via) == myInternalLanes.end()) {
                continue;
            }
            myInternalLinkFoes.push_back(*j);
        }
    }
    // thisLinks is itself an exitLink of the preceding internal lane
    thisLink->setRequestInformation(ownLinkIndex, true, false, myInternalLinkFoes, myInternalLaneFoes, thisLink->getViaLane()->getLogicalPredecessorLane());
    assert(thisLink->getViaLane()->getLinkCont().size() == 1);
    MSLink* exitLink = thisLink->getViaLane()->getLinkCont()[0];
    exitLink->setRequestInformation(ownLinkIndex, false, false, std::vector<MSLink*>(),
                                    myInternalLaneFoes, thisLink->getViaLane());
    for (const auto& ili : exitLink->getLane()->getIncomingLanes()) {
        if (ili.lane->getEdge().isWalkingArea()) {
            exitLink->addWalkingAreaFoeExit(ili.lane);
            break;
        }
    }
    for (std::vector<MSLink*>::const_iterator k = myInternalLinkFoes.begin(); k != myInternalLinkFoes.end(); ++k) {
        thisLink->addBlockedLink(*k);
        (*k)->addBlockedLink(thisLink);
    }
}


bool
MSInternalJunction::indirectBicycleTurn(const MSLane* specialLane, const MSLink* thisLink, const MSLane* foeFirstPart, const MSLink* foeLink) const {
    if (specialLane->getPermissions() == SVC_BICYCLE && foeFirstPart->getPermissions() == SVC_BICYCLE
            && thisLink->getDirection() == LinkDirection::LEFT && foeLink->getDirection() == LinkDirection::LEFT
            && thisLink->getViaLane() != nullptr
            && thisLink->getViaLane()->getShape().intersects(foeFirstPart->getShape())) {
        return true;
    } else {
        return false;
    }
}


/****************************************************************************/
