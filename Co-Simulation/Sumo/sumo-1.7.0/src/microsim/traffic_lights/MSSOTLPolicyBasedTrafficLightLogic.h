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
/// @file    MSSOTLPolicyBasedTrafficLightLogic.h
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @date    2014-03-20
///
// The class for SOTL Policy-based logics
/****************************************************************************/

#pragma once
#include <config.h>

//#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include "MSSOTLPolicy.h"
#include "MSSOTLTrafficLightLogic.h"

/**
 * @class MSSOTLPolicyBasedTrafficLightLogic
 * @brief A self-organizing traffic light logic based on a particular policy
 *
 * This traffic light logic is used to wrap the logic of a self-organizing
 * low-level policy into a functional traffic light logic. It's role is to
 * use a low-level policy without an high-level policy to command it.
 */

class MSSOTLPolicyBasedTrafficLightLogic: public MSSOTLTrafficLightLogic {
public:
    /**
     * @brief Constructor without sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This traffic light id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] logicType This tls' type (static, actuated etc.)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameters Parameters defined for the tll
     */
    MSSOTLPolicyBasedTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                       const std::string& id, const std::string& programID, const TrafficLightType logicType,
                                       const Phases& phases, int step, SUMOTime delay,
                                       const std::map<std::string, std::string>& parameters,
                                       MSSOTLPolicy* policy);

    /**
     * @brief Constructor with sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] logicType This tls' type (static, actuated etc.)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameters Parameters defined for the tll
     * @param[in] sensors The already defined sensor logic
     */
    MSSOTLPolicyBasedTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                       const std::string& id, const std::string& programID, const TrafficLightType logicType,
                                       const Phases& phases, int step, SUMOTime delay,
                                       const std::map<std::string, std::string>& parameters,
                                       MSSOTLPolicy* policy, MSSOTLSensors* sensors);

    ~MSSOTLPolicyBasedTrafficLightLogic();

    MSSOTLPolicy* getPolicy() {
        return myPolicy;
    }

    /** @brief Returns the type of the logic as a string
    * @return The type of the logic
    */
    const std::string getLogicType() const {
        return "policyBasedTrafficLightLogic";
    }
    /// @}

//	virtual bool canRelease(SUMOTime elapsed, bool thresholdPassed, const MSPhaseDefinition* stage, int vehicleCount) throw ()=0;

protected:

    /*
     * @brief Contains the logic to decide the phase change
     */
    bool canRelease();

    /*
     * This member has to contain the switching logic for SOTL policies
     */
    int decideNextPhase();

private:
    MSSOTLPolicy* myPolicy;
};

