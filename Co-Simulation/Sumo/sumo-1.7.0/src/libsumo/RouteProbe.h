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
/// @file    RouteProbe.h
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
class MSRouteProbe;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RouteProbe
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class RouteProbe {
public:

    static std::vector<std::string> getIDList();
    static int getIDCount();

    static std::string getEdgeID(const std::string& probeID);
    //static double getFrequency(const std::string& meanDataID);

    static std::string sampleLastRouteID(const std::string& probeID);
    static std::string sampleCurrentRouteID(const std::string& probeID);
    //static std::vector<std::pair<std::string, doube> > getLastRoutes(const std::string& probeID);
    //static std::vector<std::pair<std::string, doube> > getCurrentRoutes(const std::string& probeID);

    static std::string getParameter(const std::string& probeID, const std::string& param);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    static void setParameter(const std::string& probeID, const std::string& key, const std::string& value); // not needed so far

    LIBSUMO_SUBSCRIPTION_API

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static MSRouteProbe* getRouteProbe(const std::string& id);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;

    /// @brief invalidated standard constructor
    RouteProbe() = delete;
};


}
