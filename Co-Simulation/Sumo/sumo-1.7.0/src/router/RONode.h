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
/// @file    RONode.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// Base class for nodes used by the router
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/common/Named.h>
#include <utils/geom/Position.h>

// ===========================================================================
// class declarations
// ===========================================================================
class ROEdge;

typedef std::vector<const ROEdge*> ConstROEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RONode
 * @brief Base class for nodes used by the router
 */
class RONode : public Named {
public:
    /** @brief Constructor
     * @param[in] id The id of the node
     */
    RONode(const std::string& id);


    /// @brief Destructor
    ~RONode();


    /** @brief Sets the position of the node
     * @param[in] p The node's position
     */
    void setPosition(const Position& p);


    /** @brief Returns the position of the node
     * @return This node's position
     */
    const Position& getPosition() const {
        return myPosition;
    }


    inline const ConstROEdgeVector& getIncoming() const {
        return myIncoming;
    }

    inline const ConstROEdgeVector& getOutgoing() const {
        return myOutgoing;
    }

    void addIncoming(ROEdge* edge) {
        myIncoming.push_back(edge);
    }

    void addOutgoing(ROEdge* edge) {
        myOutgoing.push_back(edge);
    }

private:
    /// @brief This node's position
    Position myPosition;

    /// @brief incoming edges
    ConstROEdgeVector myIncoming;
    /// @brief outgoing edges
    ConstROEdgeVector myOutgoing;


private:
    /// @brief Invalidated copy constructor
    RONode(const RONode& src);

    /// @brief Invalidated assignment operator
    RONode& operator=(const RONode& src);

};
