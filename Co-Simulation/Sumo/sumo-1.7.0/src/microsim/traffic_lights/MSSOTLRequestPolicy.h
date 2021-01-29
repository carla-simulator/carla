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
/// @file    MSSOTLRequestPolicy.h
/// @author  Gianfilippo Slager
/// @author  Anna Chiara Bellini
/// @date    Apr 2013
///
// The class for SOTL Request logics
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSSOTLPolicy.h"

/**
 * @class MSSOTLRequestPolicy
 * @brief Class for low-level request policy.
 *
 */
class MSSOTLRequestPolicy: public MSSOTLPolicy {

public:
    MSSOTLRequestPolicy(const std::map<std::string, std::string>& parameters);
    MSSOTLRequestPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm);

    MSSOTLRequestPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm,
                        const std::map<std::string, std::string>& parameters);

    bool canRelease(SUMOTime elapsed, bool thresholdPassed,  bool pushButtonPressed,
                    const MSPhaseDefinition* stage, int vehicleCount);

    int getMinDecisionalPhaseDuration() {
        return StringUtils::toInt(getParameter("MIN_DECISIONAL_PHASE_DUR", "5000"));
    }

};
