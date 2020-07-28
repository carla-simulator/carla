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
/// @file    LineHandler.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Fri, 19 Jul 2002
///
// Interface definition for a class which retrieves lines from a LineHandler
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class LineHandler
 * @brief Interface definition for a class which retrieves lines from a LineHandler
 *
 * The LineHandler is an interface for a class which retrieves information
 *  from a file linewise. The lines are suppoted to this class using the
 *  "report"-method. The LineHandler is used together with the LineReader
 *  which reads the lines from a file.
 * @see LineReader
 */
class LineHandler {
public:
    /// @brief constructor
    LineHandler() { }


    /// @brief (virtual) destructor
    virtual ~LineHandler() { }


    /** @brief Method that obatins a line read by the LineReader
     *
     * Real interface method, used by a LineReader, which retrieves lines from a file
     *
     * @param[in] result The read line
     * @return Whether the caller shall continue with reading
     */
    virtual bool report(const std::string& result) = 0;

};
