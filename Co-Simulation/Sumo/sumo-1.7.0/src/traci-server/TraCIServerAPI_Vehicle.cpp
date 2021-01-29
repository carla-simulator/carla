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
/// @file    TraCIServerAPI_Vehicle.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Christoph Sommer
/// @author  Michael Behrisch
/// @author  Bjoern Hendriks
/// @author  Mario Krumnow
/// @author  Jakob Erdmann
/// @author  Leonhard Luecken
/// @author  Robert Hilbrich
/// @author  Lara Codeca
/// @date    07.05.2009
///
// APIs for getting/setting vehicle values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSGlobals.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <utils/geom/PositionVector.h>
#include <utils/router/DijkstraRouter.h>
#include <utils/router/DijkstraRouter.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/emissions/HelpersHarmonoise.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <libsumo/TraCIConstants.h>
#include <libsumo/Vehicle.h>
#include <libsumo/VehicleType.h>
#include "TraCIServerAPI_Simulation.h"
#include "TraCIServerAPI_Vehicle.h"
#include "TraCIServerAPI_VehicleType.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_Vehicle::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                   tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_VEHICLE_VARIABLE, variable, id);
    try {
        if (!libsumo::Vehicle::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::VAR_LEADER: {
                    double dist = 0;
                    if (!server.readTypeCheckingDouble(inputStorage, dist)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Leader retrieval requires a double.", outputStorage);
                    }
                    std::pair<std::string, double> leaderInfo = libsumo::Vehicle::getLeader(id, dist);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(leaderInfo.first);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(leaderInfo.second);
                    break;
                }
                case libsumo::VAR_FOLLOWER: {
                    double dist = 0;
                    if (!server.readTypeCheckingDouble(inputStorage, dist)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Follower retrieval requires a double.", outputStorage);
                    }
                    std::pair<std::string, double> followerInfo = libsumo::Vehicle::getFollower(id, dist);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(followerInfo.first);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(followerInfo.second);
                    break;
                }
                case libsumo::VAR_EDGE_TRAVELTIME: {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of travel time requires a compound object.", outputStorage);
                    }
                    if (inputStorage.readInt() != 2) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of travel time requires time and edge as parameter.", outputStorage);
                    }
                    double time = 0.;
                    if (!server.readTypeCheckingDouble(inputStorage, time)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of travel time requires the referenced time as first parameter.", outputStorage);
                    }
                    // edge
                    std::string edgeID;
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of travel time requires the referenced edge as second parameter.", outputStorage);
                    }
                    // retrieve
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(libsumo::Vehicle::getAdaptedTraveltime(id, time, edgeID));
                    break;
                }
                case libsumo::VAR_EDGE_EFFORT: {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of effort requires a compound object.", outputStorage);
                    }
                    if (inputStorage.readInt() != 2) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of effort requires time and edge as parameter.", outputStorage);
                    }
                    double time = 0.;
                    if (!server.readTypeCheckingDouble(inputStorage, time)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of effort requires the referenced time as first parameter.", outputStorage);
                    }
                    // edge
                    std::string edgeID;
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of effort requires the referenced edge as second parameter.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(libsumo::Vehicle::getEffort(id, time, edgeID));
                    break;
                }
                case libsumo::VAR_BEST_LANES: {
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    tcpip::Storage tempContent;
                    int cnt = 0;
                    tempContent.writeUnsignedByte(libsumo::TYPE_INTEGER);
                    std::vector<libsumo::TraCIBestLanesData> bestLanes = libsumo::Vehicle::getBestLanes(id);
                    tempContent.writeInt((int)bestLanes.size());
                    ++cnt;
                    for (std::vector<libsumo::TraCIBestLanesData>::const_iterator i = bestLanes.begin(); i != bestLanes.end(); ++i) {
                        const libsumo::TraCIBestLanesData& bld = *i;
                        tempContent.writeUnsignedByte(libsumo::TYPE_STRING);
                        tempContent.writeString(bld.laneID);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        tempContent.writeDouble(bld.length);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        tempContent.writeDouble(bld.occupation);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_BYTE);
                        tempContent.writeByte(bld.bestLaneOffset);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_UBYTE);
                        bld.allowsContinuation ? tempContent.writeUnsignedByte(1) : tempContent.writeUnsignedByte(0);
                        ++cnt;
                        tempContent.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                        tempContent.writeStringList(bld.continuationLanes);
                        ++cnt;
                    }
                    server.getWrapperStorage().writeInt((int)cnt);
                    server.getWrapperStorage().writeStorage(tempContent);
                    break;
                }
                case libsumo::VAR_NEXT_TLS: {
                    std::vector<libsumo::TraCINextTLSData> nextTLS = libsumo::Vehicle::getNextTLS(id);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    const int cnt = 1 + (int)nextTLS.size() * 4;
                    server.getWrapperStorage().writeInt(cnt);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
                    server.getWrapperStorage().writeInt((int)nextTLS.size());
                    for (std::vector<libsumo::TraCINextTLSData>::iterator it = nextTLS.begin(); it != nextTLS.end(); ++it) {
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                        server.getWrapperStorage().writeString(it->id);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
                        server.getWrapperStorage().writeInt(it->tlIndex);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                        server.getWrapperStorage().writeDouble(it->dist);
                        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_BYTE);
                        server.getWrapperStorage().writeByte(it->state);
                    }
                    break;
                }
                case libsumo::VAR_NEXT_STOPS2: {
                    // deliberate fallThrough!
                    int limit = 0;
                    if (!server.readTypeCheckingInt(inputStorage, limit)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Stop retrieval uses an optional integer.", outputStorage);
                    }
                    writeNextStops(server, id, limit, true);
                    break;
                }
                case libsumo::VAR_NEXT_STOPS: {
                    writeNextStops(server, id, 0, false);
                    break;
                }
                case libsumo::DISTANCE_REQUEST: {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of distance requires a compound object.", outputStorage);
                    }
                    if (inputStorage.readInt() != 2) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of distance requires position and distance type as parameter.", outputStorage);
                    }

                    // read position
                    int posType = inputStorage.readUnsignedByte();
                    switch (posType) {
                        case libsumo::POSITION_ROADMAP:
                            try {
                                const std::string roadID = inputStorage.readString();
                                const double edgePos = inputStorage.readDouble();
                                const int laneIndex = inputStorage.readUnsignedByte();
                                server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                                server.getWrapperStorage().writeDouble(libsumo::Vehicle::getDrivingDistance(id, roadID, edgePos, laneIndex));
                                break;
                            } catch (libsumo::TraCIException& e) {
                                return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, e.what(), outputStorage);
                            }
                        case libsumo::POSITION_2D:
                        case libsumo::POSITION_3D: {
                            const double p1x = inputStorage.readDouble();
                            const double p1y = inputStorage.readDouble();
                            if (posType == libsumo::POSITION_3D) {
                                inputStorage.readDouble();        // z value is ignored
                            }
                            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                            server.getWrapperStorage().writeDouble(libsumo::Vehicle::getDrivingDistance2D(id, p1x, p1y));
                            break;
                        }
                        default:
                            return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Unknown position format used for distance request", outputStorage);
                    }
                    // read distance type
                    int distType = inputStorage.readUnsignedByte();
                    if (distType != libsumo::REQUEST_DRIVINGDIST) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Only driving distance is supported for vehicles.", outputStorage);
                    }
                    break;
                }
                case libsumo::CMD_CHANGELANE: {
                    int direction = 0;
                    if (!server.readTypeCheckingInt(inputStorage, direction)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of lane change state requires a direction as int.", outputStorage);
                    }
                    const std::pair<int, int> state = libsumo::Vehicle::getLaneChangeState(id, direction);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
                    server.getWrapperStorage().writeInt(state.first);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
                    server.getWrapperStorage().writeInt(state.second);
                    break;
                }
                case libsumo::VAR_TAXI_FLEET: {
                    int flag = 0;
                    if (!server.readTypeCheckingInt(inputStorage, flag)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of taxi fleet requires an integer flag.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRINGLIST);
                    server.getWrapperStorage().writeStringList(libsumo::Vehicle::getTaxiFleet(flag));
                    break;
                }
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Vehicle::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Vehicle::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_NEIGHBORS: {
                    int mode;
                    if (!server.readTypeCheckingUnsignedByte(inputStorage, mode)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of neighboring vehicles needs bitset to specify mode.", outputStorage);
                    }
                    const std::vector<std::pair<std::string, double> >& neighVehicles = libsumo::Vehicle::getNeighbors(id, mode);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt((int)neighVehicles.size());
                    for (auto& p : neighVehicles) {
                        server.getWrapperStorage().writeString(p.first);
                        server.getWrapperStorage().writeDouble(p.second);
                    }
                    break;
                }
                case libsumo::VAR_FOLLOW_SPEED: {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires requires a compound object.", outputStorage);
                    }
                    int parameterCount = inputStorage.readInt();
                    double speed;
                    double gap;
                    double leaderSpeed;
                    double leaderMaxDecel;
                    std::string leaderID;
                    if (parameterCount == 5) {
                        // speed
                        if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires the speed as first parameter.", outputStorage);
                        }
                        // gap
                        if (!server.readTypeCheckingDouble(inputStorage, gap)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires the gap as second parameter.", outputStorage);
                        }
                        // leaderSpeed
                        if (!server.readTypeCheckingDouble(inputStorage, leaderSpeed)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires the leaderSpeed as third parameter.", outputStorage);
                        }
                        // leaderMaxDecel
                        if (!server.readTypeCheckingDouble(inputStorage, leaderMaxDecel)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires the leaderMaxDecel as fourth parameter.", outputStorage);
                        }
                        // leaderID
                        if (!server.readTypeCheckingString(inputStorage, leaderID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires the leaderID as fifth parameter.", outputStorage);
                        }
                    } else {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of followSpeed requires 5 parameters.", outputStorage);
                    }
                    // retrieve
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(libsumo::Vehicle::getFollowSpeed(id, speed, gap, leaderSpeed, leaderMaxDecel, leaderID));
                }
                break;
                case libsumo::VAR_SECURE_GAP: {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of secureGap requires requires a compound object.", outputStorage);
                    }
                    int parameterCount = inputStorage.readInt();
                    double speed;
                    double leaderSpeed;
                    double leaderMaxDecel;
                    std::string leaderID;
                    if (parameterCount == 4) {
                        // speed
                        if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of secureGap requires the speed as first parameter.", outputStorage);
                        }
                        // leaderSpeed
                        if (!server.readTypeCheckingDouble(inputStorage, leaderSpeed)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of secureGap requires the leaderSpeed as second parameter.", outputStorage);
                        }
                        // leaderMaxDecel
                        if (!server.readTypeCheckingDouble(inputStorage, leaderMaxDecel)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of secureGap requires the leaderMaxDecel as third parameter.", outputStorage);
                        }
                        // leaderID
                        if (!server.readTypeCheckingString(inputStorage, leaderID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of secureGap requires the leaderID as fourth parameter.", outputStorage);
                        }
                    } else {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of secureGap requires 4 parameters.", outputStorage);
                    }
                    // retrieve
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(libsumo::Vehicle::getSecureGap(id, speed, leaderSpeed, leaderMaxDecel, leaderID));
                }
                break;
                case libsumo::VAR_STOP_SPEED: {
                    if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of stopSpeed requires requires a compound object.", outputStorage);
                    }
                    int parameterCount = inputStorage.readInt();
                    double speed;
                    double gap;
                    if (parameterCount == 2) {
                        // speed
                        if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of stopSpeed requires the speed as first parameter.", outputStorage);
                        }
                        // gap
                        if (!server.readTypeCheckingDouble(inputStorage, gap)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Retrieval of stopSpeed requires the gap as second parameter.", outputStorage);
                        }
                    } else {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Retrieval of stopSpeed requires 2 parameters.", outputStorage);
                    }
                    // retrieve
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
                    server.getWrapperStorage().writeDouble(libsumo::Vehicle::getStopSpeed(id, speed, gap));
                }
                break;
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Get Vehicle Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


bool
TraCIServerAPI_Vehicle::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                   tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != libsumo::CMD_STOP && variable != libsumo::CMD_CHANGELANE
            && variable != libsumo::CMD_REROUTE_TO_PARKING
            && variable != libsumo::CMD_CHANGESUBLANE && variable != libsumo::CMD_OPENGAP
            && variable != libsumo::CMD_REPLACE_STOP
            && variable != libsumo::CMD_SLOWDOWN && variable != libsumo::CMD_CHANGETARGET && variable != libsumo::CMD_RESUME
            && variable != libsumo::VAR_TYPE && variable != libsumo::VAR_ROUTE_ID && variable != libsumo::VAR_ROUTE
            && variable != libsumo::VAR_UPDATE_BESTLANES
            && variable != libsumo::VAR_EDGE_TRAVELTIME && variable != libsumo::VAR_EDGE_EFFORT
            && variable != libsumo::CMD_REROUTE_TRAVELTIME && variable != libsumo::CMD_REROUTE_EFFORT
            && variable != libsumo::VAR_SIGNALS && variable != libsumo::VAR_MOVE_TO
            && variable != libsumo::VAR_LENGTH && variable != libsumo::VAR_MAXSPEED && variable != libsumo::VAR_VEHICLECLASS
            && variable != libsumo::VAR_SPEED_FACTOR && variable != libsumo::VAR_EMISSIONCLASS
            && variable != libsumo::VAR_WIDTH && variable != libsumo::VAR_MINGAP && variable != libsumo::VAR_SHAPECLASS
            && variable != libsumo::VAR_ACCEL && variable != libsumo::VAR_DECEL && variable != libsumo::VAR_IMPERFECTION
            && variable != libsumo::VAR_APPARENT_DECEL && variable != libsumo::VAR_EMERGENCY_DECEL
            && variable != libsumo::VAR_ACTIONSTEPLENGTH
            && variable != libsumo::VAR_TAU && variable != libsumo::VAR_LANECHANGE_MODE
            && variable != libsumo::VAR_SPEED && variable != libsumo::VAR_PREV_SPEED && variable != libsumo::VAR_SPEEDSETMODE && variable != libsumo::VAR_COLOR
            && variable != libsumo::ADD && variable != libsumo::ADD_FULL && variable != libsumo::REMOVE
            && variable != libsumo::VAR_HEIGHT
            && variable != libsumo::VAR_ROUTING_MODE
            && variable != libsumo::VAR_LATALIGNMENT
            && variable != libsumo::VAR_MAXSPEED_LAT
            && variable != libsumo::VAR_MINGAP_LAT
            && variable != libsumo::VAR_LINE
            && variable != libsumo::VAR_VIA
            && variable != libsumo::VAR_HIGHLIGHT
            && variable != libsumo::CMD_TAXI_DISPATCH
            && variable != libsumo::MOVE_TO_XY && variable != libsumo::VAR_PARAMETER/* && variable != libsumo::VAR_SPEED_TIME_LINE && variable != libsumo::VAR_LANE_TIME_LINE*/
       ) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Change Vehicle State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }
    // id
    std::string id = inputStorage.readString();
#ifdef DEBUG_MOVEXY
    std::cout << SIMTIME << " processSet veh=" << id << "\n";
#endif
    const bool shouldExist = variable != libsumo::ADD && variable != libsumo::ADD_FULL;
    SUMOVehicle* sumoVehicle = MSNet::getInstance()->getVehicleControl().getVehicle(id);
    if (sumoVehicle == nullptr) {
        if (shouldExist) {
            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Vehicle '" + id + "' is not known", outputStorage);
        }
    }
    MSBaseVehicle* v = dynamic_cast<MSBaseVehicle*>(sumoVehicle);
    if (v == nullptr && shouldExist) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "Vehicle '" + id + "' is not a proper vehicle", outputStorage);
    }
    try {
        switch (variable) {
            case libsumo::CMD_STOP: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Stop needs a compound object description.", outputStorage);
                }
                int compoundSize = inputStorage.readInt();
                if (compoundSize < 4 || compoundSize > 7) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Stop needs a compound object description of four to seven items.", outputStorage);
                }
                // read road map position
                std::string edgeID;
                if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first stop parameter must be the edge id given as a string.", outputStorage);
                }
                double pos = 0;
                if (!server.readTypeCheckingDouble(inputStorage, pos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second stop parameter must be the end position along the edge given as a double.", outputStorage);
                }
                int laneIndex = 0;
                if (!server.readTypeCheckingByte(inputStorage, laneIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third stop parameter must be the lane index given as a byte.", outputStorage);
                }
                // waitTime
                double duration = libsumo::INVALID_DOUBLE_VALUE;
                if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "The fourth stop parameter must be the stopping duration given as a double.", outputStorage);
                }
                int stopFlags = 0;
                if (compoundSize >= 5) {
                    if (!server.readTypeCheckingByte(inputStorage, stopFlags)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fifth stop parameter must be a byte indicating its parking/triggered status.", outputStorage);
                    }
                }
                double startPos = libsumo::INVALID_DOUBLE_VALUE;
                if (compoundSize >= 6) {
                    if (!server.readTypeCheckingDouble(inputStorage, startPos)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The sixth stop parameter must be the start position along the edge given as a double.", outputStorage);
                    }
                }
                double until = libsumo::INVALID_DOUBLE_VALUE;
                if (compoundSize >= 7) {
                    if (!server.readTypeCheckingDouble(inputStorage, until)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The seventh stop parameter must be the minimum departure time given as a double.", outputStorage);
                    }
                }
                libsumo::Vehicle::setStop(id, edgeID, pos, laneIndex, duration, stopFlags, startPos, until);
            }
            break;
            case libsumo::CMD_REPLACE_STOP: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Replacing stop needs a compound object description.", outputStorage);
                }
                int compoundSize = inputStorage.readInt();
                if (compoundSize != 8) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Replacing stop needs a compound object description of eight items.", outputStorage);
                }
                // read road map position
                std::string edgeID;
                if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first stop replacement parameter must be the edge id given as a string.", outputStorage);
                }
                double pos = 0;
                if (!server.readTypeCheckingDouble(inputStorage, pos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second stop replacement parameter must be the end position along the edge given as a double.", outputStorage);
                }
                int laneIndex = 0;
                if (!server.readTypeCheckingByte(inputStorage, laneIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third stop replacement parameter must be the lane index given as a byte.", outputStorage);
                }
                // waitTime
                double duration = libsumo::INVALID_DOUBLE_VALUE;
                if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_VEHICLE_VARIABLE, "The fourth stop replacement parameter must be the stopping duration given as a double.", outputStorage);
                }
                int stopFlags = 0;
                if (!server.readTypeCheckingInt(inputStorage, stopFlags)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fifth stop replacement parameter must be a int indicating its parking/triggered status.", outputStorage);
                }
                double startPos = libsumo::INVALID_DOUBLE_VALUE;
                if (!server.readTypeCheckingDouble(inputStorage, startPos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The sixth stop replacement parameter must be the start position along the edge given as a double.", outputStorage);
                }
                double until = libsumo::INVALID_DOUBLE_VALUE;
                if (!server.readTypeCheckingDouble(inputStorage, until)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The seventh stop replacement parameter must be the minimum departure time given as a double.", outputStorage);
                }
                int nextStopIndex = 0;
                if (!server.readTypeCheckingInt(inputStorage, nextStopIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The eigth stop replacement parameter must be the replacement index given as a int.", outputStorage);
                }
                libsumo::Vehicle::replaceStop(id, nextStopIndex, edgeID, pos, laneIndex, duration, stopFlags, startPos, until);
            }
            break;
            case libsumo::CMD_REROUTE_TO_PARKING: {
                // read variables
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Reroute to stop needs a compound object description.", outputStorage);
                }
                int compoundSize = inputStorage.readInt();
                if (compoundSize != 1) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Reroute to stop needs a compound object description of 1 item.", outputStorage);
                }
                std::string parkingAreaID;
                if (!server.readTypeCheckingString(inputStorage, parkingAreaID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first reroute to stop parameter must be the parking area id given as a string.", outputStorage);
                }
                libsumo::Vehicle::rerouteParkingArea(id, parkingAreaID);
            }
            break;
            case libsumo::CMD_RESUME: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    server.writeStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::RTYPE_ERR, "Resuming requires a compound object.", outputStorage);
                    return false;
                }
                if (inputStorage.readInt() != 0) {
                    server.writeStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::RTYPE_ERR, "Resuming should obtain an empty compound object.", outputStorage);
                    return false;
                }
                libsumo::Vehicle::resume(id);
            }
            break;
            case libsumo::CMD_CHANGELANE: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Lane change needs a compound object description.", outputStorage);
                }
                int compounds = inputStorage.readInt();
                if (compounds != 3 && compounds != 2) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Lane change needs a compound object description of two or three items.", outputStorage);
                }
                // Lane ID
                int laneIndex = 0;
                if (!server.readTypeCheckingByte(inputStorage, laneIndex)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first lane change parameter must be the lane index given as a byte.", outputStorage);
                }
                // duration
                double duration = 0.;
                if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second lane change parameter must be the duration given as a double.", outputStorage);
                }
                // relativelanechange
                int relative = 0;
                if (compounds == 3) {
                    if (!server.readTypeCheckingByte(inputStorage, relative)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third lane change parameter must be a Byte for defining whether a relative lane change should be applied.", outputStorage);
                    }
                }

                if ((laneIndex < 0 || laneIndex >= (int)v->getEdge()->getLanes().size()) && relative < 1) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "No lane with index '" + toString(laneIndex) + "' on road '" + v->getEdge()->getID() + "'.", outputStorage);
                }

                if (relative < 1) {
                    libsumo::Vehicle::changeLane(id, laneIndex, duration);
                } else {
                    libsumo::Vehicle::changeLaneRelative(id, laneIndex, duration);
                }
            }
            break;
            case libsumo::CMD_CHANGESUBLANE: {
                double latDist = 0;
                if (!server.readTypeCheckingDouble(inputStorage, latDist)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Sublane-changing requires a double.", outputStorage);
                }
                libsumo::Vehicle::changeSublane(id, latDist);
            }
            break;
            case libsumo::CMD_SLOWDOWN: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Slow down needs a compound object description.", outputStorage);
                }
                if (inputStorage.readInt() != 2) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Slow down needs a compound object description of two items.", outputStorage);
                }
                double newSpeed = 0;
                if (!server.readTypeCheckingDouble(inputStorage, newSpeed)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first slow down parameter must be the speed given as a double.", outputStorage);
                }
                if (newSpeed < 0) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Speed must not be negative", outputStorage);
                }
                double duration = 0.;
                if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second slow down parameter must be the duration given as a double.", outputStorage);
                }
                if (duration < 0 || SIMTIME + duration > STEPS2TIME(SUMOTime_MAX - DELTA_T)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Invalid time interval", outputStorage);
                }
                libsumo::Vehicle::slowDown(id, newSpeed, duration);
            }
            break;
            case libsumo::CMD_CHANGETARGET: {
                std::string edgeID;
                if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Change target requires a string containing the id of the new destination edge as parameter.", outputStorage);
                }
                libsumo::Vehicle::changeTarget(id, edgeID);
            }
            break;
            case libsumo::CMD_OPENGAP: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Create gap needs a compound object description.", outputStorage);
                }
                const int nParameter = inputStorage.readInt();
                if (nParameter != 5 && nParameter != 6) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Create gap needs a compound object description of five or six items.", outputStorage);
                }
                double newTimeHeadway = 0;
                if (!server.readTypeCheckingDouble(inputStorage, newTimeHeadway)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first create gap parameter must be the new desired time headway (tau) given as a double.", outputStorage);
                }
                double newSpaceHeadway = 0;
                if (!server.readTypeCheckingDouble(inputStorage, newSpaceHeadway)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second create gap parameter must be the new desired space headway given as a double.", outputStorage);
                }
                double duration = 0.;
                if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third create gap parameter must be the duration given as a double.", outputStorage);
                }
                double changeRate = 0;
                if (!server.readTypeCheckingDouble(inputStorage, changeRate)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fourth create gap parameter must be the change rate given as a double.", outputStorage);
                }
                double maxDecel = 0;
                if (!server.readTypeCheckingDouble(inputStorage, maxDecel)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fifth create gap parameter must be the maximal braking rate given as a double.", outputStorage);
                }

                if (newTimeHeadway == -1 && newSpaceHeadway == -1 && duration == -1 && changeRate == -1 && maxDecel == -1) {
                    libsumo::Vehicle::deactivateGapControl(id);
                } else {
                    if (newTimeHeadway <= 0) {
                        if (newTimeHeadway != -1) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The value for the new desired time headway (tau) must be positive for create gap", outputStorage);
                        } // else if == -1: keep vehicles current headway, see libsumo::Vehicle::openGap
                    }
                    if (newSpaceHeadway < 0) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The value for the new desired space headway must be non-negative for create gap", outputStorage);
                    }
                    if ((duration < 0 && duration != -1)  || SIMTIME + duration > STEPS2TIME(SUMOTime_MAX - DELTA_T)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Invalid time interval for create gap", outputStorage);
                    }
                    if (changeRate <= 0) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The value for the change rate must be positive for the openGap command", outputStorage);
                    }
                    if (maxDecel <= 0) {
                        if (maxDecel != -1) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The value for the maximal braking rate must be positive for the openGap command", outputStorage);
                        } // else if == -1: don't limit cf model's suggested brake rate, see libsumo::Vehicle::openGap
                    }
                    std::string refVehID = "";
                    if (nParameter == 6) {
                        if (!server.readTypeCheckingString(inputStorage, refVehID)) {
                            return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The sixth create gap parameter must be a reference vehicle's ID given as a string.", outputStorage);
                        }
                    }
                    libsumo::Vehicle::openGap(id, newTimeHeadway, newSpaceHeadway, duration, changeRate, maxDecel, refVehID);
                }
            }
            break;
            case libsumo::VAR_TYPE: {
                std::string vTypeID;
                if (!server.readTypeCheckingString(inputStorage, vTypeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The vehicle type id must be given as a string.", outputStorage);
                }
                libsumo::Vehicle::setType(id, vTypeID);
            }
            break;
            case libsumo::VAR_ROUTE_ID: {
                std::string rid;
                if (!server.readTypeCheckingString(inputStorage, rid)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The route id must be given as a string.", outputStorage);
                }
                libsumo::Vehicle::setRouteID(id, rid);
            }
            break;
            case libsumo::VAR_ROUTE: {
                std::vector<std::string> edgeIDs;
                if (!server.readTypeCheckingStringList(inputStorage, edgeIDs)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "A route must be defined as a list of edge ids.", outputStorage);
                }
                libsumo::Vehicle::setRoute(id, edgeIDs);
            }
            break;
            case libsumo::VAR_EDGE_TRAVELTIME: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time requires a compound object.", outputStorage);
                }
                int parameterCount = inputStorage.readInt();
                std::string edgeID;
                double begTime = 0.;
                double endTime = std::numeric_limits<double>::max();
                double value = libsumo::INVALID_DOUBLE_VALUE;
                if (parameterCount == 4) {
                    // begin time
                    if (!server.readTypeCheckingDouble(inputStorage, begTime)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 4 parameters requires the begin time as first parameter.", outputStorage);
                    }
                    // begin time
                    if (!server.readTypeCheckingDouble(inputStorage, endTime)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 4 parameters requires the end time as second parameter.", outputStorage);
                    }
                    // edge
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 4 parameters requires the referenced edge as third parameter.", outputStorage);
                    }
                    // value
                    if (!server.readTypeCheckingDouble(inputStorage, value)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 4 parameters requires the travel time as double as fourth parameter.", outputStorage);
                    }
                } else if (parameterCount == 2) {
                    // edge
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 2 parameters requires the referenced edge as first parameter.", outputStorage);
                    }
                    // value
                    if (!server.readTypeCheckingDouble(inputStorage, value)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 2 parameters requires the travel time as second parameter.", outputStorage);
                    }
                } else if (parameterCount == 1) {
                    // edge
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time using 1 parameter requires the referenced edge as first parameter.", outputStorage);
                    }
                } else {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting travel time requires 1, 2, or 4 parameters.", outputStorage);
                }
                libsumo::Vehicle::setAdaptedTraveltime(id, edgeID, value, begTime, endTime);
            }
            break;
            case libsumo::VAR_EDGE_EFFORT: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort requires a compound object.", outputStorage);
                }
                int parameterCount = inputStorage.readInt();
                std::string edgeID;
                double begTime = 0.;
                double endTime = std::numeric_limits<double>::max();
                double value = libsumo::INVALID_DOUBLE_VALUE;
                if (parameterCount == 4) {
                    // begin time
                    if (!server.readTypeCheckingDouble(inputStorage, begTime)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 4 parameters requires the begin time as first parameter.", outputStorage);
                    }
                    // begin time
                    if (!server.readTypeCheckingDouble(inputStorage, endTime)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 4 parameters requires the end time as second parameter.", outputStorage);
                    }
                    // edge
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 4 parameters requires the referenced edge as third parameter.", outputStorage);
                    }
                    // value
                    if (!server.readTypeCheckingDouble(inputStorage, value)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 4 parameters requires the travel time as fourth parameter.", outputStorage);
                    }
                } else if (parameterCount == 2) {
                    // edge
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 2 parameters requires the referenced edge as first parameter.", outputStorage);
                    }
                    if (!server.readTypeCheckingDouble(inputStorage, value)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 2 parameters requires the travel time as second parameter.", outputStorage);
                    }
                } else if (parameterCount == 1) {
                    // edge
                    if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort using 1 parameter requires the referenced edge as first parameter.", outputStorage);
                    }
                } else {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting effort requires 1, 2, or 4 parameters.", outputStorage);
                }
                // retrieve
                libsumo::Vehicle::setEffort(id, edgeID, value, begTime, endTime);
            }
            break;
            case libsumo::CMD_REROUTE_TRAVELTIME: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Rerouting requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 0) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Rerouting should obtain an empty compound object.", outputStorage);
                }
                libsumo::Vehicle::rerouteTraveltime(id);
            }
            break;
            case libsumo::CMD_REROUTE_EFFORT: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Rerouting requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 0) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Rerouting should obtain an empty compound object.", outputStorage);
                }
                libsumo::Vehicle::rerouteEffort(id);
            }
            break;
            case libsumo::VAR_SIGNALS: {
                int signals = 0;
                if (!server.readTypeCheckingInt(inputStorage, signals)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting signals requires an integer.", outputStorage);
                }
                libsumo::Vehicle::setSignals(id, signals);
            }
            break;
            case libsumo::VAR_MOVE_TO: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting position requires a compound object.", outputStorage);
                }
                const int numArgs = inputStorage.readInt();
                if (numArgs < 2 || numArgs > 3) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting position should obtain the lane id and the position and optionally the reason.", outputStorage);
                }
                // lane ID
                std::string laneID;
                if (!server.readTypeCheckingString(inputStorage, laneID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first parameter for setting a position must be the lane ID given as a string.", outputStorage);
                }
                // position on lane
                double position = 0;
                if (!server.readTypeCheckingDouble(inputStorage, position)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second parameter for setting a position must be the position given as a double.", outputStorage);
                }
                int reason = libsumo::MOVE_AUTOMATIC;
                if (numArgs == 3) {
                    if (!server.readTypeCheckingInt(inputStorage, reason)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third parameter for setting a position must be the reason given as an int.", outputStorage);
                    }
                }
                // process
                libsumo::Vehicle::moveTo(id, laneID, position, reason);
            }
            break;
            case libsumo::VAR_SPEED: {
                double speed = 0;
                if (!server.readTypeCheckingDouble(inputStorage, speed)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting speed requires a double.", outputStorage);
                }
                libsumo::Vehicle::setSpeed(id, speed);
            }
            break;
            case libsumo::VAR_PREV_SPEED: {
                double prevspeed = 0;
                if (!server.readTypeCheckingDouble(inputStorage, prevspeed)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting previous speed requires a double.", outputStorage);
                }
                libsumo::Vehicle::setPreviousSpeed(id, prevspeed);
            }
            break;
            case libsumo::VAR_SPEEDSETMODE: {
                int speedMode = 0;
                if (!server.readTypeCheckingInt(inputStorage, speedMode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting speed mode requires an integer.", outputStorage);
                }
                libsumo::Vehicle::setSpeedMode(id, speedMode);
            }
            break;
            case libsumo::VAR_LANECHANGE_MODE: {
                int laneChangeMode = 0;
                if (!server.readTypeCheckingInt(inputStorage, laneChangeMode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting lane change mode requires an integer.", outputStorage);
                }
                libsumo::Vehicle::setLaneChangeMode(id, laneChangeMode);
            }
            break;
            case libsumo::VAR_ROUTING_MODE: {
                int routingMode = 0;
                if (!server.readTypeCheckingInt(inputStorage, routingMode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting routing mode requires an integer.", outputStorage);
                }
                libsumo::Vehicle::setRoutingMode(id, routingMode);
            }
            break;
            case libsumo::VAR_COLOR: {
                libsumo::TraCIColor col;
                if (!server.readTypeCheckingColor(inputStorage, col)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The color must be given using the according type.", outputStorage);
                }
                libsumo::Vehicle::setColor(id, col);
                break;
            }
            case libsumo::ADD: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Adding a vehicle requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 6) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Adding a vehicle needs six parameters.", outputStorage);
                }
                std::string vTypeID;
                if (!server.readTypeCheckingString(inputStorage, vTypeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "First parameter (type) requires a string.", outputStorage);
                }
                std::string routeID;
                if (!server.readTypeCheckingString(inputStorage, routeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Second parameter (route) requires a string.", outputStorage);
                }
                int departCode;
                if (!server.readTypeCheckingInt(inputStorage, departCode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Third parameter (depart) requires an integer.", outputStorage);
                }
                std::string depart = toString(STEPS2TIME(departCode));
                if (-departCode == DEPART_TRIGGERED) {
                    depart = "triggered";
                } else if (-departCode == DEPART_CONTAINER_TRIGGERED) {
                    depart = "containerTriggered";
                } else if (-departCode == DEPART_NOW) {
                    depart = "now";
                }

                double departPosCode;
                if (!server.readTypeCheckingDouble(inputStorage, departPosCode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Fourth parameter (position) requires a double.", outputStorage);
                }
                std::string departPos = toString(departPosCode);
                if (-departPosCode == (int)DepartPosDefinition::RANDOM) {
                    departPos = "random";
                } else if (-departPosCode == (int)DepartPosDefinition::RANDOM_FREE) {
                    departPos = "random_free";
                } else if (-departPosCode == (int)DepartPosDefinition::FREE) {
                    departPos = "free";
                } else if (-departPosCode == (int)DepartPosDefinition::BASE) {
                    departPos = "base";
                } else if (-departPosCode == (int)DepartPosDefinition::LAST) {
                    departPos = "last";
                } else if (-departPosCode == (int)DepartPosDefinition::GIVEN) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Invalid departure position.", outputStorage);
                }

                double departSpeedCode;
                if (!server.readTypeCheckingDouble(inputStorage, departSpeedCode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Fifth parameter (speed) requires a double.", outputStorage);
                }
                std::string departSpeed = toString(departSpeedCode);
                if (-departSpeedCode == (int)DepartSpeedDefinition::RANDOM) {
                    departSpeed = "random";
                } else if (-departSpeedCode == (int)DepartSpeedDefinition::MAX) {
                    departSpeed = "max";
                } else if (-departSpeedCode == (int)DepartSpeedDefinition::DESIRED) {
                    departSpeed = "desired";
                } else if (-departSpeedCode == (int)DepartSpeedDefinition::LIMIT) {
                    departSpeed = "speedLimit";
                }

                int departLaneCode;
                if (!server.readTypeCheckingByte(inputStorage, departLaneCode)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Sixth parameter (lane) requires a byte.", outputStorage);
                }
                std::string departLane = toString(departLaneCode);
                if (-departLaneCode == (int)DepartLaneDefinition::RANDOM) {
                    departLane = "random";
                } else if (-departLaneCode == (int)DepartLaneDefinition::FREE) {
                    departLane = "free";
                } else if (-departLaneCode == (int)DepartLaneDefinition::ALLOWED_FREE) {
                    departLane = "allowed";
                } else if (-departLaneCode == (int)DepartLaneDefinition::BEST_FREE) {
                    departLane = "best";
                } else if (-departLaneCode == (int)DepartLaneDefinition::FIRST_ALLOWED) {
                    departLane = "first";
                }
                libsumo::Vehicle::add(id, routeID, vTypeID, depart, departLane, departPos, departSpeed);
            }
            break;
            case libsumo::ADD_FULL: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Adding a vehicle requires a compound object.", outputStorage);
                }
                if (inputStorage.readInt() != 14) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Adding a fully specified vehicle needs fourteen parameters.", outputStorage);
                }
                std::string routeID;
                if (!server.readTypeCheckingString(inputStorage, routeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Second parameter (route) requires a string.", outputStorage);
                }
                std::string vTypeID;
                if (!server.readTypeCheckingString(inputStorage, vTypeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "First parameter (type) requires a string.", outputStorage);
                }
                std::string depart;
                if (!server.readTypeCheckingString(inputStorage, depart)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Third parameter (depart) requires an string.", outputStorage);
                }
                std::string departLane;
                if (!server.readTypeCheckingString(inputStorage, departLane)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Fourth parameter (depart lane) requires a string.", outputStorage);
                }
                std::string departPos;
                if (!server.readTypeCheckingString(inputStorage, departPos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Fifth parameter (depart position) requires a string.", outputStorage);
                }
                std::string departSpeed;
                if (!server.readTypeCheckingString(inputStorage, departSpeed)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Sixth parameter (depart speed) requires a string.", outputStorage);
                }
                std::string arrivalLane;
                if (!server.readTypeCheckingString(inputStorage, arrivalLane)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Seventh parameter (arrival lane) requires a string.", outputStorage);
                }
                std::string arrivalPos;
                if (!server.readTypeCheckingString(inputStorage, arrivalPos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Eighth parameter (arrival position) requires a string.", outputStorage);
                }
                std::string arrivalSpeed;
                if (!server.readTypeCheckingString(inputStorage, arrivalSpeed)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Ninth parameter (arrival speed) requires a string.", outputStorage);
                }
                std::string fromTaz;
                if (!server.readTypeCheckingString(inputStorage, fromTaz)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Tenth parameter (from taz) requires a string.", outputStorage);
                }
                std::string toTaz;
                if (!server.readTypeCheckingString(inputStorage, toTaz)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Eleventh parameter (to taz) requires a string.", outputStorage);
                }
                std::string line;
                if (!server.readTypeCheckingString(inputStorage, line)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Twelth parameter (line) requires a string.", outputStorage);
                }
                int personCapacity;
                if (!server.readTypeCheckingInt(inputStorage, personCapacity)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "13th parameter (person capacity) requires an int.", outputStorage);
                }
                int personNumber;
                if (!server.readTypeCheckingInt(inputStorage, personNumber)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "14th parameter (person number) requires an int.", outputStorage);
                }
                libsumo::Vehicle::add(id, routeID, vTypeID, depart, departLane, departPos, departSpeed, arrivalLane, arrivalPos, arrivalSpeed,
                                      fromTaz, toTaz, line, personCapacity, personNumber);
            }
            break;
            case libsumo::REMOVE: {
                int why = 0;
                if (!server.readTypeCheckingByte(inputStorage, why)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Removing a vehicle requires a byte.", outputStorage);
                }
                libsumo::Vehicle::remove(id, (char)why);
            }
            break;
            case libsumo::MOVE_TO_XY: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "MoveToXY vehicle requires a compound object.", outputStorage);
                }
                const int numArgs = inputStorage.readInt();
                if (numArgs != 5 && numArgs != 6) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "MoveToXY vehicle should obtain: edgeID, lane, x, y, angle and optionally keepRouteFlag.", outputStorage);
                }
                // edge ID
                std::string edgeID;
                if (!server.readTypeCheckingString(inputStorage, edgeID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first parameter for moveToXY must be the edge ID given as a string.", outputStorage);
                }
                // lane index
                int laneNum = 0;
                if (!server.readTypeCheckingInt(inputStorage, laneNum)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The second parameter for moveToXY must be lane given as an int.", outputStorage);
                }
                // x
                double x = 0;
                if (!server.readTypeCheckingDouble(inputStorage, x)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third parameter for moveToXY must be the x-position given as a double.", outputStorage);
                }
                // y
                double y = 0;
                if (!server.readTypeCheckingDouble(inputStorage, y)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fourth parameter for moveToXY must be the y-position given as a double.", outputStorage);
                }
                // angle
                double angle = 0;
                if (!server.readTypeCheckingDouble(inputStorage, angle)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fifth parameter for moveToXY must be the angle given as a double.", outputStorage);
                }

                int keepRouteFlag = 1;
                if (numArgs == 6) {
                    if (!server.readTypeCheckingByte(inputStorage, keepRouteFlag)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The sixth parameter for moveToXY must be the keepRouteFlag given as a byte.", outputStorage);
                    }
                }
                libsumo::Vehicle::moveToXY(id, edgeID, laneNum, x, y, angle, keepRouteFlag);
            }
            break;
            case libsumo::VAR_SPEED_FACTOR: {
                double factor = 0;
                if (!server.readTypeCheckingDouble(inputStorage, factor)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting speed factor requires a double.", outputStorage);
                }
                libsumo::Vehicle::setSpeedFactor(id, factor);
            }
            break;
            case libsumo::VAR_LINE: {
                std::string line;
                if (!server.readTypeCheckingString(inputStorage, line)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The line must be given as a string.", outputStorage);
                }
                libsumo::Vehicle::setLine(id, line);
            }
            break;
            case libsumo::VAR_VIA: {
                std::vector<std::string> edgeIDs;
                if (!server.readTypeCheckingStringList(inputStorage, edgeIDs)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Vias must be defined as a list of edge ids.", outputStorage);
                }
                libsumo::Vehicle::setVia(id, edgeIDs);
            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //readt itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                try {
                    /// XXX but a big try/catch around all retrieval cases
                    libsumo::Vehicle::setParameter(id, name, value);
                } catch (libsumo::TraCIException& e) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, e.what(), outputStorage);
                }
            }
            break;
            case libsumo::VAR_HIGHLIGHT: {
                // Highlight the vehicle by adding a tracking polygon. (NOTE: duplicated code exists for POI domain)
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "A compound object is needed for highlighting an object.", outputStorage);
                }
                const int itemNo = inputStorage.readInt();
                if (itemNo > 5) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Highlighting an object needs zero to five parameters.", outputStorage);
                }
                libsumo::TraCIColor col = libsumo::TraCIColor(255, 0, 0);
                if (itemNo > 0) {
                    if (!server.readTypeCheckingColor(inputStorage, col)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The first parameter for highlighting must be the highlight color.", outputStorage);
                    }
                }
                double size = -1;
                if (itemNo > 1) {
                    if (!server.readTypeCheckingDouble(inputStorage, size)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The second parameter for highlighting must be the highlight size.", outputStorage);
                    }
                }
                int alphaMax = -1;
                if (itemNo > 2) {
                    if (!server.readTypeCheckingUnsignedByte(inputStorage, alphaMax)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The third parameter for highlighting must be maximal alpha.", outputStorage);
                    }
                }
                double duration = -1;
                if (itemNo > 3) {
                    if (!server.readTypeCheckingDouble(inputStorage, duration)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fourth parameter for highlighting must be the highlight duration.", outputStorage);
                    }
                }
                int type = 0;
                if (itemNo > 4) {
                    if (!server.readTypeCheckingUnsignedByte(inputStorage, type)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "The fifth parameter for highlighting must be the highlight type id as ubyte.", outputStorage);
                    }
                }
                libsumo::Vehicle::highlight(id, col, size, alphaMax, duration, type);
            }
            break;
            case libsumo::CMD_TAXI_DISPATCH: {
                std::vector<std::string> reservations;
                if (!server.readTypeCheckingStringList(inputStorage, reservations)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "A dispatch command  must be defined as a list of reservation ids.", outputStorage);
                }
                libsumo::Vehicle::dispatchTaxi(id, reservations);
            }
            break;
            case libsumo::VAR_ACTIONSTEPLENGTH: {
                double value = 0;
                if (!server.readTypeCheckingDouble(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Setting action step length requires a double.", outputStorage);
                }
                if (fabs(value) == std::numeric_limits<double>::infinity()) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Invalid action step length.", outputStorage);
                }
                bool resetActionOffset = value >= 0.0;
                libsumo::Vehicle::setActionStepLength(id, fabs(value), resetActionOffset);
            }
            break;
            case libsumo::VAR_UPDATE_BESTLANES: {
                libsumo::Vehicle::updateBestLanes(id);
            }
            break;
            default: {
                try {
                    if (!TraCIServerAPI_VehicleType::setVariable(libsumo::CMD_SET_VEHICLE_VARIABLE, variable, v->getSingularType().getID(), server, inputStorage, outputStorage)) {
                        return false;
                    }
                } catch (ProcessError& e) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, e.what(), outputStorage);
                } catch (libsumo::TraCIException& e) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, e.what(), outputStorage);
                }
            }
            break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}

void
TraCIServerAPI_Vehicle::writeNextStops(TraCIServer& server, const std::string& id, int limit, bool full) {
    std::vector<libsumo::TraCINextStopData> nextStops = libsumo::Vehicle::getStops(id, limit);
    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
    const int cnt = 1 + (int)nextStops.size() * 4;
    server.getWrapperStorage().writeInt(cnt);
    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
    server.getWrapperStorage().writeInt((int)nextStops.size());
    for (std::vector<libsumo::TraCINextStopData>::iterator it = nextStops.begin(); it != nextStops.end(); ++it) {
        int legacyStopFlags = (it->stopFlags << 1) + (it->arrival >= 0 ? 1 : 0);
        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
        server.getWrapperStorage().writeString(it->lane);
        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
        server.getWrapperStorage().writeDouble(it->endPos);
        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
        server.getWrapperStorage().writeString(it->stoppingPlaceID);
        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_INTEGER);
        server.getWrapperStorage().writeInt(full ? it->stopFlags : legacyStopFlags);
        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
        server.getWrapperStorage().writeDouble(it->duration);
        server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
        server.getWrapperStorage().writeDouble(it->until);
        if (full) {
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
            server.getWrapperStorage().writeDouble(it->startPos);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
            server.getWrapperStorage().writeDouble(it->intendedArrival);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
            server.getWrapperStorage().writeDouble(it->arrival);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
            server.getWrapperStorage().writeDouble(it->depart);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
            server.getWrapperStorage().writeString(it->split);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
            server.getWrapperStorage().writeString(it->join);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
            server.getWrapperStorage().writeString(it->actType);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
            server.getWrapperStorage().writeString(it->tripId);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
            server.getWrapperStorage().writeString(it->line);
            server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_DOUBLE);
            server.getWrapperStorage().writeDouble(it->speed);
        }
    }
}

/****************************************************************************/
