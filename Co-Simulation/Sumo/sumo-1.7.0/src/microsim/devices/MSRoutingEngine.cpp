/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2007-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSRoutingEngine.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Christoph Sommer
/// @author  Jakob Erdmann
/// @date    Tue, 04 Dec 2007
///
// A device that performs vehicle rerouting based on current edge speeds
/****************************************************************************/
#include <config.h>

#include "MSRoutingEngine.h"
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSVehicleControl.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/WrappingCommand.h>
#include <utils/common/StaticCommand.h>
#include <utils/common/StringUtils.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include <utils/router/DijkstraRouter.h>
#include <utils/router/AStarRouter.h>
#include <utils/router/CHRouter.h>
#include <utils/router/CHRouterWrapper.h>


// ===========================================================================
// static member variables
// ===========================================================================
std::vector<double> MSRoutingEngine::myEdgeSpeeds;
std::vector<double> MSRoutingEngine::myEdgeBikeSpeeds;
std::vector<std::vector<double> > MSRoutingEngine::myPastEdgeSpeeds;
std::vector<std::vector<double> > MSRoutingEngine::myPastEdgeBikeSpeeds;
Command* MSRoutingEngine::myEdgeWeightSettingCommand = nullptr;
double MSRoutingEngine::myAdaptationWeight;
int MSRoutingEngine::myAdaptationSteps;
int MSRoutingEngine::myAdaptationStepsIndex = 0;
SUMOTime MSRoutingEngine::myAdaptationInterval = -1;
SUMOTime MSRoutingEngine::myLastAdaptation = -1;
bool MSRoutingEngine::myWithTaz;
bool MSRoutingEngine::myBikeSpeeds;
MSRoutingEngine::MSRouterProvider* MSRoutingEngine::myRouterProvider = nullptr;
std::map<std::pair<const MSEdge*, const MSEdge*>, const MSRoute*> MSRoutingEngine::myCachedRoutes;
double MSRoutingEngine::myPriorityFactor(0);
double MSRoutingEngine::myMinEdgePriority(std::numeric_limits<double>::max());
double MSRoutingEngine::myEdgePriorityRange(0);

SUMOAbstractRouter<MSEdge, SUMOVehicle>::Operation MSRoutingEngine::myEffortFunc = &MSRoutingEngine::getEffort;
#ifdef HAVE_FOX
FXMutex MSRoutingEngine::myRouteCacheMutex;
#endif


// ===========================================================================
// method definitions
// ===========================================================================
void
MSRoutingEngine::initWeightUpdate() {
    if (myAdaptationInterval == -1) {
        myEdgeWeightSettingCommand = nullptr;
        myEdgeSpeeds.clear();
        myAdaptationInterval = -1;
        myAdaptationSteps = -1;
        myLastAdaptation = -1;
        const OptionsCont& oc = OptionsCont::getOptions();
        myWithTaz = oc.getBool("device.rerouting.with-taz");
        myAdaptationInterval = string2time(oc.getString("device.rerouting.adaptation-interval"));
        myAdaptationWeight = oc.getFloat("device.rerouting.adaptation-weight");
        const SUMOTime period = string2time(oc.getString("device.rerouting.period"));
        if (myAdaptationWeight < 1. && myAdaptationInterval > 0) {
            myEdgeWeightSettingCommand = new StaticCommand<MSRoutingEngine>(&MSRoutingEngine::adaptEdgeEfforts);
            MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(myEdgeWeightSettingCommand);
        } else if (period > 0) {
            WRITE_WARNING("Rerouting is useless if the edge weights do not get updated!");
        }
        OutputDevice::createDeviceByOption("device.rerouting.output", "weights", "meandata_file.xsd");
    }
}


void
MSRoutingEngine::initEdgeWeights(SUMOVehicleClass svc) {
    if (myBikeSpeeds && svc == SVC_BICYCLE) {
        _initEdgeWeights(myEdgeBikeSpeeds, myPastEdgeBikeSpeeds);
    } else {
        _initEdgeWeights(myEdgeSpeeds, myPastEdgeSpeeds);
    }
}


void
MSRoutingEngine::_initEdgeWeights(std::vector<double>& edgeSpeeds, std::vector<std::vector<double> >& pastEdgeSpeeds) {
    if (edgeSpeeds.empty()) {
        const OptionsCont& oc = OptionsCont::getOptions();
        if (myAdaptationWeight == 0 || !oc.isDefault("device.rerouting.adaptation-steps")) {
            myAdaptationSteps = oc.getInt("device.rerouting.adaptation-steps");
        }
        const bool useLoaded = oc.getBool("device.rerouting.init-with-loaded-weights");
        const double currentSecond = SIMTIME;
        double maxEdgePriority = -std::numeric_limits<double>::max();
        for (const MSEdge* const edge : MSNet::getInstance()->getEdgeControl().getEdges()) {
            while (edge->getNumericalID() >= (int)edgeSpeeds.size()) {
                edgeSpeeds.push_back(0);
                if (myAdaptationSteps > 0) {
                    pastEdgeSpeeds.push_back(std::vector<double>());
                }
            }
            if (useLoaded) {
                edgeSpeeds[edge->getNumericalID()] = edge->getLength() / MSNet::getTravelTime(edge, nullptr, currentSecond);
            } else {
                edgeSpeeds[edge->getNumericalID()] = edge->getMeanSpeed();
            }
            if (myAdaptationSteps > 0) {
                pastEdgeSpeeds[edge->getNumericalID()] = std::vector<double>(myAdaptationSteps, edgeSpeeds[edge->getNumericalID()]);
            }
            maxEdgePriority = MAX2(maxEdgePriority, (double)edge->getPriority());
            myMinEdgePriority = MIN2(myMinEdgePriority, (double)edge->getPriority());
        }
        myEdgePriorityRange = maxEdgePriority - myMinEdgePriority;
        myLastAdaptation = MSNet::getInstance()->getCurrentTimeStep();
        myPriorityFactor = oc.getFloat("weights.priority-factor");
        if (myPriorityFactor < 0) {
            throw ProcessError("weights.priority-factor cannot be negative.");
        }
        if (myPriorityFactor > 0) {
            if (myEdgePriorityRange == 0) {
                WRITE_WARNING("Option weights.priority-factor does not take effect because all edges have the same priority");
                myPriorityFactor = 0;
            }
        }
    }
}


double
MSRoutingEngine::getEffort(const MSEdge* const e, const SUMOVehicle* const v, double) {
    const int id = e->getNumericalID();
    if (id < (int)myEdgeSpeeds.size()) {
        return MAX2(e->getLength() / MAX2(myEdgeSpeeds[id], NUMERICAL_EPS), e->getMinimumTravelTime(v));
    }
    return e->getMinimumTravelTime(v);
}


double
MSRoutingEngine::getEffortBike(const MSEdge* const e, const SUMOVehicle* const v, double) {
    const int id = e->getNumericalID();
    if (id < (int)myEdgeBikeSpeeds.size()) {
        return MAX2(e->getLength() / MAX2(myEdgeBikeSpeeds[id], NUMERICAL_EPS), e->getMinimumTravelTime(v));
    }
    return e->getMinimumTravelTime(v);
}



double
MSRoutingEngine::getEffortExtra(const MSEdge* const e, const SUMOVehicle* const v, double t) {
    double effort = (!myBikeSpeeds || v == nullptr || v->getVClass() != SVC_BICYCLE
                     ? getEffort(e, v, t)
                     : getEffortBike(e, v, t));
    if (gWeightsRandomFactor != 1.) {
        effort *= RandHelper::rand(1., gWeightsRandomFactor);
    }
    if (myPriorityFactor != 0) {
        // lower priority should result in higher effort (and the edge with
        // minimum priority receives a factor of myPriorityFactor
        const double relativeInversePrio = 1 - ((e->getPriority() - myMinEdgePriority) / myEdgePriorityRange);
        effort *= 1 + relativeInversePrio * myPriorityFactor;
    }
    return effort;
}


double
MSRoutingEngine::getAssumedSpeed(const MSEdge* edge, const SUMOVehicle* veh) {
    return edge->getLength() / myEffortFunc(edge, veh, 0);
}


SUMOTime
MSRoutingEngine::adaptEdgeEfforts(SUMOTime currentTime) {
    initEdgeWeights(SVC_PASSENGER);
    if (myBikeSpeeds) {
        initEdgeWeights(SVC_BICYCLE);
    }
    if (MSNet::getInstance()->getVehicleControl().getDepartedVehicleNo() == 0) {
        return myAdaptationInterval;
    }
    std::map<std::pair<const MSEdge*, const MSEdge*>, const MSRoute*>::iterator it = myCachedRoutes.begin();
    for (; it != myCachedRoutes.end(); ++it) {
        it->second->release();
    }
    myCachedRoutes.clear();
    const MSEdgeVector& edges = MSNet::getInstance()->getEdgeControl().getEdges();
    if (myAdaptationSteps > 0) {
        // moving average
        for (MSEdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            if ((*i)->isDelayed()) {
                const int id = (*i)->getNumericalID();
                const double currSpeed = (*i)->getMeanSpeed();
                myEdgeSpeeds[id] += (currSpeed - myPastEdgeSpeeds[id][myAdaptationStepsIndex]) / myAdaptationSteps;
                myPastEdgeSpeeds[id][myAdaptationStepsIndex] = currSpeed;
            }
        }
        if (myBikeSpeeds) {
            for (MSEdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
                if ((*i)->isDelayed()) {
                    const int id = (*i)->getNumericalID();
                    const double currSpeed = (*i)->getMeanSpeedBike();
                    myEdgeBikeSpeeds[id] += (currSpeed - myPastEdgeBikeSpeeds[id][myAdaptationStepsIndex]) / myAdaptationSteps;
                    myPastEdgeBikeSpeeds[id][myAdaptationStepsIndex] = currSpeed;
                }
            }
        }
        myAdaptationStepsIndex = (myAdaptationStepsIndex + 1) % myAdaptationSteps;
    } else {
        // exponential moving average
        const double newWeightFactor = (double)(1. - myAdaptationWeight);
        for (MSEdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            if ((*i)->isDelayed()) {
                const int id = (*i)->getNumericalID();
                const double currSpeed = (*i)->getMeanSpeed();
                if (currSpeed != myEdgeSpeeds[id]) {
                    myEdgeSpeeds[id] = myEdgeSpeeds[id] * myAdaptationWeight + currSpeed * newWeightFactor;
                }
            }
        }
        if (myBikeSpeeds) {
            for (MSEdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
                if ((*i)->isDelayed()) {
                    const int id = (*i)->getNumericalID();
                    const double currSpeed = (*i)->getMeanSpeedBike();
                    if (currSpeed != myEdgeBikeSpeeds[id]) {
                        myEdgeBikeSpeeds[id] = myEdgeBikeSpeeds[id] * myAdaptationWeight + currSpeed * newWeightFactor;
                    }
                }
            }
        }
    }
    myLastAdaptation = currentTime + DELTA_T; // because we run at the end of the time step
    if (OptionsCont::getOptions().isSet("device.rerouting.output")) {
        OutputDevice& dev = OutputDevice::getDeviceByOption("device.rerouting.output");
        dev.openTag(SUMO_TAG_INTERVAL);
        dev.writeAttr(SUMO_ATTR_ID, "device.rerouting");
        dev.writeAttr(SUMO_ATTR_BEGIN, STEPS2TIME(currentTime));
        dev.writeAttr(SUMO_ATTR_END, STEPS2TIME(currentTime + myAdaptationInterval));
        for (const MSEdge* e : edges) {
            dev.openTag(SUMO_TAG_EDGE);
            dev.writeAttr(SUMO_ATTR_ID, e->getID());
            dev.writeAttr("traveltime", myEffortFunc(e, nullptr, STEPS2TIME(currentTime)));
            if (myBikeSpeeds) {
                // @note edge-priority is not included here
                dev.writeAttr("traveltimeBike", getEffortBike(e, nullptr, STEPS2TIME(currentTime)));
            }
            dev.closeTag();
        }
        dev.closeTag();
    }
    return myAdaptationInterval;
}


const MSRoute*
MSRoutingEngine::getCachedRoute(const std::pair<const MSEdge*, const MSEdge*>& key) {
    auto routeIt = myCachedRoutes.find(key);
    if (routeIt != myCachedRoutes.end()) {
        return routeIt->second;
    }
    return nullptr;
}


void
MSRoutingEngine::initRouter(SUMOVehicle* vehicle) {
    OptionsCont& oc = OptionsCont::getOptions();
    const std::string routingAlgorithm = oc.getString("routing-algorithm");
    myBikeSpeeds = oc.getBool("device.rerouting.bike-speeds");
    myEffortFunc = ((gWeightsRandomFactor != 1 || myPriorityFactor != 0 || myBikeSpeeds) ? &MSRoutingEngine::getEffortExtra : &MSRoutingEngine::getEffort);

    SUMOAbstractRouter<MSEdge, SUMOVehicle>* router = nullptr;
    if (routingAlgorithm == "dijkstra") {
        router = new DijkstraRouter<MSEdge, SUMOVehicle>(MSEdge::getAllEdges(), true, myEffortFunc, nullptr, false, nullptr, true);
    } else if (routingAlgorithm == "astar") {
        typedef AStarRouter<MSEdge, SUMOVehicle> AStar;
        std::shared_ptr<const AStar::LookupTable> lookup = nullptr;
        if (oc.isSet("astar.all-distances")) {
            lookup = std::make_shared<const AStar::FLT>(oc.getString("astar.all-distances"), (int)MSEdge::getAllEdges().size());
        } else if (oc.isSet("astar.landmark-distances") && vehicle != nullptr) {
            const double speedFactor = vehicle->getChosenSpeedFactor();
            // we need an exemplary vehicle with speedFactor 1
            vehicle->setChosenSpeedFactor(1);
            CHRouterWrapper<MSEdge, SUMOVehicle> chrouter(
                MSEdge::getAllEdges(), true, &MSNet::getTravelTime,
                string2time(oc.getString("begin")), string2time(oc.getString("end")), SUMOTime_MAX, 1);
            lookup = std::make_shared<const AStar::LMLT>(oc.getString("astar.landmark-distances"), MSEdge::getAllEdges(), &chrouter,
                     nullptr, vehicle, "", oc.getInt("device.rerouting.threads"));
            vehicle->setChosenSpeedFactor(speedFactor);
        }
        router = new AStar(MSEdge::getAllEdges(), true, myEffortFunc, lookup, true);
    } else if (routingAlgorithm == "CH") {
        const SUMOTime weightPeriod = myAdaptationInterval > 0 ? myAdaptationInterval : SUMOTime_MAX;
        router = new CHRouter<MSEdge, SUMOVehicle>(
            MSEdge::getAllEdges(), true, myEffortFunc, vehicle == nullptr ? SVC_PASSENGER : vehicle->getVClass(), weightPeriod, true, false);
    } else if (routingAlgorithm == "CHWrapper") {
        const SUMOTime weightPeriod = myAdaptationInterval > 0 ? myAdaptationInterval : SUMOTime_MAX;
        router = new CHRouterWrapper<MSEdge, SUMOVehicle>(
            MSEdge::getAllEdges(), true, myEffortFunc,
            string2time(oc.getString("begin")), string2time(oc.getString("end")), weightPeriod, oc.getInt("device.rerouting.threads"));
    } else {
        throw ProcessError("Unknown routing algorithm '" + routingAlgorithm + "'!");
    }

    RailwayRouter<MSEdge, SUMOVehicle>* railRouter = nullptr;
    if (MSNet::getInstance()->hasBidiEdges()) {
        railRouter = new RailwayRouter<MSEdge, SUMOVehicle>(MSEdge::getAllEdges(), true, myEffortFunc, nullptr, false, true, false, oc.getFloat("railway.max-train-length"));
    }
    myRouterProvider = new MSRouterProvider(router, nullptr, nullptr, railRouter);
#ifdef HAVE_FOX
    FXWorkerThread::Pool& threadPool = MSNet::getInstance()->getEdgeControl().getThreadPool();
    if (threadPool.size() > 0) {
        const std::vector<FXWorkerThread*>& threads = threadPool.getWorkers();
        if (static_cast<MSEdgeControl::WorkerThread*>(threads.front())->setRouterProvider(myRouterProvider)) {
            for (std::vector<FXWorkerThread*>::const_iterator t = threads.begin() + 1; t != threads.end(); ++t) {
                static_cast<MSEdgeControl::WorkerThread*>(*t)->setRouterProvider(myRouterProvider->clone());
            }
        }
    }
#endif
}


void
MSRoutingEngine::reroute(SUMOVehicle& vehicle, const SUMOTime currentTime, const std::string& info,
                         const bool onInit, const bool silent, const MSEdgeVector& prohibited) {
    if (myRouterProvider == nullptr) {
        initRouter(&vehicle);
    }
    auto& router = myRouterProvider->getVehicleRouter(vehicle.getVClass());
#ifdef HAVE_FOX
    FXWorkerThread::Pool& threadPool = MSNet::getInstance()->getEdgeControl().getThreadPool();
    if (threadPool.size() > 0) {
        threadPool.add(new RoutingTask(vehicle, currentTime, info, onInit, silent, prohibited));
        return;
    }
#endif
    if (!prohibited.empty()) {
        router.prohibit(prohibited);
    }
    try {
        vehicle.reroute(currentTime, info, router, onInit, myWithTaz, silent);
    } catch (ProcessError&) {
        if (!silent) {
            if (!prohibited.empty()) {
                router.prohibit(MSEdgeVector());
            }
            throw;
        }
    }
    if (!prohibited.empty()) {
        router.prohibit(MSEdgeVector());
    }
}


void
MSRoutingEngine::setEdgeTravelTime(const MSEdge* const edge, const double travelTime) {
    myEdgeSpeeds[edge->getNumericalID()] = edge->getLength() / travelTime;
}


SUMOAbstractRouter<MSEdge, SUMOVehicle>&
MSRoutingEngine::getRouterTT(const int rngIndex, SUMOVehicleClass svc, const MSEdgeVector& prohibited) {
    if (myRouterProvider == nullptr) {
        initWeightUpdate();
        initEdgeWeights(svc);
        initRouter();
    }
#ifdef HAVE_FOX
    FXWorkerThread::Pool& threadPool = MSNet::getInstance()->getEdgeControl().getThreadPool();
    if (threadPool.size() > 0) {
        auto& router = static_cast<MSEdgeControl::WorkerThread*>(threadPool.getWorkers()[rngIndex % MSGlobals::gNumThreads])->getRouter(svc);
        router.prohibit(prohibited);
        return router;
    }
#endif
    myRouterProvider->getVehicleRouter(svc).prohibit(prohibited);
    return myRouterProvider->getVehicleRouter(svc);
}


void
MSRoutingEngine::cleanup() {
    myAdaptationInterval = -1; // responsible for triggering initEdgeWeights
    myPastEdgeSpeeds.clear();
    myEdgeSpeeds.clear();
    myPastEdgeBikeSpeeds.clear();
    myEdgeBikeSpeeds.clear();
    // @todo recheck. calling release crashes in parallel routing
    //for (auto& item : myCachedRoutes) {
    //    item.second->release();
    //}
    myCachedRoutes.clear();
    myAdaptationStepsIndex = 0;
#ifdef HAVE_FOX
    if (MSGlobals::gNumThreads > 1) {
        // router deletion is done in thread destructor
        myRouterProvider = nullptr;
        return;
    }
#endif
    delete myRouterProvider;
    myRouterProvider = nullptr;
}


#ifdef HAVE_FOX
void
MSRoutingEngine::waitForAll() {
    FXWorkerThread::Pool& threadPool = MSNet::getInstance()->getEdgeControl().getThreadPool();
    if (threadPool.size() > 0) {
        threadPool.waitAll();
    }
}


// ---------------------------------------------------------------------------
// MSRoutingEngine::RoutingTask-methods
// ---------------------------------------------------------------------------
void
MSRoutingEngine::RoutingTask::run(FXWorkerThread* context) {
    SUMOAbstractRouter<MSEdge, SUMOVehicle>& router = static_cast<MSEdgeControl::WorkerThread*>(context)->getRouter(myVehicle.getVClass());
    if (!myProhibited.empty()) {
        router.prohibit(myProhibited);
    }
    try {
        myVehicle.reroute(myTime, myInfo, router, myOnInit, myWithTaz, mySilent);
    } catch (ProcessError&) {
        if (!mySilent) {
            if (!myProhibited.empty()) {
                router.prohibit(MSEdgeVector());
            }
            throw;
        }
    }
    if (!myProhibited.empty()) {
        router.prohibit(MSEdgeVector());
    }
    const MSEdge* source = *myVehicle.getRoute().begin();
    const MSEdge* dest = myVehicle.getRoute().getLastEdge();
    if (source->isTazConnector() && dest->isTazConnector()) {
        const std::pair<const MSEdge*, const MSEdge*> key = std::make_pair(source, dest);
        FXMutexLock lock(myRouteCacheMutex);
        if (MSRoutingEngine::myCachedRoutes.find(key) == MSRoutingEngine::myCachedRoutes.end()) {
            MSRoutingEngine::myCachedRoutes[key] = &myVehicle.getRoute();
            myVehicle.getRoute().addReference();
        }
    }
}
#endif


/****************************************************************************/
