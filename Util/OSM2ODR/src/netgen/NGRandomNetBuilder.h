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
/// @file    NGRandomNetBuilder.h
/// @author  Markus Hartinger
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mar, 2003
///
// Additional structures for building random nets
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <utils/distribution/RandomDistributor.h>
#include "NGNet.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NGRandomNetBuilder
 * @brief A class that builds random network using an algorithm by Markus Hartinger.
 *
 * @todo Describe the algorithm
 */
class NGRandomNetBuilder {
public:
    /** @brief Constructor
     *
     * @param[in] net The network to fill with generated structures
     * @param[in] minAngle The minimum allowed angle between two streets
     * @param[in] minDistance The minimum allowed distance between two nodes
     * @param[in] maxDistance The maximum allowed distance between two nodes
     * @param[in] connectivity The connectivity factor
     * @param[in] numTries ?
     * @todo check meanings of connectivity/numTries
     */
    NGRandomNetBuilder(NGNet& net, double minAngle, double minDistance, double maxDistance, double connectivity,
                       int numTries, const RandomDistributor<int>& neighborDist);



    /** @brief Builds a NGNet using the set values
     *
     * @param[in] numNodes Number of iterations (node insertions) to perform
     * @todo Describe the algorithm
     */
    void createNet(int numNodes, bool gridMode);


private:
    /** @brief Removes the given node from the list of outer nodes
     *
     * @param[in] node The node to remove
     */
    void removeOuterNode(NGNode* node);


    /** @brief Checks whether the angle of this node's connections are valid
     *
     * Checks whether the connections of the nodes are in common with the value of myMinLinkAngle.
     *
     * @param[in] node The node to check connections of
     * @return Whether the settings allow to connect both nodes
     */
    bool checkAngles(NGNode* node);


    /** @brief Checks whether connecting the given two nodes complies with the set restrictions
     *
     * Checks whether the distance, the angle, and the connectivity is within the defined range
     *  when both nodes would be connected
     *
     * @param[in] baseNode The first node of the combination to check
     * @param[in] newNode The second node of the combination to check
     * @return Whether the settings allow to connect both nodes
     */
    bool canConnect(NGNode* baseNode, NGNode* newNode);


    /** @brief Creates new random node
     *
     * Returns true, if creation was successfull.
     *
     * @param[in] baseNode ?
     * @todo Describe better
     */
    bool createNewNode(NGNode* baseNode, bool gridMode);


    /** @brief finds possible connections between Node and OuterNodes complying with restrictions
     *
     * @param[in] node ?
     * @todo Describe better
     */
    void findPossibleOuterNodes(NGNode* node);


private:
    /// @brief The network to fill
    NGNet& myNet;

    /// @brief The list of outer nodes
    NGNodeList myOuterNodes;

    /// @brief The list of outer links
    NGEdgeList myOuterLinks;

    // list of possible new connections
    NGNodeList myConNodes;


    /// @name restrictions
    //@{

    /// @brief Minimum angle allowed between two links
    double myMinLinkAngle;

    /// @brief Minimum distance allowed between two nodes
    double myMinDistance;

    /// @brief Maximum distance allowed between two nodes
    double myMaxDistance;

    /// @brief Probability of connecting to a existing node if possible
    double myConnectivity;
    //@}


    /// @brief Number of tries to create a new node
    int myNumTries;

    /// @brief Number of nodes to be created
    int myNumNodes;

    /// @brief The distribution of number of neighbours
    RandomDistributor<int> myNeighbourDistribution;

private:
    /// @brief Invalidated copy constructor.
    NGRandomNetBuilder(const NGRandomNetBuilder&);

    /// @brief Invalidated assignment operator.
    NGRandomNetBuilder& operator=(const NGRandomNetBuilder&);

};
