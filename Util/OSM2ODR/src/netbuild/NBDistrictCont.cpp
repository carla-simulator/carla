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
/// @file    NBDistrictCont.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// A container for districts
/****************************************************************************/
#include <config.h>

#include <string>
#include <iostream>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#include <utils/iodevices/OutputDevice.h>
#include "NBDistrict.h"
#include "NBDistrictCont.h"


// ===========================================================================
// method definitions
// ===========================================================================
NBDistrictCont::NBDistrictCont() {}


NBDistrictCont::~NBDistrictCont() {
    for (DistrictCont::iterator i = myDistricts.begin(); i != myDistricts.end(); i++) {
        delete ((*i).second);
    }
    myDistricts.clear();
}


bool
NBDistrictCont::insert(NBDistrict* const district) {
    DistrictCont::const_iterator i = myDistricts.find(district->getID());
    if (i != myDistricts.end()) {
        return false;
    }
    myDistricts.insert(DistrictCont::value_type(district->getID(), district));
    return true;
}


NBDistrict*
NBDistrictCont::retrieve(const std::string& id) const {
    DistrictCont::const_iterator i = myDistricts.find(id);
    if (i == myDistricts.end()) {
        return nullptr;
    }
    return (*i).second;
}


int
NBDistrictCont::size() const {
    return (int)myDistricts.size();
}


bool
NBDistrictCont::addSource(const std::string& dist, NBEdge* const source,
                          double weight) {
    NBDistrict* o = retrieve(dist);
    if (o == nullptr) {
        return false;
    }
    return o->addSource(source, weight);
}


bool
NBDistrictCont::addSink(const std::string& dist, NBEdge* const destination,
                        double weight) {
    NBDistrict* o = retrieve(dist);
    if (o == nullptr) {
        return false;
    }
    return o->addSink(destination, weight);
}


void
NBDistrictCont::removeFromSinksAndSources(NBEdge* const e) {
    for (DistrictCont::iterator i = myDistricts.begin(); i != myDistricts.end(); i++) {
        (*i).second->removeFromSinksAndSources(e);
    }
}


/****************************************************************************/
