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
/// @file    MSSOTLWaveTrafficLightLogic.h
/// @author  Riccardo Belletti
/// @author  Anna Chiara Bellini
/// @date    2013-10-26
///
// The class for SOTL Platoon logics
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSSOTLTrafficLightLogic.h"
class MSSOTLWaveTrafficLightLogic: public MSSOTLTrafficLightLogic {
public:
    /**
     * @brief Constructor without sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     */
    MSSOTLWaveTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                const std::string& id, const std::string& programID,
                                const Phases& phases, int step, SUMOTime delay,
                                const std::map<std::string, std::string>& parameters);

    /**
     * @brief Constructor with sensors passed
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     */
    MSSOTLWaveTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                const std::string& id, const std::string& programID,
                                const Phases& phases, int step, SUMOTime delay,
                                const std::map<std::string, std::string>& parameters,
                                MSSOTLSensors* sensors);

    /** @brief Returns the type of the logic as a string
     * @return The type of the logic
     */
    const std::string getLogicType() const {
        return "waveTrafficLightLogic";
    }
    /// @}

protected:

    /*
     * @brief Contains the logic to decide whether to release the green light
     */
    bool canRelease();

private:

    /*
     * @brief Counts the vehicles on the green lanes of this phase
     */
    int countVehicles();

};
