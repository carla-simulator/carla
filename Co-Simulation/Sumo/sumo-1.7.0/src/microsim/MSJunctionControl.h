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
/// @file    MSJunctionControl.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 12 Mar 2001
///
// Container for junctions; performs operations on all stored junctions
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <string>

#include <utils/common/NamedObjectCont.h>
#include <utils/common/UtilExceptions.h>
#include "MSJunction.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSJunctionControl
 * @brief Container for junctions; performs operations on all stored junctions
 */
class MSJunctionControl : public NamedObjectCont<MSJunction*> {
public:
    /// @brief Constructor
    MSJunctionControl();


    /// @brief Destructor
    ~MSJunctionControl();


    /** @brief Closes building of junctions
     *
     * Calls "postloadInit" on all stored junctions.
     * @exception ProcessError From the called "postloadInit"
     * @see MSJunction::postloadInit
     */
    void postloadInitContainer();


private:
    /// @brief Invalidated copy constructor.
    MSJunctionControl(const MSJunctionControl&);

    /// @brief Invalidated assignment operator.
    MSJunctionControl& operator=(const MSJunctionControl&);

};
