//
// Copyright (C) 2018 Dominik S. Buse <buse@ccs-labs.org>
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

#include "veins/base/utils/EnumBitset.h"

namespace veins {

enum class VehicleSignal : uint32_t {
    blinker_right,
    blinker_left,
    blinker_emergency,
    brakelight,
    frontlight,
    foglight,
    highbeam,
    backdrive,
    wiper,
    door_open_left,
    door_open_right,
    emergency_blue,
    emergency_red,
    emergency_yellow,
    undefined
};

template <>
struct VEINS_API EnumTraits<VehicleSignal> {
    static const VehicleSignal max = VehicleSignal::undefined;
};

using VehicleSignalSet = EnumBitset<VehicleSignal>;

} // namespace veins
