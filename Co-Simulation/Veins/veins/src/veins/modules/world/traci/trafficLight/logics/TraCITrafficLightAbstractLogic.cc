//
// Copyright (C) 2015-2018 Dominik Buse <dbuse@mail.uni-paderborn.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/world/traci/trafficLight/logics/TraCITrafficLightAbstractLogic.h"

using veins::TraCITrafficLightAbstractLogic;
using namespace omnetpp;

TraCITrafficLightAbstractLogic::TraCITrafficLightAbstractLogic()
    : cSimpleModule()
    , switchTimer(nullptr)
{
}

TraCITrafficLightAbstractLogic::~TraCITrafficLightAbstractLogic()
{
    cancelAndDelete(switchTimer);
}

void TraCITrafficLightAbstractLogic::initialize()
{
    switchTimer = new cMessage("trySwitch");
}

void TraCITrafficLightAbstractLogic::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    }
    else if (msg->arrivedOn("interface$i")) {
        TraCITrafficLightMessage* tlMsg = check_and_cast<TraCITrafficLightMessage*>(msg);
        // always check for changed switch time and (re-)schedule switch handler if so
        if (tlMsg->getChangedAttribute() == TrafficLightAtrributeType::SWITCHTIME) {
            // schedule handler right before the switch
            cancelEvent(switchTimer);
            // make sure the message is not scheduled to the past
            simtime_t nextTick = std::max(SimTime(std::stoi(tlMsg->getNewValue()), SIMTIME_MS), simTime());
            scheduleAt(nextTick, switchTimer);
        }
        // defer further handling to subclass implementation
        handleTlIfMsg(tlMsg);
    }
    else if (msg->arrivedOn("applLayer$i")) {
        handleApplMsg(msg);
    }
    else {
        throw cRuntimeError("Unknown message arrived on %s", msg->getArrivalGate()->getName());
    }
}

void TraCITrafficLightAbstractLogic::handleSelfMsg(cMessage* msg)
{
    if (msg == switchTimer) {
        handlePossibleSwitch();
    }
}
