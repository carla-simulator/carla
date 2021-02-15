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
/// @file    MSTLLogicControl.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Friedemann Wesner
/// @author  Laura Bieker
/// @author  Julia Ringel
/// @author  Michael Behrisch
/// @author  Sascha Krieg
/// @date    Sept 2002
///
// A class that stores and controls tls and switching of their programs
/****************************************************************************/
#include <config.h>

#include <vector>
#include <algorithm>
#include <cassert>
#include <iterator>
#include "MSTrafficLightLogic.h"
#include "MSSimpleTrafficLightLogic.h"
#include "MSTLLogicControl.h"
#include "MSOffTrafficLightLogic.h"
#include <microsim/MSEventControl.h>
#include <microsim/MSNet.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>

#define TRACI_PROGRAM "online"

// ===========================================================================
// method definitions
// ===========================================================================
/* -------------------------------------------------------------------------
 * MSTLLogicControl::TLSLogicVariants - methods
 * ----------------------------------------------------------------------- */
MSTLLogicControl::TLSLogicVariants::TLSLogicVariants() :
    myCurrentProgram(nullptr),
    myDefaultProgram(nullptr) {
}


MSTLLogicControl::TLSLogicVariants::~TLSLogicVariants() {
    std::map<std::string, MSTrafficLightLogic*>::const_iterator j;
    for (std::map<std::string, MSTrafficLightLogic*>::iterator j = myVariants.begin(); j != myVariants.end(); ++j) {
        delete (*j).second;
    }
    for (std::vector<OnSwitchAction*>::iterator i = mySwitchActions.begin(); i != mySwitchActions.end(); ++i) {
        delete *i;
    }
}


bool
MSTLLogicControl::TLSLogicVariants::checkOriginalTLS() const {
    bool hadErrors = false;
    for (std::map<std::string, MSTrafficLightLogic*>::const_iterator j = myVariants.begin(); j != myVariants.end(); ++j) {
        const MSTrafficLightLogic::Phases& phases = (*j).second->getPhases();
        int linkNo = (int)(*j).second->getLinks().size();
        bool hadProgramErrors = false;
        for (MSTrafficLightLogic::Phases::const_iterator i = phases.begin(); i != phases.end(); ++i) {
            if ((int)(*i)->getState().length() < linkNo) {
                hadProgramErrors = true;
            }
        }
        if (hadProgramErrors) {
            WRITE_ERROR("Mismatching phase size in tls '" + (*j).second->getID() + "', program '" + (*j).first + "'.");
            hadErrors = true;
        }
    }
    return !hadErrors;
}


void
MSTLLogicControl::TLSLogicVariants::saveInitialStates() {
    myOriginalLinkStates = myCurrentProgram->collectLinkStates();
}


bool
MSTLLogicControl::TLSLogicVariants::addLogic(const std::string& programID,
        MSTrafficLightLogic* logic, bool netWasLoaded, bool isNewDefault) {
    if (myVariants.find(programID) != myVariants.end()) {
        return false;
    }
    // assert the links are set
    if (netWasLoaded) {
        // this one has not yet its links set
        if (myCurrentProgram == nullptr) {
            throw ProcessError("No initial signal plan loaded for tls '" + logic->getID() + "'.");
        }
        logic->adaptLinkInformationFrom(*myCurrentProgram);
        if (logic->getLinks().size() > logic->getPhase(0).getState().size()) {
            throw ProcessError("Mismatching phase size in tls '" + logic->getID() + "', program '" + programID + "'.");
        }
    }
    // add to the list of active
    if (myVariants.size() == 0 || isNewDefault) {
        if (myCurrentProgram != nullptr) {
            myCurrentProgram->deactivateProgram();
        }
        myCurrentProgram = logic;
        myCurrentProgram->activateProgram();
        if (myVariants.size() == 0) {
            myDefaultProgram = logic;
        }
    }
    // add to the list of logic
    myVariants[programID] = logic;
    if (myVariants.size() == 1 || isNewDefault) {
        logic->setTrafficLightSignals(MSNet::getInstance()->getCurrentTimeStep());
        executeOnSwitchActions();
    }
    return true;
}


MSTrafficLightLogic*
MSTLLogicControl::TLSLogicVariants::getLogic(const std::string& programID) const {
    if (myVariants.find(programID) == myVariants.end()) {
        return nullptr;
    }
    return myVariants.find(programID)->second;
}


MSTrafficLightLogic*
MSTLLogicControl::TLSLogicVariants::getLogicInstantiatingOff(MSTLLogicControl& tlc,
        const std::string& programID) {
    if (myVariants.find(programID) == myVariants.end()) {
        if (programID == "off") {
            // build an off-tll if this switch indicates it
            if (!addLogic("off", new MSOffTrafficLightLogic(tlc, myCurrentProgram->getID()), true, true)) {
                // inform the user if this fails
                throw ProcessError("Could not build an off-state for tls '" + myCurrentProgram->getID() + "'.");
            }
        } else {
            // inform the user about a missing logic
            throw ProcessError("Can not switch tls '" + myCurrentProgram->getID() + "' to program '" + programID + "';\n The program is not known.");
        }
    }
    return getLogic(programID);
}


void
MSTLLogicControl::TLSLogicVariants::setStateInstantiatingOnline(MSTLLogicControl& tlc,
        const std::string& state) {
    // build only once...
    MSTrafficLightLogic* logic = getLogic(TRACI_PROGRAM);
    if (logic == nullptr) {
        MSPhaseDefinition* phase = new MSPhaseDefinition(DELTA_T, state, -1);
        std::vector<MSPhaseDefinition*> phases;
        phases.push_back(phase);
        logic = new MSSimpleTrafficLightLogic(tlc, myCurrentProgram->getID(), TRACI_PROGRAM, TrafficLightType::STATIC, phases, 0,
                                              MSNet::getInstance()->getCurrentTimeStep() + DELTA_T,
                                              std::map<std::string, std::string>());
        addLogic(TRACI_PROGRAM, logic, true, true);
        MSNet::getInstance()->createTLWrapper(logic);
    } else {
        MSPhaseDefinition nphase(DELTA_T, state, -1);
        *(dynamic_cast<MSSimpleTrafficLightLogic*>(logic)->getPhases()[0]) = nphase;
        switchTo(tlc, TRACI_PROGRAM);
    }
}


void
MSTLLogicControl::TLSLogicVariants::addSwitchCommand(OnSwitchAction* c) {
    mySwitchActions.push_back(c);
}


std::vector<MSTrafficLightLogic*>
MSTLLogicControl::TLSLogicVariants::getAllLogics() const {
    std::vector<MSTrafficLightLogic*> ret;
    std::map<std::string, MSTrafficLightLogic*>::const_iterator i;
    for (i = myVariants.begin(); i != myVariants.end(); ++i) {
        ret.push_back((*i).second);
    }
    return ret;
}


bool
MSTLLogicControl::TLSLogicVariants::isActive(const MSTrafficLightLogic* tl) const {
    return tl == myCurrentProgram;
}


MSTrafficLightLogic*
MSTLLogicControl::TLSLogicVariants::getActive() const {
    return myCurrentProgram;
}

MSTrafficLightLogic*
MSTLLogicControl::TLSLogicVariants::getDefault() const {
    return myDefaultProgram;
}


void
MSTLLogicControl::TLSLogicVariants::switchTo(MSTLLogicControl& tlc, const std::string& programID) {
    // set the found wished sub-program as this tls' current one
    myCurrentProgram->deactivateProgram();
    myCurrentProgram = getLogicInstantiatingOff(tlc, programID);
    myCurrentProgram->activateProgram();
    myCurrentProgram->setTrafficLightSignals(MSNet::getInstance()->getCurrentTimeStep());
    executeOnSwitchActions();
}


void
MSTLLogicControl::TLSLogicVariants::executeOnSwitchActions() const {
    for (std::vector<OnSwitchAction*>::const_iterator i = mySwitchActions.begin(); i != mySwitchActions.end(); ++i) {
        (*i)->execute();
    }
}


void
MSTLLogicControl::TLSLogicVariants::addLink(MSLink* link, MSLane* lane, int pos) {
    for (std::map<std::string, MSTrafficLightLogic*>::iterator i = myVariants.begin(); i != myVariants.end(); ++i) {
        (*i).second->addLink(link, lane, pos);
    }
}

void
MSTLLogicControl::TLSLogicVariants::ignoreLinkIndex(int pos) {
    for (std::map<std::string, MSTrafficLightLogic*>::iterator i = myVariants.begin(); i != myVariants.end(); ++i) {
        (*i).second->ignoreLinkIndex(pos);
    }
}


/* -------------------------------------------------------------------------
 * method definitions for the Switching Procedures
 * ----------------------------------------------------------------------- */
/* -------------------------------------------------------------------------
 * method definitions for WAUTSwitchProcedure
 * ----------------------------------------------------------------------- */
bool
MSTLLogicControl::WAUTSwitchProcedure::trySwitch(SUMOTime step) {
    // switch to the next programm if the GSP is reached
    if (isPosAtGSP(step, *myFrom)) {
        // adapt program's state
        if (mySwitchSynchron) {
            adaptLogic(step);
        } else {
            switchToPos(step, *myTo, getGSPTime(*myTo));
        }
        // switch to destination program
        return true;
    }
    // do not switch, yet
    return false;
}


SUMOTime
MSTLLogicControl::WAUTSwitchProcedure::getGSPTime(const MSTrafficLightLogic& logic) const {
    return string2time(logic.getParameter("GSP", "0"));
}


bool
MSTLLogicControl::WAUTSwitchProcedure::isPosAtGSP(SUMOTime currentTime, const MSTrafficLightLogic& logic) {
    const SUMOTime gspTime = getGSPTime(logic) % logic.getDefaultCycleTime();
    const SUMOTime programTime = logic.getOffsetFromIndex(logic.getCurrentPhaseIndex()) + logic.getSpentDuration(currentTime);
    return gspTime == programTime;
}


SUMOTime
MSTLLogicControl::WAUTSwitchProcedure::getDiffToStartOfPhase(MSTrafficLightLogic& logic, SUMOTime toTime) {
    int stepOfMyPos = logic.getIndexFromOffset(toTime);
    SUMOTime startOfPhase = logic.getOffsetFromIndex(stepOfMyPos);
    assert(toTime >= startOfPhase);
    return toTime - startOfPhase;
}


void
MSTLLogicControl::WAUTSwitchProcedure::switchToPos(SUMOTime simStep, MSTrafficLightLogic& logic, SUMOTime toTime) {
    int stepTo = logic.getIndexFromOffset(toTime);
    SUMOTime diff = getDiffToStartOfPhase(logic, toTime);
    const MSPhaseDefinition& phase = logic.getPhase(stepTo);
    SUMOTime leftDuration = phase.duration - diff;
    logic.changeStepAndDuration(myControl, simStep, stepTo, leftDuration);
}



/* -------------------------------------------------------------------------
 * method definitions for WAUTSwitchProcedure_JustSwitch
 * ----------------------------------------------------------------------- */
MSTLLogicControl::WAUTSwitchProcedure_JustSwitch::WAUTSwitchProcedure_JustSwitch(
    MSTLLogicControl& control, WAUT& waut,
    MSTrafficLightLogic* from, MSTrafficLightLogic* to, bool synchron)
    : MSTLLogicControl::WAUTSwitchProcedure(control, waut, from, to, synchron) {}


MSTLLogicControl::WAUTSwitchProcedure_JustSwitch::~WAUTSwitchProcedure_JustSwitch() {}


bool
MSTLLogicControl::WAUTSwitchProcedure_JustSwitch::trySwitch(SUMOTime) {
    return true;
}



/* -------------------------------------------------------------------------
 * method definitions for WAUTSwitchProcedure_GSP
 * ----------------------------------------------------------------------- */
MSTLLogicControl::WAUTSwitchProcedure_GSP::WAUTSwitchProcedure_GSP(
    MSTLLogicControl& control, WAUT& waut,
    MSTrafficLightLogic* from, MSTrafficLightLogic* to, bool synchron)
    : MSTLLogicControl::WAUTSwitchProcedure(control, waut, from, to, synchron) {}


MSTLLogicControl::WAUTSwitchProcedure_GSP::~WAUTSwitchProcedure_GSP() {}


void
MSTLLogicControl::WAUTSwitchProcedure_GSP::adaptLogic(SUMOTime step) {
    const SUMOTime gspTo = getGSPTime(*myTo) % myTo->getDefaultCycleTime();
    const SUMOTime currentPosTo = myTo->getOffsetFromIndex(myTo->getCurrentPhaseIndex()) + myTo->getSpentDuration(step);
    SUMOTime deltaToStretch = gspTo - currentPosTo;
    if (deltaToStretch < 0) {
        deltaToStretch += myTo->getDefaultCycleTime();
    }
    const int stepTo = myTo->getIndexFromOffset(gspTo);
    const SUMOTime newdur = myTo->getPhase(stepTo).duration - getDiffToStartOfPhase(*myTo, gspTo) + deltaToStretch;
    myTo->changeStepAndDuration(myControl, step, stepTo, newdur);
}



/* -------------------------------------------------------------------------
 * method definitions for WAUTSwitchProcedure_Stretch
 * ----------------------------------------------------------------------- */
MSTLLogicControl::WAUTSwitchProcedure_Stretch::WAUTSwitchProcedure_Stretch(
    MSTLLogicControl& control, WAUT& waut,
    MSTrafficLightLogic* from, MSTrafficLightLogic* to, bool synchron)
    : MSTLLogicControl::WAUTSwitchProcedure(control, waut, from, to, synchron) {
    int idx = 1;
    while (myTo->knowsParameter("B" + toString(idx) + ".begin")) {
        StretchRange def;
        def.begin = string2time(myTo->getParameter("B" + toString(idx) + ".begin"));
        def.end = string2time(myTo->getParameter("B" + toString(idx) + ".end"));
        def.fac = StringUtils::toDouble(myTo->getParameter("B" + toString(idx) + ".factor"));
        myStretchRanges.emplace_back(def);
    }

}


MSTLLogicControl::WAUTSwitchProcedure_Stretch::~WAUTSwitchProcedure_Stretch() {}


void
MSTLLogicControl::WAUTSwitchProcedure_Stretch::adaptLogic(SUMOTime step) {
    SUMOTime gspTo = getGSPTime(*myTo);
    SUMOTime cycleTime = myTo->getDefaultCycleTime();
    // the position, where the logic has to be after synchronisation
    SUMOTime posAfterSyn = myTo->getPhaseIndexAtTime(step);
    // calculate the difference, that has to be equalized
    SUMOTime deltaToCut = 0;
    if (posAfterSyn < gspTo) {
        deltaToCut = posAfterSyn + cycleTime - gspTo;
    } else {
        deltaToCut =  posAfterSyn - gspTo;
    }
    // test, wheter cutting of the Signalplan is possible
    SUMOTime deltaPossible = 0;
    for (const StretchRange& def : myStretchRanges) {
        assert(def.end >= def.begin);
        deltaPossible += def.end - def.begin;
    }
    int stretchUmlaufAnz = (int) StringUtils::toDouble(myTo->getParameter("StretchUmlaufAnz", ""));
    deltaPossible = stretchUmlaufAnz * deltaPossible;
    if ((deltaPossible > deltaToCut) && (deltaToCut < (cycleTime / 2))) {
        cutLogic(step, gspTo, deltaToCut);
    } else {
        SUMOTime deltaToStretch = (cycleTime - deltaToCut) % cycleTime;
        stretchLogic(step, gspTo, deltaToStretch);
    }
}


void
MSTLLogicControl::WAUTSwitchProcedure_Stretch::cutLogic(SUMOTime step, SUMOTime startPos, SUMOTime allCutTime) {
    int actStep = myTo->getIndexFromOffset(startPos);
    // switches to startPos and cuts this phase, if there is a "Bereich"
    SUMOTime toCut = 0;
    for (const StretchRange& def : myStretchRanges) {
        int stepOfBegin = myTo->getIndexFromOffset(def.begin);
        if (stepOfBegin == actStep) {
            if (def.begin < startPos) {
                toCut = def.end - startPos;
            } else {
                toCut = def.end - def.begin;
            }
            toCut = MIN2(allCutTime, toCut);
            allCutTime = allCutTime - toCut;
        }
    }
    SUMOTime remainingDur = myTo->getPhase(actStep).duration - getDiffToStartOfPhase(*myTo, startPos);
    SUMOTime newDur = remainingDur - toCut;
    myTo->changeStepAndDuration(myControl, step, actStep, newDur);

    // changes the duration of all other phases
    int currStep = (actStep + 1) % (int)myTo->getPhases().size();
    while (allCutTime > 0) {
        for (int i = currStep; i < (int) myTo->getPhases().size(); i++) {
            SUMOTime beginOfPhase = myTo->getOffsetFromIndex(i);
            SUMOTime durOfPhase = myTo->getPhase(i).duration;
            SUMOTime endOfPhase = beginOfPhase + durOfPhase;
            for (const StretchRange& def : myStretchRanges) {
                if ((beginOfPhase <= def.begin) && (endOfPhase >= def.end)) {
                    SUMOTime maxCutOfPhase = MIN2(def.end - def.begin, allCutTime);
                    allCutTime = allCutTime - maxCutOfPhase;
                    durOfPhase = durOfPhase - maxCutOfPhase;
                }
            }
            myTo->addOverridingDuration(durOfPhase);
        }
        currStep = 0;
    }
}

void
MSTLLogicControl::WAUTSwitchProcedure_Stretch::stretchLogic(SUMOTime step, SUMOTime startPos, SUMOTime allStretchTime) {
    int currStep = myTo->getIndexFromOffset(startPos);
    SUMOTime durOfPhase = myTo->getPhase(currStep).duration;
    SUMOTime remainingStretchTime = allStretchTime;
    SUMOTime StretchTimeOfPhase = 0;
    int stretchUmlaufAnz = (int) StringUtils::toDouble(myTo->getParameter("StretchUmlaufAnz", ""));
    double facSum = 0;
    for (const StretchRange& def : myStretchRanges) {
        facSum += def.fac;
    }
    facSum *= stretchUmlaufAnz;

    //switch to startPos and stretch this phase, if there is a end of "bereich" between startpos and end of phase
    SUMOTime diffToStart = getDiffToStartOfPhase(*myTo, startPos);
    for (const StretchRange& def : myStretchRanges) {
        SUMOTime endOfPhase = (startPos + durOfPhase - diffToStart);
        if (def.end <= endOfPhase && def.end >= startPos) {
            double actualfac = def.fac / facSum;
            facSum = facSum - def.fac;
            StretchTimeOfPhase = TIME2STEPS(int(STEPS2TIME(remainingStretchTime) * actualfac + 0.5));
            remainingStretchTime = allStretchTime - StretchTimeOfPhase;
        }
    }
    if (facSum == 0) {
        WRITE_WARNING("The computed factor sum in WAUT '" + myWAUT.id + "' at time '" + toString(STEPS2TIME(step)) + "' equals zero;\n assuming an error in WAUT definition.");
        return;
    }
    durOfPhase = durOfPhase - diffToStart + StretchTimeOfPhase;
    myTo->changeStepAndDuration(myControl, step, currStep, durOfPhase);

    currStep = (currStep + 1) % (int)myTo->getPhases().size();
    // stretch all other phases, if there is a "bereich"
    while (remainingStretchTime > 0) {
        for (int i = currStep; i < (int)myTo->getPhases().size() && remainingStretchTime > 0; i++) {
            durOfPhase = myTo->getPhase(i).duration;
            SUMOTime beginOfPhase = myTo->getOffsetFromIndex(i);
            SUMOTime endOfPhase = beginOfPhase + durOfPhase;
            for (const StretchRange& def : myStretchRanges) {
                if ((beginOfPhase <= def.end) && (endOfPhase >= def.end)) {
                    double actualfac = def.fac / facSum;
                    StretchTimeOfPhase = TIME2STEPS(int(STEPS2TIME(remainingStretchTime) * actualfac + 0.5));
                    facSum -= def.fac;
                    durOfPhase += StretchTimeOfPhase;
                    remainingStretchTime -= StretchTimeOfPhase;
                }
            }
            myTo->addOverridingDuration(durOfPhase);
        }
        currStep = 0;
    }
}


/* -------------------------------------------------------------------------
 * method definitions for MSTLLogicControl
 * ----------------------------------------------------------------------- */
MSTLLogicControl::MSTLLogicControl()
    : myNetWasLoaded(false) {}


MSTLLogicControl::~MSTLLogicControl() {
    // delete tls
    for (std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.begin(); i != myLogics.end(); ++i) {
        delete (*i).second;
    }
    // delete WAUTs
    for (std::map<std::string, WAUT*>::const_iterator i = myWAUTs.begin(); i != myWAUTs.end(); ++i) {
        delete (*i).second;
    }
}


void
MSTLLogicControl::setTrafficLightSignals(SUMOTime t) const {
    for (std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.begin(); i != myLogics.end(); ++i) {
        (*i).second->getActive()->setTrafficLightSignals(t);
    }
}


std::vector<MSTrafficLightLogic*>
MSTLLogicControl::getAllLogics() const {
    std::vector<MSTrafficLightLogic*> ret;
    std::map<std::string, TLSLogicVariants*>::const_iterator i;
    for (i = myLogics.begin(); i != myLogics.end(); ++i) {
        std::vector<MSTrafficLightLogic*> s = (*i).second->getAllLogics();
        copy(s.begin(), s.end(), back_inserter(ret));
    }
    return ret;
}

MSTLLogicControl::TLSLogicVariants&
MSTLLogicControl::get(const std::string& id) const {
    std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.find(id);
    if (i == myLogics.end()) {
        throw InvalidArgument("The tls '" + id + "' is not known.");
    }
    return *(*i).second;
}


MSTrafficLightLogic*
MSTLLogicControl::get(const std::string& id, const std::string& programID) const {
    std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.find(id);
    if (i == myLogics.end()) {
        return nullptr;
    }
    return (*i).second->getLogic(programID);
}


std::vector<std::string>
MSTLLogicControl::getAllTLIds() const {
    std::vector<std::string> ret;
    for (std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.begin(); i != myLogics.end(); ++i) {
        ret.push_back((*i).first);
    }
    return ret;
}


bool
MSTLLogicControl::add(const std::string& id, const std::string& programID,
                      MSTrafficLightLogic* logic, bool newDefault) {
    if (myLogics.find(id) == myLogics.end()) {
        myLogics[id] = new TLSLogicVariants();
    }
    std::map<std::string, TLSLogicVariants*>::iterator i = myLogics.find(id);
    TLSLogicVariants* tlmap = (*i).second;
    return tlmap->addLogic(programID, logic, myNetWasLoaded, newDefault);
}


bool
MSTLLogicControl::knows(const std::string& id) const {
    std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.find(id);
    if (i == myLogics.end()) {
        return false;
    }
    return true;
}


bool
MSTLLogicControl::closeNetworkReading() {
    bool hadErrors = false;
    for (std::map<std::string, TLSLogicVariants*>::iterator i = myLogics.begin(); i != myLogics.end(); ++i) {
        hadErrors |= !(*i).second->checkOriginalTLS();
        (*i).second->saveInitialStates();
    }
    myNetWasLoaded = true;
    return !hadErrors;
}


bool
MSTLLogicControl::isActive(const MSTrafficLightLogic* tl) const {
    std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.find(tl->getID());
    if (i == myLogics.end()) {
        return false;
    }
    return (*i).second->isActive(tl);
}


MSTrafficLightLogic*
MSTLLogicControl::getActive(const std::string& id) const {
    std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.find(id);
    if (i == myLogics.end()) {
        return nullptr;
    }
    return (*i).second->getActive();
}


void
MSTLLogicControl::switchTo(const std::string& id, const std::string& programID) {
    // try to get the tls program definitions
    std::map<std::string, TLSLogicVariants*>::iterator i = myLogics.find(id);
    // handle problems
    if (i == myLogics.end()) {
        throw ProcessError("Could not switch tls '" + id + "' to program '" + programID + "': No such tls exists.");
    }
    (*i).second->switchTo(*this, programID);
}


void
MSTLLogicControl::addWAUT(SUMOTime refTime, const std::string& id,
                          const std::string& startProg, SUMOTime period) {
    // check whether the waut was already defined
    if (myWAUTs.find(id) != myWAUTs.end()) {
        // report an error if so
        throw InvalidArgument("Waut '" + id + "' was already defined.");
    }
    WAUT* w = new WAUT;
    w->id = id;
    w->refTime = refTime;
    w->startProg = startProg;
    w->period = period;
    myWAUTs[id] = w;
}


void
MSTLLogicControl::addWAUTSwitch(const std::string& wautid,
                                SUMOTime when, const std::string& to) {
    // try to get the waut
    if (myWAUTs.find(wautid) == myWAUTs.end()) {
        // report an error if the waut is not known
        throw InvalidArgument("Waut '" + wautid + "' was not yet defined.");
    }
    // build and save the waut switch definition
    WAUT* waut = myWAUTs[wautid];
    WAUTSwitch s;
    s.to = to;
    s.when = (waut->refTime + when);
    if (waut->period > 0) {
        s.when = s.when % waut->period;
    }
    myWAUTs[wautid]->switches.push_back(s);
}


void
MSTLLogicControl::addWAUTJunction(const std::string& wautid,
                                  const std::string& tls,
                                  const std::string& proc,
                                  bool synchron) {
    // try to get the waut
    if (myWAUTs.find(wautid) == myWAUTs.end()) {
        // report an error if the waut is not known
        throw InvalidArgument("Waut '" + wautid + "' was not yet defined.");
    }
    // try to get the tls to switch
    if (myLogics.find(tls) == myLogics.end()) {
        // report an error if the tls is not known
        throw InvalidArgument("TLS '" + tls + "' to switch in WAUT '" + wautid + "' was not yet defined.");
    }
    WAUTJunction j;
    j.junction = tls;
    j.procedure = proc;
    j.synchron = synchron;
    myWAUTs[wautid]->junctions.push_back(j);

    std::string initProg = myWAUTs[wautid]->startProg;
    std::vector<WAUTSwitch>::const_iterator first = myWAUTs[wautid]->switches.end();
    SUMOTime minExecTime = -1;
    for (std::vector<WAUTSwitch>::const_iterator i = myWAUTs[wautid]->switches.begin(); i != myWAUTs[wautid]->switches.end(); ++i) {
        if ((*i).when > MSNet::getInstance()->getCurrentTimeStep() && (minExecTime == -1 || (*i).when < minExecTime)) {
            minExecTime = (*i).when;
            first = i;
        }
        if (first != myWAUTs[wautid]->switches.begin()) {
            initProg = (*(first - 1)).to;
        }
    }
    // activate the first one
    switchTo(tls, initProg);
}


void
MSTLLogicControl::closeWAUT(const std::string& wautid) {
    // try to get the waut
    if (myWAUTs.find(wautid) == myWAUTs.end()) {
        // report an error if the waut is not known
        throw InvalidArgument("Waut '" + wautid + "' was not yet defined.");
    }
    WAUT* w = myWAUTs.find(wautid)->second;
    std::string initProg = myWAUTs[wautid]->startProg;
    // get the switch to be performed as first
    std::vector<WAUTSwitch>::const_iterator first = w->switches.end();
    SUMOTime minExecTime = -1;
    for (std::vector<WAUTSwitch>::const_iterator i = w->switches.begin(); i != w->switches.end(); ++i) {
        if ((*i).when > MSNet::getInstance()->getCurrentTimeStep() && (minExecTime == -1 || (*i).when < minExecTime)) {
            minExecTime = (*i).when;
            first = i;
        }
    }
    // activate the first one
    if (first != w->switches.end()) {
        std::vector<WAUTSwitch>::const_iterator mbegin = w->switches.begin();
        MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(
            new SwitchInitCommand(*this, wautid, (int)distance(mbegin, first)),
            (*first).when);
    }
    /*
    // set the current program to all junctions
    for(std::vector<WAUTJunction>::const_iterator i=w->junctions.begin(); i!=w->junctions.end(); ++i) {
        switchTo((*i).junction, initProg);
    }
    */
}


SUMOTime
MSTLLogicControl::initWautSwitch(MSTLLogicControl::SwitchInitCommand& cmd) {
    const std::string& wautid = cmd.getWAUTID();
    int& index = cmd.getIndex();
    WAUT* waut = myWAUTs[wautid];
    WAUTSwitch s = waut->switches[index];
    for (std::vector<WAUTJunction>::iterator i = myWAUTs[wautid]->junctions.begin(); i != myWAUTs[wautid]->junctions.end(); ++i) {
        // get the current program and the one to instantiate
        TLSLogicVariants* vars = myLogics.find((*i).junction)->second;
        MSTrafficLightLogic* from = vars->getActive();
        MSTrafficLightLogic* to = vars->getLogicInstantiatingOff(*this, s.to);
        WAUTSwitchProcedure* proc = nullptr;
        if ((*i).procedure == "GSP") {
            proc = new WAUTSwitchProcedure_GSP(*this, *myWAUTs[wautid], from, to, (*i).synchron);
        } else if ((*i).procedure == "Stretch") {
            proc = new WAUTSwitchProcedure_Stretch(*this, *myWAUTs[wautid], from, to, (*i).synchron);
        } else {
            proc = new WAUTSwitchProcedure_JustSwitch(*this, *myWAUTs[wautid], from, to, (*i).synchron);
        }

        WAUTSwitchProcess p;
        p.junction = (*i).junction;
        p.proc = proc;
        p.from = from;
        p.to = to;

        myCurrentlySwitched.push_back(p);
    }
    index++;
    if (index == (int)waut->switches.size()) {
        if (waut->period <= 0) {
            return 0;
        } else {
            index = 0; // start over
            for (WAUTSwitch& ws : waut->switches) {
                ws.when += waut->period;
            }
        }
    }
    return myWAUTs[wautid]->switches[index].when - MSNet::getInstance()->getCurrentTimeStep();
}


void
MSTLLogicControl::check2Switch(SUMOTime step) {
    for (std::vector<WAUTSwitchProcess>::iterator i = myCurrentlySwitched.begin(); i != myCurrentlySwitched.end();) {
        const WAUTSwitchProcess& proc = *i;
        if (proc.proc->trySwitch(step)) {
            delete proc.proc;
            // do not switch away from TraCI control
            if (getActive(proc.to->getID())->getProgramID() != TRACI_PROGRAM) {
                switchTo(proc.to->getID(), proc.to->getProgramID());
            }
            i = myCurrentlySwitched.erase(i);
        } else {
            ++i;
        }
    }
}


std::pair<SUMOTime, MSPhaseDefinition>
MSTLLogicControl::getPhaseDef(const std::string& tlid) const {
    MSTrafficLightLogic* tl = getActive(tlid);
    return std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), tl->getCurrentPhaseDef());
}


void
MSTLLogicControl::switchOffAll() {
    for (std::map<std::string, TLSLogicVariants*>::const_iterator i = myLogics.begin(); i != myLogics.end(); ++i) {
        (*i).second->addLogic("off",  new MSOffTrafficLightLogic(*this, (*i).first), true, true);
    }
}


/****************************************************************************/
