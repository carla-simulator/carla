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
/// @file    MSDevice_Transportable.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @date    Fri, 30.01.2009
///
// A device which is used to keep track of persons and containers riding with a vehicle
/****************************************************************************/
#include <config.h>

#include <utils/xml/SUMOSAXAttributes.h>
#include <microsim/output/MSStopOut.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/transportables/MSStageDriving.h>
#include "MSDevice_Transportable.h"
#include "MSDevice_Taxi.h"


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
MSDevice_Transportable*
MSDevice_Transportable::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into, const bool isContainer) {
    MSDevice_Transportable* device = new MSDevice_Transportable(v, isContainer ? "container_" + v.getID() : "person_" + v.getID(), isContainer);
    into.push_back(device);
    return device;
}


// ---------------------------------------------------------------------------
// MSDevice_Transportable-methods
// ---------------------------------------------------------------------------
MSDevice_Transportable::MSDevice_Transportable(SUMOVehicle& holder, const std::string& id, const bool isContainer)
    : MSVehicleDevice(holder, id), myAmContainer(isContainer), myTransportables(), myStopped(holder.isStopped()) {
}


MSDevice_Transportable::~MSDevice_Transportable() {
    // flush any unfortunate riders still remaining
    for (auto it = myTransportables.begin(); it != myTransportables.end();) {
        MSTransportable* transportable = *it;
        WRITE_WARNING((myAmContainer ? "Removing container '" : "Removing person '") + transportable->getID() +
                      "' at removal of vehicle '" + myHolder.getID() + "'");
        MSStageDriving* const stage = dynamic_cast<MSStageDriving*>(transportable->getCurrentStage());
        if (stage != nullptr) {
            stage->setVehicle(nullptr);
        }
        if (myAmContainer) {
            MSNet::getInstance()->getContainerControl().erase(transportable);
        } else {
            MSNet::getInstance()->getPersonControl().erase(transportable);
        }
        it = myTransportables.erase(it);
    }
}

void
MSDevice_Transportable::notifyMoveInternal(const SUMOTrafficObject& veh,
        const double /* frontOnLane */,
        const double /* timeOnLane*/,
        const double /* meanSpeedFrontOnLane */,
        const double /*meanSpeedVehicleOnLane */,
        const double /* travelledDistanceFrontOnLane */,
        const double /* travelledDistanceVehicleOnLane */,
        const double /* meanLengthOnLane */) {
    notifyMove(const_cast<SUMOTrafficObject&>(veh), -1, -1, -1);
}


bool
MSDevice_Transportable::notifyMove(SUMOTrafficObject& veh, double /*oldPos*/, double /*newPos*/, double /*newSpeed*/) {
    if (myStopped) {
        if (!veh.isStopped()) {
            for (std::vector<MSTransportable*>::iterator i = myTransportables.begin(); i != myTransportables.end(); ++i) {
                (*i)->setDeparted(MSNet::getInstance()->getCurrentTimeStep());
            }
            myStopped = false;
        }
    } else {
        if (veh.isStopped()) {
            for (std::vector<MSTransportable*>::iterator i = myTransportables.begin(); i != myTransportables.end();) {
                MSTransportable* transportable = *i;
                if (transportable->getDestination() == veh.getEdge()) {
                    MSStageDriving* const stage = dynamic_cast<MSStageDriving*>(transportable->getCurrentStage());
                    // if this is the last stage, we can use the arrivalPos of the person
                    const bool unspecifiedArrivalPos = stage->unspecifiedArrivalPos() && (
                                                           transportable->getNumRemainingStages() > 1 || !transportable->getParameter().wasSet(VEHPARS_ARRIVALPOS_SET));
                    const double arrivalPos = (stage->unspecifiedArrivalPos()
                                               ? SUMOVehicleParameter::interpretEdgePos(transportable->getParameter().arrivalPos, veh.getEdge()->getLength(),
                                                       SUMO_ATTR_ARRIVALPOS, transportable->getID(), true)
                                               : stage->getArrivalPos());
                    if (unspecifiedArrivalPos ||
                            myHolder.isStoppedInRange(arrivalPos, myHolder.getLength() + MSGlobals::gStopTolerance)) {
                        i = myTransportables.erase(i); // erase first in case proceed throws an exception
                        if (!transportable->proceed(MSNet::getInstance(), MSNet::getInstance()->getCurrentTimeStep())) {
                            if (myAmContainer) {
                                MSNet::getInstance()->getContainerControl().erase(transportable);
                            } else {
                                MSNet::getInstance()->getPersonControl().erase(transportable);
                            }
                        }
                        if (MSStopOut::active()) {
                            SUMOVehicle* vehicle = dynamic_cast<SUMOVehicle*>(&veh);
                            if (myAmContainer) {
                                MSStopOut::getInstance()->unloadedContainers(vehicle, 1);
                            } else {
                                MSStopOut::getInstance()->unloadedPersons(vehicle, 1);
                            }
                        }
                        MSDevice_Taxi* taxiDevice = static_cast<MSDevice_Taxi*>(myHolder.getDevice(typeid(MSDevice_Taxi)));
                        if (taxiDevice != nullptr) {
                            taxiDevice->customerArrived(transportable);
                        }
                        continue;
                    }
                }
                ++i;
            }
            myStopped = true;
        }
    }
    return true;
}


bool
MSDevice_Transportable::notifyEnter(SUMOTrafficObject& /*veh*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (reason == MSMoveReminder::NOTIFICATION_DEPARTED) {
        for (std::vector<MSTransportable*>::iterator i = myTransportables.begin(); i != myTransportables.end(); ++i) {
            (*i)->setDeparted(MSNet::getInstance()->getCurrentTimeStep());
        }
    }
    return true;
}


bool
MSDevice_Transportable::notifyLeave(SUMOTrafficObject& veh, double /*lastPos*/,
                                    MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (reason >= MSMoveReminder::NOTIFICATION_ARRIVED) {
        for (std::vector<MSTransportable*>::iterator i = myTransportables.begin(); i != myTransportables.end();) {
            MSTransportable* transportable = *i;
            if (transportable->getDestination() != veh.getEdge()) {
                WRITE_WARNING((myAmContainer ? "Teleporting container '" : "Teleporting person '") + transportable->getID() +
                              "' from vehicle destination edge '" + veh.getEdge()->getID() +
                              "' to intended destination edge '" + transportable->getDestination()->getID() + "'");
            }
            if (!transportable->proceed(MSNet::getInstance(), MSNet::getInstance()->getCurrentTimeStep(), true)) {
                if (myAmContainer) {
                    MSNet::getInstance()->getContainerControl().erase(transportable);
                } else {
                    MSNet::getInstance()->getPersonControl().erase(transportable);
                }
            }
            i = myTransportables.erase(i);
        }
    }
    return true;
}


void
MSDevice_Transportable::addTransportable(MSTransportable* transportable) {
    myTransportables.push_back(transportable);
    if (MSStopOut::active()) {
        if (myAmContainer) {
            MSStopOut::getInstance()->loadedContainers(&myHolder, 1);
        } else {
            MSStopOut::getInstance()->loadedPersons(&myHolder, 1);
        }
    }
    MSDevice_Taxi* taxiDevice = static_cast<MSDevice_Taxi*>(myHolder.getDevice(typeid(MSDevice_Taxi)));
    if (taxiDevice != nullptr) {
        taxiDevice->customerEntered(transportable);
    }
}


void
MSDevice_Transportable::removeTransportable(MSTransportable* transportable) {
    auto it = std::find(myTransportables.begin(), myTransportables.end(), transportable);
    if (it != myTransportables.end()) {
        myTransportables.erase(it);
        if (MSStopOut::active() && myHolder.isStopped()) {
            if (myAmContainer) {
                MSStopOut::getInstance()->loadedContainers(&myHolder, 1);
            } else {
                MSStopOut::getInstance()->loadedPersons(&myHolder, 1);
            }
        }
    }
}


void
MSDevice_Transportable::saveState(OutputDevice& out) const {
    out.openTag(SUMO_TAG_DEVICE);
    out.writeAttr(SUMO_ATTR_ID, getID());
    std::vector<std::string> internals;
    internals.push_back(toString(myStopped));
    out.writeAttr(SUMO_ATTR_STATE, toString(internals));
    out.closeTag();
}


void
MSDevice_Transportable::loadState(const SUMOSAXAttributes& attrs) {
    std::istringstream bis(attrs.getString(SUMO_ATTR_STATE));
    bis >> myStopped;
}


std::string
MSDevice_Transportable::getParameter(const std::string& key) const {
    if (key == "IDList") {
        std::vector<std::string> ids;
        for (const MSTransportable* t : myTransportables) {
            ids.push_back(t->getID());
        }
        return toString(ids);
    }
    throw InvalidArgument("Parameter '" + key + "' is not supported for device of type '" + deviceName() + "'");
}


/****************************************************************************/
