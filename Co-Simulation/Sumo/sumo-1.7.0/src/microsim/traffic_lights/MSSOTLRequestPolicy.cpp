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
/// @file    MSSOTLRequestPolicy.cpp
/// @author  Gianfilippo Slager
/// @author  Anna Chiara Bellini
/// @date    Apr 2013
///
// The class for SOTL Request logics
/****************************************************************************/

#include "MSSOTLRequestPolicy.h"

MSSOTLRequestPolicy::MSSOTLRequestPolicy(
    const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Request", parameters) {
}

MSSOTLRequestPolicy::MSSOTLRequestPolicy(
    MSSOTLPolicyDesirability* desirabilityAlgorithm) :
    MSSOTLPolicy("Request", desirabilityAlgorithm) {
    getDesirabilityAlgorithm()->setKeyPrefix("REQUEST");
}

MSSOTLRequestPolicy::MSSOTLRequestPolicy(
    MSSOTLPolicyDesirability* desirabilityAlgorithm,
    const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Request", desirabilityAlgorithm, parameters) {
    getDesirabilityAlgorithm()->setKeyPrefix("REQUEST");

}

bool MSSOTLRequestPolicy::canRelease(SUMOTime elapsed, bool thresholdPassed,  bool /* pushButtonPressed */,
                                     const MSPhaseDefinition* /* stage */, int /* vehicleCount */) {
    if (elapsed >= getMinDecisionalPhaseDuration()) {
        return thresholdPassed;
    }
    return false;
}
