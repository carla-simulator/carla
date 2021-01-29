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
// description: - base class for MAC layer packets
//              - subclass to create your own MAC layer packets

cplusplus {{
#include "veins/base/utils/SimpleAddress.h"
}}

namespace veins;

class LAddress::L2Type extends void;

// A basic MAC (Media Access Control) packet format definition
// 
// subclass if you want to create your own MAC layer packet class
//
// The basic MAC packet only provides source and destination address
//
// @author Daniel Willkomm
packet MacPkt
{
    LAddress::L2Type destAddr; // destination mac address
    LAddress::L2Type srcAddr;  // source mac address
    long             sequenceId; // Sequence Number to detect duplicate messages
}
