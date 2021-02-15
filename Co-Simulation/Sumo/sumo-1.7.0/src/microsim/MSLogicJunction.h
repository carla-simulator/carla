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
/// @file    MSLogicJunction.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Wed, 12 Dez 2001
///
// with one ore more logics.
/****************************************************************************/
#pragma once
#include <config.h>

#include "MSJunction.h"
#include <utils/common/SUMOTime.h>
#include <utils/common/StdDefs.h>
#include <bitset>
#include <vector>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLogicJunction
 * A junction which may not let all vehicles through, but must perform any
 * kind of an operation to determine which cars are allowed to drive in this
 * step.
 */
class MSLogicJunction : public MSJunction {
public:


    /// Destructor.
    virtual ~MSLogicJunction();

    /** @brief Container for link response and foes */
    typedef std::bitset<SUMO_MAX_CONNECTIONS> LinkBits;

    /// initialises the junction after the whole net has been loaded
    virtual void postloadInit();

    /** @brief Returns all internal lanes on the junction
     */
    const std::vector<MSLane*> getInternalLanes() const;

protected:
    /** @brief Constructor
     * @param[in] id The id of the junction
     * @param[in] id The type of the junction
     * @param[in] position The position of the junction
     * @param[in] shape The shape of the junction
     * @param[in] incoming The incoming lanes
     * @param[in] internal The internal lanes
     */
    MSLogicJunction(const std::string& id,
                    SumoXMLNodeType type,
                    const Position& position,
                    const PositionVector& shape,
                    const std::string& name,
                    std::vector<MSLane*> incoming,
                    std::vector<MSLane*> internal
                   );

protected:
    /// list of incoming lanes
    std::vector<MSLane*> myIncomingLanes;

    /// list of internal lanes
    std::vector<MSLane*> myInternalLanes;

private:
    /// @brief Invalidated copy constructor.
    MSLogicJunction(const MSLogicJunction&);

    /// @brief Invalidated assignment operator.
    MSLogicJunction& operator=(const MSLogicJunction&);

};
