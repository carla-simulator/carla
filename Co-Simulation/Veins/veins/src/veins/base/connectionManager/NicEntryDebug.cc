//
// Copyright (C) 2005 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Daniel Willkomm
// part of:     framework implementation developed by tkn
// description: Class to store information about a nic for the
//              ConnectionManager module

#include "veins/base/connectionManager/NicEntryDebug.h"

#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/base/utils/FindModule.h"

using std::endl;
using namespace veins;

void NicEntryDebug::connectTo(NicEntry* other)
{
    EV_TRACE << "connecting nic #" << nicId << " and #" << other->nicId << endl;

    NicEntryDebug* otherNic = (NicEntryDebug*) other;

    cGate* localoutgate = requestOutGate();
    localoutgate->connectTo(otherNic->requestInGate());
    outConns[other] = localoutgate->getPathStartGate();
}

void NicEntryDebug::disconnectFrom(NicEntry* other)
{
    EV_TRACE << "disconnecting nic #" << nicId << " and #" << other->nicId << endl;

    NicEntryDebug* otherNic = (NicEntryDebug*) other;

    // search the connection in the outConns list
    GateList::iterator p = outConns.find(other);
    // no need to check whether entry is valid; is already check by ConnectionManager isConnected
    // get the hostGate
    // order is phyGate->nicGate->hostGate
    cGate* hostGate = p->second->getNextGate()->getNextGate();

    // release local out gate
    freeOutGates.push_back(hostGate);

    // release remote in gate
    otherNic->freeInGates.push_back(hostGate->getNextGate());

    // reset gates
    // hostGate->getNextGate()->connectTo(0);
    hostGate->disconnect();

    // delete the connection
    outConns.erase(p);
}

int NicEntryDebug::collectGates(const char* pattern, GateStack& gates)
{
    cModule* host = nicPtr->getParentModule();
    int i = 1;
    char gateName[20];
    // create the unique name for the gate (composed of the nic module id and a counter)
    sprintf(gateName, pattern, nicId, i);
    while (host->hasGate(gateName)) {
        cGate* hostGate = host->gate(gateName);
        if (hostGate->isConnectedOutside()) {
            throw cRuntimeError("Gate %s is still connected but not registered with this NicEntry. Either the last NicEntry for this NIC did not clean up correctly or another gate creation module is interfering with this one!", gateName);
        }
        ASSERT(hostGate->isConnectedInside());
        gates.push_back(hostGate);

        ++i;
        sprintf(gateName, pattern, nicId, i);
    }

    return i - 1;
}

void NicEntryDebug::collectFreeGates()
{
    if (!checkFreeGates) return;

    inCnt = collectGates("in%d-%d", freeInGates);
    EV_TRACE << "found " << inCnt << " already existing usable in-gates." << endl;

    outCnt = collectGates("out%d-%d", freeOutGates);
    EV_TRACE << "found " << inCnt << " already existing usable out-gates." << endl;

    checkFreeGates = false;
}

cGate* NicEntryDebug::requestInGate(void)
{
    collectFreeGates();

    // gate of the host
    cGate* hostGate;

    if (!freeInGates.empty()) {
        hostGate = freeInGates.back();
        freeInGates.pop_back();
    }
    else {
        char gateName[20];

        // we will have one more in gate
        ++inCnt;

        // get a unique name for the gate (composed of the nic module id and a counter)
        sprintf(gateName, "in%d-%d", nicId, inCnt);

        // create a new gate for the host module
        nicPtr->getParentModule()->addGate(gateName, cGate::INPUT);
        hostGate = nicPtr->getParentModule()->gate(gateName);

        // gate of the nic
        cGate* nicGate;

        // create a new gate for the nic module
        nicPtr->addGate(gateName, cGate::INPUT);
        nicGate = nicPtr->gate(gateName);

        // connect the hist gate with the nic gate
        hostGate->connectTo(nicGate);

        // pointer to the phy module
        ChannelAccess* phyModule;
        // gate of the phy module
        cGate* phyGate;

        // to avoid unnecessary dynamic_casting we check for a "phy"-named submodule first
        if ((phyModule = dynamic_cast<ChannelAccess*>(nicPtr->getSubmodule("phy"))) == nullptr) phyModule = FindModule<ChannelAccess*>::findSubModule(nicPtr);
        ASSERT(phyModule != nullptr);

        // create a new gate for the phy module
        phyModule->addGate(gateName, cGate::INPUT);
        phyGate = phyModule->gate(gateName);

        // connect the nic gate (the gate of the compound module) to
        // a "real" gate -- the gate of the phy module
        nicGate->connectTo(phyGate);
    }

    return hostGate;
}

cGate* NicEntryDebug::requestOutGate(void)
{
    collectFreeGates();

    // gate of the host
    cGate* hostGate;

    if (!freeOutGates.empty()) {
        hostGate = freeOutGates.back();
        freeOutGates.pop_back();
    }
    else {
        char gateName[20];

        // we will have one more out gate
        ++outCnt;

        // get a unique name for the gate (composed of the nic module id and a counter)
        sprintf(gateName, "out%d-%d", nicId, outCnt);

        // create a new gate for the host module
        nicPtr->getParentModule()->addGate(gateName, cGate::OUTPUT);
        hostGate = nicPtr->getParentModule()->gate(gateName);

        // gate of the nic
        cGate* nicGate;
        // create a new gate for the nic module
        nicPtr->addGate(gateName, cGate::OUTPUT);
        nicGate = nicPtr->gate(gateName);

        // connect the hist gate with the nic gate
        nicGate->connectTo(hostGate);

        // pointer to the phy module
        ChannelAccess* phyModule;
        // gate of the phy module
        cGate* phyGate;

        // to avoid unnecessary dynamic_casting we check for a "phy"-named submodule first
        if ((phyModule = dynamic_cast<ChannelAccess*>(nicPtr->getSubmodule("phy"))) == nullptr) phyModule = FindModule<ChannelAccess*>::findSubModule(nicPtr);
        ASSERT(phyModule != nullptr);

        // create a new gate for the phy module
        phyModule->addGate(gateName, cGate::OUTPUT);
        phyGate = phyModule->gate(gateName);

        // connect the nic gate (the gate of the compound module) to
        // a "real" gate -- the gate of the phy module
        phyGate->connectTo(nicGate);
    }

    return hostGate;
}
