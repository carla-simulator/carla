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
/// @file    MSEdgeWeightsStorage.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    02.11.2009
///
// A storage for edge travel times and efforts
/****************************************************************************/
#include <config.h>

#include "MSEdgeWeightsStorage.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSEdgeWeightsStorage::MSEdgeWeightsStorage() {
}


MSEdgeWeightsStorage::~MSEdgeWeightsStorage() {
}


bool
MSEdgeWeightsStorage::retrieveExistingTravelTime(const MSEdge* const e, const double t, double& value) const {
    std::map<const MSEdge*, ValueTimeLine<double> >::const_iterator i = myTravelTimes.find(e);
    if (i == myTravelTimes.end()) {
        return false;
    }
    const ValueTimeLine<double>& tl = (*i).second;
    if (!tl.describesTime(t)) {
        return false;
    }
    value = tl.getValue(t);
    return true;
}


bool
MSEdgeWeightsStorage::retrieveExistingEffort(const MSEdge* const e, const double t, double& value) const {
    std::map<const MSEdge*, ValueTimeLine<double> >::const_iterator i = myEfforts.find(e);
    if (i == myEfforts.end()) {
        return false;
    }
    const ValueTimeLine<double>& tl = (*i).second;
    if (!tl.describesTime(t)) {
        return false;
    }
    value = tl.getValue(t);
    return true;
}


void
MSEdgeWeightsStorage::addTravelTime(const MSEdge* const e,
                                    double begin, double end,
                                    double value) {
    std::map<const MSEdge*, ValueTimeLine<double> >::iterator i = myTravelTimes.find(e);
    if (i == myTravelTimes.end()) {
        myTravelTimes[e] = ValueTimeLine<double>();
        i = myTravelTimes.find(e);
    }
    (*i).second.add(begin, end, value);
}


void
MSEdgeWeightsStorage::addEffort(const MSEdge* const e,
                                double begin, double end,
                                double value) {
    std::map<const MSEdge*, ValueTimeLine<double> >::iterator i = myEfforts.find(e);
    if (i == myEfforts.end()) {
        myEfforts[e] = ValueTimeLine<double>();
        i = myEfforts.find(e);
    }
    (*i).second.add(begin, end, value);
}


void
MSEdgeWeightsStorage::removeTravelTime(const MSEdge* const e) {
    std::map<const MSEdge*, ValueTimeLine<double> >::iterator i = myTravelTimes.find(e);
    if (i != myTravelTimes.end()) {
        myTravelTimes.erase(i);
    }
}


void
MSEdgeWeightsStorage::removeEffort(const MSEdge* const e) {
    std::map<const MSEdge*, ValueTimeLine<double> >::iterator i = myEfforts.find(e);
    if (i != myEfforts.end()) {
        myEfforts.erase(i);
    }
}


bool
MSEdgeWeightsStorage::knowsTravelTime(const MSEdge* const e) const {
    return myTravelTimes.find(e) != myTravelTimes.end();
}


bool
MSEdgeWeightsStorage::knowsEffort(const MSEdge* const e) const {
    return myEfforts.find(e) != myEfforts.end();
}


/****************************************************************************/
