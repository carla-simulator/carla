//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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

namespace veins {

/**
 * TraCI compatible color container
 */
class VEINS_API TraCIColor {
public:
    TraCIColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
    static TraCIColor fromTkColor(std::string tkColorName);

public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

} // namespace veins
