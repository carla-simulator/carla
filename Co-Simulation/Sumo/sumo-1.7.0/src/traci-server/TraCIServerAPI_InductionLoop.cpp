/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCIServerAPI_InductionLoop.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    07.05.2009
///
// APIs for getting/setting induction loop values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/output/MSDetectorControl.h>
#include <libsumo/InductionLoop.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServerAPI_InductionLoop.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_InductionLoop::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
        tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_INDUCTIONLOOP_VARIABLE, variable, id);
    try {
        if (!libsumo::InductionLoop::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::LAST_STEP_VEHICLE_DATA: {
                    std::vector<libsumo::TraCIVehicleData> vd = libsumo::InductionLoop::getVehicleData(id);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    tcpip::Storage tempContent;
                    int cnt = 0;
                    tempContent.writeUnsignedByte(libsumo::TYPE_INTEGER);
                    tempContent.writeInt((int)vd.size());
                    ++cnt;
                    for (const libsumo::TraCIVehicleData& svd : vd) {
                        tempContent.writeUnsignedByte(libsumo::TYPE_STRING);
                        tempContent.writeString(svd.id);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        tempContent.writeDouble(svd.length);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        tempContent.writeDouble(svd.entryTime);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        tempContent.writeDouble(svd.leaveTime);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_STRING);
                        tempContent.writeString(svd.typeID);
                        ++cnt;
                    }
                    server.getWrapperStorage().writeInt((int)cnt);
                    server.getWrapperStorage().writeStorage(tempContent);
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE,
                                                      "Get Induction Loop Variable: unsupported variable " + toHex(variable, 2)
                                                      + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


/****************************************************************************/
