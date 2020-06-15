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
/// @file    NIVissimSingleTypeParser_Fahrzeugtypdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimVehicleType.h"
#include "NIVissimSingleTypeParser_Fahrzeugtypdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Fahrzeugtypdefinition::NIVissimSingleTypeParser_Fahrzeugtypdefinition(
    NIImporter_Vissim& parent, NIImporter_Vissim::ColorMap& colorMap)
    : NIImporter_Vissim::VissimSingleTypeParser(parent),
      myColorMap(colorMap) {}


NIVissimSingleTypeParser_Fahrzeugtypdefinition::~NIVissimSingleTypeParser_Fahrzeugtypdefinition() {}


bool
NIVissimSingleTypeParser_Fahrzeugtypdefinition::parse(std::istream& from) {
    // id
    int id;
    from >> id; // type-checking is missing!
    // name
    std::string tag;
    from >> tag;
    std::string name = readName(from);
    // category
    std::string category;
    from >> tag;
    from >> category;
    // color (optional) and length
    RGBColor color;
    tag = myRead(from);
    while (tag != "laenge") {
        if (tag == "farbe") {
            std::string colorName = myRead(from);
            NIImporter_Vissim::ColorMap::iterator i = myColorMap.find(colorName);
            if (i != myColorMap.end()) {
                color = (*i).second;
            } else {
                int r, g, b;
                r = StringUtils::toInt(colorName);
                if (!(from >> g)) {
                    throw NumberFormatException("");
                }
                if (!(from >> b)) {
                    throw NumberFormatException("");
                }
                if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
                    throw NumberFormatException("");
                }
                color = RGBColor((unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
            }
        }
        tag = myRead(from);
    }
    double length;
    from >> length;
    // overread until "Maxbeschleunigung"
    while (tag != "maxbeschleunigung") {
        tag = myRead(from);
    }
    double amax;
    from >> amax; // type-checking is missing!
    // overread until "Maxverzoegerung"
    while (tag != "maxverzoegerung") {
        tag = myRead(from);
    }
    double dmax;
    from >> dmax; // type-checking is missing!
    while (tag != "besetzungsgrad") {
        tag = myRead(from);
    }
    while (tag != "DATAEND") {
        tag = readEndSecure(from, "verlustzeit");
    }
    return NIVissimVehicleType::dictionary(id, name, category, color);
}


/****************************************************************************/
