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
/// @file    SumoNextTLS.java
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

public class SumoNextTLS implements SumoObject {

    public LinkedList<NextTLS> ll;

    public SumoNextTLS() {
        this.ll = new LinkedList<NextTLS>();
    }

    public void add(String tlsID, int ix, double dist, String state) {
        this.ll.add(new NextTLS(tlsID, ix, dist, state));
    }

    public String toString() {

        return "";
        //return this.phasedef+"#"+this.duration+"#"+this.duration1+"#"+this.duration2;
    }

    public class NextTLS {

        public String tlsID;
        int ix;
        double dist;
        String state;

        public NextTLS(String tlsID, int ix, double dist, String state) {
            this.tlsID = tlsID;
            this.ix = ix;
            this.dist = dist;
            this.state = state;
        }
    }


}
