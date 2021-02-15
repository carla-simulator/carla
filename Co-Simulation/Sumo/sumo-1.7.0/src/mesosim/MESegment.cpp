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
/// @file    MESegment.cpp
/// @author  Daniel Krajzewicz
/// @date    Tue, May 2005
///
// A single mesoscopic segment (cell)
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <limits>
#include <utils/common/StdDefs.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSEdge.h>
#include <microsim/MSJunction.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSLinkCont.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSMoveReminder.h>
#include <microsim/output/MSXMLRawOut.h>
#include <microsim/output/MSDetectorFileOutput.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/devices/MSDevice.h>
#include <utils/common/FileHelpers.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/RandHelper.h>
#include "MEVehicle.h"
#include "MELoop.h"
#include "MESegment.h"

#define DEFAULT_VEH_LENGTH_WITH_GAP (SUMOVTypeParameter::getDefault().length + SUMOVTypeParameter::getDefault().minGap)
// avoid division by zero when driving very slowly
#define MESO_MIN_SPEED (0.05)
// divide tau by lane number unless we have multiple queues
#define SCALED_TAU(x) (multiQueue ? x : (SUMOTime)(x / parent.getLanes().size()))

//#define DEBUG_OPENED
//#define DEBUG_JAMTHRESHOLD
//#define DEBUG_COND (getID() == "blocker")
//#define DEBUG_COND (true)
#define DEBUG_COND (myEdge.isSelected())
#define DEBUG_COND2(obj) ((obj != 0 && (obj)->isSelected()))


// ===========================================================================
// static member defintion
// ===========================================================================
MSEdge MESegment::myDummyParent("MESegmentDummyParent", -1, SumoXMLEdgeFunc::UNKNOWN, "", "", -1, 0);
MESegment MESegment::myVaporizationTarget("vaporizationTarget");
const double MESegment::DO_NOT_PATCH_JAM_THRESHOLD(std::numeric_limits<double>::max());


// ===========================================================================
// MESegment::Queue method definitions
// ===========================================================================
MEVehicle*
MESegment::Queue::remove(MEVehicle* v) {
    myOccupancy -= v->getVehicleType().getLengthWithGap();
    assert(std::find(myVehicles.begin(), myVehicles.end(), v) != myVehicles.end());
    if (v == myVehicles.back()) {
        myVehicles.pop_back();
        if (myVehicles.empty()) {
            myOccupancy = 0.;
        } else {
            return myVehicles.back();
        }
    } else {
        myVehicles.erase(std::find(myVehicles.begin(), myVehicles.end(), v));
    }
    return nullptr;
}


// ===========================================================================
// MESegment method definitions
// ===========================================================================
MESegment::MESegment(const std::string& id,
                     const MSEdge& parent, MESegment* next,
                     const double length, const double speed,
                     const int idx,
                     const SUMOTime tauff, const SUMOTime taufj,
                     const SUMOTime taujf, const SUMOTime taujj,
                     const double jamThresh,
                     const bool multiQueue,
                     const bool junctionControl) :
    Named(id), myEdge(parent), myNextSegment(next),
    myLength(length), myIndex(idx),
    myTau_ff(SCALED_TAU(tauff)), myTau_fj(SCALED_TAU(taufj)), // Eissfeldt p. 90 and 151 ff.
    myTau_jf(SCALED_TAU(taujf)), myTau_jj(SCALED_TAU(taujj)),
    myTau_length(SCALED_TAU(TIME2STEPS(1)) / MAX2(MESO_MIN_SPEED, speed)),
    myHeadwayCapacity(length / DEFAULT_VEH_LENGTH_WITH_GAP * parent.getLanes().size())/* Eissfeldt p. 69 */,
    myCapacity(length * parent.getLanes().size()),
    myQueueCapacity(multiQueue ? length : length * parent.getLanes().size()),
    myJunctionControl(junctionControl),
    myTLSPenalty((MSGlobals::gMesoTLSPenalty > 0 || MSGlobals::gMesoTLSFlowPenalty > 0) &&
                 // only apply to the last segment of a tls-controlled edge
                 myNextSegment == nullptr && (
                     parent.getToJunction()->getType() == SumoXMLNodeType::TRAFFIC_LIGHT ||
                     parent.getToJunction()->getType() == SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION ||
                     parent.getToJunction()->getType() == SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED)),
    myMinorPenalty(MSGlobals::gMesoMinorPenalty > 0 &&
                   // only apply to the last segment of an uncontrolled edge that has at least 1 minor link
                   myNextSegment == nullptr &&
                   parent.getToJunction()->getType() != SumoXMLNodeType::TRAFFIC_LIGHT &&
                   parent.getToJunction()->getType() != SumoXMLNodeType::TRAFFIC_LIGHT_NOJUNCTION &&
                   parent.getToJunction()->getType() != SumoXMLNodeType::TRAFFIC_LIGHT_RIGHT_ON_RED &&
                   parent.hasMinorLink()),
    myNumVehicles(0),
    myLastHeadway(TIME2STEPS(-1)),
    myMeanSpeed(speed),
    myLastMeanSpeedUpdate(SUMOTime_MIN) {
    if (multiQueue) {
        const std::vector<MSLane*>& lanes = parent.getLanes();
        for (MSLane* const l : lanes) {
            myQueues.push_back(Queue(l->getPermissions()));
        }
        if (next == nullptr) {
            for (const MSEdge* const edge : parent.getSuccessors()) {
                const std::vector<MSLane*>* const allowed = parent.allowedLanes(*edge);
                assert(allowed != nullptr);
                assert(allowed->size() > 0);
                for (MSLane* const l : *allowed) {
                    std::vector<MSLane*>::const_iterator it = std::find(lanes.begin(), lanes.end(), l);
                    myFollowerMap[edge] |= (1 << distance(lanes.begin(), it));
                }
            }
        }
    } else {
        myQueues.push_back(Queue(parent.getPermissions()));
    }
    recomputeJamThreshold(jamThresh);
}


MESegment::MESegment(const std::string& id):
    Named(id),
    myEdge(myDummyParent), // arbitrary edge needed to supply the needed reference
    myNextSegment(nullptr), myLength(0), myIndex(0),
    myTau_ff(0), myTau_fj(0), myTau_jf(0), myTau_jj(0), myTau_length(1),
    myHeadwayCapacity(0), myCapacity(0), myQueueCapacity(0), myJunctionControl(false),
    myTLSPenalty(false),
    myMinorPenalty(false) {
}



void
MESegment::recomputeJamThreshold(double jamThresh) {
    if (jamThresh == DO_NOT_PATCH_JAM_THRESHOLD) {
        return;
    }
    if (jamThresh < 0) {
        // compute based on speed
        double speed = myEdge.getSpeedLimit();
        if (myTLSPenalty || myMinorPenalty) {
            double travelTime = myLength / MAX2(speed, NUMERICAL_EPS) + getMaxPenaltySeconds();
            speed = myLength / travelTime;
        }
        myJamThreshold = jamThresholdForSpeed(speed, jamThresh);
    } else {
        // compute based on specified percentage
        myJamThreshold = jamThresh * myCapacity;
    }

    // update coefficients for the jam-jam headway function
    // this function models the effect that "empty space" needs to move
    // backwards through the downstream segment before the upstream segment may
    // send annother vehicle.
    // this allows jams to clear and move upstream.
    // the headway function f(x) depends on the number of vehicles in the
    // downstream segment x
    // f is a linear function that passes through the following fixed points:
    // f(n_jam_threshold) = tau_jf_withLength (for continuity)
    // f(myHeadwayCapacity) = myTau_jj * myHeadwayCapacity

    const SUMOTime tau_jf_withLength = tauWithVehLength(myTau_jf, DEFAULT_VEH_LENGTH_WITH_GAP);
    if (myJamThreshold < myCapacity) {
        // jamming is possible
        const double n_jam_threshold = myHeadwayCapacity * myJamThreshold / myCapacity; // number of vehicles above which the segment is jammed
        // solving f(x) = a * x + b
        myA = (STEPS2TIME(myTau_jj) * myHeadwayCapacity - STEPS2TIME(tau_jf_withLength)) / (myHeadwayCapacity - n_jam_threshold);
        myB = myHeadwayCapacity * (STEPS2TIME(myTau_jj) - myA);

        // note that the original Eissfeldt model (p. 69) used different fixed points
        // f(n_jam_threshold) = n_jam_threshold * myTau_jj
        // f(myHeadwayCapacity) = myTau_jf * myHeadwayCapacity
        //
        // However, this systematically underestimates the backpropagation speed of the jam front (see #2244)
    } else {
        // dummy values. Should not be used
        myA = 0;
        myB = STEPS2TIME(tau_jf_withLength);
    }
}


double
MESegment::jamThresholdForSpeed(double speed, double jamThresh) const {
    // vehicles driving freely at maximum speed should not jam
    // we compute how many vehicles could possible enter the segment until the first vehicle leaves
    // and multiply by the space these vehicles would occupy
    // the jamThresh parameter is scale the resulting value
    if (speed == 0) {
        return std::numeric_limits<double>::max();  // never jam. Irrelevant at speed 0 anyway
    }
#ifdef DEBUG_JAMTHRESHOLD
    if (true || DEBUG_COND) {
        std::cout << "jamThresholdForSpeed seg=" << getID() << " speed=" << speed << " jamThresh=" << jamThresh << " ffVehs=" << std::ceil(myLength / (-jamThresh * speed * STEPS2TIME(tauWithVehLength(myTau_ff, DEFAULT_VEH_LENGTH_WITH_GAP)))) << " thresh=" << std::ceil(myLength / (-jamThresh * speed * STEPS2TIME(tauWithVehLength(myTau_ff, DEFAULT_VEH_LENGTH_WITH_GAP)))) * DEFAULT_VEH_LENGTH_WITH_GAP
                  << "\n";
    }
#endif
    return std::ceil(myLength / (-jamThresh * speed * STEPS2TIME(tauWithVehLength(myTau_ff, DEFAULT_VEH_LENGTH_WITH_GAP)))) * DEFAULT_VEH_LENGTH_WITH_GAP;
}


void
MESegment::addDetector(MSMoveReminder* data) {
    myDetectorData.push_back(data);
    for (const Queue& q : myQueues) {
        for (MEVehicle* const v : q.getVehicles()) {
            v->addReminder(data);
        }
    }
}


void
MESegment::removeDetector(MSMoveReminder* data) {
    std::vector<MSMoveReminder*>::iterator it = std::find(myDetectorData.begin(), myDetectorData.end(), data);
    if (it != myDetectorData.end()) {
        myDetectorData.erase(it);
    }
    for (const Queue& q : myQueues) {
        for (MEVehicle* const v : q.getVehicles()) {
            v->removeReminder(data);
        }
    }
}


void
MESegment::prepareDetectorForWriting(MSMoveReminder& data) {
    const SUMOTime currentTime = MSNet::getInstance()->getCurrentTimeStep();
    for (const Queue& q : myQueues) {
        SUMOTime earliestExitTime = currentTime;
        for (std::vector<MEVehicle*>::const_reverse_iterator i = q.getVehicles().rbegin(); i != q.getVehicles().rend(); ++i) {
            const SUMOTime exitTime = MAX2(earliestExitTime, (*i)->getEventTime());
            (*i)->updateDetectorForWriting(&data, currentTime, exitTime);
            earliestExitTime = exitTime + tauWithVehLength(myTau_ff, (*i)->getVehicleType().getLengthWithGap());
        }
    }
}


SUMOTime
MESegment::hasSpaceFor(const MEVehicle* const veh, const SUMOTime entryTime, int& qIdx, const bool init) const {
    qIdx = 0;
    if (myNumVehicles == 0 && myQueues.size() == 1) {
        // we have always space for at least one vehicle
        return entryTime;
    }
    SUMOTime earliestEntry = SUMOTime_MAX;
    const SUMOVehicleClass svc = veh->getVClass();
    int minSize = std::numeric_limits<int>::max();
    const MSEdge* const succ = myNextSegment == nullptr ? nullptr : veh->succEdge(1);
    for (int i = 0; i < (int)myQueues.size(); i++) {
        const Queue& q = myQueues[i];
        const double newOccupancy = q.size() == 0 ? 0. : q.getOccupancy() + veh->getVehicleType().getLengthWithGap();
        if (newOccupancy <= myQueueCapacity) { // we must ensure that occupancy remains below capacity
            if (succ == nullptr || myFollowerMap.count(succ) == 0 || ((myFollowerMap.find(succ)->second & (1 << i)) != 0)) {
                if (q.allows(svc) && q.size() < minSize) {
                    if (init) {
                        // regular insertions and initial insertions must respect different constraints:
                        // - regular insertions must respect entryBlockTime
                        // - initial insertions should not cause additional jamming
                        // - inserted vehicle should be able to continue at the current speed
                        if (q.getOccupancy() <= myJamThreshold && !hasBlockedLeader()) {
                            if (newOccupancy <= myJamThreshold) {
                                qIdx = i;
                                minSize = q.size();
                            }
                        } else {
                            if (newOccupancy <= jamThresholdForSpeed(getMeanSpeed(false), -1)) {
                                qIdx = i;
                                minSize = q.size();
                            }
                        }
                    } else if (entryTime >= q.getEntryBlockTime()) {
                        qIdx = i;
                        minSize = q.size();
                    } else {
                        earliestEntry = MIN2(earliestEntry, q.getEntryBlockTime());
                    }
                }
            }
        }
    }
    if (minSize == std::numeric_limits<int>::max()) {
        return earliestEntry;
    }
    return entryTime;
}


bool
MESegment::initialise(MEVehicle* veh, SUMOTime time) {
    int qIdx = 0;
    if (hasSpaceFor(veh, time, qIdx, true) == time) {
        receive(veh, qIdx, time, true);
        // we can check only after insertion because insertion may change the route via devices
        std::string msg;
        if (MSGlobals::gCheckRoutes && !veh->hasValidRoute(msg)) {
            throw ProcessError("Vehicle '" + veh->getID() + "' has no valid route. " + msg);
        }
        return true;
    }
    return false;
}


double
MESegment::getMeanSpeed(bool useCached) const {
    const SUMOTime currentTime = MSNet::getInstance()->getCurrentTimeStep();
    if (currentTime != myLastMeanSpeedUpdate || !useCached) {
        myLastMeanSpeedUpdate = currentTime;
        double v = 0;
        int count = 0;
        for (const Queue& q : myQueues) {
            const SUMOTime tau = q.getOccupancy() < myJamThreshold ? myTau_ff : myTau_jf;
            SUMOTime earliestExitTime = currentTime;
            count += q.size();
            for (std::vector<MEVehicle*>::const_reverse_iterator veh = q.getVehicles().rbegin(); veh != q.getVehicles().rend(); ++veh) {
                v += (*veh)->getConservativeSpeed(earliestExitTime); // earliestExitTime is updated!
                earliestExitTime += tauWithVehLength(tau, (*veh)->getVehicleType().getLengthWithGap());
            }
        }
        if (count == 0) {
            myMeanSpeed = myEdge.getSpeedLimit();
        } else {
            myMeanSpeed = v / (double) count;
        }
    }
    return myMeanSpeed;
}


void
MESegment::writeVehicles(OutputDevice& of) const {
    for (const Queue& q : myQueues) {
        for (const MEVehicle* const veh : q.getVehicles()) {
            MSXMLRawOut::writeVehicle(of, *veh);
        }
    }
}


MEVehicle*
MESegment::removeCar(MEVehicle* v, SUMOTime leaveTime, const MSMoveReminder::Notification reason) {
    Queue& q = myQueues[v->getQueIndex()];
    // One could be tempted to do  v->setSegment(next); here but position on lane will be invalid if next == 0
    v->updateDetectors(leaveTime, true, reason);
    myNumVehicles--;
    myEdge.lock();
    MEVehicle* nextLeader = q.remove(v);
    myEdge.unlock();
    return nextLeader;
}


SUMOTime
MESegment::getNextInsertionTime(SUMOTime earliestEntry) const {
    // since we do not know which queue will be used we give a conservative estimate
    SUMOTime earliestLeave = earliestEntry;
    SUMOTime latestEntry = -1;
    for (const Queue& q : myQueues) {
        earliestLeave = MAX2(earliestLeave, q.getBlockTime());
        latestEntry = MAX2(latestEntry, q.getEntryBlockTime());
    }
    if (myEdge.getSpeedLimit() == 0) {
        return MAX2(earliestEntry, latestEntry);    // FIXME: This line is just an adhoc-fix to avoid division by zero (Leo)
    } else {
        return MAX3(earliestEntry, earliestLeave - TIME2STEPS(myLength / myEdge.getSpeedLimit()), latestEntry);
    }
}


MSLink*
MESegment::getLink(const MEVehicle* veh, bool penalty) const {
    if (myJunctionControl || penalty) {
        const MSEdge* const nextEdge = veh->succEdge(1);
        if (nextEdge == nullptr) {
            return nullptr;
        }
        // try to find any link leading to our next edge, start with the lane pointed to by the que index
        const MSLane* const bestLane = myEdge.getLanes()[veh->getQueIndex()];
        for (MSLink* const link : bestLane->getLinkCont()) {
            if (&link->getLane()->getEdge() == nextEdge) {
                return link;
            }
        }
        // this is for the non-multique case, maybe we should use caching here !!!
        for (const MSLane* const lane : myEdge.getLanes()) {
            if (lane != bestLane) {
                for (MSLink* const link : lane->getLinkCont()) {
                    if (&link->getLane()->getEdge() == nextEdge) {
                        return link;
                    }
                }
            }
        }
    }
    return nullptr;
}


bool
MESegment::isOpen(const MEVehicle* veh) const {
#ifdef DEBUG_OPENED
    if (DEBUG_COND || DEBUG_COND2(veh)) {
        std::cout << SIMTIME << " opened seg=" << getID() << " veh=" << Named::getIDSecure(veh)
                  << " tlsPenalty=" << myTLSPenalty;
        const MSLink* link = getLink(veh);
        if (link == 0) {
            std::cout << " link=0";
        } else {
            std::cout << " prio=" << link->havePriority()
                      << " override=" << limitedControlOverride(link)
                      << " isOpen=" << link->opened(veh->getEventTime(), veh->getSpeed(), veh->estimateLeaveSpeed(link),
                                                    veh->getVehicleType().getLengthWithGap(), veh->getImpatience(),
                                                    veh->getVehicleType().getCarFollowModel().getMaxDecel(), veh->getWaitingTime())
                      << " et=" << veh->getEventTime()
                      << " v=" << veh->getSpeed()
                      << " vLeave=" << veh->estimateLeaveSpeed(link)
                      << " impatience=" << veh->getImpatience()
                      << " tWait=" << veh->getWaitingTime();
        }
        std::cout << "\n";
    }
#endif
    if (myTLSPenalty) {
        // XXX should limited control take precedence over tls penalty?
        return true;
    }
    const MSLink* link = getLink(veh);
    return (link == nullptr
            || link->havePriority()
            || limitedControlOverride(link)
            || link->opened(veh->getEventTime(), veh->getSpeed(), veh->estimateLeaveSpeed(link),
                            veh->getVehicleType().getLengthWithGap(), veh->getImpatience(),
                            veh->getVehicleType().getCarFollowModel().getMaxDecel(), veh->getWaitingTime()));
}


bool
MESegment::limitedControlOverride(const MSLink* link) const {
    assert(link != nullptr);
    if (!MSGlobals::gMesoLimitedJunctionControl) {
        return false;
    }
    // if the target segment of this link is not saturated junction control is disabled
    const MSEdge& targetEdge = link->getLane()->getEdge();
    const MESegment* target = MSGlobals::gMesoNet->getSegmentForEdge(targetEdge);
    return (target->getBruttoOccupancy() * 2 < target->myJamThreshold) && !targetEdge.isRoundabout();
}


void
MESegment::send(MEVehicle* veh, MESegment* const next, const int nextQIdx, SUMOTime time, const MSMoveReminder::Notification reason) {
    Queue& q = myQueues[veh->getQueIndex()];
    assert(isInvalid(next) || time >= q.getBlockTime());
    MSLink* const link = getLink(veh);
    if (link != nullptr) {
        link->removeApproaching(veh);
    }
    MEVehicle* lc = removeCar(veh, time, reason); // new leaderCar
    q.setBlockTime(time);
    if (!isInvalid(next)) {
        const bool nextFree = next->myQueues[nextQIdx].getOccupancy() <= next->myJamThreshold;
        const SUMOTime tau = (q.getOccupancy() <= myJamThreshold
                              ? (nextFree ? myTau_ff : myTau_fj)
                              : (nextFree ? myTau_jf : TIME2STEPS(myA * q.size() + myB)));
        myLastHeadway = tauWithVehLength(tau, veh->getVehicleType().getLengthWithGap());
        if (myTLSPenalty) {
            const MSLink* const tllink = getLink(veh, true);
            if (tllink != nullptr) {
                assert(tllink->isTLSControlled());
                assert(tllink->getGreenFraction() > 0);
                myLastHeadway = (SUMOTime)(myLastHeadway / tllink->getGreenFraction());
            }
        }
        q.setBlockTime(q.getBlockTime() + myLastHeadway);
    }
    if (lc != nullptr) {
        lc->setEventTime(MAX2(lc->getEventTime(), q.getBlockTime()));
        MSGlobals::gMesoNet->addLeaderCar(lc, getLink(lc));
    }
    if (veh->isStopped()) {
        veh->processStop();
    }
}


bool
MESegment::overtake() {
    return MSGlobals::gMesoOvertaking && myCapacity > myLength && RandHelper::rand() > (getBruttoOccupancy() / myCapacity);
}


void
MESegment::addReminders(MEVehicle* veh) const {
    for (std::vector<MSMoveReminder*>::const_iterator i = myDetectorData.begin(); i != myDetectorData.end(); ++i) {
        veh->addReminder(*i);
    }
}


void
MESegment::receive(MEVehicle* veh, const int qIdx, SUMOTime time, const bool isDepart, const bool isTeleport, const bool newEdge) {
    const double speed = isDepart ? -1 : MAX2(veh->getSpeed(), MESO_MIN_SPEED); // on the previous segment
    veh->setSegment(this); // for arrival checking
    veh->setLastEntryTime(time);
    veh->setBlockTime(SUMOTime_MAX);
    if (!isDepart && (
                // arrival on entering a new edge
                (newEdge && veh->moveRoutePointer())
                // arrival on entering a new segment
                || veh->hasArrived())) {
        // route has ended
        veh->setEventTime(time + TIME2STEPS(myLength / speed)); // for correct arrival speed
        addReminders(veh);
        veh->activateReminders(MSMoveReminder::NOTIFICATION_JUNCTION);
        veh->updateDetectors(time, true,
                             veh->getEdge()->isVaporizing() ? MSMoveReminder::NOTIFICATION_VAPORIZED_VAPORIZER : MSMoveReminder::NOTIFICATION_ARRIVED);
        MSNet::getInstance()->getVehicleControl().scheduleVehicleRemoval(veh);
        return;
    }
    assert(veh->getEdge() == &getEdge());
    // route continues
    const double maxSpeedOnEdge = veh->getEdge()->getVehicleMaxSpeed(veh);
    const double uspeed = MAX2(maxSpeedOnEdge, MESO_MIN_SPEED);
    Queue& q = myQueues[qIdx];
    std::vector<MEVehicle*>& cars = q.getModifiableVehicles();
    MEVehicle* newLeader = nullptr; // first vehicle in the current queue
    SUMOTime tleave = MAX2(veh->getStoptime(this, time) + TIME2STEPS(myLength / uspeed) + getLinkPenalty(veh), q.getBlockTime());
    if (veh->isStopped()) {
        myEdge.addWaiting(veh);
    }
    myEdge.lock();
    if (cars.empty()) {
        cars.push_back(veh);
        newLeader = veh;
    } else {
        SUMOTime leaderOut = cars[0]->getEventTime();
        if (!isDepart && leaderOut > tleave && overtake()) {
            if (cars.size() == 1) {
                MSGlobals::gMesoNet->removeLeaderCar(cars[0]);
                newLeader = veh;
            }
            cars.insert(cars.begin() + 1, veh);
        } else {
            tleave = MAX2(leaderOut + tauWithVehLength(myTau_ff, cars[0]->getVehicleType().getLengthWithGap()), tleave);
            cars.insert(cars.begin(), veh);
        }
    }
    myEdge.unlock();
    myNumVehicles++;
    if (!isDepart && !isTeleport) {
        // departs and teleports could take place anywhere on the edge so they should not block regular flow
        // the -1 facilitates interleaving of multiple streams
        q.setEntryBlockTime(time + tauWithVehLength(myTau_ff, veh->getVehicleType().getLengthWithGap()) - 1);
    }
    veh->setEventTime(tleave);
    veh->setSegment(this, qIdx);
    q.setOccupancy(MIN2(myQueueCapacity, q.getOccupancy() + veh->getVehicleType().getLengthWithGap()));
    addReminders(veh);
    if (isDepart) {
        veh->onDepart();
        veh->activateReminders(MSMoveReminder::NOTIFICATION_DEPARTED);
    } else if (newEdge) {
        veh->activateReminders(MSMoveReminder::NOTIFICATION_JUNCTION);
    } else {
        veh->activateReminders(MSMoveReminder::NOTIFICATION_SEGMENT);
    }
    if (newLeader != nullptr) {
        MSGlobals::gMesoNet->addLeaderCar(newLeader, getLink(newLeader));
    }
}


bool
MESegment::vaporizeAnyCar(SUMOTime currentTime, const MSDetectorFileOutput* filter) {
    for (const Queue& q : myQueues) {
        if (q.size() > 0) {
            for (MEVehicle* const veh : q.getVehicles()) {
                if (filter->vehicleApplies(*veh)) {
                    MSGlobals::gMesoNet->removeLeaderCar(veh);
                    MSGlobals::gMesoNet->changeSegment(veh, currentTime + 1, &myVaporizationTarget, MSMoveReminder::NOTIFICATION_VAPORIZED_CALIBRATOR);
                    return true;
                }
            }
        }
    }
    return false;
}


void
MESegment::setSpeedForQueue(double newSpeed, SUMOTime currentTime, SUMOTime blockTime, const std::vector<MEVehicle*>& vehs) {
    MEVehicle* v = vehs.back();
    v->updateDetectors(currentTime, false);
    SUMOTime newEvent = MAX2(newArrival(v, newSpeed, currentTime), blockTime);
    if (v->getEventTime() != newEvent) {
        MSGlobals::gMesoNet->removeLeaderCar(v);
        v->setEventTime(newEvent);
        MSGlobals::gMesoNet->addLeaderCar(v, getLink(v));
    }
    for (std::vector<MEVehicle*>::const_reverse_iterator i = vehs.rbegin() + 1; i != vehs.rend(); ++i) {
        (*i)->updateDetectors(currentTime, false);
        newEvent = MAX2(newArrival(*i, newSpeed, currentTime), newEvent + myTau_ff);
        //newEvent = MAX2(newArrival(*i, newSpeed, currentTime), newEvent + myTau_ff + (SUMOTime)((*(i - 1))->getVehicleType().getLength() / myTau_length));
        (*i)->setEventTime(newEvent);
    }
}


SUMOTime
MESegment::newArrival(const MEVehicle* const v, double newSpeed, SUMOTime currentTime) {
    // since speed is only an upper bound pos may be to optimistic
    const double pos = MIN2(myLength, STEPS2TIME(currentTime - v->getLastEntryTime()) * v->getSpeed());
    // traveltime may not be 0
    return currentTime + MAX2(TIME2STEPS((myLength - pos) / newSpeed), SUMOTime(1));
}


void
MESegment::setSpeed(double newSpeed, SUMOTime currentTime, double jamThresh) {
    recomputeJamThreshold(jamThresh);
    //myTau_length = MAX2(MESO_MIN_SPEED, newSpeed) * myEdge.getLanes().size() / TIME2STEPS(1);
    for (const Queue& q : myQueues) {
        if (q.size() != 0) {
            setSpeedForQueue(newSpeed, currentTime, q.getBlockTime(), q.getVehicles());
        }
    }
}


SUMOTime
MESegment::getEventTime() const {
    SUMOTime result = SUMOTime_MAX;
    for (const Queue& q : myQueues) {
        if (q.size() != 0 && q.getVehicles().back()->getEventTime() < result) {
            result = q.getVehicles().back()->getEventTime();
        }
    }
    if (result < SUMOTime_MAX) {
        return result;
    }
    return -1;
}


void
MESegment::saveState(OutputDevice& out) {
    out.openTag(SUMO_TAG_SEGMENT);
    for (const Queue& q : myQueues) {
        out.openTag(SUMO_TAG_VIEWSETTINGS_VEHICLES).writeAttr(SUMO_ATTR_TIME, toString<SUMOTime>(q.getBlockTime()));
        out.writeAttr(SUMO_ATTR_VALUE, q.getVehicles());
        out.closeTag();
    }
    out.closeTag();
}


void
MESegment::clearState() {
    for (Queue& q : myQueues) {
        q.getModifiableVehicles().clear();
    }
}

void
MESegment::loadState(const std::vector<std::string>& vehIds, MSVehicleControl& vc, const SUMOTime block, const int queIdx) {
    Queue& q = myQueues[queIdx];
    for (const std::string& id : vehIds) {
        MEVehicle* v = static_cast<MEVehicle*>(vc.getVehicle(id));
        // vehicle could be removed due to options
        if (v != nullptr) {
            assert(v->getSegment() == this);
            q.getModifiableVehicles().push_back(v);
            myNumVehicles++;
            q.setOccupancy(q.getOccupancy() + v->getVehicleType().getLengthWithGap());
        }
    }
    if (q.size() != 0) {
        // add the last vehicle of this queue
        // !!! one question - what about the previously added vehicle? Is it stored twice?
        MEVehicle* veh = q.getVehicles().back();
        MSGlobals::gMesoNet->addLeaderCar(veh, getLink(veh));
    }
    q.setBlockTime(block);
    q.setOccupancy(MIN2(q.getOccupancy(), myQueueCapacity));
}


std::vector<const MEVehicle*>
MESegment::getVehicles() const {
    std::vector<const MEVehicle*> result;
    for (const Queue& q : myQueues) {
        result.insert(result.end(), q.getVehicles().begin(), q.getVehicles().end());
    }
    return result;
}


bool
MESegment::hasBlockedLeader() const {
    for (const Queue& q : myQueues) {
        if (q.size() > 0 && q.getVehicles().back()->getWaitingTime() > 0) {
            return true;
        }
    }
    return false;
}


double
MESegment::getFlow() const {
    return 3600 * getCarNumber() * getMeanSpeed() / myLength;
}


SUMOTime
MESegment::getLinkPenalty(const MEVehicle* veh) const {
    const MSLink* link = getLink(veh, myTLSPenalty || myMinorPenalty);
    if (link != nullptr) {
        SUMOTime result = 0;
        if (link->isTLSControlled()) {
            result += link->getMesoTLSPenalty();
        }
        // minor tls links may get an additional penalty
        if (!link->havePriority() &&
                // do not apply penalty if limited control is active
                (!MSGlobals::gMesoLimitedJunctionControl || limitedControlOverride(link))) {
            result += MSGlobals::gMesoMinorPenalty;
        }
        return result;
    } else {
        return 0;
    }
}


double
MESegment::getMaxPenaltySeconds() const {
    double maxPenalty = 0;
    for (std::vector<MSLane*>::const_iterator i = myEdge.getLanes().begin(); i != myEdge.getLanes().end(); ++i) {
        MSLane* l = *i;
        const MSLinkCont& lc = l->getLinkCont();
        for (MSLinkCont::const_iterator j = lc.begin(); j != lc.end(); ++j) {
            MSLink* link = *j;
            maxPenalty = MAX2(maxPenalty, STEPS2TIME(
                                  link->getMesoTLSPenalty() + (link->havePriority() ? 0 : MSGlobals::gMesoMinorPenalty)));
        }
    }
    return maxPenalty;
}


double
MESegment::getWaitingSeconds() const {
    double result = 0;
    for (const Queue& q : myQueues) {
        // @note: only the leader currently accumulates waitingTime but this might change in the future
        for (const MEVehicle* veh : q.getVehicles()) {
            result += veh->getWaitingSeconds();
        }
    }
    return result;
}


/****************************************************************************/
