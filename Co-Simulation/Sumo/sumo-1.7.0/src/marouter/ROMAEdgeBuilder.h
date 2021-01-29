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
/// @file    ROMAEdgeBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Interface for building instances of duarouter-edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <router/ROAbstractEdgeBuilder.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class RONode;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROMAEdgeBuilder
 * @brief Interface for building instances of duarouter-edges
 *
 * This ROAbstractEdgeBuilder implementation builds edges for the duarouter
 *  (instances of ROEdge).
 *
 * @see ROEdge
 */
class ROMAEdgeBuilder : public ROAbstractEdgeBuilder {
public:
    /** @brief Constructor
     */
    ROMAEdgeBuilder();


    /// @brief Destructor
    ~ROMAEdgeBuilder();


    /// @name Methods to be implemented, inherited from ROAbstractEdgeBuilder
    /// @{

    /** @brief Builds an edge with the given name
     *
     * This implementation builds a ROEdge.
     *
     * @param[in] name The name of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] priority The edge priority (road class)
     * @return A proper instance of the named edge
     * @see ROEdge
     */
    ROEdge* buildEdge(const std::string& name, RONode* from, RONode* to, const int priority);
    /// @}


};
