/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    RODFDetectorFlow.cpp
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Michael Behrisch
/// @author  Melanie Knocke
/// @date    Thu, 16.03.2006
///
// Storage for flows within the DFROUTER
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <cassert>
#include "RODFDetectorFlow.h"


// ===========================================================================
// method definitions
// ===========================================================================
RODFDetectorFlows::RODFDetectorFlows(SUMOTime startTime, SUMOTime endTime,
                                     SUMOTime stepOffset)
    : myBeginTime(startTime), myEndTime(endTime), myStepOffset(stepOffset),
      myMaxDetectorFlow(-1) {}


RODFDetectorFlows::~RODFDetectorFlows() {}


void
RODFDetectorFlows::addFlow(const std::string& id, SUMOTime t, const FlowDef& fd) {
    if (myFastAccessFlows.find(id) == myFastAccessFlows.end()) {
        int noItems = (int)((myEndTime - myBeginTime) / myStepOffset);
        myFastAccessFlows[id] = std::vector<FlowDef>(noItems);
        std::vector<FlowDef>& cflows = myFastAccessFlows[id];
        // initialise
        for (std::vector<FlowDef>::iterator i = cflows.begin(); i < cflows.end(); ++i) {
            (*i).qPKW = 0;
            (*i).qLKW = 0;
            (*i).vPKW = 0;
            (*i).vLKW = 0;
            (*i).fLKW = 0;
            (*i).isLKW = 0;
            (*i).firstSet = true;
        }
    }
    const int index = (int)((t - myBeginTime) / myStepOffset);
    assert(index < (int) myFastAccessFlows[id].size());
    FlowDef& ofd = myFastAccessFlows[id][index];
    if (ofd.firstSet) {
        ofd = fd;
        ofd.firstSet = false;
    } else {
        ofd.qLKW = ofd.qLKW + fd.qLKW;
        ofd.qPKW = ofd.qPKW + fd.qPKW;
        ofd.vLKW = ofd.vLKW + fd.vLKW; //!!! mean value?
        ofd.vPKW = ofd.vPKW + fd.vPKW; //!!! mean value?
    }
    if (ofd.qPKW != 0) {
        ofd.fLKW = ofd.qLKW / (ofd.qLKW + ofd.qPKW);
    } else {
        ofd.fLKW = 1;
        ofd.isLKW = 1;
    }
}


void
RODFDetectorFlows::setFlows(const std::string& detector_id,
                            std::vector<FlowDef>& flows) {
    for (std::vector<FlowDef>::iterator i = flows.begin(); i < flows.end(); ++i) {
        FlowDef& ofd = *i;
        if (ofd.qLKW != 0 && ofd.qPKW != 0) {
            ofd.fLKW = ofd.qLKW / ofd.qPKW;
        } else {
            ofd.fLKW = 0;
        }
    }
    myFastAccessFlows[detector_id] = flows;
}


void
RODFDetectorFlows::removeFlow(const std::string& detector_id) {
    myFastAccessFlows.erase(detector_id);
}


bool
RODFDetectorFlows::knows(const std::string& det_id) const {
    return myFastAccessFlows.find(det_id) != myFastAccessFlows.end();
}


const std::vector<FlowDef>&
RODFDetectorFlows::getFlowDefs(const std::string& id) const {
    assert(myFastAccessFlows.find(id) != myFastAccessFlows.end());
    assert(myFastAccessFlows.find(id)->second.size() != 0);
    return myFastAccessFlows.find(id)->second;
}


double
RODFDetectorFlows::getFlowSumSecure(const std::string& id) const {
    double ret = 0;
    if (knows(id)) {
        const std::vector<FlowDef>& flows = getFlowDefs(id);
        for (std::vector<FlowDef>::const_iterator i = flows.begin(); i != flows.end(); ++i) {
            ret += (*i).qPKW;
            ret += (*i).qLKW;
        }
    }
    return ret;
}


double
RODFDetectorFlows::getMaxDetectorFlow() const {
    if (myMaxDetectorFlow < 0) {
        double max = 0;
        std::map<std::string, std::vector<FlowDef> >::const_iterator j;
        for (j = myFastAccessFlows.begin(); j != myFastAccessFlows.end(); ++j) {
            double curr = 0;
            const std::vector<FlowDef>& flows = (*j).second;
            for (std::vector<FlowDef>::const_iterator i = flows.begin(); i != flows.end(); ++i) {
                curr += (*i).qPKW;
                curr += (*i).qLKW;
            }
            if (max < curr) {
                max = curr;
            }
        }
        myMaxDetectorFlow = max;
    }
    return myMaxDetectorFlow;
}


void
RODFDetectorFlows::mesoJoin(const std::string& nid,
                            const std::vector<std::string>& oldids) {
    for (std::vector<std::string>::const_iterator i = oldids.begin(); i != oldids.end(); ++i) {
        if (!knows(*i)) {
            continue;
        }
        std::vector<FlowDef>& flows = myFastAccessFlows[*i];
        int index = 0;
        for (SUMOTime t = myBeginTime; t != myEndTime; t += myStepOffset) {
            addFlow(nid, t, flows[index++]); // !!!
        }
        myFastAccessFlows.erase(*i);
    }
}


void
RODFDetectorFlows::printAbsolute() const {
    for (std::map<std::string, std::vector<FlowDef> >::const_iterator i = myFastAccessFlows.begin(); i != myFastAccessFlows.end(); ++i) {
        std::cout << (*i).first << ":";
        const std::vector<FlowDef>& flows = (*i).second;
        double qPKW = 0;
        double qLKW = 0;
        for (std::vector<FlowDef>::const_iterator j = flows.begin(); j != flows.end(); ++j) {
            qPKW += (*j).qPKW;
            qLKW += (*j).qLKW;
        }
        std::cout << qPKW << "/" << qLKW << std::endl;
    }
}


/****************************************************************************/
