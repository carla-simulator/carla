//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

#pragma once

#include <vector>
#include <list>

#include "veins/veins.h"

namespace veins {

class AirFrame;

class BaseWorldUtility;

/**
 * See Decider.h for definition of DeciderResult
 */
class DeciderResult;

/**
 * @brief This class is the interface for a Decider to the BasePhyLayer.
 *
 * The Decider can do the following things using it:
 *
 *        - get the current simulation time
 *         - get the list of AirFrames that intersect with a specific time interval (to
 *             calculate SNR)
 *         - tell the BasePhyLayer to hand an AirFrame up to the MACLayer
 *         - tell the BasePhyLayer to send a control message to the MACLayer
 *
 * @ingroup decider
 */
class VEINS_API DeciderToPhyInterface {
public:
    /**
     * @brief Type for container of AirFrames.
     *
     * Used as out-value in "getChannelInfo" method.
     */
    using AirFrameVector = std::list<AirFrame*>;

    virtual ~DeciderToPhyInterface()
    {
    }

    /**
     * @brief Fills the passed AirFrameVector with all AirFrames that intersect
     * with the time interval [from, to]
     */
    virtual void getChannelInfo(simtime_t_cref from, simtime_t_cref to, AirFrameVector& out) = 0;

    /**
     * @brief Returns a constant which defines the noise floor in
     * the passed time frame (in mW).
     */
    virtual double getNoiseFloorValue() = 0;

    /**
     * @brief Called by the Decider to send a control message to the MACLayer
     */
    virtual void sendControlMsgToMac(cMessage* msg) = 0;

    /**
     * @brief Called to send an AirFrame with DeciderResult to the MACLayer
     *
     * When a packet is completely received and not noise, the Decider
     * call this function to send the packet together with
     * the corresponding DeciderResult up to MACLayer
     *
     */
    virtual void sendUp(AirFrame* packet, DeciderResult* result) = 0;

    /**
     * @brief Returns a pointer to the simulations world-utility-module.
     */
    virtual BaseWorldUtility* getWorldUtility() = 0;

    /**
     * @brief Utility method to enable a Decider, which isn't an OMNeT-module, to
     * use the OMNeT-method 'recordScalar' with the help of and through its interface to BasePhyLayer.
     *
     * The method-signature is taken from OMNeTs 'ccomponent.h' but made pure virtual here.
     * See the original method-description below:
     *
     * Records a double into the scalar result file.
     */
    virtual void recordScalar(const char* name, double value, const char* unit = nullptr) = 0;

    /** @brief Returns the channel currently used by the radio. */
    virtual int getCurrentRadioChannel() = 0;
};

} // namespace veins
