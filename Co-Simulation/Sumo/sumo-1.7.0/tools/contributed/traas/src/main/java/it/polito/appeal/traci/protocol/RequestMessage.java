/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraCI4J module
// Copyright (C) 2011 ApPeAL Group, Politecnico di Torino
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
/// @file    RequestMessage.java
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2011
///
//
/****************************************************************************/
package it.polito.appeal.traci.protocol;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.zip.Checksum;


import de.uniluebeck.itm.tcpip.Storage;

/**
 * Represents a TraCI messages used to send {@link Command}s from the client to
 * server (i.e. requests). After construction, the commands are appended one
 * after one with {@link #append(Command)}. The whole message, along with its
 * length header, can be then retrieved in a serialized form via the
 * {@link #writeTo(DataOutputStream)} method.
 *
 * @author Enrico Gueli &lt;enrico.gueli@polito.it&gt;
 * @see <a href="https://sumo.dlr.de/docs/TraCI/Protocol.html#messages">TraCI/Protocol#Messages</a>
 */
public class RequestMessage {



    private final List<Command> commands = new ArrayList<Command>();

    /**
     * Adds a command to the tail of this message.
     * @param c c
     * @throws NullPointerException if the command is <code>null</code>.
     */
    public void append(Command c) {
        if (c == null) {
            throw new NullPointerException("the command can't be null");
        }
        commands.add(c);
    }

    /**
     * Writes the commands to the specified {@link DataOutputStream} object, in
     * the same order as the calls of {@link #append(Command)}.
     * @param dos data output stream
     * @throws IOException Exception
     */
    public void writeTo(DataOutputStream dos) throws IOException {
        int totalLen = Integer.SIZE / 8; // the length header

        for (Command cmd : commands) {
            totalLen += cmd.rawSize();
        }

        Checksum checksum = null;
        dos.writeInt(totalLen);


        for (Command cmd : commands) {
            Storage s = new Storage();
            cmd.writeRawTo(s);
            writeStorage(s, dos, checksum);
        }

    }

    /**
     * @param storage storage
     * @param os os
     * @param checksum checksum
     * @throws IOException Exception
     */
    private void writeStorage(Storage storage, OutputStream os, Checksum checksum)
    throws IOException {

        byte[] buf = new byte[storage.getStorageList().size()];
        //StringBuilder sb = new StringBuilder();
        int n = 0;
        for (Byte b : storage.getStorageList()) {
            if (checksum != null) {
                checksum.update(b);
            }

            //sb.append(String.format("%X ", b));
            buf[n] = b;
            n++;
        }
        //System.out.println("writeStorage " + storage.getStorageList().size() + " bytes:" + sb);
        os.write(buf);
    }

    public List<Command> commands() {
        return Collections.unmodifiableList(commands);
    }
}
