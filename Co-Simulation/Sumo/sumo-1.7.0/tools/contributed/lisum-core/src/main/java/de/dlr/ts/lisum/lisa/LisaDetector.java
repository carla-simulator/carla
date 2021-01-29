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
/// @file    LisaDetector.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.logger.ToString;
import de.dlr.ts.lisum.interfaces.DetectorInterface;
import java.util.ArrayList;
import java.util.List;

/**
 *
 */
class LisaDetector implements DetectorInterface, Comparable<LisaDetector> {
    private final int index;
    private final String id;

    private String lastVehicleId = null;
    private final List<Integer> flanken = new ArrayList<>();



    /**
     *
     * @return
     */
    @Override
    public String toString() {
        ToString tos = new ToString("Detektoren");
        tos.add("index", index);
        tos.add("name", id);
        tos.add("FlankenCount", flanken.size());
        return tos.toString();
    }

    /**
     *
     * @param id
     */
    public LisaDetector(int index, String id) {
        this.index = index;
        this.id = id;
    }

    /**
     *
     * @param slopes In deciseconds!
     */
    @Override
    public void addFlanke(int slope) {
        DLRLogger.finer(this, id + ": adding detector slope " + slope);
        flanken.add(slope);
    }

    /**
     *
     */
    void clear() {
        flanken.clear();
    }

    /**
     *
     * @return
     */
    boolean isEmpty() {
        return flanken.isEmpty();
    }

    /**
     * {71/22/(4)14!-75/9!-49}
     *
     * @param withIndex
     * @return
     */
    String getLisaString(boolean withIndex) {
        StringBuilder sb = new StringBuilder();
        if (withIndex) {
            sb.append("(").append(index - 1).append(")");
        }

        DLRLogger.finest(this, "Lisa String for detector " + this);

        /**
         */
        if (flanken.size() > 0) {
            Integer value = flanken.get(0);

            DLRLogger.finest(this, "Flanke value: " + value);

            sb.append(value);

            if (flanken.size() == 2) {
                sb.append("!").append(flanken.get(1));
            }
        }

        flanken.clear();

        return sb.toString();
    }

    @Override
    public String getId() {
        return id;
    }

    @Override
    public int compareTo(LisaDetector t) {
        if (t.index > this.index) {
            return -1;
        }
        if (t.index == this.index) {
            return 0;
        }

        return 1;
    }

    /**
     *
     */
    @Deprecated
    public void setFromSumo(String[] vehicles) {
        flanken.clear();

        if (vehicles == null) {
            if (lastVehicleId != null) { //add negative flanke, vehicle leaving the loop
                flanken.add(-10);
            }

            lastVehicleId = null;
        } else if (vehicles.length == 1) {
            if (lastVehicleId == null) { //a new car entered the loop
                flanken.add(50);    //50
            } else if (!lastVehicleId.equals(vehicles[0])) {
                flanken.add(-30);
                flanken.add(60);
            }

            lastVehicleId = vehicles[0];
        }
        //  Is it necessary to continue for vehicles.length >= 2?
    }
}
