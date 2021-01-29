/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSRailSignalConstraint.h
/// @author  Jakob Erdmann
/// @date    August 2020
///
// A constraint on rail signal switching
/****************************************************************************/
#pragma once
#include <config.h>

#include <microsim/MSMoveReminder.h>

// ===========================================================================
// class declarations
// ===========================================================================
class MSRailSignal;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRailSignalConstraint
 * @brief A base class for constraints
 */
class MSRailSignalConstraint {
public:
    /** @brief Constructor
     */
    MSRailSignalConstraint() {};

    /// @brief Destructor
    virtual ~MSRailSignalConstraint() {};

    /// @brief whether the constraint has been met
    virtual bool cleared() const = 0;

    /// @brief clean up state
    static void cleanup();
};


class MSRailSignalConstraint_Predecessor : public MSRailSignalConstraint {
public:
    /** @brief Constructor
     */
    MSRailSignalConstraint_Predecessor(const MSRailSignal* signal, const std::string& tripId, int limit);

    /// @brief Destructor
    ~MSRailSignalConstraint_Predecessor() {};

    /// @brief clean up state
    static void cleanup();

    bool cleared() const;

    class PassedTracker : public MSMoveReminder {
    public:
        PassedTracker(const MSLink* link);

        /// @name inherited from MSMoveReminder
        //@{
        /// @brief tracks vehicles that passed this link (entered the next lane)
        bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
        //@}

        void raiseLimit(int limit);

        bool hasPassed(const std::string& tripId, int limit) const;

    protected:
        /// @brief passed tripIds
        std::vector<std::string> myPassed;

        /// @brief index of the last passed object
        int myLastIndex;
    };

    /// @brief the tracker object for this constraint
    std::vector<PassedTracker*> myTrackers;

    /// @brief id of the predecessor that must already have passed
    const std::string myTripId;

    /// @brief the number of passed vehicles within which tripId must have occured
    const int myLimit;


    static std::map<const MSLink*, PassedTracker*> myTrackerLookup;

private:
    /// invalidated assignment operator
    MSRailSignalConstraint_Predecessor& operator=(const MSRailSignalConstraint_Predecessor& s) = delete;
};


