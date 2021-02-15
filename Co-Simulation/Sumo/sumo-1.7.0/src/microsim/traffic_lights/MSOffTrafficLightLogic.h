/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSOffTrafficLightLogic.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    08.05.2007
///
// A traffic lights logic which represents a tls in an off-mode
/****************************************************************************/
#pragma once
#include <config.h>

#include <utility>
#include <vector>
#include <bitset>
#include <utils/common/StdDefs.h>
#include "MSTLLogicControl.h"
#include "MSTrafficLightLogic.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSOffTrafficLightLogic
 * @brief A traffic lights logic which represents a tls in an off-mode
 */
class MSOffTrafficLightLogic : public MSTrafficLightLogic {
public:
    /** @brief Constructor
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * The sub-id is always "off".
     */
    MSOffTrafficLightLogic(MSTLLogicControl& tlcontrol,
                           const std::string& id);


    /** @brief Initialises the tls with information about incoming lanes
     * @param[in] nb The detector builder
     * @param[in] edgeContinuations Information about edge predecessors/successors
     */
    virtual void init(NLDetectorBuilder& nb);


    /// @brief Destructor
    ~MSOffTrafficLightLogic();


    /// @name Handling of controlled links
    /// @{

    /** @brief Applies information about controlled links and lanes from the given logic
     * @param[in] logic The logic to use the information about controlled links/lanes from
     * @see MSTrafficLightLogic::adaptLinkInformationFrom
     */
    void adaptLinkInformationFrom(const MSTrafficLightLogic& logic);
    /// @}



    /// @name Switching and setting current rows
    /// @{

    /** @brief Switches to the next phase
     * @return The time of the next switch (always 120)
     * @see MSTrafficLightLogic::trySwitch
     */
    SUMOTime trySwitch() {
        return 120 * DELTA_T;
    }

    /// @}


    /// @name Static Information Retrieval
    /// @{

    /** @brief Returns the number of phases
     * @return The number of this tls program's phases
     * @see MSTrafficLightLogic::getPhaseNumber
     */
    int getPhaseNumber() const;


    /** @brief Returns the phases of this tls program
     * @return The phases of this tls program
     * @see MSTrafficLightLogic::getPhases
     */
    const Phases& getPhases() const;


    /** @brief Returns the definition of the phase from the given position within the plan
     * @param[in] givenstep The index of the phase within the plan
     * @return The definition of the phase at the given position
     * @see MSTrafficLightLogic::getPhase
     */
    const MSPhaseDefinition& getPhase(int givenstep) const;

    /** @brief Returns the type of the logic as a string
     * @return The type of the logic
     */
    const std::string getLogicType() const {
        return "offTrafficLightLogic";
    }
    /// @}



    /// @name Dynamic Information Retrieval
    /// @{

    /** @brief Returns the current index within the program
     * @return The index of the current phase within the tls
     * @see MSTrafficLightLogic::getCurrentPhaseIndex
     */
    int getCurrentPhaseIndex() const;


    /** @brief Returns the definition of the current phase
     * @return The current phase
     * @see MSTrafficLightLogic::getCurrentPhaseDef
     */
    const MSPhaseDefinition& getCurrentPhaseDef() const;
    /// @}



    /// @name Conversion between time and phase
    /// @{

    /** @brief Returns the index of the logic at the given simulation step
     * @return The (estimated) index of the tls at the given simulation time step
     * @see MSTrafficLightLogic::getPhaseIndexAtTime
     */
    SUMOTime getPhaseIndexAtTime(SUMOTime simStep) const;


    /** @brief Returns the position (start of a phase during a cycle) from of a given step
     * @param[in] index The index of the phase to return the begin of
     * @return The begin time of the phase
     * @see MSTrafficLightLogic::getOffsetFromIndex
     */
    SUMOTime getOffsetFromIndex(int index) const;


    /** @brief Returns the step (the phasenumber) of a given position of the cycle
     * @param[in] offset The offset (time) for which the according phase shall be returned
     * @return The according phase
     * @see MSTrafficLightLogic::getIndexFromOffset
     */
    int getIndexFromOffset(SUMOTime offset) const;
    /// @}



    /// @name Changing phases and phase durations
    /// @{

    /** @brief Changes the current phase and her duration
     * @param[in] tlcontrol The responsible traffic lights control
     * @param[in] simStep The current simulation step
     * @param[in] step Index of the phase to use
     * @param[in] stepDuration The left duration of the phase
     * @see MSTrafficLightLogic::changeStepAndDuration
     */
    void changeStepAndDuration(MSTLLogicControl& tlcontrol, SUMOTime simStep, int step, SUMOTime stepDuration) {
        UNUSED_PARAMETER(tlcontrol);
        UNUSED_PARAMETER(simStep);
        UNUSED_PARAMETER(step);
        UNUSED_PARAMETER(stepDuration);
    }
    /// @}


private:
    /** @brief (Re)builds the internal phase definition
     */
    void rebuildPhase();


private:
    /// @brief The phase definition (only one)
    MSTrafficLightLogic::Phases myPhaseDefinition;


};
