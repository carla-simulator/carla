//
// Copyright (C) 2006-2016 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#define WANT_WINSOCK2
#include <platdep/sockets.h>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <sstream>
#include <iostream>
#include <fstream>

#include "veins/modules/mobility/traci/TraCIScenarioManagerForker.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCILauncher.h"

using veins::TraCILauncher;
using veins::TraCIScenarioManagerForker;

Define_Module(veins::TraCIScenarioManagerForker);

namespace {

template <typename T>
inline std::string replace(std::string haystack, std::string needle, T newValue)
{
    size_t i = haystack.find(needle, 0);
    if (i == std::string::npos) return haystack;
    std::ostringstream os;
    os << newValue;
    haystack.replace(i, needle.length(), os.str());
    return haystack;
}

} // namespace

TraCIScenarioManagerForker::TraCIScenarioManagerForker()
{
    server = nullptr;
}

TraCIScenarioManagerForker::~TraCIScenarioManagerForker()
{
    killServer();
}

void TraCIScenarioManagerForker::initialize(int stage)
{
    if (stage == 1) {
        commandLine = par("commandLine").stringValue();
        command = par("command").stringValue();
        configFile = par("configFile").stringValue();
        seed = par("seed");
        killServer();
    }
    TraCIScenarioManager::initialize(stage);
    if (stage == 1) {
        startServer();
    }
}

void TraCIScenarioManagerForker::finish()
{
    TraCIScenarioManager::finish();
    killServer();
}

void TraCIScenarioManagerForker::startServer()
{
    // autoset seed, if requested
    if (seed == -1) {
        const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
        seed = atoi(seed_s);
    }

    // assemble commandLine
    commandLine = replace(commandLine, "$command", command);
    commandLine = replace(commandLine, "$configFile", configFile);
    commandLine = replace(commandLine, "$seed", seed);
    commandLine = replace(commandLine, "$port", port);

    server = new TraCILauncher(commandLine);
}

void TraCIScenarioManagerForker::killServer()
{
    if (server) {
        delete server;
        server = nullptr;
    }
}

int TraCIScenarioManagerForker::getPortNumber() const
{
    int port = TraCIScenarioManager::getPortNumber();
    if (port != -1) {
        return port;
    }

    // find a free port for the forker if port is still -1

    if (initsocketlibonce() != 0) throw cRuntimeError("Could not init socketlib");

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw cRuntimeError("Failed to create socket: %s", strerror(errno));
    }

    struct sockaddr_in serv_addr;
    struct sockaddr* serv_addr_p = (struct sockaddr*) &serv_addr;
    memset(serv_addr_p, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = 0;

    if (::bind(sock, serv_addr_p, sizeof(serv_addr)) < 0) {
        throw cRuntimeError("Failed to bind socket: %s", strerror(errno));
    }

    socklen_t len = sizeof(serv_addr);
    if (getsockname(sock, serv_addr_p, &len) < 0) {
        throw cRuntimeError("Failed to get hostname: %s", strerror(errno));
    }

    port = ntohs(serv_addr.sin_port);

    closesocket(sock);
    return port;
}
