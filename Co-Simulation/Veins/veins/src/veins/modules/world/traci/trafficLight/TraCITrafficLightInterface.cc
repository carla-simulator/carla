//
// Copyright (C) 2015-2018 Dominik Buse <dbuse@mail.uni-paderborn.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#include <string>
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"
#include "veins/modules/messages/TraCITrafficLightMessage_m.h"

using namespace veins;

using veins::TraCITrafficLightInterface;
using veins::TraCITrafficLightLink;
using veins::TraCITrafficLightProgram;

Define_Module(veins::TraCITrafficLightInterface);

TraCITrafficLightInterface::TraCITrafficLightInterface()
    : cSimpleModule()
    , isPreInitialized(false)
    , updateInterval()
    , manager(nullptr)
    , commandInterface(nullptr)
    , tlCommandInterface(nullptr)
    , external_id("")
    , position()
    , programDefinition()
    , currentLogicId("")
    , currentPhaseNr(-1)
    , nextSwitchTime()
    , inOnlineSignalState(false)
{
}

TraCITrafficLightInterface::~TraCITrafficLightInterface()
{
    delete tlCommandInterface;
}

void TraCITrafficLightInterface::preInitialize(const std::string& external_id, const Coord& position, const simtime_t& updateInterval)
{
    isPreInitialized = true;
    this->updateInterval = updateInterval;
    setExternalId(external_id);
    this->position = position;
}

Coord TraCITrafficLightInterface::getPosition() const
{
    return this->position;
}
std::list<std::list<TraCITrafficLightLink>> TraCITrafficLightInterface::getControlledLinks()
{
    return controlledLinks;
}

TraCITrafficLightProgram::Logic TraCITrafficLightInterface::getCurrentLogic() const
{
    return programDefinition.getLogic(currentLogicId);
}

std::string TraCITrafficLightInterface::getCurrentLogicId() const
{
    return currentLogicId;
}

int TraCITrafficLightInterface::getCurrentPhaseId() const
{
    return currentPhaseNr;
}

TraCITrafficLightProgram::Phase TraCITrafficLightInterface::getCurrentPhase() const
{
    return getCurrentLogic().phases[currentPhaseNr];
}

simtime_t TraCITrafficLightInterface::getAssumedNextSwitch() const
{
    return nextSwitchTime;
}

simtime_t TraCITrafficLightInterface::getRemainingDuration() const
{
    return nextSwitchTime - simTime();
}

std::string TraCITrafficLightInterface::getCurrentState() const
{
    if (isInOnlineSignalState()) {
        return currentSignalState;
    }
    else {
        return getCurrentPhase().state;
    }
}

bool TraCITrafficLightInterface::isInOnlineSignalState() const
{
    return inOnlineSignalState;
}

void TraCITrafficLightInterface::setProgramDefinition(const TraCITrafficLightProgram& programDefinition)
{
    this->programDefinition = programDefinition;
}

void TraCITrafficLightInterface::setControlledLinks(const std::list<std::list<TraCITrafficLightLink>>& controlledLinks)
{
    this->controlledLinks = controlledLinks;
}

void TraCITrafficLightInterface::setCurrentLogicById(const std::string& logicId, bool setSumo)
{
    if (setSumo) {
        ASSERT(logicId != "online");
        if (!programDefinition.hasLogic(logicId)) {
            throw cRuntimeError("Logic '%s' not found in program of TraCITrafficLightInterface %s", logicId.c_str(), external_id.c_str());
        }
        tlCommandInterface->setProgram(logicId);
        const std::string newValueInSumo = tlCommandInterface->getCurrentProgramID();
        ASSERT(newValueInSumo == logicId);
    }
    if (currentLogicId != logicId || (isInOnlineSignalState() && logicId != "online")) {
        sendChangeMsg(TrafficLightAtrributeType::LOGICID, logicId, currentLogicId);
    }
    if (logicId != "online") {
        inOnlineSignalState = false;
        this->currentLogicId = logicId;
    }
}

void TraCITrafficLightInterface::setCurrentPhaseByNr(const unsigned int phaseNr, bool setSumo)
{
    if (setSumo) {
        if (phaseNr >= getCurrentLogic().phases.size()) {
            throw cRuntimeError("Cannot set current phase to %d: current logic has only %d Phases (TraCITrafficLightInterface %s)", phaseNr, getCurrentLogic().phases.size(), external_id.c_str());
        }
        tlCommandInterface->setPhaseIndex(phaseNr);
        const unsigned int newValueInSumo = tlCommandInterface->getCurrentPhaseIndex();
        ASSERT(newValueInSumo == phaseNr);
    }
    if (currentPhaseNr != static_cast<int>(phaseNr) || isInOnlineSignalState()) {
        sendChangeMsg(TrafficLightAtrributeType::PHASEID, std::to_string(phaseNr), std::to_string(currentPhaseNr));
    }
    inOnlineSignalState = false;
    currentPhaseNr = phaseNr;
}

void TraCITrafficLightInterface::setCurrentState(const std::string& state, bool setSumo)
{
    if (setSumo) {
        tlCommandInterface->setState(state);
        const std::string newValueInSumo = tlCommandInterface->getCurrentState();
        ASSERT(newValueInSumo == state);
    }
    if (currentSignalState != state) {
        sendChangeMsg(TrafficLightAtrributeType::STATE, state, currentSignalState);
    }
    inOnlineSignalState = true;
    currentSignalState = state;
}

void TraCITrafficLightInterface::setNextSwitch(const simtime_t& newNextSwitch, bool setSumo)
{
    // FIXME: not working reliably - use setRemainingDuration instead!
    // round to be feasible for SUMO
    simtime_t nextSwitch = ceil(newNextSwitch, updateInterval, 0);
    if (setSumo) {
        simtime_t remainingDuration = ceil(nextSwitch - simTime(), updateInterval, 0);
        if (remainingDuration < 0) {
            EV << "Warning: remaining duration for switch below 0: " << remainingDuration << std::endl;
            // maybe issue even an error if this occurs
            remainingDuration = 0;
        }
        getTlCommandInterface()->setPhaseDuration(remainingDuration);
        simtime_t newValueInSumo = tlCommandInterface->getAssumedNextSwitchTime();
        ASSERT(newValueInSumo == nextSwitch);
    }
    if (nextSwitchTime != nextSwitch) {
        sendChangeMsg(TrafficLightAtrributeType::SWITCHTIME, std::to_string(nextSwitch.inUnit(SIMTIME_MS)), std::to_string(nextSwitchTime.inUnit(SIMTIME_MS)));
    }
    nextSwitchTime = nextSwitch;
}

void TraCITrafficLightInterface::setRemainingDuration(const simtime_t& rawRemainingDuration, bool setSumo)
{
    ASSERT(rawRemainingDuration >= 0);
    // round (up) to match sumo time steps
    simtime_t veinsTimeNow(simTime());
    simtime_t sumoTimeNow(ceil(veinsTimeNow, updateInterval) - updateInterval);
    simtime_t roundedRemainingDuration = ceil(rawRemainingDuration, updateInterval, 0);
    // simtime_t nextSwitchInVeins = floor(simTime() + roundedRemainingDuration, updateInterval, 0) - updateInterval;
    simtime_t nextSwitchInVeins = sumoTimeNow + roundedRemainingDuration;
    if (setSumo) {
        // set value to sumo
        getTlCommandInterface()->setPhaseDuration(roundedRemainingDuration);
        // check that everything is consistent
        simtime_t nextSwitchInSumo = tlCommandInterface->getAssumedNextSwitchTime();
        ASSERT(nextSwitchInSumo == nextSwitchInVeins);
    }
    if (nextSwitchTime != nextSwitchInVeins) {
        sendChangeMsg(TrafficLightAtrributeType::SWITCHTIME, std::to_string(nextSwitchInVeins.inUnit(SIMTIME_MS)), std::to_string(nextSwitchTime.inUnit(SIMTIME_MS)));
    }
    nextSwitchTime = nextSwitchInVeins;
}

void TraCITrafficLightInterface::initialize()
{
    ASSERT(isPreInitialized);
    isPreInitialized = false;
    setProgramDefinition(getTlCommandInterface()->getProgramDefinition());
    setControlledLinks(getTlCommandInterface()->getControlledLinks());
    currentLogicId = getTlCommandInterface()->getCurrentProgramID();
    currentPhaseNr = getTlCommandInterface()->getCurrentPhaseIndex();
    nextSwitchTime = getTlCommandInterface()->getAssumedNextSwitchTime();
    currentSignalState = getTlCommandInterface()->getCurrentState();
}

void TraCITrafficLightInterface::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        // not in use (yet)
    }
    else if (msg->arrivedOn("logic$i")) {
        handleChangeCommandMessage(msg);
    }
    delete msg;
}

void TraCITrafficLightInterface::handleChangeCommandMessage(cMessage* msg)
{
    TraCITrafficLightMessage* tmMsg = check_and_cast<TraCITrafficLightMessage*>(msg);
    switch (tmMsg->getChangedAttribute()) {
    case TrafficLightAtrributeType::LOGICID:
        setCurrentLogicById(tmMsg->getNewValue(), true);
        break;

    case TrafficLightAtrributeType::PHASEID:
        setCurrentPhaseByNr(std::stoi(tmMsg->getNewValue()), true);
        break;

    case TrafficLightAtrributeType::SWITCHTIME:
        setNextSwitch(SimTime(std::stoi(tmMsg->getNewValue()), SIMTIME_MS), true);
        break;

    case TrafficLightAtrributeType::STATE:
        setCurrentState(tmMsg->getNewValue(), true);
        break;
    }
}

void TraCITrafficLightInterface::sendChangeMsg(int changedAttribute, const std::string newValue, const std::string oldValue)
{
    Enter_Method_Silent();
    TraCITrafficLightMessage* pMsg = new TraCITrafficLightMessage("TrafficLightChangeMessage");
    pMsg->setTlId(external_id.c_str());
    pMsg->setChangedAttribute(changedAttribute);
    pMsg->setChangeSource(TrafficLightChangeSource::SUMO);
    pMsg->setOldValue(oldValue.c_str());
    pMsg->setNewValue(newValue.c_str());
    send(pMsg, "logic$o");
}
