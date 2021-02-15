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
/// @file    GUIVisualizationSettings.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Stores the information about how to visualize structures
/****************************************************************************/
#include <config.h>

#include <map>
#include <vector>
#include <utils/gui/div/GUIGlobalSelection.h>

#include "GUIVisualizationSettings.h"
#include "GUIPropertyScheme.h"

// ===========================================================================
// static members
// ===========================================================================

bool GUIVisualizationSettings::UseMesoSim = false;

// -------------------------------------------------------------------------
// color constants for link states
// -------------------------------------------------------------------------

const RGBColor SUMO_color_TL_GREEN_MAJOR(0, 255, 0);
const RGBColor SUMO_color_TL_GREEN_MINOR(0, 179, 0);
const RGBColor SUMO_color_TL_RED(255, 0, 0);
const RGBColor SUMO_color_TL_REDYELLOW(255, 128, 0);
const RGBColor SUMO_color_TL_YELLOW_MAJOR(255, 255, 0);
const RGBColor SUMO_color_TL_YELLOW_MINOR(255, 255, 0);
const RGBColor SUMO_color_TL_OFF_BLINKING(128, 64, 0);
const RGBColor SUMO_color_TL_OFF_NOSIGNAL(0, 255, 255);
const RGBColor SUMO_color_MAJOR(255, 255, 255);
const RGBColor SUMO_color_MINOR(51, 51, 51);
const RGBColor SUMO_color_EQUAL(128, 128, 128);
const RGBColor SUMO_color_STOP(128, 0, 128);
const RGBColor SUMO_color_ALLWAY_STOP(0, 0, 192);
const RGBColor SUMO_color_ZIPPER(192, 128, 64);
const RGBColor SUMO_color_DEADEND(0, 0, 0);

// -------------------------------------------------------------------------
// color constants for other objects
// -------------------------------------------------------------------------

const RGBColor GUIVisualizationColorSettings::SUMO_color_DEADEND_SHOW(255, 0, 255);
const RGBColor GUIVisualizationColorSettings::childConnections(255, 235, 0);
const RGBColor GUIVisualizationColorSettings::editShape(0, 200, 0);
const RGBColor GUIVisualizationColorSettings::crossing(25, 25, 25);
const RGBColor GUIVisualizationColorSettings::crossingPriority(229, 229, 229);
const RGBColor GUIVisualizationColorSettings::crossingInvalid(255, 25, 25);
const RGBColor GUIVisualizationColorSettings::stops(220, 20, 30);
const RGBColor GUIVisualizationColorSettings::vehicleTrips(255, 128, 0);
const RGBColor GUIVisualizationColorSettings::personStops(255, 0, 0);
const RGBColor GUIVisualizationColorSettings::personTrip(200, 0, 255);
const RGBColor GUIVisualizationColorSettings::walk(0, 255, 0);
const RGBColor GUIVisualizationColorSettings::ride(0, 0, 255);

// -------------------------------------------------------------------------
// color constants for other objects
// -------------------------------------------------------------------------

const RGBColor GUIVisualizationCandidateColorSettings::possible(0, 64, 0, 255);
const RGBColor GUIVisualizationCandidateColorSettings::source(0, 255, 255, 255);    // Cyan
const RGBColor GUIVisualizationCandidateColorSettings::target(0, 255, 0, 255);      // Green
const RGBColor GUIVisualizationCandidateColorSettings::special(255, 0, 255, 255);   // Magenta
const RGBColor GUIVisualizationCandidateColorSettings::conflict(255, 255, 0, 255);  // Yellow

// -------------------------------------------------------------------------
// Netedit size values
// -------------------------------------------------------------------------

const double GUIVisualizationNeteditSizeSettings::junctionBubbleRadius(4);
const double GUIVisualizationNeteditSizeSettings::junctionGeometryPointRadius(1);
const double GUIVisualizationNeteditSizeSettings::edgeGeometryPointRadius(1.2);
const double GUIVisualizationNeteditSizeSettings::connectionGeometryPointRadius(0.8);
const double GUIVisualizationNeteditSizeSettings::crossingGeometryPointRadius(1);
const double GUIVisualizationNeteditSizeSettings::polygonGeometryPointRadius(1.2);
const double GUIVisualizationNeteditSizeSettings::polygonContourWidth(0.3);
const double GUIVisualizationNeteditSizeSettings::polylineWidth(1);

// -------------------------------------------------------------------------
// additional values
// -------------------------------------------------------------------------

const double GUIVisualizationConnectionSettings::connectionWidth(0.2);

// -------------------------------------------------------------------------
// additional values
// -------------------------------------------------------------------------

const RGBColor GUIVisualizationAdditionalSettings::rerouterColor(255, 0, 0);
const double GUIVisualizationAdditionalSettings::rerouterSize(1);
const RGBColor GUIVisualizationAdditionalSettings::VSSColor(255, 255, 255);
const double GUIVisualizationAdditionalSettings::VSSSize(1);
const RGBColor GUIVisualizationAdditionalSettings::calibratorColor(255, 204, 0);
const double GUIVisualizationAdditionalSettings::calibratorWidth(1.4);
const double GUIVisualizationAdditionalSettings::calibratorHeight(6);
const RGBColor GUIVisualizationAdditionalSettings::routeProbeColor(255, 216, 0);
const double GUIVisualizationAdditionalSettings::routeProbeSize(1);
const RGBColor GUIVisualizationAdditionalSettings::vaporizerColor(120, 216, 0);
const double GUIVisualizationAdditionalSettings::vaporizerSize(1);

// -------------------------------------------------------------------------
// detector values
// -------------------------------------------------------------------------

const RGBColor GUIVisualizationDetectorSettings::E1Color(255, 255, 0);
const double GUIVisualizationDetectorSettings::E1Width(1);
const double GUIVisualizationDetectorSettings::E1Height(2);
const RGBColor GUIVisualizationDetectorSettings::E1InstantColor(255, 0, 255);
const double GUIVisualizationDetectorSettings::E1InstantWidth(1);
const double GUIVisualizationDetectorSettings::E1InstantHeight(2);
const RGBColor GUIVisualizationDetectorSettings::E2Color(0, 204, 204);
const double GUIVisualizationDetectorSettings::E2Width(1);
const double GUIVisualizationDetectorSettings::E3Size(1);
const RGBColor GUIVisualizationDetectorSettings::E3EntryColor(0, 92, 64);
const RGBColor GUIVisualizationDetectorSettings::E3ExitColor(92, 0, 0);
const double GUIVisualizationDetectorSettings::E3EntryExitWidth(1.7);
const double GUIVisualizationDetectorSettings::E3EntryExitHeight(0.5);

// -------------------------------------------------------------------------
// StoppingPlace values
// -------------------------------------------------------------------------

const double GUIVisualizationStoppingPlaceSettings::stoppingPlaceSignOffset(1.5);
const double GUIVisualizationStoppingPlaceSettings::busStopWidth(1);
const RGBColor GUIVisualizationStoppingPlaceSettings::busStopColor(76, 170, 50);
const RGBColor GUIVisualizationStoppingPlaceSettings::busStopColorSign(255, 235, 0);
const double GUIVisualizationStoppingPlaceSettings::containerStopWidth(1);
const RGBColor GUIVisualizationStoppingPlaceSettings::containerStopColor(83, 89, 172);
const RGBColor GUIVisualizationStoppingPlaceSettings::containerStopColorSign(177, 184, 186, 171);
const double GUIVisualizationStoppingPlaceSettings::chargingStationWidth(1);
const RGBColor GUIVisualizationStoppingPlaceSettings::chargingStationColor(114, 210, 252);
const RGBColor GUIVisualizationStoppingPlaceSettings::chargingStationColorSign(255, 235, 0);
const RGBColor GUIVisualizationStoppingPlaceSettings::chargingStationColorCharge(255, 180, 0);
const RGBColor GUIVisualizationStoppingPlaceSettings::parkingAreaColor(83, 89, 172);
const RGBColor GUIVisualizationStoppingPlaceSettings::parkingAreaColorSign(177, 184, 186);
const RGBColor GUIVisualizationStoppingPlaceSettings::parkingSpaceColorContour(0, 255, 0);
const RGBColor GUIVisualizationStoppingPlaceSettings::parkingSpaceColor(255, 200, 200);

// -------------------------------------------------------------------------
// Dotted contour values
// -------------------------------------------------------------------------

const double GUIVisualizationDottedContourSettings::segmentWidth(0.2);
const double GUIVisualizationDottedContourSettings::segmentLength(2);
const RGBColor GUIVisualizationDottedContourSettings::firstInspectedColor(235, 235, 235);
const RGBColor GUIVisualizationDottedContourSettings::secondInspectedColor(20, 20, 20);
const RGBColor GUIVisualizationDottedContourSettings::firstFrontColor(0, 0, 235);
const RGBColor GUIVisualizationDottedContourSettings::secondFrontColor(0, 255, 0);

// -------------------------------------------------------------------------
// widths of certain NETEDIT objects
// -------------------------------------------------------------------------

const double GUIVisualizationWidthSettings::route(0.66);
const double GUIVisualizationWidthSettings::trip(0.2);
const double GUIVisualizationWidthSettings::personTrip(0.25);
const double GUIVisualizationWidthSettings::walk(0.25);
const double GUIVisualizationWidthSettings::ride(0.25);

// -------------------------------------------------------------------------
// details of certain NETEDIT objects (0 = drawn always)
// -------------------------------------------------------------------------

const double GUIVisualizationDetailSettings::connectionsDemandMode(5);
const double GUIVisualizationDetailSettings::laneTextures(20); // originally 10
const double GUIVisualizationDetailSettings::lockIcon(30);
const double GUIVisualizationDetailSettings::additionalTextures(20); // originally 10
const double GUIVisualizationDetailSettings::geometryPointsDetails(10);
const double GUIVisualizationDetailSettings::geometryPointsText(20);
const double GUIVisualizationDetailSettings::stoppingPlaceDetails(10);
const double GUIVisualizationDetailSettings::stoppingPlaceText(10);
const double GUIVisualizationDetailSettings::detectorDetails(10);
const double GUIVisualizationDetailSettings::detectorText(30);
const double GUIVisualizationDetailSettings::calibratorText(10);
const double GUIVisualizationDetailSettings::stopsDetails(10);
const double GUIVisualizationDetailSettings::stopsText(50);
const double GUIVisualizationDetailSettings::vehicleTriangles(2);
const double GUIVisualizationDetailSettings::vehicleBoxes(5);
const double GUIVisualizationDetailSettings::vehicleShapes(10);
const double GUIVisualizationDetailSettings::personTriangles(2);
const double GUIVisualizationDetailSettings::personCircles(5);
const double GUIVisualizationDetailSettings::personShapes(10);
const double GUIVisualizationDetailSettings::personExaggeration(4);

// -------------------------------------------------------------------------
// scheme names
// -------------------------------------------------------------------------

const std::string GUIVisualizationSettings::SCHEME_NAME_EDGE_PARAM_NUMERICAL("by param (numerical, streetwise)");
const std::string GUIVisualizationSettings::SCHEME_NAME_LANE_PARAM_NUMERICAL("by param (numerical, lanewise)");
const std::string GUIVisualizationSettings::SCHEME_NAME_PARAM_NUMERICAL("by param (numerical)");
const std::string GUIVisualizationSettings::SCHEME_NAME_EDGEDATA_NUMERICAL("by edgeData (numerical, streetwise)");
const std::string GUIVisualizationSettings::SCHEME_NAME_SELECTION("by selection");
const std::string GUIVisualizationSettings::SCHEME_NAME_TYPE("by type");
const std::string GUIVisualizationSettings::SCHEME_NAME_PERMISSION_CODE("by permission code");

const double GUIVisualizationSettings::MISSING_DATA(std::numeric_limits<double>::max());

// color constants for scheme background
#define COL_SCHEME_EMISSION RGBColor(255,255,210)
#define COL_SCHEME_MISC     RGBColor(210,220,255)
#define COL_SCHEME_STATIC   RGBColor(230,230,230)
#define COL_SCHEME_DYNAMIC  RGBColor(220,255,220)

// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GUIVisualizationTextSettings - methods
// ---------------------------------------------------------------------------

GUIVisualizationTextSettings::GUIVisualizationTextSettings(bool _show, double _size, RGBColor _color, RGBColor _bgColor, bool _constSize) :
    show(_show),
    size(_size),
    color(_color),
    bgColor(_bgColor),
    constSize(_constSize) {
}


bool
GUIVisualizationTextSettings::operator==(const GUIVisualizationTextSettings& other) {
    return (show == other.show) &&
           (size == other.size) &&
           (color == other.color) &&
           (bgColor == other.bgColor) &&
           (constSize == other.constSize);
}


bool
GUIVisualizationTextSettings::operator!=(const GUIVisualizationTextSettings& other) {
    return (show != other.show) ||
           (size != other.size) ||
           (color != other.color) ||
           (bgColor != other.bgColor) ||
           (constSize != other.constSize);
}


void
GUIVisualizationTextSettings::print(OutputDevice& dev, const std::string& name) const {
    dev.writeAttr(name + "_show", show);
    dev.writeAttr(name + "_size", size);
    dev.writeAttr(name + "_color", color);
    dev.writeAttr(name + "_bgColor", bgColor);
    dev.writeAttr(name + "_constantSize", constSize);
}


double
GUIVisualizationTextSettings::scaledSize(double scale, double constFactor) const {
    return constSize ? (size / scale) : (size * constFactor);
}

// ---------------------------------------------------------------------------
// GUIVisualizationSizeSettings - methods
// ---------------------------------------------------------------------------

GUIVisualizationSizeSettings::GUIVisualizationSizeSettings(double _minSize, double _exaggeration, bool _constantSize, bool _constantSizeSelected) :
    minSize(_minSize),
    exaggeration(_exaggeration),
    constantSize(_constantSize),
    constantSizeSelected(_constantSizeSelected) {
}


double
GUIVisualizationSizeSettings::getExaggeration(const GUIVisualizationSettings& s, const GUIGlObject* o, double factor) const {
    /// @note should look normal-sized at zoom 1000
    if (constantSize && (!constantSizeSelected || (o == nullptr) || gSelected.isSelected(o))) {
        return MAX2((double)exaggeration, exaggeration * factor / s.scale);
    } else if (!constantSizeSelected || (o == nullptr) || gSelected.isSelected(o)) {
        return exaggeration;
    } else {
        return 1;
    }
}


bool
GUIVisualizationSizeSettings::operator==(const GUIVisualizationSizeSettings& other) {
    return (constantSize == other.constantSize) &&
           (constantSizeSelected == other.constantSizeSelected) &&
           (minSize == other.minSize) &&
           (exaggeration == other.exaggeration);
}


bool
GUIVisualizationSizeSettings::operator!=(const GUIVisualizationSizeSettings& other) {
    return (constantSize != other.constantSize) ||
           (constantSizeSelected != other.constantSizeSelected) ||
           (minSize != other.minSize) ||
           (exaggeration != other.exaggeration);
}


void
GUIVisualizationSizeSettings::print(OutputDevice& dev, const std::string& name) const {
    dev.writeAttr(name + "_minSize", minSize);
    dev.writeAttr(name + "_exaggeration", exaggeration);
    dev.writeAttr(name + "_constantSize", constantSize);
    dev.writeAttr(name + "_constantSizeSelected", constantSizeSelected);
}

// ---------------------------------------------------------------------------
// GUIVisualizationColorSettings - methods
// ---------------------------------------------------------------------------

GUIVisualizationColorSettings::GUIVisualizationColorSettings() :
    selectionColor(0, 0, 204, 255),
    selectedEdgeColor(0, 0, 204, 255),
    selectedLaneColor(0, 0, 128, 255),
    selectedConnectionColor(0, 0, 100, 255),
    selectedProhibitionColor(0, 0, 120, 255),
    selectedCrossingColor(0, 100, 196, 255),
    selectedAdditionalColor(0, 0, 150, 255),
    selectedRouteColor(0, 0, 150, 255),
    selectedVehicleColor(0, 0, 100, 255),
    selectedPersonColor(0, 0, 120, 255),
    selectedPersonPlanColor(0, 0, 130, 255),
    selectedEdgeDataColor(0, 0, 150, 255) {
}


bool
GUIVisualizationColorSettings::operator==(const GUIVisualizationColorSettings& v2) {
    return (selectionColor == v2.selectionColor) &&
           (selectedEdgeColor == v2.selectedEdgeColor) &&
           (selectedLaneColor == v2.selectedLaneColor) &&
           (selectedConnectionColor == v2.selectedConnectionColor) &&
           (selectedProhibitionColor == v2.selectedProhibitionColor) &&
           (selectedCrossingColor == v2.selectedCrossingColor) &&
           (selectedAdditionalColor == v2.selectedAdditionalColor) &&
           (selectedRouteColor == v2.selectedRouteColor) &&
           (selectedVehicleColor == v2.selectedVehicleColor) &&
           (selectedPersonColor == v2.selectedPersonColor) &&
           (selectedPersonPlanColor == v2.selectedPersonPlanColor) &&
           (selectedEdgeDataColor == v2.selectedEdgeDataColor);
}


bool
GUIVisualizationColorSettings::operator!=(const GUIVisualizationColorSettings& v2) {
    return (selectionColor != v2.selectionColor) ||
           (selectedEdgeColor != v2.selectedEdgeColor) ||
           (selectedLaneColor != v2.selectedLaneColor) ||
           (selectedConnectionColor != v2.selectedConnectionColor) ||
           (selectedProhibitionColor != v2.selectedProhibitionColor) ||
           (selectedCrossingColor != v2.selectedCrossingColor) ||
           (selectedAdditionalColor != v2.selectedAdditionalColor) ||
           (selectedRouteColor != v2.selectedRouteColor) ||
           (selectedVehicleColor != v2.selectedVehicleColor) ||
           (selectedPersonColor != v2.selectedPersonColor) ||
           (selectedPersonPlanColor != v2.selectedPersonPlanColor) ||
           (selectedEdgeDataColor != v2.selectedEdgeDataColor);
}

// ---------------------------------------------------------------------------
// GUIVisualizationSettings - methods
// ---------------------------------------------------------------------------

GUIVisualizationSettings::GUIVisualizationSettings(bool _netedit) :
    name(""),
    netedit(_netedit),
    angle(0),
    dither(false),
    fps(false),
    backgroundColor(RGBColor::WHITE),
    showGrid(false), gridXSize(100), gridYSize(100),
    laneShowBorders(false), showBikeMarkings(true), showLinkDecals(true),
    showLinkRules(true), showRails(true),
    edgeName(false, 60, RGBColor(255, 128, 0, 255)),
    internalEdgeName(false, 45, RGBColor(128, 64, 0, 255)),
    cwaEdgeName(false, 60, RGBColor::MAGENTA),
    streetName(false, 60, RGBColor::YELLOW),
    edgeValue(false, 100, RGBColor::CYAN),
    hideConnectors(false),
    laneWidthExaggeration(1),
    laneMinSize(0),
    showLaneDirection(false),
    showSublanes(true),
    spreadSuperposed(false),
    edgeParam("EDGE_KEY"),
    laneParam("LANE_KEY"),
    vehicleParam("PARAM_NUMERICAL"),
    vehicleTextParam("PARAM_TEXT"),
    edgeData("speed"),
    edgeValueHideCheck(false),
    edgeValueHideThreshold(0),
    vehicleQuality(0), showBlinker(true),
    drawLaneChangePreference(false),
    drawMinGap(false),
    drawBrakeGap(false),
    showBTRange(false),
    showRouteIndex(false),
    vehicleSize(1),
    vehicleName(false, 60, RGBColor(204, 153, 0, 255)),
    vehicleValue(false, 80, RGBColor::CYAN),
    vehicleText(false, 80, RGBColor::RED),
    personQuality(0),
    personSize(1),
    personName(false, 60, RGBColor(0, 153, 204, 255)),
    personValue(false, 80, RGBColor::CYAN),
    containerQuality(0),
    containerSize(1),
    containerName(false, 60, RGBColor(0, 153, 204, 255)),
    drawLinkTLIndex(false, 65, RGBColor(128, 128, 255, 255), RGBColor::INVISIBLE, false),
    drawLinkJunctionIndex(false, 65, RGBColor(128, 128, 255, 255), RGBColor::INVISIBLE, false),
    junctionID(false, 60, RGBColor(0, 255, 128, 255)),
    junctionName(false, 60, RGBColor(192, 255, 128, 255)),
    internalJunctionName(false, 50, RGBColor(0, 204, 128, 255)),
    tlsPhaseIndex(false, 150, RGBColor::YELLOW),
    tlsPhaseName(false, 150, RGBColor::ORANGE),
    showLane2Lane(false),
    drawJunctionShape(true),
    drawCrossingsAndWalkingareas(true),
    junctionSize(1),
    addMode(0),
    addSize(1),
    addName(false, 60, RGBColor(255, 0, 128, 255)),
    addFullName(false, 60, RGBColor(255, 0, 128, 255)),
    poiSize(0), poiName(false, 50, RGBColor(255, 0, 128, 255)),
    poiType(false, 60, RGBColor(255, 0, 128, 255)),
    poiText(false, 80, RGBColor(140, 0, 255, 255)),
    poiTextParam("PARAM_TEXT"),
    polySize(0), polyName(false, 50, RGBColor(255, 0, 128, 255)),
    polyType(false, 60, RGBColor(255, 0, 128, 255)),
    showSizeLegend(true),
    showColorLegend(false),
    showVehicleColorLegend(false),
    gaming(false),
    drawBoundaries(false),
    selectionScale(1.),
    drawForPositionSelection(false),
    drawForRectangleSelection(false),
    forceDrawForPositionSelection(false),
    forceDrawForRectangleSelection(false),
    forceDrawDottedContour(false),
    lefthand(false),
    disableLaneIcons(false) {
    // init defaults depending of NETEDIT or SUMO-GUI
    if (netedit) {
        initNeteditDefaults();
    } else {
        initSumoGuiDefaults();
    }
}


void
GUIVisualizationSettings::initSumoGuiDefaults() {
    /// add lane coloring schemes
    GUIColorScheme scheme = GUIColorScheme("uniform", RGBColor::BLACK, "road", true, 0);
    scheme.addColor(RGBColor::GREY, 1, "sidewalk");
    scheme.addColor(RGBColor(192, 66, 44), 2, "bike lane");
    scheme.addColor(RGBColor(0, 0, 0, 0), 3, "green verge");
    scheme.addColor(RGBColor(150, 200, 200), 4, "waterway");
    scheme.addColor(RGBColor::BLACK, 5, "railway");
    scheme.addColor(RGBColor(64, 0, 64), 6, "rails on road");
    scheme.addColor(RGBColor(92, 92, 92), 7, "no passenger"); // paths, service roads etc
    scheme.addColor(RGBColor::RED, 8, "closed"); // road closing
    scheme.addColor(RGBColor::GREEN, 9, "connector"); // macro-connector
    scheme.addColor(RGBColor::ORANGE, 10, "forbidden"); // forbidden road
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(128, 128, 128, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 80, 180, 255), 1, "selected");
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_PERMISSION_CODE, RGBColor(240, 240, 240), "nobody");
    scheme.addColor(RGBColor(10, 10, 10), (double)SVC_PASSENGER, "passenger");
    scheme.addColor(RGBColor(128, 128, 128), (double)SVC_PEDESTRIAN, "pedestrian");
    scheme.addColor(RGBColor(80, 80, 80), (double)(SVC_PEDESTRIAN | SVC_DELIVERY), "pedestrian_delivery");
    scheme.addColor(RGBColor(192, 66, 44), (double)SVC_BICYCLE, "bicycle");
    scheme.addColor(RGBColor(40, 100, 40), (double)SVC_BUS, "bus");
    scheme.addColor(RGBColor(166, 147, 26), (double)SVC_TAXI, "taxi");
    scheme.addColor(RGBColor::BLACK, (double)(SVCAll & ~SVC_NON_ROAD), "normal_road");
    scheme.addColor(RGBColor::BLACK, (double)(SVCAll & ~(SVC_PEDESTRIAN | SVC_NON_ROAD)), "disallow_pedestrian");
    scheme.addColor(RGBColor(255, 206, 0), (double)(SVCAll & ~(SVC_PEDESTRIAN | SVC_BICYCLE | SVC_MOPED | SVC_NON_ROAD)), "motorway");
    scheme.addColor(RGBColor(150, 200, 200), (double)SVC_SHIP, "waterway");
    scheme.addColor(RGBColor::GREEN, (double)SVCAll, "all");
    laneColorer.addScheme(scheme);
    // ... traffic states ...
    scheme = GUIColorScheme("by allowed speed (lanewise)", RGBColor::RED);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current occupancy (lanewise, brutto)", RGBColor(235, 235, 235), "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::GREEN, 0.25);
    scheme.addColor(RGBColor::YELLOW, 0.5);
    scheme.addColor(RGBColor::ORANGE, 0.75);
    scheme.addColor(RGBColor::RED, 1.0);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current occupancy (lanewise, netto)", RGBColor(235, 235, 235), "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::GREEN, 0.25);
    scheme.addColor(RGBColor::YELLOW, 0.5);
    scheme.addColor(RGBColor::ORANGE, 0.75);
    scheme.addColor(RGBColor::RED, 1.0);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by first vehicle waiting time (lanewise)", RGBColor(235, 235, 235), "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)30);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::YELLOW, (double)200);
    scheme.addColor(RGBColor::RED, (double)300);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by lane number (streetwise)", RGBColor::RED);
    scheme.addColor(RGBColor::BLUE, (double)5);
    laneColorer.addScheme(scheme);
    // ... emissions ...
    scheme = GUIColorScheme("by CO2 emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 450);
    scheme.addColor(RGBColor::GREEN, 900);
    scheme.addColor(RGBColor::YELLOW, 1350);
    scheme.addColor(RGBColor::ORANGE, 1800);
    scheme.addColor(RGBColor::RED, 2250);
    scheme.addColor(RGBColor::MAGENTA, 3000);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by CO emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 30);
    scheme.addColor(RGBColor::GREEN, 60);
    scheme.addColor(RGBColor::YELLOW, 90);
    scheme.addColor(RGBColor::ORANGE, 120);
    scheme.addColor(RGBColor::RED, 150);
    scheme.addColor(RGBColor::MAGENTA, 200);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by PMx emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 0.3);
    scheme.addColor(RGBColor::GREEN, 0.5);
    scheme.addColor(RGBColor::YELLOW, 0.8);
    scheme.addColor(RGBColor::ORANGE, 1);
    scheme.addColor(RGBColor::RED, 1.3);
    scheme.addColor(RGBColor::MAGENTA, 1.6);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by NOx emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 6);
    scheme.addColor(RGBColor::GREEN, 12);
    scheme.addColor(RGBColor::YELLOW, 18);
    scheme.addColor(RGBColor::ORANGE, 24);
    scheme.addColor(RGBColor::RED, 30);
    scheme.addColor(RGBColor::MAGENTA, 40);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by HC emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 8);
    scheme.addColor(RGBColor::GREEN, 16);
    scheme.addColor(RGBColor::YELLOW, 24);
    scheme.addColor(RGBColor::ORANGE, 32);
    scheme.addColor(RGBColor::RED, 40);
    scheme.addColor(RGBColor::MAGENTA, 50);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by fuel consumption", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 0.2);
    scheme.addColor(RGBColor::GREEN, 0.4);
    scheme.addColor(RGBColor::YELLOW, 0.6);
    scheme.addColor(RGBColor::ORANGE, 0.8);
    scheme.addColor(RGBColor::RED, 1);
    scheme.addColor(RGBColor::MAGENTA, 1.3);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by noise emissions (Harmonoise)", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 1.3);
    scheme.addColor(RGBColor::GREEN, 1.4);
    scheme.addColor(RGBColor::YELLOW, 1.6);
    scheme.addColor(RGBColor::ORANGE, 1.7);
    scheme.addColor(RGBColor::RED, 1.9);
    scheme.addColor(RGBColor::MAGENTA, 2.4);
    laneColorer.addScheme(scheme);
    // ... weights (experimental) ...
    scheme = GUIColorScheme("by global travel time", RGBColor::GREEN);
    scheme.addColor(RGBColor::RED, (double)100);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by global speed percentage", RGBColor::RED);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by given length/geometrical length", RGBColor::RED);
    scheme.addColor(RGBColor::ORANGE, 0.25);
    scheme.addColor(RGBColor::YELLOW, 0.5);
    scheme.addColor(RGBColor(179, 179, 179, 255), (double)1.0);
    scheme.addColor(RGBColor::GREEN, (double)2.0);
    scheme.addColor(RGBColor::CYAN, (double)4.0);
    scheme.addColor(RGBColor::BLUE, (double)100.0);
    laneColorer.addScheme(scheme);
    laneColorer.addScheme(GUIColorScheme("by angle", RGBColor::YELLOW, "", true));
    scheme = GUIColorScheme("by loaded weight", RGBColor::GREEN);
    scheme.addColor(RGBColor::RED, (double)100);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by priority", RGBColor::YELLOW);
    scheme.addColor(RGBColor::RED, (double) - 20);
    scheme.addColor(RGBColor::GREEN, (double)20);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by height at start", RGBColor::GREY);
    scheme.addColor(RGBColor::BLUE, (double) - 10);
    scheme.addColor(RGBColor::RED, (double)10);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::MAGENTA, (double)200);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by height at geometry-segment start", RGBColor::GREY);
    scheme.addColor(RGBColor::BLUE, (double) - 10);
    scheme.addColor(RGBColor::RED, (double)10);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::MAGENTA, (double)200);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by inclination", RGBColor::GREY);
    scheme.addColor(RGBColor::YELLOW, (double) .1);
    scheme.addColor(RGBColor::RED, (double) .3);
    scheme.addColor(RGBColor::GREEN, (double) - .1);
    scheme.addColor(RGBColor::BLUE, (double) - .3);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by geometry-segment inclination", RGBColor::GREY);
    scheme.addColor(RGBColor::YELLOW, (double) .1);
    scheme.addColor(RGBColor::RED, (double) .3);
    scheme.addColor(RGBColor::GREEN, (double) - .1);
    scheme.addColor(RGBColor::BLUE, (double) - .3);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by average speed", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by average relative speed ", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(0.25));
    scheme.addColor(RGBColor::GREEN, (double)(0.5));
    scheme.addColor(RGBColor::CYAN, (double)(0.75));
    scheme.addColor(RGBColor::BLUE, (double)(1));
    scheme.addColor(RGBColor::MAGENTA, (double)(1.25));
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by routing device assumed speed ", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by electricity consumption", RGBColor::GREEN, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 0.2);
    scheme.addColor(RGBColor::GREEN, 0.4);
    scheme.addColor(RGBColor::YELLOW, 0.6);
    scheme.addColor(RGBColor::ORANGE, 0.8);
    scheme.addColor(RGBColor::RED, 1.0);
    scheme.addColor(RGBColor::MAGENTA, 2.0);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by insertion-backlog (streetwise)", RGBColor(204, 204, 204), "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::GREEN, (double)1);
    scheme.addColor(RGBColor::YELLOW, (double)10);
    scheme.addColor(RGBColor::RED, (double)100);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by TAZ (streetwise)", RGBColor(204, 204, 204), "no TAZ", true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_EDGE_PARAM_NUMERICAL, RGBColor(204, 204, 204));
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_LANE_PARAM_NUMERICAL, RGBColor(204, 204, 204));
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_EDGEDATA_NUMERICAL, RGBColor(204, 204, 204), "missing data", false, MISSING_DATA);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by distance (kilometrage)", RGBColor(204, 204, 204));
    scheme.addColor(RGBColor(255, 204, 204), (double)1);
    scheme.addColor(RGBColor::RED, (double)10000);
    scheme.addColor(RGBColor(204, 204, 255), (double) -1);
    scheme.addColor(RGBColor::BLUE, (double) -10000);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by abs distance (kilometrage)", RGBColor(204, 204, 204));
    scheme.addColor(RGBColor::RED, (double)1);
    scheme.setAllowsNegativeValues(false);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by reachability (traveltime)", RGBColor(204, 204, 204));
    scheme.addColor(RGBColor::RED, (double)1);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by thread index", RGBColor(204, 204, 204));
    scheme.addColor(RGBColor::RED, (double)1);
    laneColorer.addScheme(scheme);

    /// add vehicle coloring schemes
    vehicleColorer.addScheme(GUIColorScheme("given vehicle/type/route color", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("uniform", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("given/assigned vehicle color", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("given/assigned type color", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("given/assigned route color", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("depart position as HSV", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("arrival position as HSV", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("direction/distance as HSV", RGBColor::YELLOW, "", true));
    scheme = GUIColorScheme("by speed", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by action step", RGBColor::GREY, "no action", true, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor(0, 255, 0, 255), 1., "action in next step");
    scheme.addColor(RGBColor(80, 160, 80, 255), 2., "had action step");
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by waiting time", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)30);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::YELLOW, (double)200);
    scheme.addColor(RGBColor::RED, (double)300);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by accumulated waiting time", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)25);
    scheme.addColor(RGBColor::GREEN, (double)50);
    scheme.addColor(RGBColor::YELLOW, (double)75);
    scheme.addColor(RGBColor::RED, (double)100);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by time since lane change", RGBColor(179, 179, 179, 255), "0", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor(189, 189, 179, 255), -180);
    scheme.addColor(RGBColor(255, 255, 0, 255), -20);
    scheme.addColor(RGBColor(255,   0, 0, 255), -0.01);
    scheme.addColor(RGBColor(0,   0, 255, 255),  0.01);
    scheme.addColor(RGBColor(0, 255, 255, 255),  20);
    scheme.addColor(RGBColor(179, 189, 189, 255),  180);
    scheme.setAllowsNegativeValues(true);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by max speed", RGBColor::RED);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    vehicleColorer.addScheme(scheme);
    // ... emissions ...
    scheme = GUIColorScheme("by CO2 emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 23000);
    scheme.addColor(RGBColor::GREEN, 46000);
    scheme.addColor(RGBColor::YELLOW, 69000);
    scheme.addColor(RGBColor::ORANGE, 92000);
    scheme.addColor(RGBColor::RED, 115000);
    scheme.addColor(RGBColor::MAGENTA, 150000);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by CO emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 1500);
    scheme.addColor(RGBColor::GREEN, 3000);
    scheme.addColor(RGBColor::YELLOW, 4500);
    scheme.addColor(RGBColor::ORANGE, 6000);
    scheme.addColor(RGBColor::RED, 7500);
    scheme.addColor(RGBColor::MAGENTA, 10000);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by PMx emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 12);
    scheme.addColor(RGBColor::GREEN, 24);
    scheme.addColor(RGBColor::YELLOW, 36);
    scheme.addColor(RGBColor::ORANGE, 48);
    scheme.addColor(RGBColor::RED, 60);
    scheme.addColor(RGBColor::MAGENTA, 80);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by NOx emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 300);
    scheme.addColor(RGBColor::GREEN, 600);
    scheme.addColor(RGBColor::YELLOW, 900);
    scheme.addColor(RGBColor::ORANGE, 1200);
    scheme.addColor(RGBColor::RED, 1500);
    scheme.addColor(RGBColor::MAGENTA, 2000);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by HC emissions", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 400);
    scheme.addColor(RGBColor::GREEN, 800);
    scheme.addColor(RGBColor::YELLOW, 1200);
    scheme.addColor(RGBColor::ORANGE, 1600);
    scheme.addColor(RGBColor::RED, 2000);
    scheme.addColor(RGBColor::MAGENTA, 2500);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by fuel consumption", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 10);
    scheme.addColor(RGBColor::GREEN, 20);
    scheme.addColor(RGBColor::YELLOW, 30);
    scheme.addColor(RGBColor::ORANGE, 40);
    scheme.addColor(RGBColor::RED, 50);
    scheme.addColor(RGBColor::MAGENTA, 60);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by noise emissions (Harmonoise)", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 60);
    scheme.addColor(RGBColor::GREEN, 70);
    scheme.addColor(RGBColor::YELLOW, 80);
    scheme.addColor(RGBColor::ORANGE, 90);
    scheme.addColor(RGBColor::RED, 100);
    scheme.addColor(RGBColor::MAGENTA, 120);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by reroute number", RGBColor::GREY, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)1.);
    scheme.addColor(RGBColor::RED, (double)10.);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by offset from best lane", RGBColor(179, 179, 179, 255), "0", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor(255, 0, 255, 255), -100, "opposite lane");
    scheme.addColor(RGBColor(255,   0, 0, 255), -3, "-3");
    scheme.addColor(RGBColor(255, 255, 0, 255), -1, "-1");
    scheme.addColor(RGBColor(0, 255, 255, 255),  1,  "1");
    scheme.addColor(RGBColor(0,   0, 255, 255),  3,  "3");
    scheme.setAllowsNegativeValues(true);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by acceleration", RGBColor(179, 179, 179, 255), "0", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor(64,    0, 0, 255), -9.0);
    scheme.addColor(RGBColor(255,   0, 0, 255), -4.5 /* -SUMOVTypeParameter::getDefaultDecel() */);
    scheme.addColor(RGBColor(255, 255, 0, 255), -0.1);
    scheme.addColor(RGBColor(0, 255, 255, 255),  0.1);
    scheme.addColor(RGBColor(0,   0, 255, 255),  2.6 /* SUMOVTypeParameter::getDefaultAccel() */);
    scheme.addColor(RGBColor(255, 0, 255, 255),  5.2);
    scheme.setAllowsNegativeValues(true);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by time gap on lane", RGBColor(255, 255, 0, 255), "0", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor(179, 179, 179, 255), -1);
    scheme.addColor(RGBColor(0, 255, 255, 255), 1);
    scheme.addColor(RGBColor(0,   0, 255, 255), 2);
    scheme.setAllowsNegativeValues(true);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by depart delay", RGBColor::BLUE);
    scheme.addColor(RGBColor::CYAN, (double)30);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::YELLOW, (double)200);
    scheme.addColor(RGBColor::RED, (double)300);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by electricity consumption", RGBColor::GREY, "", false, 0, COL_SCHEME_EMISSION);
    scheme.addColor(RGBColor::CYAN, 10);
    scheme.addColor(RGBColor::GREEN, 20);
    scheme.addColor(RGBColor::YELLOW, 30);
    scheme.addColor(RGBColor::ORANGE, 40);
    scheme.addColor(RGBColor::RED, 60);
    scheme.addColor(RGBColor::MAGENTA, 100);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by time loss", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)10);
    scheme.addColor(RGBColor::GREEN, (double)60);
    scheme.addColor(RGBColor::YELLOW, (double)180);
    scheme.addColor(RGBColor::RED, (double)900);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by stop delay", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)10);
    scheme.addColor(RGBColor::GREEN, (double)60);
    scheme.addColor(RGBColor::YELLOW, (double)120);
    scheme.addColor(RGBColor::ORANGE, (double)300);
    scheme.addColor(RGBColor::RED, (double)900);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme("by lateral speed", RGBColor(179, 179, 179, 255), "0", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor(255,   0, 0, 255), -3, "-1.5");
    scheme.addColor(RGBColor(255, 255, 0, 255), -1, "-0.5");
    scheme.addColor(RGBColor(0, 255, 255, 255),  1,  "0.5");
    scheme.addColor(RGBColor(0,   0, 255, 255),  3,  "1.5");
    scheme.setAllowsNegativeValues(true);
    vehicleColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_PARAM_NUMERICAL, RGBColor(204, 204, 204));
    scheme.setAllowsNegativeValues(true);
    vehicleColorer.addScheme(scheme);
    vehicleColorer.addScheme(GUIColorScheme("random", RGBColor::YELLOW, "", true));
    vehicleColorer.addScheme(GUIColorScheme("by angle", RGBColor::YELLOW, "", true));

    /// add person coloring schemes
    personColorer.addScheme(GUIColorScheme("given person/type color", RGBColor::BLUE, "", true));
    personColorer.addScheme(GUIColorScheme("uniform", RGBColor::BLUE, "", true));
    personColorer.addScheme(GUIColorScheme("given/assigned person color", RGBColor::BLUE, "", true));
    personColorer.addScheme(GUIColorScheme("given/assigned type color", RGBColor::BLUE, "", true));
    scheme = GUIColorScheme("by speed", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(2.5 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(5 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(10 / 3.6));
    personColorer.addScheme(scheme);
    scheme = GUIColorScheme("by mode", RGBColor::GREY, "waiting for insertion", true, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, (double)(1), "stopped");
    scheme.addColor(RGBColor::GREEN, (double)(2), "walking");
    scheme.addColor(RGBColor::BLUE, (double)(3), "riding");
    scheme.addColor(RGBColor::CYAN, (double)(4), "accessing trainStop");
    scheme.addColor(RGBColor::YELLOW, (double)(5), "waiting for ride");
    personColorer.addScheme(scheme);
    scheme = GUIColorScheme("by waiting time", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)30);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::YELLOW, (double)200);
    scheme.addColor(RGBColor::RED, (double)300);
    personColorer.addScheme(scheme);
    scheme = GUIColorScheme("by jammed state", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, (double)1);
    personColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    personColorer.addScheme(scheme);
    personColorer.addScheme(GUIColorScheme("by angle", RGBColor::YELLOW, "", true));
    personColorer.addScheme(GUIColorScheme("random", RGBColor::YELLOW, "", true));

    /// add container coloring schemes
    containerColorer.addScheme(GUIColorScheme("given container/type color", RGBColor::YELLOW, "", true));
    containerColorer.addScheme(GUIColorScheme("uniform", RGBColor::YELLOW, "", true));
    containerColorer.addScheme(GUIColorScheme("given/assigned container color", RGBColor::YELLOW, "", true));
    containerColorer.addScheme(GUIColorScheme("given/assigned type color", RGBColor::YELLOW, "", true));
    scheme = GUIColorScheme("by speed", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(2.5 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(5 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(10 / 3.6));
    containerColorer.addScheme(scheme);
    scheme = GUIColorScheme("by mode", RGBColor::GREY, "waiting for insertion", true, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, (double)(1), "stopped");
    scheme.addColor(RGBColor::GREEN, (double)(2), "tranship"); // walking
    scheme.addColor(RGBColor::BLUE, (double)(3), "transport");
    scheme.addColor(RGBColor::CYAN, (double)(4), "accessing trainStop");
    scheme.addColor(RGBColor::YELLOW, (double)(5), "waiting for transport");
    containerColorer.addScheme(scheme);
    scheme = GUIColorScheme("by waiting time", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::CYAN, (double)30);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::YELLOW, (double)200);
    scheme.addColor(RGBColor::RED, (double)300);
    containerColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    containerColorer.addScheme(scheme);
    containerColorer.addScheme(GUIColorScheme("by angle", RGBColor::YELLOW, "", true));

    /// add junction coloring schemes
    scheme = GUIColorScheme("uniform", RGBColor::BLACK, "", true);
    scheme.addColor(RGBColor(150, 200, 200), 1, "waterway");
    scheme.addColor(RGBColor(0, 0, 0, 0), 2, "railway");
    junctionColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(128, 128, 128, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 80, 180, 255), 1, "selected");
    junctionColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_TYPE, RGBColor::GREEN, "traffic_light", true);
    scheme.addColor(RGBColor(0, 128, 0), 1, "traffic_light_unregulated");
    scheme.addColor(RGBColor::YELLOW, 2, "priority");
    scheme.addColor(RGBColor::RED, 3, "priority_stop");
    scheme.addColor(RGBColor::BLUE, 4, "right_before_left");
    scheme.addColor(RGBColor::CYAN, 5, "allway_stop");
    scheme.addColor(RGBColor::GREY, 6, "district");
    scheme.addColor(RGBColor::MAGENTA, 7, "unregulated");
    scheme.addColor(RGBColor::BLACK, 8, "dead_end");
    scheme.addColor(RGBColor::ORANGE, 9, "rail_signal");
    scheme.addColor(RGBColor(172, 108, 44), 10, "zipper"); // brown, darker than the zipper link rule
    scheme.addColor(RGBColor(192, 255, 192), 11, "traffic_light_right_on_red"); // light green
    scheme.addColor(RGBColor(128, 0, 128), 12, "rail_crossing"); // dark purple
    junctionColorer.addScheme(scheme);
    scheme = GUIColorScheme("by height", RGBColor::GREY);
    scheme.addColor(RGBColor::BLUE, (double) - 10);
    scheme.addColor(RGBColor::RED, (double)10);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::MAGENTA, (double)200);
    scheme.setAllowsNegativeValues(true);
    junctionColorer.addScheme(scheme);

    /// add POI coloring schemes
    poiColorer.addScheme(GUIColorScheme("given POI color", RGBColor::RED, "", true));
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    poiColorer.addScheme(scheme);
    poiColorer.addScheme(GUIColorScheme("uniform", RGBColor::RED, "", true));

    /// add polygon coloring schemes
    polyColorer.addScheme(GUIColorScheme("given polygon color", RGBColor::ORANGE, "", true));
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    polyColorer.addScheme(scheme);
    polyColorer.addScheme(GUIColorScheme("uniform", RGBColor::ORANGE, "", true));

    /// add lane scaling schemes
    {
        GUIScaleScheme laneScheme = GUIScaleScheme("default", 1, "uniform", true);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme(SCHEME_NAME_SELECTION, 0.5, "unselected", true, 0, COL_SCHEME_MISC);
        laneScheme.addColor(5, 1, "selected");
        laneScaler.addScheme(laneScheme);
        // ... traffic states ...
        laneScheme = GUIScaleScheme("by allowed speed (lanewise)", 0);
        laneScheme.addColor(10, (double)(150.0 / 3.6));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by current occupancy (lanewise, brutto)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        laneScheme.addColor(10, 0.95);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by current occupancy (lanewise, netto)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        laneScheme.addColor(10, 0.95);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by first vehicle waiting time (lanewise)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        laneScheme.addColor(10, (double)300);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by lane number (streetwise)", 1);
        laneScheme.addColor(10, (double)5);
        laneScaler.addScheme(laneScheme);
        // ... emissions ...
        laneScheme = GUIScaleScheme("by CO2 emissions", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(10. / 7.5 / 5.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by CO emissions", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(0.05 / 7.5 / 2.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by PMx emissions", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(.005 / 7.5 / 5.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by NOx emissions", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(.125 / 7.5 / 5.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by HC emissions", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(.02 / 7.5 / 4.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by fuel consumption", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(.005 / 7.5 * 100.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by noise emissions (Harmonoise)", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)100);
        laneScaler.addScheme(laneScheme);
        // ... weights (experimental) ...
        laneScheme = GUIScaleScheme("by global travel time", 0);
        laneScheme.addColor(10, (double)100);
        laneScheme.setAllowsNegativeValues(true);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by global speed percentage", 0);
        laneScheme.addColor(10, (double)100);
        laneScheme.setAllowsNegativeValues(true);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by given length/geometrical length", 0);
        laneScheme.addColor(10, (double)10.0);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by loaded weight", 0);
        laneScheme.addColor(-1000, (double) - 1000);
        laneScheme.addColor(1000, (double)1000);
        laneScheme.setAllowsNegativeValues(true);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by priority", 1);
        laneScheme.addColor(0.5, (double) - 20);
        laneScheme.addColor(5, (double)20);
        laneScheme.setAllowsNegativeValues(true);
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by average speed", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        laneScheme.addColor(10, (double)(150.0 / 3.6));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by average relative speed", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        laneScheme.addColor(0.5, (double)(0.5));
        laneScheme.addColor(2, (double)(1));
        laneScheme.addColor(10, (double)(2));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by electricity consumption", 0, "", false, 0, COL_SCHEME_EMISSION);
        laneScheme.addColor(10, (double)(1 / 7.5 / 5.));
        laneScaler.addScheme(laneScheme);
        laneScheme = GUIScaleScheme("by insertion-backlog (streetwise)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        laneScheme.addColor(1, (double)1);
        laneScheme.addColor(10, (double)10);
        laneScheme.addColor(50, (double)100);
        laneScaler.addScheme(laneScheme);
    }

    /// add edge coloring schemes
    edgeColorer.addScheme(GUIColorScheme("uniform", RGBColor(0, 0, 0, 0), "", true));
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(128, 128, 128, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 80, 180, 255), 1, "selected");
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by purpose (streetwise)", RGBColor(), "normal", true);
    scheme.addColor(RGBColor(128, 0, 128, 255), 1, "connector"); // see MSEdge::EdgeBasicFunction::EDGEFUNCTION_CONNECTOR
    scheme.addColor(RGBColor::BLUE, 2, "internal"); // see MSEdge::EdgeBasicFunction::EDGEFUNCTION_INTERNAL
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by allowed speed (streetwise)", RGBColor::RED);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current occupancy (streetwise, brutto)", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, 0.95);
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current speed (streetwise)", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current flow (streetwise)", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, (double)5000);
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by relative speed (streetwise)", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(0.25));
    scheme.addColor(RGBColor::GREEN, (double)(0.5));
    scheme.addColor(RGBColor::CYAN, (double)(0.75));
    scheme.addColor(RGBColor::BLUE, (double)(1));
    scheme.addColor(RGBColor::MAGENTA, (double)(1.25));
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by routing device assumed speed", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    edgeColorer.addScheme(scheme);
    edgeColorer.addScheme(GUIColorScheme("by angle", RGBColor::YELLOW, "", true));
    scheme = GUIColorScheme("by segments (alternating)", RGBColor::BLUE, "odd", true);
    scheme.addColor(RGBColor::RED, 1, "even");
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by jammed state (segmentwise)", RGBColor::GREEN, "free", true, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, 1, "jammed");
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current occupancy (segmentwise, brutto)", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, 0.95);
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current speed (segmentwise)", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by current flow (segmentwise)", RGBColor::BLUE, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::RED, (double)5000);
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by relative speed (segmentwise)", RGBColor::RED, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::YELLOW, (double)(0.25));
    scheme.addColor(RGBColor::GREEN, (double)(0.5));
    scheme.addColor(RGBColor::CYAN, (double)(0.75));
    scheme.addColor(RGBColor::BLUE, (double)(1));
    scheme.addColor(RGBColor::MAGENTA, (double)(1.25));
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by insertion-backlog (streetwise)", RGBColor::GREY, "", false, 0, COL_SCHEME_DYNAMIC);
    scheme.addColor(RGBColor::GREEN, (double)1);
    scheme.addColor(RGBColor::YELLOW, (double)10);
    scheme.addColor(RGBColor::RED, (double)100);
    edgeColorer.addScheme(scheme);
    scheme = GUIColorScheme("by TAZ (streetwise)", RGBColor(204, 204, 204), "no TAZ", true);
    edgeColorer.addScheme(scheme);

    /// add edge scaling schemes
    {
        edgeScaler.addScheme(GUIScaleScheme("uniform", 1, "", true));
        GUIScaleScheme edgeScheme = GUIScaleScheme(SCHEME_NAME_SELECTION, 0.5, "unselected", true, 0, COL_SCHEME_MISC);
        edgeScheme.addColor(5, 1, "selected");
        edgeScaler.addScheme(edgeScheme);
        edgeScheme = GUIScaleScheme("by allowed speed (streetwise)", 0);
        edgeScheme.addColor(10, (double)(150.0 / 3.6));
        edgeScaler.addScheme(edgeScheme);
        edgeScheme = GUIScaleScheme("by current occupancy (streetwise, brutto)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        edgeScheme.addColor(10, 0.95);
        edgeScaler.addScheme(edgeScheme);
        edgeScheme = GUIScaleScheme("by current speed (streetwise)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        edgeScheme.addColor(10, (double)(150.0 / 3.6));
        edgeScaler.addScheme(edgeScheme);
        edgeScheme = GUIScaleScheme("by current flow (streetwise)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        edgeScheme.addColor(20, (double)5000);
        edgeScaler.addScheme(edgeScheme);
        edgeScheme = GUIScaleScheme("by relative speed (streetwise)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        edgeScheme.addColor(20, (double)1);
        edgeScaler.addScheme(edgeScheme);
        edgeScheme = GUIScaleScheme("by insertion-backlog (streetwise)", 0, "", false, 0, COL_SCHEME_DYNAMIC);
        edgeScheme.addColor(1, (double)1);
        edgeScheme.addColor(10, (double)10);
        edgeScheme.addColor(50, (double)100);
        edgeScaler.addScheme(edgeScheme);
    }

}


void
GUIVisualizationSettings::initNeteditDefaults() {
    // init color schemes
    GUIColorScheme scheme = GUIColorScheme("uniform", RGBColor::BLACK, "road", true);
    scheme.addColor(RGBColor::GREY, 1, "sidewalk");
    scheme.addColor(RGBColor(192, 66, 44), 2, "bike lane");
    scheme.addColor(RGBColor(200, 255, 200), 3, "green verge");
    scheme.addColor(RGBColor(150, 200, 200), 4, "waterway");
    scheme.addColor(RGBColor::BLACK, 5, "railway");
    scheme.addColor(RGBColor(64, 0, 64), 6, "rails on road");
    scheme.addColor(RGBColor(92, 92, 92), 7, "no passenger"); // paths, service roads etc
    scheme.addColor(RGBColor::RED, 8, "closed"); // road closing
    scheme.addColor(RGBColor::GREEN, 9, "connector"); // macro-connector
    scheme.addColor(RGBColor::ORANGE, 10, "forbidden"); // forbidden road
    scheme.addColor(RGBColor(145, 145, 145), 11, "data mode");
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(128, 128, 128, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 80, 180, 255), 1, "selected");
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by permission code", RGBColor(240, 240, 240), "nobody");
    scheme.addColor(RGBColor(10, 10, 10), (double)SVC_PASSENGER, "passenger");
    scheme.addColor(RGBColor(128, 128, 128), (double)SVC_PEDESTRIAN, "pedestrian");
    scheme.addColor(RGBColor(80, 80, 80), (double)(SVC_PEDESTRIAN | SVC_DELIVERY), "pedestrian_delivery");
    scheme.addColor(RGBColor(192, 66, 44), (double)SVC_BICYCLE, "bicycle");
    scheme.addColor(RGBColor(40, 100, 40), (double)SVC_BUS, "bus");
    scheme.addColor(RGBColor(166, 147, 26), (double)SVC_TAXI, "taxi");
    scheme.addColor(RGBColor::BLACK, (double)(SVCAll & ~SVC_NON_ROAD), "normal_road");
    scheme.addColor(RGBColor::BLACK, (double)(SVCAll & ~(SVC_PEDESTRIAN | SVC_NON_ROAD)), "disallow_pedestrian");
    scheme.addColor(RGBColor(255, 206, 0), (double)(SVCAll & ~(SVC_PEDESTRIAN | SVC_BICYCLE | SVC_MOPED | SVC_NON_ROAD)), "motorway");
    scheme.addColor(RGBColor(150, 200, 200), (double)SVC_SHIP, "waterway");
    scheme.addColor(RGBColor::GREEN, (double)SVCAll, "all");
    laneColorer.addScheme(scheme);

    scheme = GUIColorScheme("by allowed speed (lanewise)", RGBColor::RED);
    scheme.addColor(RGBColor::YELLOW, (double)(30 / 3.6));
    scheme.addColor(RGBColor::GREEN, (double)(55 / 3.6));
    scheme.addColor(RGBColor::CYAN, (double)(80 / 3.6));
    scheme.addColor(RGBColor::BLUE, (double)(120 / 3.6));
    scheme.addColor(RGBColor::MAGENTA, (double)(150 / 3.6));
    laneColorer.addScheme(scheme);

    scheme = GUIColorScheme("by lane number (streetwise)", RGBColor::RED);
    scheme.addColor(RGBColor::BLUE, (double)5);
    laneColorer.addScheme(scheme);

    scheme = GUIColorScheme("by given length/geometrical length", RGBColor::RED);
    scheme.addColor(RGBColor::ORANGE, 0.25);
    scheme.addColor(RGBColor::YELLOW, 0.5);
    scheme.addColor(RGBColor(179, 179, 179, 255), (double)1.0);
    scheme.addColor(RGBColor::GREEN, (double)2.0);
    scheme.addColor(RGBColor::CYAN, (double)4.0);
    scheme.addColor(RGBColor::BLUE, (double)100.0);
    laneColorer.addScheme(scheme);
    laneColorer.addScheme(GUIColorScheme("by angle", RGBColor::YELLOW, "", true));

    scheme = GUIColorScheme("by priority", RGBColor::YELLOW);
    scheme.addColor(RGBColor::RED, (double) - 20);
    scheme.addColor(RGBColor::GREEN, (double)20);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by height at start", RGBColor::RED);
    scheme.addColor(RGBColor::BLUE, (double) - 10);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::MAGENTA, (double)200);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by height at segment start", RGBColor::RED);
    scheme.addColor(RGBColor::BLUE, (double) - 10);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::MAGENTA, (double)200);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by inclination", RGBColor::GREY);
    scheme.addColor(RGBColor::YELLOW, (double) .1);
    scheme.addColor(RGBColor::RED, (double) .3);
    scheme.addColor(RGBColor::GREEN, (double) - .1);
    scheme.addColor(RGBColor::BLUE, (double) - .3);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by segment inclination", RGBColor::GREY);
    scheme.addColor(RGBColor::YELLOW, (double) .1);
    scheme.addColor(RGBColor::RED, (double) .3);
    scheme.addColor(RGBColor::GREEN, (double) - .1);
    scheme.addColor(RGBColor::BLUE, (double) - .3);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_EDGE_PARAM_NUMERICAL, RGBColor(204, 204, 204));
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_LANE_PARAM_NUMERICAL, RGBColor(204, 204, 204));
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by distance (kilometrage)", RGBColor(204, 204, 204));
    scheme.addColor(RGBColor::RED, (double)1);
    scheme.addColor(RGBColor::RED, (double) -1);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme("by abs distance (kilometrage)", RGBColor(204, 204, 204));
    scheme.addColor(RGBColor::RED, (double)1);
    scheme.setAllowsNegativeValues(false);
    laneColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_EDGEDATA_NUMERICAL, RGBColor(204, 204, 204), "missing data", false, MISSING_DATA);
    scheme.setAllowsNegativeValues(true);
    laneColorer.addScheme(scheme);

    scheme = GUIColorScheme("uniform", RGBColor(102, 0, 0), "", true);
    scheme.addColor(RGBColor(204, 0, 0), 1, "junction bubble");
    scheme.addColor(RGBColor(230, 100, 115), 2, "geometry points");
    scheme.addColor(GUIVisualizationColorSettings::SUMO_color_DEADEND_SHOW, 3, "connections missing");
    scheme.addColor(RGBColor::GREEN, 3, "custom shape");
    scheme.addColor(RGBColor(205, 180, 180), 4, "data mode");
    junctionColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(128, 128, 128, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 80, 180, 255), 1, "selected");
    junctionColorer.addScheme(scheme);
    scheme = GUIColorScheme(SCHEME_NAME_TYPE, RGBColor::GREEN, "traffic_light", true);
    scheme.addColor(RGBColor(0, 128, 0), 1, "traffic_light_unregulated");
    scheme.addColor(RGBColor::YELLOW, 2, "priority");
    scheme.addColor(RGBColor::RED, 3, "priority_stop");
    scheme.addColor(RGBColor::BLUE, 4, "right_before_left");
    scheme.addColor(RGBColor::CYAN, 5, "allway_stop");
    scheme.addColor(RGBColor::GREY, 6, "district");
    scheme.addColor(RGBColor::MAGENTA, 7, "unregulated");
    scheme.addColor(RGBColor::BLACK, 8, "dead_end");
    scheme.addColor(RGBColor::ORANGE, 9, "rail_signal");
    scheme.addColor(RGBColor(172, 108, 44), 10, "zipper"); // brown, darker than the zipper link rule
    scheme.addColor(RGBColor(192, 255, 192), 11, "traffic_light_right_on_red");
    scheme.addColor(RGBColor(128, 0, 128), 12, "rail_crossing"); // dark purple
    junctionColorer.addScheme(scheme);
    scheme = GUIColorScheme("by height", RGBColor::GREY);
    scheme.addColor(RGBColor::BLUE, (double) - 10);
    scheme.addColor(RGBColor::RED, (double)10);
    scheme.addColor(RGBColor::YELLOW, (double)50);
    scheme.addColor(RGBColor::GREEN, (double)100);
    scheme.addColor(RGBColor::MAGENTA, (double)200);
    scheme.setAllowsNegativeValues(true);
    junctionColorer.addScheme(scheme);

    /// add POI coloring schemes
    poiColorer.addScheme(GUIColorScheme("given POI color", RGBColor::RED, "", true));
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    poiColorer.addScheme(scheme);
    poiColorer.addScheme(GUIColorScheme("uniform", RGBColor::RED, "", true));

    /// add polygon coloring schemes
    polyColorer.addScheme(GUIColorScheme("given polygon color", RGBColor::ORANGE, "", true));
    scheme = GUIColorScheme(SCHEME_NAME_SELECTION, RGBColor(179, 179, 179, 255), "unselected", true, 0, COL_SCHEME_MISC);
    scheme.addColor(RGBColor(0, 102, 204, 255), 1, "selected");
    polyColorer.addScheme(scheme);
    polyColorer.addScheme(GUIColorScheme("uniform", RGBColor::ORANGE, "", true));

    /// add edge scaling schemes
    {
        GUIScaleScheme edgeScheme = GUIScaleScheme("default", 1, "uniform", true);
        laneScaler.addScheme(edgeScheme);
    }

    // dummy schemes
    vehicleColorer.addScheme(GUIColorScheme("uniform", RGBColor::YELLOW, "", true));
    personColorer.addScheme(GUIColorScheme("uniform", RGBColor::YELLOW, "", true));
    containerColorer.addScheme(GUIColorScheme("uniform", RGBColor::YELLOW, "", true));
}


int
GUIVisualizationSettings::getLaneEdgeMode() const {
    if (UseMesoSim) {
        return edgeColorer.getActive();
    }
    return laneColorer.getActive();
}


int
GUIVisualizationSettings::getLaneEdgeScaleMode() const {
    if (UseMesoSim) {
        return edgeScaler.getActive();
    }
    return laneScaler.getActive();
}


GUIColorScheme&
GUIVisualizationSettings::getLaneEdgeScheme() {
    if (UseMesoSim) {
        return edgeColorer.getScheme();
    }
    return laneColorer.getScheme();
}


GUIScaleScheme&
GUIVisualizationSettings::getLaneEdgeScaleScheme() {
    if (UseMesoSim) {
        return edgeScaler.getScheme();
    }
    return laneScaler.getScheme();
}


void
GUIVisualizationSettings::save(OutputDevice& dev) const {
    dev.openTag(SUMO_TAG_VIEWSETTINGS_SCHEME);
    dev.writeAttr(SUMO_ATTR_NAME, name);
    dev.openTag(SUMO_TAG_VIEWSETTINGS_OPENGL);
    dev.writeAttr("dither", dither);
    dev.writeAttr("fps", fps);
    dev.writeAttr("drawBoundaries", drawBoundaries);
    dev.writeAttr("forceDrawPositionSelection", forceDrawForPositionSelection);
    dev.writeAttr("forceDrawRectangleSelection", forceDrawForRectangleSelection);
    dev.writeAttr("forceDrawDottedContour", forceDrawDottedContour);
    dev.closeTag();
    dev.openTag(SUMO_TAG_VIEWSETTINGS_BACKGROUND);
    dev.writeAttr("backgroundColor", backgroundColor);
    dev.writeAttr("showGrid", showGrid);
    dev.writeAttr("gridXSize", gridXSize);
    dev.writeAttr("gridYSize", gridYSize);
    dev.closeTag();
    // edges
    dev.openTag(SUMO_TAG_VIEWSETTINGS_EDGES);
    dev.writeAttr("laneEdgeMode", getLaneEdgeMode());
    dev.writeAttr("scaleMode", getLaneEdgeScaleMode());
    dev.writeAttr("laneShowBorders", laneShowBorders);
    dev.writeAttr("showBikeMarkings", showBikeMarkings);
    dev.writeAttr("showLinkDecals", showLinkDecals);
    dev.writeAttr("showLinkRules", showLinkRules);
    dev.writeAttr("showRails", showRails);
    dev.writeAttr("hideConnectors", hideConnectors);
    dev.writeAttr("widthExaggeration", laneWidthExaggeration);
    dev.writeAttr("minSize", laneMinSize);
    dev.writeAttr("showDirection", showLaneDirection);
    dev.writeAttr("showSublanes", showSublanes);
    dev.writeAttr("spreadSuperposed", spreadSuperposed);
    dev.writeAttr("edgeParam", edgeParam);
    dev.writeAttr("laneParam", laneParam);
    dev.writeAttr("vehicleParam", vehicleParam);
    dev.writeAttr("vehicleTextParam", vehicleTextParam);
    dev.writeAttr("edgeData", edgeData);
    dev.writeAttr("edgeValueHideCheck", edgeValueHideCheck);
    dev.writeAttr("edgeValueHideThreshold", edgeValueHideThreshold);
    dev.lf();
    dev << "               ";
    edgeName.print(dev, "edgeName");
    dev.lf();
    dev << "               ";
    internalEdgeName.print(dev, "internalEdgeName");
    dev.lf();
    dev << "               ";
    cwaEdgeName.print(dev, "cwaEdgeName");
    dev.lf();
    dev << "               ";
    streetName.print(dev, "streetName");
    dev.lf();
    dev << "               ";
    edgeValue.print(dev, "edgeValue");
    laneColorer.save(dev);
    laneScaler.save(dev);
    edgeColorer.save(dev);
    edgeScaler.save(dev);
    dev.closeTag();
    // vehicles
    dev.openTag(SUMO_TAG_VIEWSETTINGS_VEHICLES);
    dev.writeAttr("vehicleMode", vehicleColorer.getActive());
    dev.writeAttr("vehicleQuality", vehicleQuality);
    vehicleSize.print(dev, "vehicle");
    dev.writeAttr("showBlinker", showBlinker);
    dev.writeAttr("drawMinGap", drawMinGap);
    dev.writeAttr("drawBrakeGap", drawBrakeGap);
    dev.writeAttr("showBTRange", showBTRange);
    dev.writeAttr("showRouteIndex", showRouteIndex);
    dev.lf();
    dev << "                 ";
    vehicleName.print(dev, "vehicleName");
    dev.lf();
    dev << "                 ";
    vehicleValue.print(dev, "vehicleValue");
    dev.lf();
    dev << "                 ";
    vehicleText.print(dev, "vehicleText");
    vehicleColorer.save(dev);
    dev.closeTag();
    // persons
    dev.openTag(SUMO_TAG_VIEWSETTINGS_PERSONS);
    dev.writeAttr("personMode", personColorer.getActive());
    dev.writeAttr("personQuality", personQuality);
    personSize.print(dev, "person");
    dev.lf();
    dev << "                ";
    personName.print(dev, "personName");
    dev.lf();
    dev << "                 ";
    personValue.print(dev, "personValue");
    personColorer.save(dev);
    dev.closeTag();
    // containers
    dev.openTag(SUMO_TAG_VIEWSETTINGS_CONTAINERS);
    dev.writeAttr("containerMode", containerColorer.getActive());
    dev.writeAttr("containerQuality", containerQuality);
    containerSize.print(dev, "container");
    dev.lf();
    dev << "                ";
    containerName.print(dev, "containerName");
    containerColorer.save(dev);
    dev.closeTag();
    // junctions
    dev.openTag(SUMO_TAG_VIEWSETTINGS_JUNCTIONS);
    dev.writeAttr("junctionMode", junctionColorer.getActive());
    dev.lf();
    dev << "                  ";
    drawLinkTLIndex.print(dev, "drawLinkTLIndex");
    dev.lf();
    dev << "                  ";
    drawLinkJunctionIndex.print(dev, "drawLinkJunctionIndex");
    dev.lf();
    dev << "                  ";
    junctionID.print(dev, "junctionID");
    dev.lf();
    dev << "                  ";
    junctionName.print(dev, "junctionName");
    dev.lf();
    dev << "                  ";
    internalJunctionName.print(dev, "internalJunctionName");
    dev.lf();
    dev << "                  ";
    tlsPhaseIndex.print(dev, "tlsPhaseIndex");
    tlsPhaseName.print(dev, "tlsPhaseName");
    dev.lf();
    dev << "                  ";
    dev.writeAttr("showLane2Lane", showLane2Lane);
    dev.writeAttr("drawShape", drawJunctionShape);
    dev.writeAttr("drawCrossingsAndWalkingareas", drawCrossingsAndWalkingareas);
    junctionSize.print(dev, "junction");
    junctionColorer.save(dev);
    dev.closeTag();
    // additionals
    dev.openTag(SUMO_TAG_VIEWSETTINGS_ADDITIONALS);
    dev.writeAttr("addMode", addMode);
    addSize.print(dev, "add");
    addName.print(dev, "addName");
    addFullName.print(dev, "addFullName");
    dev.closeTag();
    // pois
    dev.openTag(SUMO_TAG_VIEWSETTINGS_POIS);
    dev.writeAttr("poiTextParam", poiTextParam);
    poiSize.print(dev, "poi");
    poiName.print(dev, "poiName");
    poiType.print(dev, "poiType");
    poiText.print(dev, "poiText");
    poiColorer.save(dev);
    dev.closeTag();
    // polys
    dev.openTag(SUMO_TAG_VIEWSETTINGS_POLYS);
    polySize.print(dev, "poly");
    polyName.print(dev, "polyName");
    polyType.print(dev, "polyType");
    polyColorer.save(dev);
    dev.closeTag();
    // legend
    dev.openTag(SUMO_TAG_VIEWSETTINGS_LEGEND);
    dev.writeAttr("showSizeLegend", showSizeLegend);
    dev.writeAttr("showColorLegend", showColorLegend);
    dev.writeAttr("showVehicleColorLegend", showVehicleColorLegend);
    dev.closeTag();

    dev.closeTag();
}


bool
GUIVisualizationSettings::operator==(const GUIVisualizationSettings& v2) {
    if (dither != v2.dither) {
        return false;
    }
    if (fps != v2.fps) {
        return false;
    }
    if (drawBoundaries != v2.drawBoundaries) {
        return false;
    }
    if (forceDrawForPositionSelection != v2.forceDrawForPositionSelection) {
        return false;
    }
    if (forceDrawForRectangleSelection != v2.forceDrawForRectangleSelection) {
        return false;
    }
    if (forceDrawDottedContour != v2.forceDrawDottedContour) {
        return false;
    }
    if (backgroundColor != v2.backgroundColor) {
        return false;
    }
    if (colorSettings != v2.colorSettings) {
        return false;
    }
    if (showGrid != v2.showGrid) {
        return false;
    }
    if (gridXSize != v2.gridXSize) {
        return false;
    }
    if (gridYSize != v2.gridYSize) {
        return false;
    }

    if (!(edgeColorer == v2.edgeColorer)) {
        return false;
    }
    if (!(edgeScaler == v2.edgeScaler)) {
        return false;
    }
    if (!(laneColorer == v2.laneColorer)) {
        return false;
    }
    if (!(laneScaler == v2.laneScaler)) {
        return false;
    }
    if (laneShowBorders != v2.laneShowBorders) {
        return false;
    }
    if (showBikeMarkings != v2.showBikeMarkings) {
        return false;
    }
    if (showLinkDecals != v2.showLinkDecals) {
        return false;
    }
    if (showLinkRules != v2.showLinkRules) {
        return false;
    }
    if (showRails != v2.showRails) {
        return false;
    }
    if (edgeName != v2.edgeName) {
        return false;
    }
    if (internalEdgeName != v2.internalEdgeName) {
        return false;
    }
    if (cwaEdgeName != v2.cwaEdgeName) {
        return false;
    }
    if (streetName != v2.streetName) {
        return false;
    }
    if (edgeValue != v2.edgeValue) {
        return false;
    }
    if (hideConnectors != v2.hideConnectors) {
        return false;
    }
    if (laneWidthExaggeration != v2.laneWidthExaggeration) {
        return false;
    }
    if (laneMinSize != v2.laneMinSize) {
        return false;
    }
    if (showLaneDirection != v2.showLaneDirection) {
        return false;
    }
    if (showSublanes != v2.showSublanes) {
        return false;
    }
    if (spreadSuperposed != v2.spreadSuperposed) {
        return false;
    }
    if (edgeParam != v2.edgeParam) {
        return false;
    }
    if (laneParam != v2.laneParam) {
        return false;
    }
    if (vehicleParam != v2.vehicleParam) {
        return false;
    }
    if (vehicleTextParam != v2.vehicleTextParam) {
        return false;
    }
    if (edgeData != v2.edgeData) {
        return false;
    }
    if (edgeValueHideCheck != v2.edgeValueHideCheck) {
        return false;
    }
    if (edgeValueHideThreshold != v2.edgeValueHideThreshold) {
        return false;
    }
    if (!(vehicleColorer == v2.vehicleColorer)) {
        return false;
    }
    if (vehicleQuality != v2.vehicleQuality) {
        return false;
    }
    if (vehicleSize != v2.vehicleSize) {
        return false;
    }
    if (showBlinker != v2.showBlinker) {
        return false;
    }
    if (drawLaneChangePreference != v2.drawLaneChangePreference) {
        return false;
    }
    if (drawMinGap != v2.drawMinGap) {
        return false;
    }
    if (drawBrakeGap != v2.drawBrakeGap) {
        return false;
    }
    if (showBTRange != v2.showBTRange) {
        return false;
    }
    if (showRouteIndex != v2.showRouteIndex) {
        return false;
    }
    if (vehicleName != v2.vehicleName) {
        return false;
    }
    if (vehicleValue != v2.vehicleValue) {
        return false;
    }
    if (vehicleText != v2.vehicleText) {
        return false;
    }
    if (!(personColorer == v2.personColorer)) {
        return false;
    }
    if (personQuality != v2.personQuality) {
        return false;
    }
    if (personSize != v2.personSize) {
        return false;
    }
    if (personName != v2.personName) {
        return false;
    }
    if (personValue != v2.personValue) {
        return false;
    }
    if (!(containerColorer == v2.containerColorer)) {
        return false;
    }
    if (containerQuality != v2.containerQuality) {
        return false;
    }
    if (containerSize != v2.containerSize) {
        return false;
    }
    if (containerName != v2.containerName) {
        return false;
    }
    if (!(junctionColorer == v2.junctionColorer)) {
        return false;
    }
    if (!(poiColorer == v2.poiColorer)) {
        return false;
    }
    if (!(polyColorer == v2.polyColorer)) {
        return false;
    }
    if (drawLinkTLIndex != v2.drawLinkTLIndex) {
        return false;
    }
    if (drawLinkJunctionIndex != v2.drawLinkJunctionIndex) {
        return false;
    }
    if (junctionID != v2.junctionID) {
        return false;
    }
    if (junctionName != v2.junctionName) {
        return false;
    }
    if (internalJunctionName != v2.internalJunctionName) {
        return false;
    }
    if (tlsPhaseIndex != v2.tlsPhaseIndex) {
        return false;
    }
    if (tlsPhaseName != v2.tlsPhaseName) {
        return false;
    }

    if (showLane2Lane != v2.showLane2Lane) {
        return false;
    }

    if (drawJunctionShape != v2.drawJunctionShape) {
        return false;
    }

    if (drawCrossingsAndWalkingareas != v2.drawCrossingsAndWalkingareas) {
        return false;
    }
    if (junctionSize != v2.junctionSize) {
        return false;
    }

    if (addMode != v2.addMode) {
        return false;
    }
    if (addSize != v2.addSize) {
        return false;
    }
    if (addName != v2.addName) {
        return false;
    }
    if (addFullName != v2.addFullName) {
        return false;
    }
    if (poiSize != v2.poiSize) {
        return false;
    }
    if (poiName != v2.poiName) {
        return false;
    }
    if (poiType != v2.poiType) {
        return false;
    }
    if (poiText != v2.poiText) {
        return false;
    }
    if (poiTextParam != v2.poiTextParam) {
        return false;
    }
    if (polySize != v2.polySize) {
        return false;
    }
    if (polyName != v2.polyName) {
        return false;
    }
    if (polyType != v2.polyType) {
        return false;
    }

    if (showSizeLegend != v2.showSizeLegend) {
        return false;
    }
    if (showColorLegend != v2.showColorLegend) {
        return false;
    }
    if (showVehicleColorLegend != v2.showVehicleColorLegend) {
        return false;
    }

    return true;
}


const RGBColor&
GUIVisualizationSettings::getLinkColor(const LinkState& ls) {
    switch (ls) {
        case LINKSTATE_TL_GREEN_MAJOR:
            return SUMO_color_TL_GREEN_MAJOR;
        case LINKSTATE_TL_GREEN_MINOR:
            return SUMO_color_TL_GREEN_MINOR;
        case LINKSTATE_TL_RED:
            return SUMO_color_TL_RED;
        case LINKSTATE_TL_REDYELLOW:
            return SUMO_color_TL_REDYELLOW;
        case LINKSTATE_TL_YELLOW_MAJOR:
            return SUMO_color_TL_YELLOW_MAJOR;
        case LINKSTATE_TL_YELLOW_MINOR:
            return SUMO_color_TL_YELLOW_MINOR;
        case LINKSTATE_TL_OFF_BLINKING:
            return SUMO_color_TL_OFF_BLINKING;
        case LINKSTATE_TL_OFF_NOSIGNAL:
            return SUMO_color_TL_OFF_NOSIGNAL;
        case LINKSTATE_MAJOR:
            return SUMO_color_MAJOR;
        case LINKSTATE_MINOR:
            return SUMO_color_MINOR;
        case LINKSTATE_EQUAL:
            return SUMO_color_EQUAL;
        case LINKSTATE_STOP:
            return SUMO_color_STOP;
        case LINKSTATE_ALLWAY_STOP:
            return SUMO_color_ALLWAY_STOP;
        case LINKSTATE_ZIPPER:
            return SUMO_color_ZIPPER;
        case LINKSTATE_DEADEND:
            return SUMO_color_DEADEND;
        default:
            throw ProcessError("No color defined for LinkState '" + std::string(ls, 1) + "'");
    }
}


double
GUIVisualizationSettings::getTextAngle(double objectAngle) const {
    double viewAngle = objectAngle - angle;
    while (viewAngle < 0) {
        viewAngle += 360;
    }
    // fmod round towards zero which is not want we want for negative numbers
    viewAngle = fmod(viewAngle, 360);
    if (viewAngle > 90 && viewAngle < 270) {
        // avoid reading text on it's head
        objectAngle -= 180;
    }
    return objectAngle;
}

bool
GUIVisualizationSettings::flippedTextAngle(double objectAngle) const {
    double viewAngle = objectAngle - angle;
    while (viewAngle < 0) {
        viewAngle += 360;
    }
    // fmod round towards zero which is not want we want for negative numbers
    viewAngle = fmod(viewAngle, 360);
    return (viewAngle > 90 && viewAngle < 270);
}



bool
GUIVisualizationSettings::drawAdditionals(const double exaggeration) const {
    return (scale * exaggeration) > 1.;
}


bool
GUIVisualizationSettings::drawDetail(const double detail, const double exaggeration) const {
    if (detail <= 0) {
        return true;
    } else {
        return ((scale * exaggeration) >= detail);
    }
}


int
GUIVisualizationSettings::getCircleResolution() const {
    if (drawForPositionSelection || drawForRectangleSelection) {
        return 8;
    } else if (scale >= 10) {
        return 32;
    } else if (scale >= 2) {
        return 16;
    } else {
        return 8;
    }
}


bool
GUIVisualizationSettings::drawDottedContour() const {
    if (drawForPositionSelection || drawForRectangleSelection) {
        return false;
    } else {
        return forceDrawDottedContour;
    }
}


bool
GUIVisualizationSettings::drawMovingGeometryPoint(const double exaggeration, const double radius) const {
    return (scale * radius * exaggeration > 10);
}

/****************************************************************************/
