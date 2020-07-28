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
/// @file    NIVissimSingleTypeParser_Zuflussdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimSource.h"
#include "NIVissimSingleTypeParser_Zuflussdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Zuflussdefinition::NIVissimSingleTypeParser_Zuflussdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Zuflussdefinition::~NIVissimSingleTypeParser_Zuflussdefinition() {}


bool
NIVissimSingleTypeParser_Zuflussdefinition::parse(std::istream& from) {
    std::string id, edgeid;
    from >> id; // type-checking is missing!
    std::string tag, name;
    // override some optional values till q
    while (tag != "q") {
        tag = overrideOptionalLabel(from);
        if (tag == "name") {
            name = readName(from);
        } else if (tag == "strecke") {
            from >> edgeid; // type-checking is missing!
        }
    }
    // read q
    // bool exact = false;
    tag = myRead(from);
    if (tag == "exakt") {
        // exact = true;
        tag = myRead(from);
    }
    // double q = StringUtils::toDouble(tag);
    // read the vehicle types
    from >> tag;
    int vehicle_combination;
    from >> vehicle_combination;
    // check whether optional time information is available
    tag = readEndSecure(from);
    double beg, end;
    beg = -1;
    end = -1;
    if (tag == "zeit") {
        from >> tag;
        from >> beg;
        from >> tag;
        from >> end;
    }
    return NIVissimSource::dictionary(id, name, edgeid);
}


/****************************************************************************/
