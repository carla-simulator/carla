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
/// @file    TraCIServerAPI_LaneArea.h
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @date    15.09.2013
///
// APIs for getting/setting areal detector values via TraCI
/****************************************************************************/
#pragma once
#include <config.h>

#include <foreign/tcpip/storage.h>


// ===========================================================================
// class declarations
// ===========================================================================
class TraCIServer;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TraCIServerAPI_AreaDetector
 * @brief APIs for getting/setting multi-entry/multi-exit detector values via TraCI
 */
class TraCIServerAPI_LaneArea {
public:
    /** @brief Processes a get value command (Command 0xa1: Get AreaDetector Variable)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);


private:
    /// @brief invalidated copy constructor
    TraCIServerAPI_LaneArea(const TraCIServerAPI_LaneArea& s);

    /// @brief invalidated assignment operator
    TraCIServerAPI_LaneArea& operator=(const TraCIServerAPI_LaneArea& s);


};
