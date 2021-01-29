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
/// @file    Route.java
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

public class Route {

    //getter methods

    /**
     * Returns the IDs of the edges this route covers.
     *
     * @param routeID
     *            a string identifying the route
     * @return a list of IDs of the edges
     */

    public static SumoCommand getEdges(String routeID) {
        return new SumoCommand(Constants.CMD_GET_ROUTE_VARIABLE, Constants.VAR_EDGES, routeID, Constants.RESPONSE_GET_ROUTE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns a list of IDs of all currently loaded routes.
     *
     * @return a list of ID's of all routes
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_ROUTE_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_ROUTE_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the number of all Routes in the network.
     * @return the number of routes in the network
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_ROUTE_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_ROUTE_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the chosen parameter
     *
     *  @param routeID a string identifying the route
     *  @param param a string identifying the parameter
     *
     * @return the specific parameter
     */

    public static SumoCommand getParameter(String routeID, String param) {
        Object[] array = new Object[] {param};
        return new SumoCommand(Constants.CMD_GET_ROUTE_VARIABLE, Constants.VAR_PARAMETER, routeID, array, Constants.RESPONSE_GET_ROUTE_VARIABLE, Constants.TYPE_STRING);
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

    public static SumoCommand setParameter(String routeID, String param, String value) {
        Object[] array = new Object[] {param, value};
        return new SumoCommand(Constants.CMD_SET_ROUTE_VARIABLE, Constants.VAR_PARAMETER, routeID, array);
    }

    //setter methods

    /**
     * Add a new route.
     *
     * @param routeID
     *            a string identifying the route
     * @param edges
     *            list of edges the new route is following
     *  @return SumoCommand
     */

    public static SumoCommand add(String routeID, SumoStringList edges) {
        return new SumoCommand(Constants.CMD_SET_ROUTE_VARIABLE, Constants.ADD, routeID, edges);
    }


}
