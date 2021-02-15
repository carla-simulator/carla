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
/// @file    RouteProbe.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSRoute.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/output/MSRouteProbe.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "RouteProbe.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults RouteProbe::mySubscriptionResults;
ContextSubscriptionResults RouteProbe::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
RouteProbe::getIDList() {
    std::vector<std::string> ids;
    MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_ROUTEPROBE).insertIDs(ids);
    return ids;
}


int
RouteProbe::getIDCount() {
    return (int)getIDList().size();
}

std::string
RouteProbe::getEdgeID(const std::string& probeID) {
    MSRouteProbe* rp = getRouteProbe(probeID);
    return rp->getEdge()->getID();
}

std::string
RouteProbe::sampleLastRouteID(const std::string& probeID) {
    MSRouteProbe* rp = getRouteProbe(probeID);
    const MSRoute* route = rp->sampleRoute(true);
    if (route == nullptr) {
        throw TraCIException("RouteProbe '" + probeID + "' did not collect any routes yet");
    }
    return route->getID();
}

std::string
RouteProbe::sampleCurrentRouteID(const std::string& probeID) {
    MSRouteProbe* rp = getRouteProbe(probeID);
    const MSRoute* route = rp->sampleRoute(false);
    if (route == nullptr) {
        throw TraCIException("RouteProbe '" + probeID + "' did not collect any routes yet");
    }
    return route->getID();
}

std::string
RouteProbe::getParameter(const std::string& /* probeID */, const std::string& /* param */) {
    return "";
}

LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(RouteProbe)

void
RouteProbe::setParameter(const std::string& /* probeID */, const std::string& /* key */, const std::string& /* value */) {
    //MSRouteProbe* rp = getRouteProbe(probeID);
    //r->setParameter(key, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(RouteProbe, ROUTEPROBE)


MSRouteProbe*
RouteProbe::getRouteProbe(const std::string& id) {
    MSRouteProbe* rp = dynamic_cast<MSRouteProbe*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_ROUTEPROBE).get(id));
    if (rp == nullptr) {
        throw TraCIException("Lane area detector '" + id + "' is not known");
    }
    return rp;
}


std::shared_ptr<VariableWrapper>
RouteProbe::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
RouteProbe::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_ROAD_ID:
            return wrapper->wrapString(objID, variable, getEdgeID(objID));
        case VAR_SAMPLE_LAST:
            return wrapper->wrapString(objID, variable, sampleLastRouteID(objID));
        case VAR_SAMPLE_CURRENT:
            return wrapper->wrapString(objID, variable, sampleCurrentRouteID(objID));
        default:
            return false;
    }
}

}


/****************************************************************************/
