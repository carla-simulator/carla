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
/// @file    NGFrame.h
/// @author  Daniel Krajzewicz
/// @date    06.05.2011
///
// Sets and checks options for netgen
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NGFrame
 * @brief Sets and checks options for netgen
 */
class NGFrame {
public:
    /** @brief Inserts options used by the network generator
     */
    static void fillOptions();


    /** @brief Checks set options from the OptionsCont-singleton for being valid
     * @return Whether needed (netgen) options are set and have proper values
     */
    static bool checkOptions();


};
