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
/// @file    SumoCommand.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.util;

import java.util.LinkedList;
import java.util.Map;

import de.tudresden.sumo.config.Constants;
import de.tudresden.ws.container.SumoColor;
import de.tudresden.ws.container.SumoGeometry;
import de.tudresden.ws.container.SumoPosition2D;
import de.tudresden.ws.container.SumoPosition3D;
import de.tudresden.ws.container.SumoStopFlags;
import de.tudresden.ws.container.SumoStringList;
import de.tudresden.ws.container.SumoTLSProgram;
import de.tudresden.ws.container.SumoTLSPhase;
import it.polito.appeal.traci.protocol.Command;

/**
 *
 * @author Mario Krumnow
 *
 */

public class SumoCommand {

    Command cmd;
    public LinkedList<Object> raw;
    public int input1;
    public int input2;
    String input3;
    String info;
    int response;
    int output_type;

    //Get Statements
    public SumoCommand(int input1, int input2, Object input3, int response, int output_type) {

        this.cmd = new Command(input1);
        cmd.content().writeUnsignedByte(input2);
        cmd.content().writeStringASCII(String.valueOf(input3));

        this.input1 = input1;
        this.input2 = input2;
        this.input3 = String.valueOf(input3);

        this.response = response;
        this.output_type = output_type;

        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);
        this.raw.add(input3);
        this.raw.add(response);
        this.raw.add(output_type);

    }

    //Get Statements
    public SumoCommand(int input1, int input2, Object input3, int response, int output_type, String info) {

        this.cmd = new Command(input1);
        cmd.content().writeUnsignedByte(input2);
        cmd.content().writeStringASCII(String.valueOf(input3));

        this.input1 = input1;
        this.input2 = input2;
        this.input3 = String.valueOf(input3);
        this.info = info;

        this.response = response;
        this.output_type = output_type;

        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);
        this.raw.add(input3);
        this.raw.add(response);
        this.raw.add(output_type);

    }


    public SumoCommand(int input1, int input2, Object input3, Object[] array, int response, int output_type) {

        this.cmd = new Command(input1);
        cmd.content().writeUnsignedByte(input2);
        cmd.content().writeStringASCII(String.valueOf(input3));

        if (array.length == 1) {
            add_type(array[0]);
            add_variable(array[0]);
        } else {

            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            if (input1 == Constants.CMD_GET_VEHICLE_VARIABLE && input2 == Constants.DISTANCE_REQUEST) {

                cmd.content().writeInt(2);

                if (array.length == 3) {
                    cmd.content().writeUnsignedByte(Constants.POSITION_ROADMAP);
                    cmd.content().writeStringASCII((String) array[0]);
                    cmd.content().writeDouble((double) array[1]);
                    cmd.content().writeUnsignedByte((byte) array[2]);
                } else if (array.length == 2) {
                    cmd.content().writeUnsignedByte(Constants.POSITION_2D);
                    cmd.content().writeDouble((double) array[0]);
                    cmd.content().writeDouble((double) array[1]);
                }

                cmd.content().writeUnsignedByte(Constants.REQUEST_DRIVINGDIST);

            } else if (input1 == Constants.CMD_GET_SIM_VARIABLE && input2 == Constants.POSITION_CONVERSION) {

                if (output_type == Constants.POSITION_ROADMAP) {
                    cmd.content().writeInt(3);
                    cmd.content().writeUnsignedByte((byte) array[0]);
                    cmd.content().writeDouble((double) array[1]);
                    cmd.content().writeDouble((double) array[2]);
                    cmd.content().writeUnsignedByte(Constants.TYPE_UBYTE);
                    cmd.content().writeUnsignedByte((Integer) array[3]);
                    cmd.content().writeUnsignedByte(Constants.TYPE_STRING);
                    cmd.content().writeStringASCII((String) array[4]);

                } else {
                    cmd.content().writeInt(2);
                    if (array.length == 4) {
                        cmd.content().writeUnsignedByte((byte) array[0]);
                        cmd.content().writeDouble((double) array[1]);
                        cmd.content().writeDouble((double) array[2]);
                        cmd.content().writeUnsignedByte(Constants.TYPE_UBYTE);
                        cmd.content().writeUnsignedByte((byte) array[3]);
                    }
                }

            } else {

                cmd.content().writeInt(array.length);

                for (int i = 0; i < array.length; i++) {
                    add_type(array[i]);
                    add_variable(array[i]);
                }
            }
        }

        this.input1 = input1;
        this.input2 = input2;
        this.input3 = String.valueOf(input3);

        this.response = response;
        this.output_type = output_type;

        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);
        this.raw.add(input3);
        this.raw.add(response);
        this.raw.add(output_type);

    }

    public SumoCommand(int input1, int input2) {

        this.input1 = input1;
        this.input2 = input2;

        this.cmd = new Command(input1);
        this.add_variable(input2);

        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);

    }

    public SumoCommand(int input1, int input2, Object input3, Object[] array) {

        this.cmd = new Command(input1);
        this.input1 = input1;
        this.input2 = input2;

        cmd.content().writeUnsignedByte(input2);
        cmd.content().writeStringASCII(String.valueOf(input3));

        if (input2 == Constants.VAR_COLOR) {
            cmd.content().writeUnsignedByte(Constants.TYPE_COLOR);
            for (int i = 0; i < array.length; i++) {
                add_variable(array[i]);
            }
        }

        else if (input2 == Constants.VAR_ROUTE) {

            cmd.content().writeUnsignedByte(Constants.TYPE_STRINGLIST);
            SumoStringList sl = (SumoStringList) array[0];
            cmd.content().writeInt(sl.size());
            for (String s : sl) {
                cmd.content().writeStringASCII(s);
            }

        } else if (input2 == Constants.CMD_REROUTE_EFFORT || input2 == Constants.CMD_REROUTE_TRAVELTIME || input2 == Constants.CMD_RESUME) {
            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(0);
        } else if (input2 == Constants.VAR_VIEW_OFFSET) {

            cmd.content().writeUnsignedByte(Constants.POSITION_2D);
            for (int i = 0; i < array.length; i++) {
                add_variable(array[i]);
            }

        } else if (input1 == Constants.CMD_SET_POLYGON_VARIABLE && input2 == Constants.ADD) {


            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(5);

            add_type(array[3]);
            add_variable(array[3]);

            //color
            add_type(array[1]);
            add_variable(array[1]);

            //fill
            add_type(array[2]);
            add_variable(array[2]);

            //layer
            add_type(array[4]);
            add_variable(array[4]);

            //shape
            add_type(array[0]);
            add_variable(array[0]);

        } else if (input1 == Constants.CMD_SET_POI_VARIABLE && input2 == Constants.VAR_POSITION) {

            cmd.content().writeUnsignedByte(Constants.POSITION_2D);
            add_variable(array[0]);
            add_variable(array[1]);

        } else if (input1 == Constants.CMD_SET_POI_VARIABLE && input2 == Constants.ADD) {

            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(4);

            //add name
            add_type(array[3]);
            add_variable(array[3]);

            //color
            add_type(array[2]);
            add_variable(array[2]);

            //layer
            add_type(array[4]);
            add_variable(array[4]);


            cmd.content().writeUnsignedByte(Constants.POSITION_2D);
            add_variable(array[0]);
            add_variable(array[1]);


        } else {
            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(array.length);
            for (int i = 0; i < array.length; i++) {
                add_type(array[i]);
                add_variable(array[i]);
            }
        }



        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);
        this.raw.add(input3);
        this.raw.add(array);


    }

    public SumoCommand(int input1, int input2, Object input3, Object input) {

        this.cmd = new Command(input1);
        this.input1 = input1;
        this.input2 = input2;

        cmd.content().writeUnsignedByte(input2);
        cmd.content().writeStringASCII(String.valueOf(input3));

        if (input.getClass().equals(StringList.class)) {

            StringList sl = (StringList) input;
            cmd.content().writeUnsignedByte(Constants.TYPE_STRINGLIST);
            cmd.content().writeInt(sl.size());
            for (String s : sl) {
                cmd.content().writeStringASCII(s);
            }

        } else if (input.getClass().equals(SumoStringList.class)) {

            SumoStringList sl = (SumoStringList) input;
            cmd.content().writeUnsignedByte(Constants.TYPE_STRINGLIST);
            cmd.content().writeInt(sl.size());
            for (String s : sl) {
                cmd.content().writeStringASCII(s);
            }

        } else if (input.getClass().equals(SumoTLSProgram.class)) {

            SumoTLSProgram stl = (SumoTLSProgram) input;
            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(5);

            cmd.content().writeUnsignedByte(Constants.TYPE_STRING);
            cmd.content().writeStringASCII(stl.subID);

            cmd.content().writeUnsignedByte(Constants.TYPE_INTEGER);
            cmd.content().writeInt(stl.type);

            cmd.content().writeUnsignedByte(Constants.TYPE_INTEGER);
            cmd.content().writeInt(stl.currentPhaseIndex);

            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(stl.phases.size());

            for (SumoTLSPhase phase : stl.phases) {
                add_variable(phase);
            }

            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(stl.params.size());
            for (Map.Entry<String, String> entry : stl.params.entrySet()) {
                SumoStringList keyValue = new SumoStringList();
                keyValue.add(entry.getKey());
                keyValue.add(entry.getValue());
                cmd.content().writeUnsignedByte(Constants.TYPE_STRINGLIST);
                add_variable(keyValue);
            }


        } else {
            add_type(input);
            add_variable(input);
        }

        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);
        this.raw.add(input3);
        this.raw.add(input);

    }

    public SumoCommand(int input1, int input2, Object[] array, int response, int output_type) {

        this.cmd = new Command(input1);
        this.input1 = input1;
        this.input2 = input2;

        cmd.content().writeUnsignedByte(input2);
        cmd.content().writeStringASCII("");

        if (input1 == Constants.CMD_GET_SIM_VARIABLE && input2 == Constants.DISTANCE_REQUEST && array.length == 4) {

            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(3);

            boolean isGeo = (boolean) array[2];
            boolean isDriving = (boolean) array[3];

            if (!isGeo) {
                this.cmd.content().writeUnsignedByte(Constants.POSITION_2D);
            } else {
                this.cmd.content().writeUnsignedByte(Constants.POSITION_LON_LAT);
            }

            add_variable(array[0]);

            if (!isGeo) {
                this.cmd.content().writeUnsignedByte(Constants.POSITION_2D);
            } else {
                this.cmd.content().writeUnsignedByte(Constants.POSITION_LON_LAT);
            }

            add_variable(array[1]);

            if (isDriving) {
                this.cmd.content().writeUnsignedByte(Constants.REQUEST_DRIVINGDIST);
            } else {
                this.cmd.content().writeUnsignedByte(Constants.REQUEST_AIRDIST);
            }

        } else if (input1 == Constants.CMD_GET_SIM_VARIABLE && input2 == Constants.DISTANCE_REQUEST && array.length == 5) {

            cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(3);

            String edge1 = (String) array[0];

            cmd.content().writeUnsignedByte(Constants.POSITION_ROADMAP);
            add_variable(edge1);
            add_variable(array[1]);
            cmd.content().writeUnsignedByte(0);

            String edge2 = (String) array[2];
            cmd.content().writeUnsignedByte(Constants.POSITION_ROADMAP);
            add_variable(edge2);
            add_variable(array[3]);
            cmd.content().writeUnsignedByte(0);

            boolean isDriving = (boolean) array[4];

            if (isDriving) {
                this.cmd.content().writeUnsignedByte(Constants.REQUEST_DRIVINGDIST);
            } else {
                this.cmd.content().writeUnsignedByte(Constants.REQUEST_AIRDIST);
            }

        }

        this.response = response;
        this.output_type = output_type;

        this.raw = new LinkedList<Object>();
        this.raw.add(input1);
        this.raw.add(input2);
        this.raw.add(input3);
        this.raw.add(array);

    }

    public Object[] get_raw() {

        Object[] output = new Object[this.raw.size()];
        for (int i = 0; i < this.raw.size(); i++) {
            output[i] = this.raw.get(i);
        }

        return output;
    }


    private void add_type(Object input) {

        if (input.getClass().equals(Integer.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_INTEGER);
        } else if (input.getClass().equals(String.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_STRING);
        } else if (input.getClass().equals(Double.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_DOUBLE);
        } else if (input.getClass().equals(Byte.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_BYTE);
        } else if (input.getClass().equals(SumoColor.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_COLOR);
        } else if (input.getClass().equals(SumoGeometry.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_POLYGON);
        } else if (input.getClass().equals(SumoPosition2D.class)) {
            this.cmd.content().writeUnsignedByte(Constants.POSITION_2D);
        } else if (input.getClass().equals(SumoPosition3D.class)) {
            this.cmd.content().writeUnsignedByte(Constants.POSITION_3D);
        } else if (input.getClass().equals(SumoStopFlags.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_BYTE);
        } else if (input.getClass().equals(Boolean.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_UBYTE);
        } else if (input.getClass().equals(SumoStringList.class)) {
            this.cmd.content().writeUnsignedByte(Constants.TYPE_STRINGLIST);
        }

    }

    private void add_variable(Object input) {

        if (input.getClass().equals(Integer.class)) {
            this.cmd.content().writeInt((Integer) input);
        } else if (input.getClass().equals(String.class)) {
            this.cmd.content().writeStringASCII((String) input);
        } else if (input.getClass().equals(Double.class)) {
            this.cmd.content().writeDouble((Double) input);
        } else if (input.getClass().equals(Byte.class)) {
            this.cmd.content().writeByte((Byte) input);
        } else if (input.getClass().equals(Boolean.class)) {
            boolean b = (Boolean) input;
            cmd.content().writeUnsignedByte(b ? 1 : 0);
        } else if (input.getClass().equals(SumoColor.class)) {

            SumoColor sc = (SumoColor) input;
            this.cmd.content().writeByte(sc.r);
            this.cmd.content().writeByte(sc.g);
            this.cmd.content().writeByte(sc.b);
            this.cmd.content().writeByte(sc.a);

        } else if (input.getClass().equals(SumoGeometry.class)) {

            SumoGeometry sg = (SumoGeometry) input;
            cmd.content().writeUnsignedByte(sg.coords.size());

            for (SumoPosition2D pos : sg.coords) {
                cmd.content().writeDouble(pos.x);
                cmd.content().writeDouble(pos.y);
            }
        } else if (input.getClass().equals(SumoPosition2D.class)) {

            SumoPosition2D pos = (SumoPosition2D) input;
            cmd.content().writeDouble(pos.x);
            cmd.content().writeDouble(pos.y);

        } else if (input.getClass().equals(SumoTLSPhase.class)) {

            SumoTLSPhase stp = (SumoTLSPhase) input;
            this.cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(6);
            this.cmd.content().writeUnsignedByte(Constants.TYPE_DOUBLE);
            cmd.content().writeDouble(stp.duration);
            this.cmd.content().writeUnsignedByte(Constants.TYPE_STRING);
            cmd.content().writeStringASCII(stp.phasedef);
            this.cmd.content().writeUnsignedByte(Constants.TYPE_DOUBLE);
            cmd.content().writeDouble(stp.minDur);
            this.cmd.content().writeUnsignedByte(Constants.TYPE_DOUBLE);
            cmd.content().writeDouble(stp.maxDur);
            this.cmd.content().writeUnsignedByte(Constants.TYPE_COMPOUND);
            cmd.content().writeInt(stp.next.size());
            for (int n : stp.next) {
                this.cmd.content().writeUnsignedByte(Constants.TYPE_INTEGER);
                cmd.content().writeInt(n);
            }
            this.cmd.content().writeUnsignedByte(Constants.TYPE_STRING);
            cmd.content().writeStringASCII(stp.name);

        } else if (input.getClass().equals(SumoStringList.class)) {

            SumoStringList sl = (SumoStringList) input;
            cmd.content().writeInt(sl.size());
            for (String s : sl) {
                cmd.content().writeStringASCII(s);
            }

        } else if (input.getClass().equals(SumoStopFlags.class)) {
            SumoStopFlags sf = (SumoStopFlags) input;
            this.cmd.content().writeByte(sf.getID());
        }

    }

    public Command get_command() {
        return this.cmd;
    }

}
