/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    SUMOPolygon.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Jun 2004
///
// A 2D-polygon
/****************************************************************************/
#include <config.h>

#include <utils/iodevices/OutputDevice.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/GeoConvHelper.h>
#include "SUMOPolygon.h"


// ===========================================================================
// member definitions
// ===========================================================================
SUMOPolygon::SUMOPolygon(const std::string& id, const std::string& type,
                         const RGBColor& color, const PositionVector& shape, bool geo, bool fill, double lineWidth,
                         double layer, double angle, const std::string& imgFile, bool relativePath) :
    Shape(id, type, color, layer, angle, imgFile, relativePath),
    myShape(shape),
    myGEO(geo),
    myFill(fill),
    myLineWidth(lineWidth) {
}


SUMOPolygon::~SUMOPolygon() {}


void
SUMOPolygon::writeXML(OutputDevice& out, bool geo) {
    out.openTag(SUMO_TAG_POLY);
    out.writeAttr(SUMO_ATTR_ID, StringUtils::escapeXML(getID()));
    if (getShapeType().size() > 0) {
        out.writeAttr(SUMO_ATTR_TYPE, StringUtils::escapeXML(getShapeType()));
    }
    out.writeAttr(SUMO_ATTR_COLOR, getShapeColor());
    out.writeAttr(SUMO_ATTR_FILL,  getFill());
    if (getLineWidth() != 1) {
        out.writeAttr(SUMO_ATTR_LINEWIDTH, getLineWidth());
    }
    out.writeAttr(SUMO_ATTR_LAYER, getShapeLayer());
    PositionVector shape = getShape();
    if (geo) {
        out.writeAttr(SUMO_ATTR_GEO, true);
        for (int i = 0; i < (int) shape.size(); i++) {
            GeoConvHelper::getFinal().cartesian2geo(shape[i]);
        }
    }
    out.setPrecision(gPrecisionGeo);
    out.writeAttr(SUMO_ATTR_SHAPE, shape);
    out.setPrecision();
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
    writeParams(out);
    out.closeTag();
}


/****************************************************************************/
