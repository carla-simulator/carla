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
/// @file    MSXMLRawOut.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Bjoern Hendriks
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
///
// Realises dumping the complete network state
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
class MSBaseVehicle;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSXMLRawOut
 * @brief Realises dumping the complete network state
 *
 * The class offers a static method, which writes the complete dump of
 *  the given network into the given OutputDevice.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSXMLRawOut {
public:
    /** @brief Writes the complete network state of the given edges into the given device
     *
     * Opens the current time step, goes through the edges and writes each using
     *  writeEdge.
     *
     * @param[in] of The output device to use
     * @param[in] ec The EdgeControl which holds the edges to write
     * @param[in] timestep The current time step
     * @param[in] precision The output precision
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void write(OutputDevice& of, const MSEdgeControl& ec,
                      SUMOTime timestep, int precision);


    /** @brief Writes the dump of the given vehicle into the given device
     *
     * @param[in] of The output device to use
     * @param[in] veh The vehicle to dump
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void writeVehicle(OutputDevice& of, const MSBaseVehicle& veh);


private:
    /** @brief Writes the dump of the given edge into the given device
     *
     * If the edge is not empty or also empty edges shall be dumped, the edge
     *  description is opened and writeLane is called for each lane.
     *
     * @param[in] of The output device to use
     * @param[in] edge The edge to dump
     * @todo MSGlobals::gOmitEmptyEdgesOnDump should not be used; rather the according option read in write
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void writeEdge(OutputDevice& of, const MSEdge& edge, SUMOTime timestep);


    /** @brief Writes the dump of the given lane into the given device
     *
     * Opens the lane description and goes through all vehicles, calling writeVehicle
     *  for each.
     *
     * @param[in] of The output device to use
     * @param[in] lane The lane to dump
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void writeLane(OutputDevice& of, const MSLane& lane);

    /// @brief write transportable
    static void writeTransportable(OutputDevice& of, const MSTransportable* p, SumoXMLTag tag);

private:
    /// @brief Invalidated copy constructor.
    MSXMLRawOut(const MSXMLRawOut&);

    /// @brief Invalidated assignment operator.
    MSXMLRawOut& operator=(const MSXMLRawOut&);


};
