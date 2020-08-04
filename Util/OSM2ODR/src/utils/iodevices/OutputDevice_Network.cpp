/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2006-2020 German Aerospace Center (DLR) and others.
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
/// @file    OutputDevice_Network.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Felix Brack
/// @date    2006
///
// An output device for TCP/IP Network connections
/****************************************************************************/


// ==========================================================================
// included modules
// ==========================================================================
#include <config.h>

#include <thread>
#include <chrono>
#include <vector>
#include "OutputDevice_Network.h"
#include "foreign/tcpip/socket.h"
#include "utils/common/ToString.h"


// ==========================================================================
// method definitions
// ==========================================================================
OutputDevice_Network::OutputDevice_Network(const std::string& host,
        const int port) {
    mySocket = new tcpip::Socket(host, port);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // do not warn about constant conditional expression
#endif
    for (int wait = 1000; true; wait += 1000) {
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        try {
            mySocket->connect();
            break;
        } catch (tcpip::SocketException& e) {
            if (wait == 9000) {
                throw IOError(toString(e.what()) + " (host: " + host + ", port: " + toString(port) + ")");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(wait));
        }
    }
    myFilename = host + ":" + toString(port);
}


OutputDevice_Network::~OutputDevice_Network() {
    mySocket->close();
    delete mySocket;
}


std::ostream&
OutputDevice_Network::getOStream() {
    return myMessage;
}


void
OutputDevice_Network::postWriteHook() {
    std::string toSend = myMessage.str();
    std::vector<unsigned char> msg;
    msg.insert(msg.end(), toSend.begin(), toSend.end());
    try {
        mySocket->send(msg);
    } catch (tcpip::SocketException& e) {
        throw IOError(toString(e.what()));
    }
    myMessage.str("");
}


/****************************************************************************/
