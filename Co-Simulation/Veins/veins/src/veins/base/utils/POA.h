//
// Copyright (C) 2016 Alexander Brummer <alexander.brummer@fau.de>
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <memory>

#include "veins/base/phyLayer/Antenna.h"
#include "veins/base/utils/AntennaPosition.h"
#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * @brief Container class used to attach data to Airframe s which are
 * needed by the receiver for antenna gain calculation (POA is short
 * for position, orientation, antenna).
 *
 * @author Alexander Brummer
 */
class VEINS_API POA {
public:
    /**
     * Stores the sender's position.
     */
    AntennaPosition pos;

    /**
     * Saves the sender's orientation.
     */
    Coord orientation;

    /**
     * Shared pointer to the sender's antenna, which is necessary for
     * the receiver to calculate the gain of the transmitting antenna.
     * It is a shared pointer to ensure that the antenna still exists
     * even if the sending node is already gone.
     */
    std::shared_ptr<Antenna> antenna;

    POA(){};
    POA(AntennaPosition pos, Coord orientation, std::shared_ptr<Antenna> antenna)
        : pos(pos)
        , orientation(orientation)
        , antenna(antenna){};
    virtual ~POA(){};
};

} // namespace veins
