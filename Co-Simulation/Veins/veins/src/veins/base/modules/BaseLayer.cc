//
// Copyright (C) 2006 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// author:      Andreas Koepke
// part of:     framework implementation developed by tkn
// description: basic MAC layer class
//              subclass to create your own MAC layer

#include "veins/base/modules/BaseLayer.h"

using namespace veins;

const simsignal_t BaseLayer::catPassedMsgSignal = registerSignal("org_car2x_veins_base_utils_passedmsg");
const simsignal_t BaseLayer::catPacketSignal = registerSignal("org_car2x_veins_modules_utility_packet");
const simsignal_t BaseLayer::catDroppedPacketSignal = registerSignal("org_car2x_veins_modules_utility_droppedpacket");

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseModule.
 * This module takes care of the gate initialization.
 *
 **/
void BaseLayer::initialize(int stage)
{
    BatteryAccess::initialize(stage);
    if (stage == 0) {
        passedMsg = nullptr;
        if (hasPar("stats") && par("stats").boolValue()) {
            passedMsg = new PassedMessage();
            if (passedMsg != nullptr) {
                passedMsg->fromModule = getId();
            }
        }
        upperLayerIn = findGate("upperLayerIn");
        upperLayerOut = findGate("upperLayerOut");
        lowerLayerIn = findGate("lowerLayerIn");
        lowerLayerOut = findGate("lowerLayerOut");
        upperControlIn = findGate("upperControlIn");
        upperControlOut = findGate("upperControlOut");
        lowerControlIn = findGate("lowerControlIn");
        lowerControlOut = findGate("lowerControlOut");
    }
}

/**
 * The basic handle message function.
 *
 * Depending on the gate a message arrives handleMessage just calls
 * different handle*Msg functions to further process the message.
 *
 * You should not make any changes in this function but implement all
 * your functionality into the handle*Msg functions called from here.
 *
 * @sa handleUpperMsg, handleLowerMsg, handleSelfMsg
 **/
void BaseLayer::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
    }
    else if (msg->getArrivalGateId() == upperLayerIn) {
        recordPacket(PassedMessage::INCOMING, PassedMessage::UPPER_DATA, msg);
        handleUpperMsg(msg);
    }
    else if (msg->getArrivalGateId() == upperControlIn) {
        recordPacket(PassedMessage::INCOMING, PassedMessage::UPPER_CONTROL, msg);
        handleUpperControl(msg);
    }
    else if (msg->getArrivalGateId() == lowerControlIn) {
        recordPacket(PassedMessage::INCOMING, PassedMessage::LOWER_CONTROL, msg);
        handleLowerControl(msg);
    }
    else if (msg->getArrivalGateId() == lowerLayerIn) {
        recordPacket(PassedMessage::INCOMING, PassedMessage::LOWER_DATA, msg);
        handleLowerMsg(msg);
    }
    else if (msg->getArrivalGateId() == -1) {
        /* Classes extending this class may not use all the gates, f.e.
         * BaseApplLayer has no upper gates. In this case all upper gate-
         * handles are initialized to -1. When getArrivalGateId() equals -1,
         * it would be wrong to forward the message to one of these gates,
         * as they actually don't exist, so raise an error instead.
         */
        throw cRuntimeError("No self message and no gateID?? Check configuration.");
    }
    else {
        /* msg->getArrivalGateId() should be valid, but it isn't recognized
         * here. This could signal the case that this class is extended
         * with extra gates, but handleMessage() isn't overridden to
         * check for the new gate(s).
         */
        throw cRuntimeError("Unknown gateID?? Check configuration or override handleMessage().");
    }
}

void BaseLayer::sendDown(cMessage* msg)
{
    recordPacket(PassedMessage::OUTGOING, PassedMessage::LOWER_DATA, msg);
    send(msg, lowerLayerOut);
}

void BaseLayer::sendUp(cMessage* msg)
{
    recordPacket(PassedMessage::OUTGOING, PassedMessage::UPPER_DATA, msg);
    send(msg, upperLayerOut);
}

void BaseLayer::sendControlUp(cMessage* msg)
{
    recordPacket(PassedMessage::OUTGOING, PassedMessage::UPPER_CONTROL, msg);
    if (gate(upperControlOut)->isPathOK())
        send(msg, upperControlOut);
    else {
        EV << "BaseLayer: upperControlOut is not connected; dropping message" << std::endl;
        delete msg;
    }
}

void BaseLayer::sendControlDown(cMessage* msg)
{
    recordPacket(PassedMessage::OUTGOING, PassedMessage::LOWER_CONTROL, msg);
    if (gate(lowerControlOut)->isPathOK())
        send(msg, lowerControlOut);
    else {
        EV << "BaseLayer: lowerControlOut is not connected; dropping message" << std::endl;
        delete msg;
    }
}

void BaseLayer::recordPacket(PassedMessage::direction_t dir, PassedMessage::gates_t gate, const cMessage* msg)
{
    if (passedMsg == nullptr) return;
    passedMsg->direction = dir;
    passedMsg->gateType = gate;
    passedMsg->kind = msg->getKind();
    passedMsg->name = msg->getName();
    emit(catPassedMsgSignal, passedMsg);
}

void BaseLayer::finish()
{
}

BaseLayer::~BaseLayer()
{
    if (passedMsg != nullptr) {
        delete passedMsg;
    }
}
