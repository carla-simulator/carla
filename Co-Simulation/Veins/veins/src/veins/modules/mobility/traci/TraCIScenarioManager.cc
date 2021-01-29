//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
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

#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iterator>
#include <cstdlib>

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/world/traci/trafficLight/TraCITrafficLightInterface.h"

using namespace veins::TraCIConstants;

using veins::AnnotationManagerAccess;
using veins::TraCIBuffer;
using veins::TraCICoord;
using veins::TraCIScenarioManager;
using veins::TraCITrafficLightInterface;

Define_Module(veins::TraCIScenarioManager);

const simsignal_t TraCIScenarioManager::traciInitializedSignal = registerSignal("org_car2x_veins_modules_mobility_traciInitialized");
const simsignal_t TraCIScenarioManager::traciModuleAddedSignal = registerSignal("org_car2x_veins_modules_mobility_traciModuleAdded");
const simsignal_t TraCIScenarioManager::traciModuleRemovedSignal = registerSignal("org_car2x_veins_modules_mobility_traciModuleRemoved");
const simsignal_t TraCIScenarioManager::traciTimestepBeginSignal = registerSignal("org_car2x_veins_modules_mobility_traciTimestepBegin");
const simsignal_t TraCIScenarioManager::traciTimestepEndSignal = registerSignal("org_car2x_veins_modules_mobility_traciTimestepEnd");

TraCIScenarioManager::TraCIScenarioManager()
    : connection(nullptr)
    , commandIfc(nullptr)
    , connectAndStartTrigger(nullptr)
    , executeOneTimestepTrigger(nullptr)
    , world(nullptr)
{
}

TraCIScenarioManager::~TraCIScenarioManager()
{
    if (connection) {
        TraCIBuffer buf = connection->query(CMD_CLOSE, TraCIBuffer());
    }
    cancelAndDelete(connectAndStartTrigger);
    cancelAndDelete(executeOneTimestepTrigger);
}

namespace {

std::vector<std::string> getMapping(std::string el)
{

    // search for string protection characters '
    char protection = '\'';
    size_t first = el.find(protection);
    size_t second;
    size_t eq;
    std::string type, value;
    std::vector<std::string> mapping;

    if (first == std::string::npos) {
        // there's no string protection, simply split by '='
        cStringTokenizer stk(el.c_str(), "=");
        mapping = stk.asVector();
    }
    else {
        // if there's string protection, we need to find a matching delimiter
        second = el.find(protection, first + 1);
        // ensure that a matching delimiter exists, and that it is at the end
        if (second == std::string::npos || second != el.size() - 1) throw cRuntimeError("invalid syntax for mapping \"%s\"", el.c_str());

        // take the value of the mapping as the text within the quotes
        value = el.substr(first + 1, second - first - 1);

        if (first == 0) {
            // if the string starts with a quote, there's only the value
            mapping.push_back(value);
        }
        else {
            // search for the equal sign
            eq = el.find('=');
            // this must be the character before the quote
            if (eq == std::string::npos || eq != first - 1) {
                throw cRuntimeError("invalid syntax for mapping \"%s\"", el.c_str());
            }
            else {
                type = el.substr(0, eq);
            }
            mapping.push_back(type);
            mapping.push_back(value);
        }
    }
    return mapping;
}

} // namespace

TraCIScenarioManager::TypeMapping TraCIScenarioManager::parseMappings(std::string parameter, std::string parameterName, bool allowEmpty)
{

    /**
     * possible syntaxes
     *
     * "a"          : assign module type "a" to all nodes (for backward compatibility)
     * "a=b"        : assign module type "b" to vehicle type "a". the presence of any other vehicle type in the simulation will cause the simulation to stop
     * "a=b c=d"    : assign module type "b" to vehicle type "a" and "d" to "c". the presence of any other vehicle type in the simulation will cause the simulation to stop
     * "a=b c=d *=e": everything which is not of vehicle type "a" or "b", assign module type "e"
     * "a=b c=0"    : for vehicle type "c" no module should be instantiated
     * "a=b c=d *=0": everything which is not of vehicle type a or c should not be instantiated
     *
     * For display strings key-value pairs needs to be protected with single quotes, as they use an = sign as the type mappings. For example
     * *.manager.moduleDisplayString = "'i=block/process'"
     * *.manager.moduleDisplayString = "a='i=block/process' b='i=misc/sun'"
     *
     * moduleDisplayString can also be left empty:
     * *.manager.moduleDisplayString = ""
     */

    unsigned int i;
    TypeMapping map;

    // tokenizer to split into mappings ("a=b c=d", -> ["a=b", "c=d"])
    cStringTokenizer typesTz(parameter.c_str(), " ");
    // get all mappings
    std::vector<std::string> typeMappings = typesTz.asVector();
    // and check that there exists at least one
    if (typeMappings.size() == 0) {
        if (!allowEmpty)
            throw cRuntimeError("parameter \"%s\" is empty", parameterName.c_str());
        else
            return map;
    }

    // loop through all mappings
    for (i = 0; i < typeMappings.size(); i++) {

        // tokenizer to find the mapping from vehicle type to module type
        std::string typeMapping = typeMappings[i];

        std::vector<std::string> mapping = getMapping(typeMapping);

        if (mapping.size() == 1) {
            // we are where there is no actual assignment
            // "a": this is good
            // "a b=c": this is not
            if (typeMappings.size() != 1)
                // stop simulation with an error
                throw cRuntimeError("parameter \"%s\" includes multiple mappings, but \"%s\" is not mapped to any vehicle type", parameterName.c_str(), mapping[0].c_str());
            else
                // all vehicle types should be instantiated with this module type
                map["*"] = mapping[0];
        }
        else {

            // check that mapping is valid (a=b and not like a=b=c)
            if (mapping.size() != 2) throw cRuntimeError("invalid syntax for mapping \"%s\" for parameter \"%s\"", typeMapping.c_str(), parameterName.c_str());
            // check that the mapping does not already exist
            if (map.find(mapping[0]) != map.end()) throw cRuntimeError("duplicated mapping for vehicle type \"%s\" for parameter \"%s\"", mapping[0].c_str(), parameterName.c_str());

            // finally save the mapping
            map[mapping[0]] = mapping[1];
        }
    }

    return map;
}

void TraCIScenarioManager::parseModuleTypes()
{

    TypeMapping::iterator i;
    std::vector<std::string> typeKeys, nameKeys, displayStringKeys;

    std::string moduleTypes = par("moduleType").stdstringValue();
    std::string moduleNames = par("moduleName").stdstringValue();
    std::string moduleDisplayStrings = par("moduleDisplayString").stdstringValue();

    moduleType = parseMappings(moduleTypes, "moduleType", false);
    moduleName = parseMappings(moduleNames, "moduleName", false);
    moduleDisplayString = parseMappings(moduleDisplayStrings, "moduleDisplayString", true);

    // perform consistency check. for each vehicle type in moduleType there must be a vehicle type
    // in moduleName (and in moduleDisplayString if moduleDisplayString is not empty)

    // get all the keys
    for (i = moduleType.begin(); i != moduleType.end(); i++) typeKeys.push_back(i->first);
    for (i = moduleName.begin(); i != moduleName.end(); i++) nameKeys.push_back(i->first);
    for (i = moduleDisplayString.begin(); i != moduleDisplayString.end(); i++) displayStringKeys.push_back(i->first);

    // sort them (needed for intersection)
    std::sort(typeKeys.begin(), typeKeys.end());
    std::sort(nameKeys.begin(), nameKeys.end());
    std::sort(displayStringKeys.begin(), displayStringKeys.end());

    std::vector<std::string> intersection;

    // perform set intersection
    std::set_intersection(typeKeys.begin(), typeKeys.end(), nameKeys.begin(), nameKeys.end(), std::back_inserter(intersection));
    if (intersection.size() != typeKeys.size() || intersection.size() != nameKeys.size()) throw cRuntimeError("keys of mappings of moduleType and moduleName are not the same");

    if (displayStringKeys.size() == 0) return;

    intersection.clear();
    std::set_intersection(typeKeys.begin(), typeKeys.end(), displayStringKeys.begin(), displayStringKeys.end(), std::back_inserter(intersection));
    if (intersection.size() != displayStringKeys.size()) throw cRuntimeError("keys of mappings of moduleType and moduleName are not the same");
}

void TraCIScenarioManager::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != 1) {
        return;
    }

    trafficLightModuleType = par("trafficLightModuleType").stdstringValue();
    trafficLightModuleName = par("trafficLightModuleName").stdstringValue();
    trafficLightModuleDisplayString = par("trafficLightModuleDisplayString").stdstringValue();
    trafficLightModuleIds.clear();
    std::istringstream filterstream(par("trafficLightFilter").stdstringValue());
    std::copy(std::istream_iterator<std::string>(filterstream), std::istream_iterator<std::string>(), std::back_inserter(trafficLightModuleIds));

    connectAt = par("connectAt");
    firstStepAt = par("firstStepAt");
    updateInterval = par("updateInterval");
    if (firstStepAt == -1) firstStepAt = connectAt + updateInterval;
    parseModuleTypes();
    penetrationRate = par("penetrationRate").doubleValue();
    ignoreGuiCommands = par("ignoreGuiCommands");
    host = par("host").stdstringValue();
    port = getPortNumber();
    if (port == -1) {
        throw cRuntimeError("TraCI Port autoconfiguration failed, set 'port' != -1 in omnetpp.ini or provide VEINS_TRACI_PORT environment variable.");
    }
    autoShutdown = par("autoShutdown");

    annotations = AnnotationManagerAccess().getIfExists();

    roi.clear();
    roi.addRoads(par("roiRoads"));
    roi.addRectangles(par("roiRects"));

    areaSum = 0;
    nextNodeVectorIndex = 0;
    hosts.clear();
    subscribedVehicles.clear();
    trafficLights.clear();
    activeVehicleCount = 0;
    parkingVehicleCount = 0;
    drivingVehicleCount = 0;
    autoShutdownTriggered = false;

    world = FindModule<BaseWorldUtility*>::findGlobalModule();

    vehicleObstacleControl = FindModule<VehicleObstacleControl*>::findGlobalModule();

    ASSERT(firstStepAt > connectAt);
    connectAndStartTrigger = new cMessage("connect");
    scheduleAt(connectAt, connectAndStartTrigger);
    executeOneTimestepTrigger = new cMessage("step");
    scheduleAt(firstStepAt, executeOneTimestepTrigger);

    EV_DEBUG << "initialized TraCIScenarioManager" << endl;
}

void TraCIScenarioManager::init_traci()
{
    auto* commandInterface = getCommandInterface();
    // int order = 2;
    // TraCIBuffer buf = connection->query(CMD_SETORDER, TraCIBuffer() << 2);
    // processSubcriptionResult(buf);
    // ASSERT(buf.eof());

    {
        auto apiVersion = commandInterface->getVersion();
        EV_DEBUG << "TraCI server \"" << apiVersion.second << "\" reports API version " << apiVersion.first << endl;
        commandInterface->setApiVersion(apiVersion.first);
    }

    {
        // query and set road network boundaries
        auto networkBoundaries = commandInterface->initNetworkBoundaries(par("margin"));
        if (world != nullptr && ((connection->traci2omnet(networkBoundaries.second).x > world->getPgs()->x) || (connection->traci2omnet(networkBoundaries.first).y > world->getPgs()->y))) {
            EV_DEBUG << "WARNING: Playground size (" << world->getPgs()->x << ", " << world->getPgs()->y << ") might be too small for vehicle at network bounds (" << connection->traci2omnet(networkBoundaries.second).x << ", " << connection->traci2omnet(networkBoundaries.first).y << ")" << endl;
        }
    }

    {
        // subscribe to list of departed and arrived vehicles, as well as simulation time
        simtime_t beginTime = 0;
        simtime_t endTime = SimTime::getMaxTime();
        std::string objectId = "";
        uint8_t variableNumber = 7;
        uint8_t variable1 = VAR_DEPARTED_VEHICLES_IDS;
        uint8_t variable2 = VAR_ARRIVED_VEHICLES_IDS;
        uint8_t variable3 = commandInterface->getTimeStepCmd();
        uint8_t variable4 = VAR_TELEPORT_STARTING_VEHICLES_IDS;
        uint8_t variable5 = VAR_TELEPORT_ENDING_VEHICLES_IDS;
        uint8_t variable6 = VAR_PARKING_STARTING_VEHICLES_IDS;
        uint8_t variable7 = VAR_PARKING_ENDING_VEHICLES_IDS;
        TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_SIM_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5 << variable6 << variable7);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    {
        // subscribe to list of vehicle ids
        simtime_t beginTime = 0;
        simtime_t endTime = SimTime::getMaxTime();
        std::string objectId = "";
        uint8_t variableNumber = 1;
        uint8_t variable1 = ID_LIST;
        TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1);
        processSubcriptionResult(buf);
        ASSERT(buf.eof());
    }

    if (!trafficLightModuleType.empty() && !trafficLightModuleIds.empty()) {
        // initialize traffic lights
        cModule* parentmod = getParentModule();
        if (!parentmod) {
            throw cRuntimeError("Parent Module not found (for traffic light creation)");
        }
        cModuleType* tlModuleType = cModuleType::get(trafficLightModuleType.c_str());

        // query traffic lights via TraCI
        std::list<std::string> trafficLightIds = commandInterface->getTrafficlightIds();
        size_t nrOfTrafficLights = trafficLightIds.size();
        int cnt = 0;
        for (std::list<std::string>::iterator i = trafficLightIds.begin(); i != trafficLightIds.end(); ++i) {
            std::string tlId = *i;
            if (std::find(trafficLightModuleIds.begin(), trafficLightModuleIds.end(), tlId) == trafficLightModuleIds.end()) {
                continue; // filter only selected elements
            }

            Coord position = commandInterface->junction(tlId).getPosition();

            cModule* module = tlModuleType->create(trafficLightModuleName.c_str(), parentmod, nrOfTrafficLights, cnt);
            module->par("externalId") = tlId;
            module->finalizeParameters();
            module->getDisplayString().parse(trafficLightModuleDisplayString.c_str());
            module->buildInside();
            module->scheduleStart(simTime() + updateInterval);

            cModule* tlIfSubmodule = module->getSubmodule("tlInterface");
            // initialize traffic light interface with current program
            TraCITrafficLightInterface* tlIfModule = dynamic_cast<TraCITrafficLightInterface*>(tlIfSubmodule);
            tlIfModule->preInitialize(tlId, position, updateInterval);

            // initialize mobility for positioning
            cModule* mobiSubmodule = module->getSubmodule("mobility");
            mobiSubmodule->par("x") = position.x;
            mobiSubmodule->par("y") = position.y;
            mobiSubmodule->par("z") = position.z;

            module->callInitialize();
            trafficLights[tlId] = module;
            subscribeToTrafficLightVariables(tlId); // subscribe after module is in trafficLights
            cnt++;
        }
    }

    ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
    if (obstacles) {
        {
            // get list of polygons
            std::list<std::string> ids = commandInterface->getPolygonIds();
            for (std::list<std::string>::iterator i = ids.begin(); i != ids.end(); ++i) {
                std::string id = *i;
                std::string typeId = commandInterface->polygon(id).getTypeId();
                if (!obstacles->isTypeSupported(typeId)) continue;
                std::list<Coord> coords = commandInterface->polygon(id).getShape();
                std::vector<Coord> shape;
                std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
                obstacles->addFromTypeAndShape(id, typeId, shape);
            }
        }
    }

    traciInitialized = true;
    emit(traciInitializedSignal, true);

    // draw and calculate area of rois
    for (std::list<std::pair<TraCICoord, TraCICoord>>::const_iterator r = roi.getRectangles().begin(), end = roi.getRectangles().end(); r != end; ++r) {
        TraCICoord first = r->first;
        TraCICoord second = r->second;

        std::list<Coord> pol;

        Coord a = connection->traci2omnet(first);
        Coord b = connection->traci2omnet(TraCICoord(first.x, second.y));
        Coord c = connection->traci2omnet(second);
        Coord d = connection->traci2omnet(TraCICoord(second.x, first.y));

        pol.push_back(a);
        pol.push_back(b);
        pol.push_back(c);
        pol.push_back(d);

        // draw polygon for region of interest
        if (annotations) {
            annotations->drawPolygon(pol, "black");
        }

        // calculate region area
        double ab = a.distance(b);
        double ad = a.distance(d);
        double area = ab * ad;
        areaSum += area;
    }
}

void TraCIScenarioManager::finish()
{
    while (hosts.begin() != hosts.end()) {
        deleteManagedModule(hosts.begin()->first);
    }

    recordScalar("roiArea", areaSum);
}

void TraCIScenarioManager::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
        return;
    }
    throw cRuntimeError("TraCIScenarioManager doesn't handle messages from other modules");
}

void TraCIScenarioManager::handleSelfMsg(cMessage* msg)
{
    if (msg == connectAndStartTrigger) {
        connection.reset(TraCIConnection::connect(this, host.c_str(), port));
        commandIfc.reset(new TraCICommandInterface(this, *connection, ignoreGuiCommands));

        init_traci();
        return;
    }
    if (msg == executeOneTimestepTrigger) {
        executeOneTimestep();
        return;
    }
    throw cRuntimeError("TraCIScenarioManager received unknown self-message");
}

void TraCIScenarioManager::preInitializeModule(cModule* mod, const std::string& nodeId, const Coord& position, const std::string& road_id, double speed, Heading heading, VehicleSignalSet signals)
{
    // pre-initialize TraCIMobility
    auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->preInitialize(nodeId, position, road_id, speed, heading);
    }
}

void TraCIScenarioManager::updateModulePosition(cModule* mod, const Coord& p, const std::string& edge, double speed, Heading heading, VehicleSignalSet signals)
{
    // update position in TraCIMobility
    auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->nextPosition(p, edge, speed, heading, signals);
    }
}

// name: host;Car;i=vehicle.gif
void TraCIScenarioManager::addModule(std::string nodeId, std::string type, std::string name, std::string displayString, const Coord& position, std::string road_id, double speed, Heading heading, VehicleSignalSet signals, double length, double height, double width)
{

    if (hosts.find(nodeId) != hosts.end()) throw cRuntimeError("tried adding duplicate module");

    double option1 = hosts.size() / (hosts.size() + unEquippedHosts.size() + 1.0);
    double option2 = (hosts.size() + 1) / (hosts.size() + unEquippedHosts.size() + 1.0);

    if (fabs(option1 - penetrationRate) < fabs(option2 - penetrationRate)) {
        unEquippedHosts.insert(nodeId);
        return;
    }

    int32_t nodeVectorIndex = nextNodeVectorIndex++;

    cModule* parentmod = getParentModule();
    if (!parentmod) throw cRuntimeError("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(type.c_str());
    if (!nodeType) throw cRuntimeError("Module Type \"%s\" not found", type.c_str());

    // TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    if (displayString.length() > 0) {
        mod->getDisplayString().parse(displayString.c_str());
    }
    mod->buildInside();
    mod->scheduleStart(simTime() + updateInterval);

    preInitializeModule(mod, nodeId, position, road_id, speed, heading, signals);

    mod->callInitialize();
    hosts[nodeId] = mod;

    // post-initialize TraCIMobility
    auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
    for (auto mm : mobilityModules) {
        mm->changePosition();
    }

    if (vehicleObstacleControl) {
        std::vector<AntennaPosition> initialAntennaPositions;
        for (auto& caModule : getSubmodulesOfType<ChannelAccess>(mod, true)) {
            initialAntennaPositions.push_back(caModule->getAntennaPosition());
        }
        ASSERT(mobilityModules.size() == 1);
        auto mm = mobilityModules[0];
        double offset = mm->getHostPositionOffset();
        const MobileHostObstacle* vo = vehicleObstacleControl->add(MobileHostObstacle(initialAntennaPositions, mm, length, offset, width, height));
        vehicleObstacles[mm] = vo;
    }

    emit(traciModuleAddedSignal, mod);
}

cModule* TraCIScenarioManager::getManagedModule(std::string nodeId)
{
    if (hosts.find(nodeId) == hosts.end()) return nullptr;
    return hosts[nodeId];
}

bool TraCIScenarioManager::isModuleUnequipped(std::string nodeId)
{
    if (unEquippedHosts.find(nodeId) == unEquippedHosts.end()) return false;
    return true;
}

void TraCIScenarioManager::deleteManagedModule(std::string nodeId)
{
    cModule* mod = getManagedModule(nodeId);
    if (!mod) throw cRuntimeError("no vehicle with Id \"%s\" found", nodeId.c_str());

    emit(traciModuleRemovedSignal, mod);

    auto cas = getSubmodulesOfType<ChannelAccess>(mod, true);
    for (auto ca : cas) {
        cModule* nic = ca->getParentModule();
        auto connectionManager = ChannelAccess::getConnectionManager(nic);
        connectionManager->unregisterNic(nic);
    }
    if (vehicleObstacleControl) {
        for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
            cModule* submod = *iter;
            TraCIMobility* mm = dynamic_cast<TraCIMobility*>(submod);
            if (!mm) continue;
            auto vo = vehicleObstacles.find(mm);
            ASSERT(vo != vehicleObstacles.end());
            vehicleObstacleControl->erase(vo->second);
        }
    }

    hosts.erase(nodeId);
    mod->callFinish();
    mod->deleteModule();
}

void TraCIScenarioManager::executeOneTimestep()
{

    EV_DEBUG << "Triggering TraCI server simulation advance to t=" << simTime() << endl;

    simtime_t targetTime = simTime();

    emit(traciTimestepBeginSignal, targetTime);

    if (isConnected()) {
        TraCIBuffer buf = connection->query(CMD_SIMSTEP2, TraCIBuffer() << targetTime);

        uint32_t count;
        buf >> count;
        EV_DEBUG << "Getting " << count << " subscription results" << endl;
        for (uint32_t i = 0; i < count; ++i) {
            processSubcriptionResult(buf);
        }
    }

    emit(traciTimestepEndSignal, targetTime);

    if (!autoShutdownTriggered) scheduleAt(simTime() + updateInterval, executeOneTimestepTrigger);
}

void TraCIScenarioManager::subscribeToVehicleVariables(std::string vehicleId)
{
    // subscribe to some attributes of the vehicle
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = vehicleId;
    uint8_t variableNumber = 8;
    uint8_t variable1 = VAR_POSITION;
    uint8_t variable2 = VAR_ROAD_ID;
    uint8_t variable3 = VAR_SPEED;
    uint8_t variable4 = VAR_ANGLE;
    uint8_t variable5 = VAR_SIGNALS;
    uint8_t variable6 = VAR_LENGTH;
    uint8_t variable7 = VAR_HEIGHT;
    uint8_t variable8 = VAR_WIDTH;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5 << variable6 << variable7 << variable8);
    processSubcriptionResult(buf);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::unsubscribeFromVehicleVariables(std::string vehicleId)
{
    // subscribe to some attributes of the vehicle
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = vehicleId;
    uint8_t variableNumber = 0;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber);
    ASSERT(buf.eof());
}
void TraCIScenarioManager::subscribeToTrafficLightVariables(std::string tlId)
{
    // subscribe to some attributes of the traffic light system
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = tlId;
    uint8_t variableNumber = 4;
    uint8_t variable1 = TL_CURRENT_PHASE;
    uint8_t variable2 = TL_CURRENT_PROGRAM;
    uint8_t variable3 = TL_NEXT_SWITCH;
    uint8_t variable4 = TL_RED_YELLOW_GREEN_STATE;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_TL_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4);
    processSubcriptionResult(buf);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::unsubscribeFromTrafficLightVariables(std::string tlId)
{
    // unsubscribe from some attributes of the traffic light system
    // this method is mainly for completeness as traffic lights are not supposed to be removed at runtime

    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = tlId;
    uint8_t variableNumber = 0;

    TraCIBuffer buf = connection->query(CMD_SUBSCRIBE_TL_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber);
    ASSERT(buf.eof());
}

void TraCIScenarioManager::processTrafficLightSubscription(std::string objectId, TraCIBuffer& buf)
{
    cModule* tlIfSubmodule = trafficLights[objectId]->getSubmodule("tlInterface");
    TraCITrafficLightInterface* tlIfModule = dynamic_cast<TraCITrafficLightInterface*>(tlIfSubmodule);
    if (!tlIfModule) {
        throw cRuntimeError("Could not find traffic light module %s", objectId.c_str());
    }

    uint8_t variableNumber_resp;
    buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t response_type;
        buf >> response_type;
        uint8_t isokay;
        buf >> isokay;
        if (isokay != RTYPE_OK) {
            std::string description = buf.readTypeChecked<std::string>(TYPE_STRING);
            if (isokay == RTYPE_NOTIMPLEMENTED) {
                throw cRuntimeError("TraCI server reported subscribing to 0x%2x not implemented (\"%s\"). Might need newer version.", response_type, description.c_str());
            }
            else {
                throw cRuntimeError("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", response_type, description.c_str());
            }
        }
        switch (response_type) {
        case TL_CURRENT_PHASE:
            tlIfModule->setCurrentPhaseByNr(buf.readTypeChecked<int32_t>(TYPE_INTEGER), false);
            break;

        case TL_CURRENT_PROGRAM:
            tlIfModule->setCurrentLogicById(buf.readTypeChecked<std::string>(TYPE_STRING), false);
            break;

        case TL_NEXT_SWITCH:
            tlIfModule->setNextSwitch(buf.readTypeChecked<simtime_t>(getCommandInterface()->getTimeType()), false);
            break;

        case TL_RED_YELLOW_GREEN_STATE:
            tlIfModule->setCurrentState(buf.readTypeChecked<std::string>(TYPE_STRING), false);
            break;

        default:
            throw cRuntimeError("Received unhandled traffic light subscription result; type: 0x%02x", response_type);
            break;
        }
    }
}

void TraCIScenarioManager::processSimSubscription(std::string objectId, TraCIBuffer& buf)
{
    uint8_t variableNumber_resp;
    buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t variable1_resp;
        buf >> variable1_resp;
        uint8_t isokay;
        buf >> isokay;
        if (isokay != RTYPE_OK) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRING);
            std::string description;
            buf >> description;
            if (isokay == RTYPE_NOTIMPLEMENTED) throw cRuntimeError("TraCI server reported subscribing to variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, description.c_str());
            throw cRuntimeError("TraCI server reported error subscribing to variable 0x%2x (\"%s\").", variable1_resp, description.c_str());
        }

        if (variable1_resp == VAR_DEPARTED_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " departed vehicles." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == VAR_ARRIVED_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " arrived vehicles." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                if (subscribedVehicles.find(idstring) != subscribedVehicles.end()) {
                    subscribedVehicles.erase(idstring);
                    // no unsubscription via TraCI possible/necessary as of SUMO 1.0.0 (the vehicle has arrived)
                }

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            if ((count > 0) && (count >= activeVehicleCount) && autoShutdown) autoShutdownTriggered = true;
            activeVehicleCount -= count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == VAR_TELEPORT_STARTING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to teleport." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                // check if this object has been deleted already (e.g. because it was outside the ROI)
                cModule* mod = getManagedModule(idstring);
                if (mod) deleteManagedModule(idstring);

                if (unEquippedHosts.find(idstring) != unEquippedHosts.end()) {
                    unEquippedHosts.erase(idstring);
                }
            }

            activeVehicleCount -= count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == VAR_TELEPORT_ENDING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles ending teleport." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                // adding modules is handled on the fly when entering/leaving the ROI
            }

            activeVehicleCount += count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == VAR_PARKING_STARTING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles starting to park." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                cModule* mod = getManagedModule(idstring);
                auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
                for (auto mm : mobilityModules) {
                    mm->changeParkingState(true);
                }
            }

            parkingVehicleCount += count;
            drivingVehicleCount -= count;
        }
        else if (variable1_resp == VAR_PARKING_ENDING_VEHICLES_IDS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " vehicles ending to park." << endl;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;

                cModule* mod = getManagedModule(idstring);
                auto mobilityModules = getSubmodulesOfType<TraCIMobility>(mod);
                for (auto mm : mobilityModules) {
                    mm->changeParkingState(false);
                }
            }
            parkingVehicleCount -= count;
            drivingVehicleCount += count;
        }
        else if (variable1_resp == getCommandInterface()->getTimeStepCmd()) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == getCommandInterface()->getTimeType());
            simtime_t serverTimestep;
            buf >> serverTimestep;
            EV_DEBUG << "TraCI reports current time step as " << serverTimestep << "ms." << endl;
            simtime_t omnetTimestep = simTime();
            ASSERT(omnetTimestep == serverTimestep);
        }
        else {
            throw cRuntimeError("Received unhandled sim subscription result");
        }
    }
}

void TraCIScenarioManager::processVehicleSubscription(std::string objectId, TraCIBuffer& buf)
{
    bool isSubscribed = (subscribedVehicles.find(objectId) != subscribedVehicles.end());
    double px;
    double py;
    std::string edge;
    double speed;
    double angle_traci;
    int signals;
    double length;
    double height;
    double width;
    int numRead = 0;

    uint8_t variableNumber_resp;
    buf >> variableNumber_resp;
    for (uint8_t j = 0; j < variableNumber_resp; ++j) {
        uint8_t variable1_resp;
        buf >> variable1_resp;
        uint8_t isokay;
        buf >> isokay;
        if (isokay != RTYPE_OK) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRING);
            std::string errormsg;
            buf >> errormsg;
            if (isSubscribed) {
                if (isokay == RTYPE_NOTIMPLEMENTED) throw cRuntimeError("TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.", variable1_resp, errormsg.c_str());
                throw cRuntimeError("TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").", variable1_resp, errormsg.c_str());
            }
        }
        else if (variable1_resp == ID_LIST) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRINGLIST);
            uint32_t count;
            buf >> count;
            EV_DEBUG << "TraCI reports " << count << " active vehicles." << endl;
            ASSERT(count == activeVehicleCount);
            std::set<std::string> drivingVehicles;
            for (uint32_t i = 0; i < count; ++i) {
                std::string idstring;
                buf >> idstring;
                drivingVehicles.insert(idstring);
            }

            // check for vehicles that need subscribing to
            std::set<std::string> needSubscribe;
            std::set_difference(drivingVehicles.begin(), drivingVehicles.end(), subscribedVehicles.begin(), subscribedVehicles.end(), std::inserter(needSubscribe, needSubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needSubscribe.begin(); i != needSubscribe.end(); ++i) {
                subscribedVehicles.insert(*i);
                subscribeToVehicleVariables(*i);
            }

            // check for vehicles that need unsubscribing from
            std::set<std::string> needUnsubscribe;
            std::set_difference(subscribedVehicles.begin(), subscribedVehicles.end(), drivingVehicles.begin(), drivingVehicles.end(), std::inserter(needUnsubscribe, needUnsubscribe.begin()));
            for (std::set<std::string>::const_iterator i = needUnsubscribe.begin(); i != needUnsubscribe.end(); ++i) {
                subscribedVehicles.erase(*i);
                unsubscribeFromVehicleVariables(*i);
            }
        }
        else if (variable1_resp == VAR_POSITION) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == POSITION_2D);
            buf >> px;
            buf >> py;
            numRead++;
        }
        else if (variable1_resp == VAR_ROAD_ID) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_STRING);
            buf >> edge;
            numRead++;
        }
        else if (variable1_resp == VAR_SPEED) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> speed;
            numRead++;
        }
        else if (variable1_resp == VAR_ANGLE) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> angle_traci;
            numRead++;
        }
        else if (variable1_resp == VAR_SIGNALS) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_INTEGER);
            buf >> signals;
            numRead++;
        }
        else if (variable1_resp == VAR_LENGTH) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> length;
            numRead++;
        }
        else if (variable1_resp == VAR_HEIGHT) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> height;
            numRead++;
        }
        else if (variable1_resp == VAR_WIDTH) {
            uint8_t varType;
            buf >> varType;
            ASSERT(varType == TYPE_DOUBLE);
            buf >> width;
            numRead++;
        }
        else {
            throw cRuntimeError("Received unhandled vehicle subscription result");
        }
    }

    // bail out if we didn't want to receive these subscription results
    if (!isSubscribed) return;

    // make sure we got updates for all attributes
    if (numRead != 8) return;

    Coord p = connection->traci2omnet(TraCICoord(px, py));
    if ((p.x < 0) || (p.y < 0)) throw cRuntimeError("received bad node position (%.2f, %.2f), translated to (%.2f, %.2f)", px, py, p.x, p.y);

    Heading heading = connection->traci2omnetHeading(angle_traci);

    cModule* mod = getManagedModule(objectId);

    // is it in the ROI?
    bool inRoi = !roi.hasConstraints() ? true : (roi.onAnyRectangle(TraCICoord(px, py)) || roi.partOfRoads(edge));
    if (!inRoi) {
        if (mod) {
            deleteManagedModule(objectId);
            EV_DEBUG << "Vehicle #" << objectId << " left region of interest" << endl;
        }
        else if (unEquippedHosts.find(objectId) != unEquippedHosts.end()) {
            unEquippedHosts.erase(objectId);
            EV_DEBUG << "Vehicle (unequipped) # " << objectId << " left region of interest" << endl;
        }
        return;
    }

    if (isModuleUnequipped(objectId)) {
        return;
    }

    if (!mod) {
        // no such module - need to create
        std::string vType = commandIfc->vehicle(objectId).getTypeId();
        std::string mType, mName, mDisplayString;
        TypeMapping::iterator iType, iName, iDisplayString;

        TypeMapping::iterator i;
        iType = moduleType.find(vType);
        if (iType == moduleType.end()) {
            iType = moduleType.find("*");
            if (iType == moduleType.end()) throw cRuntimeError("cannot find a module type for vehicle type \"%s\"", vType.c_str());
        }
        mType = iType->second;
        // search for module name
        iName = moduleName.find(vType);
        if (iName == moduleName.end()) {
            iName = moduleName.find(std::string("*"));
            if (iName == moduleName.end()) throw cRuntimeError("cannot find a module name for vehicle type \"%s\"", vType.c_str());
        }
        mName = iName->second;
        if (moduleDisplayString.size() != 0) {
            iDisplayString = moduleDisplayString.find(vType);
            if (iDisplayString == moduleDisplayString.end()) {
                iDisplayString = moduleDisplayString.find("*");
                if (iDisplayString == moduleDisplayString.end()) throw cRuntimeError("cannot find a module display string for vehicle type \"%s\"", vType.c_str());
            }
            mDisplayString = iDisplayString->second;
        }
        else {
            mDisplayString = "";
        }

        if (mType != "0") {
            addModule(objectId, mType, mName, mDisplayString, p, edge, speed, heading, VehicleSignalSet(signals), length, height, width);
            EV_DEBUG << "Added vehicle #" << objectId << endl;
        }
    }
    else {
        // module existed - update position
        EV_DEBUG << "module " << objectId << " moving to " << p.x << "," << p.y << endl;
        updateModulePosition(mod, p, edge, speed, heading, VehicleSignalSet(signals));
    }
}

void TraCIScenarioManager::processSubcriptionResult(TraCIBuffer& buf)
{
    uint8_t cmdLength_resp;
    buf >> cmdLength_resp;
    uint32_t cmdLengthExt_resp;
    buf >> cmdLengthExt_resp;
    uint8_t commandId_resp;
    buf >> commandId_resp;
    std::string objectId_resp;
    buf >> objectId_resp;

    if (commandId_resp == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE)
        processVehicleSubscription(objectId_resp, buf);
    else if (commandId_resp == RESPONSE_SUBSCRIBE_SIM_VARIABLE)
        processSimSubscription(objectId_resp, buf);
    else if (commandId_resp == RESPONSE_SUBSCRIBE_TL_VARIABLE)
        processTrafficLightSubscription(objectId_resp, buf);
    else {
        throw cRuntimeError("Received unhandled subscription result");
    }
}

int TraCIScenarioManager::getPortNumber() const
{
    int port = par("port");
    if (port != -1) {
        return port;
    }

    // search for externally configured traci port
    const char* env_port = std::getenv("VEINS_TRACI_PORT");
    if (env_port != nullptr) {
        port = std::atoi(env_port);
    }

    return port;
}
