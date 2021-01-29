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
/// @file    GUIGlObjectStorage.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A storage for displayed objects via their numerical id
/****************************************************************************/
#include <config.h>

#include <map>
#include <iostream>
#include <cassert>
#include <fx.h>
#include "GUIGlObject.h"
#include "GUIGlObjectStorage.h"


// ===========================================================================
// static variables (instances in this case)
// ===========================================================================
GUIGlObjectStorage GUIGlObjectStorage::gIDStorage;


// ===========================================================================
// method definitions
// ===========================================================================
GUIGlObjectStorage::GUIGlObjectStorage() :
    myAktID(1),
    myLock(true)
{}


GUIGlObjectStorage::~GUIGlObjectStorage() {}


GUIGlID
GUIGlObjectStorage::registerObject(GUIGlObject* object, const std::string& fullName) {
    FXMutexLock locker(myLock);
    GUIGlID id = myAktID++;
    myMap[id] = object;
    myFullNameMap[fullName] = object;
    return id;
}


GUIGlObject*
GUIGlObjectStorage::getObjectBlocking(GUIGlID id) {
    FXMutexLock locker(myLock);
    ObjectMap::iterator i = myMap.find(id);
    if (i == myMap.end()) {
        i = myBlocked.find(id);
        if (i != myBlocked.end()) {
            GUIGlObject* o = (*i).second;
            return o;
        }
        return nullptr;
    }
    GUIGlObject* o = (*i).second;
    myMap.erase(id);
    myBlocked[id] = o;
    return o;
}


GUIGlObject*
GUIGlObjectStorage::getObjectBlocking(const std::string& fullName) {
    FXMutexLock locker(myLock);
    if (myFullNameMap.count(fullName)) {
        GUIGlID id = myFullNameMap[fullName]->getGlID();
        return getObjectBlocking(id);
    }
    return nullptr;
}


bool
GUIGlObjectStorage::remove(GUIGlID id) {
    FXMutexLock locker(myLock);
    ObjectMap::iterator i = myMap.find(id);
    if (i == myMap.end()) {
        i = myBlocked.find(id);
        assert(i != myBlocked.end());
        GUIGlObject* o = (*i).second;
        myFullNameMap.erase(o->getFullName());
        myBlocked.erase(id);
        my2Delete[id] = o;
        return false;
    }
    myFullNameMap.erase(i->second->getFullName());
    myMap.erase(id);
    return true;
}


void
GUIGlObjectStorage::clear() {
    FXMutexLock locker(myLock);
    myMap.clear();
    myAktID = 0;
}


void
GUIGlObjectStorage::unblockObject(GUIGlID id) {
    FXMutexLock locker(myLock);
    ObjectMap::iterator i = myBlocked.find(id);
    if (i == myBlocked.end()) {
        return;
    }
    GUIGlObject* o = (*i).second;
    myBlocked.erase(id);
    myMap[id] = o;
}


std::set<GUIGlID>
GUIGlObjectStorage::getAllIDs() const {
    FXMutexLock locker(myLock);
    std::set<GUIGlID> result;
    for (ObjectMap::const_iterator it = myMap.begin(); it != myMap.end(); it++) {
        result.insert(it->first);
    }
    return result;
}


/****************************************************************************/
