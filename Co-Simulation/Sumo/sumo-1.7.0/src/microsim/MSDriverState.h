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
/// @file    MSDriverState.h
/// @author  Michael Behrisch
/// @date    Tue, 21 Apr 2015
///
// A class representing a vehicle driver's current mental state
/****************************************************************************/


/// @todo: check parameter admissibility in setter methods


#pragma once
#include <config.h>

#include <memory>
#include <utils/common/SUMOTime.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/// @class OUProcess
/// @brief An Ornstein-Uhlenbeck stochastic process
class OUProcess {
public:
    /// @brief constructor
    OUProcess(double initialState, double timeScale, double noiseIntensity);
    /// @brief destructor
    ~OUProcess();

    /// @brief evolve for a time step of length dt.
    void step(double dt);
    /// @brief static version of the step()
    static double step(double state, double dt, double timeScale, double noiseIntensity);

    /// @brief set the process' timescale to a new value
    void setTimeScale(double timeScale) {
        myTimeScale = timeScale;
    };

    /// @brief set the process' noise intensity to a new value
    void setNoiseIntensity(double noiseIntensity) {
        myNoiseIntensity = noiseIntensity;
    };

    /// @brief set the process' state to a new value
    void setState(double state) {
        myState = state;
    };

    inline double getNoiseIntensity() const {
        return myNoiseIntensity;
    };

    inline double getTimeScale() const {
        return myTimeScale;
    };


    /// @brief Obtain the current state of the process
    double getState() const;


    static std::mt19937* getRNG() {
        return &myRNG;
    }

private:
    /** @brief The current state of the process
     */
    double myState;

    /** @brief The time scale of the process
     */
    double myTimeScale;

    /** @brief The noise intensity of the process
     */
    double myNoiseIntensity;

    /// @brief Random generator for OUProcesses
    static std::mt19937 myRNG;
};


/// @class MSSimpleDriverState
/// @brief Provides an interface to an error whose fluctuation is controlled
///        via the driver's 'awareness', which can be controlled externally, @see MSDevice_ToC
class MSSimpleDriverState {

public:
    MSSimpleDriverState(MSVehicle* veh);
    virtual ~MSSimpleDriverState() {};


    /// @name Getter methods
    ///@{
    inline double getMinAwareness() const {
        return myMinAwareness;
    }

    inline double getInitialAwareness() const {
        return myInitialAwareness;
    }

    inline double getErrorTimeScaleCoefficient() const {
        return myErrorTimeScaleCoefficient;
    }

    inline double getErrorNoiseIntensityCoefficient() const {
        return myErrorNoiseIntensityCoefficient;
    }

    inline double getErrorTimeScale() const {
        return myError.getTimeScale();
    }

    inline double getErrorNoiseIntensity() const {
        return myError.getNoiseIntensity();
    }

    inline double getSpeedDifferenceErrorCoefficient() const {
        return mySpeedDifferenceErrorCoefficient;
    }

    inline double getHeadwayErrorCoefficient() const {
        return myHeadwayErrorCoefficient;
    }

    inline double getSpeedDifferenceChangePerceptionThreshold() const {
        return mySpeedDifferenceChangePerceptionThreshold;
    }

    inline double getHeadwayChangePerceptionThreshold() const {
        return myHeadwayChangePerceptionThreshold;
    }

    inline double getAwareness() const {
        return myAwareness;
    }

    inline double getMaximalReactionTime() const {
        return myMaximalReactionTime;
    }

    inline double getOriginalReactionTime() const {
        return myOriginalReactionTime;
    }

    inline double getActionStepLength() const {
        return myActionStepLength;
    }

    inline double getErrorState() const {
        return myError.getState();
    };
    ///@}


    /// @name Setter methods
    ///@{
    inline void setMinAwareness(const double value)  {
        myMinAwareness = value;
    }

    inline void setInitialAwareness(const double value)  {
        myInitialAwareness = value;
    }

    inline void setErrorTimeScaleCoefficient(const double value)  {
        myErrorTimeScaleCoefficient = value;
    }

    inline void setErrorNoiseIntensityCoefficient(const double value)  {
        myErrorNoiseIntensityCoefficient = value;
    }

    inline void setSpeedDifferenceErrorCoefficient(const double value)  {
        mySpeedDifferenceErrorCoefficient = value;
    }

    inline void setHeadwayErrorCoefficient(const double value)  {
        myHeadwayErrorCoefficient = value;
    }

    inline void setSpeedDifferenceChangePerceptionThreshold(const double value)  {
        mySpeedDifferenceChangePerceptionThreshold = value;
    }

    inline void setHeadwayChangePerceptionThreshold(const double value)  {
        myHeadwayChangePerceptionThreshold = value;
    }

    inline void setMaximalReactionTime(const double value)  {
        myMaximalReactionTime = value;
        updateReactionTime();
    }

    inline void setOriginalReactionTime(const double value)  {
        myOriginalReactionTime = value;
        updateReactionTime();
    }

    void setAwareness(const double value);

    inline void setErrorState(const double state) {
        myError.setState(state);
    };

    inline void setErrorTimeScale(const double value)  {
        myError.setTimeScale(value);
    }

    inline void setErrorNoiseIntensity(const double value)  {
        myError.setNoiseIntensity(value);
    }
    ///@}

    /// @brief Trigger updates for the errorProcess, assumed gaps, etc
    void update();


    /// @brief Update the assumed gaps to the known objects according to
    ///        the corresponding perceived speed differences.
    void updateAssumedGaps();

    /// @name Methods to obtain the current error quantities to be used by the car-following model
    /// @see TCIModel
    /// @{
//    /// @see myAccelerationError
//    inline double getAppliedAcceleration(double desiredAccel) {
//        return desiredAccel + myError.getState();
//    };

    /// @brief This method checks whether the errorneous speed difference that would be perceived for this step
    ///        differs sufficiently from the previously perceived to be actually perceived. If so, it sets the
    ///        flag myReactionFlag[objID]=true, which should be checked just after the call to this method because
    ///        it will be overwritten by subsequent calls.
    double getPerceivedSpeedDifference(const double trueSpeedDifference, const double trueGap, const void* objID = nullptr);
    /// @see myHeadwayPerceptionError
    double getPerceivedHeadway(const double trueGap, const void* objID = nullptr);
    /// @}

    inline void lockDebug() {
        myDebugLock = true;
    }

    inline void unlockDebug() {
        myDebugLock = false;
    }

    inline bool debugLocked() const {
        return myDebugLock;
    }

private:
    // @brief Update the current step duration
    void updateStepDuration();
    // Update the error process
    void updateError();
    // Update the reaction time (actionStepLength)
    void updateReactionTime();

private:

    /// @brief Vehicle corresponding to this driver state
    MSVehicle* myVehicle;

    /// @brief Driver's 'awareness' \in [0,1]
    double myAwareness;
    /// @brief Minimal value for 'awareness' \in [0,1]
    double myMinAwareness;
    /// @brief Initial value for 'awareness' \in [0,1]
    double myInitialAwareness;
    /// @brief Driver's 'error', @see TCI_Model
    OUProcess myError;
    /// @brief Coefficient controlling the impact of awareness on the time scale of the error process
    double myErrorTimeScaleCoefficient;
    /// @brief Coefficient controlling the impact of awareness on the noise intensity of the error process
    double myErrorNoiseIntensityCoefficient;

    /// @brief Scaling coefficients for the magnitude of errors
    double mySpeedDifferenceErrorCoefficient;
    double myHeadwayErrorCoefficient;
    /// @brief Thresholds above a change in the corresponding quantity is perceived.
    /// @note  In the comparison, we multiply the actual change amount by the current
    ///       gap to the object to reflect a more precise perception if the object is closer.
    double myHeadwayChangePerceptionThreshold;
    double mySpeedDifferenceChangePerceptionThreshold;
//    // @brief if a perception threshold is passed for some object, a flag is set to induce a reaction to the object
//    std::map<void*, bool> myReactionFlag;

    /// @brief Action step length (~current maximal reaction time) induced by awareness level
    /// @note  This interpolates linearly from myOriginalReactionTime for awareness==1
    ///        to myMaximalReactionTime for awareness==myMinAwareness
    double myActionStepLength;
    /// @brief Maximal reaction time (value set for the actionStepLength at awareness=1)
    double myOriginalReactionTime;
    /// @brief Maximal reaction time (value set for the actionStepLength at awareness=myMinAwareness)
    double myMaximalReactionTime;

    /// @name Variables for tracking update instants
    /// @see updateStepDuration()
    /// @{
    /// @brief Elapsed time since the last state update
    double myStepDuration;
    /// @brief Time point of the last state update
    double myLastUpdateTime;


    /// @brief The assumed gaps to different objects
    /// @todo: update each step to incorporate the assumed change given a specific speed difference
    std::map<const void*, double> myAssumedGap;
    /// @brief The last perceived speed differences to the corresponding objects
    std::map<const void*, double> myLastPerceivedSpeedDifference;
    /// @}

    /// @brief Used to prevent infinite loops in debugging outputs, @see followSpeed() and stopSpeed() (of MSCFModel_Krauss, e.g.)
    bool myDebugLock;
};





///** @class MSDriverState
// * @brief  An object representing a traffic item. Used for influencing
// *         the task demand of the TCI car-following model.
// * @see MSCFModel_TCI
// */
//class MSDriverState {
//
//protected:
//    /// @brief base class for VehicleCharacteristics, TLSCharacteristics, PedestrianCharacteristics, SpeedLimitCharacteristics, Junction Characteristics...
//    /// @see TrafficItemType, @see MSCFModel_TCI
//    struct MSTrafficItemCharacteristics {
//        inline virtual ~MSTrafficItemCharacteristics() {};
//    };
//
//    // @brief Types of traffic items, @see TrafficItem
//    enum MSTrafficItemType {
//        TRAFFIC_ITEM_VEHICLE,
//        TRAFFIC_ITEM_PEDESTRIAN,
//        TRAFFIC_ITEM_SPEED_LIMIT,
//        TRAFFIC_ITEM_JUNCTION
//    };
//
//    /** @class MSTrafficItem
//     * @brief  An object representing a traffic item. Used for influencing
//     *         the task demand of the TCI car-following model.
//     * @see MSCFModel_TCI
//     */
//    struct MSTrafficItem {
//        MSTrafficItem(MSTrafficItemType type, const std::string& id, std::shared_ptr<MSTrafficItemCharacteristics> data);
//        static std::hash<std::string> hash;
//        MSTrafficItemType type;
//        size_t id_hash;
//        std::shared_ptr<MSTrafficItemCharacteristics> data;
//        double remainingIntegrationTime;
//        double integrationDemand;
//        double latentDemand;
//    };
//
//    struct JunctionCharacteristics : MSTrafficItemCharacteristics {
//        JunctionCharacteristics(const MSJunction* junction, const MSLink* egoLink, double dist) :
//            junction(junction), approachingLink(egoLink), dist(dist) {};
//        const MSJunction* junction;
//        const MSLink* approachingLink;
//        double dist;
//    };
//
//    struct PedestrianCharacteristics : MSTrafficItemCharacteristics {
//        PedestrianCharacteristics(const MSPerson* pedestrian, double dist) :
//            pedestrian(pedestrian), dist(dist) {};
//        const MSPerson* pedestrian;
//        double dist;
//    };
//
//    struct SpeedLimitCharacteristics : MSTrafficItemCharacteristics {
//        SpeedLimitCharacteristics(const MSLane* lane, double dist, double limit) :
//            dist(dist), limit(limit), lane(lane) {};
//        const MSLane* lane;
//        double dist;
//        double limit;
//    };
//
//    struct VehicleCharacteristics : MSTrafficItemCharacteristics {
//        VehicleCharacteristics(const MSVehicle* foe, double longitudinalDist, double lateralDist, double relativeSpeed) :
//            longitudinalDist(longitudinalDist), lateralDist(lateralDist), foe(foe), relativeSpeed(relativeSpeed) {};
//        const MSVehicle* foe;
//        double longitudinalDist;
//        double lateralDist;
//        double relativeSpeed;
//    };
//
//
//public:
//
//    MSDriverState(MSVehicle* veh);
//    virtual ~MSDriverState() {};
//
//    /// @name Interfaces to inform Driver Model about traffic items, which potentially
//    ///       influence the driving difficulty.
//    /// @{
//    /** @brief Informs about leader.
//    */
//    virtual void registerLeader(const MSVehicle* leader, double gap, double relativeSpeed, double latGap = -1.);
//
//    /** @brief Informs about pedestrian.
//    */
//    virtual void registerPedestrian(const MSPerson* pedestrian, double gap);
//
//    /** @brief Informs about upcoming speed limit reduction.
//    */
//    virtual void registerSpeedLimit(const MSLane* lane, double speedLimit, double dist);
//
//    /** @brief Informs about upcoming junction.
//    */
//    virtual void registerJunction(MSLink* link, double dist);
//
//    /** @brief Takes into account vehicle-specific factors for the driving demand
//    *          For instance, whether vehicle drives on an opposite direction lane, absolute speed, etc.
//    */
//    virtual void registerEgoVehicleState();
//
//    /** @brief Trigger updates for the state variables according to the traffic situation
//     *         (present traffic items)
//    */
//    virtual void update();
//    /// @}
//
//
//    /// @name Methods to obtain the current error quantities to be used by the car-following model
//    /// @see TCIModel
//    /// @{
//    /// @see myAccelerationError
//    inline double getAppliedAcceleration(double desiredAccel) {
//        return desiredAccel + myAccelerationError.getState();
//    };
//    /// @see mySpeedPerceptionError
//    inline double getPerceivedSpeedDifference(double trueSpeedDifference) {
//        return trueSpeedDifference + mySpeedPerceptionError.getState();
//    };
//    /// @see myHeadwayPerceptionError
//    inline double getPerceivedHeadway(double trueGap) {
//        return trueGap + myHeadwayPerceptionError.getState();
//    };
//    /// @see myActionStepLength
//    inline double getActionStepLength(){
//        return myActionStepLength;
//    };
//    /// @}
//
//
//private:
//
//    /** @brief Updates the internal variables to track the time between
//     *        two calls to the state update (i.e., two action points). Needed for a consistent
//     *        evolution of the error processes.
//     */
//    void updateStepDuration();
//
//    /** @brief Calculates a value for the task difficulty given the capability and the demand
//     *         and stores the result in myCurrentDrivingDifficulty.
//     *  @see difficultyFunction()
//     */
//    void calculateDrivingDifficulty();
//
//
//    /** @brief Transformation of the quotient demand/capability to obtain the actual
//     *         difficulty value used to control driving performance.
//     *  @note  The current implementation is a continuous piecewise affine function.
//     *         It has two regions with different slopes. A slight ascend, where the capability
//     *         is larger than the demand and a region of steeper ascend, where the demand
//     *         exceeds the capability.
//     */
//    double difficultyFunction(double demandCapabilityQuotient) const;
//
//
//    /** @brief Updates the myTaskCapability in dependence of the myTaskDifficulty to model a reactive
//     *         level of attention. The characteristics of the process are determined by myHomeostasisDifficulty
//     *         and myCapabilityTimeScale.
//     *  @todo Review the implementation as simple exponential process.
//     */
//    void adaptTaskCapability();
//
//
//    /// @name Updater for error processes.
//    /// @{
//    void updateAccelerationError();
//    void updateSpeedPerceptionError();
//    void updateHeadwayPerceptionError();
//    void updateActionStepLength();
//    /// @}
//
//
//    /// @brief Updates the given error process
//    void updateErrorProcess(OUProcess& errorProcess, double timeScaleCoefficient, double noiseIntensityCoefficient) const;
//
//    /// @brief Initialize newly appeared traffic item
//    void calculateLatentDemand(std::shared_ptr<MSTrafficItem> ti) const;
//
//    /// @brief Calculate demand induced by the given junction
//    double calculateLatentJunctionDemand(std::shared_ptr<JunctionCharacteristics> ch) const;
//    /// @brief Calculate demand induced by the given pedestrian
//    double calculateLatentPedestrianDemand(std::shared_ptr<PedestrianCharacteristics> ch) const;
//    /// @brief Calculate demand induced by the given pedestrian
//    double calculateLatentSpeedLimitDemand(std::shared_ptr<SpeedLimitCharacteristics> ch) const;
//    /// @brief Calculate demand induced by the given vehicle
//    double calculateLatentVehicleDemand(std::shared_ptr<VehicleCharacteristics> ch) const;
//
//    /// @brief Calculate integration demand induced by the given junction
//    double calculateJunctionIntegrationDemand(std::shared_ptr<JunctionCharacteristics> ch) const;
//    /// @brief Calculate integration demand induced by the given pedestrian
//    double calculatePedestrianIntegrationDemand(std::shared_ptr<PedestrianCharacteristics> ch) const;
//    /// @brief Calculate integration demand induced by the given pedestrian
//    double calculateSpeedLimitIntegrationDemand(std::shared_ptr<SpeedLimitCharacteristics> ch) const;
//    /// @brief Calculate integration demand induced by the given vehicle
//    double calculateVehicleIntegrationDemand(std::shared_ptr<VehicleCharacteristics> ch) const;
//
//    /// @brief Register known traffic item to persist
//    void updateItemIntegration(std::shared_ptr<MSTrafficItem> ti) const;
//
//    /// @brief Determine the integration demand and duration for a newly encountered traffic item (updated in place)
//    ///        The integration demand takes effect during a short period after the first appearance of the item.
//    void calculateIntegrationDemandAndTime(std::shared_ptr<MSTrafficItem> ti) const;
//
//    /// @brief Calculate the integration time for an item approached with the given speed at given dist
//    double calculateIntegrationTime(double dist, double speed) const;
//
//    /// @brief Incorporate the item's demand into the total task demand.
//    void integrateDemand(std::shared_ptr<MSTrafficItem> ti);
//
//    /// @brief Called whenever the vehicle is notified about a traffic item encounter.
//    void registerTrafficItem(std::shared_ptr<MSTrafficItem> ti);
//
//private:
//
//    MSVehicle* myVehicle;
//
//    /// @name Variables for tracking update instants
//    /// @see updateStepDuration()
//    /// @{
//
//    /// @brief Elapsed time since the last state update
//    double myStepDuration;
//    /// @brief Time point of the last state update
//    double myLastUpdateTime;
//
//    /// @}
//
//
//    /// @name Dynamical quantities for the driving performance
//    /// @{
//
//    /** @brief Task capability (combines static and dynamic factors specific to the driver and the situation,
//     *         total capability, attention, etc.). Follows myTaskDemand with some inertia (task-difficulty-homeostasis).
//     */
//    double myTaskCapability;
//    double myMinTaskCapability, myMaxTaskCapability;
//
//    /** @brief Task Demand (dynamic variable representing the total demand imposed on the driver by the driving situation and environment.
//     *         For instance, number, novelty and type of traffic participants in neighborhood, speed differences, road curvature,
//     *         headway to leader, number of lanes, traffic density, street signs, traffic lights)
//     */
//    double myTaskDemand;
//    double myMaxTaskDemand;
//
//    /** @brief Cached current value of the difficulty resulting from the combination of task capability and demand.
//     *  @see calculateDrivingDifficulty()
//     */
//    double myCurrentDrivingDifficulty;
//    /// @brief Upper bound for myCurrentDrivingDifficulty
//    double myMaxDifficulty;
//    /** @brief Slopes for the dependence of the difficulty on the quotient of demand and capability.
//     *  @see   difficultyFunction();
//     */
//    double mySubCriticalDifficultyCoefficient, mySuperCriticalDifficultyCoefficient;
//
//    /// @}
//
//    /// @name Field that reflect the current driving situation
//    /// @{
//    /// @brief Whether vehicle is driving on an opposite direction lane
//    bool myAmOpposite;
//    double myCurrentSpeed;
//    double myCurrentAcceleration;
//    /// @}
//
//    /// @name Parameters for the dynamic adaptation of capability (attention) and demand
//    /// @{
//
//    /** @brief The desired value of the quotient myTaskDemand/myTaskCapability. Influences the fixed point of the
//     *         process myTaskCapability -> myTaskDemand, @see adaptTaskCapability()
//     */
//    double myHomeostasisDifficulty;
//
//    /** @brief Determines the time scale for the adaptation process of task capability towards the
//     *         task difficulty.
//     */
//    double myCapabilityTimeScale;
//
//    /** @brief Factor for the demand if driving on an opposite direction lane
//     */
//    double myOppositeDirectionDrivingDemandFactor;
//
//    /// @}
//
//
//
//    /** @brief Traffic items in the current neighborhood of the vehicle.
//     */
//    std::map<size_t, std::shared_ptr<MSTrafficItem> > myTrafficItems;
//    std::map<size_t, std::shared_ptr<MSTrafficItem> > myNewTrafficItems;
//
//    /// @name Actuation errors
//    /// @{
//
//    /** @brief Acceleration error. Modelled as an Ornstein-Uhlenbeck process.
//     *  @see updateAccelerationError()
//     */
//    OUProcess myAccelerationError;
//    /// @brief Coefficient controlling the impact of driving difficulty on the time scale of the acceleration error process
//    double myAccelerationErrorTimeScaleCoefficient;
//    /// @brief Coefficient controlling the impact of driving difficulty on the noise intensity of the acceleration error process
//    double myAccelerationErrorNoiseIntensityCoefficient;
//
//    /// @brief Action step length (increases with task difficulty, is similar to reaction time)
//    double myActionStepLength;
//    /// @brief Proportionality factor of myActionStepLength and driving difficulty
//    double myActionStepLengthCoefficient;
//    /// @brief Bounds for the action step length
//    double myMinActionStepLength, myMaxActionStepLength;
//
//    /// @}
//
//
//    /// @name Perception errors
//    /// @{
//
//    /** @brief Error of estimation of the relative speeds of neighboring vehicles
//     */
//    OUProcess mySpeedPerceptionError;
//    /// @brief Coefficient controlling the impact of driving difficulty on the time scale of the relative speed error process
//    double mySpeedPerceptionErrorTimeScaleCoefficient;
//    /// @brief Coefficient controlling the impact of driving difficulty on the noise intensity of the relative speed error process
//    double mySpeedPerceptionErrorNoiseIntensityCoefficient;
//
//    /** @brief Error of estimation of the distance/headways of neighboring vehicles
//     */
//    OUProcess myHeadwayPerceptionError;
//    /// @brief Coefficient controlling the impact of driving difficulty on the time scale of the headway error process
//    double myHeadwayPerceptionErrorTimeScaleCoefficient;
//    /// @brief Coefficient controlling the impact of driving difficulty on the noise intensity of the headway error process
//    double myHeadwayPerceptionErrorNoiseIntensityCoefficient;
//
//    /// @}
//};



/// @brief Default values for the MSDriverState parameters
struct DriverStateDefaults {
//    static double myMinTaskCapability;
//    static double myMaxTaskCapability;
//    static double myMaxTaskDemand;
//    static double myMaxDifficulty;
//    static double mySubCriticalDifficultyCoefficient;
//    static double mySuperCriticalDifficultyCoefficient;
//    static double myHomeostasisDifficulty;
//    static double myCapabilityTimeScale;
//    static double myAccelerationErrorTimeScaleCoefficient;
//    static double myAccelerationErrorNoiseIntensityCoefficient;
//    static double myActionStepLengthCoefficient;
//    static double myMinActionStepLength;
//    static double myMaxActionStepLength;
//    static double mySpeedPerceptionErrorTimeScaleCoefficient;
//    static double mySpeedPerceptionErrorNoiseIntensityCoefficient;
//    static double myHeadwayPerceptionErrorTimeScaleCoefficient;
//    static double myHeadwayPerceptionErrorNoiseIntensityCoefficient;
//    static double myOppositeDirectionDrivingFactor;

    // for MSSimpleDriverState
    static double minAwareness;
    static double initialAwareness;
    static double errorTimeScaleCoefficient;
    static double errorNoiseIntensityCoefficient;
    static double speedDifferenceErrorCoefficient;
    static double speedDifferenceChangePerceptionThreshold;
    static double headwayChangePerceptionThreshold;
    static double headwayErrorCoefficient;
    static double maximalReactionTimeFactor;
};
