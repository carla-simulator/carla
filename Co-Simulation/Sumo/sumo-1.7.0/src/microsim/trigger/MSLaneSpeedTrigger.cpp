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
/// @file    MSLaneSpeedTrigger.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// Changes the speed allowed on a set of lanes
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/common/WrappingCommand.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StringUtils.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include "MSLaneSpeedTrigger.h"

#include <microsim/MSGlobals.h>
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>


// ===========================================================================
// static members
// ===========================================================================
std::map<std::string, MSLaneSpeedTrigger*> MSLaneSpeedTrigger::myInstances;

// ===========================================================================
// method definitions
// ===========================================================================
MSLaneSpeedTrigger::MSLaneSpeedTrigger(const std::string& id,
                                       const std::vector<MSLane*>& destLanes,
                                       const std::string& file) :
    MSTrigger(id),
    SUMOSAXHandler(file),
    myDestLanes(destLanes),
    myCurrentSpeed(destLanes[0]->getSpeedLimit()),
    myDefaultSpeed(destLanes[0]->getSpeedLimit()),
    myAmOverriding(false),
    mySpeedOverrideValue(destLanes[0]->getSpeedLimit()),
    myDidInit(false) {
    myInstances[id] = this;
    if (file != "") {
        if (!XMLSubSys::runParser(*this, file)) {
            throw ProcessError();
        }
        if (!myDidInit) {
            init();
        }
    }
}

void
MSLaneSpeedTrigger::init() {
    // set it to the right value
    // assert there is at least one
    if (myLoadedSpeeds.size() == 0) {
        myLoadedSpeeds.push_back(std::make_pair(100000, myCurrentSpeed));
    }
    // set the process to the begin
    myCurrentEntry = myLoadedSpeeds.begin();
    // pass previous time steps
    const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    while ((*myCurrentEntry).first < now && myCurrentEntry != myLoadedSpeeds.end()) {
        processCommand(true, now);
    }

    // add the processing to the event handler
    MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(
        new WrappingCommand<MSLaneSpeedTrigger>(this, &MSLaneSpeedTrigger::execute),
        (*myCurrentEntry).first);
    myDidInit = true;
}


MSLaneSpeedTrigger::~MSLaneSpeedTrigger() {}


SUMOTime
MSLaneSpeedTrigger::execute(SUMOTime currentTime) {
    return processCommand(true, currentTime);
}


SUMOTime
MSLaneSpeedTrigger::processCommand(bool move2next, SUMOTime currentTime) {
    UNUSED_PARAMETER(currentTime);
    std::vector<MSLane*>::iterator i;
    const double speed = getCurrentSpeed();
    if (MSGlobals::gUseMesoSim) {
        if (myDestLanes.size() > 0 && myDestLanes.front()->getSpeedLimit() != speed) {
            myDestLanes.front()->getEdge().setMaxSpeed(speed);
            MESegment* first = MSGlobals::gMesoNet->getSegmentForEdge(myDestLanes.front()->getEdge());
            while (first != nullptr) {
                first->setSpeed(speed, currentTime, -1);
                first = first->getNextSegment();
            }
        }
    } else {
        for (i = myDestLanes.begin(); i != myDestLanes.end(); ++i) {
            (*i)->setMaxSpeed(speed);
        }
    }
    if (!move2next) {
        // changed from the gui
        return 0;
    }
    if (myCurrentEntry != myLoadedSpeeds.end()) {
        ++myCurrentEntry;
    }
    if (myCurrentEntry != myLoadedSpeeds.end()) {
        return ((*myCurrentEntry).first) - ((*(myCurrentEntry - 1)).first);
    } else {
        return 0;
    }
}


void
MSLaneSpeedTrigger::myStartElement(int element,
                                   const SUMOSAXAttributes& attrs) {
    // check whether the correct tag is read
    if (element != SUMO_TAG_STEP) {
        return;
    }
    // extract the values
    bool ok = true;
    SUMOTime next = attrs.getSUMOTimeReporting(SUMO_ATTR_TIME, getID().c_str(), ok);
    double speed = attrs.getOpt<double>(SUMO_ATTR_SPEED, getID().c_str(), ok, -1);
    // check the values
    if (next < 0) {
        WRITE_ERROR("Wrong time in vss '" + getID() + "'.");
        return;
    }
    if (speed < 0) {
        speed = myDefaultSpeed;
    }
    // set the values for the next step if they are valid
    if (myLoadedSpeeds.size() != 0 && myLoadedSpeeds.back().first == next) {
        WRITE_WARNING("Time " + time2string(next) + " was set twice for vss '" + getID() + "'; replacing first entry.");
        myLoadedSpeeds.back().second = speed;
    } else {
        myLoadedSpeeds.push_back(std::make_pair(next, speed));
    }
}


void
MSLaneSpeedTrigger::myEndElement(int element) {
    if (element == SUMO_TAG_VSS && !myDidInit) {
        init();
    }
}


double
MSLaneSpeedTrigger::getDefaultSpeed() const {
    return myDefaultSpeed;
}


void
MSLaneSpeedTrigger::setOverriding(bool val) {
    myAmOverriding = val;
    processCommand(false, MSNet::getInstance()->getCurrentTimeStep());
}


void
MSLaneSpeedTrigger::setOverridingValue(double val) {
    mySpeedOverrideValue = val;
    processCommand(false, MSNet::getInstance()->getCurrentTimeStep());
}


double
MSLaneSpeedTrigger::getLoadedSpeed() {
    if (myCurrentEntry != myLoadedSpeeds.begin()) {
        return (*(myCurrentEntry - 1)).second;
    } else {
        return (*myCurrentEntry).second;
    }
}


double
MSLaneSpeedTrigger::getCurrentSpeed() const {
    if (myAmOverriding) {
        return mySpeedOverrideValue;
    } else {
        const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
        // ok, maybe the first shall not yet be the valid one
        if (myCurrentEntry == myLoadedSpeeds.begin() && (*myCurrentEntry).first > now) {
            return myDefaultSpeed;
        }
        // try the loaded
        if (myCurrentEntry != myLoadedSpeeds.end() && (*myCurrentEntry).first <= now) {
            return (*myCurrentEntry).second;
        } else {
            // we have run past the end of the loaded steps or the current step is not yet active:
            // -> use the value of the previous step
            return (*(myCurrentEntry - 1)).second;
        }
    }
}


/****************************************************************************/
