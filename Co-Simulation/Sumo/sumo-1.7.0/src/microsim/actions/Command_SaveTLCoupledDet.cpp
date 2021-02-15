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
/// @file    Command_SaveTLCoupledDet.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    15 Feb 2004
///
// Writes e2 state on each tls switch
/****************************************************************************/
#include <config.h>

#include "Command_SaveTLCoupledDet.h"
#include <microsim/MSNet.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/MSEventControl.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// method definitions
// ===========================================================================
Command_SaveTLCoupledDet::Command_SaveTLCoupledDet(MSTLLogicControl::TLSLogicVariants& tlls,
        MSDetectorFileOutput* dtf, SUMOTime begin, OutputDevice& device)
    : myDevice(device), myLogics(tlls), myDetector(dtf),
      myStartTime(begin) {
    tlls.addSwitchCommand(this);
    dtf->writeXMLDetectorProlog(device);
}


Command_SaveTLCoupledDet::~Command_SaveTLCoupledDet() {
}


void
Command_SaveTLCoupledDet::execute() {
    SUMOTime end = MSNet::getInstance()->getCurrentTimeStep();
    if (myStartTime != end) {
        myDetector->writeXMLOutput(myDevice, myStartTime, end);
        myStartTime = end;
    }
}


/****************************************************************************/
