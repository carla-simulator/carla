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
/// @file    Lane.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.cmd;
import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.util.SumoCommand;
import de.tudresden.ws.container.SumoStringList;

/**
 *
 * @author Mario Krumnow
 * @author Evamarie Wiessner
 *
 */

public class Lane {

    //getter methods

    /**
     * Returns a list of allowed vehicle classes.
     *
     * @param laneID
     *            a string identifying a lane
     * @return list of allowed vehicle classes. An empty list means all vehicles
     *         are allowed.
     */

    public static SumoCommand getAllowed(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LANE_ALLOWED, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the number of all lanes in the network.
     * @return the number of lanes
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the CO2 emissions (in mg) on the given lane during the last time
     * step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the CO2 emissions (in mg)
     */
    public static SumoCommand getCO2Emission(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_CO2EMISSION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the CO emissions (in mg) on the given lane during the last time
     * step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the CO emissions (in mg)
     */

    public static SumoCommand getCOEmission(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_COEMISSION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns a list of disallowed vehicle classes.
     *
     * @param laneID
     *            a string identifying a lane
     * @return list of disallowed vehicle classes
     */

    public static SumoCommand getDisallowed(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LANE_DISALLOWED, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the ID of the edge this lane belongs to.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the id of the edge this lane belongs to
     */

    public static SumoCommand getEdgeID(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LANE_EDGE_ID, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the electricity consumption in ml for the last time step.
     *
     * @param laneID
     *            a string identifying a lane
     *
     * @return the energy consumption
     */

    public static SumoCommand getElectricityConsumption(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_ELECTRICITYCONSUMPTION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * Returns the ids of incoming lanes that have right of way over the connection from laneID to toLaneID
     *
     * @param laneID
     *            a string identifying a lane
     * @param toLane
     *            a string identifying the target lane
     * @return the energy consumption
     */

    public static SumoCommand getFoes(String laneID, String toLane) {
        Object[] array = new Object[] {toLane};
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_FOES, laneID, array, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the ids of internal lanes that are in conflict with the given internal lane id
     *
     * @param laneID
     *            a string identifying a lane
     * @return the energy consumption
     */

    public static SumoCommand getInternalFoes(String laneID) {
        Object[] array = new Object[] {""};
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_FOES, laneID, array, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRINGLIST);
    }



    /**
     * Returns the fuel consumption (in ml) on the given lane during the last
     * time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the fuel consumption (in ml)
     */

    public static SumoCommand getFuelConsumption(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_FUELCONSUMPTION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the HC emissions (in mg) on the given lane during the last time
     * step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the HC emissions (in mg)
     */

    public static SumoCommand getHCEmission(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_HCEMISSION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns a list of IDs of all lanes in the network.
     *
     * @return a list of IDs of all lanes
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the total number of halting vehicles on the given lane within the
     * last time step. A speed of less than 0.1 m/s is considered a halt.
     *
     * @param laneID
     *            a string identifying a lane
     * @return number of halting vehicles within the last time step
     */

    public static SumoCommand getLastStepHaltingNumber(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LAST_STEP_VEHICLE_HALTING_NUMBER, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the total vehicle length (in m) on the given lane during the last
     * time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return vehicle length (in m) within the last time step
     */

    public static SumoCommand getLastStepLength(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LAST_STEP_LENGTH, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the mean speed (in m/s) on the given lane within the last time
     * step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the average speed (in m/s) within the last time step
     */

    public static SumoCommand getLastStepMeanSpeed(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LAST_STEP_MEAN_SPEED, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the occupancy (in %) on the given lane during the last time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the percentage of time the lane was occupied
     */

    public static SumoCommand getLastStepOccupancy(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LAST_STEP_OCCUPANCY, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the IDs of the vehicles on the given lane within the last time
     * step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return list of IDs of vehicles
     */

    public static SumoCommand getLastStepVehicleIDs(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LAST_STEP_VEHICLE_ID_LIST, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns number of vehicles on this lane within the last time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return number of vehicles
     */

    public static SumoCommand getLastStepVehicleNumber(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LAST_STEP_VEHICLE_NUMBER, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the length (in m) of the named lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @return length of the named lane (in m)
     */

    public static SumoCommand getLength(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_LENGTH, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the number of links outgoing from this lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @return number of connections to successive lanes
     */

    public static SumoCommand getLinkNumber(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LANE_LINK_NUMBER, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_UBYTE);
    }

    /**
     * Returns descriptions of the links (in m) outgoing from this lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @return a list containing IDs of sucessor lanes
     */

    public static SumoCommand getLinks(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.LANE_LINKS, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_COMPOUND);
    }

    /**
     * Returns the maximum speed (in m/s) allowed on this lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @return maximum allowed speed (in m/s)
     */

    public static SumoCommand getMaxSpeed(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_MAXSPEED, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the NOx emissions (in mg) on the given lane during the last time
     * step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the NOx emissions (in mg)
     */

    public static SumoCommand getNOxEmission(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_NOXEMISSION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the noise emissions (in db) on the given lane during the last
     * time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the noise emissions (in mg)
     */

    public static SumoCommand getNoiseEmission(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_NOISEEMISSION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param laneID a string identifying the edge
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String laneID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_PARAMETER, laneID, array, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String laneID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_LANE_VARIABLE, Constants.VAR_PARAMETER, laneID, array);
    }

    /**
     * Returns the particular matter emissions (in mg) on the given lane during
     * the last time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return the particular matter emissions (in mg)
     */

    public static SumoCommand getPMxEmission(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_PMXEMISSION, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns this lane's shape.
     *
     * @param laneID
     *            a string identifying a lane
     * @return list of position describing the geometry
     */

    public static SumoCommand getShape(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_SHAPE, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_POLYGON);
    }

    /**
     * Returns the estimated travel time (in s) on the given lane during the
     * last time step.
     *
     * @param laneID
     *            a string identifying a lane
     * @return estimated travel time (in s)
     */

    public static SumoCommand getTraveltime(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_CURRENT_TRAVELTIME, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the width (in m) of the named lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @return width of lane (in m)
     */

    public static SumoCommand getWidth(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_WIDTH, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * getWaitingTime
      * @param laneID
     *            a string identifying a lane
     * @return the waiting time of the lane (in s)
     */

    public static SumoCommand getWaitingTime(String laneID) {
        return new SumoCommand(Constants.CMD_GET_LANE_VARIABLE, Constants.VAR_WAITING_TIME, laneID, Constants.RESPONSE_GET_LANE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    //setter methods

    /**
     * Set a list of allowed vehicle classes.
     *
     * @param laneID
     *            a string identifying a lane
     * @param allowedClasses
     *            allowed vehicle classes
     * @return SumoCommand
     */

    public static SumoCommand setAllowed(String laneID, SumoStringList allowedClasses) {

        return new SumoCommand(Constants.CMD_SET_LANE_VARIABLE, Constants.LANE_ALLOWED, laneID, allowedClasses);
    }

    /**
     * Set a list of disallowed vehicle classes.
     *
     * @param laneID
     *            a string identifying a lane
     * @param disallowedClasses
     *            not allowed vehicle classes
     * @return SumoCommand
     */

    public static SumoCommand setDisallowed(String laneID, SumoStringList disallowedClasses) {

        return new SumoCommand(Constants.CMD_SET_LANE_VARIABLE, Constants.LANE_DISALLOWED, laneID, disallowedClasses);
    }

    /**
     * Set the length (in m) of the named lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @param length
     *            new length (in m) of the lane
     * @return SumoCommand
     */

    public static SumoCommand setLength(String laneID, double length) {

        return new SumoCommand(Constants.CMD_SET_LANE_VARIABLE, Constants.VAR_LENGTH, laneID, length);
    }

    /**
     * Set the maximum speed (in m/s) allowed on this lane.
     *
     * @param laneID
     *            a string identifying a lane
     * @param speed
     *            maximum velocity allowed (in m/s)
     * @return SumoCommand
     */

    public static SumoCommand setMaxSpeed(String laneID, double speed) {

        return new SumoCommand(Constants.CMD_SET_LANE_VARIABLE, Constants.VAR_MAXSPEED, laneID, speed);
    }


}
