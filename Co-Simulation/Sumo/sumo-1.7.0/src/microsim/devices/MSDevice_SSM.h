/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_SSM.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Leonhard Luecken
/// @date    11.06.2013
///
// An SSM-device logs encounters / conflicts of the carrying vehicle with other surrounding vehicles.
// XXX: Preliminary implementation. Use with care. Especially rerouting vehicles could be problematic.
/****************************************************************************/
#pragma once
#include <config.h>

#include <queue>
#include "MSVehicleDevice.h"
#include <utils/common/SUMOTime.h>
#include <utils/iodevices/OutputDevice_File.h>
#include <utils/geom/Position.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;
class SUMOTrafficObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_SSM
 * @brief A device which collects info on the vehicle trip (mainly on departure and arrival)
 *
 * Each device collects departure time, lane and speed and the same for arrival.
 *
 * @see MSDevice
 */

class MSCrossSection;

class MSDevice_SSM : public MSVehicleDevice {

private:
    /// All currently existing SSM devices
    static std::set<MSDevice_SSM*, ComparatorNumericalIdLess>* myInstances;

public:
    /// @brief Different types of encounters corresponding to relative positions of the vehicles.
    ///        The name describes the type from the ego perspective
    enum EncounterType {
        // Other vehicle is closer than range, but not on a lane conflicting with the ego's route ahead
        ENCOUNTER_TYPE_NOCONFLICT_AHEAD = 0,       //!< ENCOUNTER_TYPE_NOCONFLICT_AHEAD
        // Ego and foe vehicles' edges form a part of a consecutive sequence of edges
        // This type may be specified further by ENCOUNTER_TYPE_FOLLOWING_LEADER or ENCOUNTER_TYPE_FOLLOWING_FOLLOWER
        ENCOUNTER_TYPE_FOLLOWING = 1,       //!< ENCOUNTER_TYPE_FOLLOWING
        // Ego vehicle is on an edge that has a sequence of successors connected to the other vehicle's edge
        ENCOUNTER_TYPE_FOLLOWING_FOLLOWER = 2,       //!< ENCOUNTER_TYPE_FOLLOWING_FOLLOWER
        // Other vehicle is on an edge that has a sequence of successors connected to the ego vehicle's current edge
        ENCOUNTER_TYPE_FOLLOWING_LEADER = 3,         //!< ENCOUNTER_TYPE_FOLLOWING_LEADER
        // Other vehicle is on an edge that has a sequence of successors connected to the ego vehicle's current edge
        ENCOUNTER_TYPE_ON_ADJACENT_LANES = 4,         //!< ENCOUNTER_TYPE_ON_ADJACENT_LANES
        // Ego and foe share an upcoming edge of their routes while the merging point for the routes is still ahead
        // This type may be specified further by ENCOUNTER_TYPE_MERGING_LEADER or ENCOUNTER_TYPE_MERGING_FOLLOWER
        ENCOUNTER_TYPE_MERGING = 5,  //!< ENCOUNTER_TYPE_MERGING
        // Other vehicle is on an edge that has a sequence of successors connected to an edge on the ego vehicle's route
        // and the estimated arrival vehicle at the merge point is earlier for the ego than for the foe
        ENCOUNTER_TYPE_MERGING_LEADER = 6,  //!< ENCOUNTER_TYPE_MERGING_LEADER
        // Other vehicle is on an edge that has a sequence of successors connected to an edge on the ego vehicle's route
        // and the estimated arrival vehicle at the merge point is earlier for the foe than for the ego
        ENCOUNTER_TYPE_MERGING_FOLLOWER = 7,//!< ENCOUNTER_TYPE_MERGING_FOLLOWER
        // Vehicles' bestlanes lead to the same edge but to adjacent lanes
        ENCOUNTER_TYPE_MERGING_ADJACENT = 8,//!< ENCOUNTER_TYPE_MERGING_ADJACENT
        // Ego's and foe's routes have crossing edges
        // This type may be specified further by ENCOUNTER_TYPE_CROSSING_LEADER or ENCOUNTER_TYPE_CROSSING_FOLLOWER
        ENCOUNTER_TYPE_CROSSING = 9,  //!< ENCOUNTER_TYPE_CROSSING
        // Other vehicle is on an edge that has a sequence of successors leading to an internal edge that crosses the ego vehicle's edge at a junction
        // and the estimated arrival vehicle at the merge point is earlier for the ego than for the foe
        ENCOUNTER_TYPE_CROSSING_LEADER = 10, //!< ENCOUNTER_TYPE_CROSSING_LEADER
        // Other vehicle is on an edge that has a sequence of successors leading to an internal edge that crosses the ego vehicle's edge at a junction
        // and the estimated arrival vehicle at the merge point is earlier for the foe than for the ego
        ENCOUNTER_TYPE_CROSSING_FOLLOWER = 11, //!< ENCOUNTER_TYPE_CROSSING_FOLLOWER
        // The encounter is a possible crossing conflict, and the ego vehicle has entered the conflict area
        ENCOUNTER_TYPE_EGO_ENTERED_CONFLICT_AREA = 12, //!< ENCOUNTER_TYPE_EGO_ENTERED_CONFLICT_AREA
        // The encounter is a possible crossing conflict, and the foe vehicle has entered the conflict area
        ENCOUNTER_TYPE_FOE_ENTERED_CONFLICT_AREA = 13, //!< ENCOUNTER_TYPE_FOE_ENTERED_CONFLICT_AREA
        // The encounter has been a possible crossing conflict, but the ego vehicle has left the conflict area
        ENCOUNTER_TYPE_EGO_LEFT_CONFLICT_AREA = 14, //!< ENCOUNTER_TYPE_EGO_LEFT_CONFLICT_AREA
        // The encounter has been a possible crossing conflict, but the foe vehicle has left the conflict area
        ENCOUNTER_TYPE_FOE_LEFT_CONFLICT_AREA = 15, //!< ENCOUNTER_TYPE_FOE_LEFT_CONFLICT_AREA
        // The encounter has been a possible crossing conflict, and both vehicles have entered the conflict area (one must have already left, otherwise this must be a collision)
        ENCOUNTER_TYPE_BOTH_ENTERED_CONFLICT_AREA = 16, //!< ENCOUNTER_TYPE_BOTH_ENTERED_CONFLICT_AREA
        // The encounter has been a possible crossing conflict, but both vehicle have left the conflict area
        ENCOUNTER_TYPE_BOTH_LEFT_CONFLICT_AREA = 17, //!< ENCOUNTER_TYPE_BOTH_LEFT_CONFLICT_AREA
        // FOLLOWING_PASSED and MERGING_PASSED are reserved to achieve that these encounter types may be tracked longer (see updatePassedEncounter)
        // The encounter has been a following situation, but is not active any more
        ENCOUNTER_TYPE_FOLLOWING_PASSED = 18, //!< ENCOUNTER_TYPE_FOLLOWING_PASSED
        // The encounter has been a merging situation, but is not active any more
        ENCOUNTER_TYPE_MERGING_PASSED = 19, //!< ENCOUNTER_TYPE_FOLLOWING_PASSED
        // Ego vehicle and foe are driving in opposite directions towards each other on the same lane (or sequence of consecutive lanes)
        ENCOUNTER_TYPE_ONCOMING = 20,
        // Collision (currently unused, might be differentiated further)
        ENCOUNTER_TYPE_COLLISION = 111 //!< ENCOUNTER_TYPE_COLLISION
    };

    static std::string encounterToString(EncounterType type) {
        switch (type) {
            case (ENCOUNTER_TYPE_NOCONFLICT_AHEAD):
                return ("NOCONFLICT_AHEAD");
            case (ENCOUNTER_TYPE_FOLLOWING):
                return ("FOLLOWING");
            case (ENCOUNTER_TYPE_FOLLOWING_FOLLOWER):
                return ("FOLLOWING_FOLLOWER");
            case (ENCOUNTER_TYPE_FOLLOWING_LEADER):
                return ("FOLLOWING_LEADER");
            case (ENCOUNTER_TYPE_ON_ADJACENT_LANES):
                return ("ON_ADJACENT_LANES");
            case (ENCOUNTER_TYPE_MERGING):
                return ("MERGING");
            case (ENCOUNTER_TYPE_MERGING_LEADER):
                return ("MERGING_LEADER");
            case (ENCOUNTER_TYPE_MERGING_FOLLOWER):
                return ("MERGING_FOLLOWER");
            case (ENCOUNTER_TYPE_MERGING_ADJACENT):
                return ("MERGING_ADJACENT");
            case (ENCOUNTER_TYPE_CROSSING):
                return ("CROSSING");
            case (ENCOUNTER_TYPE_CROSSING_LEADER):
                return ("CROSSING_LEADER");
            case (ENCOUNTER_TYPE_CROSSING_FOLLOWER):
                return ("CROSSING_FOLLOWER");
            case (ENCOUNTER_TYPE_EGO_ENTERED_CONFLICT_AREA):
                return ("EGO_ENTERED_CONFLICT_AREA");
            case (ENCOUNTER_TYPE_FOE_ENTERED_CONFLICT_AREA):
                return ("FOE_ENTERED_CONFLICT_AREA");
            case (ENCOUNTER_TYPE_EGO_LEFT_CONFLICT_AREA):
                return ("EGO_LEFT_CONFLICT_AREA");
            case (ENCOUNTER_TYPE_FOE_LEFT_CONFLICT_AREA):
                return ("FOE_LEFT_CONFLICT_AREA");
            case (ENCOUNTER_TYPE_BOTH_ENTERED_CONFLICT_AREA):
                return ("BOTH_ENTERED_CONFLICT_AREA");
            case (ENCOUNTER_TYPE_BOTH_LEFT_CONFLICT_AREA):
                return ("BOTH_LEFT_CONFLICT_AREA");
            case (ENCOUNTER_TYPE_FOLLOWING_PASSED):
                return ("FOLLOWING_PASSED");
            case (ENCOUNTER_TYPE_MERGING_PASSED):
                return ("MERGING_PASSED");
            case (ENCOUNTER_TYPE_ONCOMING):
                return ("ONCOMING");
            case (ENCOUNTER_TYPE_COLLISION):
                return ("COLLISION");
        }
        return ("UNKNOWN");
    };

private:
    /// @brief An encounter is an episode involving two vehicles,
    ///        which are closer to each other than some specified distance.
    class Encounter {
    private:
        /// @brief A trajectory encloses a series of positions x and speeds v for one vehicle
        /// (the times are stored only once in the enclosing encounter)
        struct Trajectory {
            // positions
            PositionVector x;
            // momentary speeds
            PositionVector v;
        };
        /// @brief ConflictPointInfo stores some information on a specific conflict point
        ///        (used to store information on ssm-extremal values)
        struct ConflictPointInfo {
            /// @brief time point of the conflict
            double time;
            /// @brief Predicted location of the conflict:
            /// In case of MERGING and CROSSING: entry point to conflict area for follower
            /// In case of FOLLOWING: position of leader's back
            Position pos;
            /// @brief Type of the conflict
            EncounterType type;
            /// @brief value of the corresponding SSM
            double value;

            ConflictPointInfo(double time, Position x, EncounterType type, double ssmValue) :
                time(time), pos(x), type(type), value(ssmValue) {};
        };

    public:
        /// @brief Constructor
        Encounter(const MSVehicle* _ego, const MSVehicle* const _foe, double _begin, double extraTime);
        /// @brief Destructor
        ~Encounter();

        /// @brief add a new data point and update encounter type
        void add(double time, EncounterType type, Position egoX, Position egoV, Position foeX, Position foeV,
                 Position conflictPoint, double egoDistToConflict, double foeDistToConflict, double ttc, double drac, std::pair<double, double> pet);

        /// @brief Returns the number of trajectory points stored
        std::size_t size() const {
            return timeSpan.size();
        }

        /// @brief resets remainingExtraTime to the given value
        void resetExtraTime(double value);
        /// @brief decreases myRemaingExtraTime by given amount in seconds
        void countDownExtraTime(double amount);
        /// @brief returns the remaining extra time
        double getRemainingExtraTime() const;

        /// @brief Compares encounters regarding to their start time
        struct compare {
            typedef bool value_type;
            bool operator()(Encounter* e1, Encounter* e2) {
                if (e1->begin == e2->begin) {
                    return e1->foeID > e2->foeID;
                } else {
                    return e1->begin > e2->begin;
                }
            };
        };



    public:
        const MSVehicle* ego;
        const MSVehicle* foe;
        const std::string egoID;
        const std::string foeID;
        double begin, end;
        EncounterType currentType;

        /// @brief Remaining extra time (decreases after an encounter ended)
        double remainingExtraTime;

        /// @brief Times when the ego vehicle entered/left the conflict area. Currently only applies for crossing situations. Used for PET calculation. (May be defined for merge conflicts in the future)
        double egoConflictEntryTime, egoConflictExitTime;
        /// @brief Times when the foe vehicle entered/left the conflict area. Currently only applies for crossing situations. Used for PET calculation. (May be defined for merge conflicts in the future)
        double foeConflictEntryTime, foeConflictExitTime;

        /// @brief time points corresponding to the trajectories
        std::vector<double> timeSpan;
        /// @brief Evolution of the encounter classification (@see EncounterType)
        std::vector<int> typeSpan;
        /// @brief Trajectory of the ego vehicle
        Trajectory egoTrajectory;
        /// @brief Trajectory of the foe vehicle
        Trajectory foeTrajectory;
        /// Evolution of the ego vehicle's distance to the conflict point
        std::vector<double> egoDistsToConflict;
        /// Evolution of the foe vehicle's distance to the conflict point
        std::vector<double> foeDistsToConflict;

        /// @brief Predicted location of the conflict:
        /// In case of MERGING and CROSSING: entry point to conflict area for follower
        /// In case of FOLLOWING: position of leader's back
        PositionVector conflictPointSpan;

        /// @brief All values for TTC
        std::vector<double> TTCspan;
        /// @brief All values for DRAC
        std::vector<double> DRACspan;

//        /// @brief Cross sections at which a PET shall be calculated for the corresponding vehicle
//        std::vector<std::pair<std::pair<const MSLane*, double>, double> > egoPETCrossSections;
//        std::vector<std::pair<std::pair<const MSLane*, double>, double> > foePETCrossSections;

        /// @name Extremal values for the SSMs
        /// @{
        ConflictPointInfo minTTC;
        ConflictPointInfo maxDRAC;
        ConflictPointInfo PET;
        /// @}

        /// @brief this flag is set by updateEncounter() or directly in processEncounters(), where encounters are closed if it is true.
        bool closingRequested;

    private:
        /// @brief Invalidated Constructor.
        Encounter(const Encounter&);
        /// @brief Invalidated assignment operator.
        Encounter& operator=(const Encounter&);
        ///
    };


    /// @brief Structure to collect some info on the encounter needed during ssm calculation by various functions.
    struct EncounterApproachInfo {
        EncounterApproachInfo(Encounter* e);
        Encounter* encounter;
        EncounterType type;
        Position conflictPoint;
        double egoConflictEntryDist;
        double foeConflictEntryDist;
        double egoConflictExitDist;
        double foeConflictExitDist;
        double egoEstimatedConflictEntryTime;
        double foeEstimatedConflictEntryTime;
        double egoEstimatedConflictExitTime;
        double foeEstimatedConflictExitTime;
        double egoConflictAreaLength;
        double foeConflictAreaLength;
        bool egoLeftConflict;
        bool foeLeftConflict;
        double ttc;
        double drac;
        std::pair<double, double> pet;
        std::pair<const MSLane*, double> egoConflictEntryCrossSection;
        std::pair<const MSLane*, double> foeConflictEntryCrossSection;
    };


    /// A new FoeInfo is created during findSurroundingVehicles() to memorize, where the potential conflict
    /// corresponding to the encounter might occur. Each FoeInfo ends up in a call to updateEncounter() and
    /// is deleted there.
    struct FoeInfo {
        virtual ~FoeInfo() {};
        const MSLane* egoConflictLane;
        double egoDistToConflictLane;
    };
    // TODO: consider introducing a class foeCollector, which holds the foe info content
    //       plus a vehicle container to be used in findSurrounding vehicles.
    //       findSurroundingVehicles() would then deliver a vector of such foeCollectors
    //       (one for each possible egoConflictLane) instead of a map vehicle->foeInfo
    //       This could be helpful to resolve the resolution for several different
    //          projected conflicts with the same foe.


    /// @brief Auxiliary structure used to handle upstream scanning start points
    /// Upstream scan has to be started after downstream scan is completed, see #5644
    struct UpstreamScanStartInfo {
        UpstreamScanStartInfo(const MSEdge* edge, double pos, double range, double egoDistToConflictLane, const MSLane* egoConflictLane) :
            edge(edge), pos(pos), range(range), egoDistToConflictLane(egoDistToConflictLane), egoConflictLane(egoConflictLane) {};
        const MSEdge* edge;
        double pos;
        double range;
        double egoDistToConflictLane;
        const MSLane* egoConflictLane;
    };

    typedef std::priority_queue<Encounter*, std::vector<Encounter*>, Encounter::compare> EncounterQueue;
    typedef std::vector<Encounter*> EncounterVector;
    typedef std::map<const MSVehicle*, FoeInfo*> FoeInfoMap;
public:

    /** @brief Inserts MSDevice_SSM-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a example-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);


    /** @brief returns all currently existing SSM devices
     */
    static const std::set<MSDevice_SSM*, ComparatorNumericalIdLess>& getInstances();

    /** @brief This is called once per time step in MSNet::writeOutput() and
     *         collects the surrounding vehicles, updates information on encounters
     *         and flushes the encounters qualified as conflicts (@see thresholds)
     *         to the output file.
     */
    void updateAndWriteOutput();

    /// @brief try to retrieve the given parameter from this device. Throw exception for unsupported key
    std::string getParameter(const std::string& key) const;

    /// @brief try to set the given parameter for this device. Throw exception for unsupported key
    void setParameter(const std::string& key, const std::string& value);

private:
    void update();
    void writeOutConflict(Encounter* e);

    /// @brief convert SUMO-positions to geo coordinates (in place)
    static void toGeo(Position& x);
    /// @brief convert SUMO-positions to geo coordinates (in place)
    static void toGeo(PositionVector& x);

public:
    /** @brief Clean up remaining devices instances
     */
    static void cleanup();


public:
    /// @brief Destructor.
    ~MSDevice_SSM();


    /** @brief Returns all vehicles, which are within the given range of the given vehicle.
     *  @note all vehicles behind and in front are collected,
     *  including vehicles on confluent edges. For instance, if the range is 20 m. and
     *  a junction lies 10 m. ahead, an upstream scan of 20 m. is performed
     *  for all incoming edges.
     *
     * @param veh   The ego vehicle, that forms the origin for the scan
     * @param range The range to be scanned.
     * @param[in/out] foeCollector container for all collected vehicles
     * @return All vehicles within range from veh
     */
    static void findSurroundingVehicles(const MSVehicle& veh, double range, FoeInfoMap& foeCollector);

    /** @brief Collects all vehicles within range 'range' upstream of the position 'pos' on the edge 'edge' into foeCollector
     */
    static void getUpstreamVehicles(const UpstreamScanStartInfo& scanStart, FoeInfoMap& foeCollector, std::set<const MSLane*>& seenLanes, const std::set<const MSJunction*>& routeJunctions);

    /** @brief Collects all vehicles on the junction into foeCollector
     */
    static void getVehiclesOnJunction(const MSJunction*, const MSLane* egoJunctionLane, double egoDistToConflictLane, const MSLane* const egoConflictLane, FoeInfoMap& foeCollector, std::set<const MSLane*>& seenLanes);


    /// @name Methods called on vehicle movement / state change, overwriting MSDevice
    /// @{

    /** @brief Checks for waiting steps when the vehicle moves
     *
     * @param[in] veh Vehicle that notifies.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     *
     * @return Always true to keep the device as it cannot be thrown away
     */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos,
                    double newPos, double newSpeed);


    /** @brief Called whenever the holder enteres a lane
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason Reason for leaving the lane
     * @param[in] enteredLane The lane entered.
     * @return Always true to keep the device as it cannot be thrown away
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Called whenever the holder leaves a lane
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] reason Reason for leaving the lane
     * @param[in] enteredLane The lane entered.
     * @return True if it did not leave the net.
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos,
                     MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
    /// @}


    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "ssm";
    }

    /** @brief Finalizes output. Called on vehicle removal
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     * @see MSDevice::generateOutput
     */
    void generateOutput(OutputDevice* tripinfoOut) const;



private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     * @param measures Vector of Surrogate Safety Measure IDs
     * @param thresholds Vector of corresponding thresholds
     * @param trajectories Flag indicating whether complete trajectories should be saved for an encounter (if false only extremal values are logged)
     * @param range Detection range. For vehicles closer than this distance from the ego vehicle, SSMs are traced
     * @param extraTime Extra time in seconds to be logged after a conflict is over
     * @param useGeoCoords Whether coordinates should be written out in the original coordinate reference system or as sumo's x,y values
     */
    MSDevice_SSM(SUMOVehicle& holder, const std::string& id, std::string outputFilename, std::map<std::string, double> thresholds,
                 bool trajectories, double range, double extraTime, bool useGeoCoords);

    /** @brief Finds encounters for which the foe vehicle has disappeared from range.
     *         remainingExtraTime is decreased until it reaches zero, which triggers closing the encounter.
     *         If an ended encounter is qualified as a conflict, it is transferred to myPastConflicts
     *         All vehicles for which an encounter instance already exists (vehicle is already tracked)
     *         are removed from 'foes' during processing.
     *  @param[in] foes Foe vehicles that have been found by findSurroundingVehicles()
     *  @param[in] forceClose whether encounters for which the foe is not in range shall be closed immediately, disregarding the remaining extra time (is requested by resetEncounters()).
     */
    void processEncounters(FoeInfoMap& foes, bool forceClose = false);


    /** @brief Closes encounters, whose duration exceeds the maximal encounter length. If it is classified as conflict, the encounter is saved.
     *         In any case, a new active encounter is created holding the trailing part (of length myOverlapTime) of the original.
     */
    void storeEncountersExceedingMaxLength();



    /** @brief Makes new encounters for all given vehicles (these should be the ones entering the device's range in the current timestep)
     */
    void createEncounters(FoeInfoMap& foes);


    /** @brief Stores measures, that are not associated to a specific encounter as headways and brake rates
     *  @todo  Manage as episodes (BR -> braking episode, SGAP/TGAP -> car-following episode) with invariant leader, and filtering applying the
     *  corresponding thresholds.
     */
    void computeGlobalMeasures();

    /** @brief Closes all current Encounters and moves conflicts to myPastConflicts, @see processEncounters
     */
    void resetEncounters();

    /** @brief Writes out all past conflicts that have begun earlier than the oldest active encounter
     * @param[in] all Whether all conflicts should be flushed or only those for which no active encounters with earlier begin can exist
     */
    void flushConflicts(bool all = false);

    /** @brief Write out all non-encounter specific measures as headways and braking rates.
     *  @todo  Adapt accordingly if episode structure is implemented, @see computeGlobalMeasures()
     */
    void flushGlobalMeasures();

    /** @brief Updates the encounter (adds a new trajectory point).
     *  @return Returns false for new encounters, which should not be kept (if one vehicle has
     *          already left the conflict zone at encounter creation). True, otherwise.
     */
    bool updateEncounter(Encounter* e, FoeInfo* foeInfo);

    /** @brief Updates an encounter, which was classified as ENCOUNTER_TYPE_NOCONFLICT_AHEAD
     *         this may be the case because the foe is out of the detection range but the encounter
     *         is still in extra time (in this case foeInfo==0), or because the foe does not head for a lane conflicting with
     *         the route of the ego vehicle.
     *         It is also used for an ongoing crossing conflict, where only the covered distances are traced
     *         until the situation is over. (A crossing conflict is ongoing, if one vehicle entered the conflict area)
     *         Writes the type of encounter which is determined for the current state into eInfo. And if appropriate some
     *         information concerning vehicles positions in relation to a crossed crossing point (for PET calculation).
     */
    void updatePassedEncounter(Encounter* e, FoeInfo* foeInfo, EncounterApproachInfo& eInfo);


    /** @brief Classifies the current type of the encounter provided some information on the opponents
     *  @param[in] foeInfo Info on distance to conflict point for the device holder.
     *  @param[in/out] eInfo  Info structure for the current state of the encounter (provides a pointer to the encounter).
     *  @return Returns an encounter type and writes a value to the relevant distances (egoEncounterDist, foeEncounterDist members of eInfo),
     *          i.e. the distances to the entry points to the potential conflict.
     *  @note: The encounter distance has a different meaning for different types of encounters:
     *          1) For rear-end conflicts (lead/follow situations) the follower's encounter distance is the distance to the actual back position of the leader. The leaders's distance is undefined.
     *          2) For merging encounters the encounter distance is the distance until the begin of the common target edge/lane.
     *          3) For crossing encounters the encounter distance is the distance until crossing point of the conflicting lanes.
     */
    EncounterType classifyEncounter(const FoeInfo* foeInfo, EncounterApproachInfo& eInfo) const;


    /** @brief Calculates the (x,y)-coordinate for the eventually predicted conflict point and stores the result in
     *         eInfo.conflictPoint. In case of MERGING and CROSSING, this is the entry point to conflict area for follower
     *         In case of FOLLOWING it is the position of leader's back.
     *  @param[in/out] eInfo  Info structure for the current state of the encounter.
     */
    static void determineConflictPoint(EncounterApproachInfo& eInfo);


    /** @brief Estimates the time until conflict for the vehicles based on the distance to the conflict entry points.
     *         For acceleration profiles, we assume that the acceleration is <= 0 (that is, braking is extrapolated,
     *         while for acceleration it is assumed that the vehicle will continue with its current speed)
     *  @param[in/out] eInfo  Info structure for the current state of the encounter.
     *  @note The '[in]'-part for eInfo are its members e->ego, e->foe (to access the vehicle parameters), egoConflictEntryDist, foeConflictEntryDist, i.e., distances to the conflict entry points.
     *        The '[out]'-part for eInfo are its members type (type information may be refined) egoConflictEntryTime, foeConflictEntryTime (estimated times until the conflict entry point is reached)
     *        and egoConflictExitTime, foeConflictExitTime (estimated time until the conflict exit point is reached).
     *        Further the type of the encounter as determined by classifyEncounter(), is refined for the cases CROSSING and MERGING here.
     */
    static void estimateConflictTimes(EncounterApproachInfo& eInfo);


    /** @brief Checks whether ego or foe have entered or left the conflict area in the last step and eventually writes
     *         the corresponding entry or exit times to eInfo.encounter. For ongoing crossing conflicts, it also manages
     *         the evolution of the conflict type.
     *  @param[in/out] eInfo  Info structure for the current state of the encounter.
     *  @note The times are to be used for SSM computation in computeSSMs(), e.g. in determinePET()
     */
    static void checkConflictEntryAndExit(EncounterApproachInfo& eInfo);


    /** @brief Computes the conflict lane for the foe
     *
     * @param foe Foe vehicle
     * @param egoConflictLane Lane, on which the ego would enter the possible conflict
     * @param routeToConflict, Series of edges, that were traced back from egoConflictLane during findSurrounding Vehicles, when collecting the foe vehicle
     * @param[out] distToConflictLane, distance to conflictlane entry link (may be negative if foe is already on the conflict lane)
     * @return Lane, on which the foe would enter the possible conflict, if foe is not on conflict course, Null-pointer is returned.
     */
    const MSLane* findFoeConflictLane(const MSVehicle* foe, const MSLane* egoConflictLane, double& distToConflictLane) const;

    /** @brief Finalizes the encounter and calculates SSM values.
     */
    void closeEncounter(Encounter* e);

    /** @brief Tests if the SSM values exceed the threshold for qualification as conflict.
     */
    bool qualifiesAsConflict(Encounter* e);

    /** @brief Compute current values of the logged SSMs (myMeasures) for the given encounter 'e'
     *  and update 'e' accordingly (add point to SSM time-series, update maximal/minimal value)
     *  This is called just after adding the current vehicle positions and velocity vectors to the encounter.
     */
    void computeSSMs(EncounterApproachInfo& e) const;


    /** @brief Discriminates between different encounter types and correspondingly determines the PET for those cases
     *         and writes the result to eInfo.pet (after both vehicles have left the conflict area)
     */
    void determinePET(EncounterApproachInfo& eInfo) const;


    /** @brief Discriminates between different encounter types and correspondingly determines TTC and DRAC for those cases
     *         and writes the result to eInfo.ttc and eInfo.drac
     */
    void determineTTCandDRAC(EncounterApproachInfo& eInfo) const;


    /** @brief Computes the time to collision (in seconds) for two vehicles with a given initial gap under the assumption
     *         that both maintain their current speeds. Returns INVALID if no collision would occur under this assumption.
     */
    double computeTTC(double gap, double followerSpeed, double leaderSpeed) const;


    /** @brief Computes the DRAC (deceleration to avoid a collision) for a lead/follow situation as defined,
     *         e.g., in Guido et al. (2011, Safety performance measures:  a comparison between microsimulation and observational data)
     *         for two vehicles with a given gap.
     *         Returns 0.0 if no deceleration is required by the follower to avoid a crash, INVALID if collision is detected.
     */
    static double computeDRAC(double gap, double followerSpeed, double leaderSpeed);

    /** @brief Computes the DRAC a crossing situation, determining the minimal constant deceleration needed
     *         for one of the vehicles to reach the conflict area after the other has left.
     *         for estimated leaving times, current deceleration is extrapolated, and acceleration is neglected.
     *         Returns 0.0 if no deceleration is required by the follower to avoid a crash, INVALID if collision is detected.
     *  @param[in] eInfo infos on the encounter. Used variables:
     *               dEntry1,dEntry2 The distances to the conflict area entry
     *               dExit1,dExit2 The distances to the conflict area exit
     *               v1,v2 The current speeds
     *               tEntry1,tEntry2 The estimated conflict entry times (including extrapolation of current acceleration)
     *               tExit1,tExit2 The estimated conflict exit times (including extrapolation of current acceleration)
     */
    static double computeDRAC(const EncounterApproachInfo& eInfo);

    /** @brief make a string of a double vector and treat a special value as invalid ("NA")
     *
     * @param v vector to be converted to string
     * @param NA value to be treated as NA
     * @param sep separator for values in string
     * @return String concatenation of the vector entries
     */
    static std::string makeStringWithNAs(const std::vector<double>& v, const double NA);
    static std::string makeStringWithNAs(const std::vector<double>& v, const std::vector<double>& NAs);
    static std::string makeStringWithNAs(const PositionVector& v, const int precision);

    /// @name parameter load helpers (introduced for readability of buildVehicleDevices())
    /// @{
    static std::string getOutputFilename(const SUMOVehicle& v, std::string deviceID);
    static double getDetectionRange(const SUMOVehicle& v);
    static double getExtraTime(const SUMOVehicle& v);
    static bool useGeoCoords(const SUMOVehicle& v);
    static bool requestsTrajectories(const SUMOVehicle& v);
    static bool getMeasuresAndThresholds(const SUMOVehicle& v, std::string deviceID,
                                         std::map<std::string, double>& thresholds);
    ///@}

private:
    /// @name Device parameters
    /// @{
    /// @brief thresholds for the ssms, i.e., critical values above or below which a value indicates that a conflict
    ///        has occurred. These are used in qualifiesAsConflict() and decide whether an encounter is saved.
    std::map<std::string, double> myThresholds;
    /// @brief This determines whether the whole trajectories of the vehicles (position, speed, ssms) shall be saved in the ssm-output
    ///        or only the most critical value shall be reported.
    bool mySaveTrajectories;
    /// Detection range. For vehicles closer than this distance from the ego vehicle, SSMs are traced
    double myRange;
    /// Extra time in seconds to be logged after a conflict is over
    double myExtraTime;
    /// Whether to use the original coordinate system for output
    bool myUseGeoCoords;
    /// Flags for switching on / off comutation of different SSMs, derived from myMeasures
    bool myComputeTTC, myComputeDRAC, myComputePET, myComputeBR, myComputeSGAP, myComputeTGAP;
    MSVehicle* myHolderMS;
    /// @}


    /// @name Internal storage for encounters/conflicts
    /// @{
    /// @brief Currently observed encounters/conflicts
    EncounterVector myActiveEncounters;
    /// @brief begin time of the oldest active encounter
    double myOldestActiveEncounterBegin;
    /// @brief Past encounters that where qualified as conflicts and are not yet flushed to the output file
    EncounterQueue myPastConflicts;
    /// @}



    /// @name Internal storage for global measures
    /// @{
    std::vector<double> myGlobalMeasuresTimeSpan;
    /// @brief All values for brake rate
    std::vector<double> myBRspan;
    /// @brief All values for space gap
    std::vector<double> mySGAPspan;
    /// @brief All values for time gap
    std::vector<double> myTGAPspan;
    /// @brief Extremal values for the global measures (as <<<time, Position>, value>, [leaderID]>-pairs)
    /// @{
    std::pair<std::pair<double, Position>, double> myMaxBR;
    std::pair<std::pair<std::pair<double, Position>, double>, std::string>  myMinSGAP;
    std::pair<std::pair<std::pair<double, Position>, double>, std::string>  myMinTGAP;
    /// @}
    /// @}

    /// Output device
    OutputDevice* myOutputFile;

    /// @brief remember which files were created already (don't duplicate xml root-elements)
    static std::set<std::string> createdOutputFiles;


    /// @brief bitset storing info whether warning has already been issued about unset parameter (warn only once!)
    static int issuedParameterWarnFlags;
    enum SSMParameterWarning {
        SSM_WARN_MEASURES = 1,
        SSM_WARN_THRESHOLDS = 1 << 1,
        SSM_WARN_TRAJECTORIES = 1 << 2,
        SSM_WARN_RANGE = 1 << 3,
        SSM_WARN_EXTRATIME = 1 << 4,
        SSM_WARN_FILE = 1 << 5,
        SSM_WARN_GEO = 1 << 6
    };



private:
    /// @brief Invalidated copy constructor.
    MSDevice_SSM(const MSDevice_SSM&);

    /// @brief Invalidated assignment operator.
    MSDevice_SSM& operator=(const MSDevice_SSM&);


};
