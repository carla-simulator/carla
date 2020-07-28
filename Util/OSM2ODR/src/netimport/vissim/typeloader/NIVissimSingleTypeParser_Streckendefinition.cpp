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
/// @file    NIVissimSingleTypeParser_Streckendefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/PositionVector.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimEdge.h"
#include "../tempstructs/NIVissimClosedLaneDef.h"
#include "../tempstructs/NIVissimClosedLanesVector.h"
#include "NIVissimSingleTypeParser_Streckendefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Streckendefinition::NIVissimSingleTypeParser_Streckendefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Streckendefinition::~NIVissimSingleTypeParser_Streckendefinition() {}


bool
NIVissimSingleTypeParser_Streckendefinition::parse(std::istream& from) {
    // read in the id
    int id;
    from >> id;
    //
    std::string tag;
    // the following elements may occure: "Name", "Beschriftung", "Typ",
    //  followed by the mandatory "Laenge"
    std::string name, label, type;
    double length = -1;
    while (length < 0) {
        tag = overrideOptionalLabel(from);
        if (tag == "name") {
            name = readName(from);
        } else if (tag == "typ") {
            type = myRead(from);
        } else if (tag == "laenge") {
            from >> length; // type-checking is missing!
        }
    }
    // read in the number of lanes
    int noLanes;
    tag = myRead(from);
    from >> noLanes;
    // skip some parameter, except optional "Zuschlag" until "Von" (mandatory)
    //  occurs
    double zuschlag1, zuschlag2;
    zuschlag1 = zuschlag2 = 0;
    while (tag != "von") {
        tag = myRead(from);
        if (tag == "zuschlag") {
            from >> zuschlag1; // type-checking is missing!
            tag = myRead(from);
            if (tag == "zuschlag") {
                from >> zuschlag2; // type-checking is missing!
            }
        }
    }
    // Read the geometry information
    PositionVector geom;
    while (tag != "nach") {
        geom.push_back_noDoublePos(getPosition(from));
        tag = myRead(from);
        try {
            StringUtils::toDouble(tag);
            tag = myRead(from);
        } catch (NumberFormatException&) {}
    }
    geom.push_back_noDoublePos(getPosition(from));
    // Read definitions of closed lanes
    NIVissimClosedLanesVector clv;
    // check whether a next close lane definition can be found
    tag = readEndSecure(from);
    while (tag != "DATAEND") {
        if (tag == "keinspurwechsel") {
            while (tag != "DATAEND") {
                tag = readEndSecure(from);
            }
        } else if (tag == "spur") {
            // get the lane number
            int laneNo;
            from >> laneNo; // unused and type-checking is missing!
            // get the list of assigned car classes
            std::vector<int> assignedVehicles;
            tag = myRead(from);
            tag = myRead(from);
            while (tag != "DATAEND" && tag != "spur" && tag != "keinspurwechsel") {
                int classes = StringUtils::toInt(tag);
                assignedVehicles.push_back(classes);
                tag = readEndSecure(from);
            }
            // build and add the definition
            NIVissimClosedLaneDef* cld = new NIVissimClosedLaneDef(assignedVehicles);
            clv.push_back(cld);
        } else {
            tag = readEndSecure(from);
        }
    }
    NIVissimEdge* e = new NIVissimEdge(id, name, type, std::vector<double>(noLanes, NBEdge::UNSPECIFIED_WIDTH),
                                       zuschlag1, zuschlag2, length, geom, clv);
    if (!NIVissimEdge::dictionary(id, e)) {
        return false;
    }
    return true;
    //return NIVissimAbstractEdge::dictionary(id, e);
}


/****************************************************************************/
