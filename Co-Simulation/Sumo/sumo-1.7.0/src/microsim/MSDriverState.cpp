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
/// @file    MSDriverState.cpp
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @author  Michael Behrisch
/// @date    Thu, 12 Jun 2014
///
// The common superclass for modelling transportable objects like persons and containers
/****************************************************************************/
#include <config.h>

#include <math.h>
#include <cmath>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOTime.h>
//#include <microsim/MSVehicle.h>
#include <microsim/transportables/MSPerson.h>
//#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
//#include <microsim/MSGlobals.h>
//#include <microsim/MSNet.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include "MSDriverState.h"

// ===========================================================================
// DEBUG constants
// ===========================================================================
//#define DEBUG_OUPROCESS
//#define DEBUG_TRAFFIC_ITEMS
//#define DEBUG_AWARENESS
//#define DEBUG_PERCEPTION_ERRORS
//#define DEBUG_DRIVERSTATE
#define DEBUG_COND (true)
//#define DEBUG_COND (myVehicle->isSelected())


/* -------------------------------------------------------------------------
 * static member definitions
 * ----------------------------------------------------------------------- */
// hash function
//std::hash<std::string> MSDriverState::MSTrafficItem::hash = std::hash<std::string>();
std::mt19937 OUProcess::myRNG;

// ===========================================================================
// Default value definitions
// ===========================================================================
//double TCIDefaults::myMinTaskCapability = 0.1;
//double TCIDefaults::myMaxTaskCapability = 10.0;
//double TCIDefaults::myMaxTaskDemand = 20.0;
//double TCIDefaults::myMaxDifficulty = 10.0;
//double TCIDefaults::mySubCriticalDifficultyCoefficient = 0.1;
//double TCIDefaults::mySuperCriticalDifficultyCoefficient = 1.0;
//double TCIDefaults::myOppositeDirectionDrivingFactor = 1.3;
//double TCIDefaults::myHomeostasisDifficulty = 1.5;
//double TCIDefaults::myCapabilityTimeScale = 0.5;
//double TCIDefaults::myAccelerationErrorTimeScaleCoefficient = 1.0;
//double TCIDefaults::myAccelerationErrorNoiseIntensityCoefficient = 1.0;
//double TCIDefaults::myActionStepLengthCoefficient = 1.0;
//double TCIDefaults::myMinActionStepLength = 0.0;
//double TCIDefaults::myMaxActionStepLength = 3.0;
//double TCIDefaults::mySpeedPerceptionErrorTimeScaleCoefficient = 1.0;
//double TCIDefaults::mySpeedPerceptionErrorNoiseIntensityCoefficient = 1.0;
//double TCIDefaults::myHeadwayPerceptionErrorTimeScaleCoefficient = 1.0;
//double TCIDefaults::myHeadwayPerceptionErrorNoiseIntensityCoefficient = 1.0;

double DriverStateDefaults::minAwareness = 0.1;
double DriverStateDefaults::initialAwareness = 1.0;
double DriverStateDefaults::errorTimeScaleCoefficient = 100.0;
double DriverStateDefaults::errorNoiseIntensityCoefficient = 0.2;
double DriverStateDefaults::speedDifferenceErrorCoefficient = 0.15;
double DriverStateDefaults::headwayErrorCoefficient = 0.75;
double DriverStateDefaults::speedDifferenceChangePerceptionThreshold = 0.1;
double DriverStateDefaults::headwayChangePerceptionThreshold = 0.1;
double DriverStateDefaults::maximalReactionTimeFactor = 1.0;


// ===========================================================================
// method definitions
// ===========================================================================

OUProcess::OUProcess(double initialState, double timeScale, double noiseIntensity)
    : myState(initialState),
      myTimeScale(timeScale),
      myNoiseIntensity(noiseIntensity) {}


OUProcess::~OUProcess() {}


void
OUProcess::step(double dt) {
#ifdef DEBUG_OUPROCESS
    const double oldstate = myState;
#endif
    myState = exp(-dt / myTimeScale) * myState + myNoiseIntensity * sqrt(2 * dt / myTimeScale) * RandHelper::randNorm(0, 1, &myRNG);
#ifdef DEBUG_OUPROCESS
    std::cout << "  OU-step (" << dt << " s.): " << oldstate << "->" << myState << std::endl;
#endif
}

double
OUProcess::step(double state, double dt, double timeScale, double noiseIntensity) {
    /// see above
    return exp(-dt / timeScale) * state + noiseIntensity * sqrt(2 * dt / timeScale) * RandHelper::randNorm(0, 1, &myRNG);
}

double
OUProcess::getState() const {
    return myState;
}


MSSimpleDriverState::MSSimpleDriverState(MSVehicle* veh) :
    myVehicle(veh),
    myAwareness(1.),
    myMinAwareness(DriverStateDefaults::minAwareness),
    myError(0., 1., 1.),
    myErrorTimeScaleCoefficient(DriverStateDefaults::errorTimeScaleCoefficient),
    myErrorNoiseIntensityCoefficient(DriverStateDefaults::errorNoiseIntensityCoefficient),
    mySpeedDifferenceErrorCoefficient(DriverStateDefaults::speedDifferenceErrorCoefficient),
    myHeadwayErrorCoefficient(DriverStateDefaults::headwayErrorCoefficient),
    myHeadwayChangePerceptionThreshold(DriverStateDefaults::headwayChangePerceptionThreshold),
    mySpeedDifferenceChangePerceptionThreshold(DriverStateDefaults::speedDifferenceChangePerceptionThreshold),
    myOriginalReactionTime(veh->getActionStepLengthSecs()),
    myMaximalReactionTime(DriverStateDefaults::maximalReactionTimeFactor * myOriginalReactionTime),
//    myActionStepLength(TS),
    myStepDuration(TS),
    myLastUpdateTime(SIMTIME - TS),
    myDebugLock(false) {
#ifdef DEBUG_DRIVERSTATE
    std::cout << "Constructing driver state for veh '" << veh->getID() << "'." << std::endl;
#endif
    updateError();
    updateReactionTime();
}


void
MSSimpleDriverState::update() {
#ifdef DEBUG_AWARENESS
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << myVehicle->getID() << ", DriverState::update()" << std::endl;
    }
#endif
    // Adapt step duration
    updateStepDuration();
    // Update error
    updateError();
    // Update actionStepLength, aka reaction time
    updateReactionTime();
    // Update assumed gaps
    updateAssumedGaps();
#ifdef DEBUG_AWARENESS
    if (DEBUG_COND) {
        std::cout << SIMTIME << " stepDuration=" << myStepDuration << ", error=" << myError.getState() << std::endl;
    }
#endif
}

void
MSSimpleDriverState::updateStepDuration() {
    myStepDuration = SIMTIME - myLastUpdateTime;
    myLastUpdateTime = SIMTIME;
}

void
MSSimpleDriverState::updateError() {
    if (myAwareness == 1.0 || myAwareness == 0.0) {
        myError.setState(0.);
    } else {
        myError.setTimeScale(myErrorTimeScaleCoefficient * myAwareness);
        myError.setNoiseIntensity(myErrorNoiseIntensityCoefficient * (1. - myAwareness));
        myError.step(myStepDuration);
    }
}

void
MSSimpleDriverState::updateReactionTime() {
    if (myAwareness == 1.0 || myAwareness == 0.0) {
        myActionStepLength = myOriginalReactionTime;
    } else {
        const double theta = (myAwareness - myMinAwareness) / (1.0 - myMinAwareness);
        myActionStepLength = myOriginalReactionTime + theta * (myMaximalReactionTime - myOriginalReactionTime);
        // Round to multiple of simstep length
        int quotient;
        remquo(myActionStepLength, TS, &quotient);
        myActionStepLength = TS * MAX2(quotient, 1);
    }
}

void
MSSimpleDriverState::setAwareness(const double value) {
    assert(value >= 0.);
    assert(value <= 1.);
#ifdef DEBUG_AWARENESS
    if (DEBUG_COND) {
        std::cout << SIMTIME << " veh=" << myVehicle->getID() << ", setAwareness(" << MAX2(value, myMinAwareness) << ")" << std::endl;
    }
#endif
    myAwareness = MAX2(value, myMinAwareness);
    if (myAwareness == 1.) {
        myError.setState(0.);
    }
    updateReactionTime();
}


double
MSSimpleDriverState::getPerceivedHeadway(const double trueGap, const void* objID) {
#ifdef DEBUG_PERCEPTION_ERRORS
    if (DEBUG_COND) {
        if (!debugLocked()) {
            std::cout << SIMTIME << " getPerceivedHeadway() for veh '" << myVehicle->getID() << "'\n"
                      << "    trueGap=" << trueGap << " objID=" << objID << std::endl;
        }
    }
#endif

    const double perceivedGap = trueGap + myHeadwayErrorCoefficient * myError.getState() * trueGap;
    const auto assumedGap = myAssumedGap.find(objID);
    if (assumedGap == myAssumedGap.end()
            || fabs(perceivedGap - assumedGap->second) > myHeadwayChangePerceptionThreshold * trueGap * (1.0 - myAwareness)) {

#ifdef DEBUG_PERCEPTION_ERRORS
        if (!debugLocked()) {
            std::cout << "    new perceived gap (=" << perceivedGap << ") differs significantly from the assumed (="
                      << (assumedGap == myAssumedGap.end() ? "NA" : toString(assumedGap->second)) << ")" << std::endl;
        }
#endif

        // new perceived gap differs significantly from the previous
        myAssumedGap[objID] = perceivedGap;
        return perceivedGap;
    } else {

#ifdef DEBUG_PERCEPTION_ERRORS
        if (DEBUG_COND) {
            if (!debugLocked()) {
                std::cout << "    new perceived gap (=" << perceivedGap << ") does *not* differ significantly from the assumed (="
                          << (assumedGap->second) << ")" << std::endl;
            }
        }
#endif
        // new perceived gap doesn't differ significantly from the previous
        return myAssumedGap[objID];
    }
}

void
MSSimpleDriverState::updateAssumedGaps() {
    for (auto& p : myAssumedGap) {
        const void* objID = p.first;
        const auto speedDiff = myLastPerceivedSpeedDifference.find(objID);
        double assumedSpeedDiff;
        if (speedDiff != myLastPerceivedSpeedDifference.end()) {
            // update the assumed gap with the last perceived speed difference
            assumedSpeedDiff = speedDiff->second;
        } else {
            // Assume the object is not moving, if no perceived speed difference is known.
            assumedSpeedDiff = -myVehicle->getSpeed();
        }
        p.second += SPEED2DIST(assumedSpeedDiff);
    }
}

double
MSSimpleDriverState::getPerceivedSpeedDifference(const double trueSpeedDifference, const double trueGap, const void* objID) {
#ifdef DEBUG_PERCEPTION_ERRORS
    if (DEBUG_COND) {
        if (!debugLocked()) {
            std::cout << SIMTIME << " getPerceivedSpeedDifference() for veh '" << myVehicle->getID() << "'\n"
                      << "    trueGap=" << trueGap << " trueSpeedDifference=" << trueSpeedDifference << " objID=" << objID << std::endl;
        }
    }
#endif
    const double perceivedSpeedDifference = trueSpeedDifference + mySpeedDifferenceErrorCoefficient * myError.getState() * trueGap;
    const auto lastPerceivedSpeedDifference = myLastPerceivedSpeedDifference.find(objID);
    if (lastPerceivedSpeedDifference == myLastPerceivedSpeedDifference.end()
            || fabs(perceivedSpeedDifference - lastPerceivedSpeedDifference->second) > mySpeedDifferenceChangePerceptionThreshold * trueGap * (1.0 - myAwareness)) {

#ifdef DEBUG_PERCEPTION_ERRORS
        if (DEBUG_COND) {
            if (!debugLocked()) {
                std::cout << "    new perceived speed difference (=" << perceivedSpeedDifference << ") differs significantly from the last perceived (="
                          << (lastPerceivedSpeedDifference == myLastPerceivedSpeedDifference.end() ? "NA" : toString(lastPerceivedSpeedDifference->second)) << ")"
                          << std::endl;
            }
        }
#endif

        // new perceived speed difference differs significantly from the previous
        myLastPerceivedSpeedDifference[objID] = perceivedSpeedDifference;
        return perceivedSpeedDifference;
    } else {
#ifdef DEBUG_PERCEPTION_ERRORS
        if (!debugLocked()) {
            std::cout << "    new perceived speed difference (=" << perceivedSpeedDifference << ") does *not* differ significantly from the last perceived (="
                      << (lastPerceivedSpeedDifference->second) << ")" << std::endl;
        }
#endif
        // new perceived speed difference doesn't differ significantly from the previous
        return lastPerceivedSpeedDifference->second;
    }
}


//MSDriverState::MSTrafficItem::MSTrafficItem(MSTrafficItemType type, const std::string& id, std::shared_ptr<MSTrafficItemCharacteristics> data) :
//    type(type),
//    id_hash(hash(id)),
//    data(data),
//    remainingIntegrationTime(0.),
//    integrationDemand(0.),
//    latentDemand(0.)
//{}
//
//MSDriverState::MSDriverState(MSVehicle* veh) :
//            myVehicle(veh),
//            myMinTaskCapability(TCIDefaults::myMinTaskCapability),
//            myMaxTaskCapability(TCIDefaults::myMaxTaskCapability),
//            myMaxTaskDemand(TCIDefaults::myMaxTaskDemand),
//            myMaxDifficulty(TCIDefaults::myMaxDifficulty),
//            mySubCriticalDifficultyCoefficient(TCIDefaults::mySubCriticalDifficultyCoefficient),
//            mySuperCriticalDifficultyCoefficient(TCIDefaults::mySuperCriticalDifficultyCoefficient),
//            myOppositeDirectionDrivingDemandFactor(TCIDefaults::myOppositeDirectionDrivingFactor),
//            myHomeostasisDifficulty(TCIDefaults::myHomeostasisDifficulty),
//            myCapabilityTimeScale(TCIDefaults::myCapabilityTimeScale),
//            myAccelerationErrorTimeScaleCoefficient(TCIDefaults::myAccelerationErrorTimeScaleCoefficient),
//            myAccelerationErrorNoiseIntensityCoefficient(TCIDefaults::myAccelerationErrorNoiseIntensityCoefficient),
//            myActionStepLengthCoefficient(TCIDefaults::myActionStepLengthCoefficient),
//            myMinActionStepLength(TCIDefaults::myMinActionStepLength),
//            myMaxActionStepLength(TCIDefaults::myMaxActionStepLength),
//            mySpeedPerceptionErrorTimeScaleCoefficient(TCIDefaults::mySpeedPerceptionErrorTimeScaleCoefficient),
//            mySpeedPerceptionErrorNoiseIntensityCoefficient(TCIDefaults::mySpeedPerceptionErrorNoiseIntensityCoefficient),
//            myHeadwayPerceptionErrorTimeScaleCoefficient(TCIDefaults::myHeadwayPerceptionErrorTimeScaleCoefficient),
//            myHeadwayPerceptionErrorNoiseIntensityCoefficient(TCIDefaults::myHeadwayPerceptionErrorNoiseIntensityCoefficient),
//            myAmOpposite(false),
//            myAccelerationError(0., 1.,1.),
//            myHeadwayPerceptionError(0., 1.,1.),
//            mySpeedPerceptionError(0., 1.,1.),
//            myTaskDemand(0.),
//            myTaskCapability(myMaxTaskCapability),
//            myCurrentDrivingDifficulty(myTaskDemand/myTaskCapability),
//            myActionStepLength(TS),
//            myStepDuration(TS),
//            myLastUpdateTime(SIMTIME-TS),
//            myCurrentSpeed(0.),
//            myCurrentAcceleration(0.)
//{}
//
//
//void
//MSDriverState::updateStepDuration() {
//    myStepDuration = SIMTIME - myLastUpdateTime;
//    myLastUpdateTime = SIMTIME;
//}
//
//
//void
//MSDriverState::calculateDrivingDifficulty() {
//    if (myAmOpposite) {
//        myCurrentDrivingDifficulty = difficultyFunction(myOppositeDirectionDrivingDemandFactor*myTaskDemand/myTaskCapability);
//    } else {
//        myCurrentDrivingDifficulty = difficultyFunction(myTaskDemand/myTaskCapability);
//    }
//}
//
//
//double
//MSDriverState::difficultyFunction(double demandCapabilityQuotient) const {
//    double difficulty;
//    if (demandCapabilityQuotient <= 1) {
//        // demand does not exceed capability -> we are in the region for a slight ascend of difficulty
//        difficulty = mySubCriticalDifficultyCoefficient*demandCapabilityQuotient;
//    } else {
//        // demand exceeds capability -> we are in the region for a steeper ascend of the effect of difficulty
//        difficulty = mySubCriticalDifficultyCoefficient + (demandCapabilityQuotient - 1)*mySuperCriticalDifficultyCoefficient;
//    }
//    return MIN2(myMaxDifficulty, difficulty);
//}
//
//
//void
//MSDriverState::adaptTaskCapability() {
//    myTaskCapability = myTaskCapability + myCapabilityTimeScale*myStepDuration*(myTaskDemand - myHomeostasisDifficulty*myTaskCapability);
//}
//
//
//void
//MSDriverState::updateAccelerationError() {
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << SIMTIME << " Updating acceleration error (for " << myStepDuration << " s.):\n  "
//                << myAccelerationError.getState() << " -> ";
//    }
//#endif
//
//    updateErrorProcess(myAccelerationError, myAccelerationErrorTimeScaleCoefficient, myAccelerationErrorNoiseIntensityCoefficient);
//
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << myAccelerationError.getState() << std::endl;
//    }
//#endif
//}
//
//void
//MSDriverState::updateSpeedPerceptionError() {
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << SIMTIME << " Updating speed perception error (for " << myStepDuration << " s.):\n  "
//        << mySpeedPerceptionError.getState() << " -> ";
//    }
//#endif
//
//    updateErrorProcess(mySpeedPerceptionError, mySpeedPerceptionErrorTimeScaleCoefficient, mySpeedPerceptionErrorNoiseIntensityCoefficient);
//
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << mySpeedPerceptionError.getState() << std::endl;
//    }
//#endif
//}
//
//void
//MSDriverState::updateHeadwayPerceptionError() {
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << SIMTIME << " Updating headway perception error (for " << myStepDuration << " s.):\n  "
//        << myHeadwayPerceptionError.getState() << " -> ";
//    }
//#endif
//
//    updateErrorProcess(myHeadwayPerceptionError, myHeadwayPerceptionErrorTimeScaleCoefficient, myHeadwayPerceptionErrorNoiseIntensityCoefficient);
//
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << myHeadwayPerceptionError.getState() << std::endl;
//    }
//#endif
//}
//
//void
//MSDriverState::updateActionStepLength() {
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << SIMTIME << " Updating action step length (for " << myStepDuration << " s.): \n" << myActionStepLength;
//    }
//#endif
//    if (myActionStepLengthCoefficient*myCurrentDrivingDifficulty <= myMinActionStepLength) {
//        myActionStepLength = myMinActionStepLength;
//    } else {
//        myActionStepLength = MIN2(myActionStepLengthCoefficient*myCurrentDrivingDifficulty - myMinActionStepLength, myMaxActionStepLength);
//    }
//#ifdef DEBUG_OUPROCESS
//    if (DEBUG_COND) {
//        std::cout << " -> " << myActionStepLength << std::endl;
//    }
//#endif
//}
//
//
//void
//MSDriverState::updateErrorProcess(OUProcess& errorProcess, double timeScaleCoefficient, double noiseIntensityCoefficient) const {
//    if (myCurrentDrivingDifficulty == 0) {
//        errorProcess.setState(0.);
//    } else {
//        errorProcess.setTimeScale(timeScaleCoefficient/myCurrentDrivingDifficulty);
//        errorProcess.setNoiseIntensity(myCurrentDrivingDifficulty*noiseIntensityCoefficient);
//        errorProcess.step(myStepDuration);
//    }
//}
//
//void
//MSDriverState::registerLeader(const MSVehicle* leader, double gap, double relativeSpeed, double latGap) {
//    std::shared_ptr<MSTrafficItemCharacteristics> tic = std::dynamic_pointer_cast<MSTrafficItemCharacteristics>(std::make_shared<VehicleCharacteristics>(leader, gap, latGap, relativeSpeed));
//    std::shared_ptr<MSTrafficItem> ti = std::make_shared<MSTrafficItem>(TRAFFIC_ITEM_VEHICLE, leader->getID(), tic);
//    registerTrafficItem(ti);
//}
//
//void
//MSDriverState::registerPedestrian(const MSPerson* pedestrian, double gap) {
//    std::shared_ptr<MSTrafficItemCharacteristics> tic = std::dynamic_pointer_cast<MSTrafficItemCharacteristics>(std::make_shared<PedestrianCharacteristics>(pedestrian, gap));
//    std::shared_ptr<MSTrafficItem> ti = std::make_shared<MSTrafficItem>(TRAFFIC_ITEM_PEDESTRIAN, pedestrian->getID(), tic);
//    registerTrafficItem(ti);
//}
//
//void
//MSDriverState::registerSpeedLimit(const MSLane* lane, double speedLimit, double dist) {
//    std::shared_ptr<MSTrafficItemCharacteristics> tic = std::dynamic_pointer_cast<MSTrafficItemCharacteristics>(std::make_shared<SpeedLimitCharacteristics>(lane, dist, speedLimit));
//    std::shared_ptr<MSTrafficItem> ti = std::make_shared<MSTrafficItem>(TRAFFIC_ITEM_SPEED_LIMIT, lane->getID(), tic);
//    registerTrafficItem(ti);
//}
//
//void
//MSDriverState::registerJunction(MSLink* link, double dist) {
//    const MSJunction* junction = link->getJunction();
//    std::shared_ptr<MSTrafficItemCharacteristics> tic = std::dynamic_pointer_cast<MSTrafficItemCharacteristics>(std::make_shared<JunctionCharacteristics>(junction, link, dist));
//    std::shared_ptr<MSTrafficItem> ti = std::make_shared<MSTrafficItem>(TRAFFIC_ITEM_JUNCTION, junction->getID(), tic);
//    registerTrafficItem(ti);
//}
//
//void
//MSDriverState::registerEgoVehicleState() {
//    myAmOpposite = myVehicle->getLaneChangeModel().isOpposite();
//    myCurrentSpeed = myVehicle->getSpeed();
//    myCurrentAcceleration = myVehicle->getAcceleration();
//}
//
//void
//MSDriverState::update() {
//    // Adapt step duration
//    updateStepDuration();
//
//    // Replace traffic items from previous step with the newly encountered.
//    myTrafficItems = myNewTrafficItems;
//
//    // Iterate through present traffic items and take into account the corresponding
//    // task demands. Further update the item's integration progress.
//    for (auto& hashItemPair : myTrafficItems) {
//        // Traffic item
//        auto ti = hashItemPair.second;
//        // Take into account the task demand associated with the item
//        integrateDemand(ti);
//        // Update integration progress
//        if (ti->remainingIntegrationTime>0) {
//            updateItemIntegration(ti);
//        }
//    }
//
//    // Update capability (~attention) according to the changed demand
//    // NOTE: Doing this before recalculating the errors seems more adequate
//    //       than after adjusting the errors, since a very fast time scale
//    //       for the capability could not be captured otherwise. A slow timescale
//    //       could still be tuned to have a desired effect.
//    adaptTaskCapability();
//
//    // Update driving difficulty
//    calculateDrivingDifficulty();
//
//    // Update errors
//    updateAccelerationError();
//    updateSpeedPerceptionError();
//    updateHeadwayPerceptionError();
//    updateActionStepLength();
//}
//
//
//void
//MSDriverState::integrateDemand(std::shared_ptr<MSTrafficItem> ti) {
//    myMaxTaskDemand += ti->integrationDemand;
//    myMaxTaskDemand += ti->latentDemand;
//}
//
//
//void
//MSDriverState::registerTrafficItem(std::shared_ptr<MSTrafficItem> ti) {
//    if (myNewTrafficItems.find(ti->id_hash) == myNewTrafficItems.end()) {
//
//        // Update demand associated with the item
//        auto knownTiIt = myTrafficItems.find(ti->id_hash);
//        if (knownTiIt == myTrafficItems.end()) {
//            // new item --> init integration demand and latent task demand
//            calculateIntegrationDemandAndTime(ti);
//        } else {
//            // known item --> only update latent task demand associated with the item
//            ti = knownTiIt->second;
//        }
//        calculateLatentDemand(ti);
//
//        // Track item
//        myNewTrafficItems[ti->id_hash] = ti;
//    }
//}
//
//
//void
//MSDriverState::updateItemIntegration(std::shared_ptr<MSTrafficItem> ti) const {
//    // Eventually decrease integration time and take into account integration cost.
//    ti->remainingIntegrationTime -= myStepDuration;
//    if (ti->remainingIntegrationTime <= 0.) {
//        ti->remainingIntegrationTime = 0.;
//        ti->integrationDemand = 0.;
//    }
//}
//
//
//void
//MSDriverState::calculateIntegrationDemandAndTime(std::shared_ptr<MSTrafficItem> ti) const {
//    // @todo Idea is that the integration demand is the quantitatively the same for a specific
//    //       item type with definite characteristics but it can be stretched over time,
//    //       if the integration is less urgent (item farther away), thus resulting in
//    //       smaller effort for a longer time.
//    switch (ti->type) {
//    case TRAFFIC_ITEM_JUNCTION: {
//        std::shared_ptr<JunctionCharacteristics> ch = std::dynamic_pointer_cast<JunctionCharacteristics>(ti->data);
//        const double totalIntegrationDemand = calculateJunctionIntegrationDemand(ch);
//        const double integrationTime = calculateIntegrationTime(ch->dist, myVehicle->getSpeed());
//        ti->integrationDemand = totalIntegrationDemand/integrationTime;
//        ti->remainingIntegrationTime = integrationTime;
//    }
//    break;
//    case TRAFFIC_ITEM_PEDESTRIAN: {
//        std::shared_ptr<PedestrianCharacteristics> ch = std::dynamic_pointer_cast<PedestrianCharacteristics>(ti->data);
//        const double totalIntegrationDemand = calculatePedestrianIntegrationDemand(ch);
//        const double integrationTime = calculateIntegrationTime(ch->dist, myVehicle->getSpeed());
//        ti->integrationDemand = totalIntegrationDemand/integrationTime;
//        ti->remainingIntegrationTime = integrationTime;
//    }
//    break;
//    case TRAFFIC_ITEM_SPEED_LIMIT: {
//        std::shared_ptr<SpeedLimitCharacteristics> ch = std::dynamic_pointer_cast<SpeedLimitCharacteristics>(ti->data);
//        const double totalIntegrationDemand = calculateSpeedLimitIntegrationDemand(ch);
//        const double integrationTime = calculateIntegrationTime(ch->dist, myVehicle->getSpeed());
//        ti->integrationDemand = totalIntegrationDemand/integrationTime;
//        ti->remainingIntegrationTime = integrationTime;
//    }
//    break;
//    case TRAFFIC_ITEM_VEHICLE: {
//        std::shared_ptr<VehicleCharacteristics> ch = std::dynamic_pointer_cast<VehicleCharacteristics>(ti->data);
//        ti->latentDemand = calculateLatentVehicleDemand(ch);
//        const double totalIntegrationDemand = calculateVehicleIntegrationDemand(ch);
//        const double integrationTime = calculateIntegrationTime(ch->longitudinalDist, ch->relativeSpeed);
//        ti->integrationDemand = totalIntegrationDemand/integrationTime;
//        ti->remainingIntegrationTime = integrationTime;
//    }
//    break;
//    default:
//        WRITE_WARNING("Unknown traffic item type!")
//        break;
//    }
//}
//
//
//double
//MSDriverState::calculatePedestrianIntegrationDemand(std::shared_ptr<PedestrianCharacteristics> ch) const {
//    // Integration demand for a pedestrian
//    const double INTEGRATION_DEMAND_PEDESTRIAN = 0.5;
//    return INTEGRATION_DEMAND_PEDESTRIAN;
//}
//
//
//double
//MSDriverState::calculateSpeedLimitIntegrationDemand(std::shared_ptr<SpeedLimitCharacteristics> ch) const {
//    // Integration demand for speed limit
//    const double INTEGRATION_DEMAND_SPEEDLIMIT = 0.1;
//    return INTEGRATION_DEMAND_SPEEDLIMIT;
//}
//
//
//double
//MSDriverState::calculateJunctionIntegrationDemand(std::shared_ptr<JunctionCharacteristics> ch) const {
//    // Latent demand for junction is proportional to number of conflicting lanes
//    // for the vehicle's path plus a factor for the total number of incoming lanes
//    // at the junction. Further, the distance to the junction is inversely proportional
//    // to the induced demand [~1/(c*dist + 1)].
//    // Traffic lights induce an additional demand
//    const MSJunction* j = ch->junction;
//
//    // Basic junction integration demand
//    const double INTEGRATION_DEMAND_JUNCTION_BASE = 0.3;
//
//    // Surplus integration demands
//    const double INTEGRATION_DEMAND_JUNCTION_TLS = 0.2;
//    const double INTEGRATION_DEMAND_JUNCTION_FOE_LANE = 0.3; // per foe lane
//    const double INTEGRATION_DEMAND_JUNCTION_LANE = 0.1; // per lane
//    const double INTEGRATION_DEMAND_JUNCTION_RAIL = 0.2;
//    const double INTEGRATION_DEMAND_JUNCTION_ZIPPER = 0.3;
//
//    double result = INTEGRATION_DEMAND_JUNCTION_BASE;
////    LinkState linkState = ch->approachingLink->getState();
//    switch (ch->junction->getType()) {
//    case SumoXMLNodeType::NOJUNCTION:
//    case SumoXMLNodeType::UNKNOWN:
//    case SumoXMLNodeType::DISTRICT:
//    case SumoXMLNodeType::DEAD_END:
//    case SumoXMLNodeType::DEAD_END_DEPRECATED:
//    case SumoXMLNodeType::RAIL_SIGNAL: {
//        result = 0.;
//    }
//    break;
//    case SumoXMLNodeType::RAIL_CROSSING: {
//        result += INTEGRATION_DEMAND_JUNCTION_RAIL;
//    }
//    break;
//    case SumoXMLNodeType::TRAFFIC_LIGHT:
//    case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
//    case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED: {
//        // TODO: Take into account traffic light state?
////        switch (linkState) {
////        case LINKSTATE_TL_GREEN_MAJOR:
////        case LINKSTATE_TL_GREEN_MINOR:
////        case LINKSTATE_TL_RED:
////        case LINKSTATE_TL_REDYELLOW:
////        case LINKSTATE_TL_YELLOW_MAJOR:
////        case LINKSTATE_TL_YELLOW_MINOR:
////        case LINKSTATE_TL_OFF_BLINKING:
////        case LINKSTATE_TL_OFF_NOSIGNAL:
////        default:
////        }
//        result += INTEGRATION_DEMAND_JUNCTION_TLS;
//    }
//    // no break. TLS has extra integration demand.
//    case SumoXMLNodeType::PRIORITY:
//    case SumoXMLNodeType::PRIORITY_STOP:
//    case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
//    case SumoXMLNodeType::ALLWAY_STOP:
//    case SumoXMLNodeType::INTERNAL: {
//        // TODO: Consider link type (major or minor...)
//        double junctionComplexity = (INTEGRATION_DEMAND_JUNCTION_LANE*j->getNrOfIncomingLanes()
//                + INTEGRATION_DEMAND_JUNCTION_FOE_LANE*j->getFoeLinks(ch->approachingLink).size());
//        result += junctionComplexity;
//    }
//    break;
//    case SumoXMLNodeType::ZIPPER: {
//        result += INTEGRATION_DEMAND_JUNCTION_ZIPPER;
//    }
//    break;
//    default:
//        assert(false);
//        result = 0.;
//    }
//    return result;
//
//}
//
//
//double
//MSDriverState::calculateVehicleIntegrationDemand(std::shared_ptr<VehicleCharacteristics> ch) const {
//    // TODO
//    return 0.;
//}
//
//
//double
//MSDriverState::calculateIntegrationTime(double dist, double speed) const {
//    // Fraction of encounter time, which is accounted for the corresponding traffic item's integration
//    const double INTEGRATION_TIME_COEFF = 0.5;
//    // Maximal time to be accounted for integration
//    const double MAX_INTEGRATION_TIME = 5.;
//    if (speed <= 0.) {
//        return MAX_INTEGRATION_TIME;
//    } else {
//        return MIN2(MAX_INTEGRATION_TIME, INTEGRATION_TIME_COEFF*dist/speed);
//    }
//}
//
//
//void
//MSDriverState::calculateLatentDemand(std::shared_ptr<MSTrafficItem> ti) const {
//    switch (ti->type) {
//    case TRAFFIC_ITEM_JUNCTION: {
//        std::shared_ptr<JunctionCharacteristics> ch = std::dynamic_pointer_cast<JunctionCharacteristics>(ti->data);
//        ti->latentDemand = calculateLatentJunctionDemand(ch);
//    }
//    break;
//    case TRAFFIC_ITEM_PEDESTRIAN: {
//        std::shared_ptr<PedestrianCharacteristics> ch = std::dynamic_pointer_cast<PedestrianCharacteristics>(ti->data);
//        ti->latentDemand = calculateLatentPedestrianDemand(ch);
//    }
//    break;
//    case TRAFFIC_ITEM_SPEED_LIMIT: {
//        std::shared_ptr<SpeedLimitCharacteristics> ch = std::dynamic_pointer_cast<SpeedLimitCharacteristics>(ti->data);
//        ti->latentDemand = calculateLatentSpeedLimitDemand(ch);
//    }
//    break;
//    case TRAFFIC_ITEM_VEHICLE: {
//        std::shared_ptr<VehicleCharacteristics> ch = std::dynamic_pointer_cast<VehicleCharacteristics>(ti->data);
//        ti->latentDemand = calculateLatentVehicleDemand(ch);
//    }
//    break;
//    default:
//        WRITE_WARNING("Unknown traffic item type!")
//        break;
//    }
//}
//
//
//double
//MSDriverState::calculateLatentPedestrianDemand(std::shared_ptr<PedestrianCharacteristics> ch) const {
//    // Latent demand for pedestrian is proportional to the euclidean distance to the
//    // pedestrian (i.e. its potential to 'jump in front of the car) [~1/(c*dist + 1)]
//    const double LATENT_DEMAND_COEFF_PEDESTRIAN_DIST = 0.1;
//    const double LATENT_DEMAND_COEFF_PEDESTRIAN = 0.5;
//    double result = LATENT_DEMAND_COEFF_PEDESTRIAN/(1. + LATENT_DEMAND_COEFF_PEDESTRIAN_DIST*ch->dist);
//    return result;
//}
//
//
//double
//MSDriverState::calculateLatentSpeedLimitDemand(std::shared_ptr<SpeedLimitCharacteristics> ch) const {
//    // Latent demand for speed limit is proportional to speed difference to current vehicle speed
//    // during approach [~c*(1+deltaV) if dist<threshold].
//    const double LATENT_DEMAND_COEFF_SPEEDLIMIT_TIME_THRESH = 5;
//    const double LATENT_DEMAND_COEFF_SPEEDLIMIT = 0.1;
//    double dist_thresh = LATENT_DEMAND_COEFF_SPEEDLIMIT_TIME_THRESH*myVehicle->getSpeed();
//    double result = 0.;
//    if (ch->dist <= dist_thresh && myVehicle->getSpeed() > ch->limit*myVehicle->getChosenSpeedFactor()) {
//        // Upcoming speed limit does require a slowdown and is close enough.
//        double dv = myVehicle->getSpeed() - ch->limit*myVehicle->getChosenSpeedFactor();
//        result = LATENT_DEMAND_COEFF_SPEEDLIMIT*(1 + dv);
//    }
//    return result;
//}
//
//
//double
//MSDriverState::calculateLatentVehicleDemand(std::shared_ptr<VehicleCharacteristics> ch) const {
//
//
//    // TODO
//
//
//    // Latent demand for neighboring vehicle is determined from the relative and absolute speed,
//    // and from the lateral and longitudinal distance.
//    double result = 0.;
//    const MSVehicle* foe = ch->foe;
//    if (foe->getEdge() == myVehicle->getEdge()) {
//        // on same edge
//    } else if (foe->getEdge() == myVehicle->getEdge()->getOppositeEdge()) {
//        // on opposite edges
//    }
//    return result;
//}
//
//
//
//double
//MSDriverState::calculateLatentJunctionDemand(std::shared_ptr<JunctionCharacteristics> ch) const {
//    // Latent demand for junction is proportional to number of conflicting lanes
//    // for the vehicle's path plus a factor for the total number of incoming lanes
//    // at the junction. Further, the distance to the junction is inversely proportional
//    // to the induced demand [~1/(c*dist + 1)].
//    // Traffic lights induce an additional demand
//    const MSJunction* j = ch->junction;
//    const double LATENT_DEMAND_COEFF_JUNCTION_TIME_DIST_THRESH = 5; // seconds till arrival, below which junction is relevant
//    const double LATENT_DEMAND_COEFF_JUNCTION_INCOMING = 0.1;
//    const double LATENT_DEMAND_COEFF_JUNCTION_FOES = 0.5;
//    const double LATENT_DEMAND_COEFF_JUNCTION_DIST = 0.1;
//
//    double v = myVehicle->getSpeed();
//    double dist_thresh = LATENT_DEMAND_COEFF_JUNCTION_TIME_DIST_THRESH*v;
//
//    if (ch->dist > dist_thresh) {
//        return 0.;
//    }
//    double result = 0.;
//    LinkState linkState = ch->approachingLink->getState();
//    switch (ch->junction->getType()) {
//    case SumoXMLNodeType::NOJUNCTION:
//    case SumoXMLNodeType::UNKNOWN:
//    case SumoXMLNodeType::DISTRICT:
//    case SumoXMLNodeType::DEAD_END:
//    case SumoXMLNodeType::DEAD_END_DEPRECATED:
//    case SumoXMLNodeType::RAIL_SIGNAL: {
//        result = 0.;
//    }
//    break;
//    case SumoXMLNodeType::RAIL_CROSSING: {
//        result = 0.5;
//    }
//    break;
//    case SumoXMLNodeType::TRAFFIC_LIGHT:
//    case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
//    case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED: {
//        // Take into account traffic light state
//        switch (linkState) {
//        case LINKSTATE_TL_GREEN_MAJOR:
//            result = 0;
//            break;
//        case LINKSTATE_TL_GREEN_MINOR:
//            result = 0.2*(1. + 0.1*v);
//            break;
//        case LINKSTATE_TL_RED:
//            result = 0.1*(1. + 0.1*v);
//            break;
//        case LINKSTATE_TL_REDYELLOW:
//            result = 0.2*(1. + 0.1*v);
//            break;
//        case LINKSTATE_TL_YELLOW_MAJOR:
//            result = 0.1*(1. + 0.1*v);
//            break;
//        case LINKSTATE_TL_YELLOW_MINOR:
//            result = 0.2*(1. + 0.1*v);
//            break;
//        case LINKSTATE_TL_OFF_BLINKING:
//            result = 0.3*(1. + 0.1*v);
//            break;
//        case LINKSTATE_TL_OFF_NOSIGNAL:
//            result = 0.2*(1. + 0.1*v);
//        }
//    }
//    // no break, TLS is accounted extra
//    case SumoXMLNodeType::PRIORITY:
//    case SumoXMLNodeType::PRIORITY_STOP:
//    case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
//    case SumoXMLNodeType::ALLWAY_STOP:
//    case SumoXMLNodeType::INTERNAL: {
//        // TODO: Consider link type (major or minor...)
//        double junctionComplexity = (LATENT_DEMAND_COEFF_JUNCTION_INCOMING*j->getNrOfIncomingLanes()
//                + LATENT_DEMAND_COEFF_JUNCTION_FOES*j->getFoeLinks(ch->approachingLink).size())
//                                             /(1 + ch->dist*LATENT_DEMAND_COEFF_JUNCTION_DIST);
//        result += junctionComplexity;
//    }
//    break;
//    case SumoXMLNodeType::ZIPPER: {
//        result = 0.5*(1. + 0.1*v);
//    }
//    break;
//    default:
//        assert(false);
//        result = 0.;
//    }
//    return result;
//}
//


/****************************************************************************/
