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

#pragma once

#include "veins/veins.h"

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightProgram.h"

namespace veins {

class VEINS_API TraCITrafficLightInterface : public cSimpleModule {
public:
    TraCITrafficLightInterface();
    ~TraCITrafficLightInterface() override;

    /** Set parameters for connection to TraCI */
    virtual void preInitialize(const std::string& external_id, const Coord& position, const simtime_t& updateInterval);

    virtual void setExternalId(const std::string& external_id)
    {
        this->external_id = external_id;
    }
    virtual std::string getExternalId() const
    {
        if (external_id == "") throw cRuntimeError("TraCITrafficLightInterface::getExternalId called with no external_id set yet");
        return external_id;
    }
    virtual TraCIScenarioManager* getManager() const
    {
        if (!manager) {
            manager = TraCIScenarioManagerAccess().get();
        }
        return manager;
    }
    virtual TraCICommandInterface* getCommandInterface() const
    {
        if (!commandInterface) {
            commandInterface = getManager()->getCommandInterface();
        }
        return commandInterface;
    }
    virtual TraCICommandInterface::Trafficlight* getTlCommandInterface() const
    {
        if (!tlCommandInterface) {
            tlCommandInterface = new TraCICommandInterface::Trafficlight(getCommandInterface(), external_id);
        }
        return tlCommandInterface;
    }

    virtual std::list<std::list<TraCITrafficLightLink>> getControlledLinks();
    virtual Coord getPosition() const;
    virtual TraCITrafficLightProgram::Logic getCurrentLogic() const;
    virtual std::string getCurrentLogicId() const;
    virtual int getCurrentPhaseId() const;
    virtual TraCITrafficLightProgram::Phase getCurrentPhase() const;
    virtual simtime_t getAssumedNextSwitch() const;
    virtual simtime_t getRemainingDuration() const;
    virtual std::string getCurrentState() const;
    virtual bool isInOnlineSignalState() const;

    virtual void setProgramDefinition(const TraCITrafficLightProgram& programDefinition);
    virtual void setControlledLinks(const std::list<std::list<TraCITrafficLightLink>>& controlledLinks);
    virtual void setCurrentLogicById(const std::string& logicId, bool setSumo = true);
    virtual void setCurrentPhaseByNr(const unsigned int phaseNr, bool setSumo = true);
    virtual void setCurrentState(const std::string& state, bool setSumo = true);
    virtual void setNextSwitch(const simtime_t& newNextSwitch, bool setSumo = true);
    virtual void setRemainingDuration(const simtime_t& timeTillSwitch, bool setSumo = true);

protected:
    void initialize() override;
    void handleMessage(cMessage* msg) override;
    virtual void handleChangeCommandMessage(cMessage* msg);
    virtual void sendChangeMsg(int changedAttribute, const std::string newValue, const std::string oldValue);

    bool isPreInitialized; /**< true if preInitialize() has been called immediately before initialize() */
    simtime_t updateInterval; /**< ScenarioManager's update interval */
    /** reference to the simulations ScenarioManager */
    mutable TraCIScenarioManager* manager;
    /** reference to the simulations traffic light-specific TraCI command interface */
    mutable TraCICommandInterface* commandInterface;
    /** reference to the simulations traffic light-specific TraCI command interface */
    mutable TraCICommandInterface::Trafficlight* tlCommandInterface;

    std::string external_id; /**< id used on the other end of TraCI */
    Coord position; /**< position of the traffic light */

    TraCITrafficLightProgram programDefinition; /**< full definition of program (all logics) */
    std::list<std::list<TraCITrafficLightLink>> controlledLinks; /**< controlledLinks[signal][link] */
    // std::list< std::list<TraCITrafficLightLink> > controlledLanes; /**< controlledLanes[signal][link] */
    std::string currentLogicId; /**< id of the currently active logic */
    int currentPhaseNr; /**< current phase of the current program */
    simtime_t nextSwitchTime; /**< predicted next phase switch time (absolute timestamp) */
    std::string currentSignalState; /**< current state of the signals (rRgGyY-String) */
    bool inOnlineSignalState; /**< whether the TLS is currently set to a manual (i.e. online) phase state */
};

} // namespace veins

namespace veins {
class VEINS_API TraCITrafficLightInterfaceAccess {
public:
    TraCITrafficLightInterface* get(cModule* host)
    {
        TraCITrafficLightInterface* traci = FindModule<TraCITrafficLightInterface*>::findSubModule(host);
        ASSERT(traci);
        return traci;
    };
};
} // namespace veins
