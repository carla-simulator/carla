/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    PointOfInterest.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Melanie Knocke
/// @date    2005-09-15
///
// A point-of-interest (2D)
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/FileHelpers.h>
#include <utils/common/Parameterised.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/geom/Position.h>
#include <utils/iodevices/OutputDevice.h>
#include "Shape.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PointOfInterest
 * @brief A point-of-interest
 */
class PointOfInterest : public Shape, public Position, public Parameterised {
public:
    /** @brief Constructor
     * @param[in] id The name of the POI
     * @param[in] type The (abstract) type of the POI
     * @param[in] color The color of the POI
     * @param[in] pos The position of the POI
     * @param[in[ geo use GEO coordinates (lon/lat)
     * @param[in] lane The Lane in which this POI is placed
     * @param[in] posOverLane The position over Lane
     * @param[in] posLat The position lateral over Lane
     * @param[in] layer The layer of the POI
     * @param[in] angle The rotation of the POI
     * @param[in] imgFile The raster image of the shape
     * @param[in] relativePath set image file as relative path
     * @param[in] width The width of the POI image
     * @param[in] height The height of the POI image
     */
    PointOfInterest(const std::string& id, const std::string& type,
                    const RGBColor& color, const Position& pos, bool geo,
                    const std::string& lane, double posOverLane, double posLat,
                    double layer = DEFAULT_LAYER,
                    double angle = DEFAULT_ANGLE,
                    const std::string& imgFile = DEFAULT_IMG_FILE,
                    bool relativePath = DEFAULT_RELATIVEPATH,
                    double width = DEFAULT_IMG_WIDTH,
                    double height = DEFAULT_IMG_HEIGHT) :
        Shape(id, type, color, layer, angle, imgFile, relativePath),
        Position(pos),
        myGeo(geo),
        myLane(lane),
        myPosOverLane(posOverLane),
        myPosLat(posLat),
        myHalfImgWidth(width / 2.0),
        myHalfImgHeight(height / 2.0) {
    }


    /// @brief Destructor
    virtual ~PointOfInterest() { }


    /// @name Getter
    /// @{

    /// @brief Returns the image width of the POI
    inline double getWidth() const {
        return myHalfImgWidth * 2.0;
    }

    /// @brief Returns the image height of the POI
    inline double getHeight() const {
        return myHalfImgHeight * 2.0;
    }

    /// @brief Returns the image center of the POI
    Position getCenter() const {
        return {x() + myHalfImgWidth, y() + myHalfImgHeight};
    }
    /// @}


    /// @name Setter
    /// @{

    /// @brief set the image width of the POI
    inline void setWidth(double width) {
        myHalfImgWidth = width / 2.0;
    }

    /// @brief set the image height of the POI
    inline void setHeight(double height) {
        myHalfImgHeight = height / 2.0;
    }
    /// @}


    /* @brief POI definition to the given device
     * @param[in] geo  Whether to write the output in geo-coordinates
     */
    void writeXML(OutputDevice& out, const bool geo = false, const double zOffset = 0., const std::string laneID = "", const double pos = 0., const double posLat = 0.) {
        out.openTag(SUMO_TAG_POI);
        out.writeAttr(SUMO_ATTR_ID, StringUtils::escapeXML(getID()));
        if (getShapeType().size() > 0) {
            out.writeAttr(SUMO_ATTR_TYPE, StringUtils::escapeXML(getShapeType()));
        }
        out.writeAttr(SUMO_ATTR_COLOR, getShapeColor());
        out.writeAttr(SUMO_ATTR_LAYER, getShapeLayer() + zOffset);
        if (laneID != "") {
            out.writeAttr(SUMO_ATTR_LANE, laneID);
            out.writeAttr(SUMO_ATTR_POSITION, pos);
            if (posLat != 0) {
                out.writeAttr(SUMO_ATTR_POSITION_LAT, posLat);
            }
        } else {
            if (geo) {
                Position POICartesianPos(*this);
                GeoConvHelper::getFinal().cartesian2geo(POICartesianPos);
                out.setPrecision(gPrecisionGeo);
                out.writeAttr(SUMO_ATTR_LON, POICartesianPos.x());
                out.writeAttr(SUMO_ATTR_LAT, POICartesianPos.y());
                out.setPrecision();
            } else {
                out.writeAttr(SUMO_ATTR_X, x());
                out.writeAttr(SUMO_ATTR_Y, y());
            }
        }
        if (getShapeNaviDegree() != Shape::DEFAULT_ANGLE) {
            out.writeAttr(SUMO_ATTR_ANGLE, getShapeNaviDegree());
        }
        if (getShapeImgFile() != Shape::DEFAULT_IMG_FILE) {
            if (getShapeRelativePath()) {
                // write only the file name, without file path
                std::string file = getShapeImgFile();
                file.erase(0, FileHelpers::getFilePath(getShapeImgFile()).size());
                out.writeAttr(SUMO_ATTR_IMGFILE, file);
            } else {
                out.writeAttr(SUMO_ATTR_IMGFILE, getShapeImgFile());
            }
        }
        if (getWidth() != Shape::DEFAULT_IMG_WIDTH) {
            out.writeAttr(SUMO_ATTR_WIDTH, getWidth());
        }
        if (getHeight() != Shape::DEFAULT_IMG_HEIGHT) {
            out.writeAttr(SUMO_ATTR_HEIGHT, getHeight());
        }
        writeParams(out);
        out.closeTag();
    }


protected:
    /// @brief flag to check if POI was loaded as GEO Position (main used by netedit)
    bool myGeo;

    /// @brief ID of lane in which this POI is placed (main used by netedit)
    std::string myLane;

    /// @brief position over lane in which this POI is placed (main used by netedit)
    double myPosOverLane;

    /// @brief latereal position over lane in which this POI is placed (main used by netedit)
    double myPosLat;

    /// @brief The half width of the image when rendering this POI
    double myHalfImgWidth;

    /// @brief The half height of the image when rendering this POI
    double myHalfImgHeight;

};
