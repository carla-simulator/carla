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
/// @file    NIVissimSingleTypeParser_Kantensperrung.cpp
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
#include "../tempstructs/NIVissimClosures.h"
#include "NIVissimSingleTypeParser_Kantensperrung.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Kantensperrung::NIVissimSingleTypeParser_Kantensperrung(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Kantensperrung::~NIVissimSingleTypeParser_Kantensperrung() {}


bool
NIVissimSingleTypeParser_Kantensperrung::parse(std::istream& from) {
    std::string tag;
    from >> tag;
    //
    std::string id;
    from >> id;
    //
    from >> tag;
    from >> tag;
    int from_node;
    from >> from_node;
    //
    from >> tag;
    from >> tag;
    int to_node;
    from >> to_node;
    //
    from >> tag;
    from >> tag;
    std::vector<int> edges;
    while (tag != "DATAEND") {
        tag = readEndSecure(from);
        if (tag != "DATAEND") {
            edges.push_back(StringUtils::toInt(tag));
        }
    }
    NIVissimClosures::dictionary(id, from_node, to_node, edges);
    return true;
}


/****************************************************************************/
