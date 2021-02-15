//
// Copyright (C) 2005 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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
// description: - control info to pass next hop to the MAC layer

#pragma once

#include "veins/veins.h"

#include "veins/base/utils/SimpleAddress.h"

namespace veins {

/**
 * @brief Control info to pass next hop L2 addr from netw to MAC layer
 *
 * Control Info to pass interface information from the network to the
 * MAC layer Currently the only information necessary is the MAC
 * address of the next hop, which has to be determined by ARP or some
 * similar mechanism
 *
 * @ingroup baseUtils
 * @ingroup macLayer
 * @ingroup netwLayer
 * @author Daniel Willkomm
 **/
class VEINS_API NetwToMacControlInfo : public cObject {
protected:
    /** @brief MAC address of the sending or receiving node*/
    LAddress::L2Type nextHopMac;

public:
    /** @brief Default constructor*/
    NetwToMacControlInfo(const LAddress::L2Type& addr = LAddress::L2NULL())
        : cObject()
        , nextHopMac(addr){};

    /** @brief Destructor*/
    ~NetwToMacControlInfo() override {};

    /** @brief Getter method */
    virtual const LAddress::L2Type& getNextHopMac() const
    {
        return nextHopMac;
    };
    virtual const LAddress::L2Type& getDest() const
    {
        return nextHopMac;
    };

    /** @brief Setter method */
    virtual void setNextHopMac(const LAddress::L2Type& addr)
    {
        nextHopMac = addr;
    };
    virtual void setDest(const LAddress::L2Type& addr)
    {
        nextHopMac = addr;
    };

    /**
     * @brief Attaches a "control info" structure (object) to the message pMsg.
     *
     * This is most useful when passing packets between protocol layers
     * of a protocol stack, the control info will contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setL3ToL2ControlInfo() call throws an error).
     *
     * @param pMsg        The message where the "control info" shall be attached.
     * @param pDestAddr    The MAC address of the message receiver.
     */
    static cObject* const setControlInfo(cMessage* const pMsg, const LAddress::L2Type& pDestAddr);
    /**
     * @brief Extracts the MAC address from the "control info" structure (object).
     *
     * Extract the destination MAC address from the "control info" which was prev. set by NetwToMacControlInfo::setControlInfo().
     *
     * @param pCtrlInfo    The "control info" structure (object) prev. set by NetwToMacControlInfo::setControlInfo().
     * @return The MAC address of message receiver.
     */
    static const LAddress::L2Type& getDestFromControlInfo(const cObject* const pCtrlInfo);
};

} // namespace veins
