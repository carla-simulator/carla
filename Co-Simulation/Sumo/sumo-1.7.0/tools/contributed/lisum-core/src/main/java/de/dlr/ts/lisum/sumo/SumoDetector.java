/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    SumoDetector.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.sumo;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.interfaces.DetectorInterface;
import de.tudresden.sumo.cmd.Inductionloop;
import de.tudresden.ws.container.SumoVehicleData;
import it.polito.appeal.traci.SumoTraciConnection;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
class SumoDetector {
    private final String name;
    private DetectorInterface cityDetector;
    private final SumoTraciConnection conn;
    private double lastEntryTime = 0L;
    private boolean onlyLeave = false;

    /**
     *
     * @param name
     * @param conn
     */
    public SumoDetector(String name /*, VehicleTypes vehicleTypes*/, SumoTraciConnection conn) {
        this.conn = conn;
        this.name = name;
    }

    /**
     *
     * @return
     */
    @Override
    public String toString() {
        return name;
    }

    /**
     *
     * @param cityDetector
     */
    public void setCityDetector(DetectorInterface cityDetector) {
        this.cityDetector = cityDetector;
    }

    /**
     *
     * @return
     */
    public String getName() {
        return name;
    }

    /**
     *
     * @throws java.lang.Exception
     */
    public void executeSimulationStep() throws Exception {
        if (cityDetector == null) {
            return;
        }

        SumoVehicleData vehData = (SumoVehicleData)conn.do_job_get(Inductionloop.getVehicleData(name));
        for (SumoVehicleData.VehicleData d : vehData.ll) {
            DLRLogger.finest(this, String.format(" veh=%s len=%s entry=%s leave=%s type=%s", d.vehID, d.length, d.entry_time, d.leave_time, d.typeID));

            double entryTime = d.entry_time;
            double leaveTime = d.leave_time;

            if (entryTime == lastEntryTime && leaveTime == -1) {
                continue;
            }

            if (!onlyLeave) {
                onlyLeave = true;
                lastEntryTime = entryTime;

                entryTime -= ((int) entryTime);
                entryTime *= 100;

                cityDetector.addFlanke((int) entryTime);
            }

            if (leaveTime != -1) {
                leaveTime -= ((int) leaveTime);
                leaveTime *= 100;

                int leaveTimeInt = (int) leaveTime;
                leaveTimeInt *= -1;

                if (leaveTimeInt == 0) { //LISA+ expect negative values for outgoing time values
                    leaveTimeInt = -1;
                }

                cityDetector.addFlanke(leaveTimeInt);

                lastEntryTime = 0L;
                onlyLeave = false;
            }
        }
    }
}
