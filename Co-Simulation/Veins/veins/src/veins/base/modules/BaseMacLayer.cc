//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

// author:      Daniel Willkomm
// part of:     framework implementation developed by tkn
// description: basic MAC layer class
//              subclass to create your own MAC layer

#include "veins/base/modules/BaseMacLayer.h"

#include <sstream>

#include "veins/base/phyLayer/MacToPhyInterface.h"
#include "veins/base/utils/MacToNetwControlInfo.h"
#include "veins/base/utils/NetwToMacControlInfo.h"
#include "veins/base/modules/AddressingInterface.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/messages/MacPkt_m.h"

using namespace veins;

Define_Module(veins::BaseMacLayer);

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseLayer.
 *
 **/
void BaseMacLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);

    if (stage == 0) {
        // get handle to phy layer
        if ((phy = FindModule<MacToPhyInterface*>::findSubModule(getParentModule())) == nullptr) {
            throw cRuntimeError("Could not find a PHY module.");
        }

        headerLength = par("headerLength");
    }
    if (myMacAddr == LAddress::L2NULL()) {
        // see if there is an addressing module available
        // otherwise use NIC modules id as MAC address
        AddressingInterface* addrScheme = FindModule<AddressingInterface*>::findSubModule(findHost());
        if (addrScheme) {
            myMacAddr = addrScheme->myMacAddr(this);
        }
        else {
            const std::string addressString = par("address").stringValue();
            if (addressString.empty() || addressString == "auto")
                myMacAddr = LAddress::L2Type(getParentModule()->getId());
            else
                myMacAddr = 0;
            // use streaming operator for string conversion, this makes it more
            // independent from the myMacAddr type
            std::ostringstream oSS;
            oSS << myMacAddr;
            par("address").setStringValue(oSS.str());
        }
        registerInterface();
    }
}

void BaseMacLayer::registerInterface()
{
}

/**
 * Decapsulates the network packet from the received MacPkt
 **/
cPacket* BaseMacLayer::decapsMsg(MacPkt* msg)
{
    cPacket* m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    // delete the macPkt
    delete msg;
    EV_TRACE << " message decapsulated " << endl;
    return m;
}

/**
 * Encapsulates the received NetwPkt into a MacPkt and set all needed
 * header fields.
 **/
MacPkt* BaseMacLayer::encapsMsg(cPacket* netwPkt)
{
    MacPkt* pkt = new MacPkt(netwPkt->getName(), netwPkt->getKind());
    pkt->setBitLength(headerLength);

    // copy dest address from the Control Info attached to the network
    // message by the network layer
    cObject* cInfo = netwPkt->removeControlInfo();

    EV_TRACE << "CInfo removed, mac addr=" << getUpperDestinationFromControlInfo(cInfo) << endl;
    pkt->setDestAddr(getUpperDestinationFromControlInfo(cInfo));

    // delete the control info
    delete cInfo;

    // set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(myMacAddr);

    // encapsulate the network packet
    pkt->encapsulate(netwPkt);
    EV_TRACE << "pkt encapsulated\n";

    return pkt;
}

/**
 * Redefine this function if you want to process messages from upper
 * layers before they are send to lower layers.
 *
 * To forward the message to lower layers after processing it please
 * use @ref sendDown. It will take care of anything needed
 **/
void BaseMacLayer::handleUpperMsg(cMessage* mac)
{
    ASSERT(dynamic_cast<cPacket*>(mac));
    sendDown(encapsMsg(static_cast<cPacket*>(mac)));
}

/**
 * This basic implementation just forwards all message that are
 * broadcast (destAddr = L2BROADCAST) or destined for this node
 * (destAddr = nic module getId()) to the network layer
 *
 * @sa sendUp
 **/

void BaseMacLayer::handleLowerMsg(cMessage* msg)
{
    MacPkt* mac = static_cast<MacPkt*>(msg);
    LAddress::L2Type dest = mac->getDestAddr();
    LAddress::L2Type src = mac->getSrcAddr();

    // only foward to upper layer if message is for me or broadcast
    if ((dest == myMacAddr) || LAddress::isL2Broadcast(dest)) {
        EV_TRACE << "message with mac addr " << src << " for me (dest=" << dest << ") -> forward packet to upper layer\n";
        sendUp(decapsMsg(mac));
    }
    else {
        EV_TRACE << "message with mac addr " << src << " not for me (dest=" << dest << ") -> delete (my MAC=" << myMacAddr << ")\n";
        delete mac;
    }
}

void BaseMacLayer::handleLowerControl(cMessage* msg)
{
    switch (msg->getKind()) {
    case MacToPhyInterface::TX_OVER:
        msg->setKind(TX_OVER);
        sendControlUp(msg);
        break;
    default:
        EV << "BaseMacLayer does not handle control messages of this type (name was " << msg->getName() << ")\n";
        delete msg;
        break;
    }
}

BaseConnectionManager* BaseMacLayer::getConnectionManager()
{
    cModule* nic = getParentModule();
    return ChannelAccess::getConnectionManager(nic);
}

const LAddress::L2Type& BaseMacLayer::getUpperDestinationFromControlInfo(const cObject* const pCtrlInfo)
{
    return NetwToMacControlInfo::getDestFromControlInfo(pCtrlInfo);
}

/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject* const BaseMacLayer::setUpControlInfo(cMessage* const pMsg, const LAddress::L2Type& pSrcAddr)
{
    return MacToNetwControlInfo::setControlInfo(pMsg, pSrcAddr);
}
