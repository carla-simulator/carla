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
/// @file    NIVissimSingleTypeParser_Signalgruppendefinition.h
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
 * @class NIVissimSingleTypeParser_Signalgruppendefinition
 *
 */
class NIVissimSingleTypeParser_Signalgruppendefinition :
    public NIImporter_Vissim::VissimSingleTypeParser {
public:
    /// Constructor
    NIVissimSingleTypeParser_Signalgruppendefinition(NIImporter_Vissim& parent);

    /// Destructor
    ~NIVissimSingleTypeParser_Signalgruppendefinition();

    /// Parses the data type from the given stream
    bool parse(std::istream& from);

private:
    /// Parses a traffic light group which belongs to a fixed time traffic light
    bool parseFixedTime(int id, const std::string& name, int lsaid,
                        std::istream& from);

    /// Parses a traffic light group which belongs to a VAS traffic light
    bool parseVAS(int id, const std::string& name, int lsaid,
                  std::istream& from);

    /// Parses a traffic light group which belongs to a VSPLUS traffic light
    bool parseVSPLUS(int id, const std::string& name, int lsaid,
                     std::istream& from);

    /// Parses a traffic light group which belongs to a TRENDS traffic light
    bool parseTRENDS(int id, const std::string& name, int lsaid,
                     std::istream& from);

    /// Parses a traffic light group which belongs to a VAStraffic light
    bool parseVAP(int id, const std::string& name, int lsaid,
                  std::istream& from);

    /// Parses a traffic light group which belongs to a TL traffic light
    bool parseTL(int id, const std::string& name, int lsaid,
                 std::istream& from);

    /// Parses a traffic light group which belongs to a POS traffic light
    bool parsePOS(int id, const std::string& name, int lsaid,
                  std::istream& from);

    /// Parses a traffic light group which belongs fixed time traffic light with an extern definition
    bool parseExternFixedTime(int id, const std::string& name, int lsaid,
                              std::istream& from);

};
