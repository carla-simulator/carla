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
 * BaseDecider.cc
 *
 *  Created on: 24.02.2009
 *      Author: karl
 */

#include "veins/base/phyLayer/BaseDecider.h"
#include "veins/base/messages/AirFrame_m.h"

using namespace veins;

simtime_t BaseDecider::processSignal(AirFrame* frame)
{

    ASSERT(frame);
    EV_TRACE << "Processing AirFrame..." << endl;

    switch (getSignalState(frame)) {
    case NEW:
        return processNewSignal(frame);
    case EXPECT_HEADER:
        return processSignalHeader(frame);
    case EXPECT_END:
        return processSignalEnd(frame);
    default:
        return processUnknownSignal(frame);
    }
}

simtime_t BaseDecider::processNewSignal(AirFrame* frame)
{
    if (currentSignal.first != 0) {
        EV_TRACE << "Already receiving another AirFrame!" << endl;
        return notAgain;
    }

    // get the receiving power of the Signal at start-time
    Signal& signal = frame->getSignal();
    double recvPower = signal.getMax();

    // check whether signal is strong enough to receive
    if (recvPower < minPowerLevel) {
        EV_TRACE << "Signal is too weak (" << recvPower << " < " << minPowerLevel << ") -> do not receive." << endl;
        // Signal too weak, we can't receive it, tell PhyLayer that we don't want it again
        return notAgain;
    }

    // Signal is strong enough, receive this Signal and schedule it
    EV_TRACE << "Signal is strong enough (" << recvPower << " > " << minPowerLevel << ") -> Trying to receive AirFrame." << endl;

    currentSignal.first = frame;
    currentSignal.second = EXPECT_END;

    // channel turned busy
    setChannelIdleStatus(false);

    return signal.getReceptionEnd();
}

simtime_t BaseDecider::processSignalEnd(AirFrame* frame)
{
    EV_INFO << "packet was received correctly, it is now handed to upper layer...\n";
    phy->sendUp(frame, new DeciderResult(true));

    // we have processed this AirFrame and we prepare to receive the next one
    currentSignal.first = 0;

    // channel is idle now
    setChannelIdleStatus(true);

    return notAgain;
}

simtime_t BaseDecider::processUnknownSignal(AirFrame* frame)
{
    throw cRuntimeError("Unknown state for the AirFrame with ID %d", frame->getId());
    return notAgain;
}

int BaseDecider::getSignalState(AirFrame* frame)
{
    if (frame == currentSignal.first) return currentSignal.second;

    return NEW;
}

void BaseDecider::setChannelIdleStatus(bool isIdle)
{
    isChannelIdle = isIdle;
}

void BaseDecider::getChannelInfo(simtime_t_cref start, simtime_t_cref end, AirFrameVector& out)
{
    phy->getChannelInfo(start, end, out);
}
