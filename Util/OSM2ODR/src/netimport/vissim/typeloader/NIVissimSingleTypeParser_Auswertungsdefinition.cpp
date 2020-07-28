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
/// @file    NIVissimSingleTypeParser_Auswertungsdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Fri, 21 Mar 2003
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimSource.h"
#include "NIVissimSingleTypeParser_Auswertungsdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Auswertungsdefinition::NIVissimSingleTypeParser_Auswertungsdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Auswertungsdefinition::~NIVissimSingleTypeParser_Auswertungsdefinition() {}


bool
NIVissimSingleTypeParser_Auswertungsdefinition::parse(std::istream& from) {
    std::string id;
    from >> id; // "typ"

    if (id == "DATENBANK") { // !!! unverified
        return true;
    }

    std::string type = myRead(from);
    if (type == "abfluss") {
        while (type != "signalgruppe") {
            type = myRead(from);
        }
        while (type != "DATAEND") {
            type = readEndSecure(from, "messung");
        }
    } else if (type == "vbv") {} else if (type == "dichte") {} else if (type == "emissionen") {} else if (type == "fzprot") {} else if (type == "spwprot") {} else if (type == "segment") {
        while (type != "konfdatei") {
            type = myRead(from);
        }
    } else if (type == "wegeausw") {} else if (type == "knoten") {} else if (type == "konvergenz") {
        while (type != "zeit") {
            type = myRead(from);
        }
    }
    return true;
}


/****************************************************************************/
