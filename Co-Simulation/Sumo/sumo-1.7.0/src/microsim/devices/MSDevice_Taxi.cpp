/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_Taxi.cpp
/// @author  Jakob Erdmann
/// @date    16.12.2019
///
// A device which controls a taxi
/****************************************************************************/
#include <config.h>

#include <utils/common/StringUtils.h>
#include <utils/common/StaticCommand.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSEdge.h>

#include "MSDispatch.h"
#include "MSDispatch_Greedy.h"
#include "MSDispatch_GreedyShared.h"
#include "MSDispatch_RouteExtension.h"
#include "MSDispatch_TraCI.h"

#include "MSIdling.h"

#include "MSRoutingEngine.h"
#include "MSDevice_Taxi.h"

// ===========================================================================
// static member variables
// ===========================================================================
SUMOTime MSDevice_Taxi::myDispatchPeriod(0);
/// @brief the dispatch algorithm
MSDispatch* MSDevice_Taxi::myDispatcher(nullptr);
/// @brief The repeated call to the dispatcher
Command* MSDevice_Taxi::myDispatchCommand(nullptr);
// @brief the list of available taxis
std::vector<MSDevice_Taxi*> MSDevice_Taxi::myFleet;

#define TAXI_SERVICE "taxi"

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_Taxi::insertOptions(OptionsCont& oc) {
    oc.addOptionSubTopic("Taxi Device");
    insertDefaultAssignmentOptions("taxi", "Taxi Device", oc);

    oc.doRegister("device.taxi.dispatch-algorithm", new Option_String("greedy"));
    oc.addDescription("device.taxi.dispatch-algorithm", "Taxi Device", "The dispatch algorithm [greedy|greedyClosest|greedyShared|routeExtension|traci]");

    oc.doRegister("device.taxi.dispatch-algorithm.output", new Option_String(""));
    oc.addDescription("device.taxi.dispatch-algorithm.output", "Taxi Device", "Write information from the dispatch algorithm to FILE");

    oc.doRegister("device.taxi.dispatch-algorithm.params", new Option_String(""));
    oc.addDescription("device.taxi.dispatch-algorithm.params", "Taxi Device", "Load dispatch algorithm parameters in format KEY1:VALUE1[,KEY2:VALUE]");

    oc.doRegister("device.taxi.dispatch-period", new Option_String("60", "TIME"));
    oc.addDescription("device.taxi.dispatch-period", "Taxi Device", "The period between successive calls to the dispatcher");

    oc.doRegister("device.taxi.idle-algorithm", new Option_String("stop"));
    oc.addDescription("device.taxi.idle-algorithm", "Taxi Device", "The behavior of idle taxis [stop|randomCircling]");

    oc.doRegister("device.taxi.idle-algorithm.output", new Option_String(""));
    oc.addDescription("device.taxi.idle-algorithm.output", "Taxi Device", "Write information from the idling algorithm to FILE");
}


void
MSDevice_Taxi::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "taxi", v, false)) {
        if (MSGlobals::gUseMesoSim) {
            WRITE_WARNING("Mesoscopic simulation does not support the taxi device yet.");
            return;
        }
        // build the device
        MSDevice_Taxi* device = new MSDevice_Taxi(v, "taxi_" + v.getID());
        into.push_back(device);
        myFleet.push_back(device);
        if (v.getParameter().line == "") {
            // automatically set the line so that persons are willing to enter
            // (see MSStageDriving::isWaitingFor)
            const_cast<SUMOVehicleParameter&>(v.getParameter()).line = TAXI_SERVICE;
        }
        if (v.getVClass() != SVC_TAXI) {
            WRITE_WARNING("Vehicle '" + v.getID() + "' with device.taxi should have vClass taxi instead of '" + toString(v.getVClass()) + "'.");
        }
    }
}

void
MSDevice_Taxi::initDispatch() {
    OptionsCont& oc = OptionsCont::getOptions();
    myDispatchPeriod = string2time(oc.getString("device.taxi.dispatch-period"));
    // init dispatch algorithm
    std::string algo = oc.getString("device.taxi.dispatch-algorithm");
    Parameterised params;
    params.setParametersStr(OptionsCont::getOptions().getString("device.taxi.dispatch-algorithm.params"), ":", ",");
    if (algo == "greedy") {
        myDispatcher = new MSDispatch_Greedy(params.getParametersMap());
    } else if (algo == "greedyClosest") {
        myDispatcher = new MSDispatch_GreedyClosest(params.getParametersMap());
    } else if (algo == "greedyShared") {
        myDispatcher = new MSDispatch_GreedyShared(params.getParametersMap());
    } else if (algo == "routeExtension") {
        myDispatcher = new MSDispatch_RouteExtension(params.getParametersMap());
    } else if (algo == "traci") {
        myDispatcher = new MSDispatch_TraCI(params.getParametersMap());
    } else {
        throw ProcessError("Dispatch algorithm '" + algo + "' is not known");
    }
    myDispatchCommand = new StaticCommand<MSDevice_Taxi>(&MSDevice_Taxi::triggerDispatch);
    // round to next multiple of myDispatchPeriod
    const SUMOTime now = MSNet::getInstance()->getCurrentTimeStep();
    const SUMOTime begin = string2time(oc.getString("begin"));
    const SUMOTime delay = (myDispatchPeriod - ((now - begin) % myDispatchPeriod)) % myDispatchPeriod;
    MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(myDispatchCommand, now + delay);
}

void
MSDevice_Taxi::addReservation(MSTransportable* person,
                              const std::set<std::string>& lines,
                              SUMOTime reservationTime,
                              SUMOTime pickupTime,
                              const MSEdge* from, double fromPos,
                              const MSEdge* to, double toPos,
                              const std::string& group) {
    if (lines.size() == 1 && *lines.begin() == TAXI_SERVICE) {
        if (myDispatchCommand == nullptr) {
            initDispatch();
        }
        myDispatcher->addReservation(person, reservationTime, pickupTime, from, fromPos, to, toPos, group);
    }
}


SUMOTime
MSDevice_Taxi::triggerDispatch(SUMOTime currentTime) {
    std::vector<MSDevice_Taxi*> active;
    for (MSDevice_Taxi* taxi : myFleet) {
        if (taxi->getHolder().hasDeparted()) {
            active.push_back(taxi);
        }
    }
    myDispatcher->computeDispatch(currentTime, active);
    return myDispatchPeriod;
}

bool
MSDevice_Taxi::hasServableReservations() {
    return myDispatcher != nullptr && myDispatcher->hasServableReservations();
}

void
MSDevice_Taxi::cleanup() {
    if (myDispatcher != nullptr) {
        delete myDispatcher;
        myDispatcher = nullptr;
    }
    myDispatchCommand = nullptr;
}

// ---------------------------------------------------------------------------
// MSDevice_Taxi-methods
// ---------------------------------------------------------------------------
MSDevice_Taxi::MSDevice_Taxi(SUMOVehicle& holder, const std::string& id) :
    MSVehicleDevice(holder, id) {
    std::string defaultServiceEnd = toString(1e15);
    const std::string algo = getStringParam(holder, OptionsCont::getOptions(), "taxi.idle-algorithm", "", false);
    if (algo == "stop") {
        myIdleAlgorithm = new MSIdling_Stop();
    } else if (algo == "randomCircling") {
        myIdleAlgorithm = new MSIdling_RandomCircling();
        // make sure simulation terminates
        defaultServiceEnd = toString(STEPS2TIME(
                                         myHolder.getParameter().departProcedure == DEPART_GIVEN
                                         ? myHolder.getParameter().depart
                                         : MSNet::getInstance()->getCurrentTimeStep()) + (3600 * 8));
    } else {
        throw ProcessError("Idle algorithm '" + algo + "' is not known for vehicle '" + myHolder.getID() + "'");
    }
    myServiceEnd = string2time(getStringParam(holder, OptionsCont::getOptions(), "taxi.end", defaultServiceEnd, false));
}


MSDevice_Taxi::~MSDevice_Taxi() {
    myFleet.erase(std::find(myFleet.begin(), myFleet.end(), this));
}


SUMOVehicle*
MSDevice_Taxi::getTaxi() {
    if (myFleet.size() > 0) {
        return &myFleet[0]->getHolder();
    } else {
        return nullptr;
    }
}


void
MSDevice_Taxi::dispatch(const Reservation& res) {
    dispatchShared({&res, &res});
}


void
MSDevice_Taxi::dispatchShared(const std::vector<const Reservation*>& reservations) {
    if (isEmpty()) {
        const SUMOTime t = MSNet::getInstance()->getCurrentTimeStep();
        myHolder.abortNextStop();
        ConstMSEdgeVector tmpEdges({ myHolder.getEdge() });
        std::vector<SUMOVehicleParameter::Stop> stops;
        double lastPos = myHolder.getPositionOnLane();
        for (const Reservation* res : reservations) {
            bool isPickup = false;
            for (MSTransportable* person : res->persons) {
                if (myCustomers.count(person) == 0) {
                    myCustomers.insert(person);
                    isPickup = true;
                }
            }
            if (isPickup) {
                prepareStop(tmpEdges, stops, lastPos, res->from, res->fromPos, "pickup " + toString(res->persons));
                stops.back().triggered = true;
                //stops.back().awaitedPersons.insert(res.person->getID());
            } else {
                prepareStop(tmpEdges, stops, lastPos, res->to, res->toPos, "dropOff " + toString(res->persons));
                stops.back().duration = TIME2STEPS(60); // pay and collect bags
            }
        }
        myHolder.replaceRouteEdges(tmpEdges, -1, 0, "taxi:prepare_dispatch", false, false, false);
        for (SUMOVehicleParameter::Stop& stop : stops) {
            std::string error;
            myHolder.addStop(stop, error);
            if (error != "") {
                WRITE_WARNINGF("Could not add taxi stop for vehicle '%' to %. time=% error=%", myHolder.getID(), stop.actType, time2string(t), error)
            }
        }
        SUMOAbstractRouter<MSEdge, SUMOVehicle>& router = MSRoutingEngine::getRouterTT(myHolder.getRNGIndex(), myHolder.getVClass());
        // SUMOAbstractRouter<MSEdge, SUMOVehicle>& router = myHolder.getInfluencer().getRouterTT(veh->getRNGIndex())
        myHolder.reroute(t, "taxi:dispatch", router, false);
    } else {
        throw ProcessError("Dispatch for busy taxis not yet implemented");
    }
    myState = PICKUP;
}


void
MSDevice_Taxi::prepareStop(ConstMSEdgeVector& edges,
                           std::vector<SUMOVehicleParameter::Stop>& stops,
                           double& lastPos, const MSEdge* stopEdge, double stopPos,
                           const std::string& action) {
    assert(!edges.empty());
    if (stopEdge == edges.back() && !stops.empty()) {
        if (stopPos >= lastPos && stopPos <= stops.back().endPos) {
            // no new stop and no adaption needed
            return;
        }
        if (stopPos >= lastPos && stopPos <= lastPos + myHolder.getVehicleType().getLength()) {
            // stop length adaption needed
            stops.back().endPos = MIN2(lastPos + myHolder.getVehicleType().getLength(), stopEdge->getLength());
            return;
        }
    }
    if (stopEdge != edges.back() || stopPos < lastPos) {
        edges.push_back(stopEdge);
    }
    lastPos = stopPos;
    SUMOVehicleParameter::Stop stop;
    stop.lane = getStopLane(stopEdge)->getID();
    stop.startPos = stopPos;
    stop.endPos = MAX2(stopPos, MIN2(myHolder.getVehicleType().getLength(), stopEdge->getLength()));
    stop.parking = true;
    stop.actType = action;
    stop.index = STOP_INDEX_END;
    stops.push_back(stop);
}


MSLane*
MSDevice_Taxi::getStopLane(const MSEdge* edge) {
    const std::vector<MSLane*>* allowedLanes = edge->allowedLanes(myHolder.getVClass());
    if (allowedLanes == nullptr) {
        throw ProcessError("Taxi '" + myHolder.getID() + "' cannot pick up person on edge '" + edge->getID() + "'");
    }
    return allowedLanes->front();
}

bool
MSDevice_Taxi::isEmpty() {
    return myState == EMPTY;
}


bool
MSDevice_Taxi::allowsBoarding(MSTransportable* t) const {
    return myCustomers.count(t) != 0;
}


bool
MSDevice_Taxi::notifyMove(SUMOTrafficObject& /*tObject*/, double oldPos,
                          double newPos, double /*newSpeed*/) {
    if (myHolder.getPersonNumber() > 0) {
        myOccupiedDistance += (newPos - oldPos);
        myOccupiedTime += DELTA_T;
    }
    if (isEmpty() && MSNet::getInstance()->getCurrentTimeStep() < myServiceEnd) {
        myIdleAlgorithm->idle(this);
    }
    if (myHolder.isStopped()) {
        if (!myIsStopped) {
            // limit duration of stop
            // @note: stops are not yet added to the vehicle so we can change the loaded parameters. Stops added from a route are not affected
            MSVehicle& veh = static_cast<MSVehicle&>(myHolder);
            veh.getNextStop().endBoarding = myServiceEnd;
        }
    }
    myIsStopped = myHolder.isStopped();
    return true; // keep the device
}


bool
MSDevice_Taxi::notifyEnter(SUMOTrafficObject& /*veh*/, MSMoveReminder::Notification /*reason*/, const MSLane* /* enteredLane */) {
    if (isEmpty() && MSNet::getInstance()->getCurrentTimeStep() < myServiceEnd) {
        myIdleAlgorithm->idle(this);
    }
    return true; // keep the device
}


bool
MSDevice_Taxi::notifyLeave(SUMOTrafficObject& /*veh*/, double /*lastPos*/, MSMoveReminder::Notification /*reason*/, const MSLane* /* enteredLane */) {
    return true; // keep the device
}

void
MSDevice_Taxi::customerEntered(const MSTransportable* /*t*/) {
    myState |= OCCUPIED;
    if (!hasFuturePickup()) {
        myState &= ~PICKUP;
    }
}


void
MSDevice_Taxi::customerArrived(const MSTransportable* person) {
    myCustomersServed++;
    myCustomers.erase(person);
    if (myHolder.getPersonNumber() == 0) {
        myState &= ~OCCUPIED;
        MSVehicle* veh = static_cast<MSVehicle*>(&myHolder);
        if (veh->getStops().size() > 1 && (myState & PICKUP) == 0) {
            WRITE_WARNINGF("All customers left vehicle '%' at time % but there are % remaining stops",
                           veh->getID(), time2string(MSNet::getInstance()->getCurrentTimeStep()), veh->getStops().size() - 1);
            while (veh->getStops().size() > 1) {
                veh->abortNextStop(1);
            }
        }
    }
}

bool
MSDevice_Taxi::hasFuturePickup() {
    MSVehicle* veh = static_cast<MSVehicle*>(&myHolder);
    for (const auto& stop : veh->getStops()) {
        if (stop.reached) {
            continue;
        }
        if (StringUtils::startsWith(stop.pars.actType, "pickup")) {
            return true;
        }
    }
    return false;
}

void
MSDevice_Taxi::generateOutput(OutputDevice* tripinfoOut) const {
    if (tripinfoOut != nullptr) {
        tripinfoOut->openTag("taxi");
        tripinfoOut->writeAttr("customers", toString(myCustomersServed));
        tripinfoOut->writeAttr("occupiedDistance", toString(myOccupiedDistance));
        tripinfoOut->writeAttr("occupiedTime", time2string(myOccupiedTime));
        tripinfoOut->closeTag();
    }
}

std::string
MSDevice_Taxi::getParameter(const std::string& key) const {
    if (key == "customers") {
        return toString(myCustomersServed);
    } else if (key == "occupiedDistance") {
        return toString(myOccupiedDistance);
    } else if (key == "occupiedTime") {
        return toString(STEPS2TIME(myOccupiedTime));
    } else if (key == "state") {
        return toString(myState);
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


void
MSDevice_Taxi::setParameter(const std::string& key, const std::string& value) {
    double doubleValue;
    try {
        doubleValue = StringUtils::toDouble(value);
    } catch (NumberFormatException&) {
        throw InvalidArgument("Setting parameter '" + key + "' requires a number for device of type '" + deviceName() + "'");
    }
    UNUSED_PARAMETER(doubleValue);
    throw InvalidArgument("Setting parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


/****************************************************************************/
