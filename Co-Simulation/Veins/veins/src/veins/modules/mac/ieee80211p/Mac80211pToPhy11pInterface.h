//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#include "veins/base/phyLayer/MacToPhyInterface.h"
#include "veins/modules/utility/ConstsPhy.h"
#include "veins/modules/utility/Consts80211p.h"

namespace veins {

/**
 * @brief
 * Interface of PhyLayer80211p exposed to Mac1609_4.
 *
 * @author Christopher Saloman
 * @author David Eckhoff
 *
 * @ingroup phyLayer
 */
class VEINS_API Mac80211pToPhy11pInterface {
public:
    enum BasePhyMessageKinds {
        CHANNEL_IDLE,
        CHANNEL_BUSY,
    };

    virtual ~Mac80211pToPhy11pInterface() = default;

    virtual void changeListeningChannel(Channel channel) = 0;
    virtual void setCCAThreshold(double ccaThreshold_dBm) = 0;
    virtual void notifyMacAboutRxStart(bool enable) = 0;
    virtual void requestChannelStatusIfIdle() = 0;
    virtual simtime_t getFrameDuration(int payloadLengthBits, MCS mcs) const = 0;
};

} // namespace veins
