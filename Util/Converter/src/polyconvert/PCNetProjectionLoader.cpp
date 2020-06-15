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
/// @file    PCNetProjectionLoader.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Thu, 02.11.2006
///
// A reader for a SUMO network's projection description
/****************************************************************************/
#include <config.h>

#include <string>
#include <map>
#include <fstream>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/RGBColor.h>
#include <utils/common/StdDefs.h>
#include <utils/common/SysUtils.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/xml/SUMOSAXReader.h>
#include <utils/geom/GeomConvHelper.h>
#include <polyconvert/PCPolyContainer.h>
#include "PCNetProjectionLoader.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static interface
// ---------------------------------------------------------------------------
void
PCNetProjectionLoader::load(const std::string& file, double scale) {
    if (!FileHelpers::isReadable(file)) {
        throw ProcessError("Could not open net-file '" + file + "'.");
    }
    // build handler and parser
    PCNetProjectionLoader handler(scale);
    handler.setFileName(file);
    SUMOSAXReader* parser = XMLSubSys::getSAXReader(handler);
    const long before = PROGRESS_BEGIN_TIME_MESSAGE("Parsing network projection from '" + file + "'");
    if (!parser->parseFirst(file)) {
        delete parser;
        throw ProcessError("Can not read XML-file '" + handler.getFileName() + "'.");
    }
    // parse
    while (parser->parseNext() && !handler.hasReadAll());
    // clean up
    PROGRESS_TIME_MESSAGE(before);
    if (!handler.hasReadAll()) {
        throw ProcessError("Could not find projection parameter in net.");
    }
    delete parser;
}


// ---------------------------------------------------------------------------
// handler methods
// ---------------------------------------------------------------------------
PCNetProjectionLoader::PCNetProjectionLoader(double scale) :
    SUMOSAXHandler("sumo-network"),
    myFoundLocation(false),
    myScale(scale) {
}


PCNetProjectionLoader::~PCNetProjectionLoader() {}


void
PCNetProjectionLoader::myStartElement(int element,
                                      const SUMOSAXAttributes& attrs) {
    if (element != SUMO_TAG_LOCATION) {
        return;
    }
    // @todo refactor parsing of location since its duplicated in NLHandler and PCNetProjectionLoader
    myFoundLocation = true;
    PositionVector s = attrs.get<PositionVector>(SUMO_ATTR_NET_OFFSET, nullptr, myFoundLocation);
    Boundary convBoundary = attrs.get<Boundary>(SUMO_ATTR_CONV_BOUNDARY, nullptr, myFoundLocation);
    Boundary origBoundary = attrs.get<Boundary>(SUMO_ATTR_ORIG_BOUNDARY, nullptr, myFoundLocation);
    std::string proj = attrs.get<std::string>(SUMO_ATTR_ORIG_PROJ, nullptr, myFoundLocation);
    if (myFoundLocation) {
        OptionsCont& oc = OptionsCont::getOptions();
        Position networkOffset = s[0] + Position(oc.getFloat("offset.x"), oc.getFloat("offset.y"));
        GeoConvHelper::init(proj, networkOffset, origBoundary, convBoundary, myScale);
    }
}


bool
PCNetProjectionLoader::hasReadAll() const {
    return myFoundLocation;
}


/****************************************************************************/
