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
/// @file    Command_SaveTLSState.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    15 Feb 2004
///
// Writes the state of the tls to a file (in each second)
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/common/Command.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSTrafficLightLogic;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Command_SaveTLSState
 * @brief Writes the state of the tls to a file (in each second)
 */
class Command_SaveTLSState : public Command {
public:
    /** @brief Constructor
     *
     * @param[in] tlls The logic to write state of
     * @param[in] od The output device to write the state into
     */
    Command_SaveTLSState(const MSTLLogicControl::TLSLogicVariants& logics,
                         OutputDevice& od);


    /// @brief Destructor
    ~Command_SaveTLSState();


    /// @name Derived from Command
    /// @{

    /** @brief Writes the current state of the tls
     *
     * Returns always 1.
     *
     * @param[in] currentTime The current simulation time (unused)
     * @return Always DELTA_T (will be executed in next time step)
     * @see Command
     */
    SUMOTime execute(SUMOTime currentTime);
    /// @}


private:
    /// @brief The device to write to
    OutputDevice& myOutputDevice;

    /// @brief The traffic light logic to use
    const MSTLLogicControl::TLSLogicVariants& myLogics;


private:
    /// @brief Invalidated copy constructor.
    Command_SaveTLSState(const Command_SaveTLSState&);

    /// @brief Invalidated assignment operator.
    Command_SaveTLSState& operator=(const Command_SaveTLSState&);

};
