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
/// @file    MSE2Collector.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Robbin Blokpoel
/// @author  Jakob Erdmann
/// @author  Leonhard Luecken
/// @date    Mon Feb 03 2014 14:13 CET
///
// An areal detector covering to a sequence of consecutive lanes
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <list>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSMoveReminder.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <utils/common/UtilExceptions.h>
#include <cassert>


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class SUMOVehicle;
class SUMOTrafficObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSE2Collector
 * @brief An areal detector corresponding to a sequence of consecutive lanes
 *
 * This detector traces vehicles which are on a sequence of consecutive lanes. A
 *  vehicle that enters the detector is stored and the stored vehicles' speeds
 *  are used within each timestep to compute the detector values. As soon as the
 *  vehicle leaves the detector, it is no longer tracked.
 *
 * Determining entering and leaving vehicles is done via the MSMoveReminder
 *  interface. The values are computed by an event-callback (at the end of
 *  a time step).
 *
 * @note As soon as a vehicle enters the detector, a VehicleInfo object is created
 *        and stored in myVehicleInfos. This is constantly updated as long as the
 *        vehicle stays on the detector (i.e. calls notifyMove()). All movement
 *        notifications sent by vehicles on the detector are temporarily stored
 *        in myMoveNotifications, see notifyMove(). Finally they are integrated
 *        into myVehicleInfos when updateDetector is called.
 * @note When subclassing this detector, it is probably sufficient to adapt the
 *        definition of the structs VehicleInfo and the MoveNotification, as well as
 *        the methods that define and create those structs, i.e., makeVehicleInfo()
 *        and makeMoveNotification(). Further the integration of new movement
 *        notifications of the last time step into the vehicle infos is done
 *        in updateVehicleInfos().
 *
 */


class MSE2Collector : public MSMoveReminder, public MSDetectorFileOutput {
public:
    /** @brief A VehicleInfo stores values that are tracked for the individual vehicles on the detector,
     *         e.g., accumulated timeloss. These infos are stored in myVehicles. If a vehicle leaves the detector
     *         (may it be temporarily), the entry in myVehicles is discarded, i.e. all information on the vehicle is reset.
    */
    struct VehicleInfo {
        /** @note Constructor expects an entryLane argument corresponding to a lane, which is part of the detector.
        */
        VehicleInfo(std::string id, std::string type, double length, double minGap, const MSLane* entryLane, double entryOffset,
                    std::size_t currentOffsetIndex, double exitOffset, double distToDetectorEnd, bool onDetector) :
            id(id),
            type(type),
            length(length),
            minGap(minGap),
            entryLaneID(entryLane->getID()),
            entryOffset(entryOffset),
            currentLane(entryLane),
            currentOffsetIndex(currentOffsetIndex),
            exitOffset(exitOffset),
            distToDetectorEnd(distToDetectorEnd),
            totalTimeOnDetector(0.),
            accumulatedTimeLoss(0.),
            onDetector(onDetector),
            hasEntered(false),
            lastAccel(0),
            lastSpeed(0),
            lastPos(0) {
            assert(exitOffset < 0);
        }
        virtual ~VehicleInfo() {};
        /// vehicle's ID
        std::string id;
        /// vehicle's type
        std::string type;
        /// vehicle's length
        double length;
        /// vehicle's minGap
        double minGap;
        /// ID of the lane, on which the vehicle entered the detector
        std::string entryLaneID;
        /// Distance of the vehicle's entry lane's beginning to the detector start (can be negative for the first lane)
        /// In notifyMove(), the positional input arguments are relative to that position (since the vehicle picks up the MoveReminder
        /// on the entry lane)
        double entryOffset;
        /// Lane, on which the vehicle currently resides (always the one for which the last notifyEnter was received)
        const MSLane* currentLane;
        /// Index of currentLane in the detector's myLanes vector.
        std::size_t currentOffsetIndex;
        /// Offset from the detector start, where the vehicle has leaves the detector (defaults to detector length and is updated
        /// if the vehicle leaves the detector via a junction before reaching its end, i.e. enters a lane not part of the detector)
        double exitOffset;
        /// Distance left till the detector end after the last integration step (may become negative if the vehicle passes beyond the detector end)
        double distToDetectorEnd;
        /// Accumulated time that this vehicle has spent on the detector since its last entry
        double totalTimeOnDetector;
        /// Accumulated time loss that this vehicle suffered since it entered the detector
        double accumulatedTimeLoss;

        /// whether the vehicle is on the detector at the end of the current timestep
        bool onDetector;
        /// Whether the vehicle has already entered the detector (don't count twice!)
        bool hasEntered;
        /// Last value of the acceleration
        double lastAccel;
        /// Last value of the speed
        double lastSpeed;
        /// Last value of the vehicle position in reference to the start lane
        /// @note NOT in reference to the entry lane as newPos argument in notifyMove()!
        double lastPos;
    };

    typedef std::map<std::string, VehicleInfo*> VehicleInfoMap;


private:
    /** @brief Values collected in notifyMove and needed in detectorUpdate() to
     *          calculate the accumulated quantities for the detector. These are
     *          temporarily stored in myMoveNotifications for each step.
    */
    struct MoveNotificationInfo {
        MoveNotificationInfo(std::string _vehID, double _oldPos, double _newPos, double _speed, double _accel, double _distToDetectorEnd, double _timeOnDetector, double _lengthOnDetector, double _timeLoss, bool _onDetector) :
            id(_vehID),
            oldPos(_oldPos),
            newPos(_newPos),
            speed(_speed),
            accel(_accel),
            distToDetectorEnd(_distToDetectorEnd),
            timeOnDetector(_timeOnDetector),
            lengthOnDetector(_lengthOnDetector),
            timeLoss(_timeLoss),
            onDetector(_onDetector) {}

        virtual ~MoveNotificationInfo() {};

        /// Vehicle's id
        std::string id;
        /// Position before the last integration step (relative to the vehicle's entry lane on the detector)
        double oldPos;
        /// Position after the last integration step (relative to the vehicle's entry lane on the detector)
        double newPos;
        /// Speed after the last integration step
        double speed;
        /// Acceleration in the last integration step
        double accel;
        /// Distance left till the detector end after the last integration step (may become negative if the vehicle passes beyond the detector end)
        double distToDetectorEnd;
        /// Time spent on the detector during the last integration step
        double timeOnDetector;
        /// The length of the part of the vehicle on the detector at the end of the last time step
        double lengthOnDetector;
        /// timeloss during the last integration step
        double timeLoss;
        /// whether the vehicle is on the detector at the end of the current timestep
        bool onDetector;
    };



    /** @brief Internal representation of a jam
     *
     * Used in execute, instances of this structure are used to track
     *  begin and end positions (as vehicles) of a jam.
     */
    struct JamInfo {
        /// @brief The first standing vehicle
        std::vector<MoveNotificationInfo*>::const_iterator firstStandingVehicle;

        /// @brief The last standing vehicle
        std::vector<MoveNotificationInfo*>::const_iterator lastStandingVehicle;
    };


public:

    /** @brief Constructor with given end position and detector length
    *
    * @param[in] id The detector's unique id.
    * @param[in] usage Information how the detector is used
    * @param[in] lane The lane the detector ends
    * @param[in] startPos The start position on the lane the detector is placed at
    * @param[in] endPos The end position on the lane the detector is placed at
    * @param[in] length The length the detector has (heuristic lane selection is done if the continuation is not unique)
    * @param[in] haltingTimeThreshold The time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
    * @param[in] haltingSpeedThreshold The speed a vehicle's speed must be below to be assigned as jammed
    * @param[in] jamDistThreshold The distance between two vehicles in order to not count them to one jam
    * @param[in] vTypes Vehicle types, that the detector takes into account
    *
    * @note Exactly one of the arguments startPos, endPos and length should be invalid (i.e. equal to std::numeric_limits<double>::max()).
    *        If length is invalid, it is required that 0 <= startPos < endPos <= lane->length
    *        If endPos is invalid, the detector may span over several lanes downstream of the lane
    *        If pos is invalid, the detector may span over several lanes upstream of the lane
    */
    MSE2Collector(const std::string& id,
                  DetectorUsage usage, MSLane* lane, double startPos, double endPos, double length,
                  SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                  const std::string& vTypes);


    /** @brief Constructor with a sequence of lanes and given start and end position on the first and last lanes
    *
    * @param[in] id The detector's unique id.
    * @param[in] usage Information how the detector is used
    * @param[in] lanes A sequence of lanes the detector covers (must form a continuous piece)
    * @param[in] startPos The position of the detector start on the first lane the detector is placed at
    * @param[in] endPos The position of the detector end on the last lane the detector is placed at
    * @param[in] haltingTimeThreshold The time a vehicle's speed must be below haltingSpeedThreshold to be assigned as jammed
    * @param[in] haltingSpeedThreshold The speed a vehicle's speed must be below to be assigned as jammed
    * @param[in] jamDistThreshold The distance between two vehicles in order to not count them to one jam
    * @param[in] vTypes Vehicle types, that the detector takes into account
    */
    MSE2Collector(const std::string& id,
                  DetectorUsage usage, std::vector<MSLane*> lanes, double startPos, double endPos,
                  SUMOTime haltingTimeThreshold, double haltingSpeedThreshold, double jamDistThreshold,
                  const std::string& vTypes);


    /// @brief Destructor
    virtual ~MSE2Collector();

    /** @brief Returns the detector's usage type
     *
     * @see DetectorUsage
     * @return How the detector is used.
     */
    virtual DetectorUsage getUsageType() const {
        return myUsage;
    }



    /// @name Methods inherited from MSMoveReminder
    /// @{

    /** @brief Adds/removes vehicles from the list of vehicles to regard
     *
     * As soon as the reported vehicle enters the detector area (position>myStartPos)
     *  it is added to the list of vehicles to regard (myKnownVehicles). It
     *  is removed from this list if it leaves the detector (position<length>myEndPos).
     * The method returns true as long as the vehicle is not beyond the detector.
     *
     * @param[in] veh The vehicle in question.
     * @param[in] oldPos Position before the move-micro-timestep.
     * @param[in] newPos Position after the move-micro-timestep.
     *                   Note that this position is given in reference
     *                   to the begin of the entry lane of the vehicle.
     * @param[in] newSpeed Unused here.
     * @return False, if vehicle passed the detector entirely, else true.
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyMove
     */
    virtual bool notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos,
                            double newSpeed);


    /** @brief Removes a known vehicle due to its lane-change
     *
     * If the reported vehicle is known, it is removed from the list of
     *  vehicles to regard (myKnownVehicles).
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @see MSMoveReminder::notifyLeave
     */
    virtual bool notifyLeave(SUMOTrafficObject& veh, double lastPos, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    /** @brief Adds the vehicle to known vehicles if not beyond the dector
     *
     * If the vehicles is within the detector are, it is added to the list
     *  of known vehicles.
     * The method returns true as long as the vehicle is not beyond the detector.
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return False, if vehicle passed the detector entirely, else true.
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    virtual bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane);
    /// @}





    /// @name Methods inherited from MSDetectorFileOutput.
    /// @{

    /** @brief Computes the detector values in each time step
     *
     * This method should be called at the end of a simulation step, when
     *  all vehicles have moved. The current values are computed and
     *  summed up with the previous.
     *
     * @param[in] currentTime The current simulation time
     */
    virtual void detectorUpdate(const SUMOTime step);


    /** @brief Write the generated output to the given device
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @exception IOError If an error on writing occurs
     */
    virtual void writeXMLOutput(OutputDevice& dev, SUMOTime startTime, SUMOTime stopTime);


    /** @brief Open the XML-output
     *
     * The implementing function should open an xml element using
     *  OutputDevice::writeXMLHeader.
     *
     * @param[in] dev The output device to write the root into
     * @exception IOError If an error on writing occurs
     */
    virtual void writeXMLDetectorProlog(OutputDevice& dev) const;

    /// @}


    /** @brief Returns the begin position of the detector
     *
     * @return The detector's begin position
     */
    double getStartPos() const {
        return myStartPos;
    }


    /** @brief Returns the end position of the detector
     *
     * @return The detector's end position
     */
    double getEndPos() const {
        return myEndPos;
    }

    /** @brief Returns the length of the detector
     *
     * @return The detector's length
     */
    double getLength() const {
        return myDetectorLength;
    }


    /** @brief Returns the id of the detector's last lane
     *
     * @return The detector's end position
     */
    MSLane* getLastLane() const {
        return myLastLane;
    }


    /** @brief Returns a vector containing pointers to the lanes covered by the detector ordered from its first to its last lane
     */
    std::vector<MSLane*> getLanes();

    /** @brief Resets all values
     *
     * This method is called on initialisation and as soon as the values
     *  were written. Values for the next interval may be collected, then.
     * The list of known vehicles stays untouched.
     */
    virtual void reset();


    /// @name Methods returning current values
    /// @{

    /** @brief Returns the number of vehicles currently on the detector */
    int getCurrentVehicleNumber() const;

    /** @brief Returns the current detector occupancy */
    double getCurrentOccupancy() const {
        return myCurrentOccupancy;
    }

    /** @brief Returns the mean vehicle speed of vehicles currently on the detector*/
    double getCurrentMeanSpeed() const {
        return myCurrentMeanSpeed;
    }

    /** @brief Returns the mean vehicle length of vehicles currently on the detector*/
    double getCurrentMeanLength() const {
        return myCurrentMeanLength;
    }

    /** @brief Returns the current number of jams */
    int getCurrentJamNumber() const {
        return myCurrentJamNo;
    }

    /** @brief Returns the length in vehicles of the currently largest jam */
    int getCurrentMaxJamLengthInVehicles() const {
        return myCurrentMaxJamLengthInVehicles;
    }

    /** @brief Returns the length in meters of the currently largest jam */
    double getCurrentMaxJamLengthInMeters() const {
        return myCurrentMaxJamLengthInMeters;
    }

    /** @brief Returns the length of all jams in vehicles */
    int getCurrentJamLengthInVehicles() const {
        return myCurrentJamLengthInVehicles;
    }

    /** @brief Returns the length of all jams in meters */
    double getCurrentJamLengthInMeters() const {
        return myCurrentJamLengthInMeters;
    }

    /** @brief Returns the length of all jams in meters */
    int getCurrentStartedHalts() const {
        return myCurrentStartedHalts;
    }

    /** @brief Returns the number of current haltings within the area
    *
    * If no vehicle is within the area, 0 is returned.
    *
    * @return The mean number of haltings within the area
    */
    int getCurrentHaltingNumber() const {
        return myCurrentHaltingsNumber;
    }

    /** @brief Returns the IDs of the vehicles within the area
     *
     * @return The IDs of the vehicles that have passed the entry, but not yet an exit point
     */
    std::vector<std::string> getCurrentVehicleIDs() const;

    /** @brief Returns the VehicleInfos for the vehicles currently on the detector
     */
    std::vector<VehicleInfo*> getCurrentVehicles() const;

    /** \brief Returns the number of vehicles passed over the sensor (i.e. entered the sensor)
     *
     * @return number of cars passed over the sensor
     */
    int getPassedVeh() {
        return myNumberOfEnteredVehicles;
    }

    /** \brief Subtract the number of vehicles indicated from passed from the sensor count.
     *
     * @param[in] passed - int that indicates the number of vehicles to subtract
     */
    void subtractPassedVeh(int passed) {
        myNumberOfEnteredVehicles -= passed;
    }

    /// @}





    /// @name Estimation methods
    /// TODO: Need documentation, used for tls control in MSSOTLE2Sensors (->Daniel?)
    /// @{
    /** @brief Returns an estimate of the number of vehicles currently on the detector */
    int getEstimatedCurrentVehicleNumber(double speedThreshold) const;

    /** @brief Returns an estimate of the length of the queue of vehicles currently stopped on the detector */
    double getEstimateQueueLength() const;
    /// @}


    virtual void setVisible(bool /*show*/) {};

    /** @brief Remove all vehicles before quick-loading state */
    virtual void clearState();

private:

    /** @brief checks whether the vehicle stands in a jam
     *
     * @param[in] mni
     * @param[in/out] haltingVehicles
     * @param[in/out] intervalHaltingVehicles
     * @return Whether vehicle is in a jam.
     */
    bool checkJam(std::vector<MoveNotificationInfo*>::const_iterator mni, std::map<std::string, SUMOTime>& haltingVehicles, std::map<std::string, SUMOTime>& intervalHaltingVehicles);


    /** @brief Either adds the vehicle to the end of an existing jam, or closes the last jam, and/or creates a new jam
     *
     * @param isInJam
     * @param mni
     * @param[in/out] currentJam
     * @param[in/out] jams
     */
    void buildJam(bool isInJam, std::vector<MoveNotificationInfo*>::const_iterator mni, JamInfo*& currentJam, std::vector<JamInfo*>& jams);


    /** @brief Calculates aggregated values from the given jam structure, deletes all jam-pointers
     *
     * @param jams
     */
    void processJams(std::vector<JamInfo*>& jams, JamInfo* currentJam);

    /** @brief Calculates the time spent on the detector in the last step and the timeloss suffered in the last step for the given vehicle
     *
     * @param[in] veh Vehicle for which the values are to be calculated
     * @param[in] oldPos Last position (before the last timestep) of the vehicle relative to the beginning of its entry lane
     * @param[in] newPos Current position of the vehicle
     * @param[in] vi VehicleInfo corresponding to the vehicle
     * @param[in/out] timeOnDetector Total time spent on the detector during the last step
     * @param[in/out] timeLoss Total time loss suffered during the last integration step
     */
    void calculateTimeLossAndTimeOnDetector(const SUMOVehicle& veh, double oldPos, double newPos, const VehicleInfo& vi, double& timeOnDetector, double& timeLoss) const;

    /** @brief Checks integrity of myLanes, adds internal-lane information, inits myLength, myFirstLane, myLastLane, myOffsets
     *         Called once at construction.
     *  @requires myLanes should form a continuous sequence.
     */
    void initAuxiliaries(std::vector<MSLane*>& lanes);

    /** @brief Adjusts positioning if the detector length is less than POSITION_EPS and tests some assertions
     */
    void checkPositioning(bool posGiven = false, double desiredLength = 0.);

    /** @brief Snaps value to snpPoint if they are closer than snapDist
     */
    static double snap(double value, double snapPoint, double snapDist);

    /** @brief Updates the detector length after myStartPos and myEndPos have been modified
     */
    void recalculateDetectorLength();



    /** @brief This is called if no lane sequence is given to the constructor. Builds myLanes from the given information.
     *          Also inits startPos (case dir=="bw") / endPos (case dir=="fw").
     *          Selects lanes heuristically if no unambiguous continuation exists.
     *
     *  @param[in] lane Lane, where the detector starts/ends
     *  @param[in] length Length of the detector
     *  @param[in] dir Direction of detector extension with value in {"fw", "bw"} (forward / backward)
     *                 If dir == "fw" lane is interpreted as corresponding to the start lane of the detector,
     *                 otherwise the lane is interpreted as the end lane.
     */
    std::vector<MSLane*> selectLanes(MSLane* endLane, double length, std::string dir);


    /** @brief This adds the detector as a MoveReminder to the associated lanes.
     */
    void addDetectorToLanes(std::vector<MSLane*>& lanes);


    /** @brief Aggregates and normalize some values for the detector output during detectorUpdate()
     */
    void aggregateOutputValues();


    /** @brief This updates the detector values and the VehicleInfo of a vehicle on the detector
     *          with the given MoveNotificationInfo generated by the vehicle during the last time step.
     *
     * @param[in/out] vi VehicleInfo corresponding to the notifying vehicle
     * @param[in] mni MoveNotification for the vehicle
     */
    void integrateMoveNotification(VehicleInfo* vi, const MoveNotificationInfo* mni);

    /** @brief Creates and returns a MoveNotificationInfo containing detector specific information on the vehicle's last movement
     *
     * @param veh The vehicle sending the notification
     * @param oldPos The vehicle's position before the last integration step
     * @param newPos The vehicle's position after the last integration step
     * @param newSpeed The vehicle's speed after the last integration step
     * @param vehInfo Info on the detector's memory of the vehicle
     * @return A MoveNotificationInfo containing quantities of interest for the detector
     */
    MoveNotificationInfo* makeMoveNotification(const SUMOVehicle& veh, double oldPos, double newPos, double newSpeed, const VehicleInfo& vehInfo) const;

    /** @brief Creates and returns a VehicleInfo (called at the vehicle's entry)
     *
     * @param veh The entering vehicle
     * @param enteredLane The entry lane
     * @return A vehicle info which can be used to store information about the vehicle's stay on the detector
     */
    VehicleInfo* makeVehicleInfo(const SUMOVehicle& veh, const MSLane* enteredLane) const;

    /** @brief Calculates the time loss for a segment with constant vmax
     *
     * @param timespan time needed to cover the segment
     * @param initialSpeed speed at segment entry
     * @param accel constant acceleration assumed during movement on the segment
     * @param vmax Maximal possible speed for the considered vehicle on the segment
     * @return Time loss (== MAX(timespan*(vmax - (initialSpeed + accel/2))/vmax), 0)
     */
    static double calculateSegmentTimeLoss(double timespan, double initialSpeed, double accel, double vmax);

    /** brief returns true if the vehicle corresponding to mni1 is closer to the detector end than the vehicle corresponding to mni2
     */
    static bool compareMoveNotification(MoveNotificationInfo* mni1, MoveNotificationInfo* mni2) {
        return mni1->distToDetectorEnd < mni2->distToDetectorEnd;
    }


private:

    /// @brief Information about how this detector is used
    DetectorUsage myUsage;

    /// @name Detector parameter
    /// @{
    /// @brief The detector's lane sequence
    std::vector<std::string> myLanes;
    /// @brief The distances of the lane-beginnings from the detector start-point
    std::vector<double> myOffsets;
    /// @brief The first lane of the detector's lane sequence
    MSLane* myFirstLane;
    /// @brief The last lane of the detector's lane sequence
    MSLane* myLastLane;
    /// @brief The position the detector starts at on the first lane
    double myStartPos;
    /// @brief The position the detector ends at on the last lane
    double myEndPos;
    /// @brief The total detector length
    double myDetectorLength;

    /// @brief A vehicle must driver slower than this to be counted as a part of a jam
    double myJamHaltingSpeedThreshold;
    /// @brief A vehicle must be that long beyond myJamHaltingSpeedThreshold to be counted as a part of a jam
    SUMOTime myJamHaltingTimeThreshold;
    /// @brief Two standing vehicles must be closer than this to be counted into the same jam
    double myJamDistanceThreshold;
    /// @}


    /// @name Container
    /// @{
    /// @brief List of informations about the vehicles currently on the detector
    VehicleInfoMap myVehicleInfos;

    /// @brief Temporal storage for notifications from vehicles that did call the
    ///        detector's notifyMove() in the last time step.
    std::vector<MoveNotificationInfo*> myMoveNotifications;

    /// @brief Keep track of vehicles that left the detector by a regular move along a junction (not lanechange, teleport, etc.)
    ///        and should be removed from myVehicleInfos after taking into account their movement. Non-longitudinal exits
    ///        are processed immediately in notifyLeave()
    std::set<std::string> myLeftVehicles;

    /// @brief Storage for halting durations of known vehicles (for halting vehicles)
    std::map<std::string, SUMOTime> myHaltingVehicleDurations;

    /// @brief Storage for halting durations of known vehicles (current interval)
    std::map<std::string, SUMOTime> myIntervalHaltingVehicleDurations;

    /// @brief Halting durations of ended halts [s]
    std::vector<SUMOTime> myPastStandingDurations;

    /// @brief Halting durations of ended halts for the current interval [s]
    std::vector<SUMOTime> myPastIntervalStandingDurations;
    /// @}



    /// @name Values generated for aggregated file output
    /// @{
    /// @brief The number of collected samples [time x vehicle] since the last reset
    double myVehicleSamples;
    /// @brief The total amount of all time losses [time x vehicle] since the last reset
    double myTotalTimeLoss;
    /// @brief The sum of collected vehicle speeds [m/s]
    double mySpeedSum;
    /// @brief The number of started halts [#]
    double myStartedHalts;
    /// @brief The sum of jam lengths [m]
    double myJamLengthInMetersSum;
    /// @brief The sum of jam lengths [#veh]
    int myJamLengthInVehiclesSum;
    /// @brief The current aggregation duration [#steps]
    int myTimeSamples;
    /// @brief The sum of occupancies [%]
    double myOccupancySum;
    /// @brief The maximum occupancy [%]
    double myMaxOccupancy;
    /// @brief The mean jam length [#veh]
    int myMeanMaxJamInVehicles;
    /// @brief The mean jam length [m]
    double myMeanMaxJamInMeters;
    /// @brief The max jam length [#veh]
    int myMaxJamInVehicles;
    /// @brief The max jam length [m]
    double myMaxJamInMeters;
    /// @brief The mean number of vehicles [#veh]
    int myMeanVehicleNumber;
    /// @}


    /// @name Values generated describing the current state
    /// @{
    /// @brief The number of vehicles, which have entered the detector since the last reset
    int myNumberOfEnteredVehicles;
    /// @brief The number of vehicles, present on the detector at the last reset
    int myNumberOfSeenVehicles;
    /// @brief The number of vehicles, which have left the detector since the last reset
    int myNumberOfLeftVehicles;
    /// @brief The maximal number of vehicles located on the detector simultaneously since the last reset
    int myMaxVehicleNumber;

    /// @brief The current vehicle samples
    double myCurrentVehicleSamples;
    /// @brief The current occupancy
    double myCurrentOccupancy;
    /// @brief The current mean speed
    double myCurrentMeanSpeed;
    /// @brief The current mean length
    double myCurrentMeanLength;
    /// @brief The current jam number
    int myCurrentJamNo;
    /// @brief the current maximum jam length in meters
    double myCurrentMaxJamLengthInMeters;
    /// @brief The current maximum jam length in vehicles
    int myCurrentMaxJamLengthInVehicles;
    /// @brief The overall jam length in meters
    double myCurrentJamLengthInMeters;
    /// @brief The overall jam length in vehicles
    int myCurrentJamLengthInVehicles;
    /// @brief The number of started halts in the last step
    int myCurrentStartedHalts;
    /// @brief The number of halted vehicles [#]
    int myCurrentHaltingsNumber;
    /// @}


private:
    /// @brief Invalidated copy constructor.
    MSE2Collector(const MSE2Collector&);

    /// @brief Invalidated assignment operator.
    MSE2Collector& operator=(const MSE2Collector&);
};
