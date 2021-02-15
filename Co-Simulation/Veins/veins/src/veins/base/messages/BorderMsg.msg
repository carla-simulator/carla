//
// Copyright (C) 2006 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
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

cplusplus {{
#include "veins/base/utils/Coord.h"
}}

namespace veins;

class noncobject Coord;

// used as a self packet containing the position, direction and BorderPolicy
//
// @author Daniel Willkomm
//
packet BorderMsg
{
    int policy;    // border policy to use for the host
    Coord startPos; // new start position of the host
    Coord direction; // new direction of the host
}
