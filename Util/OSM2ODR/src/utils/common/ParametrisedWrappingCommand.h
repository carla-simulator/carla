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
/// @file    ParametrisedWrappingCommand.h
/// @author  Leonhard Luecken
/// @date    Apr 2019
///
// A wrapper for a Command function with parameter
/****************************************************************************/

#pragma once

#include "Command.h"


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class ParametrisedWrappingCommand
 * @brief A wrapper for a Command function with parameter
 * @see WrappingCommand.h
 */
template< class T, class S  >
class ParametrisedWrappingCommand : public Command {
public:

    /// @brief Type of the function to execute. (with parameter)
    typedef SUMOTime(T::* Operation)(SUMOTime, S);

public:

    /**
     * @brief Constructor.
     *
     * @param[in] receiver Pointer to object of type T that will receive a call to one of it's methods.
     * @param[in] parameter The methods parameter (must be copy-constructable)
     * @param[in] operation The objects' method that will be called on execute()
     */
    ParametrisedWrappingCommand(T* receiver, const S& param, Operation operation)
        : myReceiver(receiver), myParameter(param), myOperation(operation),
          myAmDescheduledByParent(false) {}


    /// @brief Destructor
    ~ParametrisedWrappingCommand() {}


    /** @brief Marks this Command as being descheduled
     *
     * A simple boolean marker ("myAmDescheduledByParent") is set which
     *  prevents this command from being executed.
     */
    void deschedule() {
        myAmDescheduledByParent = true;
    }

    /// @brief whether this command has been descheduled
    bool isDescheduled() {
        return myAmDescheduledByParent;
    }


    /// @name Derived from Command
    /// @{

    /** @brief Executes the command.
     *
     * If the command is not descheduled, the stored method of the stored instance
     *  is called.
     *
     * @param[in] currentTime The current simulation time
     * @return The time after which the command shall be executed again, 0 if this command shall be descheduled.
     * @exception ProcessError Derived actions may throw this exception
     */
    SUMOTime execute(SUMOTime currentTime) {
        // do not execute if the command was descheduled
        if (myAmDescheduledByParent) {
            return 0;
        }
        // execute if stil valid
        return (myReceiver->*myOperation)(currentTime, myParameter);
    }
    /// @}


private:
    /// @brief The object the action is directed to.
    T* myReceiver;

    /// @brief The parameter
    S myParameter;

    /// @brief The object's operation to perform.
    Operation myOperation;

    /// @brief Whether this command was descheduled (is invalid) and shall not be executed
    bool myAmDescheduledByParent;

};

