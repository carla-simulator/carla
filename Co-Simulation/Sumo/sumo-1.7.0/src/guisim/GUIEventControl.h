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
/// @file    GUIEventControl.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 04 Feb 2008
///
// Stores time-dependant events and executes them at the proper time (guisim)
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <microsim/MSEventControl.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIEventControl
 * @brief Stores time-dependant events and executes them at the proper time (guisim)
 *
 * Encapsulates MSEventControl-methods using a lock, prohibiting parallel addition /
 *  processing of events what may yield in application break due to broken containers.
 */
class GUIEventControl : public MSEventControl {
public:
    /// @brief Default constructor.
    GUIEventControl();


    /// @brief Destructor.
    ~GUIEventControl();


    /** @brief Adds an Event.
     *
     * Locks itself before calling MSEventControl::addEvent. Unlock itself
     *  after the call.
     *
     * @param[in] operation The event to add
     * @param[in] execTimeStep The time the event shall be executed at (-1 means at sim start)
     * @see MSEventControl::addEvent
     */
    void addEvent(Command* operation, SUMOTime execTimeStep = -1);


    /** @brief Executes time-dependant commands
     *
     * Locks itself before calling MSEventControl::execute. Unlock itself
     *  after the call.
     *
     * @param[in] time The current simulation time
     * @exception ProcessError From an executed Command
     * @see MSEventControl::execute
     */
    void execute(SUMOTime time);


private:
    /// @brief The lock used to prohibit parallel addition and processing of events
    FXMutex myLock;


private:
    /// @brief invalid copy constructor.
    GUIEventControl(const GUIEventControl&);

    /// @brief invalid assignment operator.
    GUIEventControl& operator=(const GUIEventControl&);


};
