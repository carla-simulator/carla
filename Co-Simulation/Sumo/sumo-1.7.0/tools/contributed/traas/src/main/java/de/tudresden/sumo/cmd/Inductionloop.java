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
/// @file    Inductionloop.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.cmd;
import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.util.SumoCommand;

/**
 *
 * @author Mario Krumnow
 * @author Evamarie Wiessner
 *
 */

public class Inductionloop {

    //getter methods

    /**
     * Returns a list of IDs of all induction loops within the scenario.
     *
     * @return list of IDs of all induction loops in the network
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_STRINGLIST);
    }


    /**
     * Returns the number of all induction loops in the network.
     * @return the number of detectors
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the chosen parameter
     *
     * @param personID a string personIDentifying the person
     *  @param param a string personIDentifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String loopID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.VAR_PARAMETER, loopID, array, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_STRING);
    }


    /**
     * Returns the ID of the lane the loop is placed at.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return a string identifying the lane
     */

    public static SumoCommand getLaneID(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.VAR_LANE_ID, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the mean length of vehicles (in m) which were on the detector in
     * the last step.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return average length of the vehicles (in m)
     */

    public static SumoCommand getLastStepMeanLength(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_LENGTH, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the mean speed of vehicles (in m/s) that were on the named
     * induction loop within the last simulation step.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return average speed of the vehicles (in m/s)
     */

    public static SumoCommand getLastStepMeanSpeed(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_MEAN_SPEED, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the percentage of time (in %) the detector was occupied by a
     * vehicle within the last time step.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return occupancy of the detector (in %)
     */

    public static SumoCommand getLastStepOccupancy(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_OCCUPANCY, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the list of IDs of vehicles that were on the named induction loop
     * in the last simulation step.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return list of all vehicleIDs
     */

    public static SumoCommand getLastStepVehicleIDs(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_VEHICLE_ID_LIST, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the number of vehicles that were on the named induction loop
     * within the last simulation step.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return total number of vehicles
     */

    public static SumoCommand getLastStepVehicleNumber(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_VEHICLE_NUMBER, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the position (in m) of the induction loop at it's lane, counted
     * from the lane's begin.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return position of the detector, counted from the lane's begin
     */

    public static SumoCommand getPosition(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.VAR_POSITION, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Return the time (in s) since last detection.
     *
     * @param loopID
     *            a string identifying the induction loop
     * @return time since last detection (in s)
     */

    public static SumoCommand getTimeSinceDetection(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_TIME_SINCE_DETECTION, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * A complex structure containing several information about vehicles which
     * passed the detector.
     *
     * @param loopID
     *            a SumoStringList containing induction loops
     * @return several information about vehicles which passed the detector.
     */

    public static SumoCommand getVehicleData(String loopID) {
        return new SumoCommand(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE, Constants.LAST_STEP_VEHICLE_DATA, loopID, Constants.RESPONSE_GET_INDUCTIONLOOP_VARIABLE, Constants.TYPE_COMPOUND);
    }

    //setter methods


}
