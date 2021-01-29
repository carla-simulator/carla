//
// Copyright (C) 2019 Universidad Nacional de Colombia, Politecnico Jaime Isaza Cadavid.
// Copyright (C) 2019 Andres Acosta, Jorge Espinosa, Jairo Espinosa
// Copyright (C) 2009 Rodney Thomson
// Copyright (C) 2008 Rodney Thomson
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: BSD-3-Clause
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution
// * Neither the name of Universidad Nacional de Colombia, Politécnico Jaime Isaza Cadavid nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

// TraCI4Matlab 3.0.0.0
// $Id: constants.m 53 2019-01-03 15:18:31Z afacostag $
// The SUMO hexadecimal constants.
// Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.

#pragma once

namespace veins {
namespace TraCIConstants {

const uint32_t TRACI_VERSION = 19;

const double INVALID_DOUBLE_VALUE = -1073741824;
const int32_t INVALID_INT_VALUE = -1073741824;

const uint8_t ADD = 0x80;
const uint8_t ADD_FULL = 0x85;
const uint8_t APPEND_STAGE = 0xc4;
const int8_t ARRIVALFLAG_LANE_CURRENT = -0x02;
const int8_t ARRIVALFLAG_POS_MAX = -0x03;
const int8_t ARRIVALFLAG_POS_RANDOM = -0x02;
const int8_t ARRIVALFLAG_SPEED_CURRENT = -0x02;
const uint8_t AUTOMATIC_CONTEXT_SUBSCRIPTION = 0x03;
const uint8_t AUTOMATIC_VARIABLES_SUBSCRIPTION = 0x02;
const uint8_t CMD_ADD_SUBSCRIPTION_FILTER = 0x7e;
const uint8_t CMD_CHANGELANE = 0x13;
const uint8_t CMD_CHANGESUBLANE = 0x15;
const uint8_t CMD_CHANGETARGET = 0x31;
const uint8_t CMD_CLEAR_PENDING_VEHICLES = 0x94;
const uint8_t CMD_CLOSE = 0x7F;
const uint8_t CMD_GETVERSION = 0x00;
const uint8_t CMD_GET_EDGE_VARIABLE = 0xaa;
const uint8_t CMD_GET_GUI_VARIABLE = 0xac;
const uint8_t CMD_GET_INDUCTIONLOOP_VARIABLE = 0xa0;
const uint8_t CMD_GET_JUNCTION_VARIABLE = 0xa9;
const uint8_t CMD_GET_LANEAREA_VARIABLE = 0xad;
const uint8_t CMD_GET_LANE_VARIABLE = 0xa3;
const uint8_t CMD_GET_MULTIENTRYEXIT_VARIABLE = 0xa1; // renamed for compatibility
const uint8_t CMD_GET_PERSON_VARIABLE = 0xae;
const uint8_t CMD_GET_POI_VARIABLE = 0xa7;
const uint8_t CMD_GET_POLYGON_VARIABLE = 0xa8;
const uint8_t CMD_GET_ROUTE_VARIABLE = 0xa6;
const uint8_t CMD_GET_SIM_VARIABLE = 0xab;
const uint8_t CMD_GET_TL_VARIABLE = 0xa2;
const uint8_t CMD_GET_VEHICLETYPE_VARIABLE = 0xa5;
const uint8_t CMD_GET_VEHICLE_VARIABLE = 0xa4;
const uint8_t CMD_LOAD = 0x01;
const uint8_t CMD_OPENGAP = 0x16;
const uint8_t CMD_REROUTE_EFFORT = 0x91;
const uint8_t CMD_REROUTE_TO_PARKING = 0xc2;
const uint8_t CMD_REROUTE_TRAVELTIME = 0x90;
const uint8_t CMD_RESUME = 0x19;
const uint8_t CMD_SAVE_SIMSTATE = 0x95;
const uint8_t CMD_SETORDER = 0x03;
const uint8_t CMD_SET_EDGE_VARIABLE = 0xca;
const uint8_t CMD_SET_GUI_VARIABLE = 0xcc;
const uint8_t CMD_SET_JUNCTION_VARIABLE = 0xc9;
const uint8_t CMD_SET_LANE_VARIABLE = 0xc3;
const uint8_t CMD_SET_PERSON_VARIABLE = 0xce;
const uint8_t CMD_SET_POI_VARIABLE = 0xc7;
const uint8_t CMD_SET_POLYGON_VARIABLE = 0xc8;
const uint8_t CMD_SET_ROUTE_VARIABLE = 0xc6;
const uint8_t CMD_SET_SIM_VARIABLE = 0xcb;
const uint8_t CMD_SET_TL_VARIABLE = 0xc2;
const uint8_t CMD_SET_VEHICLETYPE_VARIABLE = 0xc5;
const uint8_t CMD_SET_VEHICLE_VARIABLE = 0xc4;
const uint8_t CMD_SIMSTEP = 0x02;
const uint8_t CMD_SIMSTEP2 = 0x02; // Veins specific (called CMD_SIMSTEP in TraCI)
const uint8_t CMD_SLOWDOWN = 0x14;
const uint8_t CMD_STOP = 0x12;
const uint8_t CMD_SUBSCRIBE_EDGE_CONTEXT = 0x8a;
const uint8_t CMD_SUBSCRIBE_EDGE_VARIABLE = 0xda;
const uint8_t CMD_SUBSCRIBE_GUI_CONTEXT = 0x8c;
const uint8_t CMD_SUBSCRIBE_GUI_VARIABLE = 0xdc;
const uint8_t CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT = 0x80;
const uint8_t CMD_SUBSCRIBE_INDUCTIONLOOP_VARIABLE = 0xd0;
const uint8_t CMD_SUBSCRIBE_JUNCTION_CONTEXT = 0x89;
const uint8_t CMD_SUBSCRIBE_JUNCTION_VARIABLE = 0xd9;
const uint8_t CMD_SUBSCRIBE_LANEAREA_CONTEXT = 0x8d;
const uint8_t CMD_SUBSCRIBE_LANEAREA_VARIABLE = 0xdd;
const uint8_t CMD_SUBSCRIBE_LANE_CONTEXT = 0x83;
const uint8_t CMD_SUBSCRIBE_LANE_VARIABLE = 0xd3;
const uint8_t CMD_SUBSCRIBE_MULTIENTRYEXIT_CONTEXT = 0x81; // renamed for compatibility
const uint8_t CMD_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE = 0xd1; // renamed for compatibility
const uint8_t CMD_SUBSCRIBE_PERSON_CONTEXT = 0x8e;
const uint8_t CMD_SUBSCRIBE_PERSON_VARIABLE = 0xde;
const uint8_t CMD_SUBSCRIBE_POI_CONTEXT = 0x87;
const uint8_t CMD_SUBSCRIBE_POI_VARIABLE = 0xd7;
const uint8_t CMD_SUBSCRIBE_POLYGON_CONTEXT = 0x88;
const uint8_t CMD_SUBSCRIBE_POLYGON_VARIABLE = 0xd8;
const uint8_t CMD_SUBSCRIBE_ROUTE_CONTEXT = 0x86;
const uint8_t CMD_SUBSCRIBE_ROUTE_VARIABLE = 0xd6;
const uint8_t CMD_SUBSCRIBE_SIM_CONTEXT = 0x8b;
const uint8_t CMD_SUBSCRIBE_SIM_VARIABLE = 0xdb;
const uint8_t CMD_SUBSCRIBE_TL_CONTEXT = 0x82;
const uint8_t CMD_SUBSCRIBE_TL_VARIABLE = 0xd2;
const uint8_t CMD_SUBSCRIBE_VEHICLETYPE_CONTEXT = 0x85;
const uint8_t CMD_SUBSCRIBE_VEHICLETYPE_VARIABLE = 0xd5;
const uint8_t CMD_SUBSCRIBE_VEHICLE_CONTEXT = 0x84;
const uint8_t CMD_SUBSCRIBE_VEHICLE_VARIABLE = 0xd4;
const uint8_t COPY = 0x88;
const int8_t DEPARTFLAG_CONTAINER_TRIGGERED = -0x02;
const int8_t DEPARTFLAG_LANE_ALLOWED_FREE = -0x04;
const int8_t DEPARTFLAG_LANE_BEST_FREE = -0x05;
const int8_t DEPARTFLAG_LANE_FIRST_ALLOWED = -0x06;
const int8_t DEPARTFLAG_LANE_FREE = -0x03;
const int8_t DEPARTFLAG_LANE_RANDOM = -0x02;
const int8_t DEPARTFLAG_NOW = -0x03;
const int8_t DEPARTFLAG_POS_BASE = -0x04;
const int8_t DEPARTFLAG_POS_FREE = -0x03;
const int8_t DEPARTFLAG_POS_LAST = -0x05;
const int8_t DEPARTFLAG_POS_RANDOM = -0x02;
const int8_t DEPARTFLAG_POS_RANDOM_FREE = -0x06;
const int8_t DEPARTFLAG_SPEED_MAX = -0x03;
const int8_t DEPARTFLAG_SPEED_RANDOM = -0x02;
const int8_t DEPARTFLAG_TRIGGERED = -0x01;
const uint8_t DISTANCE_REQUEST = 0x83;
const uint8_t FILTER_TYPE_DOWNSTREAM_DIST = 0x03;
const uint8_t FILTER_TYPE_LANES = 0x01;
const uint8_t FILTER_TYPE_LEAD_FOLLOW = 0x05;
const uint8_t FILTER_TYPE_NONE = 0x00;
const uint8_t FILTER_TYPE_NOOPPOSITE = 0x02;
const uint8_t FILTER_TYPE_TURN = 0x07;
const uint8_t FILTER_TYPE_UPSTREAM_DIST = 0x04;
const uint8_t FILTER_TYPE_VCLASS = 0x08;
const uint8_t FILTER_TYPE_VTYPE = 0x09;
const uint8_t FIND_INTERMODAL_ROUTE = 0x87;
const uint8_t FIND_ROUTE = 0x86;
const uint8_t GENERIC_ATTRIBUTE = 0x03;
const uint8_t ID_COUNT = 0x01;
const uint8_t ID_LIST = 0x00;
const uint8_t JAM_LENGTH_METERS = 0x19;
const uint8_t JAM_LENGTH_VEHICLE = 0x18;
const uint8_t LANE_ALLOWED = 0x34;
const uint8_t LANE_DISALLOWED = 0x35;
const uint8_t LANE_EDGE_ID = 0x31;
const uint8_t LANE_LINKS = 0x33;
const uint8_t LANE_LINK_NUMBER = 0x30;
const uint8_t LAST_STEP_LENGTH = 0x15;
const uint8_t LAST_STEP_MEAN_SPEED = 0x11;
const uint8_t LAST_STEP_OCCUPANCY = 0x13;
const uint8_t LAST_STEP_PERSON_ID_LIST = 0x1a;
const uint8_t LAST_STEP_TIME_SINCE_DETECTION = 0x16;
const uint8_t LAST_STEP_VEHICLE_DATA = 0x17;
const uint8_t LAST_STEP_VEHICLE_HALTING_NUMBER = 0x14;
const uint8_t LAST_STEP_VEHICLE_ID_LIST = 0x12;
const uint8_t LAST_STEP_VEHICLE_NUMBER = 0x10;
const int32_t MAX_ORDER = 1073741824;
const uint8_t MOVE_TO_XY = 0xb4;
const uint8_t OBJECT_VARIABLES_SUBSCRIPTION = 0x02; // Veins specific (called AUTOMATIC_VARIABLES_SUBSCRIPTION in TraCI)
const uint8_t POSITION_2D = 0x01;
const uint8_t POSITION_3D = 0x03;
const uint8_t POSITION_CONVERSION = 0x82;
const uint8_t POSITION_LON_LAT = 0x00;
const uint8_t POSITION_LON_LAT_ALT = 0x02;
const uint8_t POSITION_ROADMAP = 0x04;
const uint8_t REMOVE = 0x81;
const uint8_t REMOVE_ARRIVED = 0x02;
const uint8_t REMOVE_PARKING = 0x01;
const uint8_t REMOVE_STAGE = 0xc5;
const uint8_t REMOVE_TELEPORT = 0x00;
const uint8_t REMOVE_TELEPORT_ARRIVED = 0x04;
const uint8_t REMOVE_VAPORIZED = 0x03;
const uint8_t REQUEST_AIRDIST = 0x00;
const uint8_t REQUEST_DRIVINGDIST = 0x01;
const uint8_t RESPONSE_GET_EDGE_VARIABLE = 0xba;
const uint8_t RESPONSE_GET_GUI_VARIABLE = 0xbc;
const uint8_t RESPONSE_GET_INDUCTIONLOOP_VARIABLE = 0xb0;
const uint8_t RESPONSE_GET_JUNCTION_VARIABLE = 0xb9;
const uint8_t RESPONSE_GET_LANEAREA_VARIABLE = 0xbd;
const uint8_t RESPONSE_GET_LANE_VARIABLE = 0xb3;
const uint8_t RESPONSE_GET_MULTIENTRYEXIT_VARIABLE = 0xb1; // renamed for compatibility
const uint8_t RESPONSE_GET_PERSON_VARIABLE = 0xbe;
const uint8_t RESPONSE_GET_POI_VARIABLE = 0xb7;
const uint8_t RESPONSE_GET_POLYGON_VARIABLE = 0xb8;
const uint8_t RESPONSE_GET_ROUTE_VARIABLE = 0xb6;
const uint8_t RESPONSE_GET_SIM_VARIABLE = 0xbb;
const uint8_t RESPONSE_GET_TL_VARIABLE = 0xb2;
const uint8_t RESPONSE_GET_VEHICLETYPE_VARIABLE = 0xb5;
const uint8_t RESPONSE_GET_VEHICLE_VARIABLE = 0xb4;
const uint8_t RESPONSE_SUBSCRIBE_EDGE_CONTEXT = 0x9a;
const uint8_t RESPONSE_SUBSCRIBE_EDGE_VARIABLE = 0xea;
const uint8_t RESPONSE_SUBSCRIBE_GUI_CONTEXT = 0x9c;
const uint8_t RESPONSE_SUBSCRIBE_GUI_VARIABLE = 0xec;
const uint8_t RESPONSE_SUBSCRIBE_INDUCTIONLOOP_CONTEXT = 0x90;
const uint8_t RESPONSE_SUBSCRIBE_INDUCTIONLOOP_VARIABLE = 0xe0;
const uint8_t RESPONSE_SUBSCRIBE_JUNCTION_CONTEXT = 0x99;
const uint8_t RESPONSE_SUBSCRIBE_JUNCTION_VARIABLE = 0xe9;
const uint8_t RESPONSE_SUBSCRIBE_LANEAREA_CONTEXT = 0x9d;
const uint8_t RESPONSE_SUBSCRIBE_LANEAREA_VARIABLE = 0xed;
const uint8_t RESPONSE_SUBSCRIBE_LANE_CONTEXT = 0x93;
const uint8_t RESPONSE_SUBSCRIBE_LANE_VARIABLE = 0xe3;
const uint8_t RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_CONTEXT = 0x91; // renamed for compatibility
const uint8_t RESPONSE_SUBSCRIBE_MULTIENTRYEXIT_VARIABLE = 0xe1; // renamed for compatibility
const uint8_t RESPONSE_SUBSCRIBE_PERSON_CONTEXT = 0x9e;
const uint8_t RESPONSE_SUBSCRIBE_PERSON_VARIABLE = 0xee;
const uint8_t RESPONSE_SUBSCRIBE_POI_CONTEXT = 0x97;
const uint8_t RESPONSE_SUBSCRIBE_POI_VARIABLE = 0xe7;
const uint8_t RESPONSE_SUBSCRIBE_POLYGON_CONTEXT = 0x98;
const uint8_t RESPONSE_SUBSCRIBE_POLYGON_VARIABLE = 0xe8;
const uint8_t RESPONSE_SUBSCRIBE_ROUTE_CONTEXT = 0x96;
const uint8_t RESPONSE_SUBSCRIBE_ROUTE_VARIABLE = 0xe6;
const uint8_t RESPONSE_SUBSCRIBE_SIM_CONTEXT = 0x9b;
const uint8_t RESPONSE_SUBSCRIBE_SIM_VARIABLE = 0xeb;
const uint8_t RESPONSE_SUBSCRIBE_TL_CONTEXT = 0x92;
const uint8_t RESPONSE_SUBSCRIBE_TL_VARIABLE = 0xe2;
const uint8_t RESPONSE_SUBSCRIBE_VEHICLETYPE_CONTEXT = 0x95;
const uint8_t RESPONSE_SUBSCRIBE_VEHICLETYPE_VARIABLE = 0xe5;
const uint8_t RESPONSE_SUBSCRIBE_VEHICLE_CONTEXT = 0x94;
const uint8_t RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE = 0xe4;
const uint8_t ROUTING_MODE_AGGREGATED = 0x01;
const uint8_t ROUTING_MODE_COMBINED = 0x03;
const uint8_t ROUTING_MODE_DEFAULT = 0x00;
const uint8_t ROUTING_MODE_EFFORT = 0x02;
const uint8_t RTYPE_ERR = 0xFF;
const uint8_t RTYPE_NOTIMPLEMENTED = 0x01;
const uint8_t RTYPE_OK = 0x00;
const uint8_t STAGE_DRIVING = 0x03;
const uint8_t STAGE_WAITING = 0x01;
const uint8_t STAGE_WAITING_FOR_DEPART = 0x00;
const uint8_t STAGE_WALKING = 0x02;
const uint8_t STOP_BUS_STOP = 0x08;
const uint8_t STOP_CHARGING_STATION = 0x20;
const uint8_t STOP_CONTAINER_STOP = 0x10;
const uint8_t STOP_CONTAINER_TRIGGERED = 0x04;
const uint8_t STOP_DEFAULT = 0x00;
const uint8_t STOP_PARKING = 0x01;
const uint8_t STOP_PARKING_AREA = 0x40;
const uint8_t STOP_TRIGGERED = 0x02;
const uint8_t SURROUNDING_VARIABLES_SUBSCRIPTION = 0x03; // Veins specific (called AUTOMATIC_CONTEXT_SUBSCRIPTION in TraCI)
const uint8_t TL_COMPLETE_DEFINITION_RYG = 0x2b;
const uint8_t TL_COMPLETE_PROGRAM_RYG = 0x2c;
const uint8_t TL_CONTROLLED_JUNCTIONS = 0x2a;
const uint8_t TL_CONTROLLED_LANES = 0x26;
const uint8_t TL_CONTROLLED_LINKS = 0x27;
const uint8_t TL_CURRENT_PHASE = 0x28;
const uint8_t TL_CURRENT_PROGRAM = 0x29;
const uint8_t TL_EXTERNAL_STATE = 0x2e;
const uint8_t TL_NEXT_SWITCH = 0x2d;
const uint8_t TL_PHASE_DURATION = 0x24;
const uint8_t TL_PHASE_INDEX = 0x22;
const uint8_t TL_PROGRAM = 0x23;
const uint8_t TL_RED_YELLOW_GREEN_STATE = 0x20;
const uint8_t TYPE_BOUNDINGBOX = 0x05; // Retained for backwards compatibility
const uint8_t TYPE_BYTE = 0x08;
const uint8_t TYPE_COLOR = 0x11;
const uint8_t TYPE_COMPOUND = 0x0F;
const uint8_t TYPE_DOUBLE = 0x0B;
const uint8_t TYPE_INTEGER = 0x09;
const uint8_t TYPE_POLYGON = 0x06;
const uint8_t TYPE_STRING = 0x0C;
const uint8_t TYPE_STRINGLIST = 0x0E;
const uint8_t TYPE_UBYTE = 0x07;
const uint8_t VAR_ACCEL = 0x46;
const uint8_t VAR_ACCELERATION = 0x72;
const uint8_t VAR_ACCUMULATED_WAITING_TIME = 0x87;
const uint8_t VAR_ACTIONSTEPLENGTH = 0x7d;
const uint8_t VAR_ALLOWED_SPEED = 0xb7;
const uint8_t VAR_ANGLE = 0x43;
const uint8_t VAR_APPARENT_DECEL = 0x7c;
const uint8_t VAR_ARRIVED_VEHICLES_IDS = 0x7a;
const uint8_t VAR_ARRIVED_VEHICLES_NUMBER = 0x79;
const uint8_t VAR_BEST_LANES = 0xb2;
const uint8_t VAR_BUS_STOP_WAITING = 0x67;
const uint8_t VAR_CO2EMISSION = 0x60;
const uint8_t VAR_COEMISSION = 0x61;
const uint8_t VAR_COLLIDING_VEHICLES_IDS = 0x81;
const uint8_t VAR_COLLIDING_VEHICLES_NUMBER = 0x80;
const uint8_t VAR_COLOR = 0x45;
const uint8_t VAR_CURRENT_TRAVELTIME = 0x5a;
const uint8_t VAR_DECEL = 0x47;
const uint8_t VAR_DELTA_T = 0x7b;
const uint8_t VAR_DEPARTED_VEHICLES_IDS = 0x74;
const uint8_t VAR_DEPARTED_VEHICLES_NUMBER = 0x73;
const uint8_t VAR_DISTANCE = 0x84;
const uint8_t VAR_EDGES = 0x54;
const uint8_t VAR_EDGE_EFFORT = 0x59;
const uint8_t VAR_EDGE_TRAVELTIME = 0x58;
const uint8_t VAR_ELECTRICITYCONSUMPTION = 0x71;
const uint8_t VAR_EMERGENCYSTOPPING_VEHICLES_IDS = 0x8a;
const uint8_t VAR_EMERGENCYSTOPPING_VEHICLES_NUMBER = 0x89;
const uint8_t VAR_EMERGENCY_DECEL = 0x7b;
const uint8_t VAR_EMISSIONCLASS = 0x4a;
const uint8_t VAR_FILL = 0x55;
const uint8_t VAR_FOES = 0x37;
const uint8_t VAR_FUELCONSUMPTION = 0x65;
const uint8_t VAR_HAS_VIEW = 0xa7;
const uint8_t VAR_HCEMISSION = 0x62;
const uint8_t VAR_HEIGHT = 0xbc;
const uint8_t VAR_IMPERFECTION = 0x5d;
const uint8_t VAR_LANECHANGE_MODE = 0xb6;
const uint8_t VAR_LANEPOSITION = 0x56;
const uint8_t VAR_LANEPOSITION_LAT = 0xb8;
const uint8_t VAR_LANE_ID = 0x51;
const uint8_t VAR_LANE_INDEX = 0x52;
const uint8_t VAR_LASTACTIONTIME = 0x7f;
const uint8_t VAR_LATALIGNMENT = 0xb9;
const uint8_t VAR_LEADER = 0x68;
const uint8_t VAR_LENGTH = 0x44;
const uint8_t VAR_LINE = 0xbd;
const uint8_t VAR_LOADED_VEHICLES_IDS = 0x72;
const uint8_t VAR_LOADED_VEHICLES_NUMBER = 0x71;
const uint8_t VAR_MAXSPEED = 0x41;
const uint8_t VAR_MAXSPEED_LAT = 0xba;
const uint8_t VAR_MINGAP = 0x4c;
const uint8_t VAR_MINGAP_LAT = 0xbb;
const uint8_t VAR_MIN_EXPECTED_VEHICLES = 0x7d;
const uint8_t VAR_MOVE_TO = 0x5c;
const uint8_t VAR_MOVE_TO_VTD = 0xb4; // Veins specific (called MOVE_TO_XY in TraCI)
const uint8_t VAR_NAME = 0x1b;
const uint8_t VAR_NET_BOUNDING_BOX = 0x7c;
const uint8_t VAR_NEXT_EDGE = 0xc1;
const uint8_t VAR_NEXT_STOPS = 0x73;
const uint8_t VAR_NEXT_TLS = 0x70;
const uint8_t VAR_NOISEEMISSION = 0x66;
const uint8_t VAR_NOXEMISSION = 0x64;
const uint8_t VAR_PARAMETER = 0x7e;
const uint8_t VAR_PARKING_ENDING_VEHICLES_IDS = 0x6f;
const uint8_t VAR_PARKING_ENDING_VEHICLES_NUMBER = 0x6e;
const uint8_t VAR_PARKING_STARTING_VEHICLES_IDS = 0x6d;
const uint8_t VAR_PARKING_STARTING_VEHICLES_NUMBER = 0x6c;
const uint8_t VAR_PERSON_NUMBER = 0x67;
const uint8_t VAR_PMXEMISSION = 0x63;
const uint8_t VAR_POSITION = 0x42;
const uint8_t VAR_POSITION3D = 0x39;
const uint8_t VAR_ROAD_ID = 0x50;
const uint8_t VAR_ROUTE = 0x57;
const uint8_t VAR_ROUTE_ID = 0x53;
const uint8_t VAR_ROUTE_INDEX = 0x69;
const uint8_t VAR_ROUTE_VALID = 0x92;
const uint8_t VAR_ROUTING_MODE = 0x89;
const uint8_t VAR_SCREENSHOT = 0xa5;
const uint8_t VAR_SHAPE = 0x4e;
const uint8_t VAR_SHAPECLASS = 0x4b;
const uint8_t VAR_SIGNALS = 0x5b;
const uint8_t VAR_SLOPE = 0x36;
const uint8_t VAR_SPEED = 0x40;
const uint8_t VAR_SPEEDSETMODE = 0xb3;
const uint8_t VAR_SPEED_DEVIATION = 0x5f;
const uint8_t VAR_SPEED_FACTOR = 0x5e;
const uint8_t VAR_SPEED_WITHOUT_TRACI = 0xb1;
const uint8_t VAR_STAGE = 0xc0;
const uint8_t VAR_STAGES_REMAINING = 0xc2;
const uint8_t VAR_STOPSTATE = 0xb5;
const uint8_t VAR_STOP_ENDING_VEHICLES_IDS = 0x6b;
const uint8_t VAR_STOP_ENDING_VEHICLES_NUMBER = 0x6a;
const uint8_t VAR_STOP_STARTING_VEHICLES_IDS = 0x69;
const uint8_t VAR_STOP_STARTING_VEHICLES_NUMBER = 0x68;
const uint8_t VAR_TAU = 0x48;
const uint8_t VAR_TELEPORT_ENDING_VEHICLES_IDS = 0x78;
const uint8_t VAR_TELEPORT_ENDING_VEHICLES_NUMBER = 0x77;
const uint8_t VAR_TELEPORT_STARTING_VEHICLES_IDS = 0x76;
const uint8_t VAR_TELEPORT_STARTING_VEHICLES_NUMBER = 0x75;
const uint8_t VAR_TIME = 0x66;
const uint8_t VAR_TIME_STEP = 0x70;
const uint8_t VAR_TRACK_VEHICLE = 0xa6;
const uint8_t VAR_TYPE = 0x4f;
const uint8_t VAR_UPDATE_BESTLANES = 0x6a;
const uint8_t VAR_VEHICLE = 0xc3;
const uint8_t VAR_VEHICLECLASS = 0x49;
const uint8_t VAR_VIA = 0xbe;
const uint8_t VAR_VIEW_BOUNDARY = 0xa3;
const uint8_t VAR_VIEW_OFFSET = 0xa1;
const uint8_t VAR_VIEW_SCHEMA = 0xa2;
const uint8_t VAR_VIEW_ZOOM = 0xa0;
const uint8_t VAR_WAITING_TIME = 0x7a;
const uint8_t VAR_WAITING_TIME_ACCUMULATED = 0x87; // Veins specific (called VAR_ACCUMULATED_WAITING_TIME in TraCI)
const uint8_t VAR_WIDTH = 0x4d;

} // namespace TraCIConstants
} // namespace veins
