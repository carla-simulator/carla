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
/// @file    ShapeHandler.cpp
/// @author  Jakob Erdmann
/// @date    Feb 2015
///
// The XML-Handler for network loading
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/RGBColor.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/UtilExceptions.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>

#include "Shape.h"
#include "ShapeContainer.h"
#include "ShapeHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================

ShapeHandler::ShapeHandler(const std::string& file, ShapeContainer& sc, const GeoConvHelper* geoConvHelper) :
    SUMOSAXHandler(file),
    myShapeContainer(sc),
    myPrefix(""),
    myDefaultColor(RGBColor::RED),
    myDefaultLayer(0),
    myDefaultFill(false),
    myLastParameterised(nullptr),
    myGeoConvHelper(geoConvHelper) {
}


ShapeHandler::~ShapeHandler() {}


void
ShapeHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    try {
        switch (element) {
            case SUMO_TAG_POLY:
                // default layer is different depending if we're parsing a Poly or a POI, therefore it has to be here defined
                myDefaultLayer = Shape::DEFAULT_LAYER;
                addPoly(attrs, false, false);
                break;
            case SUMO_TAG_POI:
                // default layer is different depending if we're parsing a Poly or a POI, therefore it has to be here defined
                myDefaultLayer = Shape::DEFAULT_LAYER_POI;
                addPOI(attrs, false, false);
                break;
            case SUMO_TAG_PARAM:
                if (myLastParameterised != nullptr) {
                    bool ok = true;
                    const std::string key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
                    // continue if key awas sucesfully loaded
                    if (ok) {
                        // circumventing empty string value
                        const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
                        // show warnings if values are invalid
                        if (key.empty()) {
                            WRITE_WARNING("Error parsing key from shape generic parameter. Key cannot be empty");
                        } else if (!SUMOXMLDefinitions::isValidParameterKey(key)) {
                            WRITE_WARNING("Error parsing key from shape generic parameter. Key contains invalid characters");
                        } else if (!SUMOXMLDefinitions::isValidParameterValue(val)) {
                            WRITE_WARNING("Error parsing value from shape generic parameter. Value contains invalid characters");
                        } else {
                            WRITE_DEBUG("Inserting generic parameter '" + key + "|" + val + "' into shape.");
                            myLastParameterised->setParameter(key, val);
                        }
                    }
                }
            default:
                break;
        }
    } catch (InvalidArgument& e) {
        WRITE_ERROR(e.what());
    }
}


void
ShapeHandler::myEndElement(int element) {
    if (element != SUMO_TAG_PARAM) {
        myLastParameterised = nullptr;
    }
}


void
ShapeHandler::addPOI(const SUMOSAXAttributes& attrs, const bool ignorePruning, const bool useProcessing) {
    bool ok = true;
    const double INVALID_POSITION(-1000000);
    const std::string id = myPrefix + attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    double x = attrs.getOpt<double>(SUMO_ATTR_X, id.c_str(), ok, INVALID_POSITION);
    const double y = attrs.getOpt<double>(SUMO_ATTR_Y, id.c_str(), ok, INVALID_POSITION);
    double lon = attrs.getOpt<double>(SUMO_ATTR_LON, id.c_str(), ok, INVALID_POSITION);
    double lat = attrs.getOpt<double>(SUMO_ATTR_LAT, id.c_str(), ok, INVALID_POSITION);
    const double lanePos = attrs.getOpt<double>(SUMO_ATTR_POSITION, id.c_str(), ok, 0);
    const double lanePosLat = attrs.getOpt<double>(SUMO_ATTR_POSITION_LAT, id.c_str(), ok, 0);
    const double layer = attrs.getOpt<double>(SUMO_ATTR_LAYER, id.c_str(), ok, myDefaultLayer);
    const std::string type = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, id.c_str(), ok, "");
    const std::string laneID = attrs.getOpt<std::string>(SUMO_ATTR_LANE, id.c_str(), ok, "");
    const RGBColor color = attrs.hasAttribute(SUMO_ATTR_COLOR) ? attrs.get<RGBColor>(SUMO_ATTR_COLOR, id.c_str(), ok) : myDefaultColor;
    const double angle = attrs.getOpt<double>(SUMO_ATTR_ANGLE, id.c_str(), ok, Shape::DEFAULT_ANGLE);
    std::string imgFile = attrs.getOpt<std::string>(SUMO_ATTR_IMGFILE, id.c_str(), ok, Shape::DEFAULT_IMG_FILE);
    bool relativePath = attrs.getOpt<bool>(SUMO_ATTR_RELATIVEPATH, id.c_str(), ok, Shape::DEFAULT_RELATIVEPATH);
    if (imgFile != "" && !FileHelpers::isAbsolute(imgFile)) {
        imgFile = FileHelpers::getConfigurationRelative(getFileName(), imgFile);
    }
    const double width = attrs.getOpt<double>(SUMO_ATTR_WIDTH, id.c_str(), ok, Shape::DEFAULT_IMG_WIDTH);
    const double height = attrs.getOpt<double>(SUMO_ATTR_HEIGHT, id.c_str(), ok, Shape::DEFAULT_IMG_HEIGHT);
    // check if ID is valid
    if (SUMOXMLDefinitions::isValidTypeID(id) == false) {
        WRITE_WARNING("Invalid characters for PoI ID");
        ok = false;
    }
    // continue
    if (ok) {
        const GeoConvHelper* gch;
        // set GEOConverter
        if (myGeoConvHelper != nullptr) {
            gch = myGeoConvHelper;
        } else if (useProcessing) {
            gch = &GeoConvHelper::getProcessing();
        } else {
            gch = &GeoConvHelper::getFinal();
        }
        // check if GEOProjection has to be used
        if (useProcessing && gch->usingGeoProjection()) {
            if ((lat == INVALID_POSITION) || (lon == INVALID_POSITION)) {
                lon = x;
                lat = y;
                x = INVALID_POSITION;
            }
        }
        Position pos(x, y);
        bool useGeo = false;
        if ((x == INVALID_POSITION) || (y == INVALID_POSITION)) {
            // try computing x,y from lane,pos
            if (laneID != "") {
                pos = getLanePos(id, laneID, lanePos, lanePosLat);
            } else {
                // try computing x,y from lon,lat
                if ((lat == INVALID_POSITION) || (lon == INVALID_POSITION)) {
                    WRITE_ERROR("Either (x, y), (lon, lat) or (lane, pos) must be specified for PoI '" + id + "'.");
                    return;
                } else if (!gch->usingGeoProjection()) {
                    WRITE_ERROR("(lon, lat) is specified for PoI '" + id + "' but no geo-conversion is specified for the network.");
                    return;
                }
                pos.set(lon, lat);
                useGeo = true;
                bool success = true;
                if (useProcessing) {
                    success = GeoConvHelper::getProcessing().x2cartesian(pos);
                } else {
                    success = gch->x2cartesian_const(pos);
                }
                if (!success) {
                    WRITE_ERROR("Unable to project coordinates for PoI '" + id + "'.");
                    return;
                }
            }
        }
        if (!myShapeContainer.addPOI(id, type, color, pos, useGeo, laneID, lanePos, lanePosLat, layer, angle, imgFile, relativePath, width, height, ignorePruning)) {
            WRITE_ERROR("PoI '" + id + "' already exists.");
        }
        myLastParameterised = myShapeContainer.getPOIs().get(id);
        if ((laneID != "") && addLanePosParams()) {
            myLastParameterised->setParameter(toString(SUMO_ATTR_LANE), laneID);
            myLastParameterised->setParameter(toString(SUMO_ATTR_POSITION), toString(lanePos));
            myLastParameterised->setParameter(toString(SUMO_ATTR_POSITION_LAT), toString(lanePosLat));
        }
    }
}


void
ShapeHandler::addPoly(const SUMOSAXAttributes& attrs, const bool ignorePruning, const bool useProcessing) {
    bool ok = true;
    const std::string id = myPrefix + attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    // check if ID is valid
    if (SUMOXMLDefinitions::isValidTypeID(id) == false) {
        WRITE_WARNING("Invalid characters for Poly ID");
        ok = false;
    }
    // get the id, report an error if not given or empty...
    if (ok) {
        // continue loading parameters
        const double layer = attrs.getOpt<double>(SUMO_ATTR_LAYER, id.c_str(), ok, myDefaultLayer);
        const bool fill = attrs.getOpt<bool>(SUMO_ATTR_FILL, id.c_str(), ok, myDefaultFill);
        const double lineWidth = attrs.getOpt<double>(SUMO_ATTR_LINEWIDTH, id.c_str(), ok, Shape::DEFAULT_LINEWIDTH);
        const std::string type = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, id.c_str(), ok, Shape::DEFAULT_TYPE);
        const RGBColor color = attrs.hasAttribute(SUMO_ATTR_COLOR) ? attrs.get<RGBColor>(SUMO_ATTR_COLOR, id.c_str(), ok) : myDefaultColor;
        PositionVector shape = attrs.get<PositionVector>(SUMO_ATTR_SHAPE, id.c_str(), ok);
        const bool geo = attrs.getOpt<bool>(SUMO_ATTR_GEO, id.c_str(), ok, false);
        // set geo converter
        const GeoConvHelper* gch;
        if (myGeoConvHelper != nullptr) {
            gch = myGeoConvHelper;
        } else {
            gch = &GeoConvHelper::getFinal();
        }
        // check if poly use geo coordinates
        if (geo || useProcessing) {
            bool success = true;
            for (int i = 0; i < (int)shape.size(); i++) {
                if (useProcessing) {
                    success &= GeoConvHelper::getProcessing().x2cartesian(shape[i]);
                } else {
                    success &= gch->x2cartesian_const(shape[i]);
                }
            }
            if (!success) {
                WRITE_WARNING("Unable to project coordinates for polygon '" + id + "'.");
                return;
            }
        }
        const double angle = attrs.getOpt<double>(SUMO_ATTR_ANGLE, id.c_str(), ok, Shape::DEFAULT_ANGLE);
        std::string imgFile = attrs.getOpt<std::string>(SUMO_ATTR_IMGFILE, id.c_str(), ok, Shape::DEFAULT_IMG_FILE);
        bool relativePath = attrs.getOpt<bool>(SUMO_ATTR_RELATIVEPATH, id.c_str(), ok, Shape::DEFAULT_RELATIVEPATH);
        if (imgFile != "" && !FileHelpers::isAbsolute(imgFile)) {
            imgFile = FileHelpers::getConfigurationRelative(getFileName(), imgFile);
        }
        // check that shape's size is valid
        if (shape.size() == 0) {
            WRITE_ERROR("Polygon's shape cannot be empty.");
            return;
        }
        // check that lineWidth is positive
        if (lineWidth <= 0) {
            WRITE_ERROR("Polygon's lineWidth must be greather than 0.");
            return;
        }
        // create polygon, or show an error if polygon already exists
        if (!myShapeContainer.addPolygon(id, type, color, layer, angle, imgFile, relativePath, shape, geo, fill, lineWidth, ignorePruning)) {
            WRITE_ERROR("Polygon '" + id + "' already exists.");
        }
        myLastParameterised = myShapeContainer.getPolygons().get(id);
    }
}


Parameterised*
ShapeHandler::getLastParameterised() const {
    return myLastParameterised;
}


bool
ShapeHandler::loadFiles(const std::vector<std::string>& files, ShapeHandler& sh) {
    for (const auto& fileIt : files) {
        if (!XMLSubSys::runParser(sh, fileIt, false)) {
            WRITE_MESSAGE("Loading of shapes from " + fileIt + " failed.");
            return false;
        }
    }
    return true;
}


void
ShapeHandler::setDefaults(const std::string& prefix, const RGBColor& color, const double layer, const bool fill) {
    myPrefix = prefix;
    myDefaultColor = color;
    myDefaultLayer = layer;
    myDefaultFill = fill;
}


bool
ShapeHandler::addLanePosParams() {
    return false;
}


/****************************************************************************/
