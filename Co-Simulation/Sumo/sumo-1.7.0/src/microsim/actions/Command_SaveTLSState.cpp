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
/// @file    Command_SaveTLSState.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    15 Feb 2004
///
// Writes the state of the tls to a file (in each second)
/****************************************************************************/
#include <config.h>

#include "Command_SaveTLSState.h"
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
Command_SaveTLSState::Command_SaveTLSState(const MSTLLogicControl::TLSLogicVariants& logics,
        OutputDevice& od)
    : myOutputDevice(od), myLogics(logics) {
    MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(this);
    myOutputDevice.writeXMLHeader("tlsStates", "tlsstates_file.xsd");
}


Command_SaveTLSState::~Command_SaveTLSState() {
}


SUMOTime
Command_SaveTLSState::execute(SUMOTime currentTime) {
    myOutputDevice << "    <tlsState time=\"" << time2string(currentTime)
                   << "\" id=\"" << myLogics.getActive()->getID()
                   << "\" programID=\"" << myLogics.getActive()->getProgramID()
                   << "\" phase=\"" << myLogics.getActive()->getCurrentPhaseIndex()
                   << "\" state=\"" << myLogics.getActive()->getCurrentPhaseDef().getState() << "\"/>\n";
    return DELTA_T;
}


/****************************************************************************/
