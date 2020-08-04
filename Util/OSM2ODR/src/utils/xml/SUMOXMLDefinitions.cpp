/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOXMLDefinitions.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Clemens Honomichl
/// @author  Piotr Woznica
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    Sept 2002
///
// Definitions of elements and attributes known by SUMO
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utils/common/StringBijection.h>
#include <utils/common/StringTokenizer.h>

#include "SUMOXMLDefinitions.h"

// ===========================================================================
// definitions
// ===========================================================================

StringBijection<int>::Entry SUMOXMLDefinitions::tags[] = {
    // Simulation elements
    { "net",                            SUMO_TAG_NET },
    { "edge",                           SUMO_TAG_EDGE },
    { "lane",                           SUMO_TAG_LANE },
    { "neigh",                          SUMO_TAG_NEIGH },
    { "poi",                            SUMO_TAG_POI },
    { "poiLane",                        SUMO_TAG_POILANE },
    { "poly",                           SUMO_TAG_POLY },
    { "junction",                       SUMO_TAG_JUNCTION },
    { "restriction",                    SUMO_TAG_RESTRICTION },
    { "e1Detector",                     SUMO_TAG_E1DETECTOR },
    { "inductionLoop",                  SUMO_TAG_INDUCTION_LOOP },
    { "e2Detector",                     SUMO_TAG_E2DETECTOR },
    { "e2MultilaneDetector",            SUMO_TAG_E2DETECTOR_MULTILANE },
    { "laneAreaDetector",               SUMO_TAG_LANE_AREA_DETECTOR },
    { "e3Detector",                     SUMO_TAG_E3DETECTOR },
    { "entryExitDetector",              SUMO_TAG_ENTRY_EXIT_DETECTOR },
    { "edgeData",                       SUMO_TAG_MEANDATA_EDGE },
    { "laneData",                       SUMO_TAG_MEANDATA_LANE },
    { "detEntry",                       SUMO_TAG_DET_ENTRY },
    { "detExit",                        SUMO_TAG_DET_EXIT },
    { "edgeFollowDetector",             SUMO_TAG_EDGEFOLLOWDETECTOR },
    { "instantInductionLoop",           SUMO_TAG_INSTANT_INDUCTION_LOOP },
    { "variableSpeedSign",              SUMO_TAG_VSS },
    { "calibrator",                     SUMO_TAG_CALIBRATOR },
    { "laneCalibrator",                 SUMO_TAG_LANECALIBRATOR },
    { "rerouter",                       SUMO_TAG_REROUTER },
    { "busStop",                        SUMO_TAG_BUS_STOP },
    { "trainStop",                      SUMO_TAG_TRAIN_STOP },
    { "ptLine",                         SUMO_TAG_PT_LINE },
    { "access",                         SUMO_TAG_ACCESS },
    { "containerStop",                  SUMO_TAG_CONTAINER_STOP },
    { "parkingArea",                    SUMO_TAG_PARKING_AREA },
    { "space",                          SUMO_TAG_PARKING_SPACE },
    { "chargingStation",                SUMO_TAG_CHARGING_STATION },
    { "overheadWireSegment",            SUMO_TAG_OVERHEAD_WIRE_SEGMENT },
    { "overheadWire",                   SUMO_TAG_OVERHEAD_WIRE_SECTION },
    { "tractionSubstation",             SUMO_TAG_TRACTION_SUBSTATION },
    { "overheadWireClamp",              SUMO_TAG_OVERHEAD_WIRE_CLAMP },
    { "vTypeProbe",                     SUMO_TAG_VTYPEPROBE },
    { "routeProbe",                     SUMO_TAG_ROUTEPROBE },
    { "routes",                         SUMO_TAG_ROUTES },
    { "vehicle",                        SUMO_TAG_VEHICLE },
    { "vType",                          SUMO_TAG_VTYPE },
    { "pType",                          SUMO_TAG_PTYPE },
    { "route",                          SUMO_TAG_ROUTE },
    { "request",                        SUMO_TAG_REQUEST },
    { "source",                         SUMO_TAG_SOURCE },
    { "taz",                            SUMO_TAG_TAZ },
    { "tazSource",                      SUMO_TAG_TAZSOURCE },
    { "tazSink",                        SUMO_TAG_TAZSINK },
    { "trafficLight",                   SUMO_TAG_TRAFFIC_LIGHT },
    { "tlLogic",                        SUMO_TAG_TLLOGIC },
    { "phase",                          SUMO_TAG_PHASE },
    { "trip",                           SUMO_TAG_TRIP },
    { "tripTAZ",                        SUMO_TAG_TRIP_TAZ },
    { "flow",                           SUMO_TAG_FLOW },
    { "flowCalibrator",                 SUMO_TAG_FLOW_CALIBRATOR },
    { "flowState",                      SUMO_TAG_FLOWSTATE },
    { "step",                           SUMO_TAG_STEP },
    { "interval",                       SUMO_TAG_INTERVAL },
    { "edgeRelation",                   SUMO_TAG_EDGEREL },
    { "tazRelation",                    SUMO_TAG_TAZREL },
    { "timedEvent",                     SUMO_TAG_TIMEDEVENT },
    { "fromEdge",                       SUMO_TAG_FROMEDGE },
    { "toEdge",                         SUMO_TAG_TOEDGE },
    { "sink",                           SUMO_TAG_SINK },
    { "param",                          SUMO_TAG_PARAM },
    { "WAUT",                           SUMO_TAG_WAUT },
    { "wautSwitch",                     SUMO_TAG_WAUT_SWITCH },
    { "wautJunction",                   SUMO_TAG_WAUT_JUNCTION },
    { "segment",                        SUMO_TAG_SEGMENT },
    { "delete",                         SUMO_TAG_DEL },
    { "stop",                           SUMO_TAG_STOP },
    { "stopBusStop",                    SUMO_TAG_STOP_BUSSTOP },
    { "stopContainerStop",              SUMO_TAG_STOP_CONTAINERSTOP },
    { "stopChargingStation",            SUMO_TAG_STOP_CHARGINGSTATION },
    { "stopParkingArea",                SUMO_TAG_STOP_PARKINGAREA },
    { "stopLane",                       SUMO_TAG_STOP_LANE },
    { "destProbReroute",                SUMO_TAG_DEST_PROB_REROUTE },
    { "closingReroute",                 SUMO_TAG_CLOSING_REROUTE },
    { "closingLaneReroute",             SUMO_TAG_CLOSING_LANE_REROUTE },
    { "routeProbReroute",               SUMO_TAG_ROUTE_PROB_REROUTE },
    { "parkingAreaReroute",             SUMO_TAG_PARKING_ZONE_REROUTE },
    { "polygonType",                    SUMO_TAG_POLYTYPE },
    { "connection",                     SUMO_TAG_CONNECTION },
    { "prohibition",                    SUMO_TAG_PROHIBITION },
    { "split",                          SUMO_TAG_SPLIT },
    { "node",                           SUMO_TAG_NODE },
    { "type",                           SUMO_TAG_TYPE },
    { "detectorDefinition",             SUMO_TAG_DETECTOR_DEFINITION },
    { "routeDistribution",              SUMO_TAG_ROUTE_DISTRIBUTION },
    { "vTypeDistribution",              SUMO_TAG_VTYPE_DISTRIBUTION },
    { "vaporizer",                      SUMO_TAG_VAPORIZER },
    { "roundabout",                     SUMO_TAG_ROUNDABOUT },
    { "join",                           SUMO_TAG_JOIN },
    { "joinExclude",                    SUMO_TAG_JOINEXCLUDE },
    { "crossing",                       SUMO_TAG_CROSSING },
    { "walkingArea",                    SUMO_TAG_WALKINGAREA },
    { "stopOffset",                     SUMO_TAG_STOPOFFSET },
    // OSM
    { "way",                            SUMO_TAG_WAY },
    { "nd",                             SUMO_TAG_ND },
    { "tag",                            SUMO_TAG_TAG },
    { "relation",                       SUMO_TAG_RELATION },
    { "member",                         SUMO_TAG_MEMBER },
    // View
    { "viewsettings",                   SUMO_TAG_VIEWSETTINGS },
    { "decal",                          SUMO_TAG_VIEWSETTINGS_DECAL },
    { "light",                          SUMO_TAG_VIEWSETTINGS_LIGHT },
    { "scheme",                         SUMO_TAG_VIEWSETTINGS_SCHEME },
    { "opengl",                         SUMO_TAG_VIEWSETTINGS_OPENGL },
    { "background",                     SUMO_TAG_VIEWSETTINGS_BACKGROUND },
    { "edges",                          SUMO_TAG_VIEWSETTINGS_EDGES },
    { "vehicles",                       SUMO_TAG_VIEWSETTINGS_VEHICLES },
    { "persons",                        SUMO_TAG_VIEWSETTINGS_PERSONS },
    { "containers",                     SUMO_TAG_VIEWSETTINGS_CONTAINERS },
    { "junctions",                      SUMO_TAG_VIEWSETTINGS_JUNCTIONS },
    { "additionals",                    SUMO_TAG_VIEWSETTINGS_ADDITIONALS },
    { "pois",                           SUMO_TAG_VIEWSETTINGS_POIS },
    { "polys",                          SUMO_TAG_VIEWSETTINGS_POLYS },
    { "legend",                         SUMO_TAG_VIEWSETTINGS_LEGEND },
    { "event",                          SUMO_TAG_VIEWSETTINGS_EVENT },
    { "jamTime",                        SUMO_TAG_VIEWSETTINGS_EVENT_JAM_TIME },
    { "include",                        SUMO_TAG_INCLUDE },
    { "delay",                          SUMO_TAG_DELAY },
    { "viewport",                       SUMO_TAG_VIEWPORT },
    { "snapshot",                       SUMO_TAG_SNAPSHOT },
    { "breakpoint",                     SUMO_TAG_BREAKPOINT },
    { "location",                       SUMO_TAG_LOCATION },
    { "colorScheme",                    SUMO_TAG_COLORSCHEME },
    { "scalingScheme",                  SUMO_TAG_SCALINGSCHEME },
    { "entry",                          SUMO_TAG_ENTRY },
    { "rngState",                       SUMO_TAG_RNGSTATE },
    { "rngLane",                        SUMO_TAG_RNGLANE },
    { "vehicleTransfer",                SUMO_TAG_VEHICLETRANSFER },
    { "device",                         SUMO_TAG_DEVICE },
    // Cars
    { "carFollowing-IDM",               SUMO_TAG_CF_IDM },
    { "carFollowing-IDMM",              SUMO_TAG_CF_IDMM },
    { "carFollowing-Krauss",            SUMO_TAG_CF_KRAUSS },
    { "carFollowing-KraussPS",          SUMO_TAG_CF_KRAUSS_PLUS_SLOPE },
    { "carFollowing-KraussOrig1",       SUMO_TAG_CF_KRAUSS_ORIG1 },
    { "carFollowing-KraussX",           SUMO_TAG_CF_KRAUSSX },
    { "carFollowing-SmartSK",           SUMO_TAG_CF_SMART_SK },
    { "carFollowing-Daniel1",           SUMO_TAG_CF_DANIEL1 },
    { "carFollowing-PWagner2009",       SUMO_TAG_CF_PWAGNER2009 },
    { "carFollowing-BKerner",           SUMO_TAG_CF_BKERNER },
    { "carFollowing-Wiedemann",         SUMO_TAG_CF_WIEDEMANN },
    { "carFollowing-W99",               SUMO_TAG_CF_W99 },
    { "carFollowing-Rail",              SUMO_TAG_CF_RAIL },
    { "carFollowing-ACC",               SUMO_TAG_CF_ACC },
    { "carFollowing-CACC",              SUMO_TAG_CF_CACC },
    { "carFollowing-CC",                SUMO_TAG_CF_CC },
    // Person
    { "person",                         SUMO_TAG_PERSON },
    { "personTrip",                     SUMO_TAG_PERSONTRIP },
    { "ride",                           SUMO_TAG_RIDE },
    { "walk",                           SUMO_TAG_WALK },
    { "personFlow",                     SUMO_TAG_PERSONFLOW },
    // Data (Netedit)
    { "dataSet",                        SUMO_TAG_DATASET },
    { "dataInterval",                   SUMO_TAG_DATAINTERVAL },
    // Transport
    { "container",                      SUMO_TAG_CONTAINER },
    { "transport",                      SUMO_TAG_TRANSPORT },
    { "tranship",                       SUMO_TAG_TRANSHIP },
    //Trajectories
    { "trajectories",                   SUMO_TAG_TRAJECTORIES },
    { "timestep",                       SUMO_TAG_TIMESTEP },
    { "timeSlice",                      SUMO_TAG_TIMESLICE },
    { "actorConfig",                    SUMO_TAG_ACTORCONFIG },
    { "motionState",                    SUMO_TAG_MOTIONSTATE },
    { "odPair",                         SUMO_TAG_OD_PAIR },
    // ActivityGen statistics file
    { "general",                        AGEN_TAG_GENERAL },
    { "street",                         AGEN_TAG_STREET },
    { "workHours",                      AGEN_TAG_WORKHOURS },
    { "opening",                        AGEN_TAG_OPENING },
    { "closing",                        AGEN_TAG_CLOSING },
    { "schools",                        AGEN_TAG_SCHOOLS },
    { "school",                         AGEN_TAG_SCHOOL },
    { "busStation",                     AGEN_TAG_BUSSTATION },
    { "busLine",                        AGEN_TAG_BUSLINE },
    { "stations",                       AGEN_TAG_STATIONS },
    { "revStations",                    AGEN_TAG_REV_STATIONS },
    { "station",                        AGEN_TAG_STATION },
    { "frequency",                      AGEN_TAG_FREQUENCY },
    { "population",                     AGEN_TAG_POPULATION },
    { "bracket",                        AGEN_TAG_BRACKET },
    { "cityGates",                      AGEN_TAG_CITYGATES },
    { "entrance",                       AGEN_TAG_ENTRANCE },
    { "parameters",                     AGEN_TAG_PARAM },
    // GNE Person trips
    { "personTrip: edge->edge",         GNE_TAG_PERSONTRIP_EDGE_EDGE },
    { "personTrip: edge->busStop",      GNE_TAG_PERSONTRIP_EDGE_BUSSTOP },
    { "personTrip: busStop->edge",      GNE_TAG_PERSONTRIP_BUSSTOP_EDGE },
    { "personTrip: busStop->busStop",   GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP },
    // GNE Walks
    { "walk: edge->edge",               GNE_TAG_WALK_EDGE_EDGE },
    { "walk: edge->busStop",            GNE_TAG_WALK_EDGE_BUSSTOP },
    { "walk: busStop->edge",            GNE_TAG_WALK_BUSSTOP_EDGE },
    { "walk: busStop->busStop",         GNE_TAG_WALK_BUSSTOP_BUSSTOP },
    { "walk: edges",                    GNE_TAG_WALK_EDGES },
    { "walk: route",                    GNE_TAG_WALK_ROUTE },
    // GNE Rides
    { "ride: edge->edge",               GNE_TAG_RIDE_EDGE_EDGE },
    { "ride: edge->busStop",            GNE_TAG_RIDE_EDGE_BUSSTOP },
    { "ride: busStop->edge",            GNE_TAG_RIDE_BUSSTOP_EDGE },
    { "ride: busStop->busStop",         GNE_TAG_RIDE_BUSSTOP_BUSSTOP },
    // GNE person Stops
    { "personStop: busStop",            GNE_TAG_PERSONSTOP_BUSSTOP },
    { "personStop: edge",               GNE_TAG_PERSONSTOP_EDGE },
    // special tags to represent different input styles with the same tag internally
    { "vehicleWithRoute",               GNE_TAG_VEHICLEWITHROUTE },
    { "routeEmbedded",                  GNE_TAG_ROUTE_EMBEDDED },
    { "flowRoute",                      GNE_TAG_FLOW_ROUTE },
    { "flowWithRoute",                  GNE_TAG_FLOW_WITHROUTE },
    { "Person (embedded)",              GNE_TAG_PERSON_EMBEDDED },
    // Other
    { "",                               SUMO_TAG_NOTHING }  // -> must be the last one
};


StringBijection<int>::Entry SUMOXMLDefinitions::attrs[] = {
    // meandata
    { "sampledSeconds",         SUMO_ATTR_SAMPLEDSECONDS },
    { "density",                SUMO_ATTR_DENSITY },
    { "laneDensity",            SUMO_ATTR_LANEDENSITY },
    { "occupancy",              SUMO_ATTR_OCCUPANCY },
    { "waitingTime",            SUMO_ATTR_WAITINGTIME },
    { "speed",                  SUMO_ATTR_SPEED },
    { "departed",               SUMO_ATTR_DEPARTED },
    { "arrived",                SUMO_ATTR_ARRIVED },
    { "entered",                SUMO_ATTR_ENTERED },
    { "left",                   SUMO_ATTR_LEFT },
    { "vaporized",              SUMO_ATTR_VAPORIZED },
    { "traveltime",             SUMO_ATTR_TRAVELTIME },
    { "laneChangedFrom",        SUMO_ATTR_LANECHANGEDFROM },
    { "laneChangedTo",          SUMO_ATTR_LANECHANGEDTO },
    { "overlapTraveltime",      SUMO_ATTR_OVERLAPTRAVELTIME },
    { "CO_abs",                 SUMO_ATTR_CO_ABS },
    { "CO2_abs",                SUMO_ATTR_CO2_ABS },
    { "HC_abs",                 SUMO_ATTR_HC_ABS },
    { "PMx_abs",                SUMO_ATTR_PMX_ABS },
    { "NOx_abs",                SUMO_ATTR_NOX_ABS },
    { "fuel_abs",               SUMO_ATTR_FUEL_ABS },
    { "electricity_abs",        SUMO_ATTR_ELECTRICITY_ABS },
    { "CO_normed",              SUMO_ATTR_CO_NORMED },
    { "CO2_normed",             SUMO_ATTR_CO2_NORMED },
    { "HC_normed",              SUMO_ATTR_HC_NORMED },
    { "PMx_normed",             SUMO_ATTR_PMX_NORMED },
    { "NOx_normed",             SUMO_ATTR_NOX_NORMED },
    { "fuel_normed",            SUMO_ATTR_FUEL_NORMED },
    { "electricity_normed",     SUMO_ATTR_ELECTRICITY_NORMED },
    { "CO_perVeh",              SUMO_ATTR_CO_PERVEH },
    { "CO2_perVeh",             SUMO_ATTR_CO2_PERVEH },
    { "HC_perVeh",              SUMO_ATTR_HC_PERVEH },
    { "PMx_perVeh",             SUMO_ATTR_PMX_PERVEH },
    { "NOx_perVeh",             SUMO_ATTR_NOX_PERVEH },
    { "fuel_perVeh",            SUMO_ATTR_FUEL_PERVEH },
    { "electricity_perVeh",     SUMO_ATTR_ELECTRICITY_PERVEH },
    { "noise",                  SUMO_ATTR_NOISE },
    { "amount",                 SUMO_ATTR_AMOUNT },
    { "averageSpeed",           SUMO_ATTR_AVERAGESPEED },

    // Edge
    { "id",                     SUMO_ATTR_ID },
    { "refId",                  SUMO_ATTR_REFID },
    { "name",                   SUMO_ATTR_NAME },
    { "type",                   SUMO_ATTR_TYPE },
    { "version",                SUMO_ATTR_VERSION },
    { "priority",               SUMO_ATTR_PRIORITY },
    { "numLanes",               SUMO_ATTR_NUMLANES },
    { "oneway",                 SUMO_ATTR_ONEWAY },
    { "width",                  SUMO_ATTR_WIDTH },
    { "widthResolution",        SUMO_ATTR_WIDTHRESOLUTION },
    { "maxWidth",               SUMO_ATTR_MAXWIDTH },
    { "minWidth",               SUMO_ATTR_MINWIDTH },
    { "sidewalkWidth",          SUMO_ATTR_SIDEWALKWIDTH },
    { "bikeLaneWidth",          SUMO_ATTR_BIKELANEWIDTH },
    { "remove",                 SUMO_ATTR_REMOVE },
    { "length",                 SUMO_ATTR_LENGTH },
    { "bidi",                   SUMO_ATTR_BIDI },
    { "distance",               SUMO_ATTR_DISTANCE },
    // Split
    { "idBefore",               SUMO_ATTR_ID_BEFORE },
    { "idAfter",                SUMO_ATTR_ID_AFTER },
    // Positions
    { "x",                      SUMO_ATTR_X },
    { "y",                      SUMO_ATTR_Y },
    { "z",                      SUMO_ATTR_Z },
    { "centerX",                SUMO_ATTR_CENTER_X },
    { "centerY",                SUMO_ATTR_CENTER_Y },
    { "centerZ",                SUMO_ATTR_CENTER_Z },

    { "key",                    SUMO_ATTR_KEY },
    { "requestSize",            SUMO_ATTR_REQUESTSIZE },
    { "request",                SUMO_ATTR_REQUEST },
    { "response",               SUMO_ATTR_RESPONSE },
    { "programID",              SUMO_ATTR_PROGRAMID },
    { "offset",                 SUMO_ATTR_OFFSET },
    { "endOffset",              SUMO_ATTR_ENDOFFSET },
    { "incLanes",               SUMO_ATTR_INCLANES },
    { "intLanes",               SUMO_ATTR_INTLANES },

    { "weight",                 SUMO_ATTR_WEIGHT },
    { "node",                   SUMO_ATTR_NODE },
    { "edge",                   SUMO_ATTR_EDGE },
    { "edges",                  SUMO_ATTR_EDGES },
    // Vehicle
    { "depart",                 SUMO_ATTR_DEPART },
    { "departLane",             SUMO_ATTR_DEPARTLANE },
    { "departPos",              SUMO_ATTR_DEPARTPOS },
    { "departPosLat",           SUMO_ATTR_DEPARTPOS_LAT },
    { "departSpeed",            SUMO_ATTR_DEPARTSPEED },
    { "arrivalLane",            SUMO_ATTR_ARRIVALLANE },
    { "arrivalPos",             SUMO_ATTR_ARRIVALPOS },
    { "arrivalPosLat",          SUMO_ATTR_ARRIVALPOS_LAT },
    { "arrivalSpeed",           SUMO_ATTR_ARRIVALSPEED },
    { "route",                  SUMO_ATTR_ROUTE },
    { "maxSpeed",               SUMO_ATTR_MAXSPEED },
    { "maxSpeedLat",            SUMO_ATTR_MAXSPEED_LAT },
    { "latAlignment",           SUMO_ATTR_LATALIGNMENT },
    { "minGapLat",              SUMO_ATTR_MINGAP_LAT },
    { "accel",                  SUMO_ATTR_ACCEL },
    { "decel",                  SUMO_ATTR_DECEL },
    { "emergencyDecel",         SUMO_ATTR_EMERGENCYDECEL },
    { "apparentDecel",          SUMO_ATTR_APPARENTDECEL },
    { "actionStepLength",       SUMO_ATTR_ACTIONSTEPLENGTH },
    { "vClass",                 SUMO_ATTR_VCLASS },
    { "vClasses",               SUMO_ATTR_VCLASSES },
    { "exceptions",             SUMO_ATTR_EXCEPTIONS },
    { "repno",                  SUMO_ATTR_REPNUMBER },
    { "speedFactor",            SUMO_ATTR_SPEEDFACTOR },
    { "speedDev",               SUMO_ATTR_SPEEDDEV },
    { "laneChangeModel",        SUMO_ATTR_LANE_CHANGE_MODEL },
    { "carFollowModel",         SUMO_ATTR_CAR_FOLLOW_MODEL },
    { "minGap",                 SUMO_ATTR_MINGAP },
    { "collisionMinGapFactor",  SUMO_ATTR_COLLISION_MINGAP_FACTOR },
    { "boardingDuration",       SUMO_ATTR_BOARDING_DURATION },
    { "loadingDuration",        SUMO_ATTR_LOADING_DURATION },
    { "maneuverAngleTimes",     SUMO_ATTR_MANEUVER_ANGLE_TIMES },
    // MSDevice_ElecHybrid
    { "overheadWireChargingPower",      SUMO_ATTR_OVERHEADWIRECHARGINGPOWER },
    // OverheadWire
    { "overheadWireSegment",    SUMO_ATTR_OVERHEAD_WIRE_SEGMENT },
    { "segments",               SUMO_ATTR_OVERHEAD_WIRE_SECTION },
    { "voltage",                SUMO_ATTR_VOLTAGE },
    { "voltageSource",          SUMO_ATTR_VOLTAGESOURCE },
    { "currentLimit",           SUMO_ATTR_CURRENTLIMIT },
    { "substationId",           SUMO_ATTR_SUBSTATIONID },
    { "wireResistivity",        SUMO_ATTR_OVERHEAD_WIRE_RESISTIVITY },
    { "forbiddenInnerLanes",    SUMO_ATTR_OVERHEAD_WIRE_FORBIDDEN },
    { "clamps",                  SUMO_ATTR_OVERHEAD_WIRE_CLAMPS },
    { "idSegmentStartClamp",    SUMO_ATTR_OVERHEAD_WIRE_CLAMP_START },
    { "idSegmentEndClamp",      SUMO_ATTR_OVERHEAD_WIRE_CLAMP_END },
    // Charging Station
    { "power",                  SUMO_ATTR_CHARGINGPOWER },
    { "efficiency",             SUMO_ATTR_EFFICIENCY },
    { "chargeInTransit",        SUMO_ATTR_CHARGEINTRANSIT },
    { "chargeDelay",            SUMO_ATTR_CHARGEDELAY},
    // MSDevice_Battery
    { "actualBatteryCapacity",          SUMO_ATTR_ACTUALBATTERYCAPACITY },
    { "maximumBatteryCapacity",         SUMO_ATTR_MAXIMUMBATTERYCAPACITY },
    { "maximumPower",                   SUMO_ATTR_MAXIMUMPOWER },
    { "vehicleMass",                    SUMO_ATTR_VEHICLEMASS },
    { "frontSurfaceArea",               SUMO_ATTR_FRONTSURFACEAREA },
    { "airDragCoefficient",             SUMO_ATTR_AIRDRAGCOEFFICIENT },
    { "internalMomentOfInertia",        SUMO_ATTR_INTERNALMOMENTOFINERTIA },
    { "radialDragCoefficient",          SUMO_ATTR_RADIALDRAGCOEFFICIENT },
    { "rollDragCoefficient",            SUMO_ATTR_ROLLDRAGCOEFFICIENT },
    { "constantPowerIntake",            SUMO_ATTR_CONSTANTPOWERINTAKE },
    { "propulsionEfficiency",           SUMO_ATTR_PROPULSIONEFFICIENCY },
    { "recuperationEfficiency",         SUMO_ATTR_RECUPERATIONEFFICIENCY },
    { "recuperationEfficiencyByDecel",  SUMO_ATTR_RECUPERATIONEFFICIENCY_BY_DECELERATION },
    { "stoppingTreshold",               SUMO_ATTR_STOPPINGTRESHOLD },
    // MSElecHybridExport
    { "overheadWireId",         SUMO_ATTR_OVERHEADWIREID },
    { "tractionSubstationId",   SUMO_ATTR_TRACTIONSUBSTATIONID },
    { "current",                SUMO_ATTR_CURRENTFROMOVERHEADWIRE },
    { "circuitVoltage",         SUMO_ATTR_VOLTAGEOFOVERHEADWIRE },
    { "alphaCircuitSolver",     SUMO_ATTR_ALPHACIRCUITSOLVER },
    // MSBatteryExport
    { "energyConsumed",         SUMO_ATTR_ENERGYCONSUMED },
    { "chargingStationId",      SUMO_ATTR_CHARGINGSTATIONID },
    { "energyCharged",          SUMO_ATTR_ENERGYCHARGED },
    { "energyChargedInTransit", SUMO_ATTR_ENERGYCHARGEDINTRANSIT },
    { "energyChargedStopped",   SUMO_ATTR_ENERGYCHARGEDSTOPPED },
    { "posOnLane",              SUMO_ATTR_POSONLANE },
    { "timeStopped",            SUMO_ATTR_TIMESTOPPED },
    // ChargingStation output
    { "chargingStatus",                 SUMO_ATTR_CHARGING_STATUS },
    { "totalEnergyCharged",             SUMO_ATTR_TOTALENERGYCHARGED },
    { "chargingSteps",                  SUMO_ATTR_CHARGINGSTEPS },
    { "totalEnergyChargedIntoVehicle",  SUMO_ATTR_TOTALENERGYCHARGED_VEHICLE },
    { "chargingBegin",                  SUMO_ATTR_CHARGINGBEGIN },
    { "chargingEnd",                    SUMO_ATTR_CHARGINGEND },
    { "partialCharge",                  SUMO_ATTR_PARTIALCHARGE },

    { "sigma",                  SUMO_ATTR_SIGMA },
    { "tau",                    SUMO_ATTR_TAU },
    { "tmp1",                   SUMO_ATTR_TMP1 },
    { "tmp2",                   SUMO_ATTR_TMP2 },
    { "tmp3",                   SUMO_ATTR_TMP3 },
    { "tmp4",                   SUMO_ATTR_TMP4 },
    { "tmp5",                   SUMO_ATTR_TMP5 },

    { "speedControlGain",               SUMO_ATTR_SC_GAIN },
    { "gapClosingControlGainSpeed",     SUMO_ATTR_GCC_GAIN_SPEED },
    { "gapClosingControlGainSpace",     SUMO_ATTR_GCC_GAIN_SPACE },
    { "gapControlGainSpeed",            SUMO_ATTR_GC_GAIN_SPEED },
    { "gapControlGainSpace",            SUMO_ATTR_GC_GAIN_SPACE },
    { "collisionAvoidanceGainSpeed",    SUMO_ATTR_CA_GAIN_SPEED },
    { "collisionAvoidanceGainSpace",    SUMO_ATTR_CA_GAIN_SPACE },

    { "speedControlGainCACC",           SUMO_ATTR_SC_GAIN_CACC },
    { "gapClosingControlGainGap",       SUMO_ATTR_GCC_GAIN_GAP_CACC },
    { "gapClosingControlGainGapDot",    SUMO_ATTR_GCC_GAIN_GAP_DOT_CACC },
    { "gapControlGainGap",              SUMO_ATTR_GC_GAIN_GAP_CACC },
    { "gapControlGainGapDot",           SUMO_ATTR_GC_GAIN_GAP_DOT_CACC },
    { "tauCACCToACC",                   SUMO_ATTR_HEADWAY_TIME_CACC_TO_ACC },
    { "collisionAvoidanceGainGap",      SUMO_ATTR_CA_GAIN_GAP_CACC },
    { "collisionAvoidanceGainGapDot",   SUMO_ATTR_CA_GAIN_GAP_DOT_CACC },

    { "trainType",              SUMO_ATTR_TRAIN_TYPE },

    { "lcStrategic",                SUMO_ATTR_LCA_STRATEGIC_PARAM },
    { "lcCooperative",              SUMO_ATTR_LCA_COOPERATIVE_PARAM },
    { "lcSpeedGain",                SUMO_ATTR_LCA_SPEEDGAIN_PARAM },
    { "lcKeepRight",                SUMO_ATTR_LCA_KEEPRIGHT_PARAM },
    { "lcSublane",                  SUMO_ATTR_LCA_SUBLANE_PARAM },
    { "lcOpposite",                 SUMO_ATTR_LCA_OPPOSITE_PARAM },
    { "lcPushy",                    SUMO_ATTR_LCA_PUSHY },
    { "lcPushyGap",                 SUMO_ATTR_LCA_PUSHYGAP },
    { "lcAssertive",                SUMO_ATTR_LCA_ASSERTIVE },
    { "lcImpatience",               SUMO_ATTR_LCA_IMPATIENCE },
    { "lcTimeToImpatience",         SUMO_ATTR_LCA_TIME_TO_IMPATIENCE },
    { "lcAccelLat",                 SUMO_ATTR_LCA_ACCEL_LAT },
    { "lcTurnAlignmentDistance",    SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE },
    { "lcOvertakeRight",            SUMO_ATTR_LCA_OVERTAKE_RIGHT },
    { "lcLookaheadLeft",            SUMO_ATTR_LCA_LOOKAHEADLEFT },
    { "lcSpeedGainRight",           SUMO_ATTR_LCA_SPEEDGAINRIGHT },
    { "lcSpeedGainLookahead",       SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD },
    { "lcCooperativeRoundabout",    SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT },
    { "lcCooperativeSpeed",         SUMO_ATTR_LCA_COOPERATIVE_SPEED },
    { "lcMaxSpeedLatStanding",      SUMO_ATTR_LCA_MAXSPEEDLATSTANDING },
    { "lcMaxSpeedLatFactor",        SUMO_ATTR_LCA_MAXSPEEDLATFACTOR },
    { "lcLaneDiscipline",           SUMO_ATTR_LCA_LANE_DISCIPLINE },
    { "lcSigma",                    SUMO_ATTR_LCA_SIGMA },
    { "lcExperimental1",            SUMO_ATTR_LCA_EXPERIMENTAL1 },

    { "jmCrossingGap",          SUMO_ATTR_JM_CROSSING_GAP },
    { "jmDriveAfterYellowTime", SUMO_ATTR_JM_DRIVE_AFTER_YELLOW_TIME },
    { "jmDriveAfterRedTime",    SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME },
    { "jmDriveRedSpeed",        SUMO_ATTR_JM_DRIVE_RED_SPEED },
    { "jmIgnoreKeepClearTime",  SUMO_ATTR_JM_IGNORE_KEEPCLEAR_TIME },
    { "jmIgnoreFoeSpeed",       SUMO_ATTR_JM_IGNORE_FOE_SPEED },
    { "jmIgnoreFoeProb",        SUMO_ATTR_JM_IGNORE_FOE_PROB },
    { "jmSigmaMinor",           SUMO_ATTR_JM_SIGMA_MINOR },
    { "jmTimegapMinor",         SUMO_ATTR_JM_TIMEGAP_MINOR },

    { "last",                   SUMO_ATTR_LAST },
    { "cost",                   SUMO_ATTR_COST },
    { "costs",                  SUMO_ATTR_COSTS },
    { "savings",                SUMO_ATTR_SAVINGS },
    { "probability",            SUMO_ATTR_PROB },
    { "count",                  SUMO_ATTR_COUNT },
    { "probabilities",          SUMO_ATTR_PROBS },
    { "routes",                 SUMO_ATTR_ROUTES },
    { "vTypes",                 SUMO_ATTR_VTYPES },

    { "lane",                   SUMO_ATTR_LANE },
    { "lanes",                  SUMO_ATTR_LANES },
    { "from",                   SUMO_ATTR_FROM },
    { "to",                     SUMO_ATTR_TO },
    { "fromLonLat",             SUMO_ATTR_FROMLONLAT },
    { "toLonLat",               SUMO_ATTR_TOLONLAT },
    { "fromXY",                 SUMO_ATTR_FROMXY },
    { "toXY",                   SUMO_ATTR_TOXY },
    { "fromJunction",           SUMO_ATTR_FROMJUNCTION },
    { "toJunction",             SUMO_ATTR_TOJUNCTION },
    { "period",                 SUMO_ATTR_PERIOD },
    { "repeat",                 SUMO_ATTR_REPEAT },
    { "fromTaz",                SUMO_ATTR_FROM_TAZ },
    { "toTaz",                  SUMO_ATTR_TO_TAZ },
    { "reroute",                SUMO_ATTR_REROUTE },
    { "personCapacity",         SUMO_ATTR_PERSON_CAPACITY },
    { "containerCapacity",      SUMO_ATTR_CONTAINER_CAPACITY },
    { "parkingLength",          SUMO_ATTR_PARKING_LENGTH },
    { "personNumber",           SUMO_ATTR_PERSON_NUMBER },
    { "containerNumber",        SUMO_ATTR_CONTAINER_NUMBER },
    { "modes",                  SUMO_ATTR_MODES },
    { "walkFactor",             SUMO_ATTR_WALKFACTOR },

    { "function",               SUMO_ATTR_FUNCTION },
    { "pos",                    SUMO_ATTR_POSITION },
    { "posLat",                 SUMO_ATTR_POSITION_LAT },
    { "freq",                   SUMO_ATTR_FREQUENCY },
    { "style",                  SUMO_ATTR_STYLE },
    { "file",                   SUMO_ATTR_FILE },
    { "junction",               SUMO_ATTR_JUNCTION },
    { "number",                 SUMO_ATTR_NUMBER },
    { "duration",               SUMO_ATTR_DURATION },
    { "until",                  SUMO_ATTR_UNTIL },
    { "arrival",                SUMO_ATTR_ARRIVAL },
    { "extension",              SUMO_ATTR_EXTENSION },
    { "routeProbe",             SUMO_ATTR_ROUTEPROBE },
    { "crossingEdges",          SUMO_ATTR_CROSSING_EDGES },
    // Traffic light & Nodes
    { "time",                   SUMO_ATTR_TIME },
    { "begin",                  SUMO_ATTR_BEGIN },
    { "end",                    SUMO_ATTR_END },
    { "tl",                     SUMO_ATTR_TLID },
    { "tlType",                 SUMO_ATTR_TLTYPE },
    { "linkIndex",              SUMO_ATTR_TLLINKINDEX },
    { "linkIndex2",             SUMO_ATTR_TLLINKINDEX2 },
    { "shape",                  SUMO_ATTR_SHAPE },
    { "spreadType",             SUMO_ATTR_SPREADTYPE },
    { "radius",                 SUMO_ATTR_RADIUS },
    { "customShape",            SUMO_ATTR_CUSTOMSHAPE },
    { "keepClear",              SUMO_ATTR_KEEP_CLEAR },
    { "rightOfWay",             SUMO_ATTR_RIGHT_OF_WAY },
    { "fringe",                 SUMO_ATTR_FRINGE },
    { "color",                  SUMO_ATTR_COLOR },
    { "dir",                    SUMO_ATTR_DIR },
    { "state",                  SUMO_ATTR_STATE },
    { "layer",                  SUMO_ATTR_LAYER },
    { "fill",                   SUMO_ATTR_FILL },
    { "lineWidth",              SUMO_ATTR_LINEWIDTH },
    { "prefix",                 SUMO_ATTR_PREFIX },
    { "discard",                SUMO_ATTR_DISCARD },

    { "fromLane",               SUMO_ATTR_FROM_LANE },
    { "toLane",                 SUMO_ATTR_TO_LANE },
    { "dest",                   SUMO_ATTR_DEST },
    { "source",                 SUMO_ATTR_SOURCE },
    { "via",                    SUMO_ATTR_VIA },
    { "viaLonLat",              SUMO_ATTR_VIALONLAT },
    { "viaXY",                  SUMO_ATTR_VIAXY },
    { "viaJunctions",           SUMO_ATTR_VIAJUNCTIONS },
    { "nodes",                  SUMO_ATTR_NODES },
    { "visibility",             SUMO_ATTR_VISIBILITY_DISTANCE },

    { "minDur",                 SUMO_ATTR_MINDURATION },
    { "maxDur",                 SUMO_ATTR_MAXDURATION },
    { "next",                   SUMO_ATTR_NEXT },
    { "foes",                   SUMO_ATTR_FOES },
    // E2 detector
    { "cont",                   SUMO_ATTR_CONT },
    { "contPos",                SUMO_ATTR_CONTPOS },
    { "timeThreshold",          SUMO_ATTR_HALTING_TIME_THRESHOLD },
    { "speedThreshold",         SUMO_ATTR_HALTING_SPEED_THRESHOLD },
    { "jamThreshold",           SUMO_ATTR_JAM_DIST_THRESHOLD },
    { "show",                   SUMO_ATTR_SHOW_DETECTOR },
    // E3 detector
    { "openEntry",              SUMO_ATTR_OPEN_ENTRY },

    { "wautID",                 SUMO_ATTR_WAUT_ID },
    { "junctionID",             SUMO_ATTR_JUNCTION_ID },
    { "procedure",              SUMO_ATTR_PROCEDURE },
    { "synchron",               SUMO_ATTR_SYNCHRON },
    { "refTime",                SUMO_ATTR_REF_TIME },
    { "startProg",              SUMO_ATTR_START_PROG },
    { "off",                    SUMO_ATTR_OFF },
    { "friendlyPos",            SUMO_ATTR_FRIENDLY_POS },
    { "splitByType",            SUMO_ATTR_SPLIT_VTYPE },
    { "uncontrolled",           SUMO_ATTR_UNCONTROLLED },
    { "pass",                   SUMO_ATTR_PASS },
    { "busStop",                SUMO_ATTR_BUS_STOP },
    { "containerStop",          SUMO_ATTR_CONTAINER_STOP },
    { "parkingArea",            SUMO_ATTR_PARKING_AREA },
    { "roadsideCapacity",       SUMO_ATTR_ROADSIDE_CAPACITY },
    { "onRoad",                 SUMO_ATTR_ONROAD },
    { "chargingStation",        SUMO_ATTR_CHARGING_STATION},
    { "group",                  SUMO_ATTR_GROUP},
    { "line",                   SUMO_ATTR_LINE },
    { "lines",                  SUMO_ATTR_LINES },
    { "tripId",                 SUMO_ATTR_TRIP_ID },
    { "split",                  SUMO_ATTR_SPLIT },
    { "join",                   SUMO_ATTR_JOIN },
    { "intended",               SUMO_ATTR_INTENDED },
    { "value",                  SUMO_ATTR_VALUE },
    { "prohibitor",             SUMO_ATTR_PROHIBITOR },
    { "prohibited",             SUMO_ATTR_PROHIBITED },
    { "allow",                  SUMO_ATTR_ALLOW },
    { "disallow",               SUMO_ATTR_DISALLOW },
    { "prefer",                 SUMO_ATTR_PREFER },
    { "controlledInner",        SUMO_ATTR_CONTROLLED_INNER },
    { "vehsPerHour",            SUMO_ATTR_VEHSPERHOUR },
    { "personsPerHour",         SUMO_ATTR_PERSONSPERHOUR },
    { "output",                 SUMO_ATTR_OUTPUT },
    { "height",                 SUMO_ATTR_HEIGHT },
    { "guiShape",               SUMO_ATTR_GUISHAPE },
    { "osgFile",                SUMO_ATTR_OSGFILE },
    { "imgFile",                SUMO_ATTR_IMGFILE },
    { "relativePath",           SUMO_ATTR_RELATIVEPATH },
    { "angle",                  SUMO_ATTR_ANGLE },
    { "emissionClass",          SUMO_ATTR_EMISSIONCLASS },
    { "impatience",             SUMO_ATTR_IMPATIENCE },
    { "startPos",               SUMO_ATTR_STARTPOS },
    { "endPos",                 SUMO_ATTR_ENDPOS },
    { "triggered",              SUMO_ATTR_TRIGGERED },
    { "containerTriggered",     SUMO_ATTR_CONTAINER_TRIGGERED },
    { "parking",                SUMO_ATTR_PARKING },
    { "expected",               SUMO_ATTR_EXPECTED },
    { "expectedContainers",     SUMO_ATTR_EXPECTED_CONTAINERS },
    { "index",                  SUMO_ATTR_INDEX },

    { "entering",               SUMO_ATTR_ENTERING },
    { "excludeEmpty",           SUMO_ATTR_EXCLUDE_EMPTY },
    { "withInternal",           SUMO_ATTR_WITH_INTERNAL },
    { "trackVehicles",          SUMO_ATTR_TRACK_VEHICLES },
    { "detectPersons",          SUMO_ATTR_DETECT_PERSONS },
    { "maxTraveltime",          SUMO_ATTR_MAX_TRAVELTIME },
    { "minSamples",             SUMO_ATTR_MIN_SAMPLES },
    { "writeAttributes",        SUMO_ATTR_WRITE_ATTRIBUTES },

    { "lon",                    SUMO_ATTR_LON },
    { "lat",                    SUMO_ATTR_LAT },
    { "geo",                    SUMO_ATTR_GEO },
    { "geoShape",               SUMO_ATTR_GEOSHAPE },
    { "lon/lat",                SUMO_ATTR_GEOPOSITION },
    { "k",                      SUMO_ATTR_K },
    { "v",                      SUMO_ATTR_V },
    { "ref",                    SUMO_ATTR_REF },
    { "href",                   SUMO_ATTR_HREF },
    { "zoom",                   SUMO_ATTR_ZOOM },
    { "interpolated",           SUMO_ATTR_INTERPOLATED },
    { "threshold",              SUMO_ATTR_THRESHOLD },

    { "netOffset",              SUMO_ATTR_NET_OFFSET },
    { "convBoundary",           SUMO_ATTR_CONV_BOUNDARY },
    { "origBoundary",           SUMO_ATTR_ORIG_BOUNDARY },
    { "projParameter",          SUMO_ATTR_ORIG_PROJ },

    { "tauLast",                SUMO_ATTR_CF_PWAGNER2009_TAULAST },
    { "apProb",                 SUMO_ATTR_CF_PWAGNER2009_APPROB },
    { "delta",                  SUMO_ATTR_CF_IDM_DELTA },
    { "stepping",               SUMO_ATTR_CF_IDM_STEPPING },
    { "adaptFactor",            SUMO_ATTR_CF_IDMM_ADAPT_FACTOR },
    { "adaptTime",              SUMO_ATTR_CF_IDMM_ADAPT_TIME },
    { "phi",                    SUMO_ATTR_CF_KERNER_PHI },

    { "security",               SUMO_ATTR_CF_WIEDEMANN_SECURITY },
    { "estimation",             SUMO_ATTR_CF_WIEDEMANN_ESTIMATION },
    { "cc1",                    SUMO_ATTR_CF_W99_CC1 },
    { "cc2",                    SUMO_ATTR_CF_W99_CC2 },
    { "cc3",                    SUMO_ATTR_CF_W99_CC3 },
    { "cc4",                    SUMO_ATTR_CF_W99_CC4 },
    { "cc5",                    SUMO_ATTR_CF_W99_CC5 },
    { "cc6",                    SUMO_ATTR_CF_W99_CC6 },
    { "cc7",                    SUMO_ATTR_CF_W99_CC7 },
    { "cc8",                    SUMO_ATTR_CF_W99_CC8 },
    { "cc9",                    SUMO_ATTR_CF_W99_CC9 },

    { "ccDecel",                SUMO_ATTR_CF_CC_CCDECEL },
    { "constSpacing",           SUMO_ATTR_CF_CC_CONSTSPACING },
    { "kp",                     SUMO_ATTR_CF_CC_KP },
    { "lambda",                 SUMO_ATTR_CF_CC_LAMBDA },
    { "c1",                     SUMO_ATTR_CF_CC_C1 },
    { "xi",                     SUMO_ATTR_CF_CC_XI },
    { "omegaN",                 SUMO_ATTR_CF_CC_OMEGAN },
    { "tauEngine",              SUMO_ATTR_CF_CC_TAU },
    { "lanesCount",             SUMO_ATTR_CF_CC_LANES_COUNT },
    { "ccAccel",                SUMO_ATTR_CF_CC_CCACCEL },
    { "ploegKp",                SUMO_ATTR_CF_CC_PLOEG_KP },
    { "ploegKd",                SUMO_ATTR_CF_CC_PLOEG_KD },
    { "ploegH",                 SUMO_ATTR_CF_CC_PLOEG_H },
    { "flatbedKa",              SUMO_ATTR_CF_CC_FLATBED_KA },
    { "flatbedKv",              SUMO_ATTR_CF_CC_FLATBED_KV },
    { "flatbedKp",              SUMO_ATTR_CF_CC_FLATBED_KP },
    { "flatbedD",               SUMO_ATTR_CF_CC_FLATBED_D },
    { "flatbedH",               SUMO_ATTR_CF_CC_FLATBED_H },

    { "generateWalks",          SUMO_ATTR_GENERATE_WALKS },
    { "actType",                SUMO_ATTR_ACTTYPE },
    { "slope",                  SUMO_ATTR_SLOPE },
    { "junctionCornerDetail",   SUMO_ATTR_CORNERDETAIL },
    { "junctionLinkDetail",     SUMO_ATTR_LINKDETAIL },
    { "rectangularLaneCut",     SUMO_ATTR_RECTANGULAR_LANE_CUT },
    { "walkingareas",           SUMO_ATTR_WALKINGAREAS },
    { "lefthand",               SUMO_ATTR_LEFTHAND },
    { "limitTurnSpeed",         SUMO_ATTR_LIMIT_TURN_SPEED },
    { "checkLaneFoesAll",       SUMO_ATTR_CHECKLANEFOES_ALL },
    { "checkLaneFoesRoundabout", SUMO_ATTR_CHECKLANEFOES_ROUNDABOUT },
    { "tlsIgnoreInternalJunctionJam", SUMO_ATTR_TLS_IGNORE_INTERNAL_JUNCTION_JAM },
    { "avoidOverlap",           SUMO_ATTR_AVOID_OVERLAP },

    { "actorConfig",            SUMO_ATTR_ACTORCONFIG },
    { "vehicle",                SUMO_ATTR_VEHICLE },
    { "startTime",              SUMO_ATTR_STARTTIME },
    { "vehicleClass",           SUMO_ATTR_VEHICLECLASS },
    { "fuel",                   SUMO_ATTR_FUEL },
    { "acceleration",           SUMO_ATTR_ACCELERATION },
    { "origin",                 SUMO_ATTR_ORIGIN },
    { "destination",            SUMO_ATTR_DESTINATION },
    { "visible",                SUMO_ATTR_VISIBLE },

#ifndef WIN32
    { "commandPosix",   SUMO_ATTR_COMMAND },
#else
    { "commandWindows", SUMO_ATTR_COMMAND },
#endif

    // ActivityGen statistics file
    { "inhabitants",            AGEN_ATTR_INHABITANTS },
    { "households",             AGEN_ATTR_HOUSEHOLDS },
    { "childrenAgeLimit",       AGEN_ATTR_CHILDREN },
    { "retirementAgeLimit",     AGEN_ATTR_RETIREMENT },
    { "carRate",                AGEN_ATTR_CARS },
    { "unemploymentRate",       AGEN_ATTR_UNEMPLOYEMENT },
    { "laborDemand",            AGEN_ATTR_LABORDEMAND },
    { "footDistanceLimit",      AGEN_ATTR_MAX_FOOT_DIST },
    { "incomingTraffic",        AGEN_ATTR_IN_TRAFFIC },
    { "incoming",               AGEN_ATTR_INCOMING },
    { "outgoingTraffic",        AGEN_ATTR_OUT_TRAFFIC },
    { "outgoing",               AGEN_ATTR_OUTGOING },
    { "population",             AGEN_ATTR_POPULATION },
    { "workPosition",           AGEN_ATTR_OUT_WORKPOSITION },
    { "hour",                   AGEN_ATTR_HOUR },
    { "proportion",             AGEN_ATTR_PROP },
    { "capacity",               AGEN_ATTR_CAPACITY },
    { "opening",                AGEN_ATTR_OPENING },
    { "closing",                AGEN_ATTR_CLOSING },
    { "maxTripDuration",        AGEN_ATTR_MAX_TRIP_DURATION },
    { "rate",                   AGEN_ATTR_RATE },
    { "beginAge",               AGEN_ATTR_BEGINAGE },
    { "endAge",                 AGEN_ATTR_ENDAGE },
    { "peopleNbr",              AGEN_ATTR_PEOPLENBR },
    { "carPreference",          AGEN_ATTR_CARPREF },
    { "meanTimePerKmInCity",    AGEN_ATTR_CITYSPEED },
    { "freeTimeActivityRate",   AGEN_ATTR_FREETIMERATE },
    { "uniformRandomTraffic",   AGEN_ATTR_UNI_RAND_TRAFFIC },
    { "departureVariation",     AGEN_ATTR_DEP_VARIATION },

    // NETEDIT Attributes
    { "selected",                           GNE_ATTR_SELECTED },
    { "modificationStatusNotForPrinting",   GNE_ATTR_MODIFICATION_STATUS },
    { "shapeStart",                         GNE_ATTR_SHAPE_START },
    { "shapeEnd",                           GNE_ATTR_SHAPE_END },
    { "bidiRail",                           GNE_ATTR_BIDIR },
    { "blockMovement",                      GNE_ATTR_BLOCK_MOVEMENT },
    { "blockShape",                         GNE_ATTR_BLOCK_SHAPE },
    { "closedShape",                        GNE_ATTR_CLOSE_SHAPE },
    { "parentItem",                         GNE_ATTR_PARENT },
    { "dataSet",                            GNE_ATTR_DATASET },
    { "genericParameter",                   GNE_ATTR_PARAMETERS },
    { "defaultVTypeModified",               GNE_ATTR_DEFAULT_VTYPE_MODIFIED },
    { "centerAfterCreation",                GNE_ATTR_CENTER_AFTER_CREATION },
    { "fromBusStop",                        GNE_ATTR_FROM_BUSSTOP },
    { "toBusStop",                          GNE_ATTR_TO_BUSSTOP },

    { "carriageLength",     SUMO_ATTR_CARRIAGE_LENGTH },
    { "locomotiveLength",   SUMO_ATTR_LOCOMOTIVE_LENGTH },
    { "carriageGap",        SUMO_ATTR_CARRIAGE_GAP },

    { "targetLanes",    SUMO_ATTR_TARGETLANE },
    { "crossing",       SUMO_ATTR_CROSSING },

    { "xmlns:xsi",                      SUMO_ATTR_XMLNS },
    { "xsi:noNamespaceSchemaLocation",  SUMO_ATTR_SCHEMA_LOCATION },

    //@name RNG state saving attributes
    // @{
    { "default",           SUMO_ATTR_RNG_DEFAULT },
    { "routeHandler",      SUMO_ATTR_RNG_ROUTEHANDLER },
    { "insertionControl",  SUMO_ATTR_RNG_INSERTIONCONTROL },
    { "device",            SUMO_ATTR_RNG_DEVICE },
    { "device.btreceiver", SUMO_ATTR_RNG_DEVICE_BT },
    { "device.toc",        SUMO_ATTR_RNG_DEVICE_TOC },
    { "driverState",       SUMO_ATTR_RNG_DRIVERSTATE },
    // @}

    // Other
    { "",                       SUMO_ATTR_NOTHING } //< must be the last one
};


const std::string SUMO_PARAM_ORIGID("origId");


StringBijection<SumoXMLNodeType>::Entry SUMOXMLDefinitions::sumoNodeTypeValues[] = {
    {"traffic_light",               SumoXMLNodeType::TRAFFIC_LIGHT},
    {"traffic_light_unregulated",   SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION},
    {"traffic_light_right_on_red",  SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED},
    {"rail_signal",                 SumoXMLNodeType::RAIL_SIGNAL},
    {"rail_crossing",               SumoXMLNodeType::RAIL_CROSSING},
    {"priority",                    SumoXMLNodeType::PRIORITY},
    {"priority_stop",               SumoXMLNodeType::PRIORITY_STOP},
    {"right_before_left",           SumoXMLNodeType::RIGHT_BEFORE_LEFT},
    {"allway_stop",                 SumoXMLNodeType::ALLWAY_STOP},
    {"zipper",                      SumoXMLNodeType::ZIPPER},
    {"district",                    SumoXMLNodeType::DISTRICT},
    {"unregulated",                 SumoXMLNodeType::NOJUNCTION},
    {"internal",                    SumoXMLNodeType::INTERNAL},
    {"dead_end",                    SumoXMLNodeType::DEAD_END},
    {"DEAD_END",                    SumoXMLNodeType::DEAD_END_DEPRECATED},
    {"unknown",                     SumoXMLNodeType::UNKNOWN} //< must be the last one
};


StringBijection<SumoXMLEdgeFunc>::Entry SUMOXMLDefinitions::sumoEdgeFuncValues[] = {
    {"unknown",     SumoXMLEdgeFunc::UNKNOWN},
    {"normal",      SumoXMLEdgeFunc::NORMAL},
    {"connector",   SumoXMLEdgeFunc::CONNECTOR},
    {"crossing",    SumoXMLEdgeFunc::CROSSING},
    {"walkingarea", SumoXMLEdgeFunc::WALKINGAREA},
    {"internal",    SumoXMLEdgeFunc::INTERNAL} //< must be the last one
};


StringBijection<LaneSpreadFunction>::Entry SUMOXMLDefinitions::laneSpreadFunctionValues[] = {
    {"right",      LaneSpreadFunction::RIGHT }, // default: geometry is left edge border, lanes flare to the right
    {"roadCenter", LaneSpreadFunction::ROADCENTER }, // geometry is center of the bidirectional road
    {"center",     LaneSpreadFunction::CENTER } // geometry is center of the edge (must be the last one)
};

StringBijection<RightOfWay>::Entry SUMOXMLDefinitions::rightOfWayValuesInitializer[] = {
    {"edgePriority", RightOfWay::EDGEPRIORITY },
    {"default",      RightOfWay::DEFAULT } // default (must be the last one)
};

StringBijection<FringeType>::Entry SUMOXMLDefinitions::fringeTypeValuesInitializer[] = {
    {"outer",   FringeType::OUTER },
    {"inner",   FringeType::INNER },
    {"default", FringeType::DEFAULT } // default (must be the last one)
};

StringBijection<PersonMode>::Entry SUMOXMLDefinitions::personModeValuesInitializer[] = {
    {"none",    PersonMode::NONE},
    {"walk",    PersonMode::WALK},
    {"bicycle", PersonMode::BICYCLE },
    {"car",     PersonMode::CAR },
    {"public",  PersonMode::PUBLIC} // (must be the last one)
};

StringBijection<LinkState>::Entry SUMOXMLDefinitions::linkStateValues[] = {
    { "G", LINKSTATE_TL_GREEN_MAJOR },
    { "g", LINKSTATE_TL_GREEN_MINOR },
    { "r", LINKSTATE_TL_RED },
    { "u", LINKSTATE_TL_REDYELLOW },
    { "Y", LINKSTATE_TL_YELLOW_MAJOR },
    { "y", LINKSTATE_TL_YELLOW_MINOR },
    { "o", LINKSTATE_TL_OFF_BLINKING },
    { "O", LINKSTATE_TL_OFF_NOSIGNAL },
    { "M", LINKSTATE_MAJOR },
    { "m", LINKSTATE_MINOR },
    { "=", LINKSTATE_EQUAL },
    { "s", LINKSTATE_STOP },
    { "w", LINKSTATE_ALLWAY_STOP },
    { "Z", LINKSTATE_ZIPPER },
    { "-", LINKSTATE_DEADEND } //< must be the last one
};

const char SUMOXMLDefinitions::AllowedTLS_linkStatesInitializer[] = {
    LINKSTATE_TL_GREEN_MAJOR,
    LINKSTATE_TL_GREEN_MINOR,
    LINKSTATE_STOP, // used for SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED
    LINKSTATE_TL_RED,
    LINKSTATE_TL_REDYELLOW,
    LINKSTATE_TL_YELLOW_MAJOR,
    LINKSTATE_TL_YELLOW_MINOR,
    LINKSTATE_TL_OFF_BLINKING,
    LINKSTATE_TL_OFF_NOSIGNAL
};
const std::string SUMOXMLDefinitions::ALLOWED_TLS_LINKSTATES(AllowedTLS_linkStatesInitializer, 9);

StringBijection<LinkDirection>::Entry SUMOXMLDefinitions::linkDirectionValues[] = {
    { "s",      LinkDirection::STRAIGHT },
    { "t",      LinkDirection::TURN },
    { "T",      LinkDirection::TURN_LEFTHAND },
    { "l",      LinkDirection::LEFT },
    { "r",      LinkDirection::RIGHT },
    { "L",      LinkDirection::PARTLEFT },
    { "R",      LinkDirection::PARTRIGHT },
    { "invalid", LinkDirection::NODIR } //< must be the last one
};


StringBijection<TrafficLightType>::Entry SUMOXMLDefinitions::trafficLightTypesValues[] = {
    { "static",         TrafficLightType::STATIC },
    { "railSignal",     TrafficLightType::RAIL_SIGNAL },
    { "railCrossing",   TrafficLightType::RAIL_CROSSING },
    { "actuated",       TrafficLightType::ACTUATED },
    { "delay_based",    TrafficLightType::DELAYBASED },
    { "sotl_phase",     TrafficLightType::SOTL_PHASE },
    { "sotl_platoon",   TrafficLightType::SOTL_PLATOON },
    { "sotl_request",   TrafficLightType::SOTL_REQUEST },
    { "sotl_wave",      TrafficLightType::SOTL_WAVE },
    { "sotl_marching",  TrafficLightType::SOTL_MARCHING },
    { "swarm",          TrafficLightType::SWARM_BASED },
    { "deterministic",  TrafficLightType::HILVL_DETERMINISTIC },
    { "off",            TrafficLightType::OFF },
    { "<invalid>",      TrafficLightType::INVALID } //< must be the last one
};


StringBijection<LaneChangeModel>::Entry SUMOXMLDefinitions::laneChangeModelValues[] = {
    { "DK2008",     LCM_DK2008 },
    { "LC2013",     LCM_LC2013 },
    { "SL2015",     LCM_SL2015 },
    { "default",    LCM_DEFAULT } //< must be the last one
};

StringBijection<SumoXMLTag>::Entry SUMOXMLDefinitions::carFollowModelValues[] = {
    { "IDM",         SUMO_TAG_CF_IDM },
    { "IDMM",        SUMO_TAG_CF_IDMM },
    { "Krauss",      SUMO_TAG_CF_KRAUSS },
    { "KraussPS",    SUMO_TAG_CF_KRAUSS_PLUS_SLOPE },
    { "KraussOrig1", SUMO_TAG_CF_KRAUSS_ORIG1 },
    { "KraussX",     SUMO_TAG_CF_KRAUSSX }, // experimental extensions to the Krauss model
    { "SmartSK",     SUMO_TAG_CF_SMART_SK },
    { "Daniel1",     SUMO_TAG_CF_DANIEL1 },
    { "PWagner2009", SUMO_TAG_CF_PWAGNER2009 },
    { "BKerner",     SUMO_TAG_CF_BKERNER },
    { "Rail",        SUMO_TAG_CF_RAIL },
    { "CC",          SUMO_TAG_CF_CC },
    { "ACC",         SUMO_TAG_CF_ACC },
    { "CACC",        SUMO_TAG_CF_CACC },
    { "W99",         SUMO_TAG_CF_W99 },
    { "Wiedemann",   SUMO_TAG_CF_WIEDEMANN } //< must be the last one
};

StringBijection<LateralAlignment>::Entry SUMOXMLDefinitions::lateralAlignmentValues[] = {
    { "right",         LATALIGN_RIGHT },
    { "center",        LATALIGN_CENTER },
    { "arbitrary",     LATALIGN_ARBITRARY },
    { "nice",          LATALIGN_NICE },
    { "compact",       LATALIGN_COMPACT },
    { "left",          LATALIGN_LEFT } //< must be the last one
};

StringBijection<LaneChangeAction>::Entry SUMOXMLDefinitions::laneChangeActionValues[] = {
    { "stay",        LCA_STAY },
    { "left",        LCA_LEFT },
    { "right",       LCA_RIGHT },
    { "strategic",   LCA_STRATEGIC },
    { "cooperative", LCA_COOPERATIVE },
    { "speedGain",   LCA_SPEEDGAIN },
    { "keepRight",   LCA_KEEPRIGHT },
    { "sublane",     LCA_SUBLANE },
    { "traci",       LCA_TRACI },
    { "urgent",      LCA_URGENT },
    { "overlapping", LCA_OVERLAPPING },
    { "blocked",     LCA_BLOCKED },
    { "amBL",        LCA_AMBLOCKINGLEADER },
    { "amBF",        LCA_AMBLOCKINGFOLLOWER },
    { "amBB",        LCA_AMBACKBLOCKER },
    { "amBBS",       LCA_AMBACKBLOCKER_STANDING },
    { "MR",          LCA_MRIGHT },
    { "ML",          LCA_MLEFT },

    { "unknown",     LCA_UNKNOWN } //< must be the last one
};

StringBijection<TrainType>::Entry SUMOXMLDefinitions::trainTypeValues[] = {
    { "NGT400",     TRAINTYPE_NGT400 },
    { "NGT400_16",  TRAINTYPE_NGT400_16 },
    { "RB425",      TRAINTYPE_RB425 },
    { "RB628",      TRAINTYPE_RB628 },
    { "ICE1",       TRAINTYPE_ICE1 },
    { "REDosto7",   TRAINTYPE_REDOSTO7 },
    { "Freight",    TRAINTYPE_FREIGHT },
    { "ICE3",       TRAINTYPE_ICE3 }
};


StringBijection<int> SUMOXMLDefinitions::Tags(
    SUMOXMLDefinitions::tags, SUMO_TAG_NOTHING);

StringBijection<int> SUMOXMLDefinitions::Attrs(
    SUMOXMLDefinitions::attrs, SUMO_ATTR_NOTHING);

StringBijection<SumoXMLNodeType> SUMOXMLDefinitions::NodeTypes(
    SUMOXMLDefinitions::sumoNodeTypeValues, SumoXMLNodeType::UNKNOWN);

StringBijection<SumoXMLEdgeFunc> SUMOXMLDefinitions::EdgeFunctions(
    SUMOXMLDefinitions::sumoEdgeFuncValues, SumoXMLEdgeFunc::INTERNAL);

StringBijection<LaneSpreadFunction> SUMOXMLDefinitions::LaneSpreadFunctions(
    SUMOXMLDefinitions::laneSpreadFunctionValues, LaneSpreadFunction::CENTER);

StringBijection<RightOfWay> SUMOXMLDefinitions::RightOfWayValues(
    SUMOXMLDefinitions::rightOfWayValuesInitializer, RightOfWay::DEFAULT);

StringBijection<FringeType> SUMOXMLDefinitions::FringeTypeValues(
    SUMOXMLDefinitions::fringeTypeValuesInitializer, FringeType::DEFAULT);

StringBijection<PersonMode> SUMOXMLDefinitions::PersonModeValues(
    SUMOXMLDefinitions::personModeValuesInitializer, PersonMode::PUBLIC);

StringBijection<LinkState> SUMOXMLDefinitions::LinkStates(
    SUMOXMLDefinitions::linkStateValues, LINKSTATE_DEADEND);

StringBijection<LinkDirection> SUMOXMLDefinitions::LinkDirections(
    SUMOXMLDefinitions::linkDirectionValues, LinkDirection::NODIR);

StringBijection<TrafficLightType> SUMOXMLDefinitions::TrafficLightTypes(
    SUMOXMLDefinitions::trafficLightTypesValues, TrafficLightType::INVALID);

StringBijection<LaneChangeModel> SUMOXMLDefinitions::LaneChangeModels(
    SUMOXMLDefinitions::laneChangeModelValues, LCM_DEFAULT);

StringBijection<SumoXMLTag> SUMOXMLDefinitions::CarFollowModels(
    SUMOXMLDefinitions::carFollowModelValues, SUMO_TAG_CF_WIEDEMANN);

StringBijection<LateralAlignment> SUMOXMLDefinitions::LateralAlignments(
    SUMOXMLDefinitions::lateralAlignmentValues, LATALIGN_LEFT);

StringBijection<LaneChangeAction> SUMOXMLDefinitions::LaneChangeActions(
    SUMOXMLDefinitions::laneChangeActionValues, LCA_UNKNOWN);

StringBijection<TrainType> SUMOXMLDefinitions::TrainTypes(
    SUMOXMLDefinitions::trainTypeValues, TRAINTYPE_ICE3);


std::string
SUMOXMLDefinitions::getJunctionIDFromInternalEdge(const std::string internalEdge) {
    assert(internalEdge[0] == ':');
    return internalEdge.substr(1, internalEdge.rfind('_') - 1);
}


std::string
SUMOXMLDefinitions::getEdgeIDFromLane(const std::string laneID) {
    return laneID.substr(0, laneID.rfind('_'));
}


bool
SUMOXMLDefinitions::isValidNetID(const std::string& value) {
    return (value.size() > 0) && value.find_first_of(" \t\n\r|\\'\";,:!<>&*?") == std::string::npos;
}


bool
SUMOXMLDefinitions::isValidVehicleID(const std::string& value) {
    return (value.size() > 0) && value.find_first_of(" \t\n\r|\\'\";,!<>&*?") == std::string::npos;
}


bool
SUMOXMLDefinitions::isValidTypeID(const std::string& value) {
    return (value.size() > 0) && value.find_first_of(" \t\n\r|\\'\";,<>&*?") == std::string::npos;
}

bool
SUMOXMLDefinitions::isValidAdditionalID(const std::string& value) {
    return (value.size() > 0) && value.find_first_of(" \t\n\r|\\'\";,!<>&*?") == std::string::npos;
}

bool
SUMOXMLDefinitions::isValidDetectorID(const std::string& value) {
    // special case: ' ' allowed
    return (value.size() > 0) && value.find_first_of("\t\n\r|\\'\";,!<>&*?") == std::string::npos;
}

bool
SUMOXMLDefinitions::isValidAttribute(const std::string& value) {
    return value.find_first_of("\t\n\r@$%^&/|\\{}*'\";<>") == std::string::npos;
}


bool
SUMOXMLDefinitions::isValidFilename(const std::string& value) {
    return (value.find_first_of("\t\n\r@$%^&|{}*'\";<>") == std::string::npos);
}


bool
SUMOXMLDefinitions::isValidListOfNetIDs(const std::string& value) {
    const std::vector<std::string>& typeIDs = StringTokenizer(value).getVector();
    if (typeIDs.empty()) {
        return false;
    } else {
        // check that gives IDs are valid
        for (auto i : typeIDs) {
            if (!SUMOXMLDefinitions::isValidNetID(i)) {
                return false;
            }
        }
        return true;
    }
}


bool
SUMOXMLDefinitions::isValidListOfTypeID(const std::string& value) {
    const std::vector<std::string>& typeIDs = StringTokenizer(value).getVector();
    if (typeIDs.empty()) {
        return false;
    } else {
        // check that gives IDs are valid
        for (const auto& i : typeIDs) {
            if (!SUMOXMLDefinitions::isValidTypeID(i)) {
                return false;
            }
        }
        return true;
    }
}


bool
SUMOXMLDefinitions::isValidParameterKey(const std::string& value) {
    // Generic parameters keys cannot be empty
    if (value.empty()) {
        return false;
    } else {
        return isValidAttribute(value);
    }
}


bool
SUMOXMLDefinitions::isValidParameterValue(const std::string& /*value*/) {
    // Generic parameters should not be restricted (characters such as <>'" only
    // reach this function if they are properly escaped in the xml inputs (and
    // they are also escaped when writing)
    return true;
}


/****************************************************************************/
