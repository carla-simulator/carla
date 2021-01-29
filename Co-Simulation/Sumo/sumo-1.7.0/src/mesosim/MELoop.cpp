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
/// @file    MELoop.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// The main mesocopic simulation loop
/****************************************************************************/
#include <config.h>

#include <queue>
#include <vector>
#include <map>
#include <cmath>

#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/ToString.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/RandHelper.h>
#include "MELoop.h"
#include "MESegment.h"
#include "MEVehicle.h"


// ===========================================================================
// method definitions
// ===========================================================================
MELoop::MELoop(const SUMOTime recheckInterval) : myFullRecheckInterval(recheckInterval), myLinkRecheckInterval(TIME2STEPS(1)) {
}

MELoop::~MELoop() {
    for (std::vector<MESegment*>::const_iterator j = myEdges2FirstSegments.begin(); j != myEdges2FirstSegments.end(); ++j) {
        for (MESegment* s = *j; s != nullptr;) {
            MESegment* n = s->getNextSegment();
            delete s;
            s = n;
        }
    }
}


void
MELoop::simulate(SUMOTime tMax) {
    while (!myLeaderCars.empty()) {
        const SUMOTime time = myLeaderCars.begin()->first;
        std::vector<MEVehicle*> vehs = myLeaderCars[time];
        assert(time > tMax - DELTA_T || vehs.size() == 0);
        if (time > tMax) {
            return;
        }
        myLeaderCars.erase(time);
        for (std::vector<MEVehicle*>::const_iterator i = vehs.begin(); i != vehs.end(); ++i) {
            checkCar(*i);
            assert(myLeaderCars.empty() || myLeaderCars.begin()->first >= time);
        }
    }
}


SUMOTime
MELoop::changeSegment(MEVehicle* veh, SUMOTime leaveTime, MESegment* const toSegment, MSMoveReminder::Notification reason, const bool ignoreLink) const {
    int qIdx = 0;
    MESegment* const onSegment = veh->getSegment();
    if (MESegment::isInvalid(toSegment)) {
        if (onSegment != nullptr) {
            onSegment->send(veh, toSegment, qIdx, leaveTime, reason);
        } else {
            WRITE_WARNING("Vehicle '" + veh->getID() + "' teleports beyond arrival edge '" + veh->getEdge()->getID() + "', time " + time2string(leaveTime) + ".");
        }
        veh->setSegment(toSegment); // signal arrival
        MSNet::getInstance()->getVehicleControl().scheduleVehicleRemoval(veh);
        return leaveTime;
    }
    const SUMOTime entry = toSegment->hasSpaceFor(veh, leaveTime, qIdx);
    if (entry == leaveTime && (ignoreLink || veh->mayProceed())) {
        if (onSegment != nullptr) {
            onSegment->send(veh, toSegment, qIdx, leaveTime, onSegment->getNextSegment() == nullptr ? MSMoveReminder::NOTIFICATION_JUNCTION : MSMoveReminder::NOTIFICATION_SEGMENT);
            toSegment->receive(veh, qIdx, leaveTime, false, ignoreLink, &onSegment->getEdge() != &toSegment->getEdge());
        } else {
            WRITE_WARNING("Vehicle '" + veh->getID() + "' ends teleporting on edge '" + toSegment->getEdge().getID()
                          + "':" + toString(toSegment->getIndex()) + ", time " + time2string(leaveTime) + ".");
            // this is not quite correct but suffices for interrogation by
            // subsequent methods (veh->getSpeed() needs segment != 0)
            veh->setSegment(myEdges2FirstSegments[veh->getEdge()->getNumericalID()]);
            // clean up detectors (do not add traffic data)
            // note: updateDatector is not called if leaveTime == getLastEntryTime()
            veh->updateDetectors(veh->getLastEntryTime(), true, MSMoveReminder::NOTIFICATION_TELEPORT);
            toSegment->receive(veh, qIdx, leaveTime, false, true, true);
        }
        return entry;
    }
    if (entry == leaveTime && !ignoreLink) { // this is a long way of saying !veh->mayProceed() (which is a costly call)
        return entry + MAX2(SUMOTime(1), myLinkRecheckInterval);
    }
    return entry;
}


void
MELoop::checkCar(MEVehicle* veh) {
    const SUMOTime leaveTime = veh->getEventTime();
    MESegment* const onSegment = veh->getSegment();
    MESegment* const toSegment = nextSegment(onSegment, veh);
    const bool teleporting = (onSegment == nullptr); // is the vehicle currently teleporting?
    // @note reason is only evaluated if toSegment == nullptr
    const SUMOTime nextEntry = changeSegment(veh, leaveTime, toSegment, MSMoveReminder::NOTIFICATION_ARRIVED, teleporting);
    if (nextEntry == leaveTime) {
        return;
    }
    if (MSGlobals::gTimeToGridlock > 0 && veh->getWaitingTime() > MSGlobals::gTimeToGridlock) {
        teleportVehicle(veh, toSegment);
        return;
    }
    if (veh->getBlockTime() == SUMOTime_MAX) {
        veh->setBlockTime(leaveTime);
    }
    if (nextEntry == SUMOTime_MAX) {
        // all usable queues on the next segment are full
        SUMOTime newEventTime = MAX3(toSegment->getEventTime() + 1, leaveTime + 1, leaveTime + myFullRecheckInterval);
        if (MSGlobals::gTimeToGridlock > 0) {
            // if teleporting is enabled, make sure we look at the vehicle when the the gridlock-time is up
            newEventTime = MIN2(newEventTime, veh->getBlockTime() + MSGlobals::gTimeToGridlock + 1);
        }
        veh->setEventTime(newEventTime);
    } else {
        // receiving segment has recently received another vehicle or the junction is blocked
        veh->setEventTime(nextEntry);
    }
    addLeaderCar(veh, onSegment->getLink(veh));
}


void
MELoop::teleportVehicle(MEVehicle* veh, MESegment* const toSegment) {
    const SUMOTime leaveTime = veh->getEventTime();
    MESegment* const onSegment = veh->getSegment();
    const bool teleporting = (onSegment == nullptr); // is the vehicle already teleporting?
    // try to find a place on the current edge
    MESegment* teleSegment = toSegment->getNextSegment();
    while (teleSegment != nullptr && changeSegment(veh, leaveTime, teleSegment, MSMoveReminder::NOTIFICATION_TELEPORT, true) != leaveTime) {
        // @caution the time to get to the next segment here is ignored XXX
        teleSegment = teleSegment->getNextSegment();
    }
    if (teleSegment != nullptr) {
        if (!teleporting) {
            // we managed to teleport in a single jump
            WRITE_WARNING("Teleporting vehicle '" + veh->getID() + "'; waited too long, from edge '" + onSegment->getEdge().getID()
                          + "':" + toString(onSegment->getIndex())
                          + " to edge '" + teleSegment->getEdge().getID()
                          + "':" + toString(teleSegment->getIndex())
                          + ", time " + time2string(leaveTime) + ".");
            MSNet::getInstance()->getVehicleControl().registerTeleportJam();
        }
    } else {
        // teleport across the current edge and try insertion later
        if (!teleporting) {
            int qIdx = 0;
            // announce start of multi-step teleport, arrival will be announced in changeSegment()
            WRITE_WARNING("Teleporting vehicle '" + veh->getID() + "'; waited too long, from edge '" + onSegment->getEdge().getID()
                          + "':" + toString(onSegment->getIndex()) + ", time " + time2string(leaveTime) + ".");
            MSNet::getInstance()->getVehicleControl().registerTeleportJam();
            // remove from current segment
            onSegment->send(veh, nullptr, qIdx, leaveTime, MSMoveReminder::NOTIFICATION_TELEPORT);
            // mark veh as teleporting
            veh->setSegment(nullptr);
        }
        // @caution microsim uses current travel time teleport duration
        const SUMOTime teleArrival = leaveTime + TIME2STEPS(veh->getEdge()->getLength() / MAX2(veh->getEdge()->getSpeedLimit(), NUMERICAL_EPS));
        const bool atDest = veh->moveRoutePointer();
        if (atDest) {
            // teleporting to end of route
            changeSegment(veh, teleArrival, nullptr, MSMoveReminder::NOTIFICATION_TELEPORT_ARRIVED, true);
        } else {
            veh->setEventTime(teleArrival);
            addLeaderCar(veh, nullptr);
            // teleporting vehicles must react to rerouters
            getSegmentForEdge(*veh->getEdge())->addReminders(veh);
            veh->activateReminders(MSMoveReminder::NOTIFICATION_JUNCTION);
        }
    }
}


void
MELoop::addLeaderCar(MEVehicle* veh, MSLink* link) {
    myLeaderCars[veh->getEventTime()].push_back(veh);
    setApproaching(veh, link);
}


void
MELoop::setApproaching(MEVehicle* veh, MSLink* link) {
    if (link != nullptr) {
        link->setApproaching(veh, veh->getEventTime() + (link->getState() == LINKSTATE_ALLWAY_STOP ?
                             (SUMOTime)RandHelper::rand((int)2) : 0), // tie braker
                             veh->getSpeed(), veh->getSpeed(), true,
                             veh->getEventTime(), veh->getSpeed(), veh->getWaitingTime(),
                             // @note: dist is not used by meso (getZipperSpeed is never called)
                             veh->getSegment()->getLength());
    }
}

void
MELoop::clearState() {
    myLeaderCars.clear();
}

void
MELoop::removeLeaderCar(MEVehicle* v) {
    std::vector<MEVehicle*>& cands = myLeaderCars[v->getEventTime()];
    auto it = find(cands.begin(), cands.end(), v);
    if (it != cands.end()) {
        cands.erase(it);
    }
}

void
MELoop::vaporizeCar(MEVehicle* v, MSMoveReminder::Notification reason) {
    int qIdx = 0;
    v->getSegment()->send(v, nullptr, qIdx, MSNet::getInstance()->getCurrentTimeStep(), reason);
    // try removeLeaderCar
    std::vector<MEVehicle*>& cands = myLeaderCars[v->getEventTime()];
    auto it = find(cands.begin(), cands.end(), v);
    if (it != cands.end()) {
        cands.erase(it);
    }
}

MESegment*
MELoop::nextSegment(MESegment* s, MEVehicle* v) {
    if (s != nullptr) { // vehicle is not teleporting
        MESegment* next = s->getNextSegment();
        if (next != nullptr) {
            // ok, the street continues
            return next;
        }
    }
    // we have to check the next edge in the vehicle's route
    const MSEdge* nextEdge = v->succEdge(1);
    if (nextEdge == nullptr) {
        // end of route
        return nullptr;
    }
    return myEdges2FirstSegments[nextEdge->getNumericalID()];
}


int
MELoop::numSegmentsFor(const double length, const double sLength) {
    int no = (int)floor(length / sLength + 0.5);
    if (no == 0) { // assure there is at least one segment
        return 1;
    } else {
        return no;
    }
}


void
MELoop::buildSegmentsFor(const MSEdge& e, const OptionsCont& oc) {
    const double length = e.getLength();
    const int numSegments = numSegmentsFor(length, oc.getFloat("meso-edgelength"));
    const double slength = length / (double)numSegments;
    MESegment* newSegment = nullptr;
    MESegment* nextSegment = nullptr;
    const bool laneQueue = oc.getBool("meso-lane-queue");
    bool multiQueue = laneQueue || (oc.getBool("meso-multi-queue") && e.getLanes().size() > 1 && e.getNumSuccessors() > 1);
    bool junctionControl = oc.getBool("meso-junction-control") || isEnteringRoundabout(e);
    for (int s = numSegments - 1; s >= 0; s--) {
        std::string id = e.getID() + ":" + toString(s);
        newSegment =
            new MESegment(id, e, nextSegment, slength,
                          e.getLanes()[0]->getSpeedLimit(), s,
                          string2time(oc.getString("meso-tauff")), string2time(oc.getString("meso-taufj")),
                          string2time(oc.getString("meso-taujf")), string2time(oc.getString("meso-taujj")),
                          oc.getFloat("meso-jam-threshold"), multiQueue, junctionControl);
        multiQueue = laneQueue;
        junctionControl = false;
        nextSegment = newSegment;
    }
    while (e.getNumericalID() >= static_cast<int>(myEdges2FirstSegments.size())) {
        myEdges2FirstSegments.push_back(0);
    }
    myEdges2FirstSegments[e.getNumericalID()] = newSegment;
}


MESegment*
MELoop::getSegmentForEdge(const MSEdge& e, double pos) {
    if (e.getNumericalID() >= (int)myEdges2FirstSegments.size()) {
        return nullptr;
    }
    MESegment* s = myEdges2FirstSegments[e.getNumericalID()];
    if (pos > 0) {
        double cpos = 0;
        while (s->getNextSegment() != nullptr && cpos + s->getLength() < pos) {
            cpos += s->getLength();
            s = s->getNextSegment();
        }
    }
    return s;
}


bool
MELoop::isEnteringRoundabout(const MSEdge& e) {
    for (const MSEdge* succ : e.getSuccessors()) {
        if (succ->isRoundabout()) {
            return true;
        }
    }
    return false;
}


/****************************************************************************/
