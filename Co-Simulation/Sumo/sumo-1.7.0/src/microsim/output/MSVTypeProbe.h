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
/// @file    MSVTypeProbe.h
/// @author  Tino Morenz
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 24.10.2007
///
// Writes positions of vehicles that have a certain (named) type
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <utils/common/Command.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/Named.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVTypeProbe
 * @brief Writes positions of vehicles that have a certain (named) type
 *
 * This device allows to log the data of all running vehicles of the
 *  specified vehicle type, i.e. vehicle id, edge, lane=, position
 *  on lane, x/y coordinates and speed.
 *
 * A frequency can be specified to generate the output in certain intervals,
 *  (e.g. every 10 seconds).
 *
 * @see Command
 * @see Named
 */

class MSVTypeProbe : public Named, public Command {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the vehicle type probe
     * @param[in] vType The vtype of which vehicles to report must be ("" for all vehicles)
     * @param[in] od The output device to write into
     * @param[in] frequency The output frequency [ms]
     */
    MSVTypeProbe(const std::string& id, const std::string& vType,
                 OutputDevice& od, SUMOTime frequency);


    /// @brief Destructor
    virtual ~MSVTypeProbe();



    /// @name Derived from Command
    /// @{

    /** @brief Writes values into the given stream
     *
     * This method goes through all runing vehicles; if a vehicle
     *  has a type with the same id as the wished one, it is reported.
     * When the type "" is wished, all vehicles are reported
     *
     * @param[in] currentTime The current simulation time (unused)
     * @return Always myFrequency (time till next output)
     * @see Command
     */
    SUMOTime execute(SUMOTime currentTime);
    /// @}


private:
    /// @brief The id of the vehicle type vehicles must have to be reported
    std::string myVType;

    /// @brief The device to write into
    OutputDevice& myOutputDevice;

    /// @brief The frequency of reporting
    SUMOTime myFrequency;


private:
    /// @brief Invalidated copy constructor.
    MSVTypeProbe(const MSVTypeProbe&);

    /// @brief Invalidated assignment operator.
    MSVTypeProbe& operator=(const MSVTypeProbe&);


};
