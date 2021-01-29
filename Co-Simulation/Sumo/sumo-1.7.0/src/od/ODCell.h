/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    ODCell.h
/// @author  Peter Mieth
/// @author  Daniel Krajzewicz
/// @author  Yun-Pang Floetteroed
/// @date    Sept 2002
///
// A single O/D-matrix cell
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class RORoute;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @struct ODCell
 * @brief A single O/D-matrix cell
 *
 * A single cell within an O/D-matrix. Contains the information about the origin
 *  and destination via string-ids of the district, the begin and the end time
 *  for which this cell is valid, the id of the vehicle type to use, and the
 *  amount of vehicles to insert during the described interval.
 */
struct ODCell {
    /// @brief The number of vehicles
    double vehicleNumber;

    /// @brief The begin time this cell describes
    SUMOTime begin;

    /// @brief The end time this cell describes
    SUMOTime end;

    /// @brief Name of the origin district
    std::string origin;

    /// @brief Name of the destination district
    std::string destination;

    /// @brief Name of the vehicle type
    std::string vehicleType;

    /// @brief the list of paths / routes
    std::vector<RORoute*> pathsVector;  // path_id, string of edges?

    /// @brief mapping of departure times to departing vehicles, if already fixed
    std::map<SUMOTime, std::vector<std::string> > departures;

    /// @brief the origin "district" is an edge id
    bool originIsEdge = false;

    /// @brief the destination "district" is an edge id
    bool destinationIsEdge = false;
};
