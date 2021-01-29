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
/// @file    Person.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.cmd;
import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.util.SumoCommand;
import de.tudresden.ws.container.SumoColor;
import de.tudresden.ws.container.SumoStringList;

/**
 *
 * @author Mario Krumnow
 *
 */

public class Person {

    //setter methods
    /**
      Inserts a new person to the simulation at the given edge, position and
      time (in s). This function should be followed by appending Stages or the person
       will immediatly vanish on departure.
     * @param personID a string personIDentifying the person
     * @param edgeID edgeID
     * @param pos pos
     * @param depart depart
     * @param typeID typeID

    */

    public static SumoCommand add(String personID, String edgeID, double pos, double depart, String typeID) {
        Object[] array = new Object[] {typeID, edgeID, depart, pos};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.ADD, personID, array);
    }


    /**
      	Appends a waiting stage with duration in s to the plan of the given person
     * @param personID a string personIDentifying the person
     * @param duration duration
     * @param description description
     * @param stopID stopID
    */

    public static SumoCommand appendWaitingStage(String personID, double duration, String description, String stopID) {
        Object[] array = new Object[] {Constants.STAGE_WAITING, duration, description, stopID};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.APPEND_STAGE, personID, array);
    }


    /**
    	Appends a walking stage to the plan of the given person
        The walking speed can either be specified, computed from the duration parameter (in s) or taken from the type of the person

     * @param personID a string personIDentifying the person
     * @param edges edges
     * @param arrivalPos arrivalPos
     * @param duration duration
     * @param speed speed
     * @param stopID stopID
     */

    public static SumoCommand appendWalkingStage(String personID, SumoStringList edges, double arrivalPos, double duration, double speed, String stopID) {
        Object[] array = new Object[] {Constants.STAGE_WALKING, edges, arrivalPos, duration, speed, stopID};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.APPEND_STAGE, personID, array);
    }

    /**
    	Appends a driving stage to the plan of the given person
        The lines parameter should be a space-separated list of line ids

     * @param personID a string personIDentifying the person
     * @param toEdge toEdge
     * @param lines lines
     * @param stopID stopID
     */

    public static SumoCommand appendDrivingStage(String personID, String toEdge, String lines, String stopID) {
        Object[] array = new Object[] {Constants.STAGE_DRIVING, toEdge, lines, stopID};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.APPEND_STAGE, personID, array);
    }

    /**
    	Removes the nth next stage
        nextStageIndex must be lower then value of getRemainingStages(personID)
        nextStageIndex 0 immediately aborts the current stage and proceeds to the next stage
     * @param personID a string personIDentifying the person
     * @param nextStageIndex nextStageIndex
     */

    public static SumoCommand removeStage(String personID, int nextStageIndex) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.REMOVE_STAGE, personID, nextStageIndex);
    }

    /**
    Sets the maximum speed in m/s for the named person for subsequent step.
     * @param personID a string personIDentifying the person
     * @param speed speed
    */

    public static SumoCommand setSpeed(String personID, double speed) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_SPEED, personID, speed);
    }

    /**
    	Sets the id of the type for the named person.
     * @param personID a string personIDentifying the person
     * @param typeID typeID
    */

    public static SumoCommand setType(String personID, String typeID) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_TYPE, personID, typeID);
    }

    /**
    	Sets the width in m for this person.
     * @param personID a string personIDentifying the person
     * @param width width
     */

    public static SumoCommand setWidth(String personID, double width) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_WIDTH, personID, width);
    }


    /**
    Sets the height in m for this person.
     * @param personID a string personIDentifying the person
     * @param height height
     */

    public static SumoCommand setHeight(String personID, double height) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_HEIGHT, personID, height);
    }

    /**
    	Sets the length in m for the given person.
     * @param personID a string personIDentifying the person
     * @param length length
     */

    public static SumoCommand setLength(String personID, double length) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_LENGTH, personID, length);
    }

    /**
     Sets the offset (gap to front person if halting) for this vehicle.

     * @param personID a string personIDentifying the person
     * @param minGap minGap
     */

    public static SumoCommand setMinGap(String personID, double minGap) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_MINGAP, personID, minGap);
    }

    /**
    	sets color for person with the given ID.
        i.e. (255,0,0,0) for the color red.
        The fourth integer (alpha) is only used when drawing persons with raster images
      * @param personID a string personIDentifying the person
      * @param color color
     */

    public static SumoCommand setColor(String personID, SumoColor color) {
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_COLOR, personID, color);
    }

    //getter methods

    /**
     * get lane position
     * @param personID a string personIDentifying the person
     * @return The position of the person along the lane measured in m.
     */

    public static SumoCommand getColor(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_COLOR, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_COLOR);
    }


    /**
     * get length
     * @param personID a string personIDentifying the person
     * @return Returns the length in m of the given person.
     */

    public static SumoCommand getLength(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_LENGTH, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * get waiting time
     *
     * @param personID a string personIDentifying the person
     * @return The waiting time of a person is defined as the time (in seconds) spent with a
        speed below 0.1m/s since the last time it was faster than 0.1m/s.
        (basically, the waiting time of a person is reset to 0 every time it moves).
     */

    public static SumoCommand getWaitingTime(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_WAITING_TIME, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * get width
     *
     * @param personID a string personIDentifying the person
     * @return Returns the width in m of this person.
     */

    public static SumoCommand getWidth(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_WIDTH, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * get next edge
     *
     * @param personID a string personIDentifying the person
     * @return If the person is walking, returns the next edge on the persons route
        (including crossing and walkingareas). If there is no further edge or the
        person is in another stage, returns the empty string.
     */

    public static SumoCommand getNextEdge(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_MINGAP, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * get edges
     *
     * @param personID a string personIDentifying the person
     * @return If the person is walking, returns the next edge on the persons route
        (including crossing and walkingareas). If there is no further edge or the
        person is in another stage, returns the empty string.
     */

    public static SumoCommand getEdges(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_NEXT_EDGE, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * get edges
     *
     * @param personID a string personIDentifying the person
     * @param nextStageIndex
     * @return Returns a list of all edges in the next stage.
        For waiting stages this is a single edge
        For walking stages this is the complete route
        For driving stages this is [origin, destination]

        nextStageIndex 0 retrieves value for the current stage.
        nextStageIndex must be lower then value of getRemainingStages(personID)
     */

    public static SumoCommand getEdges(String personID, int nextStageIndex) {
        Object[] array = new Object[] {nextStageIndex};
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_EDGES, personID, array, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRINGLIST);
    }


    /**
     * get stage
     *
     * @param personID a string personIDentifying the person
     * @param nextStageIndex
     * @return Returns the type of the nth next stage
          0 for not-yet-departed
          1 for waiting
          2 for walking
          3 for driving
        nextStageIndex 0 retrieves value for the current stage.
        nextStageIndex must be lower then value of getRemainingStages(personID)
     */

    public static SumoCommand getStage(String personID, int nextStageIndex) {
        Object[] array = new Object[] {nextStageIndex};
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_STAGE, personID, array, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_COMPOUND);
    }

    /**
     * get remaining stages
     *
     * @param personID a string personIDentifying the person
     * @return Returns the number of remaining stages (at least 1)
     */

    public static SumoCommand getRemainingStages(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_STAGES_REMAINING, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * get vehicle
     *
     * @param personID a string personIDentifying the person
     * @return Returns the id of the current vehicle if the person is in stage driving
        and has entered a vehicle.
        Return the empty string otherwise
     */

    public static SumoCommand getVehicle(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_VEHICLE, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * get lane position
     * @param personID a string personIDentifying the person
     * @return The position of the person along the lane measured in m.
     */

    public static SumoCommand getLanePosition(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_LANEPOSITION, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }



    /**
     * Returns the number of all persons in the network.
     * @return the number of persons in the network
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_INTEGER);
    }


    /**
     * Returns a list of personIDs of all persons
     * @return list of IDs of all persons
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRINGLIST);
    }


    /**
     * Returns the chosen parameter
     *
     * @param personID a string personIDentifying the person
     *  @param param a string personIDentifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String personID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_PARAMETER, personID, array, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String personID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.VAR_PARAMETER, personID, array);
    }

    /**
     * get speed
     *
     * @param personID a string personIDentifying the person
     * @return the speed in m/s
     */

    public static SumoCommand getSpeed(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_SPEED, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * get position
     *
     * @param personID a string personIDentifying the person
     * @return the position 2D
     */

    public static SumoCommand getPosition(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_POSITION, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.POSITION_2D);
    }

    /**
     * get position3D
     *
     * @param personID a string personIDentifying the person
     * @return the position 3D
     */

    public static SumoCommand getPosition3D(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_POSITION3D, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.POSITION_3D);
    }

    /**
     * get angle
     *
     * @param personID a string personIDentifying the person
     * @return the angle
     */

    public static SumoCommand getAngle(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_ANGLE, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * get road ID
     *
     * @param personID a string personIDentifying the person
     * @return the personID of the road
     */

    public static SumoCommand getRoadID(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_ROAD_ID, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * get type ID
     *
     * @param personID a string personIDentifying the person
     * @return the type ID of the person
     */

    public static SumoCommand getTypeID(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_TYPE, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_STRING);
    }




    /**
     * get person number
     *
     * @param personID a string personIDentifying the person
     * @return the person number
     */

    public static SumoCommand getPersonNumber(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_PERSON_NUMBER, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_INTEGER);
    }


    /**
     * get minGap
     *
     * @param personID a string personIDentifying the person
     * @return the value for the minimum Ggap of the person
     */

    public static SumoCommand getMinGap(String personID) {
        return new SumoCommand(Constants.CMD_GET_PERSON_VARIABLE, Constants.VAR_MINGAP, personID, Constants.RESPONSE_GET_PERSON_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * rerouteTraveltime
     *
     * Computes a new route for the person using the global edge traveltime information.
     *
     * @param personID a string personIDentifying the person
     * @return SumoComand
     */

    public static SumoCommand rerouteTraveltime(String personID) {
        Object[] array = new Object[] {personID};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.CMD_REROUTE_TRAVELTIME, personID, array);
    }

    /**
     * Place person at the given x,y coordinates and force it's angle to
        the given value (for drawing).
        If the angle is set to INVALID_DOUBLE_VALUE, the vehicle assumes the
        natural angle of the edge on which it is driving.
        If keepRoute is set to 1, the closest position
        within the existing route is taken. If keepRoute is set to 0, the vehicle may move to
        any edge in the network but it's route then only consists of that edge.
        If keepRoute is set to 2 the person has all the freedom of keepRoute=0
        but in addition to that may even move outside the road network.
        edgeID is an optional placement hint to resolve ambiguities

     * @param personID person id
     * @param edgeID edge id
     * @param x x
     * @param y y
     * @param angle angle
     * @param  keepRoute keepRoute
     * @return SumoCommand
     */
    public static SumoCommand moveToXY(String personID, String edgeID, double x, double y, double angle, byte keepRoute) {

        Object[] array = new Object[] {edgeID,  x, y, angle, keepRoute};
        return new SumoCommand(Constants.CMD_SET_PERSON_VARIABLE, Constants.MOVE_TO_XY, personID, array);
    }

}
