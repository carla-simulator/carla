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
/// @file    MSTrafficLightLogic.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// The parent class for traffic light logics
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <string>
#include <iostream>
#include <map>
#include <microsim/MSLink.h>
#include <microsim/MSLane.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSJunctionLogic.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSGlobals.h>
#include "MSTLLogicControl.h"
#include "MSTrafficLightLogic.h"


// ===========================================================================
// static value definitions
// ===========================================================================
const MSTrafficLightLogic::LaneVector MSTrafficLightLogic::myEmptyLaneVector;


// ===========================================================================
// member method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * member method definitions
 * ----------------------------------------------------------------------- */
MSTrafficLightLogic::SwitchCommand::SwitchCommand(MSTLLogicControl& tlcontrol,
        MSTrafficLightLogic* tlLogic, SUMOTime nextSwitch)
    : myTLControl(tlcontrol), myTLLogic(tlLogic),
      myAssumedNextSwitch(nextSwitch), myAmValid(true) {}


MSTrafficLightLogic::SwitchCommand::~SwitchCommand() {}



SUMOTime
MSTrafficLightLogic::SwitchCommand::execute(SUMOTime t) {
    // check whether this command has been descheduled
    if (!myAmValid) {
        return 0;
    }
    int step1 = myTLLogic->getCurrentPhaseIndex();
    SUMOTime next = myTLLogic->trySwitch();
    int step2 = myTLLogic->getCurrentPhaseIndex();
    if (step1 != step2) {
        if (myTLLogic->isActive()) {
            // execute any action connected to this tls
            const MSTLLogicControl::TLSLogicVariants& vars = myTLControl.get(myTLLogic->getID());
            // set link priorities
            myTLLogic->setTrafficLightSignals(t);
            // execute switch actions
            vars.executeOnSwitchActions();
        }
    }
    myAssumedNextSwitch += next;
    return next;
}


void
MSTrafficLightLogic::SwitchCommand::deschedule(MSTrafficLightLogic* tlLogic) {
    if (tlLogic == myTLLogic) {
        myAmValid = false;
        myAssumedNextSwitch = -1;
    }
}

SUMOTime
MSTrafficLightLogic::SwitchCommand::shiftTime(SUMOTime currentTime, SUMOTime execTime, SUMOTime newTime) {
    if (myTLLogic->getDefaultCycleTime() == DELTA_T) {
        // MSRailSignal
        return newTime;
    } else {
        UNUSED_PARAMETER(currentTime);
        UNUSED_PARAMETER(execTime);
        // XXX changeStepAndDuration (computed as in NLJunctionControlBuilder::closeTrafficLightLogic
        return newTime;
    }
}

/* -------------------------------------------------------------------------
 * member method definitions
 * ----------------------------------------------------------------------- */
MSTrafficLightLogic::MSTrafficLightLogic(MSTLLogicControl& tlcontrol, const std::string& id,
        const std::string& programID, const TrafficLightType logicType, const SUMOTime delay,
        const std::map<std::string, std::string>& parameters) :
    Named(id), Parameterised(parameters),
    myProgramID(programID),
    myLogicType(logicType),
    myCurrentDurationIncrement(-1),
    myDefaultCycleTime(0),
    myAmActive(true) {
    mySwitchCommand = new SwitchCommand(tlcontrol, this, delay);
    MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(mySwitchCommand, delay);
}


void
MSTrafficLightLogic::init(NLDetectorBuilder&) {
    const Phases& phases = getPhases();
    if (phases.size() > 0 && (MSGlobals::gMesoTLSPenalty > 0 || MSGlobals::gMesoTLSFlowPenalty > 0)) {
        initMesoTLSPenalties();
    }
    if (phases.size() > 1) {
        bool haveWarnedAboutUnusedStates = false;
        std::vector<bool> foundGreen(phases.front()->getState().size(), false);
        for (int i = 0; i < (int)phases.size(); ++i) {
            // warn about unused states
            std::vector<int> nextPhases;
            nextPhases.push_back((i + 1) % phases.size());
            bool iNextDefault = true;
            if (phases[i]->nextPhases.size() > 0) {
                nextPhases = phases[i]->nextPhases;
                iNextDefault = false;
            }
            for (int iNext : nextPhases) {
                if (iNext < 0 || iNext >= (int)phases.size()) {
                    throw ProcessError("Invalid nextPhase " + toString(iNext) + " in tlLogic '" + getID()
                                       + "', program '" + getProgramID() + "' with " + toString(phases.size()) + " phases");
                }
                const std::string optionalFrom = iNextDefault ? "" : " from phase " + toString(i);
                const std::string& state1 = phases[i]->getState();
                const std::string& state2 = phases[iNext]->getState();
                assert(state1.size() == state2.size());
                if (!haveWarnedAboutUnusedStates && state1.size() > myLanes.size() + myIgnoredIndices.size()) {
                    WRITE_WARNING("Unused states in tlLogic '" + getID()
                                  + "', program '" + getProgramID() + "' in phase " + toString(i)
                                  + " after tl-index " + toString((int)myLanes.size() - 1));
                    haveWarnedAboutUnusedStates = true;
                }
                // detect illegal states
                const std::string::size_type illegal = state1.find_first_not_of(SUMOXMLDefinitions::ALLOWED_TLS_LINKSTATES);
                if (std::string::npos != illegal) {
                    throw ProcessError("Illegal character '" + toString(state1[illegal]) + "' in tlLogic '" + getID()
                                       + "', program '" + getProgramID() + "' in phase " + toString(i));
                }
                // warn about transitions from green to red without intermediate yellow
                bool haveWarned = false;
                for (int j = 0; j < (int)MIN3(state1.size(), state2.size(), myLanes.size()) && !haveWarned; ++j) {
                    if ((LinkState)state2[j] == LINKSTATE_TL_RED
                            && ((LinkState)state1[j] == LINKSTATE_TL_GREEN_MAJOR
                                || (LinkState)state1[j] == LINKSTATE_TL_GREEN_MINOR)) {
                        for (LaneVector::const_iterator it = myLanes[j].begin(); it != myLanes[j].end(); ++it) {
                            if ((*it)->getPermissions() != SVC_PEDESTRIAN) {
                                WRITE_WARNING("Missing yellow phase in tlLogic '" + getID()
                                              + "', program '" + getProgramID() + "' for tl-index " + toString(j)
                                              + " when switching" + optionalFrom + " to phase " + toString(iNext));
                                // one warning per program is enough
                                haveWarned = true;
                                break;
                            }
                        }
                    }
                }
                // warn about links that never get the green light
                for (int j = 0; j < (int)state1.size(); ++j) {
                    LinkState ls = (LinkState)state1[j];
                    if (ls == LINKSTATE_TL_GREEN_MAJOR || ls == LINKSTATE_TL_GREEN_MINOR) {
                        foundGreen[j] = true;
                    }
                }
            }
        }
        for (int j = 0; j < (int)foundGreen.size(); ++j) {
            if (!foundGreen[j]) {
                WRITE_WARNING("Missing green phase in tlLogic '" + getID()
                              + "', program '" + getProgramID() + "' for tl-index " + toString(j));
                break;
            }
        }
    }
    // check incompatible junction logic
    // this can happen if the network was built with a very different signal
    // plan from the one currently being used.
    // Cconnections that never had a common green phase during network building may
    // have a symmetric response relation to avoid certain kinds of jam but this
    // can lead to deadlock if a different program gives minor green to both
    // connections at the same time
    // Note: mutual conflict between 'g' and 'G' is expected for traffic_light_right_on_red

    const bool mustCheck = MSNet::getInstance()->hasInternalLinks();
    // The checks only runs for definitions from additional file and this is sufficient.
    // The distinction is implicit because original logics are loaded earlier and at that time hasInternalLinks is alwas false
    // Also, when the network has no internal links, mutual conflicts are not built by netconvert
    //std::cout << "init tlLogic=" << getID() << " prog=" << getProgramID() << " links=" << myLinks.size() << " internal=" << MSNet::getInstance()->hasInternalLinks() << "\n";
    if (mustCheck && phases.size() > 0) {
        // see NBNode::tlsConflict
        std::set<const MSJunction*> controlledJunctions;
        const int numLinks = (int)myLinks.size();
        for (int j = 0; j < numLinks; ++j) {
            for (int k = 0; k < (int)myLinks[j].size(); ++k) {
                MSLink* link = myLinks[j][k];
                assert(link->getJunction() != nullptr);
                controlledJunctions.insert(link->getJunction());
            }
        }
        const std::string minor = "gos";
        for (const MSJunction* junction : controlledJunctions) {
            const MSJunctionLogic* logic = junction->getLogic();
            if (logic != nullptr) {
                // find symmetrical response
                const int logicSize = logic->getLogicSize();
                std::vector<int> tlIndex;
                for (int u = 0; u < logicSize; u++) {
                    const MSLogicJunction::LinkBits& response =  logic->getResponseFor(u);
                    for (int v = 0; v < logicSize; v++) {
                        if (response.test(v)) {
                            if (logic->getResponseFor(v).test(u)) {
                                // get tls link index for links u and v
                                if (tlIndex.size() == 0) {
                                    // init tlindex for all links  once
                                    tlIndex.resize(logicSize, -1);
                                    for (int j = 0; j < numLinks; ++j) {
                                        for (int k = 0; k < (int)myLinks[j].size(); ++k) {
                                            MSLink* link = myLinks[j][k];
                                            if (link->getJunction() == junction) {
                                                tlIndex[link->getIndex()] = link->getTLIndex();
                                            }
                                        }
                                    }
                                }
                                const int tlu = tlIndex[u];
                                const int tlv = tlIndex[v];
                                if (tlu >= 0 && tlv >= 0) {
                                    int phaseIndex = 0;
                                    for (MSPhaseDefinition* p : phases) {
                                        if (minor.find(p->getState()[tlu]) != std::string::npos
                                                && minor.find(p->getState()[tlv]) != std::string::npos) {
                                            WRITE_ERROR("Program '" + getProgramID() + "' at tlLogic '" + getID() + "' is incompatible with logic at junction '" + junction->getID() + "'"
                                                        + " (mututal conflict between link indices " + toString(u) + "," + toString(v)
                                                        + " tl indices " + toString(tlu) + "," + toString(tlv) + " phase " + toString(phaseIndex) + ")."
                                                        + "\n       Rebuild the network with option '--tls.ignore-internal-junction-jam or include the program when building.");
                                            return;
                                        }
                                        phaseIndex++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    myNumLinks = (int)myLinks.size();
}


MSTrafficLightLogic::~MSTrafficLightLogic() {
    // no need to do something about mySwitchCommand here,
    // it is handled by the event control
}


// ----------- Handling of controlled links
void
MSTrafficLightLogic::addLink(MSLink* link, MSLane* lane, int pos) {
    // !!! should be done within the loader (checking necessary)
    myLinks.reserve(pos + 1);
    while ((int)myLinks.size() <= pos) {
        myLinks.push_back(LinkVector());
    }
    myLinks[pos].push_back(link);
    //
    myLanes.reserve(pos + 1);
    while ((int)myLanes.size() <= pos) {
        myLanes.push_back(LaneVector());
    }
    myLanes[pos].push_back(lane);
    link->setTLState((LinkState) getCurrentPhaseDef().getState()[pos], MSNet::getInstance()->getCurrentTimeStep());
}


void
MSTrafficLightLogic::adaptLinkInformationFrom(const MSTrafficLightLogic& logic) {
    myLinks = logic.myLinks;
    myLanes = logic.myLanes;
    myIgnoredIndices = logic.myIgnoredIndices;
}


std::map<MSLink*, LinkState>
MSTrafficLightLogic::collectLinkStates() const {
    std::map<MSLink*, LinkState> ret;
    for (LinkVectorVector::const_iterator i1 = myLinks.begin(); i1 != myLinks.end(); ++i1) {
        const LinkVector& l = (*i1);
        for (LinkVector::const_iterator i2 = l.begin(); i2 != l.end(); ++i2) {
            ret[*i2] = (*i2)->getState();
        }
    }
    return ret;
}


bool
MSTrafficLightLogic::setTrafficLightSignals(SUMOTime t) const {
    // get the current traffic light signal combination
    const std::string& state = getCurrentPhaseDef().getState();
    // go through the links
    for (int i = 0; i < (int)myLinks.size(); i++) {
        const LinkVector& currGroup = myLinks[i];
        LinkState ls = (LinkState) state[i];
        for (LinkVector::const_iterator j = currGroup.begin(); j != currGroup.end(); j++) {
            (*j)->setTLState(ls, t);
        }
    }
    return true;
}


void
MSTrafficLightLogic::resetLinkStates(const std::map<MSLink*, LinkState>& vals) const {
    for (LinkVectorVector::const_iterator i1 = myLinks.begin(); i1 != myLinks.end(); ++i1) {
        const LinkVector& l = (*i1);
        for (LinkVector::const_iterator i2 = l.begin(); i2 != l.end(); ++i2) {
            assert(vals.find(*i2) != vals.end());
            (*i2)->setTLState(vals.find(*i2)->second, MSNet::getInstance()->getCurrentTimeStep());
        }
    }
}


// ----------- Static Information Retrieval
int
MSTrafficLightLogic::getLinkIndex(const MSLink* const link) const {
    int index = 0;
    for (LinkVectorVector::const_iterator i1 = myLinks.begin(); i1 != myLinks.end(); ++i1, ++index) {
        const LinkVector& l = (*i1);
        for (LinkVector::const_iterator i2 = l.begin(); i2 != l.end(); ++i2) {
            if ((*i2) == link) {
                return index;
            }
        }
    }
    return -1;
}



// ----------- Dynamic Information Retrieval
SUMOTime
MSTrafficLightLogic::getNextSwitchTime() const {
    return mySwitchCommand != nullptr ? mySwitchCommand->getNextSwitchTime() : -1;
}


SUMOTime
MSTrafficLightLogic::getSpentDuration(SUMOTime simStep) const {
    if (simStep == -1) {
        simStep = SIMSTEP;
    }
    const SUMOTime nextSwitch = getNextSwitchTime();
    if (nextSwitch == -1) {
        return -1;
    } else {
        const SUMOTime remaining = nextSwitch - simStep;
        return getCurrentPhaseDef().duration - remaining;
    }
}


// ----------- Changing phases and phase durations
void
MSTrafficLightLogic::addOverridingDuration(SUMOTime duration) {
    myOverridingTimes.push_back(duration);
}


void
MSTrafficLightLogic::setCurrentDurationIncrement(SUMOTime delay) {
    myCurrentDurationIncrement = delay;
}


void MSTrafficLightLogic::initMesoTLSPenalties() {
    // set mesoscopic time penalties
    const Phases& phases = getPhases();
    const int numLinks = (int)myLinks.size();
    // warning already given if not all states are used
    assert(numLinks <= (int)phases.front()->getState().size());
    SUMOTime duration = 0;
    std::vector<double> redDuration(numLinks, 0);
    std::vector<double> totalRedDuration(numLinks, 0);
    std::vector<double> penalty(numLinks, 0);
    for (int i = 0; i < (int)phases.size(); ++i) {
        const std::string& state = phases[i]->getState();
        duration += phases[i]->duration;
        // warn about transitions from green to red without intermediate yellow
        for (int j = 0; j < numLinks; ++j) {
            if ((LinkState)state[j] == LINKSTATE_TL_RED
                    || (LinkState)state[j] == LINKSTATE_TL_REDYELLOW) {
                redDuration[j] += STEPS2TIME(phases[i]->duration);
                totalRedDuration[j] += STEPS2TIME(phases[i]->duration);
            } else if (redDuration[j] > 0) {
                penalty[j] += 0.5 * (redDuration[j] * redDuration[j] + redDuration[j]);
                redDuration[j] = 0;
            }
        }
    }
    /// XXX penalty for wrap-around red phases is underestimated
    for (int j = 0; j < numLinks; ++j) {
        if (redDuration[j] > 0) {
            penalty[j] += 0.5 * (redDuration[j] * redDuration[j] + redDuration[j]);
            redDuration[j] = 0;
        }
    }
    const double durationSeconds = STEPS2TIME(duration);
    std::set<const MSJunction*> controlledJunctions;
    for (int j = 0; j < numLinks; ++j) {
        for (int k = 0; k < (int)myLinks[j].size(); ++k) {
            double greenFraction = (durationSeconds - totalRedDuration[j]) / durationSeconds;
            if (MSGlobals::gMesoTLSFlowPenalty == 0) {
                greenFraction = 1;
            } else {
                greenFraction = MAX2(MIN2(greenFraction / MSGlobals::gMesoTLSFlowPenalty, 1.0), 0.01);
            }
            if (greenFraction == 0.01) {
                WRITE_WARNINGF("Green fraction is only 1% for link % in tlLogic '%', program '%'.", "%", j, getID(), getProgramID());
            }
            myLinks[j][k]->setMesoTLSPenalty(TIME2STEPS(MSGlobals::gMesoTLSPenalty * penalty[j] / durationSeconds));
            myLinks[j][k]->setGreenFraction(greenFraction);
            controlledJunctions.insert(myLinks[j][k]->getLane()->getEdge().getFromJunction()); // MSLink::myJunction is not yet initialized
            //std::cout << " tls=" << getID() << " i=" << j << " link=" << myLinks[j][k]->getViaLaneOrLane()->getID() << " penalty=" << penalty[j] / durationSeconds << " durSecs=" << durationSeconds << " greenTime=" << " gF=" << myLinks[j][k]->getGreenFraction() << "\n";
        }
    }
    // initialize empty-net travel times
    // XXX refactor after merging sharps (links know their incoming edge)
    for (std::set<const MSJunction*>::iterator it = controlledJunctions.begin(); it != controlledJunctions.end(); ++it) {
        const ConstMSEdgeVector incoming = (*it)->getIncoming();
        for (ConstMSEdgeVector::const_iterator it_e = incoming.begin(); it_e != incoming.end(); ++it_e) {
            const_cast<MSEdge*>(*it_e)->recalcCache();
        }
    }

}


void
MSTrafficLightLogic::ignoreLinkIndex(int pos) {
    myIgnoredIndices.insert(pos);
}


bool
MSTrafficLightLogic::isSelected() const {
    return MSNet::getInstance()->isSelected(this);
}


void
MSTrafficLightLogic::activateProgram() {
    myAmActive = true;
}


void
MSTrafficLightLogic::deactivateProgram() {
    myAmActive = false;
}


/****************************************************************************/
