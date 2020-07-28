/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    Shape.cpp
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Oct 2012
///
// A 2D- or 3D-Shape
/****************************************************************************/
#include <config.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>

#include "Shape.h"

// ===========================================================================
// static member definitions
// ===========================================================================
const std::string Shape::DEFAULT_TYPE = "";
const double Shape::DEFAULT_LAYER = 128;
const double Shape::DEFAULT_LINEWIDTH = 1;
const double Shape::DEFAULT_LAYER_POI = (double)GLO_POI;
const double Shape::DEFAULT_ANGLE = 0;
const std::string Shape::DEFAULT_IMG_FILE = "";
const bool Shape::DEFAULT_RELATIVEPATH = false;
const double Shape::DEFAULT_IMG_WIDTH = 1;
const double Shape::DEFAULT_IMG_HEIGHT = 1;

// ===========================================================================
// member definitions
// ===========================================================================
Shape::Shape(const std::string& id, const std::string& type,
             const RGBColor& color, double layer,
             double angle, const std::string& imgFile, bool relativePath) :
    Named(id),
    myType(type),
    myColor(color),
    myLayer(layer),
    myNaviDegreeAngle(angle),
    myImgFile(imgFile),
    myRelativePath(relativePath) {
}


Shape::~Shape() {}


/****************************************************************************/
