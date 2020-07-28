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
/// @file    NIVissimSingleTypeParser_Knotendefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimNodeParticipatingEdge.h"
#include "../tempstructs/NIVissimNodeParticipatingEdgeVector.h"
#include "../tempstructs/NIVissimNodeDef_Edges.h"
#include "../tempstructs/NIVissimNodeDef_Poly.h"
#include "../tempstructs/NIVissimNodeDef.h"
#include "NIVissimSingleTypeParser_Knotendefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Knotendefinition::NIVissimSingleTypeParser_Knotendefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Knotendefinition::~NIVissimSingleTypeParser_Knotendefinition() {}


bool
NIVissimSingleTypeParser_Knotendefinition::parse(std::istream& from) {
    //
    int id;
    from >> id;
    //
    std::string tag;
    from >> tag;
    std::string name = readName(from);
    //
    tag = overrideOptionalLabel(from);
    //
    while (tag != "netzausschnitt") {
        tag = myRead(from);
    }
    //
    tag = myRead(from);
    if (tag == "strecke") {
        NIVissimNodeParticipatingEdgeVector edges;
        while (tag == "strecke") {
            int edgeid;
            double from_pos, to_pos;
            from_pos = to_pos = -1.0;
            from >> edgeid;
            tag = readEndSecure(from, "strecke");
            if (tag == "von") {
                from >> from_pos; // type-checking is missing!
                from >> tag;
                from >> to_pos; // type-checking is missing!
                tag = readEndSecure(from, "strecke");
            }
            edges.push_back(new NIVissimNodeParticipatingEdge(edgeid, from_pos, to_pos));
        }
        NIVissimNodeDef_Edges::dictionary(id, name, edges);
    } else {
        int no = StringUtils::toInt(tag);
        PositionVector poly;
        for (int i = 0; i < no; i++) {
            poly.push_back(getPosition(from));
        }
        poly.closePolygon();
        NIVissimNodeDef_Poly::dictionary(id, name, poly);
    }
    return true;
}


/****************************************************************************/
