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
/// @file    Command.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @date    Thu, 20 Dec 2001
///
// Base (microsim) event class
/****************************************************************************/
#pragma once
#include <utils/common/SUMOTime.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class Command
 * @brief Base (microsim) event class
 *
 * Classes derived from Command may be added to MSEventControl instances in
 *  order to be executed at a certain time step.
 *
 * As soon as the simulation reaches the desired time step, the command (event)
 *  is executed by calling "execute" with the current time step. The method must
 *  return either 0, if the event shall not be executed again or a positive value
 *  (in simulation seconds) that described when it shall be executed again. The method
 *  must not return a value below zero, the behaviour is undefined in this case.
 *
 * @warning The EventControl the Command is added to gets responsible for
 *  this command's deletion.
 *
 * @see Design Patterns, Gamma et al.
 * @see WrappingCommand
 * @see MSEventControl
 */
class Command {
public:
    /// @brief Constructor
    Command() { }


    /// @brief Destructor.
    virtual ~Command() { }


    /** @brief Executes the command.
     *
     * The implementations should return 0 if the command shall not be repeated,
     *  or a value larger than 0 that describe the time after which the command
     *  shall be executed again. Values below 0 must not be returned.
     *
     * @param[in] currentTime The current simulation time
     * @return The time after which the command shall be executed again, 0 if this command shall be descheduled.
     * @exception ProcessError Derived actions may throw this exception
     */
    virtual SUMOTime execute(SUMOTime currentTime) = 0;

    /** @brief Reschedule or deschedule the command when quick-loading state
     *
     * The implementations should return -1 if the command shall not be re-scheduled,
     *  or a value >= 0 that describe the new time at which the command
     *  shall be executed again.
     *
     * @param[in] currentTime The current simulation time
         * @param[in] execTime The time at which the command would have been executed
     * @param[in] newTime The simulation time at which the simulation is restarted
     * @return The time at which the command shall be executed again
     */
    virtual SUMOTime shiftTime(SUMOTime /*currentTime*/, SUMOTime /*execTime*/, SUMOTime /*newTime*/) {
        return -1;
    }

};
