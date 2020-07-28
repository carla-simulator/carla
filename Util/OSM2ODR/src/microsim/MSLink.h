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
/// @file    MSLink.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A connnection between lanes
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class MSJunction;
class MSVehicle;
class MSPerson;
class OutputDevice;
class MSTrafficLightLogic;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLinks
 * @brief A connnection between lanes
 *
 * A link is basically a connection between two lanes, stored within the
 *  originating (the one that is being left) lane and pointing to the
 *  approached lane. When using inner-junction simulation, additionally
 *  a "via-lane" is stored, the one that is used to cross the junction
 *  and which represents the links shape.
 *
 * Because right-of-way rules are controlled by the junctions, the link
 *  stores the information about where to write information about approaching
 *  vehicles (the "request") and where to get the information whether the
 *  vehicle really may drive (the "respond").
 *
 * Because a link is a connection over a junction, it basically also has a
 *  length. This length is needed to assure that vehicles have the correct halting
 *  distance before approaching the link. In the case of using internal lanes,
 *  the link's length is 0.
 */
class MSLink {
public:

    // distance to link in m below which adaptation for zipper-merging should take place
    static const double ZIPPER_ADAPT_DIST;

    struct LinkLeader {
        LinkLeader(MSVehicle* _veh, double _gap, double _distToCrossing, bool _fromLeft = true) :
            vehAndGap(std::make_pair(_veh, _gap)),
            distToCrossing(_distToCrossing),
            fromLeft(_fromLeft) {
        }

        std::pair<MSVehicle*, double> vehAndGap;
        double distToCrossing;
        bool fromLeft;
    };

    typedef std::vector<LinkLeader> LinkLeaders;

    /** @struct ApproachingVehicleInformation
     * @brief A structure holding the information about vehicles approaching a link
     */
    struct ApproachingVehicleInformation {
        /** @brief Constructor
         * @param[in] waitingTime The time during which the vehicle is waiting at this link
         *   this needs to be placed here because MSVehicle::myWaitingTime is updated in between
         *   calls to opened() causing order dependencies
         **/
        ApproachingVehicleInformation(const SUMOTime _arrivalTime, const SUMOTime _leavingTime,
                                      const double _arrivalSpeed, const double _leaveSpeed,
                                      const bool _willPass,
                                      const SUMOTime _arrivalTimeBraking,
                                      const double _arrivalSpeedBraking,
                                      const SUMOTime _waitingTime,
                                      const double _dist,
                                      const double _speed
                                     ) :
            arrivalTime(_arrivalTime), leavingTime(_leavingTime),
            arrivalSpeed(_arrivalSpeed), leaveSpeed(_leaveSpeed),
            willPass(_willPass),
            arrivalTimeBraking(_arrivalTimeBraking),
            arrivalSpeedBraking(_arrivalSpeedBraking),
            waitingTime(_waitingTime),
            dist(_dist),
            speed(_speed) {
        }

        /// @brief The time the vehicle's front arrives at the link
        const SUMOTime arrivalTime;
        /// @brief The estimated time at which the vehicle leaves the link
        const SUMOTime leavingTime;
        /// @brief The estimated speed with which the vehicle arrives at the link (for headway computation)
        const double arrivalSpeed;
        /// @brief The estimated speed with which the vehicle leaves the link (for headway computation)
        const double leaveSpeed;
        /// @brief Whether the vehicle wants to pass the link (@todo: check semantics)
        const bool willPass;
        /// @brief The time the vehicle's front arrives at the link if it starts braking
        const SUMOTime arrivalTimeBraking;
        /// @brief The estimated speed with which the vehicle arrives at the link if it starts braking(for headway computation)
        const double arrivalSpeedBraking;
        /// @brief The waiting duration at the current link
        const SUMOTime waitingTime;
        /// @brief The distance up to the current link
        const double dist;
        /// @brief The current speed
        const double speed;

    private:
        /// invalidated assignment operator
        ApproachingVehicleInformation& operator=(const ApproachingVehicleInformation& s);

    };

    typedef std::map<const SUMOVehicle*, const ApproachingVehicleInformation, ComparatorNumericalIdLess> ApproachInfos;
    typedef std::vector<const SUMOVehicle*> BlockingFoes;

    /** @brief Constructor for simulation which uses internal lanes
     *
     * @param[in] succLane The lane approached by this link
     * @param[in] via The lane to use within the junction
     * @param[in] dir The direction of this link
     * @param[in] state The state of this link
     * @param[in] length The length of this link
     */
    MSLink(MSLane* predLane, MSLane* succLane, MSLane* via, LinkDirection dir, LinkState state, double length, double foeVisibilityDistance, bool keepClear, MSTrafficLightLogic* logic, int tlLinkIdx);


    /// @brief Destructor
    ~MSLink();


    /** @brief Sets the request information
     *
     * Because traffic lights and junction logics are loaded after links,
     *  we have to assign the information about the right-of-way
     *  requests and responses after the initialisation.
     * @todo Unsecure!
     */
    void setRequestInformation(int index, bool hasFoes, bool isCont,
                               const std::vector<MSLink*>& foeLinks, const std::vector<MSLane*>& foeLanes,
                               MSLane* internalLaneBefore = 0);

    /// @brief add walkingarea as foe (when entering the junction)
    void addWalkingAreaFoe(const MSLane* lane) {
        myWalkingAreaFoe = lane;
    }

    /// @brief add walkingarea as foe (when leaving the junction)
    void addWalkingAreaFoeExit(const MSLane* lane) {
        myWalkingAreaFoeExit = lane;
    }

    /// @brief get walkingarea as foes
    const MSLane* getWalkingAreaFoe() {
        return myWalkingAreaFoe;
    }
    const MSLane* getWalkingAreaFoeExit() {
        return myWalkingAreaFoeExit;
    }

    /** @brief Sets the information about an approaching vehicle
     *
     * The information is stored in myApproachingVehicles.
     */
    void setApproaching(const SUMOVehicle* approaching, const SUMOTime arrivalTime,
                        const double arrivalSpeed, const double leaveSpeed, const bool setRequest,
                        const SUMOTime arrivalTimeBraking, const double arrivalSpeedBraking,
                        const SUMOTime waitingTime, double dist);

    /** @brief Sets the information about an approaching vehicle */
    void setApproaching(const SUMOVehicle* approaching, ApproachingVehicleInformation ai);

    /// @brief removes the vehicle from myApproachingVehicles
    void removeApproaching(const SUMOVehicle* veh);

    void addBlockedLink(MSLink* link);

    /* @brief return information about this vehicle if it is registered as
     * approaching (dummy values otherwise)
     * @note used for visualisation of link items */
    ApproachingVehicleInformation getApproaching(const SUMOVehicle* veh) const;

    /// @brief return all approaching vehicles
    const ApproachInfos& getApproaching() const {
        return myApproachingVehicles;
    }

    /** @brief Remove all approaching vehicles before quick-loading state */
    void clearState(); 

    /** @brief Returns the information whether the link may be passed
     *
     * Valid after the junctions have set their reponds
     *
     * @param[in] collectFoes If a vector is passed, all blocking foes are collected and inserted into this vector
     * @return Whether this link may be passed.
     */
    bool opened(SUMOTime arrivalTime, double arrivalSpeed, double leaveSpeed, double vehicleLength,
                double impatience, double decel, SUMOTime waitingTime,
                double posLat = 0,
                BlockingFoes* collectFoes = nullptr,
                bool ignoreRed = false,
                const SUMOVehicle* ego = nullptr) const;

    /** @brief Returns the information whether this link is blocked
     * Valid after the vehicles have set their requests
     * @param[in] arrivalTime The arrivalTime of the vehicle who checks for an approaching foe
     * @param[in] leaveTime The leaveTime of the vehicle who checks for an approaching foe
     * @param[in] arrivalSpeed The speed with which the checking vehicle plans to arrive at the link
     * @param[in] leaveSpeed The speed with which the checking vehicle plans to leave the link
     * @param[in] sameTargetLane Whether the link that calls this method has the same target lane as this link
     * @param[in] impatience The impatience of the checking vehicle
     * @param[in] decel The maximum deceleration of the checking vehicle
     * @param[in] waitingTime The waiting time of the checking vehicle
     * @param[in] collectFoes If a vector is passed the return value is always False, instead all blocking foes are collected and inserted into this vector
     * @return Whether this link is blocked
     * @note Since this needs to be called without a SUMOVehicle (TraCI), we cannot simply pass the checking vehicle itself
     **/
    bool blockedAtTime(SUMOTime arrivalTime, SUMOTime leaveTime, double arrivalSpeed, double leaveSpeed,
                       bool sameTargetLane, double impatience, double decel, SUMOTime waitingTime,
                       BlockingFoes* collectFoes = nullptr, const SUMOVehicle* ego = nullptr) const;


    bool isBlockingAnyone() const {
        return myApproachingVehicles.size() != 0;
    }

    bool willHaveBlockedFoe() const;



    /** @brief Returns the information whether a vehicle is approaching on one of the link's foe streams
     *
     * Valid after the vehicles have set their requests
     * @param[in] arrivalTime The arrivalTime of the vehicle who checks for an approaching foe
     * @param[in] leaveTime The leaveTime of the vehicle who checks for an approaching foe
     * @param[in] speed The speed with which the checking vehicle plans to leave the link
     * @param[in] decel The maximum deceleration of the checking vehicle
     * @return Whether a foe of this link is approaching
     */
    bool hasApproachingFoe(SUMOTime arrivalTime, SUMOTime leaveTime, double speed, double decel) const;

    /** @brief get the foe vehicle that is closest to the intersection or nullptr along with the foe link
     * This function is used for finding circular deadlock at right_before_left junctions
     * @param[in] wrapAround The link on which the ego vehicle wants to enter the junction
    */
    std::pair<const SUMOVehicle*, const MSLink*>  getFirstApproachingFoe(const MSLink* wrapAround) const;

    MSJunction* getJunction() const {
        return myJunction;
    }


    /** @brief Returns the current state of the link
     *
     * @return The current state of this link
     */
    LinkState getState() const {
        return myState;
    }


    /** @brief Returns the off-state for the link
     *
     * @return The current state of this link
     */
    LinkState getOffState() const {
        return myOffState;
    }


    //@brief Returns the time of the last state change
    inline SUMOTime getLastStateChange() const {
        return myLastStateChange;
    }


    /** @brief Returns the direction the vehicle passing this link take
     *
     * @return The direction of this link
     */
    LinkDirection getDirection() const;


    /** @brief Sets the current tl-state
     *
     * @param[in] state The current state of the link
     * @param[in] t The time of the state change
     */
    void setTLState(LinkState state, SUMOTime t);


    /** @brief Returns the connected lane
     *
     * @return The lane approached by this link
     */
    MSLane* getLane() const;


    /** @brief Returns the respond index (for visualization)
     *
     * @return The respond index for this link
     */
    inline int getIndex() const {
        return myIndex;
    }

    /** @brief Returns the TLS index */
    inline int getTLIndex() const {
        return myTLIndex;
    }

    /** @brief Returns the TLS index */
    inline const MSTrafficLightLogic* getTLLogic() const {
        return myLogic;
    }

    /** @brief Returns whether this link is a major link
     * @return Whether the link has a large priority
     */
    inline bool havePriority() const {
        return myState >= 'A' && myState <= 'Z';
    }

    /** @brief Returns whether this link is blocked by a red (or redyellow) traffic light
     * @return Whether the link has a red light
     */
    inline bool haveRed() const {
        return myState == LINKSTATE_TL_RED || myState == LINKSTATE_TL_REDYELLOW;
    }

    inline bool haveYellow() const {
        return myState == LINKSTATE_TL_YELLOW_MINOR || myState == LINKSTATE_TL_YELLOW_MAJOR;
    }

    inline bool haveGreen() const {
        return myState == LINKSTATE_TL_GREEN_MAJOR || myState == LINKSTATE_TL_GREEN_MINOR;
    }

    inline bool isTLSControlled() const {
        return myLogic != 0;
    }

    /** @brief Returns the length of this link
     *
     * @return The length of this link
     */
    double getLength() const {
        return myLength;
    }


    /** @brief Returns the distance on the approaching lane from which an
     *         approaching vehicle is able to see all relevant foes and
     *         may accelerate if the link is minor and no foe is approaching.
     *
     * @return The foe-visibility-distance
     */
    double getFoeVisibilityDistance() const {
        return myFoeVisibilityDistance;
    }


    /** @brief Returns whether this link belongs to a junction where more than one edge is incoming
     *
     * @return Whether any foe links exist
     */
    bool hasFoes() const {
        return myHasFoes;
    }

    // @brief return whether the vehicle may continute past this link to wait within the intersection
    bool isCont() const;


    /// @brief whether the junction after this link must be kept clear
    bool keepClear() const {
        return myKeepClear;
    }

    /// @brief whether this is a link past an internal junction which currently has priority
    bool lastWasContMajor() const;

    /// @brief whether this is a link past an internal junction which currently has green major
    bool lastWasContMajorGreen() const;

    /** @brief Returns the cumulative length of all internal lanes after this link
     *  @return sum of the lengths of all internal lanes following this link
     */
    double getInternalLengthsAfter() const;

    /** @brief Returns the cumulative length of all internal lanes before this link
     *  @return sum of the lengths of all internal lanes before this link
     */
    double getInternalLengthsBefore() const;

    /** @brief Returns the sum of the lengths along internal lanes following this link
     *         to the crossing with the given foe lane, if the lane is no foe
     *         lane to any of the internal lanes, INVALID_DOUBLE is returned.
     *  @see getLengthBeforeCrossing()
     */
    double getLengthsBeforeCrossing(const MSLane* foeLane) const;


    /** @brief Returns the internal length from the beginning of the link's internal lane before
     *         to the crossing with the given foe lane if applicable, if the lane is no foe
     *         lane to the link, INVALID_DOUBLE is returned.
     *  @see getLengthsBeforeCrossing()
     */
    double getLengthBeforeCrossing(const MSLane* foeLane) const;


    /** @brief Returns the following inner lane
     *
     * @return The inner lane to use to cross the junction
     */
    MSLane* getViaLane() const;

    /** @brief Returns all potential link leaders (vehicles on foeLanes)
     * Valid during the planMove() phase
     * @param[in] ego The ego vehicle that is looking for leaders
     * @param[in] dist The distance of the vehicle who is asking about the leader to this link
     * @param[out] blocking Return blocking pedestrians if a vector is given
     * @param[in] isShadowLink whether this link is a shadowLink for ego
     * @return The all vehicles on foeLanes and their (virtual) distances to the asking vehicle
     */
    LinkLeaders getLeaderInfo(const MSVehicle* ego, double dist, std::vector<const MSPerson*>* collectBlockers = 0, bool isShadowLink = false) const;

    /// @brief check for persons on walkingarea in the path of ego vehicle
    void checkWalkingAreaFoe(const MSVehicle* ego, const MSLane* foeLane, std::vector<const MSPerson*>* collectBlockers, LinkLeaders& result) const;

    /// @brief return the speed at which ego vehicle must approach the zipper link
    double getZipperSpeed(const MSVehicle* ego, const double dist, double vSafe,
                          SUMOTime arrivalTime,
                          BlockingFoes* collectFoes) const;

    /// @brief return the via lane if it exists and the lane otherwise
    MSLane* getViaLaneOrLane() const;

    /// @brief return the internalLaneBefore if it exists and the laneBefore otherwise
    const MSLane* getLaneBefore() const;

    /// @brief return myInternalLaneBefore (always 0 when compiled without internal lanes)
    const MSLane* getInternalLaneBefore() const;

    /// @brief return the expected time at which the given vehicle will clear the link
    SUMOTime getLeaveTime(const SUMOTime arrivalTime, const double arrivalSpeed, const double leaveSpeed, const double vehicleLength) const;

    /// @brief write information about all approaching vehicles to the given output device
    void writeApproaching(OutputDevice& od, const std::string fromLaneID) const;

    /// @brief return the link that is parallel to this lane or 0
    MSLink* getParallelLink(int direction) const;

    /// @brief return whether the fromLane of this link is an internal lane
    bool fromInternalLane() const;

    /// @brief return whether the toLane of this link is an internal lane and fromLane is a normal lane
    bool isEntryLink() const;

    /// @brief return whether this link enters the conflict area (not a continuation link)
    bool isConflictEntryLink() const;

    /// @brief return whether the fromLane of this link is an internal lane and toLane is a normal lane
    bool isExitLink() const;

    /// @brief return whether the fromLane of this link is an internal lane and its incoming lane is also an internal lane
    bool isExitLinkAfterInternalJunction() const;

    /// @brief returns the corresponding exit link for entryLinks to a junction.
    MSLink* getCorrespondingExitLink() const;

    /// @brief return whether the fromLane and the toLane of this link are internal lanes
    bool isInternalJunctionLink() const;

    /** @brief Returns the time penalty for passing a tls-controlled link (meso) */
    SUMOTime getMesoTLSPenalty() const {
        return myMesoTLSPenalty;
    }

    /** @brief Returns the average proportion of green time to cycle time */
    double getGreenFraction() const {
        return myGreenFraction;
    }

    /** @brief Sets the time penalty for passing a tls-controlled link (meso) */
    void setMesoTLSPenalty(const SUMOTime penalty) {
        myMesoTLSPenalty = penalty;
    }

    /** @brief Sets the green fraction for passing a tls-controlled link (meso) */
    void setGreenFraction(const double fraction) {
        myGreenFraction = fraction;
    }

    const std::vector<const MSLane*>& getFoeLanes() const {
        return myFoeLanes;
    }

    const std::vector<std::pair<double, double> >& getLengthsBehindCrossing() const {
        return myLengthsBehindCrossing;
    }

    const std::vector<MSLink*>& getFoeLinks() const {
        return myFoeLinks;
    }

    /// @brief initialize parallel links (to be called after all links are loaded)
    void initParallelLinks();

    /// @brief return lateral shift that must be applied when passing this link
    double getLateralShift() {
        return myLateralShift;
    }

    /// @brief get string description for this link
    std::string  getDescription() const;

private:
    /// @brief return whether the given vehicles may NOT merge safely
    static inline bool unsafeMergeSpeeds(double leaderSpeed, double followerSpeed, double leaderDecel, double followerDecel) {
        // XXX mismatch between continuous an discrete deceleration
        return (leaderSpeed * leaderSpeed / leaderDecel) <= (followerSpeed * followerSpeed / followerDecel);
    }

    /// @brief whether follower could stay behind leader (possibly by braking)
    static bool couldBrakeForLeader(double followDist, double leaderDist, const MSVehicle* follow, const MSVehicle* leader);

    MSLink* computeParallelLink(int direction);

    bool blockedByFoe(const SUMOVehicle* veh, const ApproachingVehicleInformation& avi,
                      SUMOTime arrivalTime, SUMOTime leaveTime, double arrivalSpeed, double leaveSpeed,
                      bool sameTargetLane, double impatience, double decel, SUMOTime waitingTime,
                      const SUMOVehicle* ego) const;

    /// @brief figure out whether the cont status remains in effect when switching off the tls
    bool checkContOff() const;

    /// @brief check if the lane intersects with a foe cont-lane
    bool contIntersect(const MSLane* lane, const MSLane* foe);

private:
    /// @brief The lane behind the junction approached by this link
    MSLane* myLane;

    /// @brief The lane approaching this link
    MSLane* myLaneBefore;

    ApproachInfos myApproachingVehicles;
    std::set<MSLink*> myBlockedFoeLinks;

    /// @brief The position within this respond
    int myIndex;

    /// @brief the traffic light index
    const int myTLIndex;

    /// @brief the controlling logic or 0
    const MSTrafficLightLogic* myLogic;

    /// @brief The state of the link
    LinkState myState;
    /// @brief The state of the link when switching of traffic light control
    const LinkState myOffState;

    /// @brief The time of the last state change
    SUMOTime myLastStateChange;

    /// @brief An abstract (hopefully human readable) definition of the link's direction
    LinkDirection myDirection;

    /// @brief The length of the link
    /// @note This is not equal to the result of getInternalLengthsAfter for links with more than one internal lane.
    double myLength;

    /// @brief distance from which an approaching vehicle is able to
    ///        see all relevant foes and may accelerate if the link is minor
    ///        and no foe is approaching. Defaults to 4.5m.
    double myFoeVisibilityDistance;

    /// @brief Whether any foe links exist
    bool myHasFoes;

    // @brief whether vehicles may continue past this link to wait within the intersection
    bool myAmCont;
    // @brief whether vehicles may continue past this link to wait within the intersection after switching of the traffic light at this intersection
    bool myAmContOff;

    // @brief whether vehicles must keep the intersection clear if there is a downstream jam
    bool myKeepClear;

    /// @brief The following junction-internal lane if used
    MSLane* const myInternalLane;

    /* @brief The preceding junction-internal lane, only used at
     * - exit links (from internal lane to normal lane)
     * - internal junction links (from internal lane to internal lane)
     */
    const MSLane* myInternalLaneBefore;

    /// @brief penalty time at tls for mesoscopic simulation
    SUMOTime myMesoTLSPenalty;
    /// @brief green fraction at tls for mesoscopic simulation
    double myGreenFraction;

    /// @brief lateral shift to be applied when passing this link
    double myLateralShift;

    /* @brief lengths after the crossing point with foeLane
     * (lengthOnThis, lengthOnFoe)
     * (index corresponds to myFoeLanes)
     * empty vector for entry links
     * */
    std::vector<std::pair<double, double> > myLengthsBehindCrossing;

    // TODO: documentation
    std::vector<MSLink*> myFoeLinks;
    std::vector<const MSLane*> myFoeLanes;

    /// @brief walkingArea that must be checked when entering the intersection
    const MSLane* myWalkingAreaFoe;
    /// @brief walkingArea that must be checked when leaving the intersection
    const MSLane* myWalkingAreaFoeExit;

    /// @brief whether on of myFoeLanes is a crossing
    bool myHavePedestrianCrossingFoe;

    /* @brief Links with the same origin lane and the same destination edge that may
       be in conflict for sublane simulation */
    std::vector<MSLink*> mySublaneFoeLinks;
    /* @brief Links with the same origin lane and different destination edge that may
       be in conflict for sublane simulation */
    std::vector<MSLink*> mySublaneFoeLinks2;

    /* @brief Internal Lanes with the same origin lane and the same destination edge that may
       be in conflict for sublane simulation */
    std::vector<MSLane*> mySublaneFoeLanes;

    static const SUMOTime myLookaheadTime;
    static const SUMOTime myLookaheadTimeZipper;

    MSLink* myParallelRight;
    MSLink* myParallelLeft;

    /// @brief the junction to which this link belongs
    MSJunction* myJunction;

    /// invalidated copy constructor
    MSLink(const MSLink& s);

    /// invalidated assignment operator
    MSLink& operator=(const MSLink& s);

};
