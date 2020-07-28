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
/// @file    SUMOXMLDefinitions.h
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
#pragma once
#include <config.h>

#include <utils/common/StringBijection.h>

// ===========================================================================
// definitions
// ===========================================================================
/**
 * @enum SumoXMLTag
 * @brief Numbers representing SUMO-XML - element names
 * @see GenericSAXHandler
 * @see SUMOSAXHandler
 */
enum SumoXMLTag {
    /// @brief invalid tag
    SUMO_TAG_NOTHING,
    /// @brief root element of a network file
    SUMO_TAG_NET,
    /// @brief begin/end of the description of an edge
    SUMO_TAG_EDGE,
    /// @brief begin/end of the description of a single lane
    SUMO_TAG_LANE,
    /// @brief begin/end of the description of a neighboring lane
    SUMO_TAG_NEIGH,
    /// @brief begin/end of the description of a Point of interest
    SUMO_TAG_POI,
    /// @brief begin/end of the description of a Point of interest over Lane (used by Netedit)
    SUMO_TAG_POILANE,
    /// @brief begin/end of the description of a polygon
    SUMO_TAG_POLY,
    /// @brief begin/end of the description of a junction
    SUMO_TAG_JUNCTION,
    /// @brief begin/end of the description of an edge restriction
    SUMO_TAG_RESTRICTION,
    /// @brief an e1 detector
    SUMO_TAG_E1DETECTOR,
    /// @brief alternative tag for e1 detector
    SUMO_TAG_INDUCTION_LOOP,
    /// @brief an e2 detector
    SUMO_TAG_E2DETECTOR,
    /// @brief an e2 detector over multiple lanes (used by Netedit)
    SUMO_TAG_E2DETECTOR_MULTILANE,
    /// @brief alternative tag for e2 detector
    SUMO_TAG_LANE_AREA_DETECTOR,
    /// @brief an e3 detector
    SUMO_TAG_E3DETECTOR,
    /// @brief alternative tag for e3 detector
    SUMO_TAG_ENTRY_EXIT_DETECTOR,
    /// @brief an edge based mean data detector
    SUMO_TAG_MEANDATA_EDGE,
    /// @brief a lane based mean data detector
    SUMO_TAG_MEANDATA_LANE,
    /// @brief an e3 entry point
    SUMO_TAG_DET_ENTRY,
    /// @brief an e3 exit point
    SUMO_TAG_DET_EXIT,
    /// @brief  An edge-following detector
    SUMO_TAG_EDGEFOLLOWDETECTOR,
    /// @brief An instantenous induction loop
    SUMO_TAG_INSTANT_INDUCTION_LOOP,
    /// @brief A variable speed sign
    SUMO_TAG_VSS,
    /// @brief A calibrator placed over edge
    SUMO_TAG_CALIBRATOR,
    /// @brief A calibrator placed over lane (used in netedit)
    SUMO_TAG_LANECALIBRATOR,
    /// @brief  A rerouter
    SUMO_TAG_REROUTER,
    /// @brief  A bus stop
    SUMO_TAG_BUS_STOP,
    /// @brief  A train stop (alias for bus stop)
    SUMO_TAG_TRAIN_STOP,
    /// @brief A pt line
    SUMO_TAG_PT_LINE,
    /// @brief  An access point for a train stop
    SUMO_TAG_ACCESS,
    /// @brief  A container stop
    SUMO_TAG_CONTAINER_STOP,
    /// @brief A parking area
    SUMO_TAG_PARKING_AREA,
    /// @brief A parking space for a single vehicle within a parking area
    SUMO_TAG_PARKING_SPACE,
    /// @brief  A Charging Station
    SUMO_TAG_CHARGING_STATION,
    /// @brief  An overhead wire segment
    SUMO_TAG_OVERHEAD_WIRE_SEGMENT,
    /// @brief  An overhead wire section
    SUMO_TAG_OVERHEAD_WIRE_SECTION,
    /// @brief  A traction substation
    SUMO_TAG_TRACTION_SUBSTATION,
    /// @brief  An overhead wire clamp (connection of wires in opposite directions)
    SUMO_TAG_OVERHEAD_WIRE_CLAMP,
    /// @brief a vtypeprobe detector
    SUMO_TAG_VTYPEPROBE,
    /// @brief a routeprobe detector
    SUMO_TAG_ROUTEPROBE,
    /// @brief root element of a route file
    SUMO_TAG_ROUTES,
    /// @brief description of a vehicle
    SUMO_TAG_VEHICLE,
    /// @brief description of a vehicle type
    SUMO_TAG_VTYPE,
    /// @brief description of a person type (used in NETEDIT)
    SUMO_TAG_PTYPE,
    /// @brief begin/end of the description of a route
    SUMO_TAG_ROUTE,
    /// @brief description of a logic request within the junction
    SUMO_TAG_REQUEST,
    /// @brief a source
    SUMO_TAG_SOURCE,
    /// @brief a traffic assignment zone
    SUMO_TAG_TAZ,
    /// @brief a source within a district (connection road)
    SUMO_TAG_TAZSOURCE,
    /// @brief a sink within a district (connection road)
    SUMO_TAG_TAZSINK,
    /// @brief a traffic light
    SUMO_TAG_TRAFFIC_LIGHT,
    /// @brief a traffic light logic
    SUMO_TAG_TLLOGIC,
    /// @brief a single phase description
    SUMO_TAG_PHASE,
    /// @brief a single trip definition (used by router)
    SUMO_TAG_TRIP,
    /// @brief a single trip definition that uses TAZs (used in NETEDIT)
    SUMO_TAG_TRIP_TAZ,
    /// @brief a flow definitio nusing a from-to edges instead of a route (used by router)
    SUMO_TAG_FLOW,
    /// @brief a flow definition within in Calibrator (used in NETEDIT)
    SUMO_TAG_FLOW_CALIBRATOR,
    /// @brief a flow state definition (used when saving and loading simulatino state)
    SUMO_TAG_FLOWSTATE,
    /// @brief trigger: a step description
    SUMO_TAG_STEP,
    /// @brief an aggreagated-output interval
    SUMO_TAG_INTERVAL,
    /// @brief a relation between two edges
    SUMO_TAG_EDGEREL,
    /// @brief a relation between two TAZs
    SUMO_TAG_TAZREL,
    /// @brief The definition of a periodic event
    SUMO_TAG_TIMEDEVENT,
    /// @brief Incoming edge specification (jtrrouter)
    SUMO_TAG_FROMEDGE,
    /// @brief Outgoing edge specification (jtrrouter)
    SUMO_TAG_TOEDGE,
    /// @brief Sink(s) specification
    SUMO_TAG_SINK,
    /// @brief parameter associated to a certain key
    SUMO_TAG_PARAM,
    SUMO_TAG_WAUT,
    SUMO_TAG_WAUT_SWITCH,
    SUMO_TAG_WAUT_JUNCTION,
    /// @brief segment of a lane
    SUMO_TAG_SEGMENT,
    /// @brief delete certain element (note: DELETE is a macro)
    SUMO_TAG_DEL,
    /// @brief stop for vehicles
    SUMO_TAG_STOP,
    /// @brief stop placed over a lane (used in netedit)
    SUMO_TAG_STOP_LANE,
    /// @brief stop placed over a busStop (used in netedit)
    SUMO_TAG_STOP_BUSSTOP,
    /// @brief stop placed over a containerStop (used in netedit)
    SUMO_TAG_STOP_CONTAINERSTOP,
    /// @brief stop placed over a charging station (used in netedit)
    SUMO_TAG_STOP_CHARGINGSTATION,
    /// @brief stop placed over a parking area (used in netedit)
    SUMO_TAG_STOP_PARKINGAREA,
    /// @brief probability of destiny of a reroute
    SUMO_TAG_DEST_PROB_REROUTE,
    /// @brief reroute of type closing
    SUMO_TAG_CLOSING_REROUTE,
    /// @brief lane of a reroute of type closing
    SUMO_TAG_CLOSING_LANE_REROUTE,
    /// @brief probability of route of a reroute
    SUMO_TAG_ROUTE_PROB_REROUTE,
    /// @brief entry for an alternative parking zone
    SUMO_TAG_PARKING_ZONE_REROUTE,
    /// @brief type of polygon
    SUMO_TAG_POLYTYPE,
    /// @brief connectio between two lanes
    SUMO_TAG_CONNECTION,
    /// @brief prohibition of circulation between two edges
    SUMO_TAG_PROHIBITION,
    /// @brief split something
    SUMO_TAG_SPLIT,
    /// @brief alternative definition for junction
    SUMO_TAG_NODE,
    /// @brief type
    SUMO_TAG_TYPE,
    /// @brief definition of a detector
    SUMO_TAG_DETECTOR_DEFINITION,
    /// @brief distribution of a route
    SUMO_TAG_ROUTE_DISTRIBUTION,
    /// @brief distribution of a vehicle type
    SUMO_TAG_VTYPE_DISTRIBUTION,
    /// @brief vaporizer of vehicles
    SUMO_TAG_VAPORIZER,
    /// @brief roundabout defined in junction
    SUMO_TAG_ROUNDABOUT,
    /// @brief Join operation
    SUMO_TAG_JOIN,
    /// @brief join exlude operation
    SUMO_TAG_JOINEXCLUDE,
    /// @brief crossing between edges for pedestrians
    SUMO_TAG_CROSSING,
    /// @brief walking area for pedestrians
    SUMO_TAG_WALKINGAREA,
    /// @brief Information on vClass specific stop offsets at lane end
    SUMO_TAG_STOPOFFSET,

    SUMO_TAG_WAY,
    SUMO_TAG_ND,
    SUMO_TAG_TAG,
    SUMO_TAG_RELATION,
    SUMO_TAG_MEMBER,

    /// @name parameters associated to view settings
    /// @{
    SUMO_TAG_VIEWSETTINGS,
    SUMO_TAG_VIEWSETTINGS_DECAL,
    SUMO_TAG_VIEWSETTINGS_LIGHT,
    SUMO_TAG_VIEWSETTINGS_SCHEME,
    SUMO_TAG_VIEWSETTINGS_OPENGL,
    SUMO_TAG_VIEWSETTINGS_BACKGROUND,
    SUMO_TAG_VIEWSETTINGS_EDGES,
    SUMO_TAG_VIEWSETTINGS_VEHICLES,
    SUMO_TAG_VIEWSETTINGS_PERSONS,
    SUMO_TAG_VIEWSETTINGS_CONTAINERS,
    SUMO_TAG_VIEWSETTINGS_JUNCTIONS,
    SUMO_TAG_VIEWSETTINGS_ADDITIONALS,
    SUMO_TAG_VIEWSETTINGS_POIS,
    SUMO_TAG_VIEWSETTINGS_POLYS,
    SUMO_TAG_VIEWSETTINGS_LEGEND,
    SUMO_TAG_VIEWSETTINGS_EVENT,
    SUMO_TAG_VIEWSETTINGS_EVENT_JAM_TIME,
    SUMO_TAG_INCLUDE,
    SUMO_TAG_DELAY,
    SUMO_TAG_VIEWPORT,
    SUMO_TAG_SNAPSHOT,
    SUMO_TAG_BREAKPOINT,
    SUMO_TAG_LOCATION,
    SUMO_TAG_COLORSCHEME,
    SUMO_TAG_SCALINGSCHEME,
    SUMO_TAG_ENTRY,
    SUMO_TAG_RNGSTATE,
    SUMO_TAG_RNGLANE,
    /// @}

    SUMO_TAG_VEHICLETRANSFER,
    SUMO_TAG_DEVICE,

    /// @name Car-Following models
    /// @{
    SUMO_TAG_CF_KRAUSS,
    SUMO_TAG_CF_KRAUSS_PLUS_SLOPE,
    SUMO_TAG_CF_KRAUSS_ORIG1,
    SUMO_TAG_CF_KRAUSSX,
    SUMO_TAG_CF_SMART_SK,
    SUMO_TAG_CF_DANIEL1,
    SUMO_TAG_CF_IDM,
    SUMO_TAG_CF_IDMM,
    SUMO_TAG_CF_PWAGNER2009,
    SUMO_TAG_CF_BKERNER,
    SUMO_TAG_CF_WIEDEMANN,
    SUMO_TAG_CF_W99,
    SUMO_TAG_CF_ACC,
    SUMO_TAG_CF_CACC,
    SUMO_TAG_CF_RAIL,
    SUMO_TAG_CF_CC,
    /// @}

    /// @name Persons
    /// @{
    SUMO_TAG_PERSON,
    SUMO_TAG_PERSONTRIP,
    SUMO_TAG_RIDE,
    SUMO_TAG_WALK,
    SUMO_TAG_PERSONFLOW,
    /// @}

    /// @name Data elements (used by Netedit)
    /// @{
    SUMO_TAG_DATASET,
    SUMO_TAG_DATAINTERVAL,
    /// @}

    SUMO_TAG_CONTAINER,
    SUMO_TAG_TRANSPORT,
    SUMO_TAG_TRANSHIP,
    SUMO_TAG_TRAJECTORIES,
    SUMO_TAG_TIMESTEP,
    SUMO_TAG_TIMESLICE,
    SUMO_TAG_ACTORCONFIG,
    SUMO_TAG_MOTIONSTATE,
    SUMO_TAG_OD_PAIR,

    /// @brief ActivityGen Tags
    AGEN_TAG_GENERAL,
    /// @brief streets object
    AGEN_TAG_STREET,
    /// @brief workingHours object
    AGEN_TAG_WORKHOURS,
    /// @brief opening for workingHours object
    AGEN_TAG_OPENING,
    /// @brief closing for workingHours object
    AGEN_TAG_CLOSING,
    /// @brief school object
    AGEN_TAG_SCHOOLS,
    /// @brief schools object
    AGEN_TAG_SCHOOL,
    /// @brief busStation and bus objects
    AGEN_TAG_BUSSTATION,
    /// @brief  bus line
    AGEN_TAG_BUSLINE,
    /// @brief stations for certain vehicles
    AGEN_TAG_STATIONS,
    /// @brief rev stations for certain vehicles
    AGEN_TAG_REV_STATIONS,
    /// @brief station for a certain vehicle
    AGEN_TAG_STATION,
    /// @brief frequency of a object
    AGEN_TAG_FREQUENCY,
    /// @brief population and children accompaniment brackets
    AGEN_TAG_POPULATION,
    /// @brief alternative definition for Population
    AGEN_TAG_BRACKET,
    //AGEN_TAG_CHILD_ACOMP,
    /// @brief city entrances
    AGEN_TAG_CITYGATES,
    /// @brief alternative definition for city entrances
    AGEN_TAG_ENTRANCE,
    /// @brief parameters
    AGEN_TAG_PARAM,

    /// @name Persons plans (used by Netedit)
    /// @{
    // person trips
    GNE_TAG_PERSONTRIP_EDGE_EDGE,
    GNE_TAG_PERSONTRIP_EDGE_BUSSTOP,
    GNE_TAG_PERSONTRIP_BUSSTOP_EDGE,
    GNE_TAG_PERSONTRIP_BUSSTOP_BUSSTOP,
    // walks
    GNE_TAG_WALK_EDGE_EDGE,
    GNE_TAG_WALK_BUSSTOP_EDGE,
    GNE_TAG_WALK_BUSSTOP_BUSSTOP,
    GNE_TAG_WALK_EDGE_BUSSTOP,
    GNE_TAG_WALK_EDGES,
    GNE_TAG_WALK_ROUTE,
    // rides
    GNE_TAG_RIDE_EDGE_EDGE,
    GNE_TAG_RIDE_EDGE_BUSSTOP,
    GNE_TAG_RIDE_BUSSTOP_EDGE,
    GNE_TAG_RIDE_BUSSTOP_BUSSTOP,
    // person stops
    GNE_TAG_PERSONSTOP_BUSSTOP,
    GNE_TAG_PERSONSTOP_EDGE,
    /// @brief description of a vehicle with an embedded route (used in NETEDIT)
    GNE_TAG_VEHICLEWITHROUTE,
    /// @brief embedded route (used in NETEDIT)
    GNE_TAG_ROUTE_EMBEDDED,
    /// @brief a flow definition nusing a route instead of a from-to edges route (used in NETEDIT)
    GNE_TAG_FLOW_ROUTE,
    /// @brief description of a vehicle with an embedded route (used in NETEDIT)
    GNE_TAG_FLOW_WITHROUTE,
    /// @brief description of a person with an embedded route (used in NETEDIT)
    GNE_TAG_PERSON_EMBEDDED
    /// @}
};


/**
 * @enum SumoXMLAttr
 * @brief Numbers representing SUMO-XML - attributes
 * @see GenericSAXHandler
 * @see SUMOSAXHandler
 */
enum SumoXMLAttr {
    /// @brief invalid attribute
    SUMO_ATTR_NOTHING = 0,

    /// @name meanData output attributes
    /// @note: sorted first to simplify filtering written attributes with bit operations
    /// @{
    /// MSMeanData_Net
    SUMO_ATTR_SAMPLEDSECONDS     =  1,
    SUMO_ATTR_DENSITY            =  2,
    SUMO_ATTR_LANEDENSITY        =  3,
    SUMO_ATTR_OCCUPANCY          =  4,
    SUMO_ATTR_WAITINGTIME        =  5,
    SUMO_ATTR_SPEED              =  6,
    SUMO_ATTR_DEPARTED           =  7,
    SUMO_ATTR_ARRIVED            =  8,
    SUMO_ATTR_ENTERED            =  9,
    SUMO_ATTR_LEFT               = 10,
    SUMO_ATTR_VAPORIZED          = 11,
    SUMO_ATTR_TRAVELTIME         = 12,
    SUMO_ATTR_LANECHANGEDFROM    = 13,
    SUMO_ATTR_LANECHANGEDTO      = 14,
    SUMO_ATTR_OVERLAPTRAVELTIME  = 15,
    /// MSMeanData_Emissions
    SUMO_ATTR_CO_ABS             = 16,
    SUMO_ATTR_CO2_ABS            = 17,
    SUMO_ATTR_HC_ABS             = 18,
    SUMO_ATTR_PMX_ABS            = 19,
    SUMO_ATTR_NOX_ABS            = 20,
    SUMO_ATTR_FUEL_ABS           = 21,
    SUMO_ATTR_ELECTRICITY_ABS    = 22,
    SUMO_ATTR_CO_NORMED          = 23,
    SUMO_ATTR_CO2_NORMED         = 24,
    SUMO_ATTR_HC_NORMED          = 25,
    SUMO_ATTR_PMX_NORMED         = 26,
    SUMO_ATTR_NOX_NORMED         = 27,
    SUMO_ATTR_FUEL_NORMED        = 28,
    SUMO_ATTR_ELECTRICITY_NORMED = 29,
    SUMO_ATTR_CO_PERVEH          = 30,
    SUMO_ATTR_CO2_PERVEH         = 31,
    SUMO_ATTR_HC_PERVEH          = 32,
    SUMO_ATTR_PMX_PERVEH         = 33,
    SUMO_ATTR_NOX_PERVEH         = 34,
    SUMO_ATTR_FUEL_PERVEH        = 35,
    SUMO_ATTR_ELECTRICITY_PERVEH = 36,
    /// MSMeanData_Harmonoise
    SUMO_ATTR_NOISE              = 37,
    /// MSMeanData_Amitran
    SUMO_ATTR_AMOUNT             = 38,
    SUMO_ATTR_AVERAGESPEED       = 39,
    /// @}

    /// @name common attributes
    /// @{
    SUMO_ATTR_ID,
    SUMO_ATTR_REFID,
    SUMO_ATTR_NAME,
    SUMO_ATTR_TYPE,
    SUMO_ATTR_VERSION,
    SUMO_ATTR_PRIORITY,
    SUMO_ATTR_NUMLANES,
    SUMO_ATTR_ONEWAY,
    SUMO_ATTR_WIDTH,
    SUMO_ATTR_WIDTHRESOLUTION,
    SUMO_ATTR_MAXWIDTH,
    SUMO_ATTR_MINWIDTH,
    SUMO_ATTR_SIDEWALKWIDTH,
    SUMO_ATTR_BIKELANEWIDTH,
    SUMO_ATTR_REMOVE,
    SUMO_ATTR_LENGTH,
    SUMO_ATTR_BIDI,
    SUMO_ATTR_DISTANCE,
    SUMO_ATTR_ID_BEFORE,
    SUMO_ATTR_ID_AFTER,
    SUMO_ATTR_X,
    SUMO_ATTR_Y,
    SUMO_ATTR_Z,
    SUMO_ATTR_CENTER_X,
    SUMO_ATTR_CENTER_Y,
    SUMO_ATTR_CENTER_Z,
    /// @}

    /// @name sumo-junction attributes
    /// @{
    SUMO_ATTR_KEY,
    SUMO_ATTR_REQUESTSIZE,
    SUMO_ATTR_REQUEST,
    SUMO_ATTR_RESPONSE,
    SUMO_ATTR_PROGRAMID,
    SUMO_ATTR_OFFSET,
    SUMO_ATTR_ENDOFFSET,
    SUMO_ATTR_INCLANES,
    SUMO_ATTR_INTLANES,
    /// @}

    /// @name the weight of a district's source or sink
    /// @{
    SUMO_ATTR_WEIGHT,
    SUMO_ATTR_NODE,
    SUMO_ATTR_EDGE,
    /// @}

    /// @brief the edges of a route
    SUMO_ATTR_EDGES,

    /// @name vehicle attributes
    /// @{
    SUMO_ATTR_DEPART,
    SUMO_ATTR_DEPARTLANE,
    SUMO_ATTR_DEPARTPOS,
    SUMO_ATTR_DEPARTPOS_LAT,
    SUMO_ATTR_DEPARTSPEED,
    SUMO_ATTR_ARRIVALLANE,
    SUMO_ATTR_ARRIVALPOS,
    SUMO_ATTR_ARRIVALPOS_LAT,
    SUMO_ATTR_ARRIVALSPEED,
    SUMO_ATTR_ROUTE,
    SUMO_ATTR_MAXSPEED,
    SUMO_ATTR_MAXSPEED_LAT,
    SUMO_ATTR_LATALIGNMENT,
    SUMO_ATTR_MINGAP_LAT,
    SUMO_ATTR_ACCEL,
    SUMO_ATTR_DECEL,
    SUMO_ATTR_EMERGENCYDECEL,
    SUMO_ATTR_APPARENTDECEL,
    SUMO_ATTR_ACTIONSTEPLENGTH,
    SUMO_ATTR_VCLASS,
    SUMO_ATTR_VCLASSES,
    SUMO_ATTR_EXCEPTIONS,
    SUMO_ATTR_REPNUMBER,
    SUMO_ATTR_SPEEDFACTOR,
    SUMO_ATTR_SPEEDDEV,
    SUMO_ATTR_LANE_CHANGE_MODEL,
    SUMO_ATTR_CAR_FOLLOW_MODEL,
    SUMO_ATTR_MINGAP,
    SUMO_ATTR_COLLISION_MINGAP_FACTOR,
    SUMO_ATTR_BOARDING_DURATION,
    SUMO_ATTR_LOADING_DURATION,
    /// @brief Class specific timing values for vehicle manoeuvering through angle ranges
    SUMO_ATTR_MANEUVER_ANGLE_TIMES,
    /// @}

    /// @name elecHybrid output attributes
    /// @{
    /// @brief power charging from overhead wire to battery if the battery SoC is not full
    SUMO_ATTR_OVERHEADWIRECHARGINGPOWER,
    /// @}

    /// @name overheadWire attributes
    /// @{
    SUMO_ATTR_OVERHEAD_WIRE_SEGMENT,
    SUMO_ATTR_OVERHEAD_WIRE_SECTION,
    /// @brief voltage of the traction substation [V]
    SUMO_ATTR_VOLTAGE,
    /// @brief a voltage source on the overhead wire segment [bool]
    SUMO_ATTR_VOLTAGESOURCE,
    /// @brief current limit of the traction substation [A]
    SUMO_ATTR_CURRENTLIMIT,
    /// @brief id of a traction substation substation
    SUMO_ATTR_SUBSTATIONID,
    /// @brief resistivity of overhead wires
    SUMO_ATTR_OVERHEAD_WIRE_RESISTIVITY,
    /// @brief forbidden lanes for overhead wire segment
    SUMO_ATTR_OVERHEAD_WIRE_FORBIDDEN,
    /// @brief overhead wire clamps for overhead wire segment
    SUMO_ATTR_OVERHEAD_WIRE_CLAMPS,
    /// @brief id of the overhead wire segment, to the start of which the overhead wire clamp is connected
    SUMO_ATTR_OVERHEAD_WIRE_CLAMP_START,
    /// @brief id of the overhead wire segment, to the end of which the overhead wire clamp is connected
    SUMO_ATTR_OVERHEAD_WIRE_CLAMP_END,
    /// @}

    /// @name charging stations attributes
    /// @{
    /// @brief charge in W/s of the Charging Stations
    SUMO_ATTR_CHARGINGPOWER,
    /// @brief Eficiency of the charge in Charging Stations
    SUMO_ATTR_EFFICIENCY,
    /// @brief Allow/disallow charge in transit in Charging Stations
    SUMO_ATTR_CHARGEINTRANSIT,
    /// @brief Delay in the charge of charging stations
    SUMO_ATTR_CHARGEDELAY,
    /// @}

    /// @name battery device parameters
    /// @{
    /// @brief Actual battery capacity
    SUMO_ATTR_ACTUALBATTERYCAPACITY,
    /// @brief Maxium battery capacity
    SUMO_ATTR_MAXIMUMBATTERYCAPACITY,
    /// @brief Maximum Power
    SUMO_ATTR_MAXIMUMPOWER,
    /// @brief Vehicle mass
    SUMO_ATTR_VEHICLEMASS,
    /// @brief Front surface area
    SUMO_ATTR_FRONTSURFACEAREA,
    /// @brief Air drag coefficient
    SUMO_ATTR_AIRDRAGCOEFFICIENT,
    /// @brief Internal moment of inertia
    SUMO_ATTR_INTERNALMOMENTOFINERTIA,
    /// @brief Radial drag coefficient
    SUMO_ATTR_RADIALDRAGCOEFFICIENT,
    /// @brief Roll Drag coefficient
    SUMO_ATTR_ROLLDRAGCOEFFICIENT,
    /// @brief Constant Power Intake
    SUMO_ATTR_CONSTANTPOWERINTAKE,
    /// @brief Propulsion efficiency
    SUMO_ATTR_PROPULSIONEFFICIENCY,
    /// @brief Recuperation efficiency (constant)
    SUMO_ATTR_RECUPERATIONEFFICIENCY,
    /// @brief Recuperation efficiency (by deceleration)
    SUMO_ATTR_RECUPERATIONEFFICIENCY_BY_DECELERATION,
    /// @brief Stopping treshold
    SUMO_ATTR_STOPPINGTRESHOLD,
    /// @}

    /// @name elecHybrid device export parameters
    /// @{
    // @brief Overhead Wire Segment ID
    SUMO_ATTR_OVERHEADWIREID,
    // @brief Traction substation ID
    SUMO_ATTR_TRACTIONSUBSTATIONID,
    // @brief Electric current drawn from overhead wire
    SUMO_ATTR_CURRENTFROMOVERHEADWIRE,
    // @brief Voltage of overhead wire (above the vehicle)
    SUMO_ATTR_VOLTAGEOFOVERHEADWIRE,
    // @brief Circuit solver alpha parameter
    SUMO_ATTR_ALPHACIRCUITSOLVER,
    /// @}

    /// @name battery export parameters
    /// @{
    /// @brief charging status
    SUMO_ATTR_CHARGING_STATUS,
    /// @brief Energy consumed
    SUMO_ATTR_ENERGYCONSUMED,
    /// @brief Charging Station ID
    SUMO_ATTR_CHARGINGSTATIONID,
    /// @brief tgotal of Energy charged
    SUMO_ATTR_ENERGYCHARGED,
    /// @brief Energy charged in transit
    SUMO_ATTR_ENERGYCHARGEDINTRANSIT,
    /// @brief Energy charged stopped
    SUMO_ATTR_ENERGYCHARGEDSTOPPED,
    /// @brief Position on lane
    SUMO_ATTR_POSONLANE,
    /// @brief Time stopped
    SUMO_ATTR_TIMESTOPPED,
    /// @}

    /// @name chargingStations export parameters
    /// @{
    /// @brief total energy charged by charging station
    SUMO_ATTR_TOTALENERGYCHARGED,
    /// @brief number of steps that a vehicle is charging
    SUMO_ATTR_CHARGINGSTEPS,
    /// @brief total energy charged into a single vehicle
    SUMO_ATTR_TOTALENERGYCHARGED_VEHICLE,
    /// @brief timestep in which charging begins
    SUMO_ATTR_CHARGINGBEGIN,
    /// @brief timesteps in which charging ends
    SUMO_ATTR_CHARGINGEND,
    /// @brief energy provied by charging station at certain timestep
    SUMO_ATTR_PARTIALCHARGE,
    /// @}

    /// @name Car following model attributes
    /// @{
    SUMO_ATTR_SIGMA,    // used by: Krauss
    SUMO_ATTR_TAU,      // Krauss
    SUMO_ATTR_TMP1,
    SUMO_ATTR_TMP2,
    SUMO_ATTR_TMP3,
    SUMO_ATTR_TMP4,
    SUMO_ATTR_TMP5,
    /// @}

    // @name Train ACC model attributes
    /// @{
    SUMO_ATTR_SC_GAIN,
    SUMO_ATTR_GCC_GAIN_SPEED,
    SUMO_ATTR_GCC_GAIN_SPACE,
    SUMO_ATTR_GC_GAIN_SPEED,
    SUMO_ATTR_GC_GAIN_SPACE,
    SUMO_ATTR_CA_GAIN_SPEED,
    SUMO_ATTR_CA_GAIN_SPACE,
    /// @}

    // @name Train CACC model attributes
    /// @{
    SUMO_ATTR_SC_GAIN_CACC,
    SUMO_ATTR_GCC_GAIN_GAP_CACC,
    SUMO_ATTR_GCC_GAIN_GAP_DOT_CACC,
    SUMO_ATTR_GC_GAIN_GAP_CACC,
    SUMO_ATTR_GC_GAIN_GAP_DOT_CACC,
    SUMO_ATTR_CA_GAIN_GAP_CACC,
    SUMO_ATTR_CA_GAIN_GAP_DOT_CACC,
    SUMO_ATTR_HEADWAY_TIME_CACC_TO_ACC,
    /// @}


    /// @name Train model attributes
    /// @{
    SUMO_ATTR_TRAIN_TYPE, //used by: Rail
    /// @}

    /// @name Lane changing model attributes
    /// @{
    SUMO_ATTR_LCA_STRATEGIC_PARAM,
    SUMO_ATTR_LCA_COOPERATIVE_PARAM,
    SUMO_ATTR_LCA_SPEEDGAIN_PARAM,
    SUMO_ATTR_LCA_KEEPRIGHT_PARAM,
    SUMO_ATTR_LCA_SUBLANE_PARAM,
    SUMO_ATTR_LCA_OPPOSITE_PARAM,
    SUMO_ATTR_LCA_PUSHY,
    SUMO_ATTR_LCA_PUSHYGAP,
    SUMO_ATTR_LCA_ASSERTIVE,
    SUMO_ATTR_LCA_IMPATIENCE,
    SUMO_ATTR_LCA_TIME_TO_IMPATIENCE,
    SUMO_ATTR_LCA_ACCEL_LAT,
    SUMO_ATTR_LCA_LOOKAHEADLEFT,
    SUMO_ATTR_LCA_SPEEDGAINRIGHT,
    SUMO_ATTR_LCA_SPEEDGAIN_LOOKAHEAD,
    SUMO_ATTR_LCA_COOPERATIVE_ROUNDABOUT,
    SUMO_ATTR_LCA_COOPERATIVE_SPEED,
    SUMO_ATTR_LCA_MAXSPEEDLATSTANDING,
    SUMO_ATTR_LCA_MAXSPEEDLATFACTOR,
    SUMO_ATTR_LCA_TURN_ALIGNMENT_DISTANCE,
    SUMO_ATTR_LCA_OVERTAKE_RIGHT,
    SUMO_ATTR_LCA_LANE_DISCIPLINE,
    SUMO_ATTR_LCA_SIGMA,
    SUMO_ATTR_LCA_EXPERIMENTAL1,
    /// @}

    /// @name Junction model attributes
    /// @{
    SUMO_ATTR_JM_CROSSING_GAP,
    SUMO_ATTR_JM_DRIVE_AFTER_YELLOW_TIME,
    SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME,
    SUMO_ATTR_JM_DRIVE_RED_SPEED,
    SUMO_ATTR_JM_IGNORE_KEEPCLEAR_TIME,
    SUMO_ATTR_JM_IGNORE_FOE_SPEED,
    SUMO_ATTR_JM_IGNORE_FOE_PROB,
    SUMO_ATTR_JM_SIGMA_MINOR,
    SUMO_ATTR_JM_TIMEGAP_MINOR,
    /// @}

    /// @name route alternatives / distribution attributes
    /// @{
    SUMO_ATTR_LAST,
    SUMO_ATTR_COST,
    SUMO_ATTR_COSTS,
    SUMO_ATTR_SAVINGS,
    SUMO_ATTR_PROB,
    SUMO_ATTR_COUNT,
    SUMO_ATTR_PROBS,
    SUMO_ATTR_ROUTES,
    SUMO_ATTR_VTYPES,
    /// @}

    /// @name trip definition attributes
    /// @{
    SUMO_ATTR_LANE,
    SUMO_ATTR_LANES,
    SUMO_ATTR_FROM,
    SUMO_ATTR_TO,
    SUMO_ATTR_FROMLONLAT,
    SUMO_ATTR_TOLONLAT,
    SUMO_ATTR_FROMXY,
    SUMO_ATTR_TOXY,
    SUMO_ATTR_FROMJUNCTION,
    SUMO_ATTR_TOJUNCTION,
    SUMO_ATTR_PERIOD,
    SUMO_ATTR_REPEAT,
    SUMO_ATTR_FROM_TAZ,
    SUMO_ATTR_TO_TAZ,
    SUMO_ATTR_REROUTE,
    SUMO_ATTR_PERSON_CAPACITY,
    SUMO_ATTR_CONTAINER_CAPACITY,
    SUMO_ATTR_PARKING_LENGTH,
    SUMO_ATTR_PERSON_NUMBER,
    SUMO_ATTR_CONTAINER_NUMBER,
    SUMO_ATTR_MODES,
    SUMO_ATTR_WALKFACTOR,
    /// @}

    /// @name source definitions
    /// @{
    SUMO_ATTR_FUNCTION,
    SUMO_ATTR_POSITION,
    SUMO_ATTR_POSITION_LAT,
    SUMO_ATTR_FREQUENCY,
    SUMO_ATTR_STYLE,
    SUMO_ATTR_FILE,
    SUMO_ATTR_JUNCTION,
    SUMO_ATTR_NUMBER,
    SUMO_ATTR_DURATION,
    SUMO_ATTR_UNTIL,
    SUMO_ATTR_ARRIVAL,
    SUMO_ATTR_EXTENSION,
    SUMO_ATTR_ROUTEPROBE,
    /// @}

    /// @brief the edges crossed by a pedestrian crossing
    SUMO_ATTR_CROSSING_EDGES,
    /// @brief trigger: the time of the step
    SUMO_ATTR_TIME,
    /// @brief weights: time range begin
    SUMO_ATTR_BEGIN,
    /// @brief weights: time range end
    SUMO_ATTR_END,
    /// @brief link,node: the traffic light id responsible for this link
    SUMO_ATTR_TLID,
    /// @brief node: the type of traffic light
    SUMO_ATTR_TLTYPE,
    /// @brief link: the index of the link within the traffic light
    SUMO_ATTR_TLLINKINDEX,
    /// @brief link: the index of the opposite direction link of a pedestrian crossing
    SUMO_ATTR_TLLINKINDEX2,
    /// @brief edge: the shape in xml-definition
    SUMO_ATTR_SHAPE,
    /// @brief The information about how to spread the lanes from the given position
    SUMO_ATTR_SPREADTYPE,
    /// @brief The turning radius at an intersection in m
    SUMO_ATTR_RADIUS,
    /// @brief Whether vehicles must keep the junction clear
    SUMO_ATTR_KEEP_CLEAR,
    /// @brief How to compute right of way
    SUMO_ATTR_RIGHT_OF_WAY,
    /// @brief Fringe type of node
    SUMO_ATTR_FRINGE,
    /// @brief whether a given shape is user-defined
    SUMO_ATTR_CUSTOMSHAPE,
    /// @brief A color information
    SUMO_ATTR_COLOR,
    /// @brief The abstract direction of a link
    SUMO_ATTR_DIR,
    /// @brief The state of a link
    SUMO_ATTR_STATE,
    /// @brief foe visibility distance of a link
    SUMO_ATTR_VISIBILITY_DISTANCE,
    /// @brief A layer number
    SUMO_ATTR_LAYER,
    /// @brief Fill the polygon
    SUMO_ATTR_FILL,
    SUMO_ATTR_LINEWIDTH,
    SUMO_ATTR_PREFIX,
    SUMO_ATTR_DISCARD,

    SUMO_ATTR_FROM_LANE,
    SUMO_ATTR_TO_LANE,
    SUMO_ATTR_DEST,
    SUMO_ATTR_SOURCE,
    SUMO_ATTR_VIA,
    SUMO_ATTR_VIALONLAT,
    SUMO_ATTR_VIAXY,
    SUMO_ATTR_VIAJUNCTIONS,
    /// @brief a list of node ids, used for controlling joining
    SUMO_ATTR_NODES,

    /// @name Attributes for actuated traffic lights:
    /// @{
    /// @brief minimum duration of a phase
    SUMO_ATTR_MINDURATION,
    /// @brief maximum duration of a phase
    SUMO_ATTR_MAXDURATION,
    /// @brief succesor phase index
    SUMO_ATTR_NEXT,
    /// @}

    /// @name Attributes for junction-internal lanes
    /// @{
    /// @brief Information within the junction logic which internal lanes block external
    SUMO_ATTR_FOES,
    /// @}

    /// @name Attributes for detectors
    /// @{
    /// @brief Information whether the detector shall be continued on the folowing lanes
    SUMO_ATTR_CONT,
    SUMO_ATTR_CONTPOS,
    SUMO_ATTR_HALTING_TIME_THRESHOLD,
    SUMO_ATTR_HALTING_SPEED_THRESHOLD,
    SUMO_ATTR_JAM_DIST_THRESHOLD,
    SUMO_ATTR_SHOW_DETECTOR,
    SUMO_ATTR_OPEN_ENTRY,
    /// @}

    SUMO_ATTR_WAUT_ID,
    SUMO_ATTR_JUNCTION_ID,
    SUMO_ATTR_PROCEDURE,
    SUMO_ATTR_SYNCHRON,
    SUMO_ATTR_REF_TIME,
    SUMO_ATTR_START_PROG,

    SUMO_ATTR_OFF,
    SUMO_ATTR_FRIENDLY_POS,
    SUMO_ATTR_SPLIT_VTYPE,
    SUMO_ATTR_UNCONTROLLED,
    SUMO_ATTR_PASS,
    SUMO_ATTR_BUS_STOP,
    SUMO_ATTR_CONTAINER_STOP,
    SUMO_ATTR_PARKING_AREA,
    SUMO_ATTR_ROADSIDE_CAPACITY,
    SUMO_ATTR_ONROAD,
    SUMO_ATTR_CHARGING_STATION,
    SUMO_ATTR_GROUP,
    SUMO_ATTR_LINE,
    SUMO_ATTR_LINES,
    SUMO_ATTR_TRIP_ID,
    SUMO_ATTR_SPLIT,
    SUMO_ATTR_JOIN,
    SUMO_ATTR_INTENDED,
    SUMO_ATTR_VALUE,
    SUMO_ATTR_PROHIBITOR,
    SUMO_ATTR_PROHIBITED,
    SUMO_ATTR_ALLOW,
    SUMO_ATTR_DISALLOW,
    SUMO_ATTR_PREFER,
    SUMO_ATTR_CONTROLLED_INNER,
    SUMO_ATTR_VEHSPERHOUR,
    SUMO_ATTR_PERSONSPERHOUR,
    SUMO_ATTR_OUTPUT,
    SUMO_ATTR_HEIGHT,
    SUMO_ATTR_GUISHAPE,
    SUMO_ATTR_OSGFILE,
    SUMO_ATTR_IMGFILE,
    SUMO_ATTR_RELATIVEPATH,
    SUMO_ATTR_ANGLE,
    SUMO_ATTR_EMISSIONCLASS,
    SUMO_ATTR_IMPATIENCE,
    SUMO_ATTR_STARTPOS,
    SUMO_ATTR_ENDPOS,
    SUMO_ATTR_TRIGGERED,
    SUMO_ATTR_CONTAINER_TRIGGERED,
    SUMO_ATTR_PARKING,
    SUMO_ATTR_EXPECTED,
    SUMO_ATTR_EXPECTED_CONTAINERS,
    SUMO_ATTR_INDEX,

    SUMO_ATTR_ENTERING,
    SUMO_ATTR_EXCLUDE_EMPTY,
    SUMO_ATTR_WITH_INTERNAL,
    SUMO_ATTR_TRACK_VEHICLES,
    SUMO_ATTR_DETECT_PERSONS,
    SUMO_ATTR_MAX_TRAVELTIME,
    SUMO_ATTR_MIN_SAMPLES,
    SUMO_ATTR_WRITE_ATTRIBUTES,

    SUMO_ATTR_LON,
    SUMO_ATTR_LAT,
    SUMO_ATTR_GEO,
    SUMO_ATTR_GEOSHAPE,
    SUMO_ATTR_GEOPOSITION,
    SUMO_ATTR_K,
    SUMO_ATTR_V,
    SUMO_ATTR_REF,
    SUMO_ATTR_HREF,
    SUMO_ATTR_ZOOM,
    SUMO_ATTR_INTERPOLATED,
    SUMO_ATTR_THRESHOLD,

    SUMO_ATTR_NET_OFFSET,
    SUMO_ATTR_CONV_BOUNDARY,
    SUMO_ATTR_ORIG_BOUNDARY,
    SUMO_ATTR_ORIG_PROJ,

    /// @name car-following model attributes
    /// @{
    SUMO_ATTR_CF_PWAGNER2009_TAULAST,
    SUMO_ATTR_CF_PWAGNER2009_APPROB,
    SUMO_ATTR_CF_IDM_DELTA,
    SUMO_ATTR_CF_IDM_STEPPING,
    SUMO_ATTR_CF_IDMM_ADAPT_FACTOR,
    SUMO_ATTR_CF_IDMM_ADAPT_TIME,
    SUMO_ATTR_CF_KERNER_PHI,
    SUMO_ATTR_CF_WIEDEMANN_SECURITY,
    SUMO_ATTR_CF_WIEDEMANN_ESTIMATION,
    SUMO_ATTR_CF_W99_CC1,
    SUMO_ATTR_CF_W99_CC2,
    SUMO_ATTR_CF_W99_CC3,
    SUMO_ATTR_CF_W99_CC4,
    SUMO_ATTR_CF_W99_CC5,
    SUMO_ATTR_CF_W99_CC6,
    SUMO_ATTR_CF_W99_CC7,
    SUMO_ATTR_CF_W99_CC8,
    SUMO_ATTR_CF_W99_CC9,

    SUMO_ATTR_CF_CC_CCDECEL,
    SUMO_ATTR_CF_CC_CONSTSPACING,
    SUMO_ATTR_CF_CC_KP,
    SUMO_ATTR_CF_CC_LAMBDA,
    SUMO_ATTR_CF_CC_C1,
    SUMO_ATTR_CF_CC_XI,
    SUMO_ATTR_CF_CC_OMEGAN,
    SUMO_ATTR_CF_CC_TAU,
    SUMO_ATTR_CF_CC_LANES_COUNT,
    SUMO_ATTR_CF_CC_CCACCEL,
    SUMO_ATTR_CF_CC_PLOEG_KP,
    SUMO_ATTR_CF_CC_PLOEG_KD,
    SUMO_ATTR_CF_CC_PLOEG_H,
    SUMO_ATTR_CF_CC_FLATBED_KA,
    SUMO_ATTR_CF_CC_FLATBED_KV,
    SUMO_ATTR_CF_CC_FLATBED_KP,
    SUMO_ATTR_CF_CC_FLATBED_D,
    SUMO_ATTR_CF_CC_FLATBED_H,
    /// @}

    SUMO_ATTR_GENERATE_WALKS,
    SUMO_ATTR_ACTTYPE,
    SUMO_ATTR_SLOPE,
    SUMO_ATTR_CORNERDETAIL,
    SUMO_ATTR_LINKDETAIL,
    SUMO_ATTR_RECTANGULAR_LANE_CUT,
    SUMO_ATTR_WALKINGAREAS,
    SUMO_ATTR_LEFTHAND,
    SUMO_ATTR_LIMIT_TURN_SPEED,
    SUMO_ATTR_CHECKLANEFOES_ALL,
    SUMO_ATTR_CHECKLANEFOES_ROUNDABOUT,
    SUMO_ATTR_TLS_IGNORE_INTERNAL_JUNCTION_JAM,
    SUMO_ATTR_AVOID_OVERLAP,
    SUMO_ATTR_COMMAND,

    SUMO_ATTR_ACTORCONFIG,
    SUMO_ATTR_VEHICLE,
    SUMO_ATTR_STARTTIME,
    SUMO_ATTR_VEHICLECLASS,
    SUMO_ATTR_FUEL,
    SUMO_ATTR_ACCELERATION,
    SUMO_ATTR_ORIGIN,
    SUMO_ATTR_DESTINATION,
    SUMO_ATTR_VISIBLE,


    /// @name ActivityGen Tags
    /// @{

    /// @name general object
    /// @{
    AGEN_ATTR_INHABITANTS,
    AGEN_ATTR_HOUSEHOLDS,
    AGEN_ATTR_CHILDREN,
    AGEN_ATTR_RETIREMENT,
    AGEN_ATTR_CARS,
    AGEN_ATTR_UNEMPLOYEMENT,
    AGEN_ATTR_LABORDEMAND,
    AGEN_ATTR_MAX_FOOT_DIST,
    AGEN_ATTR_IN_TRAFFIC,
    AGEN_ATTR_OUT_TRAFFIC,
    /// @}

    /// @name streets object
    /// @{
    //SUMO_ATTR_EDGE already defined
    AGEN_ATTR_POPULATION,
    AGEN_ATTR_OUT_WORKPOSITION,
    /// @}

    /// @name workHours object
    /// @{
    AGEN_ATTR_HOUR,
    AGEN_ATTR_PROP,
    /// @}

    /// @name school object
    /// @{
    //SUMO_ATTR_EDGE, SUMO_ATTR_POSITION, SUMO_ATTR_TYPE already defined
    AGEN_ATTR_CAPACITY,
    AGEN_ATTR_OPENING,
    AGEN_ATTR_CLOSING,
    /// @}

    /// @name busStation and Bus objects
    /// @{
    // ID, EDGE, POSITION, REFID, BEGIN and END are already defined
    AGEN_ATTR_MAX_TRIP_DURATION,
    //AGEN_ATTR_ORDER,
    AGEN_ATTR_RATE,
    /// @}

    /// @name population and children accompaniment brackets
    /// @{
    AGEN_ATTR_BEGINAGE,
    AGEN_ATTR_ENDAGE,
    AGEN_ATTR_PEOPLENBR,
    /// @}

    /// @name parameters
    /// @{
    AGEN_ATTR_CARPREF,
    AGEN_ATTR_CITYSPEED,
    AGEN_ATTR_FREETIMERATE,
    AGEN_ATTR_UNI_RAND_TRAFFIC,
    AGEN_ATTR_DEP_VARIATION,
    /// @}

    /// @name city gates
    /// @{
    AGEN_ATTR_INCOMING,
    AGEN_ATTR_OUTGOING,
    /// @}
    /// @}

    //@name Netedit Attributes (used as virtual property holders, must be in SumoXMLAttr)
    //@{
    /// @brief element is selected
    GNE_ATTR_SELECTED,
    /// @brief whether a feature has been loaded,guessed,modified or approved
    GNE_ATTR_MODIFICATION_STATUS,
    /// @brief first coordinate of edge shape
    GNE_ATTR_SHAPE_START,
    /// @brief last coordinate of edge shape
    GNE_ATTR_SHAPE_END,
    /// @brief whether an edge is part of a bidirectional railway
    GNE_ATTR_BIDIR,
    /// @brief block movement of a graphic element
    GNE_ATTR_BLOCK_MOVEMENT,
    /// @brief block shape of a graphic element (Used mainly in GNEShapes)
    GNE_ATTR_BLOCK_SHAPE,
    /// @brief Close shape of a polygon (Used by GNEPolys)
    GNE_ATTR_CLOSE_SHAPE,
    /// @brief parent of an additional element
    GNE_ATTR_PARENT,
    /// @brief data set of a generic data
    GNE_ATTR_DATASET,
    /// @brief parameters "key1=value1|key2=value2|...|keyN=valueN"
    GNE_ATTR_PARAMETERS,
    /// @brief min source (used only by TAZs)
    GNE_ATTR_MIN_SOURCE,
    /// @brief min sink (used only by TAZs)
    GNE_ATTR_MIN_SINK,
    /// @brief max source (used only by TAZs)
    GNE_ATTR_MAX_SOURCE,
    /// @brief max sink (used only by TAZs)
    GNE_ATTR_MAX_SINK,
    /// @brief average source (used only by TAZs)
    GNE_ATTR_AVERAGE_SOURCE,
    /// @brief average sink (used only by TAZs)
    GNE_ATTR_AVERAGE_SINK,
    /// @brief Color of TAZSources/TAZSinks
    GNE_ATTR_TAZCOLOR,
    /// @brief Flag to check if VType is a default VType
    GNE_ATTR_DEFAULT_VTYPE,
    /// @brief Flag to check if a default VType was modified
    GNE_ATTR_DEFAULT_VTYPE_MODIFIED,
    /// @brief flag to center camera after element creation
    GNE_ATTR_CENTER_AFTER_CREATION,
    /// @brief from busStop (used by personPlans)
    GNE_ATTR_FROM_BUSSTOP,
    /// @brief to busStop (used by personPlans)
    GNE_ATTR_TO_BUSSTOP,

    // @}

    /// @name train parameters
    /// @{
    SUMO_ATTR_CARRIAGE_LENGTH,
    SUMO_ATTR_LOCOMOTIVE_LENGTH,
    SUMO_ATTR_CARRIAGE_GAP,
    /// @}

    SUMO_ATTR_TARGETLANE,
    SUMO_ATTR_CROSSING,
    SUMO_ATTR_XMLNS,
    SUMO_ATTR_SCHEMA_LOCATION,

    //@name RNG state saving attributes
    // @{
    SUMO_ATTR_RNG_DEFAULT,
    SUMO_ATTR_RNG_ROUTEHANDLER,
    SUMO_ATTR_RNG_INSERTIONCONTROL,
    SUMO_ATTR_RNG_DEVICE,
    SUMO_ATTR_RNG_DEVICE_BT,
    SUMO_ATTR_RNG_DEVICE_TOC,
    SUMO_ATTR_RNG_DRIVERSTATE
    // @}

};

/*
 * @brief definitions of special SumoXML-attribute values.
 * Since these enums shall be used in switch statements we keep them separated
 * @{
 */

/**
 * SumoXMLParam Key Constants. Since all usage contexts needs strings rather
 * than enum values we do not bother with a StringBijection
 */
extern const std::string SUMO_PARAM_ORIGID;

/**
 * @enum SumoXMLNodeType
 * @brief Numbers representing special SUMO-XML-attribute values
 * for representing node- (junction-) types used in netbuild/netimport and netload
 */
enum class SumoXMLNodeType {
    UNKNOWN, // terminator
    TRAFFIC_LIGHT,
    TRAFFIC_LIGHT_NOJUNCTION, // junction controlled only by traffic light but without other prohibitions,
    TRAFFIC_LIGHT_RIGHT_ON_RED,
    RAIL_SIGNAL,
    RAIL_CROSSING,
    PRIORITY,
    PRIORITY_STOP, // like priority but all minor links have stop signs
    RIGHT_BEFORE_LEFT,
    ALLWAY_STOP,
    ZIPPER,
    DISTRICT,
    NOJUNCTION,
    INTERNAL,
    DEAD_END,
    DEAD_END_DEPRECATED
};


/**
 * @enum SumoXMLEdgeFunc
 * @brief Numbers representing special SUMO-XML-attribute values
 * for representing edge functions used in netbuild/netimport and netload
 */
enum class SumoXMLEdgeFunc {
    UNKNOWN,
    NORMAL,
    CONNECTOR,
    CROSSING,
    WALKINGAREA,
    INTERNAL
};


/**
 * @enum LaneSpreadFunction
 * @brief Numbers representing special SUMO-XML-attribute values
 * Information how the edge's lateral offset shall be computed
 * In dependence to this value, lanes will be spread to the right side or
 * to both sides from the given edge geometry (Also used when node
 * positions are used as edge geometry).
 */
enum class LaneSpreadFunction {
    RIGHT = 0,
    ROADCENTER = 1,
    CENTER = 2
};


/// @brief algorithms for computing right of way
enum class RightOfWay {
    DEFAULT,
    EDGEPRIORITY
};

/// @brief algorithms for computing right of way
enum class FringeType {
    OUTER,
    INNER,
    DEFAULT
};

/// @brief travel modes for persons
enum class PersonMode {
    NONE = 0,
    WALK = 1 << 1,
    BICYCLE = 1 << 2,
    CAR = 1 << 3,
    PUBLIC = 1 << 4,
    TAXI = 1 << 5
};

/**
 * @enum LinkState
 * @brief The right-of-way state of a link between two lanes
 * used when constructing a NBTrafficLightLogic, in MSLink and GNEInternalLane
 *
 * This enumerations holds the possible right-of-way rules a link
 *  may have. Beyond the righ-of-way rules, this enumeration also
 *  holds the possible traffic light states.
 *
 *  enum values are assigned so that chars can be cast back to linkstates
 *  @todo fix redundancy
 */
enum LinkState {
    /// @brief The link has green light, may pass
    LINKSTATE_TL_GREEN_MAJOR = 'G',
    /// @brief The link has green light, has to brake
    LINKSTATE_TL_GREEN_MINOR = 'g',
    /// @brief The link has red light (must brake)
    LINKSTATE_TL_RED = 'r',
    /// @brief The link has red light (must brake) but indicates upcoming green
    LINKSTATE_TL_REDYELLOW = 'u',
    /// @brief The link has yellow light, may pass
    LINKSTATE_TL_YELLOW_MAJOR = 'Y',
    /// @brief The link has yellow light, has to brake anyway
    LINKSTATE_TL_YELLOW_MINOR = 'y',
    /// @brief The link is controlled by a tls which is off and blinks, has to brake
    LINKSTATE_TL_OFF_BLINKING = 'o',
    /// @brief The link is controlled by a tls which is off, not blinking, may pass
    LINKSTATE_TL_OFF_NOSIGNAL = 'O',
    /// @brief This is an uncontrolled, major link, may pass
    LINKSTATE_MAJOR = 'M',
    /// @brief This is an uncontrolled, minor link, has to brake
    LINKSTATE_MINOR = 'm',
    /// @brief This is an uncontrolled, right-before-left link
    LINKSTATE_EQUAL = '=',
    /// @brief This is an uncontrolled, minor link, has to stop
    LINKSTATE_STOP = 's',
    /// @brief This is an uncontrolled, all-way stop link.
    LINKSTATE_ALLWAY_STOP = 'w',
    /// @brief This is an uncontrolled, zipper-merge link
    LINKSTATE_ZIPPER = 'Z',
    /// @brief This is a dead end link
    LINKSTATE_DEADEND = '-'
};

/**
 * @enum LinkDirection
 * @brief The different directions a link between two lanes may take (or a
 * stream between two edges).
 * used in netbuild (formerly NBMMLDirection) and MSLink
 */
enum class LinkDirection {
    /// @brief The link is a straight direction
    STRAIGHT = 0,
    /// @brief The link is a 180 degree turn
    TURN,
    /// @brief The link is a 180 degree turn (left-hand network)
    TURN_LEFTHAND,
    /// @brief The link is a (hard) left direction
    LEFT,
    /// @brief The link is a (hard) right direction
    RIGHT,
    /// @brief The link is a partial left direction
    PARTLEFT,
    /// @brief The link is a partial right direction
    PARTRIGHT,
    /// @brief The link has no direction (is a dead end link)
    NODIR
};


/// @enum TrafficLightType
enum class TrafficLightType {
    STATIC,
    RAIL_SIGNAL,
    RAIL_CROSSING,
    ACTUATED,
    DELAYBASED,
    SOTL_PHASE,
    SOTL_PLATOON,
    SOTL_REQUEST,
    SOTL_WAVE,
    SOTL_MARCHING,
    SWARM_BASED,
    HILVL_DETERMINISTIC,
    OFF,
    INVALID //< must be the last one
};


/** @enum LaneChangeAction
 * @brief The state of a vehicle's lane-change behavior
 */
enum LaneChangeAction {
    /// @name currently wanted lane-change action
    /// @{
    /// @brief No action desired
    LCA_NONE = 0,
    /// @brief Needs to stay on the current lane
    LCA_STAY = 1 << 0,
    /// @brief Wants go to the left
    LCA_LEFT = 1 << 1,
    /// @brief Wants go to the right
    LCA_RIGHT = 1 << 2,
    /// @brief The action is needed to follow the route (navigational lc)
    LCA_STRATEGIC = 1 << 3,
    /// @brief The action is done to help someone else
    LCA_COOPERATIVE = 1 << 4,
    /// @brief The action is due to the wish to be faster (tactical lc)
    LCA_SPEEDGAIN = 1 << 5,
    /// @brief The action is due to the default of keeping right "Rechtsfahrgebot"
    LCA_KEEPRIGHT = 1 << 6,
    /// @brief The action is due to a TraCI request
    LCA_TRACI = 1 << 7,
    /// @brief The action is urgent (to be defined by lc-model)
    LCA_URGENT = 1 << 8,
    /// @brief The action has not been determined
    LCA_UNKNOWN = 1 << 30,
    /// @}

    /// @name External state
    /// @{
    /// @brief The vehicle is blocked by left leader
    LCA_BLOCKED_BY_LEFT_LEADER = 1 << 9,
    /// @brief The vehicle is blocked by left follower
    LCA_BLOCKED_BY_LEFT_FOLLOWER = 1 << 10,
    /// @brief The vehicle is blocked by right leader
    LCA_BLOCKED_BY_RIGHT_LEADER = 1 << 11,
    /// @brief The vehicle is blocked by right follower
    LCA_BLOCKED_BY_RIGHT_FOLLOWER = 1 << 12,
    /// @brief The vehicle is blocked being overlapping
    LCA_OVERLAPPING = 1 << 13,
    /// @brief The vehicle does not have enough space to complete a continuous change before the next turn
    LCA_INSUFFICIENT_SPACE = 1 << 14,
    /// @brief used by the sublane model
    LCA_SUBLANE = 1 << 15,
    /// @brief Vehicle is too slow to complete a continuous lane change (in case that maxSpeedLatStanding==0)
    LCA_INSUFFICIENT_SPEED = 1 << 28,
    /// @brief lane can change
    LCA_WANTS_LANECHANGE = LCA_LEFT | LCA_RIGHT,
    /// @brief lane can change or stay
    LCA_WANTS_LANECHANGE_OR_STAY = LCA_WANTS_LANECHANGE | LCA_STAY,
    /// @brief blocked left
    LCA_BLOCKED_LEFT = LCA_BLOCKED_BY_LEFT_LEADER | LCA_BLOCKED_BY_LEFT_FOLLOWER,
    /// @brief blocked right
    LCA_BLOCKED_RIGHT = LCA_BLOCKED_BY_RIGHT_LEADER | LCA_BLOCKED_BY_RIGHT_FOLLOWER,
    /// @brief blocked by leader
    LCA_BLOCKED_BY_LEADER = LCA_BLOCKED_BY_LEFT_LEADER | LCA_BLOCKED_BY_RIGHT_LEADER,
    /// @brief blocker by follower
    LCA_BLOCKED_BY_FOLLOWER = LCA_BLOCKED_BY_LEFT_FOLLOWER | LCA_BLOCKED_BY_RIGHT_FOLLOWER,
    /// @brief blocked in all directions
    LCA_BLOCKED = LCA_BLOCKED_LEFT | LCA_BLOCKED_RIGHT | LCA_INSUFFICIENT_SPACE | LCA_INSUFFICIENT_SPEED,
    /// @brief reasons of lane change
    LCA_CHANGE_REASONS = (LCA_STRATEGIC | LCA_COOPERATIVE | LCA_SPEEDGAIN | LCA_KEEPRIGHT | LCA_SUBLANE | LCA_TRACI),
    // LCA_BLOCKED_BY_CURRENT_LEADER = 1 << 28
    // LCA_BLOCKED_BY_CURRENT_FOLLOWER = 1 << 29
    /// @}

    /// @name originally model specific states (migrated here since
    ///       they were duplicated in all current models)
    /// @{
    LCA_AMBLOCKINGLEADER = 1 << 16,
    LCA_AMBLOCKINGFOLLOWER = 1 << 17,
    LCA_MRIGHT = 1 << 18,
    LCA_MLEFT = 1 << 19,
    // !!! never set LCA_UNBLOCK = 1 << 20,
    LCA_AMBLOCKINGFOLLOWER_DONTBRAKE = 1 << 21,
    // !!! never used LCA_AMBLOCKINGSECONDFOLLOWER = 1 << 22,
    LCA_CHANGE_TO_HELP = 1 << 23,
    // !!! never read LCA_KEEP1 = 1 << 24,
    // !!! never used LCA_KEEP2 = 1 << 25,
    LCA_AMBACKBLOCKER = 1 << 26,
    LCA_AMBACKBLOCKER_STANDING = 1 << 27
                                 /// @}
};


/// @enum LaneChangeModel
enum LaneChangeModel {
    LCM_DK2008,
    LCM_LC2013,
    LCM_SL2015,
    LCM_DEFAULT
};

/// @enum train types
enum TrainType {
    TRAINTYPE_NGT400,
    TRAINTYPE_NGT400_16,
    TRAINTYPE_RB425,
    TRAINTYPE_RB628,
    TRAINTYPE_ICE1,
    TRAINTYPE_REDOSTO7,
    TRAINTYPE_FREIGHT,
    TRAINTYPE_ICE3,
    TRAINTYPE_UNKNOWN
};

/**
 * @enum LateralAlignment
 * @brief Numbers representing special SUMO-XML-attribute values
 * Information how vehicles align themselves within their lane by default
 */
enum LateralAlignment {
    /// @brief drive on the right side
    LATALIGN_RIGHT,
    /// @brief drive in the middle
    LATALIGN_CENTER,
    /// @brief maintain the current alignment
    LATALIGN_ARBITRARY,
    /// @brief align with the closest sublane border
    LATALIGN_NICE,
    /// @brief align with the rightmost sublane that allows keeping the current speed
    LATALIGN_COMPACT,
    /// @brief drive on the left side
    LATALIGN_LEFT
};

// @}

/**
 * @class SUMOXMLDefinitions
 * @brief class for maintaining associations between enums and xml-strings
 */
class SUMOXMLDefinitions {

public:
    /// @brief The names of SUMO-XML elements (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry tags[];

    /// @brief The names of SUMO-XML attributes (for passing to GenericSAXHandler)
    static StringBijection<int>::Entry attrs[];

    /// @brief The names of SUMO-XML elements for use in netbuild
    static StringBijection<int> Tags;

    /// @brief The names of SUMO-XML attributes for use in netbuild
    static StringBijection<int> Attrs;

    /// @name Special values of SUMO-XML attributes
    /// @{

    /// @brief node types
    static StringBijection<SumoXMLNodeType> NodeTypes;

    /// @brief edge functions
    static StringBijection<SumoXMLEdgeFunc> EdgeFunctions;

    /// @brief lane spread functions
    static StringBijection<LaneSpreadFunction> LaneSpreadFunctions;

    /// @brief righ of way algorithms
    static StringBijection<RightOfWay> RightOfWayValues;

    /// @brief fringe types
    static StringBijection<FringeType> FringeTypeValues;

    /// @brief person modes
    static StringBijection<PersonMode> PersonModeValues;

    /// @brief link states
    static StringBijection<LinkState> LinkStates;

    /// @brief link directions
    static StringBijection<LinkDirection> LinkDirections;

    /// @brief traffic light types
    static StringBijection<TrafficLightType> TrafficLightTypes;

    /// @brief lane change models
    static StringBijection<LaneChangeModel> LaneChangeModels;

    /// @brief car following models
    static StringBijection<SumoXMLTag> CarFollowModels;

    /// @brief lateral alignments
    static StringBijection<LateralAlignment> LateralAlignments;

    /// @brief lane change actions
    static StringBijection<LaneChangeAction> LaneChangeActions;

    /// @brief train types
    static StringBijection<TrainType> TrainTypes;
    /// @}

    /// @name Helper functions for ID-string manipulations
    /// @{

    /// @brief whether the given string is a valid id for a network element
    static bool isValidNetID(const std::string& value);

    /// @brief whether the given string is a valid id for a vehicle or flow
    static bool isValidVehicleID(const std::string& value);

    /// @brief whether the given string is a valid id for an edge or vehicle type
    static bool isValidTypeID(const std::string& value);

    /// @brief whether the given string is a valid id for an additional object
    static bool isValidAdditionalID(const std::string& value);

    /// @brief whether the given string is a valid id for an detector
    static bool isValidDetectorID(const std::string& value);

    /// @brief whether the given string is a valid attribute for a certain key (for example, a name)
    static bool isValidAttribute(const std::string& value);

    /// @brief whether the given string is a valid attribute for a filename (for example, a name)
    static bool isValidFilename(const std::string& value);

    /// @brief whether the given string is a valid list of id for a network (empty aren't allowed)
    static bool isValidListOfNetIDs(const std::string& value);

    /// @brief whether the given string is a valid list of ids for an edge or vehicle type (empty aren't allowed)
    static bool isValidListOfTypeID(const std::string& value);

    /// @brief whether the given string is a valid key for a parameter
    static bool isValidParameterKey(const std::string& value);

    /// @brief whether the given string is a valid value for a parameter
    static bool isValidParameterValue(const std::string& value);

    /// @brief return the junction id when given an edge of type internal, crossing or WalkingArea
    static std::string getJunctionIDFromInternalEdge(const std::string internalEdge);

    /// @brief return edge id when given the lane ID
    static std::string getEdgeIDFromLane(const std::string laneID);
    /// @}

    /// @brief all allowed characters for phase state
    static const std::string ALLOWED_TLS_LINKSTATES;



private:
    /// @brief containers for the different SUMOXMLDefinitions
    /// @{

    /// @brief node type values
    static StringBijection<SumoXMLNodeType>::Entry sumoNodeTypeValues[];

    /// @brief edge function values
    static StringBijection<SumoXMLEdgeFunc>::Entry sumoEdgeFuncValues[];

    /// @brief lane spread function values
    static StringBijection<LaneSpreadFunction>::Entry laneSpreadFunctionValues[];

    /// @brief lane spread function values
    static StringBijection<RightOfWay>::Entry rightOfWayValuesInitializer[];

    /// @brief lane spread function values
    static StringBijection<FringeType>::Entry fringeTypeValuesInitializer[];

    /// @brief person mode values
    static StringBijection<PersonMode>::Entry personModeValuesInitializer[];

    /// @brief link state values
    static StringBijection<LinkState>::Entry linkStateValues[];

    /// @brief link direction values
    static StringBijection<LinkDirection>::Entry linkDirectionValues[];

    /// @brief traffic light types values
    static StringBijection<TrafficLightType>::Entry trafficLightTypesValues[];

    /// @brief lane change model values
    static StringBijection<LaneChangeModel>::Entry laneChangeModelValues[];

    /// @brief car follwing model values
    static StringBijection<SumoXMLTag>::Entry carFollowModelValues[];

    /// @brief lanteral alignment values
    static StringBijection<LateralAlignment>::Entry lateralAlignmentValues[];

    /// @brief lane change action values
    static StringBijection<LaneChangeAction>::Entry laneChangeActionValues[];

    /// @brief train type values values
    static StringBijection<TrainType>::Entry trainTypeValues[];
    /// @}

    /// @brief all allowed characters for phase state
    static const char AllowedTLS_linkStatesInitializer[];
};
