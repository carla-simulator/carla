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
/// @file    VariableSpeedSign.h
/// @author  Jakob Erdmann
/// @date    16.03.2020
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <libsumo/TraCIDefs.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLaneSpeedTrigger;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class VariableSpeedSign
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class VariableSpeedSign {
public:

    static std::vector<std::string> getIDList();
    static int getIDCount();

    static std::vector<std::string> getLaneIDs(const std::string& vssID);

    //static std::vector<double> getTimes(const std::string& vssID);
    //static std::vector<double> getSpeeds(const std::string& vssID);
    //static double getSpeed(const std::string& vssID);

    //static void setSpeed(const std::string& vssID, double speed, double begTime, double endTime);

    static std::string getParameter(const std::string& vssID, const std::string& param);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    static void setParameter(const std::string& vssID, const std::string& key, const std::string& value); // not needed so far

    LIBSUMO_SUBSCRIPTION_API

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static MSLaneSpeedTrigger* getVariableSpeedSign(const std::string& vssID);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

    /// @brief invalidated standard constructor
    VariableSpeedSign() = delete;
};


}
