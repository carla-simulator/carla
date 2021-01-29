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
/// @file    MSSOTLMarchingPolicy.h
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @author  Federico Caselli
/// @date    Feb 2014
///
// The class for SOTL Marching logics
/****************************************************************************/

#pragma once
#include <config.h>

#include "MSSOTLPolicy.h"
/**
 * @class MSSOTLMarchingPolicy
 * @brief Class for low-level marching policy.
 *
 */
class MSSOTLMarchingPolicy: public MSSOTLPolicy, public PushButtonLogic {


public:
    MSSOTLMarchingPolicy(const std::map<std::string, std::string>& parameters);
    MSSOTLMarchingPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm);

    MSSOTLMarchingPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm,
                         const std::map<std::string, std::string>& parameters);

    bool canRelease(SUMOTime elapsed, bool thresholdPassed, bool pushButtonPressed,
                    const MSPhaseDefinition* stage, int vehicleCount);

protected:
    void init();

};
