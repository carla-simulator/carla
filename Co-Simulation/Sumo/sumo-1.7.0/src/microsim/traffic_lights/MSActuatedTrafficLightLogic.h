/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSActuatedTrafficLightLogic.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// An actuated (adaptive) traffic light logic
/****************************************************************************/
#pragma once
#include <config.h>

#include <utility>
#include <vector>
#include <bitset>
#include <map>
#include <microsim/MSEventControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include "MSSimpleTrafficLightLogic.h"
#include <microsim/output/MSInductLoop.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NLDetectorBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSActuatedTrafficLightLogic
 * @brief An actuated (adaptive) traffic light logic
 */
class MSActuatedTrafficLightLogic : public MSSimpleTrafficLightLogic {
public:
    /** @brief Constructor
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] phases Definitions of the phases
     * @param[in] step The initial phase index
     * @param[in] delay The time to wait before the first switch
     * @param[in] parameter The parameter to use for tls set-up
     */
    MSActuatedTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                const std::string& id, const std::string& programID,
                                const MSSimpleTrafficLightLogic::Phases& phases,
                                int step, SUMOTime delay,
                                const std::map<std::string, std::string>& parameter,
                                const std::string& basePath);


    /** @brief Initialises the tls with information about incoming lanes
     * @param[in] nb The detector builder
     * @exception ProcessError If something fails on initialisation
     */
    void init(NLDetectorBuilder& nb);


    /// @brief Destructor
    ~MSActuatedTrafficLightLogic();



    /// @name Switching and setting current rows
    /// @{

    /** @brief Switches to the next phase
     * @return The time of the next switch
     * @see MSTrafficLightLogic::trySwitch
     */
    SUMOTime trySwitch();
    /// @}

    /// @brief called when switching programs
    void activateProgram();
    void deactivateProgram();

    bool showDetectors() const {
        return myShowDetectors;
    }

    void setShowDetectors(bool show);

    /**@brief Sets a parameter and updates internal constants */
    void setParameter(const std::string& key, const std::string& value);

protected:
    struct InductLoopInfo {
        InductLoopInfo(MSInductLoop* _loop, int numPhases):
            loop(_loop),
            servedPhase(numPhases, false)
        {}
        MSInductLoop* loop;
        SUMOTime lastGreenTime = 0;
        std::vector<bool> servedPhase;
    };

    /// @brief Definition of a map from phases to induct loops controlling them
    typedef std::vector<std::vector<InductLoopInfo*> > InductLoopMap;

    /// @name "actuated" algorithm methods
    /// @{

    /** @brief Returns the minimum duration of the current phase
     * @param[in] detectionGap The minimum detection gap for the current phase
     * @return The minimum duration of the current phase
     */
    SUMOTime duration(const double detectionGap) const;

    /// @brief get the minimum min duration for all stretchable phases that affect the given lane
    SUMOTime getMinimumMinDuration(MSLane* lane) const;

    /** @brief Return the minimum detection gap of all detectors if the current phase should be extended and double::max otherwise
     */
    double gapControl();


    /// @brief return whether there is a major link from the given lane in the given phase
    bool hasMajor(const std::string& state, const LaneVector& lanes) const;
    /// @}

    /// @brief select am candidate phases based on detector states
    int decideNextPhase();

    int getDetectorPriority(const InductLoopInfo& loopInfo) const;

    /// @brief count the number of active detectors for the given step
    int getPhasePriority(int step) const;

    /// @brief get the green phase following step
    int getTarget(int step);

    /// @brief whether the current phase cannot be continued due to linkMaxDur constraints
    bool maxLinkDurationReached();

    /// @brief whether the target phase is acceptable in light of linkMaxDur constraints
    bool canExtendLinkGreen(int target);

    /// @brief the minimum duratin for keeping the current phase due to linkMinDur constraints
    SUMOTime getLinkMinDuration(int target) const;

protected:
    /// @brief A map from phase to induction loops to be used for gap control
    InductLoopMap myInductLoopsForPhase;

    std::vector<InductLoopInfo> myInductLoops;


    /// The maximum gap to check in seconds
    double myMaxGap;

    /// The passing time used in seconds
    double myPassingTime;

    /// The detector distance in seconds
    double myDetectorGap;

    /// The time threshold to avoid starved phases
    SUMOTime myInactiveThreshold;

    /// Whether the detectors shall be shown in the GUI
    bool myShowDetectors;

    /// The output file for generated detectors
    std::string myFile;

    /// The frequency for aggregating detector output
    SUMOTime myFreq;

    /// Whether detector output separates by vType
    std::string myVehicleTypes;

    /// @brief last time trySwitch was called
    SUMOTime myLastTrySwitchTime;
    /// @brief consecutive time that the given link index has been green
    std::vector<SUMOTime> myLinkGreenTimes;
    /// @brief maximum consecutive time that the given link may remain green
    std::vector<SUMOTime> myLinkMaxGreenTimes;
    /// @brief minimum consecutive time that the given link must remain green
    std::vector<SUMOTime> myLinkMinGreenTimes;
};
