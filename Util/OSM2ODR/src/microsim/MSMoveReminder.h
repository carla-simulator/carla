/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2003-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSMoveReminder.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2003-05-21
///
// Something on a lane to be noticed about vehicle movement
/****************************************************************************/
#pragma once
#include <config.h>

#include <iostream>
#include <map>
#include <utils/common/SUMOTime.h>
#include <utils/common/StdDefs.h>
#ifdef HAVE_FOX
#include <fx.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOTrafficObject;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSMoveReminder
 * @brief Something on a lane to be noticed about vehicle movement
 *
 * Base class of all move-reminders. During move, the vehicles call
 *  notifyMove() for all reminders on their current lane (all lanes
 *  they pass during one step). If a vehicle enters the lane the reminder is
 *  positioned at during insertion or lanechange notifyEnter() is
 *  called. If a vehicle leaves the reminder lane it calls notifyLeave().
 *
 * The reminder knows whom to tell about move, insertion and lanechange. The
 * vehicles will remove the reminder that is not notifyMove() from
 * their reminder container.
 *
 * @see MSLane::addMoveReminder
 * @see MSLane::getMoveReminder
 * @note: cannot inherit from Named because it would couse double inheritance
 */
class MSMoveReminder {
public:
    /** @brief Constructor.
     *
     * @param[in] lane Lane on which the reminder will work.
     * @param[in] doAdd whether to add the reminder to the lane
     */
    MSMoveReminder(const std::string& description, MSLane* const lane = 0, const bool doAdd = true);


    /** @brief Destructor
     */
    virtual ~MSMoveReminder() {}


    /** @brief Returns the lane the reminder works on.
     *
     * @return The lane the reminder is anchored on.
     */
    const MSLane* getLane() const {
        return myLane;
    }


    /// @brief Definition of a vehicle state
    enum Notification {
        /// @brief The vehicle has departed (was inserted into the network)
        NOTIFICATION_DEPARTED,
        /// @brief The vehicle arrived at a junction
        NOTIFICATION_JUNCTION,
        /// @brief The vehicle changes the segment (meso only)
        NOTIFICATION_SEGMENT,
        /// @brief The vehicle changes lanes (micro only)
        NOTIFICATION_LANE_CHANGE,
        /* All notifications below must result in the vehicle not being on the net
         * (onLeaveLane sets amOnNet=false if reason>=NOTIFICATION_TELEPORT) */
        /// @brief The vehicle is being teleported
        NOTIFICATION_TELEPORT,
        /// @brief The vehicle starts or ends parking
        NOTIFICATION_PARKING,
        /// @brief The vehicle needs another parking area
        NOTIFICATION_PARKING_REROUTE,
        /// @brief The vehicle arrived at its destination (is deleted)
        NOTIFICATION_ARRIVED, // arrived and everything after is treated as permanent deletion from the net
        /// @brief The vehicle was teleported out of the net
        NOTIFICATION_TELEPORT_ARRIVED,
        /// @brief The vehicle got removed by a calibrator
        NOTIFICATION_VAPORIZED_CALIBRATOR,
        /// @brief The vehicle got removed by a collision
        NOTIFICATION_VAPORIZED_COLLISION,
        /// @brief The vehicle got removed via TraCI
        NOTIFICATION_VAPORIZED_TRACI,
        /// @brief The vehicle got removed via the GUI
        NOTIFICATION_VAPORIZED_GUI,
        /// @brief The vehicle got vaporized with a vaporizer
        NOTIFICATION_VAPORIZED_VAPORIZER
    };


    /// @name Interface methods, to be derived by subclasses
    /// @{

    /** @brief Checks whether the reminder is activated by a vehicle entering the lane
     *
     * Lane change means in this case that the vehicle changes to the lane
     *  the reminder is placed at.
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return True if vehicle enters the reminder.
     * @see Notification
     */
    virtual bool notifyEnter(SUMOTrafficObject& veh, Notification reason, const MSLane* enteredLane) {
        UNUSED_PARAMETER(reason);
        UNUSED_PARAMETER(&veh);
        UNUSED_PARAMETER(&enteredLane);
        return true;
    }


    /** @brief Checks whether the reminder still has to be notified about the vehicle moves
     *
     * Indicator if the reminders is still active for the passed
     * vehicle/parameters. If false, the vehicle will erase this reminder
     * from it's reminder-container.
     *
     * @param[in] veh Vehicle that asks this reminder.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     *
     * @return True if vehicle hasn't passed the reminder completely.
     */
    virtual bool notifyMove(SUMOTrafficObject& veh,
                            double oldPos,
                            double newPos,
                            double newSpeed) {
        UNUSED_PARAMETER(oldPos);
        UNUSED_PARAMETER(newPos);
        UNUSED_PARAMETER(newSpeed);
        UNUSED_PARAMETER(&veh);
        return true;
    }

    /** @brief Computes idling emission values and adds them to the emission sums
    *
    * Idling implied by zero velocity, acceleration and slope
    *
    * @param[in] veh The vehicle
    *
    * @see MSMoveReminder::notifyMove
    * @see PollutantsInterface
    */
    virtual bool notifyIdle(SUMOTrafficObject& veh) {
        UNUSED_PARAMETER(&veh);
        return true;
    }

    /** @brief Called if the vehicle leaves the reminder's lane
     *
     * Informs if vehicle leaves reminder lane (due to lane change, removal
     *  from the network, or leaving to the next lane).
     *  The default is to do nothing.
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] reason how the vehicle leaves the lane
     * @see Notification
     *
     * @return True if the reminder wants to receive further info.
     */
    virtual bool notifyLeave(SUMOTrafficObject& veh, double lastPos, Notification reason, const MSLane* enteredLane = 0) {
        UNUSED_PARAMETER(&veh);
        UNUSED_PARAMETER(lastPos);
        UNUSED_PARAMETER(reason);
        UNUSED_PARAMETER(enteredLane);
        return true;
    }


    // TODO: Documentation
    void updateDetector(SUMOTrafficObject& veh, double entryPos, double leavePos,
                        SUMOTime entryTime, SUMOTime currentTime, SUMOTime leaveTime,
                        bool cleanUp);

    /// @}

    /** @brief Internal notification about the vehicle moves.
     *  @note meso uses this though it never calls notifyMove()
     *
     * Indicator if the reminders is still active for the passed
     * vehicle/parameters. If false, the vehicle will erase this reminder
     * from it's reminder-container.
     *
     * @param[in] veh Vehicle that asks this reminder.
     * @param[in] frontOnLane time the front of the vehicle spent on the lane.
     * @param[in] timeOnLane time some part of the vehicle spent on the lane.
     * @param[in] meanSpeedFrontOnLane Average speed for the time that the front is on the lane.
     * @param[in] meanSpeedVehicleOnLane Average speed for the time that the vehicle is on the lane (with front or back).
     * @param[in] travelledDistanceFrontOnLane distance travelled while overlapping with the lane.
     * @param[in] travelledDistanceVehicleOnLane distance travelled while front was on the lane.
     * @param[in] meanLengthOnLane the average length of the vehicle's part on the lane during the last step (==complete length in meso case)
     */
    virtual void notifyMoveInternal(const SUMOTrafficObject& veh,
                                    const double frontOnLane,
                                    const double timeOnLane,
                                    const double meanSpeedFrontOnLane,
                                    const double meanSpeedVehicleOnLane,
                                    const double travelledDistanceFrontOnLane,
                                    const double travelledDistanceVehicleOnLane,
                                    const double meanLengthOnLane) {
        UNUSED_PARAMETER(meanLengthOnLane);
        UNUSED_PARAMETER(travelledDistanceFrontOnLane);
        UNUSED_PARAMETER(travelledDistanceVehicleOnLane);
        UNUSED_PARAMETER(meanSpeedVehicleOnLane);
        UNUSED_PARAMETER(meanSpeedFrontOnLane);
        UNUSED_PARAMETER(frontOnLane);
        UNUSED_PARAMETER(timeOnLane);
        UNUSED_PARAMETER(&veh);
    }

    void setDescription(const std::string& description) {
        myDescription = description;
    }

    const std::string& getDescription() const {
        return myDescription;
    }

protected:
    void removeFromVehicleUpdateValues(SUMOTrafficObject& veh);

protected:

    /// @brief Lane on which the reminder works
    MSLane* const myLane;
    /// @brief a description of this moveReminder
    std::string myDescription;

#ifdef HAVE_FOX
    /// @brief the mutex for notifications
    mutable FXMutex myNotificationMutex;
#endif

private:
    std::map<SUMOTrafficObject*, std::pair<SUMOTime, double> > myLastVehicleUpdateValues;


private:
    MSMoveReminder& operator=(const MSMoveReminder&); // just to avoid a compiler warning

};
