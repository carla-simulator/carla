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
/// @file    Multientryexit.java
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

public class Multientryexit {

    //getter methods

    /**
     * Returns a list of IDs of all multi-entry/multi-exit detectors within the
     * scenario.
     *
     * @return list of IDs of all detectors
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_MULTIENTRYEXIT_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the number of all multi-entry/multi-exit detectors in the network.
     * @return the number of detectors
     */

    public static SumoCommand getIDCount() {
        return new SumoCommand(Constants.CMD_GET_MULTIENTRYEXIT_VARIABLE, Constants.ID_COUNT, "", Constants.RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the number of vehicles which were halting during the last time
     * step.
     *
     * @param detID
     *            a string identifying the multi-entry/multi-exit detector
     * @return number of halting vehicles
     */

    public static SumoCommand getLastStepHaltingNumber(String detID) {
        return new SumoCommand(Constants.CMD_GET_MULTIENTRYEXIT_VARIABLE, Constants.LAST_STEP_VEHICLE_HALTING_NUMBER, detID, Constants.RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, Constants.TYPE_INTEGER);
    }

    /**
     * Returns the mean speed of vehicles (in m/s) that have been within the
     * named multi-entry/multi-exit detector within the last simulation step.
     *
     * @param detID
     *            a string identifying the multi-entry/multi-exit detector
     * @return average speed within the last time step
     */

    public static SumoCommand getLastStepMeanSpeed(String detID) {
        return new SumoCommand(Constants.CMD_GET_MULTIENTRYEXIT_VARIABLE, Constants.LAST_STEP_MEAN_SPEED, detID, Constants.RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, Constants.TYPE_DOUBLE);
    }

    /**
     * Returns the list of IDs of vehicles that have been within the named
     * multi-entry/multi-exit detector in the last simulation step.
     *
     * @param detID
     *            a string identifying the multi-entry/multi-exit detector
     * @return list of IDs of all vehicles
     */

    public static SumoCommand getLastStepVehicleIDs(String detID) {
        return new SumoCommand(Constants.CMD_GET_MULTIENTRYEXIT_VARIABLE, Constants.LAST_STEP_VEHICLE_ID_LIST, detID, Constants.RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the number of vehicles that have been within the named
     * multi-entry/multi-exit detector within the last simulation step.
     *
     * @param detID
     *            a string identifying the multi-entry/multi-exit detector
     * @return number of vehicles
     */

    public static SumoCommand getLastStepVehicleNumber(String detID) {
        return new SumoCommand(Constants.CMD_GET_MULTIENTRYEXIT_VARIABLE, Constants.LAST_STEP_VEHICLE_NUMBER, detID, Constants.RESPONSE_GET_MULTIENTRYEXIT_VARIABLE, Constants.TYPE_INTEGER);
    }

    //setter methods


}
