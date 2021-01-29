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
/// @file    LaneArea.h
/// @author  Michael Behrisch
/// @date    15.03.2017
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>


// ===========================================================================
// class declarations
// ===========================================================================
class MSE2Collector;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class LaneArea
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class LaneArea {
public:
    static std::vector<std::string> getIDList();
    static int getIDCount();
    static int getJamLengthVehicle(const std::string& detID);
    static double getJamLengthMeters(const std::string& detID);
    static double getLastStepMeanSpeed(const std::string& detID);
    static std::vector<std::string> getLastStepVehicleIDs(const std::string& detID);
    static double getLastStepOccupancy(const std::string& detID);
    static double getPosition(const std::string& detID);
    static std::string getLaneID(const std::string& detID);
    static double getLength(const std::string& detID);
    static int getLastStepVehicleNumber(const std::string& detID);
    static int getLastStepHaltingNumber(const std::string& detID);

    LIBSUMO_SUBSCRIPTION_API

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static MSE2Collector* getDetector(const std::string& detID);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

    /// @brief invalidated standard constructor
    LaneArea() = delete;

};
}
