//
// Copyright (C) 2006 Christoph Sommer <sommer@ccs-labs.org>
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

#include <algorithm>
#include <functional>

#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"

using namespace veins::TraCIConstants;

namespace veins {

struct traci2omnet_functor : public std::unary_function<TraCICoord, Coord> {
    traci2omnet_functor(const TraCIConnection& owner)
        : owner(owner)
    {
    }

    Coord operator()(const TraCICoord& coord) const
    {
        return owner.traci2omnet(coord);
    }

    const TraCIConnection& owner;
};

SOCKET socket(void* ptr)
{
    ASSERT(ptr);
    return *static_cast<SOCKET*>(ptr);
}

TraCIConnection::Result::Result()
    : success(false)
    , not_impl(false)
    , message()
{
}

TraCIConnection::Result::Result(bool success, bool not_impl, std::string message)
    : success(success)
    , not_impl(false)
    , message(message)
{
}

TraCIConnection::TraCIConnection(cComponent* owner, void* ptr)
    : HasLogProxy(owner)
    , socketPtr(ptr)
{
    ASSERT(socketPtr);
}

TraCIConnection::~TraCIConnection()
{
    if (socketPtr) {
        closesocket(socket(socketPtr));
        delete static_cast<SOCKET*>(socketPtr);
    }
}

TraCIConnection* TraCIConnection::connect(cComponent* owner, const char* host, int port)
{
    EV_STATICCONTEXT;
    EV_INFO << "TraCIScenarioManager connecting to TraCI server" << endl;

    if (initsocketlibonce() != 0) throw cRuntimeError("Could not init socketlib");

    in_addr addr;
    struct hostent* host_ent;
    struct in_addr saddr;

    saddr.s_addr = inet_addr(host);
    if (saddr.s_addr != static_cast<unsigned int>(-1)) {
        addr = saddr;
    }
    else if ((host_ent = gethostbyname(host))) {
        addr = *((struct in_addr*) host_ent->h_addr_list[0]);
    }
    else {
        throw cRuntimeError("Invalid TraCI server address: %s", host);
        return nullptr;
    }

    sockaddr_in address;
    sockaddr* address_p = (sockaddr*) &address;
    memset(address_p, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = addr.s_addr;

    SOCKET* socketPtr = new SOCKET();
    if (*socketPtr < 0) throw cRuntimeError("Could not create socket to connect to TraCI server");

    for (int tries = 1; tries <= 10; ++tries) {
        *socketPtr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (::connect(*socketPtr, address_p, sizeof(address)) >= 0) break;
        closesocket(socket(socketPtr));

        std::stringstream ss;
        ss << "Could not connect to TraCI server; error message: " << sock_errno() << ": " << strerror(sock_errno());
        std::string msg = ss.str();

        int sleepDuration = tries * .25 + 1;

        if (tries >= 10) {
            throw cRuntimeError(msg.c_str());
        }
        else if (tries == 3) {
            EV_WARN << msg << " -- Will retry in " << sleepDuration << " second(s)." << std::endl;
        }

        sleep(sleepDuration);
    }

    {
        int x = 1;
        ::setsockopt(*socketPtr, IPPROTO_TCP, TCP_NODELAY, (const char*) &x, sizeof(x));
    }

    return new TraCIConnection(owner, socketPtr);
}

TraCIBuffer TraCIConnection::query(uint8_t commandId, const TraCIBuffer& buf, Result* result)
{
    sendMessage(makeTraCICommand(commandId, buf));

    TraCIBuffer obuf(receiveMessage());
    uint8_t cmdLength;
    obuf >> cmdLength;
    uint8_t commandResp;
    obuf >> commandResp;
    ASSERT(commandResp == commandId);
    uint8_t resultCode;
    obuf >> resultCode;
    std::string description;
    obuf >> description;
    if (result != nullptr) {
        result->success = (resultCode == RTYPE_OK);
        result->not_impl = (resultCode == RTYPE_NOTIMPLEMENTED);
        result->message = description;
    }
    else {
        if (resultCode == RTYPE_NOTIMPLEMENTED) throw cRuntimeError("TraCI server reported command 0x%2x not implemented (\"%s\"). Might need newer version.", commandId, description.c_str());
        if (resultCode != RTYPE_OK) throw cRuntimeError("TraCI server reported status %d executing command 0x%2x (\"%s\").", (int) resultCode, commandId, description.c_str());
    }
    return obuf;
}

std::string TraCIConnection::receiveMessage()
{
    if (!socketPtr) throw cRuntimeError("Not connected to TraCI server");

    uint32_t msgLength;
    {
        char buf2[sizeof(uint32_t)];
        uint32_t bytesRead = 0;
        while (bytesRead < sizeof(uint32_t)) {
            int receivedBytes = ::recv(socket(socketPtr), reinterpret_cast<char*>(&buf2) + bytesRead, sizeof(uint32_t) - bytesRead, 0);
            if (receivedBytes > 0) {
                bytesRead += receivedBytes;
            }
            else if (receivedBytes == 0) {
                throw cRuntimeError("Connection to TraCI server closed unexpectedly. Check your server's log");
            }
            else {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
        TraCIBuffer(std::string(buf2, sizeof(uint32_t))) >> msgLength;
    }

    uint32_t bufLength = msgLength - sizeof(msgLength);
    char buf[bufLength];
    {
        EV_TRACE << "Reading TraCI message of " << bufLength << " bytes" << endl;
        uint32_t bytesRead = 0;
        while (bytesRead < bufLength) {
            int receivedBytes = ::recv(socket(socketPtr), reinterpret_cast<char*>(&buf) + bytesRead, bufLength - bytesRead, 0);
            if (receivedBytes > 0) {
                bytesRead += receivedBytes;
            }
            else if (receivedBytes == 0) {
                throw cRuntimeError("Connection to TraCI server closed unexpectedly. Check your server's log");
            }
            else {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
    }
    return std::string(buf, bufLength);
}

void TraCIConnection::sendMessage(std::string buf)
{
    if (!socketPtr) throw cRuntimeError("Not connected to TraCI server");

    {
        uint32_t msgLength = sizeof(uint32_t) + buf.length();
        TraCIBuffer buf2 = TraCIBuffer();
        buf2 << msgLength;
        uint32_t bytesWritten = 0;
        while (bytesWritten < sizeof(uint32_t)) {
            ssize_t sentBytes = ::send(socket(socketPtr), buf2.str().c_str() + bytesWritten, sizeof(uint32_t) - bytesWritten, 0);
            if (sentBytes > 0) {
                bytesWritten += sentBytes;
            }
            else {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
    }

    {
        EV_TRACE << "Writing TraCI message of " << buf.length() << " bytes" << endl;
        uint32_t bytesWritten = 0;
        while (bytesWritten < buf.length()) {
            ssize_t sentBytes = ::send(socket(socketPtr), buf.c_str() + bytesWritten, buf.length() - bytesWritten, 0);
            if (sentBytes > 0) {
                bytesWritten += sentBytes;
            }
            else {
                if (sock_errno() == EINTR) continue;
                if (sock_errno() == EAGAIN) continue;
                throw cRuntimeError("Connection to TraCI server lost. Check your server's log. Error message: %d: %s", sock_errno(), strerror(sock_errno()));
            }
        }
    }
}

std::string makeTraCICommand(uint8_t commandId, const TraCIBuffer& buf)
{
    if (sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length() > 0xFF) {
        uint32_t len = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint8_t) + buf.str().length();
        return (TraCIBuffer() << static_cast<uint8_t>(0) << len << commandId).str() + buf.str();
    }
    uint8_t len = sizeof(uint8_t) + sizeof(uint8_t) + buf.str().length();
    return (TraCIBuffer() << len << commandId).str() + buf.str();
}

void TraCIConnection::setNetbounds(TraCICoord netbounds1, TraCICoord netbounds2, int margin)
{
    coordinateTransformation.reset(new TraCICoordinateTransformation(netbounds1, netbounds2, margin));
}

Coord TraCIConnection::traci2omnet(TraCICoord coord) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->traci2omnet(coord);
}

std::list<Coord> TraCIConnection::traci2omnet(const std::list<TraCICoord>& list) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->traci2omnet(list);
}

TraCICoord TraCIConnection::omnet2traci(Coord coord) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->omnet2traci(coord);
}

std::list<TraCICoord> TraCIConnection::omnet2traci(const std::list<Coord>& list) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->omnet2traci(list);
}

Heading TraCIConnection::traci2omnetHeading(double heading) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->traci2omnetHeading(heading);
}

double TraCIConnection::omnet2traciHeading(Heading heading) const
{
    ASSERT(coordinateTransformation.get());
    return coordinateTransformation->omnet2traciHeading(heading);
}

} // namespace veins
