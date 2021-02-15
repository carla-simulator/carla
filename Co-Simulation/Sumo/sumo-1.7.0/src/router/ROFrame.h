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
/// @file    ROFrame.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Sept 2002
///
// Sets and checks options for routing
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ROFrame
 * @brief Sets and checks options for routing
 *
 * Normally, these methods are called from another frame (ROJTRFrame, RODUAFrame)...
 */
class ROFrame {
public:
    /** @brief Inserts options used by routing applications into the OptionsCont-singleton
     * @param[in] oc The options container to fill
     */
    static void fillOptions(OptionsCont& oc);


    /** @brief Checks whether options are valid
     *
     * To be valid,
     * @arg an output file must be given
     * @arg max-alternatives must not be lower than 2
     *
     * @param[in] oc The options container to fill
     * @return Whether all needed options are set
     * @todo probably, more things should be checked...
     */
    static bool checkOptions(OptionsCont& oc);

};
