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
/// @file    GUISettingsHandler.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Laura Bieker
/// @date    Fri, 24. Apr 2009
///
// The dialog to change the view (gui) settings.
/****************************************************************************/
#include <config.h>

#include <vector>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/RGBColor.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/FileHelpers.h>
#include <utils/gui/settings/GUIVisualizationSettings.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include <utils/gui/windows/GUIPerspectiveChanger.h>
#include <utils/foxtools/MFXImageHelper.h>
#include <utils/xml/SUMOSAXReader.h>
#include <utils/xml/XMLSubSys.h>
#include "GUISettingsHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
GUISettingsHandler::GUISettingsHandler(const std::string& content, bool isFile, bool netedit) :
    SUMOSAXHandler(content),
    mySettings(netedit),
    myDelay(-1), myLookFrom(-1, -1, -1), myLookAt(-1, -1, -1),
    myRotation(0),
    myCurrentColorer(SUMO_TAG_NOTHING),
    myCurrentScheme(nullptr),
    myJamSoundTime(-1) {
    if (isFile) {
        XMLSubSys::runParser(*this, content);
    } else {
        setFileName("registrySettings");
        SUMOSAXReader* reader = XMLSubSys::getSAXReader(*this);
        reader->parseString(content);
        delete reader;
    }
}


GUISettingsHandler::~GUISettingsHandler() {
}


void
GUISettingsHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    bool ok = true;
    switch (element) {
        case SUMO_TAG_BREAKPOINT:
            if (attrs.hasAttribute(SUMO_ATTR_TIME)) {
                myBreakpoints.push_back(attrs.getSUMOTimeReporting(SUMO_ATTR_TIME, nullptr, ok));
            } else {
                myBreakpoints.push_back(attrs.getSUMOTimeReporting(SUMO_ATTR_VALUE, nullptr, ok));
                WRITE_WARNING("The 'value' attribute is deprecated for breakpoints. Please use 'time'.");
            }
            break;
        case SUMO_TAG_VIEWSETTINGS:
            myViewType = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, nullptr, ok, "default");
            std::transform(myViewType.begin(), myViewType.end(), myViewType.begin(), tolower);
            break;
        case SUMO_TAG_DELAY:
            myDelay = attrs.getOpt<double>(SUMO_ATTR_VALUE, nullptr, ok, myDelay);
            break;
        case SUMO_TAG_VIEWPORT: {
            const double x = attrs.getOpt<double>(SUMO_ATTR_X, nullptr, ok, myLookFrom.x());
            const double y = attrs.getOpt<double>(SUMO_ATTR_Y, nullptr, ok, myLookFrom.y());
            const double z = attrs.getOpt<double>(SUMO_ATTR_ZOOM, nullptr, ok, myLookFrom.z());
            myLookFrom.set(x, y, z);
            const double cx = attrs.getOpt<double>(SUMO_ATTR_CENTER_X, nullptr, ok, myLookAt.x());
            const double cy = attrs.getOpt<double>(SUMO_ATTR_CENTER_Y, nullptr, ok, myLookAt.y());
            const double cz = attrs.getOpt<double>(SUMO_ATTR_CENTER_Z, nullptr, ok, myLookAt.z());
            myLookAt.set(cx, cy, cz);
            myRotation = attrs.getOpt<double>(SUMO_ATTR_ANGLE, nullptr, ok, myRotation);
            break;
        }
        case SUMO_TAG_SNAPSHOT: {
            std::string file = attrs.get<std::string>(SUMO_ATTR_FILE, nullptr, ok);
            if (file != "" && !FileHelpers::isAbsolute(file)) {
                file = FileHelpers::getConfigurationRelative(getFileName(), file);
            }
            mySnapshots[attrs.getOptSUMOTimeReporting(SUMO_ATTR_TIME, file.c_str(), ok, 0)].push_back(file);
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_SCHEME: {
            mySettings.name = attrs.getOpt<std::string>(SUMO_ATTR_NAME, nullptr, ok, mySettings.name);
            if (gSchemeStorage.contains(mySettings.name)) {
                mySettings = gSchemeStorage.get(mySettings.name);
            }
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_OPENGL:
            mySettings.dither = StringUtils::toBool(attrs.getStringSecure("dither", toString(mySettings.dither)));
            mySettings.fps = StringUtils::toBool(attrs.getStringSecure("fps", toString(mySettings.fps)));
            mySettings.drawBoundaries = StringUtils::toBool(attrs.getStringSecure("drawBoundaries", toString(mySettings.drawBoundaries)));
            mySettings.forceDrawForRectangleSelection = StringUtils::toBool(attrs.getStringSecure("forceDrawRectangleSelection", toString(mySettings.forceDrawForRectangleSelection)));
            mySettings.forceDrawForPositionSelection = StringUtils::toBool(attrs.getStringSecure("forceDrawPositionSelection", toString(mySettings.forceDrawForPositionSelection)));
            break;
        case SUMO_TAG_VIEWSETTINGS_BACKGROUND:
            mySettings.backgroundColor = RGBColor::parseColorReporting(attrs.getStringSecure("backgroundColor", toString(mySettings.backgroundColor)), "background", nullptr, true, ok);
            mySettings.showGrid = StringUtils::toBool(attrs.getStringSecure("showGrid", toString(mySettings.showGrid)));
            mySettings.gridXSize = StringUtils::toDouble(attrs.getStringSecure("gridXSize", toString(mySettings.gridXSize)));
            mySettings.gridYSize = StringUtils::toDouble(attrs.getStringSecure("gridYSize", toString(mySettings.gridYSize)));
            break;
        case SUMO_TAG_VIEWSETTINGS_EDGES: {
            int laneEdgeMode = StringUtils::toInt(attrs.getStringSecure("laneEdgeMode", "0"));
            int laneEdgeScaleMode = StringUtils::toInt(attrs.getStringSecure("scaleMode", "0"));
            mySettings.laneShowBorders = StringUtils::toBool(attrs.getStringSecure("laneShowBorders", toString(mySettings.laneShowBorders)));
            mySettings.showBikeMarkings = StringUtils::toBool(attrs.getStringSecure("showBikeMarkings", toString(mySettings.showBikeMarkings)));
            mySettings.showLinkDecals = StringUtils::toBool(attrs.getStringSecure("showLinkDecals", toString(mySettings.showLinkDecals)));
            mySettings.showLinkRules = StringUtils::toBool(attrs.getStringSecure("showLinkRules", toString(mySettings.showLinkRules)));
            mySettings.showRails = StringUtils::toBool(attrs.getStringSecure("showRails", toString(mySettings.showRails)));
            mySettings.edgeName = parseTextSettings("edgeName", attrs, mySettings.edgeName);
            mySettings.internalEdgeName = parseTextSettings("internalEdgeName", attrs, mySettings.internalEdgeName);
            mySettings.cwaEdgeName = parseTextSettings("cwaEdgeName", attrs, mySettings.cwaEdgeName);
            mySettings.streetName = parseTextSettings("streetName", attrs, mySettings.streetName);
            mySettings.edgeValue = parseTextSettings("edgeValue", attrs, mySettings.edgeValue);
            mySettings.hideConnectors = StringUtils::toBool(attrs.getStringSecure("hideConnectors", toString(mySettings.hideConnectors)));
            mySettings.laneWidthExaggeration = StringUtils::toDouble(attrs.getStringSecure("widthExaggeration", toString(mySettings.laneWidthExaggeration)));
            mySettings.laneMinSize = StringUtils::toDouble(attrs.getStringSecure("minSize", toString(mySettings.laneWidthExaggeration)));
            mySettings.showLaneDirection = StringUtils::toBool(attrs.getStringSecure("showDirection", toString(mySettings.showLaneDirection)));
            mySettings.showSublanes = StringUtils::toBool(attrs.getStringSecure("showSublanes", toString(mySettings.showSublanes)));
            mySettings.spreadSuperposed = StringUtils::toBool(attrs.getStringSecure("spreadSuperposed", toString(mySettings.spreadSuperposed)));
            mySettings.edgeParam = attrs.getStringSecure("edgeParam", mySettings.edgeParam);
            mySettings.laneParam = attrs.getStringSecure("laneParam", mySettings.laneParam);
            mySettings.vehicleParam = attrs.getStringSecure("vehicleParam", mySettings.vehicleParam);
            mySettings.vehicleTextParam = attrs.getStringSecure("vehicleTextParam", mySettings.vehicleTextParam);
            mySettings.edgeData = attrs.getStringSecure("edgeData", mySettings.edgeData);
            mySettings.edgeValueHideCheck = StringUtils::toBool(attrs.getStringSecure("edgeValueHideCheck", toString(mySettings.edgeValueHideCheck)));
            mySettings.edgeValueHideThreshold = StringUtils::toDouble(attrs.getStringSecure("edgeValueHideThreshold", toString(mySettings.edgeValueHideThreshold)));
            myCurrentColorer = element;
            mySettings.edgeColorer.setActive(laneEdgeMode);
            mySettings.edgeScaler.setActive(laneEdgeScaleMode);
            mySettings.laneColorer.setActive(laneEdgeMode);
            mySettings.laneScaler.setActive(laneEdgeScaleMode);
        }
        break;
        case SUMO_TAG_COLORSCHEME:
            myCurrentScheme = nullptr;
            myCurrentScaleScheme = nullptr;
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_EDGES) {
                myCurrentScheme = mySettings.laneColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
                if (myCurrentScheme == nullptr) {
                    myCurrentScheme = mySettings.edgeColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
                }
            }
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_VEHICLES) {
                myCurrentScheme = mySettings.vehicleColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
            }
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_PERSONS) {
                myCurrentScheme = mySettings.personColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
            }
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_JUNCTIONS) {
                myCurrentScheme = mySettings.junctionColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
            }
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_POIS) {
                myCurrentScheme = mySettings.poiColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
            }
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_POLYS) {
                myCurrentScheme = mySettings.polyColorer.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
            }
            if (myCurrentScheme && !myCurrentScheme->isFixed()) {
                myCurrentScheme->setInterpolated(attrs.getOpt<bool>(SUMO_ATTR_INTERPOLATED, nullptr, ok, false));
                myCurrentScheme->clear();
            }
            break;
        case SUMO_TAG_SCALINGSCHEME:
            myCurrentScheme = nullptr;
            myCurrentScaleScheme = nullptr;
            if (myCurrentColorer == SUMO_TAG_VIEWSETTINGS_EDGES) {
                myCurrentScaleScheme = mySettings.laneScaler.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
                if (myCurrentScaleScheme == nullptr) {
                    myCurrentScaleScheme = mySettings.edgeScaler.getSchemeByName(attrs.getStringSecure(SUMO_ATTR_NAME, ""));
                }
            }
            if (myCurrentScaleScheme && !myCurrentScaleScheme->isFixed()) {
                myCurrentScaleScheme->setInterpolated(attrs.getOpt<bool>(SUMO_ATTR_INTERPOLATED, nullptr, ok, false));
                myCurrentScaleScheme->clear();
            }
            break;
        case SUMO_TAG_ENTRY:
            if (myCurrentScheme != nullptr) {
                RGBColor color = attrs.get<RGBColor>(SUMO_ATTR_COLOR, nullptr, ok);
                if (myCurrentScheme->isFixed()) {
                    myCurrentScheme->setColor(attrs.getStringSecure(SUMO_ATTR_NAME, ""), color);
                } else {
                    myCurrentScheme->addColor(color, attrs.getOpt<double>(SUMO_ATTR_THRESHOLD, nullptr, ok, std::numeric_limits<double>::max()));
                }
            } else if (myCurrentScaleScheme != nullptr) {
                double scale = attrs.get<double>(SUMO_ATTR_COLOR, nullptr, ok);
                if (myCurrentScaleScheme->isFixed()) {
                    myCurrentScaleScheme->setColor(attrs.getStringSecure(SUMO_ATTR_NAME, ""), scale);
                } else {
                    myCurrentScaleScheme->addColor(scale, attrs.getOpt<double>(SUMO_ATTR_THRESHOLD, nullptr, ok, std::numeric_limits<double>::max()));
                }
            }
            break;
        case SUMO_TAG_VIEWSETTINGS_VEHICLES:
            mySettings.vehicleColorer.setActive(StringUtils::toInt(attrs.getStringSecure("vehicleMode", "0")));
            mySettings.vehicleQuality = StringUtils::toInt(attrs.getStringSecure("vehicleQuality", toString(mySettings.vehicleQuality)));
            mySettings.showBlinker = StringUtils::toBool(attrs.getStringSecure("showBlinker", toString(mySettings.showBlinker)));
            mySettings.drawMinGap = StringUtils::toBool(attrs.getStringSecure("drawMinGap", toString(mySettings.drawMinGap)));
            mySettings.drawBrakeGap = StringUtils::toBool(attrs.getStringSecure("drawBrakeGap", toString(mySettings.drawBrakeGap)));
            mySettings.showBTRange = StringUtils::toBool(attrs.getStringSecure("showBTRange", toString(mySettings.showBTRange)));
            mySettings.showRouteIndex = StringUtils::toBool(attrs.getStringSecure("showRouteIndex", toString(mySettings.showRouteIndex)));
            mySettings.vehicleSize = parseSizeSettings("vehicle", attrs, mySettings.vehicleSize);
            mySettings.vehicleName = parseTextSettings("vehicleName", attrs, mySettings.vehicleName);
            mySettings.vehicleValue = parseTextSettings("vehicleValue", attrs, mySettings.vehicleValue);
            mySettings.vehicleText = parseTextSettings("vehicleText", attrs, mySettings.vehicleText);
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_PERSONS:
            mySettings.personColorer.setActive(StringUtils::toInt(attrs.getStringSecure("personMode", "0")));
            mySettings.personQuality = StringUtils::toInt(attrs.getStringSecure("personQuality", toString(mySettings.personQuality)));
            mySettings.personSize = parseSizeSettings("person", attrs, mySettings.personSize);
            mySettings.personName = parseTextSettings("personName", attrs, mySettings.personName);
            mySettings.personValue = parseTextSettings("personValue", attrs, mySettings.personValue);
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_CONTAINERS:
            mySettings.containerColorer.setActive(StringUtils::toInt(attrs.getStringSecure("containerMode", "0")));
            mySettings.containerQuality = StringUtils::toInt(attrs.getStringSecure("containerQuality", toString(mySettings.containerQuality)));
            mySettings.containerSize = parseSizeSettings("container", attrs, mySettings.containerSize);
            mySettings.containerName = parseTextSettings("containerName", attrs, mySettings.containerName);
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_JUNCTIONS:
            mySettings.junctionColorer.setActive(StringUtils::toInt(attrs.getStringSecure("junctionMode", "0")));
            mySettings.drawLinkTLIndex = parseTextSettings("drawLinkTLIndex", attrs, mySettings.drawLinkTLIndex);
            mySettings.drawLinkJunctionIndex = parseTextSettings("drawLinkJunctionIndex", attrs, mySettings.drawLinkJunctionIndex);
            mySettings.junctionID = parseTextSettings("junctionID", attrs, mySettings.junctionID);
            mySettings.junctionName = parseTextSettings("junctionName", attrs, mySettings.junctionName);
            mySettings.internalJunctionName = parseTextSettings("internalJunctionName", attrs, mySettings.internalJunctionName);
            mySettings.tlsPhaseIndex = parseTextSettings("tlsPhaseIndex", attrs, mySettings.tlsPhaseIndex);
            mySettings.tlsPhaseName = parseTextSettings("tlsPhaseName", attrs, mySettings.tlsPhaseName);
            mySettings.showLane2Lane = StringUtils::toBool(attrs.getStringSecure("showLane2Lane", toString(mySettings.showLane2Lane)));
            mySettings.drawJunctionShape = StringUtils::toBool(attrs.getStringSecure("drawShape", toString(mySettings.drawJunctionShape)));
            mySettings.drawCrossingsAndWalkingareas = StringUtils::toBool(attrs.getStringSecure(
                        "drawCrossingsAndWalkingareas", toString(mySettings.drawCrossingsAndWalkingareas)));
            mySettings.junctionSize = parseSizeSettings("junction", attrs, mySettings.junctionSize);
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_ADDITIONALS:
            mySettings.addMode = StringUtils::toInt(attrs.getStringSecure("addMode", toString(mySettings.addMode)));
            mySettings.addSize = parseSizeSettings("add", attrs, mySettings.addSize);
            mySettings.addName = parseTextSettings("addName", attrs, mySettings.addName);
            mySettings.addFullName = parseTextSettings("addFullName", attrs, mySettings.addFullName);
            break;
        case SUMO_TAG_VIEWSETTINGS_POIS:
            mySettings.poiTextParam = attrs.getStringSecure("poiTextParam", mySettings.poiTextParam);
            mySettings.poiSize = parseSizeSettings("poi", attrs, mySettings.poiSize);
            mySettings.poiName = parseTextSettings("poiName", attrs, mySettings.poiName);
            mySettings.poiType = parseTextSettings("poiType", attrs, mySettings.poiType);
            mySettings.poiText = parseTextSettings("poiText", attrs, mySettings.poiText);
            mySettings.poiColorer.setActive(StringUtils::toInt(attrs.getStringSecure("personMode", "0")));
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_POLYS:
            mySettings.polySize = parseSizeSettings("poly", attrs, mySettings.polySize);
            mySettings.polyName = parseTextSettings("polyName", attrs, mySettings.polyName);
            mySettings.polyType = parseTextSettings("polyType", attrs, mySettings.polyType);
            mySettings.polyColorer.setActive(StringUtils::toInt(attrs.getStringSecure("personMode", "0")));
            myCurrentColorer = element;
            break;
        case SUMO_TAG_VIEWSETTINGS_LEGEND:
            mySettings.showSizeLegend = StringUtils::toBool(attrs.getStringSecure("showSizeLegend", toString(mySettings.showSizeLegend)));
            mySettings.showColorLegend = StringUtils::toBool(attrs.getStringSecure("showColorLegend", toString(mySettings.showColorLegend)));
            mySettings.showVehicleColorLegend = StringUtils::toBool(attrs.getStringSecure("showVehicleColorLegend", toString(mySettings.showVehicleColorLegend)));
            break;
        case SUMO_TAG_VIEWSETTINGS_DECAL: {
            GUISUMOAbstractView::Decal d;
            if (attrs.hasAttribute(SUMO_ATTR_FILE)) {
                d.filename = attrs.get<std::string>(SUMO_ATTR_FILE, nullptr, ok);
            } else {
                d.filename = attrs.getStringSecure("filename", d.filename);
                WRITE_WARNING("The 'filename' attribute is deprecated for decals. Please use 'file'.");
            }
            if (d.filename != "" && !FileHelpers::isAbsolute(d.filename)) {
                d.filename = FileHelpers::getConfigurationRelative(getFileName(), d.filename);
            }
            d.centerX = attrs.getOpt<double>(SUMO_ATTR_CENTER_X, nullptr, ok, d.centerX);
            d.centerY = attrs.getOpt<double>(SUMO_ATTR_CENTER_Y, nullptr, ok, d.centerY);
            d.centerZ = attrs.getOpt<double>(SUMO_ATTR_CENTER_Z, nullptr, ok, d.centerZ);
            d.width = attrs.getOpt<double>(SUMO_ATTR_WIDTH, nullptr, ok, d.width);
            d.height = attrs.getOpt<double>(SUMO_ATTR_HEIGHT, nullptr, ok, d.height);
            d.altitude = StringUtils::toDouble(attrs.getStringSecure("altitude", toString(d.height)));
            d.rot = StringUtils::toDouble(attrs.getStringSecure("rotation", toString(d.rot)));
            d.tilt = StringUtils::toDouble(attrs.getStringSecure("tilt", toString(d.tilt)));
            d.roll = StringUtils::toDouble(attrs.getStringSecure("roll", toString(d.roll)));
            d.layer = attrs.getOpt<double>(SUMO_ATTR_LAYER, nullptr, ok, d.layer);
            d.screenRelative = StringUtils::toBool(attrs.getStringSecure("screenRelative", toString(d.screenRelative)));
            d.initialised = false;
            myDecals.push_back(d);
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_LIGHT: {
            GUISUMOAbstractView::Decal d;
            d.filename = "light" + attrs.getOpt<std::string>(SUMO_ATTR_INDEX, nullptr, ok, "0");
            d.centerX = attrs.getOpt<double>(SUMO_ATTR_CENTER_X, nullptr, ok, d.centerX);
            d.centerY = attrs.getOpt<double>(SUMO_ATTR_CENTER_Y, nullptr, ok, d.centerY);
            d.centerZ = attrs.getOpt<double>(SUMO_ATTR_CENTER_Z, nullptr, ok, d.centerZ);
            d.width = attrs.getOpt<double>(SUMO_ATTR_WIDTH, nullptr, ok, d.width);
            d.height = attrs.getOpt<double>(SUMO_ATTR_HEIGHT, nullptr, ok, d.height);
            d.altitude = StringUtils::toDouble(attrs.getStringSecure("altitude", toString(d.height)));
            d.rot = StringUtils::toDouble(attrs.getStringSecure("rotation", toString(d.rot)));
            d.tilt = StringUtils::toDouble(attrs.getStringSecure("tilt", toString(d.tilt)));
            d.roll = StringUtils::toDouble(attrs.getStringSecure("roll", toString(d.roll)));
            d.layer = attrs.getOpt<double>(SUMO_ATTR_LAYER, nullptr, ok, d.layer);
            d.initialised = false;
            myDecals.push_back(d);
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_EVENT: {
            const std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            const std::string cmd = attrs.get<std::string>(SUMO_ATTR_COMMAND, nullptr, ok);
            const double prob = attrs.get<double>(SUMO_ATTR_PROB, id.c_str(), ok);
            myEventDistributions[id].add(cmd, prob);
        }
        break;
        case SUMO_TAG_VIEWSETTINGS_EVENT_JAM_TIME:
            myJamSoundTime = attrs.get<double>(SUMO_ATTR_VALUE, nullptr, ok);
            break;
        default:
            break;
    }
}


GUIVisualizationTextSettings
GUISettingsHandler::parseTextSettings(
    const std::string& prefix, const SUMOSAXAttributes& attrs,
    GUIVisualizationTextSettings defaults) {
    bool ok = true;
    return GUIVisualizationTextSettings(
               StringUtils::toBool(attrs.getStringSecure(prefix + "_show", toString(defaults.show))),
               StringUtils::toDouble(attrs.getStringSecure(prefix + "_size", toString(defaults.size))),
               RGBColor::parseColorReporting(attrs.getStringSecure(prefix + "_color", toString(defaults.color)), "textSettings", nullptr, true, ok),
               RGBColor::parseColorReporting(attrs.getStringSecure(prefix + "_bgColor", toString(defaults.bgColor)), "textSettings", nullptr, true, ok),
               StringUtils::toBool(attrs.getStringSecure(prefix + "_constantSize", toString(defaults.constSize))));
}


GUIVisualizationSizeSettings
GUISettingsHandler::parseSizeSettings(
    const std::string& prefix, const SUMOSAXAttributes& attrs,
    GUIVisualizationSizeSettings defaults) {
    return GUIVisualizationSizeSettings(
               StringUtils::toDouble(attrs.getStringSecure(prefix + "_minSize", toString(defaults.minSize))),
               StringUtils::toDouble(attrs.getStringSecure(prefix + "_exaggeration", toString(defaults.exaggeration))),
               StringUtils::toBool(attrs.getStringSecure(prefix + "_constantSize", toString(defaults.constantSize))),
               StringUtils::toBool(attrs.getStringSecure(prefix + "_constantSizeSelected", toString(defaults.constantSizeSelected))));
}


std::string
GUISettingsHandler::addSettings(GUISUMOAbstractView* view) const {
    if (mySettings.name != "") {
        gSchemeStorage.add(mySettings);
        if (view) {
            FXint index = view->getColoringSchemesCombo()->appendItem(mySettings.name.c_str());
            view->getColoringSchemesCombo()->setCurrentItem(index);
            view->setColorScheme(mySettings.name);
        }
    }
    return mySettings.name;
}


void
GUISettingsHandler::applyViewport(GUISUMOAbstractView* view) const {
    if (myLookFrom.z() > 0) {
        // z value stores zoom so we must convert first
        Position lookFrom(myLookFrom.x(), myLookFrom.y(), view->getChanger().zoom2ZPos(myLookFrom.z()));
        view->setViewportFromToRot(lookFrom, myLookAt, myRotation);
    }
}


void
GUISettingsHandler::setSnapshots(GUISUMOAbstractView* view) const {
    if (!mySnapshots.empty()) {
        for (auto item : mySnapshots) {
            for (auto file : item.second) {
                view->addSnapshot(item.first, file);
            }
        }
    }
}


bool
GUISettingsHandler::hasDecals() const {
    return !myDecals.empty();
}


const std::vector<GUISUMOAbstractView::Decal>&
GUISettingsHandler::getDecals() const {
    return myDecals;
}


double
GUISettingsHandler::getDelay() const {
    return myDelay;
}


std::vector<SUMOTime>
GUISettingsHandler::loadBreakpoints(const std::string& file) {
    std::vector<SUMOTime> result;
    std::ifstream strm(file.c_str());
    while (strm.good()) {
        std::string val;
        strm >> val;
        if (val.length() == 0) {
            continue;
        }
        try {
            SUMOTime value = string2time(val);
            result.push_back(value);
        } catch (NumberFormatException& e) {
            WRITE_ERROR(" A breakpoint-value must be an int. " + toString(e.what()));
        } catch (EmptyData&) {
        } catch (ProcessError&) {
            WRITE_ERROR(" Could not decode breakpoint '" + val + "'");
        }
    }
    return result;
}


RandomDistributor<std::string>
GUISettingsHandler::getEventDistribution(const std::string& id) {
    RandomDistributor<std::string> result = myEventDistributions[id];
    if (result.getOverallProb() > 0 && result.getOverallProb() < 1) {
        // unscaled probabilities are assumed, fill up with dummy event
        result.add("", 1. - result.getOverallProb());
    }
    return result;
}


/****************************************************************************/
