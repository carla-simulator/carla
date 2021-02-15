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
/// @file    TraCIServerAPI_MultiEntryExit.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    07.05.2009
///
// APIs for getting/setting multi-entry/multi-exit detector values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/output/MSDetectorControl.h>
#include <libsumo/MultiEntryExit.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServerAPI_MultiEntryExit.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_MultiEntryExit::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
        tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, variable, id);
    try {
        if (!libsumo::MultiEntryExit::handleVariable(id, variable, &server)) {
            return server.writeErrorStatusCmd(libsumo::CMD_GET_MULTIENTRYEXIT_VARIABLE, "Get Multi Entry Exit Detector Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_MULTIENTRYEXIT_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_MULTIENTRYEXIT_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


/****************************************************************************/
