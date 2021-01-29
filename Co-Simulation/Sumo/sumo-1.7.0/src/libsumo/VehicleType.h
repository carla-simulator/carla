/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    VehicleType.h
/// @author  Gregor Laemmel
/// @date    04.04.2017
///
// C++ TraCI client API implementation
/****************************************************************************/

#pragma once
#include <string>
#include <libsumo/TraCIDefs.h>
#include <libsumo/TraCIConstants.h>


#define LIBSUMO_VEHICLE_TYPE_GETTER \
static double getLength(const std::string& typeID); \
static double getMaxSpeed(const std::string& typeID); \
static double getActionStepLength(const std::string& typeID); \
static double getSpeedFactor(const std::string& typeID); \
static double getSpeedDeviation(const std::string& typeID); \
static double getAccel(const std::string& typeID); \
static double getDecel(const std::string& typeID); \
static double getEmergencyDecel(const std::string& typeID); \
static double getApparentDecel(const std::string& typeID); \
static double getImperfection(const std::string& typeID); \
static double getTau(const std::string& typeID); \
static std::string getVehicleClass(const std::string& typeID); \
static std::string getEmissionClass(const std::string& typeID); \
static std::string getShapeClass(const std::string& typeID); \
static double getMinGap(const std::string& typeID); \
static double getWidth(const std::string& typeID); \
static double getHeight(const std::string& typeID); \
static TraCIColor getColor(const std::string& typeID); \
static double getMinGapLat(const std::string& typeID); \
static double getMaxSpeedLat(const std::string& typeID); \
static std::string getLateralAlignment(const std::string& typeID); \
static int getPersonCapacity(const std::string& typeID);

#define LIBSUMO_VEHICLE_TYPE_SETTER \
static void setLength(const std::string& typeID, double length); \
static void setMaxSpeed(const std::string& typeID, double speed); \
static void setVehicleClass(const std::string& typeID, const std::string& clazz); \
static void setSpeedFactor(const std::string& typeID, double factor); \
static void setEmissionClass(const std::string& typeID, const std::string& clazz); \
static void setShapeClass(const std::string& typeID, const std::string& shapeClass); \
static void setWidth(const std::string& typeID, double width); \
static void setHeight(const std::string& typeID, double height); \
static void setMinGap(const std::string& typeID, double minGap); \
static void setAccel(const std::string& typeID, double accel); \
static void setDecel(const std::string& typeID, double decel); \
static void setEmergencyDecel(const std::string& typeID, double decel); \
static void setApparentDecel(const std::string& typeID, double decel); \
static void setImperfection(const std::string& typeID, double imperfection); \
static void setTau(const std::string& typeID, double tau); \
static void setColor(const std::string& typeID, const TraCIColor& c); \
static void setMinGapLat(const std::string& typeID, double minGapLat); \
static void setMaxSpeedLat(const std::string& typeID, double speed); \
static void setLateralAlignment(const std::string& typeID, const std::string& latAlignment); \
static void setActionStepLength(const std::string& typeID, double actionStepLength, bool resetActionOffset=true);

// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicleType;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class VehicleType
* @brief C++ TraCI client API implementation
*/
namespace libsumo {
class VehicleType {
public:
    static std::vector<std::string> getIDList();
    static int getIDCount();
    LIBSUMO_VEHICLE_TYPE_GETTER
    static std::string getParameter(const std::string& typeID, const std::string& key);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    LIBSUMO_VEHICLE_TYPE_SETTER
    static void copy(const std::string& origTypeID, const std::string& newTypeID);
    static MSVehicleType* getVType(std::string id);

    static void setSpeedDeviation(const std::string& typeID, double deviation);
    static void setParameter(const std::string& id, const std::string& name, const std::string& value);

    LIBSUMO_SUBSCRIPTION_API

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

    static bool handleVariableWithID(const std::string& objID, const std::string& typeID, const int variable, VariableWrapper* wrapper);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

private:
    /// @brief invalidated standard constructor
    VehicleType() = delete;
};


}
