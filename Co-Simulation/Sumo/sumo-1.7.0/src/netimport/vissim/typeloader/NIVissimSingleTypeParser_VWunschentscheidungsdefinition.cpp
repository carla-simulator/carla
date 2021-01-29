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
/// @file    NIVissimSingleTypeParser_VWunschentscheidungsdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <vector>
#include <cassert>
#include <utils/common/StringUtils.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimEdge.h"
#include "../tempstructs/NIVissimConnection.h"
#include "NIVissimSingleTypeParser_VWunschentscheidungsdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_VWunschentscheidungsdefinition::NIVissimSingleTypeParser_VWunschentscheidungsdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_VWunschentscheidungsdefinition::~NIVissimSingleTypeParser_VWunschentscheidungsdefinition() {}


bool
NIVissimSingleTypeParser_VWunschentscheidungsdefinition::parse(std::istream& from) {
    std::string tag;
    from >> tag; // id
    from >> tag; // name
    tag = readName(from);
    tag = overrideOptionalLabel(from);
    from >> tag; // strecke
    std::string edgeid;
    from >> edgeid;
    from >> tag; // spur
    std::string lane;
    from >> lane;
    from >> tag; // bei
    std::string pos;
    from >> pos;
    from >> tag; // fahrzeugklasse
    from >> tag; // <fahrzeugklasse>
    from >> tag; // vwunsch
    std::string vwunsch;
    from >> vwunsch; // vwunsch
    std::vector<std::string> tmp;
    tmp.push_back("zeit");
    tmp.push_back("fahrzeugklasse");
    tag = readEndSecure(from, tmp);
    while (tag != "DATAEND" && tag != "zeit") {
        from >> tag;
        from >> tag;
        from >> tag;
        tag = myRead(from);
    }
    if (tag == "zeit") {
        from >> tag;
        from >> tag;
        from >> tag;
        from >> tag;
    }
    int numid = StringUtils::toInt(edgeid);
    int numlane = StringUtils::toInt(lane) - 1;
    int numv = StringUtils::toInt(vwunsch);
    NIVissimEdge* e = NIVissimEdge::dictionary(numid);
    if (e == nullptr) {
        NIVissimConnection* c = NIVissimConnection::dictionary(numid);
        const std::vector<int>& lanes = c->getToLanes();
        e = NIVissimEdge::dictionary(c->getToEdgeID());
        for (std::vector<int>::const_iterator j = lanes.begin(); j != lanes.end(); j++) {
            e->setSpeed((*j), numv);
        }
        assert(e != 0);
    } else {
        e->setSpeed(numlane, numv);
    }
    return true;
}


/****************************************************************************/
