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
/// @file    NLJunctionControlBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Builder of microsim-junctions and tls
/****************************************************************************/
#include <config.h>

#include <map>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/MSJunctionLogic.h>
#include <microsim/MSNoLogicJunction.h>
#include <microsim/MSRightOfWayJunction.h>
#include <microsim/MSInternalJunction.h>
#include <microsim/MSJunctionControl.h>
#include <microsim/MSEventControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSSimpleTrafficLightLogic.h>
#include <microsim/traffic_lights/MSRailSignal.h>
#include <microsim/traffic_lights/MSRailCrossing.h>
#include <microsim/traffic_lights/MSSOTLPolicyBasedTrafficLightLogic.h>
#include <microsim/traffic_lights/MSSOTLPlatoonPolicy.h>
#include <microsim/traffic_lights/MSSOTLRequestPolicy.h>
#include <microsim/traffic_lights/MSSOTLPhasePolicy.h>
#include <microsim/traffic_lights/MSSOTLMarchingPolicy.h>
#include <microsim/traffic_lights/MSSwarmTrafficLightLogic.h>
#include <microsim/traffic_lights/MSDeterministicHiLevelTrafficLightLogic.h>
#include <microsim/traffic_lights/MSSOTLWaveTrafficLightLogic.h>
#include <microsim/traffic_lights/MSDelayBasedTrafficLightLogic.h>
#include <microsim/traffic_lights/MSOffTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include "NLBuilder.h"
#include "NLJunctionControlBuilder.h"


// ===========================================================================
// static members
// ===========================================================================
const int NLJunctionControlBuilder::NO_REQUEST_SIZE = -1;

// ===========================================================================
// method definitions
// ===========================================================================
NLJunctionControlBuilder::NLJunctionControlBuilder(MSNet& net, NLDetectorBuilder& db) :
    myNet(net),
    myDetectorBuilder(db),
    myOffset(0),
    myJunctions(nullptr),
    myNetIsLoaded(false) {
    myLogicControl = new MSTLLogicControl();
    myJunctions = new MSJunctionControl();
}


NLJunctionControlBuilder::~NLJunctionControlBuilder() {
    delete myLogicControl;
    delete myJunctions;
}


void
NLJunctionControlBuilder::openJunction(const std::string& id,
                                       const std::string& key,
                                       const SumoXMLNodeType type,
                                       const Position pos,
                                       const PositionVector& shape,
                                       const std::vector<MSLane*>& incomingLanes,
                                       const std::vector<MSLane*>& internalLanes,
                                       const std::string& name) {
    myActiveInternalLanes = internalLanes;
    myActiveIncomingLanes = incomingLanes;
    myActiveID = id;
    myActiveKey = key;
    myType = type;
    myPosition.set(pos);
    myShape = shape;
    myActiveName = name;
    myAdditionalParameter.clear();
}


void
NLJunctionControlBuilder::closeJunction(const std::string& basePath) {
    if (myJunctions == nullptr) {
        throw ProcessError("Information about the number of nodes was missing.");
    }
    MSJunction* junction = nullptr;
    switch (myType) {
        case SumoXMLNodeType::NOJUNCTION:
        case SumoXMLNodeType::DEAD_END:
        case SumoXMLNodeType::DEAD_END_DEPRECATED:
        case SumoXMLNodeType::DISTRICT:
        case SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION:
            junction = buildNoLogicJunction();
            break;
        case SumoXMLNodeType::TRAFFIC_LIGHT:
        case SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED:
        case SumoXMLNodeType::RIGHT_BEFORE_LEFT:
        case SumoXMLNodeType::PRIORITY:
        case SumoXMLNodeType::PRIORITY_STOP:
        case SumoXMLNodeType::ALLWAY_STOP:
        case SumoXMLNodeType::ZIPPER:
            junction = buildLogicJunction();
            break;
        case SumoXMLNodeType::INTERNAL:
            if (MSGlobals::gUsingInternalLanes) {
                junction = buildInternalJunction();
            }
            break;
        case SumoXMLNodeType::RAIL_SIGNAL:
        case SumoXMLNodeType::RAIL_CROSSING:
            myOffset = 0;
            myActiveKey = myActiveID;
            myActiveProgram = "0";
            myLogicType = myType == SumoXMLNodeType::RAIL_SIGNAL ? TrafficLightType::RAIL_SIGNAL : TrafficLightType::RAIL_CROSSING;
            closeTrafficLightLogic(basePath);
            junction = buildLogicJunction();
            break;
        default:
            throw InvalidArgument("False junction logic type.");
    }
    if (junction != nullptr) {
        if (!myJunctions->add(myActiveID, junction)) {
            throw InvalidArgument("Another junction with the id '" + myActiveID + "' exists.");
        }
    }
    junction->updateParameters(myAdditionalParameter);
}


MSJunctionControl*
NLJunctionControlBuilder::build() const {
    MSJunctionControl* js = myJunctions;
    myJunctions = nullptr;
    return js;
}


MSJunction*
NLJunctionControlBuilder::buildNoLogicJunction() {
    return new MSNoLogicJunction(myActiveID, myType, myPosition, myShape, myActiveName,
                                 myActiveIncomingLanes, myActiveInternalLanes);
}


MSJunction*
NLJunctionControlBuilder::buildLogicJunction() {
    MSJunctionLogic* jtype = getJunctionLogicSecure();
    // build the junction
    return new MSRightOfWayJunction(myActiveID, myType, myPosition, myShape, myActiveName,
                                    myActiveIncomingLanes,
                                    myActiveInternalLanes,
                                    jtype);
}


MSJunction*
NLJunctionControlBuilder::buildInternalJunction() {
    // build the junction
    return new MSInternalJunction(myActiveID, myType, myPosition, myShape, myActiveIncomingLanes,
                                  myActiveInternalLanes);
}


MSJunctionLogic*
NLJunctionControlBuilder::getJunctionLogicSecure() {
    // get and check the junction logic
    if (myLogics.find(myActiveID) == myLogics.end()) {
        throw InvalidArgument("Missing junction logic '" + myActiveID + "'.");
    }
    return myLogics[myActiveID];
}


MSTLLogicControl::TLSLogicVariants&
NLJunctionControlBuilder::getTLLogic(const std::string& id) const {
    return getTLLogicControlToUse().get(id);
}


void
NLJunctionControlBuilder::closeTrafficLightLogic(const std::string& basePath) {
    if (myActiveProgram == "off") {
        if (myAbsDuration > 0) {
            throw InvalidArgument("The off program for TLS '" + myActiveKey + "' has phases.");
        }
        if (!getTLLogicControlToUse().add(myActiveKey, myActiveProgram,
                                          new MSOffTrafficLightLogic(getTLLogicControlToUse(), myActiveKey))) {
            throw InvalidArgument("Another logic with id '" + myActiveKey + "' and programID '" + myActiveProgram + "' exists.");
        }
        return;
    }
    SUMOTime firstEventOffset = 0;
    int step = 0;
    MSTrafficLightLogic* existing = nullptr;
    MSSimpleTrafficLightLogic::Phases::const_iterator i = myActivePhases.begin();
    if (myLogicType != TrafficLightType::RAIL_SIGNAL && myLogicType != TrafficLightType::RAIL_CROSSING) {
        if (myAbsDuration == 0) {
            existing = getTLLogicControlToUse().get(myActiveKey, myActiveProgram);
            if (existing == nullptr) {
                throw InvalidArgument("TLS program '" + myActiveProgram + "' for TLS '" + myActiveKey + "' has a duration of 0.");
            } else {
                // only modify the offset of an existing logic
                myAbsDuration = existing->getDefaultCycleTime();
                i = existing->getPhases().begin();
            }
        }
        // compute the initial step and first switch time of the tls-logic
        // a positive offset delays all phases by x (advance by absDuration - x) while a negative offset advances all phases by x seconds
        // @note The implementation of % for negative values is implementation defined in ISO1998
        SUMOTime offset; // the time to run the traffic light in advance
        if (myOffset >= 0) {
            offset = (myNet.getCurrentTimeStep() + myAbsDuration - (myOffset % myAbsDuration)) % myAbsDuration;
        } else {
            offset = (myNet.getCurrentTimeStep() + ((-myOffset) % myAbsDuration)) % myAbsDuration;
        }
        while (offset >= (*i)->duration) {
            step++;
            offset -= (*i)->duration;
            ++i;
        }
        firstEventOffset = (*i)->duration - offset + myNet.getCurrentTimeStep();
        if (existing != nullptr) {
            existing->changeStepAndDuration(getTLLogicControlToUse(),
                                            myNet.getCurrentTimeStep(), step, (*i)->duration - offset);
            return;
        }
    }

    if (myActiveProgram == "") {
        myActiveProgram = "default";
    }
    MSTrafficLightLogic* tlLogic = nullptr;
    // build the tls-logic in dependance to its type
    switch (myLogicType) {
        case TrafficLightType::SWARM_BASED:
            firstEventOffset = DELTA_T; //this is needed because swarm needs to update the pheromone on the lanes at every step
            tlLogic = new MSSwarmTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myActivePhases, step, firstEventOffset, myAdditionalParameter);
            break;
        case TrafficLightType::HILVL_DETERMINISTIC:
            tlLogic = new MSDeterministicHiLevelTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myActivePhases, step, firstEventOffset, myAdditionalParameter);
            break;
        case TrafficLightType::SOTL_REQUEST:
            tlLogic = new MSSOTLPolicyBasedTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myLogicType, myActivePhases, step, firstEventOffset, myAdditionalParameter, new MSSOTLRequestPolicy(myAdditionalParameter));
            break;
        case TrafficLightType::SOTL_PLATOON:
            tlLogic = new MSSOTLPolicyBasedTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myLogicType, myActivePhases, step, firstEventOffset, myAdditionalParameter, new MSSOTLPlatoonPolicy(myAdditionalParameter));
            break;
        case TrafficLightType::SOTL_WAVE:
            tlLogic = new MSSOTLWaveTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myActivePhases, step, firstEventOffset, myAdditionalParameter);
            break;
        case TrafficLightType::SOTL_PHASE:
            tlLogic = new MSSOTLPolicyBasedTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myLogicType, myActivePhases, step, firstEventOffset, myAdditionalParameter, new MSSOTLPhasePolicy(myAdditionalParameter));
            break;
        case TrafficLightType::SOTL_MARCHING:
            tlLogic = new MSSOTLPolicyBasedTrafficLightLogic(getTLLogicControlToUse(), myActiveKey, myActiveProgram, myLogicType, myActivePhases, step, firstEventOffset, myAdditionalParameter, new MSSOTLMarchingPolicy(myAdditionalParameter));
            break;
        case TrafficLightType::ACTUATED:
            // @note it is unclear how to apply the given offset in the context
            // of variable-length phases
            tlLogic = new MSActuatedTrafficLightLogic(getTLLogicControlToUse(),
                    myActiveKey, myActiveProgram,
                    myActivePhases, step, (*i)->minDuration + myNet.getCurrentTimeStep(),
                    myAdditionalParameter, basePath);
            break;
        case TrafficLightType::DELAYBASED:
            tlLogic = new MSDelayBasedTrafficLightLogic(getTLLogicControlToUse(),
                    myActiveKey, myActiveProgram,
                    myActivePhases, step, (*i)->minDuration + myNet.getCurrentTimeStep(),
                    myAdditionalParameter, basePath);
            break;
        case TrafficLightType::STATIC:
            tlLogic = new MSSimpleTrafficLightLogic(getTLLogicControlToUse(),
                                                    myActiveKey, myActiveProgram, TrafficLightType::STATIC,
                                                    myActivePhases, step, firstEventOffset,
                                                    myAdditionalParameter);
            break;
        case TrafficLightType::RAIL_SIGNAL:
            tlLogic = new MSRailSignal(getTLLogicControlToUse(),
                                       myActiveKey, myActiveProgram, myNet.getCurrentTimeStep(),
                                       myAdditionalParameter);
            break;
        case TrafficLightType::RAIL_CROSSING:
            tlLogic = new MSRailCrossing(getTLLogicControlToUse(),
                                         myActiveKey, myActiveProgram, myNet.getCurrentTimeStep(),
                                         myAdditionalParameter);
            break;
        case TrafficLightType::OFF:
            tlLogic = new MSOffTrafficLightLogic(getTLLogicControlToUse(), myActiveKey);
            break;
        case TrafficLightType::INVALID:
            throw ProcessError("Invalid traffic light type '" + toString(myLogicType) + "'");
    }
    myActivePhases.clear();
    if (tlLogic != nullptr) {
        if (getTLLogicControlToUse().add(myActiveKey, myActiveProgram, tlLogic)) {
            if (myNetIsLoaded) {
                tlLogic->init(myDetectorBuilder);
            } else {
                myLogics2PostLoadInit.push_back(tlLogic);
            }
        } else {
            WRITE_ERROR("Another logic with id '" + myActiveKey + "' and programID '" + myActiveProgram + "' exists.");
            delete tlLogic;
        }
    }
}


void
NLJunctionControlBuilder::initJunctionLogic(const std::string& id) {
    myActiveKey = id;
    myActiveProgram = "";
    myActiveLogic.clear();
    myActiveFoes.clear();
    myActiveConts.reset();
    myRequestSize = NO_REQUEST_SIZE; // seems not to be used
    myRequestItemNumber = 0;
    myCurrentHasError = false;
}


void
NLJunctionControlBuilder::addLogicItem(int request,
                                       const std::string& response,
                                       const std::string& foes,
                                       bool cont) {
    if (myCurrentHasError) {
        // had an error
        return;
    }
    if (request >= SUMO_MAX_CONNECTIONS) {
        // bad request
        myCurrentHasError = true;
        throw InvalidArgument("Junction logic '" + myActiveKey + "' is larger than allowed; recheck the network.");
    }
    if (myRequestSize == NO_REQUEST_SIZE) {
        // initialize
        myRequestSize = (int)response.size();
    }
    if (static_cast<int>(response.size()) != myRequestSize) {
        myCurrentHasError = true;
        throw InvalidArgument("Invalid response size " + toString(response.size()) +
                              " in Junction logic '" + myActiveKey + "' (expected  " + toString(myRequestSize) + ")");
    }
    if (static_cast<int>(foes.size()) != myRequestSize) {
        myCurrentHasError = true;
        throw InvalidArgument("Invalid foes size " + toString(foes.size()) +
                              " in Junction logic '" + myActiveKey + "' (expected  " + toString(myRequestSize) + ")");
    }
    // assert that the logicitems come ordered by their request index
    assert((int)myActiveLogic.size() == request);
    assert((int)myActiveFoes.size() == request);
    // add the read response for the given request index
    myActiveLogic.push_back(std::bitset<SUMO_MAX_CONNECTIONS>(response));
    // add the read junction-internal foes for the given request index
    myActiveFoes.push_back(std::bitset<SUMO_MAX_CONNECTIONS>(foes));
    // add whether the vehicle may drive a little bit further
    myActiveConts.set(request, cont);
    // increse number of set information
    myRequestItemNumber++;
}


void
NLJunctionControlBuilder::initTrafficLightLogic(const std::string& id, const std::string& programID,
        TrafficLightType type, SUMOTime offset) {
    myActiveKey = id;
    myActiveProgram = programID;
    myActivePhases.clear();
    myAbsDuration = 0;
    myRequestSize = NO_REQUEST_SIZE;
    myLogicType = type;
    myOffset = offset;
    myAdditionalParameter.clear();
}


void
NLJunctionControlBuilder::addPhase(SUMOTime duration, const std::string& state, const std::vector<int>& nextPhases, SUMOTime minDuration, SUMOTime maxDuration, const std::string& name, bool transient_notdecisional, bool commit, MSPhaseDefinition::LaneIdVector* targetLanes) {
    // build and add the phase definition to the list
    myActivePhases.push_back(new MSPhaseDefinition(duration, state, minDuration, maxDuration, nextPhases, name, transient_notdecisional, commit, targetLanes));
    // add phase duration to the absolute duration
    myAbsDuration += duration;
}


void
NLJunctionControlBuilder::addPhase(SUMOTime duration, const std::string& state, const std::vector<int>& nextPhases,
                                   SUMOTime minDuration, SUMOTime maxDuration, const std::string& name) {
    // build and add the phase definition to the list
    myActivePhases.push_back(new MSPhaseDefinition(duration, state, minDuration, maxDuration, nextPhases, name));
    // add phase duration to the absolute duration
    myAbsDuration += duration;
}


void
NLJunctionControlBuilder::closeJunctionLogic() {
    if (myRequestSize == NO_REQUEST_SIZE) {
        // We have a legacy network. junction element did not contain logicitems; read the logic later
        return;
    }
    if (myCurrentHasError) {
        // had an error before...
        return;
    }
    if (myRequestItemNumber != myRequestSize) {
        throw InvalidArgument("The description for the junction logic '" + myActiveKey + "' is malicious.");
    }
    if (myLogics.count(myActiveKey) > 0) {
        throw InvalidArgument("Junction logic '" + myActiveKey + "' was defined twice.");
    }
    MSJunctionLogic* logic = new MSBitsetLogic(myRequestSize,
            new MSBitsetLogic::Logic(myActiveLogic),
            new MSBitsetLogic::Foes(myActiveFoes),
            myActiveConts);
    myLogics[myActiveKey] = logic;
}


MSTLLogicControl*
NLJunctionControlBuilder::buildTLLogics() {
    postLoadInitialization(); // must happen after edgeBuilder is finished
    if (!myLogicControl->closeNetworkReading()) {
        throw ProcessError("Traffic lights could not be built.");
    }
    MSTLLogicControl* ret = myLogicControl;
    myLogicControl = nullptr;
    return ret;
}


void
NLJunctionControlBuilder::addParam(const std::string& key,
                                   const std::string& value) {
    myAdditionalParameter[key] = value;
}


MSTLLogicControl&
NLJunctionControlBuilder::getTLLogicControlToUse() const {
    if (myLogicControl != nullptr) {
        return *myLogicControl;
    }
    return myNet.getTLSControl();
}


const std::string&
NLJunctionControlBuilder::getActiveKey() const {
    return myActiveKey;
}


const std::string&
NLJunctionControlBuilder::getActiveSubKey() const {
    return myActiveProgram;
}


void
NLJunctionControlBuilder::postLoadInitialization() {
    for (MSTrafficLightLogic* const logic : myLogics2PostLoadInit) {
        logic->init(myDetectorBuilder);
    }
    myNetIsLoaded = true;
}


MSJunction*
NLJunctionControlBuilder::retrieve(const std::string id) {
    if (myJunctions != nullptr) {
        return myJunctions->get(id);
    } else {
        return nullptr;
    }
}


/****************************************************************************/
