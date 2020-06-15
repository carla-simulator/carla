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
/// @file    OutputDevice_Network.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Felix Brack
/// @date    2006
///
// An output device for TCP/IP Network connections
/****************************************************************************/
#pragma once
#include <config.h>

#include "foreign/tcpip/socket.h"
#include "foreign/tcpip/storage.h"
#include "OutputDevice.h"
#include <utils/common/UtilExceptions.h>
#include <string>
#include <iostream>
#include <sstream>


// ==========================================================================
// class definitions
// ==========================================================================
/**
 * @class OutputDevice_Network
 * @brief An output device for TCP/IP network connections
 *
 * The implementation uses a portable socket implementation from the Shawn
 *  project (shawn.sf.net) located in src/foreign/tcpip/socket.h. It uses
 *  an internal storage for the messages, which is sent via the socket when
 *  "postWriteHook" is called.
 * @see postWriteHook
 */
class OutputDevice_Network : public OutputDevice {
public:
    /** @brief Constructor
     *
     * @param[in] host The host to connect
     * @param[in] port The port to connect
     * @exception IOError If the connection could not be established
     */
    OutputDevice_Network(const std::string& host,
                         const int port);


    /// @brief Destructor
    ~OutputDevice_Network();


protected:
    /// @name Methods that override/implement OutputDevice-methods
    /// @{

    /** @brief Returns the associated ostream
     *
     * The stream is an ostringstream, actually, into which the message
     *  is written. It is sent when postWriteHook is called.
     *
     * @return The used stream
     * @see postWriteHook
     */
    std::ostream& getOStream();


    /** @brief Sends the data which was written to the string stream over the socket.
     *
     * Converts the stored message into a vector of chars and sends them via to
     *  the socket implementation. Resets the message, afterwards.
     */
    virtual void postWriteHook();
    /// @}

private:
    /// @brief packet buffer
    std::ostringstream myMessage;

    /// @brief the socket to transfer the data
    tcpip::Socket* mySocket;

};
