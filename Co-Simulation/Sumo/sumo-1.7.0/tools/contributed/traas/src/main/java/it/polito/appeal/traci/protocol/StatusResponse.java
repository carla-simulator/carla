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
/// @file    StatusResponse.java
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2011
///
//
/****************************************************************************/
package it.polito.appeal.traci.protocol;

import java.io.IOException;

import de.tudresden.sumo.config.Constants;
import de.uniluebeck.itm.tcpip.Storage;

public class StatusResponse {
    private final int id;
    private final int result;
    private final String description;

    public StatusResponse(int id) {
        this(id, Constants.RTYPE_OK, "");
    }

    public StatusResponse(int id, int result, String description) {
        this.id = id;
        this.result = result;
        this.description = description;
    }

    public StatusResponse(Storage packet) throws IOException {
        int len = packet.readByte();
        if (len == 0) {
            packet.readInt();    // length is ignored; we can derive it
        }

        id = packet.readUnsignedByte();
        result = packet.readUnsignedByte();
        description = packet.readStringASCII();
    }

    public int id() {
        return id;
    }

    /**
     * @return the result
     */
    public int result() {
        return result;
    }

    /**
     * @return the description
     */
    public String description() {
        return description;
    }

    public void writeTo(Storage out) throws IOException {
        out.writeByte(0);
        out.writeInt(5 + 1 + 1 + 4 + description.length());
        out.writeByte(id);
        out.writeByte(result);
        out.writeStringASCII(description);
    }
}
