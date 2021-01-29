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
/// @file    SimulationFiles.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.simulation;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.exceptions.LisumException;
import de.dlr.ts.utils.xmladmin2.XMLAdmin2;
import de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException;
import de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException;
import java.io.File;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.xml.sax.SAXException;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class SimulationFiles {

    private File sumoNetFile;
    private File sumoAddFile;
    private File sumoRouFile;

    private File simulationDirectory;
    private File lisumConfigFile;
    private File sumoConfigFile;

    private File lisaDataDirectory;

    /**
     *
     */
    public SimulationFiles() {
    }

    /**
     *
     * @param lisaDataDirectory
     */
    public void setLisaDataDirectory(File lisaDataDirectory) {
        if (!lisaDataDirectory.isAbsolute()) {
            lisaDataDirectory = new File(simulationDirectory.getAbsolutePath()
                                         + File.separator + lisaDataDirectory.getName());
        }

        this.lisaDataDirectory = lisaDataDirectory;
    }

    /**
     *
     * @param lisumConfigurationFile
     * @throws de.dlr.ts.lisum.exceptions.LisumException
     */
    public void read(File lisumConfigurationFile, String sumoConfig) throws LisumException {
        DLRLogger.config(this, "Reading and setting simulation properties from " + lisumConfigurationFile + ".");

        /**
         * Setting Lisum configuration file
         */
        lisumConfigFile = lisumConfigurationFile;
        if (!lisumConfigFile.isAbsolute()) {
            lisumConfigFile = new File(lisumConfigFile.getAbsolutePath());
        }

        if (!lisumConfigFile.exists()) {
            throw new LisumException("Fatal error: Configuration file '"
                                     + lisumConfigFile.getAbsolutePath() + "' does not exist.");
        }

        /**
         * Setting simulation directory
         */
        simulationDirectory = new File(lisumConfigFile.getParent());
        DLRLogger.config(this, "Setting simulation directory: " + simulationDirectory);

        /**
         * Setting Sumo configuration file
         */
        if (sumoConfig == null) {
            for (String s : simulationDirectory.list()) {
                if (s.endsWith(".sumocfg")) {
                    sumoConfig = s;
                }
            }

            if (sumoConfig == null) {
                throw new LisumException("Fatal error: no Sumo configuration file could be found.");
            }

            sumoConfigFile = new File(simulationDirectory.getAbsolutePath() + File.separator + sumoConfig);
        } else {
            sumoConfigFile = new File(sumoConfig);
        }

        DLRLogger.config(this, "Setting Sumo configuration file: " + sumoConfigFile);

        String tmp = null;
        try {
            /**
            * Setting sumo net file
            */
            XMLAdmin2 x = new XMLAdmin2().load(sumoConfigFile);
            try {
                tmp = x.getNode("input.net-file").getAttributes().get("value").getValue();

                if (tmp == null) {
                    throw new LisumException("Fatal error: no Sumo net.xml file could be found");
                }

                sumoNetFile = new File(tmp);
                if (!sumoNetFile.isAbsolute()) {
                    sumoNetFile = new File(sumoConfigFile.getParent() + File.separator + tmp);
                }

                DLRLogger.config(this, "Setting Sumo net.xml file: " + sumoNetFile);
            } catch (XMLNodeNotFoundException ex) {
                Logger.getLogger(SimulationFiles.class.getName()).log(Level.SEVERE, null, ex);
            }

            try {
                /**
                 * Setting sumo add file
                 */
                tmp = x.getNode("input.additional-files").getAttributes().get("value").getValue();

                if (tmp != null) {
                    sumoAddFile = new File(tmp);
                    if (!sumoAddFile.isAbsolute()) {
                        sumoAddFile = new File(sumoConfigFile.getParent() + File.separator + tmp);
                    }

                    DLRLogger.config(this, "Setting Sumo add.xml file: " + sumoAddFile);
                }
            } catch (XMLNodeNotFoundException ex) {
                Logger.getLogger(SimulationFiles.class.getName()).log(Level.SEVERE, null, ex);
            }

            try {
                /**
                 * Setting rou file
                 */
                tmp = x.getNode("input.route-files").getAttributes().get("value").getValue();

                if (tmp != null) {
                    sumoRouFile = new File(tmp);
                    if (!sumoRouFile.isAbsolute()) {
                        sumoRouFile = new File(sumoConfigFile.getParent() + File.separator + tmp);
                    }

                    DLRLogger.config(this, "Setting Sumo rou.xml file: " + sumoRouFile);
                }
            } catch (XMLNodeNotFoundException ex) {
                Logger.getLogger(SimulationFiles.class.getName()).log(Level.SEVERE, null, ex);
            }
        } catch (SAXException | IOException | MalformedKeyOrNameException ex) {
            Logger.getLogger(SimulationFiles.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     *
     * @return
     */
    public File getLisumConfigFile() {
        return lisumConfigFile;
    }

    /**
     *
     * @return
     */
    public File getSumoConfigFile() {
        return sumoConfigFile;
    }

    /**
     *
     * @return
     */
    public File getSimulationDirectory() {
        return simulationDirectory;
    }

    /**
     *
     * @return
     */
    public File getLisaDataDirectory() {
        return lisaDataDirectory;
    }

    /**
     *
     * @return
     */
    public File getSumoNetFile() {
        return sumoNetFile;
    }

    /**
     *
     * @return
     */
    public File getSumoAddFile() {
        return sumoAddFile;
    }

    /**
     *
     * @return
     */
    public File getSumoRouFile() {
        return sumoRouFile;
    }

    /**
     *
     * @param simulationDirectory
     */
    void setSimulationDirectory(File simulationDirectory) {
        this.simulationDirectory = simulationDirectory;
    }

}
