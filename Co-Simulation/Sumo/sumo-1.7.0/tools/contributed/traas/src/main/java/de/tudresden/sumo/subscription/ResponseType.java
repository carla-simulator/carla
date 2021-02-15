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
/// @file    ResponseType.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.subscription;

import de.tudresden.sumo.config.Constants;

public enum ResponseType {

    UNKNOWN(-1, false),
    EDGE_VARIABLE(Constants.RESPONSE_SUBSCRIBE_EDGE_VARIABLE, true),
    AREAL_DETECTOR_VARIABLE(Constants.RESPONSE_SUBSCRIBE_LANEAREA_VARIABLE, true),
    PERSON_VARIABLE(Constants.RESPONSE_SUBSCRIBE_PERSON_VARIABLE, true),
    MULTI_ENTRY_EXIT_DETECTOR_VARIABLE(Constants.RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE, true),
    JUNCTION_VARIABLE(Constants.RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE, true),
    VEHICLE_VARIABLE(Constants.RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE, true),
    INDUCTIONLOOP_VARIABLE(Constants.RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE, true),
    GUI_VARIABLE(Constants.RESPONSE_SUBSCRIBE_GUI_VARIABLE, true),
    POI_VARIABLE(Constants.RESPONSE_SUBSCRIBE_POI_VARIABLE, true),
    VEHICLETYPE_VARIABLE(Constants.RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE, true),
    LANE_VARIABLE(Constants.RESPONSE_SUBSCRIBE_LANE_VARIABLE, true),
    POLYGON_VARIABLE(Constants.RESPONSE_SUBSCRIBE_POLYGON_VARIABLE, true),
    TL_VARIABLE(Constants.RESPONSE_SUBSCRIBE_TL_VARIABLE, true),
    ROUTE_VARIABLE(Constants.RESPONSE_SUBSCRIBE_ROUTE_VARIABLE, true),
    SIM_VARIABLE(Constants.RESPONSE_SUBSCRIBE_SIM_VARIABLE, true),

    POLYGON_CONTEXT(Constants.RESPONSE_SUBSCRIBE_POLYGON_CONTEXT, false),
    EDGE_CONTEXT(Constants.RESPONSE_SUBSCRIBE_EDGE_CONTEXT, false),
    AREAL_DETECTOR_CONTEXT(Constants.RESPONSE_SUBSCRIBE_LANEAREA_CONTEXT, false),
    PERSON_CONTEXT(Constants.RESPONSE_SUBSCRIBE_PERSON_CONTEXT, false),
    POI_CONTEXT(Constants.RESPONSE_SUBSCRIBE_POI_CONTEXT, false),
    GUI_CONTEXT(Constants.RESPONSE_SUBSCRIBE_GUI_CONTEXT, false),
    MULTI_ENTRY_EXIT_DETECTOR_CONTEXT(Constants.RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_CONTEXT, false),
    INDUCTIONLOOP_CONTEXT(Constants.RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT, false),
    ROUTE_CONTEXT(Constants.RESPONSE_SUBSCRIBE_ROUTE_CONTEXT, false),
    VEHICLE_CONTEXT(Constants.RESPONSE_SUBSCRIBE_VEHICLE_CONTEXT, false),
    VEHICLETYPE_CONTEXT(Constants.RESPONSE_SUBSCRIBE_VEHICLETYPE_CONTEXT, false),
    JUNCTION_CONTEXT(Constants.RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT, false),
    SIM_CONTEXT(Constants.RESPONSE_SUBSCRIBE_SIM_CONTEXT, false),
    LANE_CONTEXT(Constants.RESPONSE_SUBSCRIBE_LANE_CONTEXT, false),
    TL_CONTEXT(Constants.RESPONSE_SUBSCRIBE_TL_CONTEXT, false);

    boolean variable;
    int id;

    ResponseType(int id, boolean variable) {
        this.id = id;
        this.variable = variable;
    }

    public boolean isVariable() {
        return this.variable;
    }

    public boolean isContext() {
        return !this.variable;
    }

    public static ResponseType getType(int response) {

        ResponseType rt;

        switch (response) {

            case Constants.RESPONSE_SUBSCRIBE_EDGE_VARIABLE :
                rt = ResponseType.EDGE_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_LANEAREA_VARIABLE :
                rt = ResponseType.AREAL_DETECTOR_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_PERSON_VARIABLE :
                rt = ResponseType.PERSON_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE :
                rt = ResponseType.MULTI_ENTRY_EXIT_DETECTOR_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE :
                rt = ResponseType.JUNCTION_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE :
                rt = ResponseType.VEHICLE_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE :
                rt = ResponseType.INDUCTIONLOOP_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_GUI_VARIABLE :
                rt = ResponseType.GUI_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_POI_VARIABLE :
                rt = ResponseType.POI_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE :
                rt = ResponseType.VEHICLETYPE_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_LANE_VARIABLE :
                rt = ResponseType.LANE_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_POLYGON_VARIABLE :
                rt = ResponseType.POLYGON_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_TL_VARIABLE :
                rt = ResponseType.TL_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_ROUTE_VARIABLE :
                rt = ResponseType.ROUTE_VARIABLE;
                break;
            case Constants.RESPONSE_SUBSCRIBE_SIM_VARIABLE :
                rt = ResponseType.SIM_VARIABLE;
                break;

            case Constants.RESPONSE_SUBSCRIBE_POLYGON_CONTEXT :
                rt = ResponseType.POLYGON_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_EDGE_CONTEXT :
                rt = ResponseType.EDGE_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_LANEAREA_CONTEXT :
                rt = ResponseType.AREAL_DETECTOR_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_PERSON_CONTEXT :
                rt = ResponseType.PERSON_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_POI_CONTEXT :
                rt = ResponseType.POI_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_GUI_CONTEXT :
                rt = ResponseType.GUI_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_CONTEXT :
                rt = ResponseType.MULTI_ENTRY_EXIT_DETECTOR_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT :
                rt = ResponseType.INDUCTIONLOOP_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_ROUTE_CONTEXT :
                rt = ResponseType.ROUTE_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_VEHICLE_CONTEXT :
                rt = ResponseType.VEHICLE_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_VEHICLETYPE_CONTEXT :
                rt = ResponseType.VEHICLETYPE_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT :
                rt = ResponseType.JUNCTION_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_SIM_CONTEXT :
                rt = ResponseType.SIM_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_LANE_CONTEXT :
                rt = ResponseType.LANE_CONTEXT;
                break;
            case Constants.RESPONSE_SUBSCRIBE_TL_CONTEXT :
                rt = ResponseType.TL_CONTEXT;
                break;
            default:
                rt = ResponseType.UNKNOWN;

        }

        return rt;

    }
}
