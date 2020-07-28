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
/// @file    NBHelpers.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Tue, 20 Nov 2001
///
// Some mathematical helper methods
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <set>


// ===========================================================================
// class declarations
// ===========================================================================
class NBNode;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBHelpers
 * Some mathmatical methods for the computation of angles
 */
class NBHelpers {
public:
    /// @brief computes the relative angle between the two angles
    static double relAngle(double angle1, double angle2);

    /// @brief ensure that reverse relAngles (>=179.999) always count as turnarounds (-180)
    static double normRelAngle(double angle1, double angle2);

    /// @brief converts the numerical id to its "normal" string representation
    static std::string normalIDRepresentation(const std::string& id);

    /// @brief returns the distance between both nodes
    static double distance(NBNode* node1, NBNode* node2);

    /// @brief Add edge ids defined in file (either ID or edge:ID per line) into the given set
    static void loadEdgesFromFile(const std::string& file, std::set<std::string>& into);

    /// @brief Add prefixed ids defined in file
    static void loadPrefixedIDsFomFile(const std::string& file, const std::string prefix,  std::set<std::string>& into);

    /** @brief parses edge-id and index from lane-id
     * @param[in] lane_id The lane-id
     * @param[out] edge_id ID of this lane's edge
     * @param[out] index Index of this lane
     */
    static void interpretLaneID(const std::string& lane_id, std::string& edge_id, int& index);
};
