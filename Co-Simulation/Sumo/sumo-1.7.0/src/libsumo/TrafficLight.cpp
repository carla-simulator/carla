/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    TrafficLight.cpp
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    30.05.2012
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSNet.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <microsim/traffic_lights/MSActuatedTrafficLightLogic.h>
#include <microsim/traffic_lights/MSDelayBasedTrafficLightLogic.h>
#include <netload/NLDetectorBuilder.h>
#include <libsumo/TraCIConstants.h>
#include "TrafficLight.h"


namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults TrafficLight::mySubscriptionResults;
ContextSubscriptionResults TrafficLight::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
std::vector<std::string>
TrafficLight::getIDList() {
    return MSNet::getInstance()->getTLSControl().getAllTLIds();
}


int
TrafficLight::getIDCount() {
    return (int)getIDList().size();
}


std::string
TrafficLight::getRedYellowGreenState(const std::string& tlsID) {
    return getTLS(tlsID).getActive()->getCurrentPhaseDef().getState();
}


std::vector<TraCILogic>
TrafficLight::getAllProgramLogics(const std::string& tlsID) {
    std::vector<TraCILogic> result;
    const std::vector<MSTrafficLightLogic*> logics = getTLS(tlsID).getAllLogics();
    for (MSTrafficLightLogic* logic : logics) {
        TraCILogic l(logic->getProgramID(), (int)logic->getLogicType(), logic->getCurrentPhaseIndex());
        l.subParameter = logic->getParametersMap();
        for (const MSPhaseDefinition* const phase : logic->getPhases()) {
            l.phases.emplace_back(new TraCIPhase(STEPS2TIME(phase->duration), phase->getState(),
                                                 STEPS2TIME(phase->minDuration), STEPS2TIME(phase->maxDuration),
                                                 phase->getNextPhases(), phase->getName()));
        }
        result.emplace_back(l);
    }
    return result;
}


std::vector<std::string>
TrafficLight::getControlledJunctions(const std::string& tlsID) {
    std::set<std::string> junctionIDs;
    const MSTrafficLightLogic::LinkVectorVector& links = getTLS(tlsID).getActive()->getLinks();
    for (const MSTrafficLightLogic::LinkVector& llinks : links) {
        for (const MSLink* l : llinks) {
            junctionIDs.insert(l->getJunction()->getID());
        }
    }
    return std::vector<std::string>(junctionIDs.begin(), junctionIDs.end());
}


std::vector<std::string>
TrafficLight::getControlledLanes(const std::string& tlsID) {
    std::vector<std::string> laneIDs;
    const MSTrafficLightLogic::LaneVectorVector& lanes = getTLS(tlsID).getActive()->getLaneVectors();
    for (const MSTrafficLightLogic::LaneVector& llanes : lanes) {
        for (const MSLane* l : llanes) {
            laneIDs.push_back(l->getID());
        }
    }
    return laneIDs;
}


std::vector<std::vector<TraCILink> >
TrafficLight::getControlledLinks(const std::string& tlsID) {
    std::vector<std::vector<TraCILink> > result;
    const MSTrafficLightLogic::LaneVectorVector& lanes = getTLS(tlsID).getActive()->getLaneVectors();
    const MSTrafficLightLogic::LinkVectorVector& links = getTLS(tlsID).getActive()->getLinks();
    for (int i = 0; i < (int)lanes.size(); ++i) {
        std::vector<TraCILink> subList;
        const MSTrafficLightLogic::LaneVector& llanes = lanes[i];
        const MSTrafficLightLogic::LinkVector& llinks = links[i];
        // number of links controlled by this signal (signal i)
        for (int j = 0; j < (int)llanes.size(); ++j) {
            MSLink* link = llinks[j];
            // approached non-internal lane (if any)
            const std::string to = link->getLane() != nullptr ? link->getLane()->getID() : "";
            // approached "via", internal lane (if any)
            const std::string via = link->getViaLane() != nullptr ? link->getViaLane()->getID() : "";
            subList.emplace_back(TraCILink(llanes[j]->getID(), via, to));
        }
        result.emplace_back(subList);
    }
    return result;
}


std::string
TrafficLight::getProgram(const std::string& tlsID) {
    return getTLS(tlsID).getActive()->getProgramID();
}


int
TrafficLight::getPhase(const std::string& tlsID) {
    return getTLS(tlsID).getActive()->getCurrentPhaseIndex();
}


std::string
TrafficLight::getPhaseName(const std::string& tlsID) {
    return getTLS(tlsID).getActive()->getCurrentPhaseDef().getName();
}


double
TrafficLight::getPhaseDuration(const std::string& tlsID) {
    return STEPS2TIME(getTLS(tlsID).getActive()->getCurrentPhaseDef().duration);
}


double
TrafficLight::getNextSwitch(const std::string& tlsID) {
    return STEPS2TIME(getTLS(tlsID).getActive()->getNextSwitchTime());
}

int
TrafficLight::getServedPersonCount(const std::string& tlsID, int index) {
    MSTrafficLightLogic* const active = getTLS(tlsID).getActive();
    if (index < 0 || active->getPhaseNumber() <= index) {
        throw TraCIException("The phase index " + toString(index) + " is not in the allowed range [0,"
                             + toString(active->getPhaseNumber() - 1) + "].");
    }
    // find all crossings which have a green light in that phas
    int result = 0;

    const std::string& state = active->getPhases()[index]->getState();
    for (int i = 0; i < (int)state.size(); i++) {
        for (MSLink* link : active->getLinksAt(i)) {
            if (link->getLane()->getEdge().isCrossing()) {
                // walking forwards across
                for (MSTransportable* person : link->getLaneBefore()->getEdge().getPersons()) {
                    if (static_cast<MSPerson*>(person)->getNextEdge() == link->getLane()->getEdge().getID()) {
                        result += 1;
                    }
                }
                // walking backwards across
                MSLane* walkingAreaAcross = link->getLane()->getLinkCont().front()->getLane();
                for (MSTransportable* person : walkingAreaAcross->getEdge().getPersons()) {
                    if (static_cast<MSPerson*>(person)->getNextEdge() == link->getLane()->getEdge().getID()) {
                        result += 1;
                    }
                }
            } else if (link->getLaneBefore()->getEdge().isCrossing()) {
                // walking backwards across (in case both sides are separately controlled)
                for (MSTransportable* person : link->getLane()->getEdge().getPersons()) {
                    if (static_cast<MSPerson*>(person)->getNextEdge() == link->getLaneBefore()->getEdge().getID()) {
                        result += 1;
                    }
                }
            }
        }
    }
    return result;
}

std::vector<std::string>
TrafficLight::getBlockingVehicles(const std::string& tlsID, int linkIndex) {
    std::vector<std::string> result;
    // for railsignals we cannot use the "online" program
    MSTrafficLightLogic* const active = getTLS(tlsID).getDefault();
    if (linkIndex < 0 || linkIndex >= active->getNumLinks()) {
        throw TraCIException("The link index " + toString(linkIndex) + " is not in the allowed range [0,"
                             + toString(active->getNumLinks() - 1) + "].");
    }
    for (const SUMOVehicle* veh : active->getBlockingVehicles(linkIndex)) {
        result.push_back(veh->getID());
    }
    return result;
}

std::vector<std::string>
TrafficLight::getRivalVehicles(const std::string& tlsID, int linkIndex) {
    std::vector<std::string> result;
    MSTrafficLightLogic* const active = getTLS(tlsID).getDefault();
    if (linkIndex < 0 || linkIndex >= active->getNumLinks()) {
        throw TraCIException("The link index " + toString(linkIndex) + " is not in the allowed range [0,"
                             + toString(active->getNumLinks() - 1) + "].");
    }
    for (const SUMOVehicle* veh : active->getRivalVehicles(linkIndex)) {
        result.push_back(veh->getID());
    }
    return result;
}

std::vector<std::string>
TrafficLight::getPriorityVehicles(const std::string& tlsID, int linkIndex) {
    std::vector<std::string> result;
    MSTrafficLightLogic* const active = getTLS(tlsID).getDefault();
    if (linkIndex < 0 || linkIndex >= active->getNumLinks()) {
        throw TraCIException("The link index " + toString(linkIndex) + " is not in the allowed range [0,"
                             + toString(active->getNumLinks() - 1) + "].");
    }
    for (const SUMOVehicle* veh : active->getPriorityVehicles(linkIndex)) {
        result.push_back(veh->getID());
    }
    return result;
}

std::string
TrafficLight::getParameter(const std::string& tlsID, const std::string& paramName) {
    return getTLS(tlsID).getActive()->getParameter(paramName, "");
}


LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(TrafficLight)


void
TrafficLight::setRedYellowGreenState(const std::string& tlsID, const std::string& state) {
    getTLS(tlsID).setStateInstantiatingOnline(MSNet::getInstance()->getTLSControl(), state);
}


void
TrafficLight::setPhase(const std::string& tlsID, const int index) {
    MSTrafficLightLogic* const active = getTLS(tlsID).getActive();
    if (index < 0 || active->getPhaseNumber() <= index) {
        throw TraCIException("The phase index " + toString(index) + " is not in the allowed range [0,"
                             + toString(active->getPhaseNumber() - 1) + "].");
    }
    const SUMOTime cTime = MSNet::getInstance()->getCurrentTimeStep();
    const SUMOTime duration = active->getPhase(index).duration;
    active->changeStepAndDuration(MSNet::getInstance()->getTLSControl(), cTime, index, duration);
}

void
TrafficLight::setPhaseName(const std::string& tlsID, const std::string& name) {
    MSTrafficLightLogic* const active = getTLS(tlsID).getActive();
    const_cast<MSPhaseDefinition&>(active->getCurrentPhaseDef()).setName(name);
}


void
TrafficLight::setProgram(const std::string& tlsID, const std::string& programID) {
    try {
        getTLS(tlsID).switchTo(MSNet::getInstance()->getTLSControl(), programID);
    } catch (ProcessError& e) {
        throw TraCIException(e.what());
    }
}


void
TrafficLight::setPhaseDuration(const std::string& tlsID, const double phaseDuration) {
    MSTrafficLightLogic* const active = getTLS(tlsID).getActive();
    const SUMOTime cTime = MSNet::getInstance()->getCurrentTimeStep();
    const int index = active->getCurrentPhaseIndex();
    active->changeStepAndDuration(MSNet::getInstance()->getTLSControl(), cTime, index, TIME2STEPS(phaseDuration));
}


void
TrafficLight::setProgramLogic(const std::string& tlsID, const TraCILogic& logic) {
    MSTLLogicControl::TLSLogicVariants& vars = getTLS(tlsID);
    // make sure index and phaseNo are consistent
    if (logic.currentPhaseIndex >= (int)logic.phases.size()) {
        throw TraCIException("set program: parameter index must be less than parameter phase number.");
    }
    std::vector<MSPhaseDefinition*> phases;
    for (TraCIPhase* phase : logic.phases) {
        phases.push_back(new MSPhaseDefinition(TIME2STEPS(phase->duration), phase->state, TIME2STEPS(phase->minDur), TIME2STEPS(phase->maxDur), phase->next, phase->name));
    }
    if (vars.getLogic(logic.programID) == nullptr) {
        MSTLLogicControl& tlc = MSNet::getInstance()->getTLSControl();
        int step = logic.currentPhaseIndex;
        const std::string basePath = "";
        MSTrafficLightLogic* tlLogic = nullptr;
        SUMOTime nextSwitch = 0; //MSNet::getInstance()->getCurrentTimeStep();
        switch ((TrafficLightType)logic.type) {
            case TrafficLightType::ACTUATED:
                tlLogic = new MSActuatedTrafficLightLogic(tlc,
                        tlsID, logic.programID,
                        phases, step, nextSwitch,
                        logic.subParameter, basePath);
                break;
            case TrafficLightType::DELAYBASED:
                tlLogic = new MSDelayBasedTrafficLightLogic(tlc,
                        tlsID, logic.programID,
                        phases, step, nextSwitch,
                        logic.subParameter, basePath);
                break;
            case TrafficLightType::STATIC:
                tlLogic = new MSSimpleTrafficLightLogic(tlc,
                                                        tlsID, logic.programID, TrafficLightType::STATIC,
                                                        phases, step, nextSwitch,
                                                        logic.subParameter);
                break;
            default:
                throw TraCIException("Unsupported traffic light type '" + toString(logic.type) + "'");
        }
        vars.addLogic(logic.programID, tlLogic, true, true);
        // XXX pass GUIDetectorBuilder when running with gui
        NLDetectorBuilder db(*MSNet::getInstance());
        tlLogic->init(db);
    } else {
        static_cast<MSSimpleTrafficLightLogic*>(vars.getLogic(logic.programID))->setPhases(phases, logic.currentPhaseIndex);
    }
}


void
TrafficLight::setParameter(const std::string& tlsID, const std::string& paramName, const std::string& value) {
    return getTLS(tlsID).getActive()->setParameter(paramName, value);
}


LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(TrafficLight, TL)


MSTLLogicControl::TLSLogicVariants&
TrafficLight::getTLS(const std::string& id) {
    if (!MSNet::getInstance()->getTLSControl().knows(id)) {
        throw TraCIException("Traffic light '" + id + "' is not known");
    }
    return MSNet::getInstance()->getTLSControl().get(id);
}


std::shared_ptr<VariableWrapper>
TrafficLight::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
TrafficLight::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case TL_RED_YELLOW_GREEN_STATE:
            return wrapper->wrapString(objID, variable, getRedYellowGreenState(objID));
        case TL_CONTROLLED_LANES:
            return wrapper->wrapStringList(objID, variable, getControlledLanes(objID));
        case TL_CURRENT_PHASE:
            return wrapper->wrapInt(objID, variable, getPhase(objID));
        case VAR_NAME:
            return wrapper->wrapString(objID, variable, getPhaseName(objID));
        case TL_CURRENT_PROGRAM:
            return wrapper->wrapString(objID, variable, getProgram(objID));
        case TL_PHASE_DURATION:
            return wrapper->wrapDouble(objID, variable, getPhaseDuration(objID));
        case TL_NEXT_SWITCH:
            return wrapper->wrapDouble(objID, variable, getNextSwitch(objID));
        case TL_CONTROLLED_JUNCTIONS:
            return wrapper->wrapStringList(objID, variable, getControlledJunctions(objID));
        default:
            return false;
    }
}
}


/****************************************************************************/
