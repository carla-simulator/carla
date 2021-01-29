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

// author:      Marc Loebbers
// part of:     framework implementation developed by tkn
// description: - Base class for physical layers
//              - if you create your own physical layer, please subclass
//                from this class and use the sendToChannel() function!!

#pragma once

#include <vector>

#include "veins/veins.h"

#include "veins/base/utils/AntennaPosition.h"
#include "veins/base/modules/BatteryAccess.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/Heading.h"

namespace veins {

using ChannelMobilityAccessType = AccessModuleWrap<BaseMobility>;
using ChannelMobilityPtrType = ChannelMobilityAccessType::wrapType*;
class NicEntry;
class BaseConnectionManager;
class BaseWorldUtility;

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ConnectionManager to handle
 * the dynamically created gates. This means EVERY physical layer (the lowest
 * layer in a host) has to be derived from this class!!!!
 *
 * Please don't touch this class.
 *
 * @author Marc Loebbers
 * @ingroup connectionManager
 * @ingroup phyLayer
 * @ingroup baseModules
 **/
class VEINS_API ChannelAccess : public BatteryAccess, protected ChannelMobilityAccessType {
protected:
    /** @brief use sendDirect or not?*/
    bool useSendDirect;

    /** @brief Pointer to the PropagationModel module*/
    BaseConnectionManager* cc;

    /** @brief Defines if the physical layer should simulate propagation delay.*/
    bool usePropagationDelay;

    /** @brief Is this module already registered with ConnectionManager? */
    bool isRegistered;

    /** @brief Pointer to the World Utility, to obtain some global information*/
    BaseWorldUtility* world;

    /** @brief Current antenna position */
    AntennaPosition antennaPosition;

    /** @brief Current antenna heading (angle) */
    Heading antennaHeading;

    /** @brief Offset of antenna position (in m) with respect to what a BaseMobility module will tell us */
    Coord antennaOffset = Coord(0, 0, 0);

    /** @brief Offset of antenna orientation (yaw, in rad) with respect to what a BaseMobility module will tell us */
    double antennaOffsetYaw = 0;

protected:
    /**
     * @brief Calculates the propagation delay to the passed receiving nic.
     */
    simtime_t calculatePropagationDelay(const NicEntry* nic);

    /** @brief Sends a message to all nics connected to this one.
     *
     * This function has to be called whenever a packet is supposed to be
     * sent to the channel. Don't try to figure out what gates you have
     * and which ones are connected, this function does this for you!
     *
     * depending on which ConnectionManager module is used, the messages are
     * send via sendDirect() or to the respective gates.
     **/
    void sendToChannel(cPacket* msg);

public:
    /**
     * @brief Returns a pointer to the ConnectionManager responsible for the
     * passed NIC module.
     *
     * @param nic a pointer to a NIC module
     * @return a pointer to a connection manager module or NULL if an error
     * occurred
     */
    static BaseConnectionManager* getConnectionManager(cModule* nic);

    /** @brief Register with ConnectionManager.
     *
     * Upon initialization ChannelAccess registers the nic parent module
     * to have all its connections handeled by ConnectionManager
     **/
    void initialize(int stage) override;

    /**
     * @brief Called by the signalling mechanism to inform of changes.
     *
     * ChannelAccess is subscribed to position changes and informs the
     * ConnectionManager.
     */
    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

    /**
     * @brief Returns the host's mobility module.
     */
    virtual ChannelMobilityPtrType getMobilityModule()
    {
        return ChannelMobilityAccessType::get(this);
    }

    virtual AntennaPosition getAntennaPosition() const
    {
        return antennaPosition;
    }
};

} // namespace veins
