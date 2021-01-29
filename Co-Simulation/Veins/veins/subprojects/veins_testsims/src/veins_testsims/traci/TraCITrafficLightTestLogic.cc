//
// Copyright (C) 2015-2018 Dominik Buse <dbuse@mail.uni-paderborn.de>
// Copyright (C) 2018 Tobias Hardes <hardes@ccs-labs.org>
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

#include "veins_testsims/traci/TraCITrafficLightTestLogic.h"

using veins::TraCITrafficLightTestLogic;

Define_Module(veins::TraCITrafficLightTestLogic);

void TraCITrafficLightTestLogic::initialize()
{
    TraCITrafficLightAbstractLogic::initialize();
}

void TraCITrafficLightTestLogic::startChangingProgramAt(simtime_t t)
{
    Enter_Method_Silent();
    changeProgramm = new cMessage();
    scheduleAt(t, changeProgramm);
}

void TraCITrafficLightTestLogic::handleApplMsg(cMessage* msg)
{
    delete msg; // just drop it
}

void TraCITrafficLightTestLogic::handleTlIfMsg(TraCITrafficLightMessage* tlMsg)
{
    delete tlMsg; // just drop it
}
void TraCITrafficLightTestLogic::handleMessage(cMessage* msg)
{
    if (msg == changeProgramm) {
        auto phaseDuration = 10;

        TraCITrafficLightMessage* pStateMsg = new TraCITrafficLightMessage("TrafficLightChangeMessage");
        pStateMsg->setChangedAttribute(TrafficLightAtrributeType::STATE);
        pStateMsg->setChangeSource(TrafficLightChangeSource::LOGIC);
        if (currentIndex == 0) {
            pStateMsg->setNewValue("rrrrrrrrrrrr");
            currentIndex = 1;
        }
        else {
            pStateMsg->setNewValue("GGGGGGGGGGGG");
            currentIndex = 0;
        }
        send(pStateMsg, "interface$o");
        // send new signal duration
        TraCITrafficLightMessage* pDurMsg = new TraCITrafficLightMessage("TrafficLightChangeMessage");
        pDurMsg->setChangedAttribute(TrafficLightAtrributeType::SWITCHTIME);
        pDurMsg->setChangeSource(TrafficLightChangeSource::LOGIC);
        auto theTime = (simTime() + phaseDuration).inUnit(SIMTIME_MS);
        auto theTimeString = std::to_string(theTime).c_str();
        pDurMsg->setNewValue(theTimeString);
        send(pDurMsg, "interface$o");
        scheduleAt(simTime() + phaseDuration, changeProgramm);
    }
    else {
        delete msg;
    }
}

void TraCITrafficLightTestLogic::handlePossibleSwitch()
{
}
