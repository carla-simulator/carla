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
/// @file    SumoDomain.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.subscription;

import de.tudresden.sumo.config.Constants;

public enum SumoDomain {

    inductive_loops(Constants.CMD_GET_INDUCTIONLOOP_VARIABLE),
    lanes(Constants.CMD_GET_LANE_VARIABLE),
    vehicles(Constants.CMD_GET_VEHICLE_VARIABLE),
    persons(Constants.CMD_GET_PERSON_VARIABLE),
    poi(Constants.CMD_GET_POI_VARIABLE),
    polygons(Constants.CMD_GET_POLYGON_VARIABLE),
    junction(Constants.CMD_GET_JUNCTION_VARIABLE),
    edges(Constants.CMD_GET_EDGE_VARIABLE);

    int id;

    SumoDomain(int id) {
        this.id = id;
    }

    public int getID() {
        return this.id;
    }

}
