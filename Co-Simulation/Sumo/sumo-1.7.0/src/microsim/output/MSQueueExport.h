/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSQueueExport.h
/// @author  Mario Krumnow
/// @date    2012-04-26
///
// Export the queueing length in front of a junction (very experimental!)
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class MSEdgeControl;
class MSEdge;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSQueueExport
 * @brief Export the queueing length in front of a junction (very experimental!)
 *
 *  The class offers a static method, which writes the actual queueing length of
 *  the network into the given OutputDevice.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSQueueExport {
public:
    /** @brief Export the queueing length in front of a junction (very experimental!)
     *
     * Opens the current time step, goes through the lanes check for a traffic jam in front of a junction
     *
     * @param[in] of The output device to use
     * @param[in] timestep The current time step
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void write(OutputDevice& of, SUMOTime timestep);


private:
    /// @brief Invalidated copy constructor.
    MSQueueExport(const MSQueueExport&);

    /// @brief Invalidated assignment operator.
    MSQueueExport& operator=(const MSQueueExport&);

    /// @brief Iterates through all the edges and extract the lanes
    static void writeEdge(OutputDevice& of);

    /// @brief Iterates through the lanes and check for available vehicle queues
    static void writeLane(OutputDevice& of, const MSLane& lane);

};
