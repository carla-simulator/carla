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
// description: Class to store information about a nic for the
//              ConnectionManager module

#pragma once

#include <map>

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Heading.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace veins {

class ChannelAccess;

/**
 * @brief NicEntry is used by ConnectionManager to store the necessary
 * information for each nic
 *
 * @ingroup connectionManager
 * @author Daniel Willkomm
 * @sa ConnectionManager
 */
class VEINS_API NicEntry : public HasLogProxy {
protected:
    class VEINS_API NicEntryComparator {
    public:
        bool operator()(const NicEntry* nic1, const NicEntry* nic2) const
        {
            return nic1->nicId < nic2->nicId;
        }
    };

public:
    /** @brief Type for map from NicEntry pointer to a gate.*/
    typedef std::map<const NicEntry*, cGate*, NicEntryComparator> GateList;

    /** @brief module id of the nic for which information is stored*/
    int nicId;

    /** @brief Pointer to the NIC module */
    cModule* nicPtr;

    /** @brief Module id of the host module this nic belongs to*/
    int hostId;

    /** @brief Geographic location of the nic*/
    Coord pos;

    /** @brief Heading (angle) of the nic*/
    Heading heading;

    /** @brief Points to this nics ChannelAccess module */
    ChannelAccess* chAccess;

protected:
    /** @brief Outgoing connections of this nic
     *
     * This map stores all connection for this nic to other nics
     *
     * The first entry is the module id of the nic the connection is
     * going to and the second the gate to send the msg to
     **/
    GateList outConns;

public:
    /**
     * @brief Constructor, initializes all members
     */
    NicEntry(cComponent* owner)
        : HasLogProxy(owner)
        , nicId(0)
        , nicPtr(nullptr)
        , hostId(0){};

    /**
     * @brief Destructor -- needs to be there...
     */
    virtual ~NicEntry()
    {
    }

    /** @brief Connect two nics */
    virtual void connectTo(NicEntry*) = 0;

    /** @brief Disconnect two nics */
    virtual void disconnectFrom(NicEntry*) = 0;

    /** @brief return the actual gateList*/
    const GateList& getGateList()
    {
        return outConns;
    }

    /** @brief Checks if this nic is connected to the "other" nic*/
    bool isConnected(const NicEntry* other)
    {
        return (outConns.find(other) != outConns.end());
    };

    /**
     * Called by P2PPhyLayer. Needed to send a packet directly to a
     * certain nic without other nodes 'hearing' it. This is only useful
     * for physical layers that work with bit error probability like
     * P2PPhyLayer.
     *
     * @param to pointer to the NicEntry to which the packet is about to be sent
     */
    const cGate* getOutGateTo(const NicEntry* to)
    {
        return outConns[to];
    };
};

} // namespace veins
