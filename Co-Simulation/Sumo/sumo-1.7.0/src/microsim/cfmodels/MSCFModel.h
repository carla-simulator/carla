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
/// @file    MSCFModel.h
/// @author  Tobias Mayer
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 27 Jul 2009
///
// The car-following model abstraction
/****************************************************************************/
#pragma once
#include <config.h>

#include <cmath>
#include <string>
#include <utils/common/StdDefs.h>
#include <utils/common/FileHelpers.h>

#define INVALID_SPEED 299792458 + 1 // nothing can go faster than the speed of light!
// Factor that the minimum emergency decel is increased by in corresponding situations
#define EMERGENCY_DECEL_AMPLIFIER 1.2

// ===========================================================================
// class declarations
// ===========================================================================
class MSVehicleType;
class MSVehicle;
class MSLane;
class MSPerson;
class MSLink;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSCFModel
 * @brief The car-following model abstraction
 *
 * MSCFModel is an interface for different car following Models to implement.
 * It provides methods to compute a vehicles velocity for a simulation step.
 */
class MSCFModel {

public:

    class VehicleVariables {
    public:
        virtual ~VehicleVariables();
    };

    /** @brief Constructor
     *  @param[in] vtype the type for which this model is built and also the parameter object to configure this model
     */
    MSCFModel(const MSVehicleType* vtype);


    /// @brief Destructor
    virtual ~MSCFModel();


    /// @name Methods to override by model implementation
    /// @{

    /** @brief Applies interaction with stops and lane changing model
     * influences. Called at most once per simulation step (exactcly once per action step)
     * @param[in] veh The ego vehicle
     * @param[in] vPos The possible velocity
     * @return The velocity after applying interactions with stops and lane change model influences
     */
    virtual double finalizeSpeed(MSVehicle* const veh, double vPos) const;


    /// @brief apply custom speed adaptations within the given speed bounds
    virtual double patchSpeedBeforeLC(const MSVehicle* veh, double vMin, double vMax) const {
        UNUSED_PARAMETER(veh);
        UNUSED_PARAMETER(vMin);
        return vMax;
    }


    /** @brief Computes the vehicle's safe speed without a leader
     *
     * Returns the velocity of the vehicle in dependence to the length of the free street and the target
     *  velocity at the end of the free range. If onInsertion is true, the vehicle may still brake
     *  before the next movement.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] seen The look ahead distance
     * @param[in] maxSpeed The maximum allowed speed
     * @param[in] onInsertion whether speed at insertion is asked for
     * @return EGO's safe speed
     */
    virtual double freeSpeed(const MSVehicle* const veh, double speed, double seen,
                             double maxSpeed, const bool onInsertion = false) const;


    /** @brief Computes the vehicle's follow speed (no dawdling)
     *
     * Returns the velocity of the vehicle in dependence to the vehicle's and its leader's values and the distance between them.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @return EGO's safe speed
     */
    virtual double followSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const = 0;


    /** @brief Computes the vehicle's safe speed (no dawdling)
     * This method is used during the insertion stage. Whereas the method
     * followSpeed returns the desired speed which may be lower than the safe
     * speed, this method only considers safety constraints
     *
     * Returns the velocity of the vehicle in dependence to the vehicle's and its leader's values and the distance between them.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @return EGO's safe speed
     */
    virtual double insertionFollowSpeed(const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel, const MSVehicle* const pred = 0) const;


    /** @brief Computes the vehicle's safe speed for approaching a non-moving obstacle (no dawdling)
     *
     * Returns the velocity of the vehicle when approaching a static object (such as the end of a lane) assuming no reaction time is needed.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap The (netto) distance to the the obstacle
     * @return EGO's safe speed for approaching a non-moving obstacle
     * @todo generic Interface, models can call for the values they need
     */
    virtual double stopSpeed(const MSVehicle* const veh, const double speed, double gap) const = 0;


    /** @brief Computes the vehicle's safe speed for approaching an obstacle at insertion without constraints
     *         due to acceleration capabilities and previous speeds.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap The (netto) distance to the the obstacle
     * @return EGO's safe speed for approaching a non-moving obstacle at insertion
     * @see stopSpeed() and insertionFollowSpeed()
     *
     */
    virtual double insertionStopSpeed(const MSVehicle* const veh, double speed, double gap) const;

    /** @brief Computes the vehicle's follow speed that avoids a collision for the given amount of time
     *
     * Returns the velocity of the vehicle in dependence to the vehicle's and its leader's values and the distance between them.
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] predSpeed The speed of LEADER
     * @param[in] predMaxDecel The maximum leader decelration
     * @return EGO's safe speed
     */
    virtual double followSpeedTransient(double duration, const MSVehicle* const veh, double speed, double gap2pred, double predSpeed, double predMaxDecel) const;

    /** @brief Returns the maximum gap at which an interaction between both vehicles occurs
     *
     * "interaction" means that the LEADER influences EGO's speed.
     * @param[in] veh The EGO vehicle
     * @param[in] vL LEADER's speed
     * @return The interaction gap
     * @todo evaluate signature
     */
    virtual double interactionGap(const MSVehicle* const veh, double vL) const;


    /** @brief Returns the model's ID; the XML-Tag number is used
     * @return The model's ID
     */
    virtual int getModelID() const = 0;


    /** @brief Duplicates the car-following model
     * @param[in] vtype The vehicle type this model belongs to (1:1)
     * @return A duplicate of this car-following model
     */
    virtual MSCFModel* duplicate(const MSVehicleType* vtype) const = 0;


    /** @brief Returns model specific values which are stored inside a vehicle
     * and must be used with casting
     */
    virtual VehicleVariables* createVehicleVariables() const {
        return 0;
    }
    /// @}


    /** @brief Get the vehicle type's maximum acceleration [m/s^2]
     * @return The maximum acceleration (in m/s^2) of vehicles of this class
     */
    inline double getMaxAccel() const {
        return myAccel;
    }


    /** @brief Get the vehicle type's maximal comfortable deceleration [m/s^2]
     * @return The maximal comfortable deceleration (in m/s^2) of vehicles of this class
     */
    inline double getMaxDecel() const {
        return myDecel;
    }


    /** @brief Get the vehicle type's maximal phisically possible deceleration [m/s^2]
     * @return The maximal physically possible deceleration (in m/s^2) of vehicles of this class
     */
    inline double getEmergencyDecel() const {
        return myEmergencyDecel;
    }


    /** @brief Get the vehicle type's apparent deceleration [m/s^2] (the one regarded by its followers
     * @return The apparent deceleration (in m/s^2) of vehicles of this class
     */
    inline double getApparentDecel() const {
        return myApparentDecel;
    }

    /** @brief Get the factor of minGap that must be maintained to avoid a collision event
     */
    inline double getCollisionMinGapFactor() const {
        return myCollisionMinGapFactor;
    }


    /// @name Virtual methods with default implementation
    /// @{

    /** @brief Get the driver's imperfection
     * @return The imperfection of drivers of this class
     */
    virtual double getImperfection() const {
        return -1;
    }


    /** @brief Get the driver's desired headway [s]
     * @return The desired headway of this class' drivers in s
     */
    virtual double getHeadwayTime() const {
        return myHeadwayTime;
    }
    /// @}




    /// @name Currently fixed methods
    /// @{

    /** @brief Returns the maximum speed given the current speed
     *
     * The implementation of this method must take into account the time step
     *  duration.
     *
     * Justification: Due to air brake or other influences, the vehicle's next maximum
     *  speed may depend on the vehicle's current speed (given).
     *
     * @param[in] speed The vehicle's current speed
     * @param[in] veh The vehicle itself, for obtaining other values
     * @return The maximum possible speed for the next step
     */
    virtual double maxNextSpeed(double speed, const MSVehicle* const veh) const;


    /** @brief Returns the minimum speed given the current speed
     * (depends on the numerical update scheme and its step width)
     * Note that it wouldn't have to depend on the numerical update
     * scheme if the semantics would rely on acceleration instead of velocity.
     *
     * @param[in] speed The vehicle's current speed
     * @param[in] speed The vehicle itself, for obtaining other values, if needed as e.g. road conditions.
     * @return The minimum possible speed for the next step
     */
    virtual double minNextSpeed(double speed, const MSVehicle* const veh = 0) const;

    /** @brief Returns the minimum speed after emergency braking, given the current speed
     * (depends on the numerical update scheme and its step width)
     * Note that it wouldn't have to depend on the numerical update
     * scheme if the semantics would rely on acceleration instead of velocity.
     *
     * @param[in] speed The vehicle's current speed
     * @param[in] speed The vehicle itself, for obtaining other values, if needed as e.g. road conditions.
     * @return The minimum possible speed for the next step
     */
    virtual double minNextSpeedEmergency(double speed, const MSVehicle* const veh = 0) const;


    /** @brief Returns the distance the vehicle needs to halt including driver's reaction time tau (i.e. desired headway),
     * assuming that during the reaction time, the speed remains constant
     * @param[in] speed The vehicle's current speed
     * @return The distance needed to halt
     */
    inline double brakeGap(const double speed) const {
        return brakeGap(speed, myDecel, myHeadwayTime);
    }

    static double brakeGap(const double speed, const double decel, const double headwayTime);

    static double brakeGapEuler(const double speed, const double decel, const double headwayTime);

    static double freeSpeed(const double currentSpeed, const double decel, const double dist, const double maxSpeed, const bool onInsertion, const double actionStepLength);

    /** @brief Returns the minimum gap to reserve if the leader is braking at maximum (>=0)
     * @param[in] veh The vehicle itself, for obtaining other values
     * @param[in] pred The leader vehicle, for obtaining other values
     * @param[in] speed EGO's speed
     * @param[in] leaderSpeed LEADER's speed
     * @param[in] leaderMaxDecel LEADER's max. deceleration rate
     */
    inline virtual double getSecureGap(const MSVehicle* const /*veh*/, const MSVehicle* const /*pred*/, const double speed, const double leaderSpeed, const double leaderMaxDecel) const {
        // The solution approach leaderBrakeGap >= followerBrakeGap is not
        // secure when the follower can brake harder than the leader because the paths may still cross.
        // As a workaround we use a value of leaderDecel which errs on the side of caution
        const double maxDecel = MAX2(myDecel, leaderMaxDecel);
        double secureGap = MAX2((double) 0, brakeGap(speed, myDecel, myHeadwayTime) - brakeGap(leaderSpeed, maxDecel, 0));
        return secureGap;
    }

    virtual /** @brief Returns the velocity after maximum deceleration
     * @param[in] v The velocity
     * @return The velocity after maximum deceleration
     */
    inline double getSpeedAfterMaxDecel(double v) const {
        return MAX2((double) 0, v - (double) ACCEL2SPEED(myDecel));
    }
    /// @}

    /** @brief Computes the minimal time needed to cover a distance given the desired speed at arrival.
     * @param[in] dist Distance to be covered
     * @param[in] currentSpeed Actual speed of vehicle
     * @param[in] arrivalSpeed Desired speed at arrival
     */
    SUMOTime getMinimalArrivalTime(double dist, double currentSpeed, double arrivalSpeed) const;


    /** @brief Computes the time needed to travel a distance dist given an initial speed
     *         and constant acceleration. The speed during traveling is assumed not to exceed the max speed.
     * @param[in] dist Distance to be covered (assumed >= 0.)
     * @param[in] speed Initial speed of vehicle
     * @param[in] accel Assumed acceleration until reaching maxspeed or speed=0.
     * @return Returns the estimated time needed to cover the given distance
     *         If distance will never be covered with the given parameters INVALID_DOUBLE (from MSLink.h) is returned.
     */
    static double estimateArrivalTime(double dist, double speed, double maxSpeed, double accel);

    /** @brief Computes the time needed to travel a distance dist given an initial speed, arrival speed,
     *         constant acceleration and deceleration. The speed during traveling is assumed not to exceed the max speed.
     * @param[in] dist Distance to be covered (assumed >= 0.)
     * @param[in] initialSpeed Initial speed of vehicle
     * @param[in] arrivalSpeed desired arrival speed of vehicle
     * @param[in] accel Assumed acceleration until reaching maxspeed.
     * @param[in] accel Assumed deceleration until reaching targetspeed.
     * @return Returns the estimated time needed to cover the given distance
     *         If distance will never be covered with the given parameters INVALID_DOUBLE (from MSLink.h) is returned.
     * @note Currently, this is still a stub for actually very special situations in LC context:
     *       It is assumed that 0==initialSpeed==arrivalSpeed<=maxspeed, accel==decel>0 (because currently
     *       this is only used for lane change purposes, where lateral accel == lateral decel)
     */
    static double estimateArrivalTime(double dist, double initialSpeed, double arrivalSpeed, double maxSpeed, double accel, double decel);

    /** @brief Computes the acceleration needed to arrive not before the given time
     * @param[in] dist - the distance of the critical point
     * @param[in] time - the time after which an arrival at dist is allowed
     * @param[in] speed - the current speed
     * @return Returns the acceleration which would ensure an arrival at distance dist earliest for the given time
     */
    static double avoidArrivalAccel(double dist, double time, double speed, double maxDecel);


    /** @brief Computes the minimal possible arrival speed after covering a given distance
     * @param[in] dist Distance to be covered
     * @param[in] currentSpeed Actual speed of vehicle
     */
    double getMinimalArrivalSpeed(double dist, double currentSpeed) const;

    /** @brief Computes the minimal possible arrival speed after covering a given distance for Euler update
     * @param[in] dist Distance to be covered
     * @param[in] currentSpeed Actual speed of vehicle
     */
    double getMinimalArrivalSpeedEuler(double dist, double currentSpeed) const;


    /** @brief return the resulting gap if, starting with gap currentGap, two vehicles
     * continue with constant accelerations (velocities bounded by 0 and maxSpeed) for
     * a given timespan of length 'duration'.
     * @param[in] currentGap (pos(veh1) - pos(veh2) at start)
     * @param[in] v1 initial speed of vehicle 1
     * @param[in] v2 initial speed of vehicle 2
     * @param[in] a1 acceleration of vehicle 1
     * @param[in] a2 acceleration of vehicle 2
     * @param[in] maxV1 maximal speed of vehicle 1
     * @param[in] maxV2 maximal speed of vehicle 2
     * @param[in] duration time span for the process
     * @return estimated gap after 'duration' seconds
     */
    static double gapExtrapolation(const double duration, const double currentGap, double v1,  double v2, double a1 = 0, double a2 = 0, const double maxV1 = std::numeric_limits<double>::max(), const double maxV2 = std::numeric_limits<double>::max());

    /**
     * @brief Calculates the time at which the position passedPosition has been passed
     *         In case of a ballistic update, the possibility of a stop within a time step
     *         requires more information about the last time-step than in case of the euler update
     *         to determine the last position if the currentSpeed is zero.
     * @param[in] lastPos the position at time t=0 (must be < currentPos)
     * @param[in] passedPos the position for which the passing time is to be determined (has to lie within [lastPos, currentPos]!)
     * @param[in] currentPos the position at time t=TS (one time-step after lastPos) (must be > lastPos)
     * @param[in] lastSpeed the speed at moment t=0
     * @param[in] currentSpeed the speed at moment t=TS
     * @return  time t in [0,TS] at which passedPos in [lastPos, currentPos] was passed.
     */
    static double passingTime(const double lastPos, const double passedPos, const double currentPos, const double lastSpeed, const double currentSpeed);



    /**
     * @brief Calculates the speed after a time t \in [0,TS]
     *        given the initial speed and the distance traveled in an interval of step length TS.
     * @note  If the acceleration were known, this would be much nicer, but in this way
     *        we need to reconstruct it (for the ballistic update at least, where we assume that
     *        a stop may occur within the interval)
     * @param[in] t time in [0,TS] for which the speed shall be determined
     * @param[in] oldSpeed speed before the last time step (referred to as t == 0)
     * @param[in] distance covered
     * @return    speed at time t
     */
    static double speedAfterTime(const double t, const double oldSpeed, const double dist);


    /// @brief calculates the distance travelled after accelerating for time t
    static double distAfterTime(double t, double speed, double accel);



    /* @brief estimate speed while accelerating for the given distance
     * @param[in] dist The distance during which accelerating takes place
     * @param[in] v The initial speed
     * @param[in] accel The acceleration
     * XXX affected by ticket #860 (the formula is invalid for the Euler position update rule)
     * XXX (Leo) Migrated estimateSpeedAfterDistance() to MSCFModel from MSVehicle as Jakob suggested (removed inline property, because myType is fw-declared)
     */
    double estimateSpeedAfterDistance(const double dist, const double v, const double accel) const;

    /// @name Setter methods
    /// @{

    /** @brief Sets a new value for maximum acceleration [m/s^2]
     * @param[in] accel The new acceleration in m/s^2
     */
    virtual void setMaxAccel(double accel) {
        myAccel = accel;
    }


    /** @brief Sets a new value for maximal comfortable deceleration [m/s^2]
     * @param[in] decel The new deceleration in m/s^2
     */
    virtual void setMaxDecel(double decel) {
        myDecel = decel;
    }


    /** @brief Sets a new value for maximal physically possible deceleration [m/s^2]
     * @param[in] decel The new deceleration in m/s^2
     */
    virtual void setEmergencyDecel(double decel) {
        myEmergencyDecel = decel;
    }


    /** @brief Sets a new value for the apparent deceleration [m/s^2]
     * @param[in] decel The new deceleration in m/s^2
     */
    virtual void setApparentDecel(double decel) {
        myApparentDecel = decel;
    }


    /** @brief Sets a new value for driver imperfection
     * @param[in] accel The new driver imperfection
     */
    virtual void setImperfection(double imperfection) {
        UNUSED_PARAMETER(imperfection);
    }


    /** @brief Sets a new value for desired headway [s]
     * @param[in] headwayTime The new desired headway (in s)
     */
    virtual void setHeadwayTime(double headwayTime) {
        myHeadwayTime = headwayTime;
    }
    /// @}

    /** @brief Returns the maximum safe velocity for following the given leader
     * @param[in] gap2pred The (netto) distance to the LEADER
     * @param[in] egoSpeed The FOLLOWERS's speed
     * @param[in] predSpeed The LEADER's speed
     * @param[in] predMaxDecel The LEADER's maximum deceleration
     * @param[in] onInsertion Indicator whether the call is triggered during vehicle insertion
     * @return the safe velocity
     */
    double maximumSafeFollowSpeed(double gap,  double egoSpeed, double predSpeed, double predMaxDecel, bool onInsertion = false) const;


    /** @brief Returns the minimal deceleration for following the given leader safely
     * @param[in] gap The (netto) distance to the LEADER
     * @param[in] egoSpeed The FOLLOWERS's speed
     * @param[in] predSpeed The LEADER's speed
     * @param[in] predMaxDecel The LEADER's maximum deceleration
     * @return The minimal deceleration b>0 that, if applied constantly until a full stop,
     *         asserts that the vehicle does not crash into the leader.
     * @note   If b > predMaxDecel, this function actually does not calculate the tangency for the trajectories, i.e. a double root for the gap,
     *         but applies a simpler approach following the spirit of maximumSafeFollowSpeed, where the
     *         leader's decel is assumed as maximum of its actual value and the followers decel.
     */
    double calculateEmergencyDeceleration(double gap, double egoSpeed, double predSpeed, double predMaxDecel) const;


    /** @brief Returns the maximum next velocity for stopping within gap
     * @param[in] gap The (netto) distance to the desired stopping point
     * @param[in] currentSpeed The current speed of the ego vehicle
     * @param[in] onInsertion Indicator whether the call is triggered during vehicle insertion
     * @param[in] headway The desired time headway to be included in the calculations (default argument -1 induces the use of myHeadway)
     */
    double maximumSafeStopSpeed(double gap, double currentSpeed, bool onInsertion = false, double headway = -1) const;


    /** @brief Returns the maximum next velocity for stopping within gap
     * when using the semi-implicit Euler update
     * @param[in] gap The (netto) distance to the LEADER
     */
    double maximumSafeStopSpeedEuler(double gap, double headway = -1) const;


    /** @brief Returns the maximum next velocity for stopping within gap
     * when using the ballistic positional update.
     * @note This takes into account the driver's reaction time tau (i.e. the desired headway) and the car's current speed.
     * (The latter is required to calculate the distance covered in the following timestep.)
     * @param[in] gap The (netto) distance to the desired stopping point
     * @param[in] currentSpeed The current speed of the ego vehicle
     * @param[in] onInsertion Indicator whether the call is triggered during vehicle insertion
     * @param[in] headway The desired time headway to be included in the calculations (default argument -1 induces the use of myHeadway)
     * @return the safe velocity (to be attained at the end of the following time step) that assures the possibility of stopping within gap.
     * If a negative value is returned, the required stop has to take place before the end of the time step.
     */
    double maximumSafeStopSpeedBallistic(double gap, double currentSpeed, bool onInsertion = false, double headway = -1) const;

    /**
     * @brief try to get the given parameter for this carFollowingModel
     *
     * @param[in] veh the vehicle from which the parameter must be retrieved
     * @param[in] key the key of the parameter
     * @return the value of the requested parameter
     */
    virtual std::string getParameter(const MSVehicle* veh, const std::string& key) const {
        UNUSED_PARAMETER(veh);
        UNUSED_PARAMETER(key);
        return "";
    }

    /**
     * @brief try to set the given parameter for this carFollowingModel
     *
     * @param[in] veh the vehicle for which the parameter must be set
     * @param[in] key the key of the parameter
     * @param[in] value the value to be set for the given parameter
     */
    virtual void setParameter(MSVehicle* veh, const std::string& key, const std::string& value) const {
        UNUSED_PARAMETER(veh);
        UNUSED_PARAMETER(key);
        UNUSED_PARAMETER(value);
    }

protected:

    /** @brief Overwrites gap2pred and predSpeed by the perceived values obtained from the vehicle's driver state,
     *  @see MSCFModel_Krauss::stopSpeed() and MSCFModel_Krauss::followSpeed() for integration into a CF model
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in, out] gap2pred The (netto) distance to the LEADER
     * @param[in, out] predSpeed The speed of LEADER
     * @param[in] pred The leading vehicle (LEADER)
     */
    void applyHeadwayAndSpeedDifferencePerceptionErrors(const MSVehicle* const veh, double speed, double& gap, double& predSpeed, double predMaxDecel, const MSVehicle* const pred) const;

    /** @brief Overwrites gap by the perceived value obtained from the vehicle's driver state
     * @param[in] veh The vehicle (EGO)
     * @param[in] speed The vehicle's speed
     * @param[in, out] gap The (netto) distance to the the obstacle
     */
    void applyHeadwayPerceptionError(const MSVehicle* const veh, double speed, double& gap) const;


protected:
    /// @brief The type to which this model definition belongs to
    const MSVehicleType* myType;

    /// @brief The vehicle's maximum acceleration [m/s^2]
    double myAccel;

    /// @brief The vehicle's maximum deceleration [m/s^2]
    double myDecel;
    /// @brief The vehicle's maximum emergency deceleration [m/s^2]
    double myEmergencyDecel;
    /// @brief The vehicle's deceleration as expected by surrounding traffic [m/s^2]
    double myApparentDecel;
    /// @brief The factor of minGap that must be maintained to avoid a collision event
    double myCollisionMinGapFactor;

    /// @brief The driver's desired time headway (aka reaction time tau) [s]
    double myHeadwayTime;



};



