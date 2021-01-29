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
/// @file    Junction.java
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

public class Junction {

    //getter methods

    /**
     * Returns a list of IDs of all junctions within the scenario.
     *
     * @return list of IDs of all junctions in the network
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_JUNCTION_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_JUNCTION_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the number of all junctions in the network.
     * @return the number of junctions
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_JUNCTION_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_JUNCTION_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the position of the named junction.
     *
     * @param junctionID
     *            a string identifying the junction
     * @return the coordinates of the center of the junction
     */

    public static SumoCommand getPosition(String junctionID) {
        return new SumoCommand(Constants.CMD_GET_JUNCTION_VARIABLE, Constants.VAR_POSITION, junctionID, Constants.RESPONSE_GET_JUNCTION_VARIABLE, Constants.POSITION_2D);
    }

    /**
    * getShape
    *@param junctionID
    * a string identifying the junction
    * @return the shape
    */

    public static SumoCommand getShape(String junctionID) {
        return new SumoCommand(Constants.CMD_GET_JUNCTION_VARIABLE, Constants.VAR_SHAPE, junctionID, Constants.RESPONSE_GET_JUNCTION_VARIABLE, Constants.TYPE_POLYGON);
    }


}
