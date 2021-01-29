/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSSOTLMarchingPolicy.cpp
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @author  Federico Caselli
/// @date    2014-03-20
///
// The class for SOTL Marching logics
/****************************************************************************/

#include "MSSOTLMarchingPolicy.h"

MSSOTLMarchingPolicy::MSSOTLMarchingPolicy(
    const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Marching", parameters) {
    init();
}

MSSOTLMarchingPolicy::MSSOTLMarchingPolicy(
    MSSOTLPolicyDesirability* desirabilityAlgorithm) :
    MSSOTLPolicy("Marching", desirabilityAlgorithm) {
    getDesirabilityAlgorithm()->setKeyPrefix("MARCHING");
    init();
}

MSSOTLMarchingPolicy::MSSOTLMarchingPolicy(
    MSSOTLPolicyDesirability* desirabilityAlgorithm,
    const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Marching", desirabilityAlgorithm, parameters) {
    getDesirabilityAlgorithm()->setKeyPrefix("MARCHING");
    init();
}

bool MSSOTLMarchingPolicy::canRelease(SUMOTime elapsed, bool /* thresholdPassed */, bool pushButtonPressed,
                                      const MSPhaseDefinition* stage, int /* vehicleCount */) {
    if (elapsed >= stage->minDuration && pushButtonLogic(elapsed, pushButtonPressed, stage)) {
        return true;
    }
    return (elapsed >= stage->duration);
}

void MSSOTLMarchingPolicy::init() {
    PushButtonLogic::init("MSSOTLMarchingPolicy", this);
}
