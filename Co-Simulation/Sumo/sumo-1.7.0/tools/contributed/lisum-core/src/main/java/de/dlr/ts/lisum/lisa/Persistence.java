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
/// @file    Persistence.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.commons.logger.DLRLogger;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Properties;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
class Persistence {
    private final Properties properties;
    private final WunschVector vector;
    private final LisaControlUnit controlUnit;
    private final File persistenceFile;


    /**
     *
     */
    public Persistence(WunschVector vector, File lisaDirectory, LisaControlUnit controlUnit) {
        properties = new Properties();
        this.vector = vector;
        this.controlUnit = controlUnit;

        persistenceFile = new File(lisaDirectory.getAbsolutePath() + File.separator + this.controlUnit.getFullName() + ".per");
    }

    /**
     *
     * @param vector
     */
    public void store() {
        properties.put("ebene", String.valueOf(vector.getEbene()));
        properties.put("knoteneinaus", String.valueOf(vector.getKnotenEinAus()));

        properties.put("va", String.valueOf(vector.isVA()));
        properties.put("iv", String.valueOf(vector.isIV()));
        properties.put("ov", String.valueOf(vector.isOV()));

        properties.put("betriebsart", String.valueOf(vector.getBetriebsArt()));
        properties.put("signalprogram", String.valueOf(vector.getSignalProgramIndex()));


        FileOutputStream out;
        try {
            out = new FileOutputStream(persistenceFile);
            properties.store(out, "LiSuM: Persistence file");
        } catch (FileNotFoundException ex) {
            //ex.printStackTrace();
            DLRLogger.severe(this, "Error storing control unit properties to " + persistenceFile.getAbsolutePath());
        } catch (IOException ex) {
            //ex.printStackTrace();
            DLRLogger.severe(this, "Error storing control unit properties to " + persistenceFile.getAbsolutePath());
        }

    }

    void readPersistence() {
        if (persistenceFile.exists()) {
            try {
                properties.load(new FileInputStream(persistenceFile));

                String ebene = properties.getProperty("ebene", String.valueOf(controlUnit.getEbene()));
                int ebe = Integer.valueOf(ebene);
                if (ebe >= 0) {
                    controlUnit.setEbene(ebe);
                }

                String einaus = properties.getProperty("knoteneinaus", String.valueOf(controlUnit.getKnotenEinAus()));
                controlUnit.setKnotenEinAus(Integer.valueOf(einaus));

                String va = properties.getProperty("va", String.valueOf(controlUnit.isVA()));
                controlUnit.setVA(Boolean.valueOf(va));

                String iv = properties.getProperty("iv", String.valueOf(controlUnit.isIV()));
                controlUnit.setIV(Boolean.valueOf(iv));

                String ov = properties.getProperty("ov", String.valueOf(controlUnit.isOV()));
                controlUnit.setOV(Boolean.valueOf(ov));

                //String ebene = properties.getProperty("betriebsart", String.valueOf(controlUnit.get));
                //controlUnit.setEbene(Integer.valueOf(ebene));

                String sp = properties.getProperty("signalprogram", String.valueOf(vector.getSignalProgramIndex()));
                int dd = Integer.valueOf(sp);
                if (dd >= 0) {
                    controlUnit.setCurrentSignalProgram(dd);
                }
            } catch (IOException ex) {
                ex.printStackTrace(System.out);
            }
        }
    }
}
