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
/// @file    MSVTKExport.h
/// @author  Mario Krumnow
/// @date    2012-04-26
///
// Produce a VTK output to use with Tools like ParaView
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
 * @class MSVTKExport
 * @brief Produce a VTK output to use with Tools like ParaView
 *
 *  The class offers a static method, which writes VTK Files for each timestep
 *  of the simulation, where at least one vehicle is present.
 *
 * @todo
 */
class MSVTKExport {
public:
    /** @brief Produce a VTK output to use with Tools like ParaView
     *
     * @param[in] of The output device to use
     * @param[in] ec The EdgeControl which holds the edges to write
     * @param[in] timestep The current time step
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    static void write(OutputDevice& of, SUMOTime timestep);

private:
    /// @brief Invalidated copy constructor.
    MSVTKExport(const MSVTKExport&);

    /// @brief Invalidated assignment operator.
    MSVTKExport& operator=(const MSVTKExport&);

    /// @brief Deletes the whitespaces at the end of a String
    static std::string trim(std::string istring);

    /// @brief Checks if there is a whitespace
    static bool ctype_space(const char c);

    /// @brief Get a comma separated String from a Vector
    static std::string List2String(std::vector<double> input);

    /// @brief Get a Vector with the speed values of each vehicle in the actual timestep
    static std::vector<double> getSpeed();

    /// @brief Get a Vector of the Positions (x,y,z) of each vehicle in the actual timestep
    static std::vector<double> getPositions();

    /// @brief Get a String with the indexes of all vehicles (needed in the VTk File)
    static std::string getOffset(int nr);

};
