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

#include "veins/base/utils/NetwToMacControlInfo.h"
#include "veins/modules/utility/Consts80211p.h"

namespace veins {

/**
 * @brief
 * Interface between DemoBaseApplLayer Layer and Mac1609_4
 *
 * @author David Eckhoff
 *
 * @ingroup macLayer
 */
class VEINS_API DemoBaseApplLayerToMac1609_4Interface {
public:
    virtual bool isChannelSwitchingActive() = 0;

    virtual simtime_t getSwitchingInterval() = 0;

    virtual bool isCurrentChannelCCH() = 0;

    virtual void changeServiceChannel(Channel channelNumber) = 0;

    virtual ~DemoBaseApplLayerToMac1609_4Interface(){};

    /**
     * @brief Returns the MAC address of this MAC module.
     */
    virtual const LAddress::L2Type& getMACAddress() = 0;
};

} // namespace veins
