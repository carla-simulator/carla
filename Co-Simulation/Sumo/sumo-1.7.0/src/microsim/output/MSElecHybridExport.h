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
/// @file    MSElecHybridExport.h
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-11-25
///
// Realises dumping Electric hybrid vehicle data
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSElecHybridExport
 * @brief Realises dumping Battery Data
 *
 *  The class offers a static method, which writes all available Battery factors
 *  of each vehicles of the network into the given OutputDevice.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSElecHybridExport {
public:
    /** @brief Writes the complete network state of the given edges into the given device
     *
     *  Opens the current time step and export the Battery factors of all availabel vehicles
     *
     * @param[in] of The output device to use
     * @param[in] timestep The current time step
     * @param[in] precision The output precision
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void write(OutputDevice& of, const SUMOVehicle* veh, SUMOTime timestep, int precision);

    static void writeAggregated(OutputDevice& of, SUMOTime timestep, int precision);

private:
    /// @brief Invalidated copy constructor.
    MSElecHybridExport(const MSElecHybridExport&);

    /// @brief Invalidated assignment operator.
    MSElecHybridExport& operator=(const MSElecHybridExport&);

};



