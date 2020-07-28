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
/// @file    NIVissimTrafficDescription.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>


#include <string>
#include <map>
#include "NIVissimVehicleClassVector.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimTrafficDescription {
public:
    NIVissimTrafficDescription(const std::string& name,
                               const NIVissimVehicleClassVector& vehicleTypes);
    ~NIVissimTrafficDescription();
    static bool dictionary(int id, const std::string& name,
                           const NIVissimVehicleClassVector& vehicleTypes);
    static bool dictionary(int id, NIVissimTrafficDescription* o);
    static NIVissimTrafficDescription* dictionary(int id);
    static void clearDict();
    static double meanSpeed(int id);
    double meanSpeed() const;
private:
    std::string myName;
    NIVissimVehicleClassVector myVehicleTypes;
private:
    typedef std::map<int, NIVissimTrafficDescription*> DictType;
    static DictType myDict;
};
