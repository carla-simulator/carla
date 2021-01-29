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
/// @file    SimulationDetectors.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.simulation;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.lisum.interfaces.CityInterface;
import de.dlr.ts.lisum.interfaces.ControlUnitInterface;
import de.dlr.ts.lisum.interfaces.DetectorInterface;
import java.util.HashMap;
import java.util.Map;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class SimulationDetectors {
    private final Map<String, DetectorInterface> detectors = new HashMap<>();


    /**
     *
     */
    public SimulationDetectors() {
    }

    /**
     *
     * @param configFile
     * @param city
     */
    public void load(ConfigurationFile configFile, CityInterface city) {
        DLRLogger.info(this, "Loading detectors");

        for (ConfigurationFile.CFControlUnit controlUnit : configFile) {
            ControlUnitInterface controlUnitInt = city.getControlUnit(controlUnit.lisaName);

            for (int i = 0; i < controlUnit.detectors.size(); i++) {
                ConfigurationFile.CFControlUnit.Detector det = controlUnit.detectors.get(i);
                DetectorInterface detectorInt = controlUnitInt.getDetector(det.lisaName);

                if (detectorInt != null) {
                    detectors.put(det.sumoName, detectorInt);
                }
            }
        }
    }


    /**
     *
     * @param id
     * @return
     */
    public DetectorInterface getDetectorPerSumoId(String id) {
        return detectors.get(id);
    }

}
