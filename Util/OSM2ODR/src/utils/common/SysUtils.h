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
/// @file    SysUtils.h
/// @author  Daniel Krajzewicz
/// @date    Fri, 29.04.2005
///
// A few system-specific functions
/****************************************************************************/
#pragma once
#include <string>

// ===========================================================================
// class definitions
// ===========================================================================
/** @class SysUtils
 * @brief A few system-specific functions
 */
class SysUtils {
public:
    /** @brief Returns the current time in milliseconds
     * @return Current time
     */
    static long getCurrentMillis();


#ifdef WIN32
    /** @brief Returns the CPU ticks (windows only)
     *
     * Used for random number initialisation, linux version
     *  uses a different method
     */
    static long getWindowsTicks();
#endif


    /// @brief run a shell command without popping up any windows (particuarly on win32)
    static unsigned long runHiddenCommand(const std::string& cmd);
};
