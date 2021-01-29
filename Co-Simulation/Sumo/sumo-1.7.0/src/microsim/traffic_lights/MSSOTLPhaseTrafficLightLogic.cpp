/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSSOTLPhaseTrafficLightLogic.cpp
/// @author  Gianfilippo Slager
/// @date    Feb 2010
///
// The class for SOTL Phase logics
/****************************************************************************/

#include "MSSOTLPhaseTrafficLightLogic.h"

MSSOTLPhaseTrafficLightLogic::MSSOTLPhaseTrafficLightLogic(
    MSTLLogicControl& tlcontrol, const std::string& id,
    const std::string& programID, const Phases& phases, int step,
    SUMOTime delay,
    const std::map<std::string, std::string>& parameters) :
    MSSOTLTrafficLightLogic(tlcontrol, id, programID, TrafficLightType::SOTL_PHASE, phases, step, delay,
                            parameters) {
    MsgHandler::getMessageInstance()->inform(
        "*** Intersection " + id
        + " will run using MSSOTLPhaseTrafficLightLogic ***");
}

MSSOTLPhaseTrafficLightLogic::MSSOTLPhaseTrafficLightLogic(
    MSTLLogicControl& tlcontrol, const std::string& id,
    const std::string& programID, const Phases& phases, int step,
    SUMOTime delay, const std::map<std::string, std::string>& parameters,
    MSSOTLSensors* sensors) :
    MSSOTLTrafficLightLogic(tlcontrol, id, programID, TrafficLightType::SOTL_PHASE, phases, step, delay,
                            parameters, sensors) {
}

bool MSSOTLPhaseTrafficLightLogic::canRelease() {
    if (getCurrentPhaseElapsed() >= getCurrentPhaseDef().minDuration) {
        return isThresholdPassed();
    }
    return false;
}
