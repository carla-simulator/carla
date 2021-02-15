//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Steffen Sroka
//              Andreas Koepke
// part of:     framework implementation developed by tkn

#include "veins/base/modules/BaseModule.h"

#include "veins/base/utils/FindModule.h"

using namespace veins;

// Could not initialize simsignal_t it here!? I got the POST_MODEL_CHANGE id!?
const simsignal_t BaseModule::catHostStateSignal = registerSignal("org_car2x_veins_base_utils_hoststate");

BaseModule::BaseModule()
    : cSimpleModule()
{
}

BaseModule::BaseModule(unsigned stacksize)
    : cSimpleModule(stacksize)
{
}

/**
 * Subscription should be in stage==0, and firing
 * notifications in stage==1 or later.
 *
 * NOTE: You have to call this in the initialize() function of the
 * inherited class!
 */
void BaseModule::initialize(int stage)
{
    if (stage == 0) {
        notAffectedByHostState = hasPar("notAffectedByHostState") && par("notAffectedByHostState").boolValue();
        findHost()->subscribe(catHostStateSignal, this);
    }
}

void BaseModule::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    Enter_Method_Silent();
    if (signalID == catHostStateSignal) {
        const HostState* const pHostState = dynamic_cast<const HostState* const>(obj);
        if (pHostState) {
            handleHostState(*pHostState);
        }
        else {
            throw cRuntimeError("Got catHostStateSignal but obj was not a HostState pointer?");
        }
    }
}

void BaseModule::handleHostState(const HostState& state)
{
    if (notAffectedByHostState) return;

    if (state.get() != HostState::ACTIVE) {
        throw cRuntimeError("Hosts state changed to something else than active which is not handled by this module. Either handle this state correctly or if this module really isn't affected by the hosts state set the parameter \"notAffectedByHostState\" of this module to true.");
    }
}

void BaseModule::switchHostState(HostState::States state)
{
    HostState hostState(state);
    emit(catHostStateSignal, &hostState);
}

cModule* const BaseModule::findHost(void)
{
    return FindModule<>::findHost(this);
}

const cModule* const BaseModule::findHost(void) const
{
    return FindModule<>::findHost(this);
}

/**
 * This function returns the logging name of the module with the
 * specified id. It can be used for logging messages to simplify
 * debugging in TKEnv.
 *
 * Only supports ids from simple module derived from the BaseModule
 * or the nic compound module id.
 *
 * @return logging name of module id or NULL if not found
 * @sa logName
 */
std::string BaseModule::logName(void) const
{
    std::ostringstream ost;
    if (hasPar("logName")) { // let modules override
        ost << par("logName").stringValue();
    }
    else {
        const cModule* const parent = findHost();
        parent->hasPar("logName") ? ost << parent->par("logName").stringValue() : ost << parent->getName();
        ost << "[" << parent->getIndex() << "]";
    }
    return ost.str();
}
