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
/// @file    MSSOTLHiLevelTrafficLightLogic.h
/// @author  Alessio Bonfietti
/// @author  Riccardo Belletti
/// @author  Anna Chiara Bellini
/// @author  Federico Caselli
/// @date    Jun 2013
///
/****************************************************************************/
#pragma once
#include <config.h>

//#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include <vector>
#include "MSSOTLPolicy.h"
#include <stdlib.h>
#include <sstream>

#include "MSSOTLTrafficLightLogic.h"

/**
 * @class MSSOTLHiLevelTrafficLightLogic
 * @brief A self-organizing high-level traffic light logic
 *
 * A base class for a high-level traffic light logic able
 * to manage different low-level policies for different
 * traffic conditions.
 */

class MSSOTLHiLevelTrafficLightLogic: public MSSOTLTrafficLightLogic {
public:
    /**
     * @brief Constructor without sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This traffic light id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameters Parameters defined for the tll
     */
    MSSOTLHiLevelTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                   const std::string& id, const std::string& programID,
                                   const TrafficLightType logicType, const Phases& phases,
                                   int step, SUMOTime delay,
                                   const std::map<std::string, std::string>& parameters);

    /**
     * @brief Constructor with sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameters Parameters defined for the tll
     * @param[in] sensors The already defined sensor logic
     */
    MSSOTLHiLevelTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                   const std::string& id, const std::string& programID,
                                   const TrafficLightType logicType, const Phases& phases,
                                   int step, SUMOTime delay,
                                   const std::map<std::string, std::string>& parameters,
                                   MSSOTLSensors* sensors);
    ~MSSOTLHiLevelTrafficLightLogic();

    /**
     * \brief Returns the vector of the low-level policies used by this high-level tll.
     */
    std::vector<MSSOTLPolicy*>& getPolicies() {
        return policies;
    }
    /**
     * \brief Returns the low-level policy currently selected by this high-level tll.
     */
    MSSOTLPolicy* getCurrentPolicy() {
        return currentPolicy;
    }

    /**
     * @brief Initialises the tls
     * @param[in] nb The detector builder
     */
    void init(NLDetectorBuilder& nb);

    /*
     * \brief Adds a low-level policy to this high-level tll.
     * \param[in] policy The low-level policy to be added.
     */
    void addPolicy(MSSOTLPolicy* policy);

    /*
     * \brief Activates the given low-level policy.
     * \param[in] policy The low-level policy to be activated.
     */
    void activate(MSSOTLPolicy* policy);

protected:
    virtual void decidePolicy() = 0;

private:

    std::vector<MSSOTLPolicy*> policies;
    MSSOTLPolicy* currentPolicy;

};

