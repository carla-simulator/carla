/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCITestClient.cpp
/// @author  Friedemann Wesner
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Axel Wegener
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2008/04/07
///
// A test execution class
/****************************************************************************/
/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

#define BUILD_TCPIP
#include <foreign/tcpip/storage.h>
#include <foreign/tcpip/socket.h>

#include <libsumo/TraCIConstants.h>
#include <libsumo/TraCIDefs.h>
#include "TraCITestClient.h"


// ===========================================================================
// method definitions
// ===========================================================================
TraCITestClient::TraCITestClient(std::string outputFileName)
    : outputFileName(outputFileName), answerLog("") {
    answerLog.setf(std::ios::fixed, std::ios::floatfield);  // use decimal format
    answerLog.setf(std::ios::showpoint); // print decimal point
    answerLog << std::setprecision(2);
}


TraCITestClient::~TraCITestClient() {
    writeResult();
}


int
TraCITestClient::run(std::string fileName, int port, std::string host) {
    std::ifstream defFile;
    std::string fileContentStr;
    std::stringstream fileContent;
    std::string lineCommand;
    std::stringstream msg;
    int repNo = 1;
    bool commentRead = false;

    // try to connect
    try {
        TraCIAPI::connect(host, port);
    } catch (tcpip::SocketException& e) {
        std::stringstream msg;
        msg << "#Error while connecting: " << e.what();
        errorMsg(msg);
        return 2;
    }

    // read definition file and trigger commands according to it
    defFile.open(fileName.c_str());
    if (!defFile) {
        msg << "Can not open definition file " << fileName << std::endl;
        errorMsg(msg);
        return 1;
    }
    defFile.unsetf(std::ios::dec);

    while (defFile >> lineCommand) {
        repNo = 1;
        if (lineCommand.compare("%") == 0) {
            // a comment was read
            commentRead = !commentRead;
            continue;
        }
        if (commentRead) {
            // wait until end of comment is reached
            continue;
        }
        if (lineCommand.compare("repeat") == 0) {
            defFile >> repNo;
            defFile >> lineCommand;
        }
        if (lineCommand.compare("simstep2") == 0) {
            // read parameter for command simulation step and trigger command
            double time;
            defFile >> time;
            for (int i = 0; i < repNo; i++) {
                commandSimulationStep(time);
            }
        } else if (lineCommand.compare("getvariable") == 0) {
            // trigger command GetXXXVariable
            int domID, varID;
            std::string objID;
            defFile >> domID >> varID >> objID;
            commandGetVariable(domID, varID, objID);
        } else if (lineCommand.compare("getvariable_plus") == 0) {
            // trigger command GetXXXVariable with one parameter
            int domID, varID;
            std::string objID;
            defFile >> domID >> varID >> objID;
            std::stringstream msg;
            tcpip::Storage tmp;
            setValueTypeDependant(tmp, defFile, msg);
            std::string msgS = msg.str();
            if (msgS != "") {
                errorMsg(msg);
            }
            commandGetVariable(domID, varID, objID, &tmp);
        } else if (lineCommand.compare("subscribevariable") == 0) {
            // trigger command SubscribeXXXVariable
            int domID, varNo;
            double beginTime, endTime;
            std::string objID;
            defFile >> domID >> objID >> beginTime >> endTime >> varNo;
            commandSubscribeObjectVariable(domID, objID, beginTime, endTime, varNo, defFile);
        }  else if (lineCommand.compare("subscribecontext") == 0) {
            // trigger command SubscribeXXXVariable
            int domID, varNo, domain;
            double range;
            double beginTime, endTime;
            std::string objID;
            defFile >> domID >> objID >> beginTime >> endTime >> domain >> range >> varNo;
            commandSubscribeContextVariable(domID, objID, beginTime, endTime, domain, range, varNo, defFile);
        }  else if (lineCommand.compare("setvalue") == 0) {
            // trigger command SetXXXValue
            int domID, varID;
            std::string objID;
            defFile >> domID >> varID >> objID;
            commandSetValue(domID, varID, objID, defFile);
        } else if (lineCommand.compare("testAPI") == 0) {
            // call all native API methods
            testAPI();
        } else if (lineCommand.compare("setorder") == 0) {
            // call setOrder
            int order;
            defFile >> order;
            commandSetOrder(order);
        } else {
            msg << "Error in definition file: " << lineCommand << " is not a valid command";
            errorMsg(msg);
            commandClose();
            closeSocket();
            return 1;
        }
    }
    defFile.close();
    commandClose();
    closeSocket();
    return 0;
}


// ---------- Commands handling
void
TraCITestClient::commandSimulationStep(double time) {
    try {
        send_commandSimulationStep(time);
        answerLog << std::endl << "-> Command sent: <SimulationStep>:" << std::endl;
        tcpip::Storage inMsg;
        std::string acknowledgement;
        check_resultState(inMsg, libsumo::CMD_SIMSTEP, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
        validateSimulationStep2(inMsg);
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
    }
}


void
TraCITestClient::commandClose() {
    try {
        send_commandClose();
        answerLog << std::endl << "-> Command sent: <Close>:" << std::endl;
        tcpip::Storage inMsg;
        std::string acknowledgement;
        check_resultState(inMsg, libsumo::CMD_CLOSE, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
    }
}


void
TraCITestClient::commandSetOrder(int order) {
    try {
        send_commandSetOrder(order);
        answerLog << std::endl << "-> Command sent: <SetOrder>:" << std::endl;
        tcpip::Storage inMsg;
        std::string acknowledgement;
        check_resultState(inMsg, libsumo::CMD_SETORDER, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
    }
}


void
TraCITestClient::commandGetVariable(int domID, int varID, const std::string& objID, tcpip::Storage* addData) {
    tcpip::Storage inMsg;
    try {
        createCommand(domID, varID, objID, addData);
        mySocket->sendExact(myOutput);
        answerLog << std::endl << "-> Command sent: <GetVariable>:" << std::endl
                  << "  domID=" << domID << " varID=" << varID
                  << " objID=" << objID << std::endl;
        std::string acknowledgement;
        check_resultState(inMsg, domID, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
        return;
    }
    check_commandGetResult(inMsg, domID, -1, false);
    // report result state
    try {
        int variableID = inMsg.readUnsignedByte();
        std::string objectID = inMsg.readString();
        answerLog <<  "  CommandID=" << (domID + 0x10) << "  VariableID=" << variableID << "  ObjectID=" << objectID;
        int valueDataType = inMsg.readUnsignedByte();
        answerLog << " valueDataType=" << valueDataType;
        readAndReportTypeDependent(inMsg, valueDataType);
    } catch (libsumo::TraCIException& e) {
        std::stringstream msg;
        msg << "Error while receiving command: " << e.what();
        errorMsg(msg);
        return;
    }
}


void
TraCITestClient::commandSetValue(int domID, int varID, const std::string& objID, std::ifstream& defFile) {
    std::stringstream msg;
    tcpip::Storage inMsg, tmp;
    setValueTypeDependant(tmp, defFile, msg);
    std::string msgS = msg.str();
    if (msgS != "") {
        errorMsg(msg);
    }
    createCommand(domID, varID, objID, &tmp);
    mySocket->sendExact(myOutput);
    answerLog << std::endl << "-> Command sent: <SetValue>:" << std::endl
              << "  domID=" << domID << " varID=" << varID
              << " objID=" << objID << std::endl;
    try {
        std::string acknowledgement;
        check_resultState(inMsg, domID, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
    }
}


void
TraCITestClient::commandSubscribeObjectVariable(int domID, const std::string& objID, double beginTime, double endTime, int varNo, std::ifstream& defFile) {
    std::vector<int> vars;
    for (int i = 0; i < varNo; ++i) {
        int var;
        defFile >> var;
        // variable id
        vars.push_back(var);
    }
    send_commandSubscribeObjectVariable(domID, objID, beginTime, endTime, vars);
    answerLog << std::endl << "-> Command sent: <SubscribeVariable>:" << std::endl
              << "  domID=" << domID << " objID=" << objID << " with " << varNo << " variables" << std::endl;
    tcpip::Storage inMsg;
    try {
        std::string acknowledgement;
        check_resultState(inMsg, domID, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
        validateSubscription(inMsg);
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
    }
}


void
TraCITestClient::commandSubscribeContextVariable(int domID, const std::string& objID, double beginTime, double endTime,
        int domain, double range, int varNo, std::ifstream& defFile) {
    std::vector<int> vars;
    for (int i = 0; i < varNo; ++i) {
        int var;
        defFile >> var;
        // variable id
        vars.push_back(var);
    }
    send_commandSubscribeObjectContext(domID, objID, beginTime, endTime, domain, range, vars);
    answerLog << std::endl << "-> Command sent: <SubscribeContext>:" << std::endl
              << "  domID=" << domID << " objID=" << objID << " domain=" << domain << " range=" << range
              << " with " << varNo << " variables" << std::endl;
    tcpip::Storage inMsg;
    try {
        std::string acknowledgement;
        check_resultState(inMsg, domID, false, &acknowledgement);
        answerLog << acknowledgement << std::endl;
        validateSubscription(inMsg);
    } catch (libsumo::TraCIException& e) {
        answerLog << e.what() << std::endl;
    }
}


// ---------- Report helper
void
TraCITestClient::writeResult() {
    time_t seconds;
    tm* locTime;
    std::ofstream outFile(outputFileName.c_str());
    if (!outFile) {
        std::cerr << "Unable to write result file" << std::endl;
    }
    time(&seconds);
    locTime = localtime(&seconds);
    outFile << "TraCITestClient output file. Date: " << asctime(locTime) << std::endl;
    outFile << answerLog.str();
    outFile.close();
}


void
TraCITestClient::errorMsg(std::stringstream& msg) {
    std::cerr << msg.str() << std::endl;
    answerLog << "----" << std::endl << msg.str() << std::endl;
}






bool
TraCITestClient::validateSimulationStep2(tcpip::Storage& inMsg) {
    try {
        int noSubscriptions = inMsg.readInt();
        for (int s = 0; s < noSubscriptions; ++s) {
            if (!validateSubscription(inMsg)) {
                return false;
            }
        }
    } catch (std::invalid_argument& e) {
        answerLog << "#Error while reading message:" << e.what() << std::endl;
        return false;
    }
    return true;
}


bool
TraCITestClient::validateSubscription(tcpip::Storage& inMsg) {
    try {
        int length = inMsg.readUnsignedByte();
        if (length == 0) {
            length = inMsg.readInt();
        }
        int cmdId = inMsg.readUnsignedByte();
        if (cmdId >= libsumo::RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE && cmdId <= libsumo::RESPONSE_SUBSCRIBE_GUI_VARIABLE) {
            answerLog << "  CommandID=" << cmdId;
            answerLog << "  ObjectID=" << inMsg.readString();
            int varNo = inMsg.readUnsignedByte();
            answerLog << "  #variables=" << varNo << std::endl;
            for (int i = 0; i < varNo; ++i) {
                answerLog << "      VariableID=" << inMsg.readUnsignedByte();
                bool ok = inMsg.readUnsignedByte() == libsumo::RTYPE_OK;
                answerLog << "      ok=" << ok;
                int valueDataType = inMsg.readUnsignedByte();
                answerLog << " valueDataType=" << valueDataType;
                readAndReportTypeDependent(inMsg, valueDataType);
            }
        } else if (cmdId >= libsumo::RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT && cmdId <= libsumo::RESPONSE_SUBSCRIBE_GUI_CONTEXT) {
            answerLog << "  CommandID=" << cmdId;
            answerLog << "  ObjectID=" << inMsg.readString();
            answerLog << "  Domain=" << inMsg.readUnsignedByte();
            int varNo = inMsg.readUnsignedByte();
            answerLog << "  #variables=" << varNo << std::endl;
            int objNo = inMsg.readInt();
            answerLog << "  #objects=" << objNo << std::endl;
            for (int j = 0; j < objNo; ++j) {
                answerLog << "   ObjectID=" << inMsg.readString() << std::endl;
                for (int i = 0; i < varNo; ++i) {
                    answerLog << "      VariableID=" << inMsg.readUnsignedByte();
                    bool ok = inMsg.readUnsignedByte() == libsumo::RTYPE_OK;
                    answerLog << "      ok=" << ok;
                    int valueDataType = inMsg.readUnsignedByte();
                    answerLog << " valueDataType=" << valueDataType;
                    readAndReportTypeDependent(inMsg, valueDataType);
                }
            }
        } else {
            answerLog << "#Error: received response with command id: " << cmdId << " but expected a subscription response (0xe0-0xef / 0x90-0x9f)" << std::endl;
            return false;
        }
    } catch (std::invalid_argument& e) {
        answerLog << "#Error while reading message:" << e.what() << std::endl;
        return false;
    }
    return true;
}







// ---------- Conversion helper
int
TraCITestClient::setValueTypeDependant(tcpip::Storage& into, std::ifstream& defFile, std::stringstream& msg) {
    std::string dataTypeS;
    defFile >> dataTypeS;
    if (dataTypeS == "<airDist>") {
        into.writeUnsignedByte(libsumo::REQUEST_AIRDIST);
        return 1;
    } else if (dataTypeS == "<drivingDist>") {
        into.writeUnsignedByte(libsumo::REQUEST_DRIVINGDIST);
        return 1;
    } else if (dataTypeS == "<objSubscription>") {
        int beginTime, endTime, numVars;
        defFile >> beginTime >> endTime >> numVars;
        into.writeInt(beginTime);
        into.writeInt(endTime);
        into.writeInt(numVars);
        for (int i = 0; i < numVars; ++i) {
            int var;
            defFile >> var;
            into.writeUnsignedByte(var);
        }
        return 4 + 4 + 4 + numVars;
    }
    int valI;
    double valF;
    if (dataTypeS == "<int>") {
        defFile >> valI;
        into.writeUnsignedByte(libsumo::TYPE_INTEGER);
        into.writeInt(valI);
        return 4 + 1;
    } else if (dataTypeS == "<byte>") {
        defFile >> valI;
        into.writeUnsignedByte(libsumo::TYPE_BYTE);
        into.writeByte(valI);
        return 1 + 1;
    }  else if (dataTypeS == "<ubyte>") {
        defFile >> valI;
        into.writeUnsignedByte(libsumo::TYPE_UBYTE);
        into.writeUnsignedByte(valI);
        return 1 + 1;
    } else if (dataTypeS == "<double>") {
        defFile >> valF;
        into.writeUnsignedByte(libsumo::TYPE_DOUBLE);
        into.writeDouble(valF);
        return 8 + 1;
    } else if (dataTypeS == "<string>") {
        std::string valueS;
        defFile >> valueS;
        if (valueS == "\"\"") {
            valueS = "";
        }
        into.writeUnsignedByte(libsumo::TYPE_STRING);
        into.writeString(valueS);
        return 4 + 1 + (int) valueS.length();
    } else if (dataTypeS == "<string*>") {
        std::vector<std::string> slValue;
        defFile >> valI;
        int length = 1 + 4;
        for (int i = 0; i < valI; ++i) {
            std::string tmp;
            defFile >> tmp;
            slValue.push_back(tmp);
            length += 4 + int(tmp.length());
        }
        into.writeUnsignedByte(libsumo::TYPE_STRINGLIST);
        into.writeStringList(slValue);
        return length;
    } else if (dataTypeS == "<compound>") {
        defFile >> valI;
        into.writeUnsignedByte(libsumo::TYPE_COMPOUND);
        into.writeInt(valI);
        int length = 1 + 4;
        for (int i = 0; i < valI; ++i) {
            length += setValueTypeDependant(into, defFile, msg);
        }
        return length;
    } else if (dataTypeS == "<color>") {
        defFile >> valI;
        into.writeUnsignedByte(libsumo::TYPE_COLOR);
        into.writeUnsignedByte(valI);
        for (int i = 0; i < 3; ++i) {
            defFile >> valI;
            into.writeUnsignedByte(valI);
        }
        return 1 + 4;
    } else if (dataTypeS == "<position2D>") {
        defFile >> valF;
        into.writeUnsignedByte(libsumo::POSITION_2D);
        into.writeDouble(valF);
        defFile >> valF;
        into.writeDouble(valF);
        return 1 + 8 + 8;
    } else if (dataTypeS == "<position3D>") {
        defFile >> valF;
        into.writeUnsignedByte(libsumo::POSITION_3D);
        into.writeDouble(valF);
        defFile >> valF;
        into.writeDouble(valF);
        defFile >> valF;
        into.writeDouble(valF);
        return 1 + 8 + 8 + 8;
    } else if (dataTypeS == "<positionRoadmap>") {
        std::string valueS;
        defFile >> valueS;
        into.writeUnsignedByte(libsumo::POSITION_ROADMAP);
        into.writeString(valueS);
        int length = 1 + 8 + (int) valueS.length();
        defFile >> valF;
        into.writeDouble(valF);
        defFile >> valI;
        into.writeUnsignedByte(valI);
        return length + 4 + 1;
    } else if (dataTypeS == "<shape>") {
        defFile >> valI;
        into.writeUnsignedByte(libsumo::TYPE_POLYGON);
        into.writeUnsignedByte(valI);
        int length = 1 + 1;
        for (int i = 0; i < valI; ++i) {
            double x, y;
            defFile >> x >> y;
            into.writeDouble(x);
            into.writeDouble(y);
            length += 8 + 8;
        }
        return length;
    }
    msg << "## Unknown data type: " << dataTypeS;
    return 0;
}


void
TraCITestClient::readAndReportTypeDependent(tcpip::Storage& inMsg, int valueDataType) {
    if (valueDataType == libsumo::TYPE_UBYTE) {
        int ubyte = inMsg.readUnsignedByte();
        answerLog << " Unsigned Byte Value: " << ubyte << std::endl;
    } else if (valueDataType == libsumo::TYPE_BYTE) {
        int byte = inMsg.readByte();
        answerLog << " Byte value: " << byte << std::endl;
    } else if (valueDataType == libsumo::TYPE_INTEGER) {
        int integer = inMsg.readInt();
        answerLog << " Int value: " << integer << std::endl;
    } else if (valueDataType == libsumo::TYPE_DOUBLE) {
        double doublev = inMsg.readDouble();
        answerLog << " Double value: " << doublev << std::endl;
    } else if (valueDataType == libsumo::TYPE_POLYGON) {
        int size = inMsg.readUnsignedByte();
        if (size == 0) {
            size = inMsg.readInt();
        }
        answerLog << " PolygonValue: ";
        for (int i = 0; i < size; i++) {
            double x = inMsg.readDouble();
            double y = inMsg.readDouble();
            answerLog << "(" << x << "," << y << ") ";
        }
        answerLog << std::endl;
    } else if (valueDataType == libsumo::POSITION_3D) {
        double x = inMsg.readDouble();
        double y = inMsg.readDouble();
        double z = inMsg.readDouble();
        answerLog << " Position3DValue: " << std::endl;
        answerLog << " x: " << x << " y: " << y
                  << " z: " << z << std::endl;
    } else if (valueDataType == libsumo::POSITION_ROADMAP) {
        std::string roadId = inMsg.readString();
        double pos = inMsg.readDouble();
        int laneId = inMsg.readUnsignedByte();
        answerLog << " RoadMapPositionValue: roadId=" << roadId
                  << " pos=" << pos
                  << " laneId=" << laneId << std::endl;
    } else if (valueDataType == libsumo::TYPE_STRING) {
        std::string s = inMsg.readString();
        answerLog << " string value: " << s << std::endl;
    } else if (valueDataType == libsumo::TYPE_STRINGLIST) {
        std::vector<std::string> s = inMsg.readStringList();
        answerLog << " string list value: [ " << std::endl;
        for (std::vector<std::string>::iterator i = s.begin(); i != s.end(); ++i) {
            if (i != s.begin()) {
                answerLog << ", ";
            }
            answerLog << '"' << *i << '"';
        }
        answerLog << " ]" << std::endl;
    } else if (valueDataType == libsumo::TYPE_COMPOUND) {
        int no = inMsg.readInt();
        answerLog << " compound value with " << no << " members: [ " << std::endl;
        for (int i = 0; i < no; ++i) {
            int currentValueDataType = inMsg.readUnsignedByte();
            answerLog << " valueDataType=" << currentValueDataType;
            readAndReportTypeDependent(inMsg, currentValueDataType);
        }
        answerLog << " ]" << std::endl;
    } else if (valueDataType == libsumo::POSITION_2D) {
        double xv = inMsg.readDouble();
        double yv = inMsg.readDouble();
        answerLog << " position value: (" << xv << "," << yv << ")" << std::endl;
    } else if (valueDataType == libsumo::TYPE_COLOR) {
        int r = inMsg.readUnsignedByte();
        int g = inMsg.readUnsignedByte();
        int b = inMsg.readUnsignedByte();
        int a = inMsg.readUnsignedByte();
        answerLog << " color value: (" << r << "," << g << "," << b << "," << a << ")" << std::endl;
    } else {
        answerLog << "#Error: unknown valueDataType!" << std::endl;
    }
}


void
TraCITestClient::testAPI() {
    answerLog << "testAPI:\n";
    const auto& version = getVersion();
    answerLog << "  getVersion: " << version.first << ", " << version.second << "\n";
    answerLog << "  setOrder:\n";
    setOrder(0);
    // edge
    answerLog << "  edge:\n";
    answerLog << "    getIDList: " << joinToString(edge.getIDList(), " ") << "\n";
    answerLog << "    getIDCount: " << edge.getIDCount() << "\n";
    const std::string edgeID = "e_m0";
    edge.adaptTraveltime(edgeID, 42, 0, 10);
    edge.setEffort(edgeID, 420, 0, 10);
    answerLog << "    currentTraveltime: " << edge.getTraveltime(edgeID) << "\n";
    answerLog << "    adaptedTravelTime: " << edge.getAdaptedTraveltime(edgeID, 0) << "\n";
    answerLog << "    effort: " << edge.getEffort(edgeID, 0) << "\n";
    answerLog << "    laneNumber: " << edge.getLaneNumber(edgeID) << "\n";
    answerLog << "    streetName: " << edge.getStreetName(edgeID) << "\n";
    edge.setMaxSpeed(edgeID, 42);
    answerLog << "    maxSpeed: " << lane.getMaxSpeed(edgeID + "_0") << "\n";

    // lane
    answerLog << "  lane:\n";
    answerLog << "    getIDList: " << joinToString(lane.getIDList(), " ") << "\n";
    answerLog << "    getIDCount: " << lane.getIDCount() << "\n";
    const std::string laneID = "e_m6_0";
    answerLog << "    getLinkNumber: " << lane.getLinkNumber(laneID) << "\n";
    std::vector<libsumo::TraCIConnection> connections = lane.getLinks(laneID);
    answerLog << "    getLinks:\n";
    for (int i = 0; i < (int)connections.size(); ++i) {
        const libsumo::TraCIConnection& c = connections[i];
        answerLog << "    approachedLane=" << c.approachedLane
                  << " hasPrio=" << c.hasPrio
                  << " isOpen=" << c.isOpen
                  << " hasFoe=" << c.hasFoe
                  << " approachedInternal=" << c.approachedInternal
                  << " state=" << c.state
                  << " direction=" << c.direction
                  << " length=" << c.length
                  << "\n";
    }
    answerLog << "    getFoes: " << joinToString(lane.getFoes("e_vu0_0", "e_m4_0"), " ") << "\n";
    try {
        answerLog << "    getFoes (invalid): ";
        answerLog << joinToString(lane.getFoes("e_vu0_0", "e_m4_1"), " ") << "\n";
    } catch (libsumo::TraCIException& e) {
        answerLog << "    caught TraCIException(" << e.what() << ")\n";
    }
    answerLog << "    getInternalFoes: " << joinToString(lane.getInternalFoes(":n_m4_2_0"), " ") << "\n";
    try {
        answerLog << "    getInternalFoes (invalid): ";
        answerLog << joinToString(lane.getInternalFoes("dummy"), " ") << "\n";
    } catch (libsumo::TraCIException& e) {
        answerLog << "    caught TraCIException(" << e.what() << ")\n";
    }
    lane.setMaxSpeed(laneID, 42);
    answerLog << "    maxSpeed: " << lane.getMaxSpeed(laneID) << "\n";
    // poi
    answerLog << "  POI:\n";
    answerLog << "    getIDList: " << joinToString(poi.getIDList(), " ") << "\n";
    answerLog << "    getIDCount: " << poi.getIDCount() << "\n";
    answerLog << "    getPosition: " << poi.getPosition("poi0").getString() << "\n";
    answerLog << "    getColor: " << poi.getColor("poi0").getString() << "\n";

    // poly
    answerLog << "  polygon:\n";
    answerLog << "    getIDList: " << joinToString(polygon.getIDList(), " ") << "\n";
    answerLog << "    getIDCount: " << polygon.getIDCount() << "\n";
    std::vector<libsumo::TraCIPosition> shape = polygon.getShape("poly0");
    std::string shapeStr;
    for (auto pos : shape) {
        shapeStr += pos.getString() + " ";
    }
    polygon.setLineWidth("poly0", 0.6);
    answerLog << "    getLineWidth: " << polygon.getLineWidth("poly0") << "\n";
    answerLog << "    getShape: " << shapeStr << "\n";
    answerLog << "    getColor: " << polygon.getColor("poly0").getString() << "\n";
    shape[0].x = 42;
    polygon.setShape("poly0", shape);
    std::string shapeStr2;
    for (auto pos : polygon.getShape("poly0")) {
        shapeStr2 += pos.getString() + " ";
    }
    answerLog << "    getShape after modification: " << shapeStr2 << "\n";

    // junction
    answerLog << "  junction:\n";
    answerLog << "    getIDList: " << joinToString(junction.getIDList(), " ") << "\n";
    answerLog << "    getIDCount: " << junction.getIDCount() << "\n";
    std::vector<libsumo::TraCIPosition> junctionShape = junction.getShape("n_m4");
    std::string junctionShapeStr;
    for (auto pos : junctionShape) {
        junctionShapeStr += pos.getString() + " ";
    }
    answerLog << "    getShape: " << junctionShapeStr << "\n";

    // route
    answerLog << "  route:\n";
    answerLog << "    add:\n";
    std::vector<std::string> edges;
    edges.push_back("e_u1");
    edges.push_back("e_u0");
    route.add("e_u1", edges);
    edges.clear();
    edges.push_back("e_m4");
    route.add("e_m4", edges);
    answerLog << "    getIDList: " << joinToString(route.getIDList(), " ") << "\n";

    // vehicletype
    answerLog << "  vehicleType:\n";
    answerLog << "    getIDList: " << joinToString(vehicletype.getIDList(), " ") << "\n";
    vehicletype.setEmergencyDecel("t1", 9.9);
    answerLog << "    getEmergencyDecel: " << vehicletype.getEmergencyDecel("t1") << "\n";
    vehicletype.setApparentDecel("t1", 99.9);
    answerLog << "    getApparentDecel: " << vehicletype.getApparentDecel("t1") << "\n";
    vehicletype.setWidth("t1", 1.9);
    answerLog << "    getWidth: " << vehicletype.getWidth("t1") << "\n";
    vehicletype.setHeight("t1", 1.8);
    answerLog << "    getHeight: " << vehicletype.getHeight("t1") << "\n";
    vehicletype.setMinGapLat("t1", 1.5);
    answerLog << "    setMinGapLat: " << vehicletype.getMinGapLat("t1") << "\n";
    vehicletype.setMaxSpeedLat("t1", 1.2);
    answerLog << "    setMaxSpeedLat: " << vehicletype.getMaxSpeedLat("t1") << "\n";
    vehicletype.setLateralAlignment("t1", "compact");
    answerLog << "    getLateralAlignment: " << vehicletype.getLateralAlignment("t1") << "\n";
    answerLog << "    getPersonCapacity: " << vehicletype.getPersonCapacity("t1") << "\n";
    answerLog << "    copy type 't1' to 't1_copy' and set accel to 100.\n";
    vehicletype.copy("t1", "t1_copy");
    answerLog << "    getIDList: " << joinToString(vehicletype.getIDList(), " ") << "\n";
    vehicletype.setAccel("t1_copy", 100.);
    answerLog << "    getAccel('t1'): " << vehicletype.getAccel("t1") << "\n";
    answerLog << "    getAccel('t1_copy'): " << vehicletype.getAccel("t1_copy") << "\n";

    // vehicle
    answerLog << "  vehicle:\n";
    vehicle.setLine("0", "S42");
    std::vector<std::string> via;
    via.push_back("e_shape1");
    vehicle.setVia("0", via);
    vehicle.setType("0", "t1_copy");
    answerLog << "    getTypeID: " << vehicle.getTypeID("0") << "\n";
    answerLog << "    getRoadID: " << vehicle.getRoadID("0") << "\n";
    answerLog << "    getRouteID: " << vehicle.getRouteID("0") << "\n";
    answerLog << "    getLaneID: " << vehicle.getLaneID("0") << "\n";
    answerLog << "    getLanePosition: " << vehicle.getLanePosition("0") << "\n";
    answerLog << "    getLateralLanePosition: " << vehicle.getLateralLanePosition("0") << "\n";
    answerLog << "    getSpeed: " << vehicle.getSpeed("0") << "\n";
    answerLog << "    getLateralSpeed: " << vehicle.getLateralSpeed("0") << "\n";
    answerLog << "    getAcceleration: " << vehicle.getAcceleration("0") << "\n";

    answerLog << "    getFollowSpeed: " << vehicle.getFollowSpeed("0", 10, 20, 9, 4.5) << "\n";
    answerLog << "    getSecureGap: " << vehicle.getSecureGap("0", 10, 9, 4.5) << "\n";
    answerLog << "    getStopSpeed: " << vehicle.getStopSpeed("0", 10, 20) << "\n";

    answerLog << "    getSpeedMode: " << vehicle.getSpeedMode("0") << "\n";
    answerLog << "    getSlope: " << vehicle.getSlope("0") << "\n";
    answerLog << "    getLine: " << vehicle.getLine("0") << "\n";
    answerLog << "    getVia: " << joinToString(vehicle.getVia("0"), ",") << "\n";
    answerLog << "    getPersonCapacity: " << vehicle.getPersonCapacity("0") << "\n";
    vehicle.setMaxSpeed("0", 30);
    answerLog << "    getMaxSpeed: " << vehicle.getMaxSpeed("0") << "\n";
    answerLog << "    isRouteValid: " << vehicle.isRouteValid("0") << "\n";
    answerLog << "    getStopState: " << vehicle.getStopState("0") << "\n";
    answerLog << "    getStopDelay: " << vehicle.getStopDelay("0") << "\n";
    vehicle.setParameter("0", "meaningOfLife", "42");
    answerLog << "    param: " << vehicle.getParameter("0", "meaningOfLife") << "\n";
    std::pair<std::string, std::string> paramTuple = vehicle.getParameterWithKey("0", "meaningOfLife");
    answerLog << "    parameterWithKey: (" << paramTuple.first << ", " << paramTuple.second << ")\n";
    libsumo::TraCIColor col1;
    col1.r = 255;
    col1.g = 255;
    col1.b = 0;
    col1.a = 128;
    vehicle.setColor("0", col1);
    libsumo::TraCIColor col2 = vehicle.getColor("0");
    answerLog << "    getColor:  r=" << (int)col2.r << " g=" << (int)col2.g << " b=" << (int)col2.b << " a=" << (int)col2.a << "\n";
    int signals = vehicle.getSignals("0");
    answerLog << "    getSignals: " << signals << "\n";
    vehicle.setSignals("0", signals ^ TraCIAPI::VehicleScope::SIGNAL_FOGLIGHT);
    vehicle.setRoutingMode("0", libsumo::ROUTING_MODE_AGGREGATED);
    answerLog << "    getRoutingMode: " << vehicle.getRoutingMode("0") << "\n";
    answerLog << "    getNextTLS:\n";
    std::vector<libsumo::TraCINextTLSData> result = vehicle.getNextTLS("0");
    for (int i = 0; i < (int)result.size(); ++i) {
        const libsumo::TraCINextTLSData& d = result[i];
        answerLog << "      tls=" << d.id << " tlIndex=" << d.tlIndex << " dist=" << d.dist << " state=" << d.state << "\n";
    }
    answerLog << "    moveToXY, simStep:\n";
    vehicle.moveToXY("0", "dummy", 0, 2231.61, 498.29, 90, 1);
    simulationStep();
    // simulationStep(1);
    answerLog << "    getRoadID: " << vehicle.getRoadID("0") << "\n";
    answerLog << "    getLaneID: " << vehicle.getLaneID("0") << "\n";
    vehicle.changeTarget("0", "e_o0");
    std::vector<std::string> edges2 = vehicle.getRoute("0");
    answerLog << "    edges: " << joinToString(edges2, " ") << "\n";
    vehicle.setRouteID("0", "e_m4");
    answerLog << "    edges: " << joinToString(vehicle.getRoute("0"), " ") << "\n";
    vehicle.setRoute("0", edges2);
    answerLog << "    edges: " << joinToString(vehicle.getRoute("0"), " ") << "\n";
    answerLog << "    add:\n";
    vehicle.add("1", "e_u1");
    vehicle.add("2", "e_u1");
    vehicle.moveTo("2", "e_u0_0", 5);
    simulationStep();
    answerLog << "    getIDList: " << joinToString(vehicle.getIDList(), " ") << "\n";
    answerLog << "    getWaitingTime: " << vehicle.getWaitingTime("0") << "\n";
    answerLog << "    getAccumulatedWaitingTime: " << vehicle.getAccumulatedWaitingTime("0") << "\n";
    vehicle.setShapeClass("0", "bicycle");
    answerLog << "    getShapeClass: " << vehicle.getShapeClass("0") << "\n";
    std::pair<std::string, double> leader = vehicle.getLeader("1", 1000);
    answerLog << "    getLeader: " << leader.first << ", " << leader.second << "\n";
    std::pair<std::string, double> follower = vehicle.getFollower("1", 1000);
    answerLog << "    getFollower: " << follower.first << ", " << follower.second << "\n";
    std::pair<int, int> state = vehicle.getLaneChangeState("1", 1);
    answerLog << "    getLaneChangeState (left): " << state.first << ", " << state.second << "\n";
    state = vehicle.getLaneChangeState("1", -1);
    answerLog << "    getLaneChangeState (right): " << state.first << ", " << state.second << "\n";
    vehicle.rerouteTraveltime("0");
    vehicle.setSpeedFactor("0", 0.8);
    vehicle.setSpeedMode("0", 0);
    answerLog << "    getSpeedMode after change: " << vehicle.getSpeedMode("0") << "\n";
    vehicle.setLaneChangeMode("0", 0);
    answerLog << "    getLaneChangeMode after change: " << vehicle.getLaneChangeMode("0") << "\n";
    answerLog << "    remove:\n";
    vehicle.remove("0");
    answerLog << "    getIDCount: " << vehicle.getIDCount() << "\n";

    // inductionLoop
    answerLog << "  inductionloop:\n";
    answerLog << "    getIDList: " << joinToString(inductionloop.getIDList(), " ") << "\n";
    answerLog << "    getVehicleData:\n";
    std::vector<libsumo::TraCIVehicleData> result2 = inductionloop.getVehicleData("det1");
    for (int i = 0; i < (int)result2.size(); ++i) {
        const libsumo::TraCIVehicleData& vd = result2[i];
        answerLog << "      veh=" << vd.id << " length=" << vd.length << " entered=" << vd.entryTime << " left=" << vd.leaveTime << " type=" << vd.typeID << "\n";
    }

    // simulation
    answerLog << "  simulation:\n";
    answerLog << "    convert2D: " << simulation.convert2D("e_m5", 0).getString() << "\n";
    answerLog << "    convert2DGeo: " << simulation.convert2D("e_m5", 0, 0, true).getString() << "\n";
    answerLog << "    convert3D: " << simulation.convert3D("e_m5", 0).getString() << "\n";
    answerLog << "    convert3DGeo: " << simulation.convert3D("e_m5", 0, 0, true).getString() << "\n";
    answerLog << "    convertRoad: " << simulation.convertRoad(2500, 500).getString() << "\n";
    answerLog << "    convertRoadBus: " << simulation.convertRoad(2500, 500, false, "bus").getString() << "\n";
    answerLog << "    convertGeo: " << simulation.convertGeo(2500, 500).getString() << "\n";
    answerLog << "    convertCartesian: " << simulation.convertGeo(12, 52, true).getString() << "\n";
    answerLog << "    getDistance2D_air: " << simulation.getDistance2D(2500, 500, 2000, 500, false, false) << "\n";
    answerLog << "    getDistance2D_driving: " << simulation.getDistance2D(2500, 500, 2000, 500, false, true) << "\n";
    answerLog << "    getDistanceRoad_air: " << simulation.getDistanceRoad("e_m5", 0, "e_m4", 0, false) << "\n";
    answerLog << "    getDistanceRoad_driving: " << simulation.getDistanceRoad("e_m5", 0, "e_m4", 0, true) << "\n";
    answerLog << "    getCurrentTime: " << simulation.getCurrentTime() << "\n";
    answerLog << "    getDeltaT: " << simulation.getDeltaT() << "\n";
    answerLog << "    parkingArea param: " << simulation.getParameter("park1", "parkingArea.capacity") << "\n";
    answerLog << "    busStopWaiting: " << simulation.getBusStopWaiting("bs1") << "\n";
    answerLog << "    busStopWaitingIDs: " << joinToString(simulation.getBusStopWaitingIDList("bs1"), " ") << "\n";
    simulation.writeMessage("custom message test");
    answerLog << "    subscribe to road and pos of vehicle '1':\n";
    answerLog << "    findRoute: " << joinToString(simulation.findRoute("e_m5", "e_m4").edges, " ") << "\n";
    std::vector<int> vars;
    vars.push_back(libsumo::VAR_ROAD_ID);
    vars.push_back(libsumo::VAR_LANEPOSITION);
    vehicle.subscribe("1", vars, 0, 100);
    simulationStep();
    answerLog << "    subscription results:\n";
    libsumo::TraCIResults result3 = vehicle.getSubscriptionResults("1");
    answerLog << "      roadID=" << result3[libsumo::VAR_ROAD_ID]->getString() << " pos=" << result3[libsumo::VAR_LANEPOSITION]->getString() << "\n";

    answerLog << "    subscribe to vehicles around edge 'e_u1':\n";
    std::vector<int> vars2;
    vars2.push_back(libsumo::VAR_LANEPOSITION);
    edge.subscribeContext("e_u1", libsumo::CMD_GET_VEHICLE_VARIABLE, 100, vars2, 0, 100);
    simulationStep();
    answerLog << "    context subscription results:\n";
    libsumo::SubscriptionResults result4 = edge.getContextSubscriptionResults("e_u1");
    for (libsumo::SubscriptionResults::iterator it = result4.begin(); it != result4.end(); ++it) {
        answerLog << "      vehicle=" << it->first << " pos=" << it->second[libsumo::VAR_LANEPOSITION]->getString() << "\n";
    }

    answerLog << "    subscribe to vehicles around vehicle '1':\n";
    std::vector<int> vars3;
    vars3.push_back(libsumo::VAR_SPEED);
    vehicle.subscribeContext("1", libsumo::CMD_GET_VEHICLE_VARIABLE, 1000, vars3, 0, 100);
    vehicle.addSubscriptionFilterLanes(std::vector<int>({0, 1, 2}));
    vehicle.addSubscriptionFilterNoOpposite();
    vehicle.addSubscriptionFilterDownstreamDistance(1000);
    vehicle.addSubscriptionFilterUpstreamDistance(1000);
    vehicle.addSubscriptionFilterCFManeuver();
    vehicle.addSubscriptionFilterLeadFollow(std::vector<int>({0, 1, 2}));
    vehicle.addSubscriptionFilterTurn();
    vehicle.addSubscriptionFilterVClass(std::vector<std::string>({"passenger"}));
    vehicle.addSubscriptionFilterVType(std::vector<std::string>({"passenger"}));
    vehicle.addSubscriptionFilterLCManeuver(1);

    vehicle.subscribeContext("3", libsumo::CMD_GET_VEHICLE_VARIABLE, 200, vars3, 0, 100);
    vehicle.addSubscriptionFilterFieldOfVision(90);

    vehicle.subscribeContext("4", libsumo::CMD_GET_VEHICLE_VARIABLE, 200, vars3, 0, 100);
    vehicle.addSubscriptionFilterLateralDistance(50);
    //

    simulationStep();
    answerLog << "    context subscription results:\n";
    libsumo::SubscriptionResults result5 = vehicle.getContextSubscriptionResults("1");
    for (auto item : result5) {
        answerLog << "      vehicle=" << item.first << "\n";
    }

    // person
    answerLog << "  person:\n";
    person.setWidth("p0", 1);
    person.setMinGap("p0", 2);
    person.setLength("p0", 3);
    person.setHeight("p0", 4);
    person.setColor("p0", col1);
    person.setType("p0", "stilts");
    answerLog << "    getIDList: " << joinToString(person.getIDList(), " ") << "\n";
    answerLog << "    getRoadID: " << person.getRoadID("p0") << "\n";
    answerLog << "    getLaneID: " << person.getLaneID("p0") << "\n";
    answerLog << "    getTypeID: " << person.getTypeID("p0") << "\n";
    answerLog << "    getWaitingTime: " << person.getWaitingTime("p0") << "\n";
    answerLog << "    getNextEdge: " << person.getNextEdge("p0") << "\n";
    answerLog << "    getStage: " << person.getStage("p0").description << "\n";
    answerLog << "    getRemainingStages: " << person.getRemainingStages("p0") << "\n";
    answerLog << "    getVehicle: " << person.getVehicle("p0") << "\n";
    answerLog << "    getEdges: " << joinToString(person.getEdges("p0"), " ") << "\n";
    answerLog << "    getPosition: " << person.getPosition("p0").getString() << "\n";
    answerLog << "    getPosition3D: " << person.getPosition3D("p0").getString() << "\n";
    answerLog << "    getAngle: " << person.getAngle("p0") << "\n";
    answerLog << "    getSlope: " << person.getSlope("p0") << "\n";
    answerLog << "    getLanePosition: " << person.getLanePosition("p0") << "\n";
    answerLog << "    getLength: " << person.getLength("p0") << "\n";
    answerLog << "    getColor: " << person.getColor("p0").getString() << "\n";
    person.setParameter("p0", "foo", "bar");
    answerLog << "    param: " << person.getParameter("p0", "foo") << "\n";
    person.setSpeed("p0", 3);
    simulationStep();
    answerLog << "    getSpeed: " << person.getSpeed("p0") << "\n";
    person.add("p1", "e_u1", 10);
    std::vector<std::string> walkEdges;
    walkEdges.push_back("e_u1");
    walkEdges.push_back("e_shape1");
    person.appendWalkingStage("p1", walkEdges, -20);
    person.appendWaitingStage("p1", 5);
    person.appendDrivingStage("p1", "e_vu2", "BusLine42");
    libsumo::TraCIStage stage(libsumo::STAGE_WALKING);
    stage.edges.push_back("e_vu2");
    stage.edges.push_back("e_vo2");
    stage.arrivalPos = -10;
    person.appendStage("p1", stage);
    simulationStep();
    // expect 5 stages due to the initial waiting-for-departure stage
    answerLog << "    getRemainingStages: " << person.getRemainingStages("p1") << "\n";
    person.removeStage("p1", 3);
    answerLog << "    getRemainingStages: " << person.getRemainingStages("p1") << "\n";
    person.removeStages("p1");
    answerLog << "    getRemainingStages: " << person.getRemainingStages("p1") << "\n";
    answerLog << "    getStage: " << person.getStage("p1").description << "\n";
    walkEdges.push_back("e_m5");
    person.appendWalkingStage("p1", walkEdges, -20);
    simulationStep();
    person.rerouteTraveltime("p1");
    answerLog << "    getEdges after rerouting: " << joinToString(person.getEdges("p1"), " ") << "\n";

    // trafficlights
    answerLog << "  trafficlights:\n";
    trafficlights.setPhase("n_m4", 0);
    trafficlights.setPhaseName("n_m4", "nameSetByTraCI");
    answerLog << "    getIDList: " << joinToString(trafficlights.getIDList(), " ") << "\n";
    answerLog << "    getIDCount: " << trafficlights.getIDCount() << "\n";
    answerLog << "    state: " << trafficlights.getRedYellowGreenState("n_m4") << "\n";
    answerLog << "    program: " << trafficlights.getProgram("n_m4") << "\n";
    answerLog << "    phase: " << trafficlights.getPhase("n_m4") << "\n";
    answerLog << "    phaseName: " << trafficlights.getPhaseName("n_m4") << "\n";
    answerLog << "    phaseDuration: " << trafficlights.getPhaseDuration("n_m4") << "\n";
    answerLog << "    nextSwitch: " << trafficlights.getNextSwitch("n_m4") << "\n";
    answerLog << "    controlledLanes: " << joinToString(trafficlights.getControlledLanes("n_m4"), " ") << "\n";
    std::vector<std::vector<libsumo::TraCILink> > links = trafficlights.getControlledLinks("n_m4");
    answerLog << "    controlledLinks:\n";
    for (int i = 0; i < (int)links.size(); ++i) {
        for (int j = 0; j < (int)links[i].size(); ++j) {
            answerLog << "      index=" << i << " link=" << j << " fromLane=" << links[i][j].fromLane << " viaLane=" << links[i][j].viaLane << " toLane=" << links[i][j].toLane << "\n";
        }
    }
    libsumo::TraCILogic logic("custom", 0, 3);
    logic.phases = std::vector<libsumo::TraCIPhase*>({ new libsumo::TraCIPhase(5, "rrrrrrr", 5, 5),
                     new libsumo::TraCIPhase(10, "ggggggg", 5, 15),
                     new libsumo::TraCIPhase(3, "GGGGGGG", 3, 3),
                     new libsumo::TraCIPhase(3, "yyyyyyy", 3, 3)
    });
    trafficlights.setProgramLogic("n_m4", logic);

    std::vector<libsumo::TraCILogic> logics = trafficlights.getAllProgramLogics("n_m4");
    answerLog << "    completeDefinition:\n";
    for (int i = 0; i < (int)logics.size(); ++i) {
        answerLog << "      subID=" << logics[i].programID << " type=" << logics[i].type << " phase=" << logics[i].currentPhaseIndex << "\n";
        answerLog << "      params=" << joinToString(logics[i].subParameter) << "\n";
        for (int j = 0; j < (int)logics[i].phases.size(); ++j) {
            answerLog << "         phase=" << logics[i].phases[j]->state
                      << " dur=" << logics[i].phases[j]->duration
                      << " minDur=" << logics[i].phases[j]->minDur
                      << " maxDur=" << logics[i].phases[j]->maxDur
                      << "\n";
        }
    }
    simulationStep();
    answerLog << "    state=" << trafficlights.getRedYellowGreenState("n_m4") << "\n";
    trafficlights.setRedYellowGreenState("n_m4", "gGyruoO");
    answerLog << "    stateSet=" << trafficlights.getRedYellowGreenState("n_m4") << "\n";
    answerLog << "    program: " << trafficlights.getProgram("n_m4") << "\n";

    answerLog << "  load:\n";
    std::vector<std::string> args;
    args.push_back("-n");
    args.push_back("net.net.xml");
    args.push_back("-r");
    args.push_back("input_routes.rou.xml");
    args.push_back("-a");
    args.push_back("input_additional.add.xml");
    args.push_back("--no-step-log");
    load(args);
    simulationStep();
    answerLog << "    getCurrentTime: " << simulation.getCurrentTime() << "\n";
    vehicle.subscribe("0", vars, 0, 100);
    edge.subscribeContext("e_u1", libsumo::CMD_GET_VEHICLE_VARIABLE, 100, vars2, 0, 100);

    answerLog << "  gui:\n";
    try {
        answerLog << "    setScheme: \n";
        gui.setSchema("View #0", "real world");
        answerLog << "    getScheme: " << gui.getSchema("View #0") << "\n";
        gui.setZoom("View #0", 50);
        answerLog << "    getZoom: " << gui.getZoom() << "\n";
        answerLog << "    take screenshot: \n";
        gui.screenshot("View #0", "image.png", 500, 500);
    } catch (libsumo::TraCIException&) {
        answerLog << "    no support for gui commands\n";
    }
}
