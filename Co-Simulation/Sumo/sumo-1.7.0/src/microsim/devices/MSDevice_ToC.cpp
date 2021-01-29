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
/// @file    MSDevice_ToC.cpp
/// @author  Leonhard Luecken
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    01.04.2018
///
// The ToC Device controls the transition of control between automated and manual driving.
//
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <memory>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/common/WrappingCommand.h>
#include <utils/common/RGBColor.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSRouteHandler.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSDriverState.h>
#include <microsim/MSStoppingPlace.h>
#include "MSDevice_ToC.h"


// ===========================================================================
// debug constants
// ===========================================================================
//#define DEBUG_TOC
//#define DEBUG_DYNAMIC_TOC

// ===========================================================================
// parameter defaults
// ===========================================================================

// default value for the average response time, that a driver needs to take back control
#define DEFAULT_RESPONSE_TIME -1.0
// default value for the average rate at which the driver's awareness recovers to
// 1.0 after a ToC has been performed
#define DEFAULT_RECOVERY_RATE 0.1
// Default value of the awareness below which no lane-changes are performed
#define DEFAULT_LCABSTINENCE 0.0
// The default value for the average awareness a driver has initially after a ToC
#define DEFAULT_INITIAL_AWARENESS 0.5
// The default value for the deceleration rate applied during a 'minimum risk maneuver'
#define DEFAULT_MRM_DECEL 1.5
// The default value for the dynamic ToC threshold indicates that the dynamic ToCs are deactivated
#define DEFAULT_DYNAMIC_TOC_THRESHOLD 0.0
// The default value for the probability of an MRM to occur after a dynamically triggered ToC
// (Note that these MRMs will not induce full stops in most cases)
#define DEFAULT_MRM_PROBABILITY 0.05

// The factor by which the dynamic ToC threshold time is multiplied to yield the lead time given for the corresponding ToC
#define DYNAMIC_TOC_LEADTIME_FACTOR 0.75
// A factor applied to the check for the dynamic ToC condition to resist aborting an ongoing dynamic ToC (and prevent oscillations)
#define DYNAMIC_TOC_ABORT_RESISTANCE_FACTOR 2.0


// The default values for the openGap parameters applied for gap creation in preparation for a ToC
#define DEFAULT_OPENGAP_TIMEGAP -1.0
#define DEFAULT_OPENGAP_SPACING 0.0
#define DEFAULT_OPENGAP_CHANGERATE 1.0
#define DEFAULT_OPENGAP_MAXDECEL 1.0


#define DEFAULT_MANUAL_TYPE ""
#define DEFAULT_AUTOMATED_TYPE ""

// Maximal tries to sample a positive value from the gaussian distribution
// used for the driver response time when a TOR is issued. (the distribution is assumed truncated at zero)
#define MAX_RESPONSETIME_SAMPLE_TRIES 100
// Maximal variance of responsetimes (returned for pMRM outside lookup table, i.e. pMRM>0.5), see interpolateVariance()
#define MAX_RESPONSETIME_VARIANCE 10000


// ---------------------------------------------------------------------------
// static members
// ---------------------------------------------------------------------------
std::set<MSDevice_ToC*, ComparatorNumericalIdLess> MSDevice_ToC::myInstances = std::set<MSDevice_ToC*, ComparatorNumericalIdLess>();
std::set<std::string> MSDevice_ToC::createdOutputFiles;
int MSDevice_ToC::LCModeMRM = 768; // = 0b001100000000 - no autonomous changes, no speed adaptation
std::mt19937 MSDevice_ToC::myResponseTimeRNG;


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_ToC::insertOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("ToC Device");
    insertDefaultAssignmentOptions("toc", "ToC Device", oc);

    oc.doRegister("device.toc.manualType", new Option_String());
    oc.addDescription("device.toc.manualType", "ToC Device", "Vehicle type for manual driving regime.");
    oc.doRegister("device.toc.automatedType", new Option_String());
    oc.addDescription("device.toc.automatedType", "ToC Device", "Vehicle type for automated driving regime.");
    oc.doRegister("device.toc.responseTime", new Option_Float(DEFAULT_RESPONSE_TIME));
    oc.addDescription("device.toc.responseTime", "ToC Device", "Average response time needed by a driver to take back control.");
    oc.doRegister("device.toc.recoveryRate", new Option_Float(DEFAULT_RECOVERY_RATE));
    oc.addDescription("device.toc.recoveryRate", "ToC Device", "Recovery rate for the driver's awareness after a ToC.");
    oc.doRegister("device.toc.lcAbstinence", new Option_Float(DEFAULT_LCABSTINENCE));
    oc.addDescription("device.toc.lcAbstinence", "ToC Device", "Attention level below which a driver restrains from performing lane changes (value in [0,1]).");
    oc.doRegister("device.toc.initialAwareness", new Option_Float(DEFAULT_INITIAL_AWARENESS));
    oc.addDescription("device.toc.initialAwareness", "ToC Device", "Average awareness a driver has initially after a ToC (value in [0,1]).");
    oc.doRegister("device.toc.mrmDecel", new Option_Float(DEFAULT_MRM_DECEL));
    oc.addDescription("device.toc.mrmDecel", "ToC Device", "Deceleration rate applied during a 'minimum risk maneuver'.");
    oc.doRegister("device.toc.dynamicToCThreshold", new Option_Float(DEFAULT_DYNAMIC_TOC_THRESHOLD));
    oc.addDescription("device.toc.dynamicToCThreshold", "ToC Device", "Time, which the vehicle requires to have ahead to continue in automated mode. The default value of 0 indicates no dynamic triggering of ToCs.");
    oc.doRegister("device.toc.dynamicMRMProbability", new Option_Float(DEFAULT_MRM_PROBABILITY));
    oc.addDescription("device.toc.dynamicMRMProbability", "ToC Device", "Probability that a dynamically triggered TOR is not answered in time.");
    oc.doRegister("device.toc.mrmKeepRight", new Option_Bool(false));
    oc.addDescription("device.toc.mrmKeepRight", "ToC Device", "If true, the vehicle tries to change to the right during an MRM.");
    oc.doRegister("device.toc.mrmSafeSpot", new Option_String());
    oc.addDescription("device.toc.mrmSafeSpot", "ToC Device", "If set, the vehicle tries to reach the given named stopping place during an MRM.");
    oc.doRegister("device.toc.mrmSafeSpotDuration", new Option_Float(60.));
    oc.addDescription("device.toc.mrmSafeSpotDuration", "ToC Device", "Duration the vehicle stays at the safe spot after an MRM.");
    oc.doRegister("device.toc.maxPreparationAccel", new Option_Float(0.0));
    oc.addDescription("device.toc.maxPreparationAccel", "ToC Device", "Maximal acceleration that may be applied during the ToC preparation phase.");
    oc.doRegister("device.toc.ogNewTimeHeadway", new Option_Float(-1.0));
    oc.addDescription("device.toc.ogNewTimeHeadway", "ToC Device", "Timegap for ToC preparation phase.");
    oc.doRegister("device.toc.ogNewSpaceHeadway", new Option_Float(-1.0));
    oc.addDescription("device.toc.ogNewSpaceHeadway", "ToC Device", "Additional spacing for ToC preparation phase.");
    oc.doRegister("device.toc.ogMaxDecel", new Option_Float(-1.0));
    oc.addDescription("device.toc.ogMaxDecel", "ToC Device", "Maximal deceleration applied for establishing increased gap in ToC preparation phase.");
    oc.doRegister("device.toc.ogChangeRate", new Option_Float(-1.0));
    oc.addDescription("device.toc.ogChangeRate", "ToC Device", "Rate of adaptation towards the increased headway during ToC preparation.");
    oc.doRegister("device.toc.useColorScheme", new Option_Bool(true));
    oc.addDescription("device.toc.useColorScheme", "ToC Device", "Whether a coloring scheme shall by applied to indicate the different ToC stages.");
    oc.doRegister("device.toc.file", new Option_String());
    oc.addDescription("device.toc.file", "ToC Device", "Switches on output by specifying an output filename.");
}


void
MSDevice_ToC::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "toc", v, false)) {
        if (MSGlobals::gUseMesoSim) {
            WRITE_WARNING("ToC device is not supported by the mesoscopic simulation.");
            return;
        }
        const std::string manualType = getStringParam(v, oc, "toc.manualType", DEFAULT_MANUAL_TYPE, true);
        const std::string automatedType = getStringParam(v, oc, "toc.automatedType", DEFAULT_AUTOMATED_TYPE, true);
        const SUMOTime responseTime = TIME2STEPS(getFloatParam(v, oc, "toc.responseTime", DEFAULT_RESPONSE_TIME, false));
        const double recoveryRate = getFloatParam(v, oc, "toc.recoveryRate", DEFAULT_RECOVERY_RATE, false);
        const double lcAbstinence = getFloatParam(v, oc, "toc.lcAbstinence", DEFAULT_LCABSTINENCE, false);
        const double initialAwareness = getFloatParam(v, oc, "toc.initialAwareness", DEFAULT_INITIAL_AWARENESS, false);
        const double mrmDecel = getFloatParam(v, oc, "toc.mrmDecel", DEFAULT_MRM_DECEL, false);
        const bool useColoring = getBoolParam(v, oc, "toc.useColorScheme", "false", false);
        const std::string deviceID = "toc_" + v.getID();
        const std::string file = getOutputFilename(v, oc);
        const OpenGapParams ogp = getOpenGapParams(v, oc);
        const double dynamicToCThreshold = getFloatParam(v, oc, "toc.dynamicToCThreshold", DEFAULT_DYNAMIC_TOC_THRESHOLD, false);
        const double dynamicMRMProbability = getDynamicMRMProbability(v, oc);
        const bool mrmKeepRight = getBoolParam(v, oc, "toc.mrmKeepRight", false, false);
        const std::string mrmSafeSpot = getStringParam(v, oc, "toc.mrmSafeSpot", "", false);
        const SUMOTime mrmSafeSpotDuration = TIME2STEPS(getFloatParam(v, oc, "toc.mrmSafeSpotDuration", 60., false));
        const double maxPreparationAccel = getFloatParam(v, oc, "toc.maxPreparationAccel", 0.0, false);
        // build the device
        MSDevice_ToC* device = new MSDevice_ToC(v, deviceID, file,
                                                manualType, automatedType, responseTime, recoveryRate,
                                                lcAbstinence, initialAwareness, mrmDecel, dynamicToCThreshold,
                                                dynamicMRMProbability, maxPreparationAccel, mrmKeepRight,
                                                mrmSafeSpot, mrmSafeSpotDuration, useColoring, ogp);
        into.push_back(device);
    }
}


std::string
MSDevice_ToC::getOutputFilename(const SUMOVehicle& v, const OptionsCont& oc) {
    // Default of "" means no output
    std::string file = "";
    if (v.getParameter().knowsParameter("device.toc.file")) {
        try {
            file = v.getParameter().getParameter("device.toc.file", file);
        } catch (...) {
            WRITE_WARNING("Invalid value '" + v.getParameter().getParameter("device.toc.file", file) + "'for vehicle parameter 'ssm.measures'");
        }
    } else if (v.getVehicleType().getParameter().knowsParameter("device.toc.file")) {
        try {
            file = v.getVehicleType().getParameter().getParameter("device.toc.file", file);
        } catch (...) {
            WRITE_WARNING("Invalid value '" + v.getVehicleType().getParameter().getParameter("device.toc.file", file) + "'for vType parameter 'ssm.measures'");
        }
    } else {
        file = oc.getString("device.toc.file") == "" ? file : oc.getString("device.toc.file");
    }
    return file;
}


double
MSDevice_ToC::getDynamicMRMProbability(const SUMOVehicle& v, const OptionsCont& oc) {
    double pMRM = getFloatParam(v, oc, "toc.dynamicMRMProbability", DEFAULT_MRM_PROBABILITY, false);
    if (pMRM < 0 || pMRM > 0.5) {
        const double pMRMTrunc = MAX2(0.0, MIN2(0.5, pMRM));
        WRITE_WARNING("Given value for ToC device parameter 'dynamicMRMProbability' (=" + toString(pMRM) + ") is not in the admissible range [0,0.5]. Truncated to " + toString(pMRMTrunc) + ".");
        return pMRMTrunc;
    }
    return pMRM;
}


MSDevice_ToC::OpenGapParams
MSDevice_ToC::getOpenGapParams(const SUMOVehicle& v, const OptionsCont& oc) {
    double timegap = getFloatParam(v, oc, "toc.ogNewTimeHeadway", -1.0, false);
    double spacing = getFloatParam(v, oc, "toc.ogNewSpaceHeadway", -1.0, false);
    double changeRate = getFloatParam(v, oc, "toc.ogChangeRate", -1.0, false);
    double maxDecel = getFloatParam(v, oc, "toc.ogMaxDecel", -1.0, false);
    bool specifiedAny = false;

    if (changeRate == -1.0) {
        changeRate = DEFAULT_OPENGAP_CHANGERATE;
    } else {
        specifiedAny = true;
    }
    if (maxDecel == -1.0) {
        maxDecel = DEFAULT_OPENGAP_MAXDECEL;
    } else {
        specifiedAny = true;
    }
    if (specifiedAny && timegap == -1 && spacing == -1) {
        WRITE_ERROR("If any openGap parameters for the ToC model are specified, then at least one of toc.ogNewTimeHeadway and toc.ogNewSpaceHeadway must be defined.")
    }
    if (timegap == -1) {
        timegap = DEFAULT_OPENGAP_TIMEGAP;
    } else {
        specifiedAny = true;
    }
    if (spacing == -1) {
        spacing = DEFAULT_OPENGAP_SPACING;
    } else {
        specifiedAny = true;
    }
#ifdef DEBUG_TOC
    std::cout << "Parsed openGapParams: \n"
              << "  timegap=" << timegap
              << ", spacing=" << spacing
              << ", changeRate=" << changeRate
              << ", maxDecel=" << maxDecel
              << std::endl;
#endif
    return OpenGapParams(timegap, spacing, changeRate, maxDecel, specifiedAny);
}

// ---------------------------------------------------------------------------
// MSDevice_ToC-methods
// ---------------------------------------------------------------------------
MSDevice_ToC::MSDevice_ToC(SUMOVehicle& holder, const std::string& id, const std::string& outputFilename,
                           const std::string& manualType, const std::string& automatedType, SUMOTime responseTime, double recoveryRate,
                           double lcAbstinence, double initialAwareness, double mrmDecel,
                           double dynamicToCThreshold, double dynamicMRMProbability, double maxPreparationAccel,
                           bool mrmKeepRight, const std::string& mrmSafeSpot, SUMOTime mrmSafeSpotDuration, bool useColorScheme, OpenGapParams ogp) :
    MSVehicleDevice(holder, id),
    myManualTypeID(manualType),
    myAutomatedTypeID(automatedType),
    myResponseTime(responseTime),
    myRecoveryRate(recoveryRate),
    myLCAbstinence(lcAbstinence),
    myInitialAwareness(initialAwareness),
    myMRMDecel(mrmDecel),
    myCurrentAwareness(1.),
    myUseColorScheme(useColorScheme),
    myTriggerMRMCommand(nullptr),
    myTriggerToCCommand(nullptr),
    myRecoverAwarenessCommand(nullptr),
    myExecuteMRMCommand(nullptr),
    myPrepareToCCommand(nullptr),
    myOutputFile(nullptr),
    myEvents(),
    myEventLanes(),
    myEventXY(),
    myPreviousLCMode(-1),
    myOpenGapParams(ogp),
    myDynamicToCThreshold(dynamicToCThreshold),
    myMRMProbability(dynamicMRMProbability),
    myDynamicToCActive(dynamicToCThreshold > 0),
    myIssuedDynamicToC(false),
    myDynamicToCLane(-1),
    myMRMKeepRight(mrmKeepRight),
    myMRMSafeSpot(mrmSafeSpot),
    myMRMSafeSpotDuration(mrmSafeSpotDuration),
    myMaxPreparationAccel(maxPreparationAccel),
    myOriginalMaxAccel(-1) {
    // Take care! Holder is currently being constructed. Cast occurs before completion.
    myHolderMS = static_cast<MSVehicle*>(&holder);

    if (outputFilename != "") {
        myOutputFile = &OutputDevice::getDevice(outputFilename);
        // TODO: make xsd, include header
        // myOutputFile.writeXMLHeader("ToCDeviceLog", "ToCDeviceLog.xsd");
        if (createdOutputFiles.count(outputFilename) == 0) {
            myOutputFile->writeXMLHeader("ToCDeviceLog", "");
            createdOutputFiles.insert(outputFilename);
        }
    }

    // Check if the given vTypes for the ToC Device are vTypeDistributions
    MSVehicleControl& vehCtrl = MSNet::getInstance()->getVehicleControl();
    const bool automatedVTypeIsDist = vehCtrl.hasVTypeDistribution(myAutomatedTypeID);
    const bool manualVTypeIsDist = vehCtrl.hasVTypeDistribution(myManualTypeID);

    // Check if the vType of the holder matches one of the given vTypes
    std::string holderVTypeID = holder.getVehicleType().getID();
    if (holderVTypeID == myManualTypeID) {
        myState = ToCState::MANUAL;
    } else if (holderVTypeID == myAutomatedTypeID) {
        myState = ToCState::AUTOMATED;
    } else if (manualVTypeIsDist && vehCtrl.getVTypeDistributionMembership(holderVTypeID).count(myManualTypeID) > 0) {
        // Holder type id is from the given manual type distribution.
        myState = ToCState::MANUAL;
        myManualTypeID = holderVTypeID;
    } else if (automatedVTypeIsDist && vehCtrl.getVTypeDistributionMembership(holderVTypeID).count(myAutomatedTypeID) > 0) {
        // Holder type id is from the given automated type distribution.
        myState = ToCState::AUTOMATED;
        myAutomatedTypeID = holderVTypeID;
    } else {
        throw ProcessError("Vehicle type of vehicle '" + holder.getID() + "' ('" +
                           holder.getVehicleType().getID() + "') must coincide with manualType ('" +
                           manualType + "') or automatedType ('" + automatedType +
                           "') specified for its ToC-device (or drawn from the specified vTypeDistributions).");
    }
    if (!vehCtrl.hasVType(myAutomatedTypeID)) {
        throw ProcessError("The automated vehicle type '" + myAutomatedTypeID +
                           "' of vehicle '" + holder.getID() + "' is not known.");
    }
    if (!vehCtrl.hasVType(myManualTypeID)) {
        throw ProcessError("The manual vehicle type '" + myManualTypeID +
                           "' of vehicle '" + holder.getID() + "' is not known.");
    }

    // Eventually instantiate given vTypes from distributions
    if (myState == ToCState::MANUAL && automatedVTypeIsDist) {
        myAutomatedTypeID = vehCtrl.getVType(myAutomatedTypeID, MSRouteHandler::getParsingRNG())->getID();
    } else if (myState == ToCState::AUTOMATED && manualVTypeIsDist) {
        myManualTypeID = vehCtrl.getVType(myManualTypeID, MSRouteHandler::getParsingRNG())->getID();
    }

    // register at static instance container
    myInstances.insert(this);
    initColorScheme();

#ifdef DEBUG_TOC
    std::cout << "initialized device '" << id << "' with "
              << "outputFilename=" << outputFilename << ", "
              << "myManualType=" << myManualTypeID << ", "
              << "myAutomatedType=" << myAutomatedTypeID << ", "
              << "myResponseTime=" << myResponseTime << ", "
              << "myRecoveryRate=" << myRecoveryRate << ", "
              << "myInitialAwareness=" << myInitialAwareness << ", "
              << "myMRMDecel=" << myMRMDecel << ", "
              << "ogTimeHeadway=" << myOpenGapParams.newTimeHeadway << ", "
              << "ogSpaceHeadway=" << myOpenGapParams.newSpaceHeadway << ", "
              << "ogChangeRate=" << myOpenGapParams.changeRate << ", "
              << "ogMaxDecel=" << myOpenGapParams.maxDecel << ", "
              << "ogActive=" << myOpenGapParams.active << ", "
              << "myCurrentAwareness=" << myCurrentAwareness << ", "
              << "myState=" << _2string(myState) << std::endl;
#endif

    assert(myInitialAwareness <= 1.0 && myInitialAwareness >= 0.0);
}



void
MSDevice_ToC::initColorScheme() {
    //RGBColor(red, green, blue)
    myColorScheme[MANUAL] = MSNet::getInstance()->getVehicleControl().getVType(myManualTypeID)->getColor();
    myColorScheme[AUTOMATED] = MSNet::getInstance()->getVehicleControl().getVType(myAutomatedTypeID)->getColor();
    myColorScheme[PREPARING_TOC] = RGBColor(200, 200, 250); // light blue
    myColorScheme[MRM] = RGBColor(250, 50, 50); // red
    myColorScheme[RECOVERING] = RGBColor(250, 210, 150); // light yellow
    myColorScheme[UNDEFINED] = RGBColor(150, 150, 150); // gray
}


MSDevice_ToC::~MSDevice_ToC() {
    // unregister from static instance container
    myInstances.erase(this);
    // deschedule commands associated to this device
    if (myTriggerMRMCommand != nullptr) {
        myTriggerMRMCommand->deschedule();
    }
    if (myTriggerToCCommand != nullptr) {
        myTriggerToCCommand->deschedule();
    }
    if (myRecoverAwarenessCommand != nullptr) {
        myRecoverAwarenessCommand->deschedule();
    }
    if (myExecuteMRMCommand != nullptr) {
        myExecuteMRMCommand->deschedule();
        resetDeliberateLCs();
    }
    if (myPrepareToCCommand != nullptr) {
        myPrepareToCCommand->deschedule();
    }
}

void
MSDevice_ToC::setAwareness(double value) {
    if (value > 1.0 || value < 0.0) {
        std::stringstream ss;
        ss << "Truncating invalid value for awareness (" << value << ") to lie in [0,1].";
        WRITE_WARNING(ss.str());
        value = MAX2(0.0, MIN2(1.0, value));
    }
    if (myCurrentAwareness >= myLCAbstinence && value < myLCAbstinence) {
        // Awareness is now below LC abstinence level -> prevent deliberate LCs
        deactivateDeliberateLCs();
    } else if (myCurrentAwareness < myLCAbstinence && value >= myLCAbstinence) {
        // Awareness is now above LC abstinence level -> allow deliberate LCs
        resetDeliberateLCs();
    }
    myCurrentAwareness = value;
    myHolderMS->getDriverState()->setAwareness(value);
}


void
MSDevice_ToC::setState(ToCState state) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " MSDevice_ToC::setState()" << std::endl;
#endif
    if (myState == state) {
        // No state change
        return;
    }

    if (myState == MRM) {
        // reset the vehicle's maxAccel
        myHolderMS->getSingularType().getCarFollowModel().setMaxAccel(myOriginalMaxAccel);
        resetDeliberateLCs();
    } else if (myState == PREPARING_TOC) {
        if (myOpenGapParams.active) {
            // Deactivate gap control at preparation phase end
            myHolderMS->getInfluencer().deactivateGapController();
        }
        if (state != MRM) {
            // Aborting preparation
            resetDeliberateLCs();
            myHolderMS->getSingularType().getCarFollowModel().setMaxAccel(myOriginalMaxAccel);
        }
    } else if (state == PREPARING_TOC || state == MRM) {
#ifdef DEBUG_TOC
        std::cout << "  Entering ToC preparation... " << std::endl;
#endif
        // Prevent lane changing during takeover preparation
        deactivateDeliberateLCs();
        // Store original value of maxAccel for restoring it after preparation phase
        myOriginalMaxAccel = myHolderMS->getCarFollowModel().getMaxAccel();
        // Impose acceleration limit during preparation
        myHolderMS->getSingularType().getCarFollowModel().setMaxAccel(MIN2(myMaxPreparationAccel, myOriginalMaxAccel));
    }

    if (myIssuedDynamicToC) {
        // Reset dynamic ToC flag
        myIssuedDynamicToC = false;
    }

    myState = state;
    if (myUseColorScheme) {
        setVehicleColor();
    }
}

void
MSDevice_ToC::setVehicleColor() {
    const SUMOVehicleParameter& p = myHolder.getParameter();
    p.color = myColorScheme[myState];
    p.parametersSet |= VEHPARS_COLOR_SET;
}

void
MSDevice_ToC::requestMRM() {
    // Remove any preparatory process
    descheduleToCPreparation();
    // .. and any recovery process
    descheduleRecovery();
    // ... and any pending ToC to manual
    descheduleToC();
    // Immediately trigger the MRM process
    triggerMRM(0);
}


void
MSDevice_ToC::requestToC(SUMOTime timeTillMRM, SUMOTime responseTime) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " requestToC() for vehicle '" << myHolder.getID() << "', timeTillMRM=" << timeTillMRM << ", responseTime=" << responseTime << std::endl;
#endif
    if (myState == AUTOMATED) {
        // Initialize preparation phase
        if (responseTime == -1000) {
            // Sample response time from distribution
            responseTime = TIME2STEPS(sampleResponseTime(STEPS2TIME(timeTillMRM)));
        }

        // Schedule ToC Event
        myTriggerToCCommand = new WrappingCommand<MSDevice_ToC>(this, &MSDevice_ToC::triggerDownwardToC);
        MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(myTriggerToCCommand, SIMSTEP + responseTime);

        assert(myExecuteMRMCommand == nullptr);
        assert(myTriggerMRMCommand == nullptr);
        if (responseTime > timeTillMRM && myState != MRM) {
            // Schedule new MRM if driver response time is higher than permitted
            myTriggerMRMCommand = new WrappingCommand<MSDevice_ToC>(this, &MSDevice_ToC::triggerMRM);
            MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(myTriggerMRMCommand, SIMSTEP + timeTillMRM);
        }

        // Start ToC preparation process
        myPrepareToCCommand = new WrappingCommand<MSDevice_ToC>(this, &MSDevice_ToC::ToCPreparationStep);
        MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(myPrepareToCCommand, SIMSTEP + DELTA_T);
        setState(PREPARING_TOC);
        if (myOpenGapParams.active) {
            // Start gap controller
            double originalTau = myHolderMS->getCarFollowModel().getHeadwayTime();
            myHolderMS->getInfluencer().activateGapController(originalTau,
                    myOpenGapParams.newTimeHeadway, myOpenGapParams.newSpaceHeadway, -1,
                    myOpenGapParams.changeRate, myOpenGapParams.maxDecel);
        }
        // Record event
        if (generatesOutput()) {
            myEvents.push(std::make_pair(SIMSTEP, "TOR"));
            myEventLanes.push(std::make_pair(myHolder.getLane()->getID(), myHolder.getPositionOnLane())); // add lane and lanepos
            myEventXY.push(std::make_pair(myHolder.getPosition().x(), myHolder.getPosition().y()));       // add (x, y) position
        }
    } else {
        // Switch to automated mode is performed immediately
        if (timeTillMRM > 0.) {
            std::stringstream ss;
            ss << "[t=" << SIMTIME << "] Positive transition time (" << timeTillMRM / 1000. << "s.) for upward ToC of vehicle '" << myHolder.getID() << "' is ignored.";
            WRITE_WARNING(ss.str());
        }
        triggerUpwardToC(SIMSTEP + DELTA_T);
    }
}


SUMOTime
MSDevice_ToC::triggerMRM(SUMOTime /* t */) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " triggerMRM() for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
    // Clear ongoing MRM
    descheduleMRM();

    // Start MRM process
    if (myMRMSafeSpot != "") {
        SUMOVehicleParameter::Stop stop;
        MSStoppingPlace* s = MSNet::getInstance()->getStoppingPlace(myMRMSafeSpot, SUMO_TAG_PARKING_AREA);
        if (s == nullptr) {
            WRITE_WARNING("Ignoring unknown safe spot '" + myMRMSafeSpot + "' for vehicle '" + myHolder.getID() + "'.");
        } else {
            stop.parkingarea = myMRMSafeSpot;
            stop.parking = true;
            stop.lane = s->getLane().getID();
            stop.endPos = s->getEndLanePosition();
            stop.startPos = s->getBeginLanePosition();
            stop.duration = myMRMSafeSpotDuration;
            myHolderMS->getSingularType().setDecel(myMRMDecel);
            std::string error;
            if (!myHolder.addStop(stop, error)) {
                WRITE_WARNING("Could not set safe spot '" + myMRMSafeSpot + "' for vehicle '" + myHolder.getID() + "'. " + error);
            }
        }
    } else {
        myExecuteMRMCommand = new WrappingCommand<MSDevice_ToC>(this, &MSDevice_ToC::MRMExecutionStep);
        MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(myExecuteMRMCommand, SIMSTEP + DELTA_T);
    }
    if (myState == MANUAL || myState == RECOVERING) {
        switchHolderType(myAutomatedTypeID);
    }
    setState(MRM);
    setAwareness(1.);

    // Record event
    if (generatesOutput()) {
        myEvents.push(std::make_pair(SIMSTEP, "MRM"));
        myEventLanes.push(std::make_pair(myHolder.getLane()->getID(), myHolder.getPositionOnLane())); // add lane and lanepos
        myEventXY.push(std::make_pair(myHolder.getPosition().x(), myHolder.getPosition().y()));       // add (x, y) position
    }

    return 0;
}


SUMOTime
MSDevice_ToC::triggerUpwardToC(SUMOTime /* t */) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " triggerUpwardToC() for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
    descheduleToC();
    // Eventually stop ToC preparation process
    descheduleToCPreparation();
    // Eventually abort MRM
    descheduleMRM();
    // Eventually abort awareness recovery process
    descheduleRecovery();

    if (myState == MANUAL || myState == RECOVERING) {
        switchHolderType(myAutomatedTypeID);
    }
    setAwareness(1.);
    setState(AUTOMATED);

    // Record event
    if (generatesOutput()) {
        myEvents.push(std::make_pair(SIMSTEP, "ToCup"));
        myEventLanes.push(std::make_pair(myHolder.getLane()->getID(), myHolder.getPositionOnLane())); // add lane and lanepos
        myEventXY.push(std::make_pair(myHolder.getPosition().x(), myHolder.getPosition().y()));       // add (x, y) position
    }

    return 0;
}


SUMOTime
MSDevice_ToC::triggerDownwardToC(SUMOTime /* t */) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " triggerDownwardToC() for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
    descheduleToC();
    // Eventually stop ToC preparation process
    descheduleToCPreparation();
    // Eventually abort MRM
    descheduleMRM();

#ifdef DEBUG_TOC
    std::cout << SIMTIME << " Initial awareness after ToC: " << myCurrentAwareness << std::endl;
#endif

    // Start awareness recovery process
    myRecoverAwarenessCommand = new WrappingCommand<MSDevice_ToC>(this, &MSDevice_ToC::awarenessRecoveryStep);
    MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(myRecoverAwarenessCommand, SIMSTEP + DELTA_T);
    setState(RECOVERING);

    // @todo: Sample initial awareness
    double initialAwareness = myInitialAwareness;
    setAwareness(initialAwareness);

    switchHolderType(myManualTypeID);

    // Record event
    if (generatesOutput()) {
        myEvents.push(std::make_pair(SIMSTEP, "ToCdown"));
        myEventLanes.push(std::make_pair(myHolder.getLane()->getID(), myHolder.getPositionOnLane())); // add lane and lanepos
        myEventXY.push(std::make_pair(myHolder.getPosition().x(), myHolder.getPosition().y()));       // add (x, y) position
    }
    return 0;
}

void
MSDevice_ToC::descheduleMRM() {
    // Eventually abort scheduled MRM
    if (myTriggerMRMCommand != nullptr) {
        myTriggerMRMCommand->deschedule();
        myTriggerMRMCommand = nullptr;
    }
    // Eventually abort ongoing MRM
    if (myExecuteMRMCommand != nullptr) {
        myExecuteMRMCommand->deschedule();
        resetDeliberateLCs();
        myExecuteMRMCommand = nullptr;
    }
}


void
MSDevice_ToC::descheduleToC() {
    if (myTriggerToCCommand != nullptr) {
        myTriggerToCCommand->deschedule();
        myTriggerToCCommand = nullptr;
    }
}

void
MSDevice_ToC::descheduleToCPreparation() {
    // Eventually stop ToC preparation process
    if (myPrepareToCCommand != nullptr) {
        myPrepareToCCommand->deschedule();
        if (myState != MRM) {
            resetDeliberateLCs();
        }
        myPrepareToCCommand = nullptr;
    }
}

void
MSDevice_ToC::descheduleRecovery() {
    // Eventually stop ToC preparation process
    if (myRecoverAwarenessCommand != nullptr) {
        myRecoverAwarenessCommand->deschedule();
        myRecoverAwarenessCommand = nullptr;
    }
}


void
MSDevice_ToC::switchHolderType(const std::string& targetTypeID) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " Switching type of vehicle '" << myHolder.getID() << "' to '" << targetTypeID << "'" << std::endl;
#endif
    MSVehicleType* targetType = MSNet::getInstance()->getVehicleControl().getVType(targetTypeID);
    if (targetType == nullptr) {
        WRITE_ERROR("vType '" + targetType->getID() + "' for vehicle '" + myHolder.getID() + "' is not known.");
        return;
    }
    myHolderMS->replaceVehicleType(targetType);
}


SUMOTime
MSDevice_ToC::ToCPreparationStep(SUMOTime /* t */) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " ToC preparation step for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
    if (myState == PREPARING_TOC) {
        return DELTA_T;
    } else {
#ifdef DEBUG_TOC
        std::cout << SIMTIME << " Aborting ToC preparation for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
        descheduleToCPreparation();
        return 0;
    }
}


SUMOTime
MSDevice_ToC::MRMExecutionStep(SUMOTime t) {
    deactivateDeliberateLCs();
    const double currentSpeed = myHolderMS->getSpeed();
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " MRM step for vehicle '" << myHolder.getID() << "', currentSpeed=" << currentSpeed << std::endl;
#endif

    // Induce slowdown with MRMDecel
    std::vector<std::pair<SUMOTime, double> > speedTimeLine;
    const double nextSpeed = MAX2(0., currentSpeed - ACCEL2SPEED(myMRMDecel));
    speedTimeLine.push_back(std::make_pair(t - DELTA_T, currentSpeed));
    speedTimeLine.push_back(std::make_pair(t, nextSpeed));
    myHolderMS->getInfluencer().setSpeedTimeLine(speedTimeLine);

    if (myMRMKeepRight) {
        // Try to change to the right
        std::vector<std::pair<SUMOTime, int> > laneTimeLine;
        laneTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), 0));
        laneTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep() + 1, 0));
        myHolderMS->getInfluencer().setLaneTimeLine(laneTimeLine);
//    	std::cout << "Keeping right..." << std::endl;
    }

    if (myState == MRM) {
        return DELTA_T;
    } else {
#ifdef DEBUG_TOC
        std::cout << SIMTIME << " Aborting MRM for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
        resetDeliberateLCs();
        return 0;
    }
}


SUMOTime
MSDevice_ToC::awarenessRecoveryStep(SUMOTime /* t */) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " Awareness recovery step for vehicle '" << myHolder.getID() << "'" << std::endl;
#endif
    // Proceed with awareness recovery
    if (myCurrentAwareness < 1.0) {
        setAwareness(MIN2(1.0, myCurrentAwareness + TS * myRecoveryRate));
    }

#ifdef DEBUG_TOC
    std::cout << SIMTIME << " currentAwareness = " << myCurrentAwareness << std::endl;
#endif

    const bool awarenessRecoveryCompleted = myCurrentAwareness == 1.0;
    if (awarenessRecoveryCompleted) {
#ifdef DEBUG_TOC
        std::cout << SIMTIME << " Awareness recovery completed for veh '" << myHolder.getID() << "'" << std::endl;
#endif
        myRecoverAwarenessCommand->deschedule();
        myRecoverAwarenessCommand = nullptr;
        setState(MANUAL);
        return 0;
    }
    return DELTA_T;
}

bool
MSDevice_ToC::notifyMove(SUMOTrafficObject& /*veh*/,
                         double /*oldPos*/,
                         double /*newPos*/,
                         double /*newSpeed*/) {
    if (myState == AUTOMATED && checkDynamicToC()) {
        // Initiate a ToC
        // Record event
        if (generatesOutput()) {
            myEvents.push(std::make_pair(SIMSTEP, "DYNTOR"));
            myEventLanes.push(std::make_pair(myHolder.getLane()->getID(), myHolder.getPositionOnLane())); // add lane and lanepos
            myEventXY.push(std::make_pair(myHolder.getPosition().x(), myHolder.getPosition().y()));       // add (x, y) position
        }
        // Leadtime for dynamic ToC is proportional to the time assumed for the dynamic ToC threshold
        const double leadTime = myDynamicToCThreshold * 1000 * DYNAMIC_TOC_LEADTIME_FACTOR;
        requestToC((SUMOTime) leadTime);
        // TODO: Alter the response time according to the given lead time. Consider re-sampling it at each call of requestToC(). (Conditional to whether a non-negative response time was given in the configuration)
        myIssuedDynamicToC = true;
        myDynamicToCLane = myHolderMS->getLane()->getNumericalID();
    } else if (myIssuedDynamicToC && myState == PREPARING_TOC && !checkDynamicToC()) {
        // Abort dynamic ToC, FIXME: This could abort an externally requested ToC in rare occasions... (needs test)
        // Record event
        if (generatesOutput()) {
            myEvents.push(std::make_pair(SIMSTEP, "DYNTOR"));
            myEventLanes.push(std::make_pair(myHolder.getLane()->getID(), myHolder.getPositionOnLane())); // add lane and lanepos
            myEventXY.push(std::make_pair(myHolder.getPosition().x(), myHolder.getPosition().y()));       // add (x, y) position
        }
        // NOTE: This should not occur if lane changing is prevented during ToC preparation...
        // TODO: Reset response time to the original value (unnecessary if re-sampling for each call to requestToC)
        triggerUpwardToC(0);
    }
    return true;
}

std::string
MSDevice_ToC::getParameter(const std::string& key) const {
    if (key == "manualType") {
        return myManualTypeID;
    } else if (key == "automatedType") {
        return myAutomatedTypeID;
    } else if (key == "responseTime") {
        return toString(STEPS2TIME(myResponseTime));
    } else if (key == "recoveryRate") {
        return toString(myRecoveryRate);
    } else if (key == "initialAwareness") {
        return toString(myInitialAwareness);
    } else if (key == "mrmDecel") {
        return toString(myMRMDecel);
    } else if (key == "currentAwareness") {
        return toString(myCurrentAwareness);
    } else if (key == "lcAbstinence") {
        return toString(myLCAbstinence);
    } else if (key == "state") {
        return _2string(myState);
    } else if (key == "holder") {
        return myHolder.getID();
    } else if (key == "hasDynamicToC") {
        return toString(myDynamicToCActive);
    } else if (key == "dynamicToCThreshold") {
        return toString(myDynamicToCThreshold);
    } else if (key == "dynamicMRMProbability") {
        return toString(myMRMProbability);
    } else if (key == "mrmKeepRight") {
        return toString(myMRMKeepRight);
    } else if (key == "mrmSafeSpot") {
        return myMRMSafeSpot;
    } else if (key == "mrmSafeSpotDuration") {
        return toString(STEPS2TIME(myMRMSafeSpotDuration));
    } else if (key == "maxPreparationAccel") {
        return toString(myMaxPreparationAccel);
    } else if (key == "ogNewTimeHeadway") {
        return toString(myOpenGapParams.newTimeHeadway);
    } else if (key == "ogNewSpaceHeadway") {
        return toString(myOpenGapParams.newSpaceHeadway);
    } else if (key == "ogChangeRate") {
        return toString(myOpenGapParams.changeRate);
    } else if (key == "ogMaxDecel") {
        return toString(myOpenGapParams.maxDecel);
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


void
MSDevice_ToC::setParameter(const std::string& key, const std::string& value) {
#ifdef DEBUG_TOC
    std::cout << SIMTIME << " MSDevice_ToC::setParameter(key=" << key << ", value=" << value << ")" << std::endl;
#endif
    if (key == "manualType") {
        myManualTypeID = value;
        myColorScheme[MANUAL] = MSNet::getInstance()->getVehicleControl().getVType(myManualTypeID)->getColor();
        if (myState == MANUAL) {
            switchHolderType(value);
        }
    } else if (key == "automatedType") {
        myAutomatedTypeID = value;
        myColorScheme[AUTOMATED] = MSNet::getInstance()->getVehicleControl().getVType(myAutomatedTypeID)->getColor();
        if (myState == AUTOMATED || myState == PREPARING_TOC || myState == MRM) {
            switchHolderType(value);
        }
    } else if (key == "responseTime") {
        myResponseTime = TIME2STEPS(StringUtils::toDouble(value));
    } else if (key == "recoveryRate") {
        myRecoveryRate = StringUtils::toDouble(value);
    } else if (key == "initialAwareness") {
        myInitialAwareness = StringUtils::toDouble(value);
    } else if (key == "lcAbstinence") {
        myLCAbstinence = StringUtils::toDouble(value);
        if (isManuallyDriven()) {
            setAwareness(myCurrentAwareness); // to eventually trigger LC-prevention
        }
    } else if (key == "currentAwareness") {
        if (isManuallyDriven()) {
            setAwareness(StringUtils::toDouble(value));
        } else {
            WRITE_WARNING("Setting device.toc.currentAwareness during automated mode has no effect.")
        }
    } else if (key == "mrmDecel") {
        myMRMDecel = StringUtils::toDouble(value);
    } else if (key == "requestToC") {
        // setting this magic parameter gives the interface for inducing a ToC
        const SUMOTime timeTillMRM = TIME2STEPS(StringUtils::toDouble(value));
        requestToC(timeTillMRM, myResponseTime);
    } else if (key == "requestMRM") {
        // setting this magic parameter gives the interface for inducing an MRM
        requestMRM();
    } else if (key == "awareness") {
        // setting this magic parameter gives the interface for setting the driverstate's awareness
        setAwareness(StringUtils::toDouble(value));
    } else if (key == "dynamicToCThreshold") {
        const double newValue = StringUtils::toDouble(value);
        if (newValue < 0) {
            WRITE_WARNING("Value of dynamicToCThreshold must be non-negative. (Given value " + value + " for vehicle " + myHolderMS->getID() + " is ignored)");
        } else if (newValue == 0) {
            myDynamicToCThreshold = newValue;
            myDynamicToCActive = false;
        } else {
            myDynamicToCThreshold = newValue;
            myDynamicToCActive = true;
        }
    } else if (key == "dynamicMRMProbability") {
        const double newValue = StringUtils::toDouble(value);
        if (newValue < 0) {
            WRITE_WARNING("Value of dynamicMRMProbability must be non-negative. (Given value " + value + " for vehicle " + myHolderMS->getID() + " is ignored)");
        } else {
            myMRMProbability = newValue;
        }
    } else if (key == "mrmKeepRight")  {
        const bool newValue = StringUtils::toBool(value);
        myMRMKeepRight = newValue;
    } else if (key == "mrmSafeSpot") {
        myMRMSafeSpot = value;
    } else if (key == "mrmSafeSpotDuration") {
        myMRMSafeSpotDuration = TIME2STEPS(StringUtils::toDouble(value));
    } else if (key == "maxPreparationAccel") {
        const double newValue = StringUtils::toDouble(value);
        if (newValue < 0) {
            WRITE_WARNING("Value of maxPreparationAccel must be non-negative. (Given value " + value + " for vehicle " + myHolderMS->getID() + " is ignored)");
        } else {
            myMaxPreparationAccel = newValue;
        }
    } else {
        throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
    }
}


MSDevice_ToC::ToCState
MSDevice_ToC::_2ToCState(const std::string& str) {
    if (str == "UNDEFINED") {
        return UNDEFINED;
    } else if (str == "MANUAL") {
        return MANUAL;
    } else if (str == "AUTOMATED") {
        return AUTOMATED;
    } else if (str == "PREPARING_TOC") {
        return PREPARING_TOC;
    } else if (str == "MRM") {
        return MRM;
    } else if (str == "RECOVERING") {
        return RECOVERING;
    } else {
        WRITE_WARNING("Unknown ToCState '" + str + "'");
        return UNDEFINED;
    }
}


std::string
MSDevice_ToC::_2string(ToCState state) {
    if (state == UNDEFINED) {
        return "UNDEFINED";
    } else if (state == MANUAL) {
        return "MANUAL";
    } else if (state == AUTOMATED) {
        return "AUTOMATED";
    } else if (state == PREPARING_TOC) {
        return "PREPARING_TOC";
    } else if (state == MRM) {
        return "MRM";
    } else if (state == RECOVERING) {
        return "RECOVERING";
    } else {
        WRITE_WARNING("Unknown ToCState '" + toString(state) + "'");
        return toString(state);
    }
}


void
MSDevice_ToC::writeOutput() {
    if (!generatesOutput()) {
        assert(myEvents.empty());
        return;
    }
    while (!myEvents.empty()) {
        std::pair<SUMOTime, std::string>& e = myEvents.front();
        std::pair<std::string, double>& l = myEventLanes.front();
        std::pair<double, double>& p = myEventXY.front();
        myOutputFile->openTag(e.second);
        myOutputFile->writeAttr("id", myHolder.getID()).writeAttr("t", STEPS2TIME(e.first));
        myOutputFile->writeAttr("lane", l.first).writeAttr("lanePos", l.second);
        myOutputFile->writeAttr("x", p.first).writeAttr("y", p.second);
        myOutputFile->closeTag();
        myEvents.pop();
        myEventLanes.pop();
        myEventXY.pop();

        if (e.second.compare("DYNTOR") == 0 && !myEvents.empty()) { // skip "TOR" events if duplicate of "DYNTOR"
            std::pair<SUMOTime, std::string>& eNext = myEvents.front();
            if (eNext.second.compare("TOR") == 0 && eNext.first == e.first) {
                myEvents.pop();
                myEventLanes.pop();
                myEventXY.pop();
            }
        }
    }
}


void
MSDevice_ToC::cleanup() {
    // Close xml bodies for all existing files
    // TODO: Check if required
    for (auto& fn : createdOutputFiles) {
        OutputDevice* file = &OutputDevice::getDevice(fn);
        file->closeTag();
    }
}


void
MSDevice_ToC::resetDeliberateLCs() {
    if (myPreviousLCMode != -1) {
        myHolderMS->getInfluencer().setLaneChangeMode(myPreviousLCMode);
#ifdef DEBUG_TOC
        std::cout << SIMTIME << " MSDevice_ToC::resetLCMode() restoring LC Mode of vehicle '" << myHolder.getID() << "' to " << myPreviousLCMode << std::endl;
#endif
    }
    myPreviousLCMode = -1;
}


void
MSDevice_ToC::deactivateDeliberateLCs() {
    const int lcModeHolder = myHolderMS->getInfluencer().getLaneChangeMode();
    if (lcModeHolder != LCModeMRM) {
        myPreviousLCMode = lcModeHolder;
#ifdef DEBUG_TOC
        std::cout << SIMTIME << " MSDevice_ToC::setLCModeMRM() setting LC Mode of vehicle '" << myHolder.getID()
                  << "' from " << myPreviousLCMode << " to " << LCModeMRM << std::endl;
#endif
        myHolderMS->getInfluencer().setLaneChangeMode(LCModeMRM);
    }
}

bool
MSDevice_ToC::isManuallyDriven() {
    return (myState == MANUAL || myState == RECOVERING);
}

bool
MSDevice_ToC::isAutomated() {
    return (myState == AUTOMATED || myState == PREPARING_TOC || myState == MRM);
}

bool
MSDevice_ToC::checkDynamicToC() {
#ifdef DEBUG_DYNAMIC_TOC
    std::cout << SIMTIME << " # MSDevice_ToC::checkDynamicToC() for veh '" << myHolder.getID() << "'" << std::endl;
#endif
    if (!myDynamicToCActive) {
        return false;
    }
    // The vehicle's current lane
    const MSLane* currentLane = myHolderMS->getLane();

    if (currentLane->isInternal()) {
        // Don't start or abort dynamic ToCs on internal lanes
        return myIssuedDynamicToC;
    }

    if (myIssuedDynamicToC) {
#ifdef DEBUG_DYNAMIC_TOC
        std::cout << SIMTIME << " Dynamic ToC is ongoing." << std::endl;
#endif
        // Dynamic ToC in progress. Resist to aborting it if lane was not changed.
        if (myDynamicToCLane == currentLane->getNumericalID()) {
            return true;
        }
    }
    // Length for which the current route can be followed
    const std::vector<MSVehicle::LaneQ>& bestLanes = myHolderMS->getBestLanes();
    // Maximal distance for route continuation without LCs over the possible start lanes
    double maximalContinuationDistance = 0;
    // Distance for route continuation without LCs from the vehicle's current lane
    double continuationDistanceOnCurrent = 0;
    // Lane of the next stop
    const MSLane* nextStopLane = nullptr;

    if (myHolderMS->hasStops()) {
        nextStopLane = myHolderMS->getNextStop().lane;
    }
    for (auto& i : bestLanes) {
        maximalContinuationDistance = MAX2(maximalContinuationDistance, i.length);
        if (currentLane == i.lane) {
            if (myHolderMS->hasStops()) {
                // Check if the next stop lies on the route continuation from the current lane
                for (MSLane* l : i.bestContinuations) {
                    if (l == nextStopLane) {
#ifdef DEBUG_DYNAMIC_TOC
                        std::cout << SIMTIME << " Stop found on the route continuation from the current lane. => No ToC" << std::endl;
#endif
                        // Stop found on the route continuation from the current lane => no ToC necessary
                        return false;
                    }
                }
            }
            continuationDistanceOnCurrent = i.length;
        }
    }
    if (continuationDistanceOnCurrent == maximalContinuationDistance) {
        // There is no better lane than the current, hence no desire to change lanes,
        // which the driver could pursue better than the automation => no reason for ToC.
        return false;
    }
    const double distFromCurrent = continuationDistanceOnCurrent - myHolderMS->getPositionOnLane();
    const double MRMDist = 0.5 * myHolderMS->getSpeed() * myHolderMS->getSpeed() / MAX2(myMRMDecel, 0.0001);
    double distThreshold = myHolderMS->getSpeed() * myDynamicToCThreshold + MRMDist;
#ifdef DEBUG_DYNAMIC_TOC
    std::cout << "  speed=" << myHolderMS->getSpeed()
              << ", distFromCurrent=" << distFromCurrent
              << ", maximal dist=" << maximalContinuationDistance - myHolderMS->getPositionOnLane()
              << ", distThreshold=" << distThreshold
              << std::endl;
#endif

    if (myIssuedDynamicToC) {
        // In case of an ongoing ToC, add an additional resistance to abort it.
        // (The lane-check above does not capture lanes subsequent to the dynamic ToC lane)
        distThreshold *= DYNAMIC_TOC_ABORT_RESISTANCE_FACTOR;
    }

    if (distFromCurrent < distThreshold) {
        // TODO: Make this more sophisticated in dealing with low speeds/stops and route ends
#ifdef DEBUG_DYNAMIC_TOC
        std::cout << SIMTIME << "  * distAlongBest is below threshold! *" << std::endl;
#endif
        return true;
    }

    return false;
}

double
MSDevice_ToC::sampleResponseTime(double leadTime) const {
#ifdef DEBUG_DYNAMIC_TOC
    std::cout << "sampleResponseTime() leadTime=" << leadTime << std::endl;
#endif
    const double mean = responseTimeMean(leadTime);
    const double var = interpolateVariance(leadTime, myMRMProbability);
    std::normal_distribution<double> d(mean, var);
    double rt = d(myResponseTimeRNG);
#ifdef DEBUG_DYNAMIC_TOC
    std::cout << "  mean=" << mean << ", variance=" << var << " => sampled responseTime=" << rt << std::endl;
#endif
    int it_count = 0;
    while (rt < 0 && it_count < MAX_RESPONSETIME_SAMPLE_TRIES) {
        rt = d(myResponseTimeRNG);
        it_count++;
    }
    if (rt < 0) {
        // Didn't generate a positive random response time => use mean
        rt = mean;
    }
    return rt;
}

double
MSDevice_ToC::interpolateVariance(double leadTime, double pMRM) {
#ifdef DEBUG_DYNAMIC_TOC
    std::cout << "interpolateVariance() leadTime=" << leadTime << ", pMRM=" << pMRM << std::endl;
#endif
    // Calculate indices for surrounding values in lookup tables

    // Find largest p_{i-1} < pMRM < p_{i}
    const auto pi = std::lower_bound(lookupResponseTimeMRMProbs.begin(), lookupResponseTimeMRMProbs.end(), pMRM);
    if (pi == lookupResponseTimeMRMProbs.end()) {
        // requested probability lies outside lookup table.
        // => return maximal variance value
        return MAX_RESPONSETIME_VARIANCE;
    }
    const size_t pi1 = pi - lookupResponseTimeMRMProbs.begin();
    assert(pi1 > 0);
    const size_t pi0 = pi1 - 1;
    const double cp = (pMRM - * (pi - 1)) / (*pi - * (pi - 1));

#ifdef DEBUG_DYNAMIC_TOC
    std::cout << " p[=" << pi0 << "]=" << *(pi - 1) << ", p[=" << pi1 << "]=" << *pi << " => cp=" << cp << std::endl;
#endif

    // Find largest p_{i-1} < pMRM < p_{i}
    auto li = std::lower_bound(lookupResponseTimeLeadTimes.begin(), lookupResponseTimeLeadTimes.end(), leadTime);
    if (li == lookupResponseTimeLeadTimes.begin()) {
        // Given lead time smaller than minimal lookup-value.
        // Use minimal value from lookup table instead
        leadTime = *li;
        li = lookupResponseTimeLeadTimes.begin() + 1;
    } else if (li == lookupResponseTimeLeadTimes.end()) {
        // Given leadTime exceeds values in lookup table
        // => induce extrapolation
        li--;
    }
    const size_t li1 = li - lookupResponseTimeLeadTimes.begin();
    const size_t li0 = li1 - 1;
    const double cl = (leadTime - * (li - 1)) / (*li - * (li - 1));

#ifdef DEBUG_DYNAMIC_TOC
    std::cout << " l[=" << li0 << "]=" << *(li - 1) << ", l[=" << li1 << "]=" << *li << " => cp=" << cl << std::endl;
#endif

    // 2D interpolation for variance
    // First, interpolate (or extrapolate) variances along leadTimes
    const double var00 = lookupResponseTimeVariances[pi0][li0];
    const double var01 = lookupResponseTimeVariances[pi0][li1];
    const double var10 = lookupResponseTimeVariances[pi1][li0];
    const double var11 = lookupResponseTimeVariances[pi1][li1];
    const double var_0 = var00 + (var01 - var00) * cl;
    const double var_1 = var10 + (var11 - var10) * cl;
    // From these, interpolate along the pMRM-axis
    const double var = var_0 + (var_1 - var_0) * cp;
#ifdef DEBUG_DYNAMIC_TOC
    std::cout << " var00=" << var00 << ", var01=" << var01 << " var10=" << var10 << ", var11=" << var11
              << " var_0=" << var_0 << ", var_1=" << var_1 << ", var=" << var << std::endl;
#endif
    return var;
}

// Grid of the response time distribution.
// Generated by the script generateResponseTimeDistributions.py, see Appendix to TransAID Deliverable 3.1v2.
// Probability for an MRM to occur (start with 0.0, end with 0.5)
std::vector<double> MSDevice_ToC::lookupResponseTimeMRMProbs = {0.0, 0.05, 0.1, 0.15000000000000002, 0.2, 0.25, 0.30000000000000004, 0.35000000000000003, 0.4, 0.45, 0.5};
// Lead time grid
std::vector<double> MSDevice_ToC::lookupResponseTimeLeadTimes = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.7999999999999999, 0.8999999999999999, 0.9999999999999999, 1.0999999999999999, 1.2, 1.3, 1.4, 1.5, 1.5999999999999999, 1.7, 1.8, 1.9, 2.0, 2.25, 2.5, 2.75, 3.0, 3.25, 3.5, 3.75, 4.0, 4.25, 4.5, 4.75, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, 10.5, 11.0, 11.5, 12.0, 12.5, 13.0, 13.5, 14.0, 14.5, 15.0, 15.5, 16.0, 16.5, 17.0, 17.5, 18.0, 18.5, 19.0, 19.5, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0, 48.0, 49.0, 50.0};

// Variances of the response time distribution.
std::vector<std::vector<double> > MSDevice_ToC::lookupResponseTimeVariances = {
    {0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001, 0.0001},
    {0.018238371642696278, 0.03647674328603705, 0.054715114926535656, 0.07295348656987645, 0.09119185821321724, 0.10943022985371582, 0.12766860149705656, 0.14590697314039733, 0.16414534478089599, 0.18238371642423673, 0.2006220880675775, 0.21886045971091828, 0.2370988313514169, 0.25533720299475765, 0.27357557463809845, 0.291813946278597, 0.3100523179219377, 0.32829068956527846, 0.3465290612057772, 0.36476743284911795, 0.4103633619560487, 0.45595929106297967, 0.5015552201670682, 0.5471511492739992, 0.59274707838093, 0.6383430074850186, 0.6839389365919495, 0.7295348656988803, 0.7751307948058112, 0.8207267239098999, 0.8663226530168309, 0.9119185821237615, 1.003110440334781, 1.0943022985486428, 1.1854941567596624, 1.2766860149735242, 1.3678778731845436, 1.4590697313984053, 1.622588042723657, 1.8236013818166044, 2.0271872430355344, 2.2329896377033402, 2.440681612959606, 2.6499766330096066, 2.8606328744048484, 3.0724517481497657, 3.285273075453899, 3.4989689745182173, 3.713437758931686, 3.928598499444084, 4.144386478335934, 4.360749535794346, 4.577645204319768, 4.795038495182951, 5.012900204026633, 5.231205620052272, 5.449933544477286, 5.669065543877604, 5.888585381094657, 6.108478580034079, 6.328732091514834, 6.5493340353396325, 6.991540386888257, 7.4350193801571836, 7.8797023900653835, 8.325528107903486, 8.772441019472117, 9.220390365425358, 9.66932940241786, 10.11921485679467, 10.570006505095746, 11.021666840703753, 11.474160800924851, 11.927455537955435, 12.381520222795276, 12.836325874663427, 13.291845210806684, 13.748052512926236, 14.204923507573522, 14.662435258383752, 15.120566068535554, 15.57929539219183, 16.03860375377652, 16.498472674274336, 16.958884603774735, 17.41982285960362, 17.881271569514066, 18.343215619413176, 18.805640605235663, 19.268532788517863, 19.731879055399546, 20.195666878723525},
    {0.023394708584543455, 0.04678941716973141, 0.07018412575207719, 0.09357883433726513, 0.11697354292245306, 0.14036825150764104, 0.16376296009282898, 0.18715766867517475, 0.2105523772603627, 0.23394708584555063, 0.25734179443073857, 0.2807365030130844, 0.3041312115982723, 0.3275259201834602, 0.3509206287686481, 0.37431533735099387, 0.3977100459361818, 0.42110475452136986, 0.4444994631065578, 0.46789417168890357, 0.5263809431504523, 0.5848677146120012, 0.6433544860735498, 0.7018412575350985, 0.7603280289966473, 0.8188148004581961, 0.8773015719197449, 0.9357883433784517, 0.9942751148400004, 1.0527618863015489, 1.111248657763098, 1.169735429224647, 1.2867089721477445, 1.4036825150708419, 1.520656057991097, 1.6376296009141948, 1.7546031438372918, 1.8715766867603902, 2.080324923070349, 2.3356056603653466, 2.592695753086048, 2.851065033244823, 3.110348235805368, 3.37030701064756, 3.630792243424829, 3.8917140872434826, 4.153020535352749, 4.414682984212653, 4.676686852454919, 4.939025633083471, 5.201697195938797, 5.4647015339779985, 5.728039423002128, 5.991711652753062, 6.255718611539178, 6.520060086487768, 6.784735192689795, 7.0497423770254475, 7.315079462951026, 7.580743715621537, 7.846731914823575, 8.113040428416943, 8.646602223970579, 9.181394311584102, 9.717379133745458, 10.25451769188949, 10.792770521511661, 11.332098388641393, 11.872462773672844, 12.41382619675905, 12.956152426938662, 13.499406606935544, 14.04355531739462, 14.588566598200957, 15.134409939740127, 15.681056253544272, 16.228477829207687, 16.776648282531472, 17.32554249848686, 17.875136571609353, 18.425407745596857, 18.976334353419617, 19.527895758814104, 20.080072299738145, 20.632845234156534, 21.18619668838177, 21.740109608063758, 22.294567711842525, 22.849555447666962, 23.405057951674344, 23.96106100953682, 24.517551020220964},
    {0.028809965676139145, 0.05761993135292278, 0.08642989702686427, 0.11523986270364789, 0.14404982838043154, 0.172859794054373, 0.20166975973115658, 0.23047972540794023, 0.2592896910818817, 0.2880996567586654, 0.31690962243544896, 0.34571958811223263, 0.37452955378617403, 0.4033395194629576, 0.4321494851397413, 0.4609594508136828, 0.4897694164904665, 0.51857938216725, 0.5473893478411916, 0.5761993135179752, 0.6482242277085132, 0.7202491418990512, 0.7922740560867471, 0.864298970277285, 0.9363238844678232, 1.0083487986555189, 1.0803737128460569, 1.152398627036595, 1.224423541227133, 1.296448455414829, 1.368473369605367, 1.4404982837959048, 1.584548112174139, 1.7285979405552145, 1.8726477689334486, 2.0166975973145242, 2.1607474256927586, 2.3047972540738346, 2.5572219533483644, 2.8623099438447785, 3.167154753839855, 3.4715923078311928, 3.775618300633292, 4.079301412505033, 4.3827364323759, 4.686020985499979, 4.989245061612653, 5.2924871133699085, 5.595813357348367, 5.89927852360174, 6.202927167199209, 6.506795109670751, 6.810910812533207, 7.115296603265157, 7.419969732096102, 7.724943265058281, 8.030226829842663, 8.33582723446787, 8.641748978492437, 8.947994674556652, 9.254565395777925, 9.561460961999748, 10.17622101728854, 10.792256328037844, 11.409539737303032, 12.02803871883203, 12.647717622463388, 13.268539213916304, 13.890465727714453, 14.513459582113466, 15.137483858085837, 15.76250261298258, 16.388481078046894, 17.015385774331957, 17.643184571383188, 18.271846706039163, 18.901342773672106, 19.531644700723955, 20.16272570482796, 20.79456024708863, 21.427123979740387, 22.06039369148417, 22.694347252144688, 23.32896355779389, 23.964222477085105, 24.600104799357926, 25.23659218482918, 25.873667117046345, 26.51131285772261, 27.149513403967962, 27.788253447896256, 28.427518338543063},
    {0.03496845765860337, 0.06993691531785123, 0.1049053729770991, 0.13987383063634692, 0.17484228829559478, 0.20981074595200053, 0.24477920361124836, 0.2797476612704962, 0.314716118929744, 0.34968457658899194, 0.38465303424823977, 0.41962149190748765, 0.4545899495638933, 0.48955840722314126, 0.5245268648823892, 0.5594953225416369, 0.5944637802008848, 0.6294322378601326, 0.6644006955193805, 0.6993691531757862, 0.7867902973239058, 0.8742114414720256, 0.961632585617303, 1.0490537297654225, 1.1364748739135424, 1.2238960180588196, 1.3113171622069395, 1.3987383063550594, 1.4861594505003368, 1.5735805946484562, 1.6610017387965759, 1.748422882941853, 1.9232651712352506, 2.0981074595314895, 2.272949747824887, 2.4477920361182846, 2.6226343244145234, 2.7974766127079205, 3.0926745755509586, 3.44395841916428, 3.7929654946149927, 4.140199155580832, 4.48610487276643, 4.8310499148399675, 5.175327720605737, 5.519169410160118, 5.862755881539858, 6.20622837304298, 6.549697059107542, 6.893247832651301, 7.236947584569466, 7.580848293751031, 7.924990195098607, 8.2694042393908, 8.614114010475241, 8.959137226336916, 9.30448692001335, 9.650172373181427, 9.996199857775714, 10.342573227835842, 10.689294393844476, 11.036363704475946, 11.73154213703699, 12.428090426873897, 13.125980076789245, 13.825176212794954, 14.52564028426291, 15.227331883606062, 15.930209972250445, 16.634233702118422, 17.339362959284276, 18.045558715641334, 18.752783247333983, 19.461000260660008, 20.170174953762714, 20.880274034013603, 21.591265705145904, 22.303119634055196, 23.015806904341318, 23.7292999616137, 24.443572554090043, 25.158599671023588, 25.874357480669964, 26.590823269047004, 27.30797538027189, 28.025793159035764, 28.74425689552527, 29.46334777297606, 30.183047817969005, 30.903339853423454, 31.624207454305022, 32.345634905899644},
    {0.04208452197242317, 0.08416904394549082, 0.12625356591855852, 0.16833808789162616, 0.21042260986469383, 0.25250713183776147, 0.294591653807987, 0.33667617578105463, 0.3787606977541223, 0.42084521972719, 0.46292974170025764, 0.5050142636733252, 0.5470987856463931, 0.5891833076194606, 0.6312678295925284, 0.6733523515627537, 0.7154368735358214, 0.7575213955088891, 0.7996059174819568, 0.8416904394550245, 0.9469017443876938, 1.0521130493203628, 1.1573243542501899, 1.2625356591828591, 1.3677469641155282, 1.472958269045355, 1.5781695739780248, 1.6833808789106937, 1.7885921838433625, 1.8938034887731896, 1.9990147937058587, 2.104226098638527, 2.314648708501024, 2.5250713183663627, 2.7354939282288586, 2.9459165380941967, 3.1563391479566936, 3.3667617578220317, 3.703839316684887, 4.098607599735211, 4.490071039515135, 4.879192386567447, 5.266673871161598, 5.6530296960510835, 6.038639278868162, 6.423784955842712, 6.808678484059117, 7.1934797498117895, 7.578310096881422, 7.9632619279700085, 8.348405702022065, 8.733795092290785, 9.119470830182808, 9.505463599150936, 9.891796234154242, 10.278485408041451, 10.665542934966393, 11.05297678524097, 11.440791880843797, 11.828990722803214, 12.217573888696098, 12.606540429161571, 13.385614043672865, 14.166184035906006, 14.948213302277296, 15.731659164963906, 16.51647601334669, 17.302617053034968, 18.09003546045466, 18.87868513730469, 19.66852119352658, 20.459500245021562, 21.251580584675224, 22.044722266798285, 22.838887132598156, 23.634038796019215, 24.430142603281702, 25.22716557558935, 26.025076341551085, 26.823845063912813, 27.62344336382784, 28.423844244828874, 29.22502201803638, 30.026952229552563, 30.82961159071451, 31.63297791152784, 32.43703003753926, 33.24174779014256, 34.047111910360435, 34.85310400598881, 35.65970650197814, 36.46690259392711},
    {0.05029020480396514, 0.10058040960573261, 0.15087061441034225, 0.2011608192121097, 0.2514510240167194, 0.301741228821329, 0.35203143362309647, 0.40232163842770624, 0.45261184322947356, 0.5029020480340831, 0.5531922528358507, 0.6034824576404602, 0.6537726624450699, 0.7040628672468374, 0.7543530720514469, 0.8046432768532146, 0.8549334816578241, 0.9052236864595916, 0.9555138912642014, 1.0058040960688106, 1.1315296080760717, 1.2572551200833324, 1.382980632093436, 1.5087061441006968, 1.6344316561107992, 1.7601571681180603, 1.8858826801253215, 2.0116081921354243, 2.137333704142684, 2.263059216152788, 2.3887847281600485, 2.514510240170152, 2.765961264184674, 3.017412288202038, 3.2688633122194015, 3.520314336236765, 3.7717653602541295, 4.023216384271493, 4.402923686706604, 4.840923169412718, 5.275628140231794, 5.708106387011112, 6.139096012777341, 6.569117835239806, 6.998545742885471, 7.427651941064436, 7.856636809801444, 8.28564907733453, 8.714799743029685, 9.144171870687785, 9.573827594895418, 10.00381321001211, 10.43416291693517, 10.864901615202994, 11.296047006509264, 11.72761119486633, 12.159601914603718, 12.592023480161025, 13.024877525833228, 13.458163585558754, 13.891879549849177, 14.326022027631314, 15.195568219985953, 16.066758980781113, 16.93954397110994, 17.813869202173244, 18.68967930447209, 19.566918991345418, 20.44553399181503, 21.325471630737475, 22.206681173178218, 23.089114010781095, 23.972723742457497, 24.857466184805716, 25.743299336515662, 26.63018331333154, 27.518080264958197, 28.4069542817556, 29.296771296561822, 30.187498985305254, 31.07910666877645, 31.97156521721369, 32.86484695863455, 33.75892559154721, 34.653776102328635, 35.54937468734068, 36.44569867980954, 37.34272648128694, 38.24043749755077, 39.13881207875172, 40.03783146350794, 40.937477726773984},
    {0.05974016468300759, 0.11948032936381749, 0.17922049404462742, 0.23896065872827943, 0.29870082340908927, 0.35844098808989927, 0.4181811527735513, 0.47792131745436117, 0.5376614821380133, 0.5974016468188231, 0.657141811499633, 0.7168819761832851, 0.7766221408640949, 0.8363623055449051, 0.8961024702285572, 0.955842634909367, 1.0155827995930191, 1.0753229642738293, 1.135063128954639, 1.1948032936382913, 1.344153705341737, 1.493504117048025, 1.6428545287514704, 1.7922049404549163, 1.9415553521612046, 2.0909057638646504, 2.2402561755709387, 2.389606587274385, 2.5389569989806726, 2.688307410684118, 2.8376578223904056, 2.987008234093852, 3.2857090575035857, 3.58440988091332, 3.8831107043230535, 4.181811527729945, 4.48051235113968, 4.779213174549414, 5.204876111923692, 5.688848562937568, 6.170159044583231, 6.6497495644421285, 7.128261753669921, 7.606142949678828, 8.083710709627423, 8.561193555361694, 9.038757543068261, 9.516524068724227, 9.99458208377549, 10.47299664830626, 10.951815027051936, 11.431071102373838, 11.910788612986488, 12.39098355983018, 12.871666012589971, 13.352841479046571, 13.834511951858257, 14.316676714667919, 14.799332966945224, 15.282476311040416, 15.766101133711286, 16.250200906162327, 17.219795978620663, 18.1911988298489, 19.164342959599296, 20.139160814458737, 21.115585503946967, 22.093551853025446, 23.07299702417972, 24.053860858945782, 25.036086036434696, 26.019618113131642, 27.004405486541614, 27.990399311257423, 28.977553386435417, 29.965824027565322, 30.95516993094154, 31.94555203660525, 32.93693339332716, 33.92927902800424, 34.92255582087156, 35.91673238727074, 36.91177896637648, 37.907667316880286, 38.90437061956949, 39.90186338652862, 40.90012137664726, 41.899121517093974, 42.89884183037187, 43.8992613666098, 44.90036014068208, 45.90211907383619},
    {0.07067515415184052, 0.14135030830148337, 0.2120254624511261, 0.2827006166036112, 0.35337577075325394, 0.4240509249028968, 0.49472607905538185, 0.5654012332050246, 0.6360763873575095, 0.7067515415071527, 0.7774266956567951, 0.8481018498092803, 0.9187770039589231, 0.9894521581085659, 1.060127312261051, 1.1308024664106937, 1.2014776205603366, 1.2721527747128218, 1.3428279288624643, 1.41350308301495, 1.5901909683904774, 1.7668788537660056, 1.943566739144376, 2.120254624519904, 2.2969425098982743, 2.473630395273803, 2.6503182806493313, 2.8270061660277013, 3.0036940514032295, 3.1803819367816004, 3.3570698221571273, 3.5337577075354982, 3.8871334782865548, 4.240509249040453, 4.593885019794351, 4.947260790548251, 5.300636561302148, 5.654012332053204, 6.131630444292413, 6.6672022906208355, 7.200987739963464, 7.733716006955079, 8.265882445019285, 8.797830454475601, 9.32980159851235, 9.861967438834494, 10.394450401173598, 10.927337816806162, 11.460691587201293, 11.994554966496775, 12.528957402009762, 13.06391803975272, 13.599448295828148, 14.135553763816139, 14.672235643520898, 15.209491820318211, 15.747317686564726, 16.285706770680626, 16.82465122144653, 17.364142182462015, 17.904170082649248, 18.444724862108774, 19.52737339128757, 20.612003289072593, 21.69853012990063, 22.786871401229217, 23.87694757433194, 24.968682680039276, 26.062004572321317, 27.156844995848896, 28.253139532174295, 29.350827472729005, 30.449851649907924, 31.55015824661291, 32.651696597281045, 33.75441898881355, 34.85828046658357, 35.96323864863796, 37.06925354984671, 38.176287416815526, 39.2843045737864, 40.39327127936956, 41.50315559373177, 42.613927255723794, 43.72555756930443, 44.838019298684486, 45.951286571522466, 47.065334789581065, 48.18014054623036, 49.29568155028052, 50.41193655559852, 51.52888529603219},
    {0.0834623671324666, 0.16692473426273555, 0.2503871013958466, 0.33384946852611547, 0.4173118356592266, 0.5007742027894957, 0.5842365699226064, 0.6676989370528754, 0.7511613041859866, 0.8346236713162553, 0.9180860384493664, 1.0015484055796355, 1.0850107727127467, 1.1684731398430155, 1.2519355069761264, 1.3353978741063957, 1.4188602412395066, 1.5023226083697758, 1.5857849755028866, 1.6692473426331556, 1.8779032604630912, 2.0865591782930264, 2.29521509612012, 2.5038710139500555, 2.7125269317799905, 2.921182849609927, 3.1298387674370196, 3.338494685266956, 3.547150603096892, 3.7558065209268268, 3.96446243875392, 4.173118356583855, 4.590430192243725, 5.007742027900754, 5.425053863557784, 5.842365699217655, 6.259677534874682, 6.676989370534554, 7.215284811865045, 7.810942636066625, 8.405713438558344, 9.000109193262631, 9.594476065866042, 10.189051324756685, 10.783998757795331, 11.379431470741277, 11.975426983124095, 12.572037465005534, 13.169296821117609, 13.767225679430393, 14.365834957075359, 14.965128442509108, 15.565104686191797, 16.165758398192054, 16.767081489665355, 17.369063854126264, 17.971693956723634, 18.574959280506526, 19.178846665295886, 19.78334256540095, 20.38843324553279, 20.994104929469565, 22.20713664504607, 23.42233029997001, 24.63958259994195, 25.858795291719396, 27.079875555554814, 28.302736078822026, 29.527294941445533, 30.75347539415742, 31.981205580215, 33.21041823195959, 34.44105036161415, 35.67304295805245, 36.90634069627536, 38.14089166337484, 39.3766471025938, 40.61356117597085, 41.851590745280404, 43.090695170440334, 44.33083612445286, 45.57197742365028, 46.814084872185866, 48.05712611949615, 49.301070529748834, 50.54588906213925, 51.79155416112571, 53.0380396556748, 54.285320666751495, 55.533373522224736, 56.782175678619716, 58.031705649011975},
    {0.09864342769295685, 0.19728685538371601, 0.2959302830773173, 0.39457371076807657, 0.49321713846167803, 0.5918605661524371, 0.6905039938460386, 0.7891474215367978, 0.887790849230399, 0.9864342769211584, 1.0850777046147597, 1.1837211323055188, 1.2823645599991202, 1.38100798768988, 1.4796514153834808, 1.5782948430742403, 1.6769382707678415, 1.7755816984586006, 1.874225126152202, 1.972868553842961, 2.219477123074122, 2.4660856923052834, 2.7126942615364444, 2.9593028307647646, 3.2059113999959257, 3.4525199692270867, 3.6991285384582473, 3.945737107686567, 4.192345676917727, 4.438954246148888, 4.68556281538005, 4.932171384608369, 5.425388523070691, 5.918605661530172, 6.411822799992494, 6.905039938451975, 7.398257076914296, 7.891474215373778, 8.502335361374866, 9.169831074405188, 9.837265003168877, 10.504960425335845, 11.173131332846934, 11.841919360545138, 12.511417180930938, 13.181683762841976, 13.85275456878272, 14.524648510968875, 15.197372779403622, 15.870926242469201, 16.545301871890324, 17.220488489846183, 17.896472038316816, 18.573236507380663, 19.250764617377587, 19.929038321651447, 20.608039177427425, 21.287748619021162, 21.968148158299684, 22.64921953063529, 23.330944799863957, 24.013306432352234, 25.37987095153259, 26.74878238692951, 28.119918223260417, 29.4931641334551, 30.8684136985658, 32.245567980998565, 33.624535031317535, 35.005229375702534, 36.38757151118004, 37.77148742362557, 39.156908136182956, 40.54376929127801, 41.93201076678101, 43.32157632534552, 44.712413295099445, 46.1044722794653, 47.497706893719666, 48.892073525949804, 50.2875311201024, 51.68404097903562, 53.081566585544806, 54.48007343965984, 55.87952891050276, 57.27990210130182, 58.68116372625177, 60.08328599798667, 61.48624252468206, 62.890008215820075, 64.29455919573915, 65.6998727243063},
};


/****************************************************************************/
