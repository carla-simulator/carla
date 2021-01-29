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
/// @file    MSStageTranship.cpp
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @date    Thu, 12 Jun 2014
///
// The class for modelling transportable movements without interaction
/****************************************************************************/
#include <config.h>

#include <string>
#include <vector>
#include <utils/iodevices/OutputDevice.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/transportables/MSPModel.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include "MSStageTranship.h"


// ===========================================================================
// method definitions
// ===========================================================================
MSStageTranship::MSStageTranship(const std::vector<const MSEdge*>& route,
                                 MSStoppingPlace* toStop,
                                 double speed,
                                 double departPos, double arrivalPos) :
    MSStageMoving(route, toStop, speed, departPos, arrivalPos, 0., MSStageType::TRANSHIP) {
    myDepartPos = SUMOVehicleParameter::interpretEdgePos(
                      departPos, myRoute.front()->getLength(), SUMO_ATTR_DEPARTPOS,
                      "container getting transhipped from " + myRoute.front()->getID());
    myArrivalPos = SUMOVehicleParameter::interpretEdgePos(
                       arrivalPos, route.back()->getLength(), SUMO_ATTR_ARRIVALPOS,
                       "container getting transhipped to " + route.back()->getID());
}


MSStageTranship::~MSStageTranship() {
}


MSStage*
MSStageTranship::clone() const {
    return new MSStageTranship(myRoute, myDestinationStop, mySpeed, myDepartPos, myArrivalPos);
}


void
MSStageTranship::proceed(MSNet* net, MSTransportable* transportable, SUMOTime now, MSStage* previous) {
    myDeparted = now;
    //MSPModel_NonInteracting moves the transportable straight from start to end in
    //a single step and assumes that moveToNextEdge is only called once)
    //therefore we define that the transportable is already on its destination edge
    myRouteStep = myRoute.end() - 1;
    myDepartPos = previous->getEdgePos(now);
    if (transportable->isPerson()) {
        myState = net->getPersonControl().getNonInteractingModel()->add(transportable, this, now);
        (*myRouteStep)->addPerson(transportable);
    } else {
        myState = net->getContainerControl().getNonInteractingModel()->add(transportable, this, now);
        (*myRouteStep)->addContainer(transportable);
    }
}


double
MSStageTranship::getDistance() const {
    if (myArrived >= 0) {
        const SUMOTime duration = myArrived - myDeparted;
        return mySpeed * STEPS2TIME(duration);
    } else {
        return -1;
    }
}


void
MSStageTranship::tripInfoOutput(OutputDevice& os, const MSTransportable* const) const {
    os.openTag("tranship");
    os.writeAttr("depart", time2string(myDeparted));
    os.writeAttr("departPos", myDepartPos);
    os.writeAttr("arrival", time2string(myArrived));
    os.writeAttr("arrivalPos", myArrivalPos);
    os.writeAttr("duration", myArrived >= 0 ? time2string(myArrived - myDeparted) : "-1");
    os.writeAttr("routeLength", getDistance());
    os.writeAttr("maxSpeed", mySpeed);
    os.closeTag();
}


void
MSStageTranship::routeOutput(const bool /*isPerson*/, OutputDevice& os, const bool withRouteLength, const MSStage* const /* previous */) const {
    os.openTag("tranship").writeAttr(SUMO_ATTR_EDGES, myRoute);
    os.writeAttr(SUMO_ATTR_SPEED, mySpeed);
    if (withRouteLength) {
        os.writeAttr("routeLength", mySpeed * (myArrived - myDeparted));
    }
    os.closeTag();
}


bool
MSStageTranship::moveToNextEdge(MSTransportable* transportable, SUMOTime currentTime, MSEdge* /* nextInternal */) {
    if (transportable->isPerson()) {
        getEdge()->removePerson(transportable);
    } else {
        getEdge()->removeContainer(transportable);
    }
    // transship does a direct move so we are already at our destination
    if (myDestinationStop != nullptr) {
        myDestinationStop->addTransportable(transportable);    //jakob
    }
    if (!transportable->proceed(MSNet::getInstance(), currentTime)) {
        if (transportable->isPerson()) {
            MSNet::getInstance()->getPersonControl().erase(transportable);
        } else {
            MSNet::getInstance()->getContainerControl().erase(transportable);
        }
    }
    return true;
}


std::string
MSStageTranship::getStageSummary(const bool /*isPerson*/) const {
    const std::string dest = (getDestinationStop() == nullptr ?
                              " edge '" + getDestination()->getID() + "'" :
                              " stop '" + getDestinationStop()->getID() + "'");
    return "transhipped to " + dest;
}


/****************************************************************************/
