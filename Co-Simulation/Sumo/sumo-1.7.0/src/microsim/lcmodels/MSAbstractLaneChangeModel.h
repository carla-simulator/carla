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
/// @file    MSAbstractLaneChangeModel.h
/// @author  Daniel Krajzewicz
/// @author  Friedemann Wesner
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Leonhard Luecken
/// @date    Fri, 29.04.2005
///
// Interface for lane-change models
/****************************************************************************/
#pragma once
#include <config.h>

#include <microsim/MSGlobals.h>
#include <microsim/MSVehicle.h>

class MSLane;

// ===========================================================================
// used enumeration
// ===========================================================================

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSAbstractLaneChangeModel
 * @brief Interface for lane-change models
 */
class MSAbstractLaneChangeModel {
public:

    /** @class MSLCMessager
     * @brief A class responsible for exchanging messages between cars involved in lane-change interaction
     */
    class MSLCMessager {
    public:
        /** @brief Constructor
         * @param[in] leader The leader on the informed vehicle's lane
         * @param[in] neighLead The leader on the lane the vehicle want to change to
         * @param[in] neighFollow The follower on the lane the vehicle want to change to
         */
        MSLCMessager(MSVehicle* leader,  MSVehicle* neighLead, MSVehicle* neighFollow)
            : myLeader(leader), myNeighLeader(neighLead),
              myNeighFollower(neighFollow) { }


        /// @brief Destructor
        ~MSLCMessager() { }


        /** @brief Informs the leader on the same lane
         * @param[in] info The information to pass
         * @param[in] sender The sending vehicle (the lane changing vehicle)
         * @return Something!?
         */
        void* informLeader(void* info, MSVehicle* sender) {
            assert(myLeader != 0);
            return myLeader->getLaneChangeModel().inform(info, sender);
        }


        /** @brief Informs the leader on the desired lane
         * @param[in] info The information to pass
         * @param[in] sender The sending vehicle (the lane changing vehicle)
         * @return Something!?
         */
        void* informNeighLeader(void* info, MSVehicle* sender) {
            assert(myNeighLeader != 0);
            return myNeighLeader->getLaneChangeModel().inform(info, sender);
        }


        /** @brief Informs the follower on the desired lane
         * @param[in] info The information to pass
         * @param[in] sender The sending vehicle (the lane changing vehicle)
         * @return Something!?
         */
        void* informNeighFollower(void* info, MSVehicle* sender) {
            assert(myNeighFollower != 0);
            return myNeighFollower->getLaneChangeModel().inform(info, sender);
        }


    private:
        /// @brief The leader on the informed vehicle's lane
        MSVehicle* myLeader;
        /// @brief The leader on the lane the vehicle want to change to
        MSVehicle* myNeighLeader;
        /// @brief The follower on the lane the vehicle want to change to
        MSVehicle* myNeighFollower;

    };

    struct StateAndDist {
        // @brief LaneChangeAction flags
        int state;
        // @brief Lateral distance to be completed in the next step
        double latDist;
        // @brief Full lateral distance required for the completion of the envisioned maneuver
        double maneuverDist;
        // @brief direction that was checked
        int dir;

        StateAndDist(int _state, double _latDist, double _targetDist, int _dir) :
            state(_state),
            latDist(_latDist),
            maneuverDist(_targetDist),
            dir(_dir) {}

        bool sameDirection(const StateAndDist& other) const {
            return latDist * other.latDist > 0;
        }
    };

    /// @brief init global model parameters
    void static initGlobalOptions(const OptionsCont& oc);

    /** @brief Factory method for instantiating new lane changing models
     * @param[in] lcm The type of model to build
     * @param[in] vehicle The vehicle for which this model shall be built
     */
    static MSAbstractLaneChangeModel* build(LaneChangeModel lcm, MSVehicle& vehicle);

    /// @brief whether any kind of lateral dynamics is active
    inline static bool haveLateralDynamics() {
        return MSGlobals::gLateralResolution > 0 || MSGlobals::gLaneChangeDuration > 0;
    }

    /** @brief Returns the model's ID;
     * @return The model's ID
     */
    virtual LaneChangeModel getModelID() const = 0;

    /// @brief whether lanechange-output is active
    static bool haveLCOutput() {
        return myLCOutput;
    }

    /// @brief whether start of maneuvers shall be recorede
    static bool outputLCStarted() {
        return myLCStartedOutput;
    }

    /// @brief whether start of maneuvers shall be recorede
    static bool outputLCEnded() {
        return myLCEndedOutput;
    }

    /** @brief Constructor
     * @param[in] v The vehicle this lane-changer belongs to
     * @param[in] model The type of lane change model
     */
    MSAbstractLaneChangeModel(MSVehicle& v, const LaneChangeModel model);

    /// @brief Destructor
    virtual ~MSAbstractLaneChangeModel();

    inline int getOwnState() const {
        return myOwnState;
    }

    inline int getPrevState() const {
        /// at the time of this call myPreviousState already holds the new value
        return myPreviousState2;
    }

    virtual void setOwnState(const int state);

    /// @brief Updates the remaining distance for the current maneuver while it is continued within non-action steps (only used by sublane model)
    void setManeuverDist(const double dist);
    /// @brief Returns the remaining unblocked distance for the current maneuver. (only used by sublane model)
    double getManeuverDist() const;
    double getPreviousManeuverDist() const;

    /// @brief Updates the value of safe lateral distances (in SL2015) during maneuver continuation in non-action steps
    virtual void updateSafeLatDist(const double travelledLatDist);

    const std::pair<int, int>& getSavedState(const int dir) const {
        if (dir == -1) {
            return mySavedStateRight;
        } else if (dir == 0) {
            return mySavedStateCenter;
        } else {
            return mySavedStateLeft;
        }
    }

    void saveLCState(const int dir, const int stateWithoutTraCI, const int state) {
        const auto pair = std::make_pair(stateWithoutTraCI | getCanceledState(dir), state);
        if (dir == -1) {
            mySavedStateRight = pair;
        } else if (dir == 0) {
            mySavedStateCenter = pair;
        } else {
            mySavedStateLeft = pair;
        }
    }

    /// @brief Saves the lane change relevant vehicles, which are currently on neighboring lanes in the given direction
    ///        (as detected in wantsChangeSublane()). -> SL2015 case
    void saveNeighbors(const int dir, const MSLeaderDistanceInfo& followers, const MSLeaderDistanceInfo& leaders);

    /// @brief Saves the lane change relevant vehicles, which are currently on neighboring lanes in the given direction
    ///        (as detected in wantsChange()). -> LC2013 case
    void saveNeighbors(const int dir, const std::pair<MSVehicle* const, double>& follower, const std::pair<MSVehicle* const, double>& leader);

    /// @brief Clear info on neighboring vehicle from previous step
    void clearNeighbors();

    /// @brief Returns the neighboring, lc-relevant followers for the last step in the requested direction
    const std::shared_ptr<MSLeaderDistanceInfo> getFollowers(const int dir);

    /// @brief Returns the neighboring, lc-relevant leaders for the last step in the requested direction
    const std::shared_ptr<MSLeaderDistanceInfo> getLeaders(const int dir);

    int& getCanceledState(const int dir) {
        if (dir == -1) {
            return myCanceledStateRight;
        } else if (dir == 0) {
            return myCanceledStateCenter;
        } else {
            return myCanceledStateLeft;
        }
    }

    /// @return whether this vehicle is blocked from performing a strategic change
    bool isStrategicBlocked() const;

    void setFollowerGaps(CLeaderDist follower, double secGap);
    void setLeaderGaps(CLeaderDist, double secGap);
    void setOrigLeaderGaps(CLeaderDist, double secGap);
    void setFollowerGaps(const MSLeaderDistanceInfo& vehicles);
    void setLeaderGaps(const MSLeaderDistanceInfo& vehicles);
    void setOrigLeaderGaps(const MSLeaderDistanceInfo& vehicles);

    virtual void prepareStep();

    /** @brief Called to examine whether the vehicle wants to change
     * using the given laneOffset.
     * This method gets the information about the surrounding vehicles
     * and whether another lane may be more preferable */
    virtual int wantsChange(
        int laneOffset,
        MSAbstractLaneChangeModel::MSLCMessager& msgPass, int blocked,
        const std::pair<MSVehicle*, double>& leader,
        const std::pair<MSVehicle*, double>& neighLead,
        const std::pair<MSVehicle*, double>& neighFollow,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked,
        MSVehicle** firstBlocked) {
        UNUSED_PARAMETER(laneOffset);
        UNUSED_PARAMETER(&msgPass);
        UNUSED_PARAMETER(blocked);
        UNUSED_PARAMETER(&leader);
        UNUSED_PARAMETER(&neighLead);
        UNUSED_PARAMETER(&neighFollow);
        UNUSED_PARAMETER(&neighLane);
        UNUSED_PARAMETER(&preb);
        UNUSED_PARAMETER(lastBlocked);
        UNUSED_PARAMETER(firstBlocked);
        throw ProcessError("Method not implemented by model " + toString(myModel));
    };

    virtual int wantsChangeSublane(
        int laneOffset,
        LaneChangeAction alternatives,
        const MSLeaderDistanceInfo& leaders,
        const MSLeaderDistanceInfo& followers,
        const MSLeaderDistanceInfo& blockers,
        const MSLeaderDistanceInfo& neighLeaders,
        const MSLeaderDistanceInfo& neighFollowers,
        const MSLeaderDistanceInfo& neighBlockers,
        const MSLane& neighLane,
        const std::vector<MSVehicle::LaneQ>& preb,
        MSVehicle** lastBlocked,
        MSVehicle** firstBlocked,
        double& latDist, double& targetDistLat, int& blocked) {
        UNUSED_PARAMETER(laneOffset);
        UNUSED_PARAMETER(alternatives);
        UNUSED_PARAMETER(&leaders);
        UNUSED_PARAMETER(&followers);
        UNUSED_PARAMETER(&blockers);
        UNUSED_PARAMETER(&neighLeaders);
        UNUSED_PARAMETER(&neighFollowers);
        UNUSED_PARAMETER(&neighBlockers);
        UNUSED_PARAMETER(&neighLane);
        UNUSED_PARAMETER(&preb);
        UNUSED_PARAMETER(lastBlocked);
        UNUSED_PARAMETER(firstBlocked);
        UNUSED_PARAMETER(latDist);
        UNUSED_PARAMETER(targetDistLat);
        UNUSED_PARAMETER(blocked);
        throw ProcessError("Method not implemented by model " + toString(myModel));
    }

    /// @brief update expected speeds for each sublane of the current edge
    virtual void updateExpectedSublaneSpeeds(const MSLeaderDistanceInfo& ahead, int sublaneOffset, int laneIndex) {
        UNUSED_PARAMETER(&ahead);
        UNUSED_PARAMETER(sublaneOffset);
        UNUSED_PARAMETER(laneIndex);
        throw ProcessError("Method not implemented by model " + toString(myModel));
    }

    /// @brief decide in which direction to move in case both directions are desirable
    virtual StateAndDist decideDirection(StateAndDist sd1, StateAndDist sd2) const {
        UNUSED_PARAMETER(sd1);
        UNUSED_PARAMETER(sd2);
        throw ProcessError("Method not implemented by model " + toString(myModel));
    }

    virtual void* inform(void* info, MSVehicle* sender) = 0;

    /** @brief Called to adapt the speed in order to allow a lane change.
     *         It uses information on LC-related desired speed-changes from
     *         the call to wantsChange() at the end of the previous simulation step
     *
     * It is guaranteed that min<=wanted<=max, but the implementation needs
     * to make sure that the return value is between min and max.
     *
     * @param min The minimum resulting speed
     * @param wanted The aspired speed of the car following model
     * @param max The maximum resulting speed
     * @param cfModel The model used
     * @return the new speed of the vehicle as proposed by the lane changer
     */
    virtual double patchSpeed(const double min, const double wanted, const double max,
                              const MSCFModel& cfModel) = 0;

    /* @brief called once when the primary lane of the vehicle changes (updates
     * the custom variables of each child implementation */
    virtual void changed() = 0;


    /// @brief return factor for modifying the safety constraints of the car-following model
    virtual double getSafetyFactor() const {
        return 1.0;
    }

    /// @brief return factor for modifying the safety constraints for opposite-diretction overtaking of the car-following model
    virtual double getOppositeSafetyFactor() const {
        return 1.0;
    }

    /// @brief whether the current vehicles shall be debugged
    virtual bool debugVehicle() const {
        return false;
    }

    /// @brief called when a vehicle changes between lanes in opposite directions
    void changedToOpposite();

    void unchanged() {
        if (myLastLaneChangeOffset > 0) {
            myLastLaneChangeOffset += DELTA_T;
        } else if (myLastLaneChangeOffset < 0) {
            myLastLaneChangeOffset -= DELTA_T;
        }
    }

    /** @brief Returns the lane the vehicle's shadow is on during continuous/sublane lane change
     * @return The vehicle's shadow lane
     */
    MSLane* getShadowLane() const {
        return myShadowLane;
    }

    /// @brief return the shadow lane for the given lane
    MSLane* getShadowLane(const MSLane* lane) const;

    /// @brief return the shadow lane for the given lane and lateral offset
    MSLane* getShadowLane(const MSLane* lane, double posLat) const;

    /// @brief set the shadow lane
    void setShadowLane(MSLane* lane) {
        myShadowLane = lane;
    }

    const std::vector<MSLane*>& getShadowFurtherLanes() const {
        return myShadowFurtherLanes;
    }

    const std::vector<double>& getShadowFurtherLanesPosLat() const {
        return myShadowFurtherLanesPosLat;
    }

    /** @brief Returns the lane the vehicle has committed to enter during a sublane lane change
     *  @return The vehicle's target lane.
     */
    MSLane* getTargetLane() const {
        return myTargetLane;
    }

    const std::vector<MSLane*>& getFurtherTargetLanes() const {
        return myFurtherTargetLanes;
    }

    inline SUMOTime getLastLaneChangeOffset() const {
        return myLastLaneChangeOffset;
    }


    /// @brief return whether the vehicle passed the midpoint of a continuous lane change maneuver
    inline bool pastMidpoint() const {
        return myLaneChangeCompletion >= 0.5;
    }

    /// @brief Compute the remaining time until LC completion
    SUMOTime remainingTime() const;

    /// @brief Calculates the maximal time needed to complete a lane change maneuver
    ///        if lcMaxSpeedLatFactor and lcMaxSpeedStanding are set and the vehicle breaks not harder than decel.
    ///        LC when the vehicle starts breaking now. If lcMaxSpeedStanding==0 the completion may be impossible,
    /// @param[in] speed Current longitudinal speed of the changing vehicle.
    /// @param[in] remainingManeuverDist dist which is still to be covered until LC is completed
    /// @param[in] decel Maximal assumed deceleration rate applied during the LC.
    /// @return maximal LC duration (or -1) if it is possible that it can't be completed.
    /// @note 1) For the calculation it is assumed that the vehicle starts breaking with decel (>=0) immediately.
    ///       If lcMaxSpeedStanding==0 the completion may be impossible, and -1 is returned.
    ///       2) In case that no maxSpeedLat is used to control lane changing, this is only called prior to a lane change,
    ///          and the duration is MSGlobals::gLaneChangeDuration.
    virtual double estimateLCDuration(const double speed, const double remainingManeuverDist, const double decel) const;

    /// @brief return true if the vehicle currently performs a lane change maneuver
    inline bool isChangingLanes() const {
        return myLaneChangeCompletion < (1 - NUMERICAL_EPS);
    }

    /// @brief Get the current lane change completion ratio
    inline double getLaneChangeCompletion() const {
        return myLaneChangeCompletion;
    }

    /// @brief return the direction of the current lane change maneuver
    inline int getLaneChangeDirection() const {
        return myLaneChangeDirection;
    }

    /// @brief return the direction in which the current shadow lane lies
    int getShadowDirection() const;

    /// @brief return the angle offset during a continuous change maneuver
    double getAngleOffset() const;

    /// @brief reset the flag whether a vehicle already moved to false
    inline bool alreadyChanged() const {
        return myAlreadyChanged;
    }

    /// @brief reset the flag whether a vehicle already moved to false
    void resetChanged() {
        myAlreadyChanged = false;
    }

    /// @brief start the lane change maneuver and return whether it continues
    bool startLaneChangeManeuver(MSLane* source, MSLane* target, int direction);

    /// @brief Control for resetting the memorized values for LC relevant gaps until the LC output is triggered in the case of continuous LC.
    void memorizeGapsAtLCInit();
    void clearGapsAtLCInit();

    /* @brief continue the lane change maneuver and return whether the midpoint
     * was passed in this step
     */
    bool updateCompletion();

    /* @brief update lane change shadow after the vehicle moved to a new lane */
    void updateShadowLane();

    /* @brief update lane change reservations after the vehicle moved to a new lane
     * @note  The shadow lane should always be updated before updating the target lane. */
    MSLane* updateTargetLane();

    /* @brief Determines the lane which the vehicle intends to enter during its current action step.
     *        targetDir is set to the offset of the returned lane with respect to the vehicle'a current lane. */
    MSLane* determineTargetLane(int& targetDir) const;

    /* @brief finish the lane change maneuver
     */
    void endLaneChangeManeuver(const MSMoveReminder::Notification reason = MSMoveReminder::NOTIFICATION_LANE_CHANGE);

    /* @brief clean up all references to the shadow vehicle
     */
    void cleanupShadowLane();

    /* @brief clean up all references to the vehicle on its target lanes
     */
    void cleanupTargetLane();

    /// @brief reserve space at the end of the lane to avoid dead locks
    virtual void saveBlockerLength(double length) {
        UNUSED_PARAMETER(length);
    }

    void setShadowPartialOccupator(MSLane* lane) {
        myPartiallyOccupatedByShadow.push_back(lane);
    }

    void setNoShadowPartialOccupator(MSLane* lane) {
        myNoPartiallyOccupatedByShadow.push_back(lane);
    }

    /// @brief called once when the vehicles primary lane changes
    void primaryLaneChanged(MSLane* source, MSLane* target, int direction);

    /// @brief called once the vehicle ends a lane change manoeuvre (non-instant)
    void laneChangeOutput(const std::string& tag, MSLane* source, MSLane* target, int direction, double maneuverDist = 0);

    /// @brief whether the current change completes the manoeuvre
    virtual bool sublaneChangeCompleted(const double latDist) const {
        UNUSED_PARAMETER(latDist);
        throw ProcessError("Method not implemented by model " + toString(myModel));
    }

    /// @brief set approach information for the shadow vehicle
    void setShadowApproachingInformation(MSLink* link) const;
    void removeShadowApproachingInformation() const;

    bool isOpposite() const {
        return myAmOpposite;
    }

    double getCommittedSpeed() const {
        return myCommittedSpeed;
    }

    /// @brief return the lateral speed of the current lane change maneuver
    double getSpeedLat() const {
        return mySpeedLat;
    }

    /// @brief return the lateral speed of the current lane change maneuver
    double getAccelerationLat() const {
        return myAccelerationLat;
    }

    /// @brief set the lateral speed and update lateral acceleraton
    void setSpeedLat(double speedLat);

    /// @brief decides the next lateral speed depending on the remaining lane change distance to be covered
    ///        and updates maneuverDist according to lateral safety constraints.
    virtual double computeSpeedLat(double latDist, double& maneuverDist) const;

    /// @brief Returns a deceleration value which is used for the estimation of the duration of a lane change.
    /// @note  Effective only for continuous lane-changing when using attributes myMaxSpeedLatFactor and myMaxSpeedLatStanding. See #3771
    virtual double getAssumedDecelForLaneChangeDuration() const;

    /// @brief try to retrieve the given parameter from this laneChangeModel. Throw exception for unsupported key
    virtual std::string getParameter(const std::string& key) const {
        throw InvalidArgument("Parameter '" + key + "' is not supported for laneChangeModel of type '" + toString(myModel) + "'");
    }

    /// @brief try to set the given parameter for this laneChangeModel. Throw exception for unsupported key
    virtual void setParameter(const std::string& key, const std::string& value) {
        UNUSED_PARAMETER(value);
        throw InvalidArgument("Setting parameter '" + key + "' is not supported for laneChangeModel of type '" + toString(myModel) + "'");
    }


    /// @brief Check for commands issued for the vehicle via TraCI and apply the appropriate state changes
    ///        For the sublane case, this includes setting a new maneuver distance if appropriate.
    void checkTraCICommands();

    static const double NO_NEIGHBOR;

protected:
    virtual bool congested(const MSVehicle* const neighLeader);

    virtual bool predInteraction(const std::pair<MSVehicle*, double>& leader);

    /// @brief whether the influencer cancels the given request
    bool cancelRequest(int state, int laneOffset);


protected:
    /// @brief The vehicle this lane-changer belongs to
    MSVehicle& myVehicle;

    /// @brief The current state of the vehicle
    int myOwnState;
    /// @brief lane changing state from the previous simulation step
    int myPreviousState;
    /// @brief lane changing state from step before the previous simulation step
    int myPreviousState2;

    std::pair<int, int> mySavedStateRight;
    std::pair<int, int> mySavedStateCenter;
    std::pair<int, int> mySavedStateLeft;
    int myCanceledStateRight;
    int myCanceledStateCenter;
    int myCanceledStateLeft;

    /// @brief Cached info on lc-relevant neighboring vehicles
    /// @{
    std::shared_ptr<MSLeaderDistanceInfo> myLeftFollowers;
    std::shared_ptr<MSLeaderDistanceInfo> myLeftLeaders;
    std::shared_ptr<MSLeaderDistanceInfo> myRightFollowers;
    std::shared_ptr<MSLeaderDistanceInfo> myRightLeaders;
    /// @}

    /// @brief the current lateral speed
    double mySpeedLat;

    /// @brief the current lateral acceleration
    double myAccelerationLat;

    /// @brief the speed when committing to a change maneuver
    double myCommittedSpeed;

    /// @brief progress of the lane change maneuver 0:started, 1:complete
    double myLaneChangeCompletion;

    /// @brief direction of the lane change maneuver -1 means right, 1 means left
    int myLaneChangeDirection;

    /// @brief whether the vehicle has already moved this step
    bool myAlreadyChanged;

    /// @brief A lane that is partially occupied by the front of the vehicle but that is not the primary lane
    MSLane* myShadowLane;
    /* @brief Lanes that are partially (laterally) occupied by the back of the
     * vehicle (analogue to MSVehicle::myFurtherLanes) */
    std::vector<MSLane*> myShadowFurtherLanes;
    std::vector<double> myShadowFurtherLanesPosLat;


    /// @brief The target lane for the vehicle's current maneuver
    /// @note  This is used by the sublane model to register the vehicle at lanes,
    ///        it will reach within the current action step, so vehicles on that lane
    ///        may react to the started lc-maneuver during the car-following process.
    ///        If the shadow lane is the same as the lc maneuver target, myTargetLane is
    ///        set to nullptr.
    ///        The current shadow lanes and further lanes should always be updated before updating the target lane.
    MSLane* myTargetLane;

    /* @brief Further upstream lanes that are affected by the vehicle's maneuver (analogue to MSVehicle::myFurtherLanes)
     * @note  If myTargetLane==nullptr, we may assume myFurtherTargetLanes.size()==0, otherwise we have
     *        myFurtherTargetLanes.size() == myVehicle.getFurtherLanes.size()
     *        Here it may occur that an element myFurtherTargetLanes[i]==nullptr if myFurtherLanes[i] has
     *        no parallel lane in the change direction.
     *  */
    std::vector<MSLane*> myFurtherTargetLanes;

    /// @brief The vehicle's car following model
    const MSCFModel& myCarFollowModel;

    /// @brief the type of this model
    const LaneChangeModel myModel;

    /// @brief list of lanes where the shadow vehicle is partial occupator
    std::vector<MSLane*> myPartiallyOccupatedByShadow;

    /* @brief list of lanes where there is no shadow vehicle partial occupator
     * (when changing to a lane that has no predecessor) */
    std::vector<MSLane*> myNoPartiallyOccupatedByShadow;

    /// @brief the minimum lateral gaps to other vehicles that were found when last changing to the left and right
    double myLastLateralGapLeft;
    double myLastLateralGapRight;

    /// @brief the actual minimum longitudinal distances to vehicles on the target lane
    double myLastLeaderGap;
    double myLastFollowerGap;
    /// @brief the minimum longitudinal distances to vehicles on the target lane that would be necessary for stringent security
    double myLastLeaderSecureGap;
    double myLastFollowerSecureGap;
    /// @brief acutal and secure distance to closest leader vehicle on the original when performing lane change
    double myLastOrigLeaderGap;
    double myLastOrigLeaderSecureGap;
    /// @brief speeds of surrounding vehicles at the time of lane change
    double myLastLeaderSpeed;
    double myLastFollowerSpeed;
    double myLastOrigLeaderSpeed;

    /// @brief Flag to prevent resetting the memorized values for LC relevant gaps until the LC output is triggered
    ///        in the case of continuous LC.
    bool myDontResetLCGaps;

    // @brief the maximum lateral speed when standing
    double myMaxSpeedLatStanding;
    // @brief the factor of maximum lateral speed to longitudinal speed
    double myMaxSpeedLatFactor;
    // @brief factor for lane keeping imperfection
    double mySigma;

    /* @brief to be called by derived classes in their changed() method.
     * If dir=0 is given, the current value remains unchanged */
    void initLastLaneChangeOffset(int dir);

    /// @brief whether overtaking on the right is permitted
    static bool myAllowOvertakingRight;

    /// @brief whether to record lane-changing
    static bool myLCOutput;
    static bool myLCStartedOutput;
    static bool myLCEndedOutput;
    static bool myLCXYOutput;


private:
    /* @brief information how long ago the vehicle has performed a lane-change,
     * sign indicates direction of the last change
     */
    SUMOTime myLastLaneChangeOffset;

    /// @brief links which are approached by the shadow vehicle
    mutable std::vector<MSLink*> myApproachedByShadow;

    /// @brief whether the vehicle is driving in the opposite direction
    bool myAmOpposite;

    /// @brief The complete lateral distance the vehicle wants to travel to finish its maneuver
    ///        Only used by sublane model, currently.
    double myManeuverDist;

    /// @brief Maneuver distance from the previous simulation step
    double myPreviousManeuverDist;


private:
    /// @brief Invalidated assignment operator
    MSAbstractLaneChangeModel& operator=(const MSAbstractLaneChangeModel& s);
};
