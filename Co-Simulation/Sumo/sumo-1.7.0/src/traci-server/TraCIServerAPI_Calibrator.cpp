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
/// @file    TraCIServerAPI_Calibrator.cpp
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// APIs for getting/setting Calibrator values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/trigger/MSCalibrator.h>
#include <libsumo/Calibrator.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServerAPI_Calibrator.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_Calibrator::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                      tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_CALIBRATOR_VARIABLE, variable, id);
    try {
        if (!libsumo::Calibrator::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_CALIBRATOR_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Calibrator::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_CALIBRATOR_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Calibrator::getParameter(id, paramName));
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_CALIBRATOR_VARIABLE, "Get Calibrator Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_CALIBRATOR_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_CALIBRATOR_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


bool
TraCIServerAPI_Calibrator::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                      tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != libsumo::CMD_SET_FLOW && variable != libsumo::VAR_PARAMETER) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Change Calibrator State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }
    // id
    std::string id = inputStorage.readString();

    try {
        // process
        switch (variable) {
            case libsumo::CMD_SET_FLOW: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "A compound object is needed for setting calibrator flow.", outputStorage);
                }
                int parameterCount = inputStorage.readInt();
                double begin;
                double end;
                double vehsPerHour;
                double speed;
                std::string typeID;
                std::string routeID;
                std::string departLane;
                std::string departSpeed;
                if (parameterCount == 8) {
                    if (!server.readTypeCheckingDouble(inputStorage, begin)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the begin time as the first value.", outputStorage);
                    }
                    if (!server.readTypeCheckingDouble(inputStorage, end)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the end time as the second value.", outputStorage);
                    }
                    if (!server.readTypeCheckingDouble(inputStorage, vehsPerHour)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the number of vehicles per hour as the third value.", outputStorage);
                    }
                    if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the speed as the fourth value.", outputStorage);
                    }
                    if (!server.readTypeCheckingString(inputStorage, typeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the type id as the fifth value.", outputStorage);
                    }
                    if (!server.readTypeCheckingString(inputStorage, routeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the route id as the sixth value.", outputStorage);
                    }
                    if (!server.readTypeCheckingString(inputStorage, departLane)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the departLane as the seventh value.", outputStorage);
                    }
                    if (!server.readTypeCheckingString(inputStorage, departSpeed)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "Setting flow requires the departSpeed as the eigth value.", outputStorage);
                    }
                } else {
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Setting calibrator flow requires 8 parameters.", outputStorage);
                }
                libsumo::Calibrator::setFlow(id, begin, end, vehsPerHour, speed, typeID, routeID, departLane, departSpeed);
            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //read itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                libsumo::Calibrator::setParameter(id, name, value);
            }
            break;
            default:
                break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_CALIBRATOR_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}


/****************************************************************************/
