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
/// @file    SumoStopFlags.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws.container;

/**
 *
 * @author Mario Krumnow
 *
 */

public class SumoStopFlags implements SumoObject {

    public boolean stopped = false;
    public boolean parking = false;
    public boolean triggered = false;
    public boolean containerTriggered = false;
    public boolean isBusStop = false;
    public boolean isContainerStop = false;
    public boolean isChargingStationStop = false;
    public boolean isParkingAreaStop = false;

    public SumoStopFlags(boolean parking, boolean triggered, boolean containerTriggered, boolean isBusStop, boolean isContainerStop, boolean isChargingStationStop, boolean isParkingAreaStop) {
        this.parking = parking;
        this.triggered = triggered;
        this.containerTriggered = containerTriggered;
        this.isBusStop = isBusStop;
        this.isContainerStop = isContainerStop;
        this.isChargingStationStop = isChargingStationStop;
        this.isParkingAreaStop = isParkingAreaStop;
    }

    public SumoStopFlags(byte id) {

        int[] m = {1, 2, 4, 8, 16, 32, 64, 128};
        this.stopped = (id & m[0]) == m[0] ? true : false;
        this.parking = (id & m[1]) == m[1] ? true : false;
        this.triggered = (id & m[2]) == m[2] ? true : false;
        this.containerTriggered = (id & m[3]) == m[3] ? true : false;
        this.isBusStop = (id & m[4]) == m[4] ? true : false;
        this.isContainerStop = (id & m[5]) == m[5] ? true : false;
        this.isChargingStationStop = (id & m[6]) == m[6] ? true : false;
        this.isParkingAreaStop = (id & m[7]) == m[7] ? true : false;

    }

    public byte getID() {

        StringBuilder sb = new StringBuilder();
        sb.append(isParkingAreaStop ? "1" : "0");
        sb.append(isChargingStationStop ? "1" : "0");
        sb.append(isContainerStop ? "1" : "0");
        sb.append(isBusStop ? "1" : "0");
        sb.append(containerTriggered ? "1" : "0");
        sb.append(triggered ? "1" : "0");
        sb.append(parking ? "1" : "0");
        return (byte)Integer.parseInt(sb.toString(), 2);

    }


}
