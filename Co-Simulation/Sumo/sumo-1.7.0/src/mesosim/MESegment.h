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
/// @file    MESegment.h
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// A single mesoscopic segment (cell)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <utils/common/Named.h>
#include <utils/common/SUMOTime.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLink;
class MSMoveReminder;
class MSDetectorFileOutput;
class MSVehicleControl;
class MEVehicle;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MESegment
 * @brief A single mesoscopic segment (cell)
 */
class MESegment : public Named {
private:
    class Queue {
    public:
        Queue(const SVCPermissions permissions) : myPermissions(permissions) {}
        inline int size() const {
            return (int)myVehicles.size();
        }
        inline const std::vector<MEVehicle*>& getVehicles() const {
            return myVehicles;
        }
        MEVehicle* remove(MEVehicle* v);
        inline std::vector<MEVehicle*>& getModifiableVehicles() {
            return myVehicles;
        }
        inline double getOccupancy() const {
            return myOccupancy;
        }
        inline void setOccupancy(const double occ) {
            myOccupancy = occ;
        }
        inline bool allows(SUMOVehicleClass vclass) const {
            return (myPermissions & vclass) == vclass;
        }

        /// @brief return the next time at which a vehicle may enter this queue
        inline SUMOTime getEntryBlockTime() const {
            return myEntryBlockTime;
        }

        /// @brief set the next time at which a vehicle may enter this queue
        inline void setEntryBlockTime(SUMOTime entryBlockTime) {
            myEntryBlockTime = entryBlockTime;
        }

        inline SUMOTime getBlockTime() const {
            return myBlockTime;
        }
        inline void setBlockTime(SUMOTime t) {
            myBlockTime = t;
        }

    private:
        /// The vClass permissions for this queue
        const SVCPermissions myPermissions;

        std::vector<MEVehicle*> myVehicles;

        /// @brief The occupied space (in m) in the queue
        double myOccupancy = 0.;

        /// @brief The block time for vehicles who wish to enter this queue
        SUMOTime myEntryBlockTime = SUMOTime_MIN;

        /// @brief The block time
        SUMOTime myBlockTime = -1;

    private:
        /// @brief Invalidated assignment operator.
        Queue& operator=(const Queue&) = delete;
    };

public:
    /** @brief constructor
     * @param[in] id The id of this segment (currently: "<EDGEID>:<SEGMENTNO>")
     * @param[in] parent The edge this segment is located within
     * @param[in] next The following segment (belonging to the same edge)
     * @param[in] length The segment's length
     * @param[in] speed The speed allowed on this segment
     * @param[in] idx The running index of this segment within the segment's edge
     * @param[in] tauff The factor for free-free headway time
     * @param[in] taufj The factor for free-jam headway time
     * @param[in] taujf The factor for jam-free headway time
     * @param[in] taujj The factor for jam-jam headway time
     * @param[in] jamThresh percentage of occupied space before the segment is jammed
     * @param[in] multiQueue whether to install multiple queues on this segment
     * @param[in] junctionControl whether junction control is enabled on this segment
     * @todo recheck the id; using a ':' as divider is not really nice
     */
    MESegment(const std::string& id,
              const MSEdge& parent, MESegment* next,
              const double length, const double speed,
              const int idx,
              const SUMOTime tauff, const SUMOTime taufj,
              const SUMOTime taujf, const SUMOTime taujj,
              const double jamThresh,
              const bool multiQueue,
              const bool junctionControl);


    /// @name Measure collection
    /// @{

    /** @brief Adds a data collector for a detector to this segment
     *
     * @param[in] data The data collector to add
     */
    void addDetector(MSMoveReminder* data);

    /** @brief Removes a data collector for a detector from this segment
     *
     * @param[in] data The data collector to remove
     */
    void removeDetector(MSMoveReminder* data);

    /** @brief Updates data of a detector for all vehicle queues
     *
     * @param[in] data The detector data to update
     */
    void prepareDetectorForWriting(MSMoveReminder& data);
    /// @}

    /** @brief Returns whether the given vehicle would still fit into the segment
     *
     * @param[in] veh The vehicle to check space for
     * @param[in] entryTime The time at which the vehicle wants to enter
     * @param[out] qIdx The index of the queue the vehicle should choose
     * @param[in] init whether the check is done at insertion time
     * @return the earliest time a vehicle may be added to this segment
     */
    SUMOTime hasSpaceFor(const MEVehicle* const veh, const SUMOTime entryTime, int& qIdx, const bool init = false) const;

    /** @brief Inserts (emits) vehicle into the segment
     *
     * @param[in] veh The vehicle to emit
     * @param[in] time The emission time
     * @return Whether the emission was successful
     */
    bool initialise(MEVehicle* veh, SUMOTime time);

    /** @brief Returns the total number of cars on the segment
     *
     * @return the total number of cars on the segment
     */
    inline int getCarNumber() const {
        return myNumVehicles;
    }

    /// @brief return the number of queues
    inline int numQueues() const {
        return (int)myQueues.size();
    }
    /** @brief Returns the cars in the queue with the given index for visualization
     * @return the Queue (XXX not thread-safe!)
     */
    inline const std::vector<MEVehicle*>& getQueue(int index) const {
        assert(index < (int)myQueues.size());
        return myQueues[index].getVehicles();
    }

    /** @brief Returns the running index of the segment in the edge (0 is the most upstream).
     *
     * @return the running index of the segment in the edge
     */
    inline int getIndex() const {
        return myIndex;
    }

    /** @brief Returns the following segment on the same edge (0 if it is the last).
     *
     * @return the following segment on the same edge (0 if it is the last)
     */
    inline MESegment* getNextSegment() const {
        return myNextSegment;
    }

    /** @brief Returns the length of the segment in meters.
     *
     * @return the length of the segment
     */
    inline double getLength() const {
        return myLength;
    }

    /** @brief Returns the occupany of the segment (the sum of the vehicle lengths + minGaps)
     *
     * @return the occupany of the segment in meters
     */
    inline double getBruttoOccupancy() const {
        double occ = 0.;
        for (const Queue& q : myQueues) {
            occ += q.getOccupancy();
        }
        return occ;
    }

    /** @brief Returns the relative occupany of the segment (percentage of road used))
     * @return the occupany of the segment in percent
     */
    inline double getRelativeOccupancy() const {
        return getBruttoOccupancy() / myCapacity;
    }

    /** @brief Returns the relative occupany of the segment (percentage of road used))
     * at which the segment is considered jammed
     * @return the jam treshold of the segment in percent
     */
    inline double getRelativeJamThreshold() const {
        return myJamThreshold / myCapacity;
    }

    /** @brief Returns the average speed of vehicles on the segment in meters per second.
     * If there is no vehicle on the segment it returns the maximum allowed speed
     * @param[in] useCache whether to use a cached value if available
     * @note this value is cached in myMeanSpeed. Since caching only takes place
     * once every simstep there is a potential for side-influences (i.e. GUI calls to
     * this method, ...) For that reason the simulation logic doesn't use the cache.
     * This shouldn't matter much for speed since it is only used during
     * initializsation of vehicles onto the segment.
     * @return the average speed on the segment
     */
    double getMeanSpeed(bool useCache) const;

    /// @brief wrapper to satisfy the FunctionBinding signature
    inline double getMeanSpeed() const {
        return getMeanSpeed(true);
    }


    void writeVehicles(OutputDevice& of) const;

    /** @brief Removes the given car from the edge's que
     *
     * @param[in] v The vehicle to remove
     * @param[in] leaveTime The time at which the vehicle is leaving the que
     * @param[in] reason The reason for removing to send to reminders
     * @return The next first vehicle to add to the net's que
     */
    MEVehicle* removeCar(MEVehicle* v, SUMOTime leaveTime, const MSMoveReminder::Notification reason);

    /** @brief Returns the link the given car will use when passing the next junction
     *
     * This returns non-zero values only for the last segment and only
     *  if junction control is enabled.
     *
     * @param[in] veh The vehicle in question
     * @param[in] tlsPenalty Whether the link should be returned for computing tlsPenalty
     * @return The link to use or 0 without junction control
     */
    MSLink* getLink(const MEVehicle* veh, bool tlsPenalty = false) const;

    /** @brief Returns whether the vehicle may use the next link
     *
     * In case of disabled junction control it returns always true.
     *
     * @param[in] veh The vehicle in question
     * @return Whether it may pass to the next segment
     */
    bool isOpen(const MEVehicle* veh) const;

    /** @brief Removes the vehicle from the segment, adapting its parameters
     *
     * @param[in] veh The vehicle in question
     * @param[in] next The subsequent segment for delay calculation
     * @param[in] time the leave time
     * @todo Isn't always time == veh->getEventTime?
     */
    void send(MEVehicle* veh, MESegment* const next, const int nextQIdx, SUMOTime time, const MSMoveReminder::Notification reason);

    /** @brief Adds the vehicle to the segment, adapting its parameters
     *
     * @param[in] veh The vehicle in question
     * @param[in] time the leave time
     * @param[in] isDepart whether the vehicle just departed
     * @todo Isn't always time == veh->getEventTime?
     */
    void receive(MEVehicle* veh, const int qIdx, SUMOTime time, const bool isDepart = false, const bool isTeleport = false, const bool newEdge = false);


    /** @brief tries to remove any car from this segment
     *
     * @param[in] currentTime the current time
     * @return Whether vaporization was successful
     * @note: cars removed via this method do NOT count as arrivals */
    bool vaporizeAnyCar(SUMOTime currentTime, const MSDetectorFileOutput* filter);

    /** @brief Returns the edge this segment belongs to
     * @return the edge this segment belongs to
     */
    inline const MSEdge& getEdge() const {
        return myEdge;
    }


    /** @brief reset mySpeed and patch the speed of
     * all vehicles in it. Also set/recompute myJamThreshold
     * @param[in] jamThresh follows the semantic of option meso-jam-threshold
     */
    void setSpeed(double newSpeed, SUMOTime currentTime, double jamThresh = DO_NOT_PATCH_JAM_THRESHOLD);

    /** @brief Returns the (planned) time at which the next vehicle leaves this segment
     * @return The time the vehicle thinks it leaves
     */
    SUMOTime getEventTime() const;

    /// @brief Like getEventTime but returns seconds (for visualization)
    inline double getEventTimeSeconds() const {
        return STEPS2TIME(getEventTime());
    }

    /// @brief get the last headway time in seconds
    inline double getLastHeadwaySeconds() const {
        return STEPS2TIME(myLastHeadway);
    }

    /// @brief get the earliest entry time in seconds
    inline double getEntryBlockTimeSeconds() const {
        SUMOTime t = SUMOTime_MAX;
        for (const Queue& q : myQueues) {
            t = MIN2(t, q.getEntryBlockTime());
        }
        return STEPS2TIME(t);
    }

    /// @brief Get the waiting time for vehicles in all queues
    double getWaitingSeconds() const;

    /// @name State saving/loading
    /// @{

    /** @brief Saves the state of this segment into the given stream
     *
     * Some internal values which must be restored are saved as well as ids of
     *  the vehicles stored in internal queues and the last departures of connected
     *  edges.
     *
     * @param[in, filled] out The (possibly binary) device to write the state into
     * @todo What about throwing an IOError?
     */
    void saveState(OutputDevice& out);

    /** @brief Remove all vehicles before quick-loading state */
    void clearState();

    /** @brief Loads the state of this segment with the given parameters
     *
     * This method is called for every internal que the segment has.
     *  Every vehicle is retrieved from the given MSVehicleControl and added to this
     *  segment. Then, the internal queues that store vehicles dependant to their next
     *  edge are filled the same way. Then, the departure of last vehicles onto the next
     *  edge are restored.
     *
     * @param[in] vehIDs The vehicle ids for the current que
     * @param[in] vc The vehicle control to retrieve references vehicles from
     * @param[in] blockTime The time the last vehicle left the que
     * @param[in] queIdx The index of the current que
     * @todo What about throwing an IOError?
     * @todo What about throwing an error if something else fails (a vehicle can not be referenced)?
     */
    void loadState(const std::vector<std::string>& vehIDs, MSVehicleControl& vc, const SUMOTime blockTime, const int queIdx);
    /// @}


    /** @brief returns all vehicles (for debugging)
     */
    std::vector<const MEVehicle*> getVehicles() const;

    /** @brief returns flow based on headway
     * @note: returns magic number 10000 when headway cannot be computed
     */
    double getFlow() const;

    /// @brief whether the given segment is 0 or encodes vaporization
    static inline bool isInvalid(const MESegment* segment) {
        return segment == nullptr || segment == &myVaporizationTarget;
    }

    /// @brief return a time after earliestEntry at which a vehicle may be inserted at full speed
    SUMOTime getNextInsertionTime(SUMOTime earliestEntry) const;

    /// @brief return the remaining physical space on this segment
    inline int remainingVehicleCapacity(const double vehLength) const {
        int cap = 0;
        for (const Queue& q : myQueues) {
            if (q.getOccupancy() == 0. && myQueueCapacity < vehLength) {
                // even small segments can hold at least one vehicle
                cap += 1;
            } else {
                cap += (int)((myQueueCapacity - q.getOccupancy()) / vehLength);
            }
        }
        return cap;
    }

    /// @brief return the minimum headway-time with which vehicles may enter or leave this segment
    inline SUMOTime getMinimumHeadwayTime() const {
        return myTau_ff;
    }

    static const double DO_NOT_PATCH_JAM_THRESHOLD;

    /// @brief add this lanes MoveReminders to the given vehicle
    void addReminders(MEVehicle* veh) const;

    /** @brief Returns the penalty time for passing a link (if using gMesoTLSPenalty > 0 or gMesoMinorPenalty > 0)
     * @param[in] veh The vehicle in question
     * @return The time penalty
     */
    SUMOTime getLinkPenalty(const MEVehicle* veh) const;

private:
    bool overtake();

    void setSpeedForQueue(double newSpeed, SUMOTime currentTime,
                          SUMOTime blockTime, const std::vector<MEVehicle*>& vehs);

    /** @brief compute the new arrival time when switching speed
     */
    SUMOTime newArrival(const MEVehicle* const v, double newSpeed, SUMOTime currentTime);

    /// @brief whether a leader in any queue is blocked
    bool hasBlockedLeader() const;

    /** @brief compute a value for myJamThreshold
     * if jamThresh is negative, compute a value which allows free flow at mySpeed
     * interpret jamThresh as the relative occupation at which jam starts
     */
    void recomputeJamThreshold(double jamThresh);

    /// @brief compute jam threshold for the given speed and jam-threshold option
    double jamThresholdForSpeed(double speed, double jamThresh) const;

    /// @brief whether the given link may be passed because the option meso-junction-control.limited is set
    bool limitedControlOverride(const MSLink* link) const;

    /// @brief return the maximum tls penalty for all links from this edge
    double getMaxPenaltySeconds() const;

    /// @brief convert net time gap (leader back to follower front) to gross time gap (leader front to follower front)
    inline SUMOTime tauWithVehLength(SUMOTime tau, double lengthWithGap) const {
        return tau + (SUMOTime)(lengthWithGap * myTau_length);
    }

private:
    /// @brief The microsim edge this segment belongs to
    const MSEdge& myEdge;

    /// @brief The next segment of this edge, 0 if this is the last segment of this edge
    MESegment* myNextSegment;

    /// @brief The segment's length
    const double myLength;

    /// @brief Running number of the segment in the edge
    const int myIndex;

    /// @brief The time headway parameters, see the Eissfeldt thesis
    const SUMOTime myTau_ff, myTau_fj, myTau_jf, myTau_jj;
    /// @brief Headway parameter for computing gross time headyway from net time headway, length and edge speed
    double myTau_length;

    /// @brief slope and axis offset for the jam-jam headway function
    double myA, myB;

    /// @brief The capacity of the segment in number of cars, used only in time headway calculation
    /// This parameter has only an effect if tau_jf != tau_jj, which is not(!) the case per default
    const double myHeadwayCapacity;

    /// @brief The number of lanes represented by the queue * the length of the lane
    const double myCapacity;

    /// @brief The number of lanes represented by the queue * the length of the lane
    const double myQueueCapacity;

    /// @brief Whether junction control is enabled
    const bool myJunctionControl;

    /// @brief Whether tls penalty is enabled
    const bool myTLSPenalty;

    /// @brief Whether minor penalty is enabled
    const bool myMinorPenalty;

    /// @brief The space (in m) which needs to be occupied before the segment is considered jammed
    double myJamThreshold;

    /// @brief The data collection for all kinds of detectors
    std::vector<MSMoveReminder*> myDetectorData;

    /// @brief The car queues. Vehicles are inserted in the front and removed in the back
    std::vector<Queue> myQueues;

    /// @brief The cached value for the number of vehicles
    int myNumVehicles;

    /// @brief The follower edge to allowed que index mapping for multi queue segments
    std::map<const MSEdge*, int> myFollowerMap;

    /// @brief the last headway
    SUMOTime myLastHeadway;

    /* @brief segment for signifying vaporization. This segment has invalid
     * data and should only be used as a unique pointer */
    static MSEdge myDummyParent;
    static MESegment myVaporizationTarget;

    /// @brief the mean speed on this segment. Updated at event time or on demand
    mutable double myMeanSpeed;

    /// @brief the time at which myMeanSpeed was last updated
    mutable SUMOTime myLastMeanSpeedUpdate;

private:
    /// @brief Invalidated copy constructor.
    MESegment(const MESegment&);

    /// @brief Invalidated assignment operator.
    MESegment& operator=(const MESegment&);

    /// @brief constructor for dummy segment
    MESegment(const std::string& id);
};
