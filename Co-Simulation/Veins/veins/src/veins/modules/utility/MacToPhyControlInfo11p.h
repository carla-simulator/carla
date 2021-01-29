//
// Copyright (C) 2018-2019 Dominik S. Buse <buse@ccs-labs.org>
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

#include "veins/modules/utility/ConstsPhy.h"
#include "veins/modules/utility/Consts80211p.h"

namespace veins {

/**
 * Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
struct VEINS_API MacToPhyControlInfo11p : public cObject {
    Channel channelNr; ///< Channel number/index used to select frequency.
    MCS mcs; ///< The modulation and coding scheme to employ for the associated frame.
    double txPower_mW; ///< Transmission power in milliwatts.

    MacToPhyControlInfo11p(Channel channelNr, MCS mcs, double txPower_mW)
        : channelNr(channelNr)
        , mcs(mcs)
        , txPower_mW(txPower_mW)
    {
    }
};

} // namespace veins
