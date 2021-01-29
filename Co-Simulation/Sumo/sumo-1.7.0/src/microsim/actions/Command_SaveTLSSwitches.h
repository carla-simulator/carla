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
/// @file    Command_SaveTLSSwitches.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    06 Jul 2006
///
// Writes information about the green durations of a tls
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
 * @class Command_SaveTLSSwitches
 * @brief Writes information about the green durations of a tls
 *
 * @todo Revalidate this - as tls are not seting the link information directly ater being switched, the computed information may be delayed
 */
class Command_SaveTLSSwitches : public Command {
public:
    /** @brief Constructor
     *
     * @param[in] tlls The logic to write state of
     * @param[in] od The output device to write the state into
     */
    Command_SaveTLSSwitches(const MSTLLogicControl::TLSLogicVariants& logics,
                            OutputDevice& od);


    /// @brief Destructor
    ~Command_SaveTLSSwitches();


    /// @name Derived from Command
    /// @{

    /** @brief Writes the output if a change occurred
     *
     * Called in each tme step, this class computes which link have red
     *  since the last tls switch and writes the information about their
     *  green duration into the given stream.
     *
     * Information whether a link had green and since when is stored in
     *  "myPreviousLinkStates".
     *
     * @param[in] currentTime The current simulation time
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

    /// @brief Storage for prior states; map from signal group to last green time begin
    std::map<int, SUMOTime> myPreviousLinkStates;


private:
    /// @brief Invalidated copy constructor.
    Command_SaveTLSSwitches(const Command_SaveTLSSwitches&);

    /// @brief Invalidated assignment operator.
    Command_SaveTLSSwitches& operator=(const Command_SaveTLSSwitches&);

};
