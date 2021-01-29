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
/// @file    SumoLink.java
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

public class SumoLink implements SumoObject {

    public String from;
    public String to;
    public String over;

    //2nd
    public String notInternalLane;
    public String internalLane;
    public String state;
    public String direction;
    public byte hasPriority;
    public byte isOpen;
    public byte hasApproachingFoe;
    public double length;

    int type = 0;

    //1st constructor
    public SumoLink(String from, String to,	String over) {
        this.from = from;
        this.to = to;
        this.over = over;
        this.type = 0;
    }

    //2nd constructor
    public SumoLink(String notInternal, String internal, byte priority, byte isOpen, byte hasFoe, double length, String state, String direction) {
        this.notInternalLane = notInternal;
        this.internalLane = internal;
        this.hasPriority = priority;
        this.isOpen = isOpen;
        this.hasApproachingFoe = hasFoe;
        this.length = length;
        this.state = state;
        this.direction = direction;
        this.type = 1;
    }



    public String toString() {

        if (this.type == 0) {
            return this.from + "#" + this.over + "#" + this.to;
        } else {
            return this.notInternalLane + "#" + this.internalLane + "#" + this.hasPriority + "#" + this.isOpen + "#" + this.hasApproachingFoe + "#" + this.length + "#" + this.state + "#" + this.direction;
        }

    }


}
