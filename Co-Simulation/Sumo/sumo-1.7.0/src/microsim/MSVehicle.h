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
/// @file    MSVehicle.h
/// @author  Christian Roessel
/// @author  Jakob Erdmann
/// @author  Bjoern Hendriks
/// @author  Daniel Krajzewicz
/// @author  Thimor Bohn
/// @author  Friedemann Wesner
/// @author  Clemens Honomichl
/// @author  Michael Behrisch
/// @author  Axel Wegener
/// @author  Leonhard Luecken
/// @date    Mon, 12 Mar 2001
///
// Representation of a vehicle in the micro simulation
/****************************************************************************/
#pragma once
#include <config.h>

#include <list>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include "MSGlobals.h"
#include "MSVehicleType.h"
#include "MSBaseVehicle.h"
#include "MSLink.h"
#include "MSLane.h"
#include "MSNet.h"

#define INVALID_SPEED 299792458 + 1 // nothing can go faster than the speed of light! Refs. #2577

// ===========================================================================
// class declarations
// ===========================================================================
class SUMOSAXAttributes;
class MSMoveReminder;
class MSLaneChanger;
class MSVehicleTransfer;
class MSAbstractLaneChangeModel;
class MSStoppingPlace;
class MSChargingStation;
class MSOverheadWire;
class MSParkingArea;
class MSPerson;
class MSDevice;
class OutputDevice;
class Position;
class MSJunction;
class MSLeaderInfo;
class MSDevice_DriverState;
class MSSimpleDriverState;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicle
 * @brief Representation of a vehicle in the micro simulation
 */
class MSVehicle : public MSBaseVehicle {
public:

    /// the lane changer sets myLastLaneChangeOffset
    friend class MSLaneChanger;
    friend class MSLaneChangerSublane;

    /** @class State
     * @brief Container that holds the vehicles driving state (position+speed).
     */
    class State {
        /// @brief vehicle sets states directly
        friend class MSVehicle;
        friend class MSLaneChanger;
        friend class MSLaneChangerSublane;

    public:
        /// Constructor.
        State(double pos, double speed, double posLat, double backPos);

        /// Copy constructor.
        State(const State& state);

        /// Assignment operator.
        State& operator=(const State& state);

        /// Operator !=
        bool operator!=(const State& state);

        /// Position of this state.
        double pos() const {
            return myPos;
        }

        /// Speed of this state
        double speed() const {
            return mySpeed;
        };

        /// Lateral Position of this state (m relative to the centerline of the lane).
        double posLat() const {
            return myPosLat;
        }

        /// back Position of this state
        double backPos() const {
            return myBackPos;
        }

        /// previous Speed of this state
        double lastCoveredDist() const {
            return myLastCoveredDist;
        }


    private:
        /// the stored position
        double myPos;

        /// the stored speed (should be >=0 at any time)
        double mySpeed;

        /// the stored lateral position
        double myPosLat;

        /// @brief the stored back position
        // if the vehicle occupies multiple lanes, this is the position relative
        // to the lane occupied by its back
        double myBackPos;

        /// the speed at the begin of the previous time step
        double myPreviousSpeed;

        /// the distance covered in the last timestep
        /// NOTE: In case of ballistic positional update, this is not necessarily given by
        ///       myPos - SPEED2DIST(mySpeed + myPreviousSpeed)/2,
        /// because a stop may have occurred within the last step.
        double myLastCoveredDist;

    };


    /** @class WaitingTimeCollector
     * @brief Stores the waiting intervals over the previous seconds (memory is to be specified in ms.).
     */
    class WaitingTimeCollector {
        friend class MSVehicle;

        typedef std::list<std::pair<SUMOTime, SUMOTime> > waitingIntervalList;

    public:
        /// Constructor.
        WaitingTimeCollector(SUMOTime memory = MSGlobals::gWaitingTimeMemory);

        /// Copy constructor.
        WaitingTimeCollector(const WaitingTimeCollector& wt);

        /// Assignment operator.
        WaitingTimeCollector& operator=(const WaitingTimeCollector& wt);

        /// Operator !=
        bool operator!=(const WaitingTimeCollector& wt) const;

        /// Assignment operator (in place!)
        WaitingTimeCollector& operator=(SUMOTime t);

        // return the waiting time within the last memory millisecs
        SUMOTime cumulatedWaitingTime(SUMOTime memory = -1) const;

        // process time passing for dt millisecs
        void passTime(SUMOTime dt, bool waiting);

        // maximal memory time stored
        SUMOTime getMemorySize() const {
            return myMemorySize;
        }

        // maximal memory time stored
        const waitingIntervalList& getWaitingIntervals() const {
            return myWaitingIntervals;
        }

    private:
        /// the maximal memory to store
        SUMOTime myMemorySize;

        /// the stored waiting intervals within the last memory milliseconds
        /// If the current (ongoing) waiting interval has begun at time t - dt (where t is the current time)
        /// then waitingIntervalList[0]->first = 0., waitingIntervalList[0]->second = dt
        waitingIntervalList myWaitingIntervals;

        /// append an amount of dt millisecs to the stored waiting times
        void appendWaitingTime(SUMOTime dt);
    };


    /** @enum ChangeRequest
     * @brief Requests set via TraCI
     */
    enum ChangeRequest {
        /// @brief vehicle doesn't want to change
        REQUEST_NONE,
        /// @brief vehicle want's to change to left lane
        REQUEST_LEFT,
        /// @brief vehicle want's to change to right lane
        REQUEST_RIGHT,
        /// @brief vehicle want's to keep the current lane
        REQUEST_HOLD
    };

    /** @brief Constructor
     * @param[in] pars The vehicle description
     * @param[in] route The vehicle's route
     * @param[in] type The vehicle's type
     * @param[in] speedFactor The factor for driven lane's speed limits
     * @exception ProcessError If a value is wrong
     */
    MSVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
              MSVehicleType* type, const double speedFactor);

    /// @brief Destructor.
    virtual ~MSVehicle();

    void initDevices();

    /// @brief checks wether the vehicle can depart on the first edge
    bool hasValidRouteStart(std::string& msg);

    /// @name insertion/removal
    //@{

    /** @brief Called when the vehicle is removed from the network.
     *
     * Moves along work reminders and
     *  informs all devices about quitting. Calls "leaveLane" then.
     *
     * @param[in] reason why the vehicle leaves (reached its destination, parking, teleport)
     */
    void onRemovalFromNet(const MSMoveReminder::Notification reason);
    //@}



    /// @name interaction with the route
    //@{

    /** @brief Returns whether this vehicle has already arived
     * (reached the arrivalPosition on its final edge)
     */
    bool hasArrived() const;

    /** @brief Replaces the current route by the given one
     *
     * It is possible that the new route is not accepted, if it does not
     *  contain the vehicle's current edge.
     *
     * @param[in] route The new route to pass
     * @param[in] info Information regarding the replacement
     * @param[in] removeStops Whether stops should be removed if they do not fit onto the new route
     * @return Whether the new route was accepted
     */
    bool replaceRoute(const MSRoute* route, const std::string& info, bool onInit = false, int offset = 0, bool addStops = true, bool removeStops = true);

    //@}


    /// @name Interaction with move reminders
    //@{

    /** @brief Processes active move reminder
     *
     * This method goes through all active move reminder, both those for the current
     *  lane, stored in "myMoveReminders" and those of prior lanes stored in
     *  "myOldLaneMoveReminders" calling "MSMoveReminder::notifyMove".
     *
     * When processing move reminder from "myOldLaneMoveReminders",
     *  the offsets (prior lane lengths) are used, which are stored in
     *  "myOldLaneMoveReminderOffsets".
     *
     * Each move reminder which is no longer active is removed from the container.
     *
     * @param[in] oldPos The position the vehicle had before it has moved
     * @param[in] newPos The position the vehicle has after it has moved
     * @param[in] newSpeed The vehicle's speed within this move
     * @see MSMoveReminder
     */
    void workOnMoveReminders(double oldPos, double newPos, double newSpeed);
    //@}

    /** @brief cycle through vehicle devices invoking notifyIdle
      *
      *   This is only implemented on the emissions device
      *     implemented to allow capture of emissions when vehicle is not on net.
      */
    void workOnIdleReminders();

    /** @brief Returns whether the vehicle is supposed to take action in the current simulation step
     *         Updates myActionStep and myLastActionTime in case that the current simstep is an action step
     *
     *  @param[in] t
     */
    bool checkActionStep(const SUMOTime t);

    /** @brief Resets the action offset for the vehicle
     *
     *  @param[in] timeUntilNextAction time interval from now for the next action, defaults to 0, which
     *             implies an immediate action point in the current step.
     */
    void resetActionOffset(const SUMOTime timeUntilNextAction = 0);


    /** @brief Process an updated action step length value (only affects the vehicle's *action offset*,
     *         The actionStepLength is stored in the (singular) vtype)
     *
     *  @param[in] oldActionStepLength The action step length previous to the update
     *  @param[in] actionStepLength The new action step length (stored in the vehicle's vtype).
     *  @note      The current action step length is updated. This implies an immediate action
     *             point, if the new step length is smaller than the length of the currently running
     *             action interval (the difference between now and the last action time).
     */
    void updateActionOffset(const SUMOTime oldActionStepLength, const SUMOTime newActionStepLength);


    /** @brief Compute safe velocities for the upcoming lanes based on positions and
     * speeds from the last time step. Also registers
     * ApproachingVehicleInformation for all links
     *
     * This method goes through the best continuation lanes of the current lane and
     * computes the safe velocities for passing/stopping at the next link as a DriveProcessItem
     *
     * Afterwards it checks if any DriveProcessItem should be discarded to avoid
     * blocking a junction (checkRewindLinkLanes).
     *
     * Finally the ApproachingVehicleInformation is registered for all links that
     * shall be passed
     *
     * @param[in] t The current timeStep
     * @param[in] ahead The leaders (may be 0)
     * @param[in] lengthsInFront Sum of vehicle lengths in front of the vehicle
     */
    void planMove(const SUMOTime t, const MSLeaderInfo& ahead, const double lengthsInFront);

    /** @brief Register junction approaches for all link items in the current
     * plan */
    void setApproachingForAllLinks(const SUMOTime t);


    /** @brief Executes planned vehicle movements with regards to right-of-way
     *
     * This method goes through all DriveProcessItems in myLFLinkLanes in order
     * to find a speed that is safe for all upcoming links.
     *
     * Using this speed the position is updated and the vehicle is moved to the
     * next lane (myLane is updated) if the end of the current lane is reached (this may happen
     * multiple times in this method)
     *
     * The vehicle also sets the lanes it is in-lapping into and informs them about it.
     * @return Whether the vehicle has moved to the next edge
     */
    bool executeMove();

    /// @brief move vehicle forward by the given distance during insertion
    void executeFractionalMove(double dist);

    /** @brief calculates the distance covered in the next integration step given
     *         an acceleration and assuming the current velocity. (gives different
     *         results for different integration methods, e.g., euler vs. ballistic)
     *  @param[in] accel the assumed acceleration
     *  @return distance covered in next integration step
     */
    double getDeltaPos(const double accel) const;


    /// @name state setter/getter
    //@{

    /** @brief Get the vehicle's position along the lane
     * @return The position of the vehicle (in m from the lane's begin)
     */
    double getPositionOnLane() const {
        return myState.myPos;
    }

    /** @brief Get the distance the vehicle covered in the previous timestep
     * @return The distance covered in the last timestep (in m)
     */
    double getLastStepDist() const {
        return myState.lastCoveredDist();
    }

    /** @brief Get the vehicle's front position relative to the given lane
     * @return The front position of the vehicle (in m from the given lane's begin)
     */
    double getPositionOnLane(const MSLane* lane) const;

    /** @brief Get the vehicle's position relative to the given lane
     * @return The back position of the vehicle (in m from the given lane's begin)
     * @note It is assumed that this function is only called for a vehicle that has
     *       a relation to the lane which makes it 'directly' relevant for
     *       car-following behavior on that lane, i.e., either it occupies part of the
     *       lanes surface (regular or partial vehicle for the lane), or (for the sublane
     *       model) it issued a maneuver reservation for a lane change.
     */
    double getBackPositionOnLane(const MSLane* lane) const;

    /** @brief Get the vehicle's position relative to its current lane
     * @return The back position of the vehicle (in m from the current lane's begin)
     */
    double getBackPositionOnLane() const {
        return getBackPositionOnLane(myLane);
    }

    /** @brief Get the vehicle's lateral position on the lane
     * @return The lateral position of the vehicle (in m relative to the
     * centerline of the lane)
     */
    double getLateralPositionOnLane() const {
        return myState.myPosLat;
    }

    void setLateralPositionOnLane(double posLat) {
        myState.myPosLat = posLat;
    }

    /** @brief Get the vehicle's lateral position on the lane:
     * @return The lateral position of the vehicle (in m distance between right
     * side of vehicle and ride side of the lane it is on
     */
    double getRightSideOnLane() const;

    /** @brief Get the minimal lateral distance required to move fully onto the lane at given offset
     * @return The lateral distance to be covered to move the vehicle fully onto the lane (in m)
     */
    double lateralDistanceToLane(const int offset) const;

    /// @brief return the amount by which the vehicle extends laterally outside it's primary lane
    double getLateralOverlap() const;
    double getLateralOverlap(double posLat) const;

    /** @brief Get the vehicle's lateral position on the edge of the given lane
     * (or its current edge if lane == 0)
     * @return The lateral position of the vehicle (in m distance between right
     * side of vehicle and ride side of edge
     */
    double getRightSideOnEdge(const MSLane* lane = 0) const;

    /** @brief Get the vehicle's lateral position on the edge of the given lane
     * (or its current edge if lane == 0)
     * @return The lateral position of the vehicle (in m distance between center
     * of vehicle and ride side of edge
     */
    double getCenterOnEdge(const MSLane* lane = 0) const;

    /** @brief Get the offset that that must be added to interpret
     * myState.myPosLat for the given lane
     *  @note This means that latOffset + myPosLat should give the relative shift of the vehicle's center
     *        wrt the centerline of the given lane.
     */
    double getLatOffset(const MSLane* lane) const;

    /** @brief Returns the vehicle's current speed
     * @return The vehicle's speed
     */
    double getSpeed() const {
        return myState.mySpeed;
    }


    /** @brief Returns the vehicle's speed before the previous time step
     * @return The vehicle's speed before the previous time step
     */
    double getPreviousSpeed() const {
        return myState.myPreviousSpeed;
    }


    /** @brief Sets the influenced previous speed
     * @param[in] A double value with the speed that overwrites the previous speed
     */
    void setPreviousSpeed(double prevspeed) {
        myState.mySpeed = MAX2(0., prevspeed);
    }


    /** @brief Returns the vehicle's acceleration in m/s
     *         (this is computed as the last step's mean acceleration in case that a stop occurs within the middle of the time-step)
     * @return The acceleration
     */
    double getAcceleration() const {
        return myAcceleration;
    }

    /// @brief get apparent deceleration based on vType parameters and current acceleration
    double getCurrentApparentDecel() const;

    /** @brief Returns the vehicle's action step length in millisecs,
     *         i.e. the interval between two action points.
     * @return The current action step length in ms.
     */
    SUMOTime getActionStepLength() const {
        return myType->getActionStepLength();
    }

    /** @brief Returns the vehicle's action step length in secs,
     *         i.e. the interval between two action points.
     * @return The current action step length in s.
     */
    double getActionStepLengthSecs() const {
        return myType->getActionStepLengthSecs();
    }


    /** @brief Returns the time of the vehicle's last action point.
     * @return The time of the last action point
     */
    SUMOTime getLastActionTime() const {
        return myLastActionTime;
    }

    //@}



    /// @name Other getter methods
    //@{

    /** @brief Returns the slope of the road at vehicle's position
     * @return The slope
     */
    double getSlope() const;


    /** @brief Return current position (x/y, cartesian)
     *
     * If the vehicle's myLane is 0, Position::INVALID.
     * @param[in] offset optional offset in longitudinal direction
     * @return The current position (in cartesian coordinates)
     * @see myLane
     */
    Position getPosition(const double offset = 0) const;


    /** @brief Return the (x,y)-position, which the vehicle would reach
     *         if it continued along its best continuation lanes from the current
     *         for a distance of offset m.
     * @param offset
     * @return (x,y)-Position
     * @see getPosition()
     */
    Position getPositionAlongBestLanes(double offset) const;


    /** @brief Returns the lane the vehicle is on
     * @return The vehicle's current lane
     */
    MSLane* getLane() const {
        return myLane;
    }


    /** @brief Returns the maximal speed for the vehicle on its current lane (including speed factor and deviation,
     *         i.e., not necessarily the allowed speed limit)
     * @return The vehicle's max speed
     */
    double
    getMaxSpeedOnLane() const {
        if (myLane != 0) {
            return myLane->getVehicleMaxSpeed(this);
        } else {
            return myType->getMaxSpeed();
        }
    }


    /** @brief Returns the information whether the vehicle is on a road (is simulated)
     * @return Whether the vehicle is simulated
     */
    inline bool isOnRoad() const {
        return myAmOnNet;
    }

    /** @brief access function for Idling flag
     *      used to record whether vehicle is waiting to enter lane (after parking)
     */
    void
    setIdling(bool amIdling) {
        myAmIdling = amIdling;
    }

    /** @brief Returns whether a sim vehicle is waiting to enter a lane
     *      (after parking has completed)
     * @return true if the vehicle is waiting
     */
    inline bool isIdling() const {
        return myAmIdling;
    }

    /** @brief Returns whether the current simulation step is an action point for the vehicle
     * @return Whether the vehicle has an action point in the current step.
     */
    inline bool isActive() const {
        return myActionStep;
    }

    /** @brief Returns whether the next simulation step will be an action point for the vehicle
     * @return Whether the vehicle has scheduled an action point for the next step.
     */
    inline bool isActionStep(SUMOTime t) const {
        return (t - myLastActionTime) % getActionStepLength() == 0;
//        return t%getActionStepLength() == 0; // synchronized actions for all vehicles with identical actionsteplengths
    }


    /** @brief Returns the information whether the front of the vehicle is on the given lane
     * @return Whether the vehicle's front is on that lane
     */
    bool isFrontOnLane(const MSLane* lane) const;


    /** @brief Returns the starting point for reroutes (usually the current edge)
     *
     * This differs from *myCurrEdge only if the vehicle is on an internal edge or
     *  very close to the junction
     * @return The rerouting start point
     */
    const MSEdge* getRerouteOrigin() const;


    /** @brief Returns the SUMOTime waited (speed was lesser than 0.1m/s)
     *
     * The value is reset if the vehicle moves faster than 0.1m/s
     * Intentional stopping does not count towards this time.
     * @return The time the vehicle is standing
     */
    SUMOTime getWaitingTime() const {
        return myWaitingTime;
    }

    /** @brief Returns the SUMOTime lost (speed was lesser maximum speed)
     *
     * @note Intentional stopping does not count towards this time.
    // @note speedFactor is included so time loss can never be negative.
    // The value is that of a driver who compares his travel time when
    // the road is clear (which includes speed factor) with the actual travel time.
    // @note includes time lost due to low departSpeed and decelerating/accelerating for planned stops
     * @return The time the vehicle lost due to various effects
     */
    SUMOTime getTimeLoss() const {
        return TIME2STEPS(myTimeLoss);
    }


    /** @brief Returns the SUMOTime waited (speed was lesser than 0.1m/s) within the last t millisecs
     *
     * @return The time the vehicle was standing within the configured memory interval
     */
    SUMOTime getAccumulatedWaitingTime() const {
        return myWaitingTimeCollector.cumulatedWaitingTime(MSGlobals::gWaitingTimeMemory);
    }

    /** @brief Returns the number of seconds waited (speed was lesser than 0.1m/s)
     *
     * The value is reset if the vehicle moves faster than 0.1m/s
     * Intentional stopping does not count towards this time.
     * @return The time the vehicle is standing
     */
    double getWaitingSeconds() const {
        return STEPS2TIME(myWaitingTime);
    }


    /** @brief Returns the number of seconds waited (speed was lesser than 0.1m/s) within the last millisecs
     *
     * @return The time the vehicle was standing within the last t millisecs
     */

    double getAccumulatedWaitingSeconds() const {
        return STEPS2TIME(getAccumulatedWaitingTime());
    }

    /** @brief Returns the time loss in seconds
     */
    double getTimeLossSeconds() const {
        return myTimeLoss;
    }

    /** @brief Returns the public transport stop delay in seconds
     */
    double getStopDelay() const;

    /** @brief Returns the vehicle's direction in radians
     * @return The vehicle's current angle
     */
    double getAngle() const {
        return myAngle;
    }


    /** @brief Returns the vehicle's direction in radians
     * @return The vehicle's current angle
     */
    Position getVelocityVector() const {
        return Position(std::cos(myAngle) * myState.speed(), std::sin(myAngle) * myState.speed());
    }
    //@}

    /// @brief compute the current vehicle angle
    double computeAngle() const;

    /// @brief Set a custom vehicle angle in rad, optionally updates furtherLanePosLat
    void setAngle(double angle, bool straightenFurther = false);

    /** @brief Sets the action steplength of the vehicle
     *
     * @param actionStepLength New value
     * @param resetActionOffset whether the action offset should be reset to zero,
     *        i.e., the next action step should follow immediately.
     */
    void setActionStepLength(double actionStepLength, bool resetActionOffset = true);

    /** Returns true if the two vehicles overlap. */
    static bool overlap(const MSVehicle* veh1, const MSVehicle* veh2) {
        if (veh1->myState.myPos < veh2->myState.myPos) {
            return veh2->myState.myPos - veh2->getVehicleType().getLengthWithGap() < veh1->myState.myPos;
        }
        return veh1->myState.myPos - veh1->getVehicleType().getLengthWithGap() < veh2->myState.myPos;
    }

    /** Returns true if vehicle's speed is below 60km/h. This is only relevant
        on highways. Overtaking on the right is allowed then. */
    bool congested() const {
        return myState.mySpeed < double(60) / double(3.6);
    }


    /** @brief "Activates" all current move reminder
     *
     * For all move reminder stored in "myMoveReminders", their method
     *  "MSMoveReminder::notifyEnter" is called.
     *
     * @param[in] reason The reason for changing the reminders' states
     * @param[in] enteredLane The lane, which is entered (if applicable)
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    void activateReminders(const MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);

    /** @brief Update when the vehicle enters a new lane in the move step.
     *
     * @param[in] enteredLane The lane the vehicle enters
     * @param[in] onTeleporting Whether the lane was entered while being teleported
     * @return Whether the vehicle's route has ended (due to vaporization, or because the destination was reached)
     */
    bool enterLaneAtMove(MSLane* enteredLane, bool onTeleporting = false);



    /** @brief Update when the vehicle enters a new lane in the emit step
     *
     * @param[in] enteredLane The lane the vehicle enters
     * @param[in] pos The position the vehicle was inserted into the lane
     * @param[in] speed The speed with which the vehicle was inserted into the lane
     * @param[in] posLat The lateral position the vehicle was inserted into the lane
     * @param[in] notification The cause of insertion (i.e. departure, teleport, parking)
     */
    void enterLaneAtInsertion(MSLane* enteredLane, double pos, double speed, double posLat,
                              MSMoveReminder::Notification notification);

    /** @brief set tentative lane and position during insertion to ensure that
     * all cfmodels work (some of them require veh->getLane() to return a valid lane)
     * Once the vehicle is sucessfully inserted the lane is set again (see enterLaneAtInsertion)
     */
    void setTentativeLaneAndPosition(MSLane* lane, double pos, double posLat = 0);

    /** @brief Update when the vehicle enters a new lane in the laneChange step.
     *
     * @param[in] enteredLane The lane the vehicle enters
     */
    void enterLaneAtLaneChange(MSLane* enteredLane);


    /** @brief Update of members if vehicle leaves a new lane in the lane change step or at arrival. */
    void leaveLane(const MSMoveReminder::Notification reason, const MSLane* approachedLane = 0);

    /** @brief Check whether the drive items (myLFLinkLanes) are up to date,
     *         and update them if required.
     *  @note  This is the case if a lane change was completed.
     *         Only the links corresponding to the drive items are updated to the
     *         corresponding parallel links.
     */
    void updateDriveItems();

    /** @brief Get the distance and direction of the next upcoming turn for the vehicle (within its look-ahead range)
     *  @return The first entry of the returned pair is the distance for the upcoming turn, the second is the link direction
     */
    const std::pair<double, LinkDirection>& getNextTurn() {
        return myNextTurn;
    }


    MSAbstractLaneChangeModel& getLaneChangeModel();
    const MSAbstractLaneChangeModel& getLaneChangeModel() const;

    const std::vector<MSLane*>& getFurtherLanes() const {
        return myFurtherLanes;
    }

    const std::vector<double>& getFurtherLanesPosLat() const {
        return myFurtherLanesPosLat;
    }


    /// @brief whether this vehicle has its back (and no its front) on the given edge
    bool onFurtherEdge(const MSEdge* edge) const;

    /// @name strategical/tactical lane choosing methods
    /// @{

    //
    /** @struct LaneQ
     * @brief A structure representing the best lanes for continuing the current route starting at 'lane'
     */
    struct LaneQ {
        /// @brief The described lane
        MSLane* lane;
        /// @brief The overall length which may be driven when using this lane without a lane change
        double length;
        /// @brief The length which may be driven on this lane
        double currentLength;
        /// @brief The overall vehicle sum on consecutive lanes which can be passed without a lane change
        double occupation;
        /// @brief As occupation, but without the first lane
        double nextOccupation;
        /// @brief The (signed) number of lanes to be crossed to get to the lane which allows to continue the drive
        int bestLaneOffset;
        /// @brief Whether this lane allows to continue the drive
        bool allowsContinuation;
        /* @brief Longest sequence of (normal-edge) lanes that can be followed without a lane change
         * The 'length' attribute is the sum of these lane lengths
         * (There may be alternative sequences that have equal length)
         * It is the 'best' in the strategic sense of reducing required lane-changes
         */
        std::vector<MSLane*> bestContinuations;
    };

    /** @brief Returns the description of best lanes to use in order to continue the route
     * @return The LaneQ for all lanes of the current edge
     */
    const std::vector<LaneQ>& getBestLanes() const;

    /** @brief computes the best lanes to use in order to continue the route
     *
     * The information is rebuilt if the vehicle is on a different edge than
     *  the one stored in "myLastBestLanesEdge" or "forceRebuild" is true.
     *
     * Otherwise, only the density changes on the stored lanes are adapted to
     *  the container only.
     *
     * A rebuild must be done if the vehicle leaves a stop; then, another lane may become
     *  the best one.
     *
     * If no starting lane ("startLane") is given, the vehicle's current lane ("myLane")
     *  is used as start of best lanes building.
     *
     * @param[in] forceRebuild Whether the best lanes container shall be rebuilt even if the vehicle's edge has not changed
     * @param[in] startLane The lane the process shall start at ("myLane" will be used if ==0)
     */
    void updateBestLanes(bool forceRebuild = false, const MSLane* startLane = 0);


    /** @brief Returns the best sequence of lanes to continue the route starting at myLane
     * @return The bestContinuations of the LaneQ for myLane (see LaneQ)
     */
    const std::vector<MSLane*>& getBestLanesContinuation() const;


    /** @brief Returns the best sequence of lanes to continue the route starting at the given lane
     * @return The bestContinuations of the LaneQ for the given lane (see LaneQ)
     */
    const std::vector<MSLane*>& getBestLanesContinuation(const MSLane* const l) const;

    /** @brief Returns the upcoming (best followed by default 0) sequence of lanes to continue the route starting at the current lane
     * @param[in] distance The downstream distance to cover
     * @return The bestContinuations of the LaneQ for myLane (see LaneQ) concatenated with default following lanes up until
     *  the given distance has been covered
     * @note includes internal lanes if applicable
     */
    const std::vector<const MSLane*> getUpcomingLanesUntil(double distance) const;

    /** @brief Returns the sequence of past lanes (right-most on edge) based on the route starting at the current lane
     * @param[in] distance The upstream distance to cover
     * @return The myRoute-based past lanes (right-most on edge) up until the given distance has been covered
     * @note includes internal lanes if applicable
     */
    const std::vector<const MSLane*> getPastLanesUntil(double distance) const;

    /* @brief returns the current signed offset from the lane that is most
     * suited for continuing the current route (in the strategic sense of reducing lane-changes)
     * - 0 if the vehicle is one it's best lane
     * - negative if the vehicle should change to the right
     * - positive if the vehicle should change to the left
     */
    int getBestLaneOffset() const;

    /// @brief update occupation from MSLaneChanger
    void adaptBestLanesOccupation(int laneIndex, double density);

    /// @}

    /// @brief repair errors in vehicle position after changing between internal edges
    void fixPosition();

    /// @brief return lane and position along bestlanes at the given distance
    std::pair<const MSLane*, double> getLanePosAfterDist(double distance) const;

    /** @brief Returns the vehicle's car following model definition
     *
     * This is simply a wrapper around the vehicle type's car-following
     *  model retrieval for a shorter access.
     *
     * @return The vehicle's car following model definition
     */
    inline const MSCFModel& getCarFollowModel() const {
        return myType->getCarFollowModel();
    }

    /** @brief Returns the vehicle driver's state
     *
     * @return The vehicle driver's state
     *
     */

    std::shared_ptr<MSSimpleDriverState> getDriverState() const;


    /** @brief Returns the vehicle's car following model variables
     *
     * @return The vehicle's car following model variables
     */
    inline MSCFModel::VehicleVariables* getCarFollowVariables() const {
        return myCFVariables;
    }

    /// @name vehicle stops definitions and i/o
    //@{

    /** @class Stop
     * @brief Definition of vehicle stop (position and duration)
     */
    class Stop {
    public:
        Stop(const SUMOVehicleParameter::Stop& par) : pars(par) {}
        /// @brief The edge in the route to stop at
        MSRouteIterator edge;
        /// @brief The lane to stop at
        const MSLane* lane = nullptr;
        /// @brief (Optional) bus stop if one is assigned to the stop
        MSStoppingPlace* busstop = nullptr;
        /// @brief (Optional) container stop if one is assigned to the stop
        MSStoppingPlace* containerstop = nullptr;
        /// @brief (Optional) parkingArea if one is assigned to the stop
        MSParkingArea* parkingarea = nullptr;
        /// @brief (Optional) charging station if one is assigned to the stop
        MSStoppingPlace* chargingStation = nullptr;
        /// @brief (Optional) overhead wire segment if one is assigned to the stop
        /// @todo Check that this should really be a stopping place instance
        MSStoppingPlace* overheadWireSegment = nullptr;
        /// @brief The stop parameter
        const SUMOVehicleParameter::Stop pars;
        /// @brief The stopping duration
        SUMOTime duration = -1;
        /// @brief whether an arriving person lets the vehicle continue
        bool triggered = false;
        /// @brief whether an arriving container lets the vehicle continue
        bool containerTriggered = false;
        /// @brief whether coupling another vehicle (train) the vehicle continue
        bool joinTriggered = false;
        /// @brief Information whether the stop has been reached
        bool reached = false;
        /// @brief The number of still expected persons
        int numExpectedPerson = 0;
        /// @brief The number of still expected containers
        int numExpectedContainer = 0;
        /// @brief The time at which the vehicle is able to board another person
        SUMOTime timeToBoardNextPerson = 0;
        /// @brief The time at which the vehicle is able to load another container
        SUMOTime timeToLoadNextContainer = 0;
        /// @brief Whether this stop was triggered by a collision
        bool collision = false;
        /// @brief the maximum time at which persons may board this vehicle
        SUMOTime endBoarding = SUMOTime_MAX;

        /// @brief Write the current stop configuration (used for state saving)
        void write(OutputDevice& dev) const;

        /// @brief return halting position for upcoming stop;
        double getEndPos(const SUMOVehicle& veh) const;

        /// @brief get a short description for showing in the gui
        std::string getDescription() const;

        /// @brief initialize attributes from the given stop parameters
        void initPars(const SUMOVehicleParameter::Stop& stopPar);

    private:
        /// @brief Invalidated assignment operator
        Stop& operator=(const Stop& src);

    };


    /** @brief Adds a stop
     *
     * The stop is put into the sorted list.
     * @param[in] stop The stop to add
     * @return Whether the stop could be added
     */
    bool addStop(const SUMOVehicleParameter::Stop& stopPar, std::string& errorMsg, SUMOTime untilOffset = 0, bool collision = false,
                 MSRouteIterator* searchStart = 0);

    /** @brief replace the current parking area stop with a new stop with merge duration
     */
    bool replaceParkingArea(MSParkingArea* parkingArea, std::string& errorMsg);

    /** @brief get the upcoming parking area stop or nullptr
     */
    MSParkingArea* getNextParkingArea();

    /** @brief get the current  parking area stop or nullptr */
    MSParkingArea* getCurrentParkingArea();

    /** @brief Returns whether the vehicle has to stop somewhere
     * @return Whether the vehicle has to stop somewhere
     */
    bool hasStops() const {
        return !myStops.empty();
    }

    /** @brief Whether this vehicle is equipped with a MSDriverState
     */
    inline bool hasDriverState() const {
        return myDriverState != nullptr;
    }

    /** @brief Returns whether the vehicle is at a stop
     * @return Whether the vehicle has stopped
     */
    bool isStopped() const;

    /// @brief Returns the remaining stop duration for a stopped vehicle or 0
    SUMOTime remainingStopDuration() const;

    /** @brief Returns whether the vehicle stops at the given stopping place */
    bool stopsAt(MSStoppingPlace* stop) const;

    /** @brief Returns whether the vehicle stops at the given edge */
    bool stopsAtEdge(const MSEdge* edge) const;

    /** @brief Returns whether the vehicle will stop on the current edge
     */
    bool willStop() const;

    //// @brief Returns whether the vehicle is at a stop and on the correct lane
    bool isStoppedOnLane() const;

    /** @brief Returns whether the vehicle is stopped and must continue to do so */
    bool keepStopping(bool afterProcessing = false) const;

    /** @brief Returns the remaining time a vehicle needs to stop due to a
     * collision. A negative value indicates that the vehicle is not stopping due to a collision (or at all)
     */
    SUMOTime collisionStopTime() const;

    /** @brief Returns whether the vehicle is parking
     * @return whether the vehicle is parking
     */
    bool isParking() const;

    /** @brief Returns the information whether the vehicle is fully controlled via TraCI
     * @return Whether the vehicle is remote-controlled
     */
    bool isRemoteControlled() const;

    /** @brief Returns the information whether the vehicle is fully controlled via TraCI
     * within the lookBack time
     */
    bool wasRemoteControlled(SUMOTime lookBack = DELTA_T) const;

    /// @brief return the distance to the next stop or doubleMax if there is none.
    double nextStopDist() const {
        return myStopDist;
    }

    /** @brief Returns whether the vehicle is on a triggered stop
     * @return whether the vehicle is on a triggered stop
     */
    bool isStoppedTriggered() const;

    /** @brief return whether the given position is within range of the current stop
     */
    bool isStoppedInRange(const double pos, const double tolerance) const;
    /// @}

    int getLaneIndex() const;

    /**
     * Compute distance that will be covered, if the vehicle moves to a given position on its route,
     * starting at its current position.
     * @param destPos:  position on the destination edge that shall be reached
     * @param destEdge: destination edge that shall be reached
     * @return      distance from the vehicles current position to the destination position,
     *          or a near infinite real value if the destination position is not contained
     *          within the vehicles route or the vehicle is not active
     */
    double getDistanceToPosition(double destPos, const MSEdge* destEdge) const;


    /** @brief Processes stops, returns the velocity needed to reach the stop
     * @return The velocity in dependance to the next/current stop
     * @todo Describe more detailed
     * @see Stop
     * @see MSStoppingPlace
     * @see MSStoppingPlace
     */
    double processNextStop(double currentVelocity);


    /** @brief Returns the leader of the vehicle looking for a fixed distance.
     *
     * If the distance is not given it is calculated from the brake gap.
     * The gap returned does not include the minGap.
     * @param dist    up to which distance to look at least for a leader
     * @return The leading vehicle together with the gap; (0, -1) if no leader was found.
     */
    std::pair<const MSVehicle* const, double> getLeader(double dist = 0) const;

    /** @brief Returns the follower of the vehicle looking for a fixed distance.
     *
     * If the distance is not given it is set to the value of MSCFModel::brakeGap(2*roadSpeed, 4.5, 0)
     * The gap returned does not include the minGap.
     * If there are multiple followers, the one that maximizes the term (getSecureGap - gap) is returned.
     * @param dist    up to which distance to look at least for a leader
     * @return The leading vehicle together with the gap; (0, -1) if no leader was found.
     */
    std::pair<const MSVehicle* const, double> getFollower(double dist = 0) const;

    /** @brief Returns the time gap in seconds to the leader of the vehicle on the same lane.
     *
     * If the distance is too big -1 is returned.
     * The gap returned takes the minGap into account.
     * @return The time gap in seconds; -1 if no leader was found or speed is 0.
     */
    double getTimeGapOnLane() const;


    /** @brief Adds a person or container to this vehicle
     *
     * @param[in] transportable The person/container to add
     */
    void addTransportable(MSTransportable* transportable);

    /// @name Access to bool signals
    /// @{

    /** @enum Signalling
     * @brief Some boolean values which describe the state of some vehicle parts
     */
    enum Signalling {
        /// @brief Everything is switched off
        VEH_SIGNAL_NONE = 0,
        /// @brief Right blinker lights are switched on
        VEH_SIGNAL_BLINKER_RIGHT = 1,
        /// @brief Left blinker lights are switched on
        VEH_SIGNAL_BLINKER_LEFT = 2,
        /// @brief Blinker lights on both sides are switched on
        VEH_SIGNAL_BLINKER_EMERGENCY = 4,
        /// @brief The brake lights are on
        VEH_SIGNAL_BRAKELIGHT = 8,
        /// @brief The front lights are on (no visualisation)
        VEH_SIGNAL_FRONTLIGHT = 16,
        /// @brief The fog lights are on (no visualisation)
        VEH_SIGNAL_FOGLIGHT = 32,
        /// @brief The high beam lights are on (no visualisation)
        VEH_SIGNAL_HIGHBEAM = 64,
        /// @brief The backwards driving lights are on (no visualisation)
        VEH_SIGNAL_BACKDRIVE = 128,
        /// @brief The wipers are on
        VEH_SIGNAL_WIPER = 256,
        /// @brief One of the left doors is opened
        VEH_SIGNAL_DOOR_OPEN_LEFT = 512,
        /// @brief One of the right doors is opened
        VEH_SIGNAL_DOOR_OPEN_RIGHT = 1024,
        /// @brief A blue emergency light is on
        VEH_SIGNAL_EMERGENCY_BLUE = 2048,
        /// @brief A red emergency light is on
        VEH_SIGNAL_EMERGENCY_RED = 4096,
        /// @brief A yellow emergency light is on
        VEH_SIGNAL_EMERGENCY_YELLOW = 8192
    };


    /** @brief modes for resolving conflicts between external control (traci)
     * and vehicle control over lane changing. Each level of the lane-changing
     * hierarchy (strategic, cooperative, speedGain, keepRight) can be controlled
     * separately */
    enum LaneChangeMode {
        LC_NEVER      = 0,  // lcModel shall never trigger changes at this level
        LC_NOCONFLICT = 1,  // lcModel may trigger changes if not in conflict with TraCI request
        LC_ALWAYS     = 2   // lcModel may always trigger changes of this level regardless of requests
    };


    /// @brief modes for prioritizing traci lane change requests
    enum TraciLaneChangePriority {
        LCP_ALWAYS        = 0,  // change regardless of blockers, adapt own speed and speed of blockers
        LCP_NOOVERLAP     = 1,  // change unless overlapping with blockers, adapt own speed and speed of blockers
        LCP_URGENT        = 2,  // change if not blocked, adapt own speed and speed of blockers
        LCP_OPPORTUNISTIC = 3   // change if not blocked
    };


    /** @brief Switches the given signal on
     * @param[in] signal The signal to mark as being switched on
     */
    void switchOnSignal(int signal) {
        mySignals |= signal;
    }


    /** @brief Switches the given signal off
     * @param[in] signal The signal to mark as being switched off
     */
    void switchOffSignal(int signal) {
        mySignals &= ~signal;
    }


    /** @brief Returns the signals
     * @return The signals' states
     */
    int getSignals() const {
        return mySignals;
    }


    /** @brief Returns whether the given signal is on
     * @param[in] signal The signal to return the value of
     * @return Whether the given signal is on
     */
    bool signalSet(int which) const {
        return (mySignals & which) != 0;
    }
    /// @}


    /// @brief whether the vehicle may safely move to the given lane with regard to upcoming links
    bool unsafeLinkAhead(const MSLane* lane) const;

    /// @brief decide whether the vehicle is passing a minor link or has comitted to do so
    bool passingMinor() const;



    /** @brief Returns the uninfluenced velocity
     *
     * If no influencer exists (myInfluencer==0) the vehicle's current speed is
     *  returned. Otherwise the speed returned from myInfluencer->getOriginalSpeed().
     * @return The vehicle's velocity as it would without an influence
     * @see Influencer::getOriginalSpeed
     */
    double getSpeedWithoutTraciInfluence() const;

    /**
     * reroute the vehicle to the new parking area, updating routes and passengers/containers associated trips
     * @param parkingAreaID    id of the new parking area
     */
    bool rerouteParkingArea(const std::string& parkingAreaID, std::string& errorMsg);

    /**
     * schedule a new stop for the vehicle; each time a stop is reached, the vehicle
     * will wait for the given duration before continuing on its route
     * @param[in] stop Stop parameters
     * @param[out] errorMsg returned error message
     */
    bool addTraciStop(SUMOVehicleParameter::Stop stop, std::string& errorMsg);

    /**
     * replace the next stop at the given index with the given stop parameters
     * will wait for the given duration before continuing on its route
     * The route between start other stops and destination will be kept unchanged and
     * only the part around the replacement index will be adapted according to the new stop location
     * @param[in] nextStopDist The replacement index
     * @param[in] stop Stop parameters
     * @param[out] errorMsg returned error message
     */
    bool replaceStop(int nextStopIndex, SUMOVehicleParameter::Stop stop, const std::string& info, std::string& errorMsg);

    /**
    * returns the next imminent stop in the stop queue
    * @return the upcoming stop
    */
    Stop& getNextStop();

    /// @brief return parameters for the next stop (SUMOVehicle Interface)
    const SUMOVehicleParameter::Stop* getNextStopParameter() const;

    /**
    * returns the list of stops not yet reached in the stop queue
    * @return the list of upcoming stops
    */
    inline const std::list<Stop>& getStops() {
        return myStops;
    }

    inline const std::vector<SUMOVehicleParameter::Stop>& getPastStops() {
        return myPastStops;
    }

    /**
    * resumes a vehicle from stopping
    * @return true on success, the resuming fails if the vehicle wasn't parking in the first place
    */
    bool resumeFromStopping();

    /// @brief deletes the next stop at the given index if it exists
    bool abortNextStop(int nextStopIndex = 0);

    /// @brief update a vector of further lanes and return the new backPos
    double updateFurtherLanes(std::vector<MSLane*>& furtherLanes,
                              std::vector<double>& furtherLanesPosLat,
                              const std::vector<MSLane*>& passedLanes);

    /// @brief get bounding rectangle
    PositionVector getBoundingBox() const;

    /// @brief get bounding polygon
    PositionVector getBoundingPoly() const;

    /** @enum ManoeuvreType
     *  @brief  flag identifying which, if any, manoeuvre is in progress
     */
    enum ManoeuvreType {
        /// @brief Manoeuvre into stopping place
        MANOEUVRE_ENTRY,
        /// @brief Manoeuvre out of stopping place
        MANOEUVRE_EXIT,
        /// @brief not manouevring
        MANOEUVRE_NONE
    };

    /// @brief accessor function to myManoeuvre equivalent
    /// @note Setup of exit manoeuvre is invoked from MSVehicleTransfer
    bool setExitManoeuvre();
    /// @brief accessor function to myManoeuvre equivalent
    void setManoeuvreType(const MSVehicle::ManoeuvreType mType);

    /// @brief accessor function to myManoeuvre equivalent
    bool manoeuvreIsComplete() const;
    /// @brief accessor function to myManoeuvre equivalent
    MSVehicle::ManoeuvreType getManoeuvreType() const;


    /** @class Manoeuvre
      * @brief  Container for manouevering time associated with stopping.
      *
      *  Introduced to cater for lane blocking whilst entering stop/leaving stop
      *   and assure that emissions during manoeuvre are included in model
      */
    class Manoeuvre {

    public:
        /// Constructor.
        Manoeuvre();

        /// Copy constructor.
        Manoeuvre(const Manoeuvre& manoeuvre);

        /// Assignment operator.
        Manoeuvre& operator=(const Manoeuvre& manoeuvre);

        /// Operator !=
        bool operator!=(const Manoeuvre& manoeuvre);

        /// @brief Setup the entry manoeuvre for this vehicle (Sets completion time and manoeuvre type)
        bool configureEntryManoeuvre(MSVehicle* veh);

        /// @brief Setup the myManoeuvre for exiting (Sets completion time and manoeuvre type)
        bool configureExitManoeuvre(MSVehicle* veh);

        /// @brief Configure an entry manoeuvre if nothing is configured - otherwise check if complete
        bool entryManoeuvreIsComplete(MSVehicle* veh);

        /// @brief Check if specific manoeuver is ongoing and whether the completion time is beyond currentTime
        bool
        manoeuvreIsComplete(const ManoeuvreType checkType) const;

        /// @brief Check if any manoeuver is ongoing and whether the completion time is beyond currentTime
        bool
        manoeuvreIsComplete() const;

        /// @brief Accessor for GUI rotation step when parking (radians)
        double getGUIIncrement() const;

        /// @brief Accessor (get) for manoeuvre type
        MSVehicle::ManoeuvreType getManoeuvreType() const;

        /// @brief Accessor (set) for manoeuvre type
        void setManoeuvreType(const MSVehicle::ManoeuvreType mType);

    private:
        /// @brief  The name of the vehicle associated with the Manoeuvre  - for debug output
        std::string myManoeuvreVehicleID;

        /// @brief  The name of the stop associated with the Manoeuvre  - for debug output
        std::string myManoeuvreStop;

        /// @brief Time at which the Manoeuvre for this stop started
        SUMOTime myManoeuvreStartTime;

        /// @brief Time at which this manoeuvre should complete
        SUMOTime myManoeuvreCompleteTime;

        /// @brief Manoeuvre type - currently entry, exit or none
        ManoeuvreType myManoeuvreType;

        // @brief Angle (radians) through which parking vehicle moves in each sim step
        double myGUIIncrement;
    };

    // Current or previous (completed) manoeuvre
    Manoeuvre myManoeuvre;

    /** @class Influencer
      * @brief Changes the wished vehicle speed / lanes
      *
      * The class is used for passing velocities or velocity profiles obtained via TraCI to the vehicle.
      * The speed adaptation is controlled by the stored speedTimeLine
      * Additionally, the variables myConsiderSafeVelocity, myConsiderMaxAcceleration, and myConsiderMaxDeceleration
      * control whether the safe velocity, the maximum acceleration, and the maximum deceleration
      * have to be regarded.
      *
      * Furthermore this class is used to affect lane changing decisions according to
      * LaneChangeMode and any given laneTimeLine
      */
    class Influencer : public BaseInfluencer {
    private:

        /// @brief A static instance of this class in GapControlState deactivates gap control
        ///        for vehicles whose reference vehicle is removed from the road network
        class GapControlVehStateListener : public MSNet::VehicleStateListener {
            /** @brief Called if a vehicle changes its state
             * @param[in] vehicle The vehicle which changed its state
             * @param[in] to The state the vehicle has changed to
             * @param[in] info Additional information on the state change
             * @note This deactivates the corresponding gap control when a reference vehicle is removed.
             */
            void vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& info = "");
        };


        /// @brief Container for state and parameters of the gap control
        struct GapControlState {
            GapControlState();
            virtual ~GapControlState();
            /// @brief Static initalization (adds vehicle state listener)
            static void init();
            /// @brief Static cleanup (removes vehicle state listener)
            static void cleanup();
            /// @brief Start gap control with given params
            void activate(double tauOriginal, double tauTarget, double additionalGap, double duration, double changeRate, double maxDecel, const MSVehicle* refVeh);
            /// @brief Stop gap control
            void deactivate();
            /// @brief Original value for the desired headway (will be reset after duration has expired)
            double tauOriginal;
            /// @brief Current, interpolated value for the desired time headway
            double tauCurrent;
            /// @brief Target value for the desired time headway
            double tauTarget;
            /// @brief Current, interpolated value for the desired space headway
            double addGapCurrent;
            /// @brief Target value for the desired space headway
            double addGapTarget;
            /// @brief Remaining duration for keeping the target headway
            double remainingDuration;
            /// @brief Rate by which the current time and space headways are changed towards the target value.
            ///        (A rate of one corresponds to reaching the target value within one second)
            double changeRate;
            /// @brief Maximal deceleration to be applied due to the adapted headway
            double maxDecel;
            /// @brief reference vehicle for the gap - if it is null, the current leader on the ego's lane is used as a reference
            const MSVehicle* referenceVeh;
            /// @brief Whether the gap control is active
            bool active;
            /// @brief Whether the desired gap was attained during the current activity phase (induces the remaining duration to decrease)
            bool gapAttained;
            /// @brief The last recognized leader
            const MSVehicle* prevLeader;
            /// @brief Time of the last update of the gap control
            SUMOTime lastUpdate;
            /// @brief cache storage for the headway increments of the current operation
            double timeHeadwayIncrement, spaceHeadwayIncrement;

            /// @brief stores reference vehicles currently in use by a gapController
            static std::map<const MSVehicle*, GapControlState*> refVehMap;

        private:
            static GapControlVehStateListener vehStateListener;
        };


    public:
        /// @brief Constructor
        Influencer();

        /// @brief Destructor
        ~Influencer();

        /// @brief Static initalization
        static void init();
        /// @brief Static cleanup
        static void cleanup();

        /** @brief Sets a new velocity timeline
         * @param[in] speedTimeLine The time line of speeds to use
         */
        void setSpeedTimeLine(const std::vector<std::pair<SUMOTime, double> >& speedTimeLine);

        /** @brief Activates the gap control with the given parameters, @see GapControlState
         */
        void activateGapController(double originalTau, double newTimeHeadway, double newSpaceHeadway, double duration, double changeRate, double maxDecel, MSVehicle* refVeh = nullptr);

        /** @brief Deactivates the gap control
         */
        void deactivateGapController();

        /** @brief Sets a new lane timeline
         * @param[in] laneTimeLine The time line of lanes to use
         */
        void setLaneTimeLine(const std::vector<std::pair<SUMOTime, int> >& laneTimeLine);

        /** @brief Adapts lane timeline when moving to a new lane and the lane index changes
         * @param[in] indexShift The change in laneIndex
         */
        void adaptLaneTimeLine(int indexShift);

        /** @brief Sets a new sublane-change request
         * @param[in] latDist The lateral distance for changing
         */
        void setSublaneChange(double latDist);

        /// @brief return the current speed mode
        int getSpeedMode() const;

        /// @brief return the current lane change mode
        int getLaneChangeMode() const;

        SUMOTime getLaneTimeLineDuration();

        SUMOTime getLaneTimeLineEnd();

        /** @brief Applies stored velocity information on the speed to use.
         *
         * The given speed is assumed to be the non-influenced speed from longitudinal control.
         *  It is stored for further usage in "myOriginalSpeed".
         * @param[in] currentTime The current simulation time
         * @param[in] speed The undisturbed speed
         * @param[in] vSafe The safe velocity
         * @param[in] vMin The minimum velocity
         * @param[in] vMax The maximum simulation time
         * @return The speed to use
         */
        double influenceSpeed(SUMOTime currentTime, double speed, double vSafe, double vMin, double vMax);

        /** @brief Applies gap control logic on the speed.
         *
         * The given speed is assumed to be the non-influenced speed from longitudinal control.
         *  It is stored for further usage in "myOriginalSpeed".
         * @param[in] currentTime The current simulation time
         * @param[in] veh The controlled vehicle
         * @param[in] speed The undisturbed speed
         * @param[in] vSafe The safe velocity
         * @param[in] vMin The minimum velocity
         * @param[in] vMax The maximum simulation time
         * @return The speed to use (<=speed)
         */
        double gapControlSpeed(SUMOTime currentTime, const SUMOVehicle* veh, double speed, double vSafe, double vMin, double vMax);

        /** @brief Applies stored LaneChangeMode information and laneTimeLine
         * @param[in] currentTime The current simulation time
         * @param[in] currentEdge The current edge the vehicle is on
         * @param[in] currentLaneIndex The index of the lane the vehicle is currently on
         * @param[in] state The LaneChangeAction flags as computed by the laneChangeModel
         * @return The new LaneChangeAction flags to use
         */
        int influenceChangeDecision(const SUMOTime currentTime, const MSEdge& currentEdge, const int currentLaneIndex, int state);


        /** @brief Return the remaining number of seconds of the current
         * laneTimeLine assuming one exists
         * @param[in] currentTime The current simulation time
         * @return The remaining seconds to change lanes
         */
        double changeRequestRemainingSeconds(const SUMOTime currentTime) const;

        /** @brief Returns whether junction priority rules shall be respected
         * @return Whether junction priority rules be respected
         */
        inline bool getRespectJunctionPriority() const {
            return myRespectJunctionPriority;
        }


        /** @brief Returns whether red lights shall be a reason to brake
         * @return Whether red lights shall be a reason to brake
         */
        inline bool getEmergencyBrakeRedLight() const {
            return myEmergencyBrakeRedLight;
        }


        /// @brief Returns whether safe velocities shall be considered
        bool considerSafeVelocity() const {
            return myConsiderSafeVelocity;
        }

        /** @brief Sets speed-constraining behaviors
         * @param[in] value a bitset controlling the different modes
         */
        void setSpeedMode(int speedMode);

        /** @brief Sets lane changing behavior
         * @param[in] value a bitset controlling the different modes
         */
        void setLaneChangeMode(int value);

        /** @brief Returns the originally longitudinal speed to use
         * @return The speed given before influence or -1 if no influence is active
         */
        double getOriginalSpeed() const;

        void setRemoteControlled(Position xyPos, MSLane* l, double pos, double posLat, double angle, int edgeOffset, const ConstMSEdgeVector& route, SUMOTime t);

        SUMOTime getLastAccessTimeStep() const {
            return myLastRemoteAccess;
        }

        void postProcessRemoteControl(MSVehicle* v);

        /// @brief return the speed that is implicit in the new remote position
        double implicitSpeedRemote(const MSVehicle* veh, double oldSpeed);

        /// @brief return the change in longitudinal position that is implicit in the new remote position
        double implicitDeltaPosRemote(const MSVehicle* veh);

        bool isRemoteControlled() const;

        bool isRemoteAffected(SUMOTime t) const;

        void setSignals(int signals) {
            myTraCISignals = signals;
        }

        int getSignals() const {
            return myTraCISignals;
        }

        double getLatDist() const {
            return myLatDist;
        }

        void resetLatDist() {
            myLatDist = 0.;
        }

        bool ignoreOverlap() const {
            return myTraciLaneChangePriority == LCP_ALWAYS;
        }

    private:
        /// @brief The velocity time line to apply
        std::vector<std::pair<SUMOTime, double> > mySpeedTimeLine;

        /// @brief The lane usage time line to apply
        std::vector<std::pair<SUMOTime, int> > myLaneTimeLine;

        /// @brief The gap control state
        std::shared_ptr<GapControlState> myGapControlState;

        /// @brief The velocity before influence
        double myOriginalSpeed;

        /// @brief The requested lateral change
        double myLatDist;

        /// @brief Whether influencing the speed has already started
        bool mySpeedAdaptationStarted;

        /// @brief Whether the safe velocity shall be regarded
        bool myConsiderSafeVelocity;

        /// @brief Whether the maximum acceleration shall be regarded
        bool myConsiderMaxAcceleration;

        /// @brief Whether the maximum deceleration shall be regarded
        bool myConsiderMaxDeceleration;

        /// @brief Whether the junction priority rules are respected
        bool myRespectJunctionPriority;

        /// @brief Whether red lights are a reason to brake
        bool myEmergencyBrakeRedLight;

        Position myRemoteXYPos;
        MSLane* myRemoteLane;
        double myRemotePos;
        double myRemotePosLat;
        double myRemoteAngle;
        int myRemoteEdgeOffset;
        ConstMSEdgeVector myRemoteRoute;
        SUMOTime myLastRemoteAccess;

        /// @name Flags for managing conflicts between the laneChangeModel and TraCI laneTimeLine
        //@{
        /// @brief lane changing which is necessary to follow the current route
        LaneChangeMode myStrategicLC;
        /// @brief lane changing with the intent to help other vehicles
        LaneChangeMode myCooperativeLC;
        /// @brief lane changing to travel with higher speed
        LaneChangeMode mySpeedGainLC;
        /// @brief changing to the rightmost lane
        LaneChangeMode myRightDriveLC;
        /// @brief changing to the prefered lateral alignment
        LaneChangeMode mySublaneLC;
        //@}
        ///* @brief flags for determining the priority of traci lane change requests
        TraciLaneChangePriority myTraciLaneChangePriority;

        // @brief the signals set via TraCI
        int myTraCISignals;

    };


    /** @brief Returns the velocity/lane influencer
     *
     * If no influencer was existing before, one is built, first
     * @return Reference to this vehicle's speed influencer
     */
    BaseInfluencer& getBaseInfluencer();
    Influencer& getInfluencer();

    const BaseInfluencer* getBaseInfluencer() const;
    const Influencer* getInfluencer() const;

    bool hasInfluencer() const {
        return myInfluencer != nullptr;
    }

    /// @brief allow TraCI to influence a lane change decision
    int influenceChangeDecision(int state);

    /// @brief sets position outside the road network
    void setRemoteState(Position xyPos);

    /// @brief departure position where the vehicle fits fully onto the edge (if possible)
    double basePos(const MSEdge* edge) const;

    /// @brief compute safe speed for following the given leader
    double getSafeFollowSpeed(const std::pair<const MSVehicle*, double> leaderInfo,
                              const double seen, const MSLane* const lane, double distToCrossing) const;

    /// @brief get a numerical value for the priority of the  upcoming link
    static int nextLinkPriority(const std::vector<MSLane*>& conts);

    /// @brief whether the given vehicle must be followed at the given junction
    bool isLeader(const MSLink* link, const MSVehicle* veh) const;

    // @brief get the position of the back bumper;
    const Position getBackPosition() const;

    /// @brief whether this vehicle is except from collision checks
    bool ignoreCollision();

    /// @name state io
    //@{

    /// Saves the states of a vehicle
    void saveState(OutputDevice& out);

    /** @brief Loads the state of this vehicle from the given description
     */
    void loadState(const SUMOSAXAttributes& attrs, const SUMOTime offset);

    void loadPreviousApproaching(MSLink* link, bool setRequest,
                                 SUMOTime arrivalTime, double arrivalSpeed,
                                 SUMOTime arrivalTimeBraking, double arrivalSpeedBraking,
                                 double dist, double leaveSpeed);
    //@}

protected:

    double getSpaceTillLastStanding(const MSLane* l, bool& foundStopped) const;

    /// @name Interaction with move reminders
    ///@{

    /** @brief Adapts the vehicle's entering of a new lane
     *
     * All offsets already stored in "myOldLaneMoveReminderOffsets" are increased by the
     *  length that has been left. All still active move reminders from "myMoveReminders"
     *  are put into "myOldLaneMoveReminders" and the offset to the last lane is added to
     *  "myOldLaneMoveReminderOffsets" for each of these.
     *
     * Move reminder from the given lane are set into "myMoveReminders".
     *
     * "myLane" must still be the left lane!
     *
     * @param[in] enteredLane
     * @see MSMoveReminder
     * @see MSLane::getMoveReminder
     */
    void adaptLaneEntering2MoveReminder(const MSLane& enteredLane);
    ///@}


    /** @brief This method iterates through the driveprocess items for the vehicle
     *         and adapts the given in/out parameters to the appropriate values.
     *
     *  @param[in/out] vSafe The maximal safe (or admissible) velocity.
     *  @param[in/out] vSafeMin The minimal safe (or admissible) velocity (used her for ensuring the clearing of junctions in time).
     *  @param[in/out] vSafeMinDist The distance to the next link, which should either be crossed this step, or in front of which the vehicle need to stop.
     */
    void processLinkApproaches(double& vSafe, double& vSafeMin, double& vSafeMinDist);


    /** @brief This method checks if the vehicle has advanced over one or several lanes
     *         along its route and triggers the corresponding actions for the lanes and the vehicle.
     *         and adapts the given in/out parameters to the appropriate values.
     *
     *  @param[out] passedLanes Lanes, which the vehicle touched at some moment of the executed simstep
     *  @param[out] moved Whether the vehicle did move to another lane
     *  @param[out] emergencyReason Reason for a possible emergency stop
     */
    void processLaneAdvances(std::vector<MSLane*>& passedLanes, bool& moved, std::string& emergencyReason);


    /** @brief Check for speed advices from the traci client and adjust the speed vNext in
     *         the current (euler) / after the current (ballistic) simstep accordingly.
     *
     *  @param[in] vSafe The maximal safe (or admissible) velocity as determined from stops, junction approaches, car following, lane changing, etc.
     *  @param[in] vNext The next speed (possibly subject to traci influence)
     *  @return updated vNext
     */
    double processTraCISpeedControl(double vSafe, double vNext);


    /** @brief Erase passed drive items from myLFLinkLanes (and unregister approaching information for
     *         corresponding links). Further, myNextDriveItem is reset.
     *  @note  This is called in planMove() if the vehicle has no actionstep. All items until the position
     *         myNextDriveItem are deleted. This can happen if myNextDriveItem was increased in processLaneAdvances()
     *         of the previous step.
     */
    void removePassedDriveItems();

    /** @brief Updates the vehicle's waiting time counters (accumulated and consecutive)
     */
    void updateWaitingTime(double vNext);

    /** @brief Updates the vehicle's time loss
     */
    void updateTimeLoss(double vNext);

    /* @brief Check whether the vehicle is a train that can reverse its direction at the current point in its route
     * and return the speed in preparation for reversal
     *
     *  @param[out] canReverse
     *  @param[in] speedThreshold
     *  @return speed for reversal
     */
    double checkReversal(bool& canReverse, double speedThreshold = SUMO_const_haltingSpeed, double seen = 0) const;

    /** @brief sets the braking lights on/off
     */
    void setBrakingSignals(double vNext) ;

    /** @brief sets the blue flashing light for emergency vehicles
     */
    void setBlinkerInformation();

    /** @brief sets the blue flashing light for emergency vehicles
     */
    void setEmergencyBlueLight(SUMOTime currentTime);

    /// updates LaneQ::nextOccupation and myCurrentLaneInBestLanes
    void updateOccupancyAndCurrentBestLane(const MSLane* startLane);

    /** @brief Returns the list of still pending stop edges
     * also returns the first and last stop position
     */
    const ConstMSEdgeVector getStopEdges(double& firstPos, double& lastPos) const;

    /// @brief return list of route indices for the remaining stops
    std::vector<std::pair<int, double> > getStopIndices() const;

    /// @brief get distance for coming to a stop (used for rerouting checks)
    double getBrakeGap() const;

    /// @brief ensure that a vehicle-relative position is not invalid
    Position validatePosition(Position result, double offset = 0) const;

    /// @brief register vehicle for drawing while outside the network
    virtual void drawOutsideNetwork(bool /*add*/) {};

    /// @brief board persons and load transportables at the given stop
    void boardTransportables(Stop& stop);

    /// @brief try joining the given vehicle to the rear of this one (to resolve joinTriggered)
    bool joinTrainPart(MSVehicle* veh);

    /// @brief try joining the given vehicle to the front of this one (to resolve joinTriggered)
    bool joinTrainPartFront(MSVehicle* veh);

protected:

    /// @brief The time the vehicle waits (is not faster than 0.1m/s) in seconds
    SUMOTime myWaitingTime;
    WaitingTimeCollector myWaitingTimeCollector;

    /// @brief the time loss in seconds due to driving with less than maximum speed
    double myTimeLoss;

    /// @brief This Vehicles driving state (pos and speed)
    State myState;

    /// @brief This vehicle's driver state @see MSDriverState
    MSDevice_DriverState* myDriverState;

    /// @brief The flag myActionStep indicates whether the current time step is an action point for the vehicle.
    bool myActionStep;
    /// @brief Action offset (actions are taken at time myActionOffset + N*getActionStepLength())
    ///        Initialized to 0, to be set at insertion.
    SUMOTime myLastActionTime;



    /// The lane the vehicle is on
    MSLane* myLane;

    MSAbstractLaneChangeModel* myLaneChangeModel;

    const MSEdge* myLastBestLanesEdge;
    const MSLane* myLastBestLanesInternalLane;

    /* @brief Complex data structure for keeping and updating LaneQ:
     * Each element of the outer vector corresponds to an upcoming edge on the vehicles route
     * The first element corresponds to the current edge and is returned in getBestLanes()
     * The other elements are only used as a temporary structure in updateBestLanes();
     */
    std::vector<std::vector<LaneQ> > myBestLanes;

    /* @brief iterator to speed up retrieval of the current lane's LaneQ in getBestLaneOffset() and getBestLanesContinuation()
     * This is updated in updateOccupancyAndCurrentBestLane()
     */
    std::vector<LaneQ>::iterator myCurrentLaneInBestLanes;

    static std::vector<MSLane*> myEmptyLaneVector;

    /// @brief The vehicle's list of stops
    std::list<Stop> myStops;

    /// @brief The list of stops that the vehicle has already reached
    std::vector<SUMOVehicleParameter::Stop> myPastStops;

    /// @brief The current acceleration after dawdling in m/s
    double myAcceleration;

    /// @brief the upcoming turn for the vehicle
    /// @todo calculate during plan move
    std::pair<double, LinkDirection> myNextTurn;

    /// @brief The information into which lanes the vehicle laps into
    std::vector<MSLane*> myFurtherLanes;
    /// @brief lateral positions on further lanes
    std::vector<double> myFurtherLanesPosLat;

    /// @brief State of things of the vehicle that can be on or off
    int mySignals;

    /// @brief Whether the vehicle is on the network (not parking, teleported, vaporized, or arrived)
    bool myAmOnNet;

    /// @brief Whether the vehicle is trying to enter the network (eg after parking so engine is running)
    bool myAmIdling;

    /// @brief Whether this vehicle is registered as waiting for a person (for deadlock-recognition)
    bool myAmRegisteredAsWaitingForPerson;

    /// @brief Whether this vehicle is registered as waiting for a container (for deadlock-recognition)
    bool myAmRegisteredAsWaitingForContainer;

    bool myHaveToWaitOnNextLink;

    /// @brief the angle in radians (@todo consider moving this into myState)
    double myAngle;

    /// @brief distance to the next stop or doubleMax if there is none
    double myStopDist;

    /// @brief amount of time for which the vehicle is immune from collisions
    SUMOTime myCollisionImmunity;

    mutable Position myCachedPosition;

    /// @brief time at which the current junction was entered
    SUMOTime myJunctionEntryTime;
    SUMOTime myJunctionEntryTimeNeverYield;
    SUMOTime myJunctionConflictEntryTime;

protected:

    /// @brief Drive process items represent bounds on the safe velocity
    ///        corresponding to the upcoming links.
    /// @todo: improve documentation
    struct DriveProcessItem {
        MSLink* myLink;
        double myVLinkPass;
        double myVLinkWait;
        bool mySetRequest;
        SUMOTime myArrivalTime;
        double myArrivalSpeed;
        SUMOTime myArrivalTimeBraking;
        double myArrivalSpeedBraking;
        double myDistance;
        double accelV;
        bool hadStoppedVehicle;
        double availableSpace;

        DriveProcessItem(MSLink* link, double vPass, double vWait, bool setRequest,
                         SUMOTime arrivalTime, double arrivalSpeed,
                         SUMOTime arrivalTimeBraking, double arrivalSpeedBraking,
                         double distance,
                         double leaveSpeed = -1.) :
            myLink(link), myVLinkPass(vPass), myVLinkWait(vWait), mySetRequest(setRequest),
            myArrivalTime(arrivalTime), myArrivalSpeed(arrivalSpeed),
            myArrivalTimeBraking(arrivalTimeBraking), myArrivalSpeedBraking(arrivalSpeedBraking),
            myDistance(distance),
            accelV(leaveSpeed), hadStoppedVehicle(false), availableSpace(0) {
            assert(vWait >= 0 || !MSGlobals::gSemiImplicitEulerUpdate);
            assert(vPass >= 0 || !MSGlobals::gSemiImplicitEulerUpdate);
        };


        /// @brief constructor if the link shall not be passed
        DriveProcessItem(double vWait, double distance, double _availableSpace = 0) :
            myLink(0), myVLinkPass(vWait), myVLinkWait(vWait), mySetRequest(false),
            myArrivalTime(0), myArrivalSpeed(0),
            myArrivalTimeBraking(0), myArrivalSpeedBraking(0),
            myDistance(distance),
            accelV(-1), hadStoppedVehicle(false), availableSpace(_availableSpace) {
            assert(vWait >= 0 || !MSGlobals::gSemiImplicitEulerUpdate);
        };


        inline void adaptLeaveSpeed(const double v) {
            if (accelV < 0) {
                accelV = v;
            } else {
                accelV = MIN2(accelV, v);
            }
        }
        inline double getLeaveSpeed() const {
            return accelV < 0 ? myVLinkPass : accelV;
        }
    };

    /// Container for used Links/visited Lanes during planMove() and executeMove.
    // TODO: Consider making LFLinkLanes a std::deque for efficient front removal (needs refactoring in checkRewindLinkLanes()...)
    typedef std::vector< DriveProcessItem > DriveItemVector;

    /// @brief container for the planned speeds in the current step
    DriveItemVector myLFLinkLanes;

    /// @brief planned speeds from the previous step for un-registering from junctions after the new container is filled
    DriveItemVector myLFLinkLanesPrev;

    /** @brief iterator pointing to the next item in myLFLinkLanes
    *   @note  This is updated whenever the vehicle advances to a subsequent lane (see processLaneAdvances())
    *          and used for inter-actionpoint actualization of myLFLinkLanes (i.e. deletion of passed items)
    *          in planMove().
    */
    DriveItemVector::iterator myNextDriveItem;

    /// @todo: documentation
    void planMoveInternal(const SUMOTime t, MSLeaderInfo ahead, DriveItemVector& lfLinks, double& myStopDist, std::pair<double, LinkDirection>& myNextTurn) const;

    /// @brief runs heuristic for keeping the intersection clear in case of downstream jamming
    void checkRewindLinkLanes(const double lengthsInFront, DriveItemVector& lfLinks) const;

    /// @brief unregister approach from all upcoming links
    void removeApproachingInformation(const DriveItemVector& lfLinks) const;


    /// @brief estimate leaving speed when accelerating across a link
    inline double estimateLeaveSpeed(const MSLink* const link, const double vLinkPass) const {
        // estimate leave speed for passing time computation
        // l=linkLength, a=accel, t=continuousTime, v=vLeave
        // l=v*t + 0.5*a*t^2, solve for t and multiply with a, then add v
        return MIN2(link->getViaLaneOrLane()->getVehicleMaxSpeed(this),
                    getCarFollowModel().estimateSpeedAfterDistance(link->getLength(), vLinkPass, getVehicleType().getCarFollowModel().getMaxAccel()));
    }


    /* @brief adapt safe velocity in accordance to a moving obstacle:
     * - a leader vehicle
     * - a vehicle or pedestrian that crosses this vehicles path on an upcoming intersection
     * @param[in] leaderInfo The leading vehicle and the (virtual) distance to it
     * @param[in] seen the distance to the end of the current lane
     * @param[in] lastLink the lastLink index
     * @param[in] lane The current Lane the vehicle is on
     * @param[in,out] the safe velocity for driving
     * @param[in,out] the safe velocity for arriving at the next link
     * @param[in] distToCrossing The distance to the crossing point with the current leader where relevant or -1
     */
    void adaptToLeader(const std::pair<const MSVehicle*, double> leaderInfo,
                       const double seen, DriveProcessItem* const lastLink,
                       const MSLane* const lane, double& v, double& vLinkPass,
                       double distToCrossing = -1) const;

    /* @brief adapt safe velocity in accordance to multiple vehicles ahead:
     * @param[in] ahead The leader information according to the current lateral-resolution
     * @param[in] latOffset the lateral offset for locating the ego vehicle on the given lane
     * @param[in] seen the distance to the end of the current lane
     * @param[in] lastLink the lastLink index
     * @param[in] lane The current Lane the vehicle is on
     * @param[in,out] the safe velocity for driving
     * @param[in,out] the safe velocity for arriving at the next link
     */
    void adaptToLeaders(const MSLeaderInfo& ahead,
                        double latOffset,
                        const double seen, DriveProcessItem* const lastLink,
                        const MSLane* const lane, double& v, double& vLinkPass) const;

    /// @brief checks for link leaders on the given link
    void checkLinkLeader(const MSLink* link, const MSLane* lane, double seen,
                         DriveProcessItem* const lastLink, double& v, double& vLinkPass, double& vLinkWait, bool& setRequest,
                         bool isShadowLink = false) const;

    /// @brief checks for link leaders of the current link as well as the parallel link (if there is one)
    void checkLinkLeaderCurrentAndParallel(const MSLink* link, const MSLane* lane, double seen,
                                           DriveProcessItem* const lastLink, double& v, double& vLinkPass, double& vLinkWait, bool& setRequest) const;


    // @brief return the lane on which the back of this vehicle resides
    const MSLane* getBackLane() const;

    /** @brief updates the vehicles state, given a next value for its speed.
     *         This value can be negative in case of the ballistic update to indicate
     *         a stop within the next timestep. (You can call this a 'hack' to
     *         emulate reasoning based on accelerations: The assumed constant
     *         acceleration a within the next time step is then a = (vNext - vCurrent)/TS )
     *  @param[in] vNext speed in the next time step
     */
    void updateState(double vNext);


    /// @brief decide whether the given link must be kept clear
    bool keepClear(const MSLink* link) const;

    /// @brief decide whether a red (or yellow light) may be ignore
    bool ignoreRed(const MSLink* link, bool canBrake) const;


    /// @brief check whether all stop.edge MSRouteIterators are valid and in order
    bool haveValidStopEdges() const;

private:
    /// @brief The per vehicle variables of the car following model
    MSCFModel::VehicleVariables* myCFVariables;

    /// @brief An instance of a velocity/lane influencing instance; built in "getInfluencer"
    Influencer* myInfluencer;


private:
    /// @brief invalidated default constructor
    MSVehicle();

    /// @brief invalidated copy constructor
    MSVehicle(const MSVehicle&);

    /// @brief invalidated assignment operator
    MSVehicle& operator=(const MSVehicle&);

};
