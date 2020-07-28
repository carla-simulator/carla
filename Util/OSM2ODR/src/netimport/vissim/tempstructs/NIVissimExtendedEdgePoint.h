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
/// @file    NIVissimExtendedEdgePoint.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class NIVissimExtendedEdgePoint {
public:
    /** @brief Constructor
     * @param[in] edgeid The id of the Vissim-edge
     * @param[in] lanes Lanes on which this point lies
     * @param[in] position The position of this point at the edge
     * @param[in] assignedVehicles Vehicle (type) indices which should be regarded by this point
     */
    NIVissimExtendedEdgePoint(int edgeid, const std::vector<int>& lanes,
                              double position, const std::vector<int>& assignedVehicles);
    ~NIVissimExtendedEdgePoint();
    int getEdgeID() const;
    double getPosition() const;
    Position getGeomPosition() const;
    const std::vector<int>& getLanes() const;


    /** @brief Resets lane numbers if all lanes shall be used
     *
     * If myLanes contains a -1, the content of myLanes is replaced
     *  by indices of all lanes of the given edge.
     *
     * @param[in] The built edge
     */
    void recheckLanes(const NBEdge* const edge);

private:
    int myEdgeID;
    std::vector<int> myLanes;
    double myPosition;
    std::vector<int> myAssignedVehicles;
};
