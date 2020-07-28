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
/// @file    NBCapacity2Lanes.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @date    Fri, 19 Jul 2002
///
// A helper class which computes the lane number from given capacity
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NBCapacity2Lanes
 * @brief A helper class which computes the lane number from given capacity
 *
 * A helper class for the computation of the number of lanes an edge has
 *  in dependence to this edge's capacity. The computation is done by the
 *  (trivial) assumption, the number of lanes increases linear with the
 *  number of lanes.
 */
class NBCapacity2Lanes {
public:
    /** @brief Donstructor
     *
     * @param[in] divider Value for the norming divider
     */
    NBCapacity2Lanes(double divider) : myDivider(divider) { }


    /// @brief Destructor
    ~NBCapacity2Lanes() { }


    /** @brief Returns the number of lanes computed from the given capacity
     *
     * Returns the estimated number of lanes by returning the given capacity
     *  divided by the norming divider given in the constructor.
     *
     * @param[in] capacity The capacity to convert
     * @return The capacity converted to the number of lanes
     */
    int get(double capacity) const {
        capacity /= myDivider;
        if (capacity > (int) capacity) {
            capacity += 1;
        }
        // just assure that the number of lanes is not zero
        if (capacity == 0) {
            capacity = 1;
        }
        return (int) capacity;
    }

private:
    /// @brief The norming divider
    double myDivider;

};
