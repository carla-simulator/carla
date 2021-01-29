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
/// @file    MSRailCrossing.cpp
/// @author  Jakob Erdmann
/// @date    Dez 2015
///
// A rail signal logic
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utility>
#include <vector>
#include <bitset>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include "MSTrafficLightLogic.h"
#include "MSRailCrossing.h"
#include <microsim/MSLane.h>
#include "MSPhaseDefinition.h"
#include "MSTLLogicControl.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSRailCrossing::MSRailCrossing(MSTLLogicControl& tlcontrol,
                               const std::string& id, const std::string& programID, SUMOTime delay,
                               const std::map<std::string, std::string>& parameters) :
    MSSimpleTrafficLightLogic(tlcontrol, id, programID, TrafficLightType::RAIL_CROSSING, Phases(), 0, delay, parameters),
    // XXX make this configurable
    mySecurityGap(TIME2STEPS(15)),
    myMinGreenTime(TIME2STEPS(5)),
    /// XXX compute reasonable time depending on link length
    myYellowTime(TIME2STEPS(5)) {
    // dummy phase, used to avoid crashing in MSTrafficLightLogic::setTrafficLightSignals()
    myPhases.push_back(new MSPhaseDefinition(1, std::string(SUMO_MAX_CONNECTIONS, 'X')));
}

void
MSRailCrossing::init(NLDetectorBuilder&) {
    delete myPhases.front();
    myPhases.clear();
    myPhases.push_back(new MSPhaseDefinition(1, std::string(myLinks.size(), 'G')));
    myPhases.push_back(new MSPhaseDefinition(myYellowTime, std::string(myLinks.size(), 'y')));
    myPhases.push_back(new MSPhaseDefinition(1, std::string(myLinks.size(), 'r')));
    // init phases
    updateCurrentPhase();
    setTrafficLightSignals(MSNet::getInstance()->getCurrentTimeStep());
    myNumLinks = (int)myLinks.size();
}


MSRailCrossing::~MSRailCrossing() {}


// ----------- Handling of controlled links
void
MSRailCrossing::adaptLinkInformationFrom(const MSTrafficLightLogic& logic) {
    MSTrafficLightLogic::adaptLinkInformationFrom(logic);
    updateCurrentPhase();
}


// ------------ Switching and setting current rows
SUMOTime
MSRailCrossing::trySwitch() {
    SUMOTime nextTry = updateCurrentPhase();
    //if (getID() == "cluster_1088529493_1260626727") std::cout << " myStep=" << myStep << " nextTry=" << nextTry << "\n";
    return nextTry;
}


SUMOTime
MSRailCrossing::updateCurrentPhase() {
    const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    SUMOTime stayRedUntil = now;
    // check rail links for approaching foes to determine whether and how long
    // the crossing must remain closed
    for (const MSLink* const link : myIncomingRailLinks) {
        for (auto it_avi : link->getApproaching()) {
            const MSLink::ApproachingVehicleInformation& avi = it_avi.second;
            if (avi.arrivalTime - myYellowTime - now < mySecurityGap) {
                stayRedUntil = MAX2(stayRedUntil, avi.leavingTime);
            }
        }
        if (link->getViaLane() != nullptr && link->getViaLane()->getVehicleNumberWithPartials() > 0) {
            // do not open if there is still a train on the crossing
            stayRedUntil = MAX2(stayRedUntil, now + DELTA_T);
        }
    }
    //if (getID() == "cluster_1088529493_1260626727") std::cout << SIMTIME << " stayRedUntil=" << stayRedUntil;
    const SUMOTime wait = stayRedUntil - now;

    if (myStep == 0) {
        // 'G': check whether the crossing can stay open
        if (wait == 0) {
            return DELTA_T;
        } else {
            myStep++;
            return myYellowTime;
        }
    } else if (myStep == 1) {
        // 'y': yellow time is over. switch to red
        myStep++;
        return MAX2(DELTA_T, wait);
    } else {
        // 'r': check whether we may open again
        if (wait == 0) {
            myStep = 0;
            return myMinGreenTime;
        } else {
            return wait;
        }
    }
}


// ------------ Conversion between time and phase
SUMOTime
MSRailCrossing::getPhaseIndexAtTime(SUMOTime) const {
    return 0;
}

SUMOTime
MSRailCrossing::getOffsetFromIndex(int) const {
    return 0;
}

int
MSRailCrossing::getIndexFromOffset(SUMOTime) const {
    return 0;
}


void
MSRailCrossing::addLink(MSLink* link, MSLane* lane, int pos) {
    if (pos >= 0) {
        MSTrafficLightLogic::addLink(link, lane, pos);
    } else {
        myIncomingRailLinks.push_back(link);
    }
}


/****************************************************************************/
