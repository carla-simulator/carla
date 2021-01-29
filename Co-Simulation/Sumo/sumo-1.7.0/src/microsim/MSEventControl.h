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
/// @file    MSEventControl.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Matthias Heppner
/// @date    Mon, 12 Mar 2001
///
// Stores time-dependant events and executes them at the proper time
/****************************************************************************/
#pragma once
#include <config.h>

#include <utility>
#include <queue>
#include <vector>
#include <map>
#include <utils/common/SUMOTime.h>
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class Command;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSEventControl
 * @brief Stores time-dependant events and executes them at the proper time
 */
class MSEventControl {
public:
    /// @brief Combination of an event and the time it shall be executed at
    typedef std::pair< Command*, SUMOTime > Event;


public:
    /// @brief Default constructor.
    MSEventControl();


    /// @brief Destructor.
    virtual ~MSEventControl();


    /** @brief Adds an Event.
     *
     * @param[in] operation The event to add
     * @param[in] execTimeStep The time the event shall be executed at (-1 means at sim start)
     * @see Command
     */
    virtual void addEvent(Command* operation, SUMOTime execTimeStep = -1);


    /** @brief Executes time-dependant commands
     *
     * Loops over all stored events, continuing until the first event which
     *  execution time lies beyond the given time + deltaT. If the event
     *  had to be executed before the given time, a warning is generated and
     *  the event deleted. Otherwise (the event is valid), the event is executed.
     *
     * Each executed event must return the time that has to pass until it shall
     *  be executed again. If the returned time is 0, the event is deleted.
     *  Otherwise it is readded, after the new execution time (returned + current)
     *  is computed.
     *
     * ProcessErrors thrown by executed commands are rethrown.
     *
     * @param[in] time The current simulation time
     * @exception ProcessError From an executed Command
     */
    virtual void execute(SUMOTime time);


    /** @brief Returns whether events are in the que.
     *
     * @return  whether events are in the que
     */
    bool isEmpty();

    /** @brief Remove all events before quick-loading state */
    void clearState(SUMOTime currentTime, SUMOTime newTime);

protected:
    /** @brief Sort-criterion for events.
     *
     * Sorts events by their execution time
     */
    class EventSortCrit {
    public:
        /// @brief compares two events
        bool operator()(const Event& e1, const Event& e2) const {
            return e1.second > e2.second;
        }
    };


private:
    /// @brief Container for time-dependant events, e.g. traffic-light-change.
    typedef std::priority_queue< Event, std::vector< Event >, EventSortCrit > EventCont;

    /// @brief Event-container, holds executable events.
    EventCont myEvents;


private:
    /// @brief invalid copy constructor.
    MSEventControl(const MSEventControl&);

    /// @brief invalid assignment operator.
    MSEventControl& operator=(const MSEventControl&);


};
