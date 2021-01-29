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
/// @file    MSDevice_ToC.h
/// @author  Leonhard Luecken
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    01.04.2018
///
// The ToC Device controls the transition of control between automated and manual driving.
//
/****************************************************************************/
#pragma once
#include <config.h>

#include <random>
#include <queue>
#include "MSVehicleDevice.h"
#include <utils/common/SUMOTime.h>
#include <utils/common/WrappingCommand.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;
class MSVehicle;
class Command_ToCTrigger;
class Command_ToCProcess;
class RGBColor;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_ToC
 *
 * @brief The ToC Device controls transition of control between automated and manual driving.
 *
 * @see MSDevice
 */
class MSDevice_ToC : public MSVehicleDevice {
private:
    // All currently existing ToC device instances
    static std::set<MSDevice_ToC*, ComparatorNumericalIdLess> myInstances;
    // All files, that receive ToC output (TODO: check if required)
    static std::set<std::string> createdOutputFiles;

    struct OpenGapParams {
        double newTimeHeadway;
        double newSpaceHeadway;
        double changeRate;
        double maxDecel;
        bool active;
        OpenGapParams(double timegap, double spacing, double changeRate, double maxDecel, bool active) :
            newTimeHeadway(timegap), newSpaceHeadway(spacing), changeRate(changeRate), maxDecel(maxDecel), active(active)
        {};
    };

public:
    /** @brief Inserts MSDevice_ToC-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a ToC-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);

    /** @brief returns all currently existing ToC devices
     */
    static const std::set<MSDevice_ToC*, ComparatorNumericalIdLess>& getInstances() {
        return myInstances;
    };

    /** @brief Closes root tags of output files
     */
    static void cleanup();

private:

    /// @brief Enum describing the different regimes for the device, @see myState
    ///        Possible transitions:
    ///        AUTOMATED -> PREPARING_TOC
    ///        PREPARING_TOC -> PERFORMING_MRM
    ///        PREPARING_TOC -> MANUAL
    ///        PERFORMING_MRM -> MANUAL
    ///        MANUAL -> AUTOMATED
    enum ToCState {
        UNDEFINED = 0,
        MANUAL = 1,
        AUTOMATED = 2,
        PREPARING_TOC = 3, // this applies only to the transition AUTOMATED -> MANUAL !
        MRM = 4,
        RECOVERING = 5
    };

    /// @name Helpers for parameter parsing
    /// @{
    static double getDynamicMRMProbability(const SUMOVehicle& v, const OptionsCont& oc);
    static std::string getOutputFilename(const SUMOVehicle& v, const OptionsCont& oc);
    static OpenGapParams getOpenGapParams(const SUMOVehicle& v, const OptionsCont& oc);

    static ToCState _2ToCState(const std::string&);
    static std::string _2string(ToCState state);
    /// @}


public:
    /// @brief Destructor.
    ~MSDevice_ToC();

    /// @brief return the name for this type of device
    const std::string deviceName() const override {
        return "toc";
    }

    /// @brief Return value indicates whether the device still wants to be notified about the vehicle movement
    bool notifyMove(SUMOTrafficObject& veh,
                    double oldPos,
                    double newPos,
                    double newSpeed) override;

    /// @brief try to retrieve the given parameter from this device. Throw exception for unsupported key
    std::string getParameter(const std::string& key) const override;

    /// @brief try to set the given parameter for this device. Throw exception for unsupported key
    void setParameter(const std::string& key, const std::string& value) override;


    /// @brief Trigger execution of an MRM
    SUMOTime triggerMRM(SUMOTime t);

    /// @brief Trigger execution of a ToC X-->AUTOMATED ("upwards")
    SUMOTime triggerUpwardToC(SUMOTime t);

    /// @brief Trigger execution of a ToC X-->MANUAL ("downwards")
    SUMOTime triggerDownwardToC(SUMOTime t);

    /// @brief Continue the ToC preparation for one time step
    SUMOTime ToCPreparationStep(SUMOTime t);

    /// @brief Continue the MRM for one time step
    SUMOTime MRMExecutionStep(SUMOTime t);

    /// @brief Continue the awareness recovery for one time step
    SUMOTime awarenessRecoveryStep(SUMOTime t);

    /// @brief Write output to file given by option device.toc.file
    void writeOutput();

    /// @brief Whether this device requested to write output
    bool generatesOutput() {
        return myOutputFile != nullptr;
    }

    static std::mt19937* getResponseTimeRNG() {
        return &myResponseTimeRNG;
    }
private:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     * @param[in] file The file to write the device's output to
     * @param[in] manualType vType that models manual driving
     * @param[in] automatedType vType that models automated driving
     * @param[in] responseTime time lapse until vType switch after request was received
     * @param[in] recoveryRate rate at which the awareness increases after the takeover
     * @param[in] lcAbstinence awareness level below which no lane changes are taken out
     * @param[in] initialAwareness value to which the awareness is set after takeover
     * @param[in] mrmDecel constant deceleration rate assumed to be applied during an MRM
     * @param[in] mrmKeepRight whether the vehicle tries to change to the right during an MRM
     * @param[in] mrmSafeSpot stopping place to reach during an MRM
     * @param[in] useColorScheme whether the color of the vehicle should be changed according to its current ToC-state
     * @param[in] ogp parameters for the openGap mechanism applied during ToC preparation phase
     */
    MSDevice_ToC(SUMOVehicle& holder, const std::string& id, const std::string& outputFilename,
                 const std::string& manualType, const std::string& automatedType, SUMOTime responseTime, double recoveryRate,
                 double lcAbstinence, double initialAwareness, double mrmDecel,
                 double dynamicToCThreshold, double dynamicMRMProbability, double maxPreparationAccel,
                 bool mrmKeepRight, const std::string& mrmSafeSpot, SUMOTime mrmSafeSpotDuration, bool useColorScheme, OpenGapParams ogp);

    /** @brief Initialize vehicle colors for different states
     *  @note  For MANUAL and AUTOMATED, the color of the given types are used,
     *         and for the other states hardcoded colors are given.
     */
    void initColorScheme();

    /// @brief Set the awareness to the given value
    void setAwareness(double value);

    /// @brief Set the ToC device's state
    void setState(ToCState state);

    // @brief Sets the device holder's color corresponding to the current state
    void setVehicleColor();

    /// @brief Request a ToC.
    ///        If the device is in AUTOMATED or MRM state, a driver response time is sampled
    ///        and the ToC is scheduled. If the response is larger than timeTillMRM,
    ///        an MRM is scheduled as well.
    ///        If the device is in MANUAL or UNDEFINED state, it switches to AUTOMATED.
    ///        The request is ignored if the state is already PREPARING_TOC.
    /// @param timeTillMRM
    /// @param responseTime If the default is given (== -1), the response time is sampled randomly,
    ///		   @see sampleResponseTime()
    void requestToC(SUMOTime timeTillMRM, SUMOTime responseTime = -1000);

    /// @brief Request an MRM to be initiated immediately. No downward ToC will be scheduled.
    /// @note  The initiated MRM process will run forever until a new ToC is requested.
    void requestMRM();

    /// @brief Switch the device holder's vehicle type
    void switchHolderType(const std::string& targetTypeID);

    /// @brief Break MRM Process or remove MRM-Trigger command from the event-queue.
    void descheduleMRM();
    /// @brief Remove scheduled ToC-Trigger command from the event-queue.
    void descheduleToC();
    /// @brief Remove ongoing ToC-Preparation process from the event-queue.
    void descheduleToCPreparation();
    /// @brief Remove ongoing awareness recovery process from the event-queue.
    void descheduleRecovery();

    /// @brief Resets the holder's LC mode to the last differing to LCModeMRM
    void resetDeliberateLCs();
    /// @brief Resets the holder's LC mode to the operational LC-mode of the ToC Device (@see LCModeMRM)
    void deactivateDeliberateLCs();

    /// @brief Whether the current operation mode is manual
    bool isManuallyDriven();
    /// @brief Whether the current operation mode is automated
    bool isAutomated();

    /// @brief Check if the vehicle should induce a ToC due to
    ///        internal reasons. That is, if the route cannot be followed
    ///        for more time than a given threshold, @see myDynamicToCThreshold
    bool checkDynamicToC();

private:
    /// @name private state members of the ToC device
    /// @{

    /// @brief vehicle type ID for manual driving
    std::string myManualTypeID;
    /// @brief vehicle type ID for automated driving
    std::string myAutomatedTypeID;


    /// @brief Average response time needed by the driver to take back control
    SUMOTime myResponseTime;
    /// @brief Recovery rate for the driver's awareness after a ToC
    double myRecoveryRate;
    /// @brief Level of the awareness below which no lane-changes are performed
    double myLCAbstinence;
    /// @brief Average awareness the driver has initially after a ToC
    double myInitialAwareness;

    /// @brief Deceleration rate applied during MRM
    double myMRMDecel;

    /// @brief Current awareness-level of the driver in [0,1]
    double myCurrentAwareness;

    /// @brief Coloring scheme, @see initColorScheme()
    std::map<ToCState, RGBColor> myColorScheme;

    /// @brief Whether a coloring scheme shall by applied to indicate the different toc stages, @see initColorScheme()
    bool myUseColorScheme;

    /// @brief Current state of the device
    ToCState myState;

    /// @}

    /// @brief The holder vehicle casted to MSVehicle*
    MSVehicle* myHolderMS;

    /// @name Commands sent to the EventControl (used for cleanup)
    /// @note Must be removed in destructor.
    /// @{
    WrappingCommand<MSDevice_ToC>* myTriggerMRMCommand;
    WrappingCommand<MSDevice_ToC>* myTriggerToCCommand;
    WrappingCommand<MSDevice_ToC>* myRecoverAwarenessCommand;
    WrappingCommand<MSDevice_ToC>* myExecuteMRMCommand;
    WrappingCommand<MSDevice_ToC>* myPrepareToCCommand;
    /// @}

    /// @brief The file the devices output goes to
    OutputDevice* myOutputFile;

    /// @brief Storage for events to be written to the output
    std::queue<std::pair<SUMOTime, std::string> > myEvents;

    /// @brief Storage for events to be written to the output
    std::queue<std::pair<std::string, double> > myEventLanes;

    /// @brief Storage for events to be written to the output
    std::queue<std::pair<double, double>> myEventXY;

    /// @brief LC mode overridden during MRM, stored for restoration
    int myPreviousLCMode;

    /// @brief LC mode operational during an MRM
    static int LCModeMRM;

    /// @brief Parameters for the openGap mechanism applied during ToC preparation phase
    OpenGapParams myOpenGapParams;

    /// @brief Duration in s. for which the vehicle needs to be able to follow its route without a lane change
    ///        to continue in automated mode (only has effect if dynamic ToCs are activated, @see myDynamicToCActive)
    double myDynamicToCThreshold;
    /// @brief Probability of an MRM to occur after a dynamically triggered ToC
    // (Note that these MRMs will not induce full stops in most cases)
    double myMRMProbability;
    /// @brief Switch for considering dynamic ToCs, @see myDynamicToCThreshold
    bool myDynamicToCActive;
    /// @brief Flag to indicate that a dynamically triggered ToC is in preparation
    bool myIssuedDynamicToC;
    /// @brief Lane, on which the ongoing dynamic ToC was issued. It can only be aborted if the lane was changed.
    int myDynamicToCLane;

    /// @brief Whether vehicle tries to change to the right during an MRM
    bool myMRMKeepRight;

    /// @brief stop vehicle tries to reach during MRM
    std::string myMRMSafeSpot;

    /// @brief duration at stop vehicle tries to reach during MRM
    SUMOTime myMRMSafeSpotDuration;

    /// @brief Maximal acceleration that may be applied during the ToC preparation phase
    /// TODO: Make effective
    double myMaxPreparationAccel;

    /// @brief Storage for original maximal acceleration of vehicle.
    double myOriginalMaxAccel;

    /// @brief Grid of the response time distribution.
    static std::vector<double> lookupResponseTimeMRMProbs;
    static std::vector<double> lookupResponseTimeLeadTimes;
    /// @brief Mean of the response time distribution. (Only depends on given lead time)
    static double responseTimeMean(double leadTime) {
        return MIN2(2 * sqrt(leadTime), 0.7 * leadTime);
    };
    /// @brief Variances of the response time distribution. Given the lead time and the MRM probability
    /// the variances in this table ensure that for the mean returned by responseTimeMean(leadTime)
    /// an MRM will occur with probability pMRM
    static std::vector<std::vector<double> > lookupResponseTimeVariances;

    /// @brief Random generator for ToC devices
    static std::mt19937 myResponseTimeRNG;

    /// @brief Samples a random driver response time from a truncated Gaussian with
    /// parameters according to the lookup tables
    double sampleResponseTime(double leadTime) const;

    /// @brief Two-dimensional interpolation of variance from lookup table
    /// assumes pMRM >= 0, leadTime >= 0
    static double interpolateVariance(double leadTime, double pMRM);

private:
    /// @brief Invalidated copy constructor.
    MSDevice_ToC(const MSDevice_ToC&);

    /// @brief Invalidated assignment operator.
    MSDevice_ToC& operator=(const MSDevice_ToC&);


};
