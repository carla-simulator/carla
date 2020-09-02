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
/// @file    NIVissimSingleTypeParser_Laengenverteilungsdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/common/StringUtils.h>
#include <utils/geom/PositionVector.h>
#include "../NIImporter_Vissim.h"
#include <utils/distribution/Distribution_Points.h>
#include <utils/distribution/DistributionCont.h>
#include "NIVissimSingleTypeParser_Laengenverteilungsdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Laengenverteilungsdefinition::NIVissimSingleTypeParser_Laengenverteilungsdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Laengenverteilungsdefinition::~NIVissimSingleTypeParser_Laengenverteilungsdefinition() {}


bool
NIVissimSingleTypeParser_Laengenverteilungsdefinition::parse(std::istream& from) {
    // id
    std::string id;
    from >> id;
    // list of points
    Distribution_Points* points = new Distribution_Points(id);
    std::string tag;
    do {
        tag = readEndSecure(from);
        if (tag != "DATAEND") {
            double p1 = StringUtils::toDouble(tag);
            from >> tag;
            double p2 = StringUtils::toDouble(tag);
            points->add(p1, p2);
        }
    } while (tag != "DATAEND");
    DistributionCont::dictionary("length", id, points);
    return true;
}


/****************************************************************************/
