/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUITransportableControl.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 13.06.2012
///
// GUI-version of the person control for building gui persons
/****************************************************************************/
#include <config.h>

#include <vector>
#include <algorithm>
#include "GUINet.h"
#include "GUIContainer.h"
#include "GUIPerson.h"
#include "GUITransportableControl.h"


// ===========================================================================
// method definitions
// ===========================================================================
GUITransportableControl::GUITransportableControl(const bool isPerson) :
    MSTransportableControl(isPerson),
    myIsPerson(isPerson)
{}


GUITransportableControl::~GUITransportableControl() {
}


MSTransportable*
GUITransportableControl::buildPerson(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan,
                                     std::mt19937* rng) const {
    const double speedFactor = vtype->computeChosenSpeedDeviation(rng);
    return new GUIPerson(pars, vtype, plan, speedFactor);
}


MSTransportable*
GUITransportableControl::buildContainer(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan) const {
    return new GUIContainer(pars, vtype, plan);
}


void
GUITransportableControl::insertIDs(std::vector<GUIGlID>& into) {
    into.reserve(myTransportables.size());
    for (std::map<std::string, MSTransportable*>::const_iterator it = myTransportables.begin(); it != myTransportables.end(); ++it) {
        if (it->second->getCurrentStageType() != MSStageType::WAITING_FOR_DEPART) {
            if (myIsPerson) {
                into.push_back(static_cast<const GUIPerson*>(it->second)->getGlID());
            } else {
                into.push_back(static_cast<const GUIContainer*>(it->second)->getGlID());
            }
        }
    }
}


/****************************************************************************/
