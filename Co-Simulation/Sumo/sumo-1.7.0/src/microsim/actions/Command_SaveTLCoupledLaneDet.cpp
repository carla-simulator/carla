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
/// @file    Command_SaveTLCoupledLaneDet.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    15 Feb 2004
///
// Writes e2 state of a link for the time the link has yellow/red
/****************************************************************************/
#include <config.h>

#include "Command_SaveTLCoupledLaneDet.h"
#include <microsim/MSNet.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/MSEventControl.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/MSLinkCont.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
Command_SaveTLCoupledLaneDet::Command_SaveTLCoupledLaneDet(MSTLLogicControl::TLSLogicVariants& tlls,
        MSDetectorFileOutput* dtf, SUMOTime begin, OutputDevice& device, const MSLink* link)
    : Command_SaveTLCoupledDet(tlls, dtf, begin, device),
      myLink(link), myLastState(LINKSTATE_TL_RED),
      myHadOne(false) {
    execute();
}


Command_SaveTLCoupledLaneDet::~Command_SaveTLCoupledLaneDet() {
}


void
Command_SaveTLCoupledLaneDet::execute() {
    if (myLink->getState() == myLastState && myHadOne) {
        return;
    }
    myHadOne = true;
    if (myLastState == LINKSTATE_TL_RED && myLink->getState() != LINKSTATE_TL_RED) {
        SUMOTime end = MSNet::getInstance()->getCurrentTimeStep();
        if (myStartTime != end) {
            myDetector->writeXMLOutput(myDevice, myStartTime, end);
            myStartTime = end;
        }
    } else if (myLink->getState() == LINKSTATE_TL_RED) {
        myDetector->reset();
        myStartTime = MSNet::getInstance()->getCurrentTimeStep();
    }
    myLastState = myLink->getState();
}


/****************************************************************************/
