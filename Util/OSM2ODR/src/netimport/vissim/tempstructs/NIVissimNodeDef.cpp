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
/// @file    NIVissimNodeDef.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>


#include <iostream> // !!! debug
#include <cassert>
#include "NIVissimNodeDef.h"
#include "NIVissimConnection.h"
#include "NIVissimDisturbance.h"
#include "NIVissimTL.h"


// ===========================================================================
// static member variables
// ===========================================================================
NIVissimNodeDef::DictType NIVissimNodeDef::myDict;
int NIVissimNodeDef::myMaxID = 0;


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimNodeDef::NIVissimNodeDef(int id, const std::string& name)
    : myID(id), myName(name) {}


NIVissimNodeDef::~NIVissimNodeDef() {}


bool
NIVissimNodeDef::dictionary(int id, NIVissimNodeDef* o) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = o;
        myMaxID = myMaxID > id
                  ? myMaxID
                  : id;
//        o->computeBounding();
        return true;
    }
    return false;
}


NIVissimNodeDef*
NIVissimNodeDef::dictionary(int id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}

/*
void
NIVissimNodeDef::buildNodeClusters()
{
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        int cluster = (*i).second->buildNodeCluster();
    }
}
*/


/*

std::vector<int>
NIVissimNodeDef::getWithin(const AbstractPoly &p, double off)
{
    std::vector<int> ret;
    for(DictType::iterator i=myDict.begin(); i!=myDict.end(); i++) {
        NIVissimNodeDef *d = (*i).second;
        if(d->partialWithin(p, off)) {
            ret.push_back((*i).first);
        }
    }
    return ret;
}

bool
NIVissimNodeDef::partialWithin(const AbstractPoly &p, double off) const
{
    assert(myBoundary!=0&&myBoundary->xmax()>=myBoundary->xmin());
    return myBoundary->partialWithin(p, off);
}


void
NIVissimNodeDef::dict_assignConnectionsToNodes() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        (*i).second->searchAndSetConnections();
    }
}
*/


int
NIVissimNodeDef::dictSize() {
    return (int)myDict.size();
}



void
NIVissimNodeDef::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


int
NIVissimNodeDef::getMaxID() {
    return myMaxID;
}


/****************************************************************************/
