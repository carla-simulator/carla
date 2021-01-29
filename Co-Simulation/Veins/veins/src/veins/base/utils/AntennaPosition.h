//
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

#include "veins/veins.h"

#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * Stores the position of the host's antenna along with its speed, so that it can be linearly extrapolated.
 */
class VEINS_API AntennaPosition {

public:
    AntennaPosition()
        : id(-1)
        , p()
        , v()
        , t()
        , undef(true)
    {
    }

    /**
     * Store a position p that changes by v for every second after t.
     */
    AntennaPosition(int id, Coord p, Coord v, simtime_t t)
        : id(id)
        , p(p)
        , v(v)
        , t(t)
        , undef(false)
    {
    }

    /**
     * Get the (linearly extrapolated) position at time t.
     */
    Coord getPositionAt(simtime_t t = simTime()) const
    {
        ASSERT(t >= this->t);
        ASSERT(!undef);
        auto dt = t - this->t;
        return p + v * dt.dbl();
    }

    bool isSameAntenna(const AntennaPosition& o) const
    {
        ASSERT(!undef);
        ASSERT(!o.undef);
        return (id == o.id);
    }

protected:
    int id; /**< unique identifier of antenna returned by ChannelAccess::getId() */
    Coord p; /**< position for linear extrapolation */
    Coord v; /**< speed for linear extrapolation */
    simtime_t t; /**< time for linear extrapolation */
    bool undef; /**< true if created using default constructor */
};

} // namespace veins
