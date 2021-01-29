/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCIServerAPI_Polygon.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Christoph Sommer
/// @author  Gregor L\"ammel
/// @date    Sept 2002
///
// APIs for getting/setting polygon values via TraCI
/****************************************************************************/
#include <config.h>

#include <utils/common/StdDefs.h>
#include <microsim/MSNet.h>
#include <utils/shapes/ShapeContainer.h>
#include <libsumo/Polygon.h>
#include <libsumo/Helper.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServerAPI_Polygon.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_Polygon::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                                   tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_POLYGON_VARIABLE, variable, id);
    try {
        if (!libsumo::Polygon::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::VAR_SHAPE:
                    server.writePositionVector(server.getWrapperStorage(), libsumo::Polygon::getShape(id));
                    break;
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_POLYGON_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Polygon::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_POLYGON_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::Polygon::getParameter(id, paramName));
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_POLYGON_VARIABLE, "Get Polygon Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_POLYGON_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_POLYGON_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}

bool
TraCIServerAPI_Polygon::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                                   tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable
    int variable = inputStorage.readUnsignedByte();
    if (variable != libsumo::VAR_TYPE && variable != libsumo::VAR_COLOR && variable != libsumo::VAR_SHAPE && variable != libsumo::VAR_FILL
            && variable != libsumo::VAR_WIDTH && variable != libsumo::VAR_MOVE_TO
            && variable != libsumo::ADD && variable != libsumo::REMOVE && variable != libsumo::VAR_PARAMETER) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE,
                                          "Change Polygon State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }
    // id
    std::string id = inputStorage.readString();
    try {
        // process
        switch (variable) {
            case libsumo::VAR_TYPE: {
                std::string type;
                if (!server.readTypeCheckingString(inputStorage, type)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The type must be given as a string.", outputStorage);
                }
                libsumo::Polygon::setType(id, type);
            }
            break;
            case libsumo::VAR_COLOR: {
                libsumo::TraCIColor col;
                if (!server.readTypeCheckingColor(inputStorage, col)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The color must be given using an according type.", outputStorage);
                }
                libsumo::Polygon::setColor(id, col);
            }
            break;
            case libsumo::VAR_SHAPE: {
                PositionVector shape;
                if (!server.readTypeCheckingPolygon(inputStorage, shape)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The shape must be given using an according type.", outputStorage);
                }
                libsumo::Polygon::setShape(id, libsumo::Helper::makeTraCIPositionVector(shape));
            }
            break;
            case libsumo::VAR_FILL: {
                int value = 0;
                if (!server.readTypeCheckingInt(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "'fill' must be defined using an integer.", outputStorage);
                }
                libsumo::Polygon::setFilled(id, value != 0);
            }
            break;
            case libsumo::VAR_WIDTH: {
                double value = 0;
                if (!server.readTypeCheckingDouble(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "'lineWidth' must be defined using an double.", outputStorage);
                }
                libsumo::Polygon::setLineWidth(id, value);
            }
            break;
            case libsumo::ADD: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "A compound object is needed for setting a new polygon.", outputStorage);
                }
                int itemNo = inputStorage.readInt();
                if (itemNo != 5 && itemNo != 6) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Adding a polygon needs five to six parameters.", outputStorage);
                }
                std::string type;
                if (!server.readTypeCheckingString(inputStorage, type)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The type must be given as a string.", outputStorage);
                }
                libsumo::TraCIColor col;
                if (!server.readTypeCheckingColor(inputStorage, col)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The second polygon parameter must be the color.", outputStorage);
                }
                int value = 0;
                if (!server.readTypeCheckingUnsignedByte(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The third polygon parameter must be 'fill' encoded as ubyte.", outputStorage);
                }
                bool fill = value != 0;
                int layer = 0;
                if (!server.readTypeCheckingInt(inputStorage, layer)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The fourth polygon parameter must be the layer encoded as int.", outputStorage);
                }
                PositionVector shape;
                if (!server.readTypeCheckingPolygon(inputStorage, shape)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The fifth polygon parameter must be the shape.", outputStorage);
                }
                double lineWidth = 1;
                if (itemNo == 6) {
                    if (!server.readTypeCheckingDouble(inputStorage, lineWidth)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The sixth polygon parameter must be the lineWidth encoded as double.", outputStorage);
                    }
                }
                libsumo::TraCIPositionVector tp = libsumo::Helper::makeTraCIPositionVector(shape);
                libsumo::Polygon::add(id, tp, col, fill, type, layer, lineWidth);
            }
            break;
            case libsumo::VAR_ADD_DYNAMICS : {
                // Add dynamics to polygon.
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "A compound object is needed for adding dynamics to a polygon.", outputStorage);
                }
                int itemNo = inputStorage.readInt();
                if (itemNo != 5) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_VEHICLE_VARIABLE, "Adding polygon dynamics needs four parameters.", outputStorage);
                }

                std::string trackedID;
                if (!server.readTypeCheckingString(inputStorage, trackedID)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The first parameter for adding polygon dynamics must be ID of the tracked object as a string ('' to disregard tracking).", outputStorage);
                }

                std::vector<double> timeSpan;
                if (!server.readTypeCheckingDoubleList(inputStorage, timeSpan)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The second parameter for adding polygon dynamics must be the timespan of the animation (length=0 to disregard animation).", outputStorage);
                }

                std::vector<double> alphaSpan;
                if (!server.readTypeCheckingDoubleList(inputStorage, alphaSpan)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The third parameter for adding polygon dynamics must be the alphaSpanStr of the animation (length=0 to disregard alpha animation).", outputStorage);
                }

                int looped;
                if (!server.readTypeCheckingUnsignedByte(inputStorage, looped)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The fourth parameter for adding polygon dynamics must be boolean indicating whether the animation should be looped.", outputStorage);
                }

                int rotate;
                if (!server.readTypeCheckingUnsignedByte(inputStorage, rotate)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The fifth parameter for adding polygon dynamics must be boolean indicating whether the tracking polygon should be rotated.", outputStorage);
                }

                libsumo::Polygon::addDynamics(id, trackedID, timeSpan, alphaSpan, looped != 0, rotate != 0);
            }
            break;
            case libsumo::REMOVE: {
                int layer = 0; // !!! layer not used yet (shouldn't the id be enough?)
                if (!server.readTypeCheckingInt(inputStorage, layer)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The layer must be given using an int.", outputStorage);
                }

                libsumo::Polygon::remove(id, layer);

            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //readt itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                libsumo::Polygon::setParameter(id, name, value);

            }
            break;
            default:
                break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}


/****************************************************************************/
