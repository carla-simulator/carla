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
/// @file    GUIGlObject_AbstractAdd.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    2004
///
// Base class for additional objects (detectors etc.)
/****************************************************************************/
#include <config.h>

#include "GUIGlObject_AbstractAdd.h"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <utils/gui/div/GLHelper.h>


// ===========================================================================
// static member definitions
// ===========================================================================

std::map<std::string, GUIGlObject_AbstractAdd*> GUIGlObject_AbstractAdd::myObjects;
std::vector<GUIGlObject_AbstractAdd*> GUIGlObject_AbstractAdd::myObjectList;

// ===========================================================================
// method definitions
// ===========================================================================

GUIGlObject_AbstractAdd::GUIGlObject_AbstractAdd(GUIGlObjectType type, const std::string& id) :
    GUIGlObject(type, id) {
    myObjects[getFullName()] = this;
    myObjectList.push_back(this);
}


GUIGlObject_AbstractAdd::~GUIGlObject_AbstractAdd() {}


void
GUIGlObject_AbstractAdd::clearDictionary() {
    std::map<std::string, GUIGlObject_AbstractAdd*>::iterator i;
    for (i = myObjects.begin(); i != myObjects.end(); i++) {
//!!!        delete (*i).second;
    }
    myObjects.clear();
    myObjectList.clear();
}


GUIGlObject_AbstractAdd*
GUIGlObject_AbstractAdd::get(const std::string& name) {
    auto i = myObjects.find(name);
    if (i == myObjects.end()) {
        return nullptr;
    } else {
        return i->second;
    }
}


void
GUIGlObject_AbstractAdd::remove(GUIGlObject_AbstractAdd* o) {
    myObjects.erase(o->getFullName());
    myObjectList.erase(std::remove(myObjectList.begin(), myObjectList.end(), o), myObjectList.end());
}


const std::vector<GUIGlObject_AbstractAdd*>&
GUIGlObject_AbstractAdd::getObjectList() {
    return myObjectList;
}


std::vector<GUIGlID>
GUIGlObject_AbstractAdd::getIDList(GUIGlObjectType typeFilter) {
    std::vector<GUIGlID> ret;
    if (typeFilter == GLO_NETWORK) {
        return ret;
    } else if (typeFilter == GLO_NETWORKELEMENT) {
        // obtain all network elements
        for (auto i : myObjectList) {
            if ((i->getType() > GLO_NETWORKELEMENT) && (i->getType() < GLO_ADDITIONALELEMENT)) {
                ret.push_back(i->getGlID());
            }
        }
    } else if (typeFilter == GLO_ADDITIONALELEMENT) {
        // obtain all additionals
        for (auto i : myObjectList) {
            if ((i->getType() > GLO_ADDITIONALELEMENT) && (i->getType() < GLO_SHAPE)) {
                ret.push_back(i->getGlID());
            }
        }
    } else if (typeFilter == GLO_SHAPE) {
        // obtain all Shapes
        for (auto i : myObjectList) {
            if ((i->getType() > GLO_SHAPE) && (i->getType() < GLO_ROUTEELEMENT)) {
                ret.push_back(i->getGlID());
            }
        }
    } else if (typeFilter == GLO_ROUTEELEMENT) {
        // obtain all Shapes
        for (auto i : myObjectList) {
            if ((i->getType() > GLO_ROUTEELEMENT) && (i->getType() < GLO_MAX)) {
                ret.push_back(i->getGlID());
            }
        }
    } else {
        for (auto i : myObjectList) {
            if ((i->getType() & typeFilter) != 0) {
                ret.push_back(i->getGlID());
            }
        }
    }
    return ret;
}


/****************************************************************************/
