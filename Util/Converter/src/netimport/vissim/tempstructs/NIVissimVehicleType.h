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
/// @file    NIVissimVehicleType.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>


#include <utils/common/RGBColor.h>
#include <string>
#include <map>

// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimVehicleType {
public:
    NIVissimVehicleType(const std::string& name,
                        const std::string& category, const RGBColor& color);
    ~NIVissimVehicleType();
    static bool dictionary(int id, const std::string& name,
                           const std::string& category, const RGBColor& color);
    static bool dictionary(int id, NIVissimVehicleType* o);
    static NIVissimVehicleType* dictionary(int id);
    static void clearDict();

private:
    std::string myName;
    std::string myCategory;
    RGBColor myColor;
private:
    typedef std::map<int, NIVissimVehicleType*> DictType;
    static DictType myDict;
};
