/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    TraCIConstants.h
/// @author  Axel Wegener
/// @author  Friedemann Wesner
/// @author  Bjoern Hendriks
/// @author  Daniel Krajzewicz
/// @author  Thimor Bohn
/// @author  Tino Morenz
/// @author  Michael Behrisch
/// @author  Christoph Sommer
/// @author  Mario Krumnow
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @date    2007/10/24
///
// holds codes used for TraCI
/****************************************************************************/
#pragma once

#if __cplusplus >= 201103L
#define TRACI_CONST constexpr
#else
#define TRACI_CONST const
#endif

namespace libsumo {

// ****************************************
// VERSION
// ****************************************
TRACI_CONST int TRACI_VERSION = 20;

// ****************************************
// COMMANDS
// ****************************************
// command: get version
TRACI_CONST int CMD_GETVERSION = 0x00;

// command: load
TRACI_CONST int CMD_LOAD = 0x01;

// command: simulation step
TRACI_CONST int CMD_SIMSTEP = 0x02;

// command: set connection priority (execution order)
TRACI_CONST int CMD_SETORDER = 0x03;

// command: stop node
TRACI_CONST int CMD_STOP = 0x12;

// command: reroute to parking area
TRACI_CONST int CMD_REROUTE_TO_PARKING = 0xc2;

// command: Resume from parking
TRACI_CONST int CMD_RESUME = 0x19;

// command: set lane
TRACI_CONST int CMD_CHANGELANE = 0x13;

// command: slow down
TRACI_CONST int CMD_SLOWDOWN = 0x14;

// command: set sublane (vehicle)
TRACI_CONST int CMD_CHANGESUBLANE = 0x15;

// command: open gap
TRACI_CONST int CMD_OPENGAP = 0x16;

// command: change target
TRACI_CONST int CMD_CHANGETARGET = 0x31;

// command: close sumo
TRACI_CONST int CMD_CLOSE = 0x7F;

// command: add subscription filter
TRACI_CONST int CMD_ADD_SUBSCRIPTION_FILTER = 0x7e;


// command: subscribe induction loop (e1) context
TRACI_CONST int CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT = 0x80;
// response: subscribe induction loop (e1) context
TRACI_CONST int RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT = 0x90;
// command: get induction loop (e1) variable
TRACI_CONST int CMD_GET_INDUCTIONLOOP_VARIABLE = 0xa0;
// response: get induction loop (e1) variable
TRACI_CONST int RESPONSE_GET_INDUCTIONLOOP_VARIABLE = 0xb0;
// command: set induction loop (e1) variable, not used yet
//TRACI_CONST int CMD_GET_INDUCTIONLOOP_VARIABLE = 0xc0;
// command: subscribe induction loop (e1) variable
TRACI_CONST int CMD_SUBSCRIBE_INDUCTIONLOOP_VARIABLE = 0xd0;
// response: subscribe induction loop (e1) variable
TRACI_CONST int RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE = 0xe0;

// command: subscribe multi-entry/multi-exit detector (e3) context
TRACI_CONST int CMD_SUBSCRIBE_MULTIENTRYEXIT_CONTEXT = 0x81;
// response: subscribe multi-entry/multi-exit detector (e3) context
TRACI_CONST int RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_CONTEXT = 0x91;
// command: get multi-entry/multi-exit detector (e3) variable
TRACI_CONST int CMD_GET_MULTIENTRYEXIT_VARIABLE = 0xa1;
// response: get multi-entry/multi-exit detector (e3) variable
TRACI_CONST int RESPONSE_GET_MULTIENTRYEXIT_VARIABLE = 0xb1;
// command: set multi-entry/multi-exit detector (e3) variable, not used yet
//TRACI_CONST int CMD_GET_MULTIENTRYEXIT_VARIABLE = 0xc1;
// command: subscribe multi-entry/multi-exit detector (e3) variable
TRACI_CONST int CMD_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE = 0xd1;
// response: subscribe multi-entry/multi-exit detector (e3) variable
TRACI_CONST int RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE = 0xe1;

// command: subscribe traffic lights context
TRACI_CONST int CMD_SUBSCRIBE_TL_CONTEXT = 0x82;
// response: subscribe traffic lights context
TRACI_CONST int RESPONSE_SUBSCRIBE_TL_CONTEXT = 0x92;
// command: get traffic lights variable
TRACI_CONST int CMD_GET_TL_VARIABLE = 0xa2;
// response: get traffic lights variable
TRACI_CONST int RESPONSE_GET_TL_VARIABLE = 0xb2;
// command: set traffic lights variable
TRACI_CONST int CMD_SET_TL_VARIABLE = 0xc2;
// command: subscribe traffic lights variable
TRACI_CONST int CMD_SUBSCRIBE_TL_VARIABLE = 0xd2;
// response: subscribe traffic lights variable
TRACI_CONST int RESPONSE_SUBSCRIBE_TL_VARIABLE = 0xe2;

// command: subscribe lane context
TRACI_CONST int CMD_SUBSCRIBE_LANE_CONTEXT = 0x83;
// response: subscribe lane context
TRACI_CONST int RESPONSE_SUBSCRIBE_LANE_CONTEXT = 0x93;
// command: get lane variable
TRACI_CONST int CMD_GET_LANE_VARIABLE = 0xa3;
// response: get lane variable
TRACI_CONST int RESPONSE_GET_LANE_VARIABLE = 0xb3;
// command: set lane variable
TRACI_CONST int CMD_SET_LANE_VARIABLE = 0xc3;
// command: subscribe lane variable
TRACI_CONST int CMD_SUBSCRIBE_LANE_VARIABLE = 0xd3;
// response: subscribe lane variable
TRACI_CONST int RESPONSE_SUBSCRIBE_LANE_VARIABLE = 0xe3;

// command: subscribe vehicle context
TRACI_CONST int CMD_SUBSCRIBE_VEHICLE_CONTEXT = 0x84;
// response: subscribe vehicle context
TRACI_CONST int RESPONSE_SUBSCRIBE_VEHICLE_CONTEXT = 0x94;
// command: get vehicle variable
TRACI_CONST int CMD_GET_VEHICLE_VARIABLE = 0xa4;
// response: get vehicle variable
TRACI_CONST int RESPONSE_GET_VEHICLE_VARIABLE = 0xb4;
// command: set vehicle variable
TRACI_CONST int CMD_SET_VEHICLE_VARIABLE = 0xc4;
// command: subscribe vehicle variable
TRACI_CONST int CMD_SUBSCRIBE_VEHICLE_VARIABLE = 0xd4;
// response: subscribe vehicle variable
TRACI_CONST int RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE = 0xe4;

// command: subscribe vehicle type context
TRACI_CONST int CMD_SUBSCRIBE_VEHICLETYPE_CONTEXT = 0x85;
// response: subscribe vehicle type context
TRACI_CONST int RESPONSE_SUBSCRIBE_VEHICLETYPE_CONTEXT = 0x95;
// command: get vehicle type variable
TRACI_CONST int CMD_GET_VEHICLETYPE_VARIABLE = 0xa5;
// response: get vehicle type variable
TRACI_CONST int RESPONSE_GET_VEHICLETYPE_VARIABLE = 0xb5;
// command: set vehicle type variable
TRACI_CONST int CMD_SET_VEHICLETYPE_VARIABLE = 0xc5;
// command: subscribe vehicle type variable
TRACI_CONST int CMD_SUBSCRIBE_VEHICLETYPE_VARIABLE = 0xd5;
// response: subscribe vehicle type variable
TRACI_CONST int RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE = 0xe5;

// command: subscribe route context
TRACI_CONST int CMD_SUBSCRIBE_ROUTE_CONTEXT = 0x86;
// response: subscribe route context
TRACI_CONST int RESPONSE_SUBSCRIBE_ROUTE_CONTEXT = 0x96;
// command: get route variable
TRACI_CONST int CMD_GET_ROUTE_VARIABLE = 0xa6;
// response: get route variable
TRACI_CONST int RESPONSE_GET_ROUTE_VARIABLE = 0xb6;
// command: set route variable
TRACI_CONST int CMD_SET_ROUTE_VARIABLE = 0xc6;
// command: subscribe route variable
TRACI_CONST int CMD_SUBSCRIBE_ROUTE_VARIABLE = 0xd6;
// response: subscribe route variable
TRACI_CONST int RESPONSE_SUBSCRIBE_ROUTE_VARIABLE = 0xe6;

// command: subscribe poi context
TRACI_CONST int CMD_SUBSCRIBE_POI_CONTEXT = 0x87;
// response: subscribe poi context
TRACI_CONST int RESPONSE_SUBSCRIBE_POI_CONTEXT = 0x97;
// command: get poi variable
TRACI_CONST int CMD_GET_POI_VARIABLE = 0xa7;
// response: get poi variable
TRACI_CONST int RESPONSE_GET_POI_VARIABLE = 0xb7;
// command: set poi variable
TRACI_CONST int CMD_SET_POI_VARIABLE = 0xc7;
// command: subscribe poi variable
TRACI_CONST int CMD_SUBSCRIBE_POI_VARIABLE = 0xd7;
// response: subscribe poi variable
TRACI_CONST int RESPONSE_SUBSCRIBE_POI_VARIABLE = 0xe7;

// command: subscribe polygon context
TRACI_CONST int CMD_SUBSCRIBE_POLYGON_CONTEXT = 0x88;
// response: subscribe polygon context
TRACI_CONST int RESPONSE_SUBSCRIBE_POLYGON_CONTEXT = 0x98;
// command: get polygon variable
TRACI_CONST int CMD_GET_POLYGON_VARIABLE = 0xa8;
// response: get polygon variable
TRACI_CONST int RESPONSE_GET_POLYGON_VARIABLE = 0xb8;
// command: set polygon variable
TRACI_CONST int CMD_SET_POLYGON_VARIABLE = 0xc8;
// command: subscribe polygon variable
TRACI_CONST int CMD_SUBSCRIBE_POLYGON_VARIABLE = 0xd8;
// response: subscribe polygon variable
TRACI_CONST int RESPONSE_SUBSCRIBE_POLYGON_VARIABLE = 0xe8;

// command: subscribe junction context
TRACI_CONST int CMD_SUBSCRIBE_JUNCTION_CONTEXT = 0x89;
// response: subscribe junction context
TRACI_CONST int RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT = 0x99;
// command: get junction variable
TRACI_CONST int CMD_GET_JUNCTION_VARIABLE = 0xa9;
// response: get junction variable
TRACI_CONST int RESPONSE_GET_JUNCTION_VARIABLE = 0xb9;
// command: set junction variable
TRACI_CONST int CMD_SET_JUNCTION_VARIABLE = 0xc9;
// command: subscribe junction variable
TRACI_CONST int CMD_SUBSCRIBE_JUNCTION_VARIABLE = 0xd9;
// response: subscribe junction variable
TRACI_CONST int RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE = 0xe9;

// command: subscribe edge context
TRACI_CONST int CMD_SUBSCRIBE_EDGE_CONTEXT = 0x8a;
// response: subscribe edge context
TRACI_CONST int RESPONSE_SUBSCRIBE_EDGE_CONTEXT = 0x9a;
// command: get edge variable
TRACI_CONST int CMD_GET_EDGE_VARIABLE = 0xaa;
// response: get edge variable
TRACI_CONST int RESPONSE_GET_EDGE_VARIABLE = 0xba;
// command: set edge variable
TRACI_CONST int CMD_SET_EDGE_VARIABLE = 0xca;
// command: subscribe edge variable
TRACI_CONST int CMD_SUBSCRIBE_EDGE_VARIABLE = 0xda;
// response: subscribe edge variable
TRACI_CONST int RESPONSE_SUBSCRIBE_EDGE_VARIABLE = 0xea;

// command: subscribe simulation context
TRACI_CONST int CMD_SUBSCRIBE_SIM_CONTEXT = 0x8b;
// response: subscribe simulation context
TRACI_CONST int RESPONSE_SUBSCRIBE_SIM_CONTEXT = 0x9b;
// command: get simulation variable
TRACI_CONST int CMD_GET_SIM_VARIABLE = 0xab;
// response: get simulation variable
TRACI_CONST int RESPONSE_GET_SIM_VARIABLE = 0xbb;
// command: set simulation variable
TRACI_CONST int CMD_SET_SIM_VARIABLE = 0xcb;
// command: subscribe simulation variable
TRACI_CONST int CMD_SUBSCRIBE_SIM_VARIABLE = 0xdb;
// response: subscribe simulation variable
TRACI_CONST int RESPONSE_SUBSCRIBE_SIM_VARIABLE = 0xeb;

// command: subscribe GUI context
TRACI_CONST int CMD_SUBSCRIBE_GUI_CONTEXT = 0x8c;
// response: subscribe GUI context
TRACI_CONST int RESPONSE_SUBSCRIBE_GUI_CONTEXT = 0x9c;
// command: get GUI variable
TRACI_CONST int CMD_GET_GUI_VARIABLE = 0xac;
// response: get GUI variable
TRACI_CONST int RESPONSE_GET_GUI_VARIABLE = 0xbc;
// command: set GUI variable
TRACI_CONST int CMD_SET_GUI_VARIABLE = 0xcc;
// command: subscribe GUI variable
TRACI_CONST int CMD_SUBSCRIBE_GUI_VARIABLE = 0xdc;
// response: subscribe GUI variable
TRACI_CONST int RESPONSE_SUBSCRIBE_GUI_VARIABLE = 0xec;

// command: subscribe lane area detector (e2) context
TRACI_CONST int CMD_SUBSCRIBE_LANEAREA_CONTEXT = 0x8d;
// response: subscribe lane area detector (e2) context
TRACI_CONST int RESPONSE_SUBSCRIBE_LANEAREA_CONTEXT = 0x9d;
// command: get lane area detector (e2) variable
TRACI_CONST int CMD_GET_LANEAREA_VARIABLE = 0xad;
// response: get lane area detector (e2) variable
TRACI_CONST int RESPONSE_GET_LANEAREA_VARIABLE = 0xbd;
// command: set lane area detector (e2) variable, not used yet
//TRACI_CONST int CMD_GET_LANEAREA_VARIABLE = 0xcd;
// command: subscribe lane area detector (e2) variable
TRACI_CONST int CMD_SUBSCRIBE_LANEAREA_VARIABLE = 0xdd;
// response: subscribe lane area detector (e2) variable
TRACI_CONST int RESPONSE_SUBSCRIBE_LANEAREA_VARIABLE = 0xed;

// command: subscribe person context
TRACI_CONST int CMD_SUBSCRIBE_PERSON_CONTEXT = 0x8e;
// response: subscribe person context
TRACI_CONST int RESPONSE_SUBSCRIBE_PERSON_CONTEXT = 0x9e;
// command: get person variable
TRACI_CONST int CMD_GET_PERSON_VARIABLE = 0xae;
// response: get person variable
TRACI_CONST int RESPONSE_GET_PERSON_VARIABLE = 0xbe;
// command: set person variable
TRACI_CONST int CMD_SET_PERSON_VARIABLE = 0xce;
// command: subscribe person variable
TRACI_CONST int CMD_SUBSCRIBE_PERSON_VARIABLE = 0xde;
// response: subscribe person variable
TRACI_CONST int RESPONSE_SUBSCRIBE_PERSON_VARIABLE = 0xee;

// command: subscribe busstop context
TRACI_CONST int CMD_SUBSCRIBE_BUSSTOP_CONTEXT = 0x8f;
// response: subscribe busstop context
TRACI_CONST int RESPONSE_SUBSCRIBE_BUSSTOP_CONTEXT = 0x9f;
// command: get busstop variable
TRACI_CONST int CMD_GET_BUSSTOP_VARIABLE = 0xaf;
// response: get busstop variable
TRACI_CONST int RESPONSE_GET_BUSSTOP_VARIABLE = 0xbf;
// command: set busstop variable, not used yet
TRACI_CONST int CMD_SET_BUSSTOP_VARIABLE = 0xcf;
// command: subscribe busstop variable
TRACI_CONST int CMD_SUBSCRIBE_BUSSTOP_VARIABLE = 0xdf;
// response: subscribe busstop variable
TRACI_CONST int RESPONSE_SUBSCRIBE_BUSSTOP_VARIABLE = 0xef;

// command: subscribe parkingarea context
TRACI_CONST int CMD_SUBSCRIBE_PARKINGAREA_CONTEXT = 0x04;
// response: subscribe parkingarea context
TRACI_CONST int RESPONSE_SUBSCRIBE_PARKINGAREA_CONTEXT = 0x14;
// command: get parkingarea variable
TRACI_CONST int CMD_GET_PARKINGAREA_VARIABLE = 0x24;
// response: get parkingarea variable
TRACI_CONST int RESPONSE_GET_PARKINGAREA_VARIABLE = 0x34;
// command: set parkingarea variable
TRACI_CONST int CMD_SET_PARKINGAREA_VARIABLE = 0x44;
// command: subscribe parkingarea variable
TRACI_CONST int CMD_SUBSCRIBE_PARKINGAREA_VARIABLE = 0x54;
// response: subscribe parkingarea variable
TRACI_CONST int RESPONSE_SUBSCRIBE_PARKINGAREA_VARIABLE = 0x64;

// command: subscribe chargingstation context
TRACI_CONST int CMD_SUBSCRIBE_CHARGINGSTATION_CONTEXT = 0x05;
// response: subscribe chargingstation context
TRACI_CONST int RESPONSE_SUBSCRIBE_CHARGINGSTATION_CONTEXT = 0x15;
// command: get chargingstation variable
TRACI_CONST int CMD_GET_CHARGINGSTATION_VARIABLE = 0x25;
// response: get chargingstation variable
TRACI_CONST int RESPONSE_GET_CHARGINGSTATION_VARIABLE = 0x35;
// command: set chargingstation variable
TRACI_CONST int CMD_SET_CHARGINGSTATION_VARIABLE = 0x45;
// command: subscribe chargingstation variable
TRACI_CONST int CMD_SUBSCRIBE_CHARGINGSTATION_VARIABLE = 0x55;
// response: subscribe chargingstation variable
TRACI_CONST int RESPONSE_SUBSCRIBE_CHARGINGSTATION_VARIABLE = 0x65;

// command: subscribe routeprobe context
TRACI_CONST int CMD_SUBSCRIBE_ROUTEPROBE_CONTEXT = 0x06;
// response: subscribe routeprobe context
TRACI_CONST int RESPONSE_SUBSCRIBE_ROUTEPROBE_CONTEXT = 0x16;
// command: get routeprobe variable
TRACI_CONST int CMD_GET_ROUTEPROBE_VARIABLE = 0x26;
// response: get routeprobe variable
TRACI_CONST int RESPONSE_GET_ROUTEPROBE_VARIABLE = 0x36;
// command: set routeprobe variable
TRACI_CONST int CMD_SET_ROUTEPROBE_VARIABLE = 0x46;
// command: subscribe routeprobe variable
TRACI_CONST int CMD_SUBSCRIBE_ROUTEPROBE_VARIABLE = 0x56;
// response: subscribe routeprobe variable
TRACI_CONST int RESPONSE_SUBSCRIBE_ROUTEPROBE_VARIABLE = 0x66;

// command: subscribe calibrator context
TRACI_CONST int CMD_SUBSCRIBE_CALIBRATOR_CONTEXT = 0x07;
// response: subscribe calibrator context
TRACI_CONST int RESPONSE_SUBSCRIBE_CALIBRATOR_CONTEXT = 0x17;
// command: get calibrator variable
TRACI_CONST int CMD_GET_CALIBRATOR_VARIABLE = 0x27;
// response: get calibrator variable
TRACI_CONST int RESPONSE_GET_CALIBRATOR_VARIABLE = 0x37;
// command: set calibrator variable
TRACI_CONST int CMD_SET_CALIBRATOR_VARIABLE = 0x47;
// command: subscribe calibrator variable
TRACI_CONST int CMD_SUBSCRIBE_CALIBRATOR_VARIABLE = 0x57;
// response: subscribe calibrator variable
TRACI_CONST int RESPONSE_SUBSCRIBE_CALIBRATOR_VARIABLE = 0x67;

// command: subscribe rerouter context
TRACI_CONST int CMD_SUBSCRIBE_REROUTER_CONTEXT = 0x08;
// response: subscribe rerouter context
TRACI_CONST int RESPONSE_SUBSCRIBE_REROUTER_CONTEXT = 0x18;
// command: get rerouter variable
TRACI_CONST int CMD_GET_REROUTER_VARIABLE = 0x28;
// response: get rerouter variable
TRACI_CONST int RESPONSE_GET_REROUTER_VARIABLE = 0x38;
// command: set rerouter variable
TRACI_CONST int CMD_SET_REROUTER_VARIABLE = 0x48;
// command: subscribe rerouter variable
TRACI_CONST int CMD_SUBSCRIBE_REROUTER_VARIABLE = 0x58;
// response: subscribe rerouter variable
TRACI_CONST int RESPONSE_SUBSCRIBE_REROUTER_VARIABLE = 0x68;

// command: subscribe variablespeedsign context
TRACI_CONST int CMD_SUBSCRIBE_VARIABLESPEEDSIGN_CONTEXT = 0x09;
// response: subscribe variablespeedsign context
TRACI_CONST int RESPONSE_SUBSCRIBE_VARIABLESPEEDSIGN_CONTEXT = 0x19;
// command: get variablespeedsign variable
TRACI_CONST int CMD_GET_VARIABLESPEEDSIGN_VARIABLE = 0x29;
// response: get variablespeedsign variable
TRACI_CONST int RESPONSE_GET_VARIABLESPEEDSIGN_VARIABLE = 0x39;
// command: set variablespeedsign variable
TRACI_CONST int CMD_SET_VARIABLESPEEDSIGN_VARIABLE = 0x49;
// command: subscribe variablespeedsign variable
TRACI_CONST int CMD_SUBSCRIBE_VARIABLESPEEDSIGN_VARIABLE = 0x59;
// response: subscribe variablespeedsign variable
TRACI_CONST int RESPONSE_SUBSCRIBE_VARIABLESPEEDSIGN_VARIABLE = 0x69;

// command: subscribe meandata context
TRACI_CONST int CMD_SUBSCRIBE_MEANDATA_CONTEXT = 0x0a;
// response: subscribe meandata context
TRACI_CONST int RESPONSE_SUBSCRIBE_MEANDATA_CONTEXT = 0x1a;
// command: get meandata variable
TRACI_CONST int CMD_GET_MEANDATA_VARIABLE = 0x2a;
// response: get meandata variable
TRACI_CONST int RESPONSE_GET_MEANDATA_VARIABLE = 0x3a;
// command: set meandata variable, not used yet
//TRACI_CONST int CMD_SET_MEANDATA_VARIABLE = 0x4a;
// command: subscribe meandata variable
TRACI_CONST int CMD_SUBSCRIBE_MEANDATA_VARIABLE = 0x5a;
// response: subscribe meandata variable
TRACI_CONST int RESPONSE_SUBSCRIBE_MEANDATA_VARIABLE = 0x6a;

// command: subscribe overheadwire context
TRACI_CONST int CMD_SUBSCRIBE_OVERHEADWIRE_CONTEXT = 0x0b;
// response: subscribe overheadwire context
TRACI_CONST int RESPONSE_SUBSCRIBE_OVERHEADWIRE_CONTEXT = 0x1b;
// command: get overheadwire variable
TRACI_CONST int CMD_GET_OVERHEADWIRE_VARIABLE = 0x2b;
// response: get overheadwire variable
TRACI_CONST int RESPONSE_GET_OVERHEADWIRE_VARIABLE = 0x3b;
// command: set overheadwire variable
TRACI_CONST int CMD_SET_OVERHEADWIRE_VARIABLE = 0x4b;
// command: subscribe overheadwire variable
TRACI_CONST int CMD_SUBSCRIBE_OVERHEADWIRE_VARIABLE = 0x5b;
// response: subscribe overheadwire variable
TRACI_CONST int RESPONSE_SUBSCRIBE_OVERHEADWIRE_VARIABLE = 0x6b;


// ****************************************
// POSITION REPRESENTATIONS
// ****************************************
// Position in geo-coordinates
TRACI_CONST int POSITION_LON_LAT = 0x00;
// 2D cartesian coordinates
TRACI_CONST int POSITION_2D = 0x01;
// Position in geo-coordinates with altitude
TRACI_CONST int POSITION_LON_LAT_ALT = 0x02;
// 3D cartesian coordinates
TRACI_CONST int POSITION_3D = 0x03;
// Position on road map
TRACI_CONST int POSITION_ROADMAP = 0x04;


// ****************************************
// DATA TYPES
// ****************************************
// Polygon (2*n doubles)
TRACI_CONST int TYPE_POLYGON = 0x06;
// unsigned byte
TRACI_CONST int TYPE_UBYTE = 0x07;
// signed byte
TRACI_CONST int TYPE_BYTE = 0x08;
// 32 bit signed integer
TRACI_CONST int TYPE_INTEGER = 0x09;
// double precision float
TRACI_CONST int TYPE_DOUBLE = 0x0B;
// 8 bit ASCII string
TRACI_CONST int TYPE_STRING = 0x0C;
// list of strings
TRACI_CONST int TYPE_STRINGLIST = 0x0E;
// compound object
TRACI_CONST int TYPE_COMPOUND = 0x0F;
// list of double precision floats
TRACI_CONST int TYPE_DOUBLELIST = 0x10;
// color (four ubytes)
TRACI_CONST int TYPE_COLOR = 0x11;


// ****************************************
// RESULT TYPES
// ****************************************
// result type: Ok
TRACI_CONST int RTYPE_OK = 0x00;
// result type: not implemented
TRACI_CONST int RTYPE_NOTIMPLEMENTED = 0x01;
// result type: error
TRACI_CONST int RTYPE_ERR = 0xFF;

// ****************************************
// special return or parameter values
// ****************************************
// return value for invalid queries (especially vehicle is not on the road), see Position::INVALID
TRACI_CONST double INVALID_DOUBLE_VALUE = -1073741824.0;
// return value for invalid queries (especially vehicle is not on the road), see Position::INVALID
TRACI_CONST int INVALID_INT_VALUE = -1073741824;
// maximum value for client ordering (2 ^ 30)
TRACI_CONST int MAX_ORDER = 1073741824;


// ****************************************
// DIFFERENT DISTANCE REQUESTS
// ****************************************
// air distance
TRACI_CONST int REQUEST_AIRDIST = 0x00;
// driving distance
TRACI_CONST int REQUEST_DRIVINGDIST = 0x01;


// ****************************************
// VEHICLE REMOVAL REASONS
// ****************************************
// vehicle started teleport
TRACI_CONST int REMOVE_TELEPORT = 0x00;
// vehicle removed while parking
TRACI_CONST int REMOVE_PARKING = 0x01;
// vehicle arrived
TRACI_CONST int REMOVE_ARRIVED = 0x02;
// vehicle was vaporized
TRACI_CONST int REMOVE_VAPORIZED = 0x03;
// vehicle finished route during teleport
TRACI_CONST int REMOVE_TELEPORT_ARRIVED = 0x04;

// ****************************************
// VEHICLE MOVE REASONS
// ****************************************
// infer reason from move distance
TRACI_CONST int MOVE_AUTOMATIC = 0x00;
// vehicle teleports to another location
TRACI_CONST int MOVE_TELEPORT = 0x01;
// vehicle moved normally
TRACI_CONST int MOVE_NORMAL = 0x02;

// ****************************************
// PERSON/CONTAINER STAGES
// ****************************************
// person / container stopping
TRACI_CONST int STAGE_WAITING_FOR_DEPART = 0x00;
// person / container stopping
TRACI_CONST int STAGE_WAITING = 0x01;
// person walking
TRACI_CONST int STAGE_WALKING = 0x02;
// person riding / container being transported
TRACI_CONST int STAGE_DRIVING = 0x03;
// person accessing stopping place
TRACI_CONST int STAGE_ACCESS = 0x04;
// stage for encoding abstract travel demand
TRACI_CONST int STAGE_TRIP = 0x05;
// person / container transhiping
TRACI_CONST int STAGE_TRANSHIP = 0x06;

// ****************************************
// Stop Flags
// ****************************************
TRACI_CONST int STOP_DEFAULT = 0x00;
TRACI_CONST int STOP_PARKING = 0x01;
TRACI_CONST int STOP_TRIGGERED = 0x02;
TRACI_CONST int STOP_CONTAINER_TRIGGERED = 0x04;
TRACI_CONST int STOP_BUS_STOP = 0x08;
TRACI_CONST int STOP_CONTAINER_STOP = 0x10;
TRACI_CONST int STOP_CHARGING_STATION = 0x20;
TRACI_CONST int STOP_PARKING_AREA = 0x40;

// ****************************************
// Departure Flags
// ****************************************
TRACI_CONST int DEPARTFLAG_TRIGGERED = -0x01;
TRACI_CONST int DEPARTFLAG_CONTAINER_TRIGGERED = -0x02;
TRACI_CONST int DEPARTFLAG_NOW = -0x03;

TRACI_CONST int DEPARTFLAG_SPEED_RANDOM = -0x02;
TRACI_CONST int DEPARTFLAG_SPEED_MAX = -0x03;

TRACI_CONST int DEPARTFLAG_LANE_RANDOM = -0x02;
TRACI_CONST int DEPARTFLAG_LANE_FREE = -0x03;
TRACI_CONST int DEPARTFLAG_LANE_ALLOWED_FREE = -0x04;
TRACI_CONST int DEPARTFLAG_LANE_BEST_FREE = -0x05;
TRACI_CONST int DEPARTFLAG_LANE_FIRST_ALLOWED = -0x06;

TRACI_CONST int DEPARTFLAG_POS_RANDOM = -0x02;
TRACI_CONST int DEPARTFLAG_POS_FREE = -0x03;
TRACI_CONST int DEPARTFLAG_POS_BASE = -0x04;
TRACI_CONST int DEPARTFLAG_POS_LAST = -0x05;
TRACI_CONST int DEPARTFLAG_POS_RANDOM_FREE = -0x06;

TRACI_CONST int ARRIVALFLAG_LANE_CURRENT = -0x02;
TRACI_CONST int ARRIVALFLAG_SPEED_CURRENT = -0x02;

TRACI_CONST int ARRIVALFLAG_POS_RANDOM = -0x02;
TRACI_CONST int ARRIVALFLAG_POS_MAX = -0x03;

// ****************************************
// Routing modes
// ****************************************
// use custom weights if available, fall back to loaded weights and then to free-flow speed
TRACI_CONST int ROUTING_MODE_DEFAULT = 0x00;
// use aggregated travel times from device.rerouting
TRACI_CONST int ROUTING_MODE_AGGREGATED = 0x01;
// use loaded efforts
TRACI_CONST int ROUTING_MODE_EFFORT = 0x02;
// use combined costs
TRACI_CONST int ROUTING_MODE_COMBINED = 0x03;

// ****************************************
// FILTER TYPES (for context subscription filters)
// ****************************************

// Reset all filters
TRACI_CONST int FILTER_TYPE_NONE = 0x00;

// Filter by list of lanes relative to ego vehicle
TRACI_CONST int FILTER_TYPE_LANES = 0x01;

// Exclude vehicles on opposite (and other) lanes from context subscription result
TRACI_CONST int FILTER_TYPE_NOOPPOSITE = 0x02;

// Specify maximal downstream distance for vehicles in context subscription result
TRACI_CONST int FILTER_TYPE_DOWNSTREAM_DIST = 0x03;

// Specify maximal upstream distance for vehicles in context subscription result
TRACI_CONST int FILTER_TYPE_UPSTREAM_DIST = 0x04;

// Only return leader and follower on the specified lanes in context subscription result
TRACI_CONST int FILTER_TYPE_LEAD_FOLLOW = 0x05;

// Only return foes on upcoming junction in context subscription result
TRACI_CONST int FILTER_TYPE_TURN = 0x07;

// Only return vehicles of the given vClass in context subscription result
TRACI_CONST int FILTER_TYPE_VCLASS = 0x08;

// Only return vehicles of the given vType in context subscription result
TRACI_CONST int FILTER_TYPE_VTYPE = 0x09;

// Only return vehicles within field of vision in context subscription result
TRACI_CONST int FILTER_TYPE_FIELD_OF_VISION = 0x0A;

// Only return vehicles within the given lateral distance in context subscription result
TRACI_CONST int FILTER_TYPE_LATERAL_DIST = 0x0B;

// ****************************************
// VARIABLE TYPES (for CMD_GET_*_VARIABLE)
// ****************************************
// list of instances' ids (get: all)
TRACI_CONST int TRACI_ID_LIST = 0x00;

// count of instances (get: all)
TRACI_CONST int ID_COUNT = 0x01;

// subscribe object variables (get: all)
TRACI_CONST int AUTOMATIC_VARIABLES_SUBSCRIPTION = 0x02;

// subscribe context variables (get: all)
TRACI_CONST int AUTOMATIC_CONTEXT_SUBSCRIPTION = 0x03;

// generic attributes (get/set: all)
TRACI_CONST int GENERIC_ATTRIBUTE = 0x03;

// last step vehicle number (get: induction loops, multi-entry/multi-exit detector, lanes, edges)
TRACI_CONST int LAST_STEP_VEHICLE_NUMBER = 0x10;

// last step vehicle number (get: induction loops, multi-entry/multi-exit detector, lanes, edges)
TRACI_CONST int LAST_STEP_MEAN_SPEED = 0x11;

// last step vehicle list (get: induction loops, multi-entry/multi-exit detector, lanes, edges)
TRACI_CONST int LAST_STEP_VEHICLE_ID_LIST = 0x12;

// last step occupancy (get: induction loops, lanes, edges)
TRACI_CONST int LAST_STEP_OCCUPANCY = 0x13;

// last step vehicle halting number (get: multi-entry/multi-exit detector, lanes, edges)
TRACI_CONST int LAST_STEP_VEHICLE_HALTING_NUMBER = 0x14;

// last step mean vehicle length (get: induction loops, lanes, edges)
TRACI_CONST int LAST_STEP_LENGTH = 0x15;

// last step time since last detection (get: induction loops)
TRACI_CONST int LAST_STEP_TIME_SINCE_DETECTION = 0x16;

// entry times
TRACI_CONST int LAST_STEP_VEHICLE_DATA = 0x17;

// last step jam length in vehicles
TRACI_CONST int JAM_LENGTH_VEHICLE = 0x18;

// last step jam length in meters
TRACI_CONST int JAM_LENGTH_METERS = 0x19;

// last step person list (get: edges, vehicles)
TRACI_CONST int LAST_STEP_PERSON_ID_LIST = 0x1a;

// full name (get: edges, simulation, trafficlight)
TRACI_CONST int VAR_NAME = 0x1b;

// carFollowModel function followSpeed (get: vehicle)
TRACI_CONST int VAR_FOLLOW_SPEED = 0x1c;

// carFollowModel function stopSpeed (get: vehicle)
TRACI_CONST int VAR_STOP_SPEED = 0x1d;

// carFollowModel function getSecureGap (get: vehicle)
TRACI_CONST int VAR_SECURE_GAP = 0x1e;

// estimated delay for next stop
TRACI_CONST int VAR_STOP_DELAY = 0x1f;

//  begin time(get: calibrator)
TRACI_CONST int VAR_BEGIN = 0x1c;

//  end time(get: calibrator)
TRACI_CONST int VAR_END = 0x1d;

// vtype list (get: calibrator)
TRACI_CONST int VAR_VTYPES = 0x1e;

//  vehicles per hour (get: calibrator)
TRACI_CONST int VAR_VEHSPERHOUR = 0x13;

// passed vehicle count (get: calibrator)
TRACI_CONST int VAR_PASSED = 0x14;

// inserted vehicle count (get: calibrator)
TRACI_CONST int VAR_INSERTED = 0x15;

// removed vehicle count (get: calibrator)
TRACI_CONST int VAR_REMOVED = 0x16;

// routeProbe id (get: calibrator)
TRACI_CONST int VAR_ROUTE_PROBE = 0x17;

// routeProbe id (get: calibrator)
TRACI_CONST int CMD_SET_FLOW = 0x18;

// traffic light states, encoded as rRgGyYoO tuple (get: traffic lights)
TRACI_CONST int TL_RED_YELLOW_GREEN_STATE = 0x20;

// index of the phase (set: traffic lights)
TRACI_CONST int TL_PHASE_INDEX = 0x22;

// traffic light program (set: traffic lights)
TRACI_CONST int TL_PROGRAM = 0x23;

// phase duration (set: traffic lights)
TRACI_CONST int TL_PHASE_DURATION = 0x24;

// vehicles that block passing the given signal (get: traffic lights)
TRACI_CONST int TL_BLOCKING_VEHICLES = 0x25;

// controlled lanes (get: traffic lights)
TRACI_CONST int TL_CONTROLLED_LANES = 0x26;

// controlled links (get: traffic lights)
TRACI_CONST int TL_CONTROLLED_LINKS = 0x27;

// index of the current phase (get: traffic lights)
TRACI_CONST int TL_CURRENT_PHASE = 0x28;

// name of the current program (get: traffic lights)
TRACI_CONST int TL_CURRENT_PROGRAM = 0x29;

// vehicles that also wish to pass the given signal (get: traffic lights)
TRACI_CONST int TL_RIVAL_VEHICLES = 0x30;

// vehicles that also wish to pass the given signal and have higher priority (get: traffic lights)
TRACI_CONST int TL_PRIORITY_VEHICLES = 0x31;

// controlled junctions (get: traffic lights)
TRACI_CONST int TL_CONTROLLED_JUNCTIONS = 0x2a;

// complete definition (get: traffic lights)
TRACI_CONST int TL_COMPLETE_DEFINITION_RYG = 0x2b;

// complete program (set: traffic lights)
TRACI_CONST int TL_COMPLETE_PROGRAM_RYG = 0x2c;

// assumed time to next switch (get: traffic lights)
TRACI_CONST int TL_NEXT_SWITCH = 0x2d;

// current state, using external signal names (get: traffic lights)
TRACI_CONST int TL_EXTERNAL_STATE = 0x2e;

// outgoing link number (get: lanes)
TRACI_CONST int LANE_LINK_NUMBER = 0x30;

// id of parent edge (get: lanes)
TRACI_CONST int LANE_EDGE_ID = 0x31;

// outgoing link definitions (get: lanes)
TRACI_CONST int LANE_LINKS = 0x33;

// list of allowed vehicle classes (get&set: lanes)
TRACI_CONST int LANE_ALLOWED = 0x34;

// list of not allowed vehicle classes (get&set: lanes)
TRACI_CONST int LANE_DISALLOWED = 0x35;

// list of foe lanes (get: lanes)
TRACI_CONST int VAR_FOES = 0x37;

// slope (get: edge, lane, vehicle, person)
TRACI_CONST int VAR_SLOPE = 0x36;

// speed (get: vehicle)
TRACI_CONST int VAR_SPEED = 0x40;

// lateral speed (get: vehicle)
TRACI_CONST int VAR_SPEED_LAT = 0x32;

// maximum allowed/possible speed (get: vehicle types, lanes, set: edges, lanes)
TRACI_CONST int VAR_MAXSPEED = 0x41;

// position (2D) (get: vehicle, poi, inductionloop, lane area detector; set: poi)
TRACI_CONST int VAR_POSITION = 0x42;

// position (3D) (get: vehicle, poi, set: poi)
TRACI_CONST int VAR_POSITION3D = 0x39;

// angle (get: vehicle, poi; set: poi)
TRACI_CONST int VAR_ANGLE = 0x43;

// angle (get: vehicle types, lanes, lane area detector, set: lanes)
TRACI_CONST int VAR_LENGTH = 0x44;

// color (get: vehicles, vehicle types, polygons, pois)
TRACI_CONST int VAR_COLOR = 0x45;

// max. acceleration (get: vehicles, vehicle types)
TRACI_CONST int VAR_ACCEL = 0x46;

// max. comfortable deceleration (get: vehicles, vehicle types)
TRACI_CONST int VAR_DECEL = 0x47;

// max. (physically possible) deceleration (get: vehicles, vehicle types)
TRACI_CONST int VAR_EMERGENCY_DECEL = 0x7b;

// apparent deceleration (get: vehicles, vehicle types)
TRACI_CONST int VAR_APPARENT_DECEL = 0x7c;

// action step length (get: vehicles, vehicle types)
TRACI_CONST int VAR_ACTIONSTEPLENGTH = 0x7d;

// last action time (get: vehicles)
TRACI_CONST int VAR_LASTACTIONTIME = 0x7f;

// driver's desired headway (get: vehicle types)
TRACI_CONST int VAR_TAU = 0x48;

// vehicle class (get: vehicle types)
TRACI_CONST int VAR_VEHICLECLASS = 0x49;

// emission class (get: vehicle types)
TRACI_CONST int VAR_EMISSIONCLASS = 0x4a;

// shape class (get: vehicle types)
TRACI_CONST int VAR_SHAPECLASS = 0x4b;

// minimum gap (get: vehicle types)
TRACI_CONST int VAR_MINGAP = 0x4c;

// width (get: vehicle types, lanes, polygons, poi)
TRACI_CONST int VAR_WIDTH = 0x4d;

// shape (get: polygons)
TRACI_CONST int VAR_SHAPE = 0x4e;

// type id (get: vehicles, polygons, pois)
TRACI_CONST int VAR_TYPE = 0x4f;

// road id (get: vehicles)
TRACI_CONST int VAR_ROAD_ID = 0x50;

// lane id (get: vehicles, inductionloop, lane area detector)
TRACI_CONST int VAR_LANE_ID = 0x51;

// lane index (get: vehicle, edge)
TRACI_CONST int VAR_LANE_INDEX = 0x52;

// route id (get & set: vehicles)
TRACI_CONST int VAR_ROUTE_ID = 0x53;

// edges (get: routes, vehicles)
TRACI_CONST int VAR_EDGES = 0x54;

// update bestLanes (set: vehicle)
TRACI_CONST int VAR_UPDATE_BESTLANES = 0x6a;

// filled? (get: polygons)
TRACI_CONST int VAR_FILL = 0x55;

// get/set image file (poi, poly, vehicle, person, simulation)
TRACI_CONST int VAR_IMAGEFILE = 0x93;

// position (1D along lane) (get: vehicle)
TRACI_CONST int VAR_LANEPOSITION = 0x56;

// route (set: vehicles)
TRACI_CONST int VAR_ROUTE = 0x57;

// travel time information (get&set: vehicle)
TRACI_CONST int VAR_EDGE_TRAVELTIME = 0x58;

// effort information (get&set: vehicle)
TRACI_CONST int VAR_EDGE_EFFORT = 0x59;

// last step travel time (get: edge, lane)
TRACI_CONST int VAR_CURRENT_TRAVELTIME = 0x5a;

// signals state (get/set: vehicle)
TRACI_CONST int VAR_SIGNALS = 0x5b;

// vehicle: new lane/position along (set: vehicle)
TRACI_CONST int VAR_MOVE_TO = 0x5c;

// polygon: add dynamics (set: polygon)
TRACI_CONST int VAR_ADD_DYNAMICS = 0x5c;

// vehicle: highlight (set: vehicle, poi)
TRACI_CONST int VAR_HIGHLIGHT = 0x6c;

// driver imperfection (set: vehicle)
TRACI_CONST int VAR_IMPERFECTION = 0x5d;

// speed factor (set: vehicle)
TRACI_CONST int VAR_SPEED_FACTOR = 0x5e;

// speed deviation (set: vehicle)
TRACI_CONST int VAR_SPEED_DEVIATION = 0x5f;

// routing mode (get/set: vehicle)
TRACI_CONST int VAR_ROUTING_MODE = 0x89;

// speed without TraCI influence (get: vehicle)
TRACI_CONST int VAR_SPEED_WITHOUT_TRACI = 0xb1;

// best lanes (get: vehicle)
TRACI_CONST int VAR_BEST_LANES = 0xb2;

// how speed is set (set: vehicle)
TRACI_CONST int VAR_SPEEDSETMODE = 0xb3;

// move vehicle to explicit (remote controlled) position (set: vehicle)
TRACI_CONST int MOVE_TO_XY = 0xb4;

// is the vehicle stopped, and if so parked and/or triggered?
// value = stopped + 2 * parking + 4 * triggered
TRACI_CONST int VAR_STOPSTATE = 0xb5;

// how lane changing is performed (get/set: vehicle)
TRACI_CONST int VAR_LANECHANGE_MODE = 0xb6;

// maximum speed regarding max speed on the current lane and speed factor (get: vehicle)
TRACI_CONST int VAR_ALLOWED_SPEED = 0xb7;

// position (1D lateral position relative to center of the current lane) (get: vehicle)
TRACI_CONST int VAR_LANEPOSITION_LAT = 0xb8;

// get/set prefered lateral alignment within the lane (vehicle)
TRACI_CONST int VAR_LATALIGNMENT = 0xb9;

// get/set maximum lateral speed (vehicle, vtypes)
TRACI_CONST int VAR_MAXSPEED_LAT = 0xba;

// get/set minimum lateral gap (vehicle, vtypes)
TRACI_CONST int VAR_MINGAP_LAT = 0xbb;

// get/set vehicle height (vehicle, vtypes, poi)
TRACI_CONST int VAR_HEIGHT = 0xbc;

// get/set vehicle line
TRACI_CONST int VAR_LINE = 0xbd;

// get/set vehicle via
TRACI_CONST int VAR_VIA = 0xbe;

// get (lane change relevant) neighboring vehicles (vehicles)
TRACI_CONST int VAR_NEIGHBORS = 0xbf;

// current CO2 emission of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_CO2EMISSION = 0x60;

// current CO emission of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_COEMISSION = 0x61;

// current HC emission of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_HCEMISSION = 0x62;

// current PMx emission of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_PMXEMISSION = 0x63;

// current NOx emission of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_NOXEMISSION = 0x64;

// current fuel consumption of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_FUELCONSUMPTION = 0x65;

// current noise emission of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_NOISEEMISSION = 0x66;

// current person number (get: vehicle, trafficlight)
TRACI_CONST int VAR_PERSON_NUMBER = 0x67;

// person capacity (vehicle , vehicle type)
TRACI_CONST int VAR_PERSON_CAPACITY = 0x38;

TRACI_CONST int VAR_BUS_STOP_ID_LIST = 0x9f;

// number of persons waiting at a defined bus stop (get: simulation)
TRACI_CONST int VAR_BUS_STOP_WAITING = 0x67;

// ids of persons waiting at a defined bus stop (get: simulation)
TRACI_CONST int VAR_BUS_STOP_WAITING_IDS = 0xef;

// current leader together with gap (get: vehicle)
TRACI_CONST int VAR_LEADER = 0x68;

// edge index in current route (get: vehicle)
TRACI_CONST int VAR_ROUTE_INDEX = 0x69;

// current waiting time (get: vehicle, lane)
TRACI_CONST int VAR_WAITING_TIME = 0x7a;

// current waiting time (get: vehicle)
TRACI_CONST int VAR_ACCUMULATED_WAITING_TIME = 0x87;

// upcoming traffic lights (get: vehicle)
TRACI_CONST int VAR_NEXT_TLS = 0x70;

// upcoming stops (get: vehicle)
TRACI_CONST int VAR_NEXT_STOPS = 0x73;

// upcoming stops with selection (get: vehicle)
TRACI_CONST int VAR_NEXT_STOPS2 = 0x74;

// current acceleration (get: vehicle)
TRACI_CONST int VAR_ACCELERATION = 0x72;

// add log message (set: simulation)
TRACI_CONST int CMD_MESSAGE = 0x65;

// current time in seconds (get: simulation)
TRACI_CONST int VAR_TIME = 0x66;

// current time step (get: simulation)
TRACI_CONST int VAR_TIME_STEP = 0x70;

// current electricity consumption of a node (get: vehicle, lane, edge)
TRACI_CONST int VAR_ELECTRICITYCONSUMPTION = 0x71;

// number of loaded vehicles (get: simulation)
TRACI_CONST int VAR_LOADED_VEHICLES_NUMBER = 0x71;

// loaded vehicle ids (get: simulation)
TRACI_CONST int VAR_LOADED_VEHICLES_IDS = 0x72;

// number of departed vehicle (get: simulation)
TRACI_CONST int VAR_DEPARTED_VEHICLES_NUMBER = 0x73;

// departed vehicle ids (get: simulation)
TRACI_CONST int VAR_DEPARTED_VEHICLES_IDS = 0x74;

// number of vehicles starting to teleport (get: simulation)
TRACI_CONST int VAR_TELEPORT_STARTING_VEHICLES_NUMBER = 0x75;

// ids of vehicles starting to teleport (get: simulation)
TRACI_CONST int VAR_TELEPORT_STARTING_VEHICLES_IDS = 0x76;

// number of vehicles ending to teleport (get: simulation)
TRACI_CONST int VAR_TELEPORT_ENDING_VEHICLES_NUMBER = 0x77;

// ids of vehicles ending to teleport (get: simulation)
TRACI_CONST int VAR_TELEPORT_ENDING_VEHICLES_IDS = 0x78;

// number of arrived vehicles (get: simulation)
TRACI_CONST int VAR_ARRIVED_VEHICLES_NUMBER = 0x79;

// ids of arrived vehicles (get: simulation)
TRACI_CONST int VAR_ARRIVED_VEHICLES_IDS = 0x7a;

// delta t (get: simulation)
TRACI_CONST int VAR_DELTA_T = 0x7b;

// bounding box (get: simulation)
TRACI_CONST int VAR_NET_BOUNDING_BOX = 0x7c;

// minimum number of expected vehicles (get: simulation)
TRACI_CONST int VAR_MIN_EXPECTED_VEHICLES = 0x7d;

// number of vehicles starting to park (get: simulation)
TRACI_CONST int VAR_STOP_STARTING_VEHICLES_NUMBER = 0x68;

// ids of vehicles starting to park (get: simulation)
TRACI_CONST int VAR_STOP_STARTING_VEHICLES_IDS = 0x69;

// number of vehicles ending to park (get: simulation)
TRACI_CONST int VAR_STOP_ENDING_VEHICLES_NUMBER = 0x6a;

// ids of vehicles ending to park (get: simulation)
TRACI_CONST int VAR_STOP_ENDING_VEHICLES_IDS = 0x6b;

// number of vehicles starting to park (get: simulation)
TRACI_CONST int VAR_PARKING_STARTING_VEHICLES_NUMBER = 0x6c;

// ids of vehicles starting to park (get: simulation)
TRACI_CONST int VAR_PARKING_STARTING_VEHICLES_IDS = 0x6d;

// number of vehicles maneuvering (get: simulation)
TRACI_CONST int VAR_PARKING_MANEUVERING_VEHICLES_NUMBER = 0x3a;

// ids of vehicles maneuvering (get: simulation)
TRACI_CONST int VAR_PARKING_MANEUVERING_VEHICLES_IDS = 0x3b;

// number of vehicles ending to park (get: simulation)
TRACI_CONST int VAR_PARKING_ENDING_VEHICLES_NUMBER = 0x6e;

// ids of vehicles ending to park (get: simulation)
TRACI_CONST int VAR_PARKING_ENDING_VEHICLES_IDS = 0x6f;

// number of vehicles involved in a collision (get: simulation)
TRACI_CONST int VAR_COLLIDING_VEHICLES_NUMBER = 0x80;

// ids of vehicles involved in a collision (get: simulation)
TRACI_CONST int VAR_COLLIDING_VEHICLES_IDS = 0x81;

// number of vehicles involved in a collision (get: simulation)
TRACI_CONST int VAR_EMERGENCYSTOPPING_VEHICLES_NUMBER = 0x89;

// ids of vehicles involved in a collision (get: simulation)
TRACI_CONST int VAR_EMERGENCYSTOPPING_VEHICLES_IDS = 0x8a;

// clears the simulation of all not inserted vehicles (set: simulation)
TRACI_CONST int CMD_CLEAR_PENDING_VEHICLES = 0x94;

// triggers saving simulation state (set: simulation)
TRACI_CONST int CMD_SAVE_SIMSTATE = 0x95;

// triggers saving simulation state (set: simulation)
TRACI_CONST int CMD_LOAD_SIMSTATE = 0x96;

// sets/retrieves abstract parameter
TRACI_CONST int VAR_PARAMETER = 0x7e;

// retrieves abstract parameter and returns (key, value) tuple
TRACI_CONST int VAR_PARAMETER_WITH_KEY = 0x3e;


// add an instance (poi, polygon, vehicle, person, route)
TRACI_CONST int ADD = 0x80;

// remove an instance (poi, polygon, vehicle, person)
TRACI_CONST int REMOVE = 0x81;

// copy an instance (vehicle type, other TBD.)
TRACI_CONST int COPY = 0x88;

// convert coordinates
TRACI_CONST int POSITION_CONVERSION = 0x82;

// distance between points or vehicles
TRACI_CONST int DISTANCE_REQUEST = 0x83;

// the current driving distance
TRACI_CONST int VAR_DISTANCE = 0x84;

// add a fully specified instance (vehicle)
TRACI_CONST int ADD_FULL = 0x85;

// find a car based route
TRACI_CONST int FIND_ROUTE = 0x86;

// find an intermodal route
TRACI_CONST int FIND_INTERMODAL_ROUTE = 0x87;

// force rerouting based on travel time (vehicles)
TRACI_CONST int CMD_REROUTE_TRAVELTIME = 0x90;

// force rerouting based on effort (vehicles)
TRACI_CONST int CMD_REROUTE_EFFORT = 0x91;

// validates current route (vehicles)
TRACI_CONST int VAR_ROUTE_VALID = 0x92;

// retrieve information regarding the current person/container stage
TRACI_CONST int VAR_STAGE = 0xc0;

// retrieve information regarding the next edge including crossings and walkingAreas (pedestrians only)
TRACI_CONST int VAR_NEXT_EDGE = 0xc1;

// retrieve information regarding the number of remaining stages
TRACI_CONST int VAR_STAGES_REMAINING = 0xc2;

// retrieve the current vehicle id for the driving stage (person, container)
TRACI_CONST int VAR_VEHICLE = 0xc3;

// append a person stage (person)
TRACI_CONST int APPEND_STAGE = 0xc4;

// replace a person stage (person)
TRACI_CONST int REPLACE_STAGE = 0xcd;

// append a person stage (person)
TRACI_CONST int REMOVE_STAGE = 0xc5;

// zoom
TRACI_CONST int VAR_VIEW_ZOOM = 0xa0;

// view position
TRACI_CONST int VAR_VIEW_OFFSET = 0xa1;

// view schema
TRACI_CONST int VAR_VIEW_SCHEMA = 0xa2;

// view by boundary
TRACI_CONST int VAR_VIEW_BOUNDARY = 0xa3;

// screenshot
TRACI_CONST int VAR_SCREENSHOT = 0xa5;

// track vehicle
TRACI_CONST int VAR_TRACK_VEHICLE = 0xa6;

// presence of view
TRACI_CONST int VAR_HAS_VIEW = 0xa7;

} // namespace libsumo

#undef TRACI_CONST

