/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    NIVissimVehTypeClass.h
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/common/RGBColor.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimVehTypeClass {
public:
    NIVissimVehTypeClass(int id, const std::string& name,
                         const RGBColor& color, std::vector<int>& types);
    ~NIVissimVehTypeClass();
    static bool dictionary(int id, const std::string& name,
                           const RGBColor& color, std::vector<int>& types);
    static bool dictionary(int id, NIVissimVehTypeClass* o);
    static NIVissimVehTypeClass* dictionary(int name);
    static void clearDict();
    double meanSpeed() const;
private:
    int myID;
    std::string myName;
    RGBColor myColor;
    std::vector<int> myTypes;
private:
    typedef std::map<int, NIVissimVehTypeClass*> DictType;
    static DictType myDict;
};
