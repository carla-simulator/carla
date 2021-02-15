/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2016-2017 Dresden University of Technology
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
/// @file    SumoVehicleData.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws.container;

import java.util.LinkedList;

public class SumoVehicleData implements SumoObject {

    public LinkedList<VehicleData> ll;

    public SumoVehicleData() {
        this.ll = new LinkedList<VehicleData>();
    }

    public void add(String vehID, double length, double entry_time, double leave_time, String typeID) {
        this.ll.add(new VehicleData(vehID, length, entry_time, leave_time, typeID));
    }

    public static class VehicleData {

        public String vehID;
        public double length;
        public double entry_time;
        public double leave_time;
        public String typeID;

        public VehicleData(String vehID, double length, double entry_time, double leave_time, String typeID) {
            this.vehID = vehID;
            this.length = length;
            this.entry_time = entry_time;
            this.leave_time = leave_time;
            this.typeID = typeID;
        }


    }

}
