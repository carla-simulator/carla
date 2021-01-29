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
/// @file    TraCIServerAPI_TrafficLight.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    07.05.2009
///
// APIs for getting/setting traffic light values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <libsumo/TraCIConstants.h>
#include <libsumo/TrafficLight.h>
#include "TraCIServerAPI_TrafficLight.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_TrafficLight::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                        tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_TL_VARIABLE, variable, id);
    try {
        if (!libsumo::TrafficLight::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::TL_COMPLETE_DEFINITION_RYG: {
                    std::vector<libsumo::TraCILogic> logics = libsumo::TrafficLight::getCompleteRedYellowGreenDefinition(id);
                    tcpip::Storage& storage = server.getWrapperStorage();
                    storage.writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    storage.writeInt((int)logics.size());
                    for (const libsumo::TraCILogic& logic : logics) {
                        storage.writeUnsignedByte(libsumo::TYPE_COMPOUND);
                        storage.writeInt(5);
                        storage.writeUnsignedByte(libsumo::TYPE_STRING);
                        storage.writeString(logic.programID);
                        // type
                        storage.writeUnsignedByte(libsumo::TYPE_INTEGER);
                        storage.writeInt(logic.type);
                        // (current) phase index
                        storage.writeUnsignedByte(libsumo::TYPE_INTEGER);
                        storage.writeInt(logic.currentPhaseIndex);
                        // phase number
                        storage.writeUnsignedByte(libsumo::TYPE_COMPOUND);
                        storage.writeInt((int)logic.phases.size());
                        for (const libsumo::TraCIPhase* phase : logic.phases) {
                            storage.writeUnsignedByte(libsumo::TYPE_COMPOUND);
                            storage.writeInt(6);
                            storage.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                            storage.writeDouble(phase->duration);
                            storage.writeUnsignedByte(libsumo::TYPE_STRING);
                            storage.writeString(phase->state);
                            storage.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                            storage.writeDouble(phase->minDur);
                            storage.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                            storage.writeDouble(phase->maxDur);
                            storage.writeUnsignedByte(libsumo::TYPE_COMPOUND);
                            storage.writeInt((int)phase->next.size());
                            for (int n : phase->next) {
                                storage.writeUnsignedByte(libsumo::TYPE_INTEGER);
                                storage.writeInt(n);
                            }
                            storage.writeUnsignedByte(libsumo::TYPE_STRING);
                            storage.writeString(phase->name);
                        }
                        // subparameter
                        storage.writeUnsignedByte(libsumo::TYPE_COMPOUND);
                        storage.writeInt((int)logic.subParameter.size());
                        for (const auto& item : logic.subParameter) {
                            storage.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                            storage.writeInt(2);
                            storage.writeString(item.first);
                            storage.writeString(item.second);
                        }
                    }
                    break;
                }
                case libsumo::TL_CONTROLLED_LINKS: {
                    const std::vector<std::vector<libsumo::TraCILink> > links = libsumo::TrafficLight::getControlledLinks(id);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    tcpip::Storage tempContent;
                    int cnt = 0;
                    tempContent.writeUnsignedByte(libsumo::TYPE_INTEGER);
                    tempContent.writeInt((int)links.size());
                    for (const std::vector<libsumo::TraCILink>& sublinks : links) {
                        tempContent.writeUnsignedByte(libsumo::TYPE_INTEGER);
                        tempContent.writeInt((int)sublinks.size());
                        ++cnt;
                        for (const libsumo::TraCILink& link : sublinks) {
                            tempContent.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                            tempContent.writeStringList(std::vector<std::string>({ link.fromLane, link.toLane, link.viaLane }));
                            ++cnt;
                        }
                    }
                    server.getWrapperStorage().writeInt(cnt);
                    server.getWrapperStorage().writeStorage(tempContent);
                    break;
                }
                case libsumo::VAR_PERSON_NUMBER: {
                    int index = 0;
                    if (!server.readTypeCheckingInt(inputStorage, index)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The phase index must be given as an integer.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
                    server.getWrapperStorage().writeInt(libsumo::TrafficLight::getServedPersonCount(id, index));
                    break;
                }
                case libsumo::TL_BLOCKING_VEHICLES: {
                    int index = 0;
                    if (!server.readTypeCheckingInt(inputStorage, index)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The link index must be given as an integer.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                    server.getWrapperStorage().writeStringList(libsumo::TrafficLight::getBlockingVehicles(id, index));
                    break;
                }
                case libsumo::TL_RIVAL_VEHICLES: {
                    int index = 0;
                    if (!server.readTypeCheckingInt(inputStorage, index)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The link index must be given as an integer.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                    server.getWrapperStorage().writeStringList(libsumo::TrafficLight::getRivalVehicles(id, index));
                    break;
                }
                case libsumo::TL_PRIORITY_VEHICLES: {
                    int index = 0;
                    if (!server.readTypeCheckingInt(inputStorage, index)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The link index must be given as an integer.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                    server.getWrapperStorage().writeStringList(libsumo::TrafficLight::getPriorityVehicles(id, index));
                    break;
                }
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_TL_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::TrafficLight::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_TL_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::TrafficLight::getParameter(id, paramName));
                    break;
                }
                case libsumo::TL_EXTERNAL_STATE: {
                    if (!MSNet::getInstance()->getTLSControl().knows(id)) {
                        throw libsumo::TraCIException("Traffic light '" + id + "' is not known");
                    }
                    MSTrafficLightLogic* tls = MSNet::getInstance()->getTLSControl().get(id).getActive();
                    const std::string& state = tls->getCurrentPhaseDef().getState();
                    const std::map<std::string, std::string>& params = tls->getParametersMap();
                    int num = 0;
                    for (std::map<std::string, std::string>::const_iterator i = params.begin(); i != params.end(); ++i) {
                        if ("connection:" == (*i).first.substr(0, 11)) {
                            ++num;
                        }
                    }

                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
                    server.getWrapperStorage().writeInt(num * 2);
                    for (std::map<std::string, std::string>::const_iterator i = params.begin(); i != params.end(); ++i) {
                        if ("connection:" != (*i).first.substr(0, 11)) {
                            continue;
                        }
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                        server.getWrapperStorage().writeString((*i).second); // foreign id
                        std::string connection = (*i).first.substr(11);
                        std::string from, to;
                        const std::string::size_type b = connection.find("->");
                        if (b == std::string::npos) {
                            from = connection;
                        } else {
                            from = connection.substr(0, b);
                            to = connection.substr(b + 2);
                        }
                        bool denotesEdge = from.find("_") == std::string::npos;
                        MSLane* fromLane = nullptr;
                        const MSTrafficLightLogic::LaneVectorVector& lanes = tls->getLaneVectors();
                        MSTrafficLightLogic::LaneVectorVector::const_iterator j = lanes.begin();
                        for (; j != lanes.end() && fromLane == nullptr;) {
                            for (MSTrafficLightLogic::LaneVector::const_iterator k = (*j).begin(); k != (*j).end() && fromLane == nullptr;) {
                                if (denotesEdge && (*k)->getEdge().getID() == from) {
                                    fromLane = *k;
                                } else if (!denotesEdge && (*k)->getID() == from) {
                                    fromLane = *k;
                                }
                                if (fromLane == nullptr) {
                                    ++k;
                                }
                            }
                            if (fromLane == nullptr) {
                                ++j;
                            }
                        }
                        if (fromLane == nullptr) {
                            return server.writeErrorStatusCmd(libsumo::CMD_GET_TL_VARIABLE, "Could not find edge or lane '" + from + "' in traffic light '" + id + "'.", outputStorage);
                        }
                        int pos = (int)std::distance(lanes.begin(), j);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_UBYTE);
                        server.getWrapperStorage().writeUnsignedByte(state[pos]); // state
                    }
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_TL_VARIABLE, "Get TLS Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_TL_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_TL_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


bool
TraCIServerAPI_TrafficLight::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                        tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    const int variable = inputStorage.readUnsignedByte();
    if (variable != libsumo::TL_PHASE_INDEX && variable != libsumo::TL_PROGRAM && variable != libsumo::TL_PHASE_DURATION
            && variable != libsumo::TL_RED_YELLOW_GREEN_STATE && variable != libsumo::TL_COMPLETE_PROGRAM_RYG
            && variable != libsumo::VAR_NAME
            && variable != libsumo::VAR_PARAMETER) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "Change TLS State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }
    const std::string id = inputStorage.readString();
    try {
        switch (variable) {
            case libsumo::TL_PHASE_INDEX: {
                int index = 0;
                if (!server.readTypeCheckingInt(inputStorage, index)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The phase index must be given as an integer.", outputStorage);
                }
                libsumo::TrafficLight::setPhase(id, index);
            }
            break;
            case libsumo::VAR_NAME: {
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The phase name must be given as a string.", outputStorage);
                }
                libsumo::TrafficLight::setPhaseName(id, name);
            }
            break;
            case libsumo::TL_PROGRAM: {
                std::string subID;
                if (!server.readTypeCheckingString(inputStorage, subID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The program must be given as a string.", outputStorage);
                }
                libsumo::TrafficLight::setProgram(id, subID);
            }
            break;
            case libsumo::TL_PHASE_DURATION: {
                double duration = 0.;
                if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The phase duration must be given as a double.", outputStorage);
                }
                libsumo::TrafficLight::setPhaseDuration(id, duration);
            }
            break;
            case libsumo::TL_RED_YELLOW_GREEN_STATE: {
                std::string state;
                if (!server.readTypeCheckingString(inputStorage, state)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The phase must be given as a string.", outputStorage);
                }
                libsumo::TrafficLight::setRedYellowGreenState(id, state);
            }
            break;
            case libsumo::TL_COMPLETE_PROGRAM_RYG: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "A compound object is needed for setting a new program.", outputStorage);
                }
                //read itemNo
                inputStorage.readInt();
                libsumo::TraCILogic logic;
                if (!server.readTypeCheckingString(inputStorage, logic.programID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 1. parameter (programID) must be a string.", outputStorage);
                }
                if (!server.readTypeCheckingInt(inputStorage, logic.type)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 2. parameter (type) must be an int.", outputStorage);
                }
                if (!server.readTypeCheckingInt(inputStorage, logic.currentPhaseIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 3. parameter (index) must be an int.", outputStorage);
                }
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "A compound object is needed for the phases.", outputStorage);
                }
                const int numPhases = inputStorage.readInt();
                for (int j = 0; j < numPhases; ++j) {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "A compound object is needed for every phase.", outputStorage);
                    }
                    const int items = inputStorage.readInt();
                    if (items != 6 && items != 5) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "A phase compound object requires 5 or 6 items.", outputStorage);
                    }
                    double duration = 0., minDuration = 0., maxDuration = 0.;
                    std::vector<int> next;
                    std::string name;
                    if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 4.1. parameter (duration) must be a double.", outputStorage);
                    }
                    std::string state;
                    if (!server.readTypeCheckingString(inputStorage, state)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 4.2. parameter (phase) must be a string.", outputStorage);
                    }
                    if (!server.readTypeCheckingDouble(inputStorage, minDuration)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 4.3. parameter (min duration) must be a double.", outputStorage);
                    }
                    if (!server.readTypeCheckingDouble(inputStorage, maxDuration)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 4.4. parameter (max duration) must be a double.", outputStorage);
                    }
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program 4.5 parameter (next) must be a compound (list of ints).", outputStorage);
                    }
                    const int numNext = inputStorage.readInt();
                    for (int k = 0; k < numNext; k++) {
                        int nextEntry;
                        if (!server.readTypeCheckingInt(inputStorage, nextEntry)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 4.5. parameter (next) must be a list of int.", outputStorage);
                        }
                        next.push_back(nextEntry);
                    }
                    if (items == 6) {
                        if (!server.readTypeCheckingString(inputStorage, name)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 4.6. parameter (name) must be a string.", outputStorage);
                        }
                    }
                    logic.phases.emplace_back(new libsumo::TraCIPhase(duration, state, minDuration, maxDuration, next, name));
                }
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "set program: 5. parameter (subparams) must be a compound object.", outputStorage);
                }
                const int numParams = inputStorage.readInt();
                for (int j = 0; j < numParams; j++) {
                    std::vector<std::string> par;
                    server.readTypeCheckingStringList(inputStorage, par);
                    logic.subParameter[par[0]] = par[1];
                }
                libsumo::TrafficLight::setCompleteRedYellowGreenDefinition(id, logic);
            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //read itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                libsumo::TrafficLight::setParameter(id, name, value);
            }
            break;
            default:
                break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_TL_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_TL_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}


/****************************************************************************/
