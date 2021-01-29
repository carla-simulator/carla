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
/// @file    CommandProcessor.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.util;

import it.polito.appeal.traci.TraCIException;
import it.polito.appeal.traci.TraCIException.UnexpectedData;
import it.polito.appeal.traci.protocol.Command;
import it.polito.appeal.traci.protocol.ResponseContainer;

import java.io.IOException;
import java.net.Socket;
import java.util.LinkedList;
import java.util.ArrayList;

import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.subscription.Subscription;
import de.tudresden.ws.container.SumoBestLanes;
import de.tudresden.ws.container.SumoColor;
import de.tudresden.ws.container.SumoGeometry;
import de.tudresden.ws.container.SumoLeader;
import de.tudresden.ws.container.SumoLink;
import de.tudresden.ws.container.SumoLinkList;
import de.tudresden.ws.container.SumoNextTLS;
import de.tudresden.ws.container.SumoObject;
import de.tudresden.ws.container.SumoPosition2D;
import de.tudresden.ws.container.SumoPosition3D;
import de.tudresden.ws.container.SumoPrimitive;
import de.tudresden.ws.container.SumoStopFlags;
import de.tudresden.ws.container.SumoStringList;
import de.tudresden.ws.container.SumoTLSProgram;
import de.tudresden.ws.container.SumoVehicleData;
import de.uniluebeck.itm.tcpip.Storage;
import de.tudresden.ws.container.SumoTLSPhase;
import de.tudresden.ws.container.SumoTLSController;
import de.tudresden.ws.container.SumoStage;
import de.tudresden.ws.container.SumoRoadPosition;

/**
 *
 * @author Mario Krumnow
 *
 */

public class CommandProcessor extends Query {

    int temp;
    public CommandProcessor(Socket sock) throws IOException {
        super(sock);
    }

    public synchronized void do_job_set(SumoCommand sc) throws IOException {
        queryAndVerifySingle(sc.cmd);
    }

    public void do_subscription(Subscription cs) throws IOException {
        fireAndForget(cs.getCommand());
    }

    public synchronized void do_SimulationStep(double targetTime) throws IOException {
        doSimulationStep(targetTime);
    }

    public synchronized void do_setOrder(int index) throws IOException {
        doSetOrder(index);
    }

    public synchronized void do_close() throws IOException {
        doClose();
    }

    public static SumoObject read(int type, Storage s) {

        SumoObject output = null;

        if (type == Constants.TYPE_INTEGER) {
            output = new SumoPrimitive(s.readInt());
        } else if (type == Constants.TYPE_DOUBLE) {
            output = new SumoPrimitive(s.readDouble());
        } else if (type == Constants.TYPE_STRING) {
            output = new SumoPrimitive(s.readStringUTF8());
        } else if (type == Constants.POSITION_2D) {
            double x = s.readDouble();
            double y = s.readDouble();
            output = new SumoPosition2D(x, y);
        } else if (type == Constants.POSITION_3D) {
            double x = s.readDouble();
            double y = s.readDouble();
            double z = s.readDouble();
            output = new SumoPosition3D(x, y, z);
        } else if (type == Constants.TYPE_STRINGLIST) {

            SumoStringList ssl = new SumoStringList();
            int laenge = s.readInt();
            for (int i = 0; i < laenge; i++) {
                ssl.add(s.readStringASCII());
            }
            output = ssl;

        } else if (type == Constants.VAR_STOPSTATE) {

            short s0 = s.readByte();
            SumoStopFlags sf = new SumoStopFlags((byte) s0);
            output = sf;

            //if(s0.info.equals("isStopped")){output = sf.stopped;}
            //if(sc.info.equals("isStoppedTriggered")){output = sf.triggered;}
            //if(sc.info.equals("isAtContainerStop")){output = sf.isContainerStop;}
            //if(sc.info.equals("isStoppedParking")){output = sf.getID() == 12;}
            //if(sc.info.equals("isAtBusStop")){output = sf.isBusStop;}


        } else if (type == Constants.TL_CONTROLLED_LINKS) {

            SumoLinkList sll = new SumoLinkList();

            //read length
            s.readUnsignedByte();
            s.readInt();

            int laenge = s.readInt();
            for (int i = 0; i < laenge; i++) {

                s.readUnsignedByte();
                int anzahl = s.readInt();

                for (int i1 = 0; i1 < anzahl; i1++) {

                    s.readUnsignedByte();
                    s.readInt(); //length

                    String from = s.readStringASCII();
                    String to = s.readStringASCII();
                    String over = s.readStringASCII();
                    sll.add(new SumoLink(from, to, over));

                }

            }

            output = sll;

        } else if (type == Constants.TL_COMPLETE_DEFINITION_RYG) {

            s.readUnsignedByte();
            s.readInt();

            int length = s.readInt();

            SumoTLSController sp = new SumoTLSController();
            for (int i = 0; i < length; i++) {

                s.readUnsignedByte();
                String subID = s.readStringASCII();

                s.readUnsignedByte();
                int type0 = s.readInt();

                s.readUnsignedByte();
                s.readInt();

                s.readUnsignedByte();
                int currentPhaseIndex = s.readInt();

                SumoTLSProgram stl = new SumoTLSProgram(subID, type0, currentPhaseIndex);

                s.readUnsignedByte();
                int nbPhases = s.readInt();

                for (int i1 = 0; i1 < nbPhases; i1++) {
                    s.readUnsignedByte(); // type compound
                    s.readInt(); // 6

                    s.readUnsignedByte();
                    double duration = s.readDouble();

                    s.readUnsignedByte();
                    String phaseDef = s.readStringASCII();

                    s.readUnsignedByte();
                    double minDur = s.readDouble();

                    s.readUnsignedByte();
                    double maxDur = s.readDouble();

                    s.readUnsignedByte();
                    int numNext = s.readInt();
                    ArrayList<Integer> next = new ArrayList<Integer>();
                    for (int i2 = 0; i2 < numNext; i2++) {
                        s.readUnsignedByte();
                        next.add(s.readInt());
                    }
                    s.readUnsignedByte();
                    String name = s.readStringASCII();

                    stl.add(new SumoTLSPhase(duration, minDur, maxDur, phaseDef, next, name));

                }

                sp.addProgram(stl);

            }

            output = sp;

        } else if (type == Constants.LANE_LINKS) {

            s.readUnsignedByte();
            s.readInt();

            //number of links
            int length = s.readInt();
            SumoLinkList links = new SumoLinkList();
            for (int i = 0; i < length; i++) {

                s.readUnsignedByte();
                String notInternalLane = s.readStringASCII();

                s.readUnsignedByte();
                String internalLane = s.readStringASCII();

                s.readUnsignedByte();
                byte hasPriority = (byte) s.readUnsignedByte();

                s.readUnsignedByte();
                byte isOpened = (byte) s.readUnsignedByte();

                s.readUnsignedByte();
                byte hasFoes = (byte) s.readUnsignedByte();

                //not implemented
                s.readUnsignedByte();
                String state = s.readStringASCII();

                s.readUnsignedByte();
                String direction = s.readStringASCII();

                s.readUnsignedByte();
                double laneLength = s.readDouble();


                links.add(new SumoLink(notInternalLane, internalLane, hasPriority, isOpened, hasFoes, laneLength, state, direction));
            }
            output = links;

        } else if (type == Constants.VAR_NEXT_TLS) {

            s.readUnsignedByte();
            s.readInt();

            SumoNextTLS sn = new SumoNextTLS();

            int length = s.readInt();
            for (int i = 0; i < length; i++) {

                s.readUnsignedByte();
                String tlsID = s.readStringASCII();

                s.readUnsignedByte();
                int ix = s.readInt();

                s.readUnsignedByte();
                double dist = s.readDouble();

                s.readUnsignedByte();
                int k = s.readUnsignedByte();
                String state = Character.toString((char) k);

                sn.add(tlsID, ix, dist, state);

            }

            output = sn;

        } else if (type == Constants.VAR_LEADER) {

            s.readUnsignedByte();
            s.readInt();

            String vehID = s.readStringASCII();
            s.readUnsignedByte();
            double dist = s.readDouble();
            output = new SumoLeader(vehID, dist);

        } else if (type == Constants.VAR_BEST_LANES) {

            s.readUnsignedByte();
            s.readInt();

            int l = s.readInt();

            SumoBestLanes sl = new SumoBestLanes();
            for (int i = 0; i < l; i++) {

                s.readUnsignedByte();
                String laneID = s.readStringASCII();

                s.readUnsignedByte();
                double length = s.readDouble();

                s.readUnsignedByte();
                double occupation = s.readDouble();

                s.readUnsignedByte();
                int offset = s.readByte();

                s.readUnsignedByte();
                int allowsContinuation = s.readUnsignedByte();

                s.readUnsignedByte();
                int nextLanesNo = s.readInt();

                LinkedList<String> ll = new LinkedList<String>();
                for (int i1 = 0; i1 < nextLanesNo; i1++) {
                    String lane = s.readStringASCII();
                    ll.add(lane);
                }

                sl.add(laneID, length, occupation, offset, allowsContinuation, ll);
            }

            output = sl;

        } else if (type == Constants.TYPE_POLYGON) {

            int laenge = s.readUnsignedByte();

            SumoGeometry sg = new SumoGeometry();
            for (int i = 0; i < laenge; i++) {
                double x =  s.readDouble();;
                double y = s.readDouble();;
                sg.add(new SumoPosition2D(x, y));
            }

            output = sg;

        } else if (type == Constants.TYPE_COLOR) {

            int r = s.readUnsignedByte();
            int g = s.readUnsignedByte();
            int b = s.readUnsignedByte();
            int a = s.readUnsignedByte();

            output = new SumoColor(r, g, b, a);

        } else if (type == Constants.TYPE_UBYTE) {
            output = new SumoPrimitive(s.readUnsignedByte());
        }

        return output;
    }

    public synchronized Object do_job_get(SumoCommand sc) throws IOException {

        Object output = null;
        ResponseContainer rc = queryAndVerifySingle(sc.cmd);
        Command resp = rc.getResponse();

        if (resp == null) {
            throw new TraCIException("Received null repsonse for command " + sc.input1 + " " + sc.input2 + ". (Maybe you need to use do_job_set?)");
        }
        verifyGetVarResponse(resp, sc.response, sc.input2, sc.input3);
        verify("", sc.output_type, (int)resp.content().readUnsignedByte());

        if (sc.output_type == Constants.TYPE_INTEGER) {
            output = resp.content().readInt();
        } else if (sc.output_type == Constants.TYPE_DOUBLE) {
            output = resp.content().readDouble();
        } else if (sc.output_type == Constants.TYPE_STRING) {
            output = resp.content().readStringUTF8();
        } else if (sc.output_type == Constants.POSITION_2D || sc.output_type == Constants.POSITION_LON_LAT) {
            double x = resp.content().readDouble();
            double y = resp.content().readDouble();
            output = new SumoPosition2D(x, y);
        } else if (sc.output_type == Constants.POSITION_3D) {
            double x = resp.content().readDouble();
            double y = resp.content().readDouble();
            double z = resp.content().readDouble();
            output = new SumoPosition3D(x, y, z);
        } else if (sc.output_type == Constants.POSITION_ROADMAP) {
            SumoRoadPosition roadPos = new SumoRoadPosition();
            roadPos.edgeID = resp.content().readStringASCII();
            roadPos.pos = resp.content().readDouble();
            roadPos.laneIndex = resp.content().readByte();
            output = roadPos;
        } else if (sc.output_type == Constants.TYPE_STRINGLIST) {

            SumoStringList ssl = new SumoStringList();
            int laenge = resp.content().readInt();
            for (int i = 0; i < laenge; i++) {
                ssl.add(resp.content().readStringASCII());
            }
            output = ssl;

        } else if (sc.input2 == Constants.VAR_STOPSTATE) {
            short s = resp.content().readByte();
            SumoStopFlags sf = new SumoStopFlags((byte) s);
            output = sf;

            if (sc.info.equals("isStopped")) {
                output = sf.stopped;
            }
            if (sc.info.equals("isStoppedTriggered")) {
                output = sf.triggered;
            }
            if (sc.info.equals("isAtContainerStop")) {
                output = sf.isContainerStop;
            }
            if (sc.info.equals("isStoppedParking")) {
                output = sf.getID() == 12;
            }
            if (sc.info.equals("isAtBusStop")) {
                output = sf.isBusStop;
            }


        } else if (sc.output_type == Constants.TYPE_COMPOUND) {

            Object[] obj = null;

            //decision making
            if (sc.input2 == Constants.TL_CONTROLLED_LINKS) {

                SumoLinkList sll = new SumoLinkList();

                //read length
                resp.content().readUnsignedByte();
                resp.content().readInt();

                int laenge = resp.content().readInt();
                obj = new StringList[laenge];

                for (int i = 0; i < laenge; i++) {

                    resp.content().readUnsignedByte();
                    int anzahl = resp.content().readInt();

                    for (int i1 = 0; i1 < anzahl; i1++) {

                        resp.content().readUnsignedByte();
                        resp.content().readInt(); //length

                        String from = resp.content().readStringASCII();
                        String to = resp.content().readStringASCII();
                        String over = resp.content().readStringASCII();
                        sll.add(new SumoLink(from, to, over));

                    }

                }

                output = sll;

            } else if (sc.input2 == Constants.TL_COMPLETE_DEFINITION_RYG) {

                //System.out.println("read TL_COMPLETE_DEFINITION_RYG");
                //System.out.println(resp.content().debug());
                int length = resp.content().readInt();
                //System.out.println("length=" + length);

                SumoTLSController sp = new SumoTLSController();
                for (int i = 0; i < length; i++) {
                    resp.content().readUnsignedByte(); // type compound
                    resp.content().readInt(); // 5
                    //System.out.println("read compound " + tmp);

                    resp.content().readUnsignedByte();
                    String subID = resp.content().readStringASCII();
                    //System.out.println("subID=" + subID);

                    resp.content().readUnsignedByte();
                    int type = resp.content().readInt();
                    //System.out.println("type=" + type);

                    resp.content().readUnsignedByte();
                    int currentPhaseIndex = resp.content().readInt();
                    //System.out.println("currentPhaseIndex=" + currentPhaseIndex);

                    SumoTLSProgram stl = new SumoTLSProgram(subID, type, currentPhaseIndex);

                    resp.content().readUnsignedByte();
                    int nbPhases = resp.content().readInt();
                    //System.out.println("nbPhases=" + nbPhases);

                    for (int i1 = 0; i1 < nbPhases; i1++) {
                        resp.content().readUnsignedByte(); // type compound
                        resp.content().readInt(); // 6
                        //System.out.println("read compound " + tmp2);

                        resp.content().readUnsignedByte();
                        double duration = resp.content().readDouble();
                        //System.out.println("duration=" + duration);

                        resp.content().readUnsignedByte();
                        String phaseDef = resp.content().readStringASCII();
                        //System.out.println("phaseDef=" + phaseDef);

                        resp.content().readUnsignedByte();
                        double minDur = resp.content().readDouble();
                        //System.out.println("minDur=" + minDur);

                        resp.content().readUnsignedByte();
                        double maxDur = resp.content().readDouble();
                        //System.out.println("maxDur=" + maxDur);

                        resp.content().readUnsignedByte();
                        int numNext = resp.content().readInt();
                        ArrayList<Integer> next = new ArrayList<Integer>();
                        for (int i2 = 0; i2 < numNext; i2++) {
                            resp.content().readUnsignedByte();
                            next.add(resp.content().readInt());
                        }
                        //System.out.println("next=" + next);

                        resp.content().readUnsignedByte();
                        String name = resp.content().readStringASCII();
                        //System.out.println("name=" + name);

                        stl.add(new SumoTLSPhase(duration, minDur, maxDur, phaseDef, next, name));

                    }
                    resp.content().readUnsignedByte();
                    int nParams = resp.content().readInt();
                    //System.out.println("nParams=" + nParams);
                    for (int i2 = 0; i2 < nParams; i2++) {
                        resp.content().readUnsignedByte();
                        resp.content().readInt(); // 2
                        //System.out.println("tmp3=" + tmp3);
                        String key = resp.content().readStringASCII();
                        //System.out.println("key=" + key);
                        String value = resp.content().readStringASCII();
                        //System.out.println("value=" + value);
                        stl.addParam(key, value);
                    }
                    sp.addProgram(stl);

                }
                //System.out.println("done");

                output = sp;

            } else if (sc.input2 == Constants.LANE_LINKS) {

                resp.content().readUnsignedByte();
                resp.content().readInt();

                //number of links
                int length = resp.content().readInt();
                SumoLinkList links = new SumoLinkList();
                for (int i = 0; i < length; i++) {

                    resp.content().readUnsignedByte();
                    String notInternalLane = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    String internalLane = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    byte hasPriority = (byte)resp.content().readUnsignedByte();

                    resp.content().readUnsignedByte();
                    byte isOpened = (byte)resp.content().readUnsignedByte();

                    resp.content().readUnsignedByte();
                    byte hasFoes = (byte)resp.content().readUnsignedByte();

                    //not implemented
                    resp.content().readUnsignedByte();
                    String state = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    String direction = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    double laneLength = resp.content().readDouble();


                    links.add(new SumoLink(notInternalLane, internalLane, hasPriority, isOpened, hasFoes, laneLength, state, direction));
                }
                output = links;
            } else if (sc.input2 == Constants.VAR_NEXT_TLS) {

                resp.content().readUnsignedByte();
                resp.content().readInt();

                SumoNextTLS sn = new SumoNextTLS();

                int length = resp.content().readInt();
                for (int i = 0; i < length; i++) {

                    resp.content().readUnsignedByte();
                    String tlsID = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    int ix = resp.content().readInt();

                    resp.content().readUnsignedByte();
                    double dist = resp.content().readDouble();

                    resp.content().readUnsignedByte();
                    int k = resp.content().readUnsignedByte();
                    String state = Character.toString((char) k);

                    sn.add(tlsID, ix, dist, state);

                }

                output = sn;

            } else if (sc.input2 == Constants.VAR_LEADER) {

                resp.content().readUnsignedByte();
                resp.content().readInt();

                String vehID = resp.content().readStringASCII();
                resp.content().readUnsignedByte();
                double dist = resp.content().readDouble();
                output = new SumoLeader(vehID, dist);

            } else if (sc.input2 == Constants.VAR_BEST_LANES) {

                resp.content().readUnsignedByte();
                resp.content().readInt();

                int l = resp.content().readInt();

                SumoBestLanes sl = new SumoBestLanes();
                for (int i = 0; i < l; i++) {

                    resp.content().readUnsignedByte();
                    String laneID = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    double length = resp.content().readDouble();

                    resp.content().readUnsignedByte();
                    double occupation = resp.content().readDouble();

                    resp.content().readUnsignedByte();
                    int offset = resp.content().readByte();

                    resp.content().readUnsignedByte();
                    int allowsContinuation = resp.content().readUnsignedByte();

                    resp.content().readUnsignedByte();
                    int nextLanesNo = resp.content().readInt();

                    LinkedList<String> ll = new LinkedList<String>();
                    for (int i1 = 0; i1 < nextLanesNo; i1++) {
                        String lane = resp.content().readStringASCII();
                        ll.add(lane);
                    }

                    sl.add(laneID, length, occupation, offset, allowsContinuation, ll);
                }

                output = sl;

            } else if (sc.input2 == Constants.LAST_STEP_VEHICLE_DATA) {

                resp.content().readUnsignedByte();
                resp.content().readInt();

                SumoVehicleData vehData  = new SumoVehicleData();

                int numItems = resp.content().readInt();
                for (int i = 0; i < numItems; i++) {

                    resp.content().readUnsignedByte();
                    String vehID = resp.content().readStringASCII();

                    resp.content().readUnsignedByte();
                    double length = resp.content().readDouble();

                    resp.content().readUnsignedByte();
                    double entryTime = resp.content().readDouble();

                    resp.content().readUnsignedByte();
                    double leaveTime = resp.content().readDouble();

                    resp.content().readUnsignedByte();
                    String typeID = resp.content().readStringASCII();

                    vehData.add(vehID, length, entryTime, leaveTime, typeID);
                }
                output = vehData;

            } else if (sc.input2 == Constants.FIND_ROUTE) {

                output = readStage(resp.content());

            } else if (sc.input2 == Constants.VAR_STAGE) {

                output = readStage(resp.content());

            } else if (sc.input2 == Constants.FIND_INTERMODAL_ROUTE) {

                LinkedList<SumoStage> ll = new LinkedList<SumoStage>();
                int l = resp.content().readInt();
                for (int i1 = 0; i1 < l; i1++) {
                    resp.content().readUnsignedByte(); // type compound
                    ll.add(readStage(resp.content()));
                }
                output = ll;

            } else {

                int size = resp.content().readInt();
                obj = new Object[size];

                for (int i = 0; i < size; i++) {

                    //int k = resp.content().readUnsignedByte();
                    //obj[i] = this.get_value(k, resp);

                }

                output = obj;
            }


        } else if (sc.output_type == Constants.TYPE_POLYGON) {

            int laenge = resp.content().readUnsignedByte();

            SumoGeometry sg = new SumoGeometry();
            for (int i = 0; i < laenge; i++) {
                double x = resp.content().readDouble();
                double y = resp.content().readDouble();
                sg.add(new SumoPosition2D(x, y));
            }

            output = sg;

        } else if (sc.output_type == Constants.TYPE_COLOR) {

            int r = resp.content().readUnsignedByte();
            int g = resp.content().readUnsignedByte();
            int b = resp.content().readUnsignedByte();
            int a = resp.content().readUnsignedByte();

            output = new SumoColor(r, g, b, a);

        } else if (sc.output_type == Constants.TYPE_UBYTE) {

            output = resp.content().readUnsignedByte();

        }



        return output;
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

    public static SumoStage readStage(Storage content) {
        SumoStage result = new SumoStage();
        content.readInt(); // Component (13)
        content.readUnsignedByte();
        result.type = content.readInt();

        content.readUnsignedByte();
        result.vType = content.readStringASCII();
        content.readUnsignedByte();
        result.line = content.readStringASCII();
        content.readUnsignedByte();
        result.destStop = content.readStringASCII();
        content.readUnsignedByte();

        int size = content.readInt(); // number of edges
        for (int i = 0; i < size; i++) {
            result.edges.add(content.readStringASCII());
        }

        content.readUnsignedByte();
        result.travelTime = content.readDouble();
        content.readUnsignedByte();
        result.cost = content.readDouble();
        content.readUnsignedByte();
        result.length = content.readDouble();
        content.readUnsignedByte();
        result.intended = content.readStringASCII();
        content.readUnsignedByte();
        result.depart = content.readDouble();
        content.readUnsignedByte();
        result.departPos = content.readDouble();
        content.readUnsignedByte();
        result.arrivalPos = content.readDouble();
        content.readUnsignedByte();
        result.description = content.readStringASCII();
        return result;
    }

}
