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
/// @file    Command_SaveTLSSwitches.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    06 Jul 2006
///
// Writes information about the green durations of a tls
/****************************************************************************/
#include <config.h>

#include "Command_SaveTLSSwitches.h"
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSLink.h>
#include <microsim/MSLane.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
Command_SaveTLSSwitches::Command_SaveTLSSwitches(const MSTLLogicControl::TLSLogicVariants& logics,
        OutputDevice& od)
    : myOutputDevice(od), myLogics(logics) {
    MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(this);
    myOutputDevice.writeXMLHeader("tlsSwitches", "tlsswitches_file.xsd");
}


Command_SaveTLSSwitches::~Command_SaveTLSSwitches() {
}


SUMOTime
Command_SaveTLSSwitches::execute(SUMOTime currentTime) {
    MSTrafficLightLogic* light = myLogics.getActive();
    const MSTrafficLightLogic::LinkVectorVector& links = light->getLinks();
    const std::string& state = light->getCurrentPhaseDef().getState();
    for (int i = 0; i < (int) links.size(); i++) {
        if (state[i] == LINKSTATE_TL_GREEN_MAJOR || state[i] == LINKSTATE_TL_GREEN_MINOR) {
            if (myPreviousLinkStates.find(i) == myPreviousLinkStates.end()) {
                // was not saved before
                myPreviousLinkStates[i] = currentTime;
                continue;
            }
        } else {
            if (myPreviousLinkStates.find(i) == myPreviousLinkStates.end()) {
                // was not yet green
                continue;
            }
            const MSTrafficLightLogic::LinkVector& currLinks = links[i];
            const MSTrafficLightLogic::LaneVector& currLanes = light->getLanesAt(i);
            SUMOTime lastOn = myPreviousLinkStates[i];
            for (int j = 0; j < (int) currLinks.size(); j++) {
                MSLink* link = currLinks[j];
                myOutputDevice << "   <tlsSwitch id=\"" << light->getID()
                               << "\" programID=\"" << light->getProgramID()
                               << "\" fromLane=\"" << currLanes[j]->getID()
                               << "\" toLane=\"" << link->getLane()->getID()
                               << "\" begin=\"" << time2string(lastOn)
                               << "\" end=\"" << time2string(currentTime)
                               << "\" duration=\"" << time2string(currentTime - lastOn)
                               << "\"/>\n";
            }
            myPreviousLinkStates.erase(myPreviousLinkStates.find(i));
        }
    }
    return DELTA_T;
}


/****************************************************************************/
