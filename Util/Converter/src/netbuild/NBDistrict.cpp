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
/// @file    NBDistrict.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A class representing a single district
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>
#include <utils/common/Named.h>
#include <utils/common/StringUtils.h>
#include <utils/iodevices/OutputDevice.h>
#include "NBEdge.h"
#include "NBDistrict.h"


// ===========================================================================
// member method definitions
// ===========================================================================
NBDistrict::NBDistrict(const std::string& id, const Position& pos)
    : Named(StringUtils::convertUmlaute(id)),
      myPosition(pos) {}


NBDistrict::NBDistrict(const std::string& id)
    : Named(id), myPosition(0, 0) {}


NBDistrict::~NBDistrict() {}


// -----------  Applying offset
void
NBDistrict::reshiftPosition(double xoff, double yoff) {
    myPosition.add(xoff, yoff, 0);
    myShape.add(xoff, yoff, 0);
}


void
NBDistrict::mirrorX() {
    myPosition.mul(1, -1);
    myShape.mirrorX();
}


bool
NBDistrict::addSource(NBEdge* const source, double weight) {
    EdgeVector::iterator i = std::find(mySources.begin(), mySources.end(), source);
    if (i != mySources.end()) {
        return false;
    }
    mySources.push_back(source);
    mySourceWeights.push_back(weight);
    assert(source->getID() != "");
    return true;
}


bool
NBDistrict::addSink(NBEdge* const sink, double weight) {
    EdgeVector::iterator i = std::find(mySinks.begin(), mySinks.end(), sink);
    if (i != mySinks.end()) {
        return false;
    }
    mySinks.push_back(sink);
    mySinkWeights.push_back(weight);
    assert(sink->getID() != "");
    return true;
}


void
NBDistrict::setCenter(const Position& pos) {
    myPosition = pos;
}


void
NBDistrict::replaceIncoming(const EdgeVector& which, NBEdge* const by) {
    // temporary structures
    EdgeVector newList;
    WeightsCont newWeights;
    double joinedVal = 0;
    // go through the list of sinks
    EdgeVector::iterator i = mySinks.begin();
    WeightsCont::iterator j = mySinkWeights.begin();
    for (; i != mySinks.end(); i++, j++) {
        NBEdge* tmp = (*i);
        double val = (*j);
        if (find(which.begin(), which.end(), tmp) == which.end()) {
            // if the current edge shall not be replaced, add to the
            //  temporary list
            newList.push_back(tmp);
            newWeights.push_back(val);
        } else {
            // otherwise, skip it and add its weight to the one to be inserted
            //  instead
            joinedVal += val;
        }
    }
    // add the one to be inserted instead
    newList.push_back(by);
    newWeights.push_back(joinedVal);
    // assign to values
    mySinks = newList;
    mySinkWeights = newWeights;
}


void
NBDistrict::replaceOutgoing(const EdgeVector& which, NBEdge* const by) {
    // temporary structures
    EdgeVector newList;
    WeightsCont newWeights;
    double joinedVal = 0;
    // go through the list of sinks
    EdgeVector::iterator i = mySources.begin();
    WeightsCont::iterator j = mySourceWeights.begin();
    for (; i != mySources.end(); i++, j++) {
        NBEdge* tmp = (*i);
        double val = (*j);
        if (find(which.begin(), which.end(), tmp) == which.end()) {
            // if the current edge shall not be replaced, add to the
            //  temporary list
            newList.push_back(tmp);
            newWeights.push_back(val);
        } else {
            // otherwise, skip it and add its weight to the one to be inserted
            //  instead
            joinedVal += val;
        }
    }
    // add the one to be inserted instead
    newList.push_back(by);
    newWeights.push_back(joinedVal);
    // assign to values
    mySources = newList;
    mySourceWeights = newWeights;
}


void
NBDistrict::removeFromSinksAndSources(NBEdge* const e) {
    int i;
    for (i = 0; i < (int)mySinks.size(); ++i) {
        if (mySinks[i] == e) {
            mySinks.erase(mySinks.begin() + i);
            mySinkWeights.erase(mySinkWeights.begin() + i);
        }
    }
    for (i = 0; i < (int)mySources.size(); ++i) {
        if (mySources[i] == e) {
            mySources.erase(mySources.begin() + i);
            mySourceWeights.erase(mySourceWeights.begin() + i);
        }
    }
}


void
NBDistrict::addShape(const PositionVector& p) {
    myShape = p;
}


/****************************************************************************/
