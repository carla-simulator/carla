/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    ROJTREdgeBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    Tue, 20 Jan 2004
///
// Interface for building instances of jtrrouter-edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <router/ROAbstractEdgeBuilder.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROJTREdgeBuilder
 * @brief Interface for building instances of jtrrouter-edges
 *
 * This ROAbstractEdgeBuilder implementation builds edges for the jtrrouter
 *  (instances of ROJTREdge).
 *
 * @see ROJTREdge
 */
class ROJTREdgeBuilder : public ROAbstractEdgeBuilder {
public:
    /// @brief Constructor
    ROJTREdgeBuilder();


    /// @brief Destructor
    ~ROJTREdgeBuilder();


    /// @name Methods inherited from ROAbstractEdgeBuilder
    /// @{

    /** @brief Builds an edge with the given name
     *
     * This implementation builds a ROJTREdge.
     *
     * @param[in] name The name of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] priority The edge priority (road class)
     * @return A proper instance of the named edge
     * @see ROJTREdge
     */
    ROEdge* buildEdge(const std::string& name, RONode* from, RONode* to, const int priority);
    /// @}


};
