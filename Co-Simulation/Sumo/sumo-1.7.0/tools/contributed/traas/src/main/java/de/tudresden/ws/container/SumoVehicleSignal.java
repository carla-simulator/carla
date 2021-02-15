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
/// @file    SumoVehicleSignal.java
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
 * @author Anja Liebscher
 *
 */

public class SumoVehicleSignal implements SumoObject {

    LinkedList<Integer> ll_states;

    public SumoVehicleSignal(int code) {

        String s1 = this.getDual(code);
        String[] tmp = s1.split("");

        //init
        this.ll_states = new LinkedList<Integer>();
        for (int i = 0; i < 14; i++) {
            this.ll_states.add(0);
        }

        for (int i = tmp.length - 1; i > 0; i--) {
            int pos = tmp.length - i - 1;
            this.ll_states.set(pos, Integer.valueOf(tmp[i]));
        }

    }

    public boolean getState(SumoVehicleSignalState s) {

        boolean out = false;
        if (this.ll_states.get(s.getPos()) == 1) {
            out = true;
        }
        return out;

    }


    private String getDual(int code) {
        if (code < 2) {
            return "" + code;
        } else {
            return getDual(code / 2) + code % 2;

        }
    }

}
