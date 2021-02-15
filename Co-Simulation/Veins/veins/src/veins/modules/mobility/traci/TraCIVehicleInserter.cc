//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include "veins/modules/mobility/traci/TraCIVehicleInserter.h"

#include <vector>
#include <algorithm>

#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"

#include "veins/base/utils/FindModule.h"

Define_Module(veins::TraCIVehicleInserter);

using namespace veins;

TraCIVehicleInserter::TraCIVehicleInserter()
    : mobRng(nullptr)
{
}

TraCIVehicleInserter::~TraCIVehicleInserter()
{
}

int TraCIVehicleInserter::numInitStages() const
{
    return std::max(cSimpleModule::numInitStages(), 2);
}

void TraCIVehicleInserter::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != 1) {
        return;
    }

    // internal 1
    manager = TraCIScenarioManagerAccess().get();

    // signals
    manager->subscribe(TraCIScenarioManager::traciModuleAddedSignal, this);
    manager->subscribe(TraCIScenarioManager::traciTimestepBeginSignal, this);

    // parameters
    vehicleRngIndex = par("vehicleRngIndex");
    numVehicles = par("numVehicles");

    // internal 2
    vehicleNameCounter = 0;
    mobRng = getRNG(vehicleRngIndex);
}

void TraCIVehicleInserter::finish()
{
    unsubscribe(TraCIScenarioManager::traciModuleAddedSignal, this);
    unsubscribe(TraCIScenarioManager::traciTimestepBeginSignal, this);
}

void TraCIVehicleInserter::finish(cComponent* component, simsignal_t signalID)
{
    cListener::finish(component, signalID);
}

void TraCIVehicleInserter::handleMessage(cMessage* msg)
{
}

void TraCIVehicleInserter::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details)
{
    if (signalID == TraCIScenarioManager::traciModuleAddedSignal) {
        ASSERT(manager->isConnected());
        cModule* mod = check_and_cast<cModule*>(obj);
        auto* mob = FindModule<TraCIMobility*>::findSubModule(mod);
        ASSERT(mob != nullptr);
        std::string nodeId = mob->getExternalId();
        if (queuedVehicles.find(nodeId) != queuedVehicles.end()) {
            queuedVehicles.erase(nodeId);
        }
    }
}

void TraCIVehicleInserter::receiveSignal(cComponent* source, simsignal_t signalID, const SimTime& t, cObject* details)
{
    if (signalID == TraCIScenarioManager::traciTimestepBeginSignal) {
        ASSERT(manager->isConnected());
        if (simTime() > 1) {
            if (vehicleTypeIds.size() == 0) {
                std::list<std::string> vehTypes = manager->getCommandInterface()->getVehicleTypeIds();
                for (std::list<std::string>::const_iterator i = vehTypes.begin(); i != vehTypes.end(); ++i) {
                    if (i->substr(0, 8).compare("DEFAULT_") != 0) {
                        EV_DEBUG << *i << std::endl;
                        vehicleTypeIds.push_back(*i);
                    }
                }
            }
            if (routeIds.size() == 0) {
                std::list<std::string> routes = manager->getCommandInterface()->getRouteIds();
                for (std::list<std::string>::const_iterator i = routes.begin(); i != routes.end(); ++i) {
                    std::string routeId = *i;
                    if (par("useRouteDistributions").boolValue() == true) {
                        if (std::count(routeId.begin(), routeId.end(), '#') >= 1) {
                            EV_DEBUG << "Omitting route " << routeId << " as it seems to be a member of a route distribution (found '#' in name)" << std::endl;
                            continue;
                        }
                    }
                    EV_DEBUG << "Adding " << routeId << " to list of possible routes" << std::endl;
                    routeIds.push_back(routeId);
                }
            }
            for (int i = manager->getManagedHosts().size() + queuedVehicles.size(); i < numVehicles; i++) {
                insertNewVehicle();
            }
        }

        insertVehicles();
    }
}

void TraCIVehicleInserter::insertNewVehicle()
{
    std::string type;
    if (vehicleTypeIds.size()) {
        int vehTypeId = mobRng->intRand(vehicleTypeIds.size());
        type = vehicleTypeIds[vehTypeId];
    }
    else {
        type = "DEFAULT_VEHTYPE";
    }
    int routeId = mobRng->intRand(routeIds.size());
    vehicleInsertQueue[routeId].push(type);
}

void TraCIVehicleInserter::insertVehicles()
{

    for (std::map<int, std::queue<std::string>>::iterator i = vehicleInsertQueue.begin(); i != vehicleInsertQueue.end();) {
        std::string route = routeIds[i->first];
        EV_DEBUG << "process " << route << std::endl;
        std::queue<std::string> vehicles = i->second;
        while (!i->second.empty()) {
            bool suc = false;
            std::string type = i->second.front();
            std::stringstream veh;
            veh << type << "_" << vehicleNameCounter;
            EV_DEBUG << "trying to add " << veh.str() << " with " << route << " vehicle type " << type << std::endl;

            suc = manager->getCommandInterface()->addVehicle(veh.str(), type, route, simTime());
            if (!suc) {
                i->second.pop();
            }
            else {
                EV_DEBUG << "successful inserted " << veh.str() << std::endl;
                queuedVehicles.insert(veh.str());
                i->second.pop();
                vehicleNameCounter++;
            }
        }
        std::map<int, std::queue<std::string>>::iterator tmp = i;
        ++tmp;
        vehicleInsertQueue.erase(i);
        i = tmp;
    }
}
