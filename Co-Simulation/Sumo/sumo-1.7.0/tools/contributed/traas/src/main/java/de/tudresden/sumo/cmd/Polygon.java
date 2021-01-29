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
/// @file    Polygon.java
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
import de.tudresden.ws.container.SumoGeometry;
import de.tudresden.ws.container.SumoStringList;

/**
 *
 * @author Mario Krumnow
 * @author Evamarie Wiessner
 *
 */

public class Polygon {

    //getter methods

    /**
     * Returns the color of this polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @return color of the polygon
     */

    public static SumoCommand getColor(String polygonID) {
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.VAR_COLOR, polygonID, Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_COLOR);
    }

    /**
     * Returns a list of IDs of all polygons.
     *
     * @return a list of IDs of all polygons
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_STRINGLIST);
    }


    /**
     * Returns the number of all Polygons in the network.
     * @return the number of polygons in the network
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param polygonID a string identifying the polygon
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String polygonID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.VAR_PARAMETER, polygonID, array, Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String polygonID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.VAR_PARAMETER, polygonID, array);
    }

    /**
     * Returns the shape of this polygon.
     *
     * @param polygonID
     *            a string identifying the polygon return the shape of the
     *            polygon
     * @return the shape
     */

    public static SumoCommand getShape(String polygonID) {
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.VAR_SHAPE, polygonID, Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_POLYGON);
    }

    /**
     * Returns the shape of this polygon.
     *
     * @param polygonID
     *            a string identifying the polygon return the shape of the
     *            polygon
     * @return the shape
     */

    public static SumoCommand getFilled(String polygonID) {
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.VAR_FILL, polygonID, Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_BYTE);
    }


    /**
     * Returns the type of the polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @return type of the polygon
     */

    public static SumoCommand getType(String polygonID) {
        return new SumoCommand(Constants.CMD_GET_POLYGON_VARIABLE, Constants.VAR_TYPE, polygonID, Constants.RESPONSE_GET_POLYGON_VARIABLE, Constants.TYPE_STRING);
    }

    //setter methods

    /**
     * Add a new polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @param shape
     *            shape of the polygon
     * @param color
     *            value (r,g,b,a) of color
     * @param fill
     *            polygon is filled if the value is != 0
     * @param polygonType
     *            type of the polygon
     * @param layer
     *            an integer identifying the layer
     * @return SumoCommand
     */

    public static SumoCommand add(String polygonID, SumoGeometry shape, SumoColor color, boolean fill, String polygonType, int layer) {
        Object[] array = new Object[] {shape, color, fill, polygonType, layer};
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.ADD, polygonID, array);
    }

    /**
     * Remove a polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @param layer
     *            an integer identifying the layer
     * @return SumoCommand
     */

    public static SumoCommand remove(String polygonID, int layer) {
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.REMOVE, polygonID, layer);
    }

    /**
     * Set the color of this polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @param color
     *            value (r,g,b,a) of color
     * @return SumoCommand
     */

    public static SumoCommand setColor(String polygonID, SumoColor color) {
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.VAR_COLOR, polygonID, color);
    }


    /**
     * Set the color of this polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @param fill parameter if polygon should be filled
     * @return SumoCommand
     */

    public static SumoCommand setFilled(String polygonID, boolean fill) {
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.VAR_FILL, polygonID, fill);
    }

    /**
     * Set the shape of this polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @param shape
     *            shape of the polygon
     * @return SumoCommand
     */

    public static SumoCommand setShape(String polygonID, SumoStringList shape) {
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.VAR_SHAPE, polygonID, shape);
    }

    /**
     * Set the type of the polygon.
     *
     * @param polygonID
     *            a string identifying the polygon
     * @param polygonType
     *            type of the polygon
     * @return SumoCommand
     */

    public static SumoCommand setType(String polygonID, String polygonType) {
        return new SumoCommand(Constants.CMD_SET_POLYGON_VARIABLE, Constants.VAR_TYPE, polygonID, polygonType);
    }


}
