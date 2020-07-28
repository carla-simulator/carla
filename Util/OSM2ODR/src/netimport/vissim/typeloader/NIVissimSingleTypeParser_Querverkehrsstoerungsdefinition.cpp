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
/// @file    NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimExtendedEdgePoint.h"
#include "../tempstructs/NIVissimDisturbance.h"
#include "NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::~NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition() {}


bool
NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::parse(std::istream& from) {
    std::string tag;
    tag = myRead(from);
    if (tag == "nureigenestrecke") {
        return parseOnlyMe(from);
    } else if (tag == "ort") {
        return parsePositionDescribed(from);
    } else if (tag == "nummer") {
        return parseNumbered(from);
    }
    WRITE_ERROR(
        "NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition: format problem");
    throw 1;
}

bool
NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::parseOnlyMe(std::istream& from) {
    std::string tag;
    from >> tag;
    return true;
}


bool
NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::parsePositionDescribed(std::istream& from) {
    std::string tag = myRead(from);
    NIVissimExtendedEdgePoint edge = parsePos(from);
//    from >> tag; // "Durch"
    bool ok = true;
    do {
        from >> tag; // "Strecke"
        NIVissimExtendedEdgePoint by = parsePos(from);
        //
        double timegap;
        from >> timegap;

        from >> tag;
        double waygap;
        from >> waygap;

        double vmax = -1;
        tag = readEndSecure(from);
        if (tag == "vmax") {
            from >> vmax;
        }
        ok = NIVissimDisturbance::dictionary("", edge, by);
        if (tag != "DATAEND") {
            tag = readEndSecure(from);
        }
    } while (tag != "DATAEND" && ok);
    return ok;
}



bool
NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::parseNumbered(std::istream& from) {
    //
    int id;
    from >> id;
    //
    std::string tag;
    from >> tag;
    std::string name = readName(from);
    // skip optional "Beschriftung"
    while (tag != "ort") {
        tag = myRead(from);
    }
    //
    from >> tag; // "Strecke"
    NIVissimExtendedEdgePoint edge = parsePos(from);
    bool ok = true;
    do {
        from >> tag; // "Ort"
        from >> tag; // "Strecke"
        NIVissimExtendedEdgePoint by = parsePos(from);
        //
        double timegap;
        from >> timegap;

        double waygap;
        from >> tag;
        from >> waygap;

        double vmax = -1;
        tag = readEndSecure(from);
        if (tag == "vmax") {
            from >> vmax;
        }

        ok = NIVissimDisturbance::dictionary(name, edge, by);
        if (tag != "DATAEND") {
            tag = readEndSecure(from);
        }
    } while (tag != "DATAEND" && ok);
    return ok;
}



NIVissimExtendedEdgePoint
NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition::parsePos(std::istream& from) {
    int edgeid;
    from >> edgeid; // type-checking is missing!
    //
    std::string tag;
    from >> tag;
    from >> tag;
    std::vector<int> lanes;
    if (tag == "ALLE") {
        //lanes.push_back(1); // !!!
    } else {
        lanes.push_back(StringUtils::toInt(tag));
    }
    //
    double position;
    from >> tag;
    from >> position;
    // assigned vehicle types
    std::vector<int> types;
    from >> tag;
    while (tag != "zeitluecke" && tag != "durch" && tag != "DATAEND" && tag != "alle") {
        tag = readEndSecure(from);
        if (tag != "DATAEND") {
            if (tag == "alle") {
                types.push_back(-1);
                from >> tag;
                tag = "alle";
            } else if (tag != "zeitluecke" && tag != "durch" && tag != "DATAEND") {
                int tmp = StringUtils::toInt(tag);
                types.push_back(tmp);
            }
        }
    }
    return NIVissimExtendedEdgePoint(edgeid, lanes, position, types);
}


/****************************************************************************/
