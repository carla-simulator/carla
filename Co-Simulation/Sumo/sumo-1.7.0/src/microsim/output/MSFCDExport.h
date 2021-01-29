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
/// @file    MSFCDExport.h
/// @author  Mario Krumnow
/// @date    2012-04-26
///
// Realises dumping Floating Car Data (FCD) Data
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
 * @class MSFCDExport
 * @brief Realises dumping FCD Data
 *
 *  The class offers a static method, which writes Position of each vehicle of
 *  the network into the given OutputDevice.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSFCDExport {
public:
    /** @brief Writes the position and the angle of each vehicle into the given device
     *
     *  Opens the current time step and export the values vehicle id, position and angle
     *
     * @param[in] of The output device to use
     * @param[in] timestep The current time step
     * @param[in] elevation Whether elevation data shall be written
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void write(OutputDevice& of, SUMOTime timestep, bool elevation);

private:
    /// @brief write transportable
    static void writeTransportable(OutputDevice& of, const MSEdge* e, MSTransportable* p, bool inRadius, SumoXMLTag tag, bool useGeo, bool elevation);

private:
    /// @brief Invalidated copy constructor.
    MSFCDExport(const MSFCDExport&);

    /// @brief Invalidated assignment operator.
    MSFCDExport& operator=(const MSFCDExport&);


};
