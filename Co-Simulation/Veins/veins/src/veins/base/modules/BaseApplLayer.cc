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
// description: application layer: general class for the application layer
//              subclass to create your own application layer

#include "veins/base/modules/BaseApplLayer.h"
#include "veins/base/utils/PassedMessage.h"

using namespace veins;

/**
 * First we have to initialize the module from which we derived ours,
 * in this case BaseLayer.
 *
 * Then we have to intialize the gates and - if necessary - some own
 * variables.
 **/
void BaseApplLayer::initialize(int stage)
{
    BaseLayer::initialize(stage);
    if (stage == 0) {
        headerLength = par("headerLength");
    }
}

/**
 * Send message down to lower layer
 **/
void BaseApplLayer::sendDelayedDown(cMessage* msg, simtime_t_cref delay)
{
    recordPacket(PassedMessage::OUTGOING, PassedMessage::LOWER_DATA, msg);
    sendDelayed(msg, delay, lowerLayerOut);
}
