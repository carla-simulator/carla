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
/// @file    RODFDetFlowLoader.h
/// @author  Daniel Krajzewicz
/// @author  Eric Nicolay
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A loader for detector flows
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/importio/NamedColumnsParser.h>
#include <utils/common/UtilExceptions.h>
#include "RODFDetector.h"
#include "RODFDetectorFlow.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RODFDetFlowLoader
 * @brief A loader for detector flows
 */
class RODFDetFlowLoader {
public:
    /** @brief Constructor
     *
     * @param[in] dets Definitions of known detectors
     * @param[in, filled] into Container to store read values into
     * @param[in] startTime The first time step for which values shall be read
     * @param[in] endTime The last time step for which values shall be read
     * @param[in] timeOffset The offset which shall be applied to the read times
     */
    RODFDetFlowLoader(const RODFDetectorCon& dets, RODFDetectorFlows& into,
                      SUMOTime startTime, SUMOTime endTime,
                      SUMOTime timeOffset, SUMOTime timeScale);


    /// @brief Destructor
    ~RODFDetFlowLoader();


    /** @brief Reads the given file assuming it contains detector values
     *
     * Reads the first line, first, and parses it assuming it contains
     *  the names of the columns that follow within the next lines.
     *
     * Then, the rest of the file is read and the read values for vehicle/heavy vehicle
     *  amounts and speeds are stored into "myStorage". Values that lie
     *  before "myStartTime" and behind "myEndTime" as well as values
     *  which refer to an unknown detector are omitted.
     *
     * @param[in] file The name of the file to read
     * @exception IOError Not yet implemented!
     * @exception ProcessError Thrown if a value could not be parsed properly or a needed value is missing
     */
    void read(const std::string& file);


private:
    /// @brief The container for read detector values
    RODFDetectorFlows& myStorage;

    /// @brief The time offset to apply to read time values
    const SUMOTime myTimeOffset;

    /// @brief The time scale to apply to read time values
    const SUMOTime myTimeScale;

    /// @brief The first and the last time step to read
    const SUMOTime myStartTime, myEndTime;

    /// @brief The value extractor
    NamedColumnsParser myLineHandler;

    /// @brief Container holding known detectors
    const RODFDetectorCon& myDetectorContainer;

    /// @brief Whether a warning about overriding boundaries was already written
    bool myHaveWarnedAboutOverridingBoundaries;

    /// @brief Whether a warning about partial definitions was already written
    bool myHaveWarnedAboutPartialDefs;


private:
    /// @brief Invalidated copy constructor
    RODFDetFlowLoader(const RODFDetFlowLoader& src);

    /// @brief Invalidated assignment operator
    RODFDetFlowLoader& operator=(const RODFDetFlowLoader& src);

};
