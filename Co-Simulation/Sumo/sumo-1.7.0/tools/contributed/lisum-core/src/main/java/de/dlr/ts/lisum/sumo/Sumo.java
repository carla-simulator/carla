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
/// @file    Sumo.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.sumo;

import de.dlr.ts.commons.logger.DLRLogger;
//import de.dlr.ts.lisum.GlobalConfig;
import de.dlr.ts.lisum.interfaces.DetectorInterface;
import de.dlr.ts.lisum.simulation.LisumSimulation;
import de.dlr.ts.lisum.simulation.VehicleTypes;
import de.tudresden.sumo.cmd.Inductionloop;
import de.tudresden.sumo.cmd.Simulation;
import de.tudresden.sumo.cmd.Trafficlight;
import de.tudresden.ws.container.SumoStringList;
import it.polito.appeal.traci.SumoTraciConnection;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * This class administrates the connection with SUMO and the simulation
 * execution on the SUMO side.
 *
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class Sumo {

    private LisumSimulation lisumSimulation;
    private SumoControlUnits sumoControlUnits;
    private SumoDetectors sumoDetectors;
    private VehicleTypes vehicleTypes;
    private long vehiclesCount;
    private long stepsPerSecond = 0L;
    private final String sumoExec;
    private final String sumoConfig;
    private final int sumoPort;


    /**
     *
     * @param lisumSimulation
     * @param sumoExec
     */
    public Sumo(LisumSimulation lisumSimulation, String sumoExec, String sumoConfig, int sumoPort) {
        this.lisumSimulation = lisumSimulation;
        this.sumoExec = sumoExec;
        this.sumoConfig = sumoConfig;
        this.sumoPort = sumoPort;
    }

    /**
     *
     */
    public void initBeforePlay() {
        sumoControlUnits = new SumoControlUnits();
        sumoControlUnits.load(lisumSimulation.getSimulationFiles().getSumoNetFile());

        vehicleTypes = new VehicleTypes();
        vehicleTypes.load(lisumSimulation.getSimulationFiles().getSumoRouFile());
    }

    /**
     * Starts SumoGUI, which takes controls of the whole application.
     */
    public void play() {
        Thread thread = new Thread(runnable);
        thread.start();
    }

    /**
     *
     */
    private final Runnable runnable = new Runnable() {
        @Override
        public void run() {
            SumoTraciConnection conn;

            try {
                /**
                 *
                 */
                DLRLogger.info("Sumo", "Starting SumoGUI instance, port " + sumoPort);

                //startSumoGUI();

                conn = new SumoTraciConnection(sumoExec, sumoConfig);

                //Start Traci Server and Sumo
                conn.runServer();

                sumoDetectors = new SumoDetectors(conn);

                double simulationTimeSeconds = 0;

                /**
                 * Control units
                 */
                Object get = conn.do_job_get(Trafficlight.getIDList());
                SumoStringList ids = (SumoStringList) get;

                for (String id : ids) {
                    SumoControlUnit sumoControlUnit = sumoControlUnits.getControlUnit(id);

                    if (sumoControlUnit != null) {
                        sumoControlUnit.setSumoTraciConnection(conn);
                        sumoControlUnit.setControlUnits(lisumSimulation.getControlUnits());
                    }
                }

                /**
                 * Induction loops
                 */
                DLRLogger.finest(this, "Reading detectors from Sumo");

                for (String inductionId : (SumoStringList) conn.do_job_get(Inductionloop.getIDList())) {
                    DetectorInterface cityDetector = lisumSimulation.getDetectors().getDetectorPerSumoId(inductionId);

                    //sumoDetectors.addDetector(induction, induct, cityDetector);
                    sumoDetectors.addDetector(inductionId, cityDetector);
                }

                long time = System.currentTimeMillis();
                long _steps = 0L;

                while ((int) conn.do_job_get(Simulation.getMinExpectedNumber()) > 0) {
                    //System.out.println("Step=" + simulationTimeSeconds + " expected=" + (int)conn.do_job_get(Simulation.getMinExpectedNumber()));
                    conn.do_timestep();

                    simulationTimeSeconds = (int) conn.do_job_get(Simulation.getCurrentTime()) / 1000d;

                    if (System.currentTimeMillis() - time > 1000L) {
                        stepsPerSecond = _steps;
                        _steps = 0L;
                        time = System.currentTimeMillis();
                    } else {
                        _steps++;
                    }

                    vehiclesCount = (int) conn.do_job_get(de.tudresden.sumo.cmd.Vehicle.getIDCount());

                    sumoDetectors.executeSimulationStep();
                    sumoControlUnits.executeSimulationStep();
                    lisumSimulation.executeSimulationStep((long) simulationTimeSeconds);
                }

                conn.close();
                //System.out.println("Quitting at t=" + simulationTimeSeconds);
            } catch (UnknownHostException ex) {
                //DLRLogger.severe(this, ex);
                DLRLogger.info("Sumo", "Closing SumoGUI instance");
            } catch (IOException | InterruptedException ex) {
                //DLRLogger.severe(this, ex);
                DLRLogger.info("Sumo", "Closing SumoGUI instance");
            } catch (Exception ex) {
                Logger.getLogger(Sumo.class.getName()).log(Level.SEVERE, null, ex);
            }

            stepsPerSecond = 0;
            vehiclesCount = 0;
        }
    };

    /**
     *
     * @return
     */
    public Runnable getRunnable() {
        return runnable;
    }

    /**
     *
    private void startSumoGUI() {
        try {
            String sumoConfig = lisumSimulation.getSimulationFiles().getSumoConfigFile().getAbsolutePath();

            ProcessBuilder pb = new ProcessBuilder(GlobalConfig.getInstance().getSumoExec(),
                    "--remote-port",
                    String.valueOf(GlobalConfig.getInstance().getSumoPort()),
                    "-c",
                    sumoConfig);

            pb.start();
        } catch (IOException ex) {
            DLRLogger.severe(this, ex);
        }
    }
    * */

    /**
     *
     * @return
     */
    public long getVehiclesCount() {
        return vehiclesCount;
    }

    /**
     *
     * @return
     */
    public long getStepsPerSecond() {
        return stepsPerSecond;
    }

}
