//
// Copyright (C) 2014 Michele Segata <segata@ccs-labs.org>
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

namespace veins;

//
// Defines a control message that can be associated with a MAC frame to set
// transmission power and datarate on a per packet basis
//
message PhyControlMessage {
    //modulation and coding scheme to be used (see enum TxMCS in ConstsPhy.h)
    int mcs = -1;
    //transmission power to be used in mW
    double txPower_mW = -1;
}
