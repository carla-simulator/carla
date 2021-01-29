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
/// @file    SumoBoundingBox.java
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

public class SumoBoundingBox implements SumoObject {

    public double x_min;
    public double y_min;
    public double x_max;
    public double y_max;


    public SumoBoundingBox() {
        this.x_min = 0;
        this.y_min = 0;
        this.x_max = 0;
        this.y_max = 0;
    }

    public SumoBoundingBox(double x_min, double y_min, double x_max, double y_max) {

        this.x_min = x_min;
        this.y_min = y_min;
        this.x_max = x_max;
        this.y_max = y_max;

    }

    public String toString() {
        return this.x_min + "#" + this.y_min + "#" + this.x_max + "#" + this.y_max;
    }


}
