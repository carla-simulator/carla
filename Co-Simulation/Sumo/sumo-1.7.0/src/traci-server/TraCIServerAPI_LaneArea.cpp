/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCIServerAPI_LaneArea.cpp
/// @author  Mario Krumnow
/// @author  Robbin Blokpoel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    03.02.2014
///
// APIs for getting/setting areal detector values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/output/MSDetectorControl.h>
#include <libsumo/LaneArea.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServer.h"
#include "TraCIServerAPI_LaneArea.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_LaneArea::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                    tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_LANEAREA_VARIABLE, variable, id);
    try {
        if (!libsumo::LaneArea::handleVariable(id, variable, &server)) {
            return server.writeErrorStatusCmd(libsumo::CMD_GET_LANEAREA_VARIABLE, "Get Lane Area Detector Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_LANEAREA_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_LANEAREA_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


/****************************************************************************/
