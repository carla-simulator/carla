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
/// @file    SumoControlUnits.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.sumo;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.utils.xmladmin2.XMLAdmin2;
import de.dlr.ts.utils.xmladmin2.XMLNode;
import de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException;
import de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException;
import java.io.File;
import java.io.IOException;
import org.xml.sax.SAXException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
class SumoControlUnits {

    private final List<SumoControlUnit> controlUnits = new ArrayList<>();
    private final Map<String, SumoControlUnit> controlUnitsHash = new HashMap<>();

    /**
     *
     */
    public SumoControlUnits() {
    }

    /**
     *
     * @return
     */
    public int getCount() {
        return controlUnits.size();
    }

    /**
     *
     * @param index
     * @return
     */
    public SumoControlUnit getControlUnit(int index) {
        return controlUnits.get(index);
    }

    /**
     *
     */
    public void executeSimulationStep() {
        for (int i = 0; i < controlUnits.size(); i++) {
            controlUnits.get(i).executeSimulationStep();
        }
    }

    /**
     *
     * @param name
     * @return
     */
    public SumoControlUnit getControlUnit(String name) {
        return controlUnitsHash.get(name);
    }

    /**
     *
     * @param sumoNetFile
     * @return
     */
    public SumoControlUnits load(File sumoNetFile) {
        try {
            XMLAdmin2 x = new XMLAdmin2().load(sumoNetFile);

            int tlscount = x.getNodesCount("tlLogic");

            for (int i = 0; i < tlscount; i++) {
                XMLNode node = x.getNode("tlLogic", i);
                final String name = node.getAttributes().get("id").getValue();

                int phasesCount = node.getNodesCount("phase");
                String[] states = new String[phasesCount];

                for (int j = 0; j < phasesCount; j++) {
                    states[j] = node.getNode("phase", j).getAttributes().get("state").getValue();
                }

                SumoControlUnit scu = new SumoControlUnit(name, states);
                this.controlUnits.add(scu);
                this.controlUnitsHash.put(name, scu);

                DLRLogger.info(this, "Adding control unit " + name + ".");
            }

        } catch (SAXException | IOException | MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
            DLRLogger.severe(this, ex);
        }

        return this;
    }
}
