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
/// @file    Junction.h
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
#include <libsumo/TraCIConstants.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NamedRTree;
class MSJunction;
class PositionVector;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Junction
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class Junction {
public:
    static std::vector<std::string> getIDList();
    static int getIDCount();
    static TraCIPosition getPosition(const std::string& junctionID, const bool includeZ = false);
    static TraCIPositionVector getShape(const std::string& junctionID);

    LIBSUMO_SUBSCRIPTION_API

    /** @brief Returns a tree filled with junction instances
     * @return The rtree of junctions
     */
    static NamedRTree* getTree();
    static void cleanup();

    /** @brief Saves the shape of the requested object in the given container
    *  @param id The id of the poi to retrieve
    *  @param shape The container to fill
    */
    static void storeShape(const std::string& id, PositionVector& shape);

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static MSJunction* getJunction(const std::string& id);

private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;
    static NamedRTree* myTree;

private:
    /// @brief invalidated standard constructor
    Junction() = delete;
};
}
