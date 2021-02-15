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
/// @file    SumoBestLanes.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws.container;

import java.util.LinkedList;

/**
 *
 * @author Mario Krumnow
 *
 */

public class SumoBestLanes implements SumoObject {

    public LinkedList<SumoBestLane> ll;

    public SumoBestLanes() {
        this.ll = new LinkedList<SumoBestLane>();
    }

    public void add(String laneID, double length, double occupation, int offset, int allowsContinuation, LinkedList<String> ll) {
        this.ll.add(new SumoBestLane(laneID, length, occupation, offset, allowsContinuation, ll));
    }

    public String toString() {
        return "";
    }

    public class SumoBestLane {

        public LinkedList<String> ll;
        public String laneID;
        public double length;
        public double occupation;
        public int offset;
        public int allowsContinuation;

        public SumoBestLane(String laneID, double length, double occupation, int offset, int allowsContinuation, LinkedList<String> ll) {
            this.laneID = laneID;
            this.length = length;
            this.occupation = occupation;
            this.offset = offset;
            this.allowsContinuation = allowsContinuation;
            this.ll = ll;
        }
    }
}
