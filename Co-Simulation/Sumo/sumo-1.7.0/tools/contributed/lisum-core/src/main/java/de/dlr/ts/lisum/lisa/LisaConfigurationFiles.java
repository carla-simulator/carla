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
/// @file    LisaConfigurationFiles.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.commons.logger.DLRLogger;
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
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
class LisaConfigurationFiles implements Iterable<LisaConfigurationFiles.ControlUnit> {
    public List<ControlUnit> controlUnits = new ArrayList<>();
    public File lisaFolder;


    @Override
    public Iterator<LisaConfigurationFiles.ControlUnit> iterator() {
        return controlUnits.iterator();
    }

    /**
     * Looks for xml files in the Lisa directory.
     * Each of those files represent Control units and contain information related to them.
     * This method loads this information and communicates the Lisa server the data directory.
     *
     * @param lisaFolder
     */
    public void load(File lisaFolder) {
        this.lisaFolder = lisaFolder;

        String fullpath = lisaFolder.getAbsolutePath() + File.separator;

        for (String name : lisaFolder.list()) {
            if (name.endsWith(".xml")) {
                try {
                    File controlUnitConfigFile = new File(fullpath + name);
                    XMLAdmin2 x = new XMLAdmin2().load(controlUnitConfigFile);

                    /**
                     * Reading control unit metadata
                     */
                    XMLNode ocit = x.getNode("Kopfdaten.Identifikation.OCITKennung");
                    int zNr = ocit.getNode("ZNr").getValue(0);
                    int fNr = ocit.getNode("FNr").getValue(0);
                    String _name = "z" + zNr + "_fg" + fNr;
                    DLRLogger.config(this, "Creating Control Unit " + _name);

                    ControlUnit controlUnit = new ControlUnit();
                    controlUnits.add(controlUnit);
                    controlUnit.zNr = zNr;
                    controlUnit.fNr = fNr;
                    controlUnit.fullName = _name;

                    extractSignalGroups(x, controlUnit);
                    extractDetectors(x, controlUnit);
                    extractSignalPrograms(x, controlUnit);
                } catch (SAXException | IOException | MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
                    ex.printStackTrace(System.out);
                }
            }
        }

        DLRLogger.config(this, "Lisa Configuration files read successfully");
        DLRLogger.config(this, "ControlUnits count: " + controlUnits.size());
    }

    public File getLisaDirectory() {
        return lisaFolder;
    }

    /**
     *
     */
    private void extractSignalGroups(XMLAdmin2 x, ControlUnit controlUnit) {
        try {
            int gruppenCount = x.getNodesCount("SignalgruppeListe.Signalgruppe");
            for (int i = 0; i < gruppenCount; i++) {
                XMLNode node = x.getNode("SignalgruppeListe.Signalgruppe", i);
                String bezeichnung = node.getNode("Bezeichnung").getValue();
                int index = node.getNode("ObjNr").getValue(0);

                DLRLogger.info(this, "Adding signal group: " + bezeichnung);

                ControlUnit.SignalGroup sg = new ControlUnit.SignalGroup();
                controlUnit.signalGroups.add(sg);
                sg.bezeichnung = bezeichnung;
                sg.index = index;
            }
        } catch (XMLNodeNotFoundException | MalformedKeyOrNameException ex) {
            ex.printStackTrace(System.out);
        }
    }

    /**
     *
     * @param configFile
     * @param controlUnitName
     * @param lisa
     * @throws XMLNodeNotFoundException
     * @throws MalformedKeyOrNameException
     * @throws SAXException
     * @throws IOException
     */
    void extractDetectors(XMLAdmin2 x, ControlUnit controlUnit)
    throws XMLNodeNotFoundException, MalformedKeyOrNameException, SAXException, IOException {
        if (!x.hasNode("DigEingangListe")) {
            return;
        }

        int detectorsCount = x.getNodesCount("DigEingangListe.DigEingang");
        for (int i = 0; i < detectorsCount; i++) {
            XMLNode node = x.getNode("DigEingangListe.DigEingang", i);
            String bezeichnung = node.getNode("Bezeichnung").getValue();
            int nr = node.getNode("ObjNr").getValue(0);

            String fullName = controlUnit.fullName + "." + bezeichnung;

            ControlUnit.Detector detector = new ControlUnit.Detector();
            detector.objNr = nr;
            detector.bezeichnung = bezeichnung;
            detector.fullName = fullName;
            controlUnit.detectors.add(detector);

            DLRLogger.info(this, "Adding detector: " + fullName + " (" + nr + ")");
        }
    }

    void extractSignalPrograms(XMLAdmin2 x, ControlUnit controlUnit)
    throws XMLNodeNotFoundException, MalformedKeyOrNameException, SAXException, IOException {
        if (!x.hasNode("SignalprogrammListe")) {
            return;
        }

        int signalProgramsCount = x.getNodesCount("SignalprogrammListe.Signalprogramm");

        for (int i = 0; i < signalProgramsCount; i++) {
            XMLNode node = x.getNode("SignalprogrammListe.Signalprogramm", i);
            String bezeichnung = node.getNode("Bezeichnung").getValue();
            int objnr = node.getNode("ObjNr").getValue(0);

            ControlUnit.SignalProgram sp = new ControlUnit.SignalProgram();
            sp.bezeichnung = bezeichnung;
            sp.objNr = objnr;

            controlUnit.signalPrograms.add(sp);
        }

    }

    /**
     *
     */
    public static class ControlUnit {
        int zNr;
        int fNr;
        String fullName;

        public List<SignalGroup> signalGroups = new ArrayList<>();
        public List<Detector> detectors = new ArrayList<>();
        public List<SignalProgram> signalPrograms = new ArrayList<>();


        public static class SignalGroup {
            String bezeichnung;
            int index;
        }

        public static class Detector {
            String bezeichnung;
            int objNr;
            String fullName;
        }

        public static class SignalProgram {
            String bezeichnung;
            int objNr;
        }
    }

}
