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
/// @file    NINavTeqHelper.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Jul 2006
///
// Some parser methods shared around several formats containing NavTeq-Nets
/****************************************************************************/
#include <config.h>

#include "NINavTeqHelper.h"
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <netbuild/NBEdge.h>


// ===========================================================================
// method definitions
// ===========================================================================
double
NINavTeqHelper::getSpeed(const std::string& id, const std::string& speedClassS) {
    try {
        int speedClass = StringUtils::toInt(speedClassS);
        switch (speedClass) {
            case -1:
                return (double) 1.0 / (double) 3.6;
            case 1:
                return (double) 200 / (double) 3.6; //> 130 KPH / > 80 MPH
            case 2:
                return (double) 120 / (double) 3.6; //101-130 KPH / 65-80 MPH
            case 3:
                return (double) 100 / (double) 3.6; // 91-100 KPH / 55-64 MPH
            case 4:
                return (double) 80 / (double) 3.6; // 71-90 KPH / 41-54 MPH
            case 5:
                return (double) 70 / (double) 3.6; // 51-70 KPH / 31-40 MPH
            case 6:
                return (double) 50 / (double) 3.6; // 31-50 KPH / 21-30 MPH
            case 7:
                return (double) 30 / (double) 3.6; // 11-30 KPH / 6-20 MPH
            case 8:
                return (double) 5 / (double) 3.6; //< 11 KPH / < 6 MPH
            default:
                throw ProcessError("Invalid speed code (edge '" + id + "').");
        }
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for an edge's speed type occurred (edge '" + id + "').");
    }
}


int
NINavTeqHelper::getLaneNumber(const std::string& id, const std::string& laneNoS, double speed) {
    try {
        int nolanes = StringUtils::toInt(laneNoS);
        if (nolanes < 0) {
            return 1;
        } else if (nolanes / 10 > 0) {
            return nolanes / 10;
        } else {
            switch (nolanes % 10) {
                case 1:
                    return 1;
                case 2:
                    nolanes = 2;
                    if (speed > 78.0 / 3.6) {
                        nolanes = 3;
                    }
                    return nolanes;
                case 3:
                    return 4;
                default:
                    throw ProcessError("Invalid lane number (edge '" + id + "').");
            }
        }
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for an edge's lane number occurred (edge '" + id + "'.");
    }
}


bool
NINavTeqHelper::addCommonVehicleClasses(NBEdge& e, const std::string& classS, const int offset) {
    bool haveCar = false;
    // High Occupancy Vehicle -- becomes SVC_PASSENGER|SVC_HOV
    if (classS[offset] == '1') {
        e.allowVehicleClass(-1, SVC_HOV);
        e.allowVehicleClass(-1, SVC_PASSENGER);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_HOV);
    }
    // Emergency Vehicle -- becomes SVC_EMERGENCY
    if (classS[offset + 1] == '1') {
        e.allowVehicleClass(-1, SVC_EMERGENCY);
    } else {
        e.disallowVehicleClass(-1, SVC_EMERGENCY);
    }
    // Taxi -- becomes SVC_TAXI
    if (classS[offset + 2] == '1') {
        e.allowVehicleClass(-1, SVC_TAXI);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_TAXI);
    }
    // Public Bus -- becomes SVC_BUS|SVC_COACH
    if (classS[offset + 3] == '1') {
        e.allowVehicleClass(-1, SVC_BUS);
        e.allowVehicleClass(-1, SVC_COACH);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_BUS);
        e.disallowVehicleClass(-1, SVC_COACH);
    }
    // Delivery Truck -- becomes SVC_DELIVERY
    if (classS[offset + 4] == '1') {
        e.allowVehicleClass(-1, SVC_DELIVERY);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_DELIVERY);
    }
    // Transport Truck -- becomes SVC_TRUCK|SVC_TRAILER
    if (classS[offset + 5] == '1') {
        e.allowVehicleClass(-1, SVC_TRUCK);
        e.allowVehicleClass(-1, SVC_TRAILER);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_TRUCK);
        e.disallowVehicleClass(-1, SVC_TRAILER);
    }
    return haveCar;
}


void
NINavTeqHelper::addVehicleClasses(NBEdge& e, const std::string& oclassS, const SVCPermissions allPermissions, const SVCPermissions defaultPermissions) {
    std::string classS = "0000000000" + oclassS;
    classS = classS.substr(classS.length() - 10);
    // 0: allow all vehicle types
    if (classS[0] == '1') {
        e.setPermissions(allPermissions);
        return;
    }
    bool haveCar = false;
    e.setPermissions(defaultPermissions);
    // Passenger cars -- becomes SVC_PASSENGER
    if (classS[1] == '1') {
        e.allowVehicleClass(-1, SVC_PASSENGER);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_PASSENGER);
    }
    haveCar |= addCommonVehicleClasses(e, classS, 2);
    if (!haveCar) {
        e.setPermissions(0);
    }
    // Bicycle -- becomes SVC_BICYCLE
    if (classS[8] == '1') {
        e.allowVehicleClass(-1, SVC_BICYCLE);
    } else {
        e.disallowVehicleClass(-1, SVC_BICYCLE);
    }
    // Pedestrian -- becomes SVC_PEDESTRIAN
    if (classS[9] == '1') {
        e.allowVehicleClass(-1, SVC_PEDESTRIAN);
    } else {
        e.disallowVehicleClass(-1, SVC_PEDESTRIAN);
    }
}


void
NINavTeqHelper::addVehicleClassesV6(NBEdge& e, const std::string& oclassS, const SVCPermissions allPermissions, const SVCPermissions defaultPermissions) {
    std::string classS = "0000000000" + oclassS;
    classS = classS.substr(classS.length() - 12);
    // 0: allow all vehicle types
    if (classS[0] == '1') {
        e.setPermissions(allPermissions);
        return;
    }
    bool haveCar = false;
    e.setPermissions(defaultPermissions);
    // Passenger cars -- becomes SVC_PASSENGER
    if (classS[1] == '1') {
        e.allowVehicleClass(-1, SVC_PASSENGER);
        haveCar = true;
    } else {
        e.disallowVehicleClass(-1, SVC_PASSENGER);
    }
    // Residential Vehicle -- becomes SVC_PASSENGER
    if (classS[2] == '1') {
        e.allowVehicleClass(-1, SVC_PASSENGER);
        haveCar = true;
    }
    haveCar |= addCommonVehicleClasses(e, classS, 3);
    if (!haveCar) {
        e.setPermissions(0);
    }
    // Motorcycle -- becomes SVC_MOTORCYCLE
    if (classS[9] == '1') {
        e.allowVehicleClass(-1, SVC_MOTORCYCLE);
    } else {
        e.disallowVehicleClass(-1, SVC_MOTORCYCLE);
    }
    // Bicycle -- becomes SVC_BICYCLE
    if (classS[10] == '1') {
        e.allowVehicleClass(-1, SVC_BICYCLE);
    } else {
        e.disallowVehicleClass(-1, SVC_BICYCLE);
    }
    // Pedestrian -- becomes SVC_PEDESTRIAN
    if (classS[11] == '1') {
        e.allowVehicleClass(-1, SVC_PEDESTRIAN);
    } else {
        e.disallowVehicleClass(-1, SVC_PEDESTRIAN);
    }
}


/****************************************************************************/
