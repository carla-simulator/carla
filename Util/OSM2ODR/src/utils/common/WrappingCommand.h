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
/// @file    WrappingCommand.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @date    Thu, 20 Dec 2001
///
// A wrapper for a Command function
/****************************************************************************/
#pragma once
#include "Command.h"


// ===========================================================================
// class definition
// ===========================================================================
/**
 * @class WrappingCommand
 * @brief A wrapper for a Command function
 *
 * In order to ease life, this class may encapsulate a method of a class which
 *  in order to be used as a Command. This allows to use a member methods
 *  of a class to be called as Commands are, avoiding that the instance itself
 *  is destroyed by the EventHandler.
 *
 * Because in some cases, the Command may live longer than the instance class,
 *  a boolean value indicates that the Command is "descheduled". It should
 *  be set via "deschedule" as soon as the class instance of which a method
 *  is encapsulated is destroyed and forces that the command (calling of this
 *  instace's method) is not executed.
 *
 * @see Design Patterns, Gamma et al.
 * @see Command
 * @see MSEventControl
 */
template< class T  >
class WrappingCommand : public Command {
public:
    /// @brief Type of the function to execute.
    typedef SUMOTime(T::* Operation)(SUMOTime);


public:
    /**
     * @brief Constructor.
     *
     * @param[in] receiver Pointer to object of type T that will receive a call to one of it's methods.
     * @param[in] operation The objects' method that will be called on execute()
     */
    WrappingCommand(T* receiver, Operation operation)
        : myReceiver(receiver), myOperation(operation),
          myAmDescheduledByParent(false) {}


    /// @brief Destructor
    ~WrappingCommand() {}


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
        return (myReceiver->*myOperation)(currentTime);
    }
    /// @}


private:
    /// @brief The object the action is directed to.
    T* myReceiver;

    /// @brief The object's operation to perform.
    Operation myOperation;

    /// @brief Whether this command was descheduled (is invalid) and shall not be executed
    bool myAmDescheduledByParent;


};
