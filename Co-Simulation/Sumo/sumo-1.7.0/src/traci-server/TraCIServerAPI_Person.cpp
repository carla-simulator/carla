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
/// @file    TraCIServerAPI_Person.cpp
/// @author  Daniel Krajzewicz
/// @date    26.05.2014
///
// APIs for getting/setting person values via TraCI
/****************************************************************************/
#include <config.h>

#include <utils/common/StringTokenizer.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <libsumo/Person.h>
#include <libsumo/TraCIConstants.h>
#include <libsumo/VehicleType.h>
#include "TraCIServer.h"
#include "TraCIServerAPI_VehicleType.h"
#include "TraCIServerAPI_Person.h"
#include "TraCIServerAPI_Simulation.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_Person::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                  tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_PERSON_VARIABLE, variable, id);
    try {
        if (!libsumo::Person::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::VAR_EDGES: {
                    int nextStageIndex = 0;
                    if (!server.readTypeCheckingInt(inputStorage, nextStageIndex)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "The message must contain the stage index.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                    server.getWrapperStorage().writeStringList(libsumo::Person::getEdges(id, nextStageIndex));
                    break;
                }
                case libsumo::VAR_STAGE: {
                    int nextStageIndex = 0;
                    if (!server.readTypeCheckingInt(inputStorage, nextStageIndex)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "The message must contain the stage index.", outputStorage);
                    }
                    TraCIServerAPI_Simulation::writeStage(server.getWrapperStorage(), libsumo::Person::getStage(id, nextStageIndex));
                    break;
                }
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Person::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Person::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_TAXI_RESERVATIONS: {
                    int onlyNew = 0;
                    if (!server.readTypeCheckingInt(inputStorage, onlyNew)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "Retrieval of reservations requires an integer flag.", outputStorage);
                    }
                    const std::vector<libsumo::TraCIReservation> result = libsumo::Person::getTaxiReservations(onlyNew);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt((int)result.size());
                    for (const libsumo::TraCIReservation& r : result) {
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                        server.getWrapperStorage().writeInt(9);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                        server.getWrapperStorage().writeString(r.id);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                        server.getWrapperStorage().writeStringList(r.persons);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                        server.getWrapperStorage().writeString(r.group);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                        server.getWrapperStorage().writeString(r.fromEdge);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                        server.getWrapperStorage().writeString(r.toEdge);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        server.getWrapperStorage().writeDouble(r.departPos);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        server.getWrapperStorage().writeDouble(r.arrivalPos);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        server.getWrapperStorage().writeDouble(r.depart);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        server.getWrapperStorage().writeDouble(r.reservationTime);
                    }
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "Get Person Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


bool
TraCIServerAPI_Person::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                  tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != libsumo::VAR_PARAMETER
            && variable != libsumo::ADD
            && variable != libsumo::APPEND_STAGE
            && variable != libsumo::REPLACE_STAGE
            && variable != libsumo::REMOVE_STAGE
            && variable != libsumo::CMD_REROUTE_TRAVELTIME
            && variable != libsumo::MOVE_TO_XY
            && variable != libsumo::VAR_SPEED
            && variable != libsumo::VAR_TYPE
            && variable != libsumo::VAR_LENGTH
            && variable != libsumo::VAR_WIDTH
            && variable != libsumo::VAR_HEIGHT
            && variable != libsumo::VAR_MINGAP
            && variable != libsumo::VAR_COLOR
       ) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Change Person State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }

    try {
        // TODO: remove declaration of c after completion
        MSTransportableControl& c = MSNet::getInstance()->getPersonControl();
        // id
        std::string id = inputStorage.readString();
        // TODO: remove declaration of p after completion
        const bool shouldExist = variable != libsumo::ADD;
        MSTransportable* p = c.get(id);
        if (p == nullptr && shouldExist) {
            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Person '" + id + "' is not known", outputStorage);
        }
        // process
        switch (variable) {
            case libsumo::VAR_SPEED: {
                double speed = 0;
                if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Setting speed requires a double.", outputStorage);
                }
                // set the speed for all (walking) stages
                libsumo::Person::setSpeed(id, speed);
                // modify the vType so that stages added later are also affected
                TraCIServerAPI_VehicleType::setVariable(libsumo::CMD_SET_VEHICLE_VARIABLE, variable, p->getSingularType().getID(), server, inputStorage, outputStorage);
            }
            break;
            case libsumo::VAR_TYPE: {
                std::string vTypeID;
                if (!server.readTypeCheckingString(inputStorage, vTypeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The vehicle type id must be given as a string.", outputStorage);
                }
                libsumo::Person::setType(id, vTypeID);
                break;
            }
            case libsumo::VAR_COLOR: {
                libsumo::TraCIColor col;
                if (!server.readTypeCheckingColor(inputStorage, col)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The color must be given using the according type.", outputStorage);
                }
                libsumo::Person::setColor(id, col);
                break;
            }
            case libsumo::ADD: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Adding a person requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 4) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Adding a person needs four parameters.", outputStorage);
                }
                std::string vTypeID;
                if (!server.readTypeCheckingString(inputStorage, vTypeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "First parameter (type) requires a string.", outputStorage);
                }
                std::string edgeID;
                if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Second parameter (edge) requires a string.", outputStorage);
                }
                double depart;
                if (!server.readTypeCheckingDouble(inputStorage, depart)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Third parameter (depart) requires a double.", outputStorage);
                }
                double pos;
                if (!server.readTypeCheckingDouble(inputStorage, pos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Fourth parameter (position) requires a double.", outputStorage);
                }
                libsumo::Person::add(id, edgeID, pos, depart, vTypeID);
            }
            break;
            case libsumo::APPEND_STAGE: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Adding a person stage requires a compound object.", outputStorage);
                }
                int numParameters = inputStorage.readInt();
                if (numParameters == 13) {
                    libsumo::Person::appendStage(id, *TraCIServerAPI_Simulation::readStage(server, inputStorage));
                } else {
                    int stageType;
                    if (!server.readTypeCheckingInt(inputStorage, stageType)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first parameter for adding a stage must be the stage type given as int.", outputStorage);
                    }
                    if (stageType == libsumo::STAGE_DRIVING) {
                        // append driving stage
                        if (numParameters != 4) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Adding a driving stage needs four parameters.", outputStorage);
                        }
                        std::string edgeID;
                        if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Second parameter (edge) requires a string.", outputStorage);
                        }
                        std::string lines;
                        if (!server.readTypeCheckingString(inputStorage, lines)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Third parameter (lines) requires a string.", outputStorage);
                        }
                        std::string stopID;
                        if (!server.readTypeCheckingString(inputStorage, stopID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Fourth parameter (stopID) requires a string.", outputStorage);
                        }
                        libsumo::Person::appendDrivingStage(id, edgeID, lines, stopID);
                    } else if (stageType == libsumo::STAGE_WAITING) {
                        // append waiting stage
                        if (numParameters != 4) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Adding a waiting stage needs four parameters.", outputStorage);
                        }
                        double duration;
                        if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Second parameter (duration) requires a double.", outputStorage);
                        }
                        std::string description;
                        if (!server.readTypeCheckingString(inputStorage, description)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Third parameter (description) requires a string.", outputStorage);
                        }
                        std::string stopID;
                        if (!server.readTypeCheckingString(inputStorage, stopID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Fourth parameter (stopID) requires a string.", outputStorage);
                        }
                        libsumo::Person::appendWaitingStage(id, duration, description, stopID);
                    } else if (stageType == libsumo::STAGE_WALKING) {
                        // append walking stage
                        if (numParameters != 6) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Adding a walking stage needs six parameters.", outputStorage);
                        }
                        std::vector<std::string> edgeIDs;
                        if (!server.readTypeCheckingStringList(inputStorage, edgeIDs)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Second parameter (edges) route must be defined as a list of edge ids.", outputStorage);
                        }
                        double arrivalPos;
                        if (!server.readTypeCheckingDouble(inputStorage, arrivalPos)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Third parameter (arrivalPos) requires a double.", outputStorage);
                        }
                        double duration;
                        if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Fourth parameter (duration) requires a double.", outputStorage);
                        }
                        double speed;
                        if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Fifth parameter (speed) requires a double.", outputStorage);
                        }
                        std::string stopID;
                        if (!server.readTypeCheckingString(inputStorage, stopID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Fourth parameter (stopID) requires a string.", outputStorage);
                        }
                        libsumo::Person::appendWalkingStage(id, edgeIDs, arrivalPos, duration, speed, stopID);
                    } else {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Invalid stage type for person '" + id + "'", outputStorage);
                    }
                }

            }
            break;

            case libsumo::REPLACE_STAGE : {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Replacing a person stage requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 2) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Replacing a person stage requires a compound object of size 2.", outputStorage);
                }
                int nextStageIndex = 0;
                if (!server.readTypeCheckingInt(inputStorage, nextStageIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "First parameter of replace stage should be an integer", outputStorage);
                }
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Second parameter of replace stage should be a compound object", outputStorage);
                }
                if (inputStorage.readInt() != 13) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Second parameter of replace stage should be a compound object of size 13", outputStorage);
                }
                libsumo::Person::replaceStage(id, nextStageIndex, *TraCIServerAPI_Simulation::readStage(server, inputStorage));
            }
            break;

            case libsumo::REMOVE_STAGE: {
                int nextStageIndex = 0;
                if (!server.readTypeCheckingInt(inputStorage, nextStageIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_PERSON_VARIABLE, "The message must contain the stage index.", outputStorage);
                }
                libsumo::Person::removeStage(id, nextStageIndex);
            }
            break;
            case libsumo::CMD_REROUTE_TRAVELTIME: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Rerouting requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 0) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "Rerouting should obtain an empty compound object.", outputStorage);
                }
                libsumo::Person::rerouteTraveltime(id);
            }
            break;
            case libsumo::MOVE_TO_XY: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "MoveToXY person requires a compound object.", outputStorage);
                }
                const int numArgs = inputStorage.readInt();
                if (numArgs != 5) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "MoveToXY person should obtain: edgeID, x, y, angle and keepRouteFlag.", outputStorage);
                }
                // edge ID
                std::string edgeID;
                if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The first parameter for moveToXY must be the edge ID given as a string.", outputStorage);
                }
                // x
                double x = 0;
                if (!server.readTypeCheckingDouble(inputStorage, x)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The second parameter for moveToXY must be the x-position given as a double.", outputStorage);
                }
                // y
                double y = 0;
                if (!server.readTypeCheckingDouble(inputStorage, y)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The third parameter for moveToXY must be the y-position given as a double.", outputStorage);
                }
                // angle
                double angle = 0;
                if (!server.readTypeCheckingDouble(inputStorage, angle)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The fourth parameter for moveToXY must be the angle given as a double.", outputStorage);
                }
                int keepRouteFlag = 1;
                if (!server.readTypeCheckingByte(inputStorage, keepRouteFlag)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The fifth parameter for moveToXY must be the keepRouteFlag given as a byte.", outputStorage);
                }
                libsumo::Person::moveToXY(id, edgeID, x, y, angle, keepRouteFlag);
            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //read itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                libsumo::Person::setParameter(id, name, value);
            }
            break;
            default:
                try {
                    if (!TraCIServerAPI_VehicleType::setVariable(libsumo::CMD_SET_PERSON_VARIABLE, variable, p->getSingularType().getID(), server, inputStorage, outputStorage)) {
                        return false;
                    }
                } catch (ProcessError& e) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, e.what(), outputStorage);
                }
                break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}


/****************************************************************************/
