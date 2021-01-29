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
/// @file    TraCIServerAPI_Route.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    07.05.2009
///
// APIs for getting/setting route values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSRoute.h>
#include <microsim/MSEdge.h>
#include <libsumo/Route.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServerAPI_Route.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_Route::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                 tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_ROUTE_VARIABLE, variable, id);
    try {
        if (!libsumo::Route::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_ROUTE_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Route::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_ROUTE_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Route::getParameter(id, paramName));
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_ROUTE_VARIABLE, "Get Route Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_ROUTE_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_ROUTE_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


bool
TraCIServerAPI_Route::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                 tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != libsumo::ADD && variable != libsumo::VAR_PARAMETER) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, "Change Route State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }
    // id
    std::string id = inputStorage.readString();

    try {
        // process
        switch (variable) {
            case libsumo::ADD: {
                std::vector<std::string> edgeIDs;
                if (!server.readTypeCheckingStringList(inputStorage, edgeIDs)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, "A string list is needed for adding a new route.", outputStorage);
                }
                libsumo::Route::add(id, edgeIDs);
            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //read itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                libsumo::Route::setParameter(id, name, value);
            }
            break;
            default:
                break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_ROUTE_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}


/****************************************************************************/
