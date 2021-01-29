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

/*
 * BaseDecider.h
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#pragma once

#include "veins/veins.h"

#include "veins/base/phyLayer/Decider.h"

namespace veins {

/**
 * @brief Provides some base functionality for most common deciders.
 *
 * Forwards the AirFrame from "processSignal" to "processNewSignal",
 * "processSignalHeader" or "processSignalEnd" depending on the
 * state for that AirFrame returned by "getSignalState".
 *
 * If a subclassing Decider only tries to receive one signal at a time
 * it can use BaseDeciders "currentSignal" member which is a pair of
 * the signal to receive and the state for that signal. The state
 * is then used by BaseDeciders "getSignalState" to decide to which
 * "process***" method to forward the signal.
 * If a subclassing Decider needs states for more than one Signal it
 * has to store these states by itself and should probably override
 * the "getSignalState" method.
 *
 * @ingroup decider
 * @ingroup baseModules
 */
class VEINS_API BaseDecider : public Decider {
public:
    /**
     * @brief The kinds of ControlMessages this Decider sends.
     *
     * Sub-classing decider should begin their own kind enumeration
     * at the value of "LAST_BASE_DECIDER_CONTROL_KIND".
     */
    enum BaseDeciderControlKinds {
        /** @brief The phy has recognized a bit error in the packet.*/
        PACKET_DROPPED = 22100,
        /** @brief Sub-classing decider should begin their own kinds at this
         * value.*/
        LAST_BASE_DECIDER_CONTROL_KIND
    };

protected:
    /** @brief The current state of processing for a signal*/
    enum SignalState {
        /** @brief Signal is received the first time. */
        NEW,
        /** @brief Waiting for the header of the signal. */
        EXPECT_HEADER,
        /** @brief Waiting for the end of the signal. */
        EXPECT_END,
    };

    /** @brief minPowerLevel value for receiving an AirFrame */
    double minPowerLevel;

    /** @brief Pair of a AirFrame and the state it is in. */
    typedef std::pair<AirFrame*, int> ReceivedSignal;

    /** @brief pointer to the currently received AirFrame */
    ReceivedSignal currentSignal;

    /** @brief Stores the idle state of the channel.*/
    bool isChannelIdle;

    /** @brief index for this Decider-instance given by Phy-Layer (mostly
     * Host-index) */
    int myIndex;

public:
    /**
     * @brief Initializes the decider with the passed values.
     *
     * Needs a pointer to its physical layer, the minPowerLevel, and the index of the host.
     */
    BaseDecider(cComponent* owner, DeciderToPhyInterface* phy, double minPowerLevel, int myIndex)
        : Decider(owner, phy)
        , minPowerLevel(minPowerLevel)
        , isChannelIdle(true)
        , myIndex(myIndex)
    {
        currentSignal.first = 0;
        currentSignal.second = NEW;
    }

    ~BaseDecider() override
    {
    }

public:
    /**
     * @brief Processes an AirFrame given by the PhyLayer
     *
     * Returns the time point when the decider wants to be given the AirFrame
     * again.
     */
    simtime_t processSignal(AirFrame* frame) override;

protected:
    /**
     * @brief Processes a new Signal. Returns the time it wants to
     * handle the signal again.
     *
     * Default implementation checks if the signals receiving power
     * is above the minPowerLevel of the radio and we are not already trying
     * to receive another AirFrame. If thats the case it waits for the end
     * of the signal.
     */
    virtual simtime_t processNewSignal(AirFrame* frame);

    /**
     * @brief Processes the end of the header of a received Signal.
     *
     * Returns the time it wants to handle the signal again.
     *
     * Default implementation does not handle signal headers.
     */
    virtual simtime_t processSignalHeader(AirFrame* frame)
    {
        throw cRuntimeError("BaseDecider does not handle Signal headers!");
        return notAgain;
    }

    /**
     * @brief Processes the end of a received Signal.
     *
     * Returns the time it wants to handle the signal again
     * (most probably notAgain).
     *
     * Default implementation just decides every signal as correct and passes it
     * to the upper layer.
     */
    virtual simtime_t processSignalEnd(AirFrame* frame);

    /**
     * @brief Processes any Signal for which no state could be found.
     * (is an error case).
     */
    virtual simtime_t processUnknownSignal(AirFrame* frame);

    /**
     * @brief Returns the SignalState for the passed AirFrame.
     *
     * The default implementation checks if the passed AirFrame
     * is the "currentSignal" and returns its state or if not
     * "NEW".
     */
    virtual int getSignalState(AirFrame* frame);

    /**
     * @brief Changes the "isIdle"-status to the passed value.
     */
    virtual void setChannelIdleStatus(bool isIdle);

    /**
     * @brief Collects the AirFrame on the channel during the passed interval.
     *
     * Forwards to DeciderToPhyInterfaces "getChannelInfo" method.
     * Subclassing deciders can override this method to filter the returned
     * AirFrames for their own criteria, for example by removing AirFrames on
     * another not interferring channel.
     *
     * @param start The start of the interval to collect AirFrames from.
     * @param end The end of the interval to collect AirFrames from.
     * @param out The output vector in which to put the AirFrames.
     */
    virtual void getChannelInfo(simtime_t_cref start, simtime_t_cref end, AirFrameVector& out);
};

} // namespace veins
