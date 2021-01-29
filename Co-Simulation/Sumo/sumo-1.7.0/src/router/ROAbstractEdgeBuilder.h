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
/// @file    ROAbstractEdgeBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Yun-Pang Floetteroed
/// @date    Wed, 21 Jan 2004
///
// Interface for building instances of router-edges
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>


// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;
class RONode;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROAbstractEdgeBuilder
 * @brief Interface for building instances of router-edges
 *
 * As the different routing algorithms may need certain types of edges,
 *  edges are build via a factory object derived from this class.
 *
 * The only method to be implemented is "buildEdge" which builds an edge
 *  of the needed ROEdge-subtype.
 *
 * The built edges are numbered in the order they are built, the current
 *  number (index) is stored in "myCurrentIndex" and the next to use may
 *  be obtained via "getNextIndex".
 */
class ROAbstractEdgeBuilder {
public:
    /// @brief Constructor
    ROAbstractEdgeBuilder() : myCurrentIndex(0) { }


    /// @brief Destructor
    virtual ~ROAbstractEdgeBuilder() { }


    /// @name Methods to be implemented
    /// @{

    /** @brief Builds an edge with the given name
     *
     * @param[in] name The name of the edge
     * @param[in] from The node the edge begins at
     * @param[in] to The node the edge ends at
     * @param[in] priority The edge priority (road class)
     * @return A proper instance of the named edge
     */
    virtual ROEdge* buildEdge(const std::string& name, RONode* from, RONode* to, const int priority) = 0;
    /// @}


protected:
    /** @brief Returns the index of the edge to built
     * @return Next valid edge index
     */
    int getNextIndex() {
        return myCurrentIndex++;
    }


private:
    /// @brief The next edge's index
    int myCurrentIndex;


private:
    /// @brief Invalidated copy constructor
    ROAbstractEdgeBuilder(const ROAbstractEdgeBuilder& src);

    /// @brief Invalidated assignment operator
    ROAbstractEdgeBuilder& operator=(const ROAbstractEdgeBuilder& src);

};
