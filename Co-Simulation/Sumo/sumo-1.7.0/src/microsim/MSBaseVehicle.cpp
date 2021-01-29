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
/// @file    MSBaseVehicle.cpp
/// @author  Michael Behrisch
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Mon, 8 Nov 2010
///
// A base class for vehicle implementations
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <cassert>
#include <utils/common/StdDefs.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/emissions/HelpersHarmonoise.h>
#include <microsim/transportables/MSPerson.h>
#include "MSGlobals.h"
#include "MSVehicleControl.h"
#include "MSVehicleType.h"
#include "MSEdge.h"
#include "MSLane.h"
#include "MSMoveReminder.h"
#include "MSEdgeWeightsStorage.h"
#include "MSBaseVehicle.h"
#include "MSNet.h"
#include "devices/MSDevice.h"
#include "devices/MSDevice_Routing.h"
#include "devices/MSDevice_Battery.h"
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/devices/MSRoutingEngine.h>
#include <microsim/devices/MSDevice_Transportable.h>
#include <microsim/devices/MSDevice_Battery.h>
#include <microsim/devices/MSDevice_ElecHybrid.h>
#include <microsim/devices/MSDevice_Taxi.h>
#include "MSInsertionControl.h"

//#define DEBUG_REROUTE
//#define DEBUG_COND (getID() == "follower")
//#define DEBUG_COND (true)
#define DEBUG_COND (isSelected())

// ===========================================================================
// static members
// ===========================================================================
const SUMOTime MSBaseVehicle::NOT_YET_DEPARTED = SUMOTime_MAX;
std::vector<MSTransportable*> MSBaseVehicle::myEmptyTransportableVector;
#ifdef _DEBUG
std::set<std::string> MSBaseVehicle::myShallTraceMoveReminders;
#endif
SUMOVehicle::NumericalID MSBaseVehicle::myCurrentNumericalIndex = 0;

// ===========================================================================
// Influencer method definitions
// ===========================================================================

MSBaseVehicle::BaseInfluencer::BaseInfluencer() :
    myRoutingMode(0)
{}

SUMOAbstractRouter<MSEdge, SUMOVehicle>&
MSBaseVehicle::BaseInfluencer::getRouterTT(const int rngIndex, SUMOVehicleClass svc) const {
    if (myRoutingMode == 1) {
        return MSRoutingEngine::getRouterTT(rngIndex, svc);
    } else {
        return MSNet::getInstance()->getRouterTT(rngIndex);
    }
}



// ===========================================================================
// method definitions
// ===========================================================================

double
MSBaseVehicle::getPreviousSpeed() const {
    throw ProcessError("getPreviousSpeed() is not available for non-MSVehicles.");
}


MSBaseVehicle::MSBaseVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
                             MSVehicleType* type, const double speedFactor) :
    SUMOVehicle(pars->id),
    myParameter(pars),
    myRoute(route),
    myType(type),
    myCurrEdge(route->begin()),
    myChosenSpeedFactor(pars->speedFactor < 0 ? speedFactor : pars->speedFactor),
    myMoveReminders(0),
    myPersonDevice(nullptr),
    myContainerDevice(nullptr),
    myDeparture(NOT_YET_DEPARTED),
    myDepartPos(-1),
    myArrivalPos(-1),
    myArrivalLane(-1),
    myNumberReroutes(0),
    myStopUntilOffset(0),
    myOdometer(0.),
    myRouteValidity(ROUTE_UNCHECKED),
    myNumericalID(myCurrentNumericalIndex++),
    myEdgeWeights(nullptr)
#ifdef _DEBUG
    , myTraceMoveReminders(myShallTraceMoveReminders.count(pars->id) > 0)
#endif
{
    if ((*myRoute->begin())->isTazConnector() || myRoute->getLastEdge()->isTazConnector()) {
        pars->parametersSet |= VEHPARS_FORCE_REROUTE;
    }
    if (!pars->wasSet(VEHPARS_FORCE_REROUTE)) {
        calculateArrivalParams();
    }
    myRoute->addReference();
}


MSBaseVehicle::~MSBaseVehicle() {
    delete myEdgeWeights;
    myRoute->release();
    if (myParameter->repetitionNumber == 0) {
        MSRoute::checkDist(myParameter->routeid);
    }
    for (MSVehicleDevice* dev : myDevices) {
        delete dev;
    }
    delete myParameter;
}


void
MSBaseVehicle::initDevices() {
    MSDevice::buildVehicleDevices(*this, myDevices);
    for (MSVehicleDevice* dev : myDevices) {
        myMoveReminders.push_back(std::make_pair(dev, 0.));
    }
}


void
MSBaseVehicle::setID(const std::string& /*newID*/) {
    throw ProcessError("Changing a vehicle ID is not permitted");
}

const SUMOVehicleParameter&
MSBaseVehicle::getParameter() const {
    return *myParameter;
}

const std::map<int, double>*
MSBaseVehicle::getEmissionParameters() const {
    MSDevice_Battery* batteryDevice = static_cast<MSDevice_Battery*>(getDevice(typeid(MSDevice_Battery)));
    if (batteryDevice != nullptr) {
        return &batteryDevice->getEnergyParams();
    } else {
        return nullptr;
    }
}

void
MSBaseVehicle::replaceParameter(const SUMOVehicleParameter* newParameter) {
    delete myParameter;
    myParameter = newParameter;
}

double
MSBaseVehicle::getMaxSpeed() const {
    return myType->getMaxSpeed();
}


const MSEdge*
MSBaseVehicle::succEdge(int nSuccs) const {
    if (myCurrEdge + nSuccs < myRoute->end() && std::distance(myCurrEdge, myRoute->begin()) <= nSuccs) {
        return *(myCurrEdge + nSuccs);
    } else {
        return nullptr;
    }
}


const MSEdge*
MSBaseVehicle::getEdge() const {
    return *myCurrEdge;
}


void
MSBaseVehicle::reroute(SUMOTime t, const std::string& info, SUMOAbstractRouter<MSEdge, SUMOVehicle>& router, const bool onInit, const bool withTaz, const bool silent) {
    // check whether to reroute
    const MSEdge* source = withTaz && onInit ? MSEdge::dictionary(myParameter->fromTaz + "-source") : getRerouteOrigin();
    if (source == nullptr) {
        source = getRerouteOrigin();
    }
    const MSEdge* sink = withTaz ? MSEdge::dictionary(myParameter->toTaz + "-sink") : myRoute->getLastEdge();
    if (sink == nullptr) {
        sink = myRoute->getLastEdge();
    }
    ConstMSEdgeVector oldEdgesRemaining(source == *myCurrEdge ? myCurrEdge : myCurrEdge + 1, myRoute->end());
    ConstMSEdgeVector edges;
    ConstMSEdgeVector stops;
    if (myParameter->via.size() == 0) {
        double firstPos = -1;
        double lastPos = -1;
        stops = getStopEdges(firstPos, lastPos);
        if (stops.size() > 0) {
            const double sourcePos = onInit ? 0 : getPositionOnLane();
            // avoid superfluous waypoints for first and last edge
            const bool skipFirst = stops.front() == source && sourcePos <= firstPos;
            const bool skipLast = stops.back() == sink && myArrivalPos >= lastPos;
#ifdef DEBUG_REROUTE
            if (DEBUG_COND) {
                std::cout << SIMTIME << " reroute " << info << " veh=" << getID() << " lane=" << Named::getIDSecure(getLane())
                          << " source=" << source->getID() << " sourcePos=" << sourcePos << " firstPos=" << firstPos << " arrivalPos=" << myArrivalPos << " lastPos=" << lastPos
                          << " route=" << toString(myRoute->getEdges()) << " stopEdges=" << toString(stops) << " skipFirst=" << skipFirst << " skipLast=" << skipLast << "\n";
            }
#endif
            if (stops.size() == 1 && (skipFirst || skipLast)) {
                stops.clear();
            } else {
                if (skipFirst) {
                    stops.erase(stops.begin());
                }
                if (skipLast) {
                    stops.erase(stops.end() - 1);
                }
            }
        }
    } else {
        // via takes precedence over stop edges
        // XXX check for inconsistencies #2275
        for (std::vector<std::string>::const_iterator it = myParameter->via.begin(); it != myParameter->via.end(); ++it) {
            MSEdge* viaEdge = MSEdge::dictionary(*it);
            if (viaEdge == source || viaEdge == sink) {
                continue;
            }
            assert(viaEdge != 0);
            if (!viaEdge->isTazConnector() && viaEdge->allowedLanes(getVClass()) == nullptr) {
                throw ProcessError("Vehicle '" + getID() + "' is not allowed on any lane of via edge '" + viaEdge->getID() + "'.");
            }
            stops.push_back(viaEdge);
        }
    }

    for (MSRouteIterator s = stops.begin(); s != stops.end(); ++s) {
        // !!! need to adapt t here
        ConstMSEdgeVector into;
        router.computeLooped(source, *s, this, t, into, silent);
        if (into.size() > 0) {
            into.pop_back();
            edges.insert(edges.end(), into.begin(), into.end());
            if ((*s)->isTazConnector()) {
                source = into.back();
                edges.pop_back();
            } else {
                source = *s;
            }
        } else {
            std::string error = "Vehicle '" + getID() + "' has no valid route from edge '" + source->getID() + "' to stop edge '" + (*s)->getID() + "'.";
            if (MSGlobals::gCheckRoutes || silent) {
                throw ProcessError(error);
            } else {
                WRITE_WARNING(error);
                edges.push_back(source);
            }
            source = *s;
        }
    }
    router.compute(source, sink, this, t, edges, silent);
    if (edges.empty() && silent) {
        return;
    }
    if (!edges.empty() && edges.front()->isTazConnector()) {
        edges.erase(edges.begin());
    }
    if (!edges.empty() && edges.back()->isTazConnector()) {
        edges.pop_back();
    }
    const double routeCost = router.recomputeCosts(edges, this, t);
    const double previousCost = onInit ? routeCost : router.recomputeCosts(oldEdgesRemaining, this, t);
    const double savings = previousCost - routeCost;
    //if (getID() == "43") std::cout << SIMTIME << " pCost=" << previousCost << " cost=" << routeCost
    //    << " onInit=" << onInit
    //        << " prevEdges=" << toString(oldEdgesRemaining)
    //        << " newEdges=" << toString(edges)
    //        << "\n";
    replaceRouteEdges(edges, routeCost, savings, info, onInit);
    // this must be called even if the route could not be replaced
    if (onInit) {
        if (edges.empty()) {
            if (MSGlobals::gCheckRoutes) {
                throw ProcessError("Vehicle '" + getID() + "' has no valid route.");
            } else if (source->isTazConnector()) {
                WRITE_WARNING("Removing vehicle '" + getID() + "' which has no valid route.");
                MSNet::getInstance()->getInsertionControl().descheduleDeparture(this);
                return;
            }
        }
        calculateArrivalParams();
    }
}


bool
MSBaseVehicle::replaceRouteEdges(ConstMSEdgeVector& edges, double cost, double savings, const std::string& info, bool onInit, bool check, bool removeStops) {
    if (edges.empty()) {
        WRITE_WARNING("No route for vehicle '" + getID() + "' found.");
        return false;
    }
    // build a new id, first
    std::string id = getID();
    if (id[0] != '!') {
        id = "!" + id;
    }
    const std::string idSuffix = id + "!var#";
    int varIndex = 1;
    id = idSuffix + toString(varIndex);
    while (MSRoute::hasRoute(id)) {
        id = idSuffix + toString(++varIndex);
    }
    int oldSize = (int)edges.size();
    if (!onInit) {
        const MSEdge* const origin = getRerouteOrigin();
        if (origin != *myCurrEdge && edges.front() == origin) {
            edges.insert(edges.begin(), *myCurrEdge);
            oldSize = (int)edges.size();
        }
        edges.insert(edges.begin(), myRoute->begin(), myCurrEdge);
    }
    if (edges == myRoute->getEdges() && !StringUtils::endsWith(info, toString(SUMO_TAG_PARKING_ZONE_REROUTE))) {
        // re-assign stop iterators when rerouting to a new parkingArea
        return true;
    }
    const RGBColor& c = myRoute->getColor();
    MSRoute* newRoute = new MSRoute(id, edges, false, &c == &RGBColor::DEFAULT_COLOR ? nullptr : new RGBColor(c), std::vector<SUMOVehicleParameter::Stop>());
    newRoute->setCosts(cost);
    newRoute->setSavings(savings);
    if (!MSRoute::dictionary(id, newRoute)) {
        delete newRoute;
        return false;
    }

    std::string msg;
    if (check && !hasValidRoute(msg, newRoute)) {
        WRITE_WARNING("Invalid route replacement for vehicle '" + getID() + "'. " + msg);
        if (MSGlobals::gCheckRoutes) {
            newRoute->addReference();
            newRoute->release();
            return false;
        }
    }
    if (!replaceRoute(newRoute, info, onInit, (int)edges.size() - oldSize, false, removeStops)) {
        newRoute->addReference();
        newRoute->release();
        return false;
    }
    return true;
}


double
MSBaseVehicle::getAcceleration() const {
    return 0;
}


double
MSBaseVehicle::getSlope() const {
    return 0;
}


void
MSBaseVehicle::onDepart() {
    myDeparture = MSNet::getInstance()->getCurrentTimeStep();
    myDepartPos = getPositionOnLane();
    MSNet::getInstance()->getVehicleControl().vehicleDeparted(*this);
}


bool
MSBaseVehicle::hasDeparted() const {
    return myDeparture != NOT_YET_DEPARTED;
}


bool
MSBaseVehicle::hasArrived() const {
    return succEdge(1) == nullptr;
}


int
MSBaseVehicle::getRoutePosition() const {
    return (int) std::distance(myRoute->begin(), myCurrEdge);
}


void
MSBaseVehicle::resetRoutePosition(int index, DepartLaneDefinition departLaneProcedure) {
    myCurrEdge = myRoute->begin() + index;
    const_cast<SUMOVehicleParameter*>(myParameter)->departLaneProcedure = departLaneProcedure;
    // !!! hack
    myArrivalPos = (*(myRoute->end() - 1))->getLanes()[0]->getLength();
}

double
MSBaseVehicle::getOdometer() const {
    return -myDepartPos + myOdometer + (hasArrived() ? myArrivalPos : getPositionOnLane());
}

bool
MSBaseVehicle::allowsBoarding(MSTransportable* t) const {
    if (getPersonNumber() >= getVehicleType().getPersonCapacity()) {
        return false;
    }
    MSDevice_Taxi* taxiDevice = static_cast<MSDevice_Taxi*>(getDevice(typeid(MSDevice_Taxi)));
    if (taxiDevice != nullptr) {
        return taxiDevice->allowsBoarding(t);
    }
    return true;
}


void
MSBaseVehicle::addTransportable(MSTransportable* transportable) {
    if (transportable->isPerson()) {
        if (myPersonDevice == nullptr) {
            myPersonDevice = MSDevice_Transportable::buildVehicleDevices(*this, myDevices, false);
            myMoveReminders.push_back(std::make_pair(myPersonDevice, 0.));
            if (myParameter->departProcedure == DEPART_TRIGGERED && myParameter->depart == -1) {
                const_cast<SUMOVehicleParameter*>(myParameter)->depart = MSNet::getInstance()->getCurrentTimeStep();
            }
        }
        myPersonDevice->addTransportable(transportable);
    } else {
        if (myContainerDevice == nullptr) {
            myContainerDevice = MSDevice_Transportable::buildVehicleDevices(*this, myDevices, true);
            myMoveReminders.push_back(std::make_pair(myContainerDevice, 0.));
            if (myParameter->departProcedure == DEPART_CONTAINER_TRIGGERED && myParameter->depart == -1) {
                const_cast<SUMOVehicleParameter*>(myParameter)->depart = MSNet::getInstance()->getCurrentTimeStep();
            }
        }
        myContainerDevice->addTransportable(transportable);
    }
}


bool
MSBaseVehicle::hasValidRoute(std::string& msg, const MSRoute* route) const {
    MSRouteIterator start = myCurrEdge;
    if (route == nullptr) {
        route = myRoute;
    } else {
        start = route->begin();
    }
    MSRouteIterator last = route->end() - 1;
    // check connectivity, first
    for (MSRouteIterator e = start; e != last; ++e) {
        if ((*e)->allowedLanes(**(e + 1), myType->getVehicleClass()) == nullptr) {
            msg = "No connection between edge '" + (*e)->getID() + "' and edge '" + (*(e + 1))->getID() + "'.";
            return false;
        }
    }
    last = route->end();
    // check usable lanes, then
    for (MSRouteIterator e = start; e != last; ++e) {
        if ((*e)->prohibits(this)) {
            msg = "Edge '" + (*e)->getID() + "' prohibits.";
            return false;
        }
    }
    return true;
}


bool
MSBaseVehicle::hasValidRouteStart(std::string& msg) {
    if (myRoute->getEdges().size() > 0 && !myRoute->getEdges().front()->prohibits(this)) {
        myRouteValidity &= ~ROUTE_START_INVALID_PERMISSIONS;
        return true;
    } else {
        msg = "Vehicle '" + getID() + "' is not allowed to depart on its first edge.";
        myRouteValidity |= ROUTE_START_INVALID_PERMISSIONS;
        return false;
    }
}


int
MSBaseVehicle::getRouteValidity(bool update, bool silent) {
    if (!update) {
        return myRouteValidity;
    }
    // insertion check must be done in any case
    std::string msg;
    if (!hasValidRouteStart(msg)) {
        if (MSGlobals::gCheckRoutes) {
            throw ProcessError(msg);
        } else if (!silent) {
            // vehicle will be discarded
            WRITE_WARNING(msg);
        }
    }
    if (MSGlobals::gCheckRoutes
            && (myRouteValidity & ROUTE_UNCHECKED) != 0
            // we could check after the first rerouting
            && (!myParameter->wasSet(VEHPARS_FORCE_REROUTE))) {
        if (!hasValidRoute(msg, myRoute)) {
            myRouteValidity |= ROUTE_INVALID;
            throw ProcessError("Vehicle '" + getID() + "' has no valid route. " + msg);
        }
    }
    myRouteValidity &= ~ROUTE_UNCHECKED;
    return myRouteValidity;
}

void
MSBaseVehicle::addReminder(MSMoveReminder* rem) {
#ifdef _DEBUG
    if (myTraceMoveReminders) {
        traceMoveReminder("add", rem, 0, true);
    }
#endif
    myMoveReminders.push_back(std::make_pair(rem, 0.));
}


void
MSBaseVehicle::removeReminder(MSMoveReminder* rem) {
    for (MoveReminderCont::iterator r = myMoveReminders.begin(); r != myMoveReminders.end(); ++r) {
        if (r->first == rem) {
#ifdef _DEBUG
            if (myTraceMoveReminders) {
                traceMoveReminder("remove", rem, 0, false);
            }
#endif
            myMoveReminders.erase(r);
            return;
        }
    }
}


void
MSBaseVehicle::activateReminders(const MSMoveReminder::Notification reason, const MSLane* enteredLane) {
    for (MoveReminderCont::iterator rem = myMoveReminders.begin(); rem != myMoveReminders.end();) {
        if (rem->first->notifyEnter(*this, reason, enteredLane)) {
#ifdef _DEBUG
            if (myTraceMoveReminders) {
                traceMoveReminder("notifyEnter", rem->first, rem->second, true);
            }
#endif
            ++rem;
        } else {
#ifdef _DEBUG
            if (myTraceMoveReminders) {
                traceMoveReminder("notifyEnter", rem->first, rem->second, false);
            }
#endif
            rem = myMoveReminders.erase(rem);
        }
    }
}


void
MSBaseVehicle::calculateArrivalParams() {
    if (myRoute->getLastEdge()->isTazConnector()) {
        return;
    }
    const std::vector<MSLane*>& lanes = myRoute->getLastEdge()->getLanes();
    const double lastLaneLength = lanes[0]->getLength();
    switch (myParameter->arrivalPosProcedure) {
        case ArrivalPosDefinition::GIVEN:
            if (fabs(myParameter->arrivalPos) > lastLaneLength) {
                WRITE_WARNING("Vehicle '" + getID() + "' will not be able to arrive at the given position!");
            }
            // Maybe we should warn the user about invalid inputs!
            myArrivalPos = MIN2(myParameter->arrivalPos, lastLaneLength);
            if (myArrivalPos < 0) {
                myArrivalPos = MAX2(myArrivalPos + lastLaneLength, 0.);
            }
            break;
        case ArrivalPosDefinition::RANDOM:
            myArrivalPos = RandHelper::rand(lastLaneLength);
            break;
        case ArrivalPosDefinition::CENTER:
            myArrivalPos = lastLaneLength / 2.;
            break;
        default:
            myArrivalPos = lastLaneLength;
            break;
    }
    if (myParameter->arrivalLaneProcedure == ArrivalLaneDefinition::GIVEN) {
        if (myParameter->arrivalLane >= (int)lanes.size() || !lanes[myParameter->arrivalLane]->allowsVehicleClass(myType->getVehicleClass())) {
            WRITE_WARNING("Vehicle '" + getID() + "' will not be able to arrive at the given lane '" + myRoute->getLastEdge()->getID() + "_" + toString(myParameter->arrivalLane) + "'!");
        }
        myArrivalLane = MIN2(myParameter->arrivalLane, (int)(lanes.size() - 1));
    }
    if (myParameter->arrivalSpeedProcedure == ArrivalSpeedDefinition::GIVEN) {
        for (std::vector<MSLane*>::const_iterator l = lanes.begin(); l != lanes.end(); ++l) {
            if (myParameter->arrivalSpeed <= (*l)->getVehicleMaxSpeed(this)) {
                return;
            }
        }
        WRITE_WARNING("Vehicle '" + getID() + "' will not be able to arrive with the given speed!");
    }
}


double
MSBaseVehicle::getImpatience() const {
    return MAX2(0., MIN2(1., getVehicleType().getImpatience() +
                         (MSGlobals::gTimeToImpatience > 0 ? (double)getWaitingTime() / MSGlobals::gTimeToImpatience : 0)));
}


MSVehicleDevice*
MSBaseVehicle::getDevice(const std::type_info& type) const {
    for (MSVehicleDevice* const dev : myDevices) {
        if (typeid(*dev) == type) {
            return dev;
        }
    }
    return nullptr;
}


void
MSBaseVehicle::saveState(OutputDevice& out) {
    // this saves lots of departParameters which are only needed for vehicles that did not yet depart
    // the parameters may hold the name of a vTypeDistribution but we are interested in the actual type
    myParameter->write(out, OptionsCont::getOptions(), SUMO_TAG_VEHICLE, getVehicleType().getID());
    // params and stops must be written in child classes since they may wish to add additional attributes first
    out.writeAttr(SUMO_ATTR_ROUTE, myRoute->getID());
    if (!myParameter->wasSet(VEHPARS_SPEEDFACTOR_SET)) {
        out.writeAttr(SUMO_ATTR_SPEEDFACTOR, myChosenSpeedFactor);
    }
    if (myParameter->wasSet(VEHPARS_FORCE_REROUTE) && !hasDeparted()) {
        out.writeAttr(SUMO_ATTR_REROUTE, true);
    }
    if (!myParameter->wasSet(VEHPARS_LINE_SET) && myParameter->line != "") {
        // could be set from stop
        out.writeAttr(SUMO_ATTR_LINE, myParameter->line);
    }
    // here starts the vehicle internal part (see loading)
    // @note: remember to close the vehicle tag when calling this in a subclass!
}


void
MSBaseVehicle::addStops(const bool ignoreStopErrors, MSRouteIterator* searchStart) {
    for (std::vector<SUMOVehicleParameter::Stop>::const_iterator i = myRoute->getStops().begin(); i != myRoute->getStops().end(); ++i) {
        std::string errorMsg;
        if (!addStop(*i, errorMsg, myParameter->depart, false, searchStart) && !ignoreStopErrors) {
            throw ProcessError(errorMsg);
        }
        if (errorMsg != "") {
            WRITE_WARNING(errorMsg);
        }
    }
    const SUMOTime untilOffset = myParameter->repetitionOffset > 0 ? myParameter->repetitionsDone * myParameter->repetitionOffset : 0;
    for (std::vector<SUMOVehicleParameter::Stop>::const_iterator i = myParameter->stops.begin(); i != myParameter->stops.end(); ++i) {
        std::string errorMsg;
        if (!addStop(*i, errorMsg, untilOffset, false, searchStart) && !ignoreStopErrors) {
            throw ProcessError(errorMsg);
        }
        if (errorMsg != "") {
            WRITE_WARNING(errorMsg);
        }
    }
}


double
MSBaseVehicle::getCO2Emissions() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::CO2, getSpeed(), getAcceleration(), getSlope());
    } else {
        return 0.;
    }
}


double
MSBaseVehicle::getCOEmissions() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::CO, getSpeed(), getAcceleration(), getSlope());
    } else {
        return 0.;
    }
}


double
MSBaseVehicle::getHCEmissions() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::HC, getSpeed(), getAcceleration(), getSlope());
    } else {
        return 0.;
    }
}


double
MSBaseVehicle::getNOxEmissions() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::NO_X, getSpeed(), getAcceleration(), getSlope());
    } else {
        return 0.;
    }
}


double
MSBaseVehicle::getPMxEmissions() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::PM_X, getSpeed(), getAcceleration(), getSlope());
    } else {
        return 0.;
    }
}


double
MSBaseVehicle::getFuelConsumption() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::FUEL, getSpeed(), getAcceleration(), getSlope());
    } else {
        return 0.;
    }
}


double
MSBaseVehicle::getElectricityConsumption() const {
    if (isOnRoad() || isIdling()) {
        return PollutantsInterface::compute(myType->getEmissionClass(), PollutantsInterface::ELEC, getSpeed(), getAcceleration(), getSlope(), getEmissionParameters());
    } else {
        return 0.;
    }
}

double
MSBaseVehicle::getStateOfCharge() const {
    if (static_cast<MSDevice_Battery*>(getDevice(typeid(MSDevice_Battery))) != 0) {
        MSDevice_Battery* batteryOfVehicle = dynamic_cast<MSDevice_Battery*>(getDevice(typeid(MSDevice_Battery)));
        return batteryOfVehicle->getActualBatteryCapacity();
    } else {
        if (static_cast<MSDevice_ElecHybrid*>(getDevice(typeid(MSDevice_ElecHybrid))) != 0) {
            MSDevice_ElecHybrid* batteryOfVehicle = dynamic_cast<MSDevice_ElecHybrid*>(getDevice(typeid(MSDevice_ElecHybrid)));
            return batteryOfVehicle->getActualBatteryCapacity();
        }
    }

    return -1;
}

double
MSBaseVehicle::getElecHybridCurrent() const {
    if (static_cast<MSDevice_ElecHybrid*>(getDevice(typeid(MSDevice_ElecHybrid))) != 0) {
        MSDevice_ElecHybrid* batteryOfVehicle = dynamic_cast<MSDevice_ElecHybrid*>(getDevice(typeid(MSDevice_ElecHybrid)));
        return batteryOfVehicle->getCurrentFromOverheadWire();
    }

    return NAN;
}

double
MSBaseVehicle::getHarmonoise_NoiseEmissions() const {
    if (isOnRoad() || isIdling()) {
        return HelpersHarmonoise::computeNoise(myType->getEmissionClass(), getSpeed(), getAcceleration());
    } else {
        return 0.;
    }
}


const MSEdgeWeightsStorage&
MSBaseVehicle::getWeightsStorage() const {
    return _getWeightsStorage();
}


MSEdgeWeightsStorage&
MSBaseVehicle::getWeightsStorage() {
    return _getWeightsStorage();
}


MSEdgeWeightsStorage&
MSBaseVehicle::_getWeightsStorage() const {
    if (myEdgeWeights == nullptr) {
        myEdgeWeights = new MSEdgeWeightsStorage();
    }
    return *myEdgeWeights;
}




int
MSBaseVehicle::getPersonNumber() const {
    int boarded = myPersonDevice == nullptr ? 0 : myPersonDevice->size();
    return boarded + myParameter->personNumber;
}

std::vector<std::string>
MSBaseVehicle::getPersonIDList() const {
    std::vector<std::string> ret;
    const std::vector<MSTransportable*>& persons = getPersons();
    for (std::vector<MSTransportable*>::const_iterator it_p = persons.begin(); it_p != persons.end(); ++it_p) {
        ret.push_back((*it_p)->getID());
    }
    return ret;
}

int
MSBaseVehicle::getContainerNumber() const {
    int loaded = myContainerDevice == nullptr ? 0 : myContainerDevice->size();
    return loaded + myParameter->containerNumber;
}


void
MSBaseVehicle::removeTransportable(MSTransportable* t) {
    // this might be called from the MSTransportable destructor so we cannot do a dynamic cast to determine the type
    if (myPersonDevice != nullptr) {
        myPersonDevice->removeTransportable(t);
    }
    if (myContainerDevice != nullptr) {
        myContainerDevice->removeTransportable(t);
    }
}


const std::vector<MSTransportable*>&
MSBaseVehicle::getPersons() const {
    if (myPersonDevice == nullptr) {
        return myEmptyTransportableVector;
    } else {
        return myPersonDevice->getTransportables();
    }
}


const std::vector<MSTransportable*>&
MSBaseVehicle::getContainers() const {
    if (myContainerDevice == nullptr) {
        return myEmptyTransportableVector;
    } else {
        return myContainerDevice->getTransportables();
    }
}


bool
MSBaseVehicle::isLineStop(double position) const {
    if (myParameter->line == "") {
        // not a public transport line
        return false;
    }
    for (const SUMOVehicleParameter::Stop& stop : myParameter->stops) {
        if (stop.startPos <= position && position <= stop.endPos) {
            return true;
        }
    }
    for (const SUMOVehicleParameter::Stop& stop : myRoute->getStops()) {
        if (stop.startPos <= position && position <= stop.endPos) {
            return true;
        }
    }
    return false;
}


bool
MSBaseVehicle::hasDevice(const std::string& deviceName) const {
    for (MSDevice* const dev : myDevices) {
        if (dev->deviceName() == deviceName) {
            return true;
        }
    }
    return false;
}


void
MSBaseVehicle::createDevice(const std::string& deviceName) {
    if (!hasDevice(deviceName)) {
        if (deviceName == "rerouting") {
            ((SUMOVehicleParameter*)myParameter)->setParameter("has." + deviceName + ".device", "true");
            MSDevice_Routing::buildVehicleDevices(*this, myDevices);
            if (hasDeparted()) {
                // vehicle already departed: disable pre-insertion rerouting and enable regular routing behavior
                MSDevice_Routing* routingDevice = static_cast<MSDevice_Routing*>(getDevice(typeid(MSDevice_Routing)));
                assert(routingDevice != 0);
                routingDevice->notifyEnter(*this, MSMoveReminder::NOTIFICATION_DEPARTED);
            }
        } else {
            throw InvalidArgument("Creating device of type '" + deviceName + "' is not supported");
        }
    }
}


std::string
MSBaseVehicle::getDeviceParameter(const std::string& deviceName, const std::string& key) const {
    for (MSVehicleDevice* const dev : myDevices) {
        if (dev->deviceName() == deviceName) {
            return dev->getParameter(key);
        }
    }
    throw InvalidArgument("No device of type '" + deviceName + "' exists");
}


void
MSBaseVehicle::setDeviceParameter(const std::string& deviceName, const std::string& key, const std::string& value) {
    for (MSVehicleDevice* const dev : myDevices) {
        if (dev->deviceName() == deviceName) {
            dev->setParameter(key, value);
            return;
        }
    }
    throw InvalidArgument("No device of type '" + deviceName + "' exists");
}


void
MSBaseVehicle::replaceVehicleType(MSVehicleType* type) {
    assert(type != nullptr);
    if (myType->isVehicleSpecific() && type != myType) {
        MSNet::getInstance()->getVehicleControl().removeVType(myType);
    }
    myType = type;
}


MSVehicleType&
MSBaseVehicle::getSingularType() {
    if (myType->isVehicleSpecific()) {
        return *myType;
    }
    MSVehicleType* type = myType->buildSingularType(myType->getID() + "@" + getID());
    replaceVehicleType(type);
    return *type;
}


int
MSBaseVehicle::getRNGIndex() const {
    const MSLane* const lane = getLane();
    if (lane == nullptr) {
        return getEdge()->getLanes()[0]->getRNGIndex();
    } else {
        return lane->getRNGIndex();
    }
}


std::mt19937*
MSBaseVehicle::getRNG() const {
    const MSLane* lane = getLane();
    if (lane == nullptr) {
        return getEdge()->getLanes()[0]->getRNG();
    } else {
        return lane->getRNG();
    }
}

std::string
MSBaseVehicle::getPrefixedParameter(const std::string& key, std::string& error) const {
    const MSVehicle* microVeh = dynamic_cast<const MSVehicle*>(this);
    if (StringUtils::startsWith(key, "device.")) {
        StringTokenizer tok(key, ".");
        if (tok.size() < 3) {
            error = "Invalid device parameter '" + key + "' for vehicle '" + getID() + "'.";
            return "";
        }
        try {
            return getDeviceParameter(tok.get(1), key.substr(tok.get(0).size() + tok.get(1).size() + 2));
        } catch (InvalidArgument& e) {
            error = "Vehicle '" + getID() + "' does not support device parameter '" + key + "' (" + e.what() + ").";
            return "";
        }
    } else if (StringUtils::startsWith(key, "laneChangeModel.")) {
        if (microVeh == nullptr) {
            error = "Meso Vehicle '" + getID() + "' does not support laneChangeModel parameters.";
            return "";
        }
        const std::string attrName = key.substr(16);
        try {
            return microVeh->getLaneChangeModel().getParameter(attrName);
        } catch (InvalidArgument& e) {
            error = "Vehicle '" + getID() + "' does not support laneChangeModel parameter '" + key + "' (" + e.what() + ").";
            return "";
        }
    } else if (StringUtils::startsWith(key, "carFollowModel.")) {
        if (microVeh == nullptr) {
            error = "Meso Vehicle '" + getID() + "' does not support carFollowModel parameters.";
            return "";
        }
        const std::string attrName = key.substr(15);
        try {
            return microVeh->getCarFollowModel().getParameter(microVeh, attrName);
        } catch (InvalidArgument& e) {
            error = "Vehicle '" + getID() + "' does not support carFollowModel parameter '" + key + "' (" + e.what() + ").";
            return "";
        }
    } else if (StringUtils::startsWith(key, "has.") && StringUtils::endsWith(key, ".device")) {
        StringTokenizer tok(key, ".");
        if (tok.size() != 3) {
            error = "Invalid check for device. Expected format is 'has.DEVICENAME.device'.";
            return "";
        }
        return hasDevice(tok.get(1)) ? "true" : "false";
    } else {
        return getParameter().getParameter(key, "");
    }
}

#ifdef _DEBUG
void
MSBaseVehicle::initMoveReminderOutput(const OptionsCont& oc) {
    if (oc.isSet("movereminder-output.vehicles")) {
        const std::vector<std::string> vehicles = oc.getStringVector("movereminder-output.vehicles");
        myShallTraceMoveReminders.insert(vehicles.begin(), vehicles.end());
    }
}


void
MSBaseVehicle::traceMoveReminder(const std::string& type, MSMoveReminder* rem, double pos, bool keep) const {
    OutputDevice& od = OutputDevice::getDeviceByOption("movereminder-output");
    od.openTag("movereminder");
    od.writeAttr(SUMO_ATTR_TIME, STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep()));
    od.writeAttr("veh", getID());
    od.writeAttr(SUMO_ATTR_ID, rem->getDescription());
    od.writeAttr("type", type);
    od.writeAttr("pos", toString(pos));
    od.writeAttr("keep", toString(keep));
    od.closeTag();
}
#endif


/****************************************************************************/
