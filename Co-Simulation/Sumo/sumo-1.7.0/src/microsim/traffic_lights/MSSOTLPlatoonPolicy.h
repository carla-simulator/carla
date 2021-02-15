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
/// @file    MSSOTLPlatoonPolicy.h
/// @author  Gianfilippo Slager
/// @author  Federico Caselli
/// @date    Feb 2010
///
// The class for SOTL Platoon logics
/****************************************************************************/
#pragma once
#include <config.h>

#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include "MSSOTLPolicy.h"


/**
 * @class MSSOTLPlatoonPolicy
 * @brief Class for low-level platoon policy.
 *
 */
class MSSOTLPlatoonPolicy: public MSSOTLPolicy, public SigmoidLogic, public PushButtonLogic {

public:
    MSSOTLPlatoonPolicy(const std::map<std::string, std::string>& parameters);
    MSSOTLPlatoonPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm);

    MSSOTLPlatoonPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm,
                        const std::map<std::string, std::string>& parameters);

    bool canRelease(SUMOTime elapsed, bool thresholdPassed, bool pushButtonPressed,
                    const MSPhaseDefinition* stage, int vehicleCount);

protected:
    void init();
};
