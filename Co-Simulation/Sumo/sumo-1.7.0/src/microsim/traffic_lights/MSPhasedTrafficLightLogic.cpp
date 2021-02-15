/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPhasedTrafficLightLogic.cpp
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// The base class for traffic light logic with phases
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <utility>
#include <vector>
#include <bitset>
#include <sstream>
#include <microsim/MSEventControl.h>
#include "MSTrafficLightLogic.h"
#include "MSPhasedTrafficLightLogic.h"



// ===========================================================================
// member method definitions
// ===========================================================================
MSPhasedTrafficLightLogic::MSPhasedTrafficLightLogic(MSTLLogicControl& tlcontrol,
        const std::string& id, const std::string& programID, const TrafficLightType logicType, const Phases& phases,
        int step, SUMOTime delay,
        const std::map<std::string, std::string>& parameters)
    : MSTrafficLightLogic(tlcontrol, id, programID, logicType, delay, parameters), myPhases(phases),
      myStep(step) {
    for (int i = 0; i < (int)myPhases.size(); i++) {
        myDefaultCycleTime += myPhases[i]->duration;
    }
}


MSPhasedTrafficLightLogic::~MSPhasedTrafficLightLogic() {
    // MSPhasedTrafficLightLogic:deletePhases();
    /*for (int i=0; i<myPhases.size(); i++) {
        delete myPhases[i];
    }*/
}


// ------------ Switching and setting current rows
/// MEMBER FACTORIZED TO PARENT CLASS (MSTrafficLightLogic)
/*SUMOTime
MSPhasedTrafficLightLogic::trySwitch(bool) {
    // check whether the current duration shall be increased
    if (myCurrentDurationIncrement>0) {
        SUMOTime delay = myCurrentDurationIncrement;
        myCurrentDurationIncrement = 0;
        return delay;
    }

    // increment the index
    myStep++;
    // if the last phase was reached ...
    if (myStep==(int)myPhases.size()) {
        // ... set the index to the first phase
        myStep = 0;
    }
    assert((int)myPhases.size()>myStep);
    //stores the time the phase started
    myPhases[myStep]->myLastSwitch = MSNet::getInstance()->getCurrentTimeStep();
    // check whether the next duration was overridden
    if (myOverridingTimes.size()>0) {
        SUMOTime nextDuration = myOverridingTimes[0];
        myOverridingTimes.erase(myOverridingTimes.begin());
        return nextDuration;
    }
    // return offset to the next switch
    return myPhases[myStep]->duration;
}
*/



void MSPhasedTrafficLightLogic::proceedToNextStep() {
    setStep(myStep + 1);

}

void MSPhasedTrafficLightLogic::setStep(int step) {
    step = step % myPhases.size();
    if (myStep != step) {
        myStep = step;
        myPhases[myStep]->myLastSwitch = MSNet::getInstance()->getCurrentTimeStep();
    }
}

// ------------ Static Information Retrieval
int
MSPhasedTrafficLightLogic::getPhaseNumber() const {
    return (int)myPhases.size();
}


const MSPhasedTrafficLightLogic::Phases&
MSPhasedTrafficLightLogic::getPhases() const {
    return myPhases;
}

const MSPhaseDefinition&
MSPhasedTrafficLightLogic::getPhase(int givenStep) const {
    assert((int)myPhases.size() > givenStep);
    return *myPhases[givenStep];
}


// ------------ Dynamic Information Retrieval
int
MSPhasedTrafficLightLogic::getCurrentPhaseIndex() const {
    return myStep;
}


const MSPhaseDefinition&
MSPhasedTrafficLightLogic::getCurrentPhaseDef() const {
    return *myPhases[myStep];
}


// ------------ Conversion between time and phase
SUMOTime
MSPhasedTrafficLightLogic::getPhaseIndexAtTime(SUMOTime simStep) const {
    SUMOTime position = getOffsetFromIndex(myStep);
    position += simStep - getPhase(myStep).myLastSwitch;
    position = position % myDefaultCycleTime;
    assert(position <= myDefaultCycleTime);
    return position;
}


SUMOTime
MSPhasedTrafficLightLogic::getOffsetFromIndex(int index) const {
    assert(index < (int)myPhases.size());
    SUMOTime pos = 0;
    for (int i = 0; i < index; i++) {
        pos += getPhase(i).duration;
    }
    return pos;
}


int
MSPhasedTrafficLightLogic::getIndexFromOffset(SUMOTime offset) const {
    assert(offset <= myDefaultCycleTime);
    if (offset == myDefaultCycleTime) {
        return 0;
    }
    SUMOTime pos = offset;
    SUMOTime testPos = 0;
    for (int i = 0; i < (int)myPhases.size(); i++)	{
        testPos += getPhase(i).duration;
        if (testPos > pos) {
            return i;
        }
        if (testPos == pos) {
            assert((int)myPhases.size() > (i + 1));
            return i + 1;
        }
    }
    return 0;
}


// ------------ Changing phases and phase durations
void
MSPhasedTrafficLightLogic::changeStepAndDuration(MSTLLogicControl& tlcontrol,
        SUMOTime simStep, int step, SUMOTime stepDuration) {
    mySwitchCommand->deschedule(this);
    //delete mySwitchCommand;Consider this operation!!!
    mySwitchCommand = new SwitchCommand(tlcontrol, this, stepDuration + simStep);
    myStep = step;
    MSNet::getInstance()->getBeginOfTimestepEvents()->addEvent(
        mySwitchCommand, stepDuration + simStep);
}


/****************************************************************************/
void
MSPhasedTrafficLightLogic::setPhases(const Phases& phases, int step) {
    assert(step < (int)phases.size());
    deletePhases();
    myPhases = phases;
    myStep = step;
}


void
MSPhasedTrafficLightLogic::deletePhases() {
    for (int i = 0; i < (int)myPhases.size(); i++) {
        delete myPhases[i];
    }
}

