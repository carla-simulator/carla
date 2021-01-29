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
/// @file    MSSOTLPolicy.h
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @author  Anna Chiara Bellini
/// @author  Federico Caselli
/// @date    Jun 2013
///
// The class for low-level policy
/****************************************************************************/

#pragma once
#include <config.h>

//#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include <sstream>
#include <cmath>
#include <utility>
#include <vector>
#include <utils/common/Parameterised.h>
#include "MSPhaseDefinition.h"
#include "MSSOTLPolicyDesirability.h"

class PushButtonLogic {
protected:
    void init(std::string prefix, const Parameterised* parameterised);

    bool pushButtonLogic(SUMOTime elapsed, bool pushButtonPressed, const MSPhaseDefinition* stage);

    double m_pushButtonScaleFactor;
    std::string m_prefix;
};

class SigmoidLogic {
protected:
    void init(std::string prefix, const Parameterised* parameterised);

    bool sigmoidLogic(SUMOTime elapsed, const MSPhaseDefinition* stage, int vehicleCount);

    bool m_useSigmoid;
    double m_k;
    std::string m_prefix;
};

/**
 * @class MSSOTLPolicy
 * @brief Class for a low-level policy.
 *
 */
class MSSOTLPolicy: public Parameterised {
private:

    /**
     * \brief The sensitivity of this policy
     */
    double theta_sensitivity;
    /**
     * \brief The name of the policy
     */
    std::string myName;
    /**
     * \brief A pointer to the policy desirability object.\nIt's an optional component related to the computeDesirability() method and it's necessary
     * only when the policy is used in combination with an high level policy.
     */
    MSSOTLPolicyDesirability* myDesirabilityAlgorithm;

protected:
    virtual void init() {}

public:
    /** @brief Simple constructor
     * @param[in] name The name of the policy
     * @param[in] parameters Parameters defined for the policy
     */
    MSSOTLPolicy(std::string name,
                 const std::map<std::string, std::string>& parameters);
    /** @brief Constructor when the policy is a low-level policy used by an high level policy
     * @param[in] name The name of the policy
     * @param[in] desirabilityAlgorithm The desirability algorithm to be used for this policy
     */
    MSSOTLPolicy(std::string name, MSSOTLPolicyDesirability* desirabilityAlgorithm);
    /** @brief Constructor when the policy is a low-level policy used by an high level policy
     * @param[in] name The name of the policy
     * @param[in] desirabilityAlgorithm The desirability algorithm to be used for this policy
     * @param[in] parameters Parameters defined for the policy
     */
    MSSOTLPolicy(std::string name, MSSOTLPolicyDesirability* desirabilityAlgorithm,
                 const std::map<std::string, std::string>& parameters);
    virtual ~MSSOTLPolicy();

    virtual bool canRelease(SUMOTime elapsed, bool thresholdPassed, bool pushButtonPressed,
                            const MSPhaseDefinition* stage, int vehicleCount) = 0;
    virtual int decideNextPhase(SUMOTime elapsed, const MSPhaseDefinition* stage,
                                int currentPhaseIndex, int phaseMaxCTS, bool thresholdPassed, bool pushButtonPressed,
                                int vehicleCount);

    virtual double getThetaSensitivity() {
        return theta_sensitivity;
    }
    virtual void setThetaSensitivity(double val) {
        theta_sensitivity = val;
    }
    std::string getName() {
        return myName;
    }
    MSSOTLPolicyDesirability* getDesirabilityAlgorithm() {
        return myDesirabilityAlgorithm;
    }
    /**
     * @brief Computes the desirability of this policy, necessary when used in combination with an high level policy.
     */
    double computeDesirability(double vehInMeasure, double vehOutMeasure, double vehInDispersionMeasure, double vehOutDispersionMeasure);

    double computeDesirability(double vehInMeasure, double vehOutMeasure);
};

