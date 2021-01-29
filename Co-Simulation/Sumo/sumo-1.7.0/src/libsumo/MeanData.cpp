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
/// @file    MeanData.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/output/MSMeanData.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "MeanData.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults MeanData::mySubscriptionResults;
ContextSubscriptionResults MeanData::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
MeanData::getIDList() {
    std::vector<std::string> ids;
    for (auto item : MSNet::getInstance()->getDetectorControl().getMeanData()) {
        ids.push_back(item.first);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int
MeanData::getIDCount() {
    return (int)getIDList().size();
}


std::string
MeanData::getParameter(const std::string& /* dataID */, const std::string& /* param */) {
    return "";
}

LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(MeanData)

void
MeanData::setParameter(const std::string& /* dataID */, const std::string& /* key */, const std::string& /* value */) {
    //MSMeanData* r = const_cast<MSMeanData*>(getMeanData(dataID));
    //r->setParameter(key, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(MeanData, MEANDATA)


MSMeanData*
MeanData::getMeanData(const std::string& id) {
    auto mdMap = MSNet::getInstance()->getDetectorControl().getMeanData();
    auto it = mdMap.find(id);
    if (it == mdMap.end() || it->second.size() == 0) {
        throw TraCIException("MeanData '" + id + "' is not known");
    }
    if (it->second.size() > 1) {
        WRITE_WARNING("Found " + toString(it->second.size()) + " meanData definitions with id '" + id + "'.");
    }
    return it->second.front();
}


std::shared_ptr<VariableWrapper>
MeanData::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
MeanData::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        default:
            return false;
    }
}

}


/****************************************************************************/
