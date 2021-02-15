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
/// @file    MSDelayBasedTrafficLightLogic.h
/// @author  Leonhard Luecken
/// @date    Feb 2017
///
// An actuated traffic light logic based on time delay of approaching vehicles
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include "MSSimpleTrafficLightLogic.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NLDetectorBuilder;
class MSE2Collector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDelayBasedTrafficLightLogic
 * @brief An actuated traffic light logic based on time delay of approaching vehicles
 * @todo Validate against the original algorithm's details.
 * @note The current phase is not prolonged if the passing time of the next approaching vehicle
 * is larger than the remaining greentime (in contrast to the original algorithm)
 * @note The maximal green time can be exceeded if no vehicles are present on other approaches;
 */
class MSDelayBasedTrafficLightLogic : public MSSimpleTrafficLightLogic {
public:
    /// @brief Definition of a map from lanes to corresponding areal detectors
//    typedef std::map<MSLane*, MSE2Collector*> LaneDetectorMap;
    typedef std::map<MSLane*, MSE2Collector*> LaneDetectorMap;

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
    MSDelayBasedTrafficLightLogic(MSTLLogicControl& tlcontrol,
                                  const std::string& id, const std::string& programID,
                                  const MSSimpleTrafficLightLogic::Phases& phases,
                                  int step, SUMOTime delay,
                                  const std::map<std::string, std::string>& parameter,
                                  const std::string& basePath);


    /** @brief Initializes the tls with information about incoming lanes
     * @param[in] nb The detector builder
     * @exception ProcessError If something fails on initialization
     */
    void init(NLDetectorBuilder& nb);


    /// @brief Destructor
    ~MSDelayBasedTrafficLightLogic();



    /// @name Switching and setting current rows
    /// @{

    /** @brief Switches to the next phase, if possible
     * @return The time of the next switch
     * @see MSTrafficLightLogic::trySwitch
     */
    SUMOTime trySwitch();
    /// @}

    bool showDetectors() const {
        return myShowDetectors;
    }

    void setShowDetectors(bool show);


protected:
    /// @name "actuated" algorithm methods
    /// @{

    /**
     * @brief Checks for approaching vehicles on the lanes associated with green signals
     *        and returns the minimal time to keep the green phase going.
     *        This is zero if no vehicle on the lane has gathered any waiting time
     *        or if the green time is exhausted (maximal green time has passed since switch).
     * @return Minimal remaining green time.
     */
    SUMOTime checkForWaitingTime();

    /**
     * @brief The returned, proposed prolongation for the green phase is oriented on the
     *        largest estimated passing time among the vehicles with waiting time.
     * @param actDuration Duration of the current phase
     * @param maxDuration Maximal duration of the current phase
     * @param[in/out] othersEmpty Whether there are vehicles on another approach, which is not part of a green signal group for the current phase
     * @return The proposed prolongation time for the current phase
     */
    SUMOTime proposeProlongation(const SUMOTime actDuration, const SUMOTime maxDuration, bool& othersEmpty);

protected:
    /// A map from lanes to the corresponding lane detectors
    LaneDetectorMap myLaneDetectors;

    /// Range of the connected detector, which provides the information on approaching vehicles
    double myDetectionRange;

    /// If a vehicle's timeloss is below myTimeLossThreshold, this is counted as insignificant,
    /// since this may stem from dawdling, or driving only slightly slower than the maximal velocity on the lane.
    // (Idea: this might be adapted to the detector-length and the vehicle's maximal speed)
    double myTimeLossThreshold;

    /// Whether the detectors shall be shown in the GUI
    bool myShowDetectors;

    /// The output file for generated detectors
    std::string myFile;

    /// The frequency for aggregating detector output
    SUMOTime myFreq;

    /// Whether detector output separates by vType
    std::string myVehicleTypes;
};
