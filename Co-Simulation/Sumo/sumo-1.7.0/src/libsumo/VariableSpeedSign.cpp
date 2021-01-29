/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    VariableSpeedSign.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/trigger/MSLaneSpeedTrigger.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "VariableSpeedSign.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults VariableSpeedSign::mySubscriptionResults;
ContextSubscriptionResults VariableSpeedSign::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
VariableSpeedSign::getIDList() {
    std::vector<std::string> ids;
    for (auto& item : MSLaneSpeedTrigger::getInstances()) {
        ids.push_back(item.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int
VariableSpeedSign::getIDCount() {
    return (int)getIDList().size();
}

std::vector<std::string>
VariableSpeedSign::getLaneIDs(const std::string& vssID) {
    std::vector<std::string> result;
    MSLaneSpeedTrigger* vss = getVariableSpeedSign(vssID);
    for (MSLane* lane : vss->getLanes()) {
        result.push_back(lane->getID());
    }
    return result;
}

std::string
VariableSpeedSign::getParameter(const std::string& /* vssID */, const std::string& /* param */) {
    return "";
}

LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(VariableSpeedSign)

void
VariableSpeedSign::setParameter(const std::string& /* vssID */, const std::string& /* key */, const std::string& /* value */) {
    //MSVariableSpeedSign* r = const_cast<MSVariableSpeedSign*>(getVariableSpeedSign(vssID));
    //r->setParameter(key, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(VariableSpeedSign, VARIABLESPEEDSIGN)


MSLaneSpeedTrigger*
VariableSpeedSign::getVariableSpeedSign(const std::string& id) {
    const auto& dict = MSLaneSpeedTrigger::getInstances();
    auto it = dict.find(id);
    if (it == dict.end()) {
        throw TraCIException("VariableSpeedSign '" + id + "' is not known");
    }
    return it->second;
}


std::shared_ptr<VariableWrapper>
VariableSpeedSign::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
VariableSpeedSign::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_LANES:
            return wrapper->wrapStringList(objID, variable, getLaneIDs(objID));
        default:
            return false;
    }
}
}


/****************************************************************************/
