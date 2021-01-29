/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSIdling.h
/// @author  Jakob Erdmann
/// @date    17.08.2020
///
// An algorithm that performs Idling for the taxi device
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <vector>
#include <map>
#include <utils/common/Parameterised.h>
#include <utils/common/SUMOTime.h>
#include "MSDevice_Taxi.h"

// ===========================================================================
// class declarations
// ===========================================================================

// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class MSIdling
 * @brief An algorithm that performs distpach for a taxi fleet
 */
class MSIdling {
public:

    /// @brief Constructor;
    MSIdling() {}

    /// @brief Destructor
    virtual ~MSIdling() { }

    /// @brief computes Idling and updates reservations
    virtual void idle(MSDevice_Taxi* taxi) = 0;
};


class MSIdling_Stop : public MSIdling {
    void idle(MSDevice_Taxi* taxi);
};

class MSIdling_RandomCircling : public MSIdling {
    void idle(MSDevice_Taxi* taxi);
};
