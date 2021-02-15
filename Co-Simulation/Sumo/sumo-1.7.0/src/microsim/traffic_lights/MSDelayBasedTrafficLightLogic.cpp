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
/// @file    MSDelayBasedTrafficLightLogic.cpp
/// @author  Leonhard Luecken
/// @date    Feb 2017
///
// An actuated traffic light logic based on time delay of approaching vehicles
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <vector>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/output/MSDetectorControl.h>
#include "MSSimpleTrafficLightLogic.h"
#include "MSDelayBasedTrafficLightLogic.h"
#include <microsim/MSLane.h>
#include <netload/NLDetectorBuilder.h>
#include <utils/common/StringUtils.h>

#define INVALID_POSITION std::numeric_limits<double>::max()

// ===========================================================================
// parameter defaults definitions
// ===========================================================================

//#define DEBUG_TIMELOSS_CONTROL

// ===========================================================================
// method definitions
// ===========================================================================
MSDelayBasedTrafficLightLogic::MSDelayBasedTrafficLightLogic(MSTLLogicControl& tlcontrol,
        const std::string& id, const std::string& programID,
        const Phases& phases,
        int step, SUMOTime delay,
        const std::map<std::string, std::string>& parameter,
        const std::string& basePath) :
    MSSimpleTrafficLightLogic(tlcontrol, id, programID, TrafficLightType::DELAYBASED, phases, step, delay, parameter) {
#ifdef DEBUG_TIMELOSS_CONTROL
    std::cout << "Building delay based tls logic '" << id << "'" << std::endl;
#endif
    myShowDetectors = StringUtils::toBool(getParameter("show-detectors", "false"));
    myDetectionRange = StringUtils::toDouble(getParameter("detectorRange", toString(OptionsCont::getOptions().getFloat("tls.delay_based.detector-range"))));
    myTimeLossThreshold = StringUtils::toDouble(getParameter("minTimeloss", "1.0"));
    myFile = FileHelpers::checkForRelativity(getParameter("file", "NUL"), basePath);
    myFreq = TIME2STEPS(StringUtils::toDouble(getParameter("freq", "300")));
    myVehicleTypes = getParameter("vTypes", "");
#ifdef DEBUG_TIMELOSS_CONTROL
    std::cout << "show-detectors: " << myShowDetectors
              << " detectorRange: " << myDetectionRange
              << " minTimeLoss: " << myTimeLossThreshold
              << " file: " << myFile
              << " freq: " << myFreq
              << " vTypes: " << myVehicleTypes
              << std::endl;
#endif
}


void
MSDelayBasedTrafficLightLogic::init(NLDetectorBuilder& nb) {
    MSTrafficLightLogic::init(nb);
    assert(myLanes.size() > 0);
    LaneVectorVector::const_iterator i2;
    LaneVector::const_iterator i;
    // build the E2 detectors
    for (i2 = myLanes.begin(); i2 != myLanes.end(); ++i2) {
        const LaneVector& lanes = *i2;
        for (i = lanes.begin(); i != lanes.end(); i++) {
            MSLane* lane = (*i);
            if (noVehicles(lane->getPermissions())) {
                // do not build detectors on green verges or sidewalks
                continue;
            }
            // Build the detectors and register them at the detector control
            if (myLaneDetectors.find(lane) == myLaneDetectors.end()) {
                MSE2Collector* det = nullptr;
                const std::string customID = getParameter(lane->getID());
                if (customID != "") {
                    det = dynamic_cast<MSE2Collector*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_LANE_AREA_DETECTOR).get(customID));
                    if (det == nullptr) {
                        WRITE_ERROR("Unknown laneAreaDetector '" + customID + "' given as custom detector for delay_based tlLogic '" + getID() + "', program '" + getProgramID() + ".");
                        continue;
                    }
                    det->setVisible(myShowDetectors);
                } else {
                    std::string id = "TLS" + myID + "_" + myProgramID + "_E2CollectorOn_" + lane->getID();
                    det = nb.createE2Detector(id, DU_TL_CONTROL, lane, INVALID_POSITION, lane->getLength(), myDetectionRange, 0, 0, 0, myVehicleTypes, myShowDetectors);
                    MSNet::getInstance()->getDetectorControl().add(SUMO_TAG_LANE_AREA_DETECTOR, det, myFile, myFreq);
                }
                myLaneDetectors[lane] = det;
            }
        }
    }
}



MSDelayBasedTrafficLightLogic::~MSDelayBasedTrafficLightLogic() { }

// ------------ Switching and setting current rows


SUMOTime
MSDelayBasedTrafficLightLogic::proposeProlongation(const SUMOTime actDuration, const SUMOTime maxDuration, bool& othersEmpty) {
#ifdef DEBUG_TIMELOSS_CONTROL
    std::cout << "\n" << SIMTIME << " MSDelayBasedTrafficLightLogic::proposeProlongation() for TLS '" << this->getID() << "' (current phase = " << myStep << ")" << std::endl;
#endif
    SUMOTime prolongation = 0;
    const std::string& state = getCurrentPhaseDef().getState();
    // iterate over green lanes, eventually increase the proposed prolongationTime to the estimated passing time for each lane.
    for (int i = 0; i < (int) state.size(); i++)  {
        // this lane index corresponds to a non-green time
        bool igreen = state[i] == LINKSTATE_TL_GREEN_MAJOR || state[i] == LINKSTATE_TL_GREEN_MINOR;
        const std::vector<MSLane*>& lanes = getLanesAt(i);
        for (LaneVector::const_iterator j = lanes.begin(); j != lanes.end(); j++) {
            LaneDetectorMap::iterator i = myLaneDetectors.find(*j);
            if (i == myLaneDetectors.end()) {
#ifdef DEBUG_TIMELOSS_CONTROL
                // no detector for this lane!? maybe noVehicles allowed
                std::cout << "no detector on lane '" << (*j)->getID() << std::endl;
#endif
                continue;
            }
            MSE2Collector* detector = static_cast<MSE2Collector* >(i->second);
            const std::vector<MSE2Collector::VehicleInfo*> vehInfos = detector->getCurrentVehicles();
#ifdef DEBUG_TIMELOSS_CONTROL
            int nrVehs = 0; // count vehicles on detector
#endif
            if (igreen) {
                // green phase
                for (std::vector<MSE2Collector::VehicleInfo*>::const_iterator ivp = vehInfos.begin(); ivp != vehInfos.end(); ++ivp) {
                    MSE2Collector::VehicleInfo* iv = *ivp;
                    if (iv->accumulatedTimeLoss > myTimeLossThreshold && iv->distToDetectorEnd > 0) {
                        const SUMOTime estimatedTimeToJunction = TIME2STEPS((iv->distToDetectorEnd) / (*j)->getSpeedLimit());
                        if (actDuration + estimatedTimeToJunction  <= maxDuration) {
                            // only prolong if vehicle has a chance to pass until max duration is reached
                            prolongation = MAX2(prolongation, estimatedTimeToJunction);
                        }
#ifdef DEBUG_TIMELOSS_CONTROL
                        nrVehs++;
#endif

#ifdef DEBUG_TIMELOSS_CONTROL
                        std::cout << "vehicle '" << iv->id << "' with accumulated timeloss: " << iv->accumulatedTimeLoss
                                  << "\nestimated passing time: " << estimatedTimeToJunction << std::endl;
                    } else {
                        std::string reason = iv->accumulatedTimeLoss <= myTimeLossThreshold ? " (time loss below threshold)" : " (front already left detector)";
                        std::cout << "disregarded: (vehicle '" << iv->id << "' with accumulated timeloss " << iv->accumulatedTimeLoss << ")" << reason << std::endl;
#endif
                    }
                }
            } else {
                // non-green phase
                if (vehInfos.size() > 0) {
                    // here is a car on a non-green approach
                    othersEmpty = false;
                    if (actDuration >= getCurrentPhaseDef().maxDuration) {
#ifdef DEBUG_TIMELOSS_CONTROL
                        std::cout << "Actual duration exceeds maxDuration and a vehicle is on concurrent approach: " << nrVehs << std::endl;
#endif
                        // don't prolong
                        return 0;
                    }
                    break;
                }
#ifdef DEBUG_TIMELOSS_CONTROL
                std::cout << "Number of current vehicles on detector: " << nrVehs << std::endl;
#endif
            }
        }
    }
#ifdef DEBUG_TIMELOSS_CONTROL
    std::cout << "Proposed prolongation (maximal estimated passing time): " << prolongation << std::endl; // debug
#endif
    return prolongation;
}


SUMOTime
MSDelayBasedTrafficLightLogic::trySwitch() {
    /* check if the actual phase should be prolonged */
    const MSPhaseDefinition& currentPhase = getCurrentPhaseDef();
    // time since last switch
    const SUMOTime actDuration = MSNet::getInstance()->getCurrentTimeStep() - currentPhase.myLastSwitch;

#ifdef DEBUG_TIMELOSS_CONTROL
    std::cout << "last switch = " << currentPhase.myLastSwitch
              << "\nactDuration = " << actDuration
              << "\nmaxDuration = " << currentPhase.maxDuration
              << std::endl;
#endif

    // flag whether to prolong or not
    if (currentPhase.isGreenPhase() && !MSGlobals::gUseMesoSim) {
        bool othersEmpty = true; // whether no vehicles are present on concurrent approaches
        SUMOTime proposedProlongation = proposeProlongation(actDuration, currentPhase.maxDuration, othersEmpty);

#ifdef DEBUG_TIMELOSS_CONTROL
        std::cout << "othersEmpty = " << othersEmpty
                  << std::endl;
#endif

        // keep this phase a little longer?
        bool prolong =  othersEmpty || actDuration < currentPhase.maxDuration;
        // assure minimal duration
        proposedProlongation = MAX3(SUMOTime(0), proposedProlongation, currentPhase.minDuration - actDuration);
        if (othersEmpty) {
            // prolong by one second if no vehicles on other approaches
            proposedProlongation = MAX2(proposedProlongation, TIME2STEPS(1.));
        } else {
            // vehicles are present on other approaches -> prolong no further than the max green time
            proposedProlongation = MIN2(proposedProlongation, MAX2(SUMOTime(0), currentPhase.maxDuration - actDuration));
        }

#ifdef DEBUG_TIMELOSS_CONTROL
        std::cout << "Proposed prolongation = " << proposedProlongation << std::endl;
#endif

        prolong = proposedProlongation > 0;
        if (prolong) {
            // check again after the prolonged period (must be positive...)
            // XXX: Can it be harmful not to return a duration of integer seconds?
            return proposedProlongation;
        }
    }
    // Don't prolong... switch to the next phase
    myStep++;
    assert(myStep <= (int)myPhases.size());
    if (myStep == (int)myPhases.size()) {
        myStep = 0;
    }
    MSPhaseDefinition* newPhase = myPhases[myStep];
    //stores the time the phase started
    newPhase->myLastSwitch = MSNet::getInstance()->getCurrentTimeStep();
    // set the next event
    return newPhase->minDuration;
}

void
MSDelayBasedTrafficLightLogic::setShowDetectors(bool show) {
    myShowDetectors = show;
    for (auto& item : myLaneDetectors) {
        item.second->setVisible(myShowDetectors);
    }
}



/****************************************************************************/
