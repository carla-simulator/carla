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
/// @file    Poi.java
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

public class Poi {

    //getter methods

    /**
     * Add a new point-of-interest.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @param x
     *            x-coordinate of the point
     * @param y
     *            y-coordinate of the point
     * @param color
     *            value (r,g,b,a) of color
     * @param poiType
     *            a string identifying the type of a poi
     * @param layer
     *            an integer identifying the layer
     *
     * @return SumoCommand
     */

    public static SumoCommand add(String poiID, double x, double y, SumoColor color, String poiType, int layer) {
        Object[] array = new Object[] {x, y, color, poiType, layer};
        return new SumoCommand(Constants.CMD_SET_POI_VARIABLE, Constants.ADD, poiID, array);
    }

    /**
     * Returns the number of all Poi's in the network.
     * @return the number of POI's in the network
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_POI_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_POI_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the color of this poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @return color value
     */

    public static SumoCommand getColor(String poiID) {
        return new SumoCommand(Constants.CMD_GET_POI_VARIABLE, Constants.VAR_COLOR, poiID, Constants.RESPONSE_GET_POI_VARIABLE, Constants.TYPE_COLOR);
    }

    /**
     * Returns a list of IDs of all poi.
     *
     * @return a list of IDs of all points of interest
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_POI_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_POI_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param poiID a string identifying the poi
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String poiID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_POI_VARIABLE, Constants.VAR_PARAMETER, poiID, array, Constants.RESPONSE_GET_POI_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String poiID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_POI_VARIABLE, Constants.VAR_PARAMETER, poiID, array);
    }


    /**
     * Returns the position of this poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @return position of the point
     */

    public static SumoCommand getPosition(String poiID) {
        return new SumoCommand(Constants.CMD_GET_POI_VARIABLE, Constants.VAR_POSITION, poiID, Constants.RESPONSE_GET_POI_VARIABLE, Constants.POSITION_2D);
    }

    /**
     * Returns the type of the poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @return type of the point
     */

    public static SumoCommand getType(String poiID) {
        return new SumoCommand(Constants.CMD_GET_POI_VARIABLE, Constants.VAR_TYPE, poiID, Constants.RESPONSE_GET_POI_VARIABLE, Constants.TYPE_STRING);
    }

    //setter methods

    /**
     * Remove a poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @param layer
     *            an integer identifying the layer
     * @return SumoCommand
     */

    public static SumoCommand remove(String poiID, int layer) {

        return new SumoCommand(Constants.CMD_SET_POI_VARIABLE, Constants.REMOVE, poiID, layer);
    }

    /**
     * Set the color of this poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @param color
     *            value (r,g,b,a) of color
     *  @return SumoCommand
     */

    public static SumoCommand setColor(String poiID, SumoColor color) {

        return new SumoCommand(Constants.CMD_SET_POI_VARIABLE, Constants.VAR_COLOR, poiID, color);
    }

    /**
     * Set the position of this poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @param x
     *            x-coordinate of the point
     * @param y
     *            y-coordinate of the point
     * @return SumoCommand
     */

    public static SumoCommand setPosition(String poiID, double x, double y) {

        Object[] array = new Object[] {x, y};
        return new SumoCommand(Constants.CMD_SET_POI_VARIABLE, Constants.VAR_POSITION, poiID, array);
    }

    /**
     * Set the type of the poi.
     *
     * @param poiID
     *            a string identifying the point-of-interest
     * @param poiType
     *            a string identifying the type of a poi
     * @return SumoCommand
     */

    public static SumoCommand setType(String poiID, String poiType) {

        return new SumoCommand(Constants.CMD_SET_POI_VARIABLE, Constants.VAR_TYPE, poiID, poiType);
    }


}
