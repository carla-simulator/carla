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
/// @file    NIVissimSingleTypeParser_Lichtsignalanlagendefinition.cpp
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
#include "../tempstructs/NIVissimTL.h"
#include "NIVissimSingleTypeParser_Lichtsignalanlagendefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Lichtsignalanlagendefinition::NIVissimSingleTypeParser_Lichtsignalanlagendefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Lichtsignalanlagendefinition::~NIVissimSingleTypeParser_Lichtsignalanlagendefinition() {}


bool
NIVissimSingleTypeParser_Lichtsignalanlagendefinition::parse(std::istream& from) {
    //
    int id;
    from >> id;
    //
    std::string tag, name;
    tag = myRead(from);
    if (tag == "name") {
        name = readName(from);
        tag = myRead(from);
    }
    // type
    std::string type;
    type = myRead(from);
    if (type == "festzeit") {
        return parseFixedTime(id, name, from);
    }
    if (type == "vas") {
        return parseVAS(id, name, from);
    }
    if (type == "vsplus") {
        return parseRestActuated(id, name, from, type);
    }
    if (type == "trends") {
        return parseRestActuated(id, name, from, type);
    }
    if (type == "vap") {
        return parseRestActuated(id, name, from, type);
    }
    if (type == "tl") {
        return parseRestActuated(id, name, from, type);
    }
    if (type == "pos") {
        return parseRestActuated(id, name, from, type);
    }
    if (type == "nema") {
        return parseRestActuated(id, name, from, type);
    }
    if (type == "extern") {
        return parseRestActuated(id, name, from, type);
    }
    WRITE_ERROR("Unsupported LSA-Type '" + type + "' occurred.");
    return false;
}


bool
NIVissimSingleTypeParser_Lichtsignalanlagendefinition::parseFixedTime(
    int id, std::string name, std::istream& from) {
    std::string type = "festzeit";
    std::string tag;
    from >> tag;
    //
    double absdur;
    from >> absdur; // type-checking is missing!
    //
    tag = readEndSecure(from);
    double offset = 0;
    if (tag == "versatz") {
        from >> offset; // type-checking is missing!
    }
    if (tag != "szpkonfdatei" && tag != "DATAEND" && tag != "progdatei") {
        tag = readEndSecure(from);
        if (tag == "szpkonfdatei" || tag == "progdatei") {
            type = "festzeit_fake";
        }
    }
    return NIVissimTL::dictionary(id, type, name, (SUMOTime) absdur, (SUMOTime) offset);
}


bool
NIVissimSingleTypeParser_Lichtsignalanlagendefinition::parseVAS(
    int id, std::string name, std::istream& from) {
    std::string tag;
    from >> tag;
    //
    double absdur;
    from >> absdur; // type-checking is missing!
    //
    tag = readEndSecure(from);
    double offset = 0;
    if (tag == "versatz") {
        from >> offset; // type-checking is missing!
    }
    return NIVissimTL::dictionary(id, "vas", name, (SUMOTime) absdur, (SUMOTime) offset);
}


bool
NIVissimSingleTypeParser_Lichtsignalanlagendefinition::parseRestActuated(
    int id, std::string name, std::istream& from, const std::string& type) {
    std::string tag;
    from >> tag;
    //
    double absdur;
    from >> absdur; // type-checking is missing!
    //
    tag = readEndSecure(from);
    double offset = 0;
    if (tag == "versatz") {
        from >> offset; // type-checking is missing!
    }
    while (tag != "datei") {
        tag = myRead(from);
    }
    return NIVissimTL::dictionary(id, type, name, (SUMOTime) absdur, (SUMOTime) offset);
}


/****************************************************************************/
