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
/// @file    ResponseContainer.java
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2011
///
//
/****************************************************************************/
package it.polito.appeal.traci.protocol;

import java.util.List;

/**
 * Represents all the information related to a given request. It contains a
 * mandatory status response, an optional response command and an optional list
 * of sub-responses.
 *
 * @author Enrico Gueli &lt;enrico.gueli@polito.it&gt;
 *
 */
public class ResponseContainer {
    private final StatusResponse status;
    private final Command response;
    private final List<Command> subResponses;

    /**
     * Constructor for a response container with status response, response
     * command and zero or more sub-responses.
     *
     * @param status status
     * @param response response
     * @param subResponses sub response
     */
    public ResponseContainer(StatusResponse status, Command response,
                             List<Command> subResponses) {
        this.status = status;
        this.response = response;
        this.subResponses = subResponses;
    }

    /**
     * Constructor for a response container with status response and a response
     * command.
     *
     * @param status status
     * @param response response
     */
    public ResponseContainer(StatusResponse status, Command response) {
        this(status, response, null);
    }

    /**
     * Returns the status response
     * @return the status
     */
    public StatusResponse getStatus() {
        return status;
    }

    /**
     * @return the response command. It may return <code>null</code> if there is
     *         no response to such command (i.e. a command that changes
     *         something in the simulation state or the close command)
     */
    public Command getResponse() {
        return response;
    }

    /**
     * @return the sub-responses of this response container. It may return
     *         <code>null</code> if there are no sub-responses to such command
     *         (i.e. commands different than CMD_SIMSTEP2)
     */
    public List<Command> getSubResponses() {
        return subResponses;
    }


}
