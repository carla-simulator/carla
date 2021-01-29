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
/// @file    MSSOTLCongestionPolicy.cpp
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @date    Feb 2014
///
// The class for SOTL Congestion logics
/****************************************************************************/

#include "MSSOTLCongestionPolicy.h"

MSSOTLCongestionPolicy::MSSOTLCongestionPolicy(
    const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Congestion", parameters) {
}

MSSOTLCongestionPolicy::MSSOTLCongestionPolicy(
    MSSOTLPolicyDesirability* desirabilityAlgorithm) :
    MSSOTLPolicy("Congestion", desirabilityAlgorithm) {
    getDesirabilityAlgorithm()->setKeyPrefix("CONGESTION");
}

MSSOTLCongestionPolicy::MSSOTLCongestionPolicy(
    MSSOTLPolicyDesirability* desirabilityAlgorithm,
    const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Congestion", desirabilityAlgorithm, parameters) {
    getDesirabilityAlgorithm()->setKeyPrefix("CONGESTION");

}

int MSSOTLCongestionPolicy::decideNextPhase(SUMOTime elapsed,
        const MSPhaseDefinition* stage, int currentPhaseIndex,
        int /* phaseMaxCTS */, bool thresholdPassed, bool pushButtonPressed, int vehicleCount) {
    if (stage->isCommit()) {
        // decide which chain to activate. Gotta work on this
        return currentPhaseIndex;
    }
    if (stage->isTransient()) {
        //If the junction was in a transient step
        //=> go to the next step and return computeReturnTime()
        return currentPhaseIndex + 1;
    }

    if (stage->isDecisional()) {
        if (canRelease(elapsed, thresholdPassed, pushButtonPressed, stage, vehicleCount)) {
            return currentPhaseIndex + 1;
        }
    }

    return currentPhaseIndex;
}

bool MSSOTLCongestionPolicy::canRelease(SUMOTime elapsed, bool /* thresholdPassed */, bool /* pushButtonPressed */,
                                        const MSPhaseDefinition* stage, int /* vehicleCount */) {
    return (elapsed >= stage->minDuration);
}
