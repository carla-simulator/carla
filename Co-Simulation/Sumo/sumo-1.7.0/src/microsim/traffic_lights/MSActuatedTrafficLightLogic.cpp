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
/// @file    MSActuatedTrafficLightLogic.cpp
/// @author  Daniel Krajzewicz
/// @author  Julia Ringel
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    Sept 2002
///
// An actuated (adaptive) traffic light logic
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utility>
#include <vector>
#include <bitset>
#include <microsim/MSEventControl.h>
#include <microsim/output/MSDetectorControl.h>
#include <microsim/output/MSInductLoop.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include "MSTrafficLightLogic.h"
#include "MSActuatedTrafficLightLogic.h"
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <netload/NLDetectorBuilder.h>
#include <utils/common/StringUtils.h>

//#define DEBUG_DETECTORS
//#define DEBUG_PHASE_SELECTION
#define DEBUG_COND (getID()=="C")

// ===========================================================================
// parameter defaults definitions
// ===========================================================================
#define DEFAULT_MAX_GAP "3.0"
#define DEFAULT_PASSING_TIME "1.9"
#define DEFAULT_DETECTOR_GAP "2.0"
#define DEFAULT_INACTIVE_THRESHOLD "180"
#define DEFAULT_CURRENT_PRIORITY 10

#define DEFAULT_LENGTH_WITH_GAP 7.5


// ===========================================================================
// method definitions
// ===========================================================================
MSActuatedTrafficLightLogic::MSActuatedTrafficLightLogic(MSTLLogicControl& tlcontrol,
        const std::string& id, const std::string& programID,
        const Phases& phases,
        int step, SUMOTime delay,
        const std::map<std::string, std::string>& parameter,
        const std::string& basePath) :
    MSSimpleTrafficLightLogic(tlcontrol, id, programID, TrafficLightType::ACTUATED, phases, step, delay, parameter),
    myLastTrySwitchTime(0) {
    myMaxGap = StringUtils::toDouble(getParameter("max-gap", DEFAULT_MAX_GAP));
    myPassingTime = StringUtils::toDouble(getParameter("passing-time", DEFAULT_PASSING_TIME));
    myDetectorGap = StringUtils::toDouble(getParameter("detector-gap", DEFAULT_DETECTOR_GAP));
    myInactiveThreshold = string2time(getParameter("inactive-threshold", DEFAULT_INACTIVE_THRESHOLD));
    myShowDetectors = StringUtils::toBool(getParameter("show-detectors", toString(OptionsCont::getOptions().getBool("tls.actuated.show-detectors"))));
    myFile = FileHelpers::checkForRelativity(getParameter("file", "NUL"), basePath);
    myFreq = TIME2STEPS(StringUtils::toDouble(getParameter("freq", "300")));
    myVehicleTypes = getParameter("vTypes", "");
}

MSActuatedTrafficLightLogic::~MSActuatedTrafficLightLogic() { }

void
MSActuatedTrafficLightLogic::init(NLDetectorBuilder& nb) {
    MSTrafficLightLogic::init(nb);
    if (myLanes.size() == 0) {
        // must be an older network
        WRITE_WARNING("Traffic light '" + getID() + "' does not control any links");
    }
    bool warn = true; // warn only once
    const int numLinks = (int)myLinks.size();

    // Detector position should be computed based on road speed. If the position
    // is quite far away and the minDur is short this may cause the following
    // problems:
    //
    // 1)  high flow failure:
    // In a standing queue, no vehicle touches the detector.
    // By the time the queue advances, the detector gap has been exceeded and the phase terminates prematurely
    //
    // 2) low flow failure
    // The standing queue is fully between stop line and detector and there are no further vehicles.
    // The minDur is too short to let all vehicles pass
    //
    // Problem 2) is not so critical because there is less potential for
    // jamming in a low-flow situation. In contrast, problem 1) should be
    // avoided as it has big jamming potential. We compute an upper bound for the
    // detector distance to avoid it


    // change values for setting the loops and lanestate-detectors, here
    //SUMOTime inductLoopInterval = 1; //
    // build the induct loops
    std::map<const MSLane*, MSInductLoop*> laneInductLoopMap;
    std::map<MSInductLoop*, const MSLane*> inductLoopLaneMap; // in case loops are placed further upstream
    double maxDetectorGap = 0;
    for (LaneVector& lanes : myLanes) {
        for (MSLane* lane : lanes) {
            if (noVehicles(lane->getPermissions())) {
                // do not build detectors on green verges or sidewalks
                continue;
            }
            if (laneInductLoopMap.find(lane) != laneInductLoopMap.end()) {
                // only build one detector per lane
                continue;
            }
            const SUMOTime minDur = getMinimumMinDuration(lane);
            if (minDur == std::numeric_limits<SUMOTime>::max()) {
                // only build detector if this lane is relevant for an actuated phase
                continue;
            }
            const std::string customID = getParameter(lane->getID());
            double length = lane->getLength();
            double ilpos;
            double inductLoopPosition;
            MSInductLoop* loop = nullptr;
            if (customID == "") {
                double speed = lane->getSpeedLimit();
                inductLoopPosition = MIN2(
                                         myDetectorGap * speed,
                                         (STEPS2TIME(minDur) / myPassingTime + 0.5) * DEFAULT_LENGTH_WITH_GAP);

                // check whether the lane is long enough
                ilpos = length - inductLoopPosition;
                MSLane* placementLane = lane;
                while (ilpos < 0 && placementLane->getIncomingLanes().size() == 1) {
                    placementLane = placementLane->getLogicalPredecessorLane();
                    ilpos += placementLane->getLength();
                }
                if (ilpos < 0) {
                    ilpos = 0;
                }
                // Build the induct loop and set it into the container
                std::string id = "TLS" + myID + "_" + myProgramID + "_InductLoopOn_" + lane->getID();
                loop = static_cast<MSInductLoop*>(nb.createInductLoop(id, placementLane, ilpos, myVehicleTypes, myShowDetectors));
                MSNet::getInstance()->getDetectorControl().add(SUMO_TAG_INDUCTION_LOOP, loop, myFile, myFreq);
            } else {
                loop = dynamic_cast<MSInductLoop*>(MSNet::getInstance()->getDetectorControl().getTypedDetectors(SUMO_TAG_INDUCTION_LOOP).get(customID));
                if (loop == nullptr) {
                    WRITE_ERROR("Unknown inductionLoop '" + customID + "' given as custom detector for actuated tlLogic '" + getID() + "', program '" + getProgramID() + ".");
                    continue;
                }
                ilpos = loop->getPosition();
                inductLoopPosition = length - ilpos;
            }
            laneInductLoopMap[lane] = loop;
            inductLoopLaneMap[loop] = lane;
            myInductLoops.push_back(InductLoopInfo(loop, (int)myPhases.size()));
            maxDetectorGap = MAX2(maxDetectorGap, length - ilpos);

            if (warn && floor(floor(inductLoopPosition / DEFAULT_LENGTH_WITH_GAP) * myPassingTime) > STEPS2TIME(minDur)) {
                // warn if the minGap is insufficient to clear vehicles between stop line and detector
                WRITE_WARNING("At actuated tlLogic '" + getID() + "', minDur " + time2string(minDur) + " is too short for a detector gap of " + toString(inductLoopPosition) + "m.");
                warn = false;
            }
        }
    }
    // assign loops to phase index (myInductLoopsForPhase)
    //  check1: loops may not be used for a phase if there are other connections from the same lane that may not drive in that phase
    //            greenMinor is ambiguous as vehicles may not be able to drive
    //            Under the following condition we allow actuation from minor link:
    //              check1a : the minor link is minor in all phases
    //              check1b : there is another major link from the same lane in the current phase
    //            (Under these conditions we assume that the minor link is unimportant and traffic is mostly for the major link)
    //
    //              check1c: when the lane has only one edge, we treat greenMinor as green as there would be no actuation otherwise
    //
    //  check2: if there are two loops on subsequent lanes (joined tls) and the second one has a red link, the first loop may not be used

    // also assign loops to link index for validation:
    // check if all links from actuated phases (minDur != maxDur) have an inductionloop in at least one phase
    const SVCPermissions motorized = ~(SVC_PEDESTRIAN | SVC_BICYCLE);
    std::map<int, std::set<MSInductLoop*> > linkToLoops;
    std::set<int> actuatedLinks;

    std::vector<bool> neverMajor(numLinks, true);
    for (const MSPhaseDefinition* phase : myPhases) {
        const std::string& state = phase->getState();
        for (int i = 0; i < numLinks; i++)  {
            if (state[i] == LINKSTATE_TL_GREEN_MAJOR) {
                neverMajor[i] = false;
            }
        }
    }
    std::vector<bool> oneLane(numLinks, false);
    for (int i = 0; i < numLinks; i++)  {
        for (MSLane* lane : getLanesAt(i)) {
            // only count motorized vehicle lanes
            int numMotorized = 0;
            for (MSLane* l : lane->getEdge().getLanes()) {
                if ((l->getPermissions() & motorized) != 0) {
                    numMotorized++;
                }
            }
            if (numMotorized == 1) {
                oneLane[i] = true;
                break;
            }
        }
    }


    for (const MSPhaseDefinition* phase : myPhases) {
        const int phaseIndex = (int)myInductLoopsForPhase.size();
        std::set<MSInductLoop*> loops;
        if (phase->minDuration != phase->maxDuration) {
            // actuated phase
            const std::string& state = phase->getState();
            // collect indices of all green links for the phase
            std::set<int> greenLinks;
            // collect green links for each induction loops (in this phase)
            std::map<MSInductLoop*, std::set<int> > loopLinks;

            for (int i = 0; i < numLinks; i++)  {
                if (state[i] == LINKSTATE_TL_GREEN_MAJOR
                        || (state[i] == LINKSTATE_TL_GREEN_MINOR
                            && ((neverMajor[i]  // check1a
                                 && hasMajor(state, getLanesAt(i))) // check1b
                                || oneLane[i])) // check1c
                   ) {
                    greenLinks.insert(i);
                    actuatedLinks.insert(i);
                }
#ifdef DEBUG_DETECTORS
                //if (DEBUG_COND) {
                //    std::cout << " phase=" << phaseIndex << " i=" << i << " state=" << state[i] << " green=" << greenLinks.count(i) << " oneLane=" << oneLane[i]
                //        << " loopLanes=";
                //    for (MSLane* lane: getLanesAt(i)) {
                //        if (laneInductLoopMap.count(lane) != 0) {
                //            std::cout << lane->getID() << " ";
                //        }
                //    }
                //    std::cout << "\n";
                //}
#endif
                for (MSLane* lane : getLanesAt(i)) {
                    if (laneInductLoopMap.count(lane) != 0) {
                        loopLinks[laneInductLoopMap[lane]].insert(i);
                    }
                }
            }
            for (auto& item : loopLinks) {
                MSInductLoop* loop = item.first;
                const MSLane* loopLane = inductLoopLaneMap[loop];
                bool usable = true;
                // check1
                for (int j : item.second) {
                    if (greenLinks.count(j) == 0) {
                        usable = false;
#ifdef DEBUG_DETECTORS
                        if (DEBUG_COND) {
                            std::cout << " phase=" << phaseIndex << " check1: loopLane=" << loopLane->getID() << " notGreen=" << j << " oneLane[j]=" << oneLane[j] << "\n";
                        }
#endif
                        break;
                    }
                }
                // check2
                if (usable) {
                    for (MSLink* link : loopLane->getLinkCont()) {
                        const MSLane* next = link->getLane();
                        if (laneInductLoopMap.count(next) != 0) {
                            MSInductLoop* nextLoop = laneInductLoopMap[next];
                            for (int j : loopLinks[nextLoop]) {
                                if (greenLinks.count(j) == 0) {
                                    usable = false;
#ifdef DEBUG_DETECTORS
                                    if (DEBUG_COND) std::cout << " phase=" << phaseIndex << " check2: loopLane=" << loopLane->getID()
                                                                  << " nextLane=" << next->getID() << " nextLink=" << j << " nextState=" << state[j] << "\n";
#endif
                                    break;
                                }
                            }
                        }
                    }
                }

                if (usable) {
                    loops.insert(item.first);
#ifdef DEBUG_DETECTORS
                    //if (DEBUG_COND) std::cout << " phase=" << phaseIndex << " usableLoops=" << item.first->getID() << " links=" << joinToString(item.second, " ") << "\n";
#endif
                    for (int j : item.second) {
                        linkToLoops[j].insert(item.first);
                    }
                }
            }
            if (loops.size() == 0) {
                WRITE_WARNINGF("At actuated tlLogic '%', actuated phase % has no controlling detector", getID(), toString(phaseIndex));
            }
        }
#ifdef DEBUG_DETECTORS
        if (DEBUG_COND) {
            std::cout << " phase=" << phaseIndex << " loops=" << joinNamedToString(loops, " ") << "\n";
        }
        //if (DEBUG_COND) {
        //    std::cout << " linkToLoops:\n";
        //    for (auto item : linkToLoops) {
        //        std::cout << "   link=" << item.first << " loops=" << joinNamedToString(item.second, " ") << "\n";
        //    }
        //}
#endif
        std::vector<InductLoopInfo*> loopInfos;
        myInductLoopsForPhase.push_back(loopInfos);
        for (MSInductLoop* loop : loops) {
            for (InductLoopInfo& loopInfo : myInductLoops) {
                if (loopInfo.loop == loop) {
                    myInductLoopsForPhase.back().push_back(&loopInfo);
                    loopInfo.servedPhase[phaseIndex] = true;
                }
            }
        }
    }
#ifdef DEBUG_DETECTORS
    //if (DEBUG_COND) {
    //    std::cout << "final linkToLoops:\n";
    //    for (auto item : linkToLoops) {
    //        std::cout << "   link=" << item.first << " loops=" << joinNamedToString(item.second, " ") << "\n";
    //    }
    //}
#endif
    for (int i : actuatedLinks) {
        if (linkToLoops[i].size() == 0 && myLinks[i].size() > 0
                && (myLinks[i].front()->getLaneBefore()->getPermissions() & motorized) != 0) {
            WRITE_WARNINGF("At actuated tlLogic '%', linkIndex % has no controlling detector", getID(), toString(i));
        }
    }
    // parse maximum green times for each link (optional)
    for (const auto& kv : getParametersMap()) {
        if (StringUtils::startsWith(kv.first, "linkMaxDur:")) {
            int link = StringUtils::toInt(kv.first.substr(11));
            if (link < 0 || link >= myNumLinks) {
                WRITE_ERROR("Invalid link '" + kv.first.substr(11) + "' given as linkMaxDur parameter for actuated tlLogic '" + getID() + "', program '" + getProgramID() + ".");
                continue;
            }
            if (myLinkMaxGreenTimes.empty()) {
                myLinkMaxGreenTimes = std::vector<SUMOTime>(myNumLinks, std::numeric_limits<SUMOTime>::max());
            }
            myLinkMaxGreenTimes[link] = string2time(kv.second);
        } else if (StringUtils::startsWith(kv.first, "linkMinDur:")) {
            int link = StringUtils::toInt(kv.first.substr(11));
            if (link < 0 || link >= myNumLinks) {
                WRITE_ERROR("Invalid link '" + kv.first.substr(11) + "' given as linkMinDur parameter for actuated tlLogic '" + getID() + "', program '" + getProgramID() + ".");
                continue;
            }
            if (myLinkMinGreenTimes.empty()) {
                myLinkMinGreenTimes = std::vector<SUMOTime>(myNumLinks, 0);
            }
            myLinkMinGreenTimes[link] = string2time(kv.second);
        }
    }
    if (myLinkMaxGreenTimes.size() > 0 || myLinkMinGreenTimes.size() > 0) {
        myLinkGreenTimes = std::vector<SUMOTime>(myNumLinks, 0);
    }
    //std::cout << SIMTIME << " linkMaxGreenTimes=" << toString(myLinkMaxGreenTimes) << "\n";
}


SUMOTime
MSActuatedTrafficLightLogic::getMinimumMinDuration(MSLane* lane) const {
    SUMOTime result = std::numeric_limits<SUMOTime>::max();
    for (const MSPhaseDefinition* phase : myPhases) {
        const std::string& state = phase->getState();
        for (int i = 0; i < (int)state.size(); i++)  {
            if (state[i] == LINKSTATE_TL_GREEN_MAJOR || state[i] == LINKSTATE_TL_GREEN_MINOR) {
                for (MSLane* cand : getLanesAt(i)) {
                    if (lane == cand) {
                        if (phase->minDuration != phase->maxDuration) {
                            result = MIN2(result, phase->minDuration);
                        }
                    }
                }
            }
        }
    }
    return result;
}

bool
MSActuatedTrafficLightLogic::hasMajor(const std::string& state, const LaneVector& lanes) const {
    for (int i = 0; i < (int)state.size(); i++) {
        if (state[i] == LINKSTATE_TL_GREEN_MAJOR) {
            for (MSLane* cand : getLanesAt(i)) {
                for (MSLane* lane : lanes) {
                    if (lane == cand) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


// ------------ Switching and setting current rows
void
MSActuatedTrafficLightLogic::activateProgram() {
    MSTrafficLightLogic::activateProgram();
    for (InductLoopInfo& loopInfo : myInductLoops) {
        loopInfo.loop->setVisible(true);
    }
}


void
MSActuatedTrafficLightLogic::deactivateProgram() {
    MSTrafficLightLogic::deactivateProgram();
    for (InductLoopInfo& loopInfo : myInductLoops) {
        loopInfo.loop->setVisible(false);
    }
}

SUMOTime
MSActuatedTrafficLightLogic::trySwitch() {
    // checks if the actual phase should be continued
    // @note any vehicles which arrived during the previous phases which are now waiting between the detector and the stop line are not
    // considere here. RiLSA recommends to set minDuration in a way that lets all vehicles pass the detector
    SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    if (myLinkGreenTimes.size() > 0) {
        // constraints exist, record green time durations for each link
        const std::string& state = getCurrentPhaseDef().getState();
        SUMOTime lastDuration = now - myLastTrySwitchTime;
        for (int i = 0; i < myNumLinks; i++) {
            if (state[i] == 'G' || state[i] == 'g') {
                myLinkGreenTimes[i] += lastDuration;
            } else {
                myLinkGreenTimes[i] = 0;
            }
        }
        //std::cout << SIMTIME << " greenTimes=" << toString(myLinkGreenTimes) << "\n";
    }
    myLastTrySwitchTime = now;
    const double detectionGap = gapControl();
    const bool multiTarget = myPhases[myStep]->nextPhases.size() > 1 && myPhases[myStep]->nextPhases.front() >= 0;
#ifdef DEBUG_PHASE_SELECTION
    if (DEBUG_COND) {
        std::cout << SIMTIME << " p=" << myStep << " trySwitch dGap=" << detectionGap << " multi=" << multiTarget << "\n";
    }
#endif
    if (detectionGap < std::numeric_limits<double>::max() && !multiTarget) {
        return duration(detectionGap);
    }
    // decide the next phase
    const int origStep = myStep;
    int nextStep = myStep;
    SUMOTime actDuration = now - myPhases[myStep]->myLastSwitch;
    if (multiTarget) {
        nextStep = decideNextPhase();
    } else {
        if (myPhases[myStep]->nextPhases.size() == 1 && myPhases[myStep]->nextPhases.front() >= 0) {
            nextStep = myPhases[myStep]->nextPhases.front();
        } else {
            nextStep++;
        }
    }
    if (nextStep == (int)myPhases.size()) {
        nextStep = 0;
    }
    SUMOTime linkMinDur = getLinkMinDuration(getTarget(nextStep));
    if (linkMinDur > 0) {
        // for multiTarget, the current phase must be extended but if another
        // targer is chosen, earlier switching than linkMinDur is possible
        return multiTarget ? TIME2STEPS(1) : linkMinDur;
    }
    myStep = nextStep;
    assert(myStep <= (int)myPhases.size());
    assert(myStep >= 0);
    //stores the time the phase started
    if (myStep != origStep) {
        myPhases[myStep]->myLastSwitch = now;
        actDuration = 0;
    }
    // activate coloring
    if ((myShowDetectors || multiTarget) && getCurrentPhaseDef().isGreenPhase()) {
        for (InductLoopInfo* loopInfo : myInductLoopsForPhase[myStep]) {
            //std::cout << SIMTIME << " p=" << myStep << " loopinfo=" << loopInfo->loop->getID() << " set lastGreen=" << STEPS2TIME(now) << "\n";
            loopInfo->loop->setSpecialColor(&RGBColor::GREEN);
            loopInfo->lastGreenTime = now;
        }
    }
    // set the next event
    return MAX2(TIME2STEPS(1), getCurrentPhaseDef().minDuration - actDuration);
}


// ------------ "actuated" algorithm methods
SUMOTime
MSActuatedTrafficLightLogic::duration(const double detectionGap) const {
    assert(getCurrentPhaseDef().isGreenPhase());
    assert((int)myPhases.size() > myStep);
    const SUMOTime actDuration = MSNet::getInstance()->getCurrentTimeStep() - myPhases[myStep]->myLastSwitch;
    // ensure that minimum duration is kept
    SUMOTime newDuration = getCurrentPhaseDef().minDuration - actDuration;
    // try to let the last detected vehicle pass the intersection (duration must be positive)
    newDuration = MAX3(newDuration, TIME2STEPS(myDetectorGap - detectionGap), SUMOTime(1));
    // cut the decimal places to ensure that phases always have integer duration
    if (newDuration % 1000 != 0) {
        const SUMOTime totalDur = newDuration + actDuration;
        newDuration = (totalDur / 1000 + 1) * 1000 - actDuration;
    }
    // ensure that the maximum duration is not exceeded
    newDuration = MIN2(newDuration, getCurrentPhaseDef().maxDuration - actDuration);
    return newDuration;
}


double
MSActuatedTrafficLightLogic::gapControl() {
    //intergreen times should not be lengthend
    assert((int)myPhases.size() > myStep);
    double result = std::numeric_limits<double>::max();
    if (MSGlobals::gUseMesoSim) {
        return result;
    }
    // switch off active colors
    if (myShowDetectors) {
        for (InductLoopInfo& loopInfo : myInductLoops) {
            if (loopInfo.lastGreenTime < loopInfo.loop->getLastDetectionTime()) {
                loopInfo.loop->setSpecialColor(&RGBColor::RED);
            } else {
                loopInfo.loop->setSpecialColor(nullptr);
            }
        }
    }
    if (!getCurrentPhaseDef().isGreenPhase()) {
        return result; // end current phase
    }

    // Checks, if the maxDuration is kept. No phase should last longer than maxDuration.
    SUMOTime actDuration = MSNet::getInstance()->getCurrentTimeStep() - myPhases[myStep]->myLastSwitch;
    if (actDuration >= getCurrentPhaseDef().maxDuration || maxLinkDurationReached()) {
        return result; // end current phase
    }

    // now the gapcontrol starts
    for (InductLoopInfo* loopInfo : myInductLoopsForPhase[myStep]) {
        MSInductLoop* loop = loopInfo->loop;
        loop->setSpecialColor(&RGBColor::GREEN);
        const double actualGap = loop->getTimeSinceLastDetection();
        if (actualGap < myMaxGap) {
            result = MIN2(result, actualGap);
        }
    }
    return result;
}


int
MSActuatedTrafficLightLogic::decideNextPhase() {
    const auto& cands = myPhases[myStep]->nextPhases;
    // decide by priority
    // first target is the default when thre is no traffic
    // @note: the keep the current phase, even when there is no traffic, it must be added to 'next' explicitly
    int result = cands.front();
    int maxPrio = 0;
    SUMOTime actDuration = MSNet::getInstance()->getCurrentTimeStep() - myPhases[myStep]->myLastSwitch;
    const bool canExtend = actDuration < getCurrentPhaseDef().maxDuration && !maxLinkDurationReached();
    if (canExtend) {
        // consider keeping the current phase until maxDur is reached
        // (only when there is still traffic in that phase)
        int currentPrio = getPhasePriority(myStep);
#ifdef DEBUG_PHASE_SELECTION
        std::cout << SIMTIME << " p=" << myStep << " loops=" << myInductLoopsForPhase[myStep].size() << " currentPrio=" << currentPrio << "\n";
#endif
        if (currentPrio > maxPrio) {
            result = myStep;
            maxPrio = currentPrio;
        }
    }
    for (int step : cands) {
        int target = getTarget(step);
        int prio = getPhasePriority(target);
#ifdef DEBUG_PHASE_SELECTION
        if (DEBUG_COND) {
            std::cout << SIMTIME << " p=" << myStep << " step=" << step << " target=" << target << " loops=" << myInductLoopsForPhase[target].size() << " prio=" << prio << "\n";
        }
#endif
        if (prio > maxPrio && canExtendLinkGreen(target)) {
            maxPrio = prio;
            result = step;
        }
    }
    // prevent starvation in phases that are not direct targets
    for (const InductLoopInfo& loopInfo : myInductLoops) {
        int prio = getDetectorPriority(loopInfo);
        if (prio > maxPrio) {
            result = cands.front();
            if (result == myStep) {
                WRITE_WARNING("At actuated tlLogic '" + getID()
                              + "', starvation at e1Detector '" + loopInfo.loop->getID()
                              + "' which cannot be reached from the default phase " + toString(myStep) + ".");
            }
            // use default phase to reach other phases
#ifdef DEBUG_PHASE_SELECTION
            if (DEBUG_COND) {
                std::cout << SIMTIME << " p=" << myStep << " loop=" << loopInfo.loop->getID() << " prio=" << prio << " next=" << result << "\n";
            }
#endif
            break;
        }
    }
    return result;
}


int
MSActuatedTrafficLightLogic::getTarget(int step) {
    int origStep = step;
    // if step is a transition, find the upcoming green phase
    while (!myPhases[step]->isGreenPhase()) {
        if (myPhases[step]->nextPhases.size() > 0 && myPhases[step]->nextPhases.front() >= 0) {
            if (myPhases[step]->nextPhases.size() > 1) {
                WRITE_WARNING("At actuated tlLogic '" + getID() + "', transition phase " + toString(step) + " should not have multiple next phases");
            }
            step = myPhases[step]->nextPhases.front();
        } else {
            step = (step + 1) % myPhases.size();
        }
        if (step == origStep) {
            WRITE_WARNING("At actuated tlLogic '" + getID() + "', infinite transition loop from phase " + toString(origStep));
            return 0;
        }
    }
    return step;
}

int
MSActuatedTrafficLightLogic::getDetectorPriority(const InductLoopInfo& loopInfo) const {
    MSInductLoop* loop = loopInfo.loop;
    const double actualGap = loop->getTimeSinceLastDetection();
    if (actualGap < myMaxGap || loopInfo.lastGreenTime < loop->getLastDetectionTime()) {
        SUMOTime inactiveTime = MSNet::getInstance()->getCurrentTimeStep() - loopInfo.lastGreenTime;
        // @note. Inactive time could also be tracked regardless of current activity (to increase robustness in case of detection failure
        if (inactiveTime > myInactiveThreshold) {
#ifdef DEBUG_PHASE_SELECTION
            if (DEBUG_COND) {
                std::cout << "    loop=" << loop->getID() << " gap=" << loop->getTimeSinceLastDetection() << " lastGreen=" << STEPS2TIME(loopInfo.lastGreenTime)
                          << " lastDetection=" << STEPS2TIME(loop->getLastDetectionTime()) << " inactive=" << STEPS2TIME(inactiveTime) << "\n";
            }
#endif
            return (int)STEPS2TIME(inactiveTime);
        } else {
            // give bonus to detectors that are currently served (if that phase can stil be extended)
            if (loopInfo.servedPhase[myStep]) {
                SUMOTime actDuration = MSNet::getInstance()->getCurrentTimeStep() - myPhases[myStep]->myLastSwitch;
                const bool canExtend = actDuration < getCurrentPhaseDef().maxDuration;
                if (canExtend) {
                    return DEFAULT_CURRENT_PRIORITY;
                } else {
                    return 0;
                }
            }
            return 1;
        }
    }
    return 0;
}

int
MSActuatedTrafficLightLogic::getPhasePriority(int step) const {
    int result = 0;
    for (const InductLoopInfo* loopInfo : myInductLoopsForPhase[step]) {
        result += getDetectorPriority(*loopInfo);
    }
    return result;
}


void
MSActuatedTrafficLightLogic::setShowDetectors(bool show) {
    myShowDetectors = show;
    for (InductLoopInfo& loopInfo : myInductLoops) {
        loopInfo.loop->setVisible(myShowDetectors);
    }
}


bool
MSActuatedTrafficLightLogic::maxLinkDurationReached() {
    if (myLinkMaxGreenTimes.empty()) {
        return false;
    }
    for (int i = 0; i < myNumLinks; i++) {
        if (myLinkGreenTimes[i] >= myLinkMaxGreenTimes[i]) {
            //std::cout << SIMTIME << " maxLinkDurationReached i=" << i << "\n";
            return true;
        }
    }
    return false;
}

bool
MSActuatedTrafficLightLogic::canExtendLinkGreen(int target) {
    if (myLinkMaxGreenTimes.empty()) {
        return true;
    }
    const std::string& targetState = myPhases[target]->getState();
    for (int i = 0; i < myNumLinks; i++) {
        if (myLinkGreenTimes[i] >= myLinkMaxGreenTimes[i] && (
                    targetState[i] == 'G' || targetState[i] == 'g')) {
            //std::cout << SIMTIME << " cannotExtendLinkGreen target=" << target << " i=" << i << "\n";
            return false;
        }
    }
    return true;
}

SUMOTime
MSActuatedTrafficLightLogic::getLinkMinDuration(int target) const {
    SUMOTime result = 0;
    if (target != myStep && myLinkMinGreenTimes.size() > 0) {
        const std::string& state = myPhases[myStep]->getState();
        const std::string& targetState = myPhases[target]->getState();
        for (int i = 0; i < myNumLinks; i++) {
            if (myLinkGreenTimes[i] < myLinkMinGreenTimes[i]
                    && (state[i] == 'G' || state[i] == 'g')
                    && !(targetState[i] == 'G' || targetState[i] == 'g')) {
                result = MAX2(result, myLinkMinGreenTimes[i] - myLinkGreenTimes[i]);
                //std::cout << SIMTIME << " getLinkMinDuration myStep=" << myStep << " target=" << target << " i=" << i
                //    << " greenTime=" << STEPS2TIME(myLinkGreenTimes[i]) << " min=" << STEPS2TIME(myLinkMinGreenTimes[i]) << " result=" << STEPS2TIME(result) << "\n";
            }
        }
    }
    return result;
}


void
MSActuatedTrafficLightLogic::setParameter(const std::string& key, const std::string& value) {
    // some pre-defined parameters can be updated at runtime
    if (key == "detector-gap" || key == "passing-time" || key == "file" || key == "freq" || key == "vTypes"
            || StringUtils::startsWith(key, "linkMaxDur")
            || StringUtils::startsWith(key, "linkMinDur")) {
        throw InvalidArgument(key + " cannot be changed dynamically for actuated traffic light '" + getID() + "'");
    } else if (key == "max-gap") {
        myMaxGap = StringUtils::toDouble(value);
    } else if (key == "show-detectors") {
        myShowDetectors = StringUtils::toBool(value);
    } else if (key == "inactive-threshold") {
        myInactiveThreshold = string2time(value);
    }
    Parameterised::setParameter(key, value);
}


/****************************************************************************/
