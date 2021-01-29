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
/// @file    SubscribtionVariable.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.subscription;

import de.tudresden.sumo.config.Constants;

public enum SubscribtionVariable {

    inductive_loop(Constants.CMD_SUBSCRIBE_INDUCTIONLOOP_VARIABLE),
    areal_detector(Constants.CMD_SUBSCRIBE_LANEAREA_VARIABLE),
    me_detector(Constants.CMD_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE),
    trafficlight(Constants.CMD_SUBSCRIBE_TL_VARIABLE),
    lane(Constants.CMD_SUBSCRIBE_LANE_VARIABLE),
    vehicle(Constants.CMD_SUBSCRIBE_VEHICLE_VARIABLE),
    vehicleType(Constants.CMD_SUBSCRIBE_VEHICLETYPE_VARIABLE),
    route(Constants.CMD_SUBSCRIBE_ROUTE_VARIABLE),
    poi(Constants.CMD_SUBSCRIBE_POI_VARIABLE),
    polygon(Constants.CMD_SUBSCRIBE_POLYGON_VARIABLE),
    person(Constants.CMD_SUBSCRIBE_PERSON_VARIABLE),
    junction(Constants.CMD_SUBSCRIBE_JUNCTION_VARIABLE),
    simulation(Constants.CMD_SUBSCRIBE_SIM_VARIABLE),
    gui(Constants.CMD_SUBSCRIBE_GUI_VARIABLE),
    edge(Constants.CMD_SUBSCRIBE_EDGE_VARIABLE);


    int id;
    SubscribtionVariable(int id) {
        this.id = id;
    }

    public int getID() {
        return this.id;
    }

}
