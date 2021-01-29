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
/// @file    PCLoaderXML.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christoph Sommer
/// @author  Michael Behrisch
/// @date    Thu, 02.11.2006
///
// A reader for polygons and pois stored in XML-format
/****************************************************************************/
#include <config.h>

#include <string>
#include <map>
#include <fstream>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/RGBColor.h>
#include <utils/common/StdDefs.h>
#include <utils/common/SysUtils.h>
#include <polyconvert/PCPolyContainer.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "PCLoaderXML.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static interface
// ---------------------------------------------------------------------------
void
PCLoaderXML::loadIfSet(OptionsCont& oc, PCPolyContainer& toFill,
                       PCTypeMap& tm) {
    if (!oc.isSet("xml-files")) {
        return;
    }
    PCLoaderXML handler(toFill, tm, oc);
    // parse file(s)
    std::vector<std::string> files = oc.getStringVector("xml");
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        if (!FileHelpers::isReadable(*file)) {
            throw ProcessError("Could not open xml-file '" + *file + "'.");
        }
        const long before = PROGRESS_BEGIN_TIME_MESSAGE("Parsing XML from '" + *file + "'");
        if (!XMLSubSys::runParser(handler, *file)) {
            throw ProcessError();
        }
        PROGRESS_TIME_MESSAGE(before);
    }
}



// ---------------------------------------------------------------------------
// handler methods
// ---------------------------------------------------------------------------
PCLoaderXML::PCLoaderXML(PCPolyContainer& toFill,
                         PCTypeMap& tm, OptionsCont& oc)
    : ShapeHandler("xml-poi-definition", toFill),
      myTypeMap(tm), myOptions(oc) {}


PCLoaderXML::~PCLoaderXML() {}


void
PCLoaderXML::myStartElement(int element,
                            const SUMOSAXAttributes& attrs) {
    if (element != SUMO_TAG_POI && element != SUMO_TAG_POLY) {
        return;
    }
    bool ok = true;
    // get the id, report an error if not given or empty...
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    std::string type = attrs.getOpt<std::string>(SUMO_ATTR_TYPE, id.c_str(), ok, myOptions.getString("type"));
    if (!ok) {
        return;
    }
    // patch the values
    bool discard = myOptions.getBool("discard");
    if (myTypeMap.has(type)) {
        const PCTypeMap::TypeDef& def = myTypeMap.get(type);
        discard = def.discard;
        setDefaults(def.prefix, def.color, def.layer, def.allowFill);
    } else {
        setDefaults(myOptions.getString("prefix"), RGBColor::parseColor(myOptions.getString("color")),
                    myOptions.getFloat("layer"), myOptions.getBool("fill"));
    }
    if (!discard) {
        if (element == SUMO_TAG_POI) {
            addPOI(attrs, myOptions.isInStringVector("prune.keep-list", id), true);
        }
        if (element == SUMO_TAG_POLY) {
            addPoly(attrs, myOptions.isInStringVector("prune.keep-list", id), true);
        }
    }
}


Position
PCLoaderXML::getLanePos(const std::string& poiID, const std::string& laneID, double lanePos, double lanePosLat) {
    static_cast<PCPolyContainer&>(myShapeContainer).addLanePos(poiID, laneID, lanePos, lanePosLat);
    return Position::INVALID;
}


/****************************************************************************/
