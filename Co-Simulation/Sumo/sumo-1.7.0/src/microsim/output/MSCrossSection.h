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
/// @file    MSCrossSection.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @date    Tue Nov 25 15:23:28 2003
///
// A simple description of a position on a lane (crossing of a lane)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSCrossSection
 * @brief A simple description of a position on a lane (crossing of a lane)
 */
class MSCrossSection {
public:
    /** @brief Constructor
     *
     * @param[in] lane The lane to cross
     * @param[in] pos The position at the lane
     */
    MSCrossSection(MSLane* lane, double pos) : myLane(lane), myPosition(pos) {}


public:
    /// @brief The lane to cross
    MSLane* myLane;

    /// @brief The position at the lane
    double myPosition;

};


typedef std::vector< MSCrossSection > CrossSectionVector;
typedef CrossSectionVector::iterator CrossSectionVectorIt;
typedef CrossSectionVector::const_iterator CrossSectionVectorConstIt;
