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
/// @file    GUIEvent_SimulationEnded.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Thu, 19 Jun 2003
///
// Event sent when the the simulation is over
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/gui/events/GUIEvent.h>
#include <utils/common/SUMOTime.h>
#include <microsim/MSNet.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIEvent_SimulationEnded
 * @brief Event sent when the the simulation is over
 *
 * Throw from GUIRunThread to GUIApplicationWindow.
 */
class GUIEvent_SimulationEnded : public GUIEvent {
public:
    /** @brief Constructor
     * @param[in] reason The reason the simulation has ended
     * @param[in] step The time step the simulation has ended at
     */
    GUIEvent_SimulationEnded(MSNet::SimulationState reason, SUMOTime step)
        : GUIEvent(EVENT_SIMULATION_ENDED), myReason(reason), myStep(step) {}


    /// @brief Destructor
    ~GUIEvent_SimulationEnded() { }


    /** @brief Returns the time step the simulation has ended at
     * @return The time step the simulation has ended at
     */
    SUMOTime getTimeStep() const {
        return myStep;
    }


    /** @brief Returns the reason the simulation has ended due
     * @return The reason the simulation has ended
     */
    MSNet::SimulationState getReason() const {
        return myReason;
    }


protected:
    /// @brief The reason the simulation has ended
    MSNet::SimulationState myReason;

    /// @brief The time step the simulation has ended at
    SUMOTime myStep;


};
