/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    ROJTREdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    Tue, 20 Jan 2004
///
// An edge the jtr-router may route through
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <cassert>
#include <utils/common/MsgHandler.h>
#include <utils/common/RandHelper.h>
#include "ROJTREdge.h"
#include <utils/distribution/RandomDistributor.h>


// ===========================================================================
// method definitions
// ===========================================================================
ROJTREdge::ROJTREdge(const std::string& id, RONode* from, RONode* to, int index, const int priority) :
    ROEdge(id, from, to, index, priority),
    mySourceFlows(0)
{}


ROJTREdge::~ROJTREdge() {
    for (FollowerUsageCont::iterator i = myFollowingDefs.begin(); i != myFollowingDefs.end(); ++i) {
        delete (*i).second;
    }
}


void
ROJTREdge::addSuccessor(ROEdge* s, ROEdge* via, std::string dir) {
    ROEdge::addSuccessor(s, via, dir);
    ROJTREdge* js = static_cast<ROJTREdge*>(s);
    if (myFollowingDefs.find(js) == myFollowingDefs.end()) {
        myFollowingDefs[js] = new ValueTimeLine<double>();
    }
}


void
ROJTREdge::addFollowerProbability(ROJTREdge* follower, double begTime,
                                  double endTime, double probability) {
    FollowerUsageCont::iterator i = myFollowingDefs.find(follower);
    if (i == myFollowingDefs.end()) {
        WRITE_ERROR("The edges '" + getID() + "' and '" + follower->getID() + "' are not connected.");
        return;
    }
    (*i).second->add(begTime, endTime, probability);
}


ROJTREdge*
ROJTREdge::chooseNext(const ROVehicle* const veh, double time, const std::set<const ROEdge*>& avoid) const {
    // if no usable follower exist, return 0
    //  their probabilities are not yet regarded
    if (myFollowingEdges.size() == 0 || (veh != nullptr && allFollowersProhibit(veh))) {
        return nullptr;
    }
    // gather information about the probabilities at this time
    RandomDistributor<ROJTREdge*> dist;
    // use the loaded definitions, first
    for (FollowerUsageCont::const_iterator i = myFollowingDefs.begin(); i != myFollowingDefs.end(); ++i) {
        if (avoid.count(i->first) == 0) {
            if ((veh == nullptr || !(*i).first->prohibits(veh)) && (*i).second->describesTime(time)) {
                dist.add((*i).first, (*i).second->getValue(time));
            }
        }
    }
    // if no loaded definitions are valid for this time, try to use the defaults
    if (dist.getOverallProb() == 0) {
        for (int i = 0; i < (int)myParsedTurnings.size(); ++i) {
            if (avoid.count(myFollowingEdges[i]) == 0) {
                if (veh == nullptr || !myFollowingEdges[i]->prohibits(veh)) {
                    dist.add(static_cast<ROJTREdge*>(myFollowingEdges[i]), myParsedTurnings[i]);
                }
            }
        }
    }
    // if still no valid follower exists, return null
    if (dist.getOverallProb() == 0) {
        return nullptr;
    }
    // return one of the possible followers
    return dist.get();
}


void
ROJTREdge::setTurnDefaults(const std::vector<double>& defs) {
    // I hope, we'll find a less ridiculous solution for this
    std::vector<double> tmp(defs.size()*myFollowingEdges.size(), 0);
    // store in less common multiple
    for (int i = 0; i < (int)defs.size(); ++i) {
        for (int j = 0; j < (int)myFollowingEdges.size(); ++j) {
            tmp[i * myFollowingEdges.size() + j] = (double)(defs[i] / 100.0 / (myFollowingEdges.size()));
        }
    }
    // parse from less common multiple
    for (int i = 0; i < (int)myFollowingEdges.size(); ++i) {
        double value = 0;
        for (int j = 0; j < (int)defs.size(); ++j) {
            value += tmp[i * defs.size() + j];
        }
        myParsedTurnings.push_back((double) value);
    }
}


/****************************************************************************/
