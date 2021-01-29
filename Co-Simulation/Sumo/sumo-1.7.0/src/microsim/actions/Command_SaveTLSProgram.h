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
/// @file    Command_SaveTLSProgram.h
/// @author  Jakob Erdmann
/// @date    18.09.2019
///
// Writes the switch times of a tls into a file when the tls switches
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <map>
#include <utils/common/Command.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSPhaseDefinition.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSTrafficLightLogic;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Command_SaveTLSProgram
 * @brief Writes the switch times of a tls into a file when the tls switches
 *
 * @todo Revalidate this - as tls are not seting the link information directly ater being switched, the computed information may be delayed
 */
class Command_SaveTLSProgram : public Command {
public:
    /** @brief Constructor
     *
     * @param[in] tlls The logic to write state of
     * @param[in] od The output device to write the state into
     */
    Command_SaveTLSProgram(const MSTLLogicControl::TLSLogicVariants& logics,
                           OutputDevice& od);


    /// @brief Destructor
    ~Command_SaveTLSProgram();


    /// @name Derived from Command
    /// @{

    /** @brief Writes the state of the tls if a change occurred
     *
     * If the state or the active program has changed, the state is written
     *  to the output device.
     *
     * @param[in] currentTime The current simulation time
     * @return Always DELTA_T (will be executed in next time step)
     * @see Command
     * @todo Here, a discrete even (on switch / program change) would be appropriate
     */
    SUMOTime execute(SUMOTime currentTime);
    /// @}

private:
    /// @brief Write the current program
    void writeCurrent();

private:
    /// @brief The device to write to
    OutputDevice& myOutputDevice;

    /// @brief The traffic light logic to use
    const MSTLLogicControl::TLSLogicVariants& myLogics;

    /// @brief Storage for prior state
    std::vector<MSPhaseDefinition> myPreviousStates;

    /// @brief Storage for prior sub-id
    std::string myPreviousProgramID;

    /// @brief Storage for the tls ID (needed in destructor)
    std::string myTLSID;

private:
    /// @brief Invalidated copy constructor.
    Command_SaveTLSProgram(const Command_SaveTLSProgram&);

    /// @brief Invalidated assignment operator.
    Command_SaveTLSProgram& operator=(const Command_SaveTLSProgram&);

};
