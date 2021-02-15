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
/// @file    MSDevice_BTreceiver.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    14.08.2013
///
// A BT Receiver
/****************************************************************************/
#include <config.h>

#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSEventControl.h>
#include "MSDevice_Tripinfo.h"
#include "MSDevice_BTreceiver.h"
#include "MSDevice_BTsender.h"


// ===========================================================================
// static members
// ===========================================================================
bool MSDevice_BTreceiver::myWasInitialised = false;
double MSDevice_BTreceiver::myRange = -1.;
double MSDevice_BTreceiver::myOffTime = -1.;
std::mt19937 MSDevice_BTreceiver::sRecognitionRNG;
std::map<std::string, MSDevice_BTreceiver::VehicleInformation*> MSDevice_BTreceiver::sVehicles;


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_BTreceiver::insertOptions(OptionsCont& oc) {
    insertDefaultAssignmentOptions("btreceiver", "Communication", oc);

    oc.doRegister("device.btreceiver.range", new Option_Float(300));
    oc.addDescription("device.btreceiver.range", "Communication", "The range of the bt receiver");

    oc.doRegister("device.btreceiver.all-recognitions", new Option_Bool(false));
    oc.addDescription("device.btreceiver.all-recognitions", "Communication", "Whether all recognition point shall be written");

    oc.doRegister("device.btreceiver.offtime", new Option_Float(0.64));
    oc.addDescription("device.btreceiver.offtime", "Communication", "The offtime used for calculating detection probability (in seconds)");

    myWasInitialised = false;
}


void
MSDevice_BTreceiver::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into) {
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "btreceiver", v, false)) {
        MSDevice_BTreceiver* device = new MSDevice_BTreceiver(v, "btreceiver_" + v.getID());
        into.push_back(device);
        if (!myWasInitialised) {
            new BTreceiverUpdate();
            myWasInitialised = true;
            myRange = oc.getFloat("device.btreceiver.range");
            myOffTime = oc.getFloat("device.btreceiver.offtime");
            sRecognitionRNG.seed(oc.getInt("seed"));
        }
    }
}


// ---------------------------------------------------------------------------
// MSDevice_BTreceiver::BTreceiverUpdate-methods
// ---------------------------------------------------------------------------
MSDevice_BTreceiver::BTreceiverUpdate::BTreceiverUpdate() {
    MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(this);
}


MSDevice_BTreceiver::BTreceiverUpdate::~BTreceiverUpdate() {
    for (std::map<std::string, MSDevice_BTsender::VehicleInformation*>::const_iterator i = MSDevice_BTsender::sVehicles.begin(); i != MSDevice_BTsender::sVehicles.end(); ++i) {
        (*i).second->amOnNet = false;
        (*i).second->haveArrived = true;
    }
    for (std::map<std::string, MSDevice_BTreceiver::VehicleInformation*>::const_iterator i = MSDevice_BTreceiver::sVehicles.begin(); i != MSDevice_BTreceiver::sVehicles.end(); ++i) {
        (*i).second->amOnNet = false;
        (*i).second->haveArrived = true;
    }
    execute(MSNet::getInstance()->getCurrentTimeStep());
}


SUMOTime
MSDevice_BTreceiver::BTreceiverUpdate::execute(SUMOTime /*currentTime*/) {
    // build rtree with senders
    NamedRTree rt;
    for (std::map<std::string, MSDevice_BTsender::VehicleInformation*>::const_iterator i = MSDevice_BTsender::sVehicles.begin(); i != MSDevice_BTsender::sVehicles.end(); ++i) {
        MSDevice_BTsender::VehicleInformation* vi = (*i).second;
        Boundary b = vi->getBoxBoundary();
        b.grow(POSITION_EPS);
        const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
        const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
        rt.Insert(cmin, cmax, vi);
    }

    // check visibility for all receivers
    OptionsCont& oc = OptionsCont::getOptions();
    bool allRecognitions = oc.getBool("device.btreceiver.all-recognitions");
    bool haveOutput = oc.isSet("bt-output");
    for (std::map<std::string, MSDevice_BTreceiver::VehicleInformation*>::iterator i = MSDevice_BTreceiver::sVehicles.begin(); i != MSDevice_BTreceiver::sVehicles.end();) {
        // collect surrounding vehicles
        MSDevice_BTreceiver::VehicleInformation* vi = (*i).second;
        Boundary b = vi->getBoxBoundary();
        b.grow(vi->range);
        const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
        const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
        std::set<const Named*> surroundingVehicles;
        Named::StoringVisitor sv(surroundingVehicles);
        rt.Search(cmin, cmax, sv);

        // loop over surrounding vehicles, check visibility status
        for (const Named* vehicle : surroundingVehicles) {
            if ((*i).first == vehicle->getID()) {
                // seeing oneself? skip
                continue;
            }
            updateVisibility(*vi, *MSDevice_BTsender::sVehicles.find(vehicle->getID())->second);
        }

        if (vi->haveArrived) {
            // vehicle has left the simulation; remove
            if (haveOutput) {
                writeOutput((*i).first, vi->seen, allRecognitions);
            }
            delete vi;
            MSDevice_BTreceiver::sVehicles.erase(i++);
        } else {
            // vehicle is still in the simulation; reset state
            vi->updates.erase(vi->updates.begin(), vi->updates.end() - 1);
            ++i;
        }
    }

    // remove arrived senders / reset state
    for (std::map<std::string, MSDevice_BTsender::VehicleInformation*>::iterator i = MSDevice_BTsender::sVehicles.begin(); i != MSDevice_BTsender::sVehicles.end();) {
        MSDevice_BTsender::VehicleInformation* vi = (*i).second;
        if (vi->haveArrived) {
            delete vi;
            MSDevice_BTsender::sVehicles.erase(i++);
        } else {
            vi->updates.erase(vi->updates.begin(), vi->updates.end() - 1);
            ++i;
        }
    }
    return DELTA_T;
}


void
MSDevice_BTreceiver::BTreceiverUpdate::updateVisibility(MSDevice_BTreceiver::VehicleInformation& receiver,
        MSDevice_BTsender::VehicleInformation& sender) {
    const MSDevice_BTsender::VehicleState& receiverData = receiver.updates.back();
    const MSDevice_BTsender::VehicleState& senderData = sender.updates.back();
    if (!receiver.amOnNet || !sender.amOnNet) {
        // at least one of the vehicles has left the simulation area for any reason
        if (receiver.currentlySeen.find(sender.getID()) != receiver.currentlySeen.end()) {
            leaveRange(receiver, receiverData, sender, senderData, 0);
        }
    }

    const Position& oldReceiverPosition = receiver.updates.front().position;
    const Position& oldSenderPosition = sender.updates.front().position;

    // let the other's current position be the one obtained by applying the relative direction vector to the initial position
    const Position senderDelta = senderData.position - oldSenderPosition;
    const Position receiverDelta = receiverData.position - oldReceiverPosition;
    const Position translatedSender = senderData.position - receiverDelta;
    // find crossing points
    std::vector<double> intersections;
    GeomHelper::findLineCircleIntersections(oldReceiverPosition, receiver.range, oldSenderPosition, translatedSender, intersections);
    switch (intersections.size()) {
        case 0:
            // no intersections -> other vehicle either stays within or beyond range
            if (receiver.amOnNet && sender.amOnNet && receiverData.position.distanceTo(senderData.position) < receiver.range) {
                if (receiver.currentlySeen.find(sender.getID()) == receiver.currentlySeen.end()) {
                    enterRange(0., receiverData, sender.getID(), senderData, receiver.currentlySeen);
                } else {
                    addRecognitionPoint(SIMTIME, receiverData, senderData, receiver.currentlySeen[sender.getID()]);
                }
            } else {
                if (receiver.currentlySeen.find(sender.getID()) != receiver.currentlySeen.end()) {
                    leaveRange(receiver, receiverData, sender, senderData, 0.);
                }
            }
            break;
        case 1: {
            // one intersection -> other vehicle either enters or leaves the range
            MSDevice_BTsender::VehicleState intersection1ReceiverData(receiverData);
            intersection1ReceiverData.position = oldReceiverPosition + receiverDelta * intersections.front();
            MSDevice_BTsender::VehicleState intersection1SenderData(senderData);
            intersection1SenderData.position = oldSenderPosition + senderDelta * intersections.front();
            if (receiver.currentlySeen.find(sender.getID()) != receiver.currentlySeen.end()) {
                leaveRange(receiver, intersection1ReceiverData,
                           sender, intersection1SenderData, (intersections.front() - 1.) * TS);
            } else {
                enterRange((intersections.front() - 1.) * TS, intersection1ReceiverData,
                           sender.getID(), intersection1SenderData, receiver.currentlySeen);
            }
        }
        break;
        case 2:
            // two intersections -> other vehicle enters and leaves the range
            if (receiver.currentlySeen.find(sender.getID()) == receiver.currentlySeen.end()) {
                MSDevice_BTsender::VehicleState intersectionReceiverData(receiverData);
                intersectionReceiverData.position = oldReceiverPosition + receiverDelta * intersections.front();
                MSDevice_BTsender::VehicleState intersectionSenderData(senderData);
                intersectionSenderData.position = oldSenderPosition + senderDelta * intersections.front();
                enterRange((intersections.front() - 1.) * TS, intersectionReceiverData,
                           sender.getID(), intersectionSenderData, receiver.currentlySeen);
                intersectionReceiverData.position = oldReceiverPosition + receiverDelta * intersections.back();
                intersectionSenderData.position = oldSenderPosition + senderDelta * intersections.back();
                leaveRange(receiver, intersectionReceiverData,
                           sender, intersectionSenderData, (intersections.back() - 1.) * TS);
            } else {
                WRITE_WARNING("The vehicle '" + sender.getID() + "' cannot be in the range of vehicle '" + receiver.getID() + "', leave, and enter it in one step.");
            }
            break;
        default:
            WRITE_WARNING("Nope, a circle cannot be crossed more often than twice by a line.");
            break;
    }
}


void
MSDevice_BTreceiver::BTreceiverUpdate::enterRange(double atOffset, const MSDevice_BTsender::VehicleState& receiverState,
        const std::string& senderID, const MSDevice_BTsender::VehicleState& senderState,
        std::map<std::string, SeenDevice*>& currentlySeen) {
    MeetingPoint mp(SIMTIME + atOffset, receiverState, senderState);
    SeenDevice* sd = new SeenDevice(mp);
    currentlySeen[senderID] = sd;
    addRecognitionPoint(SIMTIME, receiverState, senderState, sd);
}


void
MSDevice_BTreceiver::BTreceiverUpdate::leaveRange(VehicleInformation& receiverInfo, const MSDevice_BTsender::VehicleState& receiverState,
        MSDevice_BTsender::VehicleInformation& senderInfo, const MSDevice_BTsender::VehicleState& senderState,
        double tOffset) {
    std::map<std::string, SeenDevice*>::iterator i = receiverInfo.currentlySeen.find(senderInfo.getID());
    // check whether the other was recognized
    addRecognitionPoint(SIMTIME + tOffset, receiverState, senderState, i->second);
    // build leaving point
    i->second->meetingEnd = new MeetingPoint(STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep()) + tOffset, receiverState, senderState);
    ConstMSEdgeVector::const_iterator begin = receiverInfo.route.begin() + i->second->meetingBegin.observerState.routePos;
    ConstMSEdgeVector::const_iterator end = receiverInfo.route.begin() + receiverState.routePos + 1;
    i->second->receiverRoute = toString<const MSEdge>(begin, end);
    begin = senderInfo.route.begin() + i->second->meetingBegin.seenState.routePos;
    end = senderInfo.route.begin() + senderState.routePos + 1;
    i->second->senderRoute = toString<const MSEdge>(begin, end);
    receiverInfo.seen[senderInfo.getID()].push_back(i->second);
    receiverInfo.currentlySeen.erase(i);
}


double
MSDevice_BTreceiver::inquiryDelaySlots(const int backoffLimit) {
    const int phaseOffset = RandHelper::rand(2047, &sRecognitionRNG);
    const bool interlaced = RandHelper::rand(&sRecognitionRNG) < 0.7;
    const double delaySlots = RandHelper::rand(&sRecognitionRNG) * 15;
    const int backoff = RandHelper::rand(backoffLimit, &sRecognitionRNG);
    if (interlaced) {
        return RandHelper::rand(&sRecognitionRNG) * 31 + backoff;
    }
    if (RandHelper::rand(31, &sRecognitionRNG) < 16) {
        // correct train for f0
        return delaySlots + backoff;
    }
    if (RandHelper::rand(30, &sRecognitionRNG) < 16) {
        // correct train for f1
        return 2048 - phaseOffset + delaySlots + backoff;
    }
    if (RandHelper::rand(29, &sRecognitionRNG) < 16) {
        // f2 is in train A but has overlap with both trains
        if (2 * 2048 - phaseOffset + backoff < 4096) {
            return 2 * 2048 - phaseOffset + delaySlots + backoff;
        }
        // the following is wrong but should only happen in about 3% of the non-interlaced cases
        return 2 * 2048 - phaseOffset + delaySlots + backoff;
    }
    return 2 * 2048 + delaySlots + backoff;
}


void
MSDevice_BTreceiver::BTreceiverUpdate::addRecognitionPoint(const double tEnd, const MSDevice_BTsender::VehicleState& receiverState,
        const MSDevice_BTsender::VehicleState& senderState,
        SeenDevice* senderDevice) const {
    if (senderDevice->nextView == -1.) {
        senderDevice->nextView = senderDevice->lastView + inquiryDelaySlots(int(myOffTime / 0.000625 + .5)) * 0.000625;
    }
    if (tEnd > senderDevice->nextView) {
        senderDevice->lastView = senderDevice->nextView;
        MeetingPoint* mp = new MeetingPoint(tEnd, receiverState, senderState);
        senderDevice->recognitionPoints.push_back(mp);
        senderDevice->nextView = senderDevice->lastView + inquiryDelaySlots(int(myOffTime / 0.000625 + .5)) * 0.000625;
    }
}


void
MSDevice_BTreceiver::BTreceiverUpdate::writeOutput(const std::string& id, const std::map<std::string, std::vector<SeenDevice*> >& seen, bool allRecognitions) {
    OutputDevice& os = OutputDevice::getDeviceByOption("bt-output");
    os.openTag("bt").writeAttr("id", id);
    for (std::map<std::string, std::vector<SeenDevice*> >::const_iterator j = seen.begin(); j != seen.end(); ++j) {
        const std::vector<SeenDevice*>& sts = (*j).second;
        for (std::vector<SeenDevice*>::const_iterator k = sts.begin(); k != sts.end(); ++k) {
            os.openTag("seen").writeAttr("id", (*j).first);
            const MSDevice_BTsender::VehicleState& obsBeg = (*k)->meetingBegin.observerState;
            const MSDevice_BTsender::VehicleState& seenBeg = (*k)->meetingBegin.seenState;
            os.writeAttr("tBeg", (*k)->meetingBegin.t)
            .writeAttr("observerPosBeg", obsBeg.position).writeAttr("observerSpeedBeg", obsBeg.speed)
            .writeAttr("observerLaneIDBeg", obsBeg.laneID).writeAttr("observerLanePosBeg", obsBeg.lanePos)
            .writeAttr("seenPosBeg", seenBeg.position).writeAttr("seenSpeedBeg", seenBeg.speed)
            .writeAttr("seenLaneIDBeg", seenBeg.laneID).writeAttr("seenLanePosBeg", seenBeg.lanePos);
            const MSDevice_BTsender::VehicleState& obsEnd = (*k)->meetingEnd->observerState;
            const MSDevice_BTsender::VehicleState& seenEnd = (*k)->meetingEnd->seenState;
            os.writeAttr("tEnd", (*k)->meetingEnd->t)
            .writeAttr("observerPosEnd", obsEnd.position).writeAttr("observerSpeedEnd", obsEnd.speed)
            .writeAttr("observerLaneIDEnd", obsEnd.laneID).writeAttr("observerLanePosEnd", obsEnd.lanePos)
            .writeAttr("seenPosEnd", seenEnd.position).writeAttr("seenSpeedEnd", seenEnd.speed)
            .writeAttr("seenLaneIDEnd", seenEnd.laneID).writeAttr("seenLanePosEnd", seenEnd.lanePos)
            .writeAttr("observerRoute", (*k)->receiverRoute).writeAttr("seenRoute", (*k)->senderRoute);
            for (std::vector<MeetingPoint*>::iterator l = (*k)->recognitionPoints.begin(); l != (*k)->recognitionPoints.end(); ++l) {
                os.openTag("recognitionPoint").writeAttr("t", (*l)->t)
                .writeAttr("observerPos", (*l)->observerState.position).writeAttr("observerSpeed", (*l)->observerState.speed)
                .writeAttr("observerLaneID", (*l)->observerState.laneID).writeAttr("observerLanePos", (*l)->observerState.lanePos)
                .writeAttr("seenPos", (*l)->seenState.position).writeAttr("seenSpeed", (*l)->seenState.speed)
                .writeAttr("seenLaneID", (*l)->seenState.laneID).writeAttr("seenLanePos", (*l)->seenState.lanePos)
                .closeTag();
                if (!allRecognitions) {
                    break;
                }
            }
            os.closeTag();
        }
    }
    os.closeTag();
}




// ---------------------------------------------------------------------------
// MSDevice_BTreceiver-methods
// ---------------------------------------------------------------------------
MSDevice_BTreceiver::MSDevice_BTreceiver(SUMOVehicle& holder, const std::string& id)
    : MSVehicleDevice(holder, id) {
}


MSDevice_BTreceiver::~MSDevice_BTreceiver() {
}


bool
MSDevice_BTreceiver::notifyEnter(SUMOTrafficObject& veh, Notification reason, const MSLane* /* enteredLane */) {
    if (reason == MSMoveReminder::NOTIFICATION_DEPARTED && sVehicles.find(veh.getID()) == sVehicles.end()) {
        sVehicles[veh.getID()] = new VehicleInformation(veh.getID(), myRange);
        sVehicles[veh.getID()]->route.push_back(veh.getEdge());
    }
    if (reason == MSMoveReminder::NOTIFICATION_TELEPORT && sVehicles.find(veh.getID()) != sVehicles.end()) {
        sVehicles[veh.getID()]->amOnNet = true;
    }
    if (reason == MSMoveReminder::NOTIFICATION_TELEPORT || reason == MSMoveReminder::NOTIFICATION_JUNCTION) {
        sVehicles[veh.getID()]->route.push_back(veh.getEdge());
    }
    const std::string location = MSGlobals::gUseMesoSim ? veh.getEdge()->getID() : static_cast<MSVehicle&>(veh).getLane()->getID();
    const MSBaseVehicle& v = static_cast<MSBaseVehicle&>(veh);
    sVehicles[veh.getID()]->updates.push_back(MSDevice_BTsender::VehicleState(veh.getSpeed(), veh.getPosition(), location, veh.getPositionOnLane(), v.getRoutePosition()));
    return true;
}


bool
MSDevice_BTreceiver::notifyMove(SUMOTrafficObject& veh, double /* oldPos */, double newPos, double newSpeed) {
    if (sVehicles.find(veh.getID()) == sVehicles.end()) {
        WRITE_WARNING("btreceiver: Can not update position of vehicle '" + veh.getID() + "' which is not on the road.");
        return true;
    }
    const std::string location = MSGlobals::gUseMesoSim ? veh.getEdge()->getID() : static_cast<MSVehicle&>(veh).getLane()->getID();
    const MSBaseVehicle& v = static_cast<MSBaseVehicle&>(veh);
    sVehicles[veh.getID()]->updates.push_back(MSDevice_BTsender::VehicleState(newSpeed, veh.getPosition(), location, newPos, v.getRoutePosition()));
    return true;
}


bool
MSDevice_BTreceiver::notifyLeave(SUMOTrafficObject& veh, double /* lastPos */, Notification reason, const MSLane* /* enteredLane */) {
    if (reason < MSMoveReminder::NOTIFICATION_TELEPORT) {
        return true;
    }
    if (sVehicles.find(veh.getID()) == sVehicles.end()) {
        WRITE_WARNING("btreceiver: Can not update position of vehicle '" + veh.getID() + "' which is not on the road.");
        return true;
    }
    const std::string location = MSGlobals::gUseMesoSim ? veh.getEdge()->getID() : static_cast<MSVehicle&>(veh).getLane()->getID();
    const MSBaseVehicle& v = static_cast<MSBaseVehicle&>(veh);
    sVehicles[veh.getID()]->updates.push_back(MSDevice_BTsender::VehicleState(veh.getSpeed(), veh.getPosition(), location, veh.getPositionOnLane(), v.getRoutePosition()));
    if (reason == MSMoveReminder::NOTIFICATION_TELEPORT) {
        sVehicles[veh.getID()]->amOnNet = false;
    }
    if (reason >= MSMoveReminder::NOTIFICATION_ARRIVED) {
        sVehicles[veh.getID()]->amOnNet = false;
        sVehicles[veh.getID()]->haveArrived = true;
    }
    return true;
}


/****************************************************************************/
