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

package org.car2x.veins.modules.application.ieee80211p;
import org.car2x.veins.base.modules.IBaseApplLayer;

//
// Demo application layer base class.
//
// @author David Eckhoff
//
// @see DemoBaseApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple DemoBaseApplLayer like IBaseApplLayer
{
    parameters:
        @class(veins::DemoBaseApplLayer);
        int headerLength = default(88bit) @unit(bit); //header length of the application

        bool sendBeacons = default(true); //tell the applayer to periodically send beacons
        int beaconLengthBits = default(256bit) @unit(bit); //the length of a beacon packet
        int beaconUserPriority = default(7); //the user priority (UP) of the beacon messages
        double beaconInterval = default(1s) @unit(s); //the intervall between 2 beacon messages

        int dataLengthBits = default(1024bit) @unit(bit); //the length of a data packet
        bool dataOnSch = default(false); //tells the applayer whether to use a service channel for datapackets or the control channel
        int dataUserPriority = default(7); //the default user priority (UP) for data packets

        bool avoidBeaconSynchronization = default(true); //don't start beaconing directly after node was created but delay to avoid artifical synchronization

        bool sendWSA = default(false);
        int wsaLengthBits = default(250bit) @unit(bit);
        double wsaInterval =  default(1s) @unit(s);

    gates:
        input lowerLayerIn; // from mac layer
        output lowerLayerOut; // to mac layer
        input lowerControlIn;
        output lowerControlOut;
}
