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
/// @file    MSSOTLPlatoonPolicy.cpp
/// @author  Gianfilippo Slager
/// @author  Federico Caselli
/// @date    Feb 2010
///
// The class for SOTL Platoon logics
/****************************************************************************/

#include "MSSOTLPlatoonPolicy.h"


MSSOTLPlatoonPolicy::MSSOTLPlatoonPolicy(const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Platoon", parameters) {
    init();
}

MSSOTLPlatoonPolicy::MSSOTLPlatoonPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm) :
    MSSOTLPolicy("Platoon", desirabilityAlgorithm) {
    getDesirabilityAlgorithm()->setKeyPrefix("PLATOON");
    init();
}

MSSOTLPlatoonPolicy::MSSOTLPlatoonPolicy(MSSOTLPolicyDesirability* desirabilityAlgorithm,
        const std::map<std::string, std::string>& parameters) :
    MSSOTLPolicy("Platoon", desirabilityAlgorithm, parameters) {
    getDesirabilityAlgorithm()->setKeyPrefix("PLATOON");
    init();
}

bool MSSOTLPlatoonPolicy::canRelease(SUMOTime elapsed, bool thresholdPassed, bool pushButtonPressed,
                                     const MSPhaseDefinition* stage, int vehicleCount) {
//  DBG(std::ostringstream str; str << "invoked MSTLPlatoonPolicy::canRelease()"; WRITE_MESSAGE(str.str()););
    DBG(
        std::ostringstream str;
        str << "MSSOTLPlatoonPolicy::canRelease elapsed " << elapsed << " threshold " << thresholdPassed << " pushbutton " << pushButtonPressed << " vcount " << vehicleCount
        << " minD " << stage->minDuration << " maxD " << stage->maxDuration; str << " will return " << ((thresholdPassed && ((vehicleCount == 0) || (elapsed >= stage->maxDuration))) ? "true" : "false");
        WRITE_MESSAGE(str.str());
    );
    if (elapsed >= stage->minDuration) {
        if (pushButtonLogic(elapsed, pushButtonPressed, stage)) {
            return true;
        }
        if (thresholdPassed) {
            //If there are no other vehicles approaching green lights
            //or the declared maximum duration has been reached
            return ((vehicleCount == 0) || (elapsed >= stage->maxDuration));
        } else {
            if (sigmoidLogic(elapsed, stage, vehicleCount)) {
                return true;
            }
        }
    }
    return false;
}

void MSSOTLPlatoonPolicy::init() {
    SigmoidLogic::init("MSSOTLPlatoonPolicy", this);
    PushButtonLogic::init("MSSOTLPlatoonPolicy", this);
}
