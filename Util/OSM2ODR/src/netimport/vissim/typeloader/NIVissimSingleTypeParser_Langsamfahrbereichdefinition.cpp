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
/// @file    NIVissimSingleTypeParser_Langsamfahrbereichdefinition.cpp
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
#include "NIVissimSingleTypeParser_Langsamfahrbereichdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Langsamfahrbereichdefinition::NIVissimSingleTypeParser_Langsamfahrbereichdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Langsamfahrbereichdefinition::~NIVissimSingleTypeParser_Langsamfahrbereichdefinition() {}


bool
NIVissimSingleTypeParser_Langsamfahrbereichdefinition::parse(std::istream& from) {
    std::string id;
    from >> id;
    readUntil(from, "fahrzeugklasse");
    std::string tag = "fahrzeugklasse";
    while (tag == "fahrzeugklasse") {
        readUntil(from, "maxverzoegerung");
        tag = myRead(from);
        tag = myRead(from);
    }
    return true;
}


/****************************************************************************/
