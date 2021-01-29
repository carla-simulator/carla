/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2013-2017 Dresden University of Technology
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
/// @file    Main.java
/// @author  Mario Krumnow
/// @date    2013
///
//
/****************************************************************************/
import it.polito.appeal.traci.SumoTraciConnection;
import de.tudresden.sumo.cmd.Simulation;
import de.tudresden.sumo.cmd.Vehicle;
import de.tudresden.sumo.cmd.Inductionloop;
import de.tudresden.sumo.cmd.Trafficlight;
import de.tudresden.ws.container.SumoVehicleData;

public class Main {
    public static void main(String[] args) {
        String sumo_bin = "sumo";
        String config_file = "data/config.sumocfg";
        double step_length = 0.1;

        if (args.length > 0) {
            sumo_bin = args[0];
        }
        if (args.length > 1) {
            config_file = args[1];
        }

        try {
            SumoTraciConnection conn = new SumoTraciConnection(sumo_bin, config_file);
            conn.addOption("step-length", step_length + "");
            conn.addOption("start", "true"); //start sumo immediately

            //start Traci Server
            conn.runServer();
            conn.setOrder(1);

            for (int i = 0; i < 3600; i++) {

                conn.do_timestep();
                conn.do_job_set(Vehicle.addFull("v" + i, "r1", "car", "now", "0", "0", "max", "current", "max", "current", "", "", "", 0, 0));
                double timeSeconds = (double)conn.do_job_get(Simulation.getTime());
                int tlsPhase = (int)conn.do_job_get(Trafficlight.getPhase("gneJ1"));
                String tlsPhaseName = (String)conn.do_job_get(Trafficlight.getPhaseName("gneJ1"));
                System.out.println(String.format("Step %s, tlsPhase %s (%s)", timeSeconds, tlsPhase, tlsPhaseName));

                SumoVehicleData vehData = (SumoVehicleData)conn.do_job_get(Inductionloop.getVehicleData("loop1"));
                for (SumoVehicleData.VehicleData d : vehData.ll) {
                    System.out.println(String.format("  veh=%s len=%s entry=%s leave=%s type=%s", d.vehID, d.length, d.entry_time, d.leave_time, d.typeID));
                }
            }

            conn.close();

        } catch (Exception ex) {
            ex.printStackTrace();
        }

    }

}
