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
/// @file    MSTransportableControl.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Stores all persons in the net and handles their waiting for cars.
/****************************************************************************/
#include <config.h>

#include <vector>
#include <algorithm>
#include <utils/iodevices/OutputDevice.h>
#include <utils/iodevices/OutputDevice_String.h>
#include <utils/options/OptionsCont.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/transportables/MSStageDriving.h>
#include <microsim/MSVehicle.h>
#include <microsim/transportables/MSPModel_NonInteracting.h>
#include <microsim/transportables/MSPModel_Striping.h>
#include <microsim/transportables/MSTransportableControl.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSTransportableControl::MSTransportableControl(const bool isPerson):
    myLoadedNumber(0),
    myDiscardedNumber(0),
    myRunningNumber(0),
    myJammedNumber(0),
    myWaitingForDepartureNumber(0),
    myWaitingForVehicleNumber(0),
    myWaitingUntilNumber(0),
    myEndedNumber(0),
    myArrivedNumber(0),
    myHaveNewWaiting(false) {
    const OptionsCont& oc = OptionsCont::getOptions();
    MSNet* const net = MSNet::getInstance();
    if (isPerson) {
        const std::string model = oc.getString("pedestrian.model");
        myNonInteractingModel = new MSPModel_NonInteracting(oc, net);
        if (model == "striping") {
            myMovementModel = new MSPModel_Striping(oc, net);
        } else if (model == "nonInteracting") {
            myMovementModel = myNonInteractingModel;
        } else {
            throw ProcessError("Unknown pedestrian model '" + model + "'");
        }
    } else {
        myMovementModel = myNonInteractingModel = new MSPModel_NonInteracting(oc, net);
    }
}


MSTransportableControl::~MSTransportableControl() {
    for (std::map<std::string, MSTransportable*>::iterator i = myTransportables.begin(); i != myTransportables.end(); ++i) {
        delete (*i).second;
    }
    myTransportables.clear();
    myWaiting4Vehicle.clear();
    if (myMovementModel != myNonInteractingModel) {
        delete myMovementModel;
    }
    delete myNonInteractingModel;
}


bool
MSTransportableControl::add(MSTransportable* transportable) {
    const SUMOVehicleParameter& param = transportable->getParameter();
    if (myTransportables.find(param.id) == myTransportables.end()) {
        myTransportables[param.id] = transportable;
        const SUMOTime step = param.depart % DELTA_T == 0 ? param.depart : (param.depart / DELTA_T + 1) * DELTA_T;
        myWaiting4Departure[step].push_back(transportable);
        myLoadedNumber++;
        myWaitingForDepartureNumber++;
        return true;
    }
    return false;
}


MSTransportable*
MSTransportableControl::get(const std::string& id) const {
    std::map<std::string, MSTransportable*>::const_iterator i = myTransportables.find(id);
    if (i == myTransportables.end()) {
        return nullptr;
    }
    return (*i).second;
}


void
MSTransportableControl::erase(MSTransportable* transportable) {
    if (OptionsCont::getOptions().isSet("tripinfo-output")) {
        transportable->tripInfoOutput(OutputDevice::getDeviceByOption("tripinfo-output"));
    } else if (OptionsCont::getOptions().getBool("duration-log.statistics")) {
        // collecting statistics is a sideffect
        OutputDevice_String dev;
        transportable->tripInfoOutput(dev);
    }
    if (OptionsCont::getOptions().isSet("vehroute-output")) {
        transportable->routeOutput(OutputDevice::getDeviceByOption("vehroute-output"), OptionsCont::getOptions().getBool("vehroute-output.route-length"));
    }
    const std::map<std::string, MSTransportable*>::iterator i = myTransportables.find(transportable->getID());
    if (i != myTransportables.end()) {
        myRunningNumber--;
        myEndedNumber++;
        delete i->second;
        myTransportables.erase(i);
    }
}


void
MSTransportableControl::setWaitEnd(const SUMOTime time, MSTransportable* transportable) {
    const SUMOTime step = time % DELTA_T == 0 ? time : (time / DELTA_T + 1) * DELTA_T;
    // avoid double registration
    const TransportableVector& transportables = myWaiting4Departure[step];
    if (std::find(transportables.begin(), transportables.end(), transportable) == transportables.end()) {
        myWaitingUntil[step].push_back(transportable);
        myWaitingUntilNumber++;
    }
}


void
MSTransportableControl::checkWaiting(MSNet* net, const SUMOTime time) {
    myHaveNewWaiting = false;
    while (myWaiting4Departure.find(time) != myWaiting4Departure.end()) {
        const TransportableVector& transportables = myWaiting4Departure[time];
        // we cannot use an iterator here because there might be additions to the vector while proceeding
        for (int i = 0; i < (int)transportables.size(); ++i) {
            myWaitingForDepartureNumber--;
            if (transportables[i]->proceed(net, time)) {
                myRunningNumber++;
            } else {
                erase(transportables[i]);
            }
        }
        myWaiting4Departure.erase(time);
    }
    while (myWaitingUntil.find(time) != myWaitingUntil.end()) {
        const TransportableVector& transportables = myWaitingUntil[time];
        // we cannot use an iterator here because there might be additions to the vector while proceeding
        for (int i = 0; i < (int)transportables.size(); ++i) {
            myWaitingUntilNumber--;
            if (!transportables[i]->proceed(net, time)) {
                erase(transportables[i]);
            }
        }
        myWaitingUntil.erase(time);
    }
}

void
MSTransportableControl::forceDeparture() {
    myRunningNumber++;
}

void
MSTransportableControl::addWaiting(const MSEdge* const edge, MSTransportable* transportable) {
    myWaiting4Vehicle[edge].push_back(transportable);
    myWaitingForVehicleNumber++;
    myHaveNewWaiting = true;
}


bool
MSTransportableControl::boardAnyWaiting(MSEdge* edge, SUMOVehicle* vehicle, const SUMOVehicleParameter::Stop& stop, SUMOTime& timeToBoardNextPerson, SUMOTime& stopDuration) {
    bool ret = false;
    if (myWaiting4Vehicle.find(edge) != myWaiting4Vehicle.end()) {
        TransportableVector& wait = myWaiting4Vehicle[edge];
        SUMOTime currentTime =  MSNet::getInstance()->getCurrentTimeStep();
        for (TransportableVector::iterator i = wait.begin(); i != wait.end();) {
            if ((*i)->isWaitingFor(vehicle)
                    && vehicle->allowsBoarding(*i)
                    && timeToBoardNextPerson - DELTA_T <= currentTime
                    && stop.startPos <= (*i)->getEdgePos()
                    && (*i)->getEdgePos() <= stop.endPos) {
                edge->removePerson(*i);
                vehicle->addTransportable(*i);
                if (timeToBoardNextPerson >= 0) { // meso does not have boarding times
                    const SUMOTime boardingDuration = vehicle->getVehicleType().getBoardingDuration();
                    //update the time point at which the next person can board the vehicle
                    if (timeToBoardNextPerson > currentTime - DELTA_T) {
                        timeToBoardNextPerson += boardingDuration;
                    } else {
                        timeToBoardNextPerson = currentTime + boardingDuration;
                    }
                }

                static_cast<MSStageDriving*>((*i)->getCurrentStage())->setVehicle(vehicle);
                i = wait.erase(i);
                myWaitingForVehicleNumber--;
                ret = true;
            } else {
                ++i;
            }
        }
        if (wait.size() == 0) {
            myWaiting4Vehicle.erase(myWaiting4Vehicle.find(edge));
        }
        if (ret && timeToBoardNextPerson >= 0) {
            //if the time a person needs to enter the vehicle extends the duration of the stop of the vehicle extend
            //the duration by setting it to the boarding duration of the person
            stopDuration = MAX2(stopDuration, timeToBoardNextPerson - currentTime);
            timeToBoardNextPerson -= DELTA_T;
        }
    }
    return ret;
}


bool
MSTransportableControl::loadAnyWaiting(MSEdge* edge, SUMOVehicle* vehicle, const SUMOVehicleParameter::Stop& stop, SUMOTime& timeToLoadNextContainer, SUMOTime& stopDuration) {
    bool ret = false;
    if (myWaiting4Vehicle.find(edge) != myWaiting4Vehicle.end()) {
        SUMOTime currentTime = MSNet::getInstance()->getCurrentTimeStep();
        TransportableVector& waitContainers = myWaiting4Vehicle[edge];
        for (TransportableVector::iterator i = waitContainers.begin(); i != waitContainers.end();) {
            if ((*i)->isWaitingFor(vehicle)
                    && vehicle->getVehicleType().getContainerCapacity() > vehicle->getContainerNumber()
                    && timeToLoadNextContainer - DELTA_T <= currentTime
                    && stop.startPos <= (*i)->getEdgePos()
                    && (*i)->getEdgePos() <= stop.endPos) {
                edge->removeContainer(*i);
                vehicle->addTransportable(*i);
                if (timeToLoadNextContainer >= 0) { // meso does not have loading times
                    //if the time a person needs to enter the vehicle extends the duration of the stop of the vehicle extend
                    //the duration by setting it to the boarding duration of the person
                    const SUMOTime loadingDuration = vehicle->getVehicleType().getLoadingDuration();
                    //update the time point at which the next container can be loaded on the vehicle
                    if (timeToLoadNextContainer > currentTime - DELTA_T) {
                        timeToLoadNextContainer += loadingDuration;
                    } else {
                        timeToLoadNextContainer = currentTime + loadingDuration;
                    }
                }

                static_cast<MSStageDriving*>((*i)->getCurrentStage())->setVehicle(vehicle);
                i = waitContainers.erase(i);
                myWaitingForVehicleNumber--;
                ret = true;
            } else {
                ++i;
            }
        }
        if (waitContainers.size() == 0) {
            myWaiting4Vehicle.erase(myWaiting4Vehicle.find(edge));
        }
        if (ret && timeToLoadNextContainer >= 0) {
            //if the time a container needs to get loaded on the vehicle extends the duration of the stop of the vehicle extend
            //the duration by setting it to the loading duration of the container
            stopDuration = MAX2(stopDuration, timeToLoadNextContainer - currentTime);
            timeToLoadNextContainer -= DELTA_T;
        }
    }
    return ret;
}


bool
MSTransportableControl::hasTransportables() const {
    return !myTransportables.empty();
}


bool
MSTransportableControl::hasNonWaiting() const {
    return !myWaiting4Departure.empty() || myWaitingForVehicleNumber < myRunningNumber || myHaveNewWaiting;
}


int
MSTransportableControl::getActiveCount() {
    return (int)myWaiting4Departure.size() + myRunningNumber - myWaitingForVehicleNumber;
}

int
MSTransportableControl::getMovingNumber() const {
    return myMovementModel->getActiveNumber();
}


int
MSTransportableControl::getRidingNumber() const {
    return myRunningNumber - myWaitingUntilNumber - myWaitingForVehicleNumber - getMovingNumber();
}

int
MSTransportableControl::getDepartedNumber() const {
    return myLoadedNumber - myWaitingForDepartureNumber - myDiscardedNumber;
}

void
MSTransportableControl::abortAnyWaitingForVehicle() {
    for (std::map<const MSEdge*, TransportableVector>::iterator i = myWaiting4Vehicle.begin(); i != myWaiting4Vehicle.end(); ++i) {
        const MSEdge* edge = (*i).first;
        for (MSTransportable* const p : i->second) {
            std::string transportableType;
            if (p->isPerson()) {
                edge->removePerson(p);
                transportableType = "Person";
            } else {
                transportableType = "Container";
                edge->removeContainer(p);
            }
            MSStageDriving* stage = dynamic_cast<MSStageDriving*>(p->getCurrentStage());
            const std::string waitDescription = stage == nullptr ? "waiting" : stage->getWaitingDescription();
            WRITE_WARNING(transportableType + " '" + p->getID() + "' aborted " + waitDescription + ".");
            erase(p);
        }
    }
    myWaiting4Vehicle.clear();
    myWaitingForVehicleNumber = 0;
}

void
MSTransportableControl::abortWaitingForVehicle(MSTransportable* t) {
    const MSEdge* edge = t->getEdge();
    auto it = myWaiting4Vehicle.find(edge);
    if (it != myWaiting4Vehicle.end()) {
        TransportableVector& waiting = it->second;
        auto it2 = std::find(waiting.begin(), waiting.end(), t);
        if (it2 != waiting.end()) {
            waiting.erase(it2);
        }
    }
}

void
MSTransportableControl::abortWaiting(MSTransportable* t) {
    for (std::map<SUMOTime, TransportableVector>::iterator it = myWaiting4Departure.begin(); it != myWaiting4Departure.end(); ++it) {
        TransportableVector& ts = it->second;
        TransportableVector::iterator it2 = std::find(ts.begin(), ts.end(), t);
        if (it2 != ts.end()) {
            ts.erase(it2);
        }
    }
    for (std::map<SUMOTime, TransportableVector>::iterator it = myWaitingUntil.begin(); it != myWaitingUntil.end(); ++it) {
        TransportableVector& ts = it->second;
        TransportableVector::iterator it2 = std::find(ts.begin(), ts.end(), t);
        if (it2 != ts.end()) {
            ts.erase(it2);
        }
    }
}


MSTransportable*
MSTransportableControl::buildPerson(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan,
                                    std::mt19937* rng) const {
    const double speedFactor = vtype->computeChosenSpeedDeviation(rng);
    return new MSPerson(pars, vtype, plan, speedFactor);
}


MSTransportable*
MSTransportableControl::buildContainer(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportable::MSTransportablePlan* plan) const {
    return new MSTransportable(pars, vtype, plan, false);
}


void
MSTransportableControl::saveState(OutputDevice& out) {
    out.writeAttr(SUMO_ATTR_NUMBER, myRunningNumber);
    out.writeAttr(SUMO_ATTR_BEGIN, myLoadedNumber);
    out.writeAttr(SUMO_ATTR_END, myEndedNumber);
    out.writeAttr(SUMO_ATTR_DEPART, myWaitingForDepartureNumber);
    out.writeAttr(SUMO_ATTR_ARRIVAL, myArrivedNumber);
    out.writeAttr(SUMO_ATTR_DISCARD, myDiscardedNumber);
    std::ostringstream oss;
    oss << myJammedNumber << " " << myWaitingForVehicleNumber << " " << myWaitingUntilNumber << " " << myHaveNewWaiting;
    out.writeAttr(SUMO_ATTR_STATE, oss.str());
    for (const auto it : myTransportables) {
        it.second->saveState(out);
    }
}


void
MSTransportableControl::loadState(const std::string& state) {
    std::istringstream iss(state);
    iss >> myJammedNumber >> myWaitingForVehicleNumber >> myWaitingUntilNumber >> myHaveNewWaiting;
}


/****************************************************************************/
