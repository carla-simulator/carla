/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCIAPI.cpp
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    30.05.2012
///
// C++ TraCI client API implementation
/****************************************************************************/
#include "TraCIAPI.h"


// ===========================================================================
// member definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// TraCIAPI-methods
// ---------------------------------------------------------------------------
#ifdef _MSC_VER
/* Disable "decorated name length exceeded, name was truncated" warnings for the whole file. */
#pragma warning(disable: 4503)
#pragma warning(push)
/* Disable warning about using "this" in the constructor */
#pragma warning(disable: 4355)
#endif
TraCIAPI::TraCIAPI()
    : edge(*this), gui(*this), inductionloop(*this),
      junction(*this), lane(*this), lanearea(*this), multientryexit(*this),
      person(*this), poi(*this), polygon(*this),
      rerouter(*this), route(*this), routeprobe(*this),
      simulation(*this), trafficlights(*this),
      vehicle(*this), vehicletype(*this),
      mySocket(nullptr) {
    myDomains[libsumo::RESPONSE_SUBSCRIBE_EDGE_VARIABLE] = &edge;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_GUI_VARIABLE] = &gui;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE] = &junction;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_LANE_VARIABLE] = &lane;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_LANEAREA_VARIABLE] = &lanearea;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE] = &multientryexit;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_PERSON_VARIABLE] = &person;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_POI_VARIABLE] = &poi;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_POLYGON_VARIABLE] = &polygon;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_REROUTER_VARIABLE] = &rerouter;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_ROUTE_VARIABLE] = &route;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_ROUTEPROBE_VARIABLE] = &routeprobe;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_SIM_VARIABLE] = &simulation;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_TL_VARIABLE] = &trafficlights;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE] = &vehicle;
    myDomains[libsumo::RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE] = &vehicletype;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif


TraCIAPI::~TraCIAPI() {
    delete mySocket;
}


void
TraCIAPI::connect(const std::string& host, int port) {
    mySocket = new tcpip::Socket(host, port);
    try {
        mySocket->connect();
    } catch (tcpip::SocketException&) {
        delete mySocket;
        mySocket = nullptr;
        throw;
    }
}


void
TraCIAPI::setOrder(int order) {
    tcpip::Storage outMsg;
    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4);
    // command id
    outMsg.writeUnsignedByte(libsumo::CMD_SETORDER);
    outMsg.writeInt(order);
    // send request message
    mySocket->sendExact(outMsg);
    tcpip::Storage inMsg;
    check_resultState(inMsg, libsumo::CMD_SETORDER);
}


void
TraCIAPI::close() {
    send_commandClose();
    tcpip::Storage inMsg;
    std::string acknowledgement;
    check_resultState(inMsg, libsumo::CMD_CLOSE, false, &acknowledgement);
    closeSocket();
}


void
TraCIAPI::closeSocket() {
    if (mySocket == nullptr) {
        return;
    }
    mySocket->close();
    delete mySocket;
    mySocket = nullptr;
}


void
TraCIAPI::send_commandSimulationStep(double time) const {
    tcpip::Storage outMsg;
    // command length
    outMsg.writeUnsignedByte(1 + 1 + 8);
    // command id
    outMsg.writeUnsignedByte(libsumo::CMD_SIMSTEP);
    outMsg.writeDouble(time);
    // send request message
    mySocket->sendExact(outMsg);
}


void
TraCIAPI::send_commandClose() const {
    tcpip::Storage outMsg;
    // command length
    outMsg.writeUnsignedByte(1 + 1);
    // command id
    outMsg.writeUnsignedByte(libsumo::CMD_CLOSE);
    mySocket->sendExact(outMsg);
}


void
TraCIAPI::send_commandSetOrder(int order) const {
    tcpip::Storage outMsg;
    // command length
    outMsg.writeUnsignedByte(1 + 1 + 4);
    // command id
    outMsg.writeUnsignedByte(libsumo::CMD_SETORDER);
    // client index
    outMsg.writeInt(order);
    mySocket->sendExact(outMsg);
}


void
TraCIAPI::createCommand(int cmdID, int varID, const std::string& objID, tcpip::Storage* add) const {
    myOutput.reset();
    // command length
    int length = 1 + 1 + 1 + 4 + (int) objID.length();
    if (add != nullptr) {
        length += (int)add->size();
    }
    if (length <= 255) {
        myOutput.writeUnsignedByte(length);
    } else {
        myOutput.writeUnsignedByte(0);
        myOutput.writeInt(length + 4);
    }
    myOutput.writeUnsignedByte(cmdID);
    myOutput.writeUnsignedByte(varID);
    myOutput.writeString(objID);
    // additional values
    if (add != nullptr) {
        myOutput.writeStorage(*add);
    }
}


void
TraCIAPI::createFilterCommand(int cmdID, int varID, tcpip::Storage* add) const {
    myOutput.reset();
    // command length
    int length = 1 + 1 + 1;
    if (add != nullptr) {
        length += (int)add->size();
    }
    if (length <= 255) {
        myOutput.writeUnsignedByte(length);
    } else {
        myOutput.writeUnsignedByte(0);
        myOutput.writeInt(length + 4);
    }
    myOutput.writeUnsignedByte(cmdID);
    myOutput.writeUnsignedByte(varID);
    // additional values
    if (add != nullptr) {
        myOutput.writeStorage(*add);
    }
}


void
TraCIAPI::send_commandSubscribeObjectVariable(int domID, const std::string& objID, double beginTime, double endTime,
        const std::vector<int>& vars) const {
    if (mySocket == nullptr) {
        throw tcpip::SocketException("Socket is not initialised");
    }
    tcpip::Storage outMsg;
    // command length (domID, objID, beginTime, endTime, length, vars)
    int varNo = (int) vars.size();
    outMsg.writeUnsignedByte(0);
    outMsg.writeInt(5 + 1 + 8 + 8 + 4 + (int) objID.length() + 1 + varNo);
    // command id
    outMsg.writeUnsignedByte(domID);
    // time
    outMsg.writeDouble(beginTime);
    outMsg.writeDouble(endTime);
    // object id
    outMsg.writeString(objID);
    // command id
    outMsg.writeUnsignedByte((int)vars.size());
    for (int i = 0; i < varNo; ++i) {
        outMsg.writeUnsignedByte(vars[i]);
    }
    // send message
    mySocket->sendExact(outMsg);
}


void
TraCIAPI::send_commandSubscribeObjectContext(int domID, const std::string& objID, double beginTime, double endTime,
        int domain, double range, const std::vector<int>& vars) const {
    if (mySocket == nullptr) {
        throw tcpip::SocketException("Socket is not initialised");
    }
    tcpip::Storage outMsg;
    // command length (domID, objID, beginTime, endTime, length, vars)
    int varNo = (int) vars.size();
    outMsg.writeUnsignedByte(0);
    outMsg.writeInt(5 + 1 + 8 + 8 + 4 + (int) objID.length() + 1 + 8 + 1 + varNo);
    // command id
    outMsg.writeUnsignedByte(domID);
    // time
    outMsg.writeDouble(beginTime);
    outMsg.writeDouble(endTime);
    // object id
    outMsg.writeString(objID);
    // domain and range
    outMsg.writeUnsignedByte(domain);
    outMsg.writeDouble(range);
    // command id
    outMsg.writeUnsignedByte((int)vars.size());
    for (int i = 0; i < varNo; ++i) {
        outMsg.writeUnsignedByte(vars[i]);
    }
    // send message
    mySocket->sendExact(outMsg);
}


void
TraCIAPI::check_resultState(tcpip::Storage& inMsg, int command, bool ignoreCommandId, std::string* acknowledgement) const {
    mySocket->receiveExact(inMsg);
    int cmdLength;
    int cmdId;
    int resultType;
    int cmdStart;
    std::string msg;
    try {
        cmdStart = inMsg.position();
        cmdLength = inMsg.readUnsignedByte();
        cmdId = inMsg.readUnsignedByte();
        if (command != cmdId && !ignoreCommandId) {
            throw libsumo::TraCIException("#Error: received status response to command: " + toString(cmdId) + " but expected: " + toString(command));
        }
        resultType = inMsg.readUnsignedByte();
        msg = inMsg.readString();
    } catch (std::invalid_argument&) {
        throw libsumo::TraCIException("#Error: an exception was thrown while reading result state message");
    }
    switch (resultType) {
        case libsumo::RTYPE_ERR:
            throw libsumo::TraCIException(".. Answered with error to command (" + toString(command) + "), [description: " + msg + "]");
        case libsumo::RTYPE_NOTIMPLEMENTED:
            throw libsumo::TraCIException(".. Sent command is not implemented (" + toString(command) + "), [description: " + msg + "]");
        case libsumo::RTYPE_OK:
            if (acknowledgement != nullptr) {
                (*acknowledgement) = ".. Command acknowledged (" + toString(command) + "), [description: " + msg + "]";
            }
            break;
        default:
            throw libsumo::TraCIException(".. Answered with unknown result code(" + toString(resultType) + ") to command(" + toString(command) + "), [description: " + msg + "]");
    }
    if ((cmdStart + cmdLength) != (int) inMsg.position()) {
        throw libsumo::TraCIException("#Error: command at position " + toString(cmdStart) + " has wrong length");
    }
}


int
TraCIAPI::check_commandGetResult(tcpip::Storage& inMsg, int command, int expectedType, bool ignoreCommandId) const {
    inMsg.position(); // respStart
    int length = inMsg.readUnsignedByte();
    if (length == 0) {
        length = inMsg.readInt();
    }
    int cmdId = inMsg.readUnsignedByte();
    if (!ignoreCommandId && cmdId != (command + 0x10)) {
        throw libsumo::TraCIException("#Error: received response with command id: " + toString(cmdId) + "but expected: " + toString(command + 0x10));
    }
    if (expectedType >= 0) {
        // not called from the TraCITestClient but from within the TraCIAPI
        inMsg.readUnsignedByte(); // variableID
        inMsg.readString(); // objectID
        int valueDataType = inMsg.readUnsignedByte();
        if (valueDataType != expectedType) {
            throw libsumo::TraCIException("Expected " + toString(expectedType) + " but got " + toString(valueDataType));
        }
    }
    return cmdId;
}


bool
TraCIAPI::processGet(int command, int expectedType, bool ignoreCommandId) {
    if (mySocket != nullptr) {
        mySocket->sendExact(myOutput);
        myInput.reset();
        check_resultState(myInput, command, ignoreCommandId);
        check_commandGetResult(myInput, command, expectedType, ignoreCommandId);
        return true;
    }
    return false;
}


bool
TraCIAPI::processSet(int command) {
    if (mySocket != nullptr) {
        mySocket->sendExact(myOutput);
        myInput.reset();
        check_resultState(myInput, command);
        return true;
    }
    return false;
}


void
TraCIAPI::readVariables(tcpip::Storage& inMsg, const std::string& objectID, int variableCount, libsumo::SubscriptionResults& into) {
    while (variableCount > 0) {

        const int variableID = inMsg.readUnsignedByte();
        const int status = inMsg.readUnsignedByte();
        const int type = inMsg.readUnsignedByte();

        if (status == libsumo::RTYPE_OK) {
            switch (type) {
                case libsumo::TYPE_DOUBLE:
                    into[objectID][variableID] = std::make_shared<libsumo::TraCIDouble>(inMsg.readDouble());
                    break;
                case libsumo::TYPE_STRING:
                    into[objectID][variableID] = std::make_shared<libsumo::TraCIString>(inMsg.readString());
                    break;
                case libsumo::POSITION_2D: {
                    auto p = std::make_shared<libsumo::TraCIPosition>();
                    p->x = inMsg.readDouble();
                    p->y = inMsg.readDouble();
                    p->z = 0.;
                    into[objectID][variableID] = p;
                    break;
                }
                case libsumo::POSITION_3D: {
                    auto p = std::make_shared<libsumo::TraCIPosition>();
                    p->x = inMsg.readDouble();
                    p->y = inMsg.readDouble();
                    p->z = inMsg.readDouble();
                    into[objectID][variableID] = p;
                    break;
                }
                case libsumo::TYPE_COLOR: {
                    auto c = std::make_shared<libsumo::TraCIColor>();
                    c->r = (unsigned char)inMsg.readUnsignedByte();
                    c->g = (unsigned char)inMsg.readUnsignedByte();
                    c->b = (unsigned char)inMsg.readUnsignedByte();
                    c->a = (unsigned char)inMsg.readUnsignedByte();
                    into[objectID][variableID] = c;
                    break;
                }
                case libsumo::TYPE_INTEGER:
                    into[objectID][variableID] = std::make_shared<libsumo::TraCIInt>(inMsg.readInt());
                    break;
                case libsumo::TYPE_STRINGLIST: {
                    auto sl = std::make_shared<libsumo::TraCIStringList>();
                    int n = inMsg.readInt();
                    for (int i = 0; i < n; ++i) {
                        sl->value.push_back(inMsg.readString());
                    }
                    into[objectID][variableID] = sl;
                }
                break;

                // TODO Other data types

                default:
                    throw libsumo::TraCIException("Unimplemented subscription type: " + toString(type));
            }
        } else {
            throw libsumo::TraCIException("Subscription response error: variableID=" + toString(variableID) + " status=" + toString(status));
        }

        variableCount--;
    }
}


void
TraCIAPI::readVariableSubscription(int cmdId, tcpip::Storage& inMsg) {
    const std::string objectID = inMsg.readString();
    const int variableCount = inMsg.readUnsignedByte();
    readVariables(inMsg, objectID, variableCount, myDomains[cmdId]->getModifiableSubscriptionResults());
}


void
TraCIAPI::readContextSubscription(int cmdId, tcpip::Storage& inMsg) {
    const std::string contextID = inMsg.readString();
    inMsg.readUnsignedByte(); // context domain
    const int variableCount = inMsg.readUnsignedByte();
    int numObjects = inMsg.readInt();

    while (numObjects > 0) {
        std::string objectID = inMsg.readString();
        readVariables(inMsg, objectID, variableCount, myDomains[cmdId]->getModifiableContextSubscriptionResults(contextID));
        numObjects--;
    }
}


void
TraCIAPI::simulationStep(double time) {
    send_commandSimulationStep(time);
    tcpip::Storage inMsg;
    check_resultState(inMsg, libsumo::CMD_SIMSTEP);

    for (auto it : myDomains) {
        it.second->clearSubscriptionResults();
    }
    int numSubs = inMsg.readInt();
    while (numSubs > 0) {
        int cmdId = check_commandGetResult(inMsg, 0, -1, true);
        if (cmdId >= libsumo::RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE && cmdId <= libsumo::RESPONSE_SUBSCRIBE_PERSON_VARIABLE) {
            readVariableSubscription(cmdId, inMsg);
        } else {
            readContextSubscription(cmdId + 0x50, inMsg);
        }
        numSubs--;
    }
}


void
TraCIAPI::load(const std::vector<std::string>& args) {
    int numChars = 0;
    for (int i = 0; i < (int)args.size(); ++i) {
        numChars += (int)args[i].size();
    }
    tcpip::Storage content;
    content.writeUnsignedByte(0);
    content.writeInt(1 + 4 + 1 + 1 + 4 + numChars + 4 * (int)args.size());
    content.writeUnsignedByte(libsumo::CMD_LOAD);
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeStringList(args);
    mySocket->sendExact(content);
    tcpip::Storage inMsg;
    check_resultState(inMsg, libsumo::CMD_LOAD);
}


std::pair<int, std::string>
TraCIAPI::getVersion() {
    tcpip::Storage content;
    content.writeUnsignedByte(2);
    content.writeUnsignedByte(libsumo::CMD_GETVERSION);
    mySocket->sendExact(content);
    tcpip::Storage inMsg;
    check_resultState(inMsg, libsumo::CMD_GETVERSION);
    inMsg.readUnsignedByte(); // msg length
    inMsg.readUnsignedByte(); // libsumo::CMD_GETVERSION again, see #7284
    const int traciVersion = inMsg.readInt(); // to fix evaluation order
    return std::make_pair(traciVersion, inMsg.readString());
}


// ---------------------------------------------------------------------------
// TraCIAPI::EdgeScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::EdgeScope::getAdaptedTraveltime(const std::string& edgeID, double time) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(time);
    return getDouble(libsumo::VAR_EDGE_TRAVELTIME, edgeID, &content);
}

double
TraCIAPI::EdgeScope::getEffort(const std::string& edgeID, double time) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(time);
    return getDouble(libsumo::VAR_EDGE_EFFORT, edgeID, &content);
}

double
TraCIAPI::EdgeScope::getCO2Emission(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_CO2EMISSION, edgeID);
}


double
TraCIAPI::EdgeScope::getCOEmission(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_COEMISSION, edgeID);
}

double
TraCIAPI::EdgeScope::getHCEmission(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_HCEMISSION, edgeID);
}

double
TraCIAPI::EdgeScope::getPMxEmission(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_PMXEMISSION, edgeID);
}

double
TraCIAPI::EdgeScope::getNOxEmission(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_NOXEMISSION, edgeID);
}

double
TraCIAPI::EdgeScope::getFuelConsumption(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_FUELCONSUMPTION, edgeID);
}

double
TraCIAPI::EdgeScope::getNoiseEmission(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_NOISEEMISSION, edgeID);
}

double
TraCIAPI::EdgeScope::getElectricityConsumption(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_ELECTRICITYCONSUMPTION, edgeID);
}

double
TraCIAPI::EdgeScope::getLastStepMeanSpeed(const std::string& edgeID) const {
    return getDouble(libsumo::LAST_STEP_MEAN_SPEED, edgeID);
}

double
TraCIAPI::EdgeScope::getLastStepOccupancy(const std::string& edgeID) const {
    return getDouble(libsumo::LAST_STEP_OCCUPANCY, edgeID);
}

double
TraCIAPI::EdgeScope::getLastStepLength(const std::string& edgeID) const {
    return getDouble(libsumo::LAST_STEP_LENGTH, edgeID);
}

double
TraCIAPI::EdgeScope::getTraveltime(const std::string& edgeID) const {
    return getDouble(libsumo::VAR_CURRENT_TRAVELTIME, edgeID);
}

int
TraCIAPI::EdgeScope::getLastStepVehicleNumber(const std::string& edgeID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_NUMBER, edgeID);
}

double
TraCIAPI::EdgeScope::getLastStepHaltingNumber(const std::string& edgeID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_HALTING_NUMBER, edgeID);
}

std::vector<std::string>
TraCIAPI::EdgeScope::getLastStepVehicleIDs(const std::string& edgeID) const {
    return getStringVector(libsumo::LAST_STEP_VEHICLE_ID_LIST, edgeID);
}


int
TraCIAPI::EdgeScope::getLaneNumber(const std::string& edgeID) const {
    return getInt(libsumo::VAR_LANE_INDEX, edgeID);
}


std::string
TraCIAPI::EdgeScope::getStreetName(const std::string& edgeID) const {
    return getString(libsumo::VAR_NAME, edgeID);
}


void
TraCIAPI::EdgeScope::adaptTraveltime(const std::string& edgeID, double time, double beginSeconds, double endSeconds) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    if (endSeconds != std::numeric_limits<double>::max()) {
        content.writeInt(3);
        content.writeByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(beginSeconds);
        content.writeByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(endSeconds);
    } else {
        content.writeInt(1);
    }
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(time);
    myParent.createCommand(libsumo::CMD_SET_EDGE_VARIABLE, libsumo::VAR_EDGE_TRAVELTIME, edgeID, &content);
    myParent.processSet(libsumo::CMD_SET_EDGE_VARIABLE);
}


void
TraCIAPI::EdgeScope::setEffort(const std::string& edgeID, double effort, double beginSeconds, double endSeconds) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    if (endSeconds != std::numeric_limits<double>::max()) {
        content.writeInt(3);
        content.writeByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(beginSeconds);
        content.writeByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(endSeconds);
    } else {
        content.writeInt(1);
    }
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(effort);
    myParent.createCommand(libsumo::CMD_SET_EDGE_VARIABLE, libsumo::VAR_EDGE_EFFORT, edgeID, &content);
    myParent.processSet(libsumo::CMD_SET_EDGE_VARIABLE);
}

void
TraCIAPI::EdgeScope::setMaxSpeed(const std::string& edgeID, double speed) const {
    setDouble(libsumo::VAR_MAXSPEED, edgeID, speed);
}


// ---------------------------------------------------------------------------
// TraCIAPI::GUIScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::GUIScope::getZoom(const std::string& viewID) const {
    return getDouble(libsumo::VAR_VIEW_ZOOM, viewID);
}

libsumo::TraCIPosition
TraCIAPI::GUIScope::getOffset(const std::string& viewID) const {
    return getPos(libsumo::VAR_VIEW_OFFSET, viewID);
}

std::string
TraCIAPI::GUIScope::getSchema(const std::string& viewID) const {
    return getString(libsumo::VAR_VIEW_SCHEMA, viewID);
}

libsumo::TraCIPositionVector
TraCIAPI::GUIScope::getBoundary(const std::string& viewID) const {
    return getPolygon(libsumo::VAR_VIEW_BOUNDARY, viewID);
}


void
TraCIAPI::GUIScope::setZoom(const std::string& viewID, double zoom) const {
    setDouble(libsumo::VAR_VIEW_ZOOM, viewID, zoom);
}

void
TraCIAPI::GUIScope::setOffset(const std::string& viewID, double x, double y) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::POSITION_2D);
    content.writeDouble(x);
    content.writeDouble(y);
    myParent.createCommand(libsumo::CMD_SET_GUI_VARIABLE, libsumo::VAR_VIEW_OFFSET, viewID, &content);
    myParent.processSet(libsumo::CMD_SET_GUI_VARIABLE);
}

void
TraCIAPI::GUIScope::setSchema(const std::string& viewID, const std::string& schemeName) const {
    setString(libsumo::VAR_VIEW_SCHEMA, viewID, schemeName);
}

void
TraCIAPI::GUIScope::setBoundary(const std::string& viewID, double xmin, double ymin, double xmax, double ymax) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_POLYGON);
    content.writeByte(2);
    content.writeDouble(xmin);
    content.writeDouble(ymin);
    content.writeDouble(xmax);
    content.writeDouble(ymax);
    myParent.createCommand(libsumo::CMD_SET_GUI_VARIABLE, libsumo::VAR_VIEW_BOUNDARY, viewID, &content);
    myParent.processSet(libsumo::CMD_SET_GUI_VARIABLE);
}

void
TraCIAPI::GUIScope::screenshot(const std::string& viewID, const std::string& filename, const int width, const int height) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(3);
    content.writeByte(libsumo::TYPE_STRING);
    content.writeString(filename);
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(width);
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(height);
    myParent.createCommand(libsumo::CMD_SET_GUI_VARIABLE, libsumo::VAR_SCREENSHOT, viewID, &content);
    myParent.processSet(libsumo::CMD_SET_GUI_VARIABLE);
}

void
TraCIAPI::GUIScope::trackVehicle(const std::string& viewID, const std::string& vehID) const {
    setString(libsumo::VAR_VIEW_SCHEMA, viewID, vehID);
}


// ---------------------------------------------------------------------------
// TraCIAPI::InductionLoopScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::InductionLoopScope::getPosition(const std::string& loopID) const {
    return getDouble(libsumo::VAR_POSITION, loopID);
}

std::string
TraCIAPI::InductionLoopScope::getLaneID(const std::string& loopID) const {
    return getString(libsumo::VAR_LANE_ID, loopID);
}

int
TraCIAPI::InductionLoopScope::getLastStepVehicleNumber(const std::string& loopID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_NUMBER, loopID);
}

double
TraCIAPI::InductionLoopScope::getLastStepMeanSpeed(const std::string& loopID) const {
    return getDouble(libsumo::LAST_STEP_MEAN_SPEED, loopID);
}

std::vector<std::string>
TraCIAPI::InductionLoopScope::getLastStepVehicleIDs(const std::string& loopID) const {
    return getStringVector(libsumo::LAST_STEP_VEHICLE_ID_LIST, loopID);
}

double
TraCIAPI::InductionLoopScope::getLastStepOccupancy(const std::string& loopID) const {
    return getDouble(libsumo::LAST_STEP_OCCUPANCY, loopID);
}

double
TraCIAPI::InductionLoopScope::getLastStepMeanLength(const std::string& loopID) const {
    return getDouble(libsumo::LAST_STEP_LENGTH, loopID);
}

double
TraCIAPI::InductionLoopScope::getTimeSinceDetection(const std::string& loopID) const {
    return getDouble(libsumo::LAST_STEP_TIME_SINCE_DETECTION, loopID);
}


std::vector<libsumo::TraCIVehicleData>
TraCIAPI::InductionLoopScope::getVehicleData(const std::string& loopID) const {
    std::vector<libsumo::TraCIVehicleData> result;
    myParent.createCommand(libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE, libsumo::LAST_STEP_VEHICLE_DATA, loopID);
    if (myParent.processGet(libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt(); // components
        // number of items
        myParent.myInput.readUnsignedByte();
        const int n = myParent.myInput.readInt();
        for (int i = 0; i < n; ++i) {
            libsumo::TraCIVehicleData vd;

            myParent.myInput.readUnsignedByte();
            vd.id = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            vd.length = myParent.myInput.readDouble();

            myParent.myInput.readUnsignedByte();
            vd.entryTime = myParent.myInput.readDouble();

            myParent.myInput.readUnsignedByte();
            vd.leaveTime = myParent.myInput.readDouble();

            myParent.myInput.readUnsignedByte();
            vd.typeID = myParent.myInput.readString();

            result.push_back(vd);
        }
    }
    return result;
}


// ---------------------------------------------------------------------------
// TraCIAPI::JunctionScope-methods
// ---------------------------------------------------------------------------
libsumo::TraCIPosition
TraCIAPI::JunctionScope::getPosition(const std::string& junctionID) const {
    return getPos(libsumo::VAR_POSITION, junctionID);
}

libsumo::TraCIPositionVector
TraCIAPI::JunctionScope::getShape(const std::string& junctionID) const {
    return getPolygon(libsumo::VAR_SHAPE, junctionID);
}


// ---------------------------------------------------------------------------
// TraCIAPI::LaneScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::LaneScope::getLength(const std::string& laneID) const {
    return getDouble(libsumo::VAR_LENGTH, laneID);
}

double
TraCIAPI::LaneScope::getMaxSpeed(const std::string& laneID) const {
    return getDouble(libsumo::VAR_MAXSPEED, laneID);
}

double
TraCIAPI::LaneScope::getWidth(const std::string& laneID) const {
    return getDouble(libsumo::VAR_WIDTH, laneID);
}

std::vector<std::string>
TraCIAPI::LaneScope::getAllowed(const std::string& laneID) const {
    return getStringVector(libsumo::LANE_ALLOWED, laneID);
}

std::vector<std::string>
TraCIAPI::LaneScope::getDisallowed(const std::string& laneID) const {
    return getStringVector(libsumo::LANE_DISALLOWED, laneID);
}

int
TraCIAPI::LaneScope::getLinkNumber(const std::string& laneID) const {
    return getInt(libsumo::LANE_LINK_NUMBER, laneID);
}

std::vector<libsumo::TraCIConnection>
TraCIAPI::LaneScope::getLinks(const std::string& laneID) const {
    std::vector<libsumo::TraCIConnection> ret;
    myParent.createCommand(libsumo::CMD_GET_LANE_VARIABLE, libsumo::LANE_LINKS, laneID);
    if (myParent.processGet(libsumo::CMD_GET_LANE_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readUnsignedByte();
        myParent.myInput.readInt();

        int linkNo = myParent.myInput.readInt();
        for (int i = 0; i < linkNo; ++i) {

            myParent.myInput.readUnsignedByte();
            std::string approachedLane = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            std::string approachedLaneInternal = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            bool hasPrio = myParent.myInput.readUnsignedByte() != 0;

            myParent.myInput.readUnsignedByte();
            bool isOpen = myParent.myInput.readUnsignedByte() != 0;

            myParent.myInput.readUnsignedByte();
            bool hasFoe = myParent.myInput.readUnsignedByte() != 0;

            myParent.myInput.readUnsignedByte();
            std::string state = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            std::string direction = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            double length = myParent.myInput.readDouble();

            ret.push_back(libsumo::TraCIConnection(approachedLane,
                                                   hasPrio,
                                                   isOpen,
                                                   hasFoe,
                                                   approachedLaneInternal,
                                                   state,
                                                   direction,
                                                   length));

        }

    }
    return ret;
}

libsumo::TraCIPositionVector
TraCIAPI::LaneScope::getShape(const std::string& laneID) const {
    return getPolygon(libsumo::VAR_SHAPE, laneID);
}

std::string
TraCIAPI::LaneScope::getEdgeID(const std::string& laneID) const {
    return getString(libsumo::LANE_EDGE_ID, laneID);
}

double
TraCIAPI::LaneScope::getCO2Emission(const std::string& laneID) const {
    return getDouble(libsumo::VAR_CO2EMISSION, laneID);
}

double
TraCIAPI::LaneScope::getCOEmission(const std::string& laneID) const {
    return getDouble(libsumo::VAR_COEMISSION, laneID);
}

double
TraCIAPI::LaneScope::getHCEmission(const std::string& laneID) const {
    return getDouble(libsumo::VAR_HCEMISSION, laneID);
}

double
TraCIAPI::LaneScope::getPMxEmission(const std::string& laneID) const {
    return getDouble(libsumo::VAR_PMXEMISSION, laneID);
}

double
TraCIAPI::LaneScope::getNOxEmission(const std::string& laneID) const {
    return getDouble(libsumo::VAR_NOXEMISSION, laneID);
}

double
TraCIAPI::LaneScope::getFuelConsumption(const std::string& laneID) const {
    return getDouble(libsumo::VAR_FUELCONSUMPTION, laneID);
}

double
TraCIAPI::LaneScope::getNoiseEmission(const std::string& laneID) const {
    return getDouble(libsumo::VAR_NOISEEMISSION, laneID);
}

double
TraCIAPI::LaneScope::getElectricityConsumption(const std::string& laneID) const {
    return getDouble(libsumo::VAR_ELECTRICITYCONSUMPTION, laneID);
}

double
TraCIAPI::LaneScope::getLastStepMeanSpeed(const std::string& laneID) const {
    return getDouble(libsumo::LAST_STEP_MEAN_SPEED, laneID);
}

double
TraCIAPI::LaneScope::getLastStepOccupancy(const std::string& laneID) const {
    return getDouble(libsumo::LAST_STEP_OCCUPANCY, laneID);
}

double
TraCIAPI::LaneScope::getLastStepLength(const std::string& laneID) const {
    return getDouble(libsumo::LAST_STEP_LENGTH, laneID);
}

double
TraCIAPI::LaneScope::getTraveltime(const std::string& laneID) const {
    return getDouble(libsumo::VAR_CURRENT_TRAVELTIME, laneID);
}

int
TraCIAPI::LaneScope::getLastStepVehicleNumber(const std::string& laneID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_NUMBER, laneID);
}

int
TraCIAPI::LaneScope::getLastStepHaltingNumber(const std::string& laneID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_HALTING_NUMBER, laneID);
}

std::vector<std::string>
TraCIAPI::LaneScope::getLastStepVehicleIDs(const std::string& laneID) const {
    return getStringVector(libsumo::LAST_STEP_VEHICLE_ID_LIST, laneID);
}


std::vector<std::string>
TraCIAPI::LaneScope::getFoes(const std::string& laneID, const std::string& toLaneID) const {
    std::vector<std::string> r;
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(toLaneID);
    myParent.createCommand(libsumo::CMD_GET_LANE_VARIABLE, libsumo::VAR_FOES, laneID, &content);
    if (myParent.processGet(libsumo::CMD_GET_LANE_VARIABLE, libsumo::TYPE_STRINGLIST)) {
        const int size = myParent.myInput.readInt();
        for (int i = 0; i < size; ++i) {
            r.push_back(myParent.myInput.readString());
        }
    }
    return r;
}

std::vector<std::string>
TraCIAPI::LaneScope::getInternalFoes(const std::string& laneID) const {
    return getFoes(laneID, "");
}


void
TraCIAPI::LaneScope::setAllowed(const std::string& laneID, const std::vector<std::string>& allowedClasses) const {
    setStringVector(libsumo::LANE_ALLOWED, laneID, allowedClasses);
}

void
TraCIAPI::LaneScope::setDisallowed(const std::string& laneID, const std::vector<std::string>& disallowedClasses) const {
    setStringVector(libsumo::LANE_DISALLOWED, laneID, disallowedClasses);
}

void
TraCIAPI::LaneScope::setMaxSpeed(const std::string& laneID, double speed) const {
    setDouble(libsumo::VAR_MAXSPEED, laneID, speed);
}

void
TraCIAPI::LaneScope::setLength(const std::string& laneID, double length) const {
    setDouble(libsumo::VAR_LENGTH, laneID, length);
}


// ---------------------------------------------------------------------------
// TraCIAPI::LaneAreaDetector-methods
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// TraCIAPI::MeMeScope-methods
// ---------------------------------------------------------------------------
int
TraCIAPI::MeMeScope::getLastStepVehicleNumber(const std::string& detID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_NUMBER, detID);
}

double
TraCIAPI::MeMeScope::getLastStepMeanSpeed(const std::string& detID) const {
    return getInt(libsumo::LAST_STEP_MEAN_SPEED, detID);
}

std::vector<std::string>
TraCIAPI::MeMeScope::getLastStepVehicleIDs(const std::string& detID) const {
    return getStringVector(libsumo::LAST_STEP_VEHICLE_ID_LIST, detID);
}

int
TraCIAPI::MeMeScope::getLastStepHaltingNumber(const std::string& detID) const {
    return getInt(libsumo::LAST_STEP_VEHICLE_HALTING_NUMBER, detID);
}



// ---------------------------------------------------------------------------
// TraCIAPI::POIScope-methods
// ---------------------------------------------------------------------------
std::string
TraCIAPI::POIScope::getType(const std::string& poiID) const {
    return getString(libsumo::VAR_TYPE, poiID);
}

libsumo::TraCIPosition
TraCIAPI::POIScope::getPosition(const std::string& poiID) const {
    return getPos(libsumo::VAR_POSITION, poiID);
}

libsumo::TraCIColor
TraCIAPI::POIScope::getColor(const std::string& poiID) const {
    return getCol(libsumo::VAR_COLOR, poiID);
}

double
TraCIAPI::POIScope::getWidth(const std::string& poiID) const {
    return getDouble(libsumo::VAR_WIDTH, poiID);
}

double
TraCIAPI::POIScope::getHeight(const std::string& poiID) const {
    return getDouble(libsumo::VAR_HEIGHT, poiID);
}

double
TraCIAPI::POIScope::getAngle(const std::string& poiID) const {
    return getDouble(libsumo::VAR_ANGLE, poiID);
}

std::string
TraCIAPI::POIScope::getImageFile(const std::string& poiID) const {
    return getString(libsumo::VAR_IMAGEFILE, poiID);
}


void
TraCIAPI::POIScope::setType(const std::string& poiID, const std::string& setType) const {
    setString(libsumo::VAR_TYPE, poiID, setType);
}


void
TraCIAPI::POIScope::setPosition(const std::string& poiID, double x, double y) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::POSITION_2D);
    content.writeDouble(x);
    content.writeDouble(y);
    myParent.createCommand(libsumo::CMD_SET_POI_VARIABLE, libsumo::VAR_POSITION, poiID, &content);
    myParent.processSet(libsumo::CMD_SET_POI_VARIABLE);
}


void
TraCIAPI::POIScope::setColor(const std::string& poiID, const libsumo::TraCIColor& c) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    myParent.createCommand(libsumo::CMD_SET_POI_VARIABLE, libsumo::VAR_COLOR, poiID, &content);
    myParent.processSet(libsumo::CMD_SET_POI_VARIABLE);
}


void
TraCIAPI::POIScope::setWidth(const std::string& poiID, double width) const {
    setDouble(libsumo::VAR_WIDTH, poiID, width);
}


void
TraCIAPI::POIScope::setHeight(const std::string& poiID, double height) const {
    setDouble(libsumo::VAR_HEIGHT, poiID, height);
}


void
TraCIAPI::POIScope::setAngle(const std::string& poiID, double angle) const {
    setDouble(libsumo::VAR_ANGLE, poiID, angle);
}


void
TraCIAPI::POIScope::setImageFile(const std::string& poiID, const std::string& imageFile) const {
    setString(libsumo::VAR_IMAGEFILE, poiID, imageFile);
}


void
TraCIAPI::POIScope::add(const std::string& poiID, double x, double y, const libsumo::TraCIColor& c, const std::string& type, int layer, const std::string& imgFile, double width, double height, double angle) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(8);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(type);
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(layer);
    content.writeUnsignedByte(libsumo::POSITION_2D);
    content.writeDouble(x);
    content.writeDouble(y);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(imgFile);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(width);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(height);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(angle);
    myParent.createCommand(libsumo::CMD_SET_POI_VARIABLE, libsumo::ADD, poiID, &content);
    myParent.processSet(libsumo::CMD_SET_POI_VARIABLE);
}

void
TraCIAPI::POIScope::remove(const std::string& poiID, int layer) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(layer);
    myParent.createCommand(libsumo::CMD_SET_POI_VARIABLE, libsumo::REMOVE, poiID, &content);
    myParent.processSet(libsumo::CMD_SET_POI_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::PolygonScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::PolygonScope::getLineWidth(const std::string& polygonID) const {
    return getDouble(libsumo::VAR_WIDTH, polygonID);
}

std::string
TraCIAPI::PolygonScope::getType(const std::string& polygonID) const {
    return getString(libsumo::VAR_TYPE, polygonID);
}

libsumo::TraCIPositionVector
TraCIAPI::PolygonScope::getShape(const std::string& polygonID) const {
    return getPolygon(libsumo::VAR_SHAPE, polygonID);
}

libsumo::TraCIColor
TraCIAPI::PolygonScope::getColor(const std::string& polygonID) const {
    return getCol(libsumo::VAR_COLOR, polygonID);
}

void
TraCIAPI::PolygonScope::setLineWidth(const std::string& polygonID, const double lineWidth) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(lineWidth);
    myParent.createCommand(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::VAR_WIDTH, polygonID, &content);
    myParent.processSet(libsumo::CMD_SET_POLYGON_VARIABLE);
}

void
TraCIAPI::PolygonScope::setType(const std::string& polygonID, const std::string& setType) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(setType);
    myParent.createCommand(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::VAR_TYPE, polygonID, &content);
    myParent.processSet(libsumo::CMD_SET_POLYGON_VARIABLE);
}


void
TraCIAPI::PolygonScope::setShape(const std::string& polygonID, const libsumo::TraCIPositionVector& shape) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_POLYGON);
    if (shape.size() < 256) {
        content.writeUnsignedByte((int)shape.size());
    } else {
        content.writeUnsignedByte(0);
        content.writeInt((int)shape.size());
    }
    for (const libsumo::TraCIPosition& pos : shape) {
        content.writeDouble(pos.x);
        content.writeDouble(pos.y);
    }
    myParent.createCommand(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::VAR_SHAPE, polygonID, &content);
    myParent.processSet(libsumo::CMD_SET_POLYGON_VARIABLE);
}


void
TraCIAPI::PolygonScope::setColor(const std::string& polygonID, const libsumo::TraCIColor& c) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    myParent.createCommand(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::VAR_COLOR, polygonID, &content);
    myParent.processSet(libsumo::CMD_SET_POLYGON_VARIABLE);
}

void
TraCIAPI::PolygonScope::add(const std::string& polygonID, const libsumo::TraCIPositionVector& shape, const libsumo::TraCIColor& c, bool fill, const std::string& type, int layer) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(5);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(type);
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    content.writeUnsignedByte(libsumo::TYPE_UBYTE);
    int f = fill ? 1 : 0;
    content.writeUnsignedByte(f);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(layer);
    content.writeUnsignedByte(libsumo::TYPE_POLYGON);
    content.writeUnsignedByte((int)shape.size());
    for (int i = 0; i < (int)shape.size(); ++i) {
        content.writeDouble(shape[i].x);
        content.writeDouble(shape[i].y);
    }
    myParent.createCommand(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::ADD, polygonID, &content);
    myParent.processSet(libsumo::CMD_SET_POLYGON_VARIABLE);
}

void
TraCIAPI::PolygonScope::remove(const std::string& polygonID, int layer) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(layer);
    myParent.createCommand(libsumo::CMD_SET_POLYGON_VARIABLE, libsumo::REMOVE, polygonID, &content);
    myParent.processSet(libsumo::CMD_SET_POLYGON_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::RouteScope-methods
// ---------------------------------------------------------------------------
std::vector<std::string>
TraCIAPI::RouteScope::getEdges(const std::string& routeID) const {
    return getStringVector(libsumo::VAR_EDGES, routeID);
}


void
TraCIAPI::RouteScope::add(const std::string& routeID, const std::vector<std::string>& edges) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeStringList(edges);
    myParent.createCommand(libsumo::CMD_SET_ROUTE_VARIABLE, libsumo::ADD, routeID, &content);
    myParent.processSet(libsumo::CMD_SET_ROUTE_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::SimulationScope-methods
// ---------------------------------------------------------------------------
int
TraCIAPI::SimulationScope::getCurrentTime() const {
    return getInt(libsumo::VAR_TIME_STEP, "");
}

double
TraCIAPI::SimulationScope::getTime() const {
    return getDouble(libsumo::VAR_TIME, "");
}

int
TraCIAPI::SimulationScope::getLoadedNumber() const {
    return (int) getInt(libsumo::VAR_LOADED_VEHICLES_NUMBER, "");
}

std::vector<std::string>
TraCIAPI::SimulationScope::getLoadedIDList() const {
    return getStringVector(libsumo::VAR_LOADED_VEHICLES_IDS, "");
}

int
TraCIAPI::SimulationScope::getDepartedNumber() const {
    return (int) getInt(libsumo::VAR_DEPARTED_VEHICLES_NUMBER, "");
}

std::vector<std::string>
TraCIAPI::SimulationScope::getDepartedIDList() const {
    return getStringVector(libsumo::VAR_DEPARTED_VEHICLES_IDS, "");
}

int
TraCIAPI::SimulationScope::getArrivedNumber() const {
    return (int) getInt(libsumo::VAR_ARRIVED_VEHICLES_NUMBER, "");
}

std::vector<std::string>
TraCIAPI::SimulationScope::getArrivedIDList() const {
    return getStringVector(libsumo::VAR_ARRIVED_VEHICLES_IDS, "");
}

int
TraCIAPI::SimulationScope::getStartingTeleportNumber() const {
    return (int) getInt(libsumo::VAR_TELEPORT_STARTING_VEHICLES_NUMBER, "");
}

std::vector<std::string>
TraCIAPI::SimulationScope::getStartingTeleportIDList() const {
    return getStringVector(libsumo::VAR_TELEPORT_STARTING_VEHICLES_IDS, "");
}

int
TraCIAPI::SimulationScope::getEndingTeleportNumber() const {
    return (int) getInt(libsumo::VAR_TELEPORT_ENDING_VEHICLES_NUMBER, "");
}

std::vector<std::string>
TraCIAPI::SimulationScope::getEndingTeleportIDList() const {
    return getStringVector(libsumo::VAR_TELEPORT_ENDING_VEHICLES_IDS, "");
}

double
TraCIAPI::SimulationScope::getDeltaT() const {
    return getDouble(libsumo::VAR_DELTA_T, "");
}

libsumo::TraCIPositionVector
TraCIAPI::SimulationScope::getNetBoundary() const {
    return getPolygon(libsumo::VAR_NET_BOUNDING_BOX, "");
}


int
TraCIAPI::SimulationScope::getMinExpectedNumber() const {
    return getInt(libsumo::VAR_MIN_EXPECTED_VEHICLES, "");
}

int
TraCIAPI::SimulationScope::getBusStopWaiting(const std::string& stopID) const {
    return (int) getInt(libsumo::VAR_BUS_STOP_WAITING, stopID);
}

std::vector<std::string>
TraCIAPI::SimulationScope::getBusStopWaitingIDList(const std::string& stopID) const {
    return getStringVector(libsumo::VAR_BUS_STOP_WAITING_IDS, stopID);
}


libsumo::TraCIPosition
TraCIAPI::SimulationScope::convert2D(const std::string& edgeID, double pos, int laneIndex, bool toGeo) const {
    const int posType = toGeo ? libsumo::POSITION_LON_LAT : libsumo::POSITION_2D;
    libsumo::TraCIPosition result;
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeByte(libsumo::POSITION_ROADMAP);
    content.writeString(edgeID);
    content.writeDouble(pos);
    content.writeByte(laneIndex);
    content.writeByte(libsumo::TYPE_UBYTE);
    content.writeByte(posType);
    myParent.createCommand(libsumo::CMD_GET_SIM_VARIABLE, libsumo::POSITION_CONVERSION, "", &content);
    if (myParent.processGet(libsumo::CMD_GET_SIM_VARIABLE, posType)) {
        result.x = myParent.myInput.readDouble();
        result.y = myParent.myInput.readDouble();
    }
    return result;
}


libsumo::TraCIPosition
TraCIAPI::SimulationScope::convert3D(const std::string& edgeID, double pos, int laneIndex, bool toGeo) const {
    const int posType = toGeo ? libsumo::POSITION_LON_LAT_ALT : libsumo::POSITION_3D;
    libsumo::TraCIPosition result;
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeByte(libsumo::POSITION_ROADMAP);
    content.writeString(edgeID);
    content.writeDouble(pos);
    content.writeByte(laneIndex);
    content.writeByte(libsumo::TYPE_UBYTE);
    content.writeByte(posType);
    myParent.createCommand(libsumo::CMD_GET_SIM_VARIABLE, libsumo::POSITION_CONVERSION, "", &content);
    if (myParent.processGet(libsumo::CMD_GET_SIM_VARIABLE, posType)) {
        result.x = myParent.myInput.readDouble();
        result.y = myParent.myInput.readDouble();
        result.z = myParent.myInput.readDouble();
    }
    return result;
}


libsumo::TraCIRoadPosition
TraCIAPI::SimulationScope::convertRoad(double x, double y, bool isGeo, const std::string& vClass) const {
    libsumo::TraCIRoadPosition result;
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(3);
    content.writeByte(isGeo ? libsumo::POSITION_LON_LAT : libsumo::POSITION_2D);
    content.writeDouble(x);
    content.writeDouble(y);
    content.writeByte(libsumo::TYPE_UBYTE);
    content.writeByte(libsumo::POSITION_ROADMAP);
    content.writeByte(libsumo::TYPE_STRING);
    content.writeString(vClass);
    myParent.createCommand(libsumo::CMD_GET_SIM_VARIABLE, libsumo::POSITION_CONVERSION, "", &content);
    if (myParent.processGet(libsumo::CMD_GET_SIM_VARIABLE, libsumo::POSITION_ROADMAP)) {
        result.edgeID = myParent.myInput.readString();
        result.pos = myParent.myInput.readDouble();
        result.laneIndex = myParent.myInput.readUnsignedByte();
    }
    return result;
}


libsumo::TraCIPosition
TraCIAPI::SimulationScope::convertGeo(double x, double y, bool fromGeo) const {
    const int posType = fromGeo ? libsumo::POSITION_2D : libsumo::POSITION_LON_LAT;
    libsumo::TraCIPosition result;
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeByte(fromGeo ? libsumo::POSITION_LON_LAT : libsumo::POSITION_2D);
    content.writeDouble(x);
    content.writeDouble(y);
    content.writeByte(libsumo::TYPE_UBYTE);
    content.writeByte(posType);
    myParent.createCommand(libsumo::CMD_GET_SIM_VARIABLE, libsumo::POSITION_CONVERSION, "", &content);
    if (myParent.processGet(libsumo::CMD_GET_SIM_VARIABLE, posType)) {
        result.x = myParent.myInput.readDouble();
        result.y = myParent.myInput.readDouble();
    }
    return result;
}


double
TraCIAPI::SimulationScope::getDistance2D(double x1, double y1, double x2, double y2, bool isGeo, bool isDriving) {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(3);
    content.writeByte(isGeo ? libsumo::POSITION_LON_LAT : libsumo::POSITION_2D);
    content.writeDouble(x1);
    content.writeDouble(y1);
    content.writeByte(isGeo ? libsumo::POSITION_LON_LAT : libsumo::POSITION_2D);
    content.writeDouble(x2);
    content.writeDouble(y2);
    content.writeByte(isDriving ? libsumo::REQUEST_DRIVINGDIST : libsumo::REQUEST_AIRDIST);
    myParent.createCommand(libsumo::CMD_GET_SIM_VARIABLE, libsumo::DISTANCE_REQUEST, "", &content);
    if (myParent.processGet(libsumo::CMD_GET_SIM_VARIABLE, libsumo::TYPE_DOUBLE)) {
        return myParent.myInput.readDouble();
    }
    return 0.;
}


double
TraCIAPI::SimulationScope::getDistanceRoad(const std::string& edgeID1, double pos1, const std::string& edgeID2, double pos2, bool isDriving) {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(3);
    content.writeByte(libsumo::POSITION_ROADMAP);
    content.writeString(edgeID1);
    content.writeDouble(pos1);
    content.writeByte(0); // lane
    content.writeByte(libsumo::POSITION_ROADMAP);
    content.writeString(edgeID2);
    content.writeDouble(pos2);
    content.writeByte(0); // lane
    content.writeByte(isDriving ? libsumo::REQUEST_DRIVINGDIST : libsumo::REQUEST_AIRDIST);
    myParent.createCommand(libsumo::CMD_GET_SIM_VARIABLE, libsumo::DISTANCE_REQUEST, "", &content);
    if (myParent.processGet(libsumo::CMD_GET_SIM_VARIABLE, libsumo::TYPE_DOUBLE)) {
        return myParent.myInput.readDouble();
    }
    return 0.;
}


libsumo::TraCIStage
TraCIAPI::SimulationScope::findRoute(const std::string& fromEdge, const std::string& toEdge, const std::string& vType, double pos, int routingMode) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_COMPOUND);
    content.writeInt(5);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(fromEdge);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(toEdge);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(vType);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(pos);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(routingMode);
    return getTraCIStage(libsumo::FIND_ROUTE, "", &content);
}


void
TraCIAPI::SimulationScope::writeMessage(const std::string msg) {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(msg);
    myParent.createCommand(libsumo::CMD_SET_SIM_VARIABLE, libsumo::CMD_MESSAGE, "", &content);
    myParent.processSet(libsumo::CMD_SET_SIM_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::TrafficLightScope-methods
// ---------------------------------------------------------------------------
std::string
TraCIAPI::TrafficLightScope::getRedYellowGreenState(const std::string& tlsID) const {
    return getString(libsumo::TL_RED_YELLOW_GREEN_STATE, tlsID);
}

std::vector<libsumo::TraCILogic>
TraCIAPI::TrafficLightScope::getAllProgramLogics(const std::string& tlsID) const {
    std::vector<libsumo::TraCILogic> ret;
    myParent.createCommand(libsumo::CMD_GET_TL_VARIABLE, libsumo::TL_COMPLETE_DEFINITION_RYG, tlsID);
    if (myParent.processGet(libsumo::CMD_GET_TL_VARIABLE, libsumo::TYPE_COMPOUND)) {
        const int logicNo = myParent.myInput.readInt();
        for (int i = 0; i < logicNo; ++i) {
            myParent.myInput.readUnsignedByte();
            myParent.myInput.readInt();
            myParent.myInput.readUnsignedByte();
            const std::string programID = myParent.myInput.readString();
            myParent.myInput.readUnsignedByte();
            const int type = myParent.myInput.readInt();
            myParent.myInput.readUnsignedByte();
            const int phaseIndex = myParent.myInput.readInt();
            myParent.myInput.readUnsignedByte();
            const int phaseNumber = myParent.myInput.readInt();
            libsumo::TraCILogic logic(programID, type, phaseIndex);
            for (int j = 0; j < phaseNumber; j++) {
                myParent.myInput.readUnsignedByte();
                myParent.myInput.readInt();
                myParent.myInput.readUnsignedByte();
                const double duration = myParent.myInput.readDouble();
                myParent.myInput.readUnsignedByte();
                const std::string state = myParent.myInput.readString();
                myParent.myInput.readUnsignedByte();
                const double minDur = myParent.myInput.readDouble();
                myParent.myInput.readUnsignedByte();
                const double maxDur = myParent.myInput.readDouble();
                myParent.myInput.readUnsignedByte();
                const int numNext = myParent.myInput.readInt();
                std::vector<int> next;
                for (int k = 0; k < numNext; k++) {
                    myParent.myInput.readUnsignedByte();
                    next.push_back(myParent.myInput.readInt());
                }
                myParent.myInput.readUnsignedByte();
                const std::string name = myParent.myInput.readString();
                logic.phases.emplace_back(new libsumo::TraCIPhase(duration, state, minDur, maxDur, next, name));
            }
            myParent.myInput.readUnsignedByte();
            const int paramNumber = myParent.myInput.readInt();
            for (int j = 0; j < paramNumber; j++) {
                myParent.myInput.readUnsignedByte();
                const std::vector<std::string> par = myParent.myInput.readStringList();
                logic.subParameter[par[0]] = par[1];
            }
            ret.emplace_back(logic);
        }
    }
    return ret;
}

std::vector<std::string>
TraCIAPI::TrafficLightScope::getControlledLanes(const std::string& tlsID) const {
    return getStringVector(libsumo::TL_CONTROLLED_LANES, tlsID);
}

std::vector<std::vector<libsumo::TraCILink> >
TraCIAPI::TrafficLightScope::getControlledLinks(const std::string& tlsID) const {
    std::vector<std::vector<libsumo::TraCILink> > result;
    myParent.createCommand(libsumo::CMD_GET_TL_VARIABLE, libsumo::TL_CONTROLLED_LINKS, tlsID);
    if (myParent.processGet(libsumo::CMD_GET_TL_VARIABLE, libsumo::TYPE_COMPOUND)) {

        myParent.myInput.readUnsignedByte();
        myParent.myInput.readInt();

        int linkNo = myParent.myInput.readInt();
        for (int i = 0; i < linkNo; ++i) {
            myParent.myInput.readUnsignedByte();
            int no = myParent.myInput.readInt();
            std::vector<libsumo::TraCILink> ret;
            for (int i1 = 0; i1 < no; ++i1) {
                myParent.myInput.readUnsignedByte();
                myParent.myInput.readInt();
                std::string from = myParent.myInput.readString();
                std::string to = myParent.myInput.readString();
                std::string via = myParent.myInput.readString();
                ret.emplace_back(libsumo::TraCILink(from, via, to));
            }
            result.emplace_back(ret);
        }
    }
    return result;
}

std::string
TraCIAPI::TrafficLightScope::getProgram(const std::string& tlsID) const {
    return getString(libsumo::TL_CURRENT_PROGRAM, tlsID);
}

int
TraCIAPI::TrafficLightScope::getPhase(const std::string& tlsID) const {
    return getInt(libsumo::TL_CURRENT_PHASE, tlsID);
}

std::string
TraCIAPI::TrafficLightScope::getPhaseName(const std::string& tlsID) const {
    return getString(libsumo::VAR_NAME, tlsID);
}

double
TraCIAPI::TrafficLightScope::getPhaseDuration(const std::string& tlsID) const {
    return getDouble(libsumo::TL_PHASE_DURATION, tlsID);
}

double
TraCIAPI::TrafficLightScope::getNextSwitch(const std::string& tlsID) const {
    return getDouble(libsumo::TL_NEXT_SWITCH, tlsID);
}


int
TraCIAPI::TrafficLightScope::getServedPersonCount(const std::string& tlsID, int index) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(index);
    return getInt(libsumo::VAR_PERSON_NUMBER, tlsID, &content);
}


void
TraCIAPI::TrafficLightScope::setRedYellowGreenState(const std::string& tlsID, const std::string& state) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(state);
    myParent.createCommand(libsumo::CMD_SET_TL_VARIABLE, libsumo::TL_RED_YELLOW_GREEN_STATE, tlsID, &content);
    myParent.processSet(libsumo::CMD_SET_TL_VARIABLE);
}

void
TraCIAPI::TrafficLightScope::setPhase(const std::string& tlsID, int index) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(index);
    myParent.createCommand(libsumo::CMD_SET_TL_VARIABLE, libsumo::TL_PHASE_INDEX, tlsID, &content);
    myParent.processSet(libsumo::CMD_SET_TL_VARIABLE);
}

void
TraCIAPI::TrafficLightScope::setPhaseName(const std::string& tlsID, const std::string& name) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(name);
    myParent.createCommand(libsumo::CMD_SET_TL_VARIABLE, libsumo::VAR_NAME, tlsID, &content);
    myParent.processSet(libsumo::CMD_SET_TL_VARIABLE);
}

void
TraCIAPI::TrafficLightScope::setProgram(const std::string& tlsID, const std::string& programID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(programID);
    myParent.createCommand(libsumo::CMD_SET_TL_VARIABLE, libsumo::TL_PROGRAM, tlsID, &content);
    myParent.processSet(libsumo::CMD_SET_TL_VARIABLE);
}

void
TraCIAPI::TrafficLightScope::setPhaseDuration(const std::string& tlsID, double phaseDuration) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(phaseDuration);
    myParent.createCommand(libsumo::CMD_SET_TL_VARIABLE, libsumo::TL_PHASE_DURATION, tlsID, &content);
    myParent.processSet(libsumo::CMD_SET_TL_VARIABLE);
}

void
TraCIAPI::TrafficLightScope::setProgramLogic(const std::string& tlsID, const libsumo::TraCILogic& logic) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(5);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(logic.programID);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(logic.type);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(logic.currentPhaseIndex);
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt((int)logic.phases.size());
    for (const libsumo::TraCIPhase* p : logic.phases) {
        content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
        content.writeInt(6);
        content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(p->duration);
        content.writeUnsignedByte(libsumo::TYPE_STRING);
        content.writeString(p->state);
        content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(p->minDur);
        content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(p->maxDur);
        content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
        content.writeInt((int)p->next.size());
        for (int n : p->next) {
            content.writeUnsignedByte(libsumo::TYPE_INTEGER);
            content.writeInt(n);
        }
        content.writeUnsignedByte(libsumo::TYPE_STRING);
        content.writeString(p->name);
    }
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt((int)logic.subParameter.size());
    for (const auto& item : logic.subParameter) {
        content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
        content.writeInt(2);
        content.writeString(item.first);
        content.writeString(item.second);
    }
    myParent.createCommand(libsumo::CMD_SET_TL_VARIABLE, libsumo::TL_COMPLETE_PROGRAM_RYG, tlsID, &content);
    myParent.processSet(libsumo::CMD_SET_TL_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::VehicleTypeScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::VehicleTypeScope::getLength(const std::string& typeID) const {
    return getDouble(libsumo::VAR_LENGTH, typeID);
}

double
TraCIAPI::VehicleTypeScope::getMaxSpeed(const std::string& typeID) const {
    return getDouble(libsumo::VAR_MAXSPEED, typeID);
}

double
TraCIAPI::VehicleTypeScope::getSpeedFactor(const std::string& typeID) const {
    return getDouble(libsumo::VAR_SPEED_FACTOR, typeID);
}

double
TraCIAPI::VehicleTypeScope::getSpeedDeviation(const std::string& typeID) const {
    return getDouble(libsumo::VAR_SPEED_DEVIATION, typeID);
}

double
TraCIAPI::VehicleTypeScope::getAccel(const std::string& typeID) const {
    return getDouble(libsumo::VAR_ACCEL, typeID);
}

double
TraCIAPI::VehicleTypeScope::getDecel(const std::string& typeID) const {
    return getDouble(libsumo::VAR_DECEL, typeID);
}

double
TraCIAPI::VehicleTypeScope::getEmergencyDecel(const std::string& typeID) const {
    return getDouble(libsumo::VAR_EMERGENCY_DECEL, typeID);
}

double
TraCIAPI::VehicleTypeScope::getApparentDecel(const std::string& typeID) const {
    return getDouble(libsumo::VAR_APPARENT_DECEL, typeID);
}

double
TraCIAPI::VehicleTypeScope::getImperfection(const std::string& typeID) const {
    return getDouble(libsumo::VAR_IMPERFECTION, typeID);
}

double
TraCIAPI::VehicleTypeScope::getTau(const std::string& typeID) const {
    return getDouble(libsumo::VAR_TAU, typeID);
}

std::string
TraCIAPI::VehicleTypeScope::getVehicleClass(const std::string& typeID) const {
    return getString(libsumo::VAR_VEHICLECLASS, typeID);
}

std::string
TraCIAPI::VehicleTypeScope::getEmissionClass(const std::string& typeID) const {
    return getString(libsumo::VAR_EMISSIONCLASS, typeID);
}

std::string
TraCIAPI::VehicleTypeScope::getShapeClass(const std::string& typeID) const {
    return getString(libsumo::VAR_SHAPECLASS, typeID);
}

double
TraCIAPI::VehicleTypeScope::getMinGap(const std::string& typeID) const {
    return getDouble(libsumo::VAR_MINGAP, typeID);
}

double
TraCIAPI::VehicleTypeScope::getMinGapLat(const std::string& typeID) const {
    return getDouble(libsumo::VAR_MINGAP_LAT, typeID);
}

double
TraCIAPI::VehicleTypeScope::getMaxSpeedLat(const std::string& typeID) const {
    return getDouble(libsumo::VAR_MAXSPEED_LAT, typeID);
}

std::string
TraCIAPI::VehicleTypeScope::getLateralAlignment(const std::string& typeID) const {
    return getString(libsumo::VAR_LATALIGNMENT, typeID);
}

int
TraCIAPI::VehicleTypeScope::getPersonCapacity(const std::string& typeID) const {
    return getInt(libsumo::VAR_PERSON_CAPACITY, typeID);
}

double
TraCIAPI::VehicleTypeScope::getWidth(const std::string& typeID) const {
    return getDouble(libsumo::VAR_WIDTH, typeID);
}

double
TraCIAPI::VehicleTypeScope::getHeight(const std::string& typeID) const {
    return getDouble(libsumo::VAR_HEIGHT, typeID);
}

libsumo::TraCIColor
TraCIAPI::VehicleTypeScope::getColor(const std::string& typeID) const {
    return getCol(libsumo::VAR_COLOR, typeID);
}



void
TraCIAPI::VehicleTypeScope::setLength(const std::string& typeID, double length) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(length);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_LENGTH, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setMaxSpeed(const std::string& typeID, double speed) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_MAXSPEED, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setVehicleClass(const std::string& typeID, const std::string& clazz) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(clazz);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_VEHICLECLASS, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setSpeedFactor(const std::string& typeID, double factor) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(factor);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_SPEED_FACTOR, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setSpeedDeviation(const std::string& typeID, double deviation) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(deviation);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_SPEED_DEVIATION, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}


void
TraCIAPI::VehicleTypeScope::setEmissionClass(const std::string& typeID, const std::string& clazz) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(clazz);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_EMISSIONCLASS, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setWidth(const std::string& typeID, double width) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(width);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_WIDTH, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setHeight(const std::string& typeID, double height) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(height);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_HEIGHT, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setMinGap(const std::string& typeID, double minGap) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(minGap);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_MINGAP, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}


void
TraCIAPI::VehicleTypeScope::setMinGapLat(const std::string& typeID, double minGapLat) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(minGapLat);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_MINGAP_LAT, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setMaxSpeedLat(const std::string& typeID, double speed) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_MAXSPEED_LAT, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setLateralAlignment(const std::string& typeID, const std::string& latAlignment) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(latAlignment);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_LATALIGNMENT, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::copy(const std::string& origTypeID, const std::string& newTypeID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(newTypeID);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::COPY, origTypeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setShapeClass(const std::string& typeID, const std::string& clazz) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(clazz);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_SHAPECLASS, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setAccel(const std::string& typeID, double accel) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(accel);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_ACCEL, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setDecel(const std::string& typeID, double decel) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(decel);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_DECEL, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setEmergencyDecel(const std::string& typeID, double decel) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(decel);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_EMERGENCY_DECEL, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setApparentDecel(const std::string& typeID, double decel) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(decel);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_APPARENT_DECEL, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setImperfection(const std::string& typeID, double imperfection) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(imperfection);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_IMPERFECTION, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setTau(const std::string& typeID, double tau) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(tau);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_TAU, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}

void
TraCIAPI::VehicleTypeScope::setColor(const std::string& typeID, const libsumo::TraCIColor& c) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    myParent.createCommand(libsumo::CMD_SET_VEHICLETYPE_VARIABLE, libsumo::VAR_COLOR, typeID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLETYPE_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::VehicleScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::VehicleScope::getSpeed(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_SPEED, vehicleID);
}

double
TraCIAPI::VehicleScope::getLateralSpeed(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_SPEED_LAT, vehicleID);
}

double
TraCIAPI::VehicleScope::getAcceleration(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_ACCELERATION, vehicleID);
}

double
TraCIAPI::VehicleScope::getFollowSpeed(const std::string& vehicleID, double speed, double gap, double leaderSpeed, double leaderMaxDecel, const std::string& leaderID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(5);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(gap);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(leaderSpeed);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(leaderMaxDecel);
    content.writeByte(libsumo::TYPE_STRING);
    content.writeString(leaderID);
    return getDouble(libsumo::VAR_FOLLOW_SPEED, vehicleID, &content);
}


double
TraCIAPI::VehicleScope::getSecureGap(const std::string& vehicleID, double speed, double leaderSpeed, double leaderMaxDecel, const std::string& leaderID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(4);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(leaderSpeed);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(leaderMaxDecel);
    content.writeByte(libsumo::TYPE_STRING);
    content.writeString(leaderID);
    return getDouble(libsumo::VAR_SECURE_GAP, vehicleID, &content);
}

double
TraCIAPI::VehicleScope::getStopSpeed(const std::string& vehicleID, double speed, double gap) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(gap);
    return getDouble(libsumo::VAR_STOP_SPEED, vehicleID, &content);
}


double
TraCIAPI::VehicleScope::getMaxSpeed(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_MAXSPEED, vehicleID);
}

libsumo::TraCIPosition
TraCIAPI::VehicleScope::getPosition(const std::string& vehicleID) const {
    return getPos(libsumo::VAR_POSITION, vehicleID);
}

libsumo::TraCIPosition
TraCIAPI::VehicleScope::getPosition3D(const std::string& vehicleID) const {
    return getPos3D(libsumo::VAR_POSITION3D, vehicleID);
}

double
TraCIAPI::VehicleScope::getAngle(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_ANGLE, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getRoadID(const std::string& vehicleID) const {
    return getString(libsumo::VAR_ROAD_ID, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getLaneID(const std::string& vehicleID) const {
    return getString(libsumo::VAR_LANE_ID, vehicleID);
}

int
TraCIAPI::VehicleScope::getLaneIndex(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_LANE_INDEX, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getTypeID(const std::string& vehicleID) const {
    return getString(libsumo::VAR_TYPE, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getRouteID(const std::string& vehicleID) const {
    return getString(libsumo::VAR_ROUTE_ID, vehicleID);
}

int
TraCIAPI::VehicleScope::getRouteIndex(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_ROUTE_INDEX, vehicleID);
}


std::vector<std::string>
TraCIAPI::VehicleScope::getRoute(const std::string& vehicleID) const {
    return getStringVector(libsumo::VAR_EDGES, vehicleID);
}

libsumo::TraCIColor
TraCIAPI::VehicleScope::getColor(const std::string& vehicleID) const {
    return getCol(libsumo::VAR_COLOR, vehicleID);
}

double
TraCIAPI::VehicleScope::getLanePosition(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_LANEPOSITION, vehicleID);
}

double
TraCIAPI::VehicleScope::getDistance(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_DISTANCE, vehicleID);
}

int
TraCIAPI::VehicleScope::getSignals(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_SIGNALS, vehicleID);
}

double
TraCIAPI::VehicleScope::getLateralLanePosition(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_LANEPOSITION_LAT, vehicleID);
}

double
TraCIAPI::VehicleScope::getCO2Emission(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_CO2EMISSION, vehicleID);
}

double
TraCIAPI::VehicleScope::getCOEmission(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_COEMISSION, vehicleID);
}

double
TraCIAPI::VehicleScope::getHCEmission(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_HCEMISSION, vehicleID);
}

double
TraCIAPI::VehicleScope::getPMxEmission(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_PMXEMISSION, vehicleID);
}

double
TraCIAPI::VehicleScope::getNOxEmission(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_NOXEMISSION, vehicleID);
}

double
TraCIAPI::VehicleScope::getFuelConsumption(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_FUELCONSUMPTION, vehicleID);
}

double
TraCIAPI::VehicleScope::getNoiseEmission(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_NOISEEMISSION, vehicleID);
}

double
TraCIAPI::VehicleScope::getElectricityConsumption(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_ELECTRICITYCONSUMPTION, vehicleID);
}

double
TraCIAPI::VehicleScope::getWaitingTime(const std::string& vehID) const {
    return getDouble(libsumo::VAR_WAITING_TIME, vehID);
}

int
TraCIAPI::VehicleScope::getLaneChangeMode(const std::string& vehID) const {
    return getInt(libsumo::VAR_LANECHANGE_MODE, vehID);
}


int
TraCIAPI::VehicleScope::getSpeedMode(const std::string& vehID) const {
    return getInt(libsumo::VAR_SPEEDSETMODE, vehID);
}


double
TraCIAPI::VehicleScope::getSlope(const std::string& vehID) const {
    return getDouble(libsumo::VAR_SLOPE, vehID);
}


std::string
TraCIAPI::VehicleScope::getLine(const std::string& typeID) const {
    return getString(libsumo::VAR_LINE, typeID);
}

std::vector<std::string>
TraCIAPI::VehicleScope::getVia(const std::string& vehicleID) const {
    return getStringVector(libsumo::VAR_VIA, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getEmissionClass(const std::string& vehicleID) const {
    return getString(libsumo::VAR_EMISSIONCLASS, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getShapeClass(const std::string& vehicleID) const {
    return getString(libsumo::VAR_SHAPECLASS, vehicleID);
}

std::vector<libsumo::TraCINextTLSData>
TraCIAPI::VehicleScope::getNextTLS(const std::string& vehID) const {
    std::vector<libsumo::TraCINextTLSData> result;
    myParent.createCommand(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::VAR_NEXT_TLS, vehID);
    if (myParent.processGet(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt(); // components
        // number of items
        myParent.myInput.readUnsignedByte();
        const int n = myParent.myInput.readInt();
        for (int i = 0; i < n; ++i) {
            libsumo::TraCINextTLSData d;
            myParent.myInput.readUnsignedByte();
            d.id = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            d.tlIndex = myParent.myInput.readInt();

            myParent.myInput.readUnsignedByte();
            d.dist = myParent.myInput.readDouble();

            myParent.myInput.readUnsignedByte();
            d.state = (char)myParent.myInput.readByte();

            result.push_back(d);
        }
    }
    return result;
}

std::vector<libsumo::TraCIBestLanesData>
TraCIAPI::VehicleScope::getBestLanes(const std::string& vehicleID) const {
    std::vector<libsumo::TraCIBestLanesData> result;
    myParent.createCommand(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::VAR_BEST_LANES, vehicleID);
    if (myParent.processGet(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt();
        myParent.myInput.readUnsignedByte();

        const int n = myParent.myInput.readInt(); // number of following edge information
        for (int i = 0; i < n; ++i) {
            libsumo::TraCIBestLanesData info;
            myParent.myInput.readUnsignedByte();
            info.laneID = myParent.myInput.readString();

            myParent.myInput.readUnsignedByte();
            info.length = myParent.myInput.readDouble();

            myParent.myInput.readUnsignedByte();
            info.occupation = myParent.myInput.readDouble();

            myParent.myInput.readUnsignedByte();
            info.bestLaneOffset = myParent.myInput.readByte();

            myParent.myInput.readUnsignedByte();
            info.allowsContinuation = (myParent.myInput.readUnsignedByte() == 1);

            myParent.myInput.readUnsignedByte();
            const int m = myParent.myInput.readInt();
            for (int i = 0; i < m; ++i) {
                info.continuationLanes.push_back(myParent.myInput.readString());
            }

            result.push_back(info);
        }
    }
    return result;
}


std::pair<std::string, double>
TraCIAPI::VehicleScope::getLeader(const std::string& vehicleID, double dist) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(dist);
    myParent.createCommand(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::VAR_LEADER, vehicleID, &content);
    if (myParent.processGet(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt(); // components
        myParent.myInput.readUnsignedByte();
        const std::string leaderID = myParent.myInput.readString();
        myParent.myInput.readUnsignedByte();
        const double gap = myParent.myInput.readDouble();
        return std::make_pair(leaderID, gap);
    }
    return std::make_pair("", libsumo::INVALID_DOUBLE_VALUE);
}


std::pair<std::string, double>
TraCIAPI::VehicleScope::getFollower(const std::string& vehicleID, double dist) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(dist);
    myParent.createCommand(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::VAR_FOLLOWER, vehicleID, &content);
    if (myParent.processGet(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt(); // components
        myParent.myInput.readUnsignedByte();
        const std::string leaderID = myParent.myInput.readString();
        myParent.myInput.readUnsignedByte();
        const double gap = myParent.myInput.readDouble();
        return std::make_pair(leaderID, gap);
    }
    return std::make_pair("", libsumo::INVALID_DOUBLE_VALUE);
}


std::pair<int, int>
TraCIAPI::VehicleScope::getLaneChangeState(const std::string& vehicleID, int direction) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(direction);
    myParent.createCommand(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::CMD_CHANGELANE, vehicleID, &content);
    if (myParent.processGet(libsumo::CMD_GET_VEHICLE_VARIABLE, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt(); // components
        myParent.myInput.readUnsignedByte();
        const int stateWithoutTraCI = myParent.myInput.readInt();
        myParent.myInput.readUnsignedByte();
        const int state = myParent.myInput.readInt();
        return std::make_pair(stateWithoutTraCI, state);
    }
    return std::make_pair(libsumo::INVALID_INT_VALUE, libsumo::INVALID_INT_VALUE);
}


int
TraCIAPI::VehicleScope::getStopState(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_STOPSTATE, vehicleID);
}

int
TraCIAPI::VehicleScope::getRoutingMode(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_ROUTING_MODE, vehicleID);
}

double
TraCIAPI::VehicleScope::getStopDelay(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_STOP_DELAY, vehicleID);
}


double
TraCIAPI::VehicleScope::getAccel(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_ACCEL, vehicleID);
}

double
TraCIAPI::VehicleScope::getDecel(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_DECEL, vehicleID);
}

double
TraCIAPI::VehicleScope::getTau(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_TAU, vehicleID);
}

double
TraCIAPI::VehicleScope::getImperfection(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_IMPERFECTION, vehicleID);
}

double
TraCIAPI::VehicleScope::getSpeedFactor(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_SPEED_FACTOR, vehicleID);
}

double
TraCIAPI::VehicleScope::getSpeedDeviation(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_SPEED_DEVIATION, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getVehicleClass(const std::string& vehicleID) const {
    return getString(libsumo::VAR_VEHICLECLASS, vehicleID);
}

double
TraCIAPI::VehicleScope::getMinGap(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_MINGAP, vehicleID);
}

double
TraCIAPI::VehicleScope::getWidth(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_WIDTH, vehicleID);
}

double
TraCIAPI::VehicleScope::getLength(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_LENGTH, vehicleID);
}

double
TraCIAPI::VehicleScope::getHeight(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_HEIGHT, vehicleID);
}

double
TraCIAPI::VehicleScope::getAccumulatedWaitingTime(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_ACCUMULATED_WAITING_TIME, vehicleID);
}

double
TraCIAPI::VehicleScope::getAllowedSpeed(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_ALLOWED_SPEED, vehicleID);
}

int
TraCIAPI::VehicleScope::getPersonNumber(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_PERSON_NUMBER, vehicleID);
}

int
TraCIAPI::VehicleScope::getPersonCapacity(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_PERSON_CAPACITY, vehicleID);
}

std::vector<std::string>
TraCIAPI::VehicleScope::getPersonIDList(const std::string& vehicleID) const {
    return getStringVector(libsumo::LAST_STEP_PERSON_ID_LIST, vehicleID);
}

double
TraCIAPI::VehicleScope::getSpeedWithoutTraCI(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_SPEED_WITHOUT_TRACI, vehicleID);
}

bool
TraCIAPI::VehicleScope::isRouteValid(const std::string& vehicleID) const {
    return getInt(libsumo::VAR_ROUTE_VALID, vehicleID) != 0;
}

double
TraCIAPI::VehicleScope::getMaxSpeedLat(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_MAXSPEED_LAT, vehicleID);
}

double
TraCIAPI::VehicleScope::getMinGapLat(const std::string& vehicleID) const {
    return getDouble(libsumo::VAR_MINGAP_LAT, vehicleID);
}

std::string
TraCIAPI::VehicleScope::getLateralAlignment(const std::string& vehicleID) const {
    return getString(libsumo::VAR_LATALIGNMENT, vehicleID);
}

void
TraCIAPI::VehicleScope::add(const std::string& vehicleID,
                            const std::string& routeID,
                            const std::string& typeID,
                            std::string depart,
                            const std::string& departLane,
                            const std::string& departPos,
                            const std::string& departSpeed,
                            const std::string& arrivalLane,
                            const std::string& arrivalPos,
                            const std::string& arrivalSpeed,
                            const std::string& fromTaz,
                            const std::string& toTaz,
                            const std::string& line,
                            int personCapacity,
                            int personNumber) const {

    if (depart == "-1") {
        depart = toString(myParent.simulation.getCurrentTime() / 1000.0);
    }
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(14);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(routeID);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(typeID);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(depart);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(departLane);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(departPos);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(departSpeed);

    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(arrivalLane);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(arrivalPos);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(arrivalSpeed);

    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(fromTaz);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(toTaz);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(line);

    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(personCapacity);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(personNumber);

    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::ADD_FULL, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::remove(const std::string& vehicleID, char reason) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeUnsignedByte(reason);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::REMOVE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);

}


void
TraCIAPI::VehicleScope::changeTarget(const std::string& vehicleID, const std::string& edgeID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(edgeID);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_CHANGETARGET, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::changeLane(const std::string& vehicleID, int laneIndex, double duration) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeByte(laneIndex);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_CHANGELANE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::changeLaneRelative(const std::string& vehicleID, int laneChange, double duration) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(3);
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeByte(laneChange);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeByte(1);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_CHANGELANE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::changeSublane(const std::string& vehicleID, double latDist) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(latDist);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_CHANGESUBLANE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::setRouteID(const std::string& vehicleID, const std::string& routeID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(routeID);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_ROUTE_ID, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::setRoute(const std::string& vehicleID, const std::vector<std::string>& edges) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeInt((int)edges.size());
    for (int i = 0; i < (int)edges.size(); ++i) {
        content.writeString(edges[i]);
    }
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_ROUTE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::rerouteTraveltime(const std::string& vehicleID, bool currentTravelTimes) const {
    if (currentTravelTimes) {
        // updated edge weights with current network traveltimes (at most once per simulation step)
        std::vector<std::string> edges = myParent.edge.getIDList();
        for (std::vector<std::string>::iterator it = edges.begin(); it != edges.end(); ++it) {
            myParent.edge.adaptTraveltime(*it, myParent.edge.getTraveltime(*it));
        }
    }

    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(0);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_REROUTE_TRAVELTIME, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::moveTo(const std::string& vehicleID, const std::string& laneID, double position, int reason) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(3);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(laneID);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(position);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(reason);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_MOVE_TO, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::moveToXY(const std::string& vehicleID, const std::string& edgeID, const int lane, const double x, const double y, const double angle, const int keepRoute) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(6);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(edgeID);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(lane);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(x);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(y);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(angle);
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeByte(keepRoute);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::MOVE_TO_XY, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::slowDown(const std::string& vehicleID, double speed, double duration) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_SLOWDOWN, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::openGap(const std::string& vehicleID, double newTau, double duration, double changeRate, double maxDecel) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    if (maxDecel > 0) {
        content.writeInt(4);
    } else {
        content.writeInt(3);
    }
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(newTau);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(changeRate);
    if (maxDecel > 0) {
        content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
        content.writeDouble(maxDecel);
    }
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_OPENGAP, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setSpeed(const std::string& vehicleID, double speed) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_SPEED, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setPreviousSpeed(const std::string& vehicleID, double prevspeed) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(prevspeed);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_PREV_SPEED, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setLaneChangeMode(const std::string& vehicleID, int mode) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(mode);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_LANECHANGE_MODE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setSpeedMode(const std::string& vehicleID, int mode) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(mode);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_SPEEDSETMODE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setStop(const std::string vehicleID, const std::string edgeID, const double endPos, const int laneIndex,
                                const double duration, const int flags, const double startPos, const double until) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(7);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(edgeID);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(endPos);
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeByte(laneIndex);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    content.writeUnsignedByte(libsumo::TYPE_BYTE);
    content.writeByte(flags);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(startPos);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(until);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::CMD_STOP, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setType(const std::string& vehicleID, const std::string& typeID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(typeID);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_TYPE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setSpeedFactor(const std::string& vehicleID, double factor) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(factor);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_SPEED_FACTOR, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setMinGap(const std::string& vehicleID, double minGap) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(minGap);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_MINGAP, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setMaxSpeed(const std::string& vehicleID, double speed) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_MAXSPEED, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setColor(const std::string& vehicleID, const libsumo::TraCIColor& c) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_COLOR, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setLine(const std::string& vehicleID, const std::string& line) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(line);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_LINE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setVia(const std::string& vehicleID, const std::vector<std::string>& via) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeInt((int)via.size());
    for (int i = 0; i < (int)via.size(); ++i) {
        content.writeString(via[i]);
    }
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_VIA, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setSignals(const std::string& vehicleID, int signals) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(signals);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_SIGNALS, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setRoutingMode(const std::string& vehicleID, int routingMode) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(routingMode);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_ROUTING_MODE, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::setShapeClass(const std::string& vehicleID, const std::string& clazz) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(clazz);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_SHAPECLASS, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}


void
TraCIAPI::VehicleScope::setEmissionClass(const std::string& vehicleID, const std::string& clazz) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(clazz);
    myParent.createCommand(libsumo::CMD_SET_VEHICLE_VARIABLE, libsumo::VAR_EMISSIONCLASS, vehicleID, &content);
    myParent.processSet(libsumo::CMD_SET_VEHICLE_VARIABLE);
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterLanes(const std::vector<int>& lanes,
        bool noOpposite, double downstreamDist, double upstreamDist) const {
    addSubscriptionFilterByteList(libsumo::FILTER_TYPE_LANES, lanes);
    if (noOpposite) {
        addSubscriptionFilterNoOpposite();
    }
    if (downstreamDist >= 0) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist >= 0) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterNoOpposite() const {
    addSubscriptionFilterEmpty(libsumo::FILTER_TYPE_NOOPPOSITE);
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterDownstreamDistance(double dist) const {
    addSubscriptionFilterFloat(libsumo::FILTER_TYPE_DOWNSTREAM_DIST, dist);
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterUpstreamDistance(double dist) const {
    addSubscriptionFilterFloat(libsumo::FILTER_TYPE_UPSTREAM_DIST, dist);
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterCFManeuver(double downstreamDist, double upstreamDist) const {
    addSubscriptionFilterLeadFollow(std::vector<int>({0}));
    if (downstreamDist >= 0) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist >= 0) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterLCManeuver(int direction, bool noOpposite, double downstreamDist, double upstreamDist) const {
    if (abs(direction) != 1) {
        std::cerr << "Ignoring lane change subscription filter with non-neighboring lane offset direction " << direction << "\n";
        return;
    }
    addSubscriptionFilterLeadFollow(std::vector<int>({0, direction}));
    if (noOpposite) {
        addSubscriptionFilterNoOpposite();
    }
    if (downstreamDist >= 0) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist >= 0) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterLeadFollow(const std::vector<int>& lanes) const {
    addSubscriptionFilterEmpty(libsumo::FILTER_TYPE_LEAD_FOLLOW);
    addSubscriptionFilterByteList(libsumo::FILTER_TYPE_LANES, lanes);
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterTurn(double downstreamDist, double upstreamDist) const {
    addSubscriptionFilterEmpty(libsumo::FILTER_TYPE_TURN);
    if (downstreamDist >= 0) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist >= 0) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterVClass(const std::vector<std::string>& vClasses) const {
    addSubscriptionFilterStringList(libsumo::FILTER_TYPE_VCLASS, vClasses);
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterVType(const std::vector<std::string>& vTypes) const {
    addSubscriptionFilterStringList(libsumo::FILTER_TYPE_VTYPE, vTypes);
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterFieldOfVision(double angle) const {
    addSubscriptionFilterFloat(libsumo::FILTER_TYPE_FIELD_OF_VISION, angle);
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterLateralDistance(double lateralDist, double downstreamDist, double upstreamDist) const {
    addSubscriptionFilterFloat(libsumo::FILTER_TYPE_LATERAL_DIST, lateralDist);
    if (downstreamDist >= 0) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist >= 0) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterEmpty(int filterType) const {
    myParent.createFilterCommand(libsumo::CMD_ADD_SUBSCRIPTION_FILTER, filterType);
    myParent.processSet(libsumo::CMD_ADD_SUBSCRIPTION_FILTER);
}

void
TraCIAPI::VehicleScope::addSubscriptionFilterFloat(int filterType, double val) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(val);
    myParent.createFilterCommand(libsumo::CMD_ADD_SUBSCRIPTION_FILTER, filterType, &content);
    myParent.processSet(libsumo::CMD_ADD_SUBSCRIPTION_FILTER);
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterStringList(int filterType, const std::vector<std::string>& vals) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeStringList(vals);
    myParent.createFilterCommand(libsumo::CMD_ADD_SUBSCRIPTION_FILTER, filterType, &content);
    myParent.processSet(libsumo::CMD_ADD_SUBSCRIPTION_FILTER);
}


void
TraCIAPI::VehicleScope::addSubscriptionFilterByteList(int filterType, const std::vector<int>& vals) const {
    tcpip::Storage content;
    content.writeUnsignedByte((int)vals.size());
    for (int i : vals)  {
        content.writeByte(i);
    }
    myParent.createFilterCommand(libsumo::CMD_ADD_SUBSCRIPTION_FILTER, filterType, &content);
    myParent.processSet(libsumo::CMD_ADD_SUBSCRIPTION_FILTER);
}


// ---------------------------------------------------------------------------
// TraCIAPI::PersonScope-methods
// ---------------------------------------------------------------------------
double
TraCIAPI::PersonScope::getSpeed(const std::string& personID) const {
    return getDouble(libsumo::VAR_SPEED, personID);
}

libsumo::TraCIPosition
TraCIAPI::PersonScope::getPosition(const std::string& personID) const {
    return getPos(libsumo::VAR_POSITION, personID);
}

libsumo::TraCIPosition
TraCIAPI::PersonScope::getPosition3D(const std::string& personID) const {
    return getPos3D(libsumo::VAR_POSITION3D, personID);
}

double
TraCIAPI::PersonScope::getAngle(const std::string& personID) const {
    return getDouble(libsumo::VAR_ANGLE, personID);
}

double
TraCIAPI::PersonScope::getSlope(const std::string& personID) const {
    return getDouble(libsumo::VAR_SLOPE, personID);
}

double
TraCIAPI::PersonScope::getLanePosition(const std::string& personID) const {
    return getDouble(libsumo::VAR_LANEPOSITION, personID);
}

libsumo::TraCIColor
TraCIAPI::PersonScope::getColor(const std::string& personID) const {
    return getCol(libsumo::VAR_COLOR, personID);
}

double
TraCIAPI::PersonScope::getLength(const std::string& personID) const {
    return getDouble(libsumo::VAR_LENGTH, personID);
}

std::string
TraCIAPI::PersonScope::getRoadID(const std::string& personID) const {
    return getString(libsumo::VAR_ROAD_ID, personID);
}

std::string
TraCIAPI::PersonScope::getLaneID(const std::string& personID) const {
    return getString(libsumo::VAR_LANE_ID, personID);
}

std::string
TraCIAPI::PersonScope::getTypeID(const std::string& personID) const {
    return getString(libsumo::VAR_TYPE, personID);
}

double
TraCIAPI::PersonScope::getWaitingTime(const std::string& personID) const {
    return getDouble(libsumo::VAR_WAITING_TIME, personID);
}

std::string
TraCIAPI::PersonScope::getNextEdge(const std::string& personID) const {
    return getString(libsumo::VAR_NEXT_EDGE, personID);
}


std::string
TraCIAPI::PersonScope::getVehicle(const std::string& personID) const {
    return getString(libsumo::VAR_VEHICLE, personID);
}

int
TraCIAPI::PersonScope::getRemainingStages(const std::string& personID) const {
    return getInt(libsumo::VAR_STAGES_REMAINING, personID);
}

libsumo::TraCIStage
TraCIAPI::PersonScope::getStage(const std::string& personID, int nextStageIndex) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(nextStageIndex);
    return getTraCIStage(libsumo::VAR_STAGE, personID, &content);
}

std::vector<std::string>
TraCIAPI::PersonScope::getEdges(const std::string& personID, int nextStageIndex) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(nextStageIndex);
    return getStringVector(libsumo::VAR_EDGES, personID, &content);
}

void
TraCIAPI::PersonScope::removeStages(const std::string& personID) const {
    // remove all stages after the current and then abort the current stage
    while (getRemainingStages(personID) > 1) {
        removeStage(personID, 1);
    }
    removeStage(personID, 0);
}


void
TraCIAPI::PersonScope::rerouteTraveltime(const std::string& personID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(0);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::CMD_REROUTE_TRAVELTIME, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


void
TraCIAPI::PersonScope::add(const std::string& personID, const std::string& edgeID, double pos, double depart, const std::string typeID) {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(4);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(typeID);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(edgeID);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(depart);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(pos);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::ADD, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::appendStage(const std::string& personID, const libsumo::TraCIStage& stage) {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(13);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(stage.type);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stage.vType);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stage.line);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stage.destStop);
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeStringList(stage.edges);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(stage.travelTime);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(stage.cost);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(stage.length);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stage.intended);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(stage.depart);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(stage.departPos);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(stage.arrivalPos);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stage.description);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::APPEND_STAGE, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


void
TraCIAPI::PersonScope::appendWaitingStage(const std::string& personID, double duration, const std::string& description, const std::string& stopID) {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(4);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(libsumo::STAGE_WAITING);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(description);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stopID);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::APPEND_STAGE, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::appendWalkingStage(const std::string& personID, const std::vector<std::string>& edges, double arrivalPos, double duration, double speed, const std::string& stopID) {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(6);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(libsumo::STAGE_WALKING);
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeStringList(edges);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(arrivalPos);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(duration);
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stopID);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::APPEND_STAGE, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::appendDrivingStage(const std::string& personID, const std::string& toEdge, const std::string& lines, const std::string& stopID) {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(4);
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(libsumo::STAGE_DRIVING);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(toEdge);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(lines);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(stopID);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::APPEND_STAGE, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::removeStage(const std::string& personID, int nextStageIndex) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_INTEGER);
    content.writeInt(nextStageIndex);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::REMOVE_STAGE, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


void
TraCIAPI::PersonScope::setSpeed(const std::string& personID, double speed) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(speed);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_SPEED, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


void
TraCIAPI::PersonScope::setType(const std::string& personID, const std::string& typeID) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(typeID);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_TYPE, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::setLength(const std::string& personID, double length) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(length);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_LENGTH, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


void
TraCIAPI::PersonScope::setWidth(const std::string& personID, double width) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(width);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_WIDTH, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::setHeight(const std::string& personID, double height) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(height);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_HEIGHT, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}

void
TraCIAPI::PersonScope::setMinGap(const std::string& personID, double minGap) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(minGap);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_MINGAP, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


void
TraCIAPI::PersonScope::setColor(const std::string& personID, const libsumo::TraCIColor& c) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COLOR);
    content.writeUnsignedByte(c.r);
    content.writeUnsignedByte(c.g);
    content.writeUnsignedByte(c.b);
    content.writeUnsignedByte(c.a);
    myParent.createCommand(libsumo::CMD_SET_PERSON_VARIABLE, libsumo::VAR_COLOR, personID, &content);
    myParent.processSet(libsumo::CMD_SET_PERSON_VARIABLE);
}


// ---------------------------------------------------------------------------
// TraCIAPI::TraCIScopeWrapper-methods
// ---------------------------------------------------------------------------

int
TraCIAPI::TraCIScopeWrapper::getUnsignedByte(int var, const std::string& id, tcpip::Storage* add) const {
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_UBYTE)) {
        return myParent.myInput.readUnsignedByte();
    }
    return libsumo::INVALID_INT_VALUE;
}


int
TraCIAPI::TraCIScopeWrapper::getByte(int var, const std::string& id, tcpip::Storage* add) const {
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_BYTE)) {
        return myParent.myInput.readByte();
    }
    return libsumo::INVALID_INT_VALUE;
}



int
TraCIAPI::TraCIScopeWrapper::getInt(int var, const std::string& id, tcpip::Storage* add) const {
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_INTEGER)) {
        return myParent.myInput.readInt();
    }
    return libsumo::INVALID_INT_VALUE;
}


double
TraCIAPI::TraCIScopeWrapper::getDouble(int var, const std::string& id, tcpip::Storage* add) const {
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_DOUBLE)) {
        return myParent.myInput.readDouble();
    }
    return libsumo::INVALID_DOUBLE_VALUE;
}


libsumo::TraCIPositionVector
TraCIAPI::TraCIScopeWrapper::getPolygon(int var, const std::string& id, tcpip::Storage* add) const {
    libsumo::TraCIPositionVector ret;
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_POLYGON)) {
        int size = myParent.myInput.readUnsignedByte();
        if (size == 0) {
            size = myParent.myInput.readInt();
        }
        for (int i = 0; i < size; ++i) {
            libsumo::TraCIPosition p;
            p.x = myParent.myInput.readDouble();
            p.y = myParent.myInput.readDouble();
            p.z = 0.;
            ret.push_back(p);
        }
    }
    return ret;
}


libsumo::TraCIPosition
TraCIAPI::TraCIScopeWrapper::getPos(int var, const std::string& id, tcpip::Storage* add) const {
    libsumo::TraCIPosition p;
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::POSITION_2D)) {
        p.x = myParent.myInput.readDouble();
        p.y = myParent.myInput.readDouble();
        p.z = 0;
    }
    return p;
}


libsumo::TraCIPosition
TraCIAPI::TraCIScopeWrapper::getPos3D(int var, const std::string& id, tcpip::Storage* add) const {
    libsumo::TraCIPosition p;
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::POSITION_3D)) {
        p.x = myParent.myInput.readDouble();
        p.y = myParent.myInput.readDouble();
        p.z = myParent.myInput.readDouble();
    }
    return p;
}


std::string
TraCIAPI::TraCIScopeWrapper::getString(int var, const std::string& id, tcpip::Storage* add) const {
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_STRING)) {
        return myParent.myInput.readString();
    }
    return "";
}


std::vector<std::string>
TraCIAPI::TraCIScopeWrapper::getStringVector(int var, const std::string& id, tcpip::Storage* add) const {
    std::vector<std::string> r;
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_STRINGLIST)) {
        const int size = myParent.myInput.readInt();
        for (int i = 0; i < size; ++i) {
            r.push_back(myParent.myInput.readString());
        }
    }
    return r;
}


libsumo::TraCIColor
TraCIAPI::TraCIScopeWrapper::getCol(int var, const std::string& id, tcpip::Storage* add) const {
    libsumo::TraCIColor c;
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_COLOR)) {
        c.r = (unsigned char)myParent.myInput.readUnsignedByte();
        c.g = (unsigned char)myParent.myInput.readUnsignedByte();
        c.b = (unsigned char)myParent.myInput.readUnsignedByte();
        c.a = (unsigned char)myParent.myInput.readUnsignedByte();
    }
    return c;
}


libsumo::TraCIStage
TraCIAPI::TraCIScopeWrapper::getTraCIStage(int var, const std::string& id, tcpip::Storage* add) const {
    libsumo::TraCIStage s;
    myParent.createCommand(myCmdGetID, var, id, add);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt(); // components
        myParent.myInput.readUnsignedByte();
        s.type = myParent.myInput.readInt();

        myParent.myInput.readUnsignedByte();
        s.vType = myParent.myInput.readString();

        myParent.myInput.readUnsignedByte();
        s.line = myParent.myInput.readString();

        myParent.myInput.readUnsignedByte();
        s.destStop = myParent.myInput.readString();

        myParent.myInput.readUnsignedByte();
        s.edges = myParent.myInput.readStringList();

        myParent.myInput.readUnsignedByte();
        s.travelTime = myParent.myInput.readDouble();

        myParent.myInput.readUnsignedByte();
        s.cost = myParent.myInput.readDouble();

        myParent.myInput.readUnsignedByte();
        s.length = myParent.myInput.readDouble();

        myParent.myInput.readUnsignedByte();
        s.intended = myParent.myInput.readString();

        myParent.myInput.readUnsignedByte();
        s.depart = myParent.myInput.readDouble();

        myParent.myInput.readUnsignedByte();
        s.departPos = myParent.myInput.readDouble();

        myParent.myInput.readUnsignedByte();
        s.arrivalPos = myParent.myInput.readDouble();

        myParent.myInput.readUnsignedByte();
        s.description = myParent.myInput.readString();
    }
    return s;
}


std::vector<std::string>
TraCIAPI::TraCIScopeWrapper::getIDList() const {
    return getStringVector(libsumo::TRACI_ID_LIST, "");
}


int
TraCIAPI::TraCIScopeWrapper::getIDCount() const {
    return getInt(libsumo::ID_COUNT, "");
}


std::string
TraCIAPI::TraCIScopeWrapper::getParameter(const std::string& objectID, const std::string& key) const {
    tcpip::Storage content;
    content.writeByte(libsumo::TYPE_STRING);
    content.writeString(key);
    return getString(libsumo::VAR_PARAMETER, objectID, &content);
}


std::pair<std::string, std::string>
TraCIAPI::TraCIScopeWrapper::getParameterWithKey(const std::string& objectID, const std::string& key) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(key);

    myParent.createCommand(myCmdGetID, libsumo::VAR_PARAMETER_WITH_KEY, objectID, &content);
    if (myParent.processGet(myCmdGetID, libsumo::TYPE_COMPOUND)) {
        myParent.myInput.readInt();  // number of components
        myParent.myInput.readUnsignedByte();
        const std::string returnedKey = myParent.myInput.readString();
        myParent.myInput.readUnsignedByte();
        const std::string value = myParent.myInput.readString();
        return std::make_pair(returnedKey, value);
    }
    return std::make_pair(key, "");
}


void
TraCIAPI::TraCIScopeWrapper::setParameter(const std::string& objectID, const std::string& key, const std::string& value) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_COMPOUND);
    content.writeInt(2);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(key);
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(value);
    myParent.createCommand(myCmdSetID, libsumo::VAR_PARAMETER, objectID, &content);
    myParent.processSet(myCmdSetID);
}


void
TraCIAPI::TraCIScopeWrapper::setInt(int var, const std::string& id, int value) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_INTEGER);
    content.writeInt(value);
    myParent.createCommand(myCmdSetID, var, id, &content);
    myParent.processSet(myCmdSetID);
}


void
TraCIAPI::TraCIScopeWrapper::setDouble(int var, const std::string& id, double value) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_DOUBLE);
    content.writeDouble(value);
    myParent.createCommand(myCmdSetID, var, id, &content);
    myParent.processSet(myCmdSetID);
}


void
TraCIAPI::TraCIScopeWrapper::setString(int var, const std::string& id, const std::string& value) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRING);
    content.writeString(value);
    myParent.createCommand(myCmdSetID, var, id, &content);
    myParent.processSet(myCmdSetID);
}


void
TraCIAPI::TraCIScopeWrapper::setStringVector(int var, const std::string& id, const std::vector<std::string>& value) const {
    tcpip::Storage content;
    content.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
    content.writeInt((int)value.size());
    for (const std::string& s : value) {
        content.writeString(s);
    }
    myParent.createCommand(myCmdSetID, var, id, &content);
    myParent.processSet(myCmdSetID);
}


void
TraCIAPI::TraCIScopeWrapper::subscribe(const std::string& objID, const std::vector<int>& vars, double beginTime, double endTime) const {
    myParent.send_commandSubscribeObjectVariable(mySubscribeID, objID, beginTime, endTime, vars);
    tcpip::Storage inMsg;
    myParent.check_resultState(inMsg, mySubscribeID);
    if (vars.size() > 0) {
        myParent.check_commandGetResult(inMsg, mySubscribeID);
        myParent.readVariableSubscription(mySubscribeID + 0x10, inMsg);
    }
}


void
TraCIAPI::TraCIScopeWrapper::subscribeContext(const std::string& objID, int domain, double range, const std::vector<int>& vars, double beginTime, double endTime) const {
    myParent.send_commandSubscribeObjectContext(myContextSubscribeID, objID, beginTime, endTime, domain, range, vars);
    tcpip::Storage inMsg;
    myParent.check_resultState(inMsg, myContextSubscribeID);
    myParent.check_commandGetResult(inMsg, myContextSubscribeID);
    myParent.readContextSubscription(myContextSubscribeID + 0x60, inMsg);
}


const libsumo::SubscriptionResults
TraCIAPI::TraCIScopeWrapper::getAllSubscriptionResults() const {
    return mySubscriptionResults;
}


const libsumo::TraCIResults
TraCIAPI::TraCIScopeWrapper::getSubscriptionResults(const std::string& objID) const {
    if (mySubscriptionResults.find(objID) != mySubscriptionResults.end()) {
        return mySubscriptionResults.find(objID)->second;
    } else {
        return libsumo::TraCIResults();
    }
}


const libsumo::ContextSubscriptionResults
TraCIAPI::TraCIScopeWrapper::getAllContextSubscriptionResults() const {
    return myContextSubscriptionResults;
}


const libsumo::SubscriptionResults
TraCIAPI::TraCIScopeWrapper::getContextSubscriptionResults(const std::string& objID) const {
    if (myContextSubscriptionResults.find(objID) != myContextSubscriptionResults.end()) {
        return myContextSubscriptionResults.find(objID)->second;
    } else {
        return libsumo::SubscriptionResults();
    }
}


void
TraCIAPI::TraCIScopeWrapper::clearSubscriptionResults() {
    mySubscriptionResults.clear();
    myContextSubscriptionResults.clear();
}


libsumo::SubscriptionResults&
TraCIAPI::TraCIScopeWrapper::getModifiableSubscriptionResults() {
    return mySubscriptionResults;
}


libsumo::SubscriptionResults&
TraCIAPI::TraCIScopeWrapper::getModifiableContextSubscriptionResults(const std::string& objID) {
    return myContextSubscriptionResults[objID];
}


/****************************************************************************/
