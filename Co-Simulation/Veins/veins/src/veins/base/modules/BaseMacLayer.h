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

#pragma once

#include "veins/veins.h"

#include "veins/base/modules/BaseLayer.h"
#include "veins/base/utils/SimpleAddress.h"

namespace veins {

class BaseConnectionManager;
class MacPkt;
class MacToPhyInterface;

/**
 * @brief A very simple MAC module template which provides de- and
 * encapsulation of messages using the standard addresses. It also
 * provides basic handling of lower layer messages.
 *
 * @ingroup macLayer
 * @ingroup baseModules
 * @author Daniel Willkomm, Karl Wessel
 */
class VEINS_API BaseMacLayer : public BaseLayer {
public:
    /** @brief Message kinds used by this layer.*/
    enum BaseMacMessageKinds {
        /** Stores the id on which classes extending BaseMac should
         * continue their own message kinds.*/
        LAST_BASE_MAC_MESSAGE_KIND = 23000,
    };
    /** @brief Control message kinds used by this layer.*/
    enum BaseMacControlKinds {
        /** Indicates the end of a transmission*/
        TX_OVER = 23500,
        /** Tells the netw layer that a packet to be sent has been dropped.*/
        PACKET_DROPPED,
        /** Stores the id on which classes extending BaseMac should
         * continue their own control kinds.*/
        LAST_BASE_MAC_CONTROL_KIND,
    };

protected:
    /** @brief Handler to the physical layer.*/
    MacToPhyInterface* phy;

    /** @brief Pointer to the arp module*/
    // BaseArp* arp;

    /**
     * @brief Length of the MacPkt header
     **/
    int headerLength;

    /**
     * @brief MAC address.
     **/
    LAddress::L2Type myMacAddr;

public:
    // Module_Class_Members( BaseMacLayer, BaseLayer, 0 );
    BaseMacLayer()
        : BaseLayer()
        , phy(nullptr)
        , myMacAddr(LAddress::L2NULL())
    {
    }
    BaseMacLayer(unsigned stacksize)
        : BaseLayer(stacksize)
        , phy(nullptr)
        , myMacAddr(LAddress::L2NULL())
    {
    }

    /** @brief Initialization of the module and some variables*/
    void initialize(int) override;

    /**
     * @brief Returns the MAC address of this MAC module.
     */
    const LAddress::L2Type& getMACAddress()
    {
        ASSERT(myMacAddr != LAddress::L2NULL());
        return myMacAddr;
    }

protected:
    /**
     * @brief Registers this bridge's NIC with INET's InterfaceTable.
     */
    virtual void registerInterface();

    /**
     * @brief Handle messages comming from the network layer
     *
     * Here is the place to implement a real medium access functionality.
     *
     * If message arrives from upper layer, make a MAC packet from it
     * (encapsMsg) and send it down (sendDown).
     *
     *  @sa encapsMsg, sendDown
     */
    void handleUpperMsg(cMessage* msg) override;

    /**
     * If message arrives from lower layer, check whether it is for
     * us. Send it up if yes.
     */
    void handleLowerMsg(cMessage* msg) override;

    void handleSelfMsg(cMessage* msg) override
    {
        throw cRuntimeError("BaseMacLayer does not handle self messages");
    };
    void handleLowerControl(cMessage* msg) override;

    void handleUpperControl(cMessage* msg) override
    {
        throw cRuntimeError("BaseMacLayer does not handle control messages from upper layers");
    };

    /** @brief decapsulate the network message from the MacPkt */
    virtual cPacket* decapsMsg(MacPkt*);

    /** @brief Encapsulate the NetwPkt into an MacPkt */
    virtual MacPkt* encapsMsg(cPacket*);

    /**
     * @brief Returns a pointer to this MACs NICs ConnectionManager module.
     * @return pointer to the connection manager module
     */
    BaseConnectionManager* getConnectionManager();

    /**
     * @brief Extracts the MAC address from the "control info" structure (object).
     *
     * Extract the destination MAC address from the "control info" which was prev. set by NetwToMacControlInfo::setControlInfo().
     *
     * @param pCtrlInfo    The "control info" structure (object) prev. set by NetwToMacControlInfo::setControlInfo().
     * @return The MAC address of message receiver.
     */
    virtual const LAddress::L2Type& getUpperDestinationFromControlInfo(const cObject* const pCtrlInfo);

    /**
     * @brief Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg        The message where the "control info" shall be attached.
     * @param pSrcAddr    The MAC address of the message receiver.
     */
    virtual cObject* const setUpControlInfo(cMessage* const pMsg, const LAddress::L2Type& pSrcAddr);
};

} // namespace veins
