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
/// @file    Edge.h
/// @author  Gregor Laemmel
/// @date    15.09.2017
///
// C++ TraCI client API implementation
/****************************************************************************/

#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <memory>
#include <libsumo/TraCIDefs.h>
#include <libsumo/TraCIConstants.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class PositionVector;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Edge
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {

class Edge {
public:
    static std::vector<std::string> getIDList();
    static int getIDCount();
    static double getAdaptedTraveltime(const std::string& id, double time);
    static double getEffort(const std::string& id, double time);
    static double getTraveltime(const std::string& id);
    static double getWaitingTime(const std::string& id);
    static const std::vector<std::string> getLastStepPersonIDs(const std::string& id);
    static const std::vector<std::string> getLastStepVehicleIDs(const std::string& id);
    static double getCO2Emission(const std::string& id);
    static double getCOEmission(const std::string& id);
    static double getHCEmission(const std::string& id);
    static double getPMxEmission(const std::string& id);
    static double getNOxEmission(const std::string& id);
    static double getFuelConsumption(const std::string& id);
    static double getNoiseEmission(const std::string& id);
    static double getElectricityConsumption(const std::string& id);
    static int getLastStepVehicleNumber(const std::string& id);
    static double getLastStepMeanSpeed(const std::string& id);
    static double getLastStepOccupancy(const std::string& id);
    static int getLastStepHaltingNumber(const std::string& id);
    static double getLastStepLength(const std::string& id);
    static int getLaneNumber(const std::string& id);
    static std::string getStreetName(const std::string& id);
    static std::string getParameter(const std::string& id, const std::string& paramName);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API
    static void setAllowedVehicleClasses(const std::string& id, std::vector<std::string> vector);
    static void setDisallowedVehicleClasses(const std::string& id, std::vector<std::string> classes);
    static void setAllowedSVCPermissions(const std::string& id, int permissions);
    static void adaptTraveltime(const std::string& id, double value, double begTime = 0., double endTime = std::numeric_limits<double>::max());
    static void setEffort(const std::string& id, double value, double begTime = 0., double endTime = std::numeric_limits<double>::max());
    static void setMaxSpeed(const std::string& id, double value);
    static void setParameter(const std::string& id, const std::string& name, const std::string& value);

    LIBSUMO_SUBSCRIPTION_API

    /** @brief Saves the shape of the requested object in the given container
    *  @param id The id of the edge to retrieve
    *  @param shape The container to fill
    */
    static void storeShape(const std::string& id, PositionVector& shape);

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static MSEdge* getEdge(const std::string& id);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

private:
    /// @brief invalidated standard constructor
    Edge() = delete;

};

}
