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
/// @file    TraCIServerAPI_Simulation.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    07.05.2009
///
// APIs for getting/setting edge values via TraCI
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <vector>
#include "TraCIServer.h"
#include <foreign/tcpip/storage.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TraCIServerAPI_Simulation
 * @brief APIs for getting/setting simulation values via TraCI
 */
class TraCIServerAPI_Simulation {
public:
    /** @brief Processes a get value command (Command 0xab: Get Simulation Variable)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);

    /** @brief Processes a set value command (Command 0xcb: Set Simulation Variable)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);

    static void writeStage(tcpip::Storage& outputStorage, const libsumo::TraCIStage& stage);
    static libsumo::TraCIStage* readStage(TraCIServer& server, tcpip::Storage& inputStorage);

private:
    /**
     * Converts a road map position to a cartesian position
     *
     * @param pos road map position that is to be convertes
     * @return closest 2D position
     */

    static bool commandPositionConversion(TraCIServer& server, tcpip::Storage& inputStorage,
                                          const int compoundSize, tcpip::Storage& outputStorage,
                                          const int commandId);

    static bool commandDistanceRequest(TraCIServer& server, tcpip::Storage& inputStorage,
                                       tcpip::Storage& outputStorage, int commandId);

    static void writeVehicleStateNumber(TraCIServer& server, tcpip::Storage& outputStorage, MSNet::VehicleState state);
    static void writeVehicleStateIDs(TraCIServer& server, tcpip::Storage& outputStorage, MSNet::VehicleState state);


private:
    /// @brief invalidated copy constructor
    TraCIServerAPI_Simulation(const TraCIServerAPI_Simulation& s);

    /// @brief invalidated assignment operator
    TraCIServerAPI_Simulation& operator=(const TraCIServerAPI_Simulation& s);


};
