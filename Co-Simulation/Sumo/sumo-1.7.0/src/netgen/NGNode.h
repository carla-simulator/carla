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
/// @file    NGNode.h
/// @author  Markus Hartinger
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mar, 2003
///
// A netgen-representation of a node
/****************************************************************************/
#pragma once
#include <config.h>

#include <list>
#include <utils/common/Named.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <utils/common/UtilExceptions.h>
#include "NGEdge.h"


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class NBEdge;
class NBNetBuilder;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NGNode
 * @brief A netgen-representation of a node
 */
class NGNode : public Named {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the node
     */
    NGNode(const std::string& id);


    /** @brief Constructor
     *
     * @param[in] id The id of the node
     * @param[in] xPos The x-position of the node
     * @param[in] yPos The y-position of the node
     */
    NGNode(const std::string& id, int xPos, int yPos);


    /** @brief Constructor
     *
     * @param[in] id The id of the node
     * @param[in] xPos The x-position of the node
     * @param[in] yPos The y-position of the node
     * @param[in] amCenter Information whether this is the center-node of a spider-net
     */
    NGNode(const std::string& id, int xID, int yID, bool amCenter);


    /// @brief Destructor
    ~NGNode();


    /** @brief Returns this node's position
     *
     * @return The position of the node
     */
    const Position& getPosition() const {
        return myPosition;
    }


    /** @brief Returns this node's maximum neighbour number
     *
     * @return The maximum neighbour number of the node
     */
    int getMaxNeighbours() {
        return myMaxNeighbours;
    }


    /** @brief Sets this node's maximum neighbour number
     *
     * @param[in] value The new maximum neighbour number of the node
     */
    void setMaxNeighbours(int value) {
        myMaxNeighbours = value;
    }


    /** @brief Sets a new value for x-position
     *
     * @param[in] value The new x-position of this node
     */
    void setX(double x) {
        myPosition.set(x, myPosition.y());
    }


    /** @brief Sets a new value for y-position
     *
     * @param[in] value The new y-position of this node
     */
    void setY(double y) {
        myPosition.set(myPosition.x(), y);
    }

    /// @brief mark node as fringe
    void setFringe() {
        myAmFringe = true;
    }

    /** @brief Builds and returns this node's netbuild-representation
     *
     * The position of the node is transformed to cartesian using GeoConvHelper::x2cartesian,
     *  first. If this node is the center node of a spider net, a node of the type
     *  NBNode::SumoXMLNodeType::NOJUNCTION is returned.
     * Otherwise, a plain node is built and it is checked whether the options
     *  indicate building one of the tls node-types. In this case, a logic is built and
     *  stored. A ProcessError is thrown if this fails (should never happen, in fact).
     *
     * @param[in] nb The netbuilder to retrieve the tls-container from
     * @return The built node
     * @exception ProcessError If the built tls logic could not be added (should never happen)
     * @todo There is no interaction with explicit node setting options? Where is this done?
     * @todo Check whether throwing an exception is really necessary, here
     */
    NBNode* buildNBNode(NBNetBuilder& nb, const Position& perturb) const;


    /** @brief Adds the given link to the internal list
     *
     * @param[in] link The link to add
     */
    void addLink(NGEdge* link);


    /** @brief Removes the given link
     *
     * The given pointer is compared to those in the list. A matching
     *  pointer is removed, not other same connections.
     *
     * @param[in] link The link to remove
     */
    void removeLink(NGEdge* link);


    /** @brief Returns whether the other node is connected
     *
     * @param[in] node The link to check whether it is connected
     * @return Whether the given node is connected
     */
    bool connected(NGNode* node) const;


    /** @brief Returns whether the node has the given position
     *
     * @param[in] node The link to check whether it is connected
     * @return Whether the given node is connected
     */
    bool samePos(int xPos, int yPos) const {
        return xID == xPos && yID == yPos;
    }

    // NGRandomNetBuilder needs access to links
    friend class NGRandomNetBuilder;

private:
    /// @brief Integer x-position (x-id)
    int xID;

    /// @brief Integer y-position (y-id)
    int yID;

    /// @brief List of connected links
    NGEdgeList LinkList;

    /// @brief The position of the node
    Position myPosition;

    /// @brief The maximum number of neighbours
    int myMaxNeighbours;

    /// @brief Information whether this is the center of a cpider-net
    bool myAmCenter;

    /// @brief Information whether this is the center of a cpider-net
    bool myAmFringe;

};

/**
 * @typedef NGNodeList
 * @brief A list of nodes (node pointers)
 */
typedef std::list<NGNode*> NGNodeList;
