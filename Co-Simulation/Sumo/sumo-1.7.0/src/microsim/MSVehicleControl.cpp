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
/// @file    MSVehicleControl.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Wed, 10. Dec 2003
///
// The class responsible for building and deletion of vehicles
/****************************************************************************/
#include <config.h>

#include "MSVehicleControl.h"
#include "MSVehicle.h"
#include "MSLane.h"
#include "MSEdge.h"
#include "MSNet.h"
#include "MSRouteHandler.h"
#include <microsim/devices/MSVehicleDevice.h>
#include <microsim/devices/MSDevice_Tripinfo.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/Named.h>
#include <utils/common/RGBColor.h>
#include <utils/vehicle/SUMOVTypeParameter.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/router/IntermodalRouter.h>
#ifdef HAVE_FOX
#include <utils/foxtools/FXConditionalLock.h>
#endif


// ===========================================================================
// member method definitions
// ===========================================================================
MSVehicleControl::MSVehicleControl() :
    myLoadedVehNo(0),
    myRunningVehNo(0),
    myEndedVehNo(0),
    myDiscarded(0),
    myCollisions(0),
    myTeleportsJam(0),
    myTeleportsYield(0),
    myTeleportsWrongLane(0),
    myEmergencyStops(0),
    myStoppedVehicles(0),
    myTotalDepartureDelay(0),
    myTotalTravelTime(0),
    myDefaultVTypeMayBeDeleted(true),
    myDefaultPedTypeMayBeDeleted(true),
    myDefaultContainerTypeMayBeDeleted(true),
    myDefaultBikeTypeMayBeDeleted(true),
    myDefaultTaxiTypeMayBeDeleted(true),
    myWaitingForPerson(0),
    myWaitingForContainer(0),
    myMaxSpeedFactor(1),
    myMinDeceleration(SUMOVTypeParameter::getDefaultDecel(SVC_IGNORING)),
    myPendingRemovals(MSGlobals::gNumSimThreads > 1) {
    SUMOVTypeParameter defType(DEFAULT_VTYPE_ID, SVC_PASSENGER);
    myVTypeDict[DEFAULT_VTYPE_ID] = MSVehicleType::build(defType);

    SUMOVTypeParameter defPedType(DEFAULT_PEDTYPE_ID, SVC_PEDESTRIAN);
    defPedType.parametersSet |= VTYPEPARS_VEHICLECLASS_SET;
    myVTypeDict[DEFAULT_PEDTYPE_ID] = MSVehicleType::build(defPedType);

    SUMOVTypeParameter defBikeType(DEFAULT_BIKETYPE_ID, SVC_BICYCLE);
    defBikeType.parametersSet |= VTYPEPARS_VEHICLECLASS_SET;
    myVTypeDict[DEFAULT_BIKETYPE_ID] = MSVehicleType::build(defBikeType);

    SUMOVTypeParameter defTaxiType(DEFAULT_TAXITYPE_ID, SVC_TAXI);
    defTaxiType.parametersSet |= VTYPEPARS_VEHICLECLASS_SET;
    myVTypeDict[DEFAULT_TAXITYPE_ID] = MSVehicleType::build(defTaxiType);

    SUMOVTypeParameter defContainerType(DEFAULT_CONTAINERTYPE_ID, SVC_IGNORING);
    // ISO Container TEU (cannot set this based on vClass)
    defContainerType.length = 6.1;
    defContainerType.width = 2.4;
    defContainerType.height = 2.6;
    defContainerType.parametersSet |= VTYPEPARS_VEHICLECLASS_SET;
    myVTypeDict[DEFAULT_CONTAINERTYPE_ID] = MSVehicleType::build(defContainerType);

    myScale = OptionsCont::getOptions().getFloat("scale");
}


MSVehicleControl::~MSVehicleControl() {
    clearState();
}


SUMOVehicle*
MSVehicleControl::buildVehicle(SUMOVehicleParameter* defs,
                               const MSRoute* route, MSVehicleType* type,
                               const bool ignoreStopErrors, const bool fromRouteFile) {
    MSVehicle* built = new MSVehicle(defs, route, type, type->computeChosenSpeedDeviation(fromRouteFile ? MSRouteHandler::getParsingRNG() : nullptr));
    initVehicle(built, ignoreStopErrors);
    return built;
}


void
MSVehicleControl::initVehicle(MSBaseVehicle* built, const bool ignoreStopErrors) {
    myLoadedVehNo++;
    try {
        built->initDevices();
        built->addStops(ignoreStopErrors);
    } catch (ProcessError&) {
        delete built;
        throw;
    }
    MSNet::getInstance()->informVehicleStateListener(built, MSNet::VEHICLE_STATE_BUILT);
}


void
MSVehicleControl::scheduleVehicleRemoval(SUMOVehicle* veh, bool checkDuplicate) {
    assert(myRunningVehNo > 0);
    if (!checkDuplicate || !isPendingRemoval(veh)) {
        myPendingRemovals.push_back(veh);
    }
}


bool
MSVehicleControl::isPendingRemoval(SUMOVehicle* veh) {
#ifdef HAVE_FOX
    return myPendingRemovals.contains(veh);
#else
    return std::find(myPendingRemovals.begin(), myPendingRemovals.end(), veh) == myPendingRemovals.end();
#endif
}

void
MSVehicleControl::removePending() {
    OutputDevice* const tripinfoOut = OptionsCont::getOptions().isSet("tripinfo-output") ? &OutputDevice::getDeviceByOption("tripinfo-output") : nullptr;
#ifdef HAVE_FOX
    std::vector<SUMOVehicle*>& vehs = myPendingRemovals.getContainer();
#else
    std::vector<SUMOVehicle*>& vehs = myPendingRemovals;
#endif
    std::sort(vehs.begin(), vehs.end(), ComparatorNumericalIdLess());
    for (SUMOVehicle* const veh : vehs) {
        myTotalTravelTime += STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() - veh->getDeparture());
        myRunningVehNo--;
        MSNet::getInstance()->informVehicleStateListener(veh, MSNet::VEHICLE_STATE_ARRIVED);
        // vehicle is equipped with tripinfo device (not all vehicles are)
        const bool hasTripinfo = veh->getDevice(typeid(MSDevice_Tripinfo)) != nullptr;
        for (MSVehicleDevice* const dev : veh->getDevices()) {
            dev->generateOutput(hasTripinfo ? tripinfoOut : nullptr);
        }
        if (tripinfoOut != nullptr && hasTripinfo) {
            // close tag after tripinfo (possibly including emissions from another device) have been written
            tripinfoOut->closeTag();
        }
        deleteVehicle(veh);
    }
    vehs.clear();
    if (tripinfoOut != nullptr) {
        // there seem to be people who think reading an unfinished xml is a good idea ;-)
        tripinfoOut->flush();
    }
#ifdef HAVE_FOX
    myPendingRemovals.unlock();
#endif
}


void
MSVehicleControl::vehicleDeparted(const SUMOVehicle& v) {
    ++myRunningVehNo;
    myTotalDepartureDelay += STEPS2TIME(v.getDeparture() - STEPFLOOR(v.getParameter().depart));
    MSNet::getInstance()->informVehicleStateListener(&v, MSNet::VEHICLE_STATE_DEPARTED);
    myMaxSpeedFactor = MAX2(myMaxSpeedFactor, v.getChosenSpeedFactor());
    if ((v.getVClass() & (SVC_PEDESTRIAN | SVC_NON_ROAD)) == 0) {
        // only  worry about deceleration of road users
        myMinDeceleration = MIN2(myMinDeceleration, v.getVehicleType().getCarFollowModel().getMaxDecel());
    }
}


void
MSVehicleControl::setState(int runningVehNo, int loadedVehNo, int endedVehNo, double totalDepartureDelay, double totalTravelTime) {
    myRunningVehNo = runningVehNo;
    myLoadedVehNo = loadedVehNo;
    myEndedVehNo = endedVehNo;
    myTotalDepartureDelay = totalDepartureDelay;
    myTotalTravelTime = totalTravelTime;
}


void
MSVehicleControl::saveState(OutputDevice& out) {
    out.openTag(SUMO_TAG_DELAY);
    out.writeAttr(SUMO_ATTR_NUMBER, myRunningVehNo);
    out.writeAttr(SUMO_ATTR_BEGIN, myLoadedVehNo);
    out.writeAttr(SUMO_ATTR_END, myEndedVehNo);
    out.writeAttr(SUMO_ATTR_DEPART, myTotalDepartureDelay);
    out.writeAttr(SUMO_ATTR_TIME, myTotalTravelTime).closeTag();
    // save vehicle types
    VTypeDictType vTypes = myVTypeDict;
    if (myDefaultVTypeMayBeDeleted) {
        vTypes.erase(DEFAULT_VTYPE_ID);
    }
    if (myDefaultPedTypeMayBeDeleted) {
        vTypes.erase(DEFAULT_PEDTYPE_ID);
    }
    if (myDefaultContainerTypeMayBeDeleted) {
        vTypes.erase(DEFAULT_CONTAINERTYPE_ID);
    }
    if (myDefaultBikeTypeMayBeDeleted) {
        vTypes.erase(DEFAULT_BIKETYPE_ID);
    }
    if (myDefaultTaxiTypeMayBeDeleted) {
        vTypes.erase(DEFAULT_TAXITYPE_ID);
    }
    for (const auto& item : vTypes) {
        item.second->getParameter().write(out);
    }
    for (VTypeDistDictType::iterator it = myVTypeDistDict.begin(); it != myVTypeDistDict.end(); ++it) {
        out.openTag(SUMO_TAG_VTYPE_DISTRIBUTION).writeAttr(SUMO_ATTR_ID, it->first);
        out.writeAttr(SUMO_ATTR_VTYPES, (*it).second->getVals());
        out.writeAttr(SUMO_ATTR_PROBS, (*it).second->getProbs());
        out.closeTag();
    }
    for (VehicleDictType::iterator it = myVehicleDict.begin(); it != myVehicleDict.end(); ++it) {
        (*it).second->saveState(out);
    }
}


void
MSVehicleControl::clearState() {
    for (VehicleDictType::iterator i = myVehicleDict.begin(); i != myVehicleDict.end(); ++i) {
        delete (*i).second;
    }
    myVehicleDict.clear();
    // delete vehicle type distributions
    for (VTypeDistDictType::iterator i = myVTypeDistDict.begin(); i != myVTypeDistDict.end(); ++i) {
        delete (*i).second;
    }
    myVTypeDistDict.clear();
    // delete vehicle types
    for (VTypeDictType::iterator i = myVTypeDict.begin(); i != myVTypeDict.end(); ++i) {
        delete (*i).second;
    }
    myVTypeDict.clear();
    myDefaultVTypeMayBeDeleted = true;
    myDefaultPedTypeMayBeDeleted = true;
    myDefaultContainerTypeMayBeDeleted = true;
    myDefaultBikeTypeMayBeDeleted = true;
    myDefaultTaxiTypeMayBeDeleted = true;
}


bool
MSVehicleControl::addVehicle(const std::string& id, SUMOVehicle* v) {
    VehicleDictType::iterator it = myVehicleDict.find(id);
    if (it == myVehicleDict.end()) {
        // id not in myVehicleDict.
        myVehicleDict[id] = v;
        const SUMOVehicleParameter& pars = v->getParameter();
        if (pars.departProcedure == DEPART_TRIGGERED || pars.departProcedure == DEPART_CONTAINER_TRIGGERED || pars.departProcedure == DEPART_SPLIT) {
            const MSEdge* const firstEdge = v->getRoute().getEdges()[0];
            if (!MSGlobals::gUseMesoSim) {
                // position will be checked against person position later
                static_cast<MSVehicle*>(v)->setTentativeLaneAndPosition(firstEdge->getLanes()[0], v->getParameter().departPos);
            }
            firstEdge->addWaiting(v);
            registerOneWaiting(pars.departProcedure == DEPART_TRIGGERED);
        }
        if (v->getVClass() != SVC_TAXI && pars.line != "" && pars.repetitionNumber < 0) {
            myPTVehicles.push_back(v);
        }
        return true;
    }
    return false;
}


SUMOVehicle*
MSVehicleControl::getVehicle(const std::string& id) const {
    VehicleDictType::const_iterator it = myVehicleDict.find(id);
    if (it == myVehicleDict.end()) {
        return nullptr;
    }
    return it->second;
}


void
MSVehicleControl::deleteVehicle(SUMOVehicle* veh, bool discard) {
    myEndedVehNo++;
    if (discard) {
        myDiscarded++;
    }
    if (veh != nullptr) {
        myVehicleDict.erase(veh->getID());
    }
    auto ptVehIt = std::find(myPTVehicles.begin(), myPTVehicles.end(), veh);
    if (ptVehIt != myPTVehicles.end()) {
        myPTVehicles.erase(ptVehIt);
    }
    delete veh;
}


bool
MSVehicleControl::checkVType(const std::string& id) {
    if (id == DEFAULT_VTYPE_ID) {
        if (myDefaultVTypeMayBeDeleted) {
            delete myVTypeDict[id];
            myVTypeDict.erase(myVTypeDict.find(id));
            myDefaultVTypeMayBeDeleted = false;
        } else {
            return false;
        }
    } else if (id == DEFAULT_PEDTYPE_ID) {
        if (myDefaultPedTypeMayBeDeleted) {
            delete myVTypeDict[id];
            myVTypeDict.erase(myVTypeDict.find(id));
            myDefaultPedTypeMayBeDeleted = false;
        } else {
            return false;
        }
    } else if (id == DEFAULT_CONTAINERTYPE_ID) {
        if (myDefaultContainerTypeMayBeDeleted) {
            delete myVTypeDict[id];
            myVTypeDict.erase(myVTypeDict.find(id));
            myDefaultContainerTypeMayBeDeleted = false;
        } else {
            return false;
        }
    } else if (id == DEFAULT_BIKETYPE_ID) {
        if (myDefaultBikeTypeMayBeDeleted) {
            delete myVTypeDict[id];
            myVTypeDict.erase(myVTypeDict.find(id));
            myDefaultBikeTypeMayBeDeleted = false;
        } else {
            return false;
        }
    } else if (id == DEFAULT_TAXITYPE_ID) {
        if (myDefaultTaxiTypeMayBeDeleted) {
            delete myVTypeDict[id];
            myVTypeDict.erase(myVTypeDict.find(id));
            myDefaultTaxiTypeMayBeDeleted = false;
        } else {
            return false;
        }
    } else {
        if (myVTypeDict.find(id) != myVTypeDict.end() || myVTypeDistDict.find(id) != myVTypeDistDict.end()) {
            return false;
        }
    }
    return true;
}

bool
MSVehicleControl::addVType(MSVehicleType* vehType) {
    if (checkVType(vehType->getID())) {
        myVTypeDict[vehType->getID()] = vehType;
        return true;
    }
    return false;
}


void
MSVehicleControl::removeVType(const MSVehicleType* vehType) {
    assert(vehType != 0);
    assert(myVTypeDict.find(vehType->getID()) != myVTypeDict.end());
    myVTypeDict.erase(vehType->getID());
    if (myVTypeToDist.find(vehType->getID()) != myVTypeToDist.end()) {
        myVTypeToDist.erase(vehType->getID());
    }
    delete vehType;
}


bool
MSVehicleControl::addVTypeDistribution(const std::string& id, RandomDistributor<MSVehicleType*>* vehTypeDistribution) {
    if (checkVType(id)) {
        myVTypeDistDict[id] = vehTypeDistribution;
        std::vector<MSVehicleType*> vehTypes = vehTypeDistribution->getVals();
        for (auto vehType : vehTypes) {
            if (myVTypeToDist.find(vehType->getID()) != myVTypeToDist.end()) {
                myVTypeToDist[vehType->getID()].insert(id);
            } else {
                myVTypeToDist[vehType->getID()] = { id };
            }
        }
        return true;
    }
    return false;
}


bool
MSVehicleControl::hasVType(const std::string& id) const {
    return myVTypeDict.count(id) > 0 || myVTypeDistDict.count(id) > 0;
}


bool
MSVehicleControl::hasVTypeDistribution(const std::string& id) const {
    return myVTypeDistDict.count(id) > 0;
}


MSVehicleType*
MSVehicleControl::getVType(const std::string& id, std::mt19937* rng) {
    VTypeDictType::iterator it = myVTypeDict.find(id);
    if (it == myVTypeDict.end()) {
        VTypeDistDictType::iterator it2 = myVTypeDistDict.find(id);
        if (it2 == myVTypeDistDict.end()) {
            return nullptr;
        }
        return it2->second->get(rng);
    }
    if (id == DEFAULT_VTYPE_ID) {
        myDefaultVTypeMayBeDeleted = false;
    } else if (id == DEFAULT_PEDTYPE_ID) {
        myDefaultPedTypeMayBeDeleted = false;
    }
    return it->second;
}


void
MSVehicleControl::insertVTypeIDs(std::vector<std::string>& into) const {
    into.reserve(into.size() + myVTypeDict.size() + myVTypeDistDict.size());
    for (VTypeDictType::const_iterator i = myVTypeDict.begin(); i != myVTypeDict.end(); ++i) {
        into.push_back((*i).first);
    }
    for (VTypeDistDictType::const_iterator i = myVTypeDistDict.begin(); i != myVTypeDistDict.end(); ++i) {
        into.push_back((*i).first);
    }
}


const std::set<std::string>
MSVehicleControl::getVTypeDistributionMembership(const std::string& id) const {
    std::map<std::string, std::set<std::string>>::const_iterator it = myVTypeToDist.find(id);
    if (it == myVTypeToDist.end()) {
        return std::set<std::string>();
    }
    return it->second;
}

const RandomDistributor<MSVehicleType*>*
MSVehicleControl::getVTypeDistribution(const std::string& typeDistID) const {
    auto it = myVTypeDistDict.find(typeDistID);
    if (it != myVTypeDistDict.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void
MSVehicleControl::abortWaiting() {
    for (VehicleDictType::iterator i = myVehicleDict.begin(); i != myVehicleDict.end(); ++i) {
        WRITE_WARNINGF("Vehicle '%' aborted waiting for a % that will never come.", i->first,
                       i->second->getParameter().departProcedure == DEPART_SPLIT ? "split" : "person or container")
    }
}


int
MSVehicleControl::getHaltingVehicleNo() const {
    int result = 0;
    for (MSVehicleControl::constVehIt it = loadedVehBegin(); it != loadedVehEnd(); ++it) {
        const SUMOVehicle* veh = it->second;
        if ((veh->isOnRoad() || veh->isRemoteControlled()) && veh->getSpeed() < SUMO_const_haltingSpeed)  {
            result++;
        }
    }
    return result;
}



std::pair<double, double>
MSVehicleControl::getVehicleMeanSpeeds() const {
    double speedSum = 0;
    double relSpeedSum = 0;
    int count = 0;
    for (MSVehicleControl::constVehIt it = loadedVehBegin(); it != loadedVehEnd(); ++it) {
        const SUMOVehicle* veh = it->second;
        if ((veh->isOnRoad() || veh->isRemoteControlled()) && !veh->isStopped()) {
            count++;
            speedSum += veh->getSpeed();
            relSpeedSum += veh->getSpeed() / veh->getEdge()->getSpeedLimit();
        }
    }
    if (count > 0) {
        return std::make_pair(speedSum / count, relSpeedSum / count);
    } else {
        return std::make_pair(-1, -1);
    }
}


int
MSVehicleControl::getQuota(double frac, int loaded) const {
    frac = frac < 0 ? myScale : frac;
    if (frac < 0 || frac == 1.) {
        return 1;
    }
    const int origLoaded = (loaded < 1
                            // the vehicle in question has already been loaded, hence  the '-1'
                            ? frac > 1. ? (int)(myLoadedVehNo / frac) : myLoadedVehNo - 1
                            // given transportable number reflects only previously loaded
                            : frac > 1. ? (int)(loaded / frac) : loaded);
    const int base = (int)frac;
    const int resolution = 1000;
    const int intFrac = (int)floor((frac - base) * resolution + 0.5);
    // apply % twice to avoid integer overflow
    if (((origLoaded % resolution) * intFrac) % resolution < intFrac) {
        return base + 1;
    }
    return base;
}

int
MSVehicleControl::getTeleportCount() const {
    return (MSLane::teleportOnCollision() ? myCollisions : 0) + myTeleportsJam + myTeleportsYield + myTeleportsWrongLane;
}


void
MSVehicleControl::adaptIntermodalRouter(MSNet::MSIntermodalRouter& router) const {
    for (const SUMOVehicle* const veh : myPTVehicles) {
        // add single vehicles with line attribute which are not part of a flow
        const MSRoute* const route = MSRoute::dictionary(veh->getParameter().routeid);
        router.getNetwork()->addSchedule(veh->getParameter(), route == nullptr ? nullptr : &route->getStops());
    }
}


/****************************************************************************/
