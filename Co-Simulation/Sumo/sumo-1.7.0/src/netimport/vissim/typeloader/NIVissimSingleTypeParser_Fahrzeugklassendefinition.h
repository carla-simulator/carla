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
/// @file    NIVissimSingleTypeParser_Fahrzeugklassendefinition.h
/// @author  Daniel Krajzewicz
/// @date    Wed, 18 Dec 2002
///
//
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include "../NIImporter_Vissim.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIVissimSingleTypeParser_Fahrzeugklassendefinition
 *
 */
class NIVissimSingleTypeParser_Fahrzeugklassendefinition :
    public NIImporter_Vissim::VissimSingleTypeParser {
public:
    /// Constructor
    NIVissimSingleTypeParser_Fahrzeugklassendefinition(NIImporter_Vissim& parent,
            NIImporter_Vissim::ColorMap& colorMap);

    /// Destructor
    ~NIVissimSingleTypeParser_Fahrzeugklassendefinition();

    /// Parses the data type from the given stream
    bool parse(std::istream& from);

private:
    /// color definitions
    NIImporter_Vissim::ColorMap& myColorMap;

};
