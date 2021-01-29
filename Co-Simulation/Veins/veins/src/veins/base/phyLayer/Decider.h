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

#include "veins/veins.h"

#include "veins/base/phyLayer/DeciderToPhyInterface.h"
#include "veins/modules/utility/HasLogProxy.h"

namespace veins {

/**
 * @brief A class to represent the result of a processed packet (that is not
 * noise) by the Decider.
 *
 * It stores information (i.e. basically whether a packet has been received
 * correctly) for the MACLayer that is handed up to the MACLayer by the PhyLayer
 * together with the received packet. (see also DeciderToPhyInterface)
 *
 * You can subclass DeciderResult to create a more detailed result.
 *
 * @ingroup decider
 */
class VEINS_API DeciderResult {
protected:
    /** Stores if the AirFrame for this result was received correct.*/
    bool isCorrect;

public:
    virtual ~DeciderResult()
    {
    }

    /**
     * @brief Initializes the DeciderResult with the passed bool, or true
     * if omitted.
     */
    DeciderResult(bool isCorrect = true)
        : isCorrect(isCorrect)
    {
    }

    /**
     * @brief A Function that returns a very basic result about the Signal.
     */
    virtual bool isSignalCorrect() const;
};

/**
 * @brief The basic Decider class
 *
 * The Deciders tasks are:
 *     1.    decide which packets should be handed up to the MAC Layer (primary task)
 *     2.    decide whether the channel is busy/idle at a time point or
 *         during a time interval (channel sensing)
 *
 * BasePhyLayer hands every receiving AirFrame several times to the
 * "processSignal()"-function and is returned a time point when to do so again.
 *
 * @ingroup decider
 */
class VEINS_API Decider : public HasLogProxy {
protected:
    /** @brief A pointer to the physical layer of this Decider. */
    DeciderToPhyInterface* phy;

    /** @brief simtime that tells the Phy-Layer not to pass an AirFrame again */
    const simtime_t notAgain;

    /** @brief Defines what an AirFrameVector shall be here */
    using AirFrameVector = DeciderToPhyInterface::AirFrameVector;

public:
    /**
     * @brief Initializes the Decider with a pointer to its PhyLayer
     */
    Decider(cComponent* owner, DeciderToPhyInterface* phy);

    virtual ~Decider()
    {
    }

    /**
     * @brief This function processes a AirFrame given by the PhyLayer and
     * returns the time point when Decider wants to be given the AirFrame again.
     */
    virtual simtime_t processSignal(AirFrame* frame);

    /**
     * @brief Method to be called by an OMNeT-module during its own finish(),
     * to enable a decider to do some things.
     */
    virtual void finish()
    {
    }

    /**
     * @brief Called by phy layer to indicate that the channel this radio
     * currently listens to has changed.
     *
     * Sub-classing deciders which support multiple channels should override
     * this method to handle the effects of channel changes on ongoing
     * receptions.
     *
     * @param newChannel The new channel the radio has changed to.
     */
    virtual void channelChanged(int newChannel)
    {
    }

    /**
     * @brief Notifies the decider that phy layer is starting a transmission.
     *
     * This helps the decider interrupting a current reception. In a standard
     * 802.11 MAC, this should never happen, but in other MAC layers you might
     * decide to interrupt an ongoing reception and start transmitting. Thank
     * to this method, the decider can flag the ongoing frame as non received
     * because of the transmission.
     */
    virtual void switchToTx()
    {
    }
};

} // namespace veins
