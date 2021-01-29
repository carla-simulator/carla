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
/// @file    ConfigurationFile.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.simulation;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.logger.ToString;
import de.dlr.ts.lisum.simulation.ConfigurationFile.CFControlUnit;
import de.dlr.ts.utils.xmladmin2.Attributes;
import de.dlr.ts.utils.xmladmin2.XMLAdmin2;
import de.dlr.ts.utils.xmladmin2.XMLNode;
import de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException;
import de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.xml.sax.SAXException;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class ConfigurationFile implements Iterable<ConfigurationFile.CFControlUnit> {
    private final List<CFControlUnit> controlUnits = new ArrayList<>();

    private File configurationFile;
    private String lisaDirectoryName;
    private File lisaDirectory;

    @Override
    public Iterator<CFControlUnit> iterator() {
        return controlUnits.iterator();
    }

    /**
     *
     * @return
     */
    public File getConfigurationFile() {
        return configurationFile;
    }

    /**
     *
     * @return
     */
    public File getLisaDirectory() {
        return lisaDirectory;
    }

    /**
     *
     * @param configurationFile
     */
    public void read(File configurationFile) {
        this.configurationFile = configurationFile.getAbsoluteFile();

        try {
            XMLAdmin2 x = new XMLAdmin2().load(configurationFile);

            try {
                DLRLogger.info(this, "Reading configuration file: " + configurationFile);

                lisaDirectoryName = x.getNode("input.lisa").getValue();
                lisaDirectory = new File(lisaDirectoryName);

                if (!lisaDirectory.isAbsolute())
                    lisaDirectory = new File(this.configurationFile.getParentFile()
                                             + File.separator + lisaDirectoryName);

                int count = x.getNodesCount("controlUnits.controlUnit");

                for (int i = 0; i < count; i++) {
                    XMLNode controlUnitNode = x.getNode("controlUnits.controlUnit", i);

                    Attributes atts = x.getNode("controlUnits.controlUnit", i).getAttributes();
                    String _lisa = atts.get("lisa").getValue();
                    String _sumo = atts.get("sumo").getValue();

                    CFControlUnit controlUnit = new CFControlUnit(_lisa, _sumo);
                    controlUnits.add(controlUnit);
                    DLRLogger.info(this, "Reading ControlUnit " + _lisa + " is " + _sumo);

                    if (controlUnitNode.hasNode("signalGroups")) {
                        extractSignalGroups(controlUnit, controlUnitNode);
                    }

                    if (controlUnitNode.hasNode("detectors")) {
                        extractDetectors(controlUnit, controlUnitNode);
                    }
                }
            } catch (XMLNodeNotFoundException ex) {
                DLRLogger.severe(this, ex);
            }
        } catch (SAXException | IOException | MalformedKeyOrNameException ex) {
            DLRLogger.severe(this, ex);
        }
    }

    /**
     *
     * @param controlUnit
     * @param node
     */
    private void extractSignalGroups(CFControlUnit controlUnit, XMLNode node)
    throws MalformedKeyOrNameException, XMLNodeNotFoundException {
        int count = node.getNodesCount("signalGroups.signalGroup");

        for (int j = 0; j < count; j++) {
            XMLNode signalGroupNode = node.getNode("signalGroups.signalGroup", j);

            String lisa = signalGroupNode.getAttributes().get("lisa").getValue();
            String sumo = signalGroupNode.getAttributes().get("sumo").getValue();
            String ifOff = null;

            if (signalGroupNode.getAttributes().hasAttribute("main")) {
                ifOff = signalGroupNode.getAttributes().get("main").getValue();
            }

            CFControlUnit.SignalGroup sg = new CFControlUnit.SignalGroup(lisa, sumo, ifOff);
            controlUnit.signalGroups.add(sg);

            DLRLogger.config(this, "Reading " + sg);
        }
    }

    /**
     *
     * @param controlUnit
     * @param node
     * @throws MalformedKeyOrNameException
     * @throws XMLNodeNotFoundException
     */
    private void extractDetectors(CFControlUnit controlUnit, XMLNode node)
    throws MalformedKeyOrNameException, XMLNodeNotFoundException {
        int count = node.getNodesCount("detectors.detector");

        for (int i = 0; i < count; i++) {
            XMLNode _node = node.getNode("detectors.detector", i);
            String lisa = _node.getAttributes().get("lisa").getValue();
            String sumo = _node.getAttributes().get("sumo").getValue();

            CFControlUnit.Detector det = new CFControlUnit.Detector(lisa, sumo);
            controlUnit.detectors.add(det);

            DLRLogger.config(this, "Reading " + det);
        }
    }

    /**
     *
     */
    public static class CFControlUnit {

        public String lisaName;
        public String sumoName;
        public List<SignalGroup> signalGroups = new ArrayList<>();
        public List<Detector> detectors = new ArrayList<>();

        /**
         *
         * @param lisaName
         * @param sumoName
         */
        public CFControlUnit(String lisaName, String sumoName) {
            this.lisaName = lisaName;
            this.sumoName = sumoName;
        }

        /**
         *
         */
        public static class SignalGroup {

            public String lisa;
            public String sumo;
            public String ifOff;

            public SignalGroup(String lisa, String sumo, String ifOff) {
                this.lisa = lisa;
                this.sumo = sumo;
                this.ifOff = ifOff;
            }

            @Override
            public String toString() {
                return new ToString("SignalGroup").add("Lisa", lisa).add("Sumo", sumo)
                       .add("IfOff", ifOff).toString();
            }
        }

        /**
         *
         */
        public static class Detector {

            public String lisaName;
            public String sumoName;

            public Detector(String lisa, String sumo) {
                this.lisaName = lisa;
                this.sumoName = sumo;
            }

            @Override
            public String toString() {
                return new ToString("Detector").add("Lisa", lisaName).add("Sumo", sumoName).toString();
            }
        }
    }
}
