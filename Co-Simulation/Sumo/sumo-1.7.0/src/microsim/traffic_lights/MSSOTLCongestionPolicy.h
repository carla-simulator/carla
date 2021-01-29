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
/// @file    MSSOTLCongestionPolicy.h
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @date    2014-03-20
///
// The class for SOTL Congestion logics
/****************************************************************************/

#pragma once
#include <config.h>


#include "MSSOTLPolicy.h"
/**
 * @class MSSOTLCongestionPolicy
 * @brief Class for low-level congestion policy.
 *
 */
class MSSOTLCongestionPolicy: public MSSOTLPolicy {

public:
    MSSOTLCongestionPolicy(
        const std::map<std::string, std::string>& parameters);

    MSSOTLCongestionPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm);

    MSSOTLCongestionPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm,
                           const std::map<std::string, std::string>& parameters);

    int decideNextPhase(SUMOTime elapsed, const MSPhaseDefinition* stage,
                        int currentPhaseIndex, int phaseMaxCTS, bool thresholdPassed, bool pushButtonPressed,
                        int vehicleCount);

    bool canRelease(SUMOTime elapsed, bool thresholdPassed, bool pushButtonPressed,
                    const MSPhaseDefinition* stage, int vehicleCount);

};

