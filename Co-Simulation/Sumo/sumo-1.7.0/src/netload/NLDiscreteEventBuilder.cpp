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
/// @file    NLDiscreteEventBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sep, 2003
///
// }
/****************************************************************************/
#include <config.h>

#include <utils/xml/SUMOXMLDefinitions.h>
#include <microsim/MSNet.h>
#include <microsim/actions/Command_SaveTLSState.h>
#include <microsim/actions/Command_SaveTLSSwitches.h>
#include <microsim/actions/Command_SaveTLSSwitchStates.h>
#include <microsim/actions/Command_SaveTLSProgram.h>
#include <microsim/MSEventControl.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/UtilExceptions.h>
#include <utils/iodevices/OutputDevice.h>
#include "NLDiscreteEventBuilder.h"


// ===========================================================================
// method definitions
// ===========================================================================
NLDiscreteEventBuilder::NLDiscreteEventBuilder(MSNet& net)
    : myNet(net) {
    myActions["SaveTLSStates"] = EV_SAVETLSTATE;
    myActions["SaveTLSSwitchTimes"] = EV_SAVETLSWITCHES;
    myActions["SaveTLSSwitchStates"] = EV_SAVETLSWITCHSTATES;
    myActions["SaveTLSProgram"] = EV_SAVETLSPROGRAM;
}


NLDiscreteEventBuilder::~NLDiscreteEventBuilder() {}


void
NLDiscreteEventBuilder::addAction(const SUMOSAXAttributes& attrs,
                                  const std::string& basePath) {
    bool ok = true;
    const std::string type = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, nullptr, ok, "");
    // check whether the type was given
    if (type == "" || !ok) {
        throw InvalidArgument("An action's type is not given.");
    }
    // get the numerical representation
    KnownActions::iterator i = myActions.find(type);
    if (i == myActions.end()) {
        throw InvalidArgument("The action type '" + type + "' is not known.");
    }
    // build the action
    switch ((*i).second) {
        case EV_SAVETLSTATE:
            buildSaveTLStateCommand(attrs, basePath);
            break;
        case EV_SAVETLSWITCHES:
            buildSaveTLSwitchesCommand(attrs, basePath);
            break;
        case EV_SAVETLSWITCHSTATES:
            buildSaveTLSwitchStatesCommand(attrs, basePath);
            break;
        case EV_SAVETLSPROGRAM:
            buildSaveTLSProgramCommand(attrs, basePath);
            break;
    }
}


void
NLDiscreteEventBuilder::buildSaveTLStateCommand(const SUMOSAXAttributes& attrs,
        const std::string& basePath) {
    bool ok = true;
    const std::string dest = attrs.getOpt<std::string>(SUMO_ATTR_DEST, nullptr, ok, "");
    const std::string source = attrs.getOpt<std::string>(SUMO_ATTR_SOURCE, nullptr, ok, "");
    // check the parameter
    if (dest == "" || !ok) {
        throw InvalidArgument("Incomplete description of an 'SaveTLSState'-action occurred.");
    }
    if (source == "") {
        const std::vector<std::string> ids = myNet.getTLSControl().getAllTLIds();
        for (std::vector<std::string>::const_iterator tls = ids.begin(); tls != ids.end(); ++tls) {
            const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(*tls);
            new Command_SaveTLSState(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
        }
    } else {
        // get the logic
        if (!myNet.getTLSControl().knows(source)) {
            throw InvalidArgument("The traffic light logic to save (" + source +  ") is not known.");
        }
        const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(source);
        // build the action
        new Command_SaveTLSState(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
    }
}


void
NLDiscreteEventBuilder::buildSaveTLSwitchesCommand(const SUMOSAXAttributes& attrs,
        const std::string& basePath) {
    bool ok = true;
    const std::string dest = attrs.getOpt<std::string>(SUMO_ATTR_DEST, nullptr, ok, "");
    const std::string source = attrs.getOpt<std::string>(SUMO_ATTR_SOURCE, nullptr, ok, "");
    // check the parameter
    if (dest == "" || !ok) {
        throw InvalidArgument("Incomplete description of an 'SaveTLSSwitchTimes'-action occurred.");
    }
    if (source == "") {
        const std::vector<std::string> ids = myNet.getTLSControl().getAllTLIds();
        for (std::vector<std::string>::const_iterator tls = ids.begin(); tls != ids.end(); ++tls) {
            const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(*tls);
            new Command_SaveTLSSwitches(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
        }
    } else {
        // get the logic
        if (!myNet.getTLSControl().knows(source)) {
            throw InvalidArgument("The traffic light logic to save (" + source +  ") is not known.");
        }
        const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(source);
        // build the action
        new Command_SaveTLSSwitches(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
    }
}


void
NLDiscreteEventBuilder::buildSaveTLSwitchStatesCommand(const SUMOSAXAttributes& attrs,
        const std::string& basePath) {
    bool ok = true;
    const std::string dest = attrs.getOpt<std::string>(SUMO_ATTR_DEST, nullptr, ok, "");
    const std::string source = attrs.getOpt<std::string>(SUMO_ATTR_SOURCE, nullptr, ok, "");
    // check the parameter
    if (dest == "" || !ok) {
        throw InvalidArgument("Incomplete description of an 'SaveTLSSwitchStates'-action occurred.");
    }
    if (source == "") {
        const std::vector<std::string> ids = myNet.getTLSControl().getAllTLIds();
        for (std::vector<std::string>::const_iterator tls = ids.begin(); tls != ids.end(); ++tls) {
            const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(*tls);
            new Command_SaveTLSSwitchStates(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
        }
    } else {
        // get the logic
        if (!myNet.getTLSControl().knows(source)) {
            throw InvalidArgument("The traffic light logic to save (" + source +  ") is not known.");
        }
        const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(source);
        // build the action
        new Command_SaveTLSSwitchStates(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
    }
}


void
NLDiscreteEventBuilder::buildSaveTLSProgramCommand(const SUMOSAXAttributes& attrs,
        const std::string& basePath) {
    bool ok = true;
    const std::string dest = attrs.getOpt<std::string>(SUMO_ATTR_DEST, nullptr, ok, "");
    const std::string source = attrs.getOpt<std::string>(SUMO_ATTR_SOURCE, nullptr, ok, "");
    // check the parameter
    if (dest == "" || !ok) {
        throw InvalidArgument("Incomplete description of an 'SaveTLSProgram'-action occurred.");
    }
    if (source == "") {
        const std::vector<std::string> ids = myNet.getTLSControl().getAllTLIds();
        for (std::vector<std::string>::const_iterator tls = ids.begin(); tls != ids.end(); ++tls) {
            const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(*tls);
            new Command_SaveTLSProgram(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
        }
    } else {
        // get the logic
        if (!myNet.getTLSControl().knows(source)) {
            throw InvalidArgument("The traffic light logic to save (" + source +  ") is not known.");
        }
        const MSTLLogicControl::TLSLogicVariants& logics = myNet.getTLSControl().get(source);
        // build the action
        new Command_SaveTLSProgram(logics, OutputDevice::getDevice(FileHelpers::checkForRelativity(dest, basePath)));
    }
}


/****************************************************************************/
