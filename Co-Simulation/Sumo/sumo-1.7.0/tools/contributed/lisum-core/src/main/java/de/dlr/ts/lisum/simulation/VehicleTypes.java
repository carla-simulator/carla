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
/// @file    VehicleTypes.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.simulation;

import de.dlr.ts.utils.xmladmin2.XMLAdmin2;
import de.dlr.ts.utils.xmladmin2.XMLNode;
import de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException;
import de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import org.xml.sax.SAXException;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class VehicleTypes {

    private final Map<String, VehicleType> types = new HashMap<>();

    /**
     *
     */
    public VehicleTypes() {
    }

    /**
     *
     * @param name
     * @return
     */
    public VehicleType getVehicleType(String name) {
        return types.get(name);
    }

    /**
     *
     * @param configFile
     */
    public void load(File configFile) {
        try {
            XMLAdmin2 x = new XMLAdmin2().load(configFile);

            int nodesCount = x.getNodesCount("vType");

            for (int i = 0; i < nodesCount; i++) {
                XMLNode node = x.getNode("vType", i);
                String id = node.getAttributes().get("id").getValue();
                double length = node.getAttributes().get("length").getValue(0d);

                types.put(id, new VehicleType(id, length));
            }
        } catch (SAXException | IOException | MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
            ex.printStackTrace(System.out);
        }
    }

    /**
     *
     */
    public static class VehicleType {

        private final String name;
        private final double length;

        /**
         *
         * @param name
         * @param length
         */
        public VehicleType(String name, double length) {
            this.name = name;
            this.length = length;
        }

        public String getName() {
            return name;
        }

        /**
         *
         * @return
         */
        public double getLength() {
            return length;
        }
    }
}
