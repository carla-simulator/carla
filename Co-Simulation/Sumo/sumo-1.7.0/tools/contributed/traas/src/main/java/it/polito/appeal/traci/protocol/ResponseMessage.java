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
/// @file    ResponseMessage.java
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2011
///
//
/****************************************************************************/
package it.polito.appeal.traci.protocol;

import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import de.tudresden.sumo.config.Constants;
import de.uniluebeck.itm.tcpip.Storage;

/**
 * Represents a message sent from the server. A response message is made of an
 * ordered collection of status responses, response commands and/or sub-responses.
 * @author Enrico Gueli &lt;enrico.gueli@polito.it&gt;
 *
 */
public class ResponseMessage {

    /**
     * The list of IDs of responses that have no response commands.
     */
    public static final int[] STATUS_ONLY_RESPONSES = new int[] {
        Constants.CMD_CLOSE,
        Constants.CMD_SETORDER,
        Constants.CMD_SET_EDGE_VARIABLE,
        Constants.CMD_SET_GUI_VARIABLE,
        Constants.CMD_SET_LANE_VARIABLE,
        Constants.CMD_SET_PERSON_VARIABLE,
        Constants.CMD_SET_POI_VARIABLE,
        Constants.CMD_SET_POLYGON_VARIABLE,
        Constants.CMD_SET_ROUTE_VARIABLE,
        Constants.CMD_SET_SIM_VARIABLE,
        Constants.CMD_SET_TL_VARIABLE,
        Constants.CMD_SET_VEHICLE_VARIABLE,
        Constants.CMD_SET_VEHICLETYPE_VARIABLE
    };

    private List<ResponseContainer> pairs = new ArrayList<ResponseContainer>();

    /**
     * Constructor that unpacks all the data from a {@link DataInputStream},
     * populating the list of {@link ResponseContainer}s.
     * <p>
     * A response message is made of individual packets. It is assumed that the
     * first packet is always a status response. According to the status
     * response, different data are expected next:
     * <ul>
     * <li>if the status response is not {@link Constants#RTYPE_OK}, another
     * status response or the end of the stream are expected.</li>
     * <li>if the status ID is equal to {@link Constants#CMD_SIMSTEP}, it is
     * expected an integer representing N and N following sub-responses;</li>
     * <li>if the status ID matches one of the commands in
     * {@link #STATUS_ONLY_RESPONSES} , it is expected another status response;
     * <li>if the status ID doesn't match any of the above, it is expected a
     * response command.
     * </ul>
     * A new {@link ResponseMessage}, collecting all the data that belong to the
     * same request, is built and appended to an internal list.
     *
     * @param dis dis
     * @throws IOException Exception
     */
    public ResponseMessage(DataInputStream dis) throws IOException {
        int totalLen = dis.readInt() - Integer.SIZE / 8;

        byte[] buffer = new byte[totalLen];
        dis.readFully(buffer);
        //StringBuilder sb = new StringBuilder();
        //for (byte b : buffer) {
        //    sb.append(String.format("%X ", b));
        //}
        //System.out.println("ResponseMessage::init buffer=" + sb);

        Storage s = new Storage(buffer);

        while (s.validPos()) {
            StatusResponse sr = new StatusResponse(s);
            ResponseContainer responseContainer;

            if (sr.result() != Constants.RTYPE_OK) {
                responseContainer = new ResponseContainer(sr, null);
            } else if (sr.id() == Constants.CMD_SIMSTEP) {
                int nSubResponses = s.readInt();
                List<Command> subResponses = new ArrayList<Command>(
                    nSubResponses);
                for (int i = 0; i < nSubResponses; i++) {
                    subResponses.add(new Command(s));
                }

                responseContainer = new ResponseContainer(sr, null, subResponses);

            } else if (isStatusOnlyResponse(sr.id())) {
                responseContainer = new ResponseContainer(sr, null);
            } else {
                responseContainer = new ResponseContainer(sr, new Command(s));
            }

            pairs.add(responseContainer);
        }
    }

    private boolean isStatusOnlyResponse(int statusResponseID) {
        for (int id : STATUS_ONLY_RESPONSES)
            if (id == statusResponseID) {
                return true;
            }
        return false;
    }

    public List<ResponseContainer> responses() {
        return Collections.unmodifiableList(pairs);
    }


}
