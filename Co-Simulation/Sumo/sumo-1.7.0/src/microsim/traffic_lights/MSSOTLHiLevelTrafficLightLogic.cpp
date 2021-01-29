/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSSOTLHiLevelTrafficLightLogic.cpp
/// @author  Alessio Bonfietti
/// @date    Jun 2013
///
/****************************************************************************/
#include "MSSOTLHiLevelTrafficLightLogic.h"

MSSOTLHiLevelTrafficLightLogic::MSSOTLHiLevelTrafficLightLogic(MSTLLogicControl& tlcontrol,
        const std::string& id, const std::string& programID, const TrafficLightType logicType, const Phases& phases,
        int step, SUMOTime delay,
        const std::map<std::string, std::string>& parameters) :
    MSSOTLTrafficLightLogic(tlcontrol, id, programID, logicType, phases, step, delay,
                            parameters) {
    // Setting default values

}

MSSOTLHiLevelTrafficLightLogic::MSSOTLHiLevelTrafficLightLogic(MSTLLogicControl& tlcontrol,
        const std::string& id, const std::string& programID, const TrafficLightType logicType, const Phases& phases,
        int step, SUMOTime delay,
        const std::map<std::string, std::string>& parameters,
        MSSOTLSensors* sensors) :
    MSSOTLTrafficLightLogic(tlcontrol, id, programID, logicType, phases, step, delay,
                            parameters, sensors) {
    // Setting default values

}

MSSOTLHiLevelTrafficLightLogic::~MSSOTLHiLevelTrafficLightLogic() {
    for (int i = 0; i < (int)policies.size(); i++) {
        delete (policies[i]);
    }
}

void MSSOTLHiLevelTrafficLightLogic::addPolicy(MSSOTLPolicy* policy) {
    policies.push_back(policy);
}

void MSSOTLHiLevelTrafficLightLogic::init(NLDetectorBuilder& nb) {
    MSSOTLTrafficLightLogic::init(nb);
}

void MSSOTLHiLevelTrafficLightLogic::activate(MSSOTLPolicy* policy) {
    currentPolicy = policy;
}
