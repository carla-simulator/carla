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
/// @file    SumoPrimitive.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws.container;

public class SumoPrimitive implements SumoObject {

    public Object val;
    public String type;

    public SumoPrimitive(Object o) {

        this.val = o;
        if (o.getClass().equals(Integer.class)) {
            type = "integer";
        } else if (o.getClass().equals(Double.class)) {
            type = "double";
        } else if (o.getClass().equals(String.class)) {
            type = "string";
        } else {
            System.err.println("unsupported format " + o.getClass());
        }

    }

}
