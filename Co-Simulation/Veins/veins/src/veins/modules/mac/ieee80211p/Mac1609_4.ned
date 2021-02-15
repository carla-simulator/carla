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

package org.car2x.veins.modules.mac.ieee80211p;

import org.car2x.veins.base.modules.BaseMacLayer;

//
// Manages timeslots for CCH and SCH listening and sending.
//
// @author Christopher Saloman
// @author David Eckhoff : rewrote complete model
//
// @see DemoBaseApplLayer
// @see Nic80211p
// @see PhyLayer80211p
//
simple Mac1609_4 extends BaseMacLayer
{
    parameters:
        @class(veins::Mac1609_4);

        // cycle between an SCH and the CCH (or stay on CCH all the time)
        bool useServiceChannel = default(true);

        // the service channel this maclayer listens on when started
        int serviceChannel = default(1);

        // length of MAC header, header is already added at 1609_4
        headerLength @unit(bit) = default(0 bit);

        // bit rate
        int bitrate @unit(bps) = default(6 Mbps);

        // maximum artificial asynchronization between cars to avoid synchronization effects
        double syncOffset @unit(s) = default(0.0003s);

        //tx power [mW]
        double txPower @unit(mW);

        //the maximum queue size of an EDCA queue in the MAC. 0 for unlimited. Queue strategy is "drop if full"
        int queueSize = default(0);

        // unicast parameters
        int dot11RTSThreshold @unit(bit) = default(12000bit);
        int dot11ShortRetryLimit = default(7);
        int dot11LongRetryLimit = default(4);
        int ackLength @unit(bit) = default(112bit);
        bool useAcks = default(false);
        double ackErrorRate = default(0.20);

        // signal informing interested application about channel busy state
        @signal[org_car2x_veins_modules_mac_sigChannelBusy](type=bool);
        // signal informing interested application about a collision
        @signal[org_car2x_veins_modules_mac_sigCollision](type=bool);

}
