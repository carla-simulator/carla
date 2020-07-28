/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIVissimVehTypeClass.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/RGBColor.h>
#include <utils/common/VectorHelper.h>
#include "NIVissimVehTypeClass.h"


NIVissimVehTypeClass::DictType NIVissimVehTypeClass::myDict;

NIVissimVehTypeClass::NIVissimVehTypeClass(int id,
        const std::string& name,
        const RGBColor& color,
        std::vector<int>& types)
    : myID(id), myName(name), myColor(color), myTypes(types) {}

NIVissimVehTypeClass::~NIVissimVehTypeClass() {}


bool
NIVissimVehTypeClass::dictionary(int id, const std::string& name,
                                 const RGBColor& color,
                                 std::vector<int>& types) {
    NIVissimVehTypeClass* o = new NIVissimVehTypeClass(id, name, color, types);
    if (!dictionary(id, o)) {
        delete o;
        return false;
    }
    return true;
}




bool
NIVissimVehTypeClass::dictionary(int name, NIVissimVehTypeClass* o) {
    DictType::iterator i = myDict.find(name);
    if (i == myDict.end()) {
        myDict[name] = o;
        return true;
    }
    return false;
}


NIVissimVehTypeClass*
NIVissimVehTypeClass::dictionary(int name) {
    DictType::iterator i = myDict.find(name);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}


void
NIVissimVehTypeClass::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


/****************************************************************************/
