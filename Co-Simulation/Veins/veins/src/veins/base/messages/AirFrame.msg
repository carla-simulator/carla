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

cplusplus {{
#include "veins/base/toolbox/Signal.h"
#include "veins/base/utils/POA.h"
}}

namespace veins;

class noncobject Signal;
class noncobject POA;

//
// Format of the packets that are sent to the channel
//
// subclass if you want to create your own AirFrame packet class
// (see Omnet manual, chapter Messages - Inheritance among packet
// classes).
//
// This packet format is used to send a packet from the physical
// layer to the channel. All other physical layers that can 'hear'
// this packet can evaluate noise, interference etc. from the
// information contained in this packet (especially the signal):
//
// Note: BasePhy layer creates AirFrames with a slightly higher
// priority than normal to ensure channel consistency. This means that before anything else happens
// at a time point t every AirFrame which ended at t has been removed and
// every AirFrame started at t has been added to the channel.
//
// If you need more fields for whatever reason, please do NOT create
// your own packet! Just extend (subclass) this packet format
//
packet AirFrame
{
    Signal signal;        // Contains the physical data of this AirFrame

    POA poa;            // contains a POA object with the position, orientation and antenna (pointer)
                            // of the sender

    simtime_t duration;    // time the AirFrames takes to be transmited (without propagation delay)

    int state = 1;         // state of the AirFrames, used by the physical layer
                            // as state machine for delay and transmission duration
                            // simulation

    int type = 0;        // If type isn't null then this is a control-AirFrame
                            // and type specifies the control type.

    long id;            // Unique ID of the AirFrame used as identifier for
                            // related control-AirFrames

    int protocolId;        //the id of the phy protocol of this airframe

    int channel;        //the channel of the radio used for this transmission
    int mcs; // Modulation and conding scheme of the packet
}
