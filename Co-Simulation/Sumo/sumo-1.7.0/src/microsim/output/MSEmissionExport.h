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
/// @file    MSEmissionExport.h
/// @author  Mario Krumnow
/// @date    2012-04-26
///
// Realises dumping Emission Data
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
 * @class MSEmissionExport
 * @brief Realises dumping Emission Data
 *
 *  The class offers a static method, which writes all available emission factors
 *  of each vehicles of the network into the given OutputDevice.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSEmissionExport {
public:
    /** @brief Writes the complete network state of the given edges into the given device
     *
     *  Opens the current time step and export the emission factors of all availabel vehicles
     *
     * @param[in] of The output device to use
     * @param[in] timestep The current time step
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void write(OutputDevice& of, SUMOTime timestep, int precision);


private:
    /// @brief Invalidated copy constructor.
    MSEmissionExport(const MSEmissionExport&);

    /// @brief Invalidated assignment operator.
    MSEmissionExport& operator=(const MSEmissionExport&);

};
