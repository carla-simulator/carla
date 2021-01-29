/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUIIcons.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Andreas Gaubatz
/// @date    2004
///
// An enumeration of icons used by the gui applications
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// enumerations
// ===========================================================================
/**
 * @enum GUIIcon
 * @brief An enumeration of icons used by the gui applications
 */
enum class GUIIcon {
    /// @name general Interface icons
    /// @{
    SUMO,
    SUMO_MINI,
    SUMO_LOGO,
    EMPTY,
    OPEN_CONFIG,
    OPEN_NET,
    OPEN_SHAPES,
    OPEN_ADDITIONALS,
    OPEN_TLSPROGRAMS,
    RELOAD,
    SAVE,
    CLOSE,
    HELP,
    /// @}

    /// @name simulation control icons
    /// @{
    START,
    STOP,
    STEP,
    /// @}

    /// @name simulation view icons
    /// @{
    MICROVIEW,
    OSGVIEW,
    /// @}

    /// @name simulation view icons (other)
    /// @{
    RECENTERVIEW,
    ALLOWROTATION,
    /// @}

    /// @name locate objects icons
    /// @{
    LOCATE,
    LOCATEJUNCTION,
    LOCATEEDGE,
    LOCATEVEHICLE,
    LOCATEROUTE,
    LOCATESTOP,
    LOCATEPERSON,
    LOCATECONTAINER,
    LOCATETLS,
    LOCATEADD,
    LOCATEPOI,
    LOCATEPOLY,
    /// @}

    /// @name green and yellow objects icons
    /// @{
    GREENEDGE,
    GREENVEHICLE,
    GREENPERSON,
    YELLOWEDGE,
    YELLOWVEHICLE,
    YELLOWPERSON,
    /// @}

    /// @name options icons
    /// @{
    COLORWHEEL,
    SAVEDB,
    REMOVEDB,
    SHOWTOOLTIPS,
    EDITVIEWPORT,
    ZOOMSTYLE,
    /// @}

    /// @name app icons
    /// @{
    APP_TRACKER,
    APP_FINDER,
    APP_BREAKPOINTS,
    APP_TLSTRACKER,
    APP_TABLE,
    APP_SELECTOR,
    /// @}

    /// @name decision icons
    /// @{
    YES,
    NO,
    /// @}

    /// @name flags icons
    /// @{
    FLAG,
    FLAG_PLUS,
    FLAG_MINUS,
    /// @}

    /// @name windows icosn
    /// @{
    WINDOWS_CASCADE,
    WINDOWS_TILE_VERT,
    WINDOWS_TILE_HORI,
    /// @}

    /// @name manipulation icons
    /// @{
    MANIP,
    CAMERA,
    /// @}

    /// @name graph icons
    /// @{
    EXTRACT,
    DILATE,
    ERODE,
    OPENING,
    CLOSING,
    CLOSE_GAPS,
    ERASE_STAINS,
    SKELETONIZE,
    RARIFY,
    CREATE_GRAPH,
    OPEN_BMP_DIALOG,
    EYEDROP,
    PAINTBRUSH1X,
    PAINTBRUSH2X,
    PAINTBRUSH3X,
    PAINTBRUSH4X,
    PAINTBRUSH5X,
    RUBBER1X,
    RUBBER2X,
    RUBBER3X,
    RUBBER4X,
    RUBBER5X,
    EDITGRAPH,
    /// @}

    /// @name other tools
    /// @{
    EXT,
    CUT_SWELL,
    TRACKER,
    /// @}

    /// @name net edit icons
    /// @{
    UNDO,
    REDO,
    NETEDIT,
    NETEDIT_MINI,
    LOCK,
    ADD,
    REMOVE,
    BIGARROWLEFT,
    BIGARROWRIGHT,
    FRONTELEMENT,
    /// @}

    /// @name arrows
    /// @{
    ARROW_UP,
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    /// @}

    /// @name lane icons
    /// @{
    LANEPEDESTRIAN,
    LANEBUS,
    LANEBIKE,
    LANEGREENVERGE,
    /// @}

    /// @name netedit save elements
    /// @{
    SAVENETWORKELEMENTS,
    SAVEADDITIONALELEMENTS,
    SAVEDEMANDELEMENTS,
    SAVEDATAELEMENTS,
    /// @}

    /// @name netedit supermode icons
    /// @{
    SUPERMODENETWORK,
    SUPERMODEDEMAND,
    SUPERMODEDATA,
    /// @}

    /// @name NETEDIT Network modes icons
    /// @{
    MODEADDITIONAL,
    MODECONNECTION,
    MODECREATEEDGE,
    MODECROSSING,
    MODETAZ,
    MODEDELETE,
    MODEINSPECT,
    MODEMOVE,
    MODESELECT,
    MODETLS,
    MODEPOLYGON,
    MODEPROHIBITION,
    /// @}

    /// @name NETEDIT Demand modes icons
    /// @{
    MODEROUTE,
    MODEVEHICLE,
    MODEVEHICLETYPE,
    MODESTOP,
    MODEPERSONTYPE,
    MODEPERSON,
    MODEPERSONPLAN,
    /// @}

    /// @name NETEDIT Edge modes icons
    /// @{
    MODEEDGEDATA,
    MODEEDGERELDATA,
    MODETAZRELDATA,
    /// @}

    /// @name NETEDIT processing icons
    /// @{
    COMPUTEJUNCTIONS,
    CLEANJUNCTIONS,
    JOINJUNCTIONS,
    COMPUTEDEMAND,
    CLEANROUTES,
    JOINROUTES,
    OPTIONS,
    /// @}

    /// @name NETEDIT network elements icons
    /// @{
    JUNCTION,
    EDGE,
    LANE,
    CONNECTION,
    PROHIBITION,
    CROSSING,
    /// @}

    /// @name NETEDIT additional elements icons
    /// @{
    BUSSTOP,
    ACCESS,
    CONTAINERSTOP,
    CHARGINGSTATION,
    E1,
    E2,
    E3,
    E3ENTRY,
    E3EXIT,
    E1INSTANT,
    REROUTER,
    ROUTEPROBE,
    VAPORIZER,
    VARIABLESPEEDSIGN,
    CALIBRATOR,
    PARKINGAREA,
    PARKINGSPACE,
    REROUTERINTERVAL,
    VSSSTEP,
    CLOSINGREROUTE,
    CLOSINGLANEREROUTE,
    DESTPROBREROUTE,
    PARKINGZONEREROUTE,
    ROUTEPROBREROUTE,
    TAZ,
    TAZEDGE,
    /// @}

    /// @name NETEDIT Demand elements icons
    /// @{
    ROUTE,
    VTYPE,
    PTYPE,
    VEHICLE,
    TRIP,
    FLOW,
    ROUTEFLOW,
    STOPELEMENT,
    PERSON,
    PERSONFLOW,
    PERSONTRIP_FROMTO,
    PERSONTRIP_BUSSTOP,
    WALK_EDGES,
    WALK_FROMTO,
    WALK_BUSSTOP,
    WALK_ROUTE,
    RIDE_FROMTO,
    RIDE_BUSSTOP,
    PERSON_BUSSTOP,
    PERSON_CONTAINERSTOP,
    PERSON_LANESTOP,
    /// @}

    /// @name NETEDIT data elements icons
    /// @{
    DATASET,
    DATAINTERVAL,
    EDGEDATA,
    EDGERELDATA,
    TAZRELDATA,
    /// @}

    /// @name vehicle Class icons
    /// @{
    VCLASS_IGNORING,
    VCLASS_PRIVATE,
    VCLASS_EMERGENCY,
    VCLASS_AUTHORITY,
    VCLASS_ARMY,
    VCLASS_VIP,
    VCLASS_PASSENGER,
    VCLASS_HOV,
    VCLASS_TAXI,
    VCLASS_BUS,
    VCLASS_COACH,
    VCLASS_DELIVERY,
    VCLASS_TRUCK,
    VCLASS_TRAILER,
    VCLASS_TRAM,
    VCLASS_RAIL_URBAN,
    VCLASS_RAIL,
    VCLASS_RAIL_ELECTRIC,
    VCLASS_MOTORCYCLE,
    VCLASS_MOPED,
    VCLASS_BICYCLE,
    VCLASS_PEDESTRIAN,
    VCLASS_EVEHICLE,
    VCLASS_SHIP,
    VCLASS_CUSTOM1,
    VCLASS_CUSTOM2,
    /// @}

    /// @name vehicle Shape icons
    /// @{
    VSHAPE_PEDESTRIAN,
    VSHAPE_BICYCLE,
    VSHAPE_MOPED,
    VSHAPE_MOTORCYCLE,
    VSHAPE_PASSENGER,
    VSHAPE_PASSENGER_SEDAN,
    VSHAPE_PASSENGER_HATCHBACK,
    VSHAPE_PASSENGER_WAGON,
    VSHAPE_PASSENGER_VAN,
    VSHAPE_DELIVERY,
    VSHAPE_TRUCK,
    VSHAPE_TRUCK_SEMITRAILER,
    VSHAPE_TRUCK_1TRAILER,
    VSHAPE_BUS,
    VSHAPE_BUS_COACH,
    VSHAPE_BUS_FLEXIBLE,
    VSHAPE_BUS_TROLLEY,
    VSHAPE_RAIL,
    VSHAPE_RAIL_CAR,
    VSHAPE_RAIL_CARGO,
    VSHAPE_E_VEHICLE,
    VSHAPE_ANT,
    VSHAPE_SHIP,
    VSHAPE_EMERGENCY,
    VSHAPE_FIREBRIGADE,
    VSHAPE_POLICE,
    VSHAPE_RICKSHAW,
    VSHAPE_UNKNOWN,
    /// @}

    /// @name icons for status
    /// @{
    OK,
    ACCEPT,
    CANCEL,
    CORRECT,
    INCORRECT,
    RESET,
    WARNING,
    /// @}

    /// @name icons for grid
    /// @{
    GRID1,
    GRID2,
    GRID3,
    /// @}
};
