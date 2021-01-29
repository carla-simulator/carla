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
/// @file    Trafficlight.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.cmd;
import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.util.SumoCommand;
import de.tudresden.ws.container.SumoTLSProgram;

/**
 *
 * @author Mario Krumnow
 * @author Evamarie Wiessner
 *
 */


public class Trafficlight {

    //getter methods

    /**
     * Returns the complete traffic light program.
     *
     * @param tlsID a string identifying the traffic light
     * @return program definition
     */

    public static SumoCommand getCompleteRedYellowGreenDefinition(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_COMPLETE_DEFINITION_RYG, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_COMPOUND);
    }


    /**
     * Returns the number of all traffic lights in the network.
     * @return number of traffic lights
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * getPhaseDuration
     * @param tlsID a string identifying the traffic light
     * @return phase duration
     */

    public static SumoCommand getPhaseDuration(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_PHASE_DURATION, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the list of lanes which are controlled by the named traffic light.
     *
     * @param tlsID a string identifying the traffic light
     * @return list of controlled lanes
     */


    public static SumoCommand getControlledLanes(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_CONTROLLED_LANES, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the links controlled by the traffic light, sorted by the signal index and described by giving the incoming, outgoing, and via lane.
     *
     * @param tlsID a string identifying the traffic light
     * @return list of controlled links
     */

    public static SumoCommand getControlledLinks(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_CONTROLLED_LINKS, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_COMPOUND);
    }

    /**
     * Returns the junctions controlled by the traffic light
     *
     * @param tlsID a string identifying the traffic light
     * @return list of controlled junctions
     */

    public static SumoCommand getControlledJunctions(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_CONTROLLED_JUNCTIONS, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the external state
     *
     * @param tlsID a string identifying the traffic light
     * @return compound of the external state
     */

    public static SumoCommand getExternalState(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_EXTERNAL_STATE, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_COMPOUND);
    }

    /**
     * Returns a list of IDs of all traffic lights within the scenario.
     *
     * @return a list of IDs of all TLS
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the assumed time (in ms) at which the TLS changes the phase.
     *
     * @param tlsID a string identifying the traffic light
     * @return absolute time, counting from simulation start, at which TLS may change
     */

    public static SumoCommand getNextSwitch(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_NEXT_SWITCH, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param tlsID a string identifying the traffic light
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String tlsID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.VAR_PARAMETER, tlsID, array, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRING);
    }


    /**
     * Sets the chosen parameter
     *
     *  @param vehID a string identifying the vehicle
     *  @param param a string identifying the parameter
     *  @param value a string identifying the new value
     *
     * @return SumoCommand
     */

    public static SumoCommand setParameter(String tlsID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.VAR_PARAMETER, tlsID, array);
    }

    /**
     * Returns the index of the current phase in the current program.
     *
     * @param tlsID a string identifying the traffic light
     * @return index of the current phase
     */

    public static SumoCommand getPhase(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_CURRENT_PHASE, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the name of the current phase in the current program.
     *
     * @param tlsID a string identifying the traffic light
     * @return name of the current phase
     */

    public static SumoCommand getPhaseName(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.VAR_NAME, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the id of the current program.
     *
     * @param tlsID a string identifying the traffic light
     * @return index of the current program
     */

    public static SumoCommand getProgram(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_CURRENT_PROGRAM, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the named tl's state as a tuple of light definitions from rRgGyYoO, for red, green, yellow, off, where lower case letters mean that the stream has to decelerate.
     *
     * @param tlsID a string identifying the traffic light
     * @return SumoCommand
     */

    public static SumoCommand getRedYellowGreenState(String tlsID) {
        return new SumoCommand(Constants.CMD_GET_TL_VARIABLE, Constants.TL_RED_YELLOW_GREEN_STATE, tlsID, Constants.RESPONSE_GET_TL_VARIABLE, Constants.TYPE_STRING);
    }

    //setter methods

    /**
     * Set the complete traffic light program.
     *
     * @param tlsID a string identifying the traffic light
     * @param tls complete program definition
     * @return SumoCommand
     */

    public static SumoCommand setCompleteRedYellowGreenDefinition(String tlsID, SumoTLSProgram tls) {
        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.TL_COMPLETE_PROGRAM_RYG, tlsID, tls);
    }

    /**
     * Set the index of the current phase in the current program.
     *
     * @param tlsID a string identifying the traffic light
     * @param index an integer identifying the phase (it must be between 0 and the number of phases known to the current program of the tls - 1)
     * @return SumoCommand
     */

    public static SumoCommand setPhase(String tlsID, int index) {

        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.TL_PHASE_INDEX, tlsID, index);
    }

    /**
     * Set the name of the current phase in the current program.
     *
     * @param tlsID a string identifying the traffic light
     * @param name a string with the phase name
     * @return SumoCommand
     */

    public static SumoCommand setPhaseName(String tlsID, String name) {

        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.VAR_NAME, tlsID, name);
    }

    /**
     * Set the duration of the currently active phase (in s?).
     *
     * @param tlsID a string identifying the traffic light
     * @param phaseDuration remaining duration of the current phase
     * @return SumoCommand
     */

    public static SumoCommand setPhaseDuration(String tlsID, double phaseDuration) {

        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.TL_PHASE_DURATION, tlsID, phaseDuration);
    }

    /**
     * Set the id of the current program.
     *
     * @param tlsID a string identifying the traffic light
     * @param programID a string identifying the program
     * @return SumoCommand
     */

    public static SumoCommand setProgram(String tlsID, String programID) {
        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.TL_PROGRAM, tlsID, programID);
    }

    /**
     * Set the named tl's state as a tuple of light definitions.
     *
     * @param tlsID a string identifying the traffic light
     * @param state as a tuple of light definitions from rRgGyYoO, for red, green, yellow, off, where lower case letters mean that the stream has to decelerate
     * @return SumoCommand
     */

    public static SumoCommand setRedYellowGreenState(String tlsID, String state) {
        return new SumoCommand(Constants.CMD_SET_TL_VARIABLE, Constants.TL_RED_YELLOW_GREEN_STATE, tlsID, state);
    }

}
