/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCIServerAPI_POI.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Robert Hilbrich
/// @date    07.05.2009
///
// APIs for getting/setting POI values via TraCI
/****************************************************************************/
#include <config.h>

#include <microsim/MSNet.h>
#include <utils/shapes/PointOfInterest.h>
#include <utils/shapes/ShapeContainer.h>
#include <libsumo/POI.h>
#include <libsumo/TraCIConstants.h>
#include "TraCIServerAPI_POI.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
TraCIServerAPI_POI::processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                               tcpip::Storage& outputStorage) {
    const int variable = inputStorage.readUnsignedByte();
    const std::string id = inputStorage.readString();
    server.initWrapper(libsumo::RESPONSE_GET_POI_VARIABLE, variable, id);
    try {
        if (!libsumo::POI::handleVariable(id, variable, &server)) {
            switch (variable) {
                case libsumo::VAR_PARAMETER: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_POI_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::POI::getParameter(id, paramName));
                    break;
                }
                case libsumo::VAR_PARAMETER_WITH_KEY: {
                    std::string paramName = "";
                    if (!server.readTypeCheckingString(inputStorage, paramName)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_GET_POI_VARIABLE, "Retrieval of a parameter requires its name.", outputStorage);
                    }
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_COMPOUND);
                    server.getWrapperStorage().writeInt(2);  /// length
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(paramName);
                    server.getWrapperStorage().writeUnsignedByte(libsumo::TYPE_STRING);
                    server.getWrapperStorage().writeString(libsumo::POI::getParameter(id, paramName));
                    break;
                }
                default:
                    return server.writeErrorStatusCmd(libsumo::CMD_GET_POI_VARIABLE, "Get PoI Variable: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
            }
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_GET_POI_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_GET_POI_VARIABLE, libsumo::RTYPE_OK, "", outputStorage);
    server.writeResponseWithLength(outputStorage, server.getWrapperStorage());
    return true;
}


bool
TraCIServerAPI_POI::processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                               tcpip::Storage& outputStorage) {
    std::string warning = ""; // additional description for response
    // variable & id
    int variable = inputStorage.readUnsignedByte();
    std::string id = inputStorage.readString();
    // check variable
    if (variable != libsumo::VAR_TYPE &&
            variable != libsumo::VAR_COLOR &&
            variable != libsumo::VAR_POSITION &&
            variable != libsumo::VAR_WIDTH &&
            variable != libsumo::VAR_HEIGHT &&
            variable != libsumo::VAR_ANGLE &&
            variable != libsumo::VAR_IMAGEFILE &&
            variable != libsumo::VAR_HIGHLIGHT &&
            variable != libsumo::ADD &&
            variable != libsumo::REMOVE &&
            variable != libsumo::VAR_PARAMETER) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "Change PoI State: unsupported variable " + toHex(variable, 2) + " specified", outputStorage);
    }
    // process
    try {
        switch (variable) {
            case libsumo::VAR_TYPE: {
                std::string type;
                if (!server.readTypeCheckingString(inputStorage, type)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The type must be given as a string.", outputStorage);
                }
                libsumo::POI::setType(id, type);
            }
            break;
            case libsumo::VAR_COLOR: {
                libsumo::TraCIColor col;
                if (!server.readTypeCheckingColor(inputStorage, col)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The color must be given using an according type.", outputStorage);
                }
                libsumo::POI::setColor(id, col);
            }
            break;
            case libsumo::VAR_POSITION: {
                libsumo::TraCIPosition pos;
                if (!server.readTypeCheckingPosition2D(inputStorage, pos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The position must be given using an according type.", outputStorage);
                }
                libsumo::POI::setPosition(id, pos.x, pos.y);
            }
            break;
            case libsumo::VAR_WIDTH: {
                double width;
                if (!server.readTypeCheckingDouble(inputStorage, width)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The width must be given using an according type.", outputStorage);
                }
                libsumo::POI::setWidth(id, width);
            }
            break;
            case libsumo::VAR_HEIGHT: {
                double height;
                if (!server.readTypeCheckingDouble(inputStorage, height)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The height must be given using an according type.", outputStorage);
                }
                libsumo::POI::setHeight(id, height);
            }
            break;
            case libsumo::VAR_ANGLE: {
                double angle;
                if (!server.readTypeCheckingDouble(inputStorage, angle)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The angle must be given using an according type.", outputStorage);
                }
                libsumo::POI::setAngle(id, angle);
            }
            break;
            case libsumo::VAR_IMAGEFILE: {
                std::string imageFile;
                if (!server.readTypeCheckingString(inputStorage, imageFile)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The type must be given as a string.", outputStorage);
                }
                libsumo::POI::setImageFile(id, imageFile);
            }
            break;
            case libsumo::VAR_HIGHLIGHT: {
                // Highlight the POI by adding a polygon (NOTE: duplicated code exists for vehicle domain)
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "A compound object is needed for highlighting an object.", outputStorage);
                }
                const int itemNo = inputStorage.readInt();
                if (itemNo > 5) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "Highlighting an object needs zero to five parameters.", outputStorage);
                }
                libsumo::TraCIColor col = libsumo::TraCIColor(255, 0, 0);
                if (itemNo > 0) {
                    if (!server.readTypeCheckingColor(inputStorage, col)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The first parameter for highlighting must be the highlight color.", outputStorage);
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
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The third parameter for highlighting must be maximal alpha.", outputStorage);
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
                libsumo::POI::highlight(id, col, size, alphaMax, duration, type);
            }
            break;
            case libsumo::ADD: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "A compound object is needed for setting a new PoI.", outputStorage);
                }
                //read itemNo
                const int parameterCount = inputStorage.readInt();
                std::string type;
                if (!server.readTypeCheckingString(inputStorage, type)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The first PoI parameter must be the type encoded as a string.", outputStorage);
                }
                libsumo::TraCIColor col;
                if (!server.readTypeCheckingColor(inputStorage, col)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The second PoI parameter must be the color.", outputStorage);
                }
                int layer = 0;
                if (!server.readTypeCheckingInt(inputStorage, layer)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The third PoI parameter must be the layer encoded as int.", outputStorage);
                }
                libsumo::TraCIPosition pos;
                if (!server.readTypeCheckingPosition2D(inputStorage, pos)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The fourth PoI parameter must be the position.", outputStorage);
                }
                if (parameterCount == 4) {
                    if (!libsumo::POI::add(id, pos.x, pos.y, col, type, layer)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "Could not add PoI.", outputStorage);
                    }
                } else if (parameterCount == 8) {
                    std::string imgFile;
                    if (!server.readTypeCheckingString(inputStorage, imgFile)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The fifth PoI parameter must be the imgFile encoded as a string.", outputStorage);
                    }
                    double width;
                    if (!server.readTypeCheckingDouble(inputStorage, width)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The sixth PoI parameter must be the width encoded as a double.", outputStorage);
                    }
                    double height;
                    if (!server.readTypeCheckingDouble(inputStorage, height)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The seventh PoI parameter must be the height encoded as a double.", outputStorage);
                    }
                    double angle;
                    if (!server.readTypeCheckingDouble(inputStorage, angle)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The eighth PoI parameter must be the angle encoded as a double.", outputStorage);
                    }
                    //
                    if (!libsumo::POI::add(id, pos.x, pos.y, col, type, layer, imgFile, width, height, angle)) {
                        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "Could not add PoI.", outputStorage);
                    }
                } else {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE,
                                                      "Adding a PoI requires either only type, color, layer and position parameters or these and imageFile, width, height and angle parameters.",
                                                      outputStorage);
                }
            }
            break;
            case libsumo::REMOVE: {
                int layer = 0; // !!! layer not used yet (shouldn't the id be enough?)
                if (!server.readTypeCheckingInt(inputStorage, layer)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The layer must be given using an int.", outputStorage);
                }
                if (!libsumo::POI::remove(id, layer)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "Could not remove PoI '" + id + "'", outputStorage);
                }
            }
            break;
            case libsumo::VAR_PARAMETER: {
                if (inputStorage.readUnsignedByte() != libsumo::TYPE_COMPOUND) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "A compound object is needed for setting a parameter.", outputStorage);
                }
                //readt itemNo
                inputStorage.readInt();
                std::string name;
                if (!server.readTypeCheckingString(inputStorage, name)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The name of the parameter must be given as a string.", outputStorage);
                }
                std::string value;
                if (!server.readTypeCheckingString(inputStorage, value)) {
                    return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, "The value of the parameter must be given as a string.", outputStorage);
                }
                libsumo::POI::setParameter(id, name, value);
            }
            break;
            default:
                break;
        }
    } catch (libsumo::TraCIException& e) {
        return server.writeErrorStatusCmd(libsumo::CMD_SET_POI_VARIABLE, e.what(), outputStorage);
    }
    server.writeStatusCmd(libsumo::CMD_SET_POI_VARIABLE, libsumo::RTYPE_OK, warning, outputStorage);
    return true;
}


/****************************************************************************/
