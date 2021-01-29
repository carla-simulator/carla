/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSLCHelper.h
/// @author  Jakob Erdmann
/// @date    Fri, 19.06.2020
///
// Common functions for lane change models
/****************************************************************************/
#pragma once
#include <config.h>
#include <vector>

// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLCHelper
 * @brief A lane change model developed by J. Erdmann
 */
class MSLCHelper {
public:

    /// @brief Computes the artificial bonus distance for roundabout lanes
    ///        this additional distance reduces the sense of urgency within
    ///        roundabouts and thereby promotes the use of the inner roundabout
    ///        lane in multi-lane roundabouts.
    /// @param[in] curr continuation info along veh's current lane
    /// @param[in] neigh continuation info along a neighboring lane (in MSLCM_2013::_wantsChange() the considered lane for a lanechange)
    static double getRoundaboutDistBonus(const MSVehicle& veh,
                                         double bonusParam,
                                         const MSVehicle::LaneQ& curr,
                                         const MSVehicle::LaneQ& neigh,
                                         const MSVehicle::LaneQ& best);
};
