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
/// @file    CloseQuery.java
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2011
///
//
/****************************************************************************/
package it.polito.appeal.traci;

import it.polito.appeal.traci.protocol.Command;

import java.io.IOException;
import java.net.Socket;

import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.util.Query;


public class CloseQuery extends Query {

    public CloseQuery(Socket sock) throws IOException {
        super(sock);
    }

    public void doCommand() throws IOException {
        Command req = new Command(Constants.CMD_CLOSE);
        queryAndVerifySingle(req);
    }
}
