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
/// @file    Vehicletype.java
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

/**
 *
 * @author Mario Krumnow
 * @author Evamarie Wiessner
 *
 */

public class Vehicletype {

    //getter methods

    /**
     * Returns the maximum acceleration of the named vehicle class.
     * @param typeID  type id
     * @return acceleration
     */
    public static SumoCommand getAccel(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_ACCEL, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the number of all vehicle types in the network.
     * @return number of vehicle types
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     *  Returns the vehicle type's color.
     * @param typeID  type id
     * @return color color
     */
    public static SumoCommand getColor(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_COLOR, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_COLOR);
    }

    /**
     * Returns the maximum deceleration of the names vehicle type.
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getDecel(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_DECEL, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     *  Returns the maximal physically possible deceleration in m/s^2 of vehicles of this type.
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getEmergencyDecel(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_EMERGENCY_DECEL, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns The preferred lateral alignment of the type
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getLateralAlignment(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_LATALIGNMENT, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the height in m of vehicles of this type.
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getHeight(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_HEIGHT, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * Returns The preferred lateral alignment of the type
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getApparentDecel(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_APPARENT_DECEL, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     *  Returns The desired lateral gap of this type at 50km/h in m
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getMinGapLat(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_MINGAP_LAT, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }


    /**
     * Returns the maximum lateral speed in m/s of this type.
     * @param typeID  type id
     * @return deceleration
     */

    public static SumoCommand getMaxSpeedLat(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_MAXSPEED_LAT, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the emission class of the named vehicle type.
     * @param typeID  type id
     * @return emission class
     */

    public static SumoCommand getEmissionClass(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_EMISSIONCLASS, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns a list of all known vehicle types.
     * @return list of vechicle types
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * getImperfection
     * @param typeID type id
     * @return imperfection
     */

    public static SumoCommand getImperfection(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_IMPERFECTION, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the length of the named vehicle type.
     * @param typeID type id
     * @return length
     */

    public static SumoCommand getLength(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_LENGTH, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Return the maximum speed of the named vehicle type.
     * @param typeID type id
     * @return max speed
     */

    public static SumoCommand getMaxSpeed(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_MAXSPEED, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the minimum headway gap of the named vehicle type.
     * @param typeID type id
     * @return min gap
     */

    public static SumoCommand getMinGap(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_MINGAP, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param typeID a string identifying the vehicle type
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String typeID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_PARAMETER, typeID, array, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String typeID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_PARAMETER, typeID, array);
    }

    /**
     * getShapeClass
     * @param typeID type id
     * @return shape class
     */

    public static SumoCommand getShapeClass(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_SHAPECLASS, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the speed deviation of the named vehicle type.
     * @param typeID type id
     * @return speed deviation
     */

    public static SumoCommand getSpeedDeviation(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_SPEED_DEVIATION, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Return the speed factor of the names vehicle type.
     * @param typeID type id
     * @return speed factor
     */

    public static SumoCommand getSpeedFactor(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_SPEED_FACTOR, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * getTau
     * @param typeID type id
     * @return tau
     */

    public static SumoCommand getTau(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_TAU, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the vehicle class of the names vehicle type.
     * @param typeID type id
     * @return vehicle class
     */

    public static SumoCommand getVehicleClass(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_VEHICLECLASS, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the width of the named vehicle type.
     * @param typeID type id
     * @return width
     */

    public static SumoCommand getWidth(String typeID) {
        return new SumoCommand(Constants.CMD_GET_VEHICLETYPE_VARIABLE, Constants.VAR_WIDTH, typeID, Constants.RESPONSE_GET_VEHICLETYPE_VARIABLE, Constants.TYPE_DOUBLE);
    }

    //setter methods

    /**
     * Sets the vehicle type's wished maximum acceleration to the given value.
     * @param typeID type id
     * @param accel acceleration
     * @return SumoCommand
     */
    public static SumoCommand setAccel(String typeID, double accel) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_ACCEL, typeID, accel);
    }


    /**
     * Sets the vehicle type's color.
     * @param typeID type id
     * @param color color
     * @return SumoCommand
     */
    public static SumoCommand setColor(String typeID, SumoColor color) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_COLOR, typeID, color);
    }

    /**
     * Sets the vehicle type's wished maximum deceleration to the given value.

     * @param typeID type id
     * @param decel deceleration
     * @return SumoCommand
     */
    public static SumoCommand setDecel(String typeID, double decel) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_DECEL, typeID, decel);
    }

    /**
     * Sets the vehicle type's emission class to the given value.
     * @param typeID type id
     * @param clazz emission class
     * @return SumoCommand
     */
    public static SumoCommand setEmissionClass(String typeID, String clazz) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_EMISSIONCLASS, typeID, clazz);
    }

    /**
     * setImperfection
     * @param typeID type id
     * @param minGapLat minGapLat
     * @return SumoCommand
     */
    public static SumoCommand setMinGapLat(String typeID, double minGapLat) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_MINGAP_LAT, typeID, minGapLat);
    }

    /**
     * Sets the maximum lateral speed of this type.
     * @param typeID type id
     * @param speed speed
     * @return SumoCommand
     */
    public static SumoCommand setMaxSpeedLat(String typeID, double speed) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_MAXSPEED_LAT, typeID, speed);
    }

    /**
     * Sets the apparent deceleration in m/s^2 of vehicles of this type.
     * @param typeID type id
     * @param decel decel
     * @return SumoCommand
     */
    public static SumoCommand setApparentDecel(String typeID, double decel) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_APPARENT_DECEL, typeID, decel);
    }

    /**
     * setImperfection
     * @param typeID type id
     * @param imperfection imperfection
     * @return SumoCommand
     */
    public static SumoCommand setImperfection(String typeID, double imperfection) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_IMPERFECTION, typeID, imperfection);
    }

    /**
     * Sets the preferred lateral alignment of this type.
     * @param typeID type id
     * @param latAlignment latAlignment
     * @return SumoCommand
     */
    public static SumoCommand setLateralAlignment(String typeID, String latAlignment) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_LATALIGNMENT, typeID, latAlignment);
    }

    /**
     * Sets the height in m of vehicles of this type.
     * @param typeID type id
     * @param height height
     * @return SumoCommand
     */
    public static SumoCommand setHeight(String typeID, double height) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_HEIGHT, typeID, height);
    }



    /**
     *  Sets the maximal physically possible deceleration in m/s^2 of vehicles of this type.
     * @param typeID type id
     * @param decel decel
     * @return SumoCommand
     */
    public static SumoCommand setEmergencyDecel(String typeID, double decel) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_EMERGENCY_DECEL, typeID, decel);
    }

    /**
     * Sets the vehicle type's length to the given value.
     * @param typeID type id
     * @param length length
     * @return SumoCommand
     */
    public static SumoCommand setLength(String typeID, double length) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_LENGTH, typeID, length);
    }

    /**
     * Sets the vehicle type's maximum speed to the given value.
     * @param typeID type id
     * @param speed speed
     * @return SumoCommand
     */

    public static SumoCommand setMaxSpeed(String typeID, double speed) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_MAXSPEED, typeID, speed);
    }

    /**
     * Sets the vehicle type's minimum headway gap to the given value.
     * @param typeID type id
     * @param minGap minimum gap
     * @return SumoCommand
     */
    public static SumoCommand setMinGap(String typeID, double minGap) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_MINGAP, typeID, minGap);
    }

    /**
     * setShapeClass
     * @param typeID type id
     * @param clazz shape class
     * @return SumoCommand
     */
    public static SumoCommand setShapeClass(String typeID, String clazz) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_SHAPECLASS, typeID, clazz);
    }

    /**
     * Sets the vehicle type's speed deviation to the given value.
     * @param typeID type id
     * @param deviation deviation
     * @return SumoCommand
     */
    public static SumoCommand setSpeedDeviation(String typeID, double deviation) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_SPEED_DEVIATION, typeID, deviation);
    }

    /**
     * Sets the vehicle type's speed factor to the given value.
     * @param typeID type id
     * @param factor factor
     * @return SumoCommand
     */
    public static SumoCommand setSpeedFactor(String typeID, double factor) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_SPEED_FACTOR, typeID, factor);
    }

    /**
     * setTau
     * @param typeID type id
     * @param tau tau
     * @return SumoCommand
     */
    public static SumoCommand setTau(String typeID, double tau) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_TAU, typeID, tau);
    }

    /**
     * Sets the vehicle type's vehicle class to the given value.
     * @param typeID type id
     * @param clazz vehicle class
     * @return SumoCommand
     */
    public static SumoCommand setVehicleClass(String typeID, String clazz) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_VEHICLECLASS, typeID, clazz);
    }

    /**
     * Sets the vehicle type's width to the given value.
     * @param typeID type id
     * @param width width
     * @return SumoCommand
     */
    public static SumoCommand setWidth(String typeID, double width) {

        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.VAR_WIDTH, typeID, width);
    }

    /**
     *  Duplicates the vType with ID origTypeID. The newly created vType is assigned the ID newTypeID
     * @param typeID type id
     * @param newTypeID newTypeID
     * @return SumoCommand
     */
    public static SumoCommand setWidth(String typeID, String newTypeID) {
        return new SumoCommand(Constants.CMD_SET_VEHICLETYPE_VARIABLE, Constants.COPY, typeID, newTypeID);
    }


}
