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
/// @file    Route.h
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @date    30.05.2012
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
class MSRoute;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Route
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class Route {
public:

    static std::vector<std::string> getIDList();
    static int getIDCount();
    static std::vector<std::string> getEdges(const std::string& routeID);
    static std::string getParameter(const std::string& routeID, const std::string& param);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    static void add(const std::string& routeID, const std::vector<std::string>& edges);
    static void setParameter(const std::string& routeID, const std::string& param, const std::string& value); // not needed so far

    LIBSUMO_SUBSCRIPTION_API

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static const MSRoute* getRoute(const std::string& id);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

    /// @brief invalidated standard constructor
    Route() = delete;
};


}
