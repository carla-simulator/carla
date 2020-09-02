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
/// @file    NWFrame.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 20 Nov 2001
///
// Sets and checks options for netwrite
/****************************************************************************/
#pragma once
#include <config.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OptionsCont;
class NBNetBuilder;
class Position;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NWFrame
 * @brief Sets and checks options for netwrite; saves the network
 */
class NWFrame {
public:
    /// @brief Inserts options used by the network writer
    static void fillOptions(bool forNetgen);

    /** @brief Checks set options from the OptionsCont-singleton for being valid
     * @return Whether needed options are set and have proper values
     */
    static bool checkOptions();

    /// @brief Writes the network stored in the given net builder
    static void writeNetwork(const OptionsCont& oc, NBNetBuilder& nb);

    /// @brief Writes the given position to device in long format (one attribute per dimension)
    static void writePositionLong(const Position& pos, OutputDevice& dev);

};
