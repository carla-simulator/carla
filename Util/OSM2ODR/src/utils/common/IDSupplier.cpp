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
/// @file    IDSupplier.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A class that generates enumerated and prefixed string-ids
/****************************************************************************/
#include <config.h>

#include <string>
#include <sstream>
#include <iostream>
#include "StdDefs.h"
#include "IDSupplier.h"


// ===========================================================================
// method definitions
// ===========================================================================
IDSupplier::IDSupplier(const std::string& prefix, long long int begin)
    : myCurrent(begin), myPrefix(prefix) {}



IDSupplier::IDSupplier(const std::string& prefix, const std::vector<std::string>& knownIDs)
    : myCurrent(0), myPrefix(prefix) {
    for (std::vector<std::string>::const_iterator id_it = knownIDs.begin(); id_it != knownIDs.end(); ++id_it) {
        avoid(*id_it);
    }
}


IDSupplier::~IDSupplier() {}


std::string
IDSupplier::getNext() {
    std::ostringstream strm;
    strm << myPrefix << myCurrent++;
    return strm.str();
}


void
IDSupplier::avoid(const std::string& id) {
    // does it start with prefix?
    if (id.find(myPrefix) == 0) {
        long long int number;
        std::istringstream buf(id.substr(myPrefix.size()));
        buf >> number;
        // does it continue with a number?
        if (!buf.fail()) {
            myCurrent = MAX2(myCurrent, number + 1);
        }
    }
}


/****************************************************************************/
