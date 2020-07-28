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
/// @file    DistributionCont.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// A container for distributions
/****************************************************************************/
#include <config.h>

#include "DistributionCont.h"


// ===========================================================================
// static variable definitions
// ===========================================================================
DistributionCont::TypedDistDict DistributionCont::myDict;


// ===========================================================================
// method definitions
// ===========================================================================
bool
DistributionCont::dictionary(const std::string& type, const std::string& id,
                             Distribution* d) {
    TypedDistDict::iterator i = myDict.find(type);

    if (i == myDict.end()) {
        myDict[type][id] = d;
        return true;
    }
    DistDict& dict = (*i).second;
    DistDict::iterator j = dict.find(id);
    if (j == dict.end()) {
        myDict[type][id] = d;
        return true;
    }
    return false;
}


Distribution*
DistributionCont::dictionary(const std::string& type,
                             const std::string& id) {
    TypedDistDict::iterator i = myDict.find(type);
    if (i == myDict.end()) {
        return nullptr;
    }
    DistDict& dict = (*i).second;
    DistDict::iterator j = dict.find(id);
    if (j == dict.end()) {
        return nullptr;
    }
    return (*j).second;
}


void
DistributionCont::clear() {
    for (TypedDistDict::iterator i = myDict.begin(); i != myDict.end(); i++) {
        DistDict& dict = (*i).second;
        for (DistDict::iterator j = dict.begin(); j != dict.end(); j++) {
            delete (*j).second;
        }
    }
}


/****************************************************************************/
