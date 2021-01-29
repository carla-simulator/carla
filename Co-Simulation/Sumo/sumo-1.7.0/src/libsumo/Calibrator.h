/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    Calibrator.h
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <libsumo/TraCIDefs.h>
#include <microsim/trigger/MSCalibrator.h>


// ===========================================================================
// class declarations
// ===========================================================================
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Calibrator
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class Calibrator {
public:

    static std::vector<std::string> getIDList();
    static int getIDCount();
    static std::string getEdgeID(const std::string& calibratorID);
    static std::string getLaneID(const std::string& calibratorID);
    static double getVehsPerHour(const std::string& calibratorID);
    static double getSpeed(const std::string& calibratorID);
    static std::string getTypeID(const std::string& calibratorID);
    static double getBegin(const std::string& calibratorID);
    static double getEnd(const std::string& calibratorID);
    static std::string getRouteID(const std::string& calibratorID);
    static std::string getRouteProbeID(const std::string& calibratorID);
    static std::vector<std::string> getVTypes(const std::string& routeID);
    static int getPassed(const std::string& calibratorID);
    static int getInserted(const std::string& calibratorID);
    static int getRemoved(const std::string& calibratorID);
    static std::string getParameter(const std::string& calibratorID, const std::string& param);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    static void setFlow(const std::string& calibratorID, double begin, double end, double vehsPerHour, double speed, const std::string& typeID,
                        const std::string& routeID,
                        const std::string& departLane = "first",
                        const std::string& departSpeed = "max");

    static void setParameter(const std::string& calibratorID, const std::string& key, const std::string& value); // not needed so far

    LIBSUMO_SUBSCRIPTION_API

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static MSCalibrator* getCalibrator(const std::string& id);
    static MSCalibrator::AspiredState getCalibratorState(const MSCalibrator* c);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

    /// @brief invalidated standard constructor
    Calibrator() = delete;
};


}
