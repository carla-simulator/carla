/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2016-2017 Dresden University of Technology
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
/// @file    Query.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.util;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.List;

import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.subscription.ResponseType;
import de.tudresden.sumo.subscription.SubscriptionObject;
import de.tudresden.ws.container.SumoObject;
import de.uniluebeck.itm.tcpip.Storage;
import it.polito.appeal.traci.TraCIException;
import it.polito.appeal.traci.TraCIException.UnexpectedData;
import it.polito.appeal.traci.protocol.Command;
import it.polito.appeal.traci.protocol.RequestMessage;
import it.polito.appeal.traci.protocol.ResponseContainer;
import it.polito.appeal.traci.protocol.ResponseMessage;
import it.polito.appeal.traci.protocol.StatusResponse;

public abstract class Query extends Observable {

    private final DataOutputStream outStream;
    private final DataInputStream inStream;

    public Query(Socket sock) throws IOException {
        outStream = new DataOutputStream(sock.getOutputStream());
        inStream = new DataInputStream(sock.getInputStream());
    }

    /**
     * Sends a request message to SUMO and returns a response message.
     * @param msg
     * @throws IOException
     */
    protected ResponseMessage doQuery(RequestMessage msg) throws IOException {
        msg.writeTo(getOutStream());
        return new ResponseMessage(inStream);
    }

    /**
     * Like {@link #doQuery(RequestMessage)}; in addition, verifies that all
     * responses are successful and and the statuses match the requests.
     * @param reqMsg
     * @return the verified response message
     * @throws IOException
     * @see #doQuery(RequestMessage)
     */
    protected ResponseMessage queryAndVerify(RequestMessage reqMsg) throws IOException {

        reqMsg.writeTo(getOutStream());

        ResponseMessage respMsg = new ResponseMessage(inStream);

        List<Command> commands = reqMsg.commands();
        List<ResponseContainer> responses = respMsg.responses();
        if (commands.size() > responses.size()) {
            throw new TraCIException("not enough responses received");
        }

        for (int i = 0; i < commands.size(); i++) {
            Command cmd = commands.get(i);
            ResponseContainer responsePair = responses.get(i);
            StatusResponse statusResp = responsePair.getStatus();
            verify("command and status IDs match", cmd.id(), statusResp.id());
            if (statusResp.result() != Constants.RTYPE_OK)
                throw new TraCIException("SUMO error for command "
                                         + statusResp.id() + ": " + statusResp.description());
        }

        return respMsg;
    }

    /**
     * Like {@link #doQuery(RequestMessage)}, but good for one-command/
     * one-response queries.
     *
     * @param request
     * @throws IOException
     */
    protected ResponseContainer doQuerySingle(Command request) throws IOException {
        RequestMessage msg = new RequestMessage();
        msg.append(request);
        ResponseMessage resp = doQuery(msg);
        return resp.responses().iterator().next();
    }

    /**
     * Set multi-client ordering index
     *
     * @param index
     * @throws IOException
     */
    protected void doSetOrder(int index) throws IOException {
        Command cmd = new Command(Constants.CMD_SETORDER);
        cmd.content().writeInt(index);
        queryAndVerifySingle(cmd);
    }

    /**
     * Disconnect client from the sumo server
     *
     * @throws IOException
     */
    protected void doClose() throws IOException {
        Command cmd = new Command(Constants.CMD_CLOSE);
        queryAndVerifySingle(cmd);
    }

    /**
     * Do next time step and update subscription results
     *
     * @param targetTime
     * @throws IOException
     */
    protected void doSimulationStep(double targetTime) throws IOException {

        Command cmd = new Command(Constants.CMD_SIMSTEP);
        cmd.content().writeDouble(targetTime);

        RequestMessage msg = new RequestMessage();
        msg.append(cmd);
        msg.writeTo(getOutStream());

        int totalLen = this.inStream.readInt() - Integer.SIZE / 8;
        byte[] buffer = new byte[totalLen];
        this.inStream.readFully(buffer);


        Storage s = new Storage(buffer);
        for (int i = 0; i < 7; i++) {
            s.readUnsignedByte();   //offset
        }

        int count = s.readInt();

        //subscription results
        for (int i = 0; i < count; i++) {

            for (int i1 = 0; i1 < 5; i1++) {
                s.readUnsignedByte();   //offset
            }

            int response = s.readUnsignedByte();
            ResponseType rt = ResponseType.getType(response);

            if (rt != ResponseType.UNKNOWN) {

                String id = s.readStringASCII();

                //context
                if (rt.isContext()) {

                    int domain =  s.readUnsignedByte();
                    int Variable_Count = s.readUnsignedByte();
                    int Object_Count = s.readInt();

                    for (int oc = 0; oc < Object_Count; oc++) {

                        String name = s.readStringASCII();
                        for (int vc = 0; vc < Variable_Count; vc++) {

                            int variable =  s.readUnsignedByte();
                            int status =  s.readUnsignedByte(); //RTYPE_OK
                            int return_type =  s.readUnsignedByte();
                            SumoObject o = CommandProcessor.read(return_type, s);

                            SubscriptionObject so = new SubscriptionObject(id, rt, domain, name, variable, status, return_type, o);
                            notifyObservers(so);
                      
                        }
                    }
                } else {

                    //variable
                    int Variable_Count = s.readUnsignedByte();
                    for (int vc = 0; vc < Variable_Count; vc++) {

                        int variable =  s.readUnsignedByte();
                        int status =  s.readUnsignedByte(); //RTYPE_OK
                        int return_type =  s.readUnsignedByte();
                        SumoObject o = CommandProcessor.read(return_type, s);

                        SubscriptionObject so = new SubscriptionObject(id, rt, variable, status, return_type, o);
                        notifyObservers(so);

                    }

                }

            }

        }
    }


    /**
     * Like {@link #queryAndVerify(RequestMessage)}, but good for one-command/
     * one-response queries.
     *
     * @param request
     * @return the response container for the specified request
     * @throws IOException
     */
    protected ResponseContainer queryAndVerifySingle(Command request) throws IOException {

        RequestMessage msg = new RequestMessage();
        msg.append(request);
        ResponseMessage resp = queryAndVerify(msg);
        return resp.responses().iterator().next();

    }

    /**
     * fireAndForget function
     *
     * @param request
     * @throws IOException
     */
    protected void fireAndForget(Command request) {

        try {

            RequestMessage msg = new RequestMessage();
            msg.append(request);
            msg.writeTo(this.outStream);

            int totalLen = this.inStream.readInt() - Integer.SIZE / 8;

            byte[] buffer = new byte[totalLen];
            this.inStream.readFully(buffer);

        } catch (Exception ex) {
            ex.printStackTrace();
        }

    }

    protected static String verifyGetVarResponse(Command resp, int commandID, int variable, String objectID) throws UnexpectedData {
        verify("response code", commandID, resp.id());
        verify("variable ID", variable, (int)resp.content().readUnsignedByte());
        String respObjectID = resp.content().readStringASCII();
        if (objectID != null) {
            verify("object ID", objectID, respObjectID);
        }
        return respObjectID;
    }

    protected static void verify(String description, Object expected, Object actual) throws UnexpectedData {
        if (!actual.equals(expected)) {
            throw new UnexpectedData(description, expected, actual);
        }
    }

    protected static void verify(String description, int expected, short actual) throws UnexpectedData {
        verify(description, expected, (int)actual);
    }

    protected static void verify(String description, int expected, byte actual) throws UnexpectedData {
        verify(description, expected, (int)actual);
    }

    public DataOutputStream getOutStream() {
        return outStream;
    }

}
