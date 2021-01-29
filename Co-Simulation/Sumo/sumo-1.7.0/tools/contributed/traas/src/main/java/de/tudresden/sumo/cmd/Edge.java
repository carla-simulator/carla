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
/// @file    Edge.java
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

public class Edge {


    /**
     * Returns the travel time value (in s) used for (re-)routing.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param time
     *            the point of time the returned value is valid for
     * @return the travel time value (in s)
     */

    public static SumoCommand getAdaptedTraveltime(String edgeID, double time) {
        Object[] array = new Object[] {time};
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_EDGE_TRAVELTIME, edgeID, array, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the CO2 emissions (in mg) on the given edge during the last time
     * step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the CO2 emissions (in mg)
     */

    public static SumoCommand getCO2Emission(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_CO2EMISSION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the CO emissions (in mg) on the given edge during the last time
     * step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the CO emissions (in mg)
     */

    public static SumoCommand getCOEmission(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_COEMISSION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the effort value used for (re-)routing.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param time
     *            the point of time the returned value is valid for
     * @return the edge effort value (in s), -1 is returned if such a value does
     *         not exist
     */

    public static SumoCommand getEffort(String edgeID, double time) {
        Object[] array = new Object[] {time};
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_EDGE_EFFORT, edgeID, array, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * Returns the electricity consumption in ml for the last time step.
     *
     * @param edgeID
     *            a string identifying the edge
     *
     * @return the edge effort value (in s), -1 is returned if such a value does
     *         not exist
     */

    public static SumoCommand getElectricityConsumption(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_ELECTRICITYCONSUMPTION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the fuel consumption (in ml) on the given edge during the last
     * time step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the fuel consumption (in ml)
     */

    public static SumoCommand getFuelConsumption(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_FUELCONSUMPTION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the HC emissions (in mg) on the given edge during the last time
     * step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the HC emissions (in mg)
     */

    public static SumoCommand getHCEmission(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_HCEMISSION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the number of edges in the network.
     *
     * @return the number of edges
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns a list of all edges in the network.
     *
     * @return a list of IDs of all edges
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     *
     * @param edgeID
     *            a string identifying the edge
     * @return Returns the number of lanes of this edge
     */

    public static SumoCommand getLaneNumber(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_LANE_INDEX, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the total number of halting vehicles on the given edge within the
     * last time step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return number of halting vehicle, a speed of less than 0.1 m/s is
     *         considered a halt
     */

    public static SumoCommand getLastStepHaltingNumber(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_VEHICLE_HALTING_NUMBER, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the total vehicle length (in m) on the given edge within the last
     * time step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return total vehicle length (in m)
     */

    public static SumoCommand getLastStepLength(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_LENGTH, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the mean speed of vehicles (in m/s) that were on the named
     * induction loop within the last simulation step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the average speed (in m/s)
     */

    public static SumoCommand getLastStepMeanSpeed(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_MEAN_SPEED, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the ids of the persons on the given edge during the last time step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the average speed (in m/s)
     */

    public static SumoCommand getLastStepPersonIDs(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_PERSON_ID_LIST, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the percentage of time (in %) the detector was occupied by a
     * vehicle.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the occupancy (in %)
     */

    public static SumoCommand getLastStepOccupancy(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_OCCUPANCY, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the list of IDs of the vehicles on the given edge within the last
     * simulation step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the IDs of the vehicles
     */

    public static SumoCommand getLastStepVehicleIDs(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_VEHICLE_ID_LIST, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the total number of vehicles on this edge within the last time
     * step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the total number of vehicles
     */

    public static SumoCommand getLastStepVehicleNumber(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.LAST_STEP_VEHICLE_NUMBER, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the NOx emissions (in mg) on the given edge during the last time
     * step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the NOx emissions (in mg)
     */

    public static SumoCommand getNOxEmission(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_NOXEMISSION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the noise emissions (in db)on the given edge within the last time
     * step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the noise emissions (in db)
     */

    public static SumoCommand getNoiseEmission(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_NOISEEMISSION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the particular matter emissions (in mg) on the given edge on this
     * edge during the last time step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the particular matter emissions(in mg)
     */

    public static SumoCommand getPMxEmission(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_PMXEMISSION, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the estimated travel time (in s) on the given edge on this edge
     * within the last time step.
     *
     * @param edgeID
     *            a string identifying the edge
     * @return the estimated travel time
     */

    public static SumoCommand getTraveltime(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_CURRENT_TRAVELTIME, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     *Returns the waiting time (in s) on the given edge on this edge
     * within the last time step.
     *  @param edgeID
     *            a string identifying the edge
     * @return the waiting time
     */

    public static SumoCommand getWaitingTime(String edgeID) {
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_WAITING_TIME, edgeID, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param edgeID a string identifying the edge
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String edgeID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_EDGE_VARIABLE, Constants.VAR_PARAMETER, edgeID, array, Constants.RESPONSE_GET_EDGE_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String edgeID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_EDGE_VARIABLE, Constants.VAR_PARAMETER, edgeID, array);
    }

    //setter methods

    /**
     * Adapt the travel time value (in s) used for (re-)routing on the given
     * edge.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param time
     *            travel time value (in s)
     * @return SumoCommand
     */

    public static SumoCommand adaptTraveltime(String edgeID, double time) {
        return new SumoCommand(Constants.CMD_SET_EDGE_VARIABLE, Constants.VAR_EDGE_TRAVELTIME, edgeID, time);
    }


    /**
     * Adapt the travel time value (in s) used for (re-)routing on the given
     * edge.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param time
     *            travel time value (in s)
     * @param begin
     *            begin value
     * @param end
     *            end value
     * @return SumoCommand
     */

    public static SumoCommand adaptTraveltime(String edgeID, double time, double begin, double end) {
        Object[] array = new Object[] {begin, end, time};
        return new SumoCommand(Constants.CMD_SET_EDGE_VARIABLE, Constants.VAR_EDGE_TRAVELTIME, edgeID, array);
    }

    /**
     * Adapt the effort value used for (re-)routing on the given edge.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param effort
     *            effort value
     * @return SumoCommand
     */

    public static SumoCommand setEffort(String edgeID, double effort) {
        Object[] array = new Object[] {effort};
        return new SumoCommand(Constants.CMD_SET_EDGE_VARIABLE, Constants.VAR_EDGE_EFFORT, edgeID, array);
    }

    /**
     * Adapt the effort value used for (re-)routing on the given edge.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param effort
     *            effort value
     * @param begin
     *            begin value
     * @param end
     *            end value
     * @return SumoCommand
     */

    public static SumoCommand setEffort(String edgeID, double effort, double begin, double end) {
        Object[] array = new Object[] {begin, end, effort};
        return new SumoCommand(Constants.CMD_SET_EDGE_VARIABLE, Constants.VAR_EDGE_EFFORT, edgeID, array);
    }

    /**
     * Set a new maximum speed (in m/s) for all lanes of the edge.
     *
     * @param edgeID
     *            a string identifying the edge
     * @param speed
     *            maximum speed (in m/s)
     * @return SumoCommand

     */

    public static SumoCommand setMaxSpeed(String edgeID, double speed) {

        return new SumoCommand(Constants.CMD_SET_EDGE_VARIABLE, Constants.VAR_MAXSPEED, edgeID, speed);
    }


}
