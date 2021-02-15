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
/// @file    PCTypeDefHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 16.03.2006
///
// A handler for loading polygon type maps
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/options/OptionsCont.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/RGBColor.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/shapes/Shape.h>
#include "PCTypeMap.h"
#include "PCTypeDefHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
PCTypeDefHandler::PCTypeDefHandler(OptionsCont& oc, PCTypeMap& con)
    : SUMOSAXHandler("Detector-Defintion"),
      myOptions(oc),  myContainer(con) {}


PCTypeDefHandler::~PCTypeDefHandler() {}


void
PCTypeDefHandler::myStartElement(int element,
                                 const SUMOSAXAttributes& attrs) {
    if (element == SUMO_TAG_POLYTYPE) {
        bool ok = true;
        // get the id, report an error if not given or empty...
        std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
        if (!ok) {
            return;
        }
        const double layer = attrs.getOpt<double>(SUMO_ATTR_LAYER, id.c_str(), ok, myOptions.getFloat("layer"));
        const bool discard = attrs.getOpt<bool>(SUMO_ATTR_DISCARD, id.c_str(), ok, false);
        const bool allowFill = attrs.getOpt<bool>(SUMO_ATTR_FILL, id.c_str(), ok, myOptions.getBool("fill"));
        const std::string type = attrs.getOpt<std::string>(SUMO_ATTR_NAME, id.c_str(), ok, myOptions.getString("type"));
        const std::string prefix = attrs.getOpt<std::string>(SUMO_ATTR_PREFIX, id.c_str(), ok, myOptions.getString("prefix"));
        const std::string color = attrs.getOpt<std::string>(SUMO_ATTR_COLOR, id.c_str(), ok, myOptions.getString("color"));
        const double angle = attrs.getOpt<double>(SUMO_ATTR_ANGLE, id.c_str(), ok, Shape::DEFAULT_ANGLE);
        const std::string imgFile = attrs.getOpt<std::string>(SUMO_ATTR_IMGFILE, id.c_str(), ok, Shape::DEFAULT_IMG_FILE);
        // !!! what about error handling?
        if (!myContainer.add(id, type, color, prefix, layer, angle, imgFile, discard, allowFill)) {
            WRITE_ERROR("Could not add polygon type '" + id + "' (probably the id is already used).");
        }
    }
}


/****************************************************************************/
